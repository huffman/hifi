//
//  NetworkAccessManager.cpp
//
//
//  Created by Clement on 7/1/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QThreadStorage>

#include "NetworkAccessManager.h"
#include <QtNetwork/QNetworkProxy>

QThreadStorage<QNetworkAccessManager*> networkAccessManagers;

QNetworkAccessManager& NetworkAccessManager::getInstance() {
    if (!networkAccessManagers.hasLocalData()) {
        auto man = new QNetworkAccessManager();
        auto proxy = new QNetworkProxy(QNetworkProxy::HttpProxy, "localhost", 8800);
        man->setProxy(*proxy);

        networkAccessManagers.setLocalData(man);
    }
    
    return *networkAccessManagers.localData();
}
