TEMPLATE = lib
CONFIG += qt warn_on
QT -= gui
unix:!macx: CONFIG += staticlib

# The ABI version.
# Version format is year.month.day.patch
win32: VERSION = 16.1.8.0   # major.minor.patch.build
else:  VERSION = 16.1.8     # major.minor.patch

contains(QT_ARCH, x86_64) {
    ARCH = 64
    STG_ARCH = x86_64
} else {
    ARCH = 32
    STG_ARCH = x86
}

win32 {

    QMAKE_EXT_OBJ = .obj
    CONFIG += windows

    QMAKE_TARGET_COMPANY = "Lars C. Hassing"
    QMAKE_TARGET_DESCRIPTION = "LDrawDir and SearchDirs API"
    QMAKE_TARGET_COPYRIGHT = "Copyright (c) 2004-2008  Lars C. Hassing"
    QMAKE_TARGET_PRODUCT = "LDrawIni ($$join(ARCH,,,bit))"
}

macx {
    QMAKE_CXXFLAGS += -F/System/Library/Frameworks
    LIBS += -framework CoreFoundation
}

CONFIG += skip_target_version_ext
unix: !macx: TARGET = ldrawini
else:        TARGET = LDrawIni

# Indicate build type
staticlib: BUILD = Static
else:      BUILD = Shared

CONFIG(debug, debug|release) {
    BUILD += Debug Build
    ARCH_BLD = bit_debug
    macx: TARGET = $$join(TARGET,,,_debug)
    win32: TARGET = $$join(TARGET,,,d161)
    unix:!macx: TARGET = $$join(TARGET,,,d)
} else {
    BUILD += Release Build
    ARCH_BLD = bit_release
    win32: TARGET = $$join(TARGET,,,161)
}
DESTDIR = $$join(ARCH,,,$$ARCH_BLD)
message("~~~ LDRAWINI $$join(ARCH,,,bit) $${BUILD} ~~~")

PRECOMPILED_DIR = $$DESTDIR/.pch
OBJECTS_DIR     = $$DESTDIR/.obj
MOC_DIR         = $$DESTDIR/.moc
RCC_DIR         = $$DESTDIR/.qrc
UI_DIR          = $$DESTDIR/.ui

QMAKE_EXT_CPP = .c

# Input files
include(ldrawini.pri)
include(../LPub3DPlatformSpecific.pri)

# Suppress warnings
QMAKE_CFLAGS_WARN_ON += -Wall -W \
    -Wno-sign-compare
QMAKE_CXXFLAGS_WARN_ON = $${QMAKE_CFLAGS_WARN_ON}
