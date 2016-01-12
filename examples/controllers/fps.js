// FPS Controller
//
// Events:
//   equipBegin: hand
//   equipEnd: hand
//
//   useBegin: hand
//   useEnd: hand
//
//   farGrabBegin: hand
//   farGrabEnd: hand
//
//   nearGrabBegin: hand
//   nearGrabEnd: hand
//

const STATES = {
    IDLE: 0,
    EQUIPPED: 1
};

var DISTANCE_HOLDING_ACTION_TIMEFRAME = 0.1; // how quickly objects move to their new position
var ACTION_TTL = 15; // seconds

var state = STATES.IDLE;
var equippedEntityID = null;
var actionID = null;

var MIN_EQUIP_DISTANCE = 0.1;
var MAX_EQUIP_DISTANCE = 3.0;
var equipDistance = 0.5;

var useActive = false;

function sendEventToEntity(event, args) {
    if (args === undefined) {
        args = {};
    }
    args.hand = 'right';
    Entities.callEntityMethod(equippedEntityID, event, JSON.stringify(args));
    print("sending", event, "to", equippedEntityID);
}

function buildActionProperties() {
    var position = MyAvatar.position;
    position.y += 0.5;
    var targetPosition = Vec3.sum(position, Quat.getFront(MyAvatar.headOrientation));
    return {
        targetPosition: targetPosition,
        linearTimeScale: DISTANCE_HOLDING_ACTION_TIMEFRAME,
        targetRotation: MyAvatar.headOrientation,
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
        sendEventToEntity('useBegin');
    }
});

Controller.mouseReleaseEvent.connect(function(ev) {
    if (state == STATES.EQUIPPED) {
        useActive = false;
        sendEventToEntity('useEnd');
    }
});

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

                print("Adding action");
                actionID = Entities.addAction("spring", equippedEntityID, buildActionProperties());
            }
        }
    } else if (ev.text == 'g') {
        if (state == STATES.EQUIPPED) {
            Entities.deleteAction(equippedEntityID, actionID);
            setState(STATES.IDLE);
        }
    }
});

