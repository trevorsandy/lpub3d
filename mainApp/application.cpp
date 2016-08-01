/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "application.h"

#include <QDir>
#include <iostream>
#include <QMessageBox>

#include "lpub_preferences.h"
#include "lpub.h"
#include "resolution.h"
#include "updatecheck.h"

#include "QsLog.h"
#include "QsLogDest.h"

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

// Initializes the Application instance as null
Application* Application::m_instance = NULL;

Application::Application(int &argc, char **argv)
  : m_application(argc, argv)
{
  m_instance = this;

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
  m_application.setAttribute(Qt::AA_UseDesktopOpenGL);
#endif

  QCoreApplication::setOrganizationName(VER_COMPANYNAME_STR);
  QCoreApplication::setApplicationName(VER_PRODUCTNAME_STR);
  QCoreApplication::setApplicationVersion(VER_PRODUCTVERSION_STR);

  connect(this, SIGNAL(splashMsgSig(QString)), this, SLOT(splashMsg(QString)));

}

Application* Application::instance()
{
  return m_instance;
}

void Application::initialize(int &argc, char **argv)
{
  // splash
  QPixmap pixmap(":/resources/LPub512Splash.png");
  splash = new QSplashScreen(pixmap);

  QFont splashFont;
  splashFont.setFamily("Arial");
  splashFont.setPixelSize(16);
  splashFont.setStretch(130);

  splash->setFont(splashFont);
  splash->show();

  logInfo() << QString("Initializing application.");

  emit splashMsgSig("5% - Initializing application...");

  Preferences::lpubPreferences();

  // initialize the logger
  using namespace QsLogging;

  Logger& logger = Logger::instance();

  // set default log options
  logger.setLoggingLevel(TraceLevel);
  logger.setIncludeLogLevel(true);
  logger.setIncludeTimestamp(false);
  logger.setIncludeLineNumber(true);
  logger.setIncludeFileName(true);
  logger.setColorizeFunctionInfo(true); //default should be false
  logger.setColorizeOutput(true);       //default should be false
  // define log path
  QString lpubDataPath = Preferences::lpubDataPath;
  QDir logDir(lpubDataPath+"/logs");
  if(!QDir(logDir).exists())
    logDir.mkpath(".");
  const QString sLogPath(QDir(logDir).filePath(QString("%1%2").arg(VER_PRODUCTNAME_STR).arg("Log.txt")));

  // Create log destinations
  DestinationPtr fileDestination(DestinationFactory::MakeFileDestination(
    sLogPath, EnableLogRotation, MaxSizeBytes(5000000), MaxOldLogCount(5)));
  DestinationPtr debugDestination(
        DestinationFactory::MakeDebugOutputDestination());

  // set log destinations on the logger
  logger.addDestination(debugDestination);
  logger.addDestination(fileDestination);

  // logging examples
  bool showLogExamples = false;
  if (showLogExamples){
      logInfo()   << "LPub3D started";
      logInfo()   << "Built with Qt" << QT_VERSION_STR << "running on" << qVersion();
      logNotice() << "Here's a" << QString("Notice") << "message";
      logTrace()  << "Here's a" << QString("trace") << "message";
      logDebug()  << "Here's a" << static_cast<int>(QsLogging::DebugLevel) << "message";
      logStatus()   << "Uh-oh! - this level is not displayed in the console only the log";
      qDebug()    << "This message won't be picked up by the logger";
      logError()  << "An error has occurred";
      qWarning()  << "Neither will this one";
      logFatal()  << "Fatal error!";

      logger.setLoggingLevel(QsLogging::OffLevel);
      for (int i = 0;i < 10;++i) {
          logError() << QString::fromUtf8("this message should not be visible");
      }
      logger.setLoggingLevel(QsLogging::TraceLevel);
  } // end init logging

  emit splashMsgSig("10% - Preferences loading...");

  // Preferences
  Preferences::lpub3dLibPreferences(false);
  Preferences::ldrawPreferences(false);
  Preferences::lpub3dUpdatePreferences();

  Preferences::unitsPreferences();
  Preferences::annotationPreferences();

  Preferences::fadestepPreferences();
  Preferences::pliPreferences();
  Preferences::viewerPreferences();

  // Resolution
  defaultResolutionType(Preferences::preferCentimeters);
  setResolution(150);  // DPI

  // Translator
  QTranslator Translator;
  Translator.load(QString("lpub_") + QLocale::system().name().section('_', 0, 0) + ".qm", ":../lc_lib/resources");
  m_application.installTranslator(&Translator);

#if defined(Q_OS_WIN)
  char *ptr;
  strcpy(m_libPath, argv[0]);
  ptr = strrchr(m_libPath,'\\');
  if (ptr)
    *(++ptr) = 0;
  lcSehInit();
#elif defined(Q_OS_MAC)
  QDir bundlePath = QDir(QCoreApplication::applicationDirPath());
  bundlePath.cdUp();
  bundlePath.cdUp();
  bundlePath = QDir::cleanPath(bundlePath.absolutePath() + "/Contents/lc_lib/Resources/");
  QByteArray pathArray = bundlePath.absolutePath().toLocal8Bit();
  m_libPath = pathArray.data();
#else
  m_libPath = LC_INSTALL_PREFIX "/share/lpub3d/";
#endif

#ifdef LC_LDRAW_LIBRARY_PATH
  m_LDrawPath = LC_LDRAW_LIBRARY_PATH;
#else
  m_LDrawPath = NULL;
#endif

  logInfo() << QString("-Initialize: New gui instance created.");

  emit splashMsgSig(QString("30% - %1 Mainwindow loading...").arg(VER_PRODUCTNAME_STR));

  gui = new Gui();

  emit splashMsgSig("60% - 3D Viewer Mainwindow loading...");

  if (!gui->InitializeApp(argc, argv, m_libPath, m_LDrawPath)) {
      logError() << QString("Unable to initialize 3D Viewer.");
    }

}

void Application::main()
{

  emit splashMsgSig(QString("100% %1 loaded.").arg(VER_PRODUCTNAME_STR));

  splash->finish(gui);

  gui->show();

#if !LC_DISABLE_UPDATE_CHECK
  DoInitialUpdateCheck();
#endif

 if (g_App->mLoadFile != NULL){
     QString loadFile = QString("%1").arg(g_App->mLoadFile);
     emit gui->loadFileSig(loadFile);
 }

}

int Application::run()
{
  int returnCode = EXIT_FAILURE;
  try
  {
    // Call the main function
    logInfo() << QString("Run: Starting application...");

    main();

    logInfo() << QString("Run: Application started.");

    returnCode = m_application.exec();
  }
  catch(const std::exception& ex)
  {
    logError() << QString("Run: %1\n%2").arg("LOG_ERROR").arg(ex.what());
  }
  catch(...)
  {
    logError() << QString("Run: An unhandled exception has been thrown.");
  }

  delete gMainWindow;
  gMainWindow = NULL;

  delete g_App;
  g_App = NULL;

  logInfo() << QString("Run: Application terminated with return code %1.").arg(returnCode);

  return returnCode;
}

// Implements the main function here.
ENTRY_POINT
