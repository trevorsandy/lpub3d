/****************************************************************************
**
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
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
#include <TCFoundation/TCUserDefaults.h>

#include "lpub_preferences.h"
#include "lpub.h"
#include "resolution.h"

#include "lc_math.h"
#include "lc_profile.h"

#include "updatecheck.h"

#include "QsLogDest.h"

#ifdef Q_OS_WIN

  #include <stdio.h>
  #include <fcntl.h>
  #include <io.h>
  #include <fstream>

  void Application::RedirectIOToConsole()
  {
    // Attach to the existing console of the parent process
    m_allocated_console = !AttachConsole( ATTACH_PARENT_PROCESS );

    if (m_allocated_console) {
      // maximum number of lines the output console should have
      static const WORD MAX_CONSOLE_LINES = 1000;

      // Allocate the new console.
      AllocConsole();

      // set the screen buffer to be big enough to let us scroll text
      GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
                                 &ConsoleInfo);
      ConsoleInfo.dwSize.Y = MAX_CONSOLE_LINES;
      SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),
                                 ConsoleInfo.dwSize);
    }

    // Get STDOUT handle
    ConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    int SystemOutput = _open_osfhandle(intptr_t(ConsoleOutput), _O_TEXT);
    COutputHandle = _fdopen(SystemOutput, "w");

    // Get STDERR handle
    ConsoleError = GetStdHandle(STD_ERROR_HANDLE);
    int SystemError = _open_osfhandle(intptr_t(ConsoleError), _O_TEXT);
    CErrorHandle = _fdopen(SystemError, "w");

    // Get STDIN handle
    ConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
    int SystemInput = _open_osfhandle(intptr_t(ConsoleInput), _O_TEXT);
    CInputHandle = _fdopen(SystemInput, "r");

    //retrieve and save the current attributes
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
                               &ConsoleInfo);
    m_currentConsoleAttr = ConsoleInfo.wAttributes;
    FlushConsoleInputBuffer(ConsoleOutput);

    //change the attribute to what you like
    SetConsoleTextAttribute (
                ConsoleOutput,
                FOREGROUND_RED |
                FOREGROUND_GREEN);

    //make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
    std::ios_base::sync_with_stdio(true);

    // Redirect the CRT standard input, output, and error handles to the console
    freopen_s(&CInputHandle, "CONIN$", "r", stdin);
    freopen_s(&COutputHandle, "CONOUT$", "w", stdout);
    freopen_s(&CErrorHandle, "CONOUT$", "w", stderr);

    //Clear the error state for each of the C++ standard stream objects. We need to do this, as
    //attempts to access the standard streams before they refer to a valid target will cause the
    //iostream objects to enter an error state. In versions of Visual Studio after 2005, this seems
    //to always occur during startup regardless of whether anything has been read from or written to
    //the console or not.
    std::wcout.clear();
    std::cout.clear();
    std::wcerr.clear();
    std::cerr.clear();
    std::wcin.clear();
    std::cin.clear();
  }

  #pragma warning(push)
  #pragma warning(disable : 4091)
  #include <DbgHelp.h>
  #include <direct.h>
  #include <ShlObj.h>
  #pragma warning(pop)

  #ifdef UNICODE
  #ifndef _UNICODE
  #define _UNICODE
  #endif
  #endif

  #include <tchar.h>

  static TCHAR gMinidumpPath[_MAX_PATH];

  LONG WINAPI Application::lcSehHandler(PEXCEPTION_POINTERS exceptionPointers)
  {
      if (IsDebuggerPresent())
          return EXCEPTION_CONTINUE_SEARCH;

      HMODULE dbgHelp = LoadLibrary(TEXT("dbghelp.dll"));

      if (dbgHelp == nullptr)
          return EXCEPTION_EXECUTE_HANDLER;

      HANDLE file = CreateFile(gMinidumpPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

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
          lstrcpy(message, TEXT(VER_PRODUCTNAME_STR " crashed. Crash information was saved to the file '"));
          lstrcat(message, gMinidumpPath);
          lstrcat(message, TEXT("', please send it to the developer for debugging."));

          MessageBox(nullptr, message, TEXT(VER_PRODUCTNAME_STR), MB_OK);
      }

      return EXCEPTION_EXECUTE_HANDLER;
  }

  void Application::lcSehInit()
  {
      if (GetTempPath(sizeof(gMinidumpPath) / sizeof(gMinidumpPath[0]), gMinidumpPath))
          lstrcat(gMinidumpPath, TEXT(VER_PRODUCTNAME_STR ".dmp"));

      SetUnhandledExceptionFilter(lcSehHandler);
  }
#endif

// Initializes the Application instance as null
Application* Application::m_instance = nullptr;

Application::Application(int &argc, char **argv)
  : m_application(argc, argv)
{

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5,6,0)
#ifndef Q_OS_MAC
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#endif

    //QCoreApplication::setAttribute(Qt::AA_Use96Dpi);

    QCoreApplication::setOrganizationName(VER_COMPANYNAME_STR);
    QCoreApplication::setApplicationVersion(VER_PRODUCTVERSION_STR);

    //qDebug() << "QStyleFactory valid styles:" << QStyleFactory::keys();
#ifdef Q_OS_MAC
    QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
    m_application.setStyle(QStyleFactory::create("macintosh"));
#endif

  m_instance = this;
  m_console_mode = false;
  m_print_output = false;
  m_redirect_io_to_console = true;
#ifdef Q_OS_WIN
  m_allocated_console = false;
#endif

  connect(this, SIGNAL(splashMsgSig(QString)), this, SLOT(splashMsg(QString)));

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

void Application::setTheme(){
  Theme t = ThemeDefault;
  QColor vc = QColor(THEME_VIEWER_BGCOLOR_DEFAULT);

  if (Preferences::displayTheme == THEME_DARK){
      t = ThemeDark;
      QFile f(":qdarkstyle/style.qss");
      if (!f.exists())
      {
          fprintf(stdout,"Unable to set dark stylesheet, file not found\n");
      }
      else
      {
          f.open(QFile::ReadOnly | QFile::Text);
          QTextStream ts(&f);
          qApp->setStyleSheet(ts.readAll());
          vc = QColor(THEME_VIEWER_BGCOLOR_DARK);
      }
      if (!Preferences::customSceneBackgroundColor)
          Preferences::setSceneBackgroundColorPreference(THEME_SCENE_BGCOLOR_DARK);
      if (!Preferences::customSceneGridColor)
          Preferences::setSceneGridColorPreference(THEME_GRID_PEN_DARK);
      if (!Preferences::customSceneRulerTickColor)
          Preferences::setSceneRulerTickColorPreference(THEME_RULER_TICK_PEN_DARK);
      if (!Preferences::customSceneRulerTrackingColor)
          Preferences::setSceneRulerTrackingColorPreference(THEME_RULER_TRACK_PEN_DARK);
      if (!Preferences::customSceneGuideColor)
          Preferences::setSceneGuideColorPreference(THEME_GUIDE_PEN_DARK);
    }
  else
  if (Preferences::displayTheme == THEME_DEFAULT){
      t = ThemeDefault;
      qApp->setStyleSheet( QString() );
      if (!Preferences::customSceneBackgroundColor)
          Preferences::setSceneBackgroundColorPreference(THEME_SCENE_BGCOLOR_DEFAULT);
      if (!Preferences::customSceneGridColor)
          Preferences::setSceneGridColorPreference(THEME_GRID_PEN_DEFAULT);
      if (!Preferences::customSceneRulerTickColor)
          Preferences::setSceneRulerTickColorPreference(THEME_RULER_TICK_PEN_DEFAULT);
      if (!Preferences::customSceneRulerTrackingColor)
          Preferences::setSceneRulerTrackingColorPreference(THEME_RULER_TRACK_PEN_DEFAULT);
      if (!Preferences::customSceneGuideColor)
          Preferences::setSceneGuideColorPreference(THEME_GUIDE_PEN_DEFAULT);
    }

  lcVector3 viewerBackgroundColor = lcVector3FromColor(LC_RGB(vc.red(), vc.green(), vc.blue()));
  lcSetProfileInt(LC_PROFILE_DEFAULT_BACKGROUND_COLOR, lcColorFromVector3(viewerBackgroundColor));
}

void Application::initialize()
{
    // process arguments
    bool headerPrinted = false;
#ifdef Q_OS_WIN
    // Set distribution type
    Preferences::setDistribution();
    // Request console redirect
    bool consoleRedirectTreated = false;
#endif
    QString hdr, args, ldrawLibrary, loadedLibrary;
    QStringList ListArgs, Arguments = arguments();
    const int NumArguments = Arguments.size();
    for (int ArgIdx = 1; ArgIdx < NumArguments; ArgIdx++)
    ListArgs << Arguments[ArgIdx];
#if defined LP3D_CONTINUOUS_BUILD || defined LP3D_DEVOPS_BUILD || defined LP3D_NEXT_BUILD
    hdr = QString("%1 v%2 r%3 (%4) for %5")
                            .arg(VER_PRODUCTNAME_STR)
                            .arg(VER_PRODUCTVERSION_STR)
                            .arg(VER_REVISION_STR)
                            .arg(VER_BUILD_TYPE_STR)
                            .arg(VER_COMPILED_FOR);
#else
    hdr = QString("%1 v%2%3 for %4")
                            .arg(VER_PRODUCTNAME_STR)
                            .arg(VER_PRODUCTVERSION_STR)
                            .arg(QString(VER_REVISION_STR).toInt() ?
                                     QString(" r%1").arg(VER_REVISION_STR) :
                                     QString())
                            .arg(VER_COMPILED_FOR);
#endif
    args = QString("Arguments: %1").arg(ListArgs.join(" "));
    for (int ArgIdx = 1; ArgIdx < NumArguments; ArgIdx++)
    {
        const QString& Param = Arguments[ArgIdx];
        // Items in this condition execute in GUI mode - first item executes in command console mode also)
        if (Param[0] != '-')
        {
            if (! headerPrinted)
            {
#ifdef QT_DEBUG_MODE
                qDebug() << "";
                qDebug() << hdr;
                qDebug() << "==========================";
                qDebug() << args;
#else
                fprintf(stdout, "\n%s\n",hdr.toLatin1().constData());
                fprintf(stdout, "==========================\n");
                fprintf(stdout, "%s\n",args.toLatin1().constData());
                fflush(stdout);
#endif
                headerPrinted = true;
            }

            bool EmptyFileString = m_commandline_file.isEmpty();
            bool ArgFileExists   = QFileInfo(Param).exists();
            if (EmptyFileString && ArgFileExists && !m_console_mode)
                m_commandline_file = Param;

            if (Param == QLatin1String("+cr") || Param == QLatin1String("++console-redirect"))
            {
#ifdef Q_OS_WIN
                if (!consoleRedirectTreated) {
                    consoleRedirectTreated = true;
                    RedirectIOToConsole();
                }
#else
                continue;
#endif
            }
            else
            // Invoke LEGO library in GUI mode
            if (Param == QLatin1String("+ll") || Param == QLatin1String("++liblego"))
                loadedLibrary = LEGO_LIBRARY;
            else
            // Invoke TENTE library in GUI mode
            if (Param == QLatin1String("+lt") || Param == QLatin1String("++libtente"))
                loadedLibrary = TENTE_LIBRARY;
            else
            // Invoke VEXIQ library in GUI mode
            if (Param == QLatin1String("+lv") || Param == QLatin1String("++libvexiq"))
                loadedLibrary = VEXIQ_LIBRARY;

            // Set library type from loaded library
            if (!loadedLibrary.isEmpty()) {
                if (!ldrawLibrary.isEmpty()) {
#ifdef Q_OS_WIN
                    if(consoleRedirectTreated) {
#endif
                    fprintf(stdout, "%s library already loaded. Only one library type can be loaded,\n"
                                    "The %s library will be ignored\n",
                            ldrawLibrary.toLatin1().constData(),
                            loadedLibrary.toLatin1().constData());
                    fflush(stdout);
#ifdef Q_OS_WIN
                    }
#endif
                } else {
                    ldrawLibrary = loadedLibrary;
                    loadedLibrary.clear();
                }
            }
            // Everything after this occurs in console mode so escape here.
            continue;
        }
        // Items in this condition will execute in command console mode
        else
        {
            // Print the console header
            if (! headerPrinted)
            {
                m_console_mode = true;
#ifdef QT_DEBUG_MODE
                qDebug() << "";
                qDebug() << hdr;
                qDebug() << "==========================";
                qDebug() << args;
#else
                fprintf(stdout, "\n%s\n",hdr.toLatin1().constData());
                fprintf(stdout, "==========================\n");
                fprintf(stdout, "%s\n",args.toLatin1().constData());
                fflush(stdout);
#endif
              headerPrinted = true;
            }

            if (Param == QLatin1String("-ns") || Param == QLatin1String("--no-stdout-log"))
                Preferences::setStdOutToLogPreference(true);
            else
            // Version output
            if (Param == QLatin1String("-v") || Param == QLatin1String("--version"))
            {
              m_console_mode = true;
              m_print_output = true;
              fprintf(stdout, "%s, %s %s, Revision %s, Build %s, ShaHash %s\n",VER_PRODUCTNAME_STR,VER_BUILD_TYPE_STR,VER_PRODUCTVERSION_STR,VER_REVISION_STR,VER_BUILD_STR,VER_SHA_HASH_STR);
              fprintf(stdout, "Compiled on " __DATE__ "\n");
              fflush(stdout);
              return;
            }
            else
            // 3DViewer (LeoCAD) version output
            if (Param == QLatin1String("-vv") || Param == QLatin1String("--viewer-version"))
            {
              m_console_mode = true;
              m_print_output = true;
              fprintf(stdout, "3DViewer - by LeoCAD, Version %s, ShaHash %s\n",LC_VERSION_TEXT,LC_VERSION_BUILD);
              fprintf(stdout, "Compiled " __DATE__ "\n");
              fflush(stdout);
              return;
            }
            else
            // Help output
            if (Param == QLatin1String("-?") || Param == QLatin1String("--help"))
            {
                m_console_mode = true;
                m_print_output = true;
                fprintf(stdout, "Usage: %s [options] [LDraw file]\n",qApp->applicationName().toLatin1().constData());
                fprintf(stdout, "  [LDraw file]:\n");
                fprintf(stdout, "  Use absolute file path and file name with .ldr, .mpd or .dat extension.\n");
                fprintf(stdout, "  [options]:\n");
                fprintf(stdout, "  Options preceded by '+' will launch %s in GUI mode.\n",qApp->applicationName().toLatin1().constData());
                fprintf(stdout, "\n");
                fprintf(stdout, "[%s commands]\n",qApp->applicationName().toLatin1().constData());
                fprintf(stdout, "  +cr, ++console-redirect: Create console to redirect standard output standard error and standard input. Default is off.\n");
                fprintf(stdout, "  +ll, ++liblego: Load the LDraw LEGO archive parts library in GUI mode.\n");
                fprintf(stdout, "  +lt, ++libtente: Load the LDraw TENTE archive parts library in GUI mode.\n");
                fprintf(stdout, "  +lv, ++libvexiq: Load the LDraw VEXIQ archive parts library in GUI mode.\n");
                fprintf(stdout, "  -sl --stud-logo <type>: Set the stud logo type 0 - 5, default is 0 no logo.\n");
                fprintf(stdout, "  -d, --image-output-directory <directory>: Designate the png, jpg or bmp save folder using absolute path.\n");
                fprintf(stdout, "  -fc, --fade-steps-color <LDraw color code>: Set the global fade color. Overridden by fade opacity - if opacity not 100 percent. Default is %s\n",LEGO_FADE_COLOUR_DEFAULT);
                fprintf(stdout, "  -fo, --fade-step-opacity <percent>: Set the fade steps opacity percent. Overrides fade color - if opacity not 100 percent. Default is %s percent\n",QString(FADE_OPACITY_DEFAULT).toLatin1().constData());
                fprintf(stdout, "  -fs, --fade-steps: Turn on fade previous steps. Default is off.\n");
                fprintf(stdout, "  -hc, --highlight-step-color <Hex color code>: Set the step highlight color. Color code optional. Format is #RRGGBB. Default is %s.\n",HIGHLIGHT_COLOUR_DEFAULT);
                fprintf(stdout, "  -hs, --highlight-step: Turn on highlight current step. Default is off.\n");
                fprintf(stdout, "  -ll, --liblego: Load the LDraw LEGO archive parts library in command console mode.\n");
                fprintf(stdout, "  -lt, --libtente: Load the LDraw TENTE archive parts library in command console mode.\n");
                fprintf(stdout, "  -lv, --libvexiq: Load the LDraw VEXIQ archive parts library in command console mode.\n");
                fprintf(stdout, "  -ns, --no-stdout-log: Do not enable standard output for logged entries. Useful on Linux to prevent double (stdout and QSLog) output. Default is off.\n");
                fprintf(stdout, "  -o, --export-option <option>: Set output format pdf, png, jpeg, bmp, stl, 3ds, pov, dae or obj. Used with process-export. Default is pdf.\n");
                fprintf(stdout, "  -of, --pdf-output-file <path>: Designate the pdf document save file using absolute path.\n");
                fprintf(stdout, "  -p, --preferred-renderer <renderer>: Set renderer native, ldglite, ldview, ldview-sc, ldview-scsl, povray, or povray-ldv. Default is native.\n ");
                fprintf(stdout, "  -pe, --process-export: Export instruction document or images. Used with export-option. Default is pdf document.\n");
                fprintf(stdout, "  -pf, --process-file: Process ldraw file and generate images in png format.\n");
                fprintf(stdout, "  -r, --range <page range>: Set page range - e.g. 1,2,9,10-42. Default is all pages.\n");
                fprintf(stdout, "  -rs, --reset-search-dirs: Reset the LDraw parts directories to those searched by default. Default is off.\n");
                fprintf(stdout, "  -v, --version: Output LPub3D version information and exit.\n");
                fprintf(stdout, "  -x, --clear-cache: Reset the LDraw file and image caches. Used with export-option change. Default is off.\n");
//              fprintf(stdout, "  -im, --image-matte: [Experimental] Turn on image matting for fade previous step. Combine current and previous images using pixel blending - LDView only. Default is off.\n");
                fprintf(stdout, "\n");
                fprintf(stdout, "[3DViewer commands - Not tested]\n");
                fprintf(stdout, "  -c, --camera <camera>: Set the active camera.\n");
                fprintf(stdout, "  -f, --from <time>: Set the first step to save pictures.\n");
                fprintf(stdout, "  -h, --height <height>: Set the picture height.\n");
                fprintf(stdout, "  -i, --image <outfile.ext>: Save a picture in the format specified by ext.\n");
                fprintf(stdout, "  -s, --submodel <submodel>: Set the active submodel.\n");
                fprintf(stdout, "  -t, --to <time>: Set the last step to save pictures.\n");
                fprintf(stdout, "  -w, --width <width>: Set the picture width.\n");
                fprintf(stdout, "  -vv, --viewer-version: Output 3DViewer - by LeoCAD version information and exit.\n");
                fprintf(stdout, "  -3ds, --export-3ds <outfile.3ds>: Export the model to 3D Studio 3DS format.\n");
                fprintf(stdout, "  -dae, --export-collada <outfile.dae>: Export the model to COLLADA DAE format.\n");
                fprintf(stdout, "  -html, --export-html <folder>: Create an HTML page for the model.\n");
                fprintf(stdout, "  -obj, --export-wavefront <outfile.obj>: Export the model to Wavefront OBJ format.\n");
                fprintf(stdout, "  --camera-angles <latitude> <longitude>: Set the camera angles in degrees around the model.\n");
                fprintf(stdout, "  --highlight: Highlight parts in the steps they appear.\n");
                fprintf(stdout, "  --html-parts-height <height>: Set the HTML part pictures height.\n");
                fprintf(stdout, "  --html-parts-width <width>: Set the HTML part pictures width.\n");
                fprintf(stdout, "  --line-width <width>: Set the with of the edge lines.\n");
                fprintf(stdout, "  --orthographic: Make the view orthographic.\n");
                fprintf(stdout, "  --shading <wireframe|flat|default|full>: Select shading mode for rendering.\n");
                fprintf(stdout, "  --viewpoint <front|back|left|right|top|bottom|home>: Set the viewpoint.\n");
                fprintf(stdout, "\n");
                fprintf(stdout, "[Help]\n");
                fprintf(stdout, "  -?, --help: Display this help message and exit.\n");
                fprintf(stdout, "\n");
                fflush(stdout);
                return;
            }
            else
            // Invoke LEGO library in command console mode
            if (Param == QLatin1String("-ll") || Param == QLatin1String("--liblego"))
                loadedLibrary = LEGO_LIBRARY;
            else
            // Invoke TENTE library in command console mode
            if (Param == QLatin1String("-lt") || Param == QLatin1String("--libtente"))
                loadedLibrary = TENTE_LIBRARY;
            else
            // Invoke VEXIQ library in command console mode
            if (Param == QLatin1String("-lv") || Param == QLatin1String("--libvexiq"))
                loadedLibrary = VEXIQ_LIBRARY;

            // Set library type from loaded library
            if (!loadedLibrary.isEmpty()) {
                if (!ldrawLibrary.isEmpty()) {
                    fprintf(stdout, "%s library already loaded. Only one library type can be loaded,\n"
                                    "The %s library will be ignored\n",
                            ldrawLibrary.toLatin1().constData(),
                            loadedLibrary.toLatin1().constData());
                    fflush(stdout);
                } else {
                    ldrawLibrary = loadedLibrary;
                    loadedLibrary.clear();
                }
            }
        }
    }

    // Enum registrations from name.h
    Q_ENUMS(PartType)
    Q_ENUMS(PliType)
    Q_ENUMS(LogType)
    Q_ENUMS(IniFlag)
    Q_ENUMS(DividerType)
    Q_ENUMS(ShowLoadMsgType)
    Q_ENUMS(LoadMsgType)
    Q_ENUMS(RulerTrackingType)
    Q_ENUMS(SceneGuidesPosType)
    Q_ENUMS(LibType)
    Q_ENUMS(Theme)
    Q_ENUMS(SaveOnSender)
    Q_ENUMS(NativeType)
    Q_ENUMS(SceneObjectInfo)
    Q_ENUMS(GridStepSize)
    Q_ENUMS(LDrawUnofficialFileType)
    Q_ENUMS(SubAttributes)
    Q_ENUMS(NameKeyAttributes)
    Q_ENUMS(SceneObject)

    // other enum registrations
    Q_ENUMS(Dimensions)
    Q_ENUMS(PAction)
    Q_ENUMS(Direction)
    Q_ENUMS(ExportOption)
    Q_ENUMS(ExportMode)

    // Set loaded library flags and variables
    Preferences::setLPub3DAltLibPreferences(ldrawLibrary);

    // initialize directories
    Preferences::lpubPreferences();

    // initialize the logger
    Preferences::loggingPreferences();

    using namespace QsLogging;

    Logger& logger = Logger::instance();

    int logLevelIndex = -1;

    // set default log options
    if (Preferences::logging)
    {
        bool debugLogging = false;
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

            debugLogging = Preferences::debugLevel;
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
            logLevelIndex = QStringList(QString(VER_LOGGING_LEVELS_STR).split(",")).indexOf(Preferences::loggingLevel,0);
            debugLogging = logLevelIndex > -1 && logLevelIndex <= 3;
            logger.setLoggingLevel(logLevel);
        }

        Preferences::setDebugLogging(debugLogging);
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
        DestinationPtr debugDestination(DestinationFactory::MakeDebugOutputDestination());

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

    logInfo() << QString("Initializing application...");

    // application version information
    logInfo() << "-----------------------------";
    logInfo() << hdr;
    logInfo() << "=============================";
    logInfo() << QString("Arguments....................(%1)").arg(ListArgs.join(" "));
    QDir cwd(QCoreApplication::applicationDirPath());
#ifdef Q_OS_MAC           // for macOS
  logInfo() << QString(QString("macOS Binary Directory.......(%1)").arg(cwd.dirName()));
  if (cwd.dirName() == "MacOS") {   // MacOS/         (app bundle executable folder)
      cwd.cdUp();                   // Contents/      (app bundle contents folder)
      cwd.cdUp();                   // LPub3D.app/    (app bundle folder)
      cwd.cdUp();                   // Applications/  (app bundle installation folder)
  }
  logInfo() << QString(QString("macOS Base Directory.........(%1)").arg(cwd.dirName()));
  if (QCoreApplication::applicationName() != QString(VER_PRODUCTNAME_STR))
  {
      logInfo() << QString(QString("macOS Info.plist update......(%1)").arg(Preferences::lpub3dAppName));
      QFileInfo plbInfo("/usr/libexec/PlistBuddy");
      if (!plbInfo.exists())
          logInfo() << QString(QString("ERROR - %1 not found, cannot update Info.Plist").arg(plbInfo.absoluteFilePath()));
  }
#elif defined Q_OS_LINUX   // for Linux
  QDir progDir(QString("%1/../share").arg(cwd.absolutePath()));
  QDir contentsDir(progDir.absolutePath() + "/");
  QStringList fileFilters = QStringList() << "lpub3d*";
  QStringList shareContents = contentsDir.entryList(fileFilters);
  if (shareContents.size() > 0)
      logInfo() << QString(QString("LPub3D Application Folder....(%1)").arg(Preferences::lpub3dAppName));
  else
      logInfo() << QString(QString("ERROR - Application Folder Not Found."));
#endif
  // applications paths:
  logInfo() << QString(QString("LPub3D App Data Path.........(%1)").arg(Preferences::lpubDataPath));
#ifdef Q_OS_MAC   // macOS
  logInfo() << QString(QString("LPub3D Bundle App Path.......(%1)").arg(Preferences::lpub3dPath));
#else            // Linux and Windows
  QString logPath = QString("%1/logs/%2Log.txt").arg(Preferences::lpubDataPath).arg(VER_PRODUCTNAME_STR);
  logInfo() << QString(QString("LPub3D Executable Path.......(%1)").arg(Preferences::lpub3dPath));
  logInfo() << QString(QString("LPub3D Log Path..............(%1)").arg(logPath));
#endif
#ifdef Q_OS_WIN
  QSettings Settings;
  QString dataDir = "data";
  QString dataPath = Preferences::lpub3dPath;
  if (Preferences::portableDistribution) {
      dataDir = "extras";
      logInfo() << QString("LPub3D Portable Distribution.(Yes)");
      // On Windows installer 'dataLocation' folder defaults to LPub3D install path but can be set with 'DataLocation' reg key
  } else if (Settings.contains(QString("%1/%2").arg(SETTINGS,"DataLocation"))) {
      QString validDataPath = Settings.value(QString("%1/%2").arg(SETTINGS,"DataLocation")).toString();
      QDir validDataDir(QString("%1/%2/").arg(validDataPath,dataDir));
      if(QDir(validDataDir).exists()) {
          dataPath = validDataPath;
          logInfo() << QString(QString("LPub3D Data Location.........(%1)").arg(validDataDir.absolutePath()));
      }
  }
#else
  logInfo() << QString(QString("LPub3D Extras Resource Path..(%1)").arg(Preferences::lpub3dExtrasResourcePath));
#if defined Q_OS_LINUX
  QDir rendererDir(QString("%1/../../opt/%2").arg(Preferences::lpub3dPath).arg(Preferences::lpub3dAppName));
  logInfo() << QString(QString("LPub3D Renderers Exe Path....(%1/3rdParty)").arg(rendererDir.absolutePath()));
#endif
#endif
    logInfo() << QString("LPub3D Loaded LDraw Library..(%1)").arg(Preferences::validLDrawPartsLibrary);
    logInfo() << QString("Logging Level................(%1 (%2), Levels: [%3])").arg(Preferences::loggingLevel)
                         .arg(QString::number(logLevelIndex)).arg(QString(VER_LOGGING_LEVELS_STR).toLower());
    logInfo() << QString("Debug Logging................(%1)").arg(Preferences::debugLogging ? "Enabled" : "Disabled");
    logInfo() << "-----------------------------";

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

    emit splashMsgSig(QString("5% - Loading library for %1...").arg(Preferences::validLDrawPartsLibrary));

    // Preferences
    Preferences::lpub3dLibPreferences(false);
    Preferences::ldrawPreferences(false);

    emit splashMsgSig("15% - Preferences loading...");

    Preferences::lpub3dUpdatePreferences();
    Preferences::fadestepPreferences();
    Preferences::highlightstepPreferences();
    Preferences::unitsPreferences();
    Preferences::annotationPreferences();
    Preferences::pliPreferences();
    Preferences::userInterfacePreferences();

    // Resolution
    defaultResolutionType(Preferences::preferCentimeters);

    // Translator - not implemented
    QString Language = lcGetProfileString(LC_PROFILE_LANGUAGE);
    QLocale Locale;

    if (!Language.isEmpty())
        Locale = QLocale(Language);

    QTranslator QtTranslator;
    if (QtTranslator.load(Locale, "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        m_application.installTranslator(&QtTranslator);
#ifdef Q_OS_WIN
    else if (QtTranslator.load(Locale, "qt", "_", qApp->applicationDirPath() + "/translations"))
        m_application.installTranslator(&QtTranslator);
#endif

    QTranslator QtBaseTranslator;
    if (QtBaseTranslator.load("qtbase_" + Locale.name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        m_application.installTranslator(&QtBaseTranslator);
#ifdef Q_OS_WIN
    else if (QtBaseTranslator.load("qtbase_" + Locale.name(), qApp->applicationDirPath() + "/translations"))
        m_application.installTranslator(&QtBaseTranslator);
#endif

    QTranslator Translator;
    if (Translator.load("lpub_" + Locale.name(), ":../lclib/resources"))
        m_application.installTranslator(&Translator);

    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");

    QList<QPair<QString, bool>> LibraryPaths;

#ifdef Q_OS_WIN
    lcSehInit();
#endif

    setlocale(LC_NUMERIC, "C");

/* load sequence
* Gui::gui()                                             (gui)           [LPub3D]
* lcApplication::lcApplication()                         (gApplication)  [3DViewer]
* lcApplication::LoadDefaults                            (gApplication)
* lcApplication::Initialize()                            (gApplication->Initialize)
* Initialize::gMainWindow                                (gApplication->gMainWindow)
* Initialize::lcLoadDefaultKeyboardShortcuts()           (gApplication->Initialize)
* Initialize::lcLoadDefaultMouseShortcuts()              (gApplication->Initialize)
* Initialize::LoadPartsLibrary()                         (gApplication->Initialize)
* LoadPartsLibrary::ldsearchDirPreferences()             (gApplication->Initialize)
* ldsearchDirPreferences::setLDViewExtraSearchDirs()     (gApplication->Initialize) [LDView Renderer], [LDGLite Renderer]
* Initialize::CreateWidgets()                            (gApplication->Initialize)
* gMainWindow::SetColorIndex()                           (gApplication->gMainWindow)
* gMainWindow::GetPartSelectionWidget()                  (gApplication->gMainWindow)
* Gui::initialize()                                      (gui->initialize)
* initialize::populateLdgLiteSearchDirs()                (gui->initialize)          [LDGLite Renderer] Moved
* initialize::createActions                              (gui->initialize)
* initialize::createToolBars                             (gui->initialize)
* initialize::createStatusBar                            (gui->initialize)
* initialize::createDockWindows                          (gui->initialize)
* initialize::toggleLCStatusBar                          (gui->initialize)
*/

    // set theme
    setTheme();

    emit splashMsgSig(QString("20% - %1 GUI window loading...").arg(VER_PRODUCTNAME_STR));

    // initialize gui
    gui = new Gui();

    // Check if preferred renderer set and launch Preference dialogue if not to set Renderer
    gui->getRequireds();

    emit splashMsgSig("30% - 3D Viewer window loading...");

    gApplication = new lcApplication();

    emit splashMsgSig(QString("40% - 3D Viewer initialization..."));

    if (gApplication->Initialize(LibraryPaths, gui)) {
        gui->initialize();
    } else {
        emit gui->messageSig(LOG_ERROR, QString("Unable to initialize 3D Viewer."));
        throw InitException{};
    }
}

void Application::mainApp()
{
    if (m_print_output)
        return;

    emit splashMsgSig(QString("100% - %1 loaded.").arg(VER_PRODUCTNAME_STR));

    availableVersions = new AvailableVersions(this);

    if ((Preferences::enableFadeSteps || Preferences::enableHighlightStep))
        gui->ldrawColorPartsLoad();

    if (modeGUI()) {
        splash->finish(gui);

        Preferences::setLPub3DLoaded();

        gui->show();

        if (!m_commandline_file.isEmpty())
            emit gui->loadFileSig(m_commandline_file);
        else if (Preferences::loadLastOpenedFile){
            gui->loadLastOpenedFile();
        }

#ifndef DISABLE_UPDATE_CHECK
        DoInitialUpdateCheck();
#endif
    }
}

int Application::run()
{
  int ExecReturn = EXIT_FAILURE;
  try
  {
      mainApp();

      emit gui->messageSig(LOG_INFO, QString("Run: Application ready."));

      if (modeGUI())
          ExecReturn = m_application.exec();
      else
          ExecReturn = gui->processCommandLine();

#ifdef Q_OS_WIN
    if (m_allocated_console)
        Sleep(2000);
    SetConsoleTextAttribute (       //return the console to the original attributes
        ConsoleOutput,
        m_currentConsoleAttr);
#endif
  }
  catch(const std::exception& ex)
  {
    emit gui->messageSig(LOG_ERROR, QString("Run: Exception %2 has been thrown.").arg(ex.what()));
  }
  catch(...)
  {
    emit gui->messageSig(LOG_ERROR, QString("Run: An unhandled exception has been thrown."));
  }

  emit gui->messageSig(LOG_INFO, QString("Run: Application terminated with return code %1.").arg(ExecReturn));

  if (!m_print_output)
  {
    delete gMainWindow;
    gMainWindow = nullptr;

    delete gui;
    gui = nullptr;

    delete gApplication;
    gApplication = nullptr;

    delete availableVersions;
    availableVersions = nullptr;

    ldvWidget = nullptr;
  }

  return ExecReturn;
}

void clearCustomPartCache(bool silent)
{
    gui->clearCustomPartCache(silent);
}

void clearAndReloadModelFile()
{
    gui->clearAndReloadModelFile();
}

void reloadCurrentPage(){
    gui->reloadCurrentPage();
}

void restartApplication()
{
    gui->restartApplication();
}

void messageSig(LogType logType, QString message){
    gui->messageSig(logType, message);
}

// Implements the main function here.
ENTRY_POINT
