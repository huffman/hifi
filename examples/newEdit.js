// Create transform tools
var rootTransformID = LocalEntities.addEntity({ type: "Sphere", visible: true, position: { x: 15, y: 15.7, z: 15 } });

function extend(a, b) {
    var c = {};
    for (var key in a) {
        c[key] = a[key];
    }
    for (var key in b) {
        c[key] = b[key];
    }
    return c;
}

var RED = { red: 230, green: 64, blue: 64 };
var GREEN = { red: 64, green: 230, blue: 64 };
var BLUE = { red: 64, green: 64, blue: 230 };

var xRotation = Quat.fromPitchYawRollDegrees(0, 90, 0);
var yRotation = Quat.fromPitchYawRollDegrees(-90, 0, 0);
var zRotation = Quat.fromPitchYawRollDegrees(0, 0, 0);

var toolsProperties = { type: "Box", localRotation: xRotation, visible: false, parentID: rootTransformID };
var xToolsID = LocalEntities.addEntity(extend(toolsProperties, { localRotation: xRotation }));
var yToolsID = LocalEntities.addEntity(extend(toolsProperties, { localRotation: yRotation }));
var zToolsID = LocalEntities.addEntity(extend(toolsProperties, { localRotation: zRotation }));

var scaleProperties = { type: "Box", localPosition: { x: 0, y: 0, z: 0.2 }, dimensions: { x: 0.05, y: 0.05, z: 0.05 } };
var translateProperties = { type: "Box", localPosition: { x: 0, y: 0, z: 0.1 }, localRotation: Quat.fromPitchYawRollDegrees(45, 0, 0), dimensions: { x: 0.05, y: 0.05, z: 0.05 } };

var scaleXID = LocalEntities.addEntity(extend(scaleProperties, { parentID: xToolsID, color: RED }));
var scaleYID = LocalEntities.addEntity(extend(scaleProperties, { parentID: yToolsID, color: GREEN }));
var scaleZID = LocalEntities.addEntity(extend(scaleProperties, { parentID: zToolsID, color: BLUE }));

var translateXID = LocalEntities.addEntity(extend(translateProperties, { parentID: xToolsID, color: RED }));
var translateYID = LocalEntities.addEntity(extend(translateProperties, { parentID: yToolsID, color: GREEN }));
var translateZID = LocalEntities.addEntity(extend(translateProperties, { parentID: zToolsID, color: BLUE }));

var t = 0;
Script.update.connect(function(dt) {
    t += dt;
    LocalEntities.editEntity(rootTransformID, { position: { x: 15, y: 15 + 0.5 * Math.sin(t), z: 15 }});
});

Script.scriptEnding.connect(function() {
    LocalEntities.deleteEntity(rootTransformID);
});
