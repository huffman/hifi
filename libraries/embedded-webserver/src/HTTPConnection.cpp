//
//  HTTPConnection.cpp
//  libraries/embedded-webserver/src
//
//  Created by Stephen Birarda on 1/16/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


#include <QBuffer>
#include <QCryptographicHash>
#include <QTcpSocket>

#include "HTTPConnection.h"
#include "EmbeddedWebserverLogging.h"
#include "HTTPManager.h"
#include <QUrlQuery>

const char* HTTPConnection::StatusCode200 = "200 OK";
const char* HTTPConnection::StatusCode301 = "301 Moved Permanently";
const char* HTTPConnection::StatusCode302 = "302 Found";
const char* HTTPConnection::StatusCode400 = "400 Bad Request";
const char* HTTPConnection::StatusCode401 = "401 Unauthorized";
const char* HTTPConnection::StatusCode403 = "403 Forbidden";
const char* HTTPConnection::StatusCode404 = "404 Not Found";
const char* HTTPConnection::StatusCode500 = "500 Internal server error";
const char* HTTPConnection::DefaultContentType = "text/plain; charset=ISO-8859-1";

HTTPConnection::HTTPConnection (QTcpSocket* socket, HTTPManager* parentManager) :
    QObject(parentManager),
    _parentManager(parentManager),
    _socket(socket),
    _stream(socket),
    _address(socket->peerAddress())
{
    // take over ownership of the socket
    _socket->setParent(this);

    // connect initial slots
    connect(socket, SIGNAL(readyRead()), SLOT(readRequest()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(deleteLater()));
    connect(socket, SIGNAL(disconnected()), SLOT(deleteLater()));
}

HTTPConnection::~HTTPConnection() {
    // log the destruction
    if (_socket->error() != QAbstractSocket::UnknownSocketError
        && _socket->error() != QAbstractSocket::RemoteHostClosedError) {
        qCDebug(embeddedwebserver) << _socket->errorString() << "-" << _socket->error();
    }
}

QHash<QString, QString> HTTPConnection::parseUrlEncodedForm() {
    // make sure we have the correct MIME type
    QList<QByteArray> elements = _requestHeaders.value("Content-Type").split(';');

    QString contentType = elements.at(0).trimmed();
    if (contentType != "application/x-www-form-urlencoded") {
        return QHash<QString, QString>();
    }

    QUrlQuery form { _requestContent };
    QHash<QString, QString> pairs;
    for (auto pair : form.queryItems()) {
        auto key = QUrl::fromPercentEncoding(pair.first.toLatin1().replace('+', ' '));
        auto value = QUrl::fromPercentEncoding(pair.second.toLatin1().replace('+', ' '));
        pairs[key] = value;
    }

    return pairs;
}

QList<FormData> HTTPConnection::parseFormData() const {
    // make sure we have the correct MIME type
    QList<QByteArray> elements = _requestHeaders.value("Content-Type").split(';');

    QString contentType = elements.at(0).trimmed();

    if (contentType != "multipart/form-data") {
        return QList<FormData>();
    }

    // retrieve the boundary marker
    QByteArray boundary;
    for (int ii = 1, nn = elements.size(); ii < nn; ii++) {
        QByteArray element = elements.at(ii).trimmed();
        if (element.startsWith("boundary")) {
            boundary = element.mid(element.indexOf('=') + 1).trimmed();
            break;
        }
    }
    
    QByteArray start = "--" + boundary;
    QByteArray end = "\r\n--" + boundary + "--\r\n";

    QList<FormData> data;
    QBuffer buffer(const_cast<QByteArray*>(&_requestContent));
    buffer.open(QIODevice::ReadOnly);
    while (buffer.canReadLine()) {
        QByteArray line = buffer.readLine().trimmed();
        if (line == start) {
            FormData datum;
            while (buffer.canReadLine()) {
                QByteArray line = buffer.readLine().trimmed();
                if (line.isEmpty()) {
                    // content starts after this line
                    int idx = _requestContent.indexOf(end, buffer.pos());
                    if (idx == -1) {
                        qWarning() << "Missing end boundary." << _address;
                        return data;
                    }
                    datum.second = _requestContent.mid(buffer.pos(), idx - buffer.pos());
                    data.append(datum);
                    buffer.seek(idx + end.length());

                } else {
                    // it's a header element
                    int idx = line.indexOf(':');
                    if (idx == -1) {
                        qWarning() << "Invalid header line." << _address << line;
                        continue;
                    }
                    datum.first.insert(line.left(idx).trimmed(), line.mid(idx + 1).trimmed());
                }
            }
        }
    }

    return data;
}

void HTTPConnection::respond(const char* code, const QByteArray& content, const char* contentType, const Headers& headers) {
    QByteArray data(content);
    auto device { std::unique_ptr<QBuffer>(new QBuffer()) };
    device->setBuffer(new QByteArray(content));
    if (!device->open(QIODevice::ReadOnly)) {
    }
    respond(code, std::move(device), contentType, headers);
}

void HTTPConnection::respond(const char* code, std::unique_ptr<QIODevice> device, const char* contentType, const Headers& headers) {
    _responseDevice = std::move(device);

    qDebug() << "Responding to: " << _requestOperation << _requestUrl.toString();

    _socket->write("HTTP/1.1 ");

    if (_responseDevice->isSequential()) {
        qWarning() << "Error responding to HTTPConnection: sequential IO devices not supported";
        _socket->write(StatusCode500);
        _socket->write("\r\n");
        _socket->disconnect(SIGNAL(readyRead()), this);
        _socket->disconnectFromHost();
        return;
    }

    _socket->write(code);
    _socket->write("\r\n");

    for (Headers::const_iterator it = headers.constBegin(), end = headers.constEnd();
            it != end; it++) {
        _socket->write(it.key());
        _socket->write(": ");
        _socket->write(it.value());
        _socket->write("\r\n");
    }

    int csize = _responseDevice->size();
    if (csize > 0) {
        _socket->write("Content-Length: ");
        _socket->write(QByteArray::number(csize));
        _socket->write("\r\n");

        _socket->write("Content-Type: ");
        _socket->write(contentType);
        _socket->write("\r\n");
    }
    _socket->write("Connection: close\r\n\r\n");

    qDebug() << "Setting up signals";

    if (_responseDevice->atEnd()) {
        _socket->disconnectFromHost();
    } else {
        constexpr size_t HTTP_RESPONSE_CHUNK_SIZE = 1024 * 10;
        int totalToBeWritten = csize;
        connect(_socket, &QTcpSocket::bytesWritten, this, [this, totalToBeWritten](size_t bytes) mutable {
            qDebug() << "Bytes were written " << _responseDevice.get() << bytes << totalToBeWritten;
            if (!_responseDevice->atEnd()) {
                qDebug() << "Writing chunk";
                totalToBeWritten -= _socket->write(_responseDevice->read(HTTP_RESPONSE_CHUNK_SIZE));
                if (_responseDevice->atEnd()) {
                    qDebug() << "At end of device, disconnecting";
                    _socket->disconnectFromHost();
                    disconnect(_socket, &QTcpSocket::bytesWritten, this, nullptr);
                }
            }
        });

    }

    // make sure we receive no further read notifications
    disconnect(_socket, &QTcpSocket::readyRead, this, nullptr);
}

void HTTPConnection::readRequest() {
    if (!_socket->canReadLine()) {
        return;
    }
    if (!_requestUrl.isEmpty()) {
        qDebug() << "Request URL was already set";
        return;
    }
    qDebug() << "Reading request";
    // parse out the method and resource
    QByteArray line = _socket->readLine().trimmed();
    if (line.startsWith("HEAD")) {
        _requestOperation = QNetworkAccessManager::HeadOperation;

    } else if (line.startsWith("GET")) {
        _requestOperation = QNetworkAccessManager::GetOperation;

    } else if (line.startsWith("PUT")) {
        _requestOperation = QNetworkAccessManager::PutOperation;

    } else if (line.startsWith("POST")) {
        _requestOperation = QNetworkAccessManager::PostOperation;

    } else if (line.startsWith("DELETE")) {
        _requestOperation = QNetworkAccessManager::DeleteOperation;

    } else {
        qWarning() << "Unrecognized HTTP operation." << _address << line;
        respond("400 Bad Request", "Unrecognized operation.");
        return;
    }
    int idx = line.indexOf(' ') + 1;
    _requestUrl.setUrl(line.mid(idx, line.lastIndexOf(' ') - idx));

    // switch to reading the header
    _socket->disconnect(this, SLOT(readRequest()));
    connect(_socket, SIGNAL(readyRead()), SLOT(readHeaders()));

    // read any headers immediately available
    readHeaders();
}

void HTTPConnection::readHeaders() {
    qDebug() << "Reading headers";
    while (_socket->canReadLine()) {
        QByteArray line = _socket->readLine();
        QByteArray trimmed = line.trimmed();
        if (trimmed.isEmpty()) {
            _socket->disconnect(this, SLOT(readHeaders()));

            QByteArray clength = _requestHeaders.value("Content-Length");
            if (clength.isEmpty()) {
                _parentManager->handleHTTPRequest(this, _requestUrl);

            } else {
                _requestContent.resize(clength.toInt());
                connect(_socket, SIGNAL(readyRead()), SLOT(readContent()));

                // read any content immediately available
                readContent();
            }
            return;
        }
        char first = line.at(0);
        if (first == ' ' || first == '\t') { // continuation
            _requestHeaders[_lastRequestHeader].append(trimmed);
            continue;
        }
        int idx = trimmed.indexOf(':');
        if (idx == -1) {
            qWarning() << "Invalid header." << _address << trimmed;
            respond("400 Bad Request", "The header was malformed.");
            return;
        }
        _lastRequestHeader = trimmed.left(idx);
        QByteArray& value = _requestHeaders[_lastRequestHeader];
        if (!value.isEmpty()) {
            value.append(", ");
        }
        value.append(trimmed.mid(idx + 1).trimmed());
    }
}

void HTTPConnection::readContent() {
    int size = _requestContent.size();
    if (_socket->bytesAvailable() < size) {
        return;
    }
    qDebug() << "Reading content";
    _socket->read(_requestContent.data(), size);
    _socket->disconnect(this, SLOT(readContent()));

    _parentManager->handleHTTPRequest(this, _requestUrl.path());
}
