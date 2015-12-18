Script.include("components.js");

var entities = {};


function onEntityAdded(entityID) {
    print("Added entity", entityID);
    entities[entityID] = new EntityManager(entityID, true, null, entities);
    print("Entities", JSON.stringify(Object.keys(entities)));
}

function onEntityRemoved(entityID) {
    print("Removed entity", entityID);
    var entityManager = entities[entityID];
    if (entityManager) {
        delete entities[entityID];
    }
}

function onEntitiesCleared() {
    print("Entities cleared");
    for (var key in entities) {
        entities[key].shutdown();
    }
    entities = {};
}

Entities.addingEntity.connect(onEntityAdded);
Entities.deletingEntity.connect(onEntityRemoved);
Entities.clearingEntities.connect(onEntitiesCleared);

// Setup entity viewer
EntityViewer.setPosition({ x: 0, y: 0, z: 0 });
EntityViewer.setKeyholeRadius(60000);
Script.setInterval(function() {
    EntityViewer.queryOctree();
}, 1000);
