TEMPLATE = lib
CONFIG += qt warn_on
QT -= gui

# The ABI version.
!win32:VERSION = 2.0.0

win32 {

    QMAKE_EXT_OBJ = .obj
    CONFIG += windows
    CONFIG += debug_and_release	
}

TARGET = qslog

CONFIG += shared
CONFIG(debug, debug|release) {
        DESTDIR = build/debug
} else {
        DESTDIR = build/release
}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc

# Input
include(QsLog.pri)

unix:!symbian {

    headers.path=$$PREFIX/include/$(QMAKE_TARGET)
    headers.files=$$HEADERS
    target.path=$$PREFIX/lib/$${LIB_ARCH}
    INSTALLS += headers target
	
}

win32 {

    headers.path=$$PREFIX/include/qslog
    headers.files=$$HEADERS
    target.path=$$PREFIX/lib
    INSTALLS += headers target

}


symbian {

    CONFIG += staticlib
    CONFIG += debug_and_release

    LIBS += -qslog

    #Export headers to SDK Epoc32/include directory
    exportheaders.sources = $$HEADERS
    exportheaders.path = $(QMAKE_TARGET)
    for(header, exportheaders.sources) {
        BLD_INF_RULES.prj_exports += "$$header $$exportheaders.path/$$basename(header)"
    }
}

