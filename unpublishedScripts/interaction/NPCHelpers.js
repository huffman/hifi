// this._onFocused = function(id, paramsArray) {
// 	print("GOT A MESSAGE!!!");
// 	myID = id;
// 	this.onFocused(paramsArray[0]);
// };

prepareNPC = function(NPC) {
	print("preparing NPC!");
	NPC._onFocused = function(id, paramsArray) {
		print("_onFocused!!!");
		this.onFocused(paramsArray[0]);
	};
	NPC._onLostFocused = function(id, paramsArray) {
		print("_onLostFocused!!!");
		this._onLostFocused(paramsArray[0]);
	};
}
