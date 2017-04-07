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
class NetworkTexture : public Resource, public Texture {
    Q_OBJECT

public:
     enum Type {
        DEFAULT_TEXTURE,
        STRICT_TEXTURE,
        ALBEDO_TEXTURE,
        NORMAL_TEXTURE,
        BUMP_TEXTURE,
        SPECULAR_TEXTURE,
        METALLIC_TEXTURE = SPECULAR_TEXTURE, // for now spec and metallic texture are the same, converted to grey
        ROUGHNESS_TEXTURE,
        GLOSS_TEXTURE,
        EMISSIVE_TEXTURE,
        CUBE_TEXTURE,
        OCCLUSION_TEXTURE,
        SCATTERING_TEXTURE = OCCLUSION_TEXTURE,
        LIGHTMAP_TEXTURE,
        CUSTOM_TEXTURE
    };
    Q_ENUM(Type)

    typedef gpu::Texture* TextureLoader(const QImage& image, const std::string& srcImageName);
    using TextureLoaderFunc = std::function<TextureLoader>;

    NetworkTexture(const QUrl& url, Type type, const QByteArray& content, int maxNumPixels);

    QString getType() const override { return "NetworkTexture"; }

    int getOriginalWidth() const { return _originalWidth; }
    int getOriginalHeight() const { return _originalHeight; }
    int getWidth() const { return _width; }
    int getHeight() const { return _height; }
    Type getTextureType() const { return _type;  }

    TextureLoaderFunc getTextureLoader() const;
    gpu::TexturePointer getFallbackTexture() const;

signals:
    void networkTextureCreated(const QWeakPointer<NetworkTexture>& self);

protected:
    virtual bool isCacheable() const override { return _loaded; }

    virtual void downloadFinished(const QByteArray& data) override;

    Q_INVOKABLE void loadContent(const QByteArray& content);
    Q_INVOKABLE void setImage(gpu::TexturePointer texture, int originalWidth, int originalHeight);

private:
    friend class KTXReader;
    friend class ImageReader;

    enum KTXLoadState {
        LOADING_HEADER,
        LOADING_LOWEST_SIX,
        DONE_LOADING 
    };

    Type _type;
    TextureLoaderFunc _textureLoader { [](const QImage&, const std::string&){ return nullptr; } };
    KTXLoadState _ktxLoadState { LOADING_HEADER };
    KTXFilePointer _file;
    bool _sourceIsKTX { false };
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

    using Type = NetworkTexture::Type;

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
    static gpu::TexturePointer getImageTexture(const QString& path, Type type = Type::DEFAULT_TEXTURE, QVariantMap options = QVariantMap());

    /// Loads a texture from the specified URL.
    NetworkTexturePointer getTexture(const QUrl& url, Type type = Type::DEFAULT_TEXTURE,
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
