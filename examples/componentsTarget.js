/******************************************************************************
 *** ShootingRange Component
 ******************************************************************************/
// Shooting range target
createComponentType('shootingRange', {
    init: function() {
        var running = false;
    },
    start: function() {
    }
}, {
    init: function() {
        this.targetTimer = null;
        this.running = false;
        this.numberOfTargetsLeft = 0;
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
})



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


registerComponent("rangeTarget", RangeTargetComponent, RangeTargetServerComponent);
registerComponent("shootingRange", ShootingRangeComponent, ShootingRangeServerComponent);
