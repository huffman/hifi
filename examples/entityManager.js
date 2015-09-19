
ENTITY_PROPERTIES = [
    'color',
    'text',
    'url',
];

// An enhanced entity wrapper
Entity = function(entityID, opts) {
    var self = this;

    this.id = entityID;

    this.updateWhenNotVisible = true;//opts.updateWhenNotVisible || false;

    var properties = Entities.getEntityProperties(entityID)

    var initialPosition = properties.position;
    var initialRotation = properties.rotation;
    var initialVisible = true;
    // var initialAlpha = 1.0;

    this.localPosition = initialPosition;
    this.worldPosition = initialPosition;

    this.localRotation = initialRotation;
    this.worldRotation = initialRotation;

    this.baseDimensions = properties.dimensions;
    this.localScale = 1.0;
    this.worldScale = 1.0;

    this.localVisible = initialVisible;
    this.effectiveVisible = initialVisible;

    // this.localAlpha = initialAlpha;
    // this.effectiveAlpha = initialAlpha;

    var dirty = true;
    var visibleDirty = false;

    this.parent = null;
    var children = [];

    Object.defineProperty(this, 'position', {
        get: function() { return self.localPosition; },
        set: function(newValue) { self.localPosition = newValue; dirty = true; },
    });

    // Object.defineProperty(this, 'worldPosition', {
    //     get: function() { return self.worldPosition; },
    // });

    Object.defineProperty(this, 'rotation', {
        get: function() { return self.localRotation; },
        set: function(newValue) { self.localRotation = newValue; dirty = true; },
    });

    Object.defineProperty(this, 'scale', {
        get: function() { return self.localScale; },
        set: function(newValue) { self.localScale = newValue; dirty = true; },
    });

    Object.defineProperty(this, 'visible', {
        get: function() { return self.localVisible; },
        set: function(newValue) { visibleDirty = self.localVisible != newValue; self.localVisible = newValue; },
    });

    // Object.defineProperty(this, 'alpha', {
    //     get: function() { return self.localAlpha ; },
    //     set: function(newValue) { self.localAlpha = newValue; },
    // });

    Object.defineProperty(this, 'children', {
        get: function() { return children; },
    });

    // for (var i = 0; i < OVERLAY_PROPERTIES.length; i++) {
    //     var property = OVERLAY_PROPERTIES[i];
    //     Object.defineProperty(this, property, {
    //         get: function(property) {
    //             return function() { return Overlays.getProperty(self.id, property) }
    //         }(property),
    //         set: function(property) {
    //             return function(newValue) {
    //                 var properties = {};
    //                 properties[property] = newValue;
    //                 Overlays.editOverlay(self.id, properties)
    //             }
    //         }(property),
    //     });
    //     delete property;
    // }

    this.updateProperties = function(properties) {
        Entities.editEntity(self.id, properties);
    }

    this.removeChild = function(entity) {
        var idx = children.indexOf(entity);
        if (idx >= 0) {
            var removed = children.splice(idx, 1);
            removed[0].parent = null;
        }

        return this;
    };

    this.addChild = function(entity) {
        print("Adding child");
        if (entity.parent != null) {
            entity.parent.removeChild(entity);
        }

        entity.parent = this;
        children.push(entity);

        return this;
    }

    this.update = function() {
        if (self.effectiveVisible || visibleDirty || self.updateWhenNotVisible) {
            if (visibleDirty) { print('dirty', self.effectiveVisible) }
            Entities.editEntity(self.id, {
                position: self.worldPosition,
                rotation: self.worldRotation,
                dimensions: Vec3.multiply(self.baseDimensions, self.worldScale),
                // alpha: self.effectiveAlpha,
                // visible: true,
                visible: self.effectiveVisible,
            });
            dirty = false;
            visibleDirty = false;
        } else {
        }
    }

    this.sync = function() {
        self.customSync();

        if (self.parent) {
            self.worldRotation = Quat.multiply(self.parent.worldRotation, self.localRotation);

            // self.effectiveAlpha = self.localAlpha * self.parent.effectiveAlpha;
            self.worldScale = self.localScale * self.parent.worldScale;

            var localOffset = Vec3.multiplyQbyV(self.worldRotation, self.localPosition);
            self.worldPosition = Vec3.sum(self.parent.worldPosition, localOffset);
            // self.worldPosition = Vec3.multiply(self.worldScale, self.worldPosition);

            self.effectiveVisible = self.localVisible && self.parent.effectiveVisible;
        } else {
            self.worldScale = self.localScale;
            self.worldRotation = self.localRotation;
            self.worldPosition = self.localPosition;
            // self.effectiveAlpha = self.localAlpha;
            self.effectiveVisible = self.localVisible;
        }

        self.update();

        for (var i = 0; i < children.length; i++) {
            children[i].sync();
        }
    }

    this.destroy = function() {
        Entities.deleteEntity(this.id);
    }

    this.customSync = function() { };
};

EmptyEntity = function() {
};

EmptyEntity.prototype = new Entity();
EmptyEntity.prototype.update = function() { dirty = false; };
EmptyEntity.prototype.destroy = function() { };

EntityManager = function() {
    var self = this;

    this.entities = [];
    this.idToEntityMap = {};

    Script.scriptEnding.connect(function() { self.cleanup(); });
    // Script.update.connect(function(dt) {
    // });
};

EntityManager.prototype.get = function(id) {
    return this.idToEntityMap[id];
}

EntityManager.prototype.update = function(dt) {
    for (var i = 0; i < this.entities.length; i++) {
        if (this.entities[i].parent == null) {
            this.entities[i].sync();
        }
    }
}

EntityManager.prototype.addBare = function() {
    var entity = new EmptyEntity();

    this.entities.push(entity);

    return entity;
};

EntityManager.prototype.add = function(type, opts) {
    opts.type = type;
    var id = Entities.addEntity(opts);
    var entity = new Entity(id);

    this.idToEntityMap[id] = entity; 
    this.entities.push(entity);

    return entity;
};

EntityManager.prototype.cleanup = function() {
    for (var i = 0; i < this.entities.length; ++i) {
        this.entities[i].destroy();
    }
}
