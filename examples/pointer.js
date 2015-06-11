//  pointer.js
//  examples
//
//  Created by Seth Alves on May 15th
//  Modified by Eric Levin on June 4
//  Persist toolbar by HRS 6/11/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Provides a pointer with option to draw on surfaces
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

var lineEntityID = null;
var lineIsRezzed = false;

var BUTTON_SIZE = 32;
var PADDING = 3;

var buttonOffColor = {
  red: 250,
  green: 10,
  blue: 10
};
var buttonOnColor = {
  red: 10,
  green: 200,
  blue: 100
};

HIFI_PUBLIC_BUCKET = "http://s3.amazonaws.com/hifi-public/";
var screenSize = Controller.getViewportDimensions();

var userCanPoint = false;
Script.include(["libraries/toolBars.js"]);
const persistKey = "highfidelity.pointer.toolbar.position";
var toolBar = new ToolBar(0, 0, ToolBar.HORIZONTAL);
toolBar.save = function () {
    Settings.setValue(persistKey, JSON.stringify([toolBar.x, toolBar.y]));
};
var old = JSON.parse(Settings.getValue(persistKey) || '0');
var pointerButton = toolBar.addOverlay("image", {
  x: old ? old[0] : screenSize.x / 2 - BUTTON_SIZE * 2 + PADDING,
  y: old ? old[1] : screenSize.y - (BUTTON_SIZE + PADDING),
  width: BUTTON_SIZE,
  height: BUTTON_SIZE,
  imageURL: HIFI_PUBLIC_BUCKET + "images/laser.png",
  color: buttonOffColor,
  alpha: 1
});




function nearLinePoint(targetPosition) {
  var handPosition = MyAvatar.getRightPalmPosition();
  var along = Vec3.subtract(targetPosition, handPosition);
  along = Vec3.normalize(along);
  along = Vec3.multiply(along, 0.4);
  return Vec3.sum(handPosition, along);
}


function removeLine() {
  if (lineIsRezzed) {
    Entities.deleteEntity(lineEntityID);
    lineEntityID = null;
    lineIsRezzed = false;
  }
}


function createOrUpdateLine(event) {
  var pickRay = Camera.computePickRay(event.x, event.y);
  var intersection = Entities.findRayIntersection(pickRay, true); // accurate picking
  var props = Entities.getEntityProperties(intersection.entityID);

  if (intersection.intersects && userCanPoint) {
    var points = [nearLinePoint(intersection.intersection), intersection.intersection]
    if (lineIsRezzed) {
      Entities.editEntity(lineEntityID, {
        position: nearLinePoint(intersection.intersection),
        linePoints: points,
        dimensions: {
          x: 1,
          y: 1,
          z: 1
        },
        lifetime: 15 + props.lifespan // renew lifetime
      });
    } else {
      lineIsRezzed = true;
      lineEntityID = Entities.addEntity({
        type: "Line",
        position: nearLinePoint(intersection.intersection),
        linePoints: points,
        dimensions: {
          x: 1,
          y: 1,
          z: 1
        },
        color: {
          red: 255,
          green: 255,
          blue: 255
        },
        lifetime: 15 // if someone crashes while pointing, don't leave the line there forever.
      });
    }
  } else {
    removeLine();
  }
}


function mousePressEvent(event) {
  if (!event.isLeftButton) {
    return;
  }

  createOrUpdateLine(event);
   var clickedOverlay = Overlays.getOverlayAtPoint({
    x: event.x,
    y: event.y
  });
  if (clickedOverlay == pointerButton) {
    userCanPoint = !userCanPoint;
    if (userCanPoint === true) {
      Overlays.editOverlay(pointerButton, {
        color: buttonOnColor
      });
    } else {
      Overlays.editOverlay(pointerButton, {
        color: buttonOffColor
      });
    }
  }
}


function mouseMoveEvent(event) {
  createOrUpdateLine(event);
}


function mouseReleaseEvent(event) {
  if (!event.isLeftButton) {
    return;
  }
  removeLine();
}

function cleanup() {
  Overlays.deleteOverlay(pointerButton);
}

Script.scriptEnding.connect(cleanup);

Controller.mouseMoveEvent.connect(mouseMoveEvent);
Controller.mousePressEvent.connect(mousePressEvent);
Controller.mouseReleaseEvent.connect(mouseReleaseEvent);
