//
//  TextureMeta.h
//  libraries/shared/src
//
//  Created by Ryan Huffman on 04/10/18.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_TextureMeta_h
#define hifi_TextureMeta_h

#include <unordered_map>
#include <QUrl>

extern const QString TEXTURE_META_EXTENSION;

enum class TextureType {
    ORIGINAL,
    BCN,
    ETC2
};

struct TextureMeta {
    static bool deserialize(const QByteArray& data, TextureMeta* meta);
    QByteArray serialize();

    std::unordered_map<TextureType, QUrl> availableTextureTypes;
};


#endif // hifi_TextureMeta_h
