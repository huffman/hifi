// Limitelss NPC interaction

(function(){
	print("loading interaction script");

	var Avatar = false;
	var NPC = false;
	var hasCenteredOnNPC = false;
	var distance = 10;
	var r = 8;

	var baselineX = 0;
	var baselineY = 0;
	var nodRange = 20;
	var shakeRange = 20;

	var ticker = false;

	function callOnNPC(message) {
		Messages.sendMessage("interactionComs", message + ": " + NPC);
	}

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

	function isStillFocusedNPC() {
		var avatar = AvatarList.getAvatar(NPC);
		if(avatar) {
			var avatarPosition = avatar.position;
			return Vec3.distance(MyAvatar.position, avatarPosition) <= distance && Math.abs(Quat.dot(Camera.getOrientation(), Quat.lookAtSimple(MyAvatar.position, avatarPosition))) > 0.6;
		}
		return false; // NPC reference died. Maybe it crashed or we teleported to a new world?
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

		var deltaX = Math.abs(rotation.x - baselineX);
		if(deltaX > 180)
			deltaX -= 180;
		var deltaY = Math.abs(rotation.y - baselineY);
		if(deltaY > 180)
			deltaY -= 180;

		if(deltaX >= nodRange && deltaY <= shakeRange)
			callOnNPC("onNodReceived");
		else if (deltaY >= shakeRange && deltaX <= nodRange)
			callOnNPC("onShakeReceived");
	}

	function tick() {
		checkFocus();
		if(NPC)
			checkGesture();
	}

	this.enterEntity = function(id) {
		print("Something entered me: " + id);
		if(!ticker) {
			ticker = Script.setInterval(tick, 333);
		}
	};
	this.leaveEntity = function(id) {
		print("Something left me: " + id);
		if(ticker) {
			ticker.stop();
			ticker = false;
		}
	};
	this.unload = function() {
		print("Okay. I'm Unloading!");
		if(ticker) {
			ticker.stop();
			ticker = false;
		}
	}
	print("finished loading interaction script");
})
