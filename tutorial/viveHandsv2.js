var PARENT_ID = "{00000000-0000-0000-0000-000000000001}";
var LEFT_JOINT_INDEX = MyAvatar.getJointIndex("_CONTROLLER_LEFTHAND");
var RIGHT_JOINT_INDEX = MyAvatar.getJointIndex("_CONTROLLER_RIGHTHAND");
//var LEFT_JOINT_INDEX = MyAvatar.getJointIndex("LeftHand");
//var RIGHT_JOINT_INDEX = MyAvatar.getJointIndex("RightHand");

var zeroPosition = { x: 0, y: 0, z: 0 };
var zeroRotation = { x: 0, y: 0, z: 0, w: 1 };

var CONTROLLER_LENGTH_OFFSET = 0.0762; 

var naturalPosition = {
    x: 0,
    y: -0.034076502197422087,
    z: 0.06380049744620919
};
var naturalPositionL = {
    x: 0,
    y: 0.034076502197422087,
    z: 0.06380049744620919
};
var naturalPositionR = {
    x: 0.0,
    y: 0.034076502197422087,
    z: 0.06380049744620919
};

var leftBasePosition = {
    x: CONTROLLER_LENGTH_OFFSET / 2,
    y: CONTROLLER_LENGTH_OFFSET * 2,
    z: CONTROLLER_LENGTH_OFFSET / 2
};
var rightBasePosition = {
    x: -CONTROLLER_LENGTH_OFFSET / 2,
    y: CONTROLLER_LENGTH_OFFSET * 2,
    z: CONTROLLER_LENGTH_OFFSET / 2
};

var leftBasePositionVive = Vec3.sum(leftBasePosition, { x: 0.005, y: 0.03, z: 0 });
var rightBasePositionVive = Vec3.sum(rightBasePosition, { x: -0.005, y: 0.03, z: 0 });

Vec3.print("left offset: ", leftBasePosition);

var leftBaseRotation = Quat.multiply(
    Quat.fromPitchYawRollDegrees(0, 0, 45),
    Quat.multiply(
        Quat.fromPitchYawRollDegrees(90, 0, 0),
        Quat.fromPitchYawRollDegrees(0, 0, 90)
    )
);

var rightBaseRotation = Quat.multiply(
    Quat.fromPitchYawRollDegrees(0, 0, -45),
    Quat.multiply(
        Quat.fromPitchYawRollDegrees(90, 0, 0),
        Quat.fromPitchYawRollDegrees(0, 0, -90)
    )
);


var touchLeftBaseRotation = Quat.multiply(
    Quat.fromPitchYawRollDegrees(0, 0, 0),
    Quat.multiply(
        Quat.fromPitchYawRollDegrees(0, 0, -45),
        Quat.multiply(
            Quat.fromPitchYawRollDegrees(180, 0, 0),
            Quat.fromPitchYawRollDegrees(0, -90, 0)
        )
    )
);

var touchRightBaseRotation = Quat.multiply(
    Quat.fromPitchYawRollDegrees(0, 0, 45),
    Quat.multiply(
        Quat.fromPitchYawRollDegrees(180, 0, 0),
        Quat.fromPitchYawRollDegrees(0, 90, 0)
    )
);

var TOUCH_CONTROLLER_CONFIGURATION = {
    name: "Touch",
    controllers: [
        {
            modelURL: "C:/Users/Ryan/Assets/controller/touch_l_full.fbx",
            jointIndex: MyAvatar.getJointIndex("_CONTROLLER_LEFTHAND"),
            rotation: touchLeftBaseRotation,
            //position: Vec3.sum(leftBasePosition, { x: 0.032, y: 0.0, z: -0.02 }),
            position: Vec3.sum(leftBasePosition, { x: 0.0, y: -0.016, z: -0.02 }),
            //dimensions: naturalDimensions,
        },
        {
            modelURL: "C:/Users/Ryan/Assets/controller/touch_r_full.fbx",
            jointIndex: MyAvatar.getJointIndex("_CONTROLLER_RIGHTHAND"),
            rotation: touchRightBaseRotation,
            //position: rightBasePosition,
            position: Vec3.sum(rightBasePosition, { x: 0.0, y: -0.016, z: -0.02 }),
            //dimensions: naturalDimensions,
        }
    ]
}

var TOUCH_2_CONTROLLER_CONFIGURATION = {
    name: "Touch",
    controllers: [
        {
            modelURL: "C:/Users/Ryan/Assets/controller/oculus_touch_l.fbx",
            naturalPosition: {
                x: 0.016486000269651413,
                y: -0.035518500953912735,
                z: -0.018527504056692123
            },
            jointIndex: MyAvatar.getJointIndex("_CONTROLLER_LEFTHAND"),
            rotation: touchLeftBaseRotation,
            position: leftBasePosition,

            annotationTextRotation: Quat.fromPitchYawRollDegrees(20, -90, 0),
            annotations: {

                buttonX: {
                    position: {
                        x: -0.00931,
                        y: 0.00212,
                        z: -0.01259,
                    },
                    direction: "left",
                    color: { red: 100, green: 100, blue: 100 },
                },
                buttonY: {
                    position: {
                        x: -0.01617,
                        y: 0.00216,
                        z: 0.00177,
                    },
                    direction: "left",
                    color: { red: 100, green: 255, blue: 100 },
                },
                bumper: {
                    position: {
                        x: 0.00678,
                        y: -0.02740,
                        z: -0.02537,
                    },
                    direction: "left",
                    color: { red: 100, green: 100, blue: 255 },
                },
                trigger: {
                    position: {
                        x: -0.01275,
                        y: -0.01992,
                        z: 0.02314,
                    },
                    direction: "left",
                    color: { red: 255, green: 100, blue: 100 },
                }
            },
        },
        {
            modelURL: "C:/Users/Ryan/Assets/controller/oculus_touch_r.fbx",
            naturalPosition: {
                x: -0.016486000269651413,
                y: -0.035518500953912735,
                z: -0.018527504056692123
            },
            jointIndex: MyAvatar.getJointIndex("_CONTROLLER_RIGHTHAND"),
            rotation: touchRightBaseRotation,
            position: rightBasePosition,

            annotationTextRotation: Quat.fromPitchYawRollDegrees(20, 90, 0),
            annotations: {

                buttonA: {
                    position: {
                        x: 0.00931,
                        y: 0.00212,
                        z: -0.01259,
                    },
                    direction: "right",
                    color: { red: 100, green: 100, blue: 100 },
                },
                buttonB: {
                    position: {
                        x: 0.01617,
                        y: 0.00216,
                        z: 0.00177,
                    },
                    direction: "right",
                    color: { red: 100, green: 255, blue: 100 },
                },
                bumper: {
                    position: {
                        x: 0.00678,
                        y: -0.02740,
                        z: -0.02537,
                    },
                    direction: "right",
                    color: { red: 100, green: 100, blue: 255 },
                },
                trigger: {
                    position: {
                        x: 0.01275,
                        y: -0.01992,
                        z: 0.02314,
                    },
                    direction: "right",
                    color: { red: 255, green: 100, blue: 100 },
                }
            },
        }
    ]
}


var viveNaturalDimensions = {
    x: 0.1174320001155138,
    y: 0.08361100335605443,
    z: 0.21942697931081057
};
var viveNaturalPosition = {
    x: 0,
    y: -0.034076502197422087,
    z: 0.06380049744620919
};
var viveModelURL = "https://hifi-public.s3.amazonaws.com/huffman/controllers/vive2.fbx";

var VIVE_CONTROLLER_CONFIGURATION = {
    name: "Vive",
    controllers: [
        {
            modelURL: viveModelURL,
            jointIndex: MyAvatar.getJointIndex("_CONTROLLER_LEFTHAND"),
            naturalPosition: viveNaturalPosition,
            rotation: leftBaseRotation,
            position: Vec3.multiplyQbyV(Quat.fromPitchYawRollDegrees(0, 0, 45), leftBasePosition),

            dimensions: viveNaturalDimensions,

            annotationTextRotation: Quat.fromPitchYawRollDegrees(20, -90, 0),
            annotations: {
//                red: {
//                    debug: true,
//                    position: {
//                        x: 0.1,
//                        y: 0.0,
//                        z: 0.0
//                    },
//                    direction: "right",
//                    color: { red: 255, green: 0, blue: 0 },
//                },
//                green: {
//                    debug: true,
//                    position: {
//                        x: 0.0,
//                        y: 0.1,
//                        z: 0.0
//                    },
//                    direction: "right",
//                    color: { red: 0, green: 255, blue: 0 },
//                },
//                blue: {
//                    debug: true,
//                    position: {
//                        x: 0.0,
//                        y: 0.0,
//                        z: 0.1
//                    },
//                    direction: "right",
//                    color: { red: 0, green: 0, blue: 255 },
//                },
//                white: {
//                    debug: true,
//                    position: {
//                        x: 0.0,
//                        y: 0.0,
//                        z: 0.0
//                    },
//                    direction: "right",
//                    color: { red: 255, green: 255, blue: 255 },
//                },

                center: {
                    position: zeroPosition,
                    direction: "center",
                    color: { red: 100, green: 255, blue: 255 },
                },
                trigger: {
                    position: {
                        x: 0,
                        y: -0.023978,
                        z: 0.04546
                    },
                    direction: "left",
                    color: { red: 255, green: 100, blue: 100 },
                },
                menu: {
                    position: {
                        x: 0,
                        y: 0.00770,
                        z: 0.01979
                    },
                    direction: "left",
                    color: { red: 255, green: 100, blue: 100 },
                },
                grip: {
                    position: {
                        x: 0.01980,
                        y: -0.01561,
                        z: 0.08721
                    },
                    direction: "left",
                    color: { red: 255, green: 100, blue: 100 },
                },
                pad: {
                    position: {
                        x: 0,
                        y: 0.00378,
                        z: 0.04920
                    },
                    direction: "left",
                    color: { red: 255, green: 100, blue: 100 },
                },
                steam: {
                    position: {
                        x: 0,
                        y: 0.00303,
                        z: 0.08838
                    },
                    direction: "left",
                    color: { red: 255, green: 100, blue: 100 },
                },
            },
        },
        {
            modelURL: viveModelURL,
            jointIndex: MyAvatar.getJointIndex("_CONTROLLER_RIGHTHAND"),

            rotation: rightBaseRotation,
            position: Vec3.multiplyQbyV(Quat.fromPitchYawRollDegrees(0, 0, -45), rightBasePosition),

            dimensions: viveNaturalDimensions,

            naturalPosition: {
                x: 0,
                y: -0.034076502197422087,
                z: 0.06380049744620919
            },

            annotationTextRotation: Quat.fromPitchYawRollDegrees(20, -90, 0),
            annotations: {

                trigger: {
                    position: {
                        x: 0,
                        y: -0.023978,
                        z: 0.04546
                    },
                    direction: "left",
                    color: { red: 255, green: 100, blue: 100 },
                },
                menu: {
                    position: {
                        x: 0,
                        y: 0.00770,
                        z: 0.01979
                    },
                    direction: "left",
                    color: { red: 255, green: 100, blue: 100 },
                },
                grip: {
                    position: {
                        x: 0.01980,
                        y: -0.01561,
                        z: 0.08721
                    },
                    direction: "left",
                    color: { red: 255, green: 100, blue: 100 },
                },
                pad: {
                    position: {
                        x: 0,
                        y: 0.00378,
                        z: 0.04920
                    },
                    direction: "left",
                    color: { red: 255, green: 100, blue: 100 },
                },
                steam: {
                    position: {
                        x: 0,
                        y: 0.00303,
                        z: 0.08838
                    },
                    direction: "left",
                    color: { red: 255, green: 100, blue: 100 },
                },
            }
        }
    ]
}

var DEBUG = true;

function setupController(config) {
    var controllerDisplay = {
        overlays: [],
        annotations: {
        },
        mappingName: ""
    };
    for (var i = 0; i < config.controllers.length; ++i) {
        var controller = config.controllers[i];
        var position = controller.position;
        if (controller.naturalPosition) {
            position = Vec3.sum(Vec3.multiplyQbyV(
                        controller.rotation, controller.naturalPosition), position);
        }
        var overlayID = Overlays.addOverlay("model", {
            url: controller.modelURL,
            dimensions: controller.dimensions,
            localRotation: controller.rotation,
            localPosition: position,
            parentID: PARENT_ID,
            parentJointIndex: controller.jointIndex,
            ignoreRayIntersection: true,
        });

        controllerDisplay.overlays.push(overlayID);

        if (controller.annotations) {
            for (var key in controller.annotations) {
                var annotation = controller.annotations[key];
                var annotationPosition = Vec3.sum(controller.position, Vec3.multiplyQbyV(controller.rotation, annotation.position));
                if (DEBUG) {
                    overlayID = Overlays.addOverlay("sphere", {
                        localPosition: annotationPosition,
                        //localPosition: Vec3.sum(controller.position, annotation.position),
                        //localPosition: Vec3.sum(position, annotation.position),
                        color: annotation.color || { red: 255, green: 100, blue: 100 },
                        dimensions: {
                            x: 0.01,
                            y: 0.01,
                            z: 0.01
                        },
                        parentID: PARENT_ID,
                        parentJointIndex: controller.jointIndex,
                    });
                    controllerDisplay.overlays.push(overlayID);

                    controllerDisplay.annotations[key] = {
                        overlay: overlayID,
                    };
                }

                var sign = annotation.direction == "right" ? 1 : -1;
                var textOffset = annotation.direction == "right" ? 0.04 : -0.01;
                var textOverlayID = Overlays.addOverlay("text3d", {
                    text: key,
                    localPosition: Vec3.sum(annotationPosition, Vec3.multiplyQbyV(controller.rotation, { x: textOffset, y: 0, z: 0.0 })),
                    localRotation: controller.annotationTextRotation,
                    lineHeight: 0.01,
                    leftMargin: 0,
                    rightMargin: 0,
                    topMargin: 0,
                    bottomMargin: 0,
                    backgroundAlpha: 0,
                    dimensions: { x: 0.003, y: 0.003, z: 0.003 },
                    //localPosition: Vec3.sum(controller.position, annotation.position),
                    //localPosition: Vec3.sum(position, annotation.position),
                    color: annotation.textColor || { red: 255, green: 255, blue: 255 },
                    parentID: PARENT_ID,
                    parentJointIndex: controller.jointIndex,
                });
                controllerDisplay.overlays.push(textOverlayID);

                var offset = { x: 0, y: 0, z: annotation.direction == "right" ? -0.1 : 0.1 };
                var lineOverlayID = Overlays.addOverlay("line3d", {
                    visible: false,
                    localPosition: annotationPosition,
                    localStart: { x: 0, y: 0, z: 0 },
                    localEnd: offset,
                    //localPosition: Vec3.sum(controller.position, annotation.position),
                    //localPosition: Vec3.sum(position, annotation.position),
                    color: annotation.color || { red: 255, green: 100, blue: 100 },
                    parentID: PARENT_ID,
                    parentJointIndex: controller.jointIndex,
                });
                controllerDisplay.overlays.push(lineOverlayID);
            }
        }
    }
    return controllerDisplay;
}

ControllerDisplay = function() {
};

function deleteControllerDisplay(controllerDisplay) {
    for (var i = 0; i < controllerDisplay.overlays.length; ++i) {
        Overlays.deleteOverlay(controllerDisplay.overlays[i]);
    }
    Controller.disableMapping(controllerDisplay.mappingName);
}

// var triggerAnnotationOverlayID = Overlays.addOverlay("text3d", {
//     text: "Trigger",
//     lineHeight: 0.025,
//     backgroundAlpha: 0.0,
//     dimensions: {
//         x: 0.2,
//         y: 0.2,
//     },
//     localPosition: Vec3.sum(leftBasePosition, { x: -0.09, y: -0.025, z: 0.03 }),
//     localRotation: Quat.multiply(Quat.fromPitchYawRollDegrees(180, 0, 90), leftBaseRotation),
//     parentID: MyAvatar.sessionUUID,
//     parentJointIndex: MyAvatar.getJointIndex("LeftHand")
// });

// var leftOverlayID = Overlays.addOverlay("model", {
//     url: "https://hifi-public.s3.amazonaws.com/huffman/controllers/vr_controller_vive_1_5.obj",
//     dimensions: naturalDimensions,
//     localRotation: leftBaseRotation,
//     localPosition: leftBasePosition,
//     parentID: PARENT_ID,
//     parentJointIndex: LEFT_JOINT_INDEX
// });
//
// var leftTriggerOverlayID = Overlays.addOverlay("model", {
//     url: "C:/Users/Ryan/Assets/controller/touch_l_trigger.fbx",
//     visible: false,
//     localRotation: leftBaseRotation,
//     localPosition: Vec3.sum(leftBasePosition, { x: -0.05, y: -0.025, z: 0.02 }),
//     parentID: PARENT_ID,
//     parentJointIndex: LEFT_JOINT_INDEX
// });

// var rightOverlayID = Overlays.addOverlay("model", {
//     url: "https://hifi-public.s3.amazonaws.com/huffman/controllers/vr_controller_vive_1_5.obj",
//     dimensions: naturalDimensions,
//     localRotation: rightBaseRotation,
//     localPosition: rightBasePosition,
//     parentID: PARENT_ID,
//     parentJointIndex: RIGHT_JOINT_INDEX
// });
//
// var rightTriggerOverlayID = Overlays.addOverlay("model", {
//     url: "C:/Users/Ryan/Assets/controller/touch_r_trigger.fbx",
//     visible: false,
//     localRotation: rightBaseRotation,
//     localPosition: Vec3.sum(rightBasePosition, { x: 0.05, y: -0.025, z: 0.02 }),
//     parentID: PARENT_ID,
//     parentJointIndex: RIGHT_JOINT_INDEX
// });
//
var overlays = [
    // leftOverlayID,
    // leftTriggerOverlayID,
    // triggerAnnotationOverlayID,

    // rightOverlayID,
    // rightTriggerOverlayID,
];
//
// Script.setInterval(function() {
//     // var pose = MyAvatar.getLeftHandControllerPoseInWorldFrame();
//     // Overlays.editOverlay(leftOverlayID, {
//     //     position: pose.translation,
//     //     rotation: pose.rotation
//     // });
//     // pose = MyAvatar.getRightHandControllerPoseInWorldFrame();
//     // Overlays.editOverlay(rightOverlayID, {
//     //     position: pose.translation,
//     //     rotation: pose.rotation
//     // });
// }, 10);


var MAPPING_NAME = "com.highfidelity.handControllerGrab.disable";
var mapping = Controller.newMapping(MAPPING_NAME);
mapping.from([Controller.Standard.LT]).to(function(value) {
    // print(value);
    // Overlays.editOverlay(leftTriggerOverlayID, {
    //     localRotation: Quat.multiply(Quat.fromPitchYawRollDegrees(0, 0, value * -45), leftBaseRotation)
    // });
});
mapping.from([Controller.Standard.RT]).to(function(value) {
    // print(value);
    // Overlays.editOverlay(rightTriggerOverlayID, {
    //     localRotation: Quat.multiply(Quat.fromPitchYawRollDegrees(0, 0, value * 45), rightBaseRotation)
    // });
});
Controller.enableMapping(MAPPING_NAME);

//var c = setupController(TOUCH_CONTROLLER_CONFIGURATION);
//var c = setupController(TOUCH_2_CONTROLLER_CONFIGURATION);
var c = setupController(VIVE_CONTROLLER_CONFIGURATION);
//MyAvatar.shouldRenderLocally = false;
Script.scriptEnding.connect(function() {
    deleteControllerDisplay(c);
    MyAvatar.shouldRenderLocally = true;
    for (var i = 0; i < overlays.length; ++i) {
        Overlays.deleteOverlay(overlays[i]);
    }
    Controller.disableMapping(MAPPING_NAME);
});
