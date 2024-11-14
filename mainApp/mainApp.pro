TEMPLATE = app
QT     += core
QT     += gui
QT     += opengl
QT     += network
QT     += xml
QT     += concurrent
QT     *= printsupport
CONFIG += warn_on

lessThan(QT_MAJOR_VERSION, 5) {
    error("LPub3D requires Qt5.4 or later.")
}

equals(  QT_MAJOR_VERSION, 5): \
lessThan(QT_MINOR_VERSION, 4) {
    error("LPub3D requires Qt5.4 or later.")
}

greaterThan(QT_MAJOR_VERSION, 5) {
    QT += core5compat openglwidgets
    DEFINES += QOPENGLWIDGET
}

win32:macx: \
GAMEPAD {
    qtHaveModule(gamepad) {
        QT += gamepad
        DEFINES += LC_ENABLE_GAMEPAD
    }
}

CONFIG += exceptions

CONFIG(debug, debug|release) { LPUB3D = LPub3Dd } else { LPUB3D = LPub3D }

include(../gitversion.pri)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

unix:!macx: TARGET = lpub3d
else:       TARGET = LPub3D
STG_TARGET         = $$TARGET
DIST_TARGET        = $$TARGET

VER_LDVIEW  = ldview-4.5
VER_LDGLITE = ldglite-1.3
VER_POVRAY  = lpub3d_trace_cui-3.8
DEFINES    += VER_LDVIEW=\\\"$$VER_LDVIEW\\\"
DEFINES    += VER_LDGLITE=\\\"$$VER_LDGLITE\\\"
DEFINES    += VER_POVRAY=\\\"$$VER_POVRAY\\\"

#~~~~ third party distro folder ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# - Set enviroinment variable LP3D_DIST_DIR_PATH as needed.
# - 3rd party libraries, executables, documentation and resources.
# - When building on macOS, it is necessary to add CONFIG+=dmg at
#   Projects/Build Steps/Qmake/'Additional arguments' because,
#   macOS build will also bundle all deliverables.
#   Local variable - LP3D_3RD_DIST_DIR
exists($$PWD/../builds/3rdparty) {
    THIRD_PARTY_DIST_DIR_PATH=$$system_path( $$absolute_path( $$PWD/../builds/3rdparty ) )
    3RD_DIR_SOURCE = LOCAL_3RD_PARTY_DIR
} else:!isEmpty(LP3D_3RD_DIST_DIR) {
    THIRD_PARTY_DIST_DIR_PATH = $$LP3D_3RD_DIST_DIR
    3RD_DIR_SOURCE = LP3D_3RD_DIST_DIR
} else {
#   Environment variable LP3D_DIST_DIR_PATH
    THIRD_PARTY_DIST_DIR_PATH = $$(LP3D_DIST_DIR_PATH)
    !isEmpty(THIRD_PARTY_DIST_DIR_PATH) {
        3RD_DIR_SOURCE = LP3D_DIST_DIR_PATH
    }
}
!exists($$THIRD_PARTY_DIST_DIR_PATH) {
    unix:!macx: DIST_DIR      = lpub3d_linux_3rdparty
    else:macx: DIST_DIR       = lpub3d_macos_3rdparty
    else:win32: DIST_DIR      = lpub3d_windows_3rdparty
    THIRD_PARTY_DIST_DIR_PATH = $$system_path( $$absolute_path( $$_PRO_FILE_PWD_/../../$$DIST_DIR ) )
    exists($$THIRD_PARTY_DIST_DIR_PATH) {
        3RD_DIR_SOURCE_UNSPECIFIED = "INFO - THIRD_PARTY_DIST_DIR_PATH WAS NOT SPECIFIED, USING $$THIRD_PARTY_DIST_DIR_PATH"
    } else {
        3RD_DIR_SOURCE_UNSPECIFIED = "ERROR - THIRD_PARTY_DIST_DIR_PATH WAS NOT SPECIFIED!"
        THIRD_PARTY_DIST_DIR_PATH="undefined"
    }
    3RD_DIR_SOURCE = DEFAULT_3RD_PARTY_DIR
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

DEPENDPATH  += .
INCLUDEPATH += .
INCLUDEPATH += ../lclib/common ../lclib/qt ../ldvlib ../waitingspinner ../ldrawini jsonconfig

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SYSTEM_PREFIX = /usr/local
macx:contains(QT_ARCH,arm64): SYSTEM_PREFIX = /opt/homebrew

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# If quazip is alredy installed you can suppress building it again by
# adding CONFIG+=quazipnobuild to the qmake arguments
# Update the quazip header path if not installed at default location specified
quazipnobuild {
    INCLUDEPATH += $$SYSTEM_PREFIX/include/quazip
} else {
    INCLUDEPATH += ../quazip
}

#~~ LDVQt dependencies ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

VER_USE_LDVIEW_DEV = False
CONFIG(debug, debug|release) {
    # These lines requires a git extract of ldview at the same location as the lpub3d git extract
    # and defines the ldview git extract folder name, you can set as you like
    unix: VER_LDVIEW_DEV = ldview
    else:win32: VER_LDVIEW_DEV = ldview_vs_build
    # This line defines the path of the ldview git extract relative to this project file
    VER_LDVIEW_DEV_REPOSITORY = $$system_path( $$absolute_path( $$PWD/../../$${VER_LDVIEW_DEV} ) )
    exists($$VER_LDVIEW_DEV_REPOSITORY) {
        VER_USE_LDVIEW_DEV = True
        message("~~~ $${LPUB3D} LINK LDVQt USING LDVIEW DEVELOPMENT REPOSITORY ~~~ ")
    } else {
        message("~~~ $${LPUB3D} WARNING - COULD NOT LOAD LDVIEW DEV FROM: $$VER_LDVIEW_DEV_REPOSITORY ~~~ ")
    }
}
# Load LDView libraries for LDVQt
LOAD_LDV_LIBS = True
LDVMESSAGESINI = ldvMessages.ini

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

HOST_VERSION   = $$(PLATFORM_VER)
BUILD_TARGET   = $$(TARGET_VENDOR)
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

message("~~~ $${LPUB3D} LPUB3D $$upper($$QT_ARCH) build - $${BUILD_TARGET}-$${HOST_VERSION}-$${BUILD_ARCH} ~~~")

# for aarch64, QT_ARCH = arm64, for arm7l, QT_ARCH = arm
if (contains(QT_ARCH, x86_64)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)) {
    ARCH     = 64
    STG_ARCH = x86_64
    LIB_ARCH = 64
} else {
    ARCH     = 32
    STG_ARCH = x86
    LIB_ARCH =
}

# define chipset
if (contains(QT_ARCH, arm)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)): \
CHIPSET = ARM
else: \
CHIPSET = AMD

DEFINES += VER_ARCH=\\\"$$ARCH\\\"
DEFINES += VER_CHIPSET=\\\"$$CHIPSET\\\"

# special case for OpenSuse 1320
contains(HOST_VERSION, 1320):contains(BUILD_TARGET, suse):contains(BUILD_ARCH, aarch64): \
DEFINES += OPENSUSE_1320_ARM

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# USE CPP 11
contains(USE_CPP11,NO) {
    message("NO CPP11")
} else {
    DEFINES += USE_CPP11
}

message("~~~ $${LPUB3D} BUILDING WITH QT VERSION: $$QT_VERSION ~~~")

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

!freebsd: \
DEFINES += EXPORT_3DS

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

win32 {

    DEFINES += QT_NODLL
    QMAKE_EXT_OBJ = .obj
    CONFIG += windows incremental force_debug_info
    win32-msvc* {
        DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_WARNINGS=1
        DEFINES += _WINSOCKAPI_
        DEFINES += _TC_STATIC
        DEFINES += QUAZIP_STATIC

        QMAKE_LFLAGS += -NODEFAULTLIB:LIBCMT
        QMAKE_LFLAGS_WINDOWS += /STACK:4194304 /IGNORE:4099
        QMAKE_CFLAGS_WARN_ON -= -W3
        QMAKE_ADDL_MSVC_FLAGS = -GS -Gd -fp:precise -Zc:forScope
        CONFIG(debug, debug|release) {
            QMAKE_ADDL_MSVC_DEBUG_FLAGS = -RTC1 $$QMAKE_ADDL_MSVC_FLAGS
            QMAKE_CFLAGS_WARN_ON += -W4 -WX- -wd"4005" -wd"4456" -wd"4458" -wd"4459" -wd"4127" -wd"4701" -wd"4714" -wd"4305" -wd"4099"
            QMAKE_CFLAGS_DEBUG   += $$QMAKE_ADDL_MSVC_DEBUG_FLAGS
            QMAKE_CXXFLAGS_DEBUG += $$QMAKE_ADDL_MSVC_DEBUG_FLAGS
        }
        CONFIG(release, debug|release) {
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
    QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2015 - 2024 Trevor SANDY"
    QMAKE_TARGET_PRODUCT = "LPub3D ($$join(ARCH,,,bit))"
    RC_LANG = "English (United Kingdom)"
    RC_ICONS = "lpub3d.ico"

} else {
    macx: \
    LIBS += -framework CoreFoundation -framework CoreServices
}

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
# NOTE: Reminder to update MacOS library links on QuaZip and LDrawIni major version change
#       - Files inpacted: build_checks.sh, CreateDmg.sh and macosfiledistro.pri

CONFIG(debug, debug|release) {
    DEFINES += QT_DEBUG_MODE
    BUILD_CONF = Debug
    ARCH_BLD = bit_debug

    win32 {
        LDRAWINI_LIB = LDrawInid161
        QUAZIP_LIB = QuaZIPd11
        LC_LIB = LCd233
        LDVQT_LIB = LDVQtd45
        WPNGIMAGE_LIB = WPngImaged14
        WAITING_SPINNER_LIB = WaitingSpinnerd10
    }

    macx {

        LDRAWINI_LIB = LDrawIni_debug
        QUAZIP_LIB = QuaZIP_debug
        LC_LIB = LC_debug
        LDVQT_LIB = LDVQt_debug
        WPNGIMAGE_LIB = WPngImage_debug
        WAITING_SPINNER_LIB = WaitingSpinner_debug
    }

    unix:!macx {
        LDRAWINI_LIB = ldrawinid
        QUAZIP_LIB = quazipd
        LC_LIB = lcd
        LDVQT_LIB = ldvqtd
        WPNGIMAGE_LIB = wpngimaged
        WAITING_SPINNER_LIB = waitingspinnerd

        # For Linux builds, simplify debug ops by using runtime content in build folders
        DEFINES += DEBUG_MODE_USE_BUILD_FOLDERS
        message("~~~ $${LPUB3D} INFO - RUNTIME PATHS USING BUILD FOLDERS ~~~")
    }

    # executable target name
    macx: TARGET = $$join(TARGET,,,_debug)
    win32:TARGET = $$join(TARGET,,,d)
    unix:!macx: TARGET = $$join(TARGET,,,d$$VER_MAJOR$$VER_MINOR)

    # enable copy ldvMessages to OUT_PWD/mainApp/extras (except macOS)
    !macx:DEVL_LDV_MESSAGES_INI = True

} else {

    BUILD_CONF = Release
    ARCH_BLD = bit_release

    win32 {
        LDRAWINI_LIB = LDrawIni161
        QUAZIP_LIB = QuaZIP11
        LC_LIB = LC233
        LDVQT_LIB = LDVQt45
        WPNGIMAGE_LIB = WPngImage14
        WAITING_SPINNER_LIB = WaitingSpinner10
    }

    macx {
        LDRAWINI_LIB = LDrawIni
        QUAZIP_LIB = QuaZIP
        LC_LIB = LC
        LDVQT_LIB = LDVQt
        WPNGIMAGE_LIB = WPngImage
        WAITING_SPINNER_LIB = WaitingSpinner
    }

    unix:!macx {

        LDRAWINI_LIB = ldrawini
        QUAZIP_LIB = quazip
        LC_LIB = lc
        LDVQT_LIB = ldvqt
        WPNGIMAGE_LIB = wpngimage
        WAITING_SPINNER_LIB = waitingspinner
    }

    # executable target
    !macx:!win32: TARGET = $$join(TARGET,,,$$VER_MAJOR$$VER_MINOR)
}
BUILD += $$BUILD_CONF

#manpage
MAN_PAGE = $$join(TARGET,,,.1)

message("~~~ $${LPUB3D} LPUB3D $$join(ARCH,,,bit) $${BUILD} ($${TARGET}) $${CHIPSET} Chipset ~~~")

#~~file distributions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

!isEmpty(3RD_DIR_SOURCE_UNSPECIFIED): message("~~~ $${LPUB3D} $$3RD_DIR_SOURCE_UNSPECIFIED ~~~")
message("~~~ $${LPUB3D} 3RD PARTY DISTRIBUTION REPO ($$3RD_DIR_SOURCE): $$THIRD_PARTY_DIST_DIR_PATH ~~~")

# To build and install locally or from QC, set CONFIG+=dmg|deb|rpm|pkg|exe respectively.
build_package = $$(LP3D_BUILD_PKG) # triggered from cloud build scripts
if(deb|rpm|pkg|dmg|exe|api|snp|flp|con|contains(build_package, yes)) {
    args = deb rpm pkg dmg exe api snp flp con
    for(arg, args) {
        contains(CONFIG, $$arg): opt = $$arg
    }

    isEmpty(opt) {
        opt = $$build_package
    }

    contains(opt, api) {
        DEFINES += LP3D_APPIMAGE
        DISTRO_PACKAGE = APPIMAGE ($$opt)
    } else:contains(opt, snp) {
        DEFINES += LP3D_SNAP
        DISTRO_PACKAGE = SNAP ($$opt)
    } else:contains(opt, flp) {
        DEFINES += LP3D_FLATPACK
        DISTRO_PACKAGE = FLATPACK ($$opt)
    } else:contains(opt, con) {
        DISTRO_PACKAGE = CONDA ($$opt)
        CONFIG-=$$opt
        CONFIG+=conda_build
        DEFINES += LP3D_CONDA
    } else {
        DISTRO_PACKAGE = ($$opt)
    }

    message("~~~ $${LPUB3D} BUILD DISTRIBUTION PACKAGE: $$DISTRO_PACKAGE ~~~")

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

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# build path component
DESTDIR = $$join(ARCH,,,$$ARCH_BLD)

PRECOMPILED_DIR = $$DESTDIR/.pch
OBJECTS_DIR     = $$DESTDIR/.obj
MOC_DIR         = $$DESTDIR/.moc
RCC_DIR         = $$DESTDIR/.qrc
UI_DIR          = $$DESTDIR/.ui

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Specify LDView modules develpment or third party header paths
equals(VER_USE_LDVIEW_DEV,True) {
    INCLUDEPATH += $$system_path( $${VER_LDVIEW_DEV_REPOSITORY} ) $$system_path( $${VER_LDVIEW_DEV_REPOSITORY}/include )
} else {
    INCLUDEPATH += $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/include )
}

# Needed to access ui header from LDVQt
INCLUDEPATH += $$OUT_PWD/../ldvlib/LDVQt/$$DESTDIR/.ui

win32-msvc* {
    INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib
}

#~~ includes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

win32:include(winfiledistro.pri)
macx:include(macosfiledistro.pri)
unix:!macx:include(linuxfiledistro.pri)

include(../qslog/QsLog.pri)
include(../qsimpleupdater/QSimpleUpdater.pri)

#~~~ libraries~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

quazipnobuild: \
LIBS += -lquazip
else: \
LIBS += -L$$OUT_PWD/../quazip/$$DESTDIR -l$$QUAZIP_LIB

LIBS += -L$$OUT_PWD/../ldrawini/$$DESTDIR -l$$LDRAWINI_LIB

LIBS += -L$$OUT_PWD/../waitingspinner/$$DESTDIR -l$$WAITING_SPINNER_LIB

LIBS += -L$$OUT_PWD/../lclib/$$DESTDIR -l$$LC_LIB

LIBS += -L$$OUT_PWD/../ldvlib/LDVQt/$$DESTDIR -l$$LDVQT_LIB

# WPngImage must follow LDVQT or else there will be compile errors
LIBS += -L$$OUT_PWD/../ldvlib/WPngImage/$$DESTDIR -l$$WPNGIMAGE_LIB

# LDViewLibs uses minizip which must follow the QuaZip lib to avoid LNK4006 warnings
include(../ldvlib/LDVQt/LDViewLibs.pri)

win32 {
    DEFINES += _WIN_UTF8_PATHS
    LIBS += -ladvapi32 -lshell32 -lopengl32 -lglu32 -lwininet -luser32 -lws2_32 -lgdi32
} else:!macx {
    LIBS += -lGL -lGLU
}
!win32-msvc* {
    LIBS += -lz
}

#~~ miscellaneous ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# set config to enable/disable initial update check
# CONFIG+=disable_update_check
disable_update_check: DEFINES += DISABLE_UPDATE_CHECK

#~~ source and headers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

HEADERS += \
    aboutdialog.h \
    annotations.h \
    application.h \
    archiveparts.h \
    backgrounddialog.h \
    backgrounditem.h \
    blenderpreferences.h \
    bomoptionsdialog.h \
    borderdialog.h \
    borderedlineitem.h \
    callout.h \
    calloutbackgrounditem.h \
    calloutpointeritem.h \
    color.h \
    commands.h \
    commands/command.h \
    commands/commandcollection.h \
    commands/commandcompleter.h \
    commands/commandlistmodel.h \
    commands/commandsdialog.h \
    commands/commandstablemodel.h \
    commands/commandstextedit.h \
    commands/filterlineedit.h \
    commands/jsoncommandtranslator.h \
    commands/jsoncommandtranslatorfactory.h \
    commands/snippets/jsonsnippettranslator.h \
    commands/snippets/jsonsnippettranslatorfactory.h \
    commands/snippets/snippet.h \
    commands/snippets/snippetcollection.h \
    commands/snippets/snippetcompleter.h \
    commands/snippets/snippetlistmodel.h \
    commands/snippets/snippetstablemodel.h \
    commonmenus.h \
    csiannotation.h \
    csiitem.h \
    declarations.h \
    dependencies.h \
    dialogexportpages.h \
    dividerdialog.h \
    dividerpointeritem.h \
    editwindow.h \
    excludedparts.h \
    fadehighlightdialog.h \
    findreplace.h \
    globals.h \
    gradients.h \
    highlighter.h \
    highlightersimple.h \
    historylineedit.h \
    hoverpoints.h \
    ldrawcolordialog.cpp \
    ldrawcolordialog.h \
    ldrawcolourparts.h \
    ldrawfiles.h \
    ldrawfilesload.h \
    ldsearchdirs.h \
    lgraphicsscene.h \
    lgraphicsview.h \
    lpub.h \
    lpub_object.h \
    lpub_preferences.h \
    lpub_qtcompat.h \
    messageboxresizable.h \
    meta.h \
    metagui.h \
    metaitem.h \
    metatypes.h \
    numberitem.h \
    options.h \
    pageattributepixmapitem.h \
    pageattributetextitem.h \
    pagebackgrounditem.h \
    pageorientationdialog.h \
    pagepointer.h \
    pagepointerbackgrounditem.h \
    pagepointeritem.h \
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
    pointerattribdialog.h \
    pointeritem.h \
    pointerplacementdialog.h \
    preferencesdialog.h \
    preferredrendererdialog.h \
    range.h \
    range_element.h \
    ranges.h \
    ranges_element.h \
    ranges_item.h \
    render.h \
    renderdialog.h \
    reserve.h \
    resize.h \
    resolution.h \
    rotateiconitem.h \
    rotateiconsizedialog.h \
    rotstepdialog.h \
    rx.h \
    scaledialog.h \
    separatorcombobox.h \
    sizeandorientationdialog.h \
    step.h \
    stickerparts.h \
    submodelcolordialog.h \
    submodelitem.h \
    substitutepartdialog.h \
    texteditdialog.h \
    textitem.h \
    threadworkers.h \
    updatecheck.h \
    version.h \
    where.h

SOURCES += \
    aboutdialog.cpp \
    annotations.cpp \
    application.cpp \
    archiveparts.cpp \
    assemglobals.cpp \
    backgrounddialog.cpp \
    backgrounditem.cpp \
    blenderpreferences.cpp \
    bomoptionsdialog.cpp \
    borderdialog.cpp \
    borderedlineitem.cpp \
    callout.cpp \
    calloutbackgrounditem.cpp \
    calloutglobals.cpp \
    calloutpointeritem.cpp \
    color.cpp \
    commands/commandcollection.cpp \
    commands/commandcompleter.cpp \
    commands/commandlistmodel.cpp \
    commands/commandsdialog.cpp \
    commands/commandstablemodel.cpp \
    commands/commandstextedit.cpp \
    commands/filterlineedit.cpp \
    commands/jsoncommandtranslator.cpp \
    commands/snippets/jsonsnippettranslator.cpp \
    commands/snippets/snippetcollection.cpp \
    commands/snippets/snippetcompleter.cpp \
    commands/snippets/snippetlistmodel.cpp \
    commands/snippets/snippetstablemodel.cpp \
    commandline.cpp \
    commands.cpp \
    commonmenus.cpp \
    csiannotation.cpp \
    csiitem.cpp \
    dependencies.cpp \
    dialogexportpages.cpp \
    dividerdialog.cpp \
    dividerpointeritem.cpp \
    editwindow.cpp \
    excludedparts.cpp \
    fadehighlightdialog.cpp \
    fadestepglobals.cpp \
    findreplace.cpp \
    formatpage.cpp \
    gradients.cpp \
    highlighter.cpp \
    highlightersimple.cpp \
    highlightstepglobals.cpp \
    historylineedit.cpp \
    hoverpoints.cpp \
    lclibpreferences.cpp \
    ldrawcolordialog.cpp \
    ldrawcolourparts.cpp \
    ldrawfiles.cpp \
    ldrawfilesload.cpp \
    ldrawpartdialog.cpp \
    ldsearchdirs.cpp \
    lgraphicsscene.cpp \
    lgraphicsview.cpp \
    lpub.cpp \
    lpub_object.cpp \
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
    pagepointerbackgrounditem.cpp \
    pagepointeritem.cpp \
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
    pointerattribdialog.cpp \
    pointeritem.cpp \
    pointerplacementdialog.cpp \
    preferencesdialog.cpp \
    preferredrendererdialog.cpp \
    printfile.cpp \
    projectglobals.cpp \
    range.cpp \
    range_element.cpp \
    ranges.cpp \
    ranges_element.cpp \
    ranges_item.cpp \
    render.cpp \
    renderdialog.cpp \
    reserve.cpp \
    resize.cpp \
    resolution.cpp \
    rotate.cpp \
    rotateiconitem.cpp \
    rotateiconsizedialog.cpp \
    rotstepdialog.cpp \
    rx.cpp \
    scaledialog.cpp \
    separatorcombobox.cpp \
    sizeandorientationdialog.cpp \
    step.cpp \
    stickerparts.cpp \
    submodelcolordialog.cpp \
    submodelglobals.cpp \
    submodelitem.cpp \
    substitutepartdialog.cpp \
    texteditdialog.cpp \
    textitem.cpp \
    threadworkers.cpp \
    traverse.cpp \
    undoredo.cpp \
    updatecheck.cpp \
    visualeditor.cpp

FORMS += \
    aboutdialog.ui \
    dialogexportpages.ui \
    ldrawcolordialog.ui \
    ldrawfilesloaddialog.ui \
    ldrawpartdialog.ui \
    preferences.ui \
    renderdialog.ui \
    substitutepartdialog.ui \
    texteditdialog.ui

include(otherfiles.pri)

RESOURCES += \
    ../lclib/lclib.qrc \
    ../lclib/resources/stylesheet/stylesheet.qrc \
    ../ldvlib/LDVQt/resources.qrc \
    resources/lsynth/lsynth.qrc \
    lpub3d.qrc

#~~ suppress warnings ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

!win32-msvc*:!macx {
QMAKE_CFLAGS_WARN_ON += \
    -Wno-deprecated-declarations \
    -Wno-deprecated-copy \
    -Wno-unused-parameter \
    -Wno-sign-compare
}

macx {

QMAKE_CFLAGS_WARN_ON += \
    -Wno-deprecated-copy \
    -Wno-overloaded-virtual \
    -Wno-self-assign \
    -Wno-sometimes-uninitialized \
    -Wno-unused-result \
    -Wno-deprecated-declarations
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
