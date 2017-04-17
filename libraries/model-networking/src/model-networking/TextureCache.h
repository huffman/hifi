//
//  TextureCache.h
//  libraries/model-networking/src
//
//  Created by Andrzej Kapolka on 8/6/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_TextureCache_h
#define hifi_TextureCache_h

#include <gpu/Texture.h>

#include <QImage>
#include <QMap>
#include <QColor>
#include <QMetaEnum>

#include <DependencyManager.h>
#include <ResourceCache.h>
#include <model/TextureMap.h>
#include <ktx/KTX.h>

#include "KTXCache.h"

const int ABSOLUTE_MAX_TEXTURE_NUM_PIXELS = 8192 * 8192;

namespace gpu {
class Batch;
}

/// A simple object wrapper for an OpenGL texture.
class Texture {
public:
    gpu::TexturePointer getGPUTexture() const { return _textureSource->getGPUTexture(); }
    gpu::TextureSourcePointer _textureSource;
};

/// A texture loaded from the network.
class NetworkTexture : public Resource, public Texture, public gpu::Texture::MipInterestListener {
    Q_OBJECT

public:
    NetworkTexture(const QUrl& url, gpu::TextureType type, const QByteArray& content, int maxNumPixels);

    QString getType() const override { return "NetworkTexture"; }

    int getOriginalWidth() const { return _originalWidth; }
    int getOriginalHeight() const { return _originalHeight; }
    int getWidth() const { return _width; }
    int getHeight() const { return _height; }
    gpu::TextureType getTextureType() const { return _type; }

    gpu::TexturePointer getFallbackTexture() const;

    void handleMipInterestCallback(uint16_t level) override;
    Q_INVOKABLE void handleMipInterestLevel(uint16_t level);

signals:
    void networkTextureCreated(const QWeakPointer<NetworkTexture>& self);

public slots:
    void ktxHeaderRequestProgress(uint64_t bytesReceived, uint64_t bytesTotal) { }
    void ktxHeaderRequestFinished();

    void ktxMipRequestProgress(uint64_t bytesReceived, uint64_t bytesTotal) { }
    void ktxMipRequestFinished();

protected:
    void makeRequest() override;

    virtual bool isCacheable() const override { return _loaded; }

    virtual void downloadFinished(const QByteArray& data) override;

    Q_INVOKABLE void loadContent(const QByteArray& content);
    Q_INVOKABLE void setImage(gpu::TexturePointer texture, int originalWidth, int originalHeight);

    void startRequestForNextMipLevel();

    void startMipRangeRequest(uint16_t low, uint16_t high);
    void maybeCreateKTX();

private:
    friend class KTXReader;
    friend class ImageReader;

    gpu::TextureType _type;

    enum KTXLoadState {
        LOADING_HEADER,
        LOADING_LOWEST_SIX,
        DONE_LOADING 
    };

    bool _initialKtxLoaded { false };
    KTXFilePointer _file;
    static const uint16_t NULL_MIP_LEVEL;
    bool _sourceIsKTX { false };
    bool _ktxHeaderLoaded { false };
    std::pair<uint16_t, uint16_t> _ktxMipLevelRangeInFlight{ NULL_MIP_LEVEL, NULL_MIP_LEVEL };
    ResourceRequest* _ktxHeaderRequest { nullptr };
    ResourceRequest* _ktxMipRequest { nullptr };
    uint16_t _lowestRequestedMipLevel { NULL_MIP_LEVEL };
    uint16_t _lowestKnownPopulatedMip { NULL_MIP_LEVEL };
    QByteArray _ktxHeaderData;
    QByteArray _ktxHighMipData;

    // This is a copy of the original KTX descriptor from the source url.
    // We need this because the KTX that will be cached will likely include extra data
    // in its key/value data, and so will not match up with the original, causing
    // mip offsets to change.
    ktx::KTXDescriptorPointer _ktxDescriptor;


    int _originalWidth { 0 };
    int _originalHeight { 0 };
    int _width { 0 };
    int _height { 0 };
    int _maxNumPixels { ABSOLUTE_MAX_TEXTURE_NUM_PIXELS };
};

using NetworkTexturePointer = QSharedPointer<NetworkTexture>;

/// Stores cached textures, including render-to-texture targets.
class TextureCache : public ResourceCache, public Dependency {
    Q_OBJECT
    SINGLETON_DEPENDENCY

public:
    /// Returns the ID of the permutation/normal texture used for Perlin noise shader programs.  This texture
    /// has two lines: the first, a set of random numbers in [0, 255] to be used as permutation offsets, and
    /// the second, a set of random unit vectors to be used as noise gradients.
    const gpu::TexturePointer& getPermutationNormalTexture();

    /// Returns an opaque white texture (useful for a default).
    const gpu::TexturePointer& getWhiteTexture();

    /// Returns an opaque gray texture (useful for a default).
    const gpu::TexturePointer& getGrayTexture();

    /// Returns the a pale blue texture (useful for a normal map).
    const gpu::TexturePointer& getBlueTexture();

    /// Returns the a black texture (useful for a default).
    const gpu::TexturePointer& getBlackTexture();

    /// Returns a texture version of an image file
    static gpu::TexturePointer getImageTexture(const QString& path, gpu::TextureType type = gpu::DEFAULT_TEXTURE, QVariantMap options = QVariantMap());

    /// Loads a texture from the specified URL.
    NetworkTexturePointer getTexture(const QUrl& url, gpu::TextureType type = gpu::DEFAULT_TEXTURE,
        const QByteArray& content = QByteArray(), int maxNumPixels = ABSOLUTE_MAX_TEXTURE_NUM_PIXELS);


    gpu::TexturePointer getTextureByHash(const std::string& hash);
    gpu::TexturePointer cacheTextureByHash(const std::string& hash, const gpu::TexturePointer& texture);

protected:
    // Overload ResourceCache::prefetch to allow specifying texture type for loads
    Q_INVOKABLE ScriptableResource* prefetch(const QUrl& url, int type, int maxNumPixels = ABSOLUTE_MAX_TEXTURE_NUM_PIXELS);

    virtual QSharedPointer<Resource> createResource(const QUrl& url, const QSharedPointer<Resource>& fallback,
        const void* extra) override;

private:
    friend class ImageReader;
    friend class NetworkTexture;
    friend class DilatableNetworkTexture;

    TextureCache();
    virtual ~TextureCache();

    static const std::string KTX_DIRNAME;
    static const std::string KTX_EXT;
    KTXCache _ktxCache;
    // Map from image hashes to texture weak pointers
    std::unordered_map<std::string, std::weak_ptr<gpu::Texture>> _texturesByHashes;
    std::mutex _texturesByHashesMutex;

    gpu::TexturePointer _permutationNormalTexture;
    gpu::TexturePointer _whiteTexture;
    gpu::TexturePointer _grayTexture;
    gpu::TexturePointer _blueTexture;
    gpu::TexturePointer _blackTexture;
};

#endif // hifi_TextureCache_h
