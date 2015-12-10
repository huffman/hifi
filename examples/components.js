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
        print("on: ", event);
    },
    emit: function(event) {
        var listeners = this.eventListeners[event];
        if (listeners) {
            print("emit: ", event);
            for (var i in listeners) {
                listeners[i].apply(null, Array.prototype.slice.call(arguments, 1));
            }
        }
    }
};

function parseUserDataAsJSON(userData) {
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
EntityManager = function(entityID, isServer, entityScript) {
    EventEmitter.call(this);

    this.entityID = entityID;
    this.isServer = isServer;

    var properties = Entities.getEntityProperties(entityID, ['userData']);
    var userData = parseUserDataAsJSON(properties.userData);

    this.components = {};
    var componentData = userData.components;
    if (componentData) {
        for (var componentType in componentData) {
            print("Creating component " + componentType);
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


        for (var key in callbackToEvent) {
            print(key, ' to ', callbackToEvent[key]);
            var event = callbackToEvent[key];
            entityScript[key] = function(event) {
                return function() {
                    print("got event ", event);
                    this.emit(event, arguments);
                }.bind(this);
            }(event);
        }
    }

    Script.update.connect(function(dt) { this.emit('update', dt); }.bind(this));

    // Initialize components
    for (var key in this.components) {
        this.components[key].init();
    }
};
EntityManager.prototype = Object.create(EventEmitter.prototype);
extend(EntityManager.prototype, {
});

var componentInfos = {};

// Factory method for creating components
function createComponent(entityManager, type, properties) {
    if (!(type in componentInfos)) {
        print("Unknown component type: " + type);
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

function registerComponent(name, clientCtor, serverCtor) {
    if (name in componentInfos) {
        print("Warning: registering known component type: " + name);
    }
    componentInfos[name] = new ComponentInfo(name, clientCtor, serverCtor);
};


// Base block class
Component = function(entityManager) {
    this.entityManager = entityManager;
    this.entityID = entityManager.entityID;
    this.isServer = entityManager.isServer;

    this.eventHandlers = {};
    for (var key in this) {
        if (key.indexOf('on' == 0)) {
            this.eventHandlers[key] = this[key];
        }
    }
};
Component.prototype = {
    init: function() {
        print("Initializing component " + this.type);
    },

    emit: function(event) {
        this.entityManager.emit.call(entityManager, arguments);
    },

    // Send an asynchronous message to the server
    async: function(fn) {
        print("Sending async to", fn);
        // Sending
        if (this.isServer) {
            // Call directly
        } else {
            // Send message to server actor
        }
    }
};

ButtonComponent = function(entityManager, properties) {
    Component.call(this, entityManager);

    //entityManager.on('update', this.onUpdate);
    // entityManager.on('click', this.onActivated);
    // entityManager.on('click', this.onActivated);

    this.clicked = false;
};
ButtonComponent.prototype = Object.create(Component.prototype);
extend(ButtonComponent.prototype, {
    type: "button",

    init: function() {
        this.entityManager.on('mouseDown', this.onActivated);
        this.entityManager.on('startDistanceGrab', this.onActivated);
    },

    // Event handlers
    onActivated: function(event) {
        print("Activated button!");
        this.emit('buttonActivated');
    },
    onUpdate: function(dt) {
    },

    // Public functionality
    toggleButton: function() {
        async(this.toggleButton_Server);
    },
    toggleButton_Server: function() {
        this.buttonDown = !this.buttonDown;
        emit('buttonClick', this.buttonDown);
    },
});

// Shooting range target
RangeTargetComponent = function(entityManager, properties) {
    Component.call(this, entityManager);

    this.clicked = false;
};
RangeTargetComponent.prototype = Object.create(Component.prototype);
extend(RangeTargetComponent.prototype, {
    type: "rangeTarget",

    init: function() {
    },

    // Event handlers
    onShot: function() {
    }
});

registerComponent("button", ButtonComponent);
registerComponent("rangeTarget", RangeTargetComponent);

/*
ShootingRange = function() {
};
ShootingRange.prototype = {
    startGame: function() {
        // Setup game, and beginning shooting targets
    },
    endGame: function() {
    }
};


function parseComponentData(data) {
    if (!data) {
        return null;
    }

    for (var key in data) {
        var args = data[key];
    }
}

componentData = {
    button: {
    },
};

 */
