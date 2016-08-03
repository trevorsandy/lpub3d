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

#include "QsLog.h"
#include "QsLogDest.h"
#ifdef QS_LOG_SEPARATE_THREAD
#include <QThreadPool>
#include <QRunnable>
#endif
#include <QMutex>
#include <QVector>
#include <QDateTime>
#include <QtGlobal>
#include <cstdlib>
#include <stdexcept>
#include <QFileInfo>

namespace QsLogging
{
typedef QVector<DestinationPtr> DestinationList;

static const char TraceString[] = "TRACE";
static const char NoticeString[]= "NOTICE";
static const char DebugString[] = "DEBUG";
static const char InfoString[]  = "INFO ";
static const char StatusString[]  = "STATUS ";
static const char ErrorString[] = "ERROR";
static const char FatalString[] = "FATAL";

// not using Qt::ISODate because we need the milliseconds too
static const QString fmtDateTime("yyyy-MM-ddThh:mm:ss.zzz");

static Logger* sInstance = 0;

static const char* LevelToText(Level theLevel)
{
   switch( theLevel )
   {
   case TraceLevel:
      return TraceString;
   case NoticeLevel:
      return NoticeString;
   case DebugLevel:
      return DebugString;
   case InfoLevel:
      return InfoString;
   case StatusLevel:
      return StatusString;
   case ErrorLevel:
      return ErrorString;
   case FatalLevel:
      return FatalString;
   default:
      {
         Q_ASSERT(!"bad log level");
         return InfoString;
      }
   }
}

QString ColorizeLogOutput(Level theLevel, QString output)
{

   switch( theLevel )
   {
   case TraceLevel:
      return QString("%1%2%3").arg(QS_LOG_MAGENTA).arg(output).arg(QS_LOG_NC);
   case NoticeLevel:
      return QString("%1%2%3").arg(QS_LOG_CYAN).arg(output).arg(QS_LOG_NC);
   case DebugLevel:
      return QString("%1%2%3").arg(QS_LOG_BLUE).arg(output).arg(QS_LOG_NC);
   case InfoLevel:
      return QString("%1%2%3").arg(QS_LOG_GREEN).arg(output).arg(QS_LOG_NC);
   case StatusLevel:
      return QString("%1%2%3").arg(QS_LOG_YELLOW).arg(output).arg(QS_LOG_NC);
   case ErrorLevel:
      return QString("%1%2%3").arg(QS_LOG_RED).arg(output).arg(QS_LOG_NC);
   case FatalLevel:
      return QString("%1%2%3").arg(QS_LOG_RED).arg(output).arg(QS_LOG_NC);
   default:
      {
         Q_ASSERT(!"bad log level");
         return output;
      }
   }
}

#ifdef QS_LOG_SEPARATE_THREAD
class LogWriterRunnable : public QRunnable
{
public:
    LogWriterRunnable(QString message, Level level);
    virtual void run();

private:
    QString mMessage;
    Level mLevel;
};
#endif

class LoggerImpl
{
public:
    LoggerImpl();

#ifdef QS_LOG_SEPARATE_THREAD
    QThreadPool threadPool;
#endif
    QMutex logMutex;
    Level level;
    DestinationList destList;
    bool includeLogLevel;
	bool includeTimeStamp;
	bool includeLineNumber;
	bool includeFileName;
	bool includeFunctionInfo;
	bool colorizeOutput;
	bool colorizeFunctionInfo;
};

#ifdef QS_LOG_SEPARATE_THREAD
LogWriterRunnable::LogWriterRunnable(QString message, Level level)
    : QRunnable()
    , mMessage(message)
    , mLevel(level)
{
}

void LogWriterRunnable::run()
{
    Logger::instance().write(mMessage, mLevel);
}
#endif


LoggerImpl::LoggerImpl()
    : level(InfoLevel)
    , includeLogLevel(true)
	, includeTimeStamp(true)
	, includeLineNumber(true)
	, includeFileName(true)
	, includeFunctionInfo(true)
	, colorizeOutput(false)
	, colorizeFunctionInfo(false)
{
    // assume at least file + console
    destList.reserve(2);
#ifdef QS_LOG_SEPARATE_THREAD
    threadPool.setMaxThreadCount(1);
    threadPool.setExpiryTimeout(-1);
#endif
}


Logger::Logger()
    : d(new LoggerImpl)
{
}

Logger& Logger::instance()
{
    if (!sInstance)
        sInstance = new Logger;

    return *sInstance;
}

void Logger::destroyInstance()
{
    delete sInstance;
    sInstance = 0;
}

// tries to extract the level from a string log message. If available, conversionSucceeded will
// contain the conversion result.
Level Logger::levelFromLogMessage(const QString& logMessage, bool* conversionSucceeded)
{
    if (conversionSucceeded)
        *conversionSucceeded = true;

    if (logMessage.startsWith(QLatin1String(TraceString)))
        return TraceLevel;
    if (logMessage.startsWith(QLatin1String(NoticeString)))
        return NoticeLevel;
    if (logMessage.startsWith(QLatin1String(DebugString)))
        return DebugLevel;
    if (logMessage.startsWith(QLatin1String(InfoString)))
        return InfoLevel;
    if (logMessage.startsWith(QLatin1String(StatusString)))
        return StatusLevel;
    if (logMessage.startsWith(QLatin1String(ErrorString)))
        return ErrorLevel;
    if (logMessage.startsWith(QLatin1String(FatalString)))
        return FatalLevel;

    if (conversionSucceeded)
        *conversionSucceeded = false;
    return OffLevel;
}

Logger::~Logger()
{
#ifdef QS_LOG_SEPARATE_THREAD
    d->threadPool.waitForDone();
#endif
    delete d;
    d = 0;
}

void Logger::addDestination(DestinationPtr destination)
{
    Q_ASSERT(destination.data());
    d->destList.push_back(destination);
}

void Logger::setLoggingLevel(Level newLevel)
{
    d->level = newLevel;
}

Level Logger::loggingLevel() const
{
	return d->level;
}

void Logger::setIncludeTimestamp(bool e)
{
	d->includeTimeStamp = e;
}

bool Logger::includeTimestamp() const
{
	return d->includeTimeStamp;
}

//
void Logger::setIncludeLineNumber(bool e)
{
	d->includeLineNumber = e;
}

bool Logger::includeLineNumber() const
{
	return d->includeLineNumber;
}

void Logger::setIncludeFileName(bool e)
{
	d->includeFileName = e;
}

bool Logger::includeFileName() const
{
	return d->includeFileName;
}

void Logger::setIncludeFunctionInfo(bool e)
{
	d->includeFunctionInfo = e;
	// if this is false then colorizeFunctionInfo must be set to false if included.
	if (!e && d->colorizeFunctionInfo) {
	    qDebug() << "QS_LOG Options Conflict: setIncludeFunctionInfo() is set to false but setColorizeFunctionInfo() is set to true.\n"
			"colorizeFunctionInfo() has been internally set to false. Update your QS_LOG option setting.";
		d->colorizeFunctionInfo = e;
	  }
}

bool Logger::includeFunctionInfo() const
{
	return d->includeFunctionInfo;
}

void Logger::setIncludeLogLevel(bool l)
{
	d->includeLogLevel = l;
}

bool Logger::includeLogLevel() const
{
	return d->includeLogLevel;
}

void Logger::setColorizeOutput(bool l)
{
      d->colorizeOutput = l;
      // if this is false then colorizeFunctionInfo must be set to false if included.
      if (!l && d->includeFunctionInfo && d->colorizeFunctionInfo) {
          qDebug() << "QS_LOG Options Conflict: setColorizeOutput() is set to false but setColorizeFunctionInfo() is set to true.\n"
                      "colorizeFunctionInfo() has been internally set to false. Update your QS_LOG option setting.";
          d->colorizeFunctionInfo = l;
        }
}

bool Logger::colorizeOutput() const
{
      return d->colorizeOutput;
}

void Logger::setColorizeFunctionInfo(bool l)
{
	d->colorizeFunctionInfo = l;
	// if this is true then colorizeOutput must be set to true.
	if (l && !d->colorizeOutput && !d->includeFunctionInfo){
	    qDebug() << "QS_LOG Options Conflict: setColorizeFunctionInfo() is set to true but setColorizeOutput() is set to false.\n"
			"colorizeOutput() has been internally set to true. Update your QS_LOG option setting.";
	    d->colorizeOutput = l;
	    d->includeFunctionInfo = l;
	  }
}

bool Logger::colorizeFunctionInfo() const
{
	return d->colorizeFunctionInfo;
}

//! creates the complete log message and passes it to the logger
void Logger::Helper::writeToLog()
{    
    QString split         = QString::fromLatin1(QS_LOG_SPLIT);
    QString headers       = buffer.section(split,0,0).trimmed();
    QString fileRelPath   = headers.section('|',0,0).trimmed();
    QString functionInfo  = headers.section('|',1,1).trimmed();
    QString lineNumber    = headers.section('|',2,2).trimmed();
    buffer.remove(QString("%1 %2 ").arg(headers).arg(split));
    QFileInfo info(fileRelPath);
    QString fileName      = info.fileName();

    const char* const levelName = LevelToText(level);

//    qDebug() << "\nBuffer:       " << buffer <<
//                "\nSplit:        " << split <<
//                "\nHeaders:      " << headers <<
//                "\nfileName:     " << fileName <<
//                "\nfunctionInfo: " << functionInfo <<
//                "\nlineNumber:   " << lineNumber <<
//                "\nRemove:       " << QString("%1%2 ").arg(headers).arg(split);

        QString completePlainMessage,
                completeColorizedMessage;
	Logger &logger = Logger::instance();
	if (logger.includeLogLevel()) {
		completePlainMessage.
			append(levelName).
			append(' ');
	}

	if (logger.includeFileName()) {
		completePlainMessage.
			append(fileName).
			append(' ');
	}

	completeColorizedMessage.append(logger.colorizeOutput() ?  ColorizeLogOutput(level,completePlainMessage) : completePlainMessage);

	if (logger.includeFunctionInfo()) {
		completePlainMessage.
			append(functionInfo).
			append(' ');
	    if (logger.colorizeFunctionInfo()) {
		completeColorizedMessage.
			append(logger.colorizeOutput() ?  colorizeFunctionInfo(functionInfo) : functionInfo).
			append(' ');
	    } else {
		completeColorizedMessage.
			append(logger.colorizeOutput() ?  ColorizeLogOutput(level,functionInfo) : functionInfo).
			append(' ');
	    }
	}

	if (logger.includeLineNumber()) {
		lineNumber.prepend("@ln ");
		completePlainMessage.
			append(lineNumber).
			append(' ');
		completeColorizedMessage.
			append(logger.colorizeOutput() ? ColorizeLogOutput(level,lineNumber) : lineNumber).
			append(' ');
	}

    if (logger.includeTimestamp()) {
        completeColorizedMessage.
            append(QDateTime::currentDateTime().toString(fmtDateTime)).
            append(' ');

    }

    // always add date time stamp to plain message
    completePlainMessage.
            prepend(' ').
            prepend(QDateTime::currentDateTime().toString(fmtDateTime));

	// marshal plain message for the log file - color codes are not human readable friendly.
	completePlainMessage.append(buffer);
	// marshal colorized message for the console
	completeColorizedMessage.append(logger.colorizeOutput() ? ColorizeLogOutput(level,buffer) : buffer);

    Logger::instance().enqueueWrite(completeColorizedMessage, completePlainMessage, level);
}

Logger::Helper::~Helper()
{
    try {
        writeToLog();
    }
    catch(std::exception&) {
        // you shouldn't throw exceptions from a sink
        Q_ASSERT(!"exception in logger helper destructor");
        //throw;
    }
}

//! directs the message to the task queue or writes it directly
void Logger::enqueueWrite(const QString& colourMessage, const QString& plainMessage, Level level)
{
#ifdef QS_LOG_SEPARATE_THREAD
    LogWriterRunnable *r = new LogWriterRunnable(message, level);
    d->threadPool.start(r);
#else
    write(colourMessage, plainMessage, level);
#endif
}

//! Sends the message to all the destinations. The level for this message is passed in case
//! it's useful for processing in the destination.
void Logger::write(const QString& colourMessage, const QString &plainMessage, Level level)
{
    QMutexLocker lock(&d->logMutex);
    for (DestinationList::iterator it = d->destList.begin(),
        endIt = d->destList.end();it != endIt;++it) {
        //if console, do not write status level
        if ((*it)->destType() != LogFile && level != StatusLevel)
            (*it)->write(colourMessage, level);
        else if ((*it)->destType() == LogFile)
            (*it)->write(plainMessage, level);
    }
}

//!  function to parse the Q_FUNC_INFO string and add a little more color.
QString Logger::Helper::colorizeFunctionInfo(QString functionInfo)
{
  QString output;

      QStringList classParts = functionInfo.split("::");
      QStringList nameAndReturnType = classParts.first().split(" ");

      QString returnType = "";
      if(nameAndReturnType.count() > 1)
        returnType = nameAndReturnType.first() + " ";

      QString className = nameAndReturnType.last();

      QStringList funcAndParamas = classParts.last().split("(");
      funcAndParamas.last().chop(1);

      QString functionName = funcAndParamas.first();

      QStringList params = funcAndParamas.last().split(",");

      output.append(QS_LOG_GRAY);
      output.append(returnType);
      output.append(QS_LOG_NC);
      output.append(QS_LOG_GREEN);
      output.append(className);
      output.append(QS_LOG_NC);
      output.append("::");
      output.append(QS_LOG_BLUE);
      output.append(functionName);
      output.append(QS_LOG_NC);
      output.append("(");

      QStringList::const_iterator param;
      for (param = params.begin(); param != params.constEnd(); ++param) {
          if(param != params.begin()) {
              output.append(QS_LOG_NC);
              output.append(",");
            }
          output.append(QS_LOG_CYAN);
          output.append((*param));
        }
      output.append(QS_LOG_NC);
      output.append(")");

  return output;
}

} // end namespace
