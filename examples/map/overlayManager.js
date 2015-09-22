OVERLAY_PROPERTIES = [
    'color',
    'text',
    'url',
];

// An enhanced overlay wrapper
Overlay = function(overlayID) {
    var self = this;

    this.id = overlayID;

    this.updateWhenNotVisible = true;//opts.updateWhenNotVisible || false;

    var initialPosition = Overlays.getProperty(overlayID, "position");
    var initialRotation = Overlays.getProperty(overlayID, "rotation");
    var initialVisible = Overlays.getProperty(overlayID, "visible") === true;
    var initialAlpha = Overlays.getProperty(overlayID, "alpha") | 1.0;

    this.localPosition = initialPosition;
    this.worldPosition = initialPosition;

    this.localRotation = initialRotation;
    this.worldRotation = initialRotation;

    this.baseDimensions = Overlays.getProperty(overlayID, "dimensions");
    Vec3.print("basedim", this.baseDimensions);
    this.localScale = 1.0;
    this.worldScale = 1.0;

    this.localVisible = initialVisible;
    this.effectiveVisible = initialVisible;

    this.localAlpha = initialAlpha;
    this.effectiveAlpha = initialAlpha;

    var dirty = true;
    var visibleDirty = false;

    this.parent = null;
    var children = [];

    // Event handlers
    this.onMouseOver = function() { };
    this.onMouseMove = function() { };
    this.onMouseOut = function() { };

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

    Object.defineProperty(this, 'alpha', {
        get: function() { return self.localAlpha ; },
        set: function(newValue) { self.localAlpha = newValue; },
    });

    Object.defineProperty(this, 'children', {
        get: function() { return children; },
    });

    for (var i = 0; i < OVERLAY_PROPERTIES.length; i++) {
        var property = OVERLAY_PROPERTIES[i];
        Object.defineProperty(this, property, {
            get: function(property) {
                return function() { return Overlays.getProperty(self.id, property) }
            }(property),
            set: function(property) {
                return function(newValue) {
                    var properties = {};
                    properties[property] = newValue;
                    Overlays.editOverlay(self.id, properties)
                }
            }(property),
        });
        delete property;
    }

    this.updateProperties = function(properties) {
        Overlays.editOverlay(self.id, properties);
    }

    this.removeChild = function(overlay) {
        var idx = children.indexOf(overlay);
        if (idx >= 0) {
            var removed = children.splice(idx, 1);
            removed[0].parent = null;
        }
    };

    this.addChild = function(overlay) {
        print("Adding child");
        if (overlay.parent != null) {
            overlay.parent.removeChild(overlay);
        }

        overlay.parent = this;
        children.push(overlay);
    }

    this.update = function() {
        if (self.effectiveVisible || visibleDirty || self.updateWhenNotVisible) {
            // Vec3.print("scaled dims", Vec3.multiply(self.baseDimensions, self.worldScale));
            Overlays.editOverlay(self.id, {
                position: self.worldPosition,
                rotation: self.worldRotation,
                dimensions: Vec3.multiply(self.baseDimensions, self.worldScale),
                // alpha: self.effectiveAlpha,
                visible: self.effectiveVisible,
                // visible: true,
            });
            dirty = false; 
        }
    }

    this.sync = function() {
        self.customSync();

        if (self.parent) {
            self.worldRotation = Quat.multiply(self.parent.worldRotation, self.localRotation);

            self.worldScale = self.localScale * self.parent.worldScale;

            var localOffset = Vec3.multiplyQbyV(self.worldRotation, self.localPosition);
            // self.worldPosition = Vec3.sum(self.parent.worldPosition, localOffset);
            self.worldPosition = Vec3.sum(self.parent.worldPosition, localOffset);
            self.worldPosition = Vec3.multiply(self.worldScale, self.worldPosition);

            self.effectiveAlpha = self.localAlpha * self.parent.effectiveAlpha;
            self.effectiveVisible = self.localVisible && self.parent.effectiveVisible;
        } else {
            self.worldScale = self.localScale;
            self.worldRotation = self.localRotation;
            self.worldPosition = self.localPosition;
            self.effectiveAlpha = self.localAlpha;
            self.effectiveVisible = self.localVisible;
        }

        self.update();

        for (var i = 0; i < children.length; i++) {
            children[i].sync();
        }
    }

    this.destroy = function() {
        Overlays.deleteOverlay(this.id);
    };

    this.customSync = function() { };
};

EmptyOverlay = function() {
};

EmptyOverlay.prototype = new Overlay();
EmptyOverlay.prototype.update = function() { dirty = false; };
EmptyOverlay.prototype.destroy = function() { };

OverlayManager = function() {
    var self = this;

    this.overlays = [];
    this.idToOverlayMap = {};
    this.mouseOnOverlayID = null;

    Controller.mouseMoveEvent.connect(function(event) { self.mouseMoveEvent(event); });
    Script.scriptEnding.connect(function() { self.cleanup(); });
    // Script.update.connect(function(dt) {
    // });
};

OverlayManager.prototype.update = function() {
    for (var i = 0; i < this.overlays.length; i++) {
        if (this.overlays[i].parent == null) {
            this.overlays[i].sync();
        }
    }
}

OverlayManager.prototype.mouseMoveEvent = function(event) {
    var pickRay = Camera.computePickRay(event.x, event.y);
    var result = Overlays.findRayIntersection(pickRay);

    if (result.intersects) {
        var overlayID = result.overlayID;
        if (overlayID in this.idToOverlayMap) {
            if (this.mouseOnOverlayID != overlayID) {
                if (this.mouseOnOverlayID != null) {
                    this.idToOverlayMap[this.mouseOnOverlayID].onMouseOut(event, pickRay);
                }

                this.mouseOnOverlayID = overlayID;
                this.idToOverlayMap[overlayID].onMouseOver(event, pickRay);
            }

            this.idToOverlayMap[overlayID].onMouseMove(event, pickRay);
        } else if (this.mouseOnOverlayID != null) {
            this.idToOverlayMap[this.mouseOnOverlayID].onMouseOut(event, pickRay);
            this.mouseOnOverlayID = null;
        }
    } else if (this.mouseOnOverlayID != null) {
        this.idToOverlayMap[this.mouseOnOverlayID].onMouseOut(event, pickRay);
        this.mouseOnOverlayID = null;
    }
};

OverlayManager.prototype.get = function(id) {
    return this.idToOverlayMap[id];
};

OverlayManager.prototype.addBare = function() {
    var overlay = new EmptyOverlay();

    this.overlays.push(overlay);

    return overlay;
};

var entityToOverlayTypeMap = {
    "Box": "cube",
    "Sphere": "sphere",
};
OverlayManager.prototype.add = function(type, opts) {
    type = entityToOverlayTypeMap[type] || type;
    var id = Overlays.addOverlay(type, opts);
    var overlay = new Overlay(id);

    this.idToOverlayMap[id] = overlay; 
    this.overlays.push(overlay);

    return overlay;
};

OverlayManager.prototype.cleanup = function() {
    for (var id in this.idToOverlayMap) {
        Overlays.deleteOverlay(id);
    }
}
