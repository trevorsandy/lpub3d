###################################################
# LPub3D common project declarations and directives
#~~ fine-grained host identification ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# platform architecture
BUILD_ARCH = $${QT_ARCH}
contains(BUILD_ARCH, unknown)|\
isEmpty(BUILD_ARCH): \
BUILD_ARCH = $$(TARGET_CPU)
isEmpty(BUILD_ARCH): \
BUILD_ARCH = $$system(uname -m)
isEmpty(BUILD_ARCH): \
BUILD_ARCH = UNKNOWN ARCH
# platform name
BUILD_TARGET                 = $$(TARGET_VENDOR)
isEmpty(BUILD_TARGET) {
    msys:BUILD_TARGET        = MSYS2 MICROSOFT WINDOWS
    win32-arm64-msvc|win32-msvc*:BUILD_TARGET = $$system(systeminfo | findstr /B /C:\"OS Name\")
    unix:!macx:BUILD_TARGET  = $$system(. /etc/os-release 2>/dev/null; [ -n \"$PRETTY_NAME\" ] && echo \"$PRETTY_NAME\" || echo `uname`)
    macx:BUILD_TARGET        = $$system(echo `sw_vers -productName`)
}
# platform version
HOST_VERSION                 = $$(PLATFORM_VER)
isEmpty(HOST_VERSION) {
    win32-arm64-msvc|win32-msvc*:HOST_VERSION = $$system(systeminfo | findstr /B /C:\"OS Version\")
    unix:!macx:HOST_VERSION  = $$system(. /etc/os-release 2>/dev/null; [ -n \"$VERSION_ID\" ] && echo \"$VERSION_ID\")
    macx:HOST_VERSION        = $$system(echo `sw_vers -productVersion`)
    mingw:ide_qtcreator:HOST_VERSION = MinGW_2025
    else:msys:HOST_VERSION   = $$system(VER=$(echo $(uname -a) | grep -oP \"\b\d{4}-\d{2}-\d{2}\b\") && echo ${MSYSTEM}_${VER//-/.})
}
# host
if (equals(TEMPLATE, subdirs)|equals(COMMON, LDViewGlobal)) {
    HOST     = $${BUILD_TARGET}
    macx: \
    HOST     = $${HOST} $${HOST_VERSION}
    isEmpty(HOST):HOST = UNKNOWN HOST
}
# processor architecture - for aarch64, QT_ARCH = arm64, for arm7l, QT_ARCH = arm
if (contains(QT_ARCH, x86_64)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)) {
    ARCH     = 64
    contains(QT_ARCH, x86_64): \
    STG_ARCH = x86_64
    else: STG_ARCH = ARM64
    LIB_ARCH = 64
} else {
    ARCH     = 32
    STG_ARCH = x86
    LIB_ARCH =
}
# processor chipset
if (contains(QT_ARCH, arm)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)) {
    CHIPSET  = ARM
} else {
    CHIPSET  = AMD
}

greaterThan(QT_MAJOR_VERSION, 5) {
#~~ Qt5 Core Compatibility for Qt6 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    QT += core5compat
}

#~~ Qt/OSMesa/WGL library identifiers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

ldviewqt {
    BUILD           = QT
    POSTFIX         = -qt$${QT_MAJOR_VERSION}
    CONFIG         += CUI_QT
    equals(TARGET, LDVQt): \
    DEFINES        += _QT
    equals(COMMON, LDViewGlobal) {
        CONFIG     += qt
        QT         += core
        contains(DEFINES, _OSMESA): \
        DEFINES    -= _OSMESA
        DEFINES    += _QT
    }
} else:ldviewwgl {
    BUILD           = WGL
    POSTFIX         = -wgl
    CONFIG         += CUI_WGL
    equals(COMMON, LDViewGlobal) {
        QT         -= core
        contains(DEFINES, _OSMESA): \
        DEFINES    -= _OSMESA
        contains(DEFINES, _QT): \
        DEFINES    -= _QT
        DEFINES    += _LP3D_CUI_WGL
    }
} else:!win32-arm64-msvc:!win32-msvc* {
    BUILD           = OSMesa
    POSTFIX         = -osmesa
    CONFIG         += OSMesa
    equals(TARGET, LDVQt): \
    DEFINES        += _OSMESA
    equals(COMMON, LDViewGlobal) {
        QT         -= core
        contains(DEFINES, _QT): \
        DEFINES    -= _QT
        DEFINES    += _OSMESA
    }
}

#~~ paths ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

win32 {
    SYSTEM_PREFIX_  = $${PREFIX}
} else {
    # System libraries - on Unix, change to or add /usr/local if you want
    macx:contains(QT_ARCH, arm64): \
    SYSTEM_PREFIX_  = /opt/homebrew
    else: \
    SYSTEM_PREFIX_  = $${PREFIX}/usr/local
}

#~~ renderer versions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

VER_LDVIEW          = ldview-4.6
VER_LDGLITE         = ldglite-1.3
VER_POVRAY          = lpub3d_trace_cui-3.8

#~~~~ third party distro folder ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# - Set enviroinment variable LP3D_DIST_DIR_PATH as needed.
# - 3rd party libraries, executables, documentation and resources.
# - When building on macOS, it is necessary to add CONFIG+=dmg at
#   Projects/Build Steps/Qmake/'Additional arguments' because,
#   macOS build will also bundle all deliverables.

#   Argument path - LP3D_3RD_DIST_DIR
!isEmpty(LP3D_3RD_DIST_DIR) {
    THIRD_PARTY_DIST_DIR_PATH = $$LP3D_3RD_DIST_DIR
    3RD_DIR_SOURCE = LP3D_3RD_DIST_DIR
} else {
#   Environment variable path - LP3D_DIST_DIR_PATH
    THIRD_PARTY_DIST_DIR_PATH = $$(LP3D_DIST_DIR_PATH)
    !isEmpty(THIRD_PARTY_DIST_DIR_PATH): \
    3RD_DIR_SOURCE = LP3D_DIST_DIR_PATH
}
#   Local path
isEmpty(THIRD_PARTY_DIST_DIR_PATH) {
    THIRD_PARTY_DIST_DIR_PATH = $$clean_path( $${PWD}/builds/3rdparty )
    3RD_DIR_SOURCE = LOCAL_3RD_DIST_DIR
}
#   Default path
!exists($$THIRD_PARTY_DIST_DIR_PATH) {
    unix:!macx: DIST_DIR      = lpub3d_linux_3rdparty
    else:msys:  DIST_DIR      = lpub3d_msys_3rdparty
    else:macx:  DIST_DIR      = lpub3d_macos_3rdparty
    else:win32: DIST_DIR      = lpub3d_windows_3rdparty
    THIRD_PARTY_DIST_DIR_PATH = $$clean_path( $$absolute_path( $${PWD}/../$${DIST_DIR} ) )
    exists($$THIRD_PARTY_DIST_DIR_PATH) {
        DIST_DIR_UNSPECIFIED_MSG = "INFO - THIRD_PARTY_DIST_DIR_PATH WAS NOT SPECIFIED, USING $$THIRD_PARTY_DIST_DIR_PATH"
        3RD_DIR_SOURCE = DEFAULT_3RD_PARTY_DIR    
    } else {
        DIST_DIR_UNSPECIFIED_MSG = "ERROR - THIRD_PARTY_DIST_DIR_PATH WAS NOT SPECIFIED!"
        THIRD_PARTY_DIST_DIR_PATH="undefined"
        if (equals(TARGET, LC)|equals(TARGET, LDVQt)): \
        DIST_DIR_NOT_FOUND_MSG = THIRD_PARTY_DIST_DIR_PATH FOR $$upper(TARGET) WAS NOT FOUND!
        3RD_DIR_SOURCE = UNDEFINED_3RD_PARTY_DIR
    }
}

!BUILD_LDV_LIBS {
    unix|msys: \
    LIB_LDVIEW  = libTCFoundation$${POSTFIX}.a
    else:win32-arm64-msvc|win32-msvc*: \
    LIB_LDVIEW  = TCFoundation.lib
    LIB_LDVIEW_PATH = $$clean_path( $${THIRD_PARTY_DIST_DIR_PATH}/$${VER_LDVIEW}/lib/$${QT_ARCH}/$${LIB_LDVIEW} )
    !exists($${LIB_LDVIEW_PATH}) {
        CONFIG += BUILD_LDV_LIBS
        NO_LDVIEW_DIST_LIBS = True
        contains(TEMPLATE, subdirs): \
        LDVIEW_DIST_LIBS_MSG = $$upper($${VER_LDVIEW}) LDV LIBRARIES BUILT FROM SOURCE
    }
}

BUILD_LDV_LIBS {
    USE_LDV_3RD_PARTY_LIBS:USE_LDV_SYSTEM_LIBS {
        CONFIG -= USE_LDV_SYSTEM_LIBS
        contains(TEMPLATE, subdirs): \
        USE_LDV_LIBRARIES_MSG = NOTICE 'USE_LDV_3RD_PARTY_LIBS' and 'USE_LDV_SYSTEM_LIBS' Specified. Using 'USE_LDV_3RD_PARTY_LIBS'
    }

    # Always build tinyxml, libgl2ps for MSVC and lib3ds except for MSVC
    USE_LDV_3RD_PARTY_LIBS {
        CONFIG += BUILD_3DS
        CONFIG += BUILD_TINYXML
        !USE_SYSTEM_JPEG: \
        CONFIG += BUILD_JPEG
        !USE_SYSTEM_PNG: \
        CONFIG += BUILD_PNG
        !USE_SYSTEM_GL2PS: \
        CONFIG += BUILD_GL2PS
        !USE_SYSTEM_MINIZIP: \
        CONFIG += BUILD_MINIZIP
        !USE_SYSTEM_ZLIB: \
        CONFIG += BUILD_ZLIB
    } else {
        CONFIG += BUILD_TINYXML
        win32-arm64-msvc|win32-msvc*: \
        CONFIG += BUILD_GL2PS
        else: \
        CONFIG += BUILD_3DS
    }

    # Set Qt when build LDV libs and neither Qt nor WGL was specified
    !ldviewqt:!ldviewwgl {
        BUILD           = QT
        POSTFIX         = -qt$${QT_MAJOR_VERSION}
        CONFIG         += CUI_QT
        equals(TARGET, LDVQt): \
        DEFINES        += _QT
        equals(COMMON, LDViewGlobal) {
            CONFIG     += qt
            QT         += core
            contains(DEFINES, _OSMESA): \
            DEFINES    -= _OSMESA
            DEFINES    += _QT
        }
    }
}

SUFFIX                  = $${BUILD}

#~~ extensions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

win32-arm64-msvc|win32-msvc* {
    EXT_S = lib
    EXT_D = dll
} else {
    EXT_S = a
    msys: \
    EXT_D = dll
    else:macx: \
    EXT_D = dylib
    else: \
    EXT_D = so
}

#~~ LDVQt development repo ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CONFIG(debug, debug|release) {
    VER_USE_LDVIEW_DEV = False
    BUILD_LDV_LIBS {
        # Do nothing
    } else {
        # These lines requires a git extract of ldview at the same location as the lpub3d git extract,
        # they also define the ldview git extract folder name - you can set as you like
        VER_LDVIEW_DEV = $$(LDVIEW_DEV)
        isEmpty(VER_LDVIEW_DEV) {
            mingw:ide_qtcreator: \
            VER_LDVIEW_DEV = undefined
            else:unix|msys: \
            VER_LDVIEW_DEV = ldview
            else:win32-arm64-msvc|win32-msvc*: \
            VER_LDVIEW_DEV = ldview
        }
        # This line defines the path of the ldview git extract relative to this project file
        VER_LDVIEW_DEV_REPOSITORY = $$clean_path( $$absolute_path( $${PWD}/../$${VER_LDVIEW_DEV} ) )
        exists($$VER_LDVIEW_DEV_REPOSITORY) {
            VER_USE_LDVIEW_DEV = True
            equals(TARGET, LPub3D): \
            LDVIEW_DEV_REPO_MSG = LINK LDVQt USING LDVIEW DEVELOPMENT REPOSITORY
            equals(TARGET, LDVQt) {
                LDVIEW_DEV_REPO_MSG = BUILD LDVQt USING LDVIEW DEVELOPMENT REPOSITORY
                LDVIEW_DEV_HDRS_MSG = ADD LDVIEW HEADERS TO INCLUDEPATH: $${VER_LDVIEW_DEV_REPOSITORY}
            }
        } else {
            equals(TARGET, LPub3D): \
            LDVIEW_DEV_REPO_MSG = NOTICE - COULD NOT LOAD LDVIEW DEV FROM: $${VER_LDVIEW_DEV_REPOSITORY}
        }
    }
}

#~~ LDVQt 3rdParty libs ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

contains(LIB_ARCH, 64) {
    win32-arm64-msvc: \
    LDVLIB_ARCH = /ARM64
    else: \
    LDVLIB_ARCH = /x64
}
equals(VER_USE_LDVIEW_DEV,True) {
    VER_LDVIEW_3RD_LIBS = $$clean_path( $${VER_LDVIEW_DEV_REPOSITORY}/lib$${LDVLIB_ARCH} )
} else:BUILD_LDV_LIBS {
    VER_LDVIEW_3RD_LIBS = $$clean_path( $${PWD}/ldvlib/LDVQt/LDView/lib$${LDVLIB_ARCH} )
} else {
    VER_LDVIEW_3RD_LIBS = $$clean_path( ldvlib/LDVQt/LDView/lib$${LDVLIB_ARCH} )
}

#~~ LDVQt MSVC 3rdParty libs suffix ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

win32-arm64-msvc|win32-msvc* {
    BUILD_WORKER_VERSION = $$(LP3D_VSVERSION)
    isEmpty(BUILD_WORKER_VERSION): BUILD_WORKER_VERSION = 2019
    lessThan(BUILD_WORKER_VERSION, 2019) {
        VSVER=vs2015
    } else {
        contains(QT_ARCH, i386): VSVER=vs2017
        else:win32-arm64-msvc: VSVER=vs2022
        else: VSVER=vs2019
    }
    equals(TARGET, LDVQt) {
        VS_BUILD_WORKER_MSG = BUILD WORKER: Visual Studio $${BUILD_WORKER_VERSION}
        VS_LIBRARY_VER_MSG = $$upper($${QT_ARCH}) MSVS LIBRARY VERSION: $${VSVER}
    }
}

#~~ LDraw path and archive library ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

equals(TARGET, LPub3D) {
    LDRAWDIR                 = $$(LDRAWDIR)
    isEmpty(LDRAWDIR) {
        win32-arm64-msvc|win32-msvc*: \
            LDRAWDIR         = $$(USERPROFILE)
        else: \
            LDRAWDIR         = $$(HOME)
    }
    exists($${LDRAWDIR}/parts) {
        LDRAW_PATH           = $$shell_path($$absolute_path($${LDRAWDIR}))
        LDRAW_DIR_FOUND_MSG  = LDRAW PARTS LIBRARY $${LDRAW_PATH}
        exists($${LDRAWDIR}/complete.zip) {
            LDRAW_ZIP_FOUND_MSG = LDRAW ARCHIVE PARTS LIBRARY complete.zip FOUND
        } else {
            exists($${THIRD_PARTY_DIST_DIR_PATH}/complete.zip) {
                if (mingw:ide_qtcreator)|win32-arm64-msvc|win32-msvc* {
                    LDRAW_ZIP_COPY_CMD = copy /v /y $${THIRD_PARTY_DIST_DIR_PATH}/complete.zip $${LDRAWDIR}
                } else {
                    LDRAW_ZIP_COPY_CMD = cp -f $${THIRD_PARTY_DIST_DIR_PATH}/complete.zip $${LDRAWDIR}
                }
                QMAKE_POST_LINK += $$escape_expand(\n\t) \
                                   @$${LDRAW_ZIP_COPY_CMD}
                exists($${LDRAWDIR}/complete.zip): \
                LDRAW_ZIP_FOUND_MSG = LDRAW ARCHIVE PARTS LIBRARY COPIED TO LDRAW DIRECTORY
            } else {
                LDRAW_ZIP_FOUND_MSG = ERROR - LDRAW ARCHIVE PARTS LIBRARY NOT FOUND
            }
        }
    } else {
        LDRAW_DIR_FOUND_MSG = ERROR - LDRAW PARTS LIBRARY NOT FOUND
    }

#~~ Set OBS Macro ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    OBS = $$(OBS)
    equals(OBS, true) {
        DEFINES += OBS
        OBS_BUILD_MSG = BUILDING ON OPEN BUILD SERVICE
    }
}

#~~ Use c++ 11 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# USE CPP 11
contains(USE_CPP11,NO) {
    if (equals(TARGET, LPub3D)|equals(TARGET, LC)|equals(TARGET, LDVQt)): \
        CPP11_MSG = DO NOT USE CPP11 SPECIFIED
} else {
    DEFINES += USE_CPP11
}

#~~ CXXFLAGS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

equals(QT_MAJOR_VERSION, 5) {
    win32-arm64-msvc|win32-msvc* {
        QMAKE_CXXFLAGS += /std:c++17
    } else:unix|msys {
        if (equals(TARGET, LPub3D)|equals(TARGET, LC)|equals(TARGET, LDVQt)) {
            system("g++ --help -v 2>/dev/null| grep -q std=c++17"): \
                CPP17_MSG = C++17 LANGUAGE FEATURE FOUND
        }
        # Greater than or equal to Qt 5.11
        greaterThan(QT_MINOR_VERSION, 11) {
            CONFIG += c++17
        } else {
            QMAKE_CXXFLAGS += -std=c++17
        }
    }
}

equals(QT_MAJOR_VERSION, 6) {
    win32-arm64-msvc|win32-msvc* {
        QMAKE_CXXFLAGS += /std:c++17
    } else:unix|msys {
        if (equals(TARGET, LPub3D)|equals(TARGET, LC)|equals(TARGET, LDVQt)) {
            system("g++ --help -v 2>/dev/null| grep -q std=c++17"): \
                CPP17_MSG = C++17 LANGUAGE FEATURE FOUND
        }
        CONFIG += c++17
    }
}

#~~ suppress warnings ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

win32 {
    CONFIG                     += windows
    CONFIG                     += force_debug_info
    DEFINES                    += _CRT_SECURE_NO_WARNINGS \
                                  _CRT_SECURE_NO_DEPRECATE=1 \
                                  _CRT_NONSTDC_NO_WARNINGS=1
win32-arm64-msvc|win32-msvc* {
    # warning C4996: 'stdext::make_checked_array_iterator': warning STL4043: stdext::checked_array_iterator,
    # stdext::unchecked_array_iterator, and related factory functions are non-Standard extensions a
    # nd will be removed in the future. std::span (since C++20) and gsl::span can be used instead.
    equals(QT_MAJOR_VERSION, 5):greaterThan(QMAKE_MSC_VER, 1941): \
    DEFINES                    += _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING # or
                                 #_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
    QMAKE_CXXFLAGS_RELEASE     += /FI winsock2.h /FI winsock.h
    QMAKE_CFLAGS_WARN_ON       -= -W3
    QMAKE_ADDL_MSVC_FLAGS       = -WX- -GS -Gd -fp:precise -Zc:forScope
    CONFIG(debug, debug|release) {
        QMAKE_ADDL_MSVC_DEBUG_FLAGS = -RTC1 $$QMAKE_ADDL_MSVC_FLAGS
        QMAKE_CFLAGS_WARN_ON   += -W4 -wd"4005" -wd"4456" -wd"4458" -wd"4459" -wd"4127" -wd"4701" -wd"4714" -wd"4305" -wd"4099"
        QMAKE_CFLAGS_DEBUG     += $$QMAKE_ADDL_MSVC_DEBUG_FLAGS
        QMAKE_CXXFLAGS_DEBUG   += $$QMAKE_ADDL_MSVC_DEBUG_FLAGS
    }
    CONFIG(release, debug|release) {
        QMAKE_ADDL_MSVC_RELEASE_FLAGS = $$QMAKE_ADDL_MSVC_FLAGS -GF -Gy
        QMAKE_CFLAGS_OPTIMIZE  += -Ob1 -Oi -Ot
        QMAKE_CFLAGS_WARN_ON   += -W1 -WX- -wd"4005" -wd"4456" -wd"4458" -wd"4805" -wd"4838" -wd"4700" -wd"4098"
        QMAKE_CFLAGS_RELEASE   += $$QMAKE_ADDL_MSVC_RELEASE_FLAGS
        QMAKE_CXXFLAGS_RELEASE += $$QMAKE_ADDL_MSVC_RELEASE_FLAGS
    }
    QMAKE_CXXFLAGS_WARN_ON = $$QMAKE_CFLAGS_WARN_ON
} # win32-arm64-msvc|win32-msvc*
} # win32

if (unix|msys) {
QMAKE_CFLAGS_WARN_ON = \
    -Wall -W \
    -Wno-deprecated-declarations \
    -Wno-unknown-pragmas \
    -Wno-unused-result \
    -Wno-alloc-size-larger-than
QMAKE_CXXFLAGS_WARN_ON  = $${QMAKE_CFLAGS_WARN_ON}
QMAKE_CXXFLAGS_WARN_ON += \
    -Wno-deprecated-copy \
    -Wno-overloaded-virtual
} # unix|msys
if (unix|msys):!macx {
QMAKE_CFLAGS_WARN_ON += \
    -Wno-clobbered \
    -Wno-comment \
    -Wno-format \
    -Wno-format-security \
    -Wno-implicit-fallthrough \
    -Wno-parentheses \
    -Wno-return-type \
    -Wno-sign-compare \
    -Wno-strict-aliasing \
    -Wno-stringop-overflow \
    -Wno-stringop-truncation \
    -Wno-switch \
    -Wno-uninitialized \
    -Wno-unused-but-set-variable \
    -Wno-unused-parameter \
    -Wno-unused-variable
QMAKE_CXXFLAGS_WARN_ON += \
    -Wno-class-memaccess  \
    -Wno-type-limits
CUI_WGL: \
QMAKE_CFLAGS_WARN_ON += \
    -Wno-missing-field-initializers
msys {
QMAKE_CFLAGS_WARN_ON += \
    -Wno-attributes \
    -Wno-calloc-transposed-args
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}
QMAKE_CXXFLAGS_WARN_ON += \
    -Wno-cast-function-type \
    -Wno-conversion-null \
    -Wno-cpp \
    -Wno-maybe-uninitialized \
    -Wno-template-id-cdtor
QMAKE_CFLAGS_WARN_ON += \
    -Wno-implicit-function-declaration \
    -Wno-incompatible-pointer-types \
    -Wno-misleading-indentation
} else {
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}
} # msys
} # unix|msys:!macx
macx {
QMAKE_CFLAGS_WARN_ON += \
    -Wno-for-loop-analysis \
    -Wno-incompatible-pointer-types-discards-qualifiers \
    -Wno-int-conversion \
    -Wno-invalid-source-encoding \
    -Wno-mismatched-new-delete \
    -Wno-nullability-completeness \
    -Wno-reorder \
    -Wno-self-assign \
    -Wno-sometimes-uninitialized \
    -Wno-undefined-bool-conversion
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}
QMAKE_CXXFLAGS_WARN_ON += \
    -Wno-deprecated-non-prototype
QMAKE_CFLAGS_WARN_ON += \
    -Wno-implicit-function-declaration \
    -Wno-incompatible-pointer-types
} # macx
