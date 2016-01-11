// TODO: Add an enabled property (or possibly this with 'visible') and make it
//       available to components.

var scriptURL = Script.resolvePath("componentsClient.js");

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
var HIFI_PUBLIC_BUCKET = "http://s3.amazonaws.com/hifi-public/";
var AUDIO_GUN_SHOT_URL = HIFI_PUBLIC_BUCKET + "sounds/Guns/GUN-SHOT2.raw";
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
        this.fireSound = SoundCache.getSound(AUDIO_GUN_SHOT_URL);
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

function createComponentType(name, client, server) {
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

createComponentType('flickeringLight', {}, {
    init: function() {
        this.time = 0;
        this.elapsed = 0;
        // this.entityManager.on('update', this.onUpdate.bind(this));
        Script.update.connect(this.onUpdate.bind(this));
        // this.intervalID = Script.setInterval(this.onUpdate.bind(this), 100);
    },
    onUpdate: function(dt) {
        this.time += dt;
        this.elapsed += dt;
        if (this.elapsed > 0.1) {
            this.elapsed -= 0.1;
            Entities.editEntity(this.entityManager.entityID, {
                intensity: 5.0 + (Math.sin(this.time) * 0.2) + randFloat(-3.0, 3.0)
            });
        }
    },
    destroy: function() {
        Script.clearInterval(this.intervalID);
    }
});

Script.include('componentsCreate.js');

// Return a random float in the range [min, max)
function randFloat(min, max) {
    return min + Math.random() * (max - min);
}

// Return a random integer in the range [min, max]
function randInt(min, max) {
    return Math.floor(randFloat(min, max + 1));
}

function randColor() {
    return {
        red: randInt(0, 255),
        green: randInt(0, 255),
        blue: randInt(0, 255)
    };
}

ObjectTypes = {
    ball: function() {
        return {
            type: "Sphere",
            color: randColor(),
            collisionsWillMove: true,
            gravity: { x: 0, y: -9.8, z: 0 },
            velocity: { x: 0, y: -0.01, z: 0 }
        };
    },
    box: function() {
        return {
            type: "Box",
            color: randColor(),
            collisionsWillMove: true,
            gravity: { x: 0, y: -9.8, z: 0 },
            velocity: { x: 0, y: -0.01, z: 0 }
        };
    },
    raptor: {
        type: "Model",
        modelURL: "https://hifi-public.s3.amazonaws.com/huffman/raptor.fbx",
        dimensions: { x: 0.65, y: 0.29, z: 0.10 },
        collisionsWillMove: true,
        shapeType: 'box',
        gravity: { x: 0, y: -9.8, z: 0 },
        velocity: { x: 0, y: -0.01, z: 0 }
    },
    horse: {
        type: "Model",
        modelURL: "https://hifi-public.s3.amazonaws.com/huffman/horse.fbx",
        dimensions: { x: 0.14, y: 0.36, z: 0.54 },
        collisionsWillMove: true,
        shapeType: 'box',
        gravity: { x: 0, y: -9.8, z: 0 },
        velocity: { x: 0, y: -0.01, z: 0 }
    },
    heart: {
        type: "Model",
        modelURL: "https://hifi-public.s3.amazonaws.com/huffman/heart.fbx",
        dimensions: { x: 0.09, y: 0.11, z: 0.05 },
        collisionsWillMove: true,
        shapeType: 'sphere',
        gravity: { x: 0, y: -9.8, z: 0 },
        velocity: { x: 0, y: -0.01, z: 0 }
    },
    torch: {
        name: "torch",
        type: "Box",
        collisionsWillMove: true,
        components: {
            audio: {
                url: 'http://hifi-public.s3.amazonaws.com/ryan/demo/0619_Fireplace__Tree_B.L.wav',
                volume: 0.25,
                loop: true,
                follow: true
            }
        },
        children: [
            {
                name: "torch.fire",
                type: "ParticleEffect",
                localPosition: {
                    x: 0,
                    y: 0.1,
                    z: 0
                },
                emitterShouldTrail: true
            },
            {
                "color":{"red":255,"green":255,"blue":255},"maxParticles":1000,"lifespan":5,"emitRate":2,"emitSpeed":1,"speedSpread":0,"emitOrientation":{"x":-0.7071068286895752,"y":0,"z":0,"w":0.7071068286895752},"emitDimensions":{"x":0,"y":0,"z":0},"emitRadiusStart":1,"polarStart":0,"polarFinish":0,"azimuthStart":-3.1415927410125732,"azimuthFinish":3.1415927410125732,"emitAcceleration":{"x":0,"y":-0.30000001192092896,"z":0},"accelerationSpread":{"x":0,"y":0,"z":0},"particleRadius":0.03999999910593033,"radiusSpread":0,"radiusStart":0.03999999910593033,"radiusFinish":0.03999999910593033,"colorSpread":{"red":0,"green":0,"blue":0},"colorStart":{"red":255,"green":255,"blue":255},"colorFinish":{"red":255,"green":255,"blue":255},"alpha":1,"alphaSpread":0,"alphaStart":1,"alphaFinish":1,"emitterShouldTrail":0,"textures":"https://hifi-public.s3.amazonaws.com/alan/Particles/Particle-Sprite-Smoke-1.png","queryAACube":{"x":-9.412508010864258,"y":-10.814034461975098,"z":-9.105239868164062,"scale":19.052560806274414},
                name: "torch.smoke",
                type: "ParticleEffect",
                localPosition: {
                    x: 0,
                    y: 0.15,
                    z: 0
                },
                emitterShouldTrail: true
            },
            {
                name: 'torch.light',
                type: 'Light',
                dimensions: { x: 3, y: 3, z: 3 },
                color: { red: 207, green: 150, blue: 67 },
                components: {
                    flickeringLight: {}
                }
            }
        ]
    },
    pistol: {
        type: 'Model',
        modelURL: "https://s3.amazonaws.com/hifi-public/eric/models/gun.fbx",
        dimensions: {
            x: 0.05,
            y: .23,
            z: .36
        },
        script: scriptURL,
        color: {
            red: 200,
            green: 0,
            blue: 20
        },
        shapeType: 'box',
        collisionsWillMove: true,
        gravity: {x: 0, y: -5.0, z: 0},
        restitution: 0,
        collisionSoundURL: "https://s3.amazonaws.com/hifi-public/sounds/Guns/Gun_Drop_and_Metalli_1.wav",
        userData: {
            grabbableKey: {
                spatialKey: {
                    relativePosition: {
                        x: 0,
                        y: 0.05,
                        z: -0.08
                    },
                    relativeRotation: Quat.fromPitchYawRollDegrees(90, 90, 0)
                },
                invertSolidWhileHeld: true
            }
        }
    }
};
var ObjectTypesKeys = Object.keys(ObjectTypes);
createComponentType('objectCreator', {}, {
    init: function() {
        this.entityManager.on('create', this.onCreate.bind(this));
        this.position = Entities.getEntityProperties(this.entityManager.entityID, ['position']).position;
    },
    onCreate: function(type) {
        console.log('Creating', type);
        var data = null;
        if (type == 'random') {
            var key = ObjectTypesKeys[randInt(0, ObjectTypesKeys.length - 1)];
            data = ObjectTypes[key];
        } else {
            if (!ObjectTypes.hasOwnProperty(type)) {
                console.warn("ObjectCreator: Cannot find object type " + type)
                return;
            }
            data = ObjectTypes[type];
        }
        var p = Entities.getEntityProperties(this.entityManager.entityID, ['name', 'position']);
        var props = typeof(data) == 'function' ? data() : data;
        props.position = this.position;
        props.lifetime = 30;
        console.log("Creating object", this.entityManager.entityID, p.name, JSON.stringify(p));

        createObject(props);
    }
});

createComponentType('positionPrinter', {}, {
    init: function() {
        Script.setInterval(function() {
            var props = Entities.getEntityProperties(this.entityManager.entityID);//, ['name', 'position', 'dimensions']);
            console.log('new Props:', JSON.stringify([props.name, props.position, props.dimensions]));
        }.bind(this), 1000);
    }
});

createComponentType('triggerable', {
    init: function() {
        this.equipped = false;
        this.equippedHand = null;
    },
    startEquip: function(id, params) {
        this.equipped = true;
        this.equippedHand = JSON.parse(params[0]);
    },
    unequip: function() {
        this.equipped = false;
        this.equippedHand = null;
    }
}, {
});

createComponentType('gun', {
}, {
});

createComponentType('explosive', {
}, {
    init: function() {
        this.timeoutID = Script.setTimeout(this.detonate.bind(this), 2000);
        this.fireSound = SoundCache.getSound(AUDIO_GUN_SHOT_URL);
    }, detonate: function() {
        var properties = Entities.getEntityProperties(this.entityManager.entityID, ['position']);
        Entities.deleteEntity(this.entityManager.entityID);
        Audio.playSound(this.fireSound, {
            volume: 0.9,
            position: properties.position
        });
        // Entities.addEntity({
        //     type: "ParticleEffect",
        //     position: properties.position
        // });
    }
});

createComponentType('bird', {
}, {
    init: function() {
        this.entityManager.on('update', this.onUpdate.bind(this));
    },
    onUpdate: function(dt) {
    }
});


// Components to create:
//   ephemeral


registerComponent("button", ButtonComponent, ButtonServerComponent);
registerComponent("toggle", ToggleComponent, ToggleComponent);
registerComponent("eventProxy", EventProxyComponent, EventProxyComponent);
registerComponent("audio", null, AudioServerComponent);


// Script.include('componentsTarget.js');
