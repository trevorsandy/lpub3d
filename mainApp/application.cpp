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

#pragma warning(push)
#pragma warning(disable : 4091)

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

  if (dbgHelp == nullptr)
    return EXCEPTION_EXECUTE_HANDLER;

  HANDLE file = CreateFile(minidumpPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

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

  BOOL writeDump = miniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, MiniDumpNormal, exceptionPointers ? &mei : nullptr, nullptr, nullptr);

  CloseHandle(file);
  FreeLibrary(dbgHelp);

  if (writeDump)
    {
      TCHAR message[_MAX_PATH + 256];
      lstrcpy(message, TEXT("LPub3D just crashed. Crash information was saved to the file: '"));
      lstrcat(message, minidumpPath);
      lstrcat(message, TEXT("' Please send it to the developers for debugging."));

      MessageBox(nullptr, message, TEXT("LPub3D"), MB_OK);
    }

  return EXCEPTION_EXECUTE_HANDLER;
}

static void lcSehInit()
{
  if (SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, nullptr, SHGFP_TYPE_CURRENT, minidumpPath) == S_OK)
    {
      lstrcat(minidumpPath, TEXT("\\LPub3D Software\\LPub3D\\dump\\"));
      _tmkdir(minidumpPath);
      lstrcat(minidumpPath, TEXT("lpub3ddump.dmp"));
    }

  SetUnhandledExceptionFilter(lcSehHandler);
}

#endif

// Initializes the Application instance as null
Application* Application::m_instance = nullptr;

Application::Application(int &argc, char **argv)
  : m_application(argc, argv)
{
  m_instance = this;
  m_console_mode = false;
  m_print_output = false;

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
  m_application.setAttribute(Qt::AA_UseDesktopOpenGL);
#endif

  QCoreApplication::setOrganizationName(VER_COMPANYNAME_STR);
  QCoreApplication::setApplicationName(VER_PRODUCTNAME_STR);
  QCoreApplication::setApplicationVersion(VER_PRODUCTVERSION_STR);

  connect(this, SIGNAL(splashMsgSig(QString)), this, SLOT(splashMsg(QString)));

#ifdef Q_OS_MAC
  QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

}

Application* Application::instance()
{
  return m_instance;
}

QStringList Application::arguments()
{
  return m_application.arguments();
}

bool Application::modeGUI()
{
  return ! m_console_mode;
}

void Application::initialize()
{
  // process arguments
  bool headerPrinted = false;
  QStringList ListArgs, Arguments = arguments();
  const int NumArguments = Arguments.size();
  for (int ArgIdx = 1; ArgIdx < NumArguments; ArgIdx++)
    ListArgs << Arguments[ArgIdx];
  for (int ArgIdx = 1; ArgIdx < NumArguments; ArgIdx++)
  {
    const QString& Param = Arguments[ArgIdx];
    if (Param[0] == '-' && ! headerPrinted)
    {
      m_console_mode = true;
      fprintf(stdout, "%s for %s\n",VER_PRODUCTNAME_STR,VER_COMPILED_FOR);
      fprintf(stdout, "==========================\n");
      fprintf(stdout, "Arguments: %s\n",ListArgs.join(" ").toLatin1().constData());
      fflush(stdout);
      headerPrinted = true;
    }
    if (Param == QLatin1String("-v") || Param == QLatin1String("--version"))
    {
      m_console_mode = true;
      m_print_output = true;
      fprintf(stdout, "%s, Version %s, Revision %s, Build %s, ShaHash %s\n",VER_PRODUCTNAME_STR,VER_PRODUCTVERSION_STR,VER_REVISION_STR,VER_BUILD_STR,VER_SHA_HASH_STR);
      fprintf(stdout, "Compiled on " __DATE__ "\n");
      return;
    }
    else if (Param == QLatin1String("-vv") || Param == QLatin1String("--vversion"))
    {
      m_console_mode = true;
      m_print_output = true;
      fprintf(stdout, "3DViewer - by LeoCAD, Version %s, ShaHash %s\n",LC_VERSION_TEXT,LC_VERSION_BUILD);
      fprintf(stdout, "Compiled " __DATE__ "\n");
      return;
    }
    else if (Param == QLatin1String("-?") || Param == QLatin1String("--help"))
    {
      m_console_mode = true;
      m_print_output = true;
      fprintf(stdout, "Usage: lpub3d [options] [file]\n");
      fprintf(stdout, "  [options] can be:\n");
      fprintf(stdout, "  -l, --libpath <path>: Set the Parts Library location to path.\n");
      fprintf(stdout, "  -i, --image <outfile.ext>: Save a picture in the format specified by ext.\n");
      fprintf(stdout, "  -w, --width <width>: Set the picture width.\n");
      fprintf(stdout, "  -h, --height <height>: Set the picture height.\n");
      fprintf(stdout, "  -f, --from <time>: Set the first step to save pictures.\n");
      fprintf(stdout, "  -t, --to <time>: Set the last step to save pictures.\n");
      fprintf(stdout, "  -s, --submodel <submodel>: Set the active submodel.\n");
      fprintf(stdout, "  -c, --camera <camera>: Set the active camera.\n");
      fprintf(stdout, "  --viewpoint <front|back|left|right|top|bottom|home>: Set the viewpoint.\n");
      fprintf(stdout, "  --camera-angles <latitude> <longitude>: Set the camera angles in degrees around the model.\n");
      fprintf(stdout, "  --orthographic: Make the view orthographic.\n");
      fprintf(stdout, "  --highlight: Highlight pieces in the steps they appear.\n");
      fprintf(stdout, "  -obj, --export-wavefront <outfile.obj>: Export the model to Wavefront OBJ format.\n");
      fprintf(stdout, "  -3ds, --export-3ds <outfile.3ds>: Export the model to 3D Studio 3DS format.\n");
      fprintf(stdout, "  -dae, --export-collada <outfile.dae>: Export the model to COLLADA DAE format.\n");
      fprintf(stdout, "  -html, --export-html <folder>: Create an HTML page for the model.\n");
      fprintf(stdout, "  --html-parts-width <width>: Set the HTML part pictures width.\n");
      fprintf(stdout, "  --html-parts-height <height>: Set the HTML part pictures height.\n");
      fprintf(stdout, "  -v, --version: Output LPub3D version information and exit.\n");
      fprintf(stdout, "  -vv, --vversion: Output 3DViewer - by LeoCAD version information and exit.\n");
      fprintf(stdout, "  -?, --help: Display this help message and exit.\n");
      fprintf(stdout, "  \n");
      return;
    }
  }

  // initialize directories
  Preferences::lpubPreferences();

  // initialize the logger
  Preferences::loggingPreferences();

  using namespace QsLogging;

  Logger& logger = Logger::instance();

  // set default log options
  if (Preferences::logging)
  {
    if (Preferences::logLevels)
    {

      logger.setLoggingLevels();
      logger.setDebugLevel( Preferences::debugLevel);
      logger.setTraceLevel( Preferences::traceLevel);
      logger.setNoticeLevel(Preferences::noticeLevel);
      logger.setInfoLevel(  Preferences::infoLevel);
      logger.setStatusLevel(Preferences::statusLevel);
      logger.setErrorLevel( Preferences::errorLevel);
      logger.setFatalLevel( Preferences::fatalLevel);

    }
    else if (Preferences::logLevel)
    {

      bool ok;
      Level logLevel = logger.fromLevelString(Preferences::loggingLevel,&ok);
      if (!ok)
      {
        QString Message = QMessageBox::tr("Failed to set log level %1.\n"
                                          "Logging is off - level set to OffLevel")
            .arg(Preferences::loggingLevel);
        if (modeGUI())
          QMessageBox::critical(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR), Message);
        else
          fprintf(stderr, "%s", Message.toLatin1().constData());
      }
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
    if (showLogExamples)
    {
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
      for (int i = 0;i < 10;++i)
      {
        logError() << QString::fromUtf8("this message should not be visible");
      }
      logger.setLoggingLevel(level);
    } // end init logging

  } else {
    logger.setLoggingLevel(OffLevel);
  }

  logInfo() << QString("Initializing application.");

  // splash
  if (modeGUI())
  {

    QPixmap pixmap(":/resources/LPub512Splash.png");
    splash = new QSplashScreen(pixmap);

    QFont splashFont;
    splashFont.setFamily("Arial");
    splashFont.setPixelSize(16);
    splashFont.setStretch(130);

    splash->setFont(splashFont);
    splash->show();
  }

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

  // Resolution
  defaultResolutionType(Preferences::preferCentimeters);
  setResolution(150);  // DPI

  // Translator
  QTranslator Translator;
  Translator.load(QString("lpub_") + QLocale::system().name().section('_', 0, 0) + ".qm", ":../lc_lib/resources");
  m_application.installTranslator(&Translator);

  qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");

  QList<QPair<QString, bool>> LibraryPaths;

#if defined(Q_OS_WIN)
  lcSehInit();
  if (QDir(Preferences::lpub3dPath + "/extras").exists()) { // we have a portable distribution
    LibraryPaths += qMakePair(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/extras/complete.zip"), true);
  } else {
    LibraryPaths += qMakePair(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/data/complete.zip"), true);
  }
#endif
#ifdef Q_OS_LINUX
  LibraryPaths += qMakePair(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../share/lpub3d/complete.bin"), true);
#endif

#ifdef Q_OS_MAC
  LibraryPaths += qMakePair(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../Contents/Resources/complete.bin"), true);
#endif

#ifdef LC_LDRAW_LIBRARY_PATH
  LibraryPaths += qMakePair(QString::fromLatin1(LC_LDRAW_LIBRARY_PATH), false);
#endif

  setlocale(LC_NUMERIC, "C");

  //-- #elif defined(Q_OS_MAC)
  //--   QDir bundlePath = QDir(QCoreApplication::applicationDirPath());
  //--   bundlePath.cdUp();
  //--   bundlePath.cdUp();
  //--   bundlePath = QDir::cleanPath(bundlePath.absolutePath() + "/Contents/lc_lib/Resources/");
  //--   QByteArray pathArray = bundlePath.absolutePath().toLocal8Bit();
  //--   const char* libPath = pathArray.data();
  //-- #else
  //--   const char* libPath = LC_INSTALL_PREFIX "/share/lpub3d/";
  //-- #endif

  //-- #ifdef LC_LDRAW_LIBRARY_PATH
  //--   const char* LDrawPath = LC_LDRAW_LIBRARY_PATH;
  //-- #else
  //--   const char* LDrawPath = nullptr;
  //-- #endif

  /* load sequence
* lc_application::LoadDefaults                  (gApplication)
* Gui::gui                                      (gui)
* lc_application::lcInitialize()                (gApplication->Initialize)
* lcInitialize::LoadPiecesLibrary()             (gApplication->Initialize)
* LoadPiecesLibrary::ldsearchDirPreferences()   (gApplication->Initialize)
* LoadPiecesLibrary::ldsearchDirPreferences()   (gApplication->Initialize)
* lcInitialize::CreateWidgets() [Menus]         (gApplication->Initialize)
* Gui::guiInitialize()                          (gui->Initialize)
* guiInitialize::populateLdgLiteSearchDirs()    (gui->Initialize)
* guiInitialize::createMenus    [Menus]         (gui->Initialize)
*/

  emit splashMsgSig("20% - 3D Viewer window loading...");

  gApplication = new lcApplication();
  //gApplication = new lcApplication(argc, argv);

  emit splashMsgSig(QString("30% - %1 GUI window loading...").arg(VER_PRODUCTNAME_STR));

  gui = new Gui();

  gui->getRequireds();

  emit splashMsgSig(QString("40% - 3D Viewer initialization..."));

  if (gApplication->Initialize(LibraryPaths, gui)) {
    gui->initialize();
  } else {
    logError() << QString("Unable to initialize 3D Viewer.");
    throw InitException{};
  }
}

void Application::mainApp()
{
  if (m_print_output)
    return;

  emit splashMsgSig(QString("100% - %1 loaded.").arg(VER_PRODUCTNAME_STR));

  Preferences::setLPub3DLoaded();

  GetAvailableVersions();

  if (modeGUI())
  {
    splash->finish(gui);

    gui->show();

#if !LC_DISABLE_UPDATE_CHECK
    DoInitialUpdateCheck();
#endif
  }

  // load file passed in on command line.
  if (!gApplication->mLoadFile.isEmpty())
  {
    emit gui->loadFileSig(gApplication->mLoadFile);
  }

}

int Application::run()
{
  int ExecReturn = EXIT_FAILURE;
  try
  {
    // Call the main function
    logInfo() << QString("Run: Starting application...");

    mainApp();

    logInfo() << QString("Run: Application started.");

    if (modeGUI())
      ExecReturn = m_application.exec();
    else
      ExecReturn = EXIT_SUCCESS;
  }
  catch(const std::exception& ex)
  {
    logError() << QString("Run: %1\n%2").arg("LOG_ERROR").arg(ex.what());
  }
  catch(...)
  {
    logError() << QString("Run: An unhandled exception has been thrown.");
  }

  if (!m_print_output)
  {
    delete gMainWindow;
    gMainWindow = nullptr;

    delete gApplication;
    gApplication = nullptr;

    delete gui;
    gui = nullptr;
  }

  logInfo() << QString("Run: Application terminated with return code %1.").arg(ExecReturn);

  return ExecReturn;
}

// Implements the main function here.
ENTRY_POINT
