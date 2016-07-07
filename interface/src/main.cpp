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

#include <gl/OpenGLVersionChecker.h>
#include <SharedUtil.h>

#include <DependencyManager.h>
#include <StatTracker.h>

#include "AddressManager.h"
#include "Application.h"
#include "InterfaceLogging.h"
#include "UserActivityLogger.h"
#include "MainWindow.h"
#include <thread>
#include <Trace.h>

#ifdef HAS_BUGSPLAT
#include <BuildInfo.h>
#include <BugSplat.h>
#include <CrashReporter.h>
#endif

int main(int argc, const char* argv[]) {
#if HAS_BUGSPLAT
    static QString BUG_SPLAT_DATABASE = "interface_alpha";
    static QString BUG_SPLAT_APPLICATION_NAME = "Interface";
    CrashReporter crashReporter { BUG_SPLAT_DATABASE, BUG_SPLAT_APPLICATION_NAME, BuildInfo::VERSION };
#endif

    Tracer::getInstance();

    disableQtBearerPoll(); // Fixes wifi ping spikes
    
    QString applicationName = "High Fidelity Interface - " + qgetenv("USERNAME");

    bool instanceMightBeRunning = true;

    QStringList arguments;
    for (int i = 0; i < argc; ++i) {
        arguments << argv[i];
    }

    QCommandLineParser parser;
    QCommandLineOption urlOption("url", "", "value");
    QCommandLineOption durationOption("duration", "Duration to run application");
    QCommandLineOption traceFilenameOption("trace", "Location to store trace file", "value", "F:\trace.json");
    QCommandLineOption killWhenDoneLoading("kill-when-done-loading", "", "value", "F:\trace.json");
    parser.addOption(urlOption);
    parser.addOption(durationOption);
    parser.addOption(traceFilenameOption);
    parser.addOption(killWhenDoneLoading);
    parser.addHelpOption();


#ifdef Q_OS_WIN
    // Try to create a shared memory block - if it can't be created, there is an instance of
    // interface already running. We only do this on Windows for now because of the potential
    // for crashed instances to leave behind shared memory instances on unix.
    QSharedMemory sharedMemory { applicationName };
    instanceMightBeRunning = !sharedMemory.create(1, QSharedMemory::ReadOnly);
#endif

    if (instanceMightBeRunning) {
        // Try to connect and send message to existing interface instance
        QLocalSocket socket;

        socket.connectToServer(applicationName);

        static const int LOCAL_SERVER_TIMEOUT_MS = 500;
        parser.process(arguments);

        // Try to connect - if we can't connect, interface has probably just gone down
        if (socket.waitForConnected(LOCAL_SERVER_TIMEOUT_MS)) {
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

    // Check OpenGL version.
    // This is done separately from the main Application so that start-up and shut-down logic within the main Application is
    // not made more complicated than it already is.
    bool override = false;
    QJsonObject glData;
    {
        OpenGLVersionChecker openGLVersionChecker(argc, const_cast<char**>(argv));
        bool valid = true;
        glData = openGLVersionChecker.checkVersion(valid, override);
        if (!valid) {
            if (override) {
                auto glVersion = glData["version"].toString();
                qCDebug(interfaceapp, "Running on insufficient OpenGL version: %s.", glVersion.toStdString().c_str());
            } else {
                qCDebug(interfaceapp, "Early exit due to OpenGL version.");
                return 0;
            }
        }
    }

    QElapsedTimer startupTime;
    startupTime.start();

    // Debug option to demonstrate that the client's local time does not
    // need to be in sync with any other network node. This forces clock
    // skew for the individual client
    const char* CLOCK_SKEW = "--clockSkew";
    const char* clockSkewOption = getCmdOption(argc, argv, CLOCK_SKEW);
    if (clockSkewOption) {
        qint64 clockSkew = atoll(clockSkewOption);
        usecTimestampNowForceClockSkew(clockSkew);
        qCDebug(interfaceapp) << "clockSkewOption=" << clockSkewOption << "clockSkew=" << clockSkew;
    }

    // Oculus initialization MUST PRECEDE OpenGL context creation.
    // The nature of the Application constructor means this has to be either here,
    // or in the main window ctor, before GL startup.
    Application::initPlugins(arguments);

    int exitCode;
    {
        QSettings::setDefaultFormat(QSettings::IniFormat);
        Application app(argc, const_cast<char**>(argv), startupTime);

        // If we failed the OpenGLVersion check, log it.
        if (override) {
            auto accountManager = DependencyManager::get<AccountManager>();
            if (accountManager->isLoggedIn()) {
                UserActivityLogger::getInstance().insufficientGLVersion(glData);
            } else {
                QObject::connect(accountManager.data(), &AccountManager::loginComplete, [glData](){
                    static bool loggedInsufficientGL = false;
                    if (!loggedInsufficientGL) {
                        UserActivityLogger::getInstance().insufficientGLVersion(glData);
                        loggedInsufficientGL = true;
                    }
                });
            }
        }

        // Setup local server
        QLocalServer server { &app };

        // We failed to connect to a local server, so we remove any existing servers.
        server.removeServer(applicationName);
        server.listen(applicationName);

        QObject::connect(&server, &QLocalServer::newConnection, &app, &Application::handleLocalServerConnection, Qt::DirectConnection);

#ifdef HAS_BUGSPLAT
        auto accountManager = DependencyManager::get<AccountManager>();
        crashReporter.mpSender.setDefaultUserName(qPrintable(accountManager->getAccountInfo().getUsername()));
        QObject::connect(accountManager.data(), &AccountManager::usernameChanged, &app, [&crashReporter](const QString& newUsername) {
            crashReporter.mpSender.setDefaultUserName(qPrintable(newUsername));
        });

        // BugSplat WILL NOT work with file paths that do not use OS native separators.
        auto logger = app.getLogger();
        auto logPath = QDir::toNativeSeparators(logger->getFilename());
        crashReporter.mpSender.sendAdditionalFile(qPrintable(logPath));

        QMetaObject::Connection connection;
        connection = QObject::connect(logger, &FileLogger::rollingLogFile, &app, [&crashReporter, &connection](QString newFilename) {
            // We only want to add the first rolled log file (the "beginning" of the log) to BugSplat to ensure we don't exceed the 2MB
            // zipped limit, so we disconnect here.
            QObject::disconnect(connection);
            auto rolledLogPath = QDir::toNativeSeparators(newFilename);
            crashReporter.mpSender.sendAdditionalFile(qPrintable(rolledLogPath));
        });
#endif
        QTimer timer;
        QString durationString = getCmdOption(argc, argv, "--duration");
        if (!durationString.isEmpty()) {
            bool ok;
            //QString durationString = "30000";// parser.value(durationOption);
            auto duration = durationString.toInt(&ok);
            if (ok) {
                timer.setSingleShot(true);
                timer.setInterval(duration);
                QObject::connect(&timer, &QTimer::timeout, qApp, []() {
                    qApp->quit();
                });
                timer.start();
            } else {
                qDebug() << "Unable to parse duration: " << durationString;
            }
        }

        //QString killWhenDoneLoading = getCmdOption(argc, argv, "--kill-when-done-loading");
//        QString killWhenDoneLoading = "on";
//        if (killWhenDoneLoading == "on") {
            QTimer checkTimer;
            checkTimer.setInterval(100);
            static int timesInactive = 0;
            static uint64_t inactiveTimestamp = 0;
            QObject::connect(&checkTimer, &QTimer::timeout, qApp, [&checkTimer]() {
                bool inactive = ResourceCache::getLoadingRequests().length() == 0
                    && ResourceCache::getPendingRequestCount() == 0
                    && DependencyManager::get<StatTracker>()->getStat("Processing") == 0
                    && DependencyManager::get<StatTracker>()->getStat("PendingProcessing") == 0;
                if (inactive) {
                    if (timesInactive == 0) {
                        inactiveTimestamp = usecTimestampNow();
                    }
                    timesInactive++;
                } else {
                    timesInactive = 0;
                }
                if (timesInactive >= 40) {
                    QObject::disconnect(&checkTimer, 0, qApp, 0);
                    Tracer::getInstance()->traceEvent("FinishedLoading", Instant, inactiveTimestamp,
                        QCoreApplication::applicationPid(), int64_t(QThread::currentThreadId()), "", "", {}, { { "s", "g" } });
                    qApp->quit();
                }
            });
            checkTimer.start();
//        }

        printSystemInformation();

        QTranslator translator;
        translator.load("i18n/interface_en");
        app.installTranslator(&translator);

        qCDebug(interfaceapp, "Created QT Application.");
        exitCode = app.exec();
        server.close();
    }

    Application::shutdownPlugins();

    //QString traceFilename = parser.value(traceFilenameOption);
    QString traceFilename = getCmdOption(argc, argv, "--trace");
    Tracer::getInstance()->setEnabled(false);
    Tracer::getInstance()->writeToFile(traceFilename);

    qCDebug(interfaceapp, "Normal exit.");
#if !defined(DEBUG) && !defined(Q_OS_LINUX)
    // HACK: exit immediately (don't handle shutdown callbacks) for Release build
    _exit(exitCode);
#endif
    return exitCode;
}
