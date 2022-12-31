TEMPLATE  = lib
CONFIG   += qt warn_on
QT       -= gui
CONFIG   -= console

# The ABI version.
!win32:VERSION = 2.1.0

win32 {
    QMAKE_EXT_OBJ = .obj
    CONFIG        += windows
    CONFIG        += debug_and_release	
}

TARGET  = qslog

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
    
    !macx {
        other_files.files = LICENSE.txt QsLogChanges.txt README.md
        other_files.path = $$PREFIX/local/share/$(QMAKE_TARGET)
        contains(DISTRO, .*ARCH): other_files.path = $$PREFIX/share/$(QMAKE_TARGET)

        contains(QT_ARCH, x86_64) {
            target.path = $$PREFIX/lib64
            contains(DISTRO, .*ARCH): target.path = $$PREFIX/lib
        } else {
            target.path = $$PREFIX/lib
        }
        INSTALLS += other_files headers target
    } else {
    	INSTALLS += headers target
    }
}

win32 {

	DEFINES += QSLOG_IS_SHARED_LIBRARY
	
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

