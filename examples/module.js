if (this.require === undefined) {
    this.module = null, this.exports = null;
    this.require = function(url) {
        module = {
            exports: {}
        };
        exports = module.exports;
        Script.include(url);
        return exports;
    };
}
