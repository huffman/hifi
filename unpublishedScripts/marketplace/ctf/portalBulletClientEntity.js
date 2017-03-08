(function() {
    this.preload = function(entityID) {
        print("Loading portal bullet script");

        this.teleportSound = SoundCache.getSound(Script.resolvePath("sounds/teleport.raw"));
        this.portalTargetLocation = undefined;

        var userData = Entities.getEntityProperties(entityID, 'userData').userData;
        try {
            this.portalTargetLocation = JSON.parse(userData).portalTargetLocation;
        } catch (e) {
            this.portalTargetLocation = undefined;
        }

        if (this.portalTargetLocation === undefined) {
            print("ERROR, could not find portalTargetLocation in userData");
        } else {
            print("Portal target location is: ", this.portalTargetLocation);
        }
    };

    this.enterEntity = function(entityID) {
        print("Entered bullet entity, the destination is " + this.portalTargetLocation);

        if (this.teleportSound.downloaded) {
            var position = Entities.getEntityProperties(this.entityID, 'position').position;
            Audio.playSound(this.teleportSound, {
                position: position,
                volume: 0.40,
                localOnly: true
            });
        }

        if (this.portalTargetLocation !== undefined) {
            print("Teleporting to " + this.portalTargetLocation);
            Window.location = this.portalTargetLocation;
        }

        // Notify any instances of the ball that we have been hit ensuring that
        // the ball will be dropped if we are holding it.
        Messages.sendLocalMessage("Portal-Game", "hit");
    };

    this.unload = function() {
        print("unloading teleport script");
    };
});
