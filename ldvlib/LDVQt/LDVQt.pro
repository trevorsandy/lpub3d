TEMPLATE = lib
QT      += core
QT      += opengl
QT      += network
QT      += widgets
QT      += gui
QT      += concurrent
QT      *= printsupport
CONFIG  += thread
CONFIG  += staticlib
CONFIG  += warn_on
CONFIG  += skip_target_version_ext
macx: \
CONFIG  -= app_bundle

greaterThan(QT_MAJOR_VERSION, 5) {
  QT += core5compat openglwidgets
  DEFINES += QOPENGLWIDGET
}

DEPENDPATH  += .
INCLUDEPATH += .
INCLUDEPATH += ../WPngImage
INCLUDEPATH += ../../mainApp
INCLUDEPATH += ../../lclib/common
INCLUDEPATH += ../../qslog
SYSTEM_PREFIX = /usr/local
macx {
    contains(QT_ARCH,arm64): SYSTEM_PREFIX = /opt/homebrew
    INCLUDEPATH += $${SYSTEM_PREFIX}/include
}
DEFINES += _QT
DEFINES += _NO_BOOST
DEFINES += _TC_STATIC
DEFINES += _WIN_UTF8_PATHS
DEFINES += QT_THREAD_SUPPORT
!freebsd: \
DEFINES += EXPORT_3DS

unix: !macx: TARGET = ldvqt
else:        TARGET = LDVQt

# platform name and version
BUILD_TARGET   = $$(TARGET_VENDOR)  # Platform ID
HOST_VERSION   = $$(PLATFORM_VER)   # Platform Version
BUILD_ARCH     = $$(TARGET_CPU)

# platform architecture, name and version fallback
!contains(QT_ARCH, unknown): BUILD_ARCH = $$QT_ARCH
else: isEmpty(BUILD_ARCH):   BUILD_ARCH = $$system(uname -m)
isEmpty(BUILD_ARCH):         BUILD_ARCH = UNKNOWN ARCH
isEmpty(BUILD_TARGET) {
    win32:BUILD_TARGET = $$system(systeminfo | findstr /B /C:\"OS Name\")
    unix:!macx:BUILD_TARGET = $$system(. /etc/os-release 2>/dev/null; [ -n \"$PRETTY_NAME\" ] && echo \"$PRETTY_NAME\" || echo `uname`)
    macx:BUILD_TARGET = $$system(echo `sw_vers -productName`)
}
isEmpty(HOST_VERSION) {
    win32:HOST_VERSION = $$system(systeminfo | findstr /B /C:\"OS Version\")
    unix:!macx:HOST_VERSION = $$system(. /etc/os-release 2>/dev/null; [ -n \"$VERSION_ID\" ] && echo \"$VERSION_ID\")
    macx:HOST_VERSION = $$system(echo `sw_vers -productVersion`)
}

# for aarch64, QT_ARCH = arm64, for arm7l, QT_ARCH = arm
if (contains(QT_ARCH, x86_64)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)) {
    ARCH  = 64
    LIB_ARCH = 64
} else {
    ARCH  = 32
    LIB_ARCH =
}

# specify flag for ARM platforms to use System OpenGL headers
!macx:contains(BUILD_ARCH,arm64)|contains(BUILD_ARCH,arm): \
UNIX_ARM_BUILD_ARCH = True

# The ABI version.
VER_MAJ = 4
VER_MIN = 5
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

VER_LDVIEW = ldview-4.5

CONFIG(debug, debug|release) {
    DEFINES += QT_DEBUG_MODE
    BUILD_CONF = Debug
    ARCH_BLD = bit_debug
    macx: TARGET = $$join(TARGET,,,_debug)
    win32: TARGET = $$join(TARGET,,,d$${VER_MAJ}$${VER_MIN})
    unix:!macx: TARGET = $$join(TARGET,,,d)
    # The remaining lines in this block adds the LDView header and source files...
    # This line requires a git extract of ldview at the same location as the lpub3d git extract
    VER_USE_LDVIEW_DEV = True
    # These lines defines the ldview git extract folder name, you can set as you like
    unix: VER_LDVIEW_DEV = ldview
    else:win32: VER_LDVIEW_DEV = ldview_vs_build
    # This line defines the path of the ldview git extract relative to this project file
    VER_LDVIEW_DEV_REPOSITORY = $$system_path( $$absolute_path( $$PWD/../../../$${VER_LDVIEW_DEV} ) )
    exists($$VER_LDVIEW_DEV_REPOSITORY) {
        message("~~~ lib$${TARGET} BUILD LDVQt USING LDVIEW DEVELOPMENT REPOSITORY ~~~ ")
        message("~~~ lib$${TARGET} ADD LDVIEW HEADERS TO INCLUDEPATH: $$VER_LDVIEW_DEV_REPOSITORY ~~~ ")
        INCLUDEPATH += $${VER_LDVIEW_DEV_REPOSITORY}
    } else {
        VER_USE_LDVIEW_DEV = False
        message("~~~ WARNING lib$${TARGET}: - COULD NOT LOAD LDVIEW DEV FROM: $$VER_LDVIEW_DEV_REPOSITORY ~~~ ")
    }
} else {
    BUILD_CONF = Release
    ARCH_BLD = bit_release
    win32: TARGET = $$join(TARGET,,,$${VER_MAJ}$${VER_MIN})
}
BUILD += $$BUILD_CONF Build
DESTDIR = $$join(ARCH,,,$$ARCH_BLD)

#~~ LDView headers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Load LDView headers
LOAD_LDV_HEADERS = True

include(LDViewLibs.pri)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

message("~~~ lib$${TARGET} $$join(ARCH,,,bit) $$BUILD_ARCH $${BUILD} ~~~")

# specify ARM platforms that need to suppress local glext.h header
contains(UNIX_ARM_BUILD_ARCH,True): contains(BUILD_TARGET,suse): contains(HOST_VERSION,1320) {
    DEFINES += ARM_SKIP_GL_HEADERS
    message("~~~ lib$${TARGET} $$upper($$QT_ARCH) build - $${BUILD_TARGET}-$${HOST_VERSION}-$${BUILD_ARCH} define SKIP_LDV_GL_HEADERS ~~~")
} else {
    message("~~~ lib$${TARGET} $$upper($$QT_ARCH) build - $${BUILD_TARGET}-$${HOST_VERSION}-$${BUILD_ARCH} ~~~")
}

PRECOMPILED_DIR = $$DESTDIR/.pch
OBJECTS_DIR     = $$DESTDIR/.obj
MOC_DIR         = $$DESTDIR/.moc
RCC_DIR         = $$DESTDIR/.qrc
UI_DIR          = $$DESTDIR/.ui

# USE GNU_SOURCE
unix:!macx: DEFINES += _GNU_SOURCE

# stdlib.h fix placeholder - do not remove

# USE CPP 11
contains(USE_CPP11,NO) {
  message("NO CPP11")
} else {
  DEFINES += USE_CPP11
}

contains(QT_VERSION, ^5\\..*) {
  unix:!macx {  
    GCC_VERSION = $$system(g++ -dumpversion)
    greaterThan(GCC_VERSION, 4.8) {
      QMAKE_CXXFLAGS += -std=c++11
    } else {
      QMAKE_CXXFLAGS += -std=c++0x
    }
  } else {
    CONFIG += c++11
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

#~~ suppress warnings ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

win32 {
    CONFIG       += windows incremental force_debug_info
    QMAKE_EXT_OBJ = .obj

    win32-msvc* {
        DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_WARNINGS=1
        DEFINES += _WINSOCKAPI_
        DEFINES += _TC_STATIC
        QMAKE_CXXFLAGS_RELEASE += /FI winsock2.h /FI winsock.h

        QMAKE_LFLAGS_WINDOWS += /IGNORE:4099
        QMAKE_CFLAGS_WARN_ON -= -W3
        QMAKE_ADDL_MSVC_FLAGS = -WX- -GS -Gd -fp:precise -Zc:forScope
        CONFIG(debug, debug|release) {
            QMAKE_ADDL_MSVC_DEBUG_FLAGS = -RTC1 $$QMAKE_ADDL_MSVC_FLAGS
            QMAKE_CFLAGS_WARN_ON += -W4  -wd"4005" -wd"4456" -wd"4458" -wd"4459" -wd"4127" -wd"4701"
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
    $$PWD/LDVAlertHandler.cpp \
    $$PWD/LDViewExportOption.cpp \
    $$PWD/LDVHtmlInventory.cpp \
    $$PWD/LDVImageMatte.cpp \
    $$PWD/LDViewPartList.cpp \
    $$PWD/LDVMisc.cpp \
    $$PWD/LDVPreferences.cpp \
    $$PWD/LDVWidget.cpp

win32-msvc* {
SOURCES += \
    $$PWD/LDVExtensionsSetup.cpp
}

HEADERS += \
    $$PWD/LDVWidgetDefaultKeys.h \
    $$PWD/LDVAlertHandler.h \
    $$PWD/LDVHtmlInventory.h \
    $$PWD/LDViewExportOption.h \
    $$PWD/LDVImageMatte.h \
    $$PWD/LDViewPartList.h \
    $$PWD/LDVMisc.h \
    $$PWD/LDVPreferences.h \
    $$PWD/LDVWidget.h

win32-msvc* {
HEADERS += \
    $$PWD/LDVExtensionsSetup.h
}

FORMS += \
    $$PWD/LDVExportOptionPanel.ui \
    $$PWD/LDVPreferencesPanel.ui \
    $$PWD/LDVPartList.ui

RESOURCES += \
    $$PWD/resources.qrc

OTHER_FILES += \
    $$PWD/LDVWidgetMessages.ini \
    $$PWD/../../mainApp/extras/ldvMessages.ini

# suppress warnings
!win32-msvc*:!macx {
QMAKE_CFLAGS_WARN_ON += \
     -Wall -W \
     -Wno-deprecated-declarations \
     -Wno-deprecated-copy \
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
     -Wno-deprecated-copy \
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
      -Wno-deprecated-copy \
      -Wno-format-security \
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
      -Wno-deprecated-copy \
      -Wno-overloaded-virtual \

QMAKE_CXXFLAGS_WARN_ON = $${QMAKE_CFLAGS_WARN_ON}

QMAKE_CFLAGS_WARN_ON += \
      -Wno-for-loop-analysis \
      -Wno-incompatible-pointer-types-discards-qualifiers \
      -Wno-int-conversion \
      -Wno-invalid-source-encoding \
      -Wno-mismatched-new-delete \
      -Wno-reorder \
      -Wno-undefined-bool-conversion
}
