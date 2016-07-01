const childProcess = require('child_process');
const osHomeDir = require('os-homedir');
const path = require('path');
const process = require('process');
const rimraf = require('rimraf');

function debug() {
    return;
    console.log.apply(this, arguments);
}

var SERVER_SETUP_TIME = 5000;
var INTERFACE_DURATION = 30000;

var CLUMSY_PARAMS = [
    '--filter', 'outbound',
    '--drop', 'on',
    '--drop-chance', '2'
]

var build = "dev/hifi/build"
var cacheDir = path.resolve(osHomeDir(), 'AppData/Local/High Fidelity/Interface/data8');
var interfaceBin = path.resolve(osHomeDir(), build, "interface/Release/interface.exe")
var dsBin = path.resolve(osHomeDir(), build, "domain-server/Release/domain-server.exe")
var acBin = path.resolve(osHomeDir(), build, "assignment-client/Release/assignment-client.exe")

console.log("interface: ", interfaceBin);
console.log("ds: ", dsBin);
console.log("ac: ", acBin);

function traceContentSet(contentSet, traceName, runtimeSeconds, onFinish) {
    console.log("\tRunning content set: ", contentSet, traceName);

    var dsConfig = path.resolve(osHomeDir(), 'AppData/Roaming/High Fidelity - dev/content_sets/' + contentSet + '/domain-server/config.json');
    console.log("\tdsconfig: ", dsConfig);


    ds = childProcess.spawn(dsBin, ['--user-config', dsConfig], {
        detached: false,
        //stdio: ['ignore', 'ignore', 'ignore']
    });
    var types = [0, 1, 2, 3, 6]
    var acs = [];
    for (var i = 0; i < types.length; ++i) {
        var ac = childProcess.spawn(acBin, ['-t' + types[i]], {
            detached: false,
        });
        acs.push(ac);
    }

    setTimeout(function() {
        function finish() {
            ds.kill();
            acs.forEach(function(ac) {
                ac.kill();
            });
            onFinish();
        }

        console.log("\tStarting interface");
        var traceFile = 'F:\\trace_' + traceName + '.json';
        var interfaceFlags = [
            '--url', 'hifi://localhost/',
            '--trace', traceFile,
            '--duration', runtimeSeconds
        ]
        var hifi = childProcess.spawn(interfaceBin, interfaceFlags, {
            detached: false,
            stdio: ['ignore', 'ignore', 'ignore']
        });
        hifi.on('error', finish);
        hifi.on('close', finish);
    }, SERVER_SETUP_TIME);
}

function CommandClearCache() {
    return function(onFinish) {
        console.log("CommandClearCache");
        console.log("\tDeleting cache: ", cacheDir);
        rimraf(cacheDir, onFinish);
    }
}

function CommandStartClumsy() {
    return function(onFinish) {
    }
}

function CommandStopClumsy() {
    return function(onFinish) {
    }
}

function CommandContentSet(contentSet, traceName, runtimeSeconds) {
    return function(onFinish) {
        console.log("CommandContentSet");
        traceContentSet(contentSet, traceName, runtimeSeconds, onFinish);
    }
}

var commands = [
    //CommandClearCache(),
    //CommandContentSet('playa', 'playa_nocache', 90000),
    //CommandContentSet('playa', 'playa_cache', 90000),

    CommandClearCache(),
    CommandContentSet('home', 'home_nocache', 30000),
    //CommandContentSet('home', 'home_cache', 30000),
];

var curIndex = -1;
function runNextCommand() {
    //console.log("In runNextCommand");
    ++curIndex;
    if (curIndex >= commands.length) {
        process.exit();
    }
    debug("Pre runNextCommand");
    commands[curIndex](runNextCommand);
    debug("End runNextCommand");
};

runNextCommand();
