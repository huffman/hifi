createComponent('planeGameManager', {
}, {
});

createComponent('planeGameBaton', {
    init: function() {
        this.on('triggerDown', this.onTriggerDown);
    },
    onTriggerDown: function() {
    }
}, {
});
