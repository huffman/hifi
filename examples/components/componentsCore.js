//
// componentsCore.js
//
// Provides:
//    EntityManager - Loads and manages components in an entity
//    Component - The base class for all components
//    registerComponent - A function to expose new component types
//

ComponentInfo = function(name, clientCtor, serverCtor) {
    this.name = name;
    this.clientCtor = clientCtor;
    this.serverCtor = serverCtor;
};

// A wrapper around an entity that loads and manages its components,
// as defined in its `userData` property.
EntityManager = function(entityID, isServer, entityScript, serverEntityList) {
    EventEmitter.call(this);

    this.entityID = entityID;
    this.isServer = isServer;
    this.serverEntityList = serverEntityList;

    var properties = Entities.getEntityProperties(entityID, ['userData']);
    var userData = parseJSON(properties.userData);

    this.components = {};
    var componentData = userData.components;
    if (componentData) {
        for (var componentType in componentData) {
            console.log("Creating component " + componentType);
            console.warn('component', entityID, componentType);
            var component = createComponent(this, componentType, componentData[componentType], isServer);
            if (component) {
                this.components[componentType] = component;
            }
        }
    }

    if (entityScript) {
        var callbackToEvent = {
            clickDownOnEntity: 'mouseDown',
            enterEntity: 'enterEntity',
            leaveEntity: 'exitEntity',

            startDistanceGrab: 'startDistanceGrab',
            releaseGrab: 'releaseGrab',

            equipBegin: 'equipBegin',
            equipEnd: 'equipEnd',
            useBegin: 'useBegin',
            useEnd: 'useEnd',
            farGrabBegin: 'farGrabBegin',
            farGrabEnd: 'farGrabEnd',
            nearGrabBegin: 'nearGrabBegin',
            nearGrabEnd: 'nearGrabEnd'
        };


        var self = this;
        for (var key in callbackToEvent) {
            console.log(key, ' to ', callbackToEvent[key]);
            var event = callbackToEvent[key];
            entityScript[key] = function(event) {
                return function() {
                    console.log("emitting, ", event);
                    self.emitWithArgs.apply(self, [event, arguments]);
                }.bind(this);
            }(event);
        }

        // remoteEvent is called on the client when an EntityManager wants to send
        // a message to another entity that should also be governed by an EntityManager.
        entityScript.remoteEvent = this.onRemoteEvent.bind(this);
    }

    Script.update.connect(function(dt) { this.emit('update', dt); }.bind(this));

    // Initialize components
    for (var key in this.components) {
        this.components[key].init();
    }
};
EntityManager.prototype = Object.create(EventEmitter.prototype);
extend(EntityManager.prototype, {
    getProperties: function(keys) {
        if (keys) {
            return Entities.getEntityProperties(this.entityID, keys);
        } else {
            return Entities.getEntityProperties(this.entityID);
        }
    },
    sendEvent: function(entityID, event, args) {
        if (!args) args = [];
        if (this.isServer) {
            var entityManager = this.serverEntityList[entityID];
            // console.log(JSON.stringify(Object.keys(this.serverEntityList)));
            if (entityManager) {
                entityManager.emitWithArgs(event, args);
            } else {
                console.warn("Could not send event to unknown entity: ", entityID);
            }
        } else {
            Entities.callEntityMethod(entityID, 'remoteEvent', [event, args]);
        }
    },
    onRemoteEvent: function(event, args) {
        console.log("Got remote event: ", event, args);
        this.emitWithArgs(event, args);
    },
    destroy: function() {
        for (var key in this.components) {
            this.components[key].destroy();
        }
    }
});

var componentInfos = {};

// Factory method for creating components
// TODO Move this into EntityManager
function createComponent(entityManager, type, properties) {
    if (!(type in componentInfos)) {
        console.error("Unknown component type: " + type);
        return null;
    }

    var componentInfo = componentInfos[type];
    var component;
    if (entityManager.isServer) {
        if (componentInfo.serverCtor) {
            component = new componentInfo.serverCtor(entityManager, properties);
        }
    } else {
        if (componentInfo.clientCtor) {
            component = new componentInfo.clientCtor(entityManager, properties);
        }
    }

    return component;
};

registerComponent = function(name, clientCtor, serverCtor) {
    if (name in componentInfos) {
        console.warn("Registering known component type: " + name);
    }
    componentInfos[name] = new ComponentInfo(name, clientCtor, serverCtor);
};


// Base block class
Component = function(entityManager) {
    this.entityManager = entityManager;
    this.entityID = entityManager.entityID;
    this.isServer = entityManager.isServer;

    this.serverChannel = "ec-server-" + this.type + "-" + this.entityID;
    this.clientChannel = "ec-client-" + this.type + "-" + this.entityID;

    if (this.isServer) {
        Messages.subscribe(this.serverChannel);
    } else {
        Messages.subscribe(this.clientChannel);
    }
    Messages.messageReceived.connect(this.handleMessage.bind(this));

    this.eventHandlers = {};
    for (var key in this) {
        if (key.indexOf('on' == 0)) {
            this.eventHandlers[key] = this[key];
        }
    }
    console.log("Done settings up event handlers");
};
Component.prototype = {
    init: function() {
        console.log("Initializing component " + this.type);
    },

    destroy: function() {
        console.log("Initializing component " + this.type);
    },

    on: function() {
        this.entityManager.apply(this.entityManager, arguments);
    },

    handleMessage: function(channel, message) {
        //console.log("Got", this.type, this, channel, message);
        //console.log('this', this);
        if (this.isServer) {
            if (channel == this.serverChannel) {
                var msg = JSON.parse(message);
                console.log(this.entityManager);
                this.entityManager.emit.apply(this.entityManager, [msg.event]);
            }
        } else {
            if (channel == this.clientChannel) {
                var msg = JSON.parse(message);
                this.entityManager.emit.apply(this.entityManager, [msg.event]);
            }
        }
    },

    // Send an asynchronous message to the server or clients
    async: function(event) {
        console.log("Sending async to", event);
        var args =  Array.prototype.slice.call(arguments, 1);
        // Sending
        if (this.isServer) {
            // Call directly ??
            Messages.sendMessage(this.clientChannel, JSON.stringify({ event: event, args: args }));
        } else {
            // Send message to server actor
            console.log("Sending message to server ", event);
            Messages.sendMessage(this.serverChannel, JSON.stringify({ event: event, args: args }));
        }
        console.log("Done");
    }
};
