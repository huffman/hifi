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

#ifndef hifi_SpeechRecognitionScriptingInterface_h
#define hifi_SpeechRecognitionScriptingInterface_h

#include <AudioClient.h>
#include <QObject>

class LimitlessVoiceRecognitionScriptingInterface : public QObject, public Dependency {
    Q_OBJECT
public:
    LimitlessVoiceRecognitionScriptingInterface();

    void update();

public slots:
    void setListeningToVoice(bool listening);

signals:
    void onFinishedSpeaking(QString speech);

private:

    bool _shouldStartListeningForVoice;
    bool _streamingAudioForTranscription;

    QTimer _voiceTimer;
    std::unique_ptr<QTcpSocket> _transcribeServerSocket;
    QByteArray _serverDataBuffer;
    QString _currentTranscription;

    void transcriptionReceived();
    void connectToTranscriptionServer();
    void audioInputReceived(const QByteArray& inputSamples);
    void voiceTimeout();
};

#endif //hifi_SpeechRecognitionScriptingInterface_h
