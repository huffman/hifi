var angle = 0;
Script.setInterval(function() {
    angle += 360/32;
    MyAvatar.orientation = Quat.fromPitchYawRollDegrees(0, angle, 0);
}, 250);
