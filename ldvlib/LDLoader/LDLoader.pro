TEMPLATE = lib
CONFIG  += core
CONFIG  += staticlib

unix: !macx: TARGET = ldloader
else:        TARGET = LDLoader

# The ABI version.
VER_MAJ = 4
VER_MIN = 3
VER_PAT = 0
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

LDV_WARNINGS = true
include(../ldvlib.pri)

message("~~~ lib$${TARGET} $$join(ARCH,,,bit) $$BUILD_ARCH $${BUILD} ~~~")

include(LDLoader.pri)
