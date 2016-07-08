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

var build = path.resolve(__dirname, '../build')
var cacheDir = path.resolve(osHomeDir(), 'AppData/Local/High Fidelity/Interface/data8');
var interfaceBin = path.resolve(build, "interface/Release/interface.exe")
var dsBin = path.resolve(build, "domain-server/Release/domain-server.exe")
var acBin = path.resolve(build, "assignment-client/Release/assignment-client.exe")

console.log("interface: ", interfaceBin);
console.log("ds: ", dsBin);
console.log("ac: ", acBin);

function traceContentSet(contentSet, traceName, runtimeSeconds, interfaceParams, onFinish) {
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
            //stdio: ['pipe', 'pipe', 'pipe']
            //stdio: i == 3 ? [0,1,2] : ['ignore', 'ignore', 'ignore']
            //stdio: i == 3 ? [0,1,2] : ['pipe', 'pipe', 'pipe']
            stdio: ['ignore', 'ignore', 'ignore']
        });
        acs.push(ac);
    }

    setTimeout(function() {
        function finish() {
            console.log("DONE");
            ds.kill();
            acs.forEach(function(ac) {
                ac.kill();
            });
            onFinish();
        }

        console.log("\tStarting interface");
        //var traceFile = 'F:\\trace_' + traceName + '.json';
        var traceFile = path.resolve(__dirname, 'trace', 'trace_' + traceName + '.json');
        console.log("Trace file: ", traceFile);
        var interfaceFlags = [
            '--url', 'hifi://localhost/',
            '--trace', traceFile,
            '--duration', runtimeSeconds,
            '--suppress-settings-reset',
            ///'--kill-when-done-loading', 'on'
        ].concat(interfaceParams);
        var hifi = childProcess.spawn(interfaceBin, interfaceFlags, {
            detached: false,
            stdio: ['ignore', 'ignore', 'ignore'],
            //stdio: ['pipe', 'pipe', 'pipe'],
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

function CommandContentSet(contentSet, traceName, params, runtimeSeconds) {
    return function(onFinish) {
        console.log("CommandContentSet");
        traceContentSet(contentSet, traceName, runtimeSeconds, params, onFinish);
    }
}

function CommandContentSetRemote(host, traceName, runtimeSeconds) {
    return function(onFinish) {
        console.log("CommandContentSet");
        traceContentSet(contentSet, traceName, runtimeSeconds, params, onFinish);
    }
}

var commands = [
    CommandClearCache(),
    CommandContentSet('playa', 'playa_nocache_3_2', ['--concurrent-downloads', '6', '--processing-threads', '2'], 90000),
    CommandContentSet('playa', 'playa_cache_3_2', ['--concurrent-downloads', '3', '--processing-threads', '2'], 90000),
    CommandClearCache(),
    CommandContentSet('single_atp_16_instances', 'single_atp_nocache', 30000),
    CommandContentSet('single_atp_16_instances', 'single_atp_cache', 30000),
];
/*
    for (var i = 0; i < 1; ++i) {
        commands.push(CommandClearCache());
        commands.push(CommandContentSet('home', 'home_' + i + '_nocache', 30000));
        commands.push(CommandContentSet('home', 'home_' + i + '_cache', 30000));
    }
    */

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
