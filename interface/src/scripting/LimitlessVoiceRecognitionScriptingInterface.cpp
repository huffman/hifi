//
//  SpeechRecognitionScriptingInterface.h
//  interface/src/scripting
//
//  Created by Trevor Berninger on 3/20/17.
//  Copyright 2017 Limitless ltd.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <src/InterfaceLogging.h>
#include <QJsonDocument>
#include <QJsonArray>
#include <src/ui/AvatarInputs.h>
#include <QtConcurrent/QtConcurrentRun>
#include "LimitlessVoiceRecognitionScriptingInterface.h"

LimitlessVoiceRecognitionScriptingInterface::LimitlessVoiceRecognitionScriptingInterface() :
        _transcribeServerSocket(nullptr),
        _streamingAudioForTranscription(false),
        _shouldStartListeningForVoice(false),
        _currentTranscription(""),
        _authenticated(false)
{
    connect(DependencyManager::get<AudioClient>().data(), &AudioClient::inputReceived, this, &LimitlessVoiceRecognitionScriptingInterface::audioInputReceived);
    connect(&_voiceTimer, &QTimer::timeout, this, &LimitlessVoiceRecognitionScriptingInterface::voiceTimeout);
}

void LimitlessVoiceRecognitionScriptingInterface::update() {
    const float audioLevel = AvatarInputs::getInstance()->loudnessToAudioLevel(DependencyManager::get<AudioClient>()->getAudioAverageInputLoudness());
    const bool connected = _transcribeServerSocket && _transcribeServerSocket->isWritable()
                           && _transcribeServerSocket->state() != QAbstractSocket::SocketState::UnconnectedState;
    if (_shouldStartListeningForVoice) {
        if (connected) {
            if (audioLevel > 0.33f) {
                if (_voiceTimer.isActive()) {
                    _voiceTimer.stop();
                }
            } else {
                _voiceTimer.start(2000);
            }
        } else if (audioLevel > 0.33f) {
            QtConcurrent::run(this, &LimitlessVoiceRecognitionScriptingInterface::talkToServer);
        }
    }
}

void LimitlessVoiceRecognitionScriptingInterface::setListeningToVoice(bool listening) {
    _shouldStartListeningForVoice = listening;
}

void LimitlessVoiceRecognitionScriptingInterface::setAuthKey(QString key) {
    _speechAuthCode = key;
}

void LimitlessVoiceRecognitionScriptingInterface::transcriptionReceived() {
    while (_transcribeServerSocket && _transcribeServerSocket->bytesAvailable() > 0) {
        const QByteArray data = _transcribeServerSocket->readAll();
        _serverDataBuffer.append(data);
        int begin = _serverDataBuffer.indexOf('<');
        int end = _serverDataBuffer.indexOf('>');
        while (begin > -1 && end > -1) {
            const int len = end - begin;
            const QByteArray serverMessage = _serverDataBuffer.mid(begin+1, len-1);
            if (serverMessage.contains("1407")) {
                qCDebug(interfaceapp) << "Limitless Speech Server denied.";
                _streamingAudioForTranscription = false;
                _shouldStartListeningForVoice = false;
                return;
            } else if (serverMessage.contains("1408")) {
                qCDebug(interfaceapp) << "Authenticated!";
                _serverDataBuffer.clear();
                _authenticated = true;
                return;
            }
            QJsonObject json = QJsonDocument::fromJson(serverMessage.data()).object();
            _serverDataBuffer.remove(begin, len+1);
            _currentTranscription = json["alternatives"].toArray()[0].toObject()["transcript"].toString();
            if (json["isFinal"] == true) {
                _streamingAudioForTranscription = false;
                qCDebug(interfaceapp) << "Final transcription: " << _currentTranscription;
                return;
            }
            begin = _serverDataBuffer.indexOf('<');
            end = _serverDataBuffer.indexOf('>');
        }
    }
}

void LimitlessVoiceRecognitionScriptingInterface::talkToServer() {
    _serverDataBuffer.clear();
    _audioDataBuffer.clear();
    _streamingAudioForTranscription = true;
    _authenticated = false;
    _transcribeServerSocket.reset(new QTcpSocket(this));
    connect(_transcribeServerSocket.get(), &QTcpSocket::readyRead, this,
            &LimitlessVoiceRecognitionScriptingInterface::transcriptionReceived);

    qCDebug(interfaceapp) << "Setting up connection";
    static const auto host = "gserv_devel.studiolimitless.com";
    _transcribeServerSocket->connectToHost(host, 1407);
    _transcribeServerSocket->waitForConnected();
    QString requestHeader = QString::asprintf("Authorization: %s\r\nfs: %i\r\n", _speechAuthCode.toLocal8Bit().data(), AudioConstants::SAMPLE_RATE);
    qCDebug(interfaceapp) << "Sending: " << requestHeader;
    _transcribeServerSocket->write(requestHeader.toLocal8Bit());
    _transcribeServerSocket->waitForBytesWritten();
    _transcribeServerSocket->waitForReadyRead(); // Wait for server to tell us if the auth was successful.

    while(_streamingAudioForTranscription) {
        QThread::msleep(100);
        while (_authenticated && !_audioDataBuffer.isEmpty()) {
            const QByteArray samples = _audioDataBuffer.dequeue();
            qCDebug(interfaceapp) << "Sending Data!";
            _transcribeServerSocket->write(samples.data(), samples.size());
            _transcribeServerSocket->waitForBytesWritten();
        }
    }

    onFinishedSpeaking(_currentTranscription);
    _currentTranscription = "";
    _authenticated = false;
    _transcribeServerSocket->close();
    _transcribeServerSocket.reset(nullptr);
}

void LimitlessVoiceRecognitionScriptingInterface::audioInputReceived(const QByteArray& inputSamples) {
    if (_transcribeServerSocket && _transcribeServerSocket->isWritable()
       && _transcribeServerSocket->state() != QAbstractSocket::SocketState::UnconnectedState) {
        _audioDataBuffer.enqueue(inputSamples);
    }
}

void LimitlessVoiceRecognitionScriptingInterface::voiceTimeout() {
    qCDebug(interfaceapp) << "Timeout timer called";
    if (_transcribeServerSocket && _transcribeServerSocket->isWritable()
       &&  _transcribeServerSocket->state() != QAbstractSocket::SocketState::UnconnectedState) {
        _streamingAudioForTranscription = false;
        qCDebug(interfaceapp) << "Timeout!";
    }
    _voiceTimer.stop();
}
