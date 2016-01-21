require = function(url) {
    exports = {};
    Script.include(url);
    return exports;
};
