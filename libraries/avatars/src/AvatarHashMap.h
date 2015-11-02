//
//  AvatarHashMap.h
//  libraries/avatars/src
//
//  Created by Andrew Meadows on 1/28/2014.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AvatarHashMap_h
#define hifi_AvatarHashMap_h

#include <QtCore/QHash>
#include <QtCore/QSharedPointer>
#include <QtCore/QUuid>

#include <functional>
#include <memory>

#include <DependencyManager.h>
#include <NLPacket.h>
#include <Node.h>

#include "AvatarData.h"
#include <glm/glm.hpp>

class AvatarHashMap : public QObject, public Dependency {
    Q_OBJECT
    SINGLETON_DEPENDENCY

public:
    void withAvatarHash(std::function<void(const AvatarHash& hash)>);
    int size() { return _avatarHash.size(); }

signals:
    void avatarAddedEvent(const QUuid& sessionUUID);
    void avatarRemovedEvent(const QUuid& sessionUUID);
    void avatarSessionChangedEvent(const QUuid& sessionUUID,const QUuid& oldUUID);

public slots:
    bool isAvatarInRange(const glm::vec3 & position, const float range);
    
private slots:
    void sessionUUIDChanged(const QUuid& sessionUUID, const QUuid& oldUUID);
    
    void processAvatarDataPacket(QSharedPointer<NLPacket> packet, SharedNodePointer sendingNode);
    void processAvatarIdentityPacket(QSharedPointer<NLPacket> packet, SharedNodePointer sendingNode);
    void processAvatarBillboardPacket(QSharedPointer<NLPacket> packet, SharedNodePointer sendingNode);
    void processKillAvatar(QSharedPointer<NLPacket> packet, SharedNodePointer sendingNode);

protected:
    AvatarHashMap();

    virtual AvatarSharedPointer newSharedAvatar();
    virtual AvatarSharedPointer addAvatar(const QUuid& sessionUUID, const QWeakPointer<Node>& mixerWeakPointer);
    virtual void removeAvatar(const QUuid& sessionUUID);

    AvatarHash _avatarHash;
    // "Case-based safety": Most access to the _avatarHash is on the same thread. Write access is protected by a write-lock.
    // If you read from a different thread, you must read-lock the _hashLock. (Scripted write access is not supported).
    QReadWriteLock _hashLock;

private:
    QUuid _lastOwnerSessionUUID;
};

#endif // hifi_AvatarHashMap_h
