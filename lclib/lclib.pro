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
INCLUDEPATH += ../mainApp ../ldvlib/LDVQt/include ../qsimpleupdater/include ../qsimpleupdater/src
INCLUDEPATH += ../qslog ../ldrawini ../quazip ../qsimpleupdater/src/progress_bar
win32-msvc* {
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib
}

# The ABI version.
VER_MAJ = 21
VER_MIN = 1
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

#QMAKE_CXXFLAGS  += $(Q_CXXFLAGS)
#QMAKE_LFLAGS    += $(Q_LDFLAGS)
#QMAKE_CFLAGS    += $(Q_CFLAGS)

DEFINES += LC_DISABLE_UPDATE_CHECK=1

# USE CPP 11
unix:!freebsd:!macx {
    GCC_VERSION = $$system(g++ -dumpversion)
    greaterThan(GCC_VERSION, 4.6) {
        QMAKE_CXXFLAGS += -std=c++11
    } else {
        QMAKE_CXXFLAGS += -std=c++0x
    }
} else {
    CONFIG += c++11
}

CONFIG += precompile_header
PRECOMPILED_HEADER = common/lc_global.h

win32 {

    win32-msvc* {
        DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_WARNINGS=1
        DEFINES += _WINSOCKAPI_
        DEFINES += _TC_STATIC

        QMAKE_CFLAGS_WARN_ON -= -W3
        QMAKE_ADDL_MSVC_FLAGS = -GS -Gd -fp:precise -Zc:forScope
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

    PRECOMPILED_SOURCE = common/lc_global.cpp
    QMAKE_EXT_OBJ = .obj

    LIBS += -ladvapi32 -lshell32 -lopengl32 -lwininet -luser32

    !win32-msvc* {
        LIBS += -lz
    }

} else {

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
