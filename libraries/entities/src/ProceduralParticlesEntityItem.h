//
//  ProceduralParticlesEntityItem.h
//  libraries/entities/src
//
//  Created by Sam Gondelman on 7/13/16.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_ProceduralParticlesEntityItem_h
#define hifi_ProceduralParticlesEntityItem_h

#include "EntityItem.h"

#include "ColorUtils.h"

class ProceduralParticlesEntityItem : public EntityItem {
public:
    ALLOW_INSTANTIATION // This class can be instantiated

    static EntityItemPointer factory(const EntityItemID& entityID, const EntityItemProperties& properties);

    ProceduralParticlesEntityItem(const EntityItemID& entityItemID);

    // methods for getting/setting all properties of this entity
    virtual EntityItemProperties getProperties(EntityPropertyFlags desiredProperties = EntityPropertyFlags()) const;
    virtual bool setProperties(const EntityItemProperties& properties);

    virtual EntityPropertyFlags getEntityProperties(EncodeBitstreamParams& params) const;

    virtual void appendSubclassData(OctreePacketData* packetData, EncodeBitstreamParams& params,
                                    EntityTreeElementExtraEncodeData* entityTreeElementExtraEncodeData,
                                    EntityPropertyFlags& requestedProperties,
                                    EntityPropertyFlags& propertyFlags,
                                    EntityPropertyFlags& propertiesDidntFit,
                                    int& propertyCount,
                                    OctreeElement::AppendState& appendState) const;

    virtual int readEntitySubclassDataFromBuffer(const unsigned char* data, int bytesLeftToRead,
                                                 ReadBitstreamToTreeParams& args,
                                                 EntityPropertyFlags& propertyFlags, bool overwriteLocalData,
                                                 bool& somethingChanged);

    virtual void update(const quint64& now);
    virtual bool needsToCallUpdate() const;

    const rgbColor& getColor() const { return _color; }
    xColor getXColor() const { xColor color = { _color[RED_INDEX], _color[GREEN_INDEX], _color[BLUE_INDEX] }; return color; }
    glm::vec3 getColorRGB() const { return  ColorUtils::sRGBToLinearVec3(toGlm(getXColor())); }

    static const xColor DEFAULT_COLOR;
    void setColor(const rgbColor& value) { memcpy(_color, value, sizeof(_color)); }
    virtual void setColor(const xColor& value) {
        _color[RED_INDEX] = value.red;
        _color[GREEN_INDEX] = value.green;
        _color[BLUE_INDEX] = value.blue;
    }

    static const float DEFAULT_ALPHA;
    virtual void setAlpha(float alpha) { _alpha = alpha; }
    float getAlpha() const { return _alpha; }

    virtual void debugDump() const;

    static const quint32 MAX_DIM;
    static const quint32 MAXIMUM_MAX_PARTICLES;
    static const quint32 DEFAULT_MAX_PARTICLES;
    virtual void setMaxParticles(quint32 maxParticles);
    quint32 getMaxParticles() const { return _maxParticles; }

    static const float DEFAULT_PARTICLE_RADIUS;
    virtual void setParticleRadius(float particleRadius) { _particleRadius = particleRadius; }
    float getParticleRadius() const { return _particleRadius; }

    float getSimulationTime() const { return _simulationTime; }
    void setSimulationTime(float simulationTime) { _simulationTime = simulationTime; }

    float getDeltaTime() const { return _deltaTime; }
    void setDeltaTime(float deltaTime) { _deltaTime = deltaTime; }

    void computeAndUpdateDimensions();

    static const QString DEFAULT_TEXTURES;
    const QString& getTextures() const { return _textures; }
    virtual void setTextures(const QString& textures) {
        if (_textures != textures) {
            _textures = textures;
            _texturesChangedFlag = true;
        } else {
            _texturesChangedFlag = false;
        }
    }

    virtual bool supportsDetailedRayIntersection() const { return false; }

protected:
    void stepSimulation(float deltaTime);

    // Particles properties
    rgbColor _color;
    float _alpha { DEFAULT_ALPHA };
    float _particleRadius { DEFAULT_PARTICLE_RADIUS };

    // Emiter properties
    quint32 _maxParticles { DEFAULT_MAX_PARTICLES };

    QString _textures { DEFAULT_TEXTURES };
    bool _texturesChangedFlag { false };

    float _simulationTime { 0.0f };
    float _deltaTime { 0.0f };

};

#endif // hifi_ProceduralParticlesEntityItem_h
