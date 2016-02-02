//
//  main.cpp
//  interface/src
//
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QLocalSocket>
#include <QLocalServer>
#include <QSettings>
#include <QSharedMemory>
#include <QTranslator>

#include <SharedUtil.h>

#include "AddressManager.h"
#include "Application.h"
#include "InterfaceLogging.h"
#include "MainWindow.h"

#include <BuildInfo.h>

#include <winsock2.h>
#include <CrashRpt.h>


// Define the callback function that will be called on crash
int CALLBACK CrashCallback(CR_CRASH_CALLBACK_INFO* pInfo)
{
    // The application has crashed!
    qDebug() << "Application has crashed!";
    // Return CR_CB_DODEFAULT to generate error report
    return CR_CB_DODEFAULT;
}

int main(int argc, const char* argv[]) {
    QString applicationName = "High Fidelity Interface";
    // Setup crash reporting via CrashRpt
    CR_INSTALL_INFO info;
    memset(&info, 0, sizeof(CR_INSTALL_INFO));
    info.cb = sizeof(CR_INSTALL_INFO);
    info.pszAppName = applicationName.toUtf8();
    info.pszAppVersion = BuildInfo::VERSION.toUtf8();
    info.pszUrl = "http://crashes.highfidelity.io/index.php/crashReport/uploadExternal";
    info.uPriorities[CR_HTTP] = 1;  // First try send report over HTTP 
    info.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS;
    info.dwFlags |= CR_INST_AUTO_THREAD_HANDLERS;
    // Restart the app on crash 
    //info.dwFlags |= CR_INST_APP_RESTART;
    //info.dwFlags |= CR_INST_SEND_QUEUED_REPORTS;
    //info.pszRestartCmdLine = _T("/restart");

    // Define the Privacy Policy URL 
    info.pszPrivacyPolicyURL = "http://highfidelity.com/privacypolicy.html";

    // Install crash reporting
    int nResult = crInstall(&info);
    if (nResult != 0)
    {
        // Something goes wrong. Get error message.
        char errorMsg[512] = "";
        crGetLastErrorMsg(errorMsg, 512);
        qDebug() << "Error! " << QString(errorMsg);
        return 1;
    }

    // Set crash callback function
    crSetCrashCallback(CrashCallback, NULL);

    disableQtBearerPoll(); // Fixes wifi ping spikes
    
    QString applicationNameWithUsername = applicationName + qgetenv("USERNAME");

    bool instanceMightBeRunning = true;

#ifdef Q_OS_WIN
    // Try to create a shared memory block - if it can't be created, there is an instance of
    // interface already running. We only do this on Windows for now because of the potential
    // for crashed instances to leave behind shared memory instances on unix.
    QSharedMemory sharedMemory { applicationNameWithUsername };
    instanceMightBeRunning = !sharedMemory.create(1, QSharedMemory::ReadOnly);
#endif

    if (instanceMightBeRunning) {
        // Try to connect and send message to existing interface instance
        QLocalSocket socket;

        socket.connectToServer(applicationNameWithUsername);

        static const int LOCAL_SERVER_TIMEOUT_MS = 500;

        // Try to connect - if we can't connect, interface has probably just gone down
        if (socket.waitForConnected(LOCAL_SERVER_TIMEOUT_MS)) {

            QStringList arguments;
            for (int i = 0; i < argc; ++i) {
                arguments << argv[i];
            }

            QCommandLineParser parser;
            QCommandLineOption urlOption("url", "", "value");
            parser.addOption(urlOption);
            parser.process(arguments);

            if (parser.isSet(urlOption)) {
                QUrl url = QUrl(parser.value(urlOption));
                if (url.isValid() && url.scheme() == HIFI_URL_SCHEME) {
                    qDebug() << "Writing URL to local socket";
                    socket.write(url.toString().toUtf8());
                    if (!socket.waitForBytesWritten(5000)) {
                        qDebug() << "Error writing URL to local socket";
                    }
                }
            }

            socket.close();

            qDebug() << "Interface instance appears to be running, exiting";

            return EXIT_SUCCESS;
        }

#ifdef Q_OS_WIN
        return EXIT_SUCCESS;
#endif
    }

    QElapsedTimer startupTime;
    startupTime.start();

    // Debug option to demonstrate that the client's local time does not
    // need to be in sync with any other network node. This forces clock
    // skew for the individual client
    const char* CLOCK_SKEW = "--clockSkew";
    const char* clockSkewOption = getCmdOption(argc, argv, CLOCK_SKEW);
    if (clockSkewOption) {
        int clockSkew = atoi(clockSkewOption);
        usecTimestampNowForceClockSkew(clockSkew);
        qCDebug(interfaceapp, "clockSkewOption=%s clockSkew=%d", clockSkewOption, clockSkew);
    }
    // Oculus initialization MUST PRECEDE OpenGL context creation.
    // The nature of the Application constructor means this has to be either here,
    // or in the main window ctor, before GL startup.
    Application::initPlugins();

    int exitCode;
    {
        QSettings::setDefaultFormat(QSettings::IniFormat);
        Application app(argc, const_cast<char**>(argv), startupTime);

        // Setup local server
        QLocalServer server { &app };

        // We failed to connect to a local server, so we remove any existing servers.
        server.removeServer(applicationNameWithUsername);
        server.listen(applicationNameWithUsername);

        QObject::connect(&server, &QLocalServer::newConnection, &app, &Application::handleLocalServerConnection);

        QTranslator translator;
        translator.load("i18n/interface_en");
        app.installTranslator(&translator);

        qCDebug(interfaceapp, "Created QT Application.");
        exitCode = app.exec();
        server.close();
    }

    Application::shutdownPlugins();

    qCDebug(interfaceapp, "Normal exit.");

    // Shutdown CrashRpt
    crUninstall();

    return exitCode;
}
