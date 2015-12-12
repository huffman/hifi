Script.include("components.js");

var entities = {};

function onEntityAdded(entityID) {
    print("Added entity", entityID);
    entities[entityID] = new EntityManager(entityID, true);
    //entities.push(entityID);
}

function onEntityRemoved(entityID) {
    print("Removed entity", entityID);
    var entityManager = entities[entityID];
    if (entityManager) {
        entityManager.shutdown();
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
EntityViewer.setKeyholeRadius(33000);
Script.setInterval(function() {
    EntityViewer.queryOctree();
}, 1000);
