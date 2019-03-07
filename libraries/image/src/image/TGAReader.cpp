//
//  TGAReader.cpp
//  image/src/image
//
//  Created by Ryan Huffman
//  Copyright 2019 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "TGAReader.h"

#include <QIODevice>
#include <QDebug>

static const size_t TGA_HEADER_SIZE { 0 };

QImage image::readTGA(QIODevice& content) {
    enum class TGAImageType : uint8_t
    {
        NoImageData = 0,
        UncompressedColorMapped = 1,
        UncompressedTrueColor = 2,
        UncompressedBlackWhite = 3,
        RunLengthEncodedColorMapped = 9,
        RunLengthEncodedTrueColor = 10,
        RunLengthEncodedBlackWhite = 11,
    };

    struct TGAHeader {
        uint8_t idLength;
        uint8_t colorMapType;
        TGAImageType imageType;
        struct {
            uint64_t firstEntryIndex : 16;
            uint64_t length : 16;
            uint64_t entrySize : 8;
        } colorMap;
        uint16_t xOrigin;
        uint16_t yOrigin;
        uint16_t width;
        uint16_t height;
        uint8_t pixelDepth;
        struct {
            uint8_t attributeBitsPerPixel : 4;
            uint8_t orientation : 2;
            uint8_t padding : 2;
        } imageDescriptor;
    };

    TGAHeader header;

    content.read((char*)&header.idLength, 1);
    content.read((char*)&header.colorMapType, 1);
    content.read((char*)&header.imageType, 1);
    content.read((char*)&header.colorMap, 5);
    content.read((char*)&header.xOrigin, 2);
    content.read((char*)&header.yOrigin, 2);
    content.read((char*)&header.width, 2);
    content.read((char*)&header.height, 2);
    content.read((char*)&header.pixelDepth, 1);
    content.read((char*)&header.imageDescriptor, 1);

    qDebug() << "Id Length: " << (int)header.idLength << "..";
    qDebug() << "Color map: " << (int)header.colorMap.firstEntryIndex << header.colorMap.length
             << header.colorMap.entrySize;
    qDebug() << "Color map type: " << (int)header.colorMapType << "..";
    qDebug() << "Image type: " << (int)header.imageType << "..";
    qDebug() << "Origin: " << header.xOrigin << header.yOrigin;
    qDebug() << "Size: " << header.width << header.height;
    qDebug() << "Depth: " << header.pixelDepth;
    qDebug() << "Image desc: " << header.imageDescriptor.attributeBitsPerPixel << header.imageDescriptor.orientation;

    if (header.xOrigin != 0 || header.yOrigin != 0) {
        return QImage();
    }

    char alphaMask = 0xFF >> (8 - header.imageDescriptor.attributeBitsPerPixel);

    content.skip(header.idLength);
    if (header.imageType == TGAImageType::UncompressedTrueColor) {
        QImage image { header.width, header.height, QImage::Format_ARGB32 };

        if (header.pixelDepth == 24 && header.imageDescriptor.attributeBitsPerPixel == 0) {
            for (int y = 0; y < header.height; ++y) {
                uchar* line = image.scanLine(y);
                for (int x = 0; x < header.width; ++x) {

                    content.read((char*)line, 1);     // B
                    content.read((char*)line + 1, 1); // G
                    content.read((char*)line + 2, 1); // R
                    *(line + 3) = 255;

                    line += 4;
                }
            }
            return image;
        } else if (header.pixelDepth == 32) {
            for (int y = 0; y < header.height; ++y) {
                uchar* line = image.scanLine(y);
                for (int x = 0; x < header.width; ++x) {

                    content.read((char*)line, 1);     // B
                    content.read((char*)line + 1, 1); // G
                    content.read((char*)line + 2, 1); // R
                    content.read((char*)line + 3, 1); // A
                    *(line + 3) &= alphaMask;
                    *(line + 3) = 255;

                    line += 4;
                }
            }
            return image;
        }
    } else if (header.imageType == TGAImageType::RunLengthEncodedTrueColor) {
        QImage image{ header.width, header.height, QImage::Format_ARGB32 };
        if (header.pixelDepth == 32
            || (header.pixelDepth == 24 && header.imageDescriptor.attributeBitsPerPixel == 0)) {

            bool hasAlpha = header.imageDescriptor.attributeBitsPerPixel == 8;

            for (int y = 0; y < header.height; ++y) {
                char* line = (char*)image.scanLine(y);
                int col = 0;
                while (col < header.width) {
                    char repetition;
                    content.read(&repetition, 1);
                    bool isRepetition = repetition & 0x80;
                    int length = repetition & 0x7F;
                    length++;
                    if (isRepetition) {
                        char r;
                        char g;
                        char b;
                        char a;

                        content.read(&b, 1);
                        content.read(&g, 1);
                        content.read(&r, 1);
                        if (hasAlpha) {
                            content.read(&a, 1);
                        } else {
                            a = 255;
                        }

                        while (length-- > 0) {
                            *line = b;
                            *(line + 1) = g;
                            *(line + 2) = r;
                            *(line + 3) = a;
                            line += 4;
                            col++;
                        }
                    } else {
                        while (length-- > 0) {
                            content.read(line + 0, 1);  // B
                            content.read(line + 1, 1);  // G
                            content.read(line + 2, 1);  // R
                            if (hasAlpha) {
                                content.read(line + 3, 1);
                            } else {
                                *(line + 3) + 255;

                            }

                            line += 4;
                            col++;
                        }
                    }
                }
            }
            return image;
        } else if (header.pixelDepth == 32) {
            for (int y = 0; y < header.height; ++y) {
                char* line = (char*)image.scanLine(y);
                int col = 0;
                while (col < header.width) {
                    char repetition;
                    content.read(&repetition, 1);
                    bool isRepetition = repetition & 0x80;
                    int length = repetition & 0x7F;
                    length++;
                    if (isRepetition) {
                        char r;
                        char g;
                        char b;
                        char a;
                        content.read(&b, 1);
                        content.read(&g, 1);
                        content.read(&r, 1);
                        content.read(&a, 1);
                        a &= alphaMask;
                        a = 255;
                        while (length-- > 0) {
                            *line = b;
                            *(line + 1) = g;
                            *(line + 2) = r;
                            *(line + 3) = a;
                            line += 4;
                            col++;
                        }
                    } else {
                        while (length-- > 0) {
                            content.read(line + 0, 1);  // B
                            content.read(line + 1, 1);  // G
                            content.read(line + 2, 1);  // R
                            content.read(line + 3, 1);  // A
                            line += 4;
                            col++;
                        }
                    }
                }
            }
            return image;
        }
    }


    return QImage();
}
