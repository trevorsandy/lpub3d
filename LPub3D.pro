TEMPLATE=subdirs
CONFIG+=ordered # This tells Qt to compile the following SUBDIRS in order

win32:HOST = $$system(systeminfo | findstr /B /C:\"OS Name\")
unix:!macx:HOST = $$system(. /etc/os-release 2>/dev/null; [ -n \"$PRETTY_NAME\" ] && echo \"$PRETTY_NAME\" || echo `uname`)
macx:HOST = $$system(echo `sw_vers -productName` `sw_vers -productVersion`)
isEmpty(HOST):HOST = UNKNOWN HOST

BUILD_ARCH = $$(TARGET_CPU)
!contains(QT_ARCH, unknown):  BUILD_ARCH = $$QT_ARCH
else: isEmpty(BUILD_ARCH):    BUILD_ARCH = UNKNOWN ARCH
CONFIG(debug, debug|release): BUILD = DEBUG BUILD
else:	                        BUILD = RELEASE BUILD

message("~~~ LPUB3D $$upper($$BUILD_ARCH) $${BUILD} ON $$upper($$HOST) ~~~")

SUBDIRS = 3rdParty_ldrawini
3rdParty_ldrawini.subdir   = $$PWD/ldrawini
3rdParty_ldrawini.makefile = Makefile.ldrawini
3rdParty_ldrawini.target   = sub-ldrawini
3rdParty_ldrawini.depends  =

isEmpty(quazipnobuild) {
  SUBDIRS += 3rdParty_quazip
  3rdParty_quazip.subdir   = $$PWD/quazip
  3rdParty_quazip.makefile = Makefile.quazip
  3rdParty_quazip.target   = sub-quazip
  3rdParty_quazip.depends  =
}

if (unix:exists(/usr/include/gl2ps.h)|exists(/usr/local/include/gl2ps.h)) {
    message("~~~ Library gl2ps found ~~~")
} else {
    !win32:message("~~~ ALERT: library gl2ps not found, building it in... ~~~")
    CONFIG+=BUILD_GL2PS
}
if (unix:exists(/usr/include/png.h)|exists(/usr/local/include/png.h)|BUILD_GL2PS) {
    if (contains(HOST, Ubuntu):contains(HOST, 14.04.5)) {
        message("~~~ $$HOST detected, building in libpng version 1.6.28... ~~~")
        CONFIG+=BUILD_PNG
    } else:BUILD_GL2PS {
        CONFIG+=BUILD_PNG
    }
} else {
    !win32:message("~~~ ALERT: library libpng not found, building it in... ~~~")
    CONFIG+=BUILD_PNG
}
if (unix:exists(/usr/include/jpeglib.h)|exists(/usr/local/include/jpeglib.h)) {
    message("~~~ Library jpeglib found ~~~")
} else {
    !win32:message("~~~ ALERT: library jpeglib not found, building it in... ~~~")
    CONFIG+=BUILD_JPEG
}
if (unix:exists(/usr/include/tinyxml.h)|exists(/usr/local/include/tinyxml.h)) {
    message("~~~ Library tinyxml found ~~~")
} else {
    !win32:message("~~~ ALERT: library tinyxml not found, building it in... ~~~")
    CONFIG+=BUILD_TINYXML
}

if (BUILD_PNG) {
    SUBDIRS += 3rdParty_png
    3rdParty_png.subdir      = $$PWD/ldvlib/libpng
    3rdParty_png.makefile    = Makefile.libpng
    3rdParty_png.target      = sub-3rdParty_png
    3rdParty_png.depends     =
}

if (BUILD_TINYXML) {
    SUBDIRS += 3rdParty_tinyxml
    3rdParty_tinyxml.subdir   = $$PWD/ldvlib/tinyxml
    3rdParty_tinyxml.makefile = Makefile.libtinyxml
    3rdParty_tinyxml.target   = sub-3rdParty_tinyxml
    3rdParty_tinyxml.depends  =
}

if (BUILD_GL2PS) {
    SUBDIRS += 3rdParty_gl2ps
    3rdParty_gl2ps.subdir     = $$PWD/ldvlib/gl2ps
    3rdParty_gl2ps.makefile   = Makefile.libgl2ps
    3rdParty_gl2ps.target     = sub-3rdParty_gl2ps
    3rdParty_gl2ps.depends    = 3rdParty_png
}

if (BUILD_JPEG) {
    SUBDIRS += 3rdParty_jpeg
    3rdParty_jpeg.subdir     = $$PWD/ldvlib/libjpeg
    3rdParty_jpeg.makefile   = Makefile.libjpeg
    3rdParty_jpeg.target     = sub-3rdParty_jpeg
    3rdParty_jpeg.depends    =
}

SUBDIRS += tcfoundation
tcfoundation.subdir   = $$PWD/ldvlib/TCFoundation
tcfoundation.makefile = Makefile.tcfoundation
tcfoundation.target   = sub-tcfoundation
if (BUILD_PNG) {
    tcfoundation.depends = 3rdParty_png
}
if (BUILD_JPEG) {
    tcfoundation.depends = 3rdParty_jpeg
}

SUBDIRS += tre
tre.subdir   = $$PWD/ldvlib/TRE
tre.makefile = Makefile.tre
tre.target   = sub-tre
tre.depends  = tcfoundation
if (BUILD_GL2PS) {
    tre.depends = 3rdParty_gl2ps
}

SUBDIRS += ldloader
ldloader.subdir    = $$PWD/ldvlib/LDLoader
ldloader.makefile  = Makefile.ldloader
ldloader.target    = sub-ldloader
ldexporter.depends = tre

SUBDIRS += ldexporter
ldexporter.subdir   = $$PWD/ldvlib/LDExporter
ldexporter.makefile = Makefile.ldexporter
ldexporter.target   = sub-ldexporter
ldexporter.depends  = ldloader
if (BUILD_TINYXML) {
    ldexporter.depends = 3rdParty_tinyxml
}

SUBDIRS += ldlib
ldlib.subdir   = $$PWD/ldvlib/LDLib
ldlib.makefile = Makefile.ldlib
ldlib.target   = sub-ldlib
ldlib.depends  = ldexporter
if (BUILD_GL2PS) {
    ldlib.depends = 3rdParty_gl2ps
}

SUBDIRS += ldvqt
ldvqt.subdir   = $$PWD/ldvlib/LDVQt
ldvqt.makefile = Makefile.ldvqt
ldvqt.target   = sub-ldvqt
ldvqt.depends  = ldlib

SUBDIRS += lclib
lclib.subdir   = $$PWD/lclib
lclib.makefile = Makefile.lc
lclib.target   = sub-lclib
lclib.depends  =

SUBDIRS += mainApp
mainApp.subdir   = $$PWD/mainApp
mainApp.makefile = Makefile.mainapp
mainApp.target   = sub-mainApp
mainApp.depends  = quazip
mainApp.depends  = ldrawini
mainApp.depends  = ldlib
mainApp.depends  = ldvqt

RESOURCES += \
    qsimpleupdater/res/qsu_resources.qrc \
    mainApp/lpub3d.qrc
