var loggingRules = [
    "trace.*=false",
    "trace.resource=true",
    "trace.resource.load=true",
].join("\n");

function epochSeconds() {
    return Math.floor(Date.now() / 1000);
}
var startSound = SoundCache.getSound(Script.resolvePath("start.wav"));
var stopSound = SoundCache.getSound(Script.resolvePath("stop.wav"));

var tracePath = Script.resolvePath("resource-load-" + epochSeconds() + ".json.gz").substring("file:///".length);

print("Saving trace to", tracePath);

location = "localhost/0,0,0"
var running = false;

var keyReleaseHandler = function(event) {
   if (event.text == "F1" && !running) {
        running = true;
        Audio.playSound(startSound, {
            position: MyAvatar.position,
            volume: 0.7,
            loop: false
        });
        Test.startTracing(loggingRules); 
   } else if (event.text == "F2" && running) {
        running = false;
        Audio.playSound(stopSound, {
            position: MyAvatar.position,
            volume: 0.7,
            loop: false
        });
        Test.stopTracing(tracePath);
        Test.quit();
   }
};
Controller.keyReleaseEvent.connect(keyReleaseHandler);

