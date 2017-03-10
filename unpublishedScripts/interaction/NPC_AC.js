Script.include("file:///home/delamare/gitclones/hifi/unpublishedScripts/interaction/NPCHelpers.js", function(){print("NPCHelpers included.");main();});

var skeleFST = "http://mpassets.highfidelity.com/72e083ee-194d-4113-9c61-0591d8257493-v1/skeleton_Rigged.fst";
var walkForwardAnim = "file:///home/delamare/gitclones/hifi/build/interface/resources/avatar/animations/walk_fwd.fbx";
var idleAnim = "file:///home/delamare/gitclones/hifi/build/interface/resources/avatar/animations/idle.fbx";

var lightFST = "http://192.168.1.10:8080/beingOfLight.fst";
var lightThankful = "http://192.168.1.10:8080/beingOfLightThankful.fbx";
var lightWave = "http://192.168.1.10:8080/beingOfLightWave.fbx";
var gameOverURL = "http://192.168.1.10:8080/ArcadeGameover.wav";

Agent.isAvatar = true;
Avatar.skeletonModelURL = lightFST;
Avatar.displayName = "NPC";
Avatar.position = {x: -1537, y: 53.5, z: -1118};

var startingOrientation = Avatar.orientation;

function main() {
	Messages.subscribe("interactionComs");
	Messages.messageReceived.connect(function (channel, message, sender) {
		print(sender + " -> NPC @" + Agent.sessionUUID + ": " + message);
		if(channel === "interactionComs" && message.search(Agent.sessionUUID) != -1) {
			if(message.search("onFocused") != -1) {
				Avatar.orientation = Quat.lookAtSimple(Avatar.position, AvatarList.getAvatar(sender).position);
				playAnim(idleAnim, true);
			}
			else if (message.search("onLostFocused") != -1) {
				Avatar.orientation = startingOrientation;
				playAnim(idleAnim, true);
			}
			else if (message.search("onNodReceived") != -1) {
				npcRespond(null, lightThankful, function(){print("finished onNodReceived response");playAnim(idleAnim, true);});
			}
			else if (message.search("onShakeReceived") != -1) {
				npcRespond(null, lightWave, function(){print("finished onShakeReceived response");playAnim(idleAnim, true);});
			}
			else if (message.search("voiceData") != -1) {
				if(message.search("thank") != -1) {
				npcRespond(null, lightThankful, function(){print("finished thank response");playAnim(idleAnim, true);});
				}
				else if (message.search("bye") != -1) {
				npcRespond(gameOverURL, lightWave, function(){print("finished bye response");playAnim(idleAnim, true);});
				}
				else if (message.search("test") != -1) {
					npcRespond(gameOverURL, null, function(){print("finished test");playAnim(idleAnim, true);});
				}
			}
		}
	});
	playAnim(idleAnim, true);
}

