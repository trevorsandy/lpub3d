TEMPLATE = lib
CONFIG  += staticlib

unix: !macx: TARGET = tinyxml
else:        TARGET = TinyXml

# The ABI version.
VER_MAJ = 2
VER_MIN = 6
VER_PAT = 2
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

LDVLIB_WARNINGS = true
include(../ldvlib.pri)

message("~~~ lib$${TARGET} $$join(ARCH,,,bit) $$BUILD_ARCH $${BUILD} ~~~")

DEFINES += TIXML_USE_STL

include(tinyxml.pri)
