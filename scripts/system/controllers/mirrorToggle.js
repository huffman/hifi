if ("OculusTouch" in Controller.Hardware) {
    var MAPPING_NAME = 'com.highfidelity.mirror-toggle';

    var lastCameraMode = null;

    mirrorMapping = Controller.newMapping(MAPPING_NAME);
    mirrorMapping.from([Controller.Hardware.OculusTouch.B, Controller.Hardware.OculusTouch.Y]).to(function(value) {
        if (value > 0) {
            // Button down
            lastCameraMode = Camera.mode;
            Camera.mode = "mirror";
        } else {
            // Button up
            if (lastCameraMode !== null) {
                Camera.mode = lastCameraMode;
                lastCameraMode = null;
            }
        }
    });

    Controller.enableMapping(MAPPING_NAME);

    function cleanup() {
        mirrorMapping.disable();
    }
    Script.scriptEnding.connect(cleanup);
}
