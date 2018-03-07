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
#ifndef APPLICATION_H
#define APPLICATION_H

#include <QScopedPointer>
#include <QException>
#include <QtWidgets>
#include <QFile>
#include <string>
#include <vector>

#include "QsLog.h"
#include "lc_global.h"

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

    /// Returns a pointer to the current Application instance;
    static Application* instance();

    /// Returns the command line arguments
    QStringList arguments();

    /// Initialize the Application and process the command line arguments.
    void initialize();

    /// This is the equivalent of the main function.
    void mainApp();

    /// Runs the Application and returns the exit code.
    int run();

    /// Return applicaion launch mode
    bool modeGUI();

    /// Initialize the splash screen
    QSplashScreen *splash;

public slots:
    /// Splash message function to display message updates during startup
    void splashMsg(QString message)
    {
      logStatus() << message;
      if (m_console_mode)
        return;
      splash->showMessage(QSplashScreen::tr(message.toLatin1().constData()),Qt::AlignBottom | Qt::AlignLeft, Qt::white);
      m_application.processEvents();
    }

signals:
    /// Splash message sinal to pass messages
    void splashMsgSig(QString message);

private:
    /// Qt application
    QApplication m_application;

    /// Current application instance
    static Application* m_instance;

    /// Command console mode flag;
    bool m_console_mode;

    /// Print details flag
    bool m_print_output;

};

/// ENTRY_POINT is a macro that implements the main function.
#define ENTRY_POINT \
    int main(int argc, char** argv) \
    { \
        QScopedPointer<Application> app(new Application(argc, argv)); \
        try \
        { \
            app->initialize(); \
        } \
        catch(const InitException &ex) \
        { \
           qDebug() << QString("Could not initialize the application."); \
        } \
        catch(...) \
        { \
           qDebug() << QString("A fatal error ocurred."); \
        } \
        return app->run(); \
    }
#endif // APPLICATION_H
