/****************************************************************************
**
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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

#ifdef Q_OS_MAC
  QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

  connect(this, SIGNAL(splashMsgSig(QString)), this, SLOT(splashMsg(QString)));

}

Application* Application::instance()
{
  return m_instance;
}

void Application::initialize(int &argc, char **argv)
{
  // initialize directories
  Preferences::lpubPreferences();

  // initialize the logger
  Preferences::loggingPreferences();

  using namespace QsLogging;

  Logger& logger = Logger::instance();

  // set default log options
  if (Preferences::logging) {
      if (Preferences::logLevels){

          logger.setLoggingLevels();
          logger.setDebugLevel( Preferences::debugLevel);
          logger.setTraceLevel( Preferences::traceLevel);
          logger.setNoticeLevel(Preferences::noticeLevel);
          logger.setInfoLevel(  Preferences::infoLevel);
          logger.setStatusLevel(Preferences::statusLevel);
          logger.setErrorLevel( Preferences::errorLevel);
          logger.setFatalLevel( Preferences::fatalLevel);

        } else if (Preferences::logLevel){

          bool ok;
          Level logLevel = logger.fromLevelString(Preferences::loggingLevel,&ok);
          if (!ok)
            QMessageBox::critical(NULL,QMessageBox::tr(VER_PRODUCTNAME_STR),
                                  QMessageBox::tr("Failed to set log level %1.\n"
                                                  "Logging is off - level set to OffLevel")
                                  .arg(Preferences::loggingLevel));
          logger.setLoggingLevel(logLevel);
        }

      logger.setIncludeLogLevel(    Preferences::includeLogLevel);
      logger.setIncludeTimestamp(   Preferences::includeTimestamp);
      logger.setIncludeLineNumber(  Preferences::includeLineNumber);
      logger.setIncludeFileName(    Preferences::includeFileName);
      logger.setIncludeFunctionInfo(Preferences::includeFunction);

      logger.setColorizeFunctionInfo(true);
      logger.setColorizeOutput(true);

      // Create log destinations
      DestinationPtr fileDestination(DestinationFactory::MakeFileDestination(
        Preferences::logPath, EnableLogRotation, MaxSizeBytes(5000000), MaxOldLogCount(5)));
      DestinationPtr debugDestination(
            DestinationFactory::MakeDebugOutputDestination());

      // set log destinations on the logger
      logger.addDestination(debugDestination);
      logger.addDestination(fileDestination);

      // logging examples
      bool showLogExamples = false;
      if (showLogExamples){
          logStatus() << "Uh-oh! - this level is not displayed in the console only the log";
          logInfo()   << "LPub3D started";
          logInfo()   << "Built with Qt" << QT_VERSION_STR << "running on" << qVersion();
          logTrace()  << "Here's a" << QString("trace") << "message";
          logDebug()  << "Here's a" << static_cast<int>(QsLogging::DebugLevel) << "message";
          logNotice() << "Here's a" << QString("Notice") << "message";
          qDebug()    << "This message won't be picked up by the logger";
          logError()  << "An error has occurred";
          qWarning()  << "Neither will this one";
          logFatal()  << "Fatal error!";

          Level level = logger.loggingLevel();
          logger.setLoggingLevel(QsLogging::OffLevel);
          for (int i = 0;i < 10;++i) {
              logError() << QString::fromUtf8("this message should not be visible");
          }
          logger.setLoggingLevel(level);
      } // end init logging

    } else {
      logger.setLoggingLevel(OffLevel);
    }

  logInfo() << QString("Initializing application.");

  // splash
  QPixmap pixmap(":/resources/LPub512Splash.png");
  splash = new QSplashScreen(pixmap);

  QFont splashFont;
  splashFont.setFamily("Arial");
  splashFont.setPixelSize(16);
  splashFont.setStretch(130);

  splash->setFont(splashFont);
  splash->show();

  emit splashMsgSig("5% - Initializing application...");

  // Preferences
  Preferences::lpub3dLibPreferences(false);
  Preferences::ldrawPreferences(false);

  emit splashMsgSig("15% - Preferences loading...");

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

  /* load sequence
   * lc_application::LoadDefaults                  (g_App)
   * Gui::gui                                      (gui)
   * lc_application::lcInitialize()                (g_App->Initialize)
   * lcInitialize::LoadPiecesLibrary()             (g_App->Initialize)
   * LoadPiecesLibrary::ldsearchDirPreferences()   (g_App->Initialize)
   * LoadPiecesLibrary::ldsearchDirPreferences()   (g_App->Initialize)
   * lcInitialize::CreateWidgets() [Menus]         (g_App->Initialize)
   * Gui::guiInitialize()                          (gui->Initialize)
   * guiInitialize::populateLdgLiteSearchDirs()    (gui->Initialize)
   * guiInitialize::createMenus    [Menus]         (gui->Initialize)
   */

  logInfo() << QString("-Initialize: New gui instance created.");

  emit splashMsgSig("20% - 3D Viewer window loading...");

  g_App = new lcApplication();

  emit splashMsgSig(QString("30% - %1 window loading...").arg(VER_PRODUCTNAME_STR));

  gui = new Gui();

  gui->getRequireds();

  emit splashMsgSig(QString("40% - 3D Viewer initialization..."));

  if (g_App->Initialize(argc, argv,libPath, LDrawPath)) {

      gui->initialize();
      gMainWindow->SetColorIndex(lcGetColorIndex(4));
      gMainWindow->UpdateRecentFiles();

    } else {

      logError() << QString("Unable to initialize 3D Viewer.");
      throw InitException{};

    }
}

void Application::main()
{

  emit splashMsgSig(QString("100% - %1 loaded.").arg(VER_PRODUCTNAME_STR));
  Preferences::setLPub3DLoaded();

  splash->finish(gui);

  GetAvailableVersions();

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

  delete gui;
  gui = NULL;

  logInfo() << QString("Run: Application terminated with return code %1.").arg(returnCode);

  return returnCode;
}

// Implements the main function here.
ENTRY_POINT
