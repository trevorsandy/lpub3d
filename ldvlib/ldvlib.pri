QT           -= gui
CONFIG       += warn_on
macx: CONFIG -= app_bundle

DEPENDPATH  += .
INCLUDEPATH += .
INCLUDEPATH += $$PWD/../mainApp

DEFINES += _QT
DEFINES += _NO_BOOST
DEFINES += _TC_STATIC
DEFINES += QT_THREAD_SUPPORT

CONFIG += skip_target_version_ext

# platform switch
BUILD_ARCH = $$(TARGET_CPU)
if (contains(QT_ARCH, x86_64)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)) {
    ARCH  = 64
} else {
    ARCH  = 32
}

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

PRECOMPILED_DIR = $$DESTDIR/.pch
OBJECTS_DIR     = $$DESTDIR/.obj
MOC_DIR         = $$DESTDIR/.moc
RCC_DIR         = $$DESTDIR/.qrc
UI_DIR          = $$DESTDIR/.ui

INCLUDEPATH    += $$PWD $$PWD/include
!contains(LDVQT_BUILD, true) {
unix:INCLUDEPATH += /usr/include
macx:INCLUDEPATH += /usr/local/include
}

macx:exists(/usr/X11/include){
    message("~~~ X11 found ~~~")
    INCLUDEPATH += /usr/X11/include
}

# USE GNU_SOURCE
unix:!macx: DEFINES += _GNU_SOURCE

QMAKE_CXXFLAGS  += $(Q_CXXFLAGS)
QMAKE_LFLAGS    += $(Q_LDFLAGS)
QMAKE_CFLAGS    += $(Q_CFLAGS)

# USE CPP 11
unix:!freebsd:!macx {
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
    win32-msvc*: \
    DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_WARNINGS=1
    DEFINES += _WINSOCKAPI_
    QMAKE_CXXFLAGS_RELEASE += /FI winsock2.h /FI winsock.h
}

# suppress warnings
contains(LDV_WARNINGS, true) {
    !win32-msvc* {
    QMAKE_CFLAGS_WARN_ON += \
         -Wall -W \
         -Wno-unknown-pragmas \
         -Wno-unused-parameter \
         -Wno-parentheses \
         -Wno-unused-variable \
         -Wno-deprecated-declarations \
         -Wno-return-type \
         -Wno-sign-compare \
         -Wno-uninitialized \
         -Wno-unused-result \
         -Wno-implicit-fallthrough
    }
    macx {
    QMAKE_CFLAGS_WARN_ON += \
         -Wno-implicit-function-declaration \
         -Wno-incompatible-pointer-types-discards-qualifiers \
         -Wno-incompatible-pointer-types \
         -Wno-nullability-completeness \
         -Wno-undefined-bool-conversion \
         -Wno-invalid-source-encoding \
         -Wno-mismatched-new-delete \
         -Wno-for-loop-analysis \
         -Wno-int-conversion \
         -Wno-reorder
    } else {
    !win32-msvc* {
    QMAKE_CFLAGS_WARN_ON += \
         -Wno-clobbered
    }
    }
    QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}
}
contains(LDVLIB_WARNINGS, true) {
    !win32-msvc* {
    QMAKE_CFLAGS_WARN_ON =  \
          -Wno-unused-parameter \
          -Wno-parentheses \
          -Wno-unused-variable \
          -Wno-deprecated-declarations \
          -Wno-return-type \
          -Wno-sign-compare \
          -Wno-uninitialized \
          -Wno-format \
          -Wno-switch \
          -Wno-comment \
          -Wno-unused-result \
          -Wno-unused-but-set-variable

    QMAKE_CXXFLAGS_WARN_ON = $${QMAKE_CFLAGS_WARN_ON}

    QMAKE_CFLAGS_WARN_ON +=  \
          -Wno-implicit-function-declaration \
          -Wno-incompatible-pointer-types
    }

    macx {
    QMAKE_CFLAGS_WARN_ON += \
          -Wno-incompatible-pointer-types-discards-qualifiers \
          -Wno-undefined-bool-conversion \
          -Wno-invalid-source-encoding \
          -Wno-mismatched-new-delete \
          -Wno-for-loop-analysis \
          -Wno-int-conversion \
          -Wno-reorder
    }
}
