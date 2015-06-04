// Copyright (c) 2010, Razvan Petru
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

#include <QDebug>
#include <QString>
#include <QStringList>

#ifdef QS_LOG_USE_COLOURS
    #define QS_LOG_BLACK      "\\033[22;30m"
    #define QS_LOG_GRAY       "\\033[01;30m"
    #define QS_LOG_RED        "\\033[22;31m"
    #define QS_LOG_LRED       "\\033[01;21m"
    #define QS_LOG_GREEN      "\\033[22;32m"
    #define QS_LOG_LGREEN 	  "\\033[01;32m"
    #define QS_LOG_BLUE       "\\033[22;34m"
    #define QS_LOG_LBLUE      "\\033[01;34m"
    #define QS_LOG_BROWN      "\\033[22;33m"
    #define QS_LOG_YELLOW 	  "\\033[01;33m"
    #define QS_LOG_CYAN       "\\033[22;36m"
    #define QS_LOG_LCYAN      "\\033[22;36m"
    #define QS_LOG_MAGENTA    "\\033[22;35m"
    #define QS_LOG_LMAGENTA   "\\033[01;35m"
    #define QS_LOG_NC 		  "\\033[0m"
    #define QS_LOG_BOLD       "\\033[1m"
    #define QS_LOG_ULINE      "\\033[4m" //underline
    #define QS_LOG_BLINK      "\\033[5m"
    #define QS_LOG_INVERT     "\\033[7m"
#else
    #define QS_LOG_BLACK      "\\"
    #define QS_LOG_GRAY       "\\"
    #define QS_LOG_RED        "\\"
    #define QS_LOG_LRED       "\\"
    #define QS_LOG_GREEN      "\\"
    #define QS_LOG_LGREEN     "\\"
    #define QS_LOG_BLUE       "\\"
    #define QS_LOG_LBLUE      "\\"
    #define QS_LOG_BROWN      "\\"
    #define QS_LOG_YELLOW 	  "\\"
    #define QS_LOG_CYAN       "\\"
    #define QS_LOG_LCYAN      "\\"
    #define QS_LOG_MAGENTA    "\\"
    #define QS_LOG_LMAGENTA   "\\"
    #define QS_LOG_NC 		  "\\"
    #define QS_LOG_BOLD       "\\"
    #define QS_LOG_ULINE      "\\"
    #define QS_LOG_BLINK      "\\"
    #define QS_LOG_INVERT     "\\"
#endif

#define QS_LOG_TRACE "TRACE"
#define QS_LOG_DEBUG "DEBUG"
#define QS_LOG_INFO  "INFO"
#define QS_LOG_WARN  "WARN"
#define QS_LOG_ERROR "ERROR"
#define QS_LOG_FATAL "FATAL"

namespace QsLogging
{
class Destination;
enum Level
{
   TraceLevel = 0,
   NoticeLevel,
   DebugLevel,
   InfoLevel,
   WarnLevel,
   ErrorLevel,
   FatalLevel
};

class LoggerImpl; // d pointer
class Logger
{
public:
   static Logger& instance()
   {
      static Logger staticLog;
      return staticLog;
   }

   //! Adds a log message destination. Don't add null destinations.
   void addDestination(Destination* destination);
   //! Logging at a level < 'newLevel' will be ignored
   void setLoggingLevel(Level newLevel);
   //! The default level is INFO
   Level loggingLevel() const;

   //! The helper forwards the streaming to QDebug and builds the final
   //! log message.
   class Helper
   {
   public:
      explicit Helper(Level logLevel) :
            level(logLevel),
            qtDebug(&buffer) {}
      ~Helper();
      QDebug& stream(){ return qtDebug; }
      QString colourFunctionName(QString name);

   private:
      void writeToLog();

      Level level;
      QString buffer;
      QDebug qtDebug;
   };

private:
   Logger();
   Logger(const Logger&);
   Logger& operator=(const Logger&);
   ~Logger();

   void write(const QString& message);

   LoggerImpl* d;
};

} // end namespace

//! Logging macros: define QS_LOG_LINE_NUMBERS to get the file and line number
//! in the log output.
#ifndef QS_LOG_LINE_DETAIL
#define logTrace() \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::TraceLevel ){} \
   else QsLogging::Logger::Helper(QsLogging::TraceLevel).stream()
#define logNotice() \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::NoticeLevel ){} \
   else QsLogging::Logger::Helper(QsLogging::NoticeLevel).stream()
#define logDebug() \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::DebugLevel ){} \
   else QsLogging::Logger::Helper(QsLogging::DebugLevel).stream()
#define logInfo()  \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::InfoLevel ){} \
   else QsLogging::Logger::Helper(QsLogging::InfoLevel).stream()
#define logWarn()  \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::WarnLevel ){} \
   else QsLogging::Logger::Helper(QsLogging::WarnLevel).stream()
#define logError() \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::ErrorLevel ){} \
   else QsLogging::Logger::Helper(QsLogging::ErrorLevel).stream()
#define logFatal() \
   QsLogging::Logger::Helper(QsLogging::FatalLevel).stream()
#else
#define logTrace() \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::TraceLevel ){} \
   else  QsLogging::Logger::Helper(QsLogging::TraceLevel).stream() << Q_FUNC_INFO << '@' << __LINE__
#define logNotice() \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::NoticeLevel ){} \
   else QsLogging::Logger::Helper(QsLogging::NoticeLevel).stream() << Q_FUNC_INFO << '@' << __LINE__
#define logDebug() \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::DebugLevel ){} \
   else QsLogging::Logger::Helper(QsLogging::DebugLevel).stream() << Q_FUNC_INFO << '@' << __LINE__
#define logInfo()  \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::InfoLevel ){} \
   else QsLogging::Logger::Helper(QsLogging::InfoLevel).stream() << Q_FUNC_INFO << '@' << __LINE__
#define logWarn()  \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::WarnLevel ){} \
   else QsLogging::Logger::Helper(QsLogging::WarnLevel).stream() << Q_FUNC_INFO << '@' << __LINE__
#define logError() \
   if( QsLogging::Logger::instance().loggingLevel() > QsLogging::ErrorLevel ){} \
   else QsLogging::Logger::Helper(QsLogging::ErrorLevel).stream()<< Q_FUNC_INFO << '@' << __LINE__
#define logFatal() \
   QsLogging::Logger::Helper(QsLogging::FatalLevel).stream() << Q_FUNC_INFO << '@' << __LINE__
#endif

//  else QsLogging::Logger::Helper(QsLogging::InfoLevel).stream() << __FILE__ << QsLogging::Logger::Helper(QsLogging::InfoLevel).colourFunctionName(Q_FUNC_INFO) << '@' << __LINE__

#ifdef QS_LOG_DISABLE
#include "QsLogDisableForThisFile.h"
#endif

#endif // QSLOG_H
