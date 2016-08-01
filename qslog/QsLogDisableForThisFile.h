#ifndef QSLOGDISABLEFORTHISFILE_H
#define QSLOGDISABLEFORTHISFILE_H

#include <QtDebug>
// When included AFTER QsLog.h, this file will disable logging in that C++ file. When included
// before, it will lead to compiler warnings or errors about macro redefinitions.

#undef logTrace
#undef logNotice
#undef logDebug
#undef logInfo
#undef logStatus
#undef logError
#undef logFatal

#define logTrace() if (1) {} else qDebug()
#define logNotice() if (1) {} else qDebug()
#define logDebug()  if (1) {} else qDebug()
#define logInfo()  if (1) {} else qDebug()
#define logStatus()  if (1) {} else qDebug()
#define logError() if (1) {} else qDebug()
#define logFatal() if (1) {} else qDebug()

#endif // QSLOGDISABLEFORTHISFILE_H
