console = {
    log: function() {
        console._message('[DEBUG]', arguments);
    },
    warn: function() {
        console._message('[WARNING]', arguments);
    },
    error: function() {
        console._message('[ERROR]', arguments);
    },
    _message: function(prefix, args) {
        args = Array.prototype.slice.call(args);
        args.unshift(prefix);
        print.apply(null, args);
    }
};
