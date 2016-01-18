//
// FPS Controller
//
// Events:
//   equipBegin: hand, entityID
//   equipEnd: hand, entityID
//
//   useBegin: hand, entityID, , position, direction
//   useEnd: hand, entityID
//
//   farGrabBegin: hand, entityID
//   farGrabEnd: hand, entityID
//
//   nearGrabBegin: hand, entityID
//   nearGrabEnd: hand, entityID
//

const STATES = {
    IDLE: 0,
    EQUIPPED: 1
};

var DISTANCE_HOLDING_ACTION_TIMEFRAME = 0.001; // how quickly objects move to their new position
var ACTION_TTL = 15; // seconds

var state = STATES.IDLE;
var equippedEntityID = null;
var actionID = null;

var MIN_EQUIP_DISTANCE = 0.1;
var MAX_EQUIP_DISTANCE = 3.0;
var equipDistance = 0.5;

var useActive = false;
var relativePosition = Vec3.ZERO;
var relativeRotation = Quat.fromPitchYawRollDegrees(0, 0, 0);

var crosshairOverlayID = Overlays.addOverlay('text', {
    x: Window.innerWidth / 2 - 1,
    y: Window.innerHeight / 2 - 1,
    width: 2,
    height: 2,
    backgroundAlpha: 1.0,
    backgroundColor: { red: 255, green: 255, blue: 255 }
});

Script.scriptEnding.connect(function() {
    Overlays.deleteOverlay(crosshairOverlayID);
    unequip();
});

function sendEventToEntity(event, args) {
    if (args === undefined) {
        args = {};
    }
    args.hand = 'right';
    Entities.callEntityMethod(equippedEntityID, event, [JSON.stringify(args)]);
    print("sending", event, "to", equippedEntityID);
}

function buildActionProperties() {
    var position = MyAvatar.position;
    var offset = Vec3.multiplyQbyV(MyAvatar.headOrientation, { x: 0.3, y: 0.5, z: -0.8 });
    var targetPosition = Vec3.sum(position, offset);
    return {
        targetPosition: targetPosition,
        linearTimeScale: DISTANCE_HOLDING_ACTION_TIMEFRAME,
        targetRotation: Quat.multiply(MyAvatar.headOrientation, relativeRotation),
        angularTimeScale: DISTANCE_HOLDING_ACTION_TIMEFRAME,
        tag: 'grab' + MyAvatar.sessionUUID,
        ttl: ACTION_TTL
    };
}

function setState(newState) {
    if (state != newState) {
        // Handle leave state
        switch (state) {
            case STATES.IDLE:
                break;
            case STATES.EQUIPPED:
                if (useActive) {
                    useActive = false;
                    sendEventToEntity('useEnd');
                }
                sendEventToEntity('equipEnd');
                sendEventToEntity('unequip'); // BACKWARDS-COMPATIBILITY

                equippedEntityID = null;
                break;
        }

        // Handle enter state
        state = newState;
        switch (state) {
            case STATES.IDLE:
                break;
            case STATES.EQUIPPED:
                sendEventToEntity('equipBegin');
                sendEventToEntity('startEquip'); // BACKWARDS-COMPATIBILITY
                break;
        }
    }
}

Script.update.connect(function(dt) {
    if (state == STATES.EQUIPPED) {
        Entities.updateAction(equippedEntityID, actionID, buildActionProperties());
    }
});

Controller.mousePressEvent.connect(function(ev) {
    if (state == STATES.EQUIPPED) {
        useActive = true;
        var args = {
            position: Camera.position,
            direction: Vec3.multiplyQbyV(Camera.orientation, Vec3.FRONT)
        };
        sendEventToEntity('useBegin', args);
        sendEventToEntity('fire');
    }
});

Controller.mouseReleaseEvent.connect(function(ev) {
    if (state == STATES.EQUIPPED) {
        useActive = false;
        sendEventToEntity('useEnd');
    }
});
parseJSON = function(jsonString) {
    var data;
    try {
        data = JSON.parse(jsonString);
    } catch(e) {
        data = {};
    }
    return data;
}

Controller.keyPressEvent.connect(function(ev) {
    print(ev.text);
    if (ev.text == 'e') {
        if (state == STATES.IDLE || state == STATES.EQUIPPED) {
            print('equip');
            var pickRay = Camera.computePickRay(Window.innerWidth/2, Window.innerHeight/2);
            var entityResult = Entities.findRayIntersection(pickRay, true); // want precision picking
            if (entityResult.intersects && entityResult.entityID != equippedEntityID) {
                setState(STATES.EQUIPPED);
                print('hit!', entityResult.entityID);
                equippedEntityID = entityResult.entityID;
                var properties = Entities.getEntityProperties(equippedEntityID, ['userData']);
                var data = parseJSON(properties.userData);
                try {
                    relativePosition = data.grabbableKey.spatialKey.relativePosition;
                } catch (e) {
                    relativePosition = Vec3.ZERO;
                }
                try {
                    relativeRotation = data.grabbableKey.spatialKey.relativeRotation;
                } catch (e) {
                    relativeRotation = Quat.fromPitchYawRollDegrees(0, 0, 0);
                }

                print("Adding action");
                actionID = Entities.addAction("spring", equippedEntityID, buildActionProperties());
            }
        }
    } else if (ev.text == 'g') {
        unequip();
    }
});

function unequip() {
    if (state == STATES.EQUIPPED) {
        Entities.deleteAction(equippedEntityID, actionID);
        setState(STATES.IDLE);
    }
}

