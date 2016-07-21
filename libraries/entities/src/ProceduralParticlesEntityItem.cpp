//
//  ProceduralParticlesEntityItem.cpp
//  libraries/entities/src
//
//  Created by Sam Gondelman on 7/13/16.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


#include <glm/gtx/transform.hpp>
#include <QtCore/QJsonDocument>

#include <ByteCountCoding.h>
#include <GeometryUtil.h>
#include <Interpolate.h>

#include "EntityTree.h"
#include "EntityTreeElement.h"
#include "EntitiesLogging.h"
#include "EntityScriptingInterface.h"
#include "ProceduralParticlesEntityItem.h"

const xColor ProceduralParticlesEntityItem::DEFAULT_COLOR = { 255, 255, 255 };
const float ProceduralParticlesEntityItem::DEFAULT_ALPHA = 1.0f;
const quint32 ProceduralParticlesEntityItem::MAX_DIM = 1024;                                    // Limit particles so they fit in a 1024 * 1024 texture
const quint32 ProceduralParticlesEntityItem::MAXIMUM_MAX_PARTICLES = MAX_DIM * MAX_DIM / 2;     // 1024 * 1024 / 2 = 524288
const quint32 ProceduralParticlesEntityItem::DEFAULT_MAX_PARTICLES = 1000;
const float ProceduralParticlesEntityItem::DEFAULT_PARTICLE_RADIUS = 0.025f;
const QString ProceduralParticlesEntityItem::DEFAULT_TEXTURES = "";

EntityItemPointer ProceduralParticlesEntityItem::factory(const EntityItemID& entityID, const EntityItemProperties& properties) {
    EntityItemPointer entity { new ProceduralParticlesEntityItem(entityID) };
    entity->setProperties(properties);
    return entity;
}

// our non-pure virtual subclass for now...
ProceduralParticlesEntityItem::ProceduralParticlesEntityItem(const EntityItemID& entityItemID) :
    EntityItem(entityItemID)
{
    _type = EntityTypes::ProceduralParticles;
    setColor(DEFAULT_COLOR);
}

EntityItemProperties ProceduralParticlesEntityItem::getProperties(EntityPropertyFlags desiredProperties) const {
    EntityItemProperties properties = EntityItem::getProperties(desiredProperties); // get the properties from our base class

    COPY_ENTITY_PROPERTY_TO_PROPERTIES(color, getXColor);
    COPY_ENTITY_PROPERTY_TO_PROPERTIES(maxParticles, getMaxParticles);
    COPY_ENTITY_PROPERTY_TO_PROPERTIES(particleRadius, getParticleRadius);
    COPY_ENTITY_PROPERTY_TO_PROPERTIES(textures, getTextures);
    COPY_ENTITY_PROPERTY_TO_PROPERTIES(alpha, getAlpha);

    return properties;
}

bool ProceduralParticlesEntityItem::setProperties(const EntityItemProperties& properties) {
    bool somethingChanged = EntityItem::setProperties(properties); // set the properties in our base class

    SET_ENTITY_PROPERTY_FROM_PROPERTIES(color, setColor);
    SET_ENTITY_PROPERTY_FROM_PROPERTIES(maxParticles, setMaxParticles);
    SET_ENTITY_PROPERTY_FROM_PROPERTIES(particleRadius, setParticleRadius);
    SET_ENTITY_PROPERTY_FROM_PROPERTIES(textures, setTextures);
    SET_ENTITY_PROPERTY_FROM_PROPERTIES(alpha, setAlpha);

    if (somethingChanged) {
        bool wantDebug = false;
        if (wantDebug) {
            uint64_t now = usecTimestampNow();
            int elapsed = now - getLastEdited();
            qCDebug(entities) << "ProceduralParticlesEntityItem::setProperties() AFTER update... edited AGO=" << elapsed <<
                "now=" << now << " getLastEdited()=" << getLastEdited();
        }
        setLastEdited(properties.getLastEdited());
    }
    return somethingChanged;
}

int ProceduralParticlesEntityItem::readEntitySubclassDataFromBuffer(const unsigned char* data, int bytesLeftToRead,
                                                                    ReadBitstreamToTreeParams& args,
                                                                    EntityPropertyFlags& propertyFlags, bool overwriteLocalData,
                                                                    bool& somethingChanged) {

    int bytesRead = 0;
    const unsigned char* dataAt = data;

    READ_ENTITY_PROPERTY(PROP_COLOR, rgbColor, setColor);
    READ_ENTITY_PROPERTY(PROP_MAX_PARTICLES, quint32, setMaxParticles);
    READ_ENTITY_PROPERTY(PROP_PARTICLE_RADIUS, float, setParticleRadius);
    READ_ENTITY_PROPERTY(PROP_TEXTURES, QString, setTextures);
    READ_ENTITY_PROPERTY(PROP_ALPHA, float, setAlpha);

    return bytesRead;
}

// TODO: eventually only include properties changed since the params.lastViewFrustumSent time
EntityPropertyFlags ProceduralParticlesEntityItem::getEntityProperties(EncodeBitstreamParams& params) const {
    EntityPropertyFlags requestedProperties = EntityItem::getEntityProperties(params);

    requestedProperties += PROP_COLOR;
    requestedProperties += PROP_MAX_PARTICLES;
    requestedProperties += PROP_PARTICLE_RADIUS;
    requestedProperties += PROP_TEXTURES;
    requestedProperties += PROP_ALPHA;

    return requestedProperties;
}

void ProceduralParticlesEntityItem::appendSubclassData(OctreePacketData* packetData, EncodeBitstreamParams& params,
                                                  EntityTreeElementExtraEncodeData* entityTreeElementExtraEncodeData,
                                                  EntityPropertyFlags& requestedProperties,
                                                  EntityPropertyFlags& propertyFlags,
                                                  EntityPropertyFlags& propertiesDidntFit,
                                                  int& propertyCount,
                                                  OctreeElement::AppendState& appendState) const {

    bool successPropertyFits = true;
    APPEND_ENTITY_PROPERTY(PROP_COLOR, getColor());
    APPEND_ENTITY_PROPERTY(PROP_MAX_PARTICLES, getMaxParticles());
    APPEND_ENTITY_PROPERTY(PROP_PARTICLE_RADIUS, getParticleRadius());
    APPEND_ENTITY_PROPERTY(PROP_TEXTURES, getTextures());
    APPEND_ENTITY_PROPERTY(PROP_ALPHA, getAlpha());
}

bool ProceduralParticlesEntityItem::needsToCallUpdate() const {
    return true;
}

void ProceduralParticlesEntityItem::update(const quint64& now) {

    // we check for 'now' in the past in case users set their clock backward
    if (now < _lastSimulated) {
        _lastSimulated = now;
        return;
    }

    float deltaTime = (float)(now - _lastSimulated) / (float)USECS_PER_SECOND;
    _lastSimulated = now;

    stepSimulation(deltaTime);

    EntityItem::update(now); // let our base class handle it's updates...
}

void ProceduralParticlesEntityItem::debugDump() const {
    quint64 now = usecTimestampNow();
    qCDebug(entities) << "PROCEDURAL PARTICLES EntityItem id:" << getEntityItemID() << "---------------------------------------------";
    qCDebug(entities) << "                             color:" << _color[0] << "," << _color[1] << "," << _color[2];
    qCDebug(entities) << "                          position:" << debugTreeVector(getPosition());
    qCDebug(entities) << "                        dimensions:" << debugTreeVector(getDimensions());
    qCDebug(entities) << "                     getLastEdited:" << debugTime(getLastEdited(), now);
}

void ProceduralParticlesEntityItem::stepSimulation(float deltaTime) {
    _simulationTime += deltaTime;
    _deltaTime = deltaTime;
}

void ProceduralParticlesEntityItem::setMaxParticles(quint32 maxParticles) {
    auto prevMaxParticles = _maxParticles;
    maxParticles = std::min(maxParticles, MAXIMUM_MAX_PARTICLES);
    if (maxParticles != prevMaxParticles) {
        _maxParticles = maxParticles;

        // Restart the simulation if the number of particles changes
        setSimulationTime(0.0f);
    }
}
