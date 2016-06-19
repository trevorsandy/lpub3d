#
# Copyright (c) 2014-2016 Alex Spataru <alex_spataru@outlook.com>
#
# This file is part of the QSimpleUpdater library, which is released under
# the DBAD license, you can read a copy of it below:
#
# DON'T BE A DICK PUBLIC LICENSE TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION
# AND MODIFICATION:
#
# Do whatever you like with the original work, just don't be a dick.
# Being a dick includes - but is not limited to - the following instances:
#
# 1a. Outright copyright infringement - Don't just copy this and change the
#     name.
# 1b. Selling the unmodified original with no work done what-so-ever, that's
#     REALLY being a dick.
# 1c. Modifying the original work to contain hidden harmful content.
#     That would make you a PROPER dick.
#
# If you become rich through modifications, related works/services, or
# supporting the original work, share the love.
# Only a dick would make loads off this work and not buy the original works
# creator(s) a pint.
#
# Code is provided with no warranty. Using somebody else's code and bitching
# when it goes wrong makes you a DONKEY dick.
# Fix the problem yourself. A non-dick would submit the fix back.

QT += gui
QT += core
QT += network
QT += widgets

INCLUDEPATH += $$PWD/src

SOURCES += \
    $$PWD/src/updater.cpp \
    $$PWD/src/downloader.cpp \
    $$PWD/src/qsimpleupdater.cpp \
    $$PWD/src/progress_dialog.cpp

HEADERS += \
    $$PWD/src/qsimpleupdater.h \
    $$PWD/src/updater.h \
    $$PWD/src/downloader.h \
    $$PWD/src/progress_dialog.h

FORMS += $$PWD/src/downloader.ui \
         $$PWD/src/progress_dialog.ui
		 
RESOURCES += $$PWD/res/qsu_resources.qrc
