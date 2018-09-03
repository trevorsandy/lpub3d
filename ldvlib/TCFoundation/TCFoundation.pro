TEMPLATE = lib
QT      += core
QT 	+= opengl
CONFIG  += staticlib

unix: !macx: TARGET = tcfoundation
else:        TARGET = TCFoundation

# The ABI version.
VER_MAJ = 4
VER_MIN = 3
VER_PAT = 0
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

LDV_WARNINGS = true
include(../ldvlib.pri)

if (macx:|win32:) {
DEFINES += HAVE_MINIZIP
} else {
DEFINES += UNZIP_CMD
}

message("~~~ lib$${TARGET} $$join(ARCH,,,bit) $$BUILD_ARCH $${BUILD} ~~~")

include(TCFoundation.pri)
