TEMPLATE = lib
CONFIG += qt warn_on
QT -= gui

# The ABI version.
!win32:VERSION = 1.0.0

win32 {

    QMAKE_EXT_OBJ = .obj
    CONFIG += windows
    CONFIG += debug_and_release

}

macx {

    QMAKE_CXXFLAGS += -F/System/Library/Frameworks
    LIBS += -framework CoreFoundation

}

CONFIG(debug, debug|release) {
    message("~~~ LDRAWINI DEBUG build ~~~")
    DESTDIR = build/debug
    mac: TARGET = $$join(TARGET,,,_debug)
    win32: TARGET = $$join(TARGET,,,d)
} else {
    message("~~~ LDRAWINI RELEASE build ~~~")
    DESTDIR = build/release
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

unix:!symbian {
    isEmpty(PREFIX):PREFIX = /usr
    headers.path=$$PREFIX/include
    headers.files=$$HEADERS
    deb {
        contains(QT_ARCH, x86_64){
            LIB_ARCH = x86_64-linux-gnu
        } else {
            LIB_ARCH = i386-linux-gnu
        }
        target.path=$$PREFIX/lib/$$LIB_ARCH
        message("~~~ LDRAWINI DEB LIB ~~~")
    }
    rpm {
        contains(QT_ARCH, x86_64){
            target.path=$$PREFIX/lib64
        } else {
            target.path=$$PREFIX/lib32
        }
        message("~~~ LDRAWINI RPM LIB ~~~")
    } else {
        target.path=$$PREFIX/lib
        message("~~~ LDRAWINI LIB ~~~")
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
