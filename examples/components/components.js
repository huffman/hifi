var Component;

Component = function() {
};
Component.prototype = {
    requiredParameters: [],
    optionalParameters: { },
    destroy: function() { },
    onActivate: function() { }
};


AudioComponent = function(entityID, _params) {
    Component.call(this);

    this.entityID = entityID;

    this.audioURL = data.audioURL;
};
AudioComponent.prototype = Object.create(Component.prototype);

AudioComponent.prototype.requiredParameters = ['audioURL'];
AudioComponent.prototype.optionalParameters = {
};

AudioComponent.prototype.destroy = function() {
};


IncludeComponent = function(_entityID, _params) {
    this.urls = data.urls;
};
IncludeComponent.prototype.requiredParameters = ['urls'];



COMPONENTS = {
    audio: AudioComponent,
    include: IncludeComponent,
};

(function() {
    var components = [];

    this.preload = function(entityID) {
        var properties = Entities.getEntityProperties(entityID);

        var data = {};
        try {
            data = JSON.parse(properties.userData);
        } catch {
        }

        for (var key in data) {
            var componentType = COMPONENTS[key];
            if (componentType !== undefined) {
                var component = componentType(entityID, data[key]);
                components.push(component);
            }
        }
    };

    this.unload = function() {
        for (var i = 0; i < components.length; ++i) {
            components[i].destroy();
        }
    };
});
