TEMPLATE = lib
CONFIG  += staticlib

unix: !macx: TARGET = png
else:        TARGET = PNG

# The ABI version.
VER_MAJ = 1
VER_MIN = 6
VER_PAT = 28
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

win32-msvc* {
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib
}

LDVLIB_WARNINGS = true
include(../ldvlib.pri)

message("~~~ lib$${TARGET} $$join(ARCH,,,bit) $$BUILD_ARCH $${BUILD} ~~~")

include(libpng.pri)
