'use strict'

var electron = require('electron');
var app = electron.app;  // Module to control application life.
var BrowserWindow = require('browser-window');  // Module to create native browser window.
var Menu = require('menu');
var Tray = require('tray');
var shell = require('shell');
var os = require('os');
var childProcess = require('child_process');
var path = require('path');
var fs = require('fs');
// var Tail = require('tail').Tail;
var Tail = require('always-tail');

var hfprocess = require('./modules/hf-process.js');
var Process = hfprocess.Process;
var ProcessGroup = hfprocess.ProcessGroup;

const ipcMain = electron.ipcMain;

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
var mainWindow = null;
var logWindow = null;
var appIcon = null;

var TRAY_ICON = path.join(__dirname, '../resources/console-tray.png');
var APP_ICON = path.join(__dirname, '../resources/console.png');

// Quit when all windows are closed.
app.on('window-all-closed', function() {
    // On OS X it is common for applications and their menu bar
    // to stay active until the user quits explicitly with Cmd + Q
    if (process.platform != 'darwin') {
        app.quit();
    }
});

// Check command line arguments to see how to find binaries
var argv = require('yargs').argv;
var pathFinder = require('./modules/path-finder.js');

var interfacePath = null;
var dsPath = null;
var acPath = null;

if (argv.localDebugBuilds || argv.localReleaseBuilds) {
    interfacePath = pathFinder.discoveredPath("Interface", argv.localReleaseBuilds);
    dsPath = pathFinder.discoveredPath("domain-server", argv.localReleaseBuilds);
    acPath = pathFinder.discoveredPath("assignment-client", argv.localReleaseBuilds);
}

function openFileBrowser(path) {
    var type = os.type();
    if (type == "Windows_NT") {
        childProcess.exec('start ' + path);
    } else if (type == "Darwin") {
        childProcess.exec('open ' + path);
    } else if (type == "Linux") {
        childProcess.exec('xdg-open ' + path);
    }
}

var logMonitors = {};

function ProcessLogMonitor(pid, path) {
    console.log(pid, path);
    this.pid = pid;
    this.path = path;

    this.data = "";

    var logTail = new Tail(path, '\n', { start: 0 });

    logTail.on('line', function(pid) {
        return function(data) {
            this.data += data;
            logWindow.webContents.send('log-message', { pid: pid, message: data });
        };
    }(pid).bind(this));
    logTail.on('error', function(error) {
        console.log("ERROR:", error);
    });
};



// if at this point any of the paths are null, we're missing something we wanted to find
// TODO: show an error for the binaries that couldn't be found

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
app.on('ready', function() {
    // Create tray icon
    appIcon = new Tray(TRAY_ICON);
    appIcon.setToolTip('High Fidelity Console');
    var contextMenu = Menu.buildFromTemplate([{
        label: 'Quit',
        accelerator: 'Command+Q',
        click: function() { app.quit(); }
    }]);
    appIcon.setContextMenu(contextMenu);

    var homeServer = null;
    // Create the browser window.
    mainWindow = new BrowserWindow({width: 800, height: 600, icon: APP_ICON});

    // and load the index.html of the app.
    mainWindow.loadURL('file://' + __dirname + '/index.html');

    // Open the DevTools.
    mainWindow.webContents.openDevTools();

    // Emitted when the window is closed.
    mainWindow.on('closed', function() {
        // Dereference the window object, usually you would store windows
        // in an array if your app supports multi windows, this is the time
        // when you should delete the corresponding element.
        mainWindow = null;
        if (homeServer) {
            homeServer.stop();
        }
    });

    // When a link is clicked that has `_target="_blank"`, open it in the user's native browser
    mainWindow.webContents.on('new-window', function(e, url) {
        e.preventDefault();
        shell.openExternal(url);
    });

    // Create the browser window.
    logWindow = new BrowserWindow({ width: 700, height: 500, icon: APP_ICON });
    logWindow.loadURL('file://' + __dirname + '/log.html');
    logWindow.webContents.openDevTools();
    logWindow.on('closed', function() {
        logWindow = null;
    });
    // When a link is clicked that has `_target="_blank"`, open it in the user's native browser
    logWindow.webContents.on('new-window', function(e, url) {
        e.preventDefault();
        shell.openExternal(url);
    });

    var logPath = path.join(app.getAppPath(), 'logs');

    var httpStatusPort = 60332;

    if (interfacePath && dsPath && acPath) {
        var pInterface = new Process('interface', interfacePath);

        var acMonitor = new Process('ac_monitor', acPath, ['-n1',
                                                           '--log-directory', logPath,
                                                           '--http-status-port', httpStatusPort], logPath);
        homeServer = new ProcessGroup('home', [
            new Process('domain_server', dsPath),
            acMonitor
        ]);
        homeServer.start();

        var processes = {
            interface: pInterface,
            home: homeServer
        };

        function sendProcessUpdate() {
            console.log("Sending process update to web view");
            mainWindow.webContents.send('process-update', processes);
        };

        logMonitors[acMonitor.child.pid] = new ProcessLogMonitor(acMonitor.child.pid, path.join(app.getAppPath(), acMonitor.logStdout));
        var http = require('http');
        var request = require('request');
        function checkAC() {
            console.log("Checking AC");

            var options = {
                url: "http://localhost:" + httpStatusPort + "/status",
                json: true
            };
            request(options, function(error, response, body) {
                if (error) {
                    console.log('ERROR', error);
                } else {
                    console.log("Response", body);
                    for (var pid in body.servers) {
                        if (!(pid in logMonitors)) {
                            console.log(pid);
                            var info = body.servers[pid];
                            console.log(info);
                            var path = info.logStdout;

                            logMonitors[pid] = new ProcessLogMonitor(pid, path);
                        }
                    }
                }
            });

            // setTimeout(checkAC, 5000);
        }
        setTimeout(checkAC, 1000);

        pInterface.on('state-update', sendProcessUpdate);
        homeServer.on('state-update', sendProcessUpdate);

        ipcMain.on('start-process', function(event, arg) {
            pInterface.start();
            sendProcessUpdate();
        });
        ipcMain.on('stop-process', function(event, arg) {
            pInterface.stop();
            sendProcessUpdate();
        });
        ipcMain.on('start-server', function(event, arg) {
            homeServer.start();
            sendProcessUpdate();
        });
        ipcMain.on('stop-server', function(event, arg) {
            homeServer.stop();
            sendProcessUpdate();
        });
        ipcMain.on('open-logs', function(event, arg) {
            openFileBrowser(logPath);
        });
        ipcMain.on('update', sendProcessUpdate);

        sendProcessUpdate();
    }
});
