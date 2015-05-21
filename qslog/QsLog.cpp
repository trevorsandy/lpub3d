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

#include "QsLog.h"
#include "QsLogDest.h"
#include <QMutex>
#include <QList>
#include <QDateTime>
#include <QtGlobal>
#include <cassert>
#include <cstdlib>
#include <stdexcept>

namespace QsLogging
{
typedef QList<Destination*> DestinationList;

static const char TraceString[] = "\033[01;35mTRACE\033[0m";    //MAGENTA
static const char DebugString[] = "\033[22;34mDEBUG\033[0m";    //BLUE
static const char InfoString[]  = "\33[22;32mINFO\033[0m";      //GREEN
static const char WarnString[]  = "\33[01;33mWARN\033[0m";      //YELLOW
static const char ErrorString[] = "\33[22;31mERROR\033[0m";     //RED
static const char FatalString[] = "\33[22;31mFATAL\033[0m";     //RED

// not using Qt::ISODate because we need the milliseconds too
static const QString fmtDateTime("yyyy-MM-ddThh:mm:ss.zzz");

static const char* LevelToText(Level theLevel)
{
   switch( theLevel )
   {
   case TraceLevel:
      return TraceString;
   case DebugLevel:
      return DebugString;
   case InfoLevel:
      return InfoString;
   case WarnLevel:
      return WarnString;
   case ErrorLevel:
      return ErrorString;
   case FatalLevel:
      return FatalString;
   default:
      {
         assert(!"bad log level");
         return InfoString;
      }
   }
}

QString OutputToColour(Level theLevel, QString output)
{
   switch( theLevel )
   {
   case TraceLevel:
      return QString("%1%2%3").arg("\033[01;35m").arg(output).arg("\033[0m");
   case DebugLevel:
      return QString("%1%2%3").arg("\033[22;34m").arg(output).arg("\033[0m");
   case InfoLevel:
      return QString("%1%2%3").arg("\033[22;32m").arg(output).arg("\033[0m");
   case WarnLevel:
      return QString("%1%2%3").arg("\033[01;33m").arg(output).arg("\033[0m");
   case ErrorLevel:
      return QString("%1%2%3").arg("\033[22;31m").arg(output).arg("\033[0m");
   case FatalLevel:
      return QString("%1%2%3").arg("\033[22;31m").arg(output).arg("\033[0m");
   default:
      {
         assert(!"bad log level");
         return InfoString;
      }
   }
}

class LoggerImpl
{
public:
   LoggerImpl() :
      level(InfoLevel)
   {

   }
   QMutex logMutex;
   Level level;
   DestinationList destList;
};

Logger::Logger() :
   d(new LoggerImpl)
{
}

Logger::~Logger()
{
   delete d;
}

void Logger::addDestination(Destination* destination)
{
   assert(destination);
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

//! creates the complete log message and passes it to the logger
void Logger::Helper::writeToLog()
{
   const char* const levelName = LevelToText(level);
   const QString completeMessage(QString("%1 %2 %3")
      .arg(levelName, 5)
      .arg(OutputToColour(level,QDateTime::currentDateTime().toString(fmtDateTime)))
      .arg(OutputToColour(level,buffer))
      );

   Logger& logger = Logger::instance();
   QMutexLocker lock(&logger.d->logMutex);
   logger.write(completeMessage);
}

Logger::Helper::~Helper()
{
   try
   {
      writeToLog();
   }
   catch(std::exception& e)
   {
      // you shouldn't throw exceptions from a sink
      Q_UNUSED(e);
      assert(!"exception in logger helper destructor");
      throw;
   }
}

//! sends the message to all the destinations
void Logger::write(const QString& message)
{
   for(DestinationList::iterator it = d->destList.begin(),
       endIt = d->destList.end();it != endIt;++it)
   {
      if( !(*it) )
      {
         assert(!"null log destination");
         continue;
      }
      (*it)->write(message);
   }
}

//!  function to parse the Q_FUNC_INFO string and add a little more color.
QString Logger::Helper::colourFunctionName(QString name)
{
    QString output;

    QStringList classParts = name.split("::");
    QStringList nameAndReturnType = classParts.first().split(" ");

    QString returnType = ""; //ctor, dtor don't have return types
    if(nameAndReturnType.count() > 1)
        returnType = nameAndReturnType.first() + " ";

    QString className = nameAndReturnType.last();

    QStringList funcAndParamas = classParts.last().split("(");
    funcAndParamas.last().chop(1);

    QString functionName = funcAndParamas.first();

    QStringList params = funcAndParamas.last().split(",");

    output.append("\033[036m");
    output.append(returnType);
    output.append("\033[0m\033[32m");
    output.append(className);
    output.append("\033[0m::");
    output.append("\033[34m");
    output.append(functionName);
    output.append("\033[0m(");

    QStringList::const_iterator param;
    for (param = params.begin(); param != params.constEnd(); ++param) {
        if(param != params.begin()) {
            output.append("\033[0m,");
        }
        output.append("\033[036m");
        output.append((*param));
    }
    output.append("\033[0m)");


    return output;
}

} // end namespace
