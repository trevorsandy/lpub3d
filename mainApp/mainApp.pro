TEMPLATE = app
QT += core
QT += gui
QT += opengl
QT += network
QT += xml

greaterThan(QT_MAJOR_VERSION, 4) {
    QT *= printsupport
    QT += concurrent
}

CONFIG += exceptions

include(../gitversion.pri)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TARGET +=
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../ldrawini ../lclib/common ../lclib/qt ../ldvlib
INCLUDEPATH += ../ldvlib/LDVQt ../ldvlib/TCFoundation ../ldvlib/TRE ../ldvlib/LDLoader ../ldvlib/LDExporter
win32-msvc* {
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib
}
# If quazip is alredy installed you can suppress building it again by
# adding CONFIG+=quazipnobuild to the qmake arguments
# Update the quazip header path if not installed at default location below
quazipnobuild {
    INCLUDEPATH += /usr/include/quazip
} else {
    INCLUDEPATH += ../quazip
}
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

HOST_VERSION = $$(PLATFORM_VER)
BUILD_TARGET = $$(TARGET_VENDOR)
BUILD_ARCH   = $$(TARGET_CPU)
!contains(QT_ARCH, unknown):  BUILD_ARCH = $$QT_ARCH
else: isEmpty(BUILD_ARCH):    BUILD_ARCH = UNKNOWN ARCH
contains(HOST_VERSION, 1320):contains(BUILD_TARGET, suse):contains(BUILD_ARCH, aarch64): DEFINES += OPENSUSE_1320_ARM
if (contains(QT_ARCH, x86_64)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)) {
    ARCH     = 64
    STG_ARCH = x86_64
    LIB_ARCH = 64
} else {
    ARCH     = 32
    STG_ARCH = x86
    LIB_ARCH =
}
if (contains(QT_ARCH, arm)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)): CHIPSET = ARM
else:                                                                               CHIPSET = X86
DEFINES     += VER_ARCH=\\\"$$ARCH\\\"

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

QMAKE_CXXFLAGS       += $(Q_CXXFLAGS)
QMAKE_LFLAGS         += $(Q_LDFLAGS)
QMAKE_CFLAGS         += $(Q_CFLAGS)

!win32-msvc* {
QMAKE_CFLAGS_WARN_ON += -Wno-unused-parameter -Wno-unknown-pragmas
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

win32 {

    DEFINES += QT_NODLL
    QMAKE_EXT_OBJ = .obj
    CONFIG += windows
    win32-msvc* {
        DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_WARNINGS=1
        DEFINES += _WINSOCKAPI_
        DEFINES += _TC_STATIC
        DEFINES += QUAZIP_STATIC
    }

    QMAKE_TARGET_COMPANY = "LPub3D Software"
    QMAKE_TARGET_DESCRIPTION = "LPub3D - An LDraw Building Instruction Editor."
    QMAKE_TARGET_COPYRIGHT = "Copyright (c) 2015-2018 Trevor SANDY"
    QMAKE_TARGET_PRODUCT = "LPub3D ($$join(ARCH,,,bit))"
    RC_LANG = "English (United Kingdom)"
    RC_ICONS = "lpub3d.ico"

} else {
    macx: \
    LIBS        += -framework CoreFoundation -framework CoreServices
}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

unix:!macx: TARGET = lpub3d
else:       TARGET = LPub3D
STG_TARGET   = $$TARGET
DIST_TARGET  = $$TARGET

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static {                                     # everything below takes effect with CONFIG ''= static
    CONFIG+= static
    LIBS += -static
    BUILD = Static
    DEFINES += STATIC
    DEFINES += QUAZIP_STATIC                 # this is so the compiler can detect quazip static
    macx: TARGET = $$join(TARGET,,,_static)  # this adds an _static in the end, so you can seperate static build from non static build
    win32: TARGET = $$join(TARGET,,,s)       # this adds an s in the end, so you can seperate static build from non static build
} else {
    BUILD = Shared
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Note on x11 platforms you can also pre-install install quazip ($ sudo apt-get install libquazip-dev)
# If quazip is already installed, set CONFIG+=quazipnobuild to use installed library

CONFIG(debug, debug|release) {
    DEFINES += QT_DEBUG_MODE
    BUILD_CONF = Debug
    ARCH_BLD = bit_debug
    macx {
        LDRAWINI_LIB = LDrawIni_debug
        QUAZIP_LIB = QuaZIP_debug
        LC_LIB = LC_debug
        TRE_LIB = TRE_debug
        LDLIB_LIB = LDLib_debug
        LDLOADER_LIB = LDLoader_debug
        LDEXPORTER_LIB = LDExporter_debug
        TCFOUNDATION_LIB = TCFoundation_debug
        LDVQT_LIB = LDVQt_debug
        PNG_LIB = PNG_debug
        JPEG_LIB = JPEG_debug
        GL2PS_LIB = GL2PS_debug
        TINYXML_LIB = TinyXml_debug
    }
    win32 {
        LDRAWINI_LIB = LDrawInid161
        QUAZIP_LIB = QuaZIPd07
        LC_LIB = LCd18
        TRE_LIB = TREd43
        LDLIB_LIB = LDLibd43
        LDLOADER_LIB = LDLoaderd43
        LDEXPORTER_LIB = LDExporterd43
        TCFOUNDATION_LIB = TCFoundationd43
        LDVQT_LIB = LDVQtd43
        PNG_LIB = PNGd16
        JPEG_LIB = JPEGd92
        GL2PS_LIB = GL2PSd13
        TINYXML_LIB = TinyXmld26
    }
    unix:!macx {
        LDRAWINI_LIB = ldrawinid
        QUAZIP_LIB = quazipd
        LC_LIB = lcd
        TRE_LIB = tred
        LDLIB_LIB = ldlibd
        LDLOADER_LIB = ldloaderd
        LDEXPORTER_LIB = ldexporterd
        TCFOUNDATION_LIB = tcfoundationd
        LDVQT_LIB = ldvqtd
        PNG_LIB = pngd
        JPEG_LIB = jpegd
        GL2PS_LIB = gl2psd
        TINYXML_LIB = tinyxmld
    }
    # executable target name
    macx: TARGET = $$join(TARGET,,,_debug)
    win32:TARGET = $$join(TARGET,,,d)
    unix:!macx: TARGET = $$join(TARGET,,,d$$VER_MAJOR$$VER_MINOR)
} else {
    BUILD_CONF = Release
    ARCH_BLD = bit_release
    macx {
        LDRAWINI_LIB = LDrawIni
        QUAZIP_LIB = QuaZIP
        LC_LIB = LC
        TRE_LIB = TRE
        LDLIB_LIB = LDLib
        LDLOADER_LIB = LDLoader
        LDEXPORTER_LIB = LDExporter
        TCFOUNDATION_LIB = TCFoundation
        LDVQT_LIB = LDVQt
        PNG_LIB = PNG
        JPEG_LIB = JPEG
        GL2PS_LIB = GL2PS
        TINYXML_LIB = TinyXml
    }
    win32 {
        LDRAWINI_LIB = LDrawIni161
        QUAZIP_LIB = QuaZIP07
        LC_LIB = LC18
        TRE_LIB = TRE43
        LDLIB_LIB = LDLib43
        LDLOADER_LIB = LDLoader43
        LDEXPORTER_LIB = LDExporter43
        TCFOUNDATION_LIB = TCFoundation43
        LDVQT_LIB = LDVQt43
        PNG_LIB = PNG16
        JPEG_LIB = JPEG92
        GL2PS_LIB = GL2PS13
        TINYXML_LIB = TinyXml26
    }
    unix:!macx {
        LDRAWINI_LIB = ldrawini
        QUAZIP_LIB = quazip
        LC_LIB = lc
        TRE_LIB = tre
        LDLIB_LIB = ldlib
        LDLOADER_LIB = ldloader
        LDEXPORTER_LIB = ldexporter
        TCFOUNDATION_LIB = tcfoundation
        LDVQT_LIB = ldvqt
        PNG_LIB = png
        JPEG_LIB = jpeg
        GL2PS_LIB = gl2ps
        TINYXML_LIB = tinyxml
    }
    # executable target
    !macx:!win32: TARGET = $$join(TARGET,,,$$VER_MAJOR$$VER_MINOR)
}
BUILD += $$BUILD_CONF

# build path component
DESTDIR = $$join(ARCH,,,$$ARCH_BLD)

#manpage
MAN_PAGE = $$join(TARGET,,,.1)

message("~~~ MAIN_APP $$join(ARCH,,,bit) $${BUILD} $${CHIPSET} ~~~")

#~~~libraries~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# needed to access ui header from LDVQt
INCLUDEPATH += $$OUT_PWD/../ldvlib/LDVQt/$$DESTDIR/.ui

LIBS += -L$$OUT_PWD/../lclib/$$DESTDIR -l$$LC_LIB

LIBS += \
    -L$$OUT_PWD/../ldvlib/LDVQt/$$DESTDIR \
    -L$$OUT_PWD/../ldvlib/LDLib/$$DESTDIR \
    -L$$OUT_PWD/../ldvlib/LDExporter/$$DESTDIR  \
    -L$$OUT_PWD/../ldvlib/LDLoader/$$DESTDIR \
    -L$$OUT_PWD/../ldvlib/TRE/$$DESTDIR \
    -L$$OUT_PWD/../ldvlib/TCFoundation/$$DESTDIR

LIBS += \
    -l$$LDVQT_LIB \
    -l$$LDLIB_LIB \
    -l$$LDEXPORTER_LIB \
    -l$$LDLOADER_LIB \
    -l$$TRE_LIB \
    -l$$TCFOUNDATION_LIB

macx{
exists(/usr/X11/lib){
message("~~~ X11 found ~~~")
LIBS += -L/usr/X11/lib
}
LIBS += -L/usr/local/lib
}

quazipnobuild: \
LIBS += -lquazip
else: \
LIBS += -L$$OUT_PWD/../quazip/$$DESTDIR -l$$QUAZIP_LIB
LIBS += -L$$OUT_PWD/../ldrawini/$$DESTDIR -l$$LDRAWINI_LIB

if (unix:exists(/usr/include/gl2ps.h)|exists(/usr/local/include/gl2ps.h)) {
    LIBS += -lgl2ps
} else {
    LIBS += -L$$OUT_PWD/../ldvlib/gl2ps/$$DESTDIR -l$$GL2PS_LIB
}
if (unix:exists(/usr/include/png.h)|exists(/usr/local/include/png.h)|BUILD_GL2PS) {
    if (contains(HOST, Ubuntu):contains(HOST, 14.04.5)|BUILD_GL2PS) {
        LIBS += -L$$OUT_PWD/../ldvlib/libpng/$$DESTDIR -l$$PNG_LIB
    } else {
        LIBS += -lpng
    }
} else {
    LIBS += -L$$OUT_PWD/../ldvlib/libpng/$$DESTDIR -l$$PNG_LIB
}
if (unix:exists(/usr/include/jpeglib.h)|exists(/usr/local/include/jpeglib.h)) {
    LIBS += -ljpeg
} else {
    !win32:message("~~~ ALERT: library jpeg not found, building it... ~~~")
    LIBS += -L$$OUT_PWD/../ldvlib/libjpeg/$$DESTDIR -l$$JPEG_LIB
}
if (unix:exists(/usr/include/tinyxml.h)|exists(/usr/local/include/tinyxml.h)) {
    LIBS += -ltinyxml
} else {
    LIBS += -L$$OUT_PWD/../ldvlib/tinyxml/$$DESTDIR -l$$TINYXML_LIB
}

win32:LIBS += -ladvapi32 -lshell32 -lopengl32 -lglu32 -lwininet -luser32 -lws2_32 -lgdi32
else: LIBS += -lX11 -lGL -lGLU
!win32-msvc* {
    LIBS += -lz
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

PRECOMPILED_DIR = $$DESTDIR/.pch
OBJECTS_DIR     = $$DESTDIR/.obj
MOC_DIR         = $$DESTDIR/.moc
RCC_DIR         = $$DESTDIR/.qrc
UI_DIR          = $$DESTDIR/.ui

#~~ Merge ldv ini files and move to extras dir ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

LDV_MESSAGES_INI = LDVMessages.ini

win32 {
    COPY_CMD     = COPY /y /a /b
    PLUS_CMD     = +
    REDIRECT_CMD =
} else {
    COPY_CMD     = cat
    PLUS_CMD     =
    REDIRECT_CMD = >
}

merge_ini_commands = \
$$COPY_CMD \
$$system_path($$_PRO_FILE_PWD_/../ldvlib/LDLib/LDViewMessages.ini) $$PLUS_CMD \
$$system_path($$_PRO_FILE_PWD_/../ldvlib/LDExporter/LDExportMessages.ini) $$REDIRECT_CMD \
$$system_path($$_PRO_FILE_PWD_/extras/$$LDV_MESSAGES_INI) && $$COPY_CMD \
$$system_path($$_PRO_FILE_PWD_/extras/$$LDV_MESSAGES_INI) $$REDIRECT_CMD \
$$system_path($$OUT_PWD/$$DESTDIR/$$LDV_MESSAGES_INI)

merge_ini.target   = $$LDV_MESSAGES_INI
merge_ini.commands = $$merge_ini_commands
merge_ini.depends  = $$_PRO_FILE_PWD_/../ldvlib/LDLib/LDViewMessages.ini \
                     $$_PRO_FILE_PWD_/../ldvlib/LDExporter/LDExportMessages.ini

QMAKE_EXTRA_TARGETS += merge_ini
PRE_TARGETDEPS      += $$LDV_MESSAGES_INI
QMAKE_CLEAN         += $$_PRO_FILE_PWD_/extras/$$LDV_MESSAGES_INI \
                       $$OUT_PWD/$$DESTDIR/$$LDV_MESSAGES_INI

#~~file distributions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Install 3rd party executables, documentation and resources.
# When building on macOS, it is necessary to add CONFIG+=dmg at
# Projects/Build Steps/Qmake/'Additional arguments' because,
# macOS build will also bundle all deliverables.

# If you wish to build and install on Linux or Windows, then
# set deb|rpm|pkg|exe respectively.
macx: build_package = $$(INSTALL_3RD_PARTY)
else: build_package = $$(LP3D_BUILD_PKG)
if(deb|rpm|pkg|dmg|exe|contains(build_package, yes)) {
    args = deb rpm pkg dmg exe
    for(arg, args) {
        contains(CONFIG, $$arg): opt = $$arg
    }
    isEmpty(opt): opt = $$build_package
    message("~~~ BUILD DISTRIBUTION PACKAGE: $$opt ~~~")

    THIRD_PARTY_DIST_DIR_PATH = $$(LP3D_DIST_DIR_PATH)
    !exists($$THIRD_PARTY_DIST_DIR_PATH) {
        unix:!macx: DIST_DIR=lpub3d_linux_3rdparty
        macx: DIST_DIR=lpub3d_macos_3rdparty
        win32: DIST_DIR=lpub3d_windows_3rdparty
        THIRD_PARTY_DIST_DIR_PATH = $$_PRO_FILE_PWD_/../../$$DIST_DIR
        message("~~~ INFO - THIRD_PARTY_DIST_DIR_PATH WAS NOT SPECIFIED, USING $$THIRD_PARTY_DIST_DIR_PATH ~~~")
    }
    message("~~~ 3RD PARTY DISTRIBUTION REPO $$THIRD_PARTY_DIST_DIR_PATH ~~~")

    if (unix|copy3rd) {
        CONFIG+=copy3rdexe
        CONFIG+=copy3rdexeconfig
        CONFIG+=copy3rdcontent
    }
    win32 {
        CONFIG+=stagewindistcontent
        CONFIG+=stage3rdexe
        CONFIG+=stage3rdexeconfig
        CONFIG+=stage3rdcontent
    }
}

VER_LDVIEW     = ldview-4.3
VER_LDGLITE    = ldglite-1.3
VER_POVRAY     = lpub3d_trace_cui-3.8
DEFINES       += VER_LDVIEW=\\\"$$VER_LDVIEW\\\"
DEFINES       += VER_LDGLITE=\\\"$$VER_LDGLITE\\\"
DEFINES       += VER_POVRAY=\\\"$$VER_POVRAY\\\"

win32:include(winfiledistro.pri)
macx:include(macosfiledistro.pri)
unix:!macx:include(linuxfiledistro.pri)

#~~ includes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

include(../qslog/QsLog.pri)
include(../qsimpleupdater/QSimpleUpdater.pri)
include(../LPub3DPlatformSpecific.pri)

#~~ inputs ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

HEADERS += \
    aboutdialog.h \
    application.h \
    annotations.h \
    archiveparts.h \
    backgrounddialog.h \
    backgrounditem.h \
    borderdialog.h \
    callout.h \
    calloutbackgrounditem.h \
    color.h \
    commands.h \
    commonmenus.h \
    csiitem.h \
    dependencies.h \
    dialogexportpages.h \
    dividerdialog.h \
    editwindow.h \
    excludedparts.h \
    globals.h \
    gradients.h \
    highlighter.h \
    hoverpoints.h \
    imagematt.h \
    ldrawcolourparts.h \
    ldrawfiles.h \
    ldsearchdirs.h \
    lpub.h \
    lpub_messages.h \
    lpub_preferences.h \
    messageboxresizable.h \
    meta.h \
    metagui.h \
    metaitem.h \
    metatypes.h \
    name.h \
    numberitem.h \
    pagebackgrounditem.h \
    pageattributetextitem.h \
    pageattributepixmapitem.h \
    pairdialog.h \
    pageorientationdialog.h \
    pagesizedialog.h \
    parmshighlighter.h \
    parmswindow.h \
    paths.h \
    placement.h \
    placementdialog.h \
    pli.h \
    pliannotationdialog.h \
    pliconstraindialog.h \
    plisortdialog.h \
    plisubstituteparts.h \
    pointer.h \
    pointeritem.h \
    preferencesdialog.h \
    range.h \
    range_element.h \
    ranges.h \
    ranges_element.h \
    ranges_item.h \
    render.h \
    reserve.h \
    resize.h \
    resolution.h \
    rotateiconitem.h \
    rx.h \
    scaledialog.h \
    step.h \
    textitem.h \
    threadworkers.h \
    updatecheck.h \
    where.h \
    sizeandorientationdialog.h \
    version.h

SOURCES += \
    aboutdialog.cpp \
    application.cpp \
    annotations.cpp \
    archiveparts.cpp \
    assemglobals.cpp \
    backgrounddialog.cpp \
    backgrounditem.cpp \
    borderdialog.cpp \
    callout.cpp \
    calloutbackgrounditem.cpp \
    calloutglobals.cpp \
    color.cpp \
    commands.cpp \
    commandline.cpp \
    commonmenus.cpp \
    csiitem.cpp \
    dependencies.cpp \
    dialogexportpages.cpp \
    dividerdialog.cpp \
    editwindow.cpp \
    excludedparts.cpp \
    fadestepglobals.cpp \
    formatpage.cpp \
    gradients.cpp \
    highlighter.cpp \
    highlightstepglobals.cpp \
    hoverpoints.cpp \
    imagematt.cpp \
    ldrawcolourparts.cpp \
    ldrawfiles.cpp \
    ldsearchdirs.cpp \
    lpub.cpp \
    lpub_messages.cpp \
    lpub_preferences.cpp \
    meta.cpp \
    metagui.cpp \
    metaitem.cpp \
    multistepglobals.cpp \
    numberitem.cpp \
    openclose.cpp \
    pagebackgrounditem.cpp \
    pageattributetextitem.cpp \
    pageattributepixmapitem.cpp \
    pageglobals.cpp \
    pageorientationdialog.cpp \
    pagesizedialog.cpp \
    pairdialog.cpp \
    parmshighlighter.cpp \
    parmswindow.cpp \
    paths.cpp \
    placement.cpp \
    placementdialog.cpp \
    pli.cpp \
    pliannotationdialog.cpp \
    pliconstraindialog.cpp \
    pliglobals.cpp \
    plisortdialog.cpp \
    plisubstituteparts.cpp \
    pointeritem.cpp \
    preferencesdialog.cpp \
    printfile.cpp \
    projectglobals.cpp \
    range.cpp \
    range_element.cpp \
    ranges.cpp \
    ranges_element.cpp \
    ranges_item.cpp \
    render.cpp \
    resize.cpp \
    resolution.cpp \
    rotateiconitem.cpp \
    rotate.cpp \
    rx.cpp \
    scaledialog.cpp \
    sizeandorientationdialog.cpp \
    step.cpp \
    textitem.cpp \
    threadworkers.cpp \
    traverse.cpp \
    updatecheck.cpp \
    undoredo.cpp

FORMS += \
    preferences.ui \
    aboutdialog.ui \
    dialogexportpages.ui

OTHER_FILES += \
    Info.plist \
    lpub3d.desktop \
    org.trevorsandy.lpub3d.appdata.xml \
    lpub3d.xml \
    lpub3d.sh \
    $$lower($$MAN_PAGE) \
    lpub3d.1 \
    ../README.md \
    ../.gitignore \
    ../.travis.yml \
    ../appveyor.yml

include(otherfiles.pri)

RESOURCES += \
    ../lclib/resources/lclib.qrc \
    lpub3d.qrc

DISTFILES += \
    ldraw_document.icns

# set config to enable initial update check
# CONFIG+=update_check
update_check: DEFINES += DISABLE_UPDATE_CHECK

# Suppress warnings
!win32-msvc* {
QMAKE_CFLAGS_WARN_ON += \
    -Wno-deprecated-declarations \
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

#message($$CONFIG)
