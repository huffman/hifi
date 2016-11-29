(function() {		
    var SOUND_FX = SoundCache.getSound("http://hifi-content.s3.amazonaws.com/caitlyn/production/sounds/quickPop.wav");
	var soundFX_Injector = Audio.playSound(SOUND_FX, {
            position: MyAvatar.position,
            volume: 0.7,
            loop: false,
            localOnly: true
        });
	soundFX_Injector = undefined;
    MyAvatar.resetSize();
}());