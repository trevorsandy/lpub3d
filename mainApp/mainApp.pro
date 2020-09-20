TEMPLATE = app
QT     += core
QT     += gui
QT     += opengl
QT     += network
QT     += xml
CONFIG += warn_on

greaterThan(QT_MAJOR_VERSION, 4) {
    QT *= printsupport
    QT += concurrent
}

win32:macx: \
GAMEPAD {
    equals(QT_MAJOR_VERSION, 5) {
        qtHaveModule(gamepad) {
            QT += gamepad
            DEFINES += LC_ENABLE_GAMEPAD
        }
    }
}

CONFIG += exceptions

include(../gitversion.pri)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TARGET +=
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../ldrawini ../lclib/common ../lclib/qt ../qsimpleupdater/include ../qsimpleupdater/src
INCLUDEPATH += ../ldvlib ../ldvlib/LDVQt/include ../ldvlib/WPngImage
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

LDVMESSAGESINI = ldvMessages.ini
HOST_VERSION   = $$(PLATFORM_VER)
BUILD_TARGET   = $$(TARGET_VENDOR)
BUILD_ARCH     = $$(TARGET_CPU)
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

!freebsd: \
DEFINES += EXPORT_3DS

QMAKE_CXXFLAGS       += $(Q_CXXFLAGS)
QMAKE_LFLAGS         += $(Q_LDFLAGS)
QMAKE_CFLAGS         += $(Q_CFLAGS)

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

        QMAKE_CFLAGS_WARN_ON -= -W3
        QMAKE_ADDL_MSVC_FLAGS = -GS -Gd -fp:precise -Zc:forScope
        CONFIG(debug, debug|release) {
            QMAKE_ADDL_MSVC_DEBUG_FLAGS = -RTC1 $$QMAKE_ADDL_MSVC_FLAGS
            QMAKE_CFLAGS_WARN_ON += -W4 -WX- -wd"4005" -wd"4456" -wd"4458" -wd"4459" -wd"4127" -wd"4701" -wd"4714" -wd"4305" -wd"4099"
            QMAKE_CFLAGS_DEBUG   += $$QMAKE_ADDL_MSVC_DEBUG_FLAGS
            QMAKE_CXXFLAGS_DEBUG += $$QMAKE_ADDL_MSVC_DEBUG_FLAGS
        }
        CONFIG(release, debug|release) {
            QMAKE_LFLAGS_WINDOWS  += /STACK:4194304
            QMAKE_ADDL_MSVC_RELEASE_FLAGS = $$QMAKE_ADDL_MSVC_FLAGS -GF -Gy
            QMAKE_CFLAGS_OPTIMIZE += -Ob1 -Oi -Ot
            QMAKE_CFLAGS_WARN_ON  += -W1 -WX- -wd"4005" -wd"4456" -wd"4458" -wd"4805" -wd"4838" -wd"4700" -wd"4098"
            QMAKE_CFLAGS_RELEASE  += $$QMAKE_ADDL_MSVC_RELEASE_FLAGS
            QMAKE_CXXFLAGS_RELEASE += $$QMAKE_ADDL_MSVC_RELEASE_FLAGS
        }
        QMAKE_CXXFLAGS_WARN_ON = $$QMAKE_CFLAGS_WARN_ON
    } else {
        QMAKE_CFLAGS_WARN_ON += -Wno-unused-parameter -Wno-unknown-pragmas
    }

    QMAKE_TARGET_COMPANY = "LPub3D Software"
    QMAKE_TARGET_DESCRIPTION = "LPub3D - An LDraw Building Instruction Editor."
    QMAKE_TARGET_COPYRIGHT = "Copyright (c) 2015-2019 Trevor SANDY"
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

    win32 {
        LDRAWINI_LIB = LDrawInid161
        QUAZIP_LIB = QuaZIPd07
        LC_LIB = LCd18
        LDVQT_LIB = LDVQtd43
        WPNGIMAGE_LIB = WPngImaged14
    }

    macx {

        LDRAWINI_LIB = LDrawIni_debug
        QUAZIP_LIB = QuaZIP_debug
        LC_LIB = LC_debug
        LDVQT_LIB = LDVQt_debug
        WPNGIMAGE_LIB = WPngImage_debug
    }

    unix:!macx {
        LDRAWINI_LIB = ldrawinid
        QUAZIP_LIB = quazipd
        LC_LIB = lcd
        LDVQT_LIB = ldvqtd
        WPNGIMAGE_LIB = wpngimaged
    }

    # executable target name
    macx: TARGET = $$join(TARGET,,,_debug)
    win32:TARGET = $$join(TARGET,,,d)
    unix:!macx: TARGET = $$join(TARGET,,,d$$VER_MAJOR$$VER_MINOR)

    DO_COPY_LDVLIBS = #True

    # enable copy ldvMessages to OUT_PWD/mainApp/extras
    DEVL_LDV_MESSAGES_INI = True

} else {

    BUILD_CONF = Release
    ARCH_BLD = bit_release

    win32 {
        LDRAWINI_LIB = LDrawIni161
        QUAZIP_LIB = QuaZIP07
        LC_LIB = LC18
        LDVQT_LIB = LDVQt43
        WPNGIMAGE_LIB = WPngImage14
    }

    macx {
        LDRAWINI_LIB = LDrawIni
        QUAZIP_LIB = QuaZIP
        LC_LIB = LC
        LDVQT_LIB = LDVQt
        WPNGIMAGE_LIB = WPngImage
    }

    unix:!macx {

        LDRAWINI_LIB = ldrawini
        QUAZIP_LIB = quazip
        LC_LIB = lc
        LDVQT_LIB = ldvqt
        WPNGIMAGE_LIB = wpngimage
    }

    # executable target
    !macx:!win32: TARGET = $$join(TARGET,,,$$VER_MAJOR$$VER_MINOR)

    DO_COPY_LDVLIBS = True
}
BUILD += $$BUILD_CONF

# build path component foo
DESTDIR = $$join(ARCH,,,$$ARCH_BLD)

#manpage
MAN_PAGE = $$join(TARGET,,,.1)

message("~~~ $${TARGET} $$join(ARCH,,,bit) $${BUILD} $${CHIPSET} ~~~")

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

PRECOMPILED_DIR = $$DESTDIR/.pch
OBJECTS_DIR     = $$DESTDIR/.obj
MOC_DIR         = $$DESTDIR/.moc
RCC_DIR         = $$DESTDIR/.qrc
UI_DIR          = $$DESTDIR/.ui

#~~file distributions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Install 3rd party executables, documentation and resources.
# When building on macOS, it is necessary to add CONFIG+=dmg at
# Projects/Build Steps/Qmake/'Additional arguments' because,
# macOS build will also bundle all deliverables.

THIRD_PARTY_DIST_DIR_PATH = $$(LP3D_DIST_DIR_PATH)
isEmpty(THIRD_PARTY_DIST_DIR_PATH):THIRD_PARTY_DIST_DIR_PATH = NotDefined
!exists($$THIRD_PARTY_DIST_DIR_PATH) {
    unix:!macx: DIST_DIR      = lpub3d_linux_3rdparty
    else:macx: DIST_DIR       = lpub3d_macos_3rdparty
    else:win32: DIST_DIR      = lpub3d_windows_3rdparty
    THIRD_PARTY_DIST_DIR_PATH = $$system_path( $$absolute_path( $$_PRO_FILE_PWD_/../../$$DIST_DIR ) )
    exists($$THIRD_PARTY_DIST_DIR_PATH): \
    message("~~~ INFO - THIRD_PARTY_DIST_DIR_PATH WAS NOT SPECIFIED, USING $$THIRD_PARTY_DIST_DIR_PATH ~~~")
    else: \
    message("~~~ ERROR - THIRD_PARTY_DIST_DIR_PATH WAS NOT SPECIFIED! ~~~")
}
message("~~~ 3RD PARTY DISTRIBUTION REPO $$THIRD_PARTY_DIST_DIR_PATH ~~~")

# To build and install, set CONFIG+=deb|rpm|pkg|exe respectively.
macx: build_package = $$(INSTALL_3RD_PARTY)       # INSTALL_3RD_PARTY no longer used
else: build_package = $$(LP3D_BUILD_PKG)
if(deb|rpm|pkg|dmg|exe|contains(build_package, yes)) {
    args = deb rpm pkg dmg exe
    for(arg, args) {
        contains(CONFIG, $$arg): opt = $$arg
    }
    isEmpty(opt): opt = $$build_package
    message("~~~ BUILD DISTRIBUTION PACKAGE: $$opt ~~~")

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

#~~~libraries~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# needed to access ui header from LDVQt
INCLUDEPATH += $$OUT_PWD/../ldvlib/LDVQt/$$DESTDIR/.ui

LIBS += -L$$OUT_PWD/../lclib/$$DESTDIR -l$$LC_LIB

LIBS += -L$$OUT_PWD/../ldvlib/LDVQt/$$DESTDIR -l$$LDVQT_LIB

LIBS += -L$$OUT_PWD/../ldvlib/WPngImage/$$DESTDIR -l$$WPNGIMAGE_LIB

# Setup LDVQt Libraries
LOAD_LDVLIBS = True
include(../ldvlib/LDVQt/LDViewLibs.pri)

quazipnobuild: \
LIBS += -lquazip
else: \
LIBS += -L$$OUT_PWD/../quazip/$$DESTDIR -l$$QUAZIP_LIB
LIBS += -L$$OUT_PWD/../ldrawini/$$DESTDIR -l$$LDRAWINI_LIB

win32 {
    LIBS += -ladvapi32 -lshell32 -lopengl32 -lglu32 -lwininet -luser32 -lws2_32 -lgdi32
} else:!macx {
    LIBS += -lGL -lGLU
}
!win32-msvc* {
    LIBS += -lz
}

#~~ inputs ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

HEADERS += \
    aboutdialog.h \
    annotations.h \
    application.h \
    archiveparts.h \
    backgrounddialog.h \
    backgrounditem.h \
    borderdialog.h \
    callout.h \
    calloutbackgrounditem.h \
    calloutpointeritem.h \
    color.h \
    cameradistfactordialog.h \
    commands.h \
    commonmenus.h \
    csiitem.h \
    dependencies.h \
    dialogexportpages.h \
    dividerdialog.h \
    dividerpointeritem.h \
    editwindow.h \
    excludedparts.h \
    globals.h \
    gradients.h \
    highlighter.h \
	historylineedit.h \
    hoverpoints.h \
    ldrawcolourparts.h \
    ldrawfiles.h \
    ldsearchdirs.h \
    lgraphicsscene.h \
    lgraphicsview.h \
    lpub.h \
    lpubalert.h \
    lpub_preferences.h \
    messageboxresizable.h \
    meta.h \
    metagui.h \
    metaitem.h \
    metatypes.h \
    name.h \
    numberitem.h \
    pageattributepixmapitem.h \
    pageattributetextitem.h \
    pagebackgrounditem.h \
    pageorientationdialog.h \
    pagepointer.h \
    pagepointeritem.h \
    pagepointerbackgrounditem.h \
    pagesizedialog.h \
    pagesizes.h \
    pairdialog.h \
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
    pointerplacementdialog.h \
    povrayrenderdialog.h \  
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
    rotateiconsizedialog.h \
    rx.h \
    scaledialog.h \
    sizeandorientationdialog.h \
    step.h \
    submodelcolordialog.h \
    textitem.h \
    threadworkers.h \
    updatecheck.h \
    version.h \
    where.h \
    submodelitem.h \
    rotstepdialog.h \
    borderedlineitem.h \
    pointerattribdialog.h \
    csiannotation.h \
    ldrawfilesload.h \
    substitutepartdialog.h \
    ldrawcolordialog.h \
    ldrawcolordialog.cpp \
    texteditdialog.h \
    options.h

SOURCES += \
    aboutdialog.cpp \
    annotations.cpp \
    application.cpp \
    archiveparts.cpp \
    assemglobals.cpp \
    backgrounddialog.cpp \
    backgrounditem.cpp \
    borderdialog.cpp \
    callout.cpp \
    calloutbackgrounditem.cpp \
    calloutglobals.cpp \
    calloutpointeritem.cpp \
    color.cpp \
    cameradistfactordialog.cpp \
    commandline.cpp \
    commands.cpp \
    commonmenus.cpp \
    csiitem.cpp \
    dependencies.cpp \
    dialogexportpages.cpp \
    dividerdialog.cpp \
    dividerpointeritem.cpp \
    editwindow.cpp \
    excludedparts.cpp \
    fadestepglobals.cpp \
    formatpage.cpp \
    gradients.cpp \
    highlighter.cpp \
    highlightstepglobals.cpp \
	historylineedit.cpp \
    hoverpoints.cpp \
    ldrawcolourparts.cpp \
    ldrawfiles.cpp \
    ldsearchdirs.cpp \
    lgraphicsscene.cpp \
    lgraphicsview.cpp \
    lpub.cpp \
    lpubalert.cpp \
    lpub_preferences.cpp \
    messageboxresizable.cpp \
    meta.cpp \
    metagui.cpp \
    metaitem.cpp \
    multistepglobals.cpp \
    numberitem.cpp \
    openclose.cpp \
    pageattributepixmapitem.cpp \
    pageattributetextitem.cpp \
    pagebackgrounditem.cpp \
    pageglobals.cpp \
    pageorientationdialog.cpp \
    pagepointer.cpp \
    pagepointeritem.cpp \
    pagepointerbackgrounditem.cpp \
    pagesizedialog.cpp \
    pagesizes.cpp \
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
    pointerplacementdialog.cpp \
    povrayrenderdialog.cpp \    
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
    rotate.cpp \
    rotateiconitem.cpp \
    rotateiconsizedialog.cpp \
    rx.cpp \
    scaledialog.cpp \
    sizeandorientationdialog.cpp \
    step.cpp \
    submodelcolordialog.cpp \
    textitem.cpp \
    threadworkers.cpp \
    traverse.cpp \
    undoredo.cpp \
    updatecheck.cpp \
    submodelitem.cpp \
    submodelglobals.cpp \
    rotstepdialog.cpp \
    borderedlineitem.cpp \
    pointerattribdialog.cpp \
    csiannotation.cpp \
    ldrawfilesload.cpp \
    substitutepartdialog.cpp \
    ldrawcolordialog.cpp \
    ldrawpartdialog.cpp \
    nativeviewer.cpp \
    texteditdialog.cpp

FORMS += \
    aboutdialog.ui \
    dialogexportpages.ui \
    povrayrenderdialog.ui \
    ldrawfilesloaddialog.ui \
    ldrawcolordialog.ui \
    ldrawpartdialog.ui \
    substitutepartdialog.ui \
    preferences.ui \
    texteditdialog.ui

OTHER_FILES += \
    $$lower($$MAN_PAGE) \
    ../.gitignore \
    ../.travis.yml \
    ../appveyor.yml \
    ../README.md \
    Info.plist \
    lpub3d.1 \
    lpub3d.desktop \
    lpub3d.sh \
    lpub3d.xml \
    org.trevorsandy.lpub3d.appdata.xml

include(otherfiles.pri)

RESOURCES += \
    ../lclib/lclib.qrc \
    ../ldvlib/LDVQt/resources.qrc \
    qdarkstyle/style.qrc \
    resources/lsynth/lsynth.qrc \
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
    -Wno-deprecated-copy \
    -Wno-unused-parameter \
    -Wno-sign-compare
}
macx {

QMAKE_CFLAGS_WARN_ON += \
    -Wno-overloaded-virtual \
    -Wno-self-assign \
    -Wno-sometimes-uninitialized \
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
   -Wno-cpp \
   -Wno-implicit-fallthrough \
   -Wno-maybe-uninitialized \
   -Wno-strict-aliasing \
   -Wno-unused-result
}

} else {

QMAKE_CFLAGS_WARN_ON += \
    -Wno-strict-aliasing
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}
}

#message($$CONFIG)
