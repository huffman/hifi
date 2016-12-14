if ("OculusTouch" in Controller.Hardware) {
    var MAPPING_NAME = 'com.highfidelity.mirror-toggle';

    var lastCameraMode = null;

    function toggleMirrorAction(value) {
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
    }

    mirrorMapping = Controller.newMapping(MAPPING_NAME);

    mirrorMapping.from(Controller.Standard.B).to(toggleMirrorAction);
    mirrorMapping.from(Controller.Standard.Y).to(toggleMirrorAction);

    Controller.enableMapping(MAPPING_NAME);

    function cleanup() {
        mirrorMapping.disable();
    }
    Script.scriptEnding.connect(cleanup);
}
