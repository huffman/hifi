Script.include('componentsCreate.js');

SCENE = {
    name: "home",
    entities: [
        {
            name: "island",
            type: "Model",
            position: { x: 0.27, y: -14.29, z: 1.47 },
            dimensions: { x: 50.76, y: 25.32, z: 43.61 },
            modelURL: "https://hifi-public.s3.amazonaws.com/huffman/island.fbx",
        },
        {
            name: "fireplaceSwitch",
            type: "Box",
            position: { x: 0.5, y: 0, z: 0 },
            components: {
                button: {},
                eventProxy: {
                    buttonActivated: {
                        to: 'warpArea',
                        method: 'create',
                        args: ['random']
                    }
                }
            }
        },
        {
            name: "warpArea",
            position: { x: 0, y: 0.5, z: 0 },
            ignoreForCollisions: true,
            components: {
                objectCreator: {}
            }
        },
        {
            name: "fireplace",
            children: [
                {
                    name: 'fireplace.light',
                    type: 'Light',
                    dimensions: { x: 3, y: 3, z: 3 },
                    color: { red: 207, green: 150, blue: 67 },
                    components: {
                        flickeringLight: {}
                    }
                },
            ],
            components: {
                audio: {
                    url: 'http://hifi-public.s3.amazonaws.com/ryan/demo/0619_Fireplace__Tree_B.L.wav',
                    volume: 0.25,
                    loop: true,
                    follow: true
                }
            }
        },
        {
            name: 'grenade-launcher',
            type: 'Model',
            // modelURL: "https://s3.amazonaws.com/hifi-public/eric/models/gun.fbx",
            modelURL: "https://hifi-content.s3.amazonaws.com/ozan/dev/props/guns/grenade_launcher/grenade_launcher.fbx",
            dimensions: {
                x: 0.26,
                y: 0.33,
                z: 0.92
            },
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
                        relativeRotation: Quat.fromPitchYawRollDegrees(0, 0, 0)
                    },
                    invertSolidWhileHeld: true
                }
            },
            components: {
                gun: {
                    projectileBlueprint: 'grenade',
                    fireSoundURL: Script.resolvePath("../gunshow/gatling.wav")
                }
            },
            children: [
                {
                    type: "ParticleEffect",
                    "name": "muzzle-smoke",
                    position: {
                        x: 0.4,
                        y: 0.05,
                        z: 0
                    },
                    "maxParticles": 1000,
                    isEmitting: false,
                    "emitRate": 20,
                    emitSpeed: 0,
                    "speedSpread": 0,
                    "emitDimensions": {
                        "x": 0,
                        "y": 0,
                        "z": 0
                    },
                    "polarStart": 0,
                    "polarFinish": 0,
                    "azimuthStart": -3.1415927410125732,
                    "azimuthFinish": 3.14,
                    "emitAcceleration": {
                        "x": 0,
                        "y": 0.5,
                        "z": 0
                    },
                    "accelerationSpread": {
                        "x": 0.2,
                        "y": 0,
                        "z": 0.2
                    },
                    "radiusSpread": 0.04,
                    "particleRadius": 0.07,
                    "radiusStart": 0.07,
                    "radiusFinish": 0.07,
                    "alpha": 0.7,
                    "alphaSpread": 0,
                    "alphaStart": 0,
                    "alphaFinish": 0,
                    "additiveBlending": 0,
                    "textures": "https://hifi-public.s3.amazonaws.com/alan/Particles/Particle-Sprite-Smoke-1.png"
                },
                {
                    type: "ParticleEffect",
                    position: {
                        x: 0.4,
                        y: 0.05,
                        z: 0
                    },
                    isEmitting: false,
                    "name": "muzzle-flash",
                    parentID: this.entityID,
                    "color": {
                        red: 228,
                        green: 128,
                        blue: 12
                    },
                    "maxParticles": 1000,
                    "lifespan": 0.1,
                    "emitRate": 1000,
                    "emitSpeed": 0.5,
                    "speedSpread": 0,
                    "emitOrientation": {
                        "x": -0.4,
                        "y": 1,
                        "z": -0.2,
                        "w": 0.7071068286895752
                    },
                    "emitDimensions": {
                        "x": 0,
                        "y": 0,
                        "z": 0
                    },
                    "polarStart": 0,
                    "polarFinish": Math.PI,
                    "azimuthStart": -3.1415927410125732,
                    "azimuthFinish": 2,
                    "emitAcceleration": {
                        "x": 0,
                        "y": 0,
                        "z": 0
                    },
                    "accelerationSpread": {
                        "x": 0,
                        "y": 0,
                        "z": 0
                    },
                    "particleRadius": 0.05,
                    "radiusSpread": 0.01,
                    "radiusStart": 0.05,
                    "radiusFinish": 0.05,
                    "colorSpread": {
                        red: 100,
                        green: 100,
                        blue: 20
                    },
                    "alpha": 1,
                    "alphaSpread": 0,
                    "alphaStart": 0,
                    "alphaFinish": 0,
                    "additiveBlending": true,
                    "textures": "http://ericrius1.github.io/PartiArt/assets/star.png"
                }
            ]
        },
        {
            name: 'nail-gun',
            type: 'Model',
            // modelURL: "https://s3.amazonaws.com/hifi-public/eric/models/gun.fbx",
            // modelURL: Script.resolvePath('../gunshow/nail_gun.fbx'),
            modelURL: "https://hifi-content.s3.amazonaws.com/ozan/dev/props/guns/nail_gun/nail_gun.fbx",
            dimensions: {
                x: 0.18,
                y: 0.59,
                z: 0.68
            },
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
                        relativeRotation: Quat.fromPitchYawRollDegrees(0, 0, 0)
                    },
                    invertSolidWhileHeld: true
                }
            },
            components: {
                gun: {
                    projectileBlueprint: 'nailgun.nail',
                    fireSoundURL: Script.resolvePath("../gunshow/gatling.wav")
                }
            },
            children: [
                {
                    type: "ParticleEffect",
                    "name": "muzzle-smoke",
                    position: {
                        x: 0.4,
                        y: 0.05,
                        z: 0
                    },
                    "maxParticles": 1000,
                    isEmitting: false,
                    "emitRate": 20,
                    emitSpeed: 0,
                    "speedSpread": 0,
                    "emitDimensions": {
                        "x": 0,
                        "y": 0,
                        "z": 0
                    },
                    "polarStart": 0,
                    "polarFinish": 0,
                    "azimuthStart": -3.1415927410125732,
                    "azimuthFinish": 3.14,
                    "emitAcceleration": {
                        "x": 0,
                        "y": 0.5,
                        "z": 0
                    },
                    "accelerationSpread": {
                        "x": 0.2,
                        "y": 0,
                        "z": 0.2
                    },
                    "radiusSpread": 0.04,
                    "particleRadius": 0.07,
                    "radiusStart": 0.07,
                    "radiusFinish": 0.07,
                    "alpha": 0.7,
                    "alphaSpread": 0,
                    "alphaStart": 0,
                    "alphaFinish": 0,
                    "additiveBlending": 0,
                    "textures": "https://hifi-public.s3.amazonaws.com/alan/Particles/Particle-Sprite-Smoke-1.png"
                },
                {
                    type: "ParticleEffect",
                    position: {
                        x: 0.4,
                        y: 0.05,
                        z: 0
                    },
                    isEmitting: false,
                    "name": "muzzle-flash",
                    parentID: this.entityID,
                    "color": {
                        red: 228,
                        green: 128,
                        blue: 12
                    },
                    "maxParticles": 1000,
                    "lifespan": 0.1,
                    "emitRate": 1000,
                    "emitSpeed": 0.5,
                    "speedSpread": 0,
                    "emitOrientation": {
                        "x": -0.4,
                        "y": 1,
                        "z": -0.2,
                        "w": 0.7071068286895752
                    },
                    "emitDimensions": {
                        "x": 0,
                        "y": 0,
                        "z": 0
                    },
                    "polarStart": 0,
                    "polarFinish": Math.PI,
                    "azimuthStart": -3.1415927410125732,
                    "azimuthFinish": 2,
                    "emitAcceleration": {
                        "x": 0,
                        "y": 0,
                        "z": 0
                    },
                    "accelerationSpread": {
                        "x": 0,
                        "y": 0,
                        "z": 0
                    },
                    "particleRadius": 0.05,
                    "radiusSpread": 0.01,
                    "radiusStart": 0.05,
                    "radiusFinish": 0.05,
                    "colorSpread": {
                        red: 100,
                        green: 100,
                        blue: 20
                    },
                    "alpha": 1,
                    "alphaSpread": 0,
                    "alphaStart": 0,
                    "alphaFinish": 0,
                    "additiveBlending": true,
                    "textures": "http://ericrius1.github.io/PartiArt/assets/star.png"
                }
            ]
        }
    ]
};

Script.scriptEnding.connect(function() {
    print("Script ending");
    destroyScene(SCENE.name);
});

createScene(SCENE);
