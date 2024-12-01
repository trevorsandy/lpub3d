TEMPLATE = lib
CONFIG += qt warn_on
QT -= gui
CONFIG += staticlib

# The ABI version.
VER_MAJ = 16
VER_MIN = 1
VER_PAT = 8
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

BUILD_ARCH   = $$(TARGET_CPU)
!contains(QT_ARCH, unknown):  BUILD_ARCH = $$QT_ARCH
else: isEmpty(BUILD_ARCH):    BUILD_ARCH = UNKNOWN ARCH
if (contains(QT_ARCH, x86_64)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)) {
    ARCH     = 64
    STG_ARCH = x86_64
} else {
    ARCH     = 32
    STG_ARCH = x86
}

win32 {

    QMAKE_EXT_OBJ = .obj
    CONFIG += windows

    win32-msvc* {
        DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_WARNINGS=1

        QMAKE_CFLAGS_WARN_ON -= -W3
        QMAKE_ADDL_MSVC_FLAGS = -WX- -GS -Gd -fp:precise -Zc:forScope
        CONFIG(debug, debug|release) {
            QMAKE_ADDL_MSVC_DEBUG_FLAGS = -RTC1 $$QMAKE_ADDL_MSVC_FLAGS
            QMAKE_CFLAGS_WARN_ON += -W4 
            QMAKE_CFLAGS_DEBUG   += $$QMAKE_ADDL_MSVC_DEBUG_FLAGS
            QMAKE_CXXFLAGS_DEBUG += $$QMAKE_ADDL_MSVC_DEBUG_FLAGS
        }
        CONFIG(release, debug|release) {
            QMAKE_ADDL_MSVC_RELEASE_FLAGS = $$QMAKE_ADDL_MSVC_FLAGS -GF -Gy
            QMAKE_CFLAGS_OPTIMIZE += -Ob1 -Oi -Ot
            QMAKE_CFLAGS_WARN_ON  += -W1 -WX- -wd"4005" -wd"4456" -wd"4458" -wd"4805"
            QMAKE_CFLAGS_RELEASE  += $$QMAKE_ADDL_MSVC_RELEASE_FLAGS
            QMAKE_CXXFLAGS_RELEASE += $$QMAKE_ADDL_MSVC_RELEASE_FLAGS
        }
        QMAKE_CXXFLAGS_WARN_ON = $$QMAKE_CFLAGS_WARN_ON
    }

    QMAKE_TARGET_COMPANY = "Lars C. Hassing"
    QMAKE_TARGET_DESCRIPTION = "LDrawDir and SearchDirs API"
    QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2004-2008  Lars C. Hassing"
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
    win32: TARGET = $$join(TARGET,,,d$${VER_MAJ}$${VER_MIN})
    unix:!macx: TARGET = $$join(TARGET,,,d)
} else {
    BUILD += Release Build
    ARCH_BLD = bit_release
    win32: TARGET = $$join(TARGET,,,$${VER_MAJ}$${VER_MIN})
}
DESTDIR = $$join(ARCH,,,$$ARCH_BLD)
message("~~~ lib$${TARGET} $$join(ARCH,,,bit) $$BUILD_ARCH $${BUILD} ~~~")

PRECOMPILED_DIR = $$DESTDIR/.pch
OBJECTS_DIR     = $$DESTDIR/.obj
MOC_DIR         = $$DESTDIR/.moc
RCC_DIR         = $$DESTDIR/.qrc
UI_DIR          = $$DESTDIR/.ui

QMAKE_EXT_CPP = .c

# Input files
include(ldrawini.pri)

# Suppress warnings
!win32-msvc* {
QMAKE_CFLAGS_WARN_ON += -Wall -W \
    -Wno-sign-compare
QMAKE_CXXFLAGS_WARN_ON = $${QMAKE_CFLAGS_WARN_ON}
}
