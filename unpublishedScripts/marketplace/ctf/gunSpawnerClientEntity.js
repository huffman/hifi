(function() {
    var GUN_RED_MODEL_URL = Script.resolvePath("models/portalgun_red.fbx");
    var GUN_BLUE_MODEL_URL = Script.resolvePath("models/portalgun_blue.fbx");

    var gunProps = {
        "collisionsWillMove": 1,
        "compoundShapeURL": Script.resolvePath("models/portalgun_collider.obj"),
        "dimensions": {
            "x": 0.17742760479450226,
            "y": 0.38749998807907104,
            "z": 0.99309998750686646
        },
        "dynamic": 1,
        "gravity": {
            "x": 0,
            "y": -5,
            "z": 0
        },
        "modelURL": Script.resolvePath("models/portalgun_red.fbx"),
        "name": "Portal/Gun",
        "position": {
            "x": 1.4289360046386719,
            "y": 0,
            "z": 1.4532890319824219
        },
        "rotation": {
            "w": 0.51259636878967285,
            "x": -0.5248645544052124,
            "y": 0.5236133337020874,
            "z": -0.43306630849838257
        },
        lifetime: 100,
        velocity: {
            x: 0,
            y: 0.1,
            z: 0
        },
        "script": Script.resolvePath("portalGunClientEntity.js?" + Date.now()),
        "shapeType": "compound",
        "type": "Model",
        "userData": "{\"grabbableKey\":{\"invertSolidWhileHeld\":true},\"wearable\":{\"joints\":{\"RightHand\":[{\"x\":0.1177130937576294,\"y\":0.12922893464565277,\"z\":0.08307232707738876},{\"x\":0.4934672713279724,\"y\":0.3605862259864807,\"z\":0.6394805908203125,\"w\":-0.4664038419723511}],\"LeftHand\":[{\"x\":0.09151676297187805,\"y\":0.13639454543590546,\"z\":0.09354984760284424},{\"x\":-0.19628101587295532,\"y\":0.6418180465698242,\"z\":0.2830369472503662,\"w\":0.6851521730422974}]}}}"
    };

    var lightProps = {
        lifetime: 100,
        "color": {
            "blue": 255,
            "green": 166,
            "red": 41
        },
        "cutoff": 90,
        "dimensions": {
            "x": 5.0777130126953125,
            "y": 5.0777130126953125,
            "z": 5.0777130126953125
        },
        "falloffRadius": 2.2000000476837158,
        "intensity": 5,
        "name": "Portal/GunLight",
        "localPosition": {
            x: 0,
            y: 0.1,
            z: 0.5
        },
        "type": "Light"
    };

    var gunData = {
        red: {
            modelURL: GUN_RED_MODEL_URL,
            lightColor: {
                red: 255,
                green: 23,
                blue: 96,
            }
        },
        blue: {
            modelURL: GUN_BLUE_MODEL_URL,
            lightColor: {
                red: 41,
                green: 166,
                blue: 255,
            }
        }
    };

    var inCooldown = false;

    this.preload = function(entityID) {
        this.entityID = entityID;
    };

    function spawnWeapons() {
        if (inCooldown) {
            return;
        }

        var position = Entities.getEntityProperties(this.entityID, 'position').position;

        var userData = JSON.parse(gunProps.userData);
        var colors = ['red', 'blue'];
        for (var i = 0; i < colors.length; ++i) {
            var color = colors[i];
            gunProps.modelURL = gunData[color].modelURL;
            gunProps.position = position;
            userData.color = color;
            gunProps.userData = JSON.stringify(userData);
            var gunID = Entities.addEntity(gunProps);

            lightProps.color = gunData[color].lightColor;
            lightProps.parentID = gunID;
            Entities.addEntity(lightProps);
        }

        inCooldown = true;
        Script.setTimeout(function() {
            inCooldown = false;
        }, 2000);
    }

    this.startNearTrigger = spawnWeapons;
    this.startFarTrigger = spawnWeapons;
    this.clickDownOnEntity = spawnWeapons;
});
