var skeleFST = "http://mpassets.highfidelity.com/72e083ee-194d-4113-9c61-0591d8257493-v1/skeleton_Rigged.fst";
var walkForwardAnim = "file:///home/delamare/gitclones/hifi/build/interface/resources/avatar/animations/walk_fwd.fbx";
var idleAnim = "file:///home/delamare/gitclones/hifi/build/interface/resources/avatar/animations/idle.fbx";

var lightFST = "http://127.0.0.1:8080/beingOfLight.fst";
var lightThankful = "http://127.0.0.1:8080/beingOfLightThankful.fbx";
var lightWave = "http://127.0.0.1:8080/beingOfLightWave.fbx";

Avatar.skeletonModelURL = lightFST;
Avatar.displayName = "NPC";
Agent.isAvatar = true;
var animationData = {url: idleAnim, lastFrame: 35};
var millisecondsToWaitBeforeStarting = 10 * 1000;
Avatar.position = {x: -1537, y: 53.5, z: -1118};
Avatar.startAnimation(animationData.url, animationData.fps || 30, 1, true, false, animationData.firstFrame || 0, animationData.lastFrame);

Messages.subscribe("interactionComs");
print("New NPC @" + Agent.sessionUUID);
Messages.messageReceived.connect(function (channel, message, sender) {
  print("Messaged NPC @" + Agent.sessionUUID);
	if(channel === "interactionComs") {
		if(message.search(Agent.sessionUUID) != -1) {
			Avatar.orientation = Quat.lookAtSimple(Avatar.position, AvatarList.getAvatar(sender).position);
			Avatar.startAnimation(lightWave, animationData.fps || 30, 1, true, false, animationData.firstFrame || 0, animationData.lastFrame);
		}
	}
});
