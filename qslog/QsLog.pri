INCLUDEPATH += $$PWD

#Log output options
DEFINES += QS_LOG_LINE_DETAIL   # writes the line number for each log entry
DEFINES += QS_LOG_USE_COLOURS   # format output in colour
#DEFINES += QS_LOG_DISABLE      # logging code is replaced with a no-op

SOURCES += \
    $$PWD/QsLogDest.cpp \
    $$PWD/QsLog.cpp \
    $$PWD/QsDebugOutput.cpp

HEADERS += \
    $$PWD/QSLogDest.h \
    $$PWD/QsLog.h \
    $$PWD/QsDebugOutput.h \
    $$PWD/QsLogDisableForThisFile.h
