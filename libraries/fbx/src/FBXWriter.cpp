//
//  FBXWriter.cpp
//  libraries/fbx/src
//
//  Created by Ryan Huffman on 9/5/17.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "FBXWriter.h"

#include <QDebug>

#ifdef USE_FBX_2016_FORMAT
    using FBXEndOffset = int64_t;
    using FBXPropertyCount = uint64_t;
    using FBXListLength = uint64_t;
#else
    using FBXEndOffset = int32_t;
    using FBXPropertyCount = uint32_t;
    using FBXListLength = uint32_t;
#endif

template <typename T>
void writeVector(QDataStream& out, char ch,  QVector<T> list) {
    out.device()->write(&ch, 1);
    out << (int32_t)list.length();
    out << (int32_t)0;
    out << (int32_t)0;
    for (auto& value : list) {
        out << value;
    }
}


QByteArray FBXWriter::encodeFBX(const FBXNode& root) {
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);
    out.setVersion(QDataStream::Qt_4_5);

    out.writeRawData(FBX_BINARY_PROLOG, FBX_BINARY_PROLOG.size());
    auto bytes = QByteArray(FBX_HEADER_BYTES_BEFORE_VERSION - FBX_BINARY_PROLOG.size(), '\0');
    out.writeRawData(bytes, bytes.size());

#ifdef USE_FBX_2016_FORMAT
    out << FBX_VERSION_2016;
#else
    out << FBX_VERSION_2015;
#endif

    for (auto& child : root.children) {
        encodeNode(out, child);
    }
    encodeNode(out, FBXNode());

    return data;
}

void FBXWriter::encodeNode(QDataStream& out, const FBXNode& node) {
    auto device = out.device();
    auto nodeStartPos = device->pos();

    // endOffset (temporary, updated later)
    out << (FBXEndOffset)0;

    // Property count
    out << (FBXPropertyCount)node.properties.size();

    // Property list length (temporary, updated later)
    out << (FBXListLength)0;

    out << (quint8)node.name.size();
    out.writeRawData(node.name, node.name.size());

    auto nodePropertiesStartPos = device->pos();

    for (const auto& prop : node.properties) {
        encodeFBXProperty(out, prop);
    }

    // Go back and write property list length
    auto nodePropertiesEndPos = device->pos();
    device->seek(nodeStartPos + sizeof(FBXEndOffset) + sizeof(FBXPropertyCount));
    out << (FBXListLength)(nodePropertiesEndPos - nodePropertiesStartPos);

    device->seek(nodePropertiesEndPos);

    for (auto& child : node.children) {
        encodeNode(out, child);
    }

    if (node.children.length() > 0) {
        encodeNode(out, FBXNode());
    }

    // Go back and write actual endOffset
    auto nodeEndPos = device->pos();
    device->seek(nodeStartPos);
    out << (FBXEndOffset)(nodeEndPos);

    device->seek(nodeEndPos);
}

void FBXWriter::encodeFBXProperty(QDataStream& out, const QVariant& prop) {
    auto type = prop.userType();
    switch (type) {
        case QMetaType::Short:
            out.device()->write("Y", 1);
            out << prop.value<int16_t>();
            break;

        case QVariant::Type::Bool:
            out.device()->write("C", 1);
            //out.device()->write(prop.toBool() ? 1 : 0, 1);
            out << prop.toBool();
            break;

        case QMetaType::Int:
            out.device()->write("I", 1);
            out << prop.toInt();
            break;

        encodeNode(out, FBXNode());
        case QMetaType::Float:
            out.device()->write("F", 1);
            out << prop.toFloat();
            break;

        case QMetaType::Double:
            out.device()->write("D", 1);
            out << prop.toDouble();
            break;

        case QMetaType::LongLong:
            out.device()->write("L", 1);
            out << prop.toLongLong();
            break;

        case QMetaType::QString:
        {
            auto bytes = prop.toString().toUtf8();
            out << 'S';
            out << bytes.length();
            out << bytes;
            out << (int32_t)bytes.size();
            out.writeRawData(bytes, bytes.size());
            break;
        }

        case QMetaType::QByteArray:
            {
                auto bytes = prop.toByteArray();
                out.device()->write("S", 1);
                out << (int32_t)bytes.size();
                out.writeRawData(bytes, bytes.size());
                break;
            }

        // TODO Delete? Do we ever use QList instead of QVector?
        case QVariant::Type::List:
            {
                auto list = prop.toList();
                auto listType = prop.userType();

                switch (listType) {
                    case QMetaType::Float:
                        out.device()->write("f", 1);
                        out << (int32_t)list.length();
                        out << (int32_t)0;
                        out << (int32_t)0;
                        for (auto& innerProp : list) {
                            out << innerProp.toFloat();
                        }
                        break;

                    case QMetaType::Double:
                        out.device()->write("d", 1);
                        out << (int32_t)list.length();
                        out << (int32_t)0;
                        out << (int32_t)0;
                        for (auto& innerProp : list) {
                            out << innerProp.toDouble();
                        }
                        break;

                    case QMetaType::LongLong:
                        out.device()->write("l", 1);
                        out << (int32_t)list.length();
                        out << (int32_t)0;
                        out << (int32_t)0;
                        for (auto& innerProp : list) {
                            out << innerProp.toLongLong();
                        }
                        break;

                    case QMetaType::Int:
                        out.device()->write("i", 1);
                        out << (int32_t)list.length();
                        out << (int32_t)0;
                        out << (int32_t)0;
                        for (auto& innerProp : list) {
                            out << innerProp.toInt();
                        }
                        break;

                    case QMetaType::Bool:
                        out.device()->write("b", 1);
                        out << (int32_t)list.length();
                        out << (int32_t)0;
                        out << (int32_t)0;
                        for (auto& innerProp : list) {
                            out << innerProp.toBool();
                        }
                        break;
                }
            }
            break;

        default:
        {
            if (prop.canConvert<QVector<float>>()) {
                writeVector(out, 'f', prop.value<QVector<float>>());
            } else if (prop.canConvert<QVector<double>>()) {
                writeVector(out, 'd', prop.value<QVector<double>>());
            } else if (prop.canConvert<QVector<qint64>>()) {
                writeVector(out, 'l', prop.value<QVector<qint64>>());
            } else if (prop.canConvert<QVector<qint32>>()) {
                writeVector(out, 'i', prop.value<QVector<qint32>>());
            } else if (prop.canConvert<QVector<bool>>()) {
                writeVector(out, 'b', prop.value<QVector<bool>>());
            } else {
                qDebug() << "Unsupported property type in FBXWriter::encodeNode: " << type << prop;
                throw("Unsupported property type in FBXWriter::encodeNode: " + QString::number(type) + " " + prop.toString());
            }
        }

    }
}
