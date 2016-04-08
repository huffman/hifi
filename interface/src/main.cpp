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

#include "AddressManager.h"
#include "Application.h"
#include "InterfaceLogging.h"
#include "UserActivityLogger.h"
#include "MainWindow.h"
#include <thread>

#ifdef HAS_BUGSPLAT
#include <BuildInfo.h>
#include <BugSplat.h>
#endif

// Patch for SetUnhandledExceptionFilter 
const BYTE PatchBytes[5] = { 0x33, 0xC0, 0xC2, 0x04, 0x00 };

// Original bytes at the beginning of SetUnhandledExceptionFilter 
BYTE OriginalBytes[5] = { 0 };




////////////////////////////////////////////////////////////////////////////////
// WriteMemory function 
// 

bool WriteMemory(BYTE* pTarget, const BYTE* pSource, DWORD Size)
{
    DWORD ErrCode = 0;


    // Check parameters 

    if (pTarget == 0)
    {
        _ASSERTE(!"Target address is null.");
        return false;
    }

    if (pSource == 0)
    {
        _ASSERTE(!"Source address is null.");
        return false;
    }

    if (Size == 0)
    {
        _ASSERTE(!("Source size is null."));
        return false;
    }

    if (IsBadReadPtr(pSource, Size))
    {
        _ASSERTE(!("Source is unreadable."));
        return false;
    }


    // Modify protection attributes of the target memory page 

    DWORD OldProtect = 0;

    if (!VirtualProtect(pTarget, Size, PAGE_EXECUTE_READWRITE, &OldProtect))
    {
        ErrCode = GetLastError();
        _ASSERTE(!("VirtualProtect() failed."));
        return false;
    }


    // Write memory 

    memcpy(pTarget, pSource, Size);


    // Restore memory protection attributes of the target memory page 

    DWORD Temp = 0;

    if (!VirtualProtect(pTarget, Size, OldProtect, &Temp))
    {
        ErrCode = GetLastError();
        _ASSERTE(!("VirtualProtect() failed."));
        return false;
    }


    // Success 

    return true;

}

//
//
bool EnforceFilter(bool bEnforce)
{
    DWORD ErrCode = 0;


    // Obtain the address of SetUnhandledExceptionFilter 

//    HMODULE hLib = GetModuleHandle(("kernel32.dll"));
    HMODULE hLib = GetModuleHandle(("msvcrt.dll"));

    if (hLib == NULL)
    {
        ErrCode = GetLastError();
        _ASSERTE(!("GetModuleHandle(kernel32.dll) failed."));
        return false;
    }

//    BYTE* pTarget = (BYTE*)GetProcAddress(hLib, "SetUnhandledExceptionFilter");
    BYTE* pTarget = (BYTE*)GetProcAddress(hLib, "_set_purecall_handler");

    if (pTarget == 0)
    {
        ErrCode = GetLastError();
        _ASSERTE(!("GetProcAddress(SetUnhandledExceptionFilter) failed."));
        return false;
    }

    if (IsBadReadPtr(pTarget, sizeof(OriginalBytes)))
    {
        _ASSERTE(!("Target is unreadable."));
        return false;
    }


    if (bEnforce)
    {
        // Save the original contents of SetUnhandledExceptionFilter 

        memcpy(OriginalBytes, pTarget, sizeof(OriginalBytes));


        // Patch SetUnhandledExceptionFilter 

        if (!WriteMemory(pTarget, PatchBytes, sizeof(PatchBytes)))
            return false;

    }
    else
    {
        // Restore the original behavior of SetUnhandledExceptionFilter 

        if (!WriteMemory(pTarget, OriginalBytes, sizeof(OriginalBytes)))
            return false;

    }


    // Success 

    return true;

}




LPTOP_LEVEL_EXCEPTION_FILTER WINAPI
MyDummySetUnhandledExceptionFilter(
    LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
    return NULL;
}


BOOL redirectLibraryFunctionToNOOP(char* library, char* function)
{
//    HMODULE hKernel32 = LoadLibrary("msvcr120.dll");
    HMODULE hKernel32 = LoadLibrary(library);
    if (hKernel32 == NULL) return FALSE;
    void *pOrgEntry = GetProcAddress(hKernel32, function);
    if (pOrgEntry == NULL) return FALSE;

    DWORD dwOldProtect = 0;
    SIZE_T jmpSize = 5;
#ifdef _M_X64
    jmpSize = 13;
#endif
    BOOL bProt = VirtualProtect(pOrgEntry, jmpSize,
        PAGE_EXECUTE_READWRITE, &dwOldProtect);
    BYTE newJump[20];
    void *pNewFunc = &MyDummySetUnhandledExceptionFilter;
#ifdef _M_IX86
    DWORD dwOrgEntryAddr = (DWORD)pOrgEntry;
    dwOrgEntryAddr += jmpSize; // add 5 for 5 op-codes for jmp rel32
    DWORD dwNewEntryAddr = (DWORD)pNewFunc;
    DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;
    // JMP rel32: Jump near, relative, displacement relative to next instruction.
    newJump[0] = 0xE9;  // JMP rel32
    memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));
#elif _M_X64
    // We must use R10 or R11, because these are "scratch" registers 
    // which need not to be preserved accross function calls
    // For more info see: Register Usage for x64 64-Bit
    // http://msdn.microsoft.com/en-us/library/ms794547.aspx
    // Thanks to Matthew Smith!!!
    newJump[0] = 0x49;  // MOV R11, ...
    newJump[1] = 0xBB;  // ...
    memcpy(&newJump[2], &pNewFunc, sizeof(pNewFunc));
    //pCur += sizeof (ULONG_PTR);
    newJump[10] = 0x41;  // JMP R11, ...
    newJump[11] = 0xFF;  // ...
    newJump[12] = 0xE3;  // ...
#endif
    SIZE_T bytesWritten;
    BOOL bRet = WriteProcessMemory(GetCurrentProcess(),
        pOrgEntry, newJump, jmpSize, &bytesWritten);

    if (bProt != FALSE)
    {
        DWORD dwBuf;
        VirtualProtect(pOrgEntry, jmpSize, dwOldProtect, &dwBuf);
    }
    return bRet;
}






#if HAS_BUGSPLAT
    // Prevent other threads from hijacking the Exception filter, and allocate 4MB up-front that may be useful in
    // low-memory scenarios.
static const DWORD BUG_SPLAT_FLAGS = MDSF_PREVENTHIJACKING | MDSF_USEGUARDMEMORY;// | MDSF_CUSTOMEXCEPTIONFILTER;
    static const char* BUG_SPLAT_DATABASE = "interface_alpha";
    static const char* BUG_SPLAT_APPLICATION_NAME = "Interface";
    static MiniDmpSender mpSender { BUG_SPLAT_DATABASE, BUG_SPLAT_APPLICATION_NAME, qPrintable(BuildInfo::VERSION),
                                    nullptr, BUG_SPLAT_FLAGS };
#endif


#include <csignal>

#include <limits>
namespace crash {

    void pureVirtualCall() {
        struct B {
            B() {
                qDebug() << "Pure Virtual Function Call crash!";
                Bar();
            }

            virtual void Foo() = 0;

            void Bar() {
                Foo();
            }
        };

        struct D : public B {
            void Foo() {
                qDebug() << "D:Foo()";
            }
        };

        B* b = new D;
        qDebug() << "About to make a pure virtual call";
        b->Foo();
    }
        
    void doubleFree() {
        qDebug() << "About to double delete memory";
        int* blah = new int(200);
        delete blah;
        delete blah;
//        free(blah);
//        free(blah);
//        free(blah);
//        free(blah);
//        free(blah);
    }

    void nullDeref() {
        qDebug() << "About to dereference a null pointer";
        int* p = nullptr;
        *p = 1;
    }

    void doAbort() {
        qDebug() << "About to abort";
        abort();
    }

    void outOfBoundsVectorCrash() {
        qDebug() << "std::vector out of bounds crash!";
        std::vector<int> v;
        v[0] = 5;
    }

    void newFault() {
        qDebug() << "About to crash inside new fault";
        // Force crash with large allocation
        int *pi = new int[std::numeric_limits<uint64_t>::max()];
    }
}
void handleSignal(int signal) {
    qDebug() << "Got signal: " << signal;
    throw(signal);
}

void handlePureVirtualCall() {
    qDebug() << "In handlePureVirtualcall";
    throw("Pure Virtual Call");
}

LONG WINAPI TopLevelExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo) {
    qDebug() << "Got exception";

    mpSender.unhandledExceptionHandler(pExceptionInfo);

    return EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI
VectoredHandler(
struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    std::cout << "In vectoredHandelr";

    mpSender.unhandledExceptionHandler(nullptr);
    return EXCEPTION_CONTINUE_SEARCH;
}

int main(int argc, const char* argv[]) {
    signal(SIGSEGV, handleSignal);
    signal(SIGABRT, handleSignal);
    _set_purecall_handler(handlePureVirtualCall);

    // Disable SetUnhandledExceptionFilter from being called by other libraries (CRT in particular)
    bool s = redirectLibraryFunctionToNOOP("msvcr120.dll", "_set_purecall_handler");
    qDebug() << "SUCCESS " << s;
    s = redirectLibraryFunctionToNOOP("kernel32.dll", "SetUnhandledExceptionFilter");
    qDebug() << "SUCCESS " << s;

    disableQtBearerPoll(); // Fixes wifi ping spikes
    
    QString applicationName = "High Fidelity Interface - " + qgetenv("USERNAME");

    bool instanceMightBeRunning = true;

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

    // Check OpenGL version.
    // This is done separately from the main Application so that start-up and shut-down logic within the main Application is
    // not made more complicated than it already is.
    bool override = false;
    QString glVersion;
    {
        OpenGLVersionChecker openGLVersionChecker(argc, const_cast<char**>(argv));
        bool valid = true;
        glVersion = openGLVersionChecker.checkVersion(valid, override);
        if (!valid) {
            if (override) {
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

        // If we failed the OpenGLVersion check, log it.
        if (override) {
            auto& accountManager = AccountManager::getInstance();
            if (accountManager.isLoggedIn()) {
                UserActivityLogger::getInstance().insufficientGLVersion(glVersion);
            } else {
                QObject::connect(&AccountManager::getInstance(), &AccountManager::loginComplete, [glVersion](){
                    static bool loggedInsufficientGL = false;
                    if (!loggedInsufficientGL) {
                        UserActivityLogger::getInstance().insufficientGLVersion(glVersion);
                        loggedInsufficientGL = true;
                    }
                });
            }
        }




        // Disable WER popup
        SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
        _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

        // Error handlers
//        SetUnhandledExceptionFilter(TopLevelExceptionHandler);
        //AddVectoredExceptionHandler(0, VectoredHandler);
//        _set_purecall_handler(handlePureVirtualCall);

//        crash::doAbort(); // works
//        crash::outOfBoundsVectorCrash(); //works
//        crash::newFault(); // works
//        crash::pureVirtualCall(); // works with handler
        crash::doubleFree();

//        std::thread([]() {
////            _set_purecall_handler(handlePureVirtualCall);
////            crash::pureVirtualCall();
////        crash::outOfBoundsVectorCrash(); //works
//        });

//        crash::pureVirtualCall();

//        QThread thread;
//        QObject::connect(&thread, &QThread::started, []() {
//            crash::doAbort(); // works
//        });
//        thread.start();
//        crash::doubleFree();

        // Setup local server
        QLocalServer server { &app };

        // We failed to connect to a local server, so we remove any existing servers.
        server.removeServer(applicationName);
        server.listen(applicationName);

        QObject::connect(&server, &QLocalServer::newConnection, &app, &Application::handleLocalServerConnection, Qt::DirectConnection);

#ifdef HAS_BUGSPLAT
        AccountManager& accountManager = AccountManager::getInstance();
        mpSender.setDefaultUserName(qPrintable(accountManager.getAccountInfo().getUsername()));
        QObject::connect(&accountManager, &AccountManager::usernameChanged, &app, [](const QString& newUsername) {
            mpSender.setDefaultUserName(qPrintable(newUsername));
        });

        // BugSplat WILL NOT work with file paths that do not use OS native separators.
        auto logPath = QDir::toNativeSeparators(app.getLogger()->getFilename());
        mpSender.sendAdditionalFile(qPrintable(logPath));
#endif

        printSystemInformation();

        QTranslator translator;
        translator.load("i18n/interface_en");
        app.installTranslator(&translator);

        qCDebug(interfaceapp, "Created QT Application.");
        exitCode = app.exec();
        server.close();
    }

    Application::shutdownPlugins();

    qCDebug(interfaceapp, "Normal exit.");
#ifndef DEBUG
    // HACK: exit immediately (don't handle shutdown callbacks) for Release build
    _exit(exitCode);
#endif
    return exitCode;
}
