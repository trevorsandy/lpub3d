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
else:	                      BUILD = RELEASE BUILD

message("~~~ LPUB3D $$upper($$BUILD_ARCH) $${BUILD} ON $$upper($$HOST) ~~~")

SUBDIRS = ldrawini
ldrawini.subdir   = $$PWD/ldrawini
ldrawini.makefile = Makefile.ldrawini
ldrawini.target   = sub-ldrawini
ldrawini.depends  =

isEmpty(quazipnobuild) {
  SUBDIRS += quazip
  quazip.subdir   = $$PWD/quazip
  quazip.makefile = Makefile.quazip
  quazip.target   = sub-quazip
  quazip.depends  =
}

if (unix:exists(/usr/include/tinyxml.h)|exists(/usr/local/include/tinyxml.h)) {
    message("~~~ System library tinyxml found ~~~")
} else:!win32-msvc* {
    message("~~~ ALERT: System library tinyxml not found, using local... ~~~")
}
if (unix:exists(/usr/include/gl2ps.h)|exists(/usr/local/include/gl2ps.h)) {
    message("~~~ System library gl2ps found ~~~")
} else:!win32-msvc* {
    message("~~~ ALERT: System library gl2ps not found, using local ~~~")
}
if (unix:exists(/usr/include/lib3ds.h)|exists(/usr/local/include/lib3ds.h)){
    message("~~~ System library 3ds found ~~~")
} else:!win32-msvc* {
    message("~~~ ALERT: System library 3ds not found, using local... ~~~")
}
if (unix:macx:exists(/usr/include/zip.h)|exists(/usr/local/include/minizip/zip.h)) {
    message("~~~ System library minizip found ~~~")
} else:macx:!win32-msvc* {
    message("~~~ ALERT: System library minizip not found, using local... ~~~")
}
if (unix:exists(/usr/include/png.h)|exists(/usr/local/include/png.h)) {
    message("~~~ System library png found ~~~")
} else:!win32-msvc* {
    message("~~~ ALERT: System library png not found, using local... ~~~")
}
if (unix:exists(/usr/include/jpeglib.h)|exists(/usr/local/include/jpeglib.h)) {
    message("~~~ System library jpeglib found ~~~")
} else:!win32-msvc* {
    message("~~~ ALERT: System library jpeg not found, using local... ~~~")
}

SUBDIRS += ldvqt
ldvqt.subdir   = $$PWD/ldvlib/LDVQt
ldvqt.makefile = Makefile.ldvqt
ldvqt.target   = sub-ldvqt
ldvqt.depends  = #ldlib

SUBDIRS += wpngimage
wpngimage.subdir   = $$PWD/ldvlib/WPngImage
wpngimage.makefile = Makefile.WPngImage
wpngimage.target   = sub-WPngImage
wpngimage.depends  =

SUBDIRS += lclib
lclib.subdir   = $$PWD/lclib
lclib.makefile = Makefile.lc
lclib.target   = sub-lclib
lclib.depends  =

SUBDIRS += mainApp
mainApp.subdir   = $$PWD/mainApp
mainApp.makefile = Makefile.mainapp
mainApp.target   = sub-mainApp
isEmpty(quazipnobuild): \
mainApp.depends  = quazip
mainApp.depends  = ldrawini
mainApp.depends  = lclib
mainApp.depends  = ldvqt
mainApp.depends  = wpngimage

RESOURCES += \
    qsimpleupdater/etc/resources/qsimpleupdater.qrc \
    mainApp/lpub3d.qrc
