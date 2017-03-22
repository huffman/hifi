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
#include "LimitlessVoiceRecognitionScriptingInterface.h"

LimitlessVoiceRecognitionScriptingInterface::LimitlessVoiceRecognitionScriptingInterface() :
        _transcribeServerSocket(nullptr),
        _streamingAudioForTranscription(false),
        _shouldStartListeningForVoice(false),
        _currentTranscription("")
{
    connect(DependencyManager::get<AudioClient>().data(), &AudioClient::inputReceived, this, &LimitlessVoiceRecognitionScriptingInterface::audioInputReceived);
    connect(&_voiceTimer, &QTimer::timeout, this, &LimitlessVoiceRecognitionScriptingInterface::voiceTimeout);
}

void LimitlessVoiceRecognitionScriptingInterface::update() {
    const float audioLevel = AvatarInputs::getInstance()->loudnessToAudioLevel(DependencyManager::get<AudioClient>()->getAudioAverageInputLoudness());
    if (_transcribeServerSocket && _transcribeServerSocket->isWritable()
       && _transcribeServerSocket->state() != QAbstractSocket::SocketState::UnconnectedState) {
        if (audioLevel < 0.05f && !_voiceTimer.isActive()) { // socket is open and we stopped speaking and no timeout is set.
            _voiceTimer.start(2000);
        }
    } else if (audioLevel > 0.33f && _shouldStartListeningForVoice) { // socket is closed and we are speaking
        connectToTranscriptionServer();
    }
}

void LimitlessVoiceRecognitionScriptingInterface::setListeningToVoice(bool listening) {
    _shouldStartListeningForVoice = listening;
}

void LimitlessVoiceRecognitionScriptingInterface::transcriptionReceived() {
    while (_transcribeServerSocket->bytesAvailable() > 0) {
        const QByteArray data = _transcribeServerSocket->readAll();
        qCDebug(interfaceapp) << "Data got!" << data;
        _serverDataBuffer.append(data);
        qCDebug(interfaceapp) << "serverDataBuffer: " << _serverDataBuffer;
        int begin = _serverDataBuffer.indexOf('<');
        int end = _serverDataBuffer.indexOf('>');
        while (begin > -1 && end > -1) {
            const int len = end - begin;
            qCDebug(interfaceapp) << "Found JSON object: " << _serverDataBuffer.mid(begin+1, len-1);
            try {
                QJsonObject json = QJsonDocument::fromJson(_serverDataBuffer.mid(begin+1, len-1).data()).object();
                _serverDataBuffer.remove(begin, len+1);
                _currentTranscription = json["alternatives"].toArray()[0].toObject()["transcript"].toString();
                if (json["isFinal"] == true) {
                    _streamingAudioForTranscription = false;
                    qCDebug(interfaceapp) << "Final transcription: " << _currentTranscription;
                    return;
                }
            } catch (...) {
                qCDebug(interfaceapp) << "Server sent us a non-JSON object?!";
            }
            begin = _serverDataBuffer.indexOf('<');
            end = _serverDataBuffer.indexOf('>');
        }
    }
}

void LimitlessVoiceRecognitionScriptingInterface::connectToTranscriptionServer() {
    _serverDataBuffer.clear();
    _streamingAudioForTranscription = true;
    _transcribeServerSocket.reset(new QTcpSocket(this));
    connect(_transcribeServerSocket.get(), &QTcpSocket::readyRead, this,
            &LimitlessVoiceRecognitionScriptingInterface::transcriptionReceived);
    static const auto host = "104.198.102.137";
    qCDebug(interfaceapp) << "Setting up connection";
    _transcribeServerSocket->connectToHost(host, 1407);
    _transcribeServerSocket->waitForConnected();
    QString requestHeader = QString::asprintf("Authorization: testKey\r\nfs: %i\r\n", AudioConstants::SAMPLE_RATE);
    qCDebug(interfaceapp) << "Sending: " << requestHeader;
    _transcribeServerSocket->write(requestHeader.toLocal8Bit());
    _transcribeServerSocket->waitForBytesWritten();
    qCDebug(interfaceapp) << "Completed send";
}

void LimitlessVoiceRecognitionScriptingInterface::audioInputReceived(const QByteArray& inputSamples) {
    if (_transcribeServerSocket && _transcribeServerSocket->isWritable()
       && _transcribeServerSocket->state() != QAbstractSocket::SocketState::UnconnectedState) {
        if (_streamingAudioForTranscription) {
            qCDebug(interfaceapp) << "Sending Data!";
            _transcribeServerSocket->write(inputSamples.data(), inputSamples.size());
            _transcribeServerSocket->waitForBytesWritten();
        } else {
            qCDebug(interfaceapp) << "Closing socket with data" << _currentTranscription;
            onFinishedSpeaking(_currentTranscription);
            _currentTranscription = "";
            _transcribeServerSocket->close();
            _transcribeServerSocket.reset(nullptr);
        }
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
