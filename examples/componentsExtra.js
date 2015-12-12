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
        print("Activated button!");
        Entities.editEntity(this.entityID, {
            color: {
                red: Math.random() * 255,
                green: Math.random() * 255,
                blue: Math.random() * 255,
            }
        });
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
        HIFI_PUBLIC_BUCKET = "http://s3.amazonaws.com/hifi-public/";
        this.fireSound = SoundCache.getSound(HIFI_PUBLIC_BUCKET + "sounds/Guns/GUN-SHOT2.raw");
        this.audioOptions = {
            volume: 0.9,
            position: { x: 0, y: 0, z: 0 }
        };
    },

    onActivated: function() {
        print("Button activated!!");
        Audio.playSound(this.fireSound, this.audioOptions);
        var properties = Entities.getEntityProperties(this.entityID, ['position']);
        Entities.editEntity(this.entityID, {
            position: Vec3.sum(properties.position, {x: 0, y: 0.05, z: 0 }),
        });
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

        print("Deploying a target");

        if (this.numberOfTargetsLeft <= 0) {
            this.running = false;
            Script.clearInterval(this.targetTimer);
        }
    }
});

registerComponent("button", ButtonComponent, ButtonServerComponent);
registerComponent("rangeTarget", RangeTargetComponent, RangeTargetServerComponent);
registerComponent("shootingRange", ShootingRangeComponent, ShootingRangeServerComponent);
