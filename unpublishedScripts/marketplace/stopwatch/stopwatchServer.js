//
//  stopwatchServer.js
//
//  Created by Ryan Huffman on 1/20/17.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

(function() {
    var self = this;

    self.equipped = false;
    self.isActive = false;

    self.secondHandID = null;
    self.minuteHandID = null;

    self.tickSound = SoundCache.getSound(Script.resolvePath("sounds/tick.wav"));
    self.tickInjector = null;
    self.tickIntervalID = null;

    self.chimeSound = SoundCache.getSound(Script.resolvePath("sounds/chime.wav"));

    // Search for the minute and second hands which are a child of this entity.
    //
    // Q: Why don't we just do this in the preload? 
    // A: It is not guaranteed that we will have received our children yet
    //    from the entity server when preload is called. To handle that case
    //    we occasionally check to see if they are available until we find them.
    self.findHands = function() {
        var childrenIDs = Entities.getChildrenIDs(self.entityID);

        for (var i = 0; i < childrenIDs.length; ++i) {
            var id = childrenIDs[i];
            var name = Entities.getEntityProperties(id, 'name').name;
            if (name === 'stopwatch/seconds') {
                self.secondHandID = id;
            } else if (name === 'stopwatch/minutes') {
                self.minuteHandID = id;
            }
        }

        if (self.secondHandID !== null && self.secondHandID !== null) {
            Script.clearInterval(self.findHandsIntervalID);
            self.findHandsIntervalID = null;
            self.resetTimer();
        }
    };

    self.preload = function(entityID) {
        print("Preloading stopwatch: ", entityID);
        self.entityID = entityID;
        self.messageChannel = "STOPWATCH-" + entityID;

        self.findHandsIntervalID = Script.setInterval(self.findHands, 500);
        self.findHands();

        Messages.subscribe(self.messageChannel);
        Messages.messageReceived.connect(this, self.messageReceived);
    };
    self.unload = function() {
        print("Unloading stopwatch:", self.entityID);
        self.resetTimer();
        Messages.unsubscribe(self.messageChannel);
        Messages.messageReceived.disconnect(this, self.messageReceived);
    };
    self.messageReceived = function(channel, message, sender) {
        print("Message received", channel, sender, message); 
        if (channel === self.messageChannel && message === 'click') {
            if (self.isActive) {
                self.resetTimer();
            } else {
                self.startTimer();
            }
        }
    };
    self.getStopwatchPosition = function() {
        return Entities.getEntityProperties(self.entityID, "position").position;
    };
    self.resetTimer = function() {
        print("Stopping stopwatch");
        if (self.tickInjector) {
            self.tickInjector.stop();
        }
        if (self.tickIntervalID !== null) {
            Script.clearInterval(self.tickIntervalID);
            self.tickIntervalID = null;
        }
        Entities.editEntity(self.secondHandID, {
            localRotation: Quat.fromPitchYawRollDegrees(0, 0, 0),
        });
        Entities.editEntity(self.minuteHandID, {
            localRotation: Quat.fromPitchYawRollDegrees(0, 0, 0),
        });
        self.isActive = false;
    };
    self.startTimer = function() {
        print("Starting stopwatch");
        if (!self.tickInjector) {
            self.tickInjector = Audio.playSound(self.tickSound, {
                position: self.getStopwatchPosition(),
                volume: 0.7,
                loop: true
            });
        } else {
            self.tickInjector.restart();
        }

        var seconds = 0;
        self.tickIntervalID = Script.setInterval(function() {
            if (self.tickInjector) {
                self.tickInjector.setOptions({
                    position: self.getStopwatchPosition(),
                    volume: 0.7,
                    loop: true
                });
            }
            seconds++;
            const degreesPerTick = -360 / 60;
            Entities.editEntity(self.secondHandID, {
                localRotation: Quat.fromPitchYawRollDegrees(0, seconds * degreesPerTick, 0),
            });
            if (seconds % 60 == 0) {
                Entities.editEntity(self.minuteHandID, {
                    localRotation: Quat.fromPitchYawRollDegrees(0, (seconds / 60) * degreesPerTick, 0),
                });
                Audio.playSound(self.chimeSound, {
                    position: self.getStopwatchPosition(),
                    volume: 1.0,
                    loop: false
                });
            }
        }, 1000);

        self.isActive = true;
    };
});
