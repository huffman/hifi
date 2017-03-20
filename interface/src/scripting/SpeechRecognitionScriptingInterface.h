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

#ifndef hifi_SpeechRecognitionScriptingInterface_h
#define hifi_SpeechRecognitionScriptingInterface_h

#include <AudioClient.h>
#include <QObject>

class SpeechRecognitionScriptingInterface : public QObject {
    Q_OBJECT
public:
    static SpeechRecognitionScriptingInterface* getInstance();
    void update();

public slots:
    void setListeningToVoice(bool listening);

signals:
    void onFinishedSpeaking(QString speech);

private:
    SpeechRecognitionScriptingInterface();

    bool shouldStartListeningForVoice;
    bool streamingAudioForTranscription;

    QTimer voiceTimer;
    QTcpSocket* transcribeServerSocket;
    QByteArray serverDataBuffer;
    AudioClient* audioClient;
    QString currentTranscription;

    void TranscriptionReceived();
    void connectToTranscriptionServer();
    void audioInputReceived(const QByteArray& inputSamples);
    void voiceTimeout();
};

#endif //hifi_SpeechRecognitionScriptingInterface_h
