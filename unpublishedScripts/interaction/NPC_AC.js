var skeleFST = "http://mpassets.highfidelity.com/72e083ee-194d-4113-9c61-0591d8257493-v1/skeleton_Rigged.fst";
var walkForwardAnim = "file:///home/delamare/gitclones/hifi/build/interface/resources/avatar/animations/walk_fwd.fbx";
var idleAnim = "file:///home/delamare/gitclones/hifi/build/interface/resources/avatar/animations/idle.fbx";

var lightFST = "http://192.168.1.10:8080/beingOfLight.fst";
var lightThankful = "http://192.168.1.10:8080/beingOfLightThankful.fbx";
var lightWave = "http://192.168.1.10:8080/beingOfLightWave.fbx";

var animationData = {url: idleAnim, lastFrame: 35};

Agent.isAvatar = true;
Avatar.skeletonModelURL = lightFST;
Avatar.displayName = "NPC";
Avatar.position = {x: -1537, y: 53.5, z: -1118};
Avatar.startAnimation(animationData.url, animationData.fps || 30, 1, true, false, animationData.firstFrame || 0, animationData.lastFrame);

var startingOrientation = Avatar.orientation;

Messages.subscribe("interactionComs");
Messages.messageReceived.connect(function (channel, message, sender) {
	print(sender + " -> NPC @" + Agent.sessionUUID + ": " + message);
	if(channel === "interactionComs" && message.search(Agent.sessionUUID) != -1) {
		if(message.search("onFocused") != -1) {
			Avatar.orientation = Quat.lookAtSimple(Avatar.position, AvatarList.getAvatar(sender).position);
			Avatar.startAnimation(animationData.url, animationData.fps || 30, 1, true, false, animationData.firstFrame || 0, animationData.lastFrame);
		}
		else if (message.search("onLostFocused") != -1) {
			Avatar.orientation = startingOrientation;
			Avatar.startAnimation(animationData.url, animationData.fps || 30, 1, true, false, animationData.firstFrame || 0, animationData.lastFrame);
		}
		else if (message.search("onNodReceived") != -1) {
			Avatar.startAnimation(lightThankful, animationData.fps || 30, 1, true, false, animationData.firstFrame || 0, animationData.lastFrame);
		}
		else if (message.search("onShakeReceived") != -1) {
			Avatar.startAnimation(lightWave, animationData.fps || 30, 1, true, false, animationData.firstFrame || 0, animationData.lastFrame);
		}
		else if (message.search("voiceData") != -1) {
			if(message.search("thank") != -1) {
				Avatar.startAnimation(lightThankful, animationData.fps || 30, 1, true, false, animationData.firstFrame || 0, animationData.lastFrame);
			}
			else if (message.search("bye") != -1) {
				Avatar.startAnimation(lightWave, animationData.fps || 30, 1, true, false, animationData.firstFrame || 0, animationData.lastFrame);
			}
		}
	}
});
