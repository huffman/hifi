//
//  IceServer.cpp
//  ice-server/src
//
//  Created by Stephen Birarda on 2014-10-01.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <PacketHeaders.h>

#include "IceServer.h"

IceServer::IceServer(int argc, char* argv[]) :
	QCoreApplication(argc, argv),
    _id(QUuid::createUuid()),
    _serverSocket()
{
    // start the ice-server socket
    qDebug() << "ice-server socket is listening on" << ICE_SERVER_DEFAULT_PORT;
    _serverSocket.bind(QHostAddress::AnyIPv4, ICE_SERVER_DEFAULT_PORT);
    
    // call our process datagrams slot when the UDP socket has packets ready
    connect(&_serverSocket, &QUdpSocket::readyRead, this, &IceServer::processDatagrams);
}

void IceServer::processDatagrams() {
    HifiSockAddr sendingSockAddr;
    QByteArray incomingPacket;
    
    while (_serverSocket.hasPendingDatagrams()) {
        incomingPacket.resize(_serverSocket.pendingDatagramSize());
        
        _serverSocket.readDatagram(incomingPacket.data(), incomingPacket.size(),
                                   sendingSockAddr.getAddressPointer(), sendingSockAddr.getPortPointer());
        
        
        if (packetTypeForPacket(incomingPacket) == PacketTypeIceServerHeartbeat) {
            QUuid senderUUID = uuidFromPacketHeader(incomingPacket);
            
            // pull the public and private sock addrs for this peer
            HifiSockAddr publicSocket, localSocket;
            
            QDataStream hearbeatStream(incomingPacket);
            hearbeatStream.skipRawData(numBytesForPacketHeader(incomingPacket));
            
            hearbeatStream >> publicSocket >> localSocket;
            
            // make sure we have this sender in our peer hash
            SharedNetworkPeer matchingPeer = _activePeers.value(senderUUID);
            
            if (!matchingPeer) {
                // if we don't have this sender we need to create them now
                matchingPeer = SharedNetworkPeer(new NetworkPeer(senderUUID, publicSocket, localSocket));
                _activePeers.insert(senderUUID, matchingPeer);
                
                qDebug() << "Added a new network peer" << *matchingPeer;
            } else {
                // we already had the peer so just potentially update their sockets
                matchingPeer->setPublicSocket(publicSocket);
                matchingPeer->setLocalSocket(localSocket);
                
                qDebug() << "Matched hearbeat to existing network peer" << *matchingPeer;
            }
            
            // check if this node also included a UUID that they would like to connect to
            QUuid connectRequestID;
            hearbeatStream >> connectRequestID;
            
            if (!connectRequestID.isNull()) {
                qDebug() << "Peer wants to connect to peer with ID" << uuidStringWithoutCurlyBraces(connectRequestID);
                
                // check if we have that ID - if we do we can respond with their info, otherwise nothing we can do
                SharedNetworkPeer matchingConectee = _activePeers.value(connectRequestID);
                if (matchingConectee) {
                    QByteArray heartbeatResponse =  byteArrayWithPopulatedHeader(PacketTypeIceServerHeartbeatResponse, _id);
                    QDataStream responseStream(&heartbeatResponse, QIODevice::Append);
                    
                    responseStream << matchingConectee;
                    
                    _serverSocket.writeDatagram(heartbeatResponse, sendingSockAddr.getAddress(), sendingSockAddr.getPort());
                }
            }
        }
    }
}
