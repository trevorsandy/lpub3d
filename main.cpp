/****************************************************************************
**
** Copyright (C) 2004-2009 Kevin Clague. All rights reserved.
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

#include <QApplication>

#include "version.h"
#include "lpub_preferences.h"
#include "lpub.h"
#include "resolution.h"
#include "updatedialog.h"
#include <QMessageBox>

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
        lstrcpy(message, TEXT("LPubV just crashed. Crash information was saved to the file: \n'"));
        lstrcat(message, minidumpPath);
        lstrcat(message, TEXT("'\n Please send it to the developers for debugging."));

        MessageBox(NULL, message, TEXT("LPubV"), MB_OK);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

static void lcSehInit()
{
    if (SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, minidumpPath) == S_OK)
    {
        lstrcat(minidumpPath, TEXT("\\LPub Software\\LPubV\\dump\\"));
        _tmkdir(minidumpPath);
        lstrcat(minidumpPath, TEXT("minidump.dmp"));
    }

    SetUnhandledExceptionFilter(lcSehHandler);
}

#endif

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(lpub);

    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(VER_COMPANYNAME_STR);
    QCoreApplication::setApplicationName(VER_PRODUCTNAME_STR);
    QCoreApplication::setApplicationVersion(VER_TEXT);

    QTranslator Translator;
    Translator.load(QString("lpub_") + QLocale::system().name().section('_', 0, 0) + ".qm", ":/lc_lib/resources");
    app.installTranslator(&Translator);

    Preferences::lpubPreferences();
    Preferences::ldrawPreferences(false);
    Preferences::unitsPreferences();
    Preferences::leocadLibPreferences(false);
    Preferences::annotationPreferences();
    Preferences::fadestepPreferences();
    Preferences::viewerPreferences();

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
    const char* libPath = LC_INSTALL_PREFIX "/share/lpubv/";
#endif

#ifdef LC_LDRAW_LIBRARY_PATH
    const char* LDrawPath = LC_LDRAW_LIBRARY_PATH;
#else
    const char* LDrawPath = NULL;
#endif

//    QMessageBox::information(NULL,QMessageBox::tr("LPubV"),QMessageBox::tr("Startup"));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QStringList cachePathList = QStandardPaths::standardLocations(QStandardPaths::CacheLocation);
    QString cachePath = cachePathList.first();
#else
    QString cachePath = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
#endif

    QDir dir;
    dir.mkpath(cachePath);

    if (!g_App->Initialize(argc, argv, libPath, LDrawPath, cachePath.toLocal8Bit().data()))
        return 1;

    Gui     LPubVApp;

    gMainWindow->SetColorIndex(lcGetColorIndex(4));
    gMainWindow->UpdateRecentFiles();

    LPubVApp.show();
    LPubVApp.sizeit();

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
