//
//  UserActivityLogger.h
//
//
//  Created by Clement on 5/21/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_UserActivityLogger_h
#define hifi_UserActivityLogger_h

#include "AccountManager.h"

#include <QJsonArray>
#include <QTimer>

#include <SettingHandle.h>

class UserActivityLogger : public QObject {
    Q_OBJECT
    
    struct Event {
        QString type;
        QJsonObject properties;
    };

public:
    static UserActivityLogger& getInstance();
    
    void launch(QString applicationVersion, bool previousSessionCrashed, int previousSessionRuntime);
    void insufficientGLVersion(const QJsonObject& glData);
    void changedDisplayName(QString displayName);
    void changedModel(QString typeOfModel, QString modelURL);
    void changedDomain(QString domainURL);
    void connectedDevice(QString typeOfDevice, QString deviceName);
    void loadedScript(QString scriptName);
    void wentTo(QString destinationType, QString destinationName);

    bool isEnabled() { return !_disabled.get(); }
    
public slots:
    void disable(bool disable);
    void logAction(QString action, QJsonObject details = QJsonObject());

private slots:
    void requestError(QNetworkReply& errorReply);
    void flushEvents();
    
private:
    UserActivityLogger();

    void appendEvent(const QString& eventType, const QJsonObject& properties = QJsonObject());

    QTimer _flushEventsTimer { this };
    QJsonArray _pendingEvents;

    Setting::Handle<bool> _disabled { "UserActivityLoggerDisabled", false };
};

#endif // hifi_UserActivityLogger_h
