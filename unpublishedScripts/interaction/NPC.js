(function() {
	var myID = 0;

	this.preload = function(id) {
		print("Preloading!");
		print(id);
		myID = id;
		print(myID);
		// prepareNPC(this);
	};

	this.onFocused = function(myID, paramsArray) {
		print("onFocused called!");
		Entities.editEntity(myID, {color: {red:255, green:255, blue: 255}});
	};

	this.onLostFocused = function(myID, paramsArray) {
		print("onLostFocused called!");
		Entities.editEntity(myID, {color: {red:255, green:0, blue: 255}});
	};

	this.onNodReceived = function(myID, paramsArray) {
		print("onNodReceived called!");
		Entities.editEntity(myID, {color: {red:0, green:255, blue: 0}});
	};

	this.onShakeReceived = function(myID, paramsArray) {
		print("onShakeReceived called!");
		Entities.editEntity(myID, {color: {red:255, green:0, blue: 0}});
	};
})
