//
//  TextureMeta.cpp
//  libraries/shared/src
//
//  Created by Ryan Huffman on 04/10/18.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "TextureMeta.h"

#include <QJsonDocument>
#include <QJsonObject>

const QString TEXTURE_META_EXTENSION = ".texmeta.json";

QString textureTypeToString(TextureType type) {
    switch (type) {
    case TextureType::ORIGINAL: return "original";
    case TextureType::BCN: return "bcn";
    case TextureType::ETC2: return "etc2";
    }
    Q_ASSERT(false);
    return "";
}

bool TextureMeta::deserialize(const QByteArray& data, TextureMeta* meta) {
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(data, &error);
    if (!doc.isObject()) {
        return false;
    }

    auto root = doc.object();
    if (root.contains("original")) {
        meta->availableTextureTypes[TextureType::ORIGINAL] = root["original"].toString();
    }
    if (root.contains("bcn")) {
        meta->availableTextureTypes[TextureType::BCN] = root["bcn"].toString();
    }

    return true;
}

QByteArray TextureMeta::serialize() {
    QJsonDocument doc;
    QJsonObject root;

    for (auto kv : availableTextureTypes) {
        root[textureTypeToString(kv.first)] = kv.second.toString();
    }
    doc.setObject(root);

    return doc.toJson();
}
