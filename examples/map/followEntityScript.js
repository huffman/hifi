(function() {
    entityID = null;
    lastPosition = null;
    porting = false;

    function vec3Equals(a, b) {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }

    var timeSinceLastChecked = 0;
    function update(dt) {
        timeSinceLastChecked += dt;
        if (timeSinceLastChecked > 1.0 && !porting) {
            // print("Checking for change in position");
            var properties = Entities.getEntityProperties(entityID);
            if (!vec3Equals(lastPosition, properties.position)) {
                print("Position changed");

                var dPos = Vec3.subtract(properties.position, lastPosition);
                lastPosition = properties.position;

                MyAvatar.position = Vec3.sum(MyAvatar.position, dPos);
            }
            timeSinceLastChecked = 0;
        }
    }

    this.preload = function(thisEntityID) {
        print("Preloading follow script");
        entityID = thisEntityID;
        var properties = Entities.getEntityProperties(entityID);
        lastPosition = properties.position;
        Script.update.connect(update);
    };
    this.unload = function(entityID) {
        print("Unloading follow script");
        Script.update.disconnect(update);
    };
});
