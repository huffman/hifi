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

var audioInjector = false;

function callbackOnCondition(conditionFunc, ms, callback, count) {
	var thisCount = 0;
	if(typeof count !== 'undefined')
		thisCount = count;
	print("Checking condition");
	if(conditionFunc()) {
		callback();
	}
	else if(thisCount < 10) {
		print("condition is false");
		Script.setTimeout(function() {
			callbackOnCondition(conditionFunc, ms, callback, thisCount + 1);
		}, ms);
		return;
	}
	else {
		print("callbackOnCondition timeout");
	}
}

function playAnim(animURL, looping, onFinished) {
	// Start caching the animation if not already cached.
	AnimationCache.getAnimation(animURL);

	// Tell the avatar to animate so that we can tell if the animation is ready without crashing
	Avatar.startAnimation(animURL);

	// Continually check if the animation is ready
	callbackOnCondition(function(){
		var details = Avatar.getAnimationDetails();
		// if we are running the request animation and are past the first frame, the anim is loaded properly
		print("attempting to play " + animURL);
		return details.running && details.url == animURL && details.currentFrame > 0;
	}, 100, function(){
		var timeOfAnim = ((AnimationCache.getAnimation(animURL).frames.length / 30) * 1000) + 100; // frames to miliseconds plus a small buffer
		print("animation loaded. Will play for: " + timeOfAnim);
		// Start the animation again but this time with frame information
		Avatar.startAnimation(animURL, 30, 1, looping, true, 0, AnimationCache.getAnimation(animURL).frames.length);
		if(typeof onFinished !== 'undefined') {
			print("onFinished found for anim. Setting a timeout");
			Script.setTimeout(onFinished, timeOfAnim); 
		}
	});
}

function playSound(soundURL, onFinished) {
	print("Got sound URL: " + soundURL);
	callbackOnCondition(function() {
		return SoundCache.getSound(soundURL).downloaded;
	}, 100, function() {
		print("sound downloaded. playing now");
		audioInjector = Audio.playSound(SoundCache.getSound(soundURL), {position: Avatar.position, volume: 1.0});
		if(typeof onFinished !== 'undefined') {
			print("onFinished found. connecting to AudioInjector");
			audioInjector.finished.connect(onFinished);
		}
	});
}

function npcRespond(soundURL, animURL, onFinished) {
	if(soundURL) {
		playSound(soundURL, function(){
			print("Sound finished playing");
			var animDetails = Avatar.getAnimationDetails();
			print("lastFrame: " + animDetails.lastFrame);
			print("currentFrame: " + animDetails.currentFrame);
			if(animDetails.lastFrame > animDetails.currentFrame + 1)
				onFinished();
			audioInjector = false;
		});
	}
	if(animURL) {
		playAnim(animURL, false, function() {
			print("Animation finished playing.");
			if(!audioInjector || !audioInjector.isPlaying())
				onFinished();
		});
	}
}

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
