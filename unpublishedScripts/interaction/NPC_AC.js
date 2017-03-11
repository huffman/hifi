Script.include("file:///home/delamare/gitclones/hifi/unpublishedScripts/interaction/NPCHelpers.js", function(){print("NPCHelpers included.");main();});

var walkForwardAnim = "https://storage.googleapis.com/limitlessserv-144100.appspot.com/hifi%20assets/walk_fwd.fbx";
var idleAnim = "https://storage.googleapis.com/limitlessserv-144100.appspot.com/hifi%20assets/idle.fbx";

var lightFST = "https://storage.googleapis.com/limitlessserv-144100.appspot.com/hifi%20assets/beingOfLight.fst";
var lightThankful = "https://storage.googleapis.com/limitlessserv-144100.appspot.com/hifi%20assets/beingOfLightThankful.fbx";
var lightWave = "https://storage.googleapis.com/limitlessserv-144100.appspot.com/hifi%20assets/beingOfLightWave.fbx";
var gameOverURL = "https://storage.googleapis.com/limitlessserv-144100.appspot.com/hifi%20assets/ArcadeGameover.wav";

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
				blocked = false;
				Avatar.orientation = Quat.lookAtSimple(Avatar.position, AvatarList.getAvatar(sender).position);
				playAnim(idleAnim, true);
			}
			else if (message.search("onLostFocused") != -1) {
				blocked = false;
				Avatar.orientation = startingOrientation;
				playAnim(idleAnim, true);
			}
			else if (message.search("onNodReceived") != -1) {
				npcRespondBlocking(null, lightThankful, function(){print("finished onNodReceived response");playAnim(idleAnim, true);});
			}
			else if (message.search("onShakeReceived") != -1) {
				npcRespondBlocking(null, lightWave, function(){print("finished onShakeReceived response");playAnim(idleAnim, true);});
			}
			else if (message.search("voiceData") != -1) {
				if(message.search("thank") != -1) {
				npcRespondBlocking(null, lightThankful, function(){print("finished thank response");playAnim(idleAnim, true);});
				}
				else if (message.search("bye") != -1) {
				npcRespondBlocking(gameOverURL, lightWave, function(){print("finished bye response");playAnim(idleAnim, true);});
				}
				else if (message.search("test") != -1) {
					npcRespondBlocking(gameOverURL, null, function(){print("finished test");playAnim(idleAnim, true);});
				}
			}
		}
	});
	playAnim(idleAnim, true);
}
