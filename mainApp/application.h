/****************************************************************************
**
** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.
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
#ifndef APPLICATION_H
#define APPLICATION_H

#include <QScopedPointer>
#include <QException>
#include <QtWidgets>
#include <QFile>
#include <string>
#include <vector>

#ifdef Q_OS_WIN
  #include <Windows.h>
#endif

#include "lc_global.h"
#include "lc_math.h"

#include "QsLog.h"
#include "declarations.h"

class InitException: public QException
{
public:
    void raise() const {throw InitException{};}
};

/// The Application class is responsible for further initialization of the app
/// and provides acessors to the current instance and internal resources. It
/// also take cares of shutdown cleanup. An Application class must be
/// instantiaded only once.
class Application : public QObject
{
     Q_OBJECT

public:
    /// Creates the Application.
    Application(int& argc, char **argv);
    ~Application();

    /// Returns a pointer to the current Application instance;
    static Application* instance();

    /// Returns the command line arguments
    QStringList arguments();

    /// Initialize the Application and process the command line arguments.
    int initialize();

    /// This is the equivalent of the main function.
    void mainApp();

    /// Runs the Application and returns the exit code.
    int run();

    /// Clear any allocated memory and close the application
    void shutdown();

    /// Return applicaion launch mode
    bool modeGUI();

    /// Sets the theme
    void setTheme(bool appStarted = true);

    /// Gets the theme
    QString getTheme();

#ifdef Q_OS_WIN
    /// Console redirection for Windows
    void RedirectIOToConsole();

    /// Release the windows console
    int ReleaseConsole(void);

    /// Windows dmp file generation
    static void lcSehInit();
    static LONG WINAPI lcSehHandler(PEXCEPTION_POINTERS exceptionPointers);
#endif

    /// Initialize the splash screen
    QSplashScreen *splash;

public slots:
    /// Splash message function to display message updates during startup
    void splashMsg(const QString &message);

signals:
    /// Splash message signal to pass messages
    void splashMsgSig(const QString &message);

private:
    /// Qt application
    QApplication m_application;

    /// Current application instance
    static Application* m_instance;

    /// Command console mode flag;
    bool m_console_mode;

    /// Print details flag
    bool m_print_output;

    /// File specified on via commandline
    QString m_commandline_file;

    /// Theme set at startup
    QString m_theme;

#ifdef Q_OS_WIN
    /// Windows console information
    CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;

    /// Windows standard output handle
    HANDLE h_ConsoleStdOut;
    FILE *f_ConsoleStdOut;

    /// Windows standard error handle
    HANDLE h_ConsoleStdErr;
    FILE *f_ConsoleStdErr;

    /// Windows standard input handle
    HANDLE h_ConsoleStdIn;
    FILE *f_ConsoleStdIn;

    /// Windows console save attributes
    WORD m_currentConsoleAttr;

    /// indicate if using allocated console
    BOOL m_allocate_new_console;

    /// indicate if using parent console
    BOOL m_parent_console;
#endif
};
#endif // APPLICATION_H
