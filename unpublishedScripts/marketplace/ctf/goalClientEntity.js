/* globals playFireworkShow */

(function() {
    Script.include("firework.js?" + Date.now());

    var self = this;
    var teleportSound = SoundCache.getSound(Script.resolvePath("sounds/teleport.raw"));
    var inCooldown = false;

    self.preload = function(entityID) {
        self.entityID = entityID;

        var userData = Entities.getEntityProperties(entityID, 'userData').userData;
        try {
            userData = JSON.parse(userData);
            if (userData.beginColor !== undefined && userData.endColor !== undefined) {
                self.beginColor = userData.beginColor;
                self.endColor = userData.endColor;
                print("bgin:", self.beginColor.red, self.beginColor.green, self.beginColor.blue);
            } else {
                print("ERROR, colors not found");
            }
        } catch (e) {
            print("ERROR, could not find gun color");
        }
    };

    self.onScored = function() {
        if (inCooldown) {
            return;
        }

        var position = Entities.getEntityProperties(self.entityID, 'position').position;
        playFireworkShow(position, 20, 3000,
                { x: 5, y: 2, z: 5 },
                self.beginColor,
                self.endColor);
        Audio.playSound(teleportSound, {
            position: position,
            volume: 0.40
        });
        inCooldown = true;
        Script.setTimeout(function() {
            inCooldown = false;
        }, 2000);
    };
});


// http://hifi-content.s3.amazonaws.com/caitlyn/production/soundEmitter/soundLoopEmitter.js
