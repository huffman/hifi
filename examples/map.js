Script.include("entityManager.js");
Script.include("overlayManager.js");


// Poll for nearby map data

var entityManager = new EntityManager();

// From http://evanw.github.io/lightgl.js/docs/raytracer.html
function raySphereIntersection(origin, ray, center, radius) {
    var offset = Vec3.subtract(origin, center);
    var a = Vec3.dot(ray, ray);
    // var a = ray.dot(ray);
    var b = 2 * Vec3.dot(ray, offset);
    // var b = 2 * ray.dot(offset);
    var c = Vec3.dot(offset, offset) - radius * radius;
    // var c = offset.dot(offset) - radius * radius;
    var discriminant = b * b - 4 * a * c;

    if (discriminant > 0) {
        return true;
    }

    return null;
};


Map = function(data) {
    var visible = false;

    var ROOT_OFFSET = Vec3.multiply(1, Quat.getFront(MyAvatar.orientation));
    var ROOT_POSITION = Vec3.sum(MyAvatar.position, ROOT_OFFSET);

    var ROOT_SCALE = 0.0005;

    // Create object in objectManager
    var rootObject = entityManager.addBare();
    var position = ROOT_POSITION;
    rootObject.position = ROOT_POSITION;
    rootObject.scale = ROOT_SCALE
    Vec3.print("Position:", position);

    // Search for all nearby objects that have the userData "mapped"
    // TODO Update to use the zone's bounds
    var entities = Entities.findEntities(MyAvatar.position, 32000);
    var mappedEntities = [];
    var minCorner = {
        x: 4294967295,
        y: 4294967295,
        z: 4294967295,
    };
    var maxCorner = {
        x: -4294967295,
        y: -4294967295,
        z: -4294967295,
    };

    for (var i = 0; i < entities.length; ++i) {
        var entityID = entities[i];
        var properties = Entities.getEntityProperties(entityID);
        if (properties.userData == "mapped" || properties.userData == "tracked") {

            print("Found: ", properties.name);

            minCorner.x = Math.min(minCorner.x, properties.position.x - (properties.dimensions.x / 2));
            minCorner.y = Math.min(minCorner.y, properties.position.y - (properties.dimensions.y / 2));
            minCorner.z = Math.min(minCorner.z, properties.position.z - (properties.dimensions.z / 2));

            maxCorner.x = Math.max(maxCorner.x, properties.position.x - (properties.dimensions.x / 2));
            maxCorner.y = Math.max(maxCorner.y, properties.position.y - (properties.dimensions.y / 2));
            maxCorner.z = Math.max(maxCorner.z, properties.position.z - (properties.dimensions.z / 2));

        }
        // if (properties.userData == "mapped") {
        //     properties.visible = false;
        //     var entity = entityManager.add(properties.type, properties);
        //     mappedEntities.push(entity);
        // } else if (properties.userData == "tracked") {
        //     // TODO implement tracking of objects
        // }
    }

    var dimensions = {
        x: maxCorner.x - minCorner.x,
        y: maxCorner.y - minCorner.y,
        z: maxCorner.z - minCorner.z,
    };
    Vec3.print("dims", dimensions);

    var center = {
        x: minCorner.x + (dimensions.x / 2),
        y: minCorner.y + (dimensions.y / 2),
        z: minCorner.z + (dimensions.z / 2),
    };
    Vec3.print("center", center);

    var trackedEntities = [];
    var waypointEntities = [];
    for (var i = 0; i < entities.length; ++i) {
        var entityID = entities[i];
        var properties = Entities.getEntityProperties(entityID);
        var mapData = null;
        try {
            var data = JSON.parse(properties.userData.replace(/(\r\n|\n|\r)/gm,""));
            mapData = data.mapData;
        } catch (e) {
            print("Caught: ", properties.name);
        }

        if (mapData) {
            print("Creating copy of", properties.name);
            properties.name += " (COPY)";
            properties.userData = "";
            properties.visible = true;
            var position = properties.position;
            properties.position = Vec3.subtract(properties.position, center);
            properties.position = Vec3.multiply(properties.position, ROOT_SCALE);
            var extra = { };

            if (mapData.track) {
                extra.trackingEntityID= entityID;
                trackedEntities.push(entity);
                rootObject.addChild(entity);
            }
            if (mapData.waypoint) {
                print("Waypoint: ", mapData.waypoint.name);
                // properties.type = "Model";
                // properties.modelURL = "atp:ca49a13938376b3eb68d7b2b9189afb3f580c07b6950ea9e65b5260787204145.fbx";
                extra.waypoint = mapData.waypoint;
                extra.waypoint.position = position;
            }

            var entity = entityManager.add(properties.type, properties);
            entity.__extra__ = extra;
            if (mapData.waypoint) {
                waypointEntities.push(entity);
            }
            mappedEntities.push(entity);

            rootObject.addChild(entity);

        } else {
            // print("Not creating copy of", properties.name);
        }
    }

    var avatarArrowEntity = entityManager.add("Model", {
        name: "You Are Here",
        modelURL: "atp:ce4f0c4e491e40b73d28f2646da4f676fe9ea364cf5f1bf5615522ef6acfd80e.fbx",
        position: Vec3.multiply(Vec3.subtract(MyAvatar.position, center), ROOT_SCALE),
        dimensions: { x: 30, y: 100, z: 100 },
    });
    rootObject.addChild(avatarArrowEntity);

    this.isVisible = function() {
        return visible;
    }

    Controller.mousePressEvent.connect(mousePressEvent);
    function mousePressEvent(event) {
        // Entities.setZonesArePickable(false);

        var pickRay = Camera.computePickRay(event.x, event.y);
        for (var i = 0; i < waypointEntities.length; ++i) {
            var entity = waypointEntities[i];
            print("Checkit for hit", entity.__extra__.waypoint.name);
            var result = raySphereIntersection(pickRay.origin, pickRay.direction, entity.worldPosition, 0.1);//entity.worldScale);
            if (result) {
                print("Pressed entity: ", entity.id);
                print("Pressed waypoint: ", entity.__extra__.waypoint.name);
                print("Teleporting...");
                MyAvatar.position = entity.__extra__.waypoint.position;
                break;
            }
        }
        // var result = Entities.findRayIntersection(pickRay, false);
        // if (result.intersects) {
        //     var entity = entityManager.get(result.entityID);
        //     if (entity) {
        //         print("Pressed entity: ", entity.id);
        //     }
        //     if (entity && entity.__extra__.waypoint) {
        //         print("Pressed waypoint: ", entity.__extra__.waypoint.name);
        //         print("Teleporting...");
        //         MyAvatar.position = entity.__extra__.waypoint.position;
        //     }
        // }

        // Entities.setZonesArePickable(true);
    };

    var time = 0;
    Script.update.connect(function(dt) {
        time += dt;
        // Update tracked entities
        for (var i = 0; i < trackedEntities.length; ++i) {
            entity = trackedEntities[i];
            var entityID = entity.__extra__.trackingEntityID;
            var properties = Entities.getEntityProperties(entityID);
            properties.position = Vec3.subtract(properties.position, center);
            properties.position = Vec3.multiply(properties.position, ROOT_SCALE);
            entity.position = properties.position;
        }

        
        var position = Vec3.subtract(MyAvatar.position, center)
        position.y += 60 + (Math.sin(time) * 10);
        position = Vec3.multiply(position, ROOT_SCALE);
        avatarArrowEntity.position = position;
        // Vec3.print("Position:", avatarArrowEntity.position);

        // rootObject.position = Vec3.sum(position, { x: 0, y: Math.sin(time) / 30, z: 0 });
        var ROOT_OFFSET = Vec3.multiply(1, Quat.getFront(MyAvatar.orientation));
        var ROOT_POSITION = Vec3.sum(MyAvatar.position, ROOT_OFFSET);
        // position = ROOT_POSITION;
        rootObject.position = ROOT_POSITION;
        entityManager.update();
    });

    function setVisible(newValue) {
        if (visible != newValue) {
            visible = newValue;

            if (visible) {
            } else {
            }
        }
    }

    this.show = function() {
        setVisible(true);
    }

    this.hide = function() {
        setVisible(false);
    }
};

var map = null;
map = Map(mapData);

// On press key
Controller.keyPressEvent.connect(function(event) {
    if (event.text == "m") {
        if (!map) {
            map = Map(mapData);
        }

        map.show();
        print("MAP!");
    }
});





var mapData = {
    config: {
        // World dimensions that the minimap maps to
        worldDimensions: {
            x: 10.0,
            y: 10.0,
            z: 10.0,
        },
        // The center of the map should map to this location in the center of the area
        worldCenter: {
            x: 5.0,
            y: 5.0,
            z: 5.0,
        },
        // Map dimensions
        mapDimensions: {
            x: 10.0,
            y: 10.0,
            z: 10.0,
        },

        // Can this be automated? Tag entities that should be included? Store in UserData?
        objects: [
            {
                type: "Model",
                modelURL: "https://hifi-public.s3.amazonaws.com/ozan/sets/huffman_set/huffman_set.fbx",
            },
        ],
    },
    waypoints: [
        {
            name: "Forest's Edge",
            position: {
            },
        },
    ],
};


// entityManager = new OverlayManager();
// entityManager = new EntityManager();
//
// var rootEntity = entityManager.addBare();
//
// var time = 0;
//
//
// rootEntity.scale = 0.1;
// Script.include("sfData.js");
// rootEntity.addChild(entity);
entityManager.update();
