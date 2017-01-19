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
} else {
        message("~~~ LDRAWINI RELEASE build ~~~")
        DESTDIR = build/release
}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc

# To enable static builds,
# be sure to add CONFIG+=staticlib in Additional Arguments of qmake build steps
staticlib {
    message("~~~ LDRAWINI STATIC build ~~~")
    CONFIG += staticlib # this is needed if you create a static library
} else {
    # This one handles dllimport/dllexport directives.
    message("~~~ LDRAWINI SHARED build ~~~")
}

# Input
include(ldrawini.pri)
include(../LPub3DPlatformSpecific.pri)

unix:!symbian {
    headers.path=$$PREFIX/include/ldrawini
    headers.files=$$HEADERS
    target.path=$$PREFIX/lib/$${LIB_ARCH}
    INSTALLS += headers target	
}

win32 {
    headers.path=$$PREFIX/include/ldrawini
    headers.files=$$HEADERS
    target.path=$$PREFIX/lib
    INSTALLS += headers target
    # workaround for qdatetime.h macro bug
    DEFINES += NOMINMAX
}


