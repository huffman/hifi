var console = {
    log: function() {
    }
};
// Taken from MDN
if (!Function.prototype.bind) {
    Function.prototype.bind = function(oThis) {
        if (typeof this !== 'function') {
            // closest thing possible to the ECMAScript 5
            // internal IsCallable function
            throw new TypeError('Function.prototype.bind - what is trying to be bound is not callable');
        }

        var aArgs   = Array.prototype.slice.call(arguments, 1),
            fToBind = this,
            fNOP    = function() {},
            fBound  = function() {
                return fToBind.apply(this instanceof fNOP
                                     ? this
                                     : oThis,
                                     aArgs.concat(Array.prototype.slice.call(arguments)));
            };

        if (this.prototype) {
            // native functions don't have a prototype
            fNOP.prototype = this.prototype; 
        }
        fBound.prototype = new fNOP();

        return fBound;
    };
}

function extend(a, b) {
    for (var key in b) {
        a[key] = b[key];
    }
}

EventEmitter = function() {
    this.eventListeners = {};
};
EventEmitter.prototype = {
    on: function(event, fn) {
        if (event in this.eventListeners) {
            this.eventListeners[event].push(fn);
        } else {
            this.eventListeners[event] = [fn];
        }
        console.log("on: ", event);
    },
    emit: function(event) {
        console.log('emit', this.entityID, event, this);
        var listeners = this.eventListeners[event];
        if (listeners) {
            console.log('emit', event);
            for (var i in listeners) {
                listeners[i].apply(null, Array.prototype.slice.call(arguments, 1));
            }
        } else {
            console.log("No listeners found for", event);
        }
    }
};

function parseJSON(userData) {
    var data;
    try {
        data = JSON.parse(userData);
    } catch(e) {
        data = {};
    }
    return data;
}

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
    console.log("Got data: ", properties.userData);

    this.components = {};
    var componentData = userData.components;
    if (componentData) {
        for (var componentType in componentData) {
            console.log("Creating component " + componentType);
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
            releaseGrab: 'releaseGrab'
        };


        var self = this;
        for (var key in callbackToEvent) {
            console.log(key, ' to ', callbackToEvent[key]);
            var event = callbackToEvent[key];
            entityScript[key] = function(event) {
                return function() {
                    console.log("emitting, ", event);
                    self.emit.apply(self, [event]);
                }.bind(this);
            }(event);
        }

        // remoteEvent is called on the client when an EntityManager wants to send
        // a message to another entity that should also be governed by an EntityManager.
        entityScript.remoteEvent = this.onRemoteEvent.bind(this);
    }

    //Script.update.connect(function(dt) { this.emit('update', dt); }.bind(this));

    // Initialize components
    for (var key in this.components) {
        this.components[key].init();
    }
};
EntityManager.prototype = Object.create(EventEmitter.prototype);
extend(EntityManager.prototype, {
    sendEvent: function(entityID, event) {
        if (this.isServer) {
            var entityManager = this.serverEntityList[entityID];
            console.log(JSON.stringify(Object.keys(this.serverEntityList))); if (entityManager) {entityManager.emit(event);
            } else {
                console.log("Could not send event to unknown entity: ", entityID);
            }
        } else {
            Entities.callEntityMethod(entityID, 'remoteEvent', [event]);
        }
    },
    onRemoteEvent: function(event) {
        console.log("Got remote event: ", event);
        this.emit(event);
    }
});

var componentInfos = {};

// Factory method for creating components
function createComponent(entityManager, type, properties) {
    if (!(type in componentInfos)) {
        console.log("Unknown component type: " + type);
        return null;
    }

    var componentInfo = componentInfos[type];
    var component;
    if (entityManager.isServer) {
        component = new componentInfo.serverCtor(entityManager, properties);
    } else {
        component = new componentInfo.clientCtor(entityManager, properties);
    }

    return component;
};

registerComponent = function(name, clientCtor, serverCtor) {
    if (name in componentInfos) {
        console.log("Warning: registering known component type: " + name);
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

    on: function() {
        this.entityManager.apply(this.entityManager, arguments);
    },

    handleMessage: function(channel, message) {
        console.log("Got", this.type, this, channel, message);
        console.log('this', this);
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

Script.include("componentsExtra.js");
