//
//  RenderableProceduralParticlesEntityItem.h
//  interface/src/entities
//
//  Created by Sam Gondelman on 7/13/16.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_RenderableProceduralParticlesEntityItem_h
#define hifi_RenderableProceduralParticlesEntityItem_h

#include <ProceduralParticlesEntityItem.h>
#include <TextureCache.h>
#include "RenderableEntityItem.h"

class ProceduralParticles;

class RenderableProceduralParticlesEntityItem : public ProceduralParticlesEntityItem  {
    using Pointer = std::shared_ptr<RenderableProceduralParticlesEntityItem>;
    static Pointer baseFactory(const EntityItemID& entityID, const EntityItemProperties& properties);
public:
    static EntityItemPointer factory(const EntityItemID& entityID, const EntityItemProperties& properties);
    RenderableProceduralParticlesEntityItem(const EntityItemID& entityItemID);

    virtual void update(const quint64& now) override;
    void setParticleRadius(float radius) override;
    void setColor(const xColor& color) override;
    void setAlpha(float alpha) override;
    void setMaxParticles(quint32 maxParticles) override;

    void render(RenderArgs* args) override;

    // TODO: allow configurable shaders
//    void setUserData(const QString& value) override;

    SIMPLE_RENDERABLE();

private:
    QSharedPointer<ProceduralParticles> _particles;
};


#endif // hifi_RenderableProceduralParticlesEntityItem_h
