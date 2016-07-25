//
//  Created by Sam Gondelman on 7/20/16
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QJsonObject>
#include <QJsonArray>
#include <RenderArgs.h>
#include <gpu/Stream.h>
#include <model-networking/TextureCache.h>
#include <model-networking/ShaderCache.h>

#pragma once
#ifndef hifi_RenderableProcedrualParticlesItem_h
#define hifi_RenderableProcedrualParticlesItem_h

class ProceduralParticles {
public:
    static const size_t VERTEX_PER_PARTICLE = 3;
    static QJsonValue getProceduralData(const QString& proceduralJson);

    struct ParticleUniforms {
        glm::vec4 color;                  // rgba
        float radius;
        float firstPass { true };         // needs to be a float to fit in buffer
        float iGlobalTime { 0.0f };
        float iDeltaTime { 0.0f };
        glm::vec4 iResolution { 0.0f };   // iResolution.xy = texture size, z = maxParticles, w = spare
    };

    using Buffer = gpu::Buffer;
    using BufferView = gpu::BufferView;

    ProceduralParticles(glm::vec4 color, float radius, quint32 maxParticles, quint32 MAX_DIM);
    ProceduralParticles(glm::vec4 color, float radius, quint32 maxParticles, quint32 MAX_DIM, const QString& userDataJson);
    ProceduralParticles(glm::vec4 color, float radius, quint32 maxParticles, quint32 MAX_DIM, const QVariantMap& userDataJson);

    void setUserData(const QString& userDataJson);
    void setUserData(const QVariantMap& userDataJson);

    void setColor(const rgbColor& color) {
        _particleUniforms.color.x = color[0];
        _particleUniforms.color.y = color[1];
        _particleUniforms.color.z = color[2];
    }
    void setAlpha(float alpha) { _particleUniforms.color.w = alpha; }
    void setParticleRadius(float radius) { _particleUniforms.radius = radius; }
    void setMaxParticles(quint32 maxParticles, const quint32 MAX_DIM);
    void setTextures(const QString& textures);

    const ParticleUniforms& getParticleUniforms() const { return _uniformBuffer.get<ParticleUniforms>(); }
    ParticleUniforms& editParticleUniforms() { return _uniformBuffer.edit<ParticleUniforms>(); }
    float& editHifiUniforms() { return _hifiBuffer.edit<float>(); }

    void update(float iGlobalTime, float iDeltaTime);

    bool ready();
    void render(RenderArgs* args);

protected:
    // ProceduralParticles metadata
    bool _enabled { false };

    QJsonObject _proceduralData;
    std::mutex _proceduralDataMutex;
    std::atomic_bool _proceduralDataDirty;

    QUrl _shaderUrl;
    NetworkShaderPointer _networkShader;
    QString _shaderSource;
    QString _shaderPath;
    quint64 _shaderModified { 0 };
    QJsonArray _parsedUniforms;
    bool _shadersCompiled { true };
    bool _shaderDirty { true };
    bool _uniformsDirty { true };

private:
    // This should only be called from the render thread, as it shares data with ProceduralParticles::prepare
    void parse(const QJsonObject&);
    bool parseUrl(const QUrl& url);
    bool parseUniforms(const QJsonArray& uniforms);

    void setupUniforms();

    void createPipelines();

    ParticleUniforms _particleUniforms;
    BufferView _uniformBuffer;
    BufferView _hifiBuffer;

    bool _evenPass { true };
    QList<gpu::FramebufferPointer> _particleBuffers;

    // Rendering
    NetworkTexturePointer _texture;
    gpu::PipelinePointer _updatePipeline;
    gpu::PipelinePointer _untexturedPipeline;
    gpu::PipelinePointer _texturedPipeline;

};

#endif
