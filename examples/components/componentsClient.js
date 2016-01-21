//
// componentsClient.js
//
// Implements a client-side entity script for loading components
//

(function() {
    Script.include("components.js");

    var entityComponentManager = null;

    this.preload = function(entityID) {
        print("This is: ", this);
        entityComponentManager = new EntityManager(entityID, false, this);
    };
    this.unload = function() {
        entityComponentManager.destroy();
    };
});

// Script.include("components.js");
// var pos = Vec3.sum(MyAvatar.position, Quat.getFront(MyAvatar.orientation));
// pos.y += 0.5;
// var userData = {
//     components: {
//         other: {
//         },
//         button: {
//         }
//     }
// };
// var entityID = Entities.addEntity({
//     type: "Box",
//     position: pos,
//     userData: JSON.stringify(userData)
// });
// var entityComponentManager = new EntityManager(entityID, false);

// Script.scriptEnding.connect(function() {
//     Entities.deleteEntity(entityID);
// });
