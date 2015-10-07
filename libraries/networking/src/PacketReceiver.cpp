//
//  PacketReceiver.cpp
//  libraries/networking/src
//
//  Created by Stephen Birarda on 1/23/2014.
//  Update by Ryan Huffman on 7/8/2015.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "PacketReceiver.h"

#include <QMutexLocker>

#include "DependencyManager.h"
#include "NetworkLogging.h"
#include "NodeList.h"
#include "SharedUtil.h"

Q_DECLARE_METATYPE(QSharedPointer<NLPacketList>);
Q_DECLARE_METATYPE(QSharedPointer<ReceivedMessage>);
PacketReceiver::PacketReceiver(QObject* parent) : QObject(parent) {
    qRegisterMetaType<QSharedPointer<NLPacket>>();
    qRegisterMetaType<QSharedPointer<NLPacketList>>();
    qRegisterMetaType<QSharedPointer<ReceivedMessage>>();
}

bool PacketReceiver::registerListenerForTypes(PacketTypeList types, QObject* listener, const char* slot) {
    Q_ASSERT_X(!types.empty(), "PacketReceiver::registerListenerForTypes", "No types to register");
    Q_ASSERT_X(listener, "PacketReceiver::registerListenerForTypes", "No object to register");
    Q_ASSERT_X(slot, "PacketReceiver::registerListenerForTypes", "No slot to register");
    
    // Partition types based on whether they are sourced or not (non sourced in front)
    auto middle = std::partition(std::begin(types), std::end(types), [](PacketType type) {
        return NON_SOURCED_PACKETS.contains(type);
    });
    
    QMetaMethod nonSourcedMethod, sourcedMethod;
    
    // Check we have a valid method for non sourced types if any
    if (middle != std::begin(types)) {
        nonSourcedMethod = matchingMethodForListener(*std::begin(types), listener, slot);
        if (!nonSourcedMethod.isValid()) {
            return false;
        }
    }
    
    // Check we have a valid method for sourced types if any
    if (middle != std::end(types)) {
        sourcedMethod = matchingMethodForListener(*middle, listener, slot);
        if (!sourcedMethod.isValid()) {
            return false;
        }
    }
    
    // Register non sourced types
    std::for_each(std::begin(types), middle, [this, &listener, &nonSourcedMethod](PacketType type) {
        registerVerifiedListener(type, listener, nonSourcedMethod);
    });
    
    // Register sourced types
    std::for_each(middle, std::end(types), [this, &listener, &sourcedMethod](PacketType type) {
        registerVerifiedListener(type, listener, sourcedMethod);
    });
    
    return true;
}

void PacketReceiver::registerDirectListener(PacketType type, QObject* listener, const char* slot) {
    Q_ASSERT_X(listener, "PacketReceiver::registerDirectListener", "No object to register");
    Q_ASSERT_X(slot, "PacketReceiver::registerDirectListener", "No slot to register");
    
    bool success = registerListener(type, listener, slot);
    if (success) {
        QMutexLocker locker(&_directConnectSetMutex);
        
        // if we successfully registered, add this object to the set of objects that are directly connected
        _directlyConnectedObjects.insert(listener);
    }
}

void PacketReceiver::registerDirectListenerForTypes(PacketTypeList types,
                                                    QObject* listener, const char* slot) {
    Q_ASSERT_X(listener, "PacketReceiver::registerDirectListenerForTypes", "No object to register");
    Q_ASSERT_X(slot, "PacketReceiver::registerDirectListenerForTypes", "No slot to register");
    
    // just call register listener for types to start
    bool success = registerListenerForTypes(std::move(types), listener, slot);
    if (success) {
        QMutexLocker locker(&_directConnectSetMutex);
        
        // if we successfully registered, add this object to the set of objects that are directly connected
        _directlyConnectedObjects.insert(listener);
    }
}

bool PacketReceiver::registerMessageListener(PacketType type, QObject* listener, const char* slot) {
    Q_ASSERT_X(listener, "PacketReceiver::registerMessageListener", "No object to register");
    Q_ASSERT_X(slot, "PacketReceiver::registerMessageListener", "No slot to register");

    QMetaMethod matchingMethod = matchingMethodForListener(type, listener, slot);

    if (matchingMethod.isValid()) {
        QMutexLocker locker(&_packetListenerLock);

        if (_packetListListenerMap.contains(type)) {
            qCWarning(networking) << "Registering a packet listener for packet type" << type
                << "that will remove a previously registered listener";
        }

        // add the mapping
        _packetListListenerMap[type] = ObjectMethodPair(QPointer<QObject>(listener), matchingMethod);
        return true;
    } else {
        return false;
    }
}

bool PacketReceiver::registerListener(PacketType type, QObject* listener, const char* slot) {
    Q_ASSERT_X(listener, "PacketReceiver::registerListener", "No object to register");
    Q_ASSERT_X(slot, "PacketReceiver::registerListener", "No slot to register");

    QMetaMethod matchingMethod = matchingMethodForListener(type, listener, slot);

    if (matchingMethod.isValid()) {
        qDebug() << "Found: " << matchingMethod.methodSignature();
        registerVerifiedListener(type, listener, matchingMethod);
        return true;
    } else {
        return false;
    }
}

QMetaMethod PacketReceiver::matchingMethodForListener(PacketType type, QObject* object, const char* slot) const {
    Q_ASSERT_X(object, "PacketReceiver::matchingMethodForListener", "No object to call");
    Q_ASSERT_X(slot, "PacketReceiver::matchingMethodForListener", "No slot to call");

    // normalize the slot with the expected parameters
    
    static const QString SIGNATURE_TEMPLATE("%1(%2)");
    // static const QString NON_SOURCED_PACKET_LISTENER_PARAMETERS = "QSharedPointer<NLPacket>";
    static const QString NON_SOURCED_PACKETLIST_LISTENER_PARAMETERS = "QSharedPointer<ReceivedMessage>";

    QSet<QString> possibleSignatures {
        // SIGNATURE_TEMPLATE.arg(slot, NON_SOURCED_PACKET_LISTENER_PARAMETERS),
        SIGNATURE_TEMPLATE.arg(slot, NON_SOURCED_PACKETLIST_LISTENER_PARAMETERS)
    };

    if (!NON_SOURCED_PACKETS.contains(type)) {
        // static const QString SOURCED_PACKET_LISTENER_PARAMETERS = "QSharedPointer<NLPacket>,QSharedPointer<Node>";
        // static const QString TYPEDEF_SOURCED_PACKET_LISTENER_PARAMETERS = "QSharedPointer<NLPacket>,SharedNodePointer";
        static const QString SOURCED_PACKETLIST_LISTENER_PARAMETERS = "QSharedPointer<ReceivedMessage>,QSharedPointer<Node>";
        static const QString TYPEDEF_SOURCED_PACKETLIST_LISTENER_PARAMETERS = "QSharedPointer<ReceivedMessage>,SharedNodePointer";

        // a sourced packet must take the shared pointer to the packet but optionally could include
        // a shared pointer to the node

        // possibleSignatures << SIGNATURE_TEMPLATE.arg(slot, TYPEDEF_SOURCED_PACKET_LISTENER_PARAMETERS);
        // possibleSignatures << SIGNATURE_TEMPLATE.arg(slot, SOURCED_PACKET_LISTENER_PARAMETERS);
        possibleSignatures << SIGNATURE_TEMPLATE.arg(slot, TYPEDEF_SOURCED_PACKETLIST_LISTENER_PARAMETERS);
        possibleSignatures << SIGNATURE_TEMPLATE.arg(slot, SOURCED_PACKETLIST_LISTENER_PARAMETERS);
    }

    int methodIndex = -1;

    foreach(const QString& signature, possibleSignatures) {
        QByteArray normalizedSlot =
            QMetaObject::normalizedSignature(signature.toStdString().c_str());
        
        // does the constructed normalized method exist?
        methodIndex = object->metaObject()->indexOfSlot(normalizedSlot.toStdString().c_str());

        if (methodIndex >= 0) {
            break;
        }
    }

    if (methodIndex < 0) {
        qCDebug(networking) << "PacketReceiver::registerListener expected a slot with one of the following signatures:"
                 << possibleSignatures.toList() << "- but such a slot was not found."
                 << "Could not complete listener registration for type" << type;
    }

    Q_ASSERT(methodIndex >= 0);

    // return the converted QMetaMethod
    if (methodIndex >= 0) {
        return object->metaObject()->method(methodIndex);
    } else {
        // if somehow (scripting?) something bad gets in here at runtime that doesn't hit the asserts above
        // return a non-valid QMetaMethod
        return QMetaMethod();
    }
}

void PacketReceiver::registerVerifiedListener(PacketType type, QObject* object, const QMetaMethod& slot) {
    Q_ASSERT_X(object, "PacketReceiver::registerVerifiedListener", "No object to register");
    QMutexLocker locker(&_packetListenerLock);

    if (_packetListListenerMap.contains(type)) {
        qCWarning(networking) << "Registering a packet listener for packet type" << type
            << "that will remove a previously registered listener";
    }
    
    // add the mapping
    _packetListListenerMap[type] = ObjectMethodPair(QPointer<QObject>(object), slot);
}

void PacketReceiver::unregisterListener(QObject* listener) {
    Q_ASSERT_X(listener, "PacketReceiver::unregisterListener", "No listener to unregister");
    
    {
        QMutexLocker packetListenerLocker(&_packetListenerLock);
        
        // TODO: replace the two while loops below with a replace_if on the vector (once we move to Message everywhere)
        
        // clear any registrations for this listener in _packetListListenerMap
        auto it = _packetListListenerMap.begin();
        
        while (it != _packetListListenerMap.end()) {
            if (it.value().first == listener) {
                it = _packetListListenerMap.erase(it);
            } else {
                ++it;
            }
        }
        
        // clear any registrations for this listener in _packetListListener
        auto listIt = _packetListListenerMap.begin();
        
        while (listIt != _packetListListenerMap.end()) {
            if (listIt.value().first == listener) {
                listIt = _packetListListenerMap.erase(listIt);
            } else {
                ++listIt;
            }
        }
    }
    
    QMutexLocker directConnectSetLocker(&_directConnectSetMutex);
    _directlyConnectedObjects.remove(listener);
}

void PacketReceiver::handleVerifiedPacketList(std::unique_ptr<udt::PacketList> packetList) {
    // if we're supposed to drop this packet then break out here
    if (_shouldDropPackets) {
        return;
    }

    // setup an NLPacketList from the PacketList we were passed
    auto nlPacketList = NLPacketList::fromPacketList(std::move(packetList));
    auto receivedMessage = std::unique_ptr<ReceivedMessage>(new ReceivedMessage(*nlPacketList.release()));

    handleVerifiedMessage(std::move(receivedMessage));
}

bool PacketReceiver::handleVerifiedMessage(std::unique_ptr<ReceivedMessage> receivedMessage) {

    auto nodeList = DependencyManager::get<LimitedNodeList>();
    
    _inPacketCount += receivedMessage->getNumPackets();
    _inByteCount += receivedMessage->size();
    
    SharedNodePointer matchingNode;
    
    if (!receivedMessage->getSourceID().isNull()) {
        matchingNode = nodeList->nodeWithUUID(receivedMessage->getSourceID());
    }
    
    QMutexLocker packetListenerLocker(&_packetListenerLock);
    
    bool listenerIsDead = false;
    
    auto it = _packetListListenerMap.find(receivedMessage->getType());
    
    if (receivedMessage->getType() == PacketType::MixedAudio) {
        qDebug() << "Audio";
    }
            
    bool success = false;
    if (it != _packetListListenerMap.end() && it->second.isValid()) {
        
        auto listener = it.value();
        if (listener.second.methodSignature().toStdString().find("ReceivedMessage") == std::string::npos) {
            qDebug() << "listener is not registered for receivedmessage: " << listener.second.methodSignature();
            return false;
        }
    
        if (listener.first) {
            
            Qt::ConnectionType connectionType;
            // check if this is a directly connected listener
            {
                QMutexLocker directConnectLocker(&_directConnectSetMutex);
 
                connectionType = _directlyConnectedObjects.contains(listener.first) ? Qt::DirectConnection : Qt::AutoConnection;
            }
            
            PacketType packetType = receivedMessage->getType();
            
            if (matchingNode) {
                emit dataReceived(matchingNode->getType(), receivedMessage->size());
                QMetaMethod metaMethod = listener.second;

                // qDebug() << "Got verified packet list: " << QString(receivedMessage->getMessage());
                
                static const QByteArray QSHAREDPOINTER_NODE_NORMALIZED = QMetaObject::normalizedType("QSharedPointer<Node>");
                static const QByteArray SHARED_NODE_NORMALIZED = QMetaObject::normalizedType("SharedNodePointer");
                
                // one final check on the QPointer before we go to invoke
                if (listener.first) {
                    if (metaMethod.parameterTypes().contains(SHARED_NODE_NORMALIZED)) {
                        success = metaMethod.invoke(listener.first,
                                                    connectionType,
                                                    Q_ARG(QSharedPointer<ReceivedMessage>,
                                                          QSharedPointer<ReceivedMessage>(receivedMessage.release())),
                                                    Q_ARG(SharedNodePointer, matchingNode));
                        
                    } else if (metaMethod.parameterTypes().contains(QSHAREDPOINTER_NODE_NORMALIZED)) {
                        success = metaMethod.invoke(listener.first,
                                                    connectionType,
                                                    Q_ARG(QSharedPointer<ReceivedMessage>,
                                                          QSharedPointer<ReceivedMessage>(receivedMessage.release())),
                                                    Q_ARG(QSharedPointer<Node>, matchingNode));
                        
                    } else {
                        success = metaMethod.invoke(listener.first,
                                                    connectionType,
                                                    Q_ARG(QSharedPointer<ReceivedMessage>,
                                                          QSharedPointer<ReceivedMessage>(receivedMessage.release())));
                    }
                } else {
                    listenerIsDead = true;
                }
            } else {
                // qDebug() << "Got verified unsourced packet list: " << QString(nlPacketList->getMessage());
                emit dataReceived(NodeType::Unassigned, receivedMessage->size());
                
                // one final check on the QPointer before we invoke
                if (listener.first) {
                    success = listener.second.invoke(listener.first,
                                                     Q_ARG(QSharedPointer<ReceivedMessage>,
                                                           QSharedPointer<ReceivedMessage>(receivedMessage.release())));
                } else {
                    listenerIsDead = true;
                }
                
            }
            
            if (!success) {
                qCDebug(networking).nospace() << "Error delivering packet " << packetType << " to listener "
                    << listener.first << "::" << qPrintable(listener.second.methodSignature());
            }
            
        } else {
            listenerIsDead = true;
        }
        
        if (listenerIsDead) {
            qCDebug(networking).nospace() << "Listener for packet " << receivedMessage->getType()
                << " has been destroyed. Removing from listener map.";
            it = _packetListListenerMap.erase(it);
            
            // if it exists, remove the listener from _directlyConnectedObjects
            {
                QMutexLocker directConnectLocker(&_directConnectSetMutex);
                _directlyConnectedObjects.remove(listener.first);
            }
        }
        
    } else if (it == _packetListListenerMap.end()) {
        qCWarning(networking) << "No listener found for packet type" << receivedMessage->getType();
        
        // insert a dummy listener so we don't print this again
        _packetListListenerMap.insert(receivedMessage->getType(), { nullptr, QMetaMethod() });
    }

    return success;
}

void PacketReceiver::handleVerifiedPacket(std::unique_ptr<udt::Packet> packet) {
    
    // if we're supposed to drop this packet then break out here
    if (_shouldDropPackets) {
        return;
    }
    
    auto nodeList = DependencyManager::get<LimitedNodeList>();
    
    // setup an NLPacket from the packet we were passed
    auto nlPacket = NLPacket::fromBase(std::move(packet));
    auto receivedMessage = std::unique_ptr<ReceivedMessage>(new ReceivedMessage(*nlPacket.get()));
    bool success = handleVerifiedMessage(std::move(receivedMessage));
    nlPacket->seek(0);

    if (success) {
        // qDebug() << "succesfully called handleVerifiedMessage";
        return;
    }
    qDebug() << "Did not successfully call handleVerifiedMessage: " << nlPacket->getType();
    return;
    
    _inPacketCount++;
    _inByteCount += nlPacket->getDataSize();
    
    SharedNodePointer matchingNode;
    
    if (!nlPacket->getSourceID().isNull()) {
        matchingNode = nodeList->nodeWithUUID(nlPacket->getSourceID());
    }
    
    QMutexLocker packetListenerLocker(&_packetListenerLock);
    
    bool listenerIsDead = false;
    
    auto it = _packetListListenerMap.find(nlPacket->getType());
    
    if (it != _packetListListenerMap.end() && it->second.isValid()) {
        
        auto listener = it.value();
        
        if (listener.first) {
            
            bool success = false;
            
            // check if this is a directly connected listener
            QMutexLocker directConnectSetLocker(&_directConnectSetMutex);
            Qt::ConnectionType connectionType =
                _directlyConnectedObjects.contains(listener.first) ? Qt::DirectConnection : Qt::AutoConnection;
            directConnectSetLocker.unlock();
            
            PacketType packetType = nlPacket->getType();
            
            if (matchingNode) {
                emit dataReceived(matchingNode->getType(), nlPacket->getDataSize());
                matchingNode->recordBytesReceived(nlPacket->getDataSize());
                
                QMetaMethod metaMethod = listener.second;
                
                static const QByteArray QSHAREDPOINTER_NODE_NORMALIZED = QMetaObject::normalizedType("QSharedPointer<Node>");
                static const QByteArray SHARED_NODE_NORMALIZED = QMetaObject::normalizedType("SharedNodePointer");
                
                // one final check on the QPointer before we go to invoke
                if (listener.first) {
                    if (metaMethod.parameterTypes().contains(SHARED_NODE_NORMALIZED)) {
                        success = metaMethod.invoke(listener.first,
                                                    connectionType,
                                                    Q_ARG(QSharedPointer<NLPacket>,
                                                          QSharedPointer<NLPacket>(nlPacket.release())),
                                                    Q_ARG(SharedNodePointer, matchingNode));
                        
                    } else if (metaMethod.parameterTypes().contains(QSHAREDPOINTER_NODE_NORMALIZED)) {
                        success = metaMethod.invoke(listener.first,
                                                    connectionType,
                                                    Q_ARG(QSharedPointer<NLPacket>,
                                                          QSharedPointer<NLPacket>(nlPacket.release())),
                                                    Q_ARG(QSharedPointer<Node>, matchingNode));
                        
                    } else {
                        success = metaMethod.invoke(listener.first,
                                                    connectionType,
                                                    Q_ARG(QSharedPointer<NLPacket>,
                                                          QSharedPointer<NLPacket>(nlPacket.release())));
                    }
                } else {
                    listenerIsDead = true;
                }
            } else {
                emit dataReceived(NodeType::Unassigned, nlPacket->getDataSize());
                
                // one final check on the QPointer before we invoke
                if (listener.first) {
                    success = listener.second.invoke(listener.first,
                                                     Q_ARG(QSharedPointer<NLPacket>,
                                                           QSharedPointer<NLPacket>(nlPacket.release())));
                } else {
                    listenerIsDead = true;
                }
                
            }
            
            if (!success) {
                qCDebug(networking).nospace() << "Error delivering packet " << packetType << " to listener "
                    << listener.first << "::" << qPrintable(listener.second.methodSignature());
            }
            
        } else {
            listenerIsDead = true;
        }
        
        if (listenerIsDead) {
            qCDebug(networking).nospace() << "Listener for packet " << nlPacket->getType()
                << " has been destroyed. Removing from listener map.";
            it = _packetListListenerMap.erase(it);
            
            // if it exists, remove the listener from _directlyConnectedObjects
            QMutexLocker locker(&_directConnectSetMutex);
            _directlyConnectedObjects.remove(listener.first);
        }
        
    } else if (it == _packetListListenerMap.end()) {
        qCWarning(networking) << "No listener found for packet type" << nlPacket->getType();
        
        // insert a dummy listener so we don't print this again
        _packetListListenerMap.insert(nlPacket->getType(), { nullptr, QMetaMethod() });
    }
}
