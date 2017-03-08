// Limitelss NPC interaction
print("loading interaction script");

var NPC = false;
var hasCenteredOnNPC = false;
var distance = 10;
var r = 8;

var baselineX = 0;
var baselineY = 0;
var nodRange = 20;
var shakeRange = 20;

MyAvatar.setListeningToVoice(false);

MyAvatar.onFinishedSpeaking.connect(function(speech) {
	print("Got: " + speech);
	callOnNPC("voiceData: " + speech);
});

function setBaselineRotations(rot) {
	baselineX = rot.x;
	baselineY = rot.y;
}

function findLookedAtNPC() {
	var intersection = AvatarList.findRayIntersection({origin: MyAvatar.position, direction: Quat.getFront(Camera.getOrientation())}, true);
	if(intersection.intersects && intersection.distance <= distance){
		var npcAvatar = AvatarList.getAvatar(intersection.avatarID);
		if(npcAvatar.displayName.search("NPC") != -1) {
			print("Found NPC!");
			setBaselineRotations(Quat.safeEulerAngles(Camera.getOrientation()));
			return intersection.avatarID;
		}
	}
	return false;
}

function callOnNPC(message) {
	Messages.sendMessage("interactionComs", message + ": " + NPC);
}

function isStillFocusedNPC() {
	var avatarPosition = AvatarList.getAvatar(NPC).position;
	return Vec3.distance(MyAvatar.position, avatarPosition) <= distance && Math.abs(Quat.dot(Camera.getOrientation(), Quat.lookAtSimple(MyAvatar.position, avatarPosition))) > 0.6;
}

function onWeLostFocus() {
	print("lost NPC: " + NPC);
	callOnNPC("onLostFocused");
	var baselineX = 0;
	var baselineY = 0;
	MyAvatar.setListeningToVoice(false);
}

function onWeGainedFocus() {
	print("found NPC: " + NPC);
	callOnNPC("onFocused");
	var rotation = Quat.safeEulerAngles(Camera.getOrientation());
	baselineX = rotation.x;
	baselineY = rotation.y;
	MyAvatar.setListeningToVoice(true);
}

function checkFocus() {
	var newNPC = findLookedAtNPC();

	if(NPC && newNPC != NPC && !isStillFocusedNPC()) {
		onWeLostFocus();
		NPC = false;
	}
	if(!NPC && newNPC != false) {
		NPC = newNPC;
		onWeGainedFocus();
	}
}

function checkGesture() {
	var rotation = Quat.safeEulerAngles(Camera.getOrientation());

	deltaX = Math.abs(rotation.x - baselineX);
	if(deltaX > 180)
		deltaX -= 180;
	deltaY = Math.abs(rotation.y - baselineY);
	if(deltaY > 180)
		deltaY -= 180;

	if(deltaX >= nodRange && rotation.y <= nodRange)
		callOnNPC("onNodReceived");
	else if (deltaY >= shakeRange && rotation.x <= shakeRange)
		callOnNPC("onShakeReceived");
}

function tick () {
	checkFocus();
	if(NPC)
		checkGesture();
}

if(typeof ticker === 'undefined')
	ticker = Script.setInterval(tick, 666);
else {
	Script.clearInterval(ticker);
	ticker = Script.setInterval(tick, 666);
}

print("finished loading interaction script");
