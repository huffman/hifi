//
//  ParticleOverlay.h
//
//  Created by Sam Gondelman on 7/21/16.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_ParticleOverlay_h
#define hifi_ParticleOverlay_h

#include <procedural/ProceduralParticles.h>

#include "Volume3DOverlay.h"

#include "OctalCode.h"
#include "ColorUtils.h"

class ParticleOverlay : public Volume3DOverlay {
    Q_OBJECT
public:
    static QString const TYPE;
    virtual QString getType() const override { return TYPE; }

    ParticleOverlay() { setColor(DEFAULT_XCOLOR); }
    ParticleOverlay(const ParticleOverlay* particleOverlay);

    virtual void update(float deltaTime) override;
    virtual void render(RenderArgs* args) override;

    static const xColor DEFAULT_XCOLOR;
    xColor getXColor() const { xColor color = { _color[RED_INDEX], _color[GREEN_INDEX], _color[BLUE_INDEX] }; return color; }
    glm::vec3 getColorRGB() const { return  ColorUtils::sRGBToLinearVec3(toGlm(getXColor())); }
    void setColor(const xColor& value) {
        _color[RED_INDEX] = value.red;
        _color[GREEN_INDEX] = value.green;
        _color[BLUE_INDEX] = value.blue;
    }
    void setColor(glm::vec3 color) {
        _color[RED_INDEX] = color.r;
        _color[GREEN_INDEX] = color.g;
        _color[BLUE_INDEX] = color.b;
    }

    static const float DEFAULT_ALPHA;
    void setAlpha(float alpha) { _alpha = alpha; }
    float getAlpha() const { return _alpha; }

    static const quint32 MAX_DIM;
    static const quint32 MAXIMUM_MAX_PARTICLES;
    static const quint32 DEFAULT_MAX_PARTICLES;
    void setMaxParticles(uint maxParticles);
    uint getMaxParticles() const { return _maxParticles; }

    static const float DEFAULT_PARTICLE_RADIUS;
    void setParticleRadius(float particleRadius) { _particleRadius = particleRadius; }
    float getParticleRadius() const { return _particleRadius; }

    float getSimulationTime() const { return _simulationTime; }
    void setSimulationTime(float simulationTime) { _simulationTime = simulationTime; }

    float getDeltaTime() const { return _deltaTime; }
    void setDeltaTime(float deltaTime) { _deltaTime = deltaTime; }

    static const QString DEFAULT_TEXTURES;
    const QString& getTextures() const { return _textures; }
    void setTextures(const QString& textures) {
        if (_textures != textures) {
            _textures = textures;
            _texturesChangedFlag = true;
        }
    }

    const QVariantMap& getUserData() { return _userData; }
    void setUserData(const QVariantMap& userData) { _userData = userData; }

    void setProperties(const QVariantMap& properties) override;
    QVariant getProperty(const QString& property) override;

    virtual ParticleOverlay* createClone() const override;

private:
    // Particles properties
    rgbColor _color;
    float _alpha { DEFAULT_ALPHA };
    float _particleRadius { DEFAULT_PARTICLE_RADIUS };

    // Emiter properties
    uint _maxParticles { DEFAULT_MAX_PARTICLES };

    QString _textures { DEFAULT_TEXTURES };
    bool _texturesChangedFlag { false };

    QVariantMap _userData;

    float _simulationTime { 0.0f };
    float _deltaTime { 0.0f };

    QSharedPointer<ProceduralParticles> _particles;

};

#endif // hifi_ParticleOverlay_h
