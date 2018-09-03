INCLUDEPATH += $$PWD $$PWD/../gl2ps $$PWD../TCFoundation
DEPENDPATH += $$PWD

# Input
HEADERS += \
    $$PWD/LDConsoleAlertHandler.h \
    $$PWD/LDHtmlInventory.h \
    $$PWD/LDInputHandler.h \
    $$PWD/LDModelParser.h \
    $$PWD/LDModelTree.h \
    $$PWD/LDObiInfo.h \
    $$PWD/LDPartCount.h \
    $$PWD/LDPartsList.h \
    $$PWD/LDPreferences.h \
    $$PWD/LDrawModelViewer.h \
    $$PWD/LDSnapshotTaker.h \
    $$PWD/LDUserDefaultsKeys.h \
    $$PWD/LDViewPoint.h

SOURCES += \
    $$PWD/LDConsoleAlertHandler.cpp \
    $$PWD/LDHtmlInventory.cpp \
    $$PWD/LDInputHandler.cpp \
    $$PWD/LDModelParser.cpp \
    $$PWD/LDModelTree.cpp \
    $$PWD/LDObiInfo.cpp \
    $$PWD/LDPartCount.cpp \
    $$PWD/LDPartsList.cpp \
    $$PWD/LDPreferences.cpp \
    $$PWD/LDrawModelViewer.cpp \
    $$PWD/LDSnapshotTaker.cpp \
    $$PWD/LDViewPoint.cpp
    
OTHER_FILES += \
    $$PWD/LDViewMessages.ini \
    $$PWD/StudLogo.png
