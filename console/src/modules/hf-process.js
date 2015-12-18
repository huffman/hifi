'use strict'

var extend = require('extend');
var util = require('util');
var events = require('events');
var childProcess = require('child_process');
var fs = require('fs');

const ProcessGroupStates = {
    STOPPED: 'stopped',
    STARTED: 'started',
    STOPPING: 'stopping'
};

const ProcessStates = {
    STOPPED: 'stopped',
    STARTED: 'started',
    STOPPING: 'stopping'
};

function ProcessGroup(name, processes) {
    events.EventEmitter.call(this);

    this.name = name;
    this.state = ProcessGroupStates.STOPPED;
    this.processes = [];

    for (let process of processes) {
        this.addProcess(process);
    }
};
util.inherits(ProcessGroup, events.EventEmitter);
ProcessGroup.prototype = extend(ProcessGroup.prototype, {
    addProcess: function(process) {
        this.processes.push(process);
        process.on('state-update', this.onProcessStateUpdate.bind(this));
    },
    start: function() {
        if (this.state != ProcessGroupStates.STOPPED) {
            console.warn("Can't start process group that is not stopped.");
            return;
        }

        for (let process of this.processes) {
            process.start();
        }

        this.state = ProcessGroupStates.STARTED;
    },
    stop: function() {
        if (this.state != ProcessGroupStates.STARTED) {
            console.warn("Can't stop process group that is not started.");
            return;
        }
        for (let process of this.processes) {
            process.stop();
        }
        this.state = ProcessGroupStates.STOPPING;
    },

    // Event handlers
    onProcessStateUpdate: function(process) {
        var processesStillRunning = false;
        for (let process of this.processes) {
            if (process.state != ProcessStates.STOPPED) {
                processesStillRunning = true;
                break;
            }
        }
        if (!processesStillRunning) {
            this.state = ProcessGroupStates.STOPPED;
        }
        this.emit('state-update', this, process);
    }
});

var ID = 0;
function Process(name, command, commandArgs, logDirectory) {
    events.EventEmitter.call(this);

    this.id = ++ID;
    this.name = name;
    this.command = command;
    this.commandArgs = commandArgs ? commandArgs : [];
    this.child = null;
    this.logDirectory = logDirectory;
    this.logStdout = null;
    this.logStderr = null;

    this.state = ProcessStates.STOPPED;
};
util.inherits(Process, events.EventEmitter);
Process.prototype = extend(Process.prototype, {
    start: function() {
        if (this.state != ProcessStates.STOPPED) {
            console.warn("Can't start process that is not stopped.");
            return;
        }
        console.log("Starting " + this.command);

        var logStdout = 'ignore',
            logStderr = 'ignore';

        if (this.logDirectory) {
            var logDirectoryCreated = false;

            try {
                fs.mkdirSync('logs');
                logDirectoryCreated = true;
            } catch (e) {
                if (e.code == 'EEXIST') {
                    logDirectoryCreated = true;
                } else {
                    console.error("Error creating log directory");
                }
            }

            if (logDirectoryCreated) {
                // Create a temporary file with the current time
                var time = (new Date).getTime();
                var tmpLogStdout = this.logDirectory + '/' + this.name + '-' + time + '-stdout.txt';
                var tmpLogStderr = this.logDirectory + '/' + this.name + '-' + time + '-stderr.txt';

                try {
                    logStdout = fs.openSync(tmpLogStdout, 'ax');
                } catch(e) {
                    console.log("Error creating stdout log file", e);
                    logStdout = 'ignore';
                }
                try {
                    logStderr = fs.openSync(tmpLogStderr, 'ax');
                } catch(e) {
                    console.log("Error creating stderr log file", e);
                    logStderr = 'ignore';
                }
            }
        }

        try {
            this.child = childProcess.spawn(this.command, this.commandArgs, {
                detached: false,
                stdio: ['ignore', logStdout, logStderr]
            });
        } catch (e) {
            console.log("Got error starting child process for " + this.name, e);
            this.child = null;
            this.state = ProcessStates.STOPPED;
        }

        if (logStdout != 'ignore') {
            var pidLogStdout = './logs/' + this.name + "-" + this.child.pid + "-" + time + "-stdout.txt";
            fs.rename(tmpLogStdout, pidLogStdout, function(e) {
                if (e !== null) {
                    console.log("Error renaming log file from " + tmpLogStdout + " to " + pidLogStdout, e);
                }
            });
            this.logStdout = pidLogStdout;
        }

        if (logStderr != 'ignore') {
            var pidLogStderr = './logs/' + this.name + "-" + this.child.pid + "-" + time + "-stderr.txt";
            fs.rename(tmpLogStderr, pidLogStderr, function(e) {
                if (e !== null) {
                    console.log("Error renaming log file from " + tmpLogStdout + " to " + pidLogStdout, e);
                }
            });
            this.logStderr = pidLogStderr;
        }

        this.child.on('error', this.onChildStartError.bind(this));
        this.child.on('close', this.onChildClose.bind(this));
        this.state = ProcessStates.STARTED;
        console.log("Child process started");

        this.emit('state-update', this);
    },
    stop: function() {
        if (this.state != ProcessStates.STARTED) {
            console.warn("Can't stop process that is not started.");
            return;
        }
        this.child.kill();
        this.state = ProcessStates.STOPPING;
    },

    // Events
    onChildStartError: function(error) {
        console.log("Child process error ", error);
        this.state = ProcessStates.STOPPED;
        this.emit('state-update', this);
    },
    onChildClose: function(code) {
        console.log("Child process closed with code ", code);
        this.state = ProcessStates.STOPPED;
        this.emit('state-update', this);
    }
});

module.exports.Process = Process;
module.exports.ProcessGroup = ProcessGroup;
module.exports.ProcessStates = ProcessStates;
