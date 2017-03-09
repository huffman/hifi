//  pingPongGun.js
//
//  Script Type: Entity
//  Created by James B. Pollack @imgntn on 9/21/2015
//  Copyright 2015 High Fidelity, Inc.
//  
//  This script shoots a ping pong ball.
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

(function() {
    var TRIGGER_THRESHOLD = 0.3;

    var BULLET_BLUE_MODEL_URL = "http://hifi-content.s3.amazonaws.com/alan/dev/Glow-ball-blue.fbx";
    var BULLET_RED_MODEL_URL = "http://hifi-content.s3.amazonaws.com/alan/dev/Glow-ball-red.fbx";

    var GUN_LOAD_SOUND_URL = "https://hifi-content.s3.amazonaws.com/DomainContent/Event%20/Sounds/Gun/GunLoad02.wav";
    var GUN_FIRE1_SOUND_URL = "https://hifi-content.s3.amazonaws.com/DomainContent/Event%20/Sounds/Gun/GunFire01.wav";
    var GUN_FIRE2_SOUND_URL = "https://hifi-content.s3.amazonaws.com/DomainContent/Event%20/Sounds/Gun/GunFire02.wav";
    var GUN_FIRE3_SOUND_URL = "https://hifi-content.s3.amazonaws.com/DomainContent/Event%20/Sounds/Gun/GunFire03.wav";

    var GUN_TIP_FWD_OFFSET = -1;
    var GUN_TIP_UP_OFFSET = 0.12;
    var GUN_FORCE = 20;
    var BALL_RESTITUTION = 0.2;
    var BALL_LINEAR_DAMPING = 0.1;
    var BALL_DENSITY = 10000;

    var BALL_GRAVITY = {
        x: 0,
        y: -10,
        z: 0
    };

    var BALL_DIMENSIONS = {
        x: 2.5,
        y: 2.5,
        z: 2.5
    };

    var BALL_COLLIDER_DIMENSIONS = {
        x: 0.94,
        y: 2.0,
        z: 0.94
    };

    var BALL_ANGULAR_VEL = {
        x: 2,
        y: 3,
        z: 1
    };

    var TRIGGER_CONTROLS = [
        Controller.Standard.LT,
        Controller.Standard.RT
    ];

    function PortalGun() {
    }

    PortalGun.prototype = {
        hand: null,
        gunTipPosition: null,
        cooldownActive: false,

        preload: function(entityID) {
            this.entityID = entityID;
            this.gunFireSound = SoundCache.getSound(GUN_FIRE1_SOUND_URL);
            this.gunLoadSound = SoundCache.getSound(GUN_LOAD_SOUND_URL);

            this.hasReleasedInitialTriggerPress = false;

            this.color = 'red';

            var userData = Entities.getEntityProperties(entityID, 'userData').userData;
            try {
                userData = JSON.parse(userData);
                if (userData.color === 'red' || userData.color === 'blue') {
                    this.color = userData.color;
                } else {
                    print("ERROR, invalid color:", userData.color);
                }
            } catch (e) {
                print("ERROR, could not find gun color");
            }
        },
        startEquip: function(entityID, args) {
            this.hand = args[0] === "left" ? 0 : 1;
        },
        continueEquip: function(entityID, args) {
            if (this.cooldownActive) {
                return;
            }

            var triggerValue = Controller.getValue(TRIGGER_CONTROLS[this.hand]);
            if (triggerValue > TRIGGER_THRESHOLD) {
                if (this.hasReleasedInitialTriggerPress) {
                    this.shootBall();
                }
            } else {
                this.hasReleasedInitialTriggerPress = true;
            }
        },
        releaseEquip: function(entityID, args) {
        },
        shootBall: function() {
            if (this.cooldownActive) {
                print("Not allowing portal bullet to be shot while cooldown active");
                return;
            }
            Controller.triggerHapticPulse(10.0, 0.5, this.hand);

            var gunProperties = Entities.getEntityProperties(this.entityID, ["position", "rotation"]);

            var gunTipPosition = this.getGunTipPosition(gunProperties);

            var audioProperties = {
                volume: 0.9,
                position: gunTipPosition,
                localOnly: true
            };

            Audio.playSound(this.gunFireSound, audioProperties);

            var forwardVec = Quat.getFront(Quat.multiply(gunProperties.rotation, Quat.fromPitchYawRollDegrees(0, 180, 0)));
            forwardVec = Vec3.normalize(forwardVec);
            forwardVec = Vec3.multiply(forwardVec, GUN_FORCE);

            var projectileProperties = {
                name: 'CTF/PortalBullet',
                type: 'Model',
                modelURL: this.color === 'blue' ? BULLET_BLUE_MODEL_URL : BULLET_RED_MODEL_URL,
                dimensions: BALL_DIMENSIONS,
                damping: BALL_LINEAR_DAMPING,
                gravity: BALL_GRAVITY,
                restitution: BALL_RESTITUTION,
                density: BALL_DENSITY,
                dynamic: true,
                rotation: gunProperties.rotation,
                position: gunTipPosition,
                angularDamping: 0,
                angularVelocity: BALL_ANGULAR_VEL,
                velocity: forwardVec,
                lifetime: 10,
                collisionless: true,
                visible: true
            };
            var projectileID = Entities.addEntity(projectileProperties);

            var portalProperties = {
                name: 'CTF/PortalCollider',
                type: 'Sphere',
                localPosition: { x: 0, y: 0, z: 0 },
                dimensions: BALL_DIMENSIONS,
                parentID: projectileID,
                dynamic: false,
                lifetime: 10,
                visible: false,
                script: Script.resolvePath("portalBulletClientEntity.js"),
                collisionless: true,
                userData: JSON.stringify({
                    //portalTargetLocation: "hifi://lionsgate.highfidelity.io/-104.376,-0.496867,-8.28224/0,0.721841,0,0.692059"
                    portalTargetLocation: "/16.0555,-0.372284,18.5259/0,0.631228,0,0.775597"
                })
            };
            Entities.addEntity(portalProperties);

            var self = this;
            this.cooldownActive = true;
            Script.setTimeout(function() {
                var gunProperties = Entities.getEntityProperties(self.entityID, ["position", "rotation"]);
                var gunTipPosition = self.getGunTipPosition(gunProperties);
                var audioProperties = {
                    volume: 0.9,
                    position: gunTipPosition,
                    localOnly: true
                };
                Audio.playSound(self.gunLoadSound, audioProperties);
                self.cooldownActive = false;
            }, 1000);
        },
        getGunTipPosition: function(properties) {
            // the tip of the gun is going to be in a different place than the center,
            // so we move in space relative to the model to find that position
            var frontVector = Quat.getFront(properties.rotation);
            var frontOffset = Vec3.multiply(frontVector, GUN_TIP_FWD_OFFSET);
            var upVector = Quat.getUp(properties.rotation);
            var upOffset = Vec3.multiply(upVector, GUN_TIP_UP_OFFSET);

            var gunTipPosition = Vec3.sum(properties.position, frontOffset);
            gunTipPosition = Vec3.sum(gunTipPosition, upOffset);

            return gunTipPosition;
        },
        createTipEntity: function(entityID) {
            // for debugging where its going to shoot from
            var gunProperties = Entities.getEntityProperties(entityID, ["position", "rotation"]);

            var tipProps = {
                name: 'Ping pong tip test',
                dimensions: {
                    x: 0.1,
                    y: 0.1,
                    z: 0.1
                },
                color: {
                    red: 0,
                    green: 255,
                    blue: 0
                },
                type: 'Box',
                parentID: entityID,
                position: this.getGunTipPosition(gunProperties)
            };
            Entities.addEntity(tipProps);
        }
    };

    // entity scripts should return a newly varructed object of our type
    return new PortalGun();
});
