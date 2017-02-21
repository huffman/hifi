function getForwardOffset(distance) {
	return Vec3.sum(MyAvatar.position, Vec3.multiply(distance, Quat.getFront(MyAvatar.orientation)));
}

var NPC = false;
var hasCenteredOnNPC = false;
var distance = 5;
var r = 8;

function spawnDebugSphere() {
	Entities.addEntity({
		type: "Sphere",
		position: getForwardOffset(distance),
		dimensions: {x: r, y: r, z: r},
		lifetime: 0.1
	});
}

function findlookedAtNPC() {
	var intersection = Entities.findRayIntersection({origin: MyAvatar.position, direction: Quat.getFront(Camera.getOrientation())}, true);
	if(intersection.intersects && intersection.distance <= distance){
		var name = Entities.getEntityProperties(intersection.entityID, 'name').name
		if(name.search("NPC") != -1)
			return intersection.entityID;
	}
	return false;
}

function findNPCInFocusArea() {
	// spawnDebugSphere();
	var ents = Entities.findEntities(getForwardOffset(distance), r);
	return ents.filter(function(x){return Entities.getEntityProperties(x, 'name').name.search("NPC") != -1;});
}

var baselineX = 0;
var baselineY = 0;
var nodRange = 0.35;
var shakeRange = 0.2;

function onWeLostFocus() {
	print("lost NPC: " + NPC);
	Entities.callEntityMethod(NPC, 'onLostFocused', [MyAvatar.sessionUUID]);
	var baselineX = 0;
	var baselineY = 0;
}

function onWeGainedFocus() {
	print("found NPC: " + NPC);
	Entities.callEntityMethod(NPC, 'onFocused', [MyAvatar.sessionUUID]);
	var rotation = Quat.safeEulerAngles(Camera.getOrientation());
	baselineX = rotation.x;
	baselineY = rotation.y;
}

function tick () {

 	var rotation = Quat.safeEulerAngles(Camera.getOrientation());
 	if(!NPC || NPC === "false") {
 		// start by looking for an NPC in a direct line
 		NPC = findlookedAtNPC();
 		if(NPC) 
 			onWeGainedFocus();
 		NPC = NPC.toString();
 	}
 	else {
 		print("NPC: " + NPC);
 		print("typeof NPC: " + typeof NPC);
 		newNPC = findNPCInFocusArea().toString();
 		print("newNPC: " + newNPC);
 		if(newNPC != NPC)
 		{
			onWeLostFocus();
			NPC = newNPC;
			if(newNPC && newNPC == findlookedAtNPC().toString())
				onWeGainedFocus();
 		}
 		// check for a nod or shake
 		deltaX = Math.abs(rotation.x - baselineX);
 		deltaY = Math.abs(rotation.y - baselineY);
 		if(deltaX >= nodRange && rotation.y <= nodRange)
 			Entities.callEntityMethod(NPC, 'onNodReceived', [MyAvatar.sessionUUID]);
 		else if (deltaY >= shakeRange && rotation.x <= shakeRange)
 			Entities.callEntityMethod(NPC, 'onShakeReceived', [MyAvatar.sessionUUID]);
 	}
}

print("Avatar preload");

if(typeof ticker === 'undefined')
	ticker = Script.setInterval(tick, 666);
else {
	Script.clearInterval(ticker);
	ticker = Script.setInterval(tick, 666);
}
