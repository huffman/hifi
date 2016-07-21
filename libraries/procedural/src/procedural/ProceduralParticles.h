//
//  Created by Sam Gondelman on 7/20/16
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <RenderArgs.h>
#include <gpu/Stream.h>
#include <model-networking/TextureCache.h>

#pragma once
#ifndef hifi_RenderableProcedrualParticlesItem_h
#define hifi_RenderableProcedrualParticlesItem_h

class ProceduralParticles {
public:
    static const size_t VERTEX_PER_PARTICLE = 3;

    struct ParticleUniforms {
        glm::vec4 color;                  // rgba
        float radius;
        float firstPass{ true };         // needs to be a float to fit in buffer
        float iGlobalTime{ 0.0f };
        float iDeltaTime{ 0.0f };
        glm::vec4 iResolution{ 0.0f };   // iResolution.xy = texture size, z = maxParticles, w = spare
    };

    using Buffer = gpu::Buffer;
    using BufferView = gpu::BufferView;

    ProceduralParticles(glm::vec4 color, float radius, quint32 maxParticles, quint32 MAX_DIM);

    void setParticleRadius(float radius) { _uniforms.radius = radius; }

    void setColor(const rgbColor& color) {
        _uniforms.color.x = color[0];
        _uniforms.color.y = color[1];
        _uniforms.color.z = color[2];
    }

    void setAlpha(float alpha) { _uniforms.color.w = alpha; }

    void setMaxParticles(quint32 maxParticles, const quint32 MAX_DIM);

    const ParticleUniforms& getParticleUniforms() const { return _uniformBuffer.get<ParticleUniforms>(); }
    ParticleUniforms& editParticleUniforms() { return _uniformBuffer.edit<ParticleUniforms>(); }

    void update(float iGlobalTime, float iDeltaTime);

    void render(RenderArgs* args);

private:
    void createPipelines();

    ParticleUniforms _uniforms;
    BufferView _uniformBuffer;

    bool _evenPass{ true };
    QList<gpu::FramebufferPointer> _particleBuffers;

    // Rendering
    NetworkTexturePointer _texture;
    gpu::PipelinePointer _updatePipeline;
    gpu::PipelinePointer _untexturedPipeline;
    gpu::PipelinePointer _texturedPipeline;

};

#endif
