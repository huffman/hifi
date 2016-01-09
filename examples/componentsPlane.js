createComponent('planeGameManager', {
    init: function() {
    }
}, {
    init: function() {
        this.running = false;
    },
    start: function() {
        if (!this.running) {
            this.running = true;
            // ...
        }
    }
});

createComponent('planeGameBaton', {
    init: function() {
        this.on('triggerPress', this.onTriggerPress);
        this.on('triggerRelease', this.onTriggerRelease);
        this.on('update', this.onUpdate);

        this.guidingPlane = false;

        // state variables when guiding the plane
        this.planeID = null;
        this.lastMovePosition = null;
    },
    onTriggerPress: function() {
        // Get point at end of baton

        // Search for nearby planes

        // If plane found, begin sending position updates to server
        this.guidingPlane = true;
    },
    onTriggerRelease: function() {
        this.guidingPlane = false;
    },
    onUpdate: function() {
        if (this.guidingPlane) {
            // get trigger position
            // send message to plane
        }
    }
}, {
    init: function() {
    }
});

createComponent('plane', {
    init: function() {
    },
    onUpdate: function() {
    }
}, {
    init: function() {
        this.points = [];
        this.trailLine = null;
    },
    onUpdate: function() {
    },
    onAppendPoint: function(reset, point) {
        if (reset) {
            var position = this.entityManager.getProperties(['position']);
            this.points = [position, point];
        } else {
            this.points.push(point);
        }
    }
});

