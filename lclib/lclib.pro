TEMPLATE = lib
QT      += core
QT      += gui
QT      += opengl
QT      += network
QT      += xml
QT      += concurrent
QT      *= printsupport
CONFIG  += staticlib
CONFIG  += warn_on

win32:macx: \
GAMEPAD {
    qtHaveModule(gamepad) {
        QT += gamepad
        DEFINES += LC_ENABLE_GAMEPAD
    }
}

TARGET +=
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += qt common
INCLUDEPATH += ../mainApp ../qsimpleupdater/include ../qsimpleupdater/src
INCLUDEPATH += ../qslog ../ldrawini ../quazip ../qsimpleupdater/src/progress_bar

#~~ LDView headers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# 3rd party executables, documentation and resources.
!isEmpty(LP3D_3RD_DIST_DIR) {
    THIRD_PARTY_DIST_DIR_PATH = $$LP3D_3RD_DIST_DIR
} else {
    THIRD_PARTY_DIST_DIR_PATH = $$(LP3D_DIST_DIR_PATH)
    isEmpty(THIRD_PARTY_DIST_DIR_PATH): THIRD_PARTY_DIST_DIR_PATH="undefined"
}
!exists($$THIRD_PARTY_DIST_DIR_PATH) {
    unix:!macx: DIST_DIR      = lpub3d_linux_3rdparty
    else:macx: DIST_DIR       = lpub3d_macos_3rdparty
    else:win32: DIST_DIR      = lpub3d_windows_3rdparty
    THIRD_PARTY_DIST_DIR_PATH = $$system_path( $$absolute_path( $$PWD/../../$$DIST_DIR ) )
    !exists($$THIRD_PARTY_DIST_DIR_PATH) {
        message("~~~ ERROR - THIRD_PARTY_DIST_DIR_PATH (LCLIB) WAS NOT FOUND! ~~~ ")
        THIRD_PARTY_DIST_DIR_PATH="undefined"
    }
}
VER_LDVIEW  = ldview-4.5

# Copy LDView headers (Disabled)
#INCLUDEPATH += ../ldvlib/LDVQt/include

# Reference LDView headers
INCLUDEPATH += $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/include )

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

win32-msvc* {
    INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib
}

# The ABI version.
# REMINDER: Update LC_LIB = LC233 in mainApp.pro on version change
VER_MAJ = 23
VER_MIN = 3
VER_PAT = 0
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

# platform switch
BUILD_ARCH   = $$(TARGET_CPU)
!contains(QT_ARCH, unknown):  BUILD_ARCH = $$QT_ARCH
else: isEmpty(BUILD_ARCH):    BUILD_ARCH = UNKNOWN ARCH
if (contains(QT_ARCH, x86_64)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)) {
    ARCH  = 64
} else {
    ARCH  = 32
}

DEFINES += LC_DISABLE_UPDATE_CHECK=1

equals(QT_MAJOR_VERSION, 5) {
    win32-msvc* {
        QMAKE_CXXFLAGS += /std:c++17
    } else:unix {
        greaterThan(QT_MINOR_VERSION, 11) {
            CONFIG += c++17
        } else {
            QMAKE_CXXFLAGS += -std=c++17
        }
    }
}

contains(QT_VERSION, ^6\\..*) {
    win32-msvc* {
        QMAKE_CXXFLAGS += /std:c++17
    }
    macx {
        QMAKE_CXXFLAGS+= -std=c++17
    }
    unix:!macx {
        GCC_VERSION = $$system(g++ -dumpversion)
        greaterThan(GCC_VERSION, 5) {
            QMAKE_CXXFLAGS += -std=c++17
        } else {
            QMAKE_CXXFLAGS += -std=c++0x
        }
    }
}

CONFIG += incremental force_debug_info

win32 {

    win32-msvc* {
        CONFIG += precompile_header

        DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_WARNINGS=1
        DEFINES += _WINSOCKAPI_
        DEFINES += _TC_STATIC

        QMAKE_LFLAGS_WINDOWS += /IGNORE:4099
        QMAKE_CFLAGS_WARN_ON -= -W3
        QMAKE_ADDL_MSVC_FLAGS = -GS -Gd -fp:precise -Zc:forScope

        PRECOMPILED_HEADER = common/lc_global.h
        PRECOMPILED_SOURCE = common/lc_global.cpp

        CONFIG(debug, debug|release) {
            DEFINES += QT_DEBUG_MODE
            QMAKE_ADDL_MSVC_DEBUG_FLAGS = -RTC1 $$QMAKE_ADDL_MSVC_FLAGS
            QMAKE_CFLAGS_WARN_ON += -W4 -WX- -wd"4005" -wd"4456" -wd"4458" -wd"4459" -wd"4127" -wd"4701"
            QMAKE_CFLAGS_DEBUG   += $$QMAKE_ADDL_MSVC_DEBUG_FLAGS
            QMAKE_CXXFLAGS_DEBUG += $$QMAKE_ADDL_MSVC_DEBUG_FLAGS
        }
        CONFIG(release, debug|release) {
            QMAKE_ADDL_MSVC_RELEASE_FLAGS = $$QMAKE_ADDL_MSVC_FLAGS -GF -Gy
            QMAKE_CFLAGS_OPTIMIZE += -Ob1 -Oi -Ot
            QMAKE_CFLAGS_WARN_ON  += -W1 -WX- -wd"4005" -wd"4456" -wd"4458" -wd"4714" -wd"4805"
            QMAKE_CFLAGS_RELEASE  += $$QMAKE_ADDL_MSVC_RELEASE_FLAGS
            QMAKE_CXXFLAGS_RELEASE += $$QMAKE_ADDL_MSVC_RELEASE_FLAGS
        }
        QMAKE_CXXFLAGS_WARN_ON = $$QMAKE_CFLAGS_WARN_ON
    }

    QMAKE_EXT_OBJ = .obj

    LIBS += -ladvapi32 -lshell32 -lopengl32 -lwininet -luser32

    !win32-msvc* {
        LIBS += -lz
    }

} else {

    CONFIG += precompile_header
    PRECOMPILED_HEADER = common/lc_global.h
    LIBS += -lz

}

CONFIG += skip_target_version_ext

unix: !macx: TARGET = lc
else:        TARGET = LC

# Indicate build type
staticlib {
    BUILD    = Static
} else {
    BUILD    = Shared
}

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

include(lclib.pri)

# Suppress warnings
!win32-msvc*:!macx {
QMAKE_CFLAGS_WARN_ON += \
    -Wno-deprecated-declarations \
    -Wno-deprecated-copy \
    -Wno-unused-parameter \
    -Wno-sign-compare
}
macx {

QMAKE_CFLAGS_WARN_ON += \
    -Wno-deprecated-copy \
    -Wno-overloaded-virtual \
    -Wno-sometimes-uninitialized \
    -Wno-self-assign \
    -Wno-unused-result
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}

} else: win32 {

!win32-msvc* {
QMAKE_CFLAGS_WARN_ON += \
    -Wno-attributes
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}

QMAKE_CFLAGS_WARN_ON += \
   -Wno-misleading-indentation
QMAKE_CXXFLAGS_WARN_ON += \
   -Wno-maybe-uninitialized \
   -Wno-implicit-fallthrough \
   -Wno-strict-aliasing \
   -Wno-unused-result \
   -Wno-cpp
}

} else {
QMAKE_CFLAGS_WARN_ON += \
    -Wno-strict-aliasing
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}
}
