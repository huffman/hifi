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
#include <AbstractViewStateInterface.h>
#include "EntitiesRendererLogging.h"
#include <procedural/ProceduralParticles.h>

#include "RenderableProceduralParticlesEntityItem.h"

RenderableProceduralParticlesEntityItem::Pointer RenderableProceduralParticlesEntityItem::baseFactory(const EntityItemID& entityID, const EntityItemProperties& properties) {
    Pointer entity = std::make_shared<RenderableProceduralParticlesEntityItem>(entityID);
    entity->setProperties(properties);
    return entity;
}

EntityItemPointer RenderableProceduralParticlesEntityItem::factory(const EntityItemID& entityID, const EntityItemProperties& properties) {
    return baseFactory(entityID, properties);
}

RenderableProceduralParticlesEntityItem::RenderableProceduralParticlesEntityItem(const EntityItemID& entityItemID) :
    ProceduralParticlesEntityItem(entityItemID) {

}

void RenderableProceduralParticlesEntityItem::setParticleRadius(float radius) {
    ProceduralParticlesEntityItem::setParticleRadius(radius);
    if (_particles) {
        _particles->setParticleRadius(_particleRadius);
    }
}

void RenderableProceduralParticlesEntityItem::setColor(const xColor& color) {
    ProceduralParticlesEntityItem::setColor(color);
    if (_particles) {
        _particles->setColor(_color);
    }
}

void RenderableProceduralParticlesEntityItem::setAlpha(float alpha) {
    ProceduralParticlesEntityItem::setAlpha(alpha);
    if (_particles) {
        _particles->setAlpha(_alpha);
    }
}

void RenderableProceduralParticlesEntityItem::setMaxParticles(quint32 maxParticles) {
    ProceduralParticlesEntityItem::setMaxParticles(maxParticles);
    if (_particles) {
        _particles->setMaxParticles(_maxParticles, MAX_DIM);
    }
}

void RenderableProceduralParticlesEntityItem::update(const quint64& now) {
    ProceduralParticlesEntityItem::update(now);

    if (_particles) {
        _particles->update(_simulationTime, _deltaTime);
    }
}

void RenderableProceduralParticlesEntityItem::render(RenderArgs* args) {
    if (!_particles) {
        _particles.reset(new ProceduralParticles(glm::vec4(getColorRGB(), getAlpha()), getParticleRadius(), getMaxParticles(), MAX_DIM));
    }

    _particles->render(args);

    args->_details._trianglesRendered += (int) _maxParticles;
}