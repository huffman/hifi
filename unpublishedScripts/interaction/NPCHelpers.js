// Helper functinons for NPC_AC.js
var audioInjector = false;
var blocked = false;

function callbackOnCondition(conditionFunc, ms, callback, count) {
	var thisCount = 0;
	if(typeof count !== 'undefined')
		thisCount = count;
	if(conditionFunc()) {
		callback();
	}
	else if(thisCount < 10) {
		Script.setTimeout(function() {
			callbackOnCondition(conditionFunc, ms, callback, thisCount + 1);
		}, ms);
	}
	else {
		print("callbackOnCondition timeout");
	}
}

function playAnim(animURL, looping, onFinished) {
	print("got anim: " + animURL);
	print("looping: " + looping);
	// Start caching the animation if not already cached.
	AnimationCache.getAnimation(animURL);

	// Tell the avatar to animate so that we can tell if the animation is ready without crashing
	Avatar.startAnimation(animURL);

	// Continually check if the animation is ready
	callbackOnCondition(function(){
		var details = Avatar.getAnimationDetails();
		// if we are running the request animation and are past the first frame, the anim is loaded properly
		print("running: " + details.running);
		print("url and animURL: " + details.url.trim().replace(/ /g, "%20") + " | " + animURL.trim().replace(/ /g, "%20"));
		print("currentFrame: " + details.currentFrame);
		return details.running && details.url.trim().replace(/ /g, "%20") == animURL.trim().replace(/ /g, "%20") && details.currentFrame > 0;
	}, 250, function(){
		var timeOfAnim = ((AnimationCache.getAnimation(animURL).frames.length / 30) * 1000) + 100; // frames to miliseconds plus a small buffer
		print("animation loaded. length: " + timeOfAnim);
		// Start the animation again but this time with frame information
		Avatar.startAnimation(animURL, 30, 1, looping, true, 0, AnimationCache.getAnimation(animURL).frames.length);
		if(typeof onFinished !== 'undefined') {
			print("onFinished defined. setting the timeout with timeOfAnim");
			Script.setTimeout(onFinished, timeOfAnim); 
		}
	});
}

function playSound(soundURL, onFinished) {
	callbackOnCondition(function() {
		return SoundCache.getSound(soundURL).downloaded;
	}, 250, function() {
		audioInjector = Audio.playSound(SoundCache.getSound(soundURL), {position: Avatar.position, volume: 1.0});
		if(typeof onFinished !== 'undefined') {
			audioInjector.finished.connect(onFinished);
		}
	});
}

function npcRespond(soundURL, animURL, onFinished) {
	if(soundURL) {
		playSound(soundURL, function(){
			print("sound finished");
			var animDetails = Avatar.getAnimationDetails();
			print("animDetails.lastFrame: " + animDetails.lastFrame);
			if(animDetails.lastFrame > animDetails.currentFrame + 1)
				onFinished();
			audioInjector = false;
		});
	}
	if(animURL) {
		playAnim(animURL, false, function() {
			print("anim finished");
			print("injector: " + audioInjector);
			// print("audioInjector.isPlaying(): " + audioInjector.isPlaying());
			if(!audioInjector || !audioInjector.isPlaying())
				onFinished();
		});
	}
}

function npcRespondBlocking(soundURL, animURL, onFinished) {
	print("blocking response requested");
	if(!blocked) {
		print("not already blocked");
		blocked = true;
		npcRespond(soundURL, animURL, function(){ print("blocking response finished");onFinished();blocked = false; });
	}
}
