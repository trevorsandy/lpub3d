/****************************************************************************
**
** Copyright (C) 2015 - 2023 Trevor SANDY. All rights reserved.
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

#include <QDir>
#include <iostream>
#include <QMessageBox>
#include <TCFoundation/TCUserDefaults.h>
#include <LDVQt/LDVWidget.h>
#include <QSslSocket>

#include "application.h"
#include "lpub_preferences.h"
#include "lpub.h"
#include "lpub_object.h"
#include "resolution.h"

#include "updatecheck.h"
#include "QsLogDest.h"

#include "lc_application.h"
#include "lc_profile.h"
#include "lc_context.h"
#include "pieceinf.h"

#define RUN_APPLICATION 2

#ifdef Q_OS_WIN

  #include <stdio.h>
  #include <fcntl.h>
  #include <io.h>
  #include <fstream>
  #include <QtPlatformHeaders\QWindowsWindowFunctions>

  #if (NTDDI_VERSION >= NTDDI_WIN8)
    #include <pathcch.h>
    #pragma comment(lib, "<pathcch.lib")
  #else
    #include <shlwapi.h>
    #pragma comment(lib, "shlwapi.lib")
  #endif

  WCHAR* GetThisPath(WCHAR* dest, size_t destSize)
  {
    if (!dest) return NULL;

    GetModuleFileName(NULL, dest, destSize );

    #if (NTDDI_VERSION >= NTDDI_WIN8)
      PathCchRemoveFileSpec(dest, destSize);
    #else
      if (MAX_PATH > destSize) return NULL;
      PathRemoveFileSpec(dest);
    #endif
      return dest;
  }

  void Application::RedirectIOToConsole()
  {
    // Attach to the existing console of the parent process
    m_allocate_new_console = !AttachConsole( ATTACH_PARENT_PROCESS );
    m_parent_console = ! m_allocate_new_console;
    if (m_allocate_new_console) {
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
    h_ConsoleStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    int SystemOutput = _open_osfhandle(intptr_t(h_ConsoleStdOut), _O_TEXT);
    f_ConsoleStdOut = _fdopen(SystemOutput, "w");

    // Get STDERR handle
    h_ConsoleStdErr = GetStdHandle(STD_ERROR_HANDLE);
    int SystemError = _open_osfhandle(intptr_t(h_ConsoleStdErr), _O_TEXT);
    f_ConsoleStdErr = _fdopen(SystemError, "w");

    // Get STDIN handle
    h_ConsoleStdIn = GetStdHandle(STD_INPUT_HANDLE);
    int SystemInput = _open_osfhandle(intptr_t(h_ConsoleStdIn), _O_TEXT);
    f_ConsoleStdIn = _fdopen(SystemInput, "r");

    //retrieve and save the current attributes
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
                               &ConsoleInfo);
    m_currentConsoleAttr = ConsoleInfo.wAttributes;

    FlushConsoleInputBuffer(h_ConsoleStdOut);

    //change the attribute to what you like
    SetConsoleTextAttribute (
                h_ConsoleStdOut,
                FOREGROUND_RED |
                FOREGROUND_GREEN);

    //make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
    std::ios_base::sync_with_stdio(true);

    // Redirect the CRT standard input, output, and error handles to the console
    freopen_s(&f_ConsoleStdIn, "CONIN$", "r", stdin);
    freopen_s(&f_ConsoleStdOut, "CONOUT$", "w", stdout);
    freopen_s(&f_ConsoleStdErr, "CONOUT$", "w", stderr);

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

  int Application::ReleaseConsole(void)
  {
    if (m_allocate_new_console || m_parent_console) {
      if (m_allocate_new_console)
         Sleep(500);

      // Return the console to the original attributes
      SetConsoleTextAttribute (
         h_ConsoleStdOut,
         m_currentConsoleAttr);

      // This is a hack, but if not used the console doesn't know the application has returned.
      // The "enter" key only sent if the console window is in focus.
      if ((GetConsoleWindow() == GetForegroundWindow())){
        // Send "enter" to release application from the console
        INPUT keyInput;
        // Set up a generic keyboard event.
        keyInput.type = INPUT_KEYBOARD;
        keyInput.ki.wScan = 0; // hardware scan code for key
        keyInput.ki.time = 0;
        keyInput.ki.dwExtraInfo = 0;

        // Send the "Enter" key
        keyInput.ki.wVk = 0x0D; // virtual-key code for the "Enter" key
        keyInput.ki.dwFlags = 0; // 0 for key press
        SendInput(1, &keyInput, sizeof(INPUT));

        // Release the "Enter" key
        keyInput.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
        SendInput(1, &keyInput, sizeof(INPUT));
      }

      // Functions used are compatible with Win2000 or greater
      if (QSysInfo::windowsVersion() >= QSysInfo::WV_2000)
        FreeConsole();

      m_allocate_new_console = false;
      m_parent_console = false;
    }

    return EXIT_SUCCESS;
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

  static WCHAR gApplicationPath[MAX_PATH];
  static WCHAR gMinidumpPath[_MAX_PATH];
  static BOOL  gConsoleMode = false;

  LONG WINAPI Application::lcSehHandler(PEXCEPTION_POINTERS exceptionPointers)
  {
      if (IsDebuggerPresent())
          return EXCEPTION_CONTINUE_SEARCH;

      HMODULE dbgHelp = LoadLibrary(TEXT("dbghelp.dll"));

      if (dbgHelp == NULL)
          return EXCEPTION_EXECUTE_HANDLER;

      HANDLE file = CreateFile(gMinidumpPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

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
          WCHAR message[_MAX_PATH + 256];
          lstrcpy(message, TEXT(VER_PRODUCTNAME_STR " crashed. Crash information was saved to '"));
          lstrcat(message, gMinidumpPath);
          lstrcat(message, TEXT("', please send it along with files " VER_PRODUCTNAME_STR ".exe, " VER_PRODUCTNAME_STR ".pdb located at '"));
          lstrcat(message, gApplicationPath);
          lstrcat(message, TEXT("' to the developer for debugging."));

          if (gConsoleMode)
          {
              fprintf(stdout, "%ls\n", message);
              fflush(stdout);
          }
          else
          {
              MessageBox(NULL, message, TEXT(VER_PRODUCTNAME_STR " Crashed!"), MB_ICONSTOP | MB_OK);
          }
      }

      return EXCEPTION_EXECUTE_HANDLER;
  }

  void Application::lcSehInit()
  {
      if (GetTempPath(sizeof(gMinidumpPath) / sizeof(gMinidumpPath[0]), gMinidumpPath))
      {
          lstrcat(gMinidumpPath, TEXT(VER_PRODUCTNAME_STR ".dmp"));

          GetThisPath(gApplicationPath, MAX_PATH);
      }

      SetUnhandledExceptionFilter(lcSehHandler);
  }
#endif

// Initializes the Application instance as null
Application* Application::m_instance = nullptr;

Application::Application(int &argc, char **argv)
  : m_application(argc, argv)
{
  Preferences::setDistribution();
  m_instance = this;
}

Application::~Application()
{ }

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

QString Application::getTheme()
{
    return m_theme;
}

void Application::setTheme(bool appStarted)
{
  m_theme = Preferences::displayTheme;
  lcColorTheme visualEditorTheme = static_cast<lcColorTheme>(lcGetProfileInt(LC_PROFILE_COLOR_THEME));
  bool darkTheme = visualEditorTheme == lcColorTheme::Dark;
  bool setDarkTheme = m_theme == THEME_DARK;

  auto getColorFromHex = [] (const QString &hexColor, int alpha = 255)
  {
      QColor c = QColor(hexColor);
      return quint32(LC_RGBA(c.red(), c.green(), c.blue(), alpha));
  };

  auto setThemeColor = [&appStarted, &getColorFromHex, &darkTheme, &setDarkTheme] (
          LC_PROFILE_KEY key, QString darkHex, QString defaultHex, int alpha = 255) {
      // check if is lclib object colour
      bool lcObjectColor = false;
      switch (key)
      {
      case LC_PROFILE_OBJECT_SELECTED_COLOR:
      case LC_PROFILE_OBJECT_FOCUSED_COLOR:
      case LC_PROFILE_CAMERA_COLOR:
      case LC_PROFILE_LIGHT_COLOR:
      case LC_PROFILE_CONTROL_POINT_COLOR:
      case LC_PROFILE_CONTROL_POINT_FOCUSED_COLOR:
      case LC_PROFILE_BM_OBJECT_SELECTED_COLOR:
          lcObjectColor = true;
          break;
      default:
          break;
      }
      // get current viewer theme color (before change)
      quint32 pc = quint32(lcObjectColor ? lcGetProfileUInt(key) : lcGetProfileInt(key));
      // get corresponding default theme color (before change)
      quint32 tc = getColorFromHex(darkTheme ? darkHex : defaultHex, alpha);
      // set visual editor theme color to new theme color if color not user specified - i.e. current color is a theme color
      if (pc == tc) {
          // get new theme color
          tc = getColorFromHex(setDarkTheme ? darkHex : defaultHex, alpha);
          // set view profile key
          if (lcObjectColor)
              lcSetProfileUInt(key, int(tc));
          else
              lcSetProfileInt(key, int(tc));

          // set preference property if applicaiton already started - i.e. change preference
          if (appStarted) {
              switch (key)
              {
              case LC_PROFILE_AXES_COLOR:
                  lcGetPreferences().mAxesColor = tc;
                  break;
              case LC_PROFILE_OVERLAY_COLOR:
                  lcGetPreferences().mOverlayColor = tc;
                  break;
              case LC_PROFILE_MARQUEE_BORDER_COLOR:
                  lcGetPreferences().mMarqueeBorderColor = tc;
                  break;
              case LC_PROFILE_MARQUEE_FILL_COLOR:
                  lcGetPreferences().mMarqueeFillColor = tc;
                  break;
              case LC_PROFILE_INACTIVE_VIEW_COLOR:
                  lcGetPreferences().mInactiveViewColor = tc;
                  break;
              case LC_PROFILE_ACTIVE_VIEW_COLOR:
                  lcGetPreferences().mActiveViewColor = tc;
                  break;
              case LC_PROFILE_GRID_STUD_COLOR:
                  lcGetPreferences().mGridStudColor = tc;
                  break;
              case LC_PROFILE_GRID_LINE_COLOR:
                  lcGetPreferences().mGridLineColor = tc;
                  break;
              case LC_PROFILE_TEXT_COLOR:
                  lcGetPreferences().mGridLineColor = tc;
                  break;
              case LC_PROFILE_VIEW_SPHERE_COLOR:
                  lcGetPreferences().mViewSphereColor = tc;
                  break;
              case LC_PROFILE_VIEW_SPHERE_TEXT_COLOR:
                  lcGetPreferences().mViewSphereTextColor = tc;
                  break;
              case LC_PROFILE_VIEW_SPHERE_HIGHLIGHT_COLOR:
                  lcGetPreferences().mViewSphereHighlightColor = tc;
                  break;
              case LC_PROFILE_OBJECT_SELECTED_COLOR:
                  lcGetPreferences().mObjectSelectedColor = tc;
                  break;
              case LC_PROFILE_OBJECT_FOCUSED_COLOR:
                  lcGetPreferences().mObjectFocusedColor = tc;
                  break;
              case LC_PROFILE_CAMERA_COLOR:
                  lcGetPreferences().mCameraColor = tc;
                  break;
              case LC_PROFILE_LIGHT_COLOR:
                  lcGetPreferences().mLightColor = tc;
                  break;
              case LC_PROFILE_CONTROL_POINT_COLOR:
                  lcGetPreferences().mControlPointColor = tc;
                  break;
              case LC_PROFILE_CONTROL_POINT_FOCUSED_COLOR:
                  lcGetPreferences().mControlPointFocusedColor = tc;
                  break;
              case LC_PROFILE_BM_OBJECT_SELECTED_COLOR:
                  lcGetPreferences().mBMObjectSelectedColor = tc;
                  break;

              case LC_PROFILE_BACKGROUND_COLOR:
                  lcGetPreferences().mBackgroundSolidColor = tc;
                  break;
              case LC_PROFILE_GRADIENT_COLOR_TOP:
                  lcGetPreferences().mBackgroundGradientColorTop = tc;
                  break;
              case LC_PROFILE_GRADIENT_COLOR_BOTTOM:
                  lcGetPreferences().mBackgroundGradientColorBottom = tc;
                  break;
              default:
                  break;
              }
          }
      }
  };

  if (setDarkTheme){

      if (!QApplication::setStyle("fusion"))
          return;

      // Set palette
      QPalette Palette = QApplication::palette();

      Palette.setColor(QPalette::Window, QColor(Preferences::themeColors[THEME_DARK_PALETTE_WINDOW]));                 // 49, 52, 55,    #313437
      Palette.setColor(QPalette::WindowText, QColor(Preferences::themeColors[THEME_DARK_PALETTE_WINDOW_TEXT]));        // 240, 240, 240  #F0F0F0
      Palette.setColor(QPalette::Base, QColor(Preferences::themeColors[THEME_DARK_PALETTE_BASE]));                     // 35, 38, 41     #232629
      Palette.setColor(QPalette::AlternateBase, QColor(Preferences::themeColors[THEME_DARK_PALETTE_ALT_BASE]));        // 44, 47, 50     #2C2F32
      Palette.setColor(QPalette::ToolTipBase, QColor(Preferences::themeColors[THEME_DARK_PALETTE_TIP_BASE]));          // 224, 224, 244  #E0E0F4
      Palette.setColor(QPalette::ToolTipText, QColor(Preferences::themeColors[THEME_DARK_PALETTE_TIP_TEXT]));          // 58, 58, 58     #3A3A3A
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
      Palette.setColor(QPalette::PlaceholderText, QColor(Preferences::themeColors[THEME_DARK_PALETTE_PHOLDER_TEXT]));  // 100, 100, 100  #646464
#endif
      Palette.setColor(QPalette::Text, QColor(Preferences::themeColors[THEME_DARK_PALETTE_TEXT]));                     // 224, 224, 224  #E0E0E0
      Palette.setColor(QPalette::Button, QColor(Preferences::themeColors[THEME_DARK_PALETTE_BUTTON]));                 // 45, 48, 51     #2D3033
      Palette.setColor(QPalette::ButtonText, QColor(Preferences::themeColors[THEME_DARK_PALETTE_BUTTON_TEXT]));        // 224, 224, 244  #E0E0F4
      Palette.setColor(QPalette::Light, QColor(Preferences::themeColors[THEME_DARK_PALETTE_LIGHT]));                   // 65, 65, 65     #414141
      Palette.setColor(QPalette::Midlight, QColor(Preferences::themeColors[THEME_DARK_PALETTE_MIDLIGHT]));             // 62, 62, 62     #3E3E3E
      Palette.setColor(QPalette::Dark, QColor(Preferences::themeColors[THEME_DARK_PALETTE_DARK]));                     // 35, 35, 35     #232323
      Palette.setColor(QPalette::Mid, QColor(Preferences::themeColors[THEME_DARK_PALETTE_MID]));                       // 50, 50, 50     #323232
      Palette.setColor(QPalette::Shadow, QColor(Preferences::themeColors[THEME_DARK_PALETTE_SHADOW]));                 // 20, 20, 20     #141414
      Palette.setColor(QPalette::Highlight, QColor( Preferences::themeColors[THEME_DARK_PALETTE_HILIGHT]));            // 41, 128, 185   #2980B9
      Palette.setColor(QPalette::HighlightedText, QColor(Preferences::themeColors[THEME_DARK_PALETTE_HILIGHT_TEXT]));  // 232, 232, 232  #E8E8E8
      Palette.setColor(QPalette::Link, QColor(Preferences::themeColors[THEME_DARK_PALETTE_LINK]));                     // 41, 128, 185   #2980B9

      Palette.setColor(QPalette::Disabled, QPalette::Text, QColor(Preferences::themeColors[THEME_DARK_PALETTE_DISABLED_TEXT]));       // 128, 128, 128 #808080
      Palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(Preferences::themeColors[THEME_DARK_PALETTE_DISABLED_TEXT])); // 128, 128, 128 #808080
      Palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(Preferences::themeColors[THEME_DARK_PALETTE_DISABLED_TEXT])); // 128, 128, 128 #808080

      QApplication::setPalette(Palette);

      // Set dark style settings
      QFile styleSheetFile(QLatin1String(":/stylesheet/stylesheet.qss"));
      if (styleSheetFile.open(QIODevice::ReadOnly)) {
          QString stylesheet = QString::fromLatin1(styleSheetFile.readAll());
          qApp->setStyleSheet(stylesheet);
          visualEditorTheme = lcColorTheme::Dark;
      } else {
          QString styleSheetMessage = tr("Dark mode styleSheet. %1 (%2)")
                                         .arg(styleSheetFile.errorString())
                                         .arg(styleSheetFile.fileName());

          if (modeGUI()) {
              logInfo() << styleSheetMessage;
          } else {
              Preferences::fprintMessage(QString(styleSheetMessage.append("\n")));
          }
      }
    }
  else
  if (m_theme == THEME_DEFAULT){
      // Set default style settings
      QApplication::setStyle(QApplication::style()->objectName());
      QApplication::setPalette(qApp->style()->standardPalette());
      qApp->setStyleSheet( QString() );
      visualEditorTheme = lcColorTheme::System;
    }

  // Set default scene colours
  if (!Preferences::customSceneBackgroundColor)
      Preferences::setSceneBackgroundColorPreference(
                   setDarkTheme ? Preferences::themeColors[THEME_DARK_SCENE_BACKGROUND_COLOR] :
                                  Preferences::themeColors[THEME_DEFAULT_SCENE_BACKGROUND_COLOR]);
  if (!Preferences::customSceneGridColor)
      Preferences::setSceneGridColorPreference(
                   setDarkTheme ? Preferences::themeColors[THEME_DARK_GRID_PEN] :
                                  Preferences::themeColors[THEME_DEFAULT_GRID_PEN]);
  if (!Preferences::customSceneRulerTickColor)
      Preferences::setSceneRulerTickColorPreference(
                   setDarkTheme ? Preferences::themeColors[THEME_DARK_RULER_TICK_PEN] :
                                  Preferences::themeColors[THEME_DEFAULT_RULER_TICK_PEN]);
  if (!Preferences::customSceneRulerTrackingColor)
      Preferences::setSceneRulerTrackingColorPreference(
                   setDarkTheme ? Preferences::themeColors[THEME_DARK_RULER_TRACK_PEN] :
                                  Preferences::themeColors[THEME_DEFAULT_RULER_TRACK_PEN]);
  if (!Preferences::customSceneGuideColor)
      Preferences::setSceneGuideColorPreference(
                   setDarkTheme ? Preferences::themeColors[THEME_DARK_GUIDE_PEN] :
                                  Preferences::themeColors[THEME_DEFAULT_GUIDE_PEN]);

  // Set Visual Editor interface colours
  setThemeColor(LC_PROFILE_AXES_COLOR,
                Preferences::themeColors[THEME_DARK_AXES_COLOR],
                Preferences::themeColors[THEME_DEFAULT_AXES_COLOR]);
  setThemeColor(LC_PROFILE_OVERLAY_COLOR,
                Preferences::themeColors[THEME_DARK_OVERLAY_COLOR],
                Preferences::themeColors[THEME_DEFAULT_OVERLAY_COLOR]);
  setThemeColor(LC_PROFILE_MARQUEE_BORDER_COLOR,
                Preferences::themeColors[THEME_DARK_MARQUEE_BORDER_COLOR],
                Preferences::themeColors[THEME_DEFAULT_MARQUEE_BORDER_COLOR]);

  setThemeColor(LC_PROFILE_MARQUEE_FILL_COLOR,
                Preferences::themeColors[THEME_DARK_MARQUEE_FILL_COLOR],
                Preferences::themeColors[THEME_DEFAULT_MARQUEE_FILL_COLOR]);

  setThemeColor(LC_PROFILE_INACTIVE_VIEW_COLOR,
                Preferences::themeColors[THEME_DARK_INACTIVE_VIEW_COLOR],
                Preferences::themeColors[THEME_DEFAULT_INACTIVE_VIEW_COLOR]);
  setThemeColor(LC_PROFILE_ACTIVE_VIEW_COLOR,
                Preferences::themeColors[THEME_DARK_ACTIVE_VIEW_COLOR],
                Preferences::themeColors[THEME_DEFAULT_ACTIVE_VIEW_COLOR]);
  setThemeColor(LC_PROFILE_GRID_STUD_COLOR,
                Preferences::themeColors[THEME_DARK_GRID_STUD_COLOR],
                Preferences::themeColors[THEME_DEFAULT_GRID_STUD_COLOR], 192/*alpha*/);
  setThemeColor(LC_PROFILE_GRID_LINE_COLOR,
                Preferences::themeColors[THEME_DARK_GRID_LINE_COLOR],
                Preferences::themeColors[THEME_DEFAULT_GRID_LINE_COLOR]);
  setThemeColor(LC_PROFILE_TEXT_COLOR,
                Preferences::themeColors[THEME_DARK_TEXT_COLOR],
                Preferences::themeColors[THEME_DEFAULT_TEXT_COLOR]);
  setThemeColor(LC_PROFILE_VIEW_SPHERE_COLOR,
                Preferences::themeColors[THEME_DARK_VIEW_SPHERE_COLOR],
                Preferences::themeColors[THEME_DEFAULT_VIEW_SPHERE_COLOR]);
  setThemeColor(LC_PROFILE_VIEW_SPHERE_TEXT_COLOR,
                Preferences::themeColors[THEME_DARK_VIEW_SPHERE_TEXT_COLOR],
                Preferences::themeColors[THEME_DEFAULT_VIEW_SPHERE_TEXT_COLOR]);
  setThemeColor(LC_PROFILE_VIEW_SPHERE_HIGHLIGHT_COLOR,
                Preferences::themeColors[THEME_DARK_VIEW_SPHERE_HLIGHT_COLOR],
                Preferences::themeColors[THEME_DEFAULT_VIEW_SPHERE_HLIGHT_COLOR]);

  setThemeColor(LC_PROFILE_OBJECT_SELECTED_COLOR,
                Preferences::themeColors[THEME_DARK_OBJECT_SELECTED_COLOR],
                Preferences::themeColors[THEME_DEFAULT_OBJECT_SELECTED_COLOR]);
  setThemeColor(LC_PROFILE_OBJECT_FOCUSED_COLOR,
                Preferences::themeColors[THEME_DARK_OBJECT_FOCUSED_COLOR],
                Preferences::themeColors[THEME_DEFAULT_OBJECT_FOCUSED_COLOR]);
  setThemeColor(LC_PROFILE_CAMERA_COLOR,
                Preferences::themeColors[THEME_DARK_CAMERA_COLOR],
                Preferences::themeColors[THEME_DEFAULT_CAMERA_COLOR]);
  setThemeColor(LC_PROFILE_LIGHT_COLOR,
                Preferences::themeColors[THEME_DARK_LIGHT_COLOR],
                Preferences::themeColors[THEME_DEFAULT_LIGHT_COLOR]);
  setThemeColor(LC_PROFILE_CONTROL_POINT_COLOR,
                Preferences::themeColors[THEME_DARK_CONTROL_POINT_COLOR],
                Preferences::themeColors[THEME_DEFAULT_CONTROL_POINT_COLOR]);
  setThemeColor(LC_PROFILE_CONTROL_POINT_FOCUSED_COLOR,
                Preferences::themeColors[THEME_DARK_CONTROL_POINT_FOCUSED_COLOR],
                Preferences::themeColors[THEME_DEFAULT_CONTROL_POINT_FOCUSED_COLOR]);
  setThemeColor(LC_PROFILE_BM_OBJECT_SELECTED_COLOR,
                Preferences::themeColors[THEME_DARK_BM_OBJECT_SELECTED_COLOR],
                Preferences::themeColors[THEME_DEFAULT_BM_OBJECT_SELECTED_COLOR]);

  setThemeColor(LC_PROFILE_BACKGROUND_COLOR,
                Preferences::themeColors[THEME_DARK_VIEWER_BACKGROUND_COLOR],
                Preferences::themeColors[THEME_DEFAULT_VIEWER_BACKGROUND_COLOR]);
  setThemeColor(LC_PROFILE_GRADIENT_COLOR_TOP,
                Preferences::themeColors[THEME_DARK_VIEWER_GRADIENT_COLOR_TOP],
                Preferences::themeColors[THEME_DEFAULT_VIEWER_GRADIENT_COLOR_TOP]);
  setThemeColor(LC_PROFILE_GRADIENT_COLOR_BOTTOM,
                Preferences::themeColors[THEME_DARK_VIEWER_GRADIENT_COLOR_BOTTOM],
                Preferences::themeColors[THEME_DEFAULT_VIEWER_GRADIENT_COLOR_BOTTOM]);

  // Set Visual Editor colour theme - apply after interface colour update
  lcSetProfileInt(LC_PROFILE_COLOR_THEME, static_cast<int>(visualEditorTheme));
}

void Application::splashMsg(const QString &message)
{
  Preferences::setMessageLogging(true/*useLogLevel*/);
  logInfo() << qUtf8Printable(message);
  Preferences::setMessageLogging();
  if (m_console_mode)
      return;
  splash->showMessage(QSplashScreen::tr(message.toLatin1().constData()),Qt::AlignBottom | Qt::AlignLeft, QColor(QString(SPLASH_FONT_COLOUR)));
  m_application.processEvents();
}

int Application::initialize(lcCommandLineOptions &Options)
{
#ifdef Q_OS_MAC
    m_application.setStyle(QStyleFactory::create("macintosh"));
#endif
    m_console_mode = false;
    m_print_output = false;
#ifdef Q_OS_WIN
    m_allocate_new_console = false;
    m_parent_console = false;
#endif

    connect(this, SIGNAL(splashMsgSig(const QString &)),
            this, SLOT(  splashMsg(   const QString &)));

    // process arguments
    bool header_printed = false;
    bool suppressFPrint = false;
#ifdef Q_OS_WIN
    // Request console redirect
    bool consoleRedirect = true;
    bool consoleRedirectTreated = false;
#endif
    QString hdr, args, ldrawLibrary;
    const int NumArgsIdx = arguments().size() - 1;

#if defined LP3D_CONTINUOUS_BUILD || defined LP3D_DEVOPS_BUILD || defined LP3D_NEXT_BUILD
    hdr = QString("%1 v%2 r%3 (%4) for %5")
                  .arg(QString::fromLatin1(VER_PRODUCTNAME_STR), QString::fromLatin1(VER_PRODUCTVERSION_STR), QString::fromLatin1(VER_REVISION_STR), QString::fromLatin1(VER_BUILD_TYPE_STR), QString::fromLatin1(VER_COMPILED_FOR));
#else
    int rev = QString::fromLatin1(VER_REVISION_STR).toInt();
    hdr = QString("%1 v%2%3 for %4")
                 .arg(QString::fromLatin1(VER_PRODUCTNAME_STR), QString::fromLatin1(VER_PRODUCTVERSION_STR), rev ? QString(" r%1").arg(VER_REVISION_STR) : "", QString::fromLatin1(VER_COMPILED_FOR));
#endif

    args = QString("Arguments:");
    for (int i = 1; i < arguments().size(); i++)
        args.append(" " + arguments().at(i));

    for (int ArgIdx = 1; ArgIdx < arguments().size(); ArgIdx++)
    {
        const QString Param = arguments().at(ArgIdx);
        // Items in this condition execute in GUI mode - first item executes in command console mode also)
        if (Param[0] != '-')
        {
            bool EmptyFileString = m_commandline_file.isEmpty();
            bool ArgFileExists   = QFileInfo(Param).exists();
            if (EmptyFileString && ArgFileExists && !m_console_mode)
                m_commandline_file = Param;

            if (Param == QLatin1String("+cr") || Param == QLatin1String("++console-redirect"))
            {
#ifdef Q_OS_WIN
                if (! consoleRedirectTreated) {
                    consoleRedirectTreated = true;
                    RedirectIOToConsole();
                }
#else
                fprintf(stdout, "%s\n", qUtf8Printable(tr("+cr/++console-redirect is only supported on Windows.\n")));
#endif
            }
            if (!header_printed) {
                header_printed = true;
                fprintf(stdout, "%s", qUtf8Printable(tr("\n%1\n").arg(hdr)));
                fprintf(stdout, "==========================\n");
                fprintf(stdout, "%s", qUtf8Printable(tr("%1\n").arg(args)));
                fflush(stdout);
            }

            // Invoke LEGO library in GUI mode
            if (Param == QLatin1String("+ll") || Param == QLatin1String("++liblego"))
                ldrawLibrary = LEGO_LIBRARY;
            else
            // Invoke TENTE library in GUI mode
            if (Param == QLatin1String("+lt") || Param == QLatin1String("++libtente"))
                ldrawLibrary = TENTE_LIBRARY;
            else
            // Invoke VEXIQ library in GUI mode
            if (Param == QLatin1String("+lv") || Param == QLatin1String("++libvexiq"))
                ldrawLibrary = VEXIQ_LIBRARY;
            // Everything after this occurs in console mode so escape here.
            continue;
        }
        // Items in this condition will execute in command console mode
        else
        {
            if (!header_printed) {
                header_printed = true;
                m_console_mode = true;
#ifdef Q_OS_WIN
                if (! consoleRedirectTreated) {
                    consoleRedirectTreated = true;
                    for (const QString &argument : arguments()) {
                        if (argument == QLatin1String("-ncr") ||
                            argument == QLatin1String("--no-console-redirect")) {
                            consoleRedirect = false;
                            break;
                        }
                    }
                    if (consoleRedirect)
                        RedirectIOToConsole();
                }
#endif
                fprintf(stdout, "%s", qUtf8Printable(tr("\n%1\n").arg(hdr)));
                fprintf(stdout, "==========================\n");
                fprintf(stdout, "%s", qUtf8Printable(tr("%1\n").arg(args)));
                fflush(stdout);
            }

            if (Param == QLatin1String("-ns") || Param == QLatin1String("--no-stdout-log"))
                suppressFPrint = true;
            else
            // Version output
            if (Param == QLatin1String("-v") || Param == QLatin1String("--version"))
            {
                m_console_mode = true;
                m_print_output = true;
                fprintf(stdout, "%s", qUtf8Printable(tr("\n%1, %2 %3, Revision %4, Commit %5, SHA %6\n").arg(VER_PRODUCTNAME_STR).arg(VER_BUILD_TYPE_STR).arg(VER_PRODUCTVERSION_STR).arg(VER_REVISION_STR).arg(VER_COMMIT_STR).arg(VER_GIT_SHA_STR)));
                fprintf(stdout, "%s", qUtf8Printable(tr("Compiled on %1\n").arg(__DATE__)));
                fflush(stdout);
                if (ArgIdx == NumArgsIdx) {
#ifdef Q_OS_WIN
                    return  ReleaseConsole();
#else
                    return EXIT_SUCCESS;
#endif
              }
            }
            else
            // Visual Editor (LeoCAD) version output
            if (Param == QLatin1String("-ve") || Param == QLatin1String("--visual-editor-version"))
            {
                m_console_mode = true;
                m_print_output = true;
                fprintf(stdout, "%s", qUtf8Printable(tr("Visual Editor - by LeoCAD, Version %1, SHA %2\n").arg(LC_VERSION_TEXT).arg(LC_VERSION_SHA)));
                fprintf(stdout, "%s", qUtf8Printable(tr("Compiled %1\n").arg(__DATE__)));
                fflush(stdout);
                if (ArgIdx == NumArgsIdx) {
#ifdef Q_OS_WIN
                    return  ReleaseConsole();
#else
                    return EXIT_SUCCESS;
#endif
              }
            }
            else
            // Help output
            if (Param == QLatin1String("-?") || Param == QLatin1String("--help"))
            {
                m_console_mode = true;
                m_print_output = true;
                fprintf(stdout, "%s", qUtf8Printable(tr("Usage: %1 [options] [LDraw file]\n").arg(qApp->applicationName())));
                fprintf(stdout, "%s", qUtf8Printable(tr("  [LDraw file]:\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  Use absolute file path and file name with .ldr, .mpd or .dat extension.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  [options]:\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  Options preceded by '+/++' are only applicable to GUI mode. They have no effect in Console mode.\n")));
                fprintf(stdout, "\n");
                fprintf(stdout, "%s", qUtf8Printable(tr("[%1 commands]\n").arg(qApp->applicationName())));
#ifdef Q_OS_WIN
                fprintf(stdout, "%s", qUtf8Printable(tr("  +cr, ++console-redirect: Create console to redirect standard output standard error and standard input. Default is off.\n")));
#endif
                fprintf(stdout, "%s", qUtf8Printable(tr("  +ll, ++liblego: Load the LDraw LEGO archive parts library in GUI mode.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  +lt, ++libtente: Load the LDraw TENTE archive parts library in GUI mode.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  +lv, ++libvexiq: Load the LDraw VEXIQ archive parts library in GUI mode.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -aec, --automate-edge-color: Enable automatically adjusted edge colors.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -bec, --black-edge-color <#AARRGGBB>: High contrast edge color for black parts.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -cc, --color-contrast <decimal>: Color contrast value between 0.0 and 1.0.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -ccf, --color-config-file <file>: Full path for alternate LDConfig colour file.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -dec, --dark-edge-color <#AARRGGBB>: High contrast edge color for dark color parts.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -ec, --edge-color <#AARRGGBB>: High contrast edge color.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -emc, --export-meta-commands <file>: Export LPub meta commands to specified file.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -fc, --fade-steps-color <LDraw_colour_name>: Set the global fade color. Overridden by fade opacity - if opacity not 100 percent. Default is %1\n").arg(LEGO_FADE_COLOUR_DEFAULT)));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -fo, --fade-step-opacity <percent>: Set the fade steps opacity percent. Overrides fade color - if opacity not 100 percent. Default is %1 percent\n").arg(FADE_OPACITY_DEFAULT)));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -fs, --fade-steps: Turn on fade previous steps. Default is off.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -hc, --highlight-step-color <#AARRGGBB>: Set the step highlight color. Color code optional. Format is #RRGGBB. Default is %1.\n").arg(HIGHLIGHT_COLOUR_DEFAULT)));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -hs, --highlight-step: Turn on highlight current step. Default is off.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -hw, --highlight-line-width: Set the width of the highlighted edge lines - Enabled for LDGlite renderer only. Default is 1.0f.\n")));
//              fprintf(stdout, "%s", qUtf8Printable(tr("  -im, --image-matte: [Experimental] Turn on image matting for fade previous step. Combine current and previous images using pixel blending - LDView only. Default is off.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -ldv, --light-dark-value <decimal>: Light/Dark color value between 0.0 and 1.0.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -ll, --liblego: Load the LDraw LEGO archive parts library in command console mode.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -lt, --libtente: Load the LDraw TENTE archive parts library in command console mode.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -lv, --libvexiq: Load the LDraw VEXIQ archive parts library in command console mode.\n")));
#ifdef Q_OS_WIN
                fprintf(stdout, "%s", qUtf8Printable(tr("..-ncr, --no-console-redirect: Do not automatically redirect output and errors to console. Useful when running headless. Default is off.\n")));
#endif
                fprintf(stdout, "%s", qUtf8Printable(tr("  -ns, --no-stdout-log: Do not enable standard output for logged entries. Useful to prevent double (stdout and QSLog) output. Default is off.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -o, --export-option <option>: Set output format pdf, png, jpeg, bmp, stl, 3ds, pov, csv, bl-xml, dae, htmlparts, htmlsteps, or obj. Used with process-export. Default is pdf.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -od, --export-directory <path>: Designate the export option save folder using absolute path. Used with export-option. Default is model file folder\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -of, --output-file/--pdf-output-file <path>: Designate the document save file using absolute path.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -p, --preferred-renderer <renderer>: Set renderer native, ldglite, ldview, ldview-sc, ldview-scsl, povray, or povray-ldv. Default is native.\n ")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -pe, --process-export: Export instruction document, images, files, or objects. Used with export-option. Default is pdf document.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -pf, --process-file: Process ldraw model file and generate images in png format.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -pr, --projection <p,perspective|o,orthographic>: Set camera projection.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -r, --range <page range>: Set page range - e.g. 1,2,9,10-42. Default is all pages.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -rs, --reset-search-dirs: Reset the LDraw parts directories to those searched by default. Default is off.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -scc, --stud-cylinder-color <#AARRGGBB>: High contrast stud cylinder color.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -ss, --stud-style <id>: Set the stud style 0=Plain, 1=Thin Lines Logo, 2=Outline Logo, 3=Sharp Top Logo, 4=Rounded Top Logo, 5=Flattened Logo, 6=High Contrast, 7=High Contrast with Logo.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -v, --version: Output LPub3D version information and exit.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -x, --clear-cache: Reset the LDraw file and image caches. Used with export-option change. Default is off.\n")));
                fprintf(stdout, "\n");
                fprintf(stdout, "%s", qUtf8Printable(tr("[Visual Editor commands]\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -c, --camera <camera>: Set the active camera.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("..-cl, --draw-conditional-lines: Enable draw conditional lines.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -f, --from <step>: Set the first step to save pictures.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -h, --height <height>: Set the picture height.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -i, --image <outfile.ext>: Save a picture in the format specified by ext.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -s, --submodel <submodel>: Set the active submodel.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -t, --to <step>: Set the last step to save pictures.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -w, --width <width>: Set the picture width.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -ve, --visual-editor-version: Output Visual Editor - by LeoCAD version information and exit.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -3ds, --export-3ds <outfile.3ds>: Export the model to 3D Studio 3DS format.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -dae, --export-collada <outfile.dae>: Export the model to COLLADA DAE format.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -csv, --export-csv <outfile.csv>: Export the list of parts used in csv format.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -html, --export-html <folder>: Create an HTML page for the model.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -obj, --export-wavefront <outfile.obj>: Export the model to Wavefront OBJ format.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  --aa-samples <count>: AntiAliasing sample size (1, 2, 4, or 8).\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  --camera-angles <latitude> <longitude>: Set the camera angles in degrees around the model.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  --camera-position <x> <y> <z> <tx> <ty> <tz> <ux> <uy> <uz>: Set the camera position, target and up vector.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  --camera-position-ldraw <x> <y> <z> <tx> <ty> <tz> <ux> <uy> <uz>: Set the camera position, target and up vector using LDraw coordinates.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  --fov <degrees>: Set the vertical field of view used to render images (< 180).\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  --zplanes <near> <far>: Set the near and far clipping planes used to render images (1 <= <near> < <far>).\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  --fade-steps: Render parts from prior steps faded.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  --no-fade-steps: Do not render parts from prior steps faded.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  --fade-steps-color <rgba>: Renderinng color for prior step parts (#AARRGGBB).\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  --highlight: Highlight parts in the steps they appear.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  --no-highlight: Do not highlight parts in the steps they appear.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  --highlight-color: Renderinng color for highlighted parts (#AARRGGBB).\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  --line-width <width>: Set the width of the edge lines.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  --shading <wireframe|flat|default|full>: Select shading mode for rendering.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  --viewpoint <front|back|left|right|top|bottom|home>: Set the viewpoint.\n")));
                fprintf(stdout, "\n");
                fprintf(stdout, "%s", qUtf8Printable(tr("[Help]\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -?, --help: Display this help message and exit.\n")));
                fprintf(stdout, "%s", qUtf8Printable(tr("  -ap, --app-paths: Display application data, cache and configuration paths.\n")));
                fprintf(stdout, "\n");
                fflush(stdout);
                if (ArgIdx == NumArgsIdx) {
#ifdef Q_OS_WIN
                    return  ReleaseConsole();
#else
                    return EXIT_SUCCESS;
#endif
                }
            }

            // Invoke LEGO library in command console mode
            if (Param == QLatin1String("-ll") || Param == QLatin1String("--liblego"))
                ldrawLibrary = LEGO_LIBRARY;
            else
            // Invoke TENTE library in command console mode
            if (Param == QLatin1String("-lt") || Param == QLatin1String("--libtente"))
                ldrawLibrary = TENTE_LIBRARY;
            else
            // Invoke VEXIQ library in command console mode
            if (Param == QLatin1String("-lv") || Param == QLatin1String("--libvexiq"))
                ldrawLibrary = VEXIQ_LIBRARY;
            else
            // Display application data, cache and configuration paths
            if (Param == QLatin1String("-ap") || Param == QLatin1String("--app-paths")) {
                // Set loaded library flags and variables
                Preferences::setLPub3DAltLibPreferences(ldrawLibrary);
                // initialize directories
                Preferences::lpubPreferences();
                if (ArgIdx == NumArgsIdx) {
#ifdef Q_OS_WIN
                    return  ReleaseConsole();
#else
                    return EXIT_SUCCESS;
#endif
                }
            }
        }
    }

    // Set loaded library flags and variables
    Preferences::setLPub3DAltLibPreferences(ldrawLibrary);

    // initialize directories
    Preferences::lpubPreferences();

    // initialize the logger
    Preferences::loggingPreferences();

    // Do not log to standard output (fprint) - usually disabled on Windows and enabled on Unix when logging is enabled
    Preferences::setSuppressFPrintPreference(suppressFPrint);

    Preferences::printInfo(tr("Initializing application %1...").arg(VER_PRODUCTNAME_STR));

    // application version information
    Preferences::printInfo("-----------------------------");
    Preferences::printInfo(hdr);
    Preferences::printInfo("=============================");
    Preferences::printInfo(tr("Arguments....................(%1)").arg(args));
#ifndef Q_OS_WIN
    QDir cwd(QCoreApplication::applicationDirPath());
#ifdef Q_OS_MAC           // for macOS
    Preferences::printInfo(tr("macOS Binary Directory.......(%1)").arg(cwd.dirName()));
    if (cwd.dirName() == "MacOS") {   // MacOS/         (app bundle executable folder)
        cwd.cdUp();                   // Contents/      (app bundle contents folder)
        cwd.cdUp();                   // LPub3D.app/    (app bundle folder)
        cwd.cdUp();                   // Applications/  (app bundle installation folder)
    }
    Preferences::printInfo(tr("macOS Base Directory.........(%1)").arg(QDir::toNativeSeparators(cwd.dirName())));
    if (QCoreApplication::applicationName() != QString(VER_PRODUCTNAME_STR))
    {
        Preferences::printInfo(tr("macOS Info.plist update......(%1)").arg(Preferences::lpub3dAppName));
        QFileInfo plbInfo("/usr/libexec/PlistBuddy");
        if (!plbInfo.exists())
            Preferences::printInfo(tr("ERROR - %1 not found, cannot update Info.Plist").arg(plbInfo.absoluteFilePath()));
    }
#elif defined Q_OS_LINUX   // for Linux
    QDir progDir(QString("%1/../share").arg(cwd.absolutePath()));
    QDir contentsDir(progDir.absolutePath() + "/");
    QStringList fileFilters = QStringList() << "lpub3d*";
    QStringList shareContents = contentsDir.entryList(fileFilters);
    if (shareContents.size() > 0) {
        Preferences::printInfo(tr("%1 Application Folder....(%2)").arg(VER_PRODUCTNAME_STR).arg(Preferences::lpub3dAppName));
    } else {
        Preferences::printInfo(tr("ERROR - Application Folder Not Found."));
    }
#endif
#endif // NOT Q_OS_WIN
    Preferences::printInfo(tr("%1 Application Data Path.(%2)").arg(VER_PRODUCTNAME_STR).arg(QDir::toNativeSeparators(Preferences::lpubDataPath)));
#ifdef Q_OS_MAC
    Preferences::printInfo(tr("%1 Bundle App Path.......(%2)").arg(VER_PRODUCTNAME_STR).arg(QDir::toNativeSeparators(Preferences::lpub3dPath)));
#else // Q_OS_LINUX and Q_OS_WIN
    Preferences::printInfo(tr("%1 Executable Path.......(%2)").arg(VER_PRODUCTNAME_STR).arg(QDir::toNativeSeparators(Preferences::lpub3dPath)));
    Preferences::printInfo(tr("%1 Log File Path.........(%2)").arg(VER_PRODUCTNAME_STR).arg(QDir::toNativeSeparators(Preferences::logFilePath)));
#endif
#ifdef Q_OS_WIN
    Preferences::printInfo(tr("%1 Parameters Location...(%2)").arg(VER_PRODUCTNAME_STR).arg(QDir::toNativeSeparators(Preferences::dataLocation)));
#else // Q_OS_LINUX and Q_OS_MAC
    Preferences::printInfo(tr("LPub3D Extras Resource Path..(%1)").arg(QDir::toNativeSeparators(Preferences::lpub3dExtrasResourcePath)));
#if defined Q_OS_LINUX
    QDir rendererDir(QString("%1/../../%2/%3").arg(Preferences::lpub3dPath)
                     .arg(Preferences::optPrefix.isEmpty() ? "opt" : Preferences::optPrefix+"/opt")
                     .arg(Preferences::lpub3dAppName));
    Preferences::printInfo(tr("%1 Renderers Exe Path....(%2)").arg(VER_PRODUCTNAME_STR).arg(QDir::toNativeSeparators(QString("%1/3rdParty").arg(rendererDir.absolutePath()))));
#endif // Q_OS_LINUX
#endif //  Q_OS_WIN or Q_OS_LINUX and Q_OS_MAC
    Preferences::printInfo(tr("%1 Config File Path......(%2)").arg(VER_PRODUCTNAME_STR).arg(QDir::toNativeSeparators(Preferences::lpub3dConfigPath)));
    Preferences::printInfo(tr("%1 3D Editor Cache Path..(%2)").arg(VER_PRODUCTNAME_STR).arg(QDir::toNativeSeparators(Preferences::lpub3dCachePath)));
    Preferences::printInfo(tr("%1 Loaded LDraw Library..(%2)").arg(VER_PRODUCTNAME_STR).arg(Preferences::validLDrawPartsLibrary));
    Preferences::printInfo(tr("%1 Logging Level.........(%2 (%3), Levels: [%4])").arg(VER_PRODUCTNAME_STR).arg(Preferences::loggingLevel)
                                                                  .arg(QStringList(QString(VER_LOGGING_LEVELS_STR).split(",")).indexOf(Preferences::loggingLevel,0))
                                                                  .arg(QString(VER_LOGGING_LEVELS_STR).toLower()));
    Preferences::printInfo(tr("Debug Logging................(%1)").arg(Preferences::debugLogging ? tr("Enabled") : tr("Disabled")));
    Preferences::printInfo(tr("Log To Standard Output.......(%1)").arg(Preferences::suppressFPrint ? tr("Disabled") : tr("Enabled")));
#ifdef Q_OS_WIN
    if (consoleRedirectTreated)
        Preferences::printInfo(tr("Redirect Output To Console...(%1)").arg(consoleRedirect ? tr("Enabled") : tr("Disabled")));
#endif
#ifndef QT_NO_SSL
    Preferences::printInfo(tr("Secure Socket Layer..........(%1)").arg(QSslSocket::supportsSsl() ? tr("Supported") : tr("Not Supported %1")
                                                                  .arg(QSslSocket::sslLibraryBuildVersionString().isEmpty() ? tr(", Build not detected") : tr(", Build: %1 %2")
                                                                  .arg(QSslSocket::sslLibraryBuildVersionString(),
                                                                       QSslSocket::sslLibraryVersionString().isEmpty() ? tr(", Library not detected") : tr(", Detected: %1")
                                                                  .arg(QSslSocket::sslLibraryVersionString())))));
#else
    Preferences::printInfo(tr("Secure Socket Layer..........(QT_NO_SSL Declaration Detected.)"));
#endif // QT_NO_SSL
#ifdef Q_OS_WIN
    Preferences::printInfo(tr("Application Distribution.....(%1)").arg(Preferences::portableDistribution ? "Portable" : "Installed"));
#endif
    Preferences::printInfo("-----------------------------");

    // splash
    // if you want to rewrite, here is a good example:
    // https://wiki.qt.io/Custom_splashscreen_with_text
    if (modeGUI())
    {
        QPixmap pixmap(":/resources/LPub512Splash.png");
        splash = new QSplashScreen(pixmap);

        QFont splashFont;
#ifdef Q_OS_LINUX
        splashFont.setFamily("Geneva");
        splashFont.setPointSize(16);
#elif defined(Q_OS_MAC)
        splashFont.setFamily("Menlo");
        splashFont.setPointSize(14);
#else
        splashFont.setFamily("Segoe UI");
        splashFont.setPointSize(16);
#endif
        splashFont.setStretch(100);

        splash->setFont(splashFont);
        splash->show();
    }

    emit splashMsgSig(tr("5% - Loading library for %1...").arg(Preferences::validLDrawPartsLibrary));

    // Preferences
    Preferences::lpub3dLibPreferences(false);
    Preferences::ldrawPreferences(false);

    emit splashMsgSig(tr("15% - Preferences loading..."));

    Preferences::themePreferences();
    Preferences::lpub3dUpdatePreferences();
    Preferences::lgeoPreferences();
    Preferences::rendererPreferences();
    Preferences::fadestepPreferences();
    Preferences::highlightstepPreferences();
    Preferences::unitsPreferences();
    Preferences::annotationPreferences();
    Preferences::pliPreferences();
    Preferences::userInterfacePreferences();
    Preferences::editorPreferences();

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

    qRegisterMetaType<PieceInfo*>("PieceInfo*");
    qRegisterMetaType<QList<int> >("QList<int>");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");
#endif

    QList<QPair<QString, bool>> LibraryPaths;

#ifdef Q_OS_WIN
    gConsoleMode = !modeGUI();
    lcSehInit();
#endif

    setlocale(LC_NUMERIC, "C");

/* load sequence
* LPub::LPub()                                           (lpub)          [LPub3D]
* Gui::Gui()                                             (gui)           [LPub3D]
* lcApplication::lcApplication()                         (gApplication)  [Visual Editor]
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
    if (modeGUI())
        setTheme(false/*appStarted*/);

    emit splashMsgSig(tr("20% - OpenGL context initialization..."));

    if (!Options.StdErr.isEmpty())
    {
        Preferences::printInfo(Options.StdErr,true/*isError*/);
    }

    if (!Options.ParseOK)
    {
        const QString message = tr("Parse command line options failed. (return code 1)");
        Preferences::printInfo(message,true);
        throw InitException(qPrintable(message));
    }

    // initialize Visual Editor application
    gApplication = new lcApplication(Options);

    // Set global Visual Editor shared OpenGL context
    if (!lcContext::InitializeRenderer())
    {
        gApplication->Shutdown();
        const QString message = tr("Error creating shared OpenGL context. (return code 1)");
        Preferences::printInfo(message,true);
        throw InitException(qPrintable(message));
    }

    emit splashMsgSig(tr("25% - %1 GUI window loading...").arg(VER_PRODUCTNAME_STR));

    // initialize LPub object
    lpub = new LPub();

    // initialize gui
    gui = new Gui();

    // Check if preferred renderer set and launch Preference dialogue if not to set Renderer
    gui->getRequireds();

    emit splashMsgSig(tr("30% - Visual Editor loading..."));

    Preferences::viewerPreferences();

    emit splashMsgSig(tr("40% - Visual Editor initialization..."));

    if (gApplication->Initialize(LibraryPaths, gui) == lcStartupMode::Error) {
        gApplication->Shutdown();
        const QString message = tr("Unable to initialize Visual Editor. (return code 1)");
        Preferences::printInfo(message,true);
        throw InitException(qPrintable(message));
    } else {
        gui->initialize();
    }

    return RUN_APPLICATION;
}

void Application::mainApp()
{
    if (m_print_output)
        return;

    emit splashMsgSig(tr("100% - %1 loaded.").arg(VER_PRODUCTNAME_STR));

    availableVersions = new AvailableVersions(this);

    if ((Preferences::enableFadeSteps || Preferences::enableHighlightStep))
        gui->ldrawColorPartsLoad();

    if (modeGUI()) {
        splash->finish(gui);

        Preferences::setLPub3DLoaded();

        gui->show();

#ifdef Q_OS_WIN
        QWindowsWindowFunctions::setHasBorderInFullScreen(gui->windowHandle(), true);
#endif
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
    QString ExecMessage;
    int ExecReturn = EXIT_FAILURE;
    try
    {
      mainApp();

      if (modeGUI()) {
          ExecReturn = m_application.exec();
      } else {
          ExecReturn = lpub->processCommandLine();
      }
#ifdef Q_OS_WIN
      ReleaseConsole();
#endif
    }
    catch(const std::exception& ex)
    {
      ExecMessage = tr("%1 Run: Exception %2 has been thrown.").arg(VER_PRODUCTNAME_STR).arg(ex.what());
    }
    catch(...)
    {
      ExecMessage = tr("%1 Run: An unhandled exception has been thrown.").arg(VER_PRODUCTNAME_STR);
    }

    ExecMessage = QString("%1 Run: Application terminated with return code %2.")
                          .arg(QString("%1%2").arg(ExecMessage.isEmpty() ? QString() : QString("%1\n\n").arg(ExecMessage)).arg(VER_PRODUCTNAME_STR))
                          .arg(ExecReturn);

    if (Preferences::loggingEnabled) {
      emit lpub->messageSig(ExecReturn ? LOG_ERROR : LOG_INFO, ExecMessage);
    } else {
      Preferences::printInfo(ExecMessage, ExecReturn);
    }

    if (!m_print_output)
    {
       shutdown();
    }

    return ExecReturn;
}

void Application::shutdown()
{
    delete gui;
    gui = nullptr;

    delete lpub;
    lpub = nullptr;

    gApplication->Shutdown();

    delete availableVersions;
    availableVersions = nullptr;

    ldvWidget = nullptr;
}

static void initializeSurfaceFormat(int argc, char* argv[], lcCommandLineOptions &Options)
{
    QCoreApplication Initialize(argc, argv);

    Preferences::setDistribution();

    Options = lcApplication::ParseCommandLineOptions();

    if (Options.ParseOK && Options.AASamples > 1)
    {
        QSurfaceFormat Format;
#ifdef __APPLE__
        // Set the OpenGL profile before constructing the MainWindow.
        // This must be done on the mac so that it does not use the
        // legacy profile.
        Format.setMajorVersion(3);
        Format.setMinorVersion(3);
        Format.setDepthBufferSize(24);
        Format.setProfile(QSurfaceFormat::CoreProfile);
        QSurfaceFormat::setDefaultFormat(Format);
#else
        Format = QSurfaceFormat::defaultFormat();
#endif
        Format.setSamples(Options.AASamples);
        QSurfaceFormat::setDefaultFormat(Format);
    }
}

int main(int argc, char** argv)
{
    QCoreApplication::setOrganizationDomain(QLatin1String(VER_COMPANYDOMAIN_STR));
    QCoreApplication::setOrganizationName(  QLatin1String(VER_COMPANYNAME_STR));
    QCoreApplication::setApplicationVersion(QLatin1String(VER_PRODUCTVERSION_STR));

    lcCommandLineOptions Options;

    initializeSurfaceFormat(argc, argv, Options);

    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

//#if !defined(Q_OS_MAC) && QT_VERSION >= QT_VERSION_CHECK(5,6,0)
//    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//#endif
#ifdef Q_OS_MAC
    QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

    Application app(argc, argv);

    int rc = EXIT_SUCCESS;

    try
    {
        rc = app.initialize(Options);
    }
    catch(const InitException &ex)
    {
        fprintf(stderr, "%s\n", qUtf8Printable(QObject::tr("Could not initialize %1. Exception: %2.").arg(VER_PRODUCTNAME_STR).arg(ex.what())));
        rc = EXIT_FAILURE;
    }
    catch(...)
    {
        fprintf(stderr, "%s\n", qUtf8Printable(QObject::tr("A fatal %1 error ocurred.").arg(VER_PRODUCTNAME_STR)));
        rc = EXIT_FAILURE;
    }

    if (rc == RUN_APPLICATION)
    {
        return app.run();
    }
    else
    {
       return rc;
    }
}
