(function() {
    var self = this;

    var leftHeld = false;
    var rightHeld = false;

    self.preload = function(entityID) {
        self.entityID = entityID;
        Script.addEventHandler(entityID, "collisionWithEntity", self.onCollide);
        print('preload');
    };

    self.startDistanceGrab = function(entityID, args) {
        if (args[0] === 'left') {
            leftHeld = true;
        } else {
            rightHeld = true;
        }
        print("startDistance", leftHeld, rightHeld);
    };
    self.startNearGrab = function(entityID, args) {
        if (args[0] === 'left') {
            leftHeld = true;
        } else {
            rightHeld = true;
        }
        print("startNear", leftHeld, rightHeld);
    };
    self.releaseGrab = function(entityID, args) {
        if (args[0] === 'left') {
            leftHeld = false;
        } else {
            rightHeld = false;
        }
        print("release", leftHeld, rightHeld);
    };

    self.onCollide = function(entityA, entityB, collision) {
        var colliderName = Entities.getEntityProperties(entityB, 'name').name;
        print("Collide", colliderName);
        if (colliderName === "Portal/Goal") {
            self.releaseBall();
            Entities.callEntityMethod(entityB, 'onScored');
        }
    };

    self.releaseBall = function() {
        if (leftHeld || rightHeld) {
            var hand;
            if (leftHeld && rightHeld) {
                hand = 'both';
            } else if (!leftHeld && rightHeld) {
                hand = 'right';
            } else if (leftHeld && !rightHeld) {
                hand = 'left';
            }
            Messages.sendMessage("Hifi-Hand-Drop", hand);
        }
    };

    Messages.subscribe("Portal-Game");
    Messages.messageReceived.connect(function(channel, message, sender) {
        if (channel === "Portal-Game") {
            self.releaseBall();
        }
    });
});
