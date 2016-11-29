(function() {
    var SOUND_FX = SoundCache.getSound("http://hifi-content.s3.amazonaws.com/caitlyn/production/sounds/slidewhistleUp.wav");
	var soundFX_Injector = Audio.playSound(SOUND_FX, {
            position: MyAvatar.position,
            volume: 0.75,
            loop: false,
            localOnly: true
        });
	soundFX_Injector = undefined;
    for (var i = 0; i < 5; i++){
        MyAvatar.increaseSize();
    }
}());