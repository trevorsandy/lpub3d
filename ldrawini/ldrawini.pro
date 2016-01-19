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

# This one handles dllimport/dllexport directive

DEFINES += LDRAWINI_BUILD

# You'll need to define this one manually if using a build system other
# than qmake or using LDrawIni sources directly in your project.

CONFIG(staticlib): DEFINES += LDRAWINI_STATIC
CONFIG(debug, debug|release) {
        DESTDIR = build/debug
} else {
        DESTDIR = build/release
}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc

# Input
include(ldrawini.pri)

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


