TEMPLATE = lib
CONFIG += qt warn_on
QT -= gui

# The ABI version.
# Version format is year.month.day.patch
win32: VERSION = 16.1.8.0  # major.minor.patch.build
else: VERSION = 16.1.8     # major.minor.patch

contains(QT_ARCH, x86_64) {
    ARCH = 64
} else {
    ARCH = 32
}

win32 {

    QMAKE_EXT_OBJ = .obj
    CONFIG += windows
    CONFIG += debug_and_release

    QMAKE_TARGET_COMPANY = "Lars C. Hassing"
    QMAKE_TARGET_DESCRIPTION = "LDrawDir and SearchDirs API"
    QMAKE_TARGET_COPYRIGHT = "Copyright (c) 2004-2008  Lars C. Hassing"
    QMAKE_TARGET_PRODUCT = "LDrawIni ($$ARCH-bit)"
}

macx {
    QMAKE_CXXFLAGS += -F/System/Library/Frameworks
    LIBS += -framework CoreFoundation
}

CONFIG += skip_target_version_ext
unix:!macx: TARGET = ldrawini
else: TARGET = LDrawIni

CONFIG(debug, debug|release) {
    message("~~~ LDRAWINI DEBUG build ~~~")
    DESTDIR = debug
    macx: TARGET = $$join(TARGET,,,_debug)
    win32: TARGET = $$join(TARGET,,,d161)
    unix:!macx: TARGET = $$join(TARGET,,,d)
} else {
    message("~~~ LDRAWINI RELEASE build ~~~")
    DESTDIR = release
    win32: TARGET = $$join(TARGET,,,161)
}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
QMAKE_EXT_CPP = .c

# Indicate build type,
# be sure to add CONFIG+=staticlib in Additional Arguments of qmake build steps
staticlib {
    message("~~~ LDRAWINI STATIC build ~~~")
} else {
    message("~~~ LDRAWINI SHARED build ~~~")
}

# Input
include(ldrawini.pri)
include(../LPub3DPlatformSpecific.pri)

unix {
    isEmpty(PREFIX):PREFIX = /usr
    headers.path=$$PREFIX/include
    headers.files=$$HEADERS
    deb {
        target.path=$$PREFIX/lib/$$QT_ARCH-linux-gnu
        message("~~~ LDRAWINI DEB $$ARCH-bit LIB ~~~")
    }
    rpm {
        target.path=$$PREFIX/lib$$ARCH
        message("~~~ LDRAWINI RPM $$ARCH-bit LIB ~~~")
    }
    !deb:!rpm {
        target.path=$$PREFIX/lib
        message("~~~ LDRAWINI $$ARCH-bit LIB ~~~")
    }
    INSTALLS += target
    libheaders: INSTALLS += headers
    libheaders: message("~~~ INSTALL LDRAWINI LIB HEADERS ~~~")

}

win32 {
    headers.path=$$PREFIX/include
    headers.files=$$HEADERS
    target.path=$$PREFIX/lib
    INSTALLS += headers target
    # workaround for qdatetime.h macro bug
    DEFINES += NOMINMAX
}
