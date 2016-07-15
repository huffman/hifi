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

class RenderableProceduralParticlesEntityItem : public ProceduralParticlesEntityItem  {
    friend class ProceduralParticlePayloadData;
public:
    static EntityItemPointer factory(const EntityItemID& entityID, const EntityItemProperties& properties);
    RenderableProceduralParticlesEntityItem(const EntityItemID& entityItemID);

    virtual void update(const quint64& now) override;

    void updateRenderItem();

    virtual bool addToScene(EntityItemPointer self, render::ScenePointer scene, render::PendingChanges& pendingChanges) override;
    virtual void removeFromScene(EntityItemPointer self, render::ScenePointer scene, render::PendingChanges& pendingChanges) override;

    void setMaxParticles(quint32 maxParticles) override;

protected:
    virtual void locationChanged(bool tellPhysics = true) override { EntityItem::locationChanged(tellPhysics); notifyBoundChanged(); }
    virtual void dimensionsChanged() override { EntityItem::dimensionsChanged(); notifyBoundChanged(); }

    void notifyBoundChanged();

    void createPipelines();

    render::ScenePointer _scene;
    render::ItemID _renderItemId { render::Item::INVALID_ITEM_ID };

    bool _evenPass { true };
    QList<gpu::FramebufferPointer> _particleBuffers;

    NetworkTexturePointer _texture;
    gpu::PipelinePointer _updatePipeline;
    gpu::PipelinePointer _untexturedPipeline;
    gpu::PipelinePointer _texturedPipeline;
};


#endif // hifi_RenderableProceduralParticlesEntityItem_h
