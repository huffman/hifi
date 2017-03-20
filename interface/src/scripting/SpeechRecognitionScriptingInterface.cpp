//
//  SpeechRecognitionScriptingInterface.h
//  interface/src/scripting
//
//  Created by Trevor Berninger on 3/20/17.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <src/InterfaceLogging.h>
#include <QJsonDocument>
#include <QJsonArray>
#include <src/ui/AvatarInputs.h>
#include "SpeechRecognitionScriptingInterface.h"

SpeechRecognitionScriptingInterface* SpeechRecognitionScriptingInterface::getInstance() {
    static SpeechRecognitionScriptingInterface sharedInstance;
    return &sharedInstance;
}

SpeechRecognitionScriptingInterface::SpeechRecognitionScriptingInterface() :
        transcribeServerSocket(nullptr),
        streamingAudioForTranscription(false),
        shouldStartListeningForVoice(false),
        currentTranscription("")
{
    audioClient = DependencyManager::get<AudioClient>().data();
    connect(audioClient, &AudioClient::inputReceived, this, &SpeechRecognitionScriptingInterface::audioInputReceived);
    connect(&voiceTimer, &QTimer::timeout, this, &SpeechRecognitionScriptingInterface::voiceTimeout);
}

void SpeechRecognitionScriptingInterface::update() {
    const float audioLevel = AvatarInputs::getInstance()->loudnessToAudioLevel(audioClient->getAudioAverageInputLoudness());
    if(transcribeServerSocket && transcribeServerSocket->isWritable() && transcribeServerSocket->state() != QAbstractSocket::SocketState::UnconnectedState) {
        if(audioLevel == 0.f && !voiceTimer.isActive()) { // socket is open and we stopped speaking and no timeout is set.
            voiceTimer.start(2000);
        }
    }
    else if(audioLevel > 0.33f && shouldStartListeningForVoice) { // socket is closed and we are speaking
        connectToTranscriptionServer();
    }
}

void SpeechRecognitionScriptingInterface::setListeningToVoice(bool listening) {
    // delay so that the enter key doesn't start the listening process when debugging.
    QTimer::singleShot(100, [=]{shouldStartListeningForVoice = listening;});
}

void SpeechRecognitionScriptingInterface::TranscriptionReceived() {
    while(transcribeServerSocket->bytesAvailable() > 0) {
        const QByteArray data = transcribeServerSocket->readAll();
        qCDebug(interfaceapp) << "Data got!" << data;
        serverDataBuffer.append(data);
        qCDebug(interfaceapp) << "serverDataBuffer: " << serverDataBuffer;
        int begin = serverDataBuffer.indexOf('<');
        int end = serverDataBuffer.indexOf('>');
        while(begin > -1 && end > -1) {
            const int len = end - begin;
            qCDebug(interfaceapp) << "Found JSON object: " << serverDataBuffer.mid(begin+1, len-1);
            QJsonObject json = QJsonDocument::fromJson(serverDataBuffer.mid(begin+1, len-1).data()).object();
            serverDataBuffer.remove(begin, len+1);
            currentTranscription = json["alternatives"].toArray()[0].toObject()["transcript"].toString();
            if(json["isFinal"] == true) {
                streamingAudioForTranscription = false;
                qCDebug(interfaceapp) << "Final transcription: " << currentTranscription;
                return;
            }
            begin = serverDataBuffer.indexOf('<');
            end = serverDataBuffer.indexOf('>');
        }
    }
}

void SpeechRecognitionScriptingInterface::connectToTranscriptionServer() {
    serverDataBuffer.clear();
    streamingAudioForTranscription = true;
    transcribeServerSocket = new QTcpSocket(this);
    connect(transcribeServerSocket, &QTcpSocket::readyRead, this, &SpeechRecognitionScriptingInterface::TranscriptionReceived);
    static const auto host = "gserv_devel.studiolimitless.com";
    qCDebug(interfaceapp) << "Setting up connection";
    transcribeServerSocket->connectToHost(host, 1407);
    transcribeServerSocket->waitForConnected();
    QString requestHeader = QString::asprintf("Authorization: testKey\r\nfs: %i\r\n", AudioConstants::SAMPLE_RATE);
    qCDebug(interfaceapp) << "Sending: " << requestHeader;
    transcribeServerSocket->write(requestHeader.toLocal8Bit());
    transcribeServerSocket->waitForBytesWritten();
    qCDebug(interfaceapp) << "Completed send";
}

void SpeechRecognitionScriptingInterface::audioInputReceived(const QByteArray& inputSamples) {
    if(transcribeServerSocket && transcribeServerSocket->isWritable() && transcribeServerSocket->state() != QAbstractSocket::SocketState::UnconnectedState) {
        if(streamingAudioForTranscription) {
            qCDebug(interfaceapp) << "Sending Data!";
            transcribeServerSocket->write(inputSamples.data(), inputSamples.size());
            transcribeServerSocket->waitForBytesWritten();
        }
        else {
            qCDebug(interfaceapp) << "Closing socket with data: " << currentTranscription;
            onFinishedSpeaking(currentTranscription);
            currentTranscription = "";
            transcribeServerSocket->close();
            delete transcribeServerSocket;
            transcribeServerSocket = nullptr;
        }
    }
}

void SpeechRecognitionScriptingInterface::voiceTimeout() {
    qCDebug(interfaceapp) << "Timeout timer called";
    if(transcribeServerSocket && transcribeServerSocket->isWritable() && transcribeServerSocket->state() != QAbstractSocket::SocketState::UnconnectedState) {
        streamingAudioForTranscription = false;
        qCDebug(interfaceapp) << "Timeout!";
    }
    voiceTimer.stop();
}
