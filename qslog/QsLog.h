// Copyright (c) 2013, Razvan Petru
// All rights reserved.

// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:

// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice, this
//   list of conditions and the following disclaimer in the documentation and/or other
//   materials provided with the distribution.
// * The name of the contributors may not be used to endorse or promote products
//   derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef QSLOG_H
#define QSLOG_H

#include "QsLogLevel.h"
#include "QsLogDest.h"
#include <QDebug>
#include <QString>

#define QS_LOG_VERSION "2.0b4"

#define QS_LOG_SPLIT "__sp|it__"

#define QS_LOG_BLACK      "\033[22;30m"
#define QS_LOG_GRAY       "\033[01;30m"
#define QS_LOG_RED        "\033[22;31m"
#define QS_LOG_LRED       "\033[01;21m"
#define QS_LOG_GREEN      "\033[22;32m"
#define QS_LOG_LGREEN     "\033[01;32m"
#define QS_LOG_BLUE       "\033[22;34m"
#define QS_LOG_LBLUE      "\033[01;34m"
#define QS_LOG_BROWN      "\033[22;33m"
#define QS_LOG_YELLOW     "\033[01;33m"
#define QS_LOG_CYAN       "\033[22;36m"
#define QS_LOG_LCYAN      "\033[22;36m"
#define QS_LOG_MAGENTA    "\033[22;35m"
#define QS_LOG_LMAGENTA   "\033[01;35m"
#define QS_LOG_NC         "\033[0m"
#define QS_LOG_BOLD       "\033[1m"
#define QS_LOG_ULINE      "\033[4m"    //underline
#define QS_LOG_BLINK      "\033[5m"
#define QS_LOG_INVERT     "\033[7m"

namespace QsLogging
{
class Destination;

class LoggerImpl; // d pointer

class QSLOG_SHARED_OBJECT Logger
{
public:
  static Logger& instance();
  static void destroyInstance();
  static Level levelFromLogMessage(const QString& logMessage, bool* conversionSucceeded = 0);

  ~Logger();

  //! Adds a log message destination. Don't add null destinations.
  void addDestination(DestinationPtr destination);
  //! Logging at a level < 'newLevel' will be ignored
  void setLoggingLevel(Level newLevel);
  //! The default level is INFO
  Level loggingLevel() const;
  //! Set to false to disable timestamp inclusion in log messages
  void setIncludeTimestamp(bool e);
  //! Default value is true.
  bool includeTimestamp() const;
  //! Set to false to disable line number inclusion in log messages
  void setIncludeLineNumber(bool e);
  //! Default value is true.
  bool includeLineNumber() const;
  //! Set to false to disable function informatino inclusion in log messages
  void setIncludeFunctionInfo(bool e);
  //! Default value is true.
  bool includeFunctionInfo() const;
  //! Set to false to disable file name inclusion in log messages
  void setIncludeFileName(bool e);
  //! Default value is true.
  bool includeFileName() const;
  //! Set to false to disable log level inclusion in log messages
  void setIncludeLogLevel(bool l);
  //! Default value is true.
  bool includeLogLevel() const;
  //! Set to false to disable colorized output inclusion in log message
  void setColorizeOutput(bool l);
  //! Default value is true.
  bool colorizeOutput() const;
  //! Set to false to disable colorized function info inclusion in log message
  void setColorizeFunctionInfo(bool l);
  //! Default value is true.
  bool colorizeFunctionInfo() const;

  //! The helper forwards the streaming to QDebug and builds the final
  //! log message.
  class QSLOG_SHARED_OBJECT Helper
  {
  public:
    explicit Helper(Level logLevel) :
      level(logLevel),
      qtDebug(&buffer) {}
    ~Helper();
    QDebug& stream(){ return qtDebug; }
    QString colorizeFunctionInfo(QString functionInfo);

  private:
    void writeToLog();

    Level level;
    QString buffer;
    QDebug qtDebug;
  };

private:
  Logger();
  Logger(const Logger&);            // not available
  Logger& operator=(const Logger&); // not available

  void enqueueWrite(const QString& colourMessage, const QString& plainMessage, Level level);
  void write(const QString& colourMessage, const QString& plainMessage, Level level);

  LoggerImpl* d;

  friend class LogWriterRunnable;
};

} // end namespace


//! in the log output.
#define logTrace() \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::TraceLevel ){} \
   else QsLogging::Logger::Helper(QsLogging::TraceLevel).stream() \
      << __FILE__ << '|' << Q_FUNC_INFO << '|' << __LINE__ << QS_LOG_SPLIT
#define logNotice() \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::NoticeLevel ){} \
   else QsLogging::Logger::Helper(QsLogging::NoticeLevel).stream() \
      << __FILE__ << '|' << Q_FUNC_INFO << '|' << __LINE__ << QS_LOG_SPLIT
#define logDebug() \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::DebugLevel ){} \
   else QsLogging::Logger::Helper(QsLogging::DebugLevel).stream() \
      << __FILE__ << '|' << Q_FUNC_INFO << '|' << __LINE__ << QS_LOG_SPLIT
#define logInfo()  \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::InfoLevel ){} \
   else QsLogging::Logger::Helper(QsLogging::InfoLevel).stream() \
      << __FILE__ << '|' << Q_FUNC_INFO << '|' << __LINE__ << QS_LOG_SPLIT
#define logStatus()  \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::StatusLevel ){} \
   else QsLogging::Logger::Helper(QsLogging::StatusLevel).stream() \
      << __FILE__ << '|' << Q_FUNC_INFO << '|' << __LINE__ << QS_LOG_SPLIT
#define logError() \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::ErrorLevel ){} \
   else QsLogging::Logger::Helper(QsLogging::ErrorLevel).stream() \
      << __FILE__ << '|' << Q_FUNC_INFO << '|' << __LINE__ << QS_LOG_SPLIT
#define logFatal() \
   QsLogging::Logger::Helper(QsLogging::FatalLevel).stream() \
      << __FILE__ << '|' << Q_FUNC_INFO << '|' << __LINE__ << QS_LOG_SPLIT

#ifdef QS_LOG_DISABLE
#include "QsLogDisableForThisFile.h"
#endif

#endif // QSLOG_H
