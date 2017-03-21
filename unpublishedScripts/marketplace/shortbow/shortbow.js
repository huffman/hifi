//
//  Created by Ryan Huffman on 1/10/2017
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

var shortbowEntities = require("./shortbow.json");

// Add LocalPosition to entity data if parent properties are available
var entities = shortbowEntities.Entities;
var entitiesByID = {};
var i, entity;
for (i = 0; i < entities.length; ++i) {
    entity = entities[i];
    entitiesByID[entity.id] = entity;
}
for (i = 0; i < entities.length; ++i) {
    entity = entities[i];
    if (entity.parentID !== undefined) {
        var parent = entitiesByID[entity.parentID];
        if (parent !== undefined) {
            entity.localPosition = Vec3.subtract(entity.position, parent.position);
            delete entity.position;
        }
    }
}

module.exports = shortbowEntities;
