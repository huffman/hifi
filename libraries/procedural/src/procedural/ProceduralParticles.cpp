//
//  Created by Sam Gondelman on 7/20/16
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ProceduralParticles.h"

#include "glm/glm.hpp"

#include <gpu/Batch.h>
#include <gpu/Framebuffer.h>
#include <FramebufferCache.h>

#include <DependencyManager.h>

#include "procedural_particle_update_frag.h"
#include "draw_procedural_particle_vert.h"
#include "untextured_procedural_particle_frag.h"
#include "textured_procedural_particle_frag.h"
#include <gpu/DrawUnitQuadTexcoord_vert.h>

static int UPDATE_BUFFER = -1;
static int UPDATE_PARTICLES = -1;
static int NOTEX_DRAW_BUFFER = -1;
static int NOTEX_DRAW_PARTICLES = -1;
static int DRAW_BUFFER = -1;
static int DRAW_PARTICLES = 0;
static int DRAW_TEXTURE = 1;

ProceduralParticles::ProceduralParticles(glm::vec4 color, float radius, quint32 maxParticles, quint32 MAX_DIM) {
    _uniforms = ParticleUniforms();
    _uniforms.color = color;
    _uniforms.radius = radius;
    _uniforms.iResolution.z = maxParticles;

    // Create the FBOs
    for (int i = 0; i < 2; i++) {
        _particleBuffers.append(gpu::FramebufferPointer(gpu::Framebuffer::create(gpu::Format(gpu::Dimension::VEC4,
                                gpu::Type::FLOAT,
                                gpu::Semantic::RGBA), 1, 1)));
    }

    // Resize the FBOs
    setMaxParticles(maxParticles, MAX_DIM);

    _uniformBuffer = std::make_shared<Buffer>(sizeof(ParticleUniforms), (const gpu::Byte*) &_uniforms);
}

void ProceduralParticles::setMaxParticles(quint32 maxParticles, const quint32 MAX_DIM) {
    _uniforms.iResolution.z = maxParticles;

    int width = std::min(MAX_DIM, maxParticles);
    int height = ((int)(maxParticles / MAX_DIM) + 1) * 2;
    for (auto& buffer : _particleBuffers) {
        buffer->resize(width, height);
    }

    _uniforms.iResolution.x = width;
    _uniforms.iResolution.y = height;

    // Restart the simulation when the number of particles changes
    _uniforms.firstPass = true;
}

void ProceduralParticles::setTextures(const QString& textures) {
    if (textures.isEmpty()) {
        _texture.clear();
    } else {
        _texture = DependencyManager::get<TextureCache>()->getTexture(textures);
    }
}

void ProceduralParticles::update(float iGlobalTime, float iDeltaTime) {
    _uniforms.iGlobalTime = iGlobalTime;
    _uniforms.iDeltaTime = iDeltaTime;

    // TODO: remove (for testing)
    if (!_texture) {
        _texture = DependencyManager::get<TextureCache>()->getTexture(QString("https://hifi-public.s3.amazonaws.com/alan/Particles/Particle-Sprite-Smoke-1.png"));
    }
}

void ProceduralParticles::render(RenderArgs* args) {
    // lazy creation of particle system pipeline
    if (!_updatePipeline || !_untexturedPipeline || !_texturedPipeline) {
        createPipelines();
    }

    auto mainViewport = args->_viewport;
    gpu::Batch& batch = *args->_batch;

    memcpy(&editParticleUniforms(), &_uniforms, sizeof(ParticleUniforms));

    // Update the particles in the other FBO based on the current FBO's texture
    batch.setPipeline(_updatePipeline);
    batch.setUniformBuffer(UPDATE_BUFFER, _uniformBuffer);
    batch.setFramebuffer(_particleBuffers[(int)!_evenPass]);
    glm::ivec4 viewport = glm::ivec4(0, 0, _particleBuffers[(int)!_evenPass]->getWidth(), _particleBuffers[(int)!_evenPass]->getHeight());
    batch.setViewportTransform(viewport);
    batch.setResourceTexture(UPDATE_PARTICLES, _particleBuffers[(int)_evenPass]->getRenderBuffer(0));
    batch.draw(gpu::TRIANGLE_STRIP, 4);

    // Render using the updated FBO's texture
    auto lightingFramebuffer = DependencyManager::get<FramebufferCache>()->getLightingFramebuffer();
    batch.setFramebuffer(lightingFramebuffer);
    batch.setViewportTransform(mainViewport);
    if (_texture && _texture->isLoaded()) {
        batch.setPipeline(_texturedPipeline);
        batch.setUniformBuffer(DRAW_BUFFER, _uniformBuffer);
        batch.setResourceTexture(DRAW_PARTICLES, _particleBuffers[(int)!_evenPass]->getRenderBuffer(0));
        batch.setResourceTexture(DRAW_TEXTURE, _texture->getGPUTexture());
    }
    else {
        batch.setPipeline(_untexturedPipeline);
        batch.setUniformBuffer(NOTEX_DRAW_BUFFER, _uniformBuffer);
        batch.setResourceTexture(NOTEX_DRAW_PARTICLES, _particleBuffers[(int)!_evenPass]->getRenderBuffer(0));
    }

    batch.setModelTransform(Transform());

    batch.draw(gpu::TRIANGLES, 3 * _uniforms.iResolution.z);
    //batch.drawInstanced((gpu::uint32)_uniforms.iResolution.z, gpu::TRIANGLES, (gpu::uint32)VERTEX_PER_PARTICLE);

    _uniforms.firstPass = false;
    _evenPass = !_evenPass;
}

void ProceduralParticles::createPipelines() {
    std::string uniformBuffer = "particleBuffer";
    std::string particlesTex = "particlesTex";
    std::string colorMap = "colorMap";
    if (!_updatePipeline) {
        auto state = std::make_shared<gpu::State>();
        state->setCullMode(gpu::State::CULL_BACK);

        auto vertShader = gpu::Shader::createVertex(std::string(DrawUnitQuadTexcoord_vert));
        auto fragShader = gpu::Shader::createPixel(std::string(procedural_particle_update_frag));

        auto program = gpu::Shader::createProgram(vertShader, fragShader);

        gpu::Shader::BindingSet slotBindings;
        gpu::Shader::makeProgram(*program, slotBindings);

        UPDATE_BUFFER = program->getBuffers().findLocation(uniformBuffer);
        UPDATE_PARTICLES = program->getTextures().findLocation(particlesTex);

        _updatePipeline = gpu::Pipeline::create(program, state);
    }
    if (!_untexturedPipeline) {
        auto state = std::make_shared<gpu::State>();
        state->setCullMode(gpu::State::CULL_BACK);
        state->setDepthTest(true, false, gpu::LESS_EQUAL);
        state->setBlendFunction(true, gpu::State::SRC_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE,
            gpu::State::FACTOR_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE);

        auto vertShader = gpu::Shader::createVertex(std::string(draw_procedural_particle_vert));
        auto fragShader = gpu::Shader::createPixel(std::string(untextured_procedural_particle_frag));
        auto program = gpu::Shader::createProgram(vertShader, fragShader);

        gpu::Shader::BindingSet slotBindings;
        gpu::Shader::makeProgram(*program, slotBindings);

        NOTEX_DRAW_BUFFER = program->getBuffers().findLocation(uniformBuffer);
        NOTEX_DRAW_PARTICLES = program->getTextures().findLocation(particlesTex);

        _untexturedPipeline = gpu::Pipeline::create(program, state);
    }
    if (!_texturedPipeline) {
        auto state = std::make_shared<gpu::State>();
        state->setCullMode(gpu::State::CULL_BACK);
        state->setDepthTest(true, false, gpu::LESS_EQUAL);
        state->setBlendFunction(true, gpu::State::SRC_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE,
            gpu::State::FACTOR_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE);

        auto vertShader = gpu::Shader::createVertex(std::string(draw_procedural_particle_vert));
        auto fragShader = gpu::Shader::createPixel(std::string(textured_procedural_particle_frag));
        auto program = gpu::Shader::createProgram(vertShader, fragShader);

        // Request these specifically because it doesn't work otherwise
        gpu::Shader::BindingSet slotBindings;
        slotBindings.insert(gpu::Shader::Binding(particlesTex, DRAW_PARTICLES));
        slotBindings.insert(gpu::Shader::Binding(colorMap, DRAW_TEXTURE));
        gpu::Shader::makeProgram(*program, slotBindings);

        DRAW_BUFFER = program->getBuffers().findLocation(uniformBuffer);
        DRAW_PARTICLES = program->getTextures().findLocation(particlesTex);
        DRAW_TEXTURE = program->getTextures().findLocation(colorMap);

        _texturedPipeline = gpu::Pipeline::create(program, state);
    }
}