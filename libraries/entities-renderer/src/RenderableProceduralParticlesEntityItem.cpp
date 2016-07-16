//
//  RenderableProceduralParticlesEntityItem.cpp
//  interface/src
//
//  Created by Sam Gondelman on 7/13/16.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <glm/gtx/quaternion.hpp>

#include <DependencyManager.h>
#include <PerfStat.h>
#include <GeometryCache.h>
#include <FramebufferCache.h>
#include <AbstractViewStateInterface.h>
#include "EntitiesRendererLogging.h"

#include "RenderableProceduralParticlesEntityItem.h"

#include "untextured_procedural_particle_vert.h"
#include "untextured_procedural_particle_frag.h"
#include "textured_procedural_particle_vert.h"
#include "textured_procedural_particle_frag.h"
#include "procedural_particle_update_frag.h"
#include <gpu/DrawUnitQuadTexcoord_vert.h>


class ProceduralParticlePayloadData {
public:
    static const size_t VERTEX_PER_PARTICLE = 3;

    struct ParticleUniforms {
        float radius;
        glm::vec4 color; // rgba
        bool firstPass;
        float iGlobalTime;
        float iDeltaTime;
        glm::vec2 iResolution;
        float maxParticles;
    };

    using Payload = render::Payload<ProceduralParticlePayloadData>;
    using Pointer = Payload::DataPointer;
    using PipelinePointer = gpu::PipelinePointer;
    using FormatPointer = gpu::Stream::FormatPointer;
    using BufferPointer = gpu::BufferPointer;
    using TexturePointer = gpu::TexturePointer;
    using Format = gpu::Stream::Format;
    using Buffer = gpu::Buffer;
    using BufferView = gpu::BufferView;

    ProceduralParticlePayloadData() {
        ParticleUniforms uniforms;
        _uniformBuffer = std::make_shared<Buffer>(sizeof(ParticleUniforms), (const gpu::Byte*) &uniforms);
    }

    void setUpdatePipeline(PipelinePointer updatePipeline) { _updatePipeline = updatePipeline; }
    const PipelinePointer& getUpdatePipeline() const { return _updatePipeline; }

    void setPipeline(PipelinePointer pipeline) { _pipeline = pipeline; }
    const PipelinePointer& getPipeline() const { return _pipeline; }

    const Transform& getModelTransform() const { return _modelTransform; }
    void setModelTransform(const Transform& modelTransform) { _modelTransform = modelTransform; }

    const AABox& getBound() const { return _bound; }
    void setBound(const AABox& bound) { _bound = bound; }

    const int& getMaxParticles() const { return _maxParticles; }
    void setMaxParticles(int maxParticles) { _maxParticles = maxParticles; }

    const ParticleUniforms& getParticleUniforms() const { return _uniformBuffer.get<ParticleUniforms>(); }
    ParticleUniforms& editParticleUniforms() { return _uniformBuffer.edit<ParticleUniforms>(); }

    QList<gpu::FramebufferPointer> getParticleBuffers() { return _particleBuffers; }
    void setParticleBuffers(QList<gpu::FramebufferPointer> particleBuffers) { _particleBuffers = particleBuffers; }

    void setTexture(TexturePointer texture) { _texture = texture; }
    const TexturePointer& getTexture() const { return _texture; }

    bool getVisibleFlag() const { return _visibleFlag; }
    void setVisibleFlag(bool visibleFlag) { _visibleFlag = visibleFlag; }

    bool getEvenPass() const { return _evenPass; }
    void setEvenPass(bool evenPass) { _evenPass = evenPass; }

    void render(RenderArgs* args) const {
        assert(_pipeline && _updatePipeline && _particleBuffers.length() == 2);
        auto mainViewport = args->_viewport;
        gpu::Batch& batch = *args->_batch;

        // Set particle uniforms (used for both updating and rendering)
        batch.setUniformBuffer(0, _uniformBuffer);

        // Update the particles in the other FBO based on the current FBO's texture
        batch.setPipeline(_updatePipeline);
        batch.setFramebuffer(_particleBuffers[(int)!_evenPass]);
        glm::ivec4 viewport = glm::ivec4(0, 0, _particleBuffers[(int)!_evenPass]->getWidth(), _particleBuffers[(int)!_evenPass]->getHeight());
        batch.setViewportTransform(viewport);
        batch.setResourceTexture(0, _particleBuffers[(int)_evenPass]->getRenderBuffer(0));
        batch.draw(gpu::TRIANGLE_STRIP, 4);

        // Render using the updated FBO's texture
        batch.setPipeline(_pipeline);
        auto lightingFramebuffer = DependencyManager::get<FramebufferCache>()->getLightingFramebuffer();
        batch.setFramebuffer(lightingFramebuffer);
        batch.setViewportTransform(mainViewport);
        batch.setResourceTexture(0, _particleBuffers[(int)!_evenPass]->getRenderBuffer(0));
        if (_texture) {
            batch.setResourceTexture(1, _texture);
        }

        batch.setModelTransform(_modelTransform);

        batch.drawInstanced((gpu::uint32)_maxParticles, gpu::TRIANGLES, (gpu::uint32)VERTEX_PER_PARTICLE);
    }

protected:
    Transform _modelTransform;
    AABox _bound;
    PipelinePointer _pipeline;
    int _maxParticles;
    BufferView _uniformBuffer;
    TexturePointer _texture;
    bool _visibleFlag { true };

    PipelinePointer _updatePipeline;
    bool _evenPass;
    QList<gpu::FramebufferPointer> _particleBuffers;
};

namespace render {
    template <>
    const ItemKey payloadGetKey(const ProceduralParticlePayloadData::Pointer& payload) {
        if (payload->getVisibleFlag()) {
            return ItemKey::Builder::transparentShape();
        } else {
            return ItemKey::Builder().withInvisible().build();
        }
    }

    template <>
    const Item::Bound payloadGetBound(const ProceduralParticlePayloadData::Pointer& payload) {
        return payload->getBound();
    }

    template <>
    void payloadRender(const ProceduralParticlePayloadData::Pointer& payload, RenderArgs* args) {
        if (payload->getVisibleFlag()) {
            payload->render(args);
        }
    }
}

EntityItemPointer RenderableProceduralParticlesEntityItem::factory(const EntityItemID& entityID,
                                                                   const EntityItemProperties& properties) {
    auto entity = std::make_shared<RenderableProceduralParticlesEntityItem>(entityID);
    entity->setProperties(properties);
    return entity;
}

RenderableProceduralParticlesEntityItem::RenderableProceduralParticlesEntityItem(const EntityItemID& entityItemID) :
    ProceduralParticlesEntityItem(entityItemID) {
    // lazy creation of particle system pipeline
    if (!_updatePipeline || !_untexturedPipeline || !_texturedPipeline) {
        createPipelines();
    }
    // Create the FBOs
    // TODO: don't default to MAX_DIM * MAX_DIM
    for (int i = 0; i < 2; i++) {
        _particleBuffers.append(gpu::FramebufferPointer(gpu::Framebuffer::create(gpu::Format(gpu::Dimension::VEC4,
                                                                                             gpu::Type::FLOAT,
                                                                                             gpu::Semantic::RGBA), MAX_DIM, MAX_DIM)));
    }
    // Resize the FBOs
    setMaxParticles(_maxParticles);
}

bool RenderableProceduralParticlesEntityItem::addToScene(EntityItemPointer self,
                                                         render::ScenePointer scene,
                                                         render::PendingChanges& pendingChanges) {
    _scene = scene;
    _renderItemId = _scene->allocateID();
    auto particlePayloadData = std::make_shared<ProceduralParticlePayloadData>();
    particlePayloadData->setUpdatePipeline(_updatePipeline);
    particlePayloadData->setPipeline(_untexturedPipeline);
    particlePayloadData->setParticleBuffers(_particleBuffers);
    auto renderPayload = std::make_shared<ProceduralParticlePayloadData::Payload>(particlePayloadData);
    render::Item::Status::Getters statusGetters;
    makeEntityItemStatusGetters(getThisPointer(), statusGetters);
    renderPayload->addStatusGetters(statusGetters);
    pendingChanges.resetItem(_renderItemId, renderPayload);
    return true;
}

void RenderableProceduralParticlesEntityItem::removeFromScene(EntityItemPointer self,
                                                              render::ScenePointer scene,
                                                              render::PendingChanges& pendingChanges) {
    pendingChanges.removeItem(_renderItemId);
    _scene = nullptr;
    render::Item::clearID(_renderItemId);
};

void RenderableProceduralParticlesEntityItem::setMaxParticles(quint32 maxParticles) {
    ProceduralParticlesEntityItem::setMaxParticles(maxParticles);

    int width = std::min(MAX_DIM, _maxParticles);
    int height = ((int)(_maxParticles / MAX_DIM) + 1) * 2;
    for (auto& buffer : _particleBuffers) {
        buffer->resize(width, height);
    }
}

void RenderableProceduralParticlesEntityItem::update(const quint64& now) {
    ProceduralParticlesEntityItem::update(now);

    if (_texturesChangedFlag) {
        if (_textures.isEmpty()) {
            _texture.clear();
        } else {
            // for now use the textures string directly.
            // Eventually we'll want multiple textures in a map or array.
            _texture = DependencyManager::get<TextureCache>()->getTexture(_textures);
        }
        _texturesChangedFlag = false;
    }

    updateRenderItem();

    // These needs to be below updateRenderItem() to guarentee that the payload gets the correct state
    _firstPass = false;
    _evenPass = !_evenPass;
}

void RenderableProceduralParticlesEntityItem::updateRenderItem() {
    // this 2 tests are synonyms for this class, but we would like to get rid of the _scene pointer ultimately
    if (!_scene || !render::Item::isValidID(_renderItemId)) {
        return;
    }
    if (!getVisible()) {
        render::PendingChanges pendingChanges;
        pendingChanges.updateItem<ProceduralParticlePayloadData>(_renderItemId, [](ProceduralParticlePayloadData& payload) {
            payload.setVisibleFlag(false);
        });

        _scene->enqueuePendingChanges(pendingChanges);
        return;
    }

    using ParticleUniforms = ProceduralParticlePayloadData::ParticleUniforms;

    // Fill in Uniforms structure
    ParticleUniforms particleUniforms;
    particleUniforms.radius = getParticleRadius();
    particleUniforms.color = glm::vec4(getColorRGB(), getAlpha());
    particleUniforms.firstPass = getFirstPass();
    particleUniforms.iGlobalTime = getSimulationTime();
    particleUniforms.iDeltaTime = getDeltaTime();
    particleUniforms.iResolution = glm::vec2(_particleBuffers[0]->getWidth(), _particleBuffers[0]->getHeight());
    particleUniforms.maxParticles = getMaxParticles();

    int maxParticles = getMaxParticles();
    bool evenPass = _evenPass;

    auto particleBuffers = _particleBuffers;

    bool success;
    auto bounds = getAABox(success);;
    if (!success) {
        return;
    }

    Transform transform;
    render::PendingChanges pendingChanges;
    pendingChanges.updateItem<ProceduralParticlePayloadData>(_renderItemId, [=](ProceduralParticlePayloadData& payload) {
        payload.setVisibleFlag(true);

        // Update particle uniforms
        memcpy(&payload.editParticleUniforms(), &particleUniforms, sizeof(ParticleUniforms));

        payload.setParticleBuffers(particleBuffers);
        payload.setMaxParticles(maxParticles);
        payload.setEvenPass(evenPass);

        // Update transform and bounds
        payload.setModelTransform(transform);
        payload.setBound(bounds);

        payload.setUpdatePipeline(_updatePipeline);

        if (_texture && _texture->isLoaded()) {
            payload.setTexture(_texture->getGPUTexture());
            payload.setPipeline(_texturedPipeline);
        } else {
            payload.setTexture(nullptr);
            payload.setPipeline(_untexturedPipeline);
        }
    });

    _scene->enqueuePendingChanges(pendingChanges);
}

void RenderableProceduralParticlesEntityItem::createPipelines() {
    if (!_updatePipeline) {
        auto state = std::make_shared<gpu::State>();
        state->setCullMode(gpu::State::CULL_BACK);

        auto vertShader = gpu::Shader::createVertex(std::string(DrawUnitQuadTexcoord_vert));
        auto fragShader = gpu::Shader::createPixel(std::string(procedural_particle_update_frag));

        auto program = gpu::Shader::createProgram(vertShader, fragShader);
        _updatePipeline = gpu::Pipeline::create(program, state);
    }
    if (!_untexturedPipeline) {
        auto state = std::make_shared<gpu::State>();
        state->setCullMode(gpu::State::CULL_BACK);
        state->setDepthTest(true, false, gpu::LESS_EQUAL);
        state->setBlendFunction(true, gpu::State::SRC_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE,
                                gpu::State::FACTOR_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE);

        auto vertShader = gpu::Shader::createVertex(std::string(untextured_procedural_particle_vert));
        auto fragShader = gpu::Shader::createPixel(std::string(untextured_procedural_particle_frag));

        auto program = gpu::Shader::createProgram(vertShader, fragShader);
        _untexturedPipeline = gpu::Pipeline::create(program, state);
    }
    if (!_texturedPipeline) {
        auto state = std::make_shared<gpu::State>();
        state->setCullMode(gpu::State::CULL_BACK);
        state->setDepthTest(true, false, gpu::LESS_EQUAL);
        state->setBlendFunction(true, gpu::State::SRC_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE,
                                gpu::State::FACTOR_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE);

        auto vertShader = gpu::Shader::createVertex(std::string(textured_procedural_particle_vert));
        auto fragShader = gpu::Shader::createPixel(std::string(textured_procedural_particle_frag));

        auto program = gpu::Shader::createProgram(vertShader, fragShader);
        _texturedPipeline = gpu::Pipeline::create(program, state);
    }
}

void RenderableProceduralParticlesEntityItem::notifyBoundChanged() {
    if (!render::Item::isValidID(_renderItemId)) {
        return;
    }
    render::PendingChanges pendingChanges;
    pendingChanges.updateItem<ProceduralParticlePayloadData>(_renderItemId, [](ProceduralParticlePayloadData& payload) {
    });

    _scene->enqueuePendingChanges(pendingChanges);
}