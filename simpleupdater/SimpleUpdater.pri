#
#  This file is part of SimpleUpdater
#
#  Copyright (c) 2014 Alex Spataru <alex_spataru@outlook.com>
#
#  Please check the license.txt file for more information.
#

QT += gui
QT += network
QT += widgets

HEADERS += $$PWD/src/SimpleUpdater.h \
           $$PWD/src/dialogs/download_dialog.h \
           $$PWD/src/dialogs/progress_dialog.h

SOURCES += $$PWD/src/SimpleUpdater.cpp \
           $$PWD/src/dialogs/download_dialog.cpp \
           $$PWD/src/dialogs/progress_dialog.cpp

INCLUDEPATH += $$PWD/src

OTHER_FILES += $$PWD/src/SimpleUpdater

unix:!android {
    LIBS += -lcrypto -lssl
}

win32* {
    LIBS += -L$$[QT_INSTALL_PREFIX]/bin -llibeay32
}

RESOURCES += $$PWD/res/su_resources.qrc

FORMS += $$PWD/src/dialogs/download_dialog.ui \
         $$PWD/src/dialogs/progress_dialog.ui


#message($$CONFIG)
