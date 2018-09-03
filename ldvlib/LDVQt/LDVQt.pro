TEMPLATE = lib
QT      += core
QT      += opengl
QT      += widgets
QT      += gui
CONFIG  += thread
CONFIG  += staticlib
CONFIG  += warn_on
CONFIG  += skip_target_version_ext
macx: \
CONFIG  -= app_bundle

DEPENDPATH  += .
INCLUDEPATH += .
DEPENDPATH  += include
INCLUDEPATH += include
INCLUDEPATH += ../WPngImage
INCLUDEPATH += ../../mainApp
INCLUDEPATH += ../../qslog
macx: \
INCLUDEPATH += /usr/local/include

DEFINES += _QT
DEFINES += _NO_BOOST
DEFINES += _TC_STATIC
DEFINES += QT_THREAD_SUPPORT

# platform switch
HOST_VERSION   = $$(PLATFORM_VER)
BUILD_TARGET   = $$(TARGET_VENDOR)
BUILD_ARCH     = $$(TARGET_CPU)
!contains(QT_ARCH, unknown):  BUILD_ARCH = $$QT_ARCH
else: isEmpty(BUILD_ARCH):    BUILD_ARCH = UNKNOWN ARCH
contains(HOST_VERSION, 1320):contains(BUILD_TARGET, suse):contains(BUILD_ARCH, aarch64): DEFINES += _OPENSUSE_1320_ARM
if (contains(QT_ARCH, x86_64)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)) {
    ARCH  = 64
    LIB_ARCH = 64
} else {
    ARCH  = 32
    LIB_ARCH =
}

# Setup LDVQt headers
LOAD_LDVHEADERS = True
include(LDViewLibs.pri)

unix: !macx: TARGET = ldvqt
else:        TARGET = LDVQt

# The ABI version.
VER_MAJ = 4
VER_MIN = 3
VER_PAT = 0
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

# Indicate build type
staticlib {
    BUILD    = Static
} else {
    BUILD    = Shared
}

CONFIG(debug, debug|release) {
    BUILD_CONF = Debug
    ARCH_BLD = bit_debug
    macx: TARGET = $$join(TARGET,,,_debug)
    win32: TARGET = $$join(TARGET,,,d$${VER_MAJ}$${VER_MIN})
    unix:!macx: TARGET = $$join(TARGET,,,d)
} else {
    BUILD_CONF = Release
    ARCH_BLD = bit_release
    win32: TARGET = $$join(TARGET,,,$${VER_MAJ}$${VER_MIN})
}
BUILD += $$BUILD_CONF Build
DESTDIR = $$join(ARCH,,,$$ARCH_BLD)

message("~~~ lib$${TARGET} $$join(ARCH,,,bit) $$BUILD_ARCH $${BUILD} ~~~")

PRECOMPILED_DIR = $$DESTDIR/.pch
OBJECTS_DIR     = $$DESTDIR/.obj
MOC_DIR         = $$DESTDIR/.moc
RCC_DIR         = $$DESTDIR/.qrc
UI_DIR          = $$DESTDIR/.ui

# USE GNU_SOURCE
unix:!macx: DEFINES += _GNU_SOURCE

QMAKE_CXXFLAGS  += $(Q_CXXFLAGS)
QMAKE_LFLAGS    += $(Q_LDFLAGS)
QMAKE_CFLAGS    += $(Q_CFLAGS)
# stdlib.h fix placeholder - do not remove

# USE CPP 11
unix:!macx {
    GCC_VERSION = $$system(g++ -dumpversion)
    greaterThan(GCC_VERSION, 4.6) {
        QMAKE_CXXFLAGS += -std=c++11
        DEFINES += USE_CPP11
    } else {
        QMAKE_CXXFLAGS += -std=c++0x
    }
} else {
    CONFIG += c++11
    DEFINES += USE_CPP11
}

win32 {
    CONFIG       += windows
    QMAKE_EXT_OBJ = .obj

    win32-msvc* {
        DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_WARNINGS=1
        DEFINES += _WINSOCKAPI_
        DEFINES += _TC_STATIC
        QMAKE_CXXFLAGS_RELEASE += /FI winsock2.h /FI winsock.h

        QMAKE_CFLAGS_WARN_ON -= -W3
        QMAKE_ADDL_MSVC_FLAGS = -GS -Gd -fp:precise -Zc:forScope
        CONFIG(debug, debug|release) {
            QMAKE_ADDL_MSVC_DEBUG_FLAGS = -RTC1 -Gm $$QMAKE_ADDL_MSVC_FLAGS
            QMAKE_CFLAGS_WARN_ON += -W4 -WX- -wd"4005" -wd"4456" -wd"4458" -wd"4459" -wd"4127" -wd"4701"
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
}

SOURCES += \
    LDVAlertHandler.cpp \
    LDViewExportOption.cpp \
    LDVImageMatte.cpp \
    LDVMisc.cpp \
    LDVPreferences.cpp \
    LDVWidget.cpp

HEADERS += \
    LDVAlertHandler.h \
    LDViewExportOption.h \
    LDVImageMatte.h \
    LDVMisc.h \
    LDVPovUserDefaultsKeys.h \
    LDVPreferences.h \
    LDVWidget.h

FORMS += \
    LDVExportOptionPanel.ui \
    LDVPreferencesPanel.ui

# suppress warnings
!win32-msvc* {
QMAKE_CFLAGS_WARN_ON += \
     -Wall -W \
     -Wno-deprecated-declarations \
     -Wno-implicit-fallthrough \
     -Wno-parentheses \
     -Wno-return-type \
     -Wno-sign-compare \
     -Wno-uninitialized \
     -Wno-unknown-pragmas \
     -Wno-unused-parameter \
     -Wno-unused-result \
     -Wno-unused-variable
}
macx {
QMAKE_CFLAGS_WARN_ON += \
     -Wno-for-loop-analysis \
     -Wno-implicit-function-declaration \
     -Wno-incompatible-pointer-types \
     -Wno-incompatible-pointer-types-discards-qualifiers \
     -Wno-int-conversion \
     -Wno-invalid-source-encoding \
     -Wno-mismatched-new-delete \
     -Wno-nullability-completeness \
     -Wno-reorder \
     -Wno-undefined-bool-conversion
} else {
!win32-msvc* {
QMAKE_CFLAGS_WARN_ON += \
     -Wno-clobbered
}
}
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}

!win32-msvc*:!macx {
QMAKE_CFLAGS_WARN_ON =  \
      -Wno-comment \
      -Wno-deprecated-declarations \
      -Wno-format \
      -Wno-parentheses \
      -Wno-return-type \
      -Wno-sign-compare \
      -Wno-switch \
      -Wno-uninitialized \
      -Wno-unused-but-set-variable \
      -Wno-unused-parameter \
      -Wno-unused-result \
      -Wno-unused-variable

QMAKE_CXXFLAGS_WARN_ON = $${QMAKE_CFLAGS_WARN_ON}

QMAKE_CFLAGS_WARN_ON +=  \
      -Wno-implicit-function-declaration \
      -Wno-incompatible-pointer-types
}

macx {
QMAKE_CFLAGS_WARN_ON += \
      -Wno-for-loop-analysis \
      -Wno-incompatible-pointer-types-discards-qualifiers \
      -Wno-int-conversion \
      -Wno-invalid-source-encoding \
      -Wno-mismatched-new-delete \
      -Wno-reorder \
      -Wno-undefined-bool-conversion
}
