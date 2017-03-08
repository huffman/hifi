//
//  firework.js
//  examples/baseball/
//
//  Created by Ryan Huffman on Nov 9, 2015
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

/* globals randomVec3, randomInt, randomColor, getSounds, playFireworkShow:true */

Script.include("utils.js");

var emitters = [];

var smokeTrailSettings = {
    "name":"ParticlesTest Emitter",
    "type": "ParticleEffect",
    "color":{"red":205,"green":84.41176470588235,"blue":84.41176470588235},
    "maxParticles":1000,
    "velocity": { x: 0, y: 18.0, z: 0 },
    "lifetime": 20,
    "lifespan":3,
    "emitRate":100,
    "emitSpeed":0.5,
    "speedSpread":0,
    "emitOrientation":{"x":0,"y":0,"z":0,"w":1},
    "emitDimensions":{"x":0,"y":0,"z":0},
    "emitRadiusStart":0.5,
    "polarStart":1,
    "polarFinish":1,
    "azimuthStart":0,
    "azimuthFinish":0,
    "emitterShouldTrail": true,
    "emitAcceleration":{"x":0,"y":-0.70000001192092896,"z":0},
    "accelerationSpread":{"x":0,"y":0,"z":0},
    "particleRadius":0.03999999910593033,
    "radiusSpread":0,
    "radiusStart":0.13999999910593033,
    "radiusFinish":0.14,
    "colorSpread":{"red":0,"green":0,"blue":0},
    "colorStart":{"red":255,"green":255,"blue":255},
    "colorFinish":{"red":255,"green":255,"blue":255},
    "alpha":1,
    "alphaSpread":0,
    "alphaStart":1,
    "alphaFinish":0,
    "textures":"https://hifi-public.s3.amazonaws.com/alan/Particles/Particle-Sprite-Smoke-1.png"
};

var fireworkSettings = {
    "name":"ParticlesTest Emitter",
    "type": "ParticleEffect",
    "color":{"red":205,"green":84.41176470588235,"blue":84.41176470588235},
    "maxParticles":1000,
    "lifetime": 20,
    "lifespan":4,
    "emitRate":1000,
    "emitSpeed":1.5,
    "speedSpread":1.0,
    "emitOrientation":{"x":-0.2,"y":0,"z":0,"w":0.7000000000000001},
    "emitDimensions":{"x":0,"y":0,"z":0},
    "emitRadiusStart":0.5,
    "polarStart":1,
    "polarFinish":1.2,
    "azimuthStart":-Math.PI,
    "azimuthFinish":Math.PI,
    "emitAcceleration":{"x":0,"y":-0.70000001192092896,"z":0},
    "accelerationSpread":{"x":0,"y":0,"z":0},
    "particleRadius":0.03999999910593033,
    "radiusSpread":0,
    "radiusStart":0.13999999910593033,
    "radiusFinish":0.14,
    "colorSpread":{"red":0,"green":0,"blue":0},
    "colorStart":{"red":255,"green":255,"blue":255},
    "colorFinish":{"red":255,"green":255,"blue":255},
    "alpha":1,
    "alphaSpread":0,
    "alphaStart":0,
    "alphaFinish":1,
    "textures":"http://hifi-content.s3.amazonaws.com/alan/dev/Particles/Particle-Spark.png"
};

var popSounds = getSounds([
    "http://hifi-public.s3.amazonaws.com/birarda/baseball/fireworks/pop1.wav",
    "http://hifi-public.s3.amazonaws.com/birarda/baseball/fireworks/pop2.wav",
    "http://hifi-public.s3.amazonaws.com/birarda/baseball/fireworks/pop3.wav",
    "http://hifi-public.s3.amazonaws.com/birarda/baseball/fireworks/pop4.wav"
]);

var launchSounds = getSounds([
    "http://hifi-public.s3.amazonaws.com/birarda/baseball/fireworks/fire1.wav",
    "http://hifi-public.s3.amazonaws.com/birarda/baseball/fireworks/fire2.wav",
    "http://hifi-public.s3.amazonaws.com/birarda/baseball/fireworks/fire3.wav",
    "http://hifi-public.s3.amazonaws.com/birarda/baseball/fireworks/fire4.wav"
]);

function playRandomSound(sounds, options) {
    Audio.playSound(sounds[randomInt(sounds.length)], options);
}

function shootFirework(position, color, options) {
    smokeTrailSettings.position = position;
    smokeTrailSettings.velocity = randomVec3(-5, 5, 10, 20, -5, 5);
    smokeTrailSettings.gravity = randomVec3(-5, 5, -9.8, -9.8, -5, 5);

    playRandomSound(launchSounds, { position: position, volume: 3.0 });
    var smokeID = Entities.addEntity(smokeTrailSettings);

    Script.setTimeout(function() {
        Entities.editEntity(smokeID, { emitRate: 0 });
        var position = Entities.getEntityProperties(smokeID, ['position']).position;
        fireworkSettings.position = position;
        fireworkSettings.colorStart = color;
        fireworkSettings.colorFinish = color;
        var burstID = Entities.addEntity(fireworkSettings);
        playRandomSound(popSounds, { position: position, volume: 3.0 });
        Script.setTimeout(function() {
            Entities.editEntity(burstID, { emitRate: 0 });
        }, 500);
        Script.setTimeout(function() {
            Entities.deleteEntity(smokeID);
            Entities.deleteEntity(burstID);
        }, 10000);
    }, 2000);
}

playFireworkShow = function(position, numberOfFireworks, duration, offsetRange, colorBegin, colorEnd) {
    for (var i = 0; i < numberOfFireworks; i++) {
        var randomOffset = randomVec3(-offsetRange.x/2, offsetRange.x/2,
                                      -offsetRange.y/2, offsetRange.y/2,
                                      -offsetRange.z/2, offsetRange.z/2);
        var randomPosition = Vec3.sum(position, randomOffset);
        Script.setTimeout(function(position) {
            return function() {
                var color = randomColor(colorBegin.red, colorEnd.red, 
                                        colorBegin.green, colorEnd.green, 
                                        colorBegin.blue, colorEnd.blue);
                shootFirework(position, color, fireworkSettings);
            };
        }(randomPosition), Math.random() * duration);
    }
};
playFireworkShow(MyAvatar.position, 10, 1, { x: 0, y: 0, z: 0 }, { red: 255, green: 0, blue: 0 }, { red: 255, green: 0, blue: 0 });
