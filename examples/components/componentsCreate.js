Script.include('utils.js');

var SCRIPT_URL = Script.resolvePath('componentsClient.js');

// Create a single entity in a scene, including its children, recursively
function _createEntity(sceneName, entityManager, parentID, data) {
    var children = [];
    if (data.hasOwnProperty('children')) {
        children = data.children;
        delete data.children;
    }
    if (data.hasOwnProperty('components')) {
        //data.userData = parseJSON(data.userData);
        if (typeof data.userData !== 'object') {
            data.userData = {};
        }
        data.userData.scene = sceneName;
        data.userData.components = data.components;
        data.userData = JSON.stringify(data.userData);
        delete data.components;
    } else {
        data.userData = JSON.stringify({ scene: sceneName });
    }
    print(data.userData);
    if (!data.hasOwnProperty('type')) {
        data.type = "Box";
        data.visible = false;
    }

    data.script = SCRIPT_URL;

    if (parentID) {
        data.parentID = parentID;
    }

    print("Creating ", data.name, data.type, data.parentID);
    var entityID = entityManager.addEntity(data);

    for (var i in children) {
        _createEntity(sceneName, entityManager, entityID, children[i]);
    }
}

createScene = function(scene) {
    for (var i in scene.entities) {
        _createEntity(scene.name, Entities, null, scene.entities[i]);
    }
};

createObject = function(object) {
    _createEntity('unknown', Entities, null, object);
};

destroyScene = function(name) {
    print("Destroying scene: ", name);

    // Find all entities that are tagged with this scene name
    var entities = Entities.findEntities({ x: 0, y: 0, z: 0 }, 1000000);
    for (var i = 0; i < entities.length; ++i) {
        var entityID = entities[i];
        var properties = Entities.getEntityProperties(entityID, ['userData']);
        var data = parseJSON(properties.userData);
        if (data.hasOwnProperty('scene') && data.scene == name) {
            print("Deleting", entityID);
            Entities.deleteEntity(entityID);
        }
    }
};

var types = {};
registerBlueprint = function(name, data) {
    if (data.indexOf(name) >= 0) {
        console.warning("Overwriting type ", name);
    }
    types[name] = data;
};

spawnBlueprint = function(name) {
    var blueprint = types[name];
    if (!blueprint) {
        console.warn("Cannot find blueprint: " + name);
        return null;
    }
    return Entities.addEntity(blueprint);
};
