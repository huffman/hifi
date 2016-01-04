// TODO: Add an enabled property (or possibly this with 'visible') and make it
//       available to components.

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

function getArg(args, key, defaultValue) {
    if (args.hasOwnProperty(key)) {
        return args[key];
    }
    return defaultValue;
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
        this.async('buttonActivated');
    }
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
        //var audioURL = "atp:cc6df0de217f6c9350098ef3f3a6bce65596ca5611b21f2c253325619c70d21a.wav";
        this.fireSound = SoundCache.getSound(audioURL);
        this.audioOptions = {
            volume: 0.9,
            position: { x: 0, y: 0, z: 0 }
        };
    },

    onActivated: function() {
        console.log("Button activated!!");
        Audio.playSound(this.fireSound, this.audioOptions);
    }
});



/******************************************************************************
 *** ToggleButton Component
 ******************************************************************************/
ToggleButtonComponent = function(entityManager, properties) {
    Component.call(this, entityManager);

    this.triggerOnServer = properties.triggerOnServer;
};
ToggleButtonComponent.prototype = Object.create(Component.prototype);
extend(ToggleButtonComponent.prototype, {
    type: "toggleButton",

    init: function() {
        this.entityManager.on('mouseDown', this.onActivated.bind(this));
        this.entityManager.on('startDistanceGrab', this.onActivated.bind(this));
    },

    // Event handlers
    onActivated: function(event) {
        console.log("Activated button!");
        this.async('buttonActivated');
    }
});
ToggleButtonServerComponent = function(entityManager, properties) {
    Component.call(this, entityManager);

    this.triggerOnServer = properties.triggerOnServer;

    this.entityManager.on('buttonActivated', this.onActivated.bind(this));
};
ToggleButtonServerComponent.prototype = Object.create(Component.prototype);
extend(ToggleButtonServerComponent.prototype, {
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
        Audio.playSound(this.fireSound, this.audioOptions);
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
            var args = getArg(route, 'args', []);

            if (typeof to == "string" || to instanceof String) {
                // Search for entity by name
                to = { name: to };
            }

            console.log("Routing " + event + " to " + JSON.stringify(to) + " with " + JSON.stringify(args));
            var entityIDs = findEntities(to);
            for (var i in entityIDs) {
                this.entityManager.sendEvent(entityIDs[i], method, args);
            }
            if (entityIDs.length == 0) {
                console.log("Routing failed, could not find entities: ", JSON.stringify(to));
            }
        }
    }
});



/******************************************************************************
 *** Audio Component
 ******************************************************************************/
AudioComponent = function(entityManager, properties) {
    Component.call(this, entityManager);
};
AudioComponent.prototype = Object.create(Component.prototype);
extend(AudioComponent.prototype, {
    type: "audio",

    init: function() {
    },
});
AudioServerComponent = function(entityManager, properties) {
    Component.call(this, entityManager);

    this.url = getArg(properties, 'url', '');
    this.volume = getArg(properties, 'volume', 1.0);
    this.loop = getArg(properties, 'loop', true);

    console.warn("In audio component");
};
AudioServerComponent.prototype = Object.create(Component.prototype);
extend(AudioServerComponent.prototype, {
    type: "audio",

    init: function() {
        console.warn("Audio component init", this.url);
        var properties = Entities.getEntityProperties(['position']);
        console.warn("Getting url", this.url);
        this.sound = SoundCache.getSound(this.url);
        this.timer = Script.setInterval(function() {
            console.log("READY??");
            if (this.sound.downloaded) {
                console.log("READY");
                Script.clearInterval(this.timer);
                this.injector = Audio.playSound(this.sound, {
                    volume: this.volume,
                    position: properties.position,
                    loop: true
                });
            }
        }.bind(this), 500);
    },

    destroy: function() {
        this.injector.stop();
    }
});

function createComponent(name, client, server) {
    var ClientComponent = function(entityManager, properties) {
        Component.call(this, entityManager);
        if (client.ctor) {
            client.ctor();
        }
    }
    ClientComponent.prototype = Object.create(Component.prototype);
    ClientComponent.prototype.type = name;
    extend(ClientComponent.prototype, client);

    var ServerComponent = function(entityManager, properties) {
        Component.call(this, entityManager);
        if (server.ctor) {
            server.ctor();
        }
    };
    ServerComponent.prototype = Object.create(Component.prototype);
    ServerComponent.prototype.type = name;
    extend(ServerComponent.prototype, server);

    registerComponent(name, ClientComponent, ServerComponent);
};

createComponent('flickeringLight', {}, {
    init: function() {
        this.time = 0;
        this.elapsed = 0;
        this.entityManager.on('update', this.onUpdate.bind(this));
    },
    onUpdate: function(dt) {
        this.time += dt;
        this.elapsed += dt;
        if (this.elapsed > 0.1) {
            this.elapsed -= 0.1;
            Entities.editEntity(this.entityManager.entityID, {
                intensity: 1.0 + (Math.sin(this.time) * 0.2) + (0.3 + Math.random() * 0.6)
            });
        }
    }
});

Script.include('componentsCreate.js');
ObjectTypes = {
    ball: {
        type: "Sphere",
        color: { red: 128, green: 255, blue: 192 },
        collisionsWillMove: true,
        gravity: { x: 0, y: -9.8, z: 0 },
        velocity: { x: 0, y: 0.1, z: 0 }
    }
};
createComponent('objectCreator', {}, {
    init: function() {
        this.entityManager.on('create', this.onCreate.bind(this));
        this.position = Entities.getEntityProperties(this.entityManager.entityID, ['position']).position;
    },
    onCreate: function(type) {
        if (!ObjectTypes.hasOwnProperty(type)) {
            console.warn("ObjectCreator: Cannot find object type " + type)
            return;
        }

        console.log('Creating', type);
        var data = ObjectTypes[type];
        data.position = this.position;

        createObject(data);
    }
});


// Components to create:
//   ephemeral


registerComponent("button", ButtonComponent, ButtonServerComponent);
registerComponent("rangeTarget", RangeTargetComponent, RangeTargetServerComponent);
registerComponent("shootingRange", ShootingRangeComponent, ShootingRangeServerComponent);
registerComponent("toggle", ToggleComponent, ToggleComponent);
registerComponent("eventProxy", EventProxyComponent, EventProxyComponent);
registerComponent("audio", null, AudioServerComponent);
