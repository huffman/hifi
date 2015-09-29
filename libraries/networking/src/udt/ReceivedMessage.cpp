//
//  ReceivedMessage.cpp
//  libraries/networking/src/udt
//
//  Created by Ryan Huffman on 2015/09/17
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


#include "ReceivedMessage.h"

#include "QSharedPointer"

ReceivedMessage::ReceivedMessage(const udt::PacketList& packetList)
    : _data(packetList.getMessage()),
      _packetType(packetList.getType())
{
    qDebug() << "Creating ReceivedMessage with data: " << packetList.getMessage();
}

ReceivedMessage::ReceivedMessage(NLPacket& packet)
    : _data(packet.readAll()),
      _packetType(packet.getType())
{
}

ReceivedMessage::ReceivedMessage(udt::Packet& packet)
    : _data(packet.readAll())
{
}

qint64 ReceivedMessage::peek(char* data, qint64 size) {
    memcpy(data, _data.constData() + _position, size);
    return size;
}

qint64 ReceivedMessage::read(char* data, qint64 size) {
    memcpy(data, _data.constData() + _position, size);
    _position += size;
    return size;
}

QByteArray ReceivedMessage::peek(qint64 size) {
    return _data.mid(_position, size);
}

QByteArray ReceivedMessage::read(qint64 size) {
    auto data = _data.mid(_position, size);
    _position += size;
    return data;
}
