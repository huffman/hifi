//
//  ReceivedMessage.h
//  libraries/networking/src/udt
//
//  Created by Ryan Huffman on 2015/09/15
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


#ifndef hifi_ReceivedMessage_h
#define hifi_ReceivedMessage_h

// #include <QBuffer>
#include <QByteArray>
#include <QObject>

// #include "../NLPacket.h"
#include "../NLPacketList.h"

// #include "PacketList.h"

class ReceivedMessage : public QObject {

    Q_OBJECT
public:
    // ReceivedMessage(const udt::PacketList& packetList);
    ReceivedMessage(const NLPacketList& packetList);
    // ReceivedMessage(NLPacket& packet);
    // ReceivedMessage(udt::Packet& packet);

    QByteArray getMessage() const { return _data; }
    // QBuffer getMessageAsBuffer() { return QBuffer(&_data); }
    //
    PacketType getType() const { return _packetType; }

    bool isComplete() const { return _isComplete; }
    const QUuid& getSourceID() const { return _sourceID; }

    void seek(qint64 position) { _position = position; }
    qint64 size() const { return _data.size(); }
    qint64 getNumPackets() const { return _numPackets; }

    qint64 peek(char* data, qint64 size);
    qint64 read(char* data, qint64 size);

    QByteArray peek(qint64 size);
    QByteArray read(qint64 size);

    template<typename T> qint64 peekPrimitive(T* data);
    template<typename T> qint64 readPrimitive(T* data);

private:
    QByteArray _data;
    QUuid _sourceID;
    qint64 _numPackets;
    PacketType _packetType;
    qint64 _position { 0 };

    std::atomic<bool> _isComplete { true };  
};

template<typename T> qint64 ReceivedMessage::peekPrimitive(T* data) {
    return peek(reinterpret_cast<char*>(data), sizeof(T));
}

template<typename T> qint64 ReceivedMessage::readPrimitive(T* data) {
    return read(reinterpret_cast<char*>(data), sizeof(T));
}

#endif
