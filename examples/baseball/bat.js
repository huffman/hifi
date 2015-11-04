(function() {
    Script.include("pitching.js");
    var pitchingMachine = null;
    this.startNearGrab = function() {
        print("Started near grab!");
        if (!pitchingMachine) {
            pitchingMachine = getPitchingMachine();
            Script.update.connect(function(dt) { pitchingMachine.update(dt); });
        }
        pitchingMachine.start();
        MyAvatar.shouldRenderLocally = false;
    };
    this.releaseGrab = function() {
        print("Stopped near grab!");
        if (pitchingMachine) {
            pitchingMachine.stop();
        }
        MyAvatar.shouldRenderLocally = true;
    };
});
