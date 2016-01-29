/****************************************************************************
**
** Copyright (C) 2004-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtWidgets/QApplication>
#include <QDir>
#include <iostream>
#include <QMessageBox>

#include "lpub_preferences.h"
#include "lpub.h"
#include "resolution.h"
#include "updatecheck.h"

//**3D
#include "lc_global.h"
#include "lc_application.h"
#include "lc_mainwindow.h"
#include "project.h"
#include "lc_colors.h"
//**

#ifdef Q_OS_WIN

#include <dbghelp.h>
#include <direct.h>
#include <shlobj.h>

#include "QsLogDest.h"
#include "QsLog.h"


#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif

#include <tchar.h>

QAction *actions[LC_NUM_COMMANDS];

static TCHAR minidumpPath[_MAX_PATH];

static LONG WINAPI lcSehHandler(PEXCEPTION_POINTERS exceptionPointers)
{
    if (IsDebuggerPresent())
        return EXCEPTION_CONTINUE_SEARCH;

    HMODULE dbgHelp = LoadLibrary(TEXT("dbghelp.dll"));

    if (dbgHelp == NULL)
        return EXCEPTION_EXECUTE_HANDLER;

    HANDLE file = CreateFile(minidumpPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file == INVALID_HANDLE_VALUE)
        return EXCEPTION_EXECUTE_HANDLER;

    typedef BOOL (WINAPI *LPMINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType, CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, CONST PMINIDUMP_USER_STREAM_INFORMATION UserEncoderParam, CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);
    LPMINIDUMPWRITEDUMP miniDumpWriteDump = (LPMINIDUMPWRITEDUMP)GetProcAddress(dbgHelp, "MiniDumpWriteDump");
    if (!miniDumpWriteDump)
        return EXCEPTION_EXECUTE_HANDLER;

    MINIDUMP_EXCEPTION_INFORMATION mei;

    mei.ThreadId = GetCurrentThreadId();
    mei.ExceptionPointers = exceptionPointers;
    mei.ClientPointers = TRUE;

    BOOL writeDump = miniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, MiniDumpNormal, exceptionPointers ? &mei : NULL, NULL, NULL);

    CloseHandle(file);
    FreeLibrary(dbgHelp);

    if (writeDump)
    {
        TCHAR message[_MAX_PATH + 256];
        lstrcpy(message, TEXT("LPub3D just crashed. Crash information was saved to the file: \n'"));
        lstrcat(message, minidumpPath);
        lstrcat(message, TEXT("'\n Please send it to the developers for debugging."));

        MessageBox(NULL, message, TEXT("LPub3D"), MB_OK);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

static void lcSehInit()
{
    if (SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, minidumpPath) == S_OK)
    {
        lstrcat(minidumpPath, TEXT("\\LPub3D Software\\LPub3D\\dump\\"));
        _tmkdir(minidumpPath);
        lstrcat(minidumpPath, TEXT("minidump.dmp"));
    }

    SetUnhandledExceptionFilter(lcSehHandler);
}

#endif

int main(int argc, char *argv[])
{
//    Q_INIT_RESOURCE(lpub);

    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseDesktopOpenGL);

    // splash
    QPixmap pixmap(":/resources/LPub512Splash.png");
    QSplashScreen *splash = new QSplashScreen(pixmap);

    QFont splashFont;
    splashFont.setFamily("Arial");
    splashFont.setPixelSize(14);
    splashFont.setStretch(130);

    splash->setFont(splashFont);
    splash->show();

    splash->showMessage(QSplashScreen::tr("LPub3D is loading..."),Qt::AlignBottom | Qt::AlignCenter, Qt::white);
    app.processEvents();

    QCoreApplication::setOrganizationName(VER_COMPANYNAME_STR);
    QCoreApplication::setApplicationName(VER_PRODUCTNAME_STR);
    QCoreApplication::setApplicationVersion(VER_PRODUCTVERSION_STR);

    Preferences::lpubPreferences();

    // init the logging mechanism
    QString lpubDataPath = Preferences::lpubDataPath;
    QDir logDir(lpubDataPath+"/logs");
    if(!QDir(logDir).exists())
        logDir.mkpath(".");

    const QString sLogPath(QDir(logDir).filePath(QString("%1%2").arg(VER_PRODUCTNAME_STR).arg("Log.txt")));

    QsLogging::Logger& logger = QsLogging::Logger::instance();

    // set minimum log level and file name
    logger.setLoggingLevel(QsLogging::TraceLevel);

    // Create log destinations
    QsLogging::DestinationPtr fileDestination(
       QsLogging::DestinationFactory::MakeFileDestination(sLogPath));
    QsLogging::DestinationPtr debugDestination(
       QsLogging::DestinationFactory::MakeDebugOutputDestination());

    // set log destinations on the logger
    logger.addDestination(debugDestination.get());
    logger.addDestination(fileDestination.get());

    // write an info message using one of six macros:
    bool showLogExamples = false;
    if (showLogExamples){
        logInfo()   << "LPub3D started";
        logInfo()   << "Built with Qt" << QT_VERSION_STR << "running on" << qVersion();
        logNotice() <<  "Here's a" << QString("Notice") << "message";
        logTrace()  << "Here's a" << QString("trace") << "message";
        logDebug()  << "Here's a" << static_cast<int>(QsLogging::DebugLevel) << "message";
        logWarn()   << "Uh-oh!";
        qDebug()    << "This message won't be picked up by the logger";
        logError()  << "An error has occurred";
        qWarning()  << "Neither will this one";
        logFatal()  << "Fatal error!";
    } // end init logging

    Preferences::ldrawPreferences(false);
    Preferences::unitsPreferences();

    Preferences::viewerLibPreferences(false);
    Preferences::annotationPreferences();

    Preferences::fadestepPreferences();
    Preferences::pliPreferences();
    Preferences::viewerPreferences();

    QTranslator Translator;
    Translator.load(QString("lpub_") + QLocale::system().name().section('_', 0, 0) + ".qm", ":../lc_lib/resources");
    app.installTranslator(&Translator);

    defaultResolutionType(Preferences::preferCentimeters);
    setResolution(150);  // DPI

    g_App = new lcApplication();

#if defined(Q_OS_WIN)
    char libPath[LC_MAXPATH], *ptr;
    strcpy(libPath, argv[0]);
    ptr = strrchr(libPath,'\\');
    if (ptr)
        *(++ptr) = 0;

    lcSehInit();
#elif defined(Q_OS_MAC)
    QDir bundlePath = QDir(QCoreApplication::applicationDirPath());
    bundlePath.cdUp();
    bundlePath.cdUp();
    bundlePath = QDir::cleanPath(bundlePath.absolutePath() + "/Contents/lc_lib/Resources/");
    QByteArray pathArray = bundlePath.absolutePath().toLocal8Bit();
    const char* libPath = pathArray.data();
#else
    const char* libPath = LC_INSTALL_PREFIX "/share/lpub3d/";
#endif

#ifdef LC_LDRAW_LIBRARY_PATH
    const char* LDrawPath = LC_LDRAW_LIBRARY_PATH;
#else
    const char* LDrawPath = NULL;
#endif

    if (!g_App->Initialize(argc, argv, libPath, LDrawPath))
        return 1;

    Gui     LPub3DApp;

    gMainWindow->SetColorIndex(lcGetColorIndex(4));
    gMainWindow->UpdateRecentFiles();

    splash->finish(&LPub3DApp);
    app.processEvents();

    LPub3DApp.show();

    LPub3DApp.sizeit();

#if !DISABLE_UPDATE_CHECK
    DoInitialUpdateCheck();
#endif

    int execReturn = app.exec();

    delete gMainWindow;
    gMainWindow = NULL;
    delete g_App;
    g_App = NULL;

    return execReturn;
}
