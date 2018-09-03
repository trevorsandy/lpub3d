INCLUDEPATH += $$PWD $$PWD/../../qslog
DEPENDPATH += $$PWD

SOURCES += \
        $$PWD/misc.cpp \
        $$PWD/LDVPreferences.cpp \
        $$PWD/LDViewExportOption.cpp \
        $$PWD/LDVAlertHandler.cpp \
        $$PWD/LDVWidget.cpp

HEADERS += \
        $$PWD/misc.h \
        $$PWD/LDVPreferences.h \
        $$PWD/LDViewExportOption.h \
        $$PWD/LDVAlertHandler.h \
        $$PWD/LDVWidget.h

FORMS += \
        $$PWD/LDVExportOptionPanel.ui \
        $$PWD/LDVPreferencesPanel.ui

RESOURCES += \
        $$PWD/resources.qrc
