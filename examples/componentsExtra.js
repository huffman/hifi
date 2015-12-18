
/*
 {"components":{"button":{}, "eventProxy": {"buttonActivated": {"to": "light1", "method":"toggle"} } }}

 {"components":{"toggle":{}}}

 http://localhost:8000/componentsClient.js
 */
findEntity = function(properties, searchRadius) {
    var entities = findEntities(properties, searchRadius);
    return entities.length > 0 ? entities[0] : null;
}

// Return all entities with properties `properties` within radius `searchRadius`
findEntities = function(properties, searchRadius) {
    searchRadius = searchRadius ? searchRadius : 100000;
    var entities = Entities.findEntities({ x: 0, y: 0, z: 0 }, searchRadius);
    var matchedEntities = [];
    var keys = Object.keys(properties);
    for (var i = 0; i < entities.length; ++i) {
        var match = true;
        var candidateProperties = Entities.getEntityProperties(entities[i], keys);
        for (var key in properties) {
            if (candidateProperties[key] != properties[key]) {
                // This isn't a match, move to next entity
                match = false;
                break;
            }
        }
        if (match) {
            matchedEntities.push(entities[i]);
        }
    }

    return matchedEntities;
}

/******************************************************************************
 *** Button Component
 ******************************************************************************/
ButtonComponent = function(entityManager, properties) {
    Component.call(this, entityManager);

    this.triggerOnServer = properties.triggerOnServer;
};
ButtonComponent.prototype = Object.create(Component.prototype);
extend(ButtonComponent.prototype, {
    type: "button",

    init: function() {
        this.entityManager.on('mouseDown', this.onActivated.bind(this));
        this.entityManager.on('startDistanceGrab', this.onActivated.bind(this));
    },

    // Event handlers
    onActivated: function(event) {
        console.log("Activated button!");
        // Entities.editEntity(this.entityID, {
        //     color: {
        //         red: Math.random() * 255,
        //         green: Math.random() * 255,
        //         blue: Math.random() * 255,
        //     }
        // });
        this.async('buttonActivated');
    },

    // // Public functionality
    // toggleButton: function() {
    //     async(this.toggleButton_Server);
    // },
    // toggleButton_Server: function() {
    //     this.buttonDown = !this.buttonDown;
    //     emit('buttonClick', this.buttonDown);
    // },
});
ButtonServerComponent = function(entityManager, properties) {
    Component.call(this, entityManager);

    this.triggerOnServer = properties.triggerOnServer;

    this.entityManager.on('buttonActivated', this.onActivated.bind(this));
};
ButtonServerComponent.prototype = Object.create(Component.prototype);
extend(ButtonServerComponent.prototype, {
    type: "button",

    init: function() {
        var HIFI_PUBLIC_BUCKET = "http://s3.amazonaws.com/hifi-public/";
        var audioURL = HIFI_PUBLIC_BUCKET + "sounds/Guns/GUN-SHOT2.raw";
        var audioURL = "atp:cc6df0de217f6c9350098ef3f3a6bce65596ca5611b21f2c253325619c70d21a.wav";
        this.fireSound = SoundCache.getSound(audioURL);
        this.audioOptions = {
            volume: 0.9,
            position: { x: 0, y: 0, z: 0 }
        };
    },

    onActivated: function() {
        console.log("Button activated!!");
        Audio.playSound(this.fireSound, this.audioOptions);
        // var properties = Entities.getEntityProperties(this.entityID, ['position']);
        // Entities.editEntity(this.entityID, {
        //     position: Vec3.sum(properties.position, {x: 0, y: 0.05, z: 0 }),
        // });
    }
});



/******************************************************************************
 *** RangeTarget Component
 ******************************************************************************/
RangeTargetComponent = function(entityManager, properties) {
    Component.call(this, entityManager);
};
RangeTargetComponent.prototype = Object.create(Component.prototype);
extend(RangeTargetComponent.prototype, {
    type: "rangeTarget",

    init: function() {
    },

    // Event handlers
    onShot: function(shooterUUID, shotType) {
    }
});
RangeTargetServerComponent = function(entityManager, properties) {
    Component.call(this, entityManager);
};
RangeTargetServerComponent.prototype = Object.create(Component.prototype);
extend(RangeTargetServerComponent.prototype, {
    type: "rangeTarget",

    init: function() {
    }
});



/******************************************************************************
 *** ShootingRange Component
 ******************************************************************************/
// Shooting range target
ShootingRangeComponent = function(entityManager, properties) {
    Component.call(this, entityManager);

    var running = false;
};
ShootingRangeComponent.prototype = Object.create(Component.prototype);
extend(ShootingRangeComponent.prototype, {
    type: "shootingRange",

    init: function() {
    },

    start: function() {
    }
});

// Shooting range target
ShootingRangeServerComponent = function(entityManager, properties) {
    Component.call(this, entityManager);

    this.targetTimer = null;
    this.running = false;
    this.numberOfTargetsLeft = 0;
};
ShootingRangeServerComponent.prototype = Object.create(Component.prototype);
extend(ShootingRangeServerComponent.prototype, {
    type: "shootingRange",

    init: function() {
    },

    start: function() {
        if (this.running) {
            return false;
        }
        this.running = true;
        this.numberOfTargetsLeft = 10;
        this.targetTimer = Script.setInterval(this.shootTarget.bind(this), 4000);
        return true;
    },
    shootTarget: function() {
        --this.numberOfTargetsLeft;

        console.log("Deploying a target");

        if (this.numberOfTargetsLeft <= 0) {
            this.running = false;
            Script.clearInterval(this.targetTimer);
        }
    }
});


ToggleComponent = function(entityManager, properties) {
    Component.call(this, entityManager);
    console.log("In toggle component");

    this.property = properties.property ? properties.property : 'visible';

    console.log("in toggle component");
    // How do we do slots??
    this.entityManager.on('toggle', this.onToggle.bind(this));
    console.log("Done in toggle component");
};
ToggleComponent.prototype = Object.create(Component.prototype);
extend(ToggleComponent.prototype, {
    type: "toggle",

    init: function() {
    },

    onToggle: function() {
        console.log("onToggle()", this.property, this.entityID);
        var properties = Entities.getEntityProperties(this.entityID, [this.property]);
        console.log(this.property, properties[this.property]);
        properties[this.property] = !properties[this.property];
        Entities.editEntity(this.entityID, properties);
        console.log(this.property, properties[this.property]);
    }
});


EventProxyComponent = function(entityManager, properties) {
    Component.call(this, entityManager);

    console.log("In event proxy");
    this.routes = properties;

    console.log("routes:", JSON.stringify(this.routes));
    for (var event in this.routes) {
        console.log("Routing " + event + " to " + this.route);
        var self = this;
        this.entityManager.on(event, function(event) {
            return function() { console.log(event); self.route(event); }.bind(this);
        }(event));
    }
};
EventProxyComponent.prototype = Object.create(Component.prototype);
extend(EventProxyComponent.prototype, {
    type: "eventProxy",

    init: function() {
    },

    route: function(event) {
        console.log("Got event in router: ", event)
        var route = this.routes[event];
        if (route) {
            var to = route.to;
            var method = route.method;

            if (typeof to == "string" || to instanceof String) {
                // Search for entity by name
                to = { name: to };
            }

            console.log("Routing " + event + " to " + JSON.stringify(to));
            var entityIDs = findEntities(to);
            for (var i in entityIDs) {
                this.entityManager.sendEvent(entityIDs[i], method);
            }
            if (entityIDs.length == 0) {
                console.log("Routing failed, could not find entities: ", JSON.stringify(to));
            }
        }
    }
});

// Components to create:
//   ephemeral


registerComponent("button", ButtonComponent, ButtonServerComponent);
registerComponent("rangeTarget", RangeTargetComponent, RangeTargetServerComponent);
registerComponent("shootingRange", ShootingRangeComponent, ShootingRangeServerComponent);
registerComponent("toggle", ToggleComponent, ToggleComponent);
registerComponent("eventProxy", EventProxyComponent, EventProxyComponent);
