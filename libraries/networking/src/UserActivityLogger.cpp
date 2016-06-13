//
//  UserActivityLogger.cpp
//
//
//  Created by Clement on 5/21/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QEventLoop>
#include <QJsonDocument>
#include <QHttpMultiPart>
#include <QTimer>

#include "NetworkLogging.h"

#include "UserActivityLogger.h"
#include <DependencyManager.h>

static const QString USER_ACTIVITY_URL = "/api/v1/user_activities";

static int SEND_EVENTS_PERIOD_SECONDS = 2;
static int MAX_EVENTS_PER_PERIOD = 40;

UserActivityLogger& UserActivityLogger::getInstance() {
    static UserActivityLogger sharedInstance;
    return sharedInstance;
}

void UserActivityLogger::appendEvent(const QString& eventType, const QJsonObject& properties) {
    QJsonObject event {
//        { "event", eventType },
//        { "properties", properties }
        { "action_name", eventType },
        { "action_details", properties }
    };
    _pendingEvents.append(event);
}

UserActivityLogger::UserActivityLogger() {
    // start timer
    _flushEventsTimer.setInterval(SEND_EVENTS_PERIOD_SECONDS * 1000);
    connect(&_flushEventsTimer, &QTimer::timeout, this, &UserActivityLogger::flushEvents);
    _flushEventsTimer.start();

}

void UserActivityLogger::flushEvents() {
    if (_pendingEvents.size() > 0) {
        if (_pendingEvents.size() == MAX_EVENTS_PER_PERIOD) {
            qWarning() << "EVENT LIMIT REACHED";
            appendEvent("event_limit_reached");
        }
        qDebug() << "Sending " << _pendingEvents.size() << " events";

        auto accountManager = DependencyManager::get<AccountManager>();

        QJsonObject actions {
            { "actions", _pendingEvents }
        };
        auto data = QJsonDocument(actions).toJson(QJsonDocument::Compact);
        qDebug().noquote() << "Event data: " << data;

        JSONCallbackParameters params;
        params.errorCallbackReceiver = this;
        params.errorCallbackMethod = "requestError";

        _pendingEvents = QJsonArray();

        accountManager->sendRequest(USER_ACTIVITY_URL,
            AccountManagerAuth::Optional,
            QNetworkAccessManager::PostOperation,
            params, data);
    }
}

void UserActivityLogger::disable(bool disable) {
    _disabled.set(disable);
}

void UserActivityLogger::logAction(QString action, QJsonObject details) {
    if (_disabled.get() || _pendingEvents.size() == MAX_EVENTS_PER_PERIOD) {
        return;
    }

    appendEvent(action, details);
}

void UserActivityLogger::requestError(QNetworkReply& errorReply) {
    qCDebug(networking) << errorReply.error() << "-" << errorReply.errorString();
}

void UserActivityLogger::launch(QString applicationVersion, bool previousSessionCrashed, int previousSessionRuntime) {
    const QString ACTION_NAME = "launch";
    QJsonObject actionDetails;
    QString VERSION_KEY = "version";
    QString CRASH_KEY = "previousSessionCrashed";
    QString RUNTIME_KEY = "previousSessionRuntime";
    actionDetails.insert(VERSION_KEY, applicationVersion);
    actionDetails.insert(CRASH_KEY, previousSessionCrashed);
    actionDetails.insert(RUNTIME_KEY, previousSessionRuntime);
    
    logAction(ACTION_NAME, actionDetails);
}

void UserActivityLogger::insufficientGLVersion(const QJsonObject& glData) {
    const QString ACTION_NAME = "insufficient_gl";
    QJsonObject actionDetails;
    QString GL_DATA = "glData";
    actionDetails.insert(GL_DATA, glData);

    logAction(ACTION_NAME, actionDetails);
}

void UserActivityLogger::changedDisplayName(QString displayName) {
    const QString ACTION_NAME = "changed_display_name";
    QJsonObject actionDetails;
    const QString DISPLAY_NAME = "display_name";
    
    actionDetails.insert(DISPLAY_NAME, displayName);
    
    logAction(ACTION_NAME, actionDetails);
}

void UserActivityLogger::changedModel(QString typeOfModel, QString modelURL) {
    const QString ACTION_NAME = "changed_model";
    QJsonObject actionDetails;
    const QString TYPE_OF_MODEL = "type_of_model";
    const QString MODEL_URL = "model_url";
    
    actionDetails.insert(TYPE_OF_MODEL, typeOfModel);
    actionDetails.insert(MODEL_URL, modelURL);
    
    logAction(ACTION_NAME, actionDetails);
}

void UserActivityLogger::changedDomain(QString domainURL) {
    const QString ACTION_NAME = "changed_domain";
    QJsonObject actionDetails;
    const QString DOMAIN_URL = "domain_url";
    
    actionDetails.insert(DOMAIN_URL, domainURL);
    
    logAction(ACTION_NAME, actionDetails);
}

void UserActivityLogger::connectedDevice(QString typeOfDevice, QString deviceName) {
    const QString ACTION_NAME = "connected_device";
    QJsonObject actionDetails;
    const QString TYPE_OF_DEVICE = "type_of_device";
    const QString DEVICE_NAME = "device_name";
    
    actionDetails.insert(TYPE_OF_DEVICE, typeOfDevice);
    actionDetails.insert(DEVICE_NAME, deviceName);
    
    logAction(ACTION_NAME, actionDetails);

}

void UserActivityLogger::loadedScript(QString scriptName) {
    const QString ACTION_NAME = "loaded_script";
    QJsonObject actionDetails;
    const QString SCRIPT_NAME = "script_name";
    
    actionDetails.insert(SCRIPT_NAME, scriptName);
    
    logAction(ACTION_NAME, actionDetails);

}

void UserActivityLogger::wentTo(QString destinationType, QString destinationName) {
    const QString ACTION_NAME = "went_to";
    QJsonObject actionDetails;
    const QString DESTINATION_TYPE_KEY = "destination_type";
    const QString DESTINATION_NAME_KEY = "detination_name";
    
    actionDetails.insert(DESTINATION_TYPE_KEY, destinationType);
    actionDetails.insert(DESTINATION_NAME_KEY, destinationName);
    
    logAction(ACTION_NAME, actionDetails);
}
