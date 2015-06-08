#
#  This file is part of QSimpleUpdater
#
#  Copyright (c) 2014 Alex Spataru <alex_spataru@outlook.com>
#
#  Please check the license.txt file for more information.
#

QT += gui
QT += widgets
QT += network

HEADERS += $$PWD/src/qsimpleupdater.h \
           $$PWD/src/dialogs/download_dialog.h \
    $$PWD/src/dialogs/progress_dialog.h

SOURCES += $$PWD/src/qsimpleupdater.cpp \
           $$PWD/src/dialogs/download_dialog.cpp \
    $$PWD/src/dialogs/progress_dialog.cpp

OTHER_FILES += $$PWD/src/QSimpleUpdater

INCLUDEPATH += $$PWD/src

unix:!android {
    LIBS += -lcrypto -lssl
}

#BUILD_ARCH is a custom Environment Variable set under Projects=>Build Environments
QMAKE_TARGET.arch = $$(BUILD_ARCH)

win32* {

    !contains(QMAKE_TARGET.arch, x86_64) {

        ## Windows x86 (32bit) specific build here
        message("~~~ using x86 OpenSSL ~~~")
        LIBS += -LC:/OpenSSL-Win32/bin -llibeay32

    } else {

        ## Windows x64 (64bit) specific build here
        message("~~~ using x86_64 OpenSSL ~~~")
        LIBS += -LC:/OpenSSL-Win64/bin -llibeay32

    }
}

RESOURCES += $$PWD/res/qsu_resources.qrc

FORMS += $$PWD/src/dialogs/download_dialog.ui \
    $$PWD/src/dialogs/progress_dialog.ui
