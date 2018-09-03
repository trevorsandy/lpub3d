TEMPLATE = lib
CONFIG  += staticlib

unix: !macx: TARGET = gl2ps
else:        TARGET = GL2PS

# The ABI version.
VER_MAJ = 1
VER_MIN = 3
VER_PAT = 3
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

LDVLIB_WARNINGS = true
include(../ldvlib.pri)

message("~~~ lib$${TARGET} $$join(ARCH,,,bit) $$BUILD_ARCH $${BUILD} ~~~")

DEFINES += GL2PS_HAVE_ZLIB \
           GL2PS_HAVE_LIBPNG

win32-msvc*: \
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib
INCLUDEPATH += $$_PRO_FILE_PWD_/../libpng
macx: INCLUDEPATH += $$_PRO_FILE_PWD_/../libpng/MacOSX

# use libpng14 or higher. Ubunu's default is libpng12
CONFIG(debug, debug|release) {
    macx:PNG_LIB       = PNG_debug
    win32:PNG_LIB      = PNGd16
    unix:!macx:PNG_LIB = pngd
} else {
    macx::PNG_LIB      = PNG
    win32:PNG_LIB      = PNG16
    unix:!macx:PNG_LIB = png
}

PNG_LIBDIR  = ../libpng/$$DESTDIR
win32-msvc* {
    ORIG_LIB_NAME = $${PNG_LIB}.lib
    LIB_NAME = png.lib
} else {
    ORIG_LIB_NAME = lib$${PNG_LIB}.a
    LIB_NAME = libpng.a
}
win32:PNG_COPY_CMD = CD $${PNG_LIBDIR} && IF NOT EXIST \"$${LIB_NAME}\" COPY $${ORIG_LIB_NAME} $${LIB_NAME}
else:PNG_COPY_CMD = cd $${PNG_LIBDIR}; if ! test -f $${LIB_NAME}; then ln -s $${ORIG_LIB_NAME} $${LIB_NAME}; fi
copypng.target = $${ORIG_LIB_NAME}
copypng.depends = $${PNG_LIBDIR}/$${ORIG_LIB_NAME}
copypng.commands = $${PNG_COPY_CMD}
QMAKE_EXTRA_TARGETS += copypng
PRE_TARGETDEPS += $${ORIG_LIB_NAME}
LIBS += -L$${PNG_LIBDIR} $${PNG_LIBDIR}/$${ORIG_LIB_NAME}

# Input
HEADERS += $$PWD/gl2ps.h
SOURCES += $$PWD/gl2ps.c
