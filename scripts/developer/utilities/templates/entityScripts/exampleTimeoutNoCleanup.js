//
//  exampleTimeoutNoCleanup.js
//  examples/entityScripts
//
//  Created by Brad Hefta-Gaub on 4/18/16.
//  Copyright 2016 High Fidelity, Inc.
//
//  This is an example of an entity script which hooks the update signal
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

(function() {
    var _this;

    // this is the "constructor" for the entity as a JS object we don't do much here, but we do want to remember
    // our this object, so we can access it in cases where we're called without a this (like in the case of various global signals)
    ExampleUpdate = function() { 
         _this = this;
    };

    ExampleUpdate.prototype = {

        // preload() will be called when the entity has become visible (or known) to the interface
        // it gives us a chance to set our local JavaScript object up. In this case it means:
        //   * remembering our entityID, so we can access it in cases where we're called without an entityID
        //   * connecting to the update signal so we can check our grabbed state
        preload: function(entityID) {
            print("preload - entityID:" + entityID);
            this.entityID = entityID;

            Script.setInterval(function() {
                    var entityID = _this.entityID;
                    print("timer interval in entityID:" + entityID);
                }, 3000);
        },

        // unload() will be called when our entity is no longer available. It may be because we were deleted,
        // or because we've left the domain or quit the application. In all cases we want to unhook our connection
        // to the update signal
        unload: function(entityID) {
            print("unload - entityID:" + entityID);
            print("NOTE --- WE DID NOT CALL clear our timeout");
        },
    };

    // entity scripts always need to return a newly constructed object of our type
    return new ExampleUpdate();
})
