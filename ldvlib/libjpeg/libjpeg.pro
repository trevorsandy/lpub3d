TEMPLATE = lib
CONFIG  += staticlib

unix: !macx: TARGET = jpeg
else:        TARGET = JPEG

# The ABI version.
VER_MAJ = 9
VER_MIN = 2
VER_PAT = 0
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

LDVLIB_WARNINGS = true
include(../ldvlib.pri)

message("~~~ lib$${TARGET} $$join(ARCH,,,bit) $$BUILD_ARCH $${BUILD} ~~~")

include(libjpeg.pri)
