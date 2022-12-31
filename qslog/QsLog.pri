INCLUDEPATH += $$PWD

#Log output options
#DEFINES += QS_LOG_DISABLE            # logging code is replaced with a no-op
#DEFINES += QS_LOG_SEPARATE_THREAD    # messages are queued and written from a separate thread
#DEFINES += QS_LOG_WIN_PRINTF_CONSOLE # Use fprintf instead of OutputDebugString on Windows
SOURCES += \
	$$PWD/QsLogDest.cpp \
    $$PWD/QsLog.cpp \
    $$PWD/QsLogDestConsole.cpp \
    $$PWD/QsLogDestFile.cpp \
    $$PWD/QsLogDestFunctor.cpp

HEADERS += \
	$$PWD/QsLogDest.h \
    $$PWD/QsLog.h \
    $$PWD/QsLogDestConsole.h \
    $$PWD/QsLogLevel.h \
    $$PWD/QsLogDestFile.h \
    $$PWD/QsLogDisableForThisFile.h \
    $$PWD/QsLogDestFunctor.h

OTHER_FILES += \
    $$PWD/QsLogChanges.txt \
    $$PWD/README.md \
    $$PWD/LICENSE.txt
