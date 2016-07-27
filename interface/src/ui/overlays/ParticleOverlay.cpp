//
//  ParticleOverlay.cpp
//
//  Created by Sam Gondelman on 7/21/16.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ParticleOverlay.h"

#include <Application.h>

const xColor ParticleOverlay::DEFAULT_XCOLOR = { 255, 255, 255 };
const float ParticleOverlay::DEFAULT_ALPHA = 1.0f;
const uint ParticleOverlay::MAX_DIM = 1024;                                    // Limit particles so they fit in a 1024 * 1024 texture
const uint ParticleOverlay::MAXIMUM_MAX_PARTICLES = MAX_DIM * MAX_DIM / 2;     // 1024 * 1024 / 2 = 524288
const uint ParticleOverlay::DEFAULT_MAX_PARTICLES = 1000;
const float ParticleOverlay::DEFAULT_PARTICLE_RADIUS = 0.025f;
const QString ParticleOverlay::DEFAULT_TEXTURES = "";

QString const ParticleOverlay::TYPE = "particles";

ParticleOverlay::ParticleOverlay(const ParticleOverlay* particleOverlay) :
    Volume3DOverlay(particleOverlay)
{
    setColor(DEFAULT_XCOLOR);
}

void ParticleOverlay::update(float deltaTime) {
    if (_particles) {
        _simulationTime += deltaTime;
        _deltaTime = deltaTime;

        _particles->update(_simulationTime, _deltaTime);
    }
}

void ParticleOverlay::render(RenderArgs* args) {
    if (!_particles) {
        _particles.reset(new ProceduralParticles(glm::vec4(getColorRGB(), getAlpha()), getParticleRadius(), getMaxParticles(), MAX_DIM, getUserData()));
    }

    if (_particles->ready()) {
        _particles->render(args);
    }

    args->_details._trianglesRendered += (int)_maxParticles;
}

void ParticleOverlay::setMaxParticles(uint maxParticles) {
    auto prevMaxParticles = _maxParticles;
    maxParticles = std::min(maxParticles, MAXIMUM_MAX_PARTICLES);
    if (maxParticles != prevMaxParticles) {
        _maxParticles = maxParticles;

        // Restart the simulation if the number of particles changes
        setSimulationTime(0.0f);
    }
}

void ParticleOverlay::setProperties(const QVariantMap& properties) {
    Volume3DOverlay::setProperties(properties);

    auto color = properties["color"];
    if (color.isValid()) {
        setColor(vec3FromVariant(color));
        if (_particles) {
            _particles->setColor(_color);
        }
    }

    auto alpha = properties["alpha"];
    if (alpha.isValid() && alpha.canConvert<float>()) {
        setAlpha(alpha.toFloat());
        if (_particles) {
            _particles->setAlpha(_alpha);
        }
    }

    auto particleRadius = properties["particleRadius"];
    if (particleRadius.isValid() && particleRadius.canConvert<float>()) {
        setParticleRadius(particleRadius.toFloat());
        if (_particles) {
            _particles->setParticleRadius(_particleRadius);
        }
    }

    auto maxParticles = properties["maxParticles"];
    if (maxParticles.isValid() && maxParticles.canConvert<uint>()) {
        setMaxParticles(maxParticles.toUInt());
        if (_particles) {
            _particles->setMaxParticles(_maxParticles, MAX_DIM);
        }
    }

    auto textures = properties["textures"];
    if (textures.isValid() && textures.canConvert<QString>()) {
        setTextures(textures.toString());
        if (_particles && _texturesChangedFlag) {
            _particles->setTextures(_textures);
        }
    }

    auto userData = properties["userData"];
    if (userData.isValid() && userData.canConvert<QVariantMap>()) {
        setUserData(userData.toMap());
        if (_particles) {
            _particles->setUserData(_userData);
        }
    }

    // Always ignore ray intersection
    setIgnoreRayIntersection(true);
}

QVariant ParticleOverlay::getProperty(const QString& property) {
    if (property == "color") {
        return vec3toVariant(getColorRGB());
    }

    if (property == "alpha") {
        return QVariant(getAlpha());
    }

    if (property == "particleRadius") {
        return QVariant(getParticleRadius());
    }

    if (property == "maxParticles") {
        return QVariant(getMaxParticles());
    }

    if (property == "textures") {
        return QVariant(getTextures());
    }

    if (property == "userData") {
        return QVariant(getUserData());
    }

    return Volume3DOverlay::getProperty(property);
}

ParticleOverlay* ParticleOverlay::createClone() const {
    return new ParticleOverlay(this);
}
