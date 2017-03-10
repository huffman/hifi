// Helper functinons for NPC_AC.js
var audioInjector = false;

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
		return details.running && details.url == animURL && details.currentFrame > 0;
	}, 100, function(){
		var timeOfAnim = ((AnimationCache.getAnimation(animURL).frames.length / 30) * 1000) + 100; // frames to miliseconds plus a small buffer
		// Start the animation again but this time with frame information
		Avatar.startAnimation(animURL, 30, 1, looping, true, 0, AnimationCache.getAnimation(animURL).frames.length);
		if(typeof onFinished !== 'undefined') {
			Script.setTimeout(onFinished, timeOfAnim); 
		}
	});
}

function playSound(soundURL, onFinished) {
	callbackOnCondition(function() {
		return SoundCache.getSound(soundURL).downloaded;
	}, 100, function() {
		audioInjector = Audio.playSound(SoundCache.getSound(soundURL), {position: Avatar.position, volume: 1.0});
		if(typeof onFinished !== 'undefined') {
			audioInjector.finished.connect(onFinished);
		}
	});
}

function npcRespond(soundURL, animURL, onFinished) {
	if(soundURL) {
		playSound(soundURL, function(){
			var animDetails = Avatar.getAnimationDetails();
			if(animDetails.lastFrame > animDetails.currentFrame + 1)
				onFinished();
			audioInjector = false;
		});
	}
	if(animURL) {
		playAnim(animURL, false, function() {
			if(!audioInjector || !audioInjector.isPlaying())
				onFinished();
		});
	}
}
