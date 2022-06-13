win32 {
    COPY_CMD = COPY /y /a /b
} else:unix {
    COPY_CMD = cp -f
}

contains(LOAD_LDV_HEADERS,True) {
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # 3rd party executables, documentation and resources.
    isEmpty(THIRD_PARTY_DIST_DIR_PATH) {
        !isEmpty(LP3D_3RD_DIST_DIR) {
            THIRD_PARTY_DIST_DIR_PATH = $$LP3D_3RD_DIST_DIR
        } else {
            THIRD_PARTY_DIST_DIR_PATH = $$(LP3D_DIST_DIR_PATH)
            isEmpty(THIRD_PARTY_DIST_DIR_PATH): THIRD_PARTY_DIST_DIR_PATH="undefined"
        }
        !exists($$THIRD_PARTY_DIST_DIR_PATH) {
            unix:!macx: DIST_DIR      = lpub3d_linux_3rdparty
            else:macx: DIST_DIR       = lpub3d_macos_3rdparty
            else:win32: DIST_DIR      = lpub3d_windows_3rdparty
            THIRD_PARTY_DIST_DIR_PATH = $$system_path( $$absolute_path( $$PWD/../../../$$DIST_DIR ) )
            !exists($$THIRD_PARTY_DIST_DIR_PATH) {
                message("~~~ ERROR - THIRD_PARTY_DIST_DIR_PATH (LDVLIB) WAS NOT FOUND! ~~~ ")
                THIRD_PARTY_DIST_DIR_PATH="undefined"
            }
        }
    }
    VER_LDVIEW = ldview-4.4
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # Header soruce path
    isEmpty(LDVHDRDIR):LDVHDRDIR = $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/include )
#    message("~~~ lib$${TARGET} Headers source: $$LDVHDRDIR ~~~ ")

    # Copy LDView headers to LDV (Enabled on Linux until I figure out what's blocking)
    contains(COPY_LDV_HEADERS,True) {
        # Header destination path
        isEmpty(LDVINCLUDE):LDVINCLUDE = $$system_path( $$absolute_path( ./include ) )
        isEmpty(LDV3RDHDRDIR):LDV3RDHDRDIR = $$system_path( $${LDVINCLUDE}/3rdParty )
        message("~~~ ENABLE COPY LDVIEW HEADERS TO: $$system_path( $$LDVINCLUDE ) ~~~ ")
        DEPENDPATH  += ./include
        INCLUDEPATH += ./include
        INCLUDEPATH += ./include/3rdParty

        # Construct delete command
        win32 {
            DELETE_HDR_DIRS_CMD = IF EXIST $$LDVINCLUDE RMDIR /S /Q $$LDVINCLUDE
            CREATE_HDR_DIRS_CMD = MKDIR $$LDVINCLUDE\LDLib $$LDVINCLUDE\LDExporter $$LDVINCLUDE\LDLoader $$LDVINCLUDE\TRE $$LDVINCLUDE\TCFoundation $$LDVINCLUDE\3rdParty $$LDVINCLUDE\GL
        } else {
            DELETE_HDR_DIRS_CMD = if test -e $$LDVINCLUDE; then rm -rf $$LDVINCLUDE; fi
            CREATE_HDR_DIRS_CMD = mkdir -p $$LDVINCLUDE/LDExporter $$LDVINCLUDE/LDLib \
                                  $$LDVINCLUDE/LDLoader $$LDVINCLUDE/TRE $$LDVINCLUDE/TCFoundation \
                                  $$LDVINCLUDE/3rdParty $$LDVINCLUDE/GL
        }

        # Initialize local header folders
        system( $$DELETE_HDR_DIRS_CMD )
        system( $$CREATE_HDR_DIRS_CMD )

        # Copy headers from LDView LDVHDRDIR (lpub3d_<platform>_3rdParty) to LDVINCLUDE
        system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/LDLib/*.h) $$system_path( $${LDVINCLUDE}/LDLib/ ) )
        system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/LDExporter/*.h) $$system_path( $${LDVINCLUDE}/LDExporter/ ) )
        system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/LDLoader/*.h) $$system_path( $${LDVINCLUDE}/LDLoader/) )
        system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/TRE/*.h) $$system_path( $${LDVINCLUDE}/TRE/) )
        system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/TCFoundation/*.h) $$system_path( $${LDVINCLUDE}/TCFoundation/ ) )
        system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/3rdParty/*.h) $$system_path( $${LDVINCLUDE}/3rdParty/ ) )

        exists( $$system_path( $$LDVINCLUDE/TCFoundation/TCObject.h ) ): \
        message("~~~ lib$${TARGET} LDView headers copied to $$system_path( $${LDVINCLUDE} ) ~~~")

        if (contains(ARM_BUILD_ARCH,True)) {
            if(!contains(DEFINES,ARM_SKIP_GL_HEADERS)) {
                system( touch $$system_path( $${LDVHDRDIR}/GL/glext.h ) )
                message("~~~ lib$${TARGET} touch glext.h ~~~")
            }
            message("~~~ lib$${TARGET} skip copy LDView GL headers ~~~")
        } else {
            system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/GL/*.h) $$system_path( $${LDVINCLUDE}/GL/ ) )
            exists( $$system_path( $${LDVINCLUDE}/GL/gl.h) ): \
            message("~~~ lib$${TARGET} LDView GL headers copied to $$system_path( $${LDVINCLUDE}/GL/ ) ~~~")
        }
    }  else {
        message("~~~ ADD LDVIEW HEADERS TO INCLUDEPATH: $$system_path( $$LDVHDRDIR ) ~~~ ")
        isEmpty(LDV3RDHDRDIR):LDV3RDHDRDIR = $$system_path( $${LDVHDRDIR}/3rdParty )
        DEPENDPATH  += $${LDVHDRDIR}
        INCLUDEPATH += $${LDVHDRDIR}
        INCLUDEPATH += $${LDV3RDHDRDIR}
        if (contains(ARM_BUILD_ARCH,True)) {
            if(!contains(DEFINES,ARM_SKIP_GL_HEADERS)) {
                system( mkdir -p ./include/GL && touch ./include/GL/glext.h )
                INCLUDEPATH += $$system_path( ./include/GL )
                message("~~~ lib$${TARGET} touch include/GL/glext.h for $$upper($$QT_ARCH) build ~~~")
            }
            exists ($$system_path( $${LDVHDRDIR}/GL )): \
            system(rm -rf $$system_path( $${LDVHDRDIR}/GL ))
            !exists ($$system_path( $${LDVHDRDIR}/GL )): \
            message("~~~ lib$${TARGET} disable LDView GL headers for $$upper($$QT_ARCH) build ~~~")
        } else {
            INCLUDEPATH += $$system_path( $${LDVHDRDIR}/GL )
            message("~~~ lib$${TARGET} include LDView GL headers ) ~~~")
        }
    }

    if (unix:exists(/usr/include/tinyxml.h)|exists(/usr/local/include/tinyxml.h)) {
        message("~~~ lib$${TARGET} system library tinyxml found ~~~")
    } else:exists($${LDV3RDHDRDIR}/tinyxml.h) {
        message("~~~ lib$${TARGET} local library header for tinyxml found ~~~")
    } else {
        message("~~~ ERROR: Library header for tinyxml not found ~~~")
    }
    if (unix:exists(/usr/include/gl2ps.h)|exists(/usr/local/include/gl2ps.h)) {
        message("~~~ lib$${TARGET} system library gl2ps found ~~~")
    } else:exists($${LDV3RDHDRDIR}/gl2ps.h) {
        message("~~~ lib$${TARGET} local library header for gl2ps found ~~~")
    } else {
        message("~~~ ERROR: Library header for gl2ps not found, using local ~~~")
    }
    if (unix:exists(/usr/include/lib3ds.h)|exists(/usr/local/include/lib3ds.h)){
        message("~~~ lib$${TARGET} system library 3ds found ~~~")
    } else:exists($${LDV3RDHDRDIR}/lib3ds.h) {
        message("~~~ lib$${TARGET} local library header for 3ds found ~~~")
    } else {
        message("~~~ ERROR: Library header for 3ds not found ~~~")
    }
    if (unix:macx:exists(/usr/include/zip.h)|exists(/usr/local/include/minizip/zip.h)) {
        message("~~~ lib$${TARGET} system library minizip found ~~~")
    } else:exists($${LDV3RDHDRDIR}/zip.h) {
        message("~~~ lib$${TARGET} local library header for minizip found ~~~")
    } else:macx {
        message("~~~ ERROR: Library header for minizip not found ~~~")
    }
    if (unix:exists(/usr/include/png.h)|exists(/usr/local/include/png.h)) {
        message("~~~ lib$${TARGET} system library png found ~~~")
    } else:exists($${LDV3RDHDRDIR}/png.h) {
        message("~~~ lib$${TARGET} local library header for png found ~~~")
    } else {
        message("~~~ ERROR: Library header for png not found ~~~")
    }
    if (unix:exists(/usr/include/jpeglib.h)|exists(/usr/local/include/jpeglib.h)) {
        message("~~~ lib$${TARGET} system library jpeg found ~~~")
    } else:exists($${LDV3RDHDRDIR}/gl2ps.h) {
        message("~~~ lib$${TARGET} local library header for jpeg found ~~~")
    } else {
        message("~~~ ERROR: Library header for jpeg not found ~~~")
    }

    # These includes are only processed in debug on Windows mode
    win32-msvc*:equals(LOAD_LDV_SOURCE_FILES,True) {
        # Copy source from LDView LDVHDRDIR - lpub3d_<platfor>_3rdParty
        system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/LDLib/*.c*) $$system_path( $${LDVINCLUDE}/LDLib/ ) )
        system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/LDExporter/*.c*) $$system_path( $${LDVINCLUDE}/LDExporter/ ) )
        system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/LDLoader/*.c*) $$system_path( $${LDVINCLUDE}/LDLoader/) )
        system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/TRE/*.c*) $$system_path( $${LDVINCLUDE}/TRE/) )
        system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/TCFoundation/*.c*) $$system_path( $${LDVINCLUDE}/TCFoundation/ ) )
        exists($$system_path( $$LDVINCLUDE/TCFoundation/TCFoundation.cpp )): \
        message("~~~ lib$${TARGET} project source files copied to $$system_path( $${LDVINCLUDE} ) ~~~")

        # Copy headers from LDView LDVHDRDIR - lpub3d_<platfor>_3rdParty
        system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/LDLib/*.pri) $$system_path( $${LDVINCLUDE}/LDLib/ ) )
        system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/LDExporter/*.pri) $$system_path( $${LDVINCLUDE}/LDExporter/ ) )
        system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/LDLoader/*.pri) $$system_path( $${LDVINCLUDE}/LDLoader/) )
        system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/TRE/*.pri) $$system_path( $${LDVINCLUDE}/TRE/) )
        system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/TCFoundation/*.pri) $$system_path( $${LDVINCLUDE}/TCFoundation/ ) )
        exists($$system_path( $$LDVINCLUDE/TCFoundation/TCFoundation.pri )): \
        message("~~~ lib$${TARGET} project include files copied to $$system_path( $${LDVINCLUDE} ) ~~~")
    }
}

win32 {
    BUILD_WORKER_VERSION = $$(LP3D_VSVERSION)
    isEmpty(BUILD_WORKER_VERSION): BUILD_WORKER_VERSION = 2019
    !contains(LOAD_LDVLIBS,True): \
    message("~~~ lib$${TARGET} build worker: Visual Studio $$BUILD_WORKER_VERSION ~~~")
    equals(BUILD_WORKER_VERSION, 2019) {
        contains(QT_ARCH,i386): VSVER=vs2017
        else: VSVER=vs2019
    } else {
        VSVER=vs2015
    }
    !contains(LOAD_LDVLIBS,True): \
    message("~~~ lib$${TARGET} $$upper($$QT_ARCH) MSVS library version: $$VSVER ~~~")
}

# This block is executed by LPub3D mainApp to enable linking the LDVlib
contains(LOAD_LDVLIBS,True) {
    isEmpty(LDVLIBRARY):LDVLIBRARY = $$system_path( $$absolute_path( $$OUT_PWD/../ldvlib/LDVQt/$$DESTDIR ) )
    win32-msvc*:CONFIG(debug, debug|release): \
    LDVLIBDIR = $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/Build/Debug$$LIB_ARCH )
    else: \
    LDVLIBDIR = $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/bin/$$QT_ARCH )

#    message("~~~ lib$${TARGET} Library path: $$LDVLIBRARY ~~~ ")
#    message("~~~ lib$${TARGET} Library source: $$LDVLIBDIR ~~~ ")

    # Set library names, source pahts and local paths
    win32-msvc* {
        # library name
        LDLIB_LIB        = -lLDLib
        LDEXPORTER_LIB   = -lLDExporter
        LDLOADER_LIB     = -lLDLoader
        TRE_LIB          = -lTRE
        TCFOUNDATION_LIB = -lTCFoundation

        GL2PS_LIB        = -lgl2ps
        TINYXML_LIB      = -ltinyxml_STL
        3DS_LIB          = -llib3ds-$${VSVER}
        ZIP_LIB          = -lunzip32-$${VSVER}
        PNG_LIB          = -llibpng16-$${VSVER}
        JPEG_LIB         = -llibjpeg-$${VSVER}
        ZLIB_LIB         = -lzlib-$${VSVER}

        # source paths
        LDLIB_SRC        = $$system_path( $${LDVLIBDIR}/LDLib.lib )
        LDEXPORTER_SRC   = $$system_path( $${LDVLIBDIR}/LDExporter.lib )
        LDLOADER_SRC     = $$system_path( $${LDVLIBDIR}/LDLoader.lib )
        TRE_SRC          = $$system_path( $${LDVLIBDIR}/TRE.lib )
        TCFOUNDATION_SRC = $$system_path( $${LDVLIBDIR}/TCFoundation.lib )

        GL2PS_SRC        = $$system_path( $${LDVLIBDIR}/gl2ps.lib )
        TINYXML_SRC      = $$system_path( $${LDVLIBDIR}/tinyxml_STL.lib )
        3DS_SRC          = $$system_path( $${LDVLIBDIR}/lib3ds-$${VSVER}.lib )
        MINIZIP_SRC      = $$system_path( $${LDVLIBDIR}/unzip32-$${VSVER}.lib )
        PNG_SRC          = $$system_path( $${LDVLIBDIR}/libpng16-$${VSVER}.lib )
        JPEG_SRC         = $$system_path( $${LDVLIBDIR}/libjpeg-$${VSVER}.lib )
        ZLIB_SRC         = $$system_path( $${LDVLIBDIR}/zlib-$${VSVER}.lib )
    } else {
        unix:!macx: IS_LINUX = True
        # library name
        LDLIB_LIB        = -lLDraw-osmesa
        LDEXPORTER_LIB   = -lLDExporter-osmesa
        LDLOADER_LIB     = -lLDLoader-osmesa
        TRE_LIB          = -lTRE-osmesa
        TCFOUNDATION_LIB = -lTCFoundation-osmesa

        GL2PS_LIB        = -lgl2ps
        TINYXML_LIB      = -ltinyxml
        3DS_LIB          = -l3ds
        macx:ZIP_LIB     = -lminizip
        else:ZIP_LIB     =
        PNG_LIB          = -lpng16
        JPEG_LIB         = -ljpeg

        # source paths
        LDLIB_SRC        = $$system_path( $${LDVLIBDIR}/libLDraw-osmesa.a )
        LDEXPORTER_SRC   = $$system_path( $${LDVLIBDIR}/libLDExporter-osmesa.a )
        LDLOADER_SRC     = $$system_path( $${LDVLIBDIR}/libLDLoader-osmesa.a )
        TRE_SRC          = $$system_path( $${LDVLIBDIR}/libTRE-osmesa.a )
        TCFOUNDATION_SRC = $$system_path( $${LDVLIBDIR}/libTCFoundation-osmesa.a )

        GL2PS_SRC        = $$system_path( $${LDVLIBDIR}/libgl2ps.a )
        TINYXML_SRC      = $$system_path( $${LDVLIBDIR}/libtinyxml.a )
        3DS_SRC          = $$system_path( $${LDVLIBDIR}/lib3ds.a )
        PNG_SRC          = $$system_path( $${LDVLIBDIR}/libpng16.a )
        JPEG_SRC         = $$system_path( $${LDVLIBDIR}/libjpeg.a )
        MINIZIP_SRC      =
    }

    # Set 'use local' flags
    !exists($${GL2PS_SRC}) {
        USE_LOCAL_GL2PS_LIB = False
        # message("~~~ GL2PS LIBRARY $${GL2PS_SRC} NOT FOUND ~~~")
    } else:message("~~~ GL2PS LIBRARY $${GL2PS_SRC} FOUND ~~~")

    !exists($${TINYXML_SRC}) {
        USE_LOCAL_TINYXML_LIB = False
        # message("~~~ TINYXML LIBRARY $${TINYXML_SRC} NOT FOUND ~~~")
    } else:message("~~~ TINYXML LIBRARY $${TINYXML_SRC} FOUND ~~~")

    !exists($${3DS_SRC}) {
        USE_LOCAL_3DS_LIB = False
        # message("~~~ 3DS LIBRARY $${3DS_SRC} NOT FOUND ~~~")
    } else:message("~~~ 3DS LIBRARY $${3DS_SRC} FOUND ~~~")

    !exists($${PNG_SRC}) {
        USE_LOCAL_PNG_LIB = False
        # message("~~~ PNG LIBRARY $${PNG_SRC} NOT FOUND ~~~")
    } else:message("~~~ PNG LIBRARY $${PNG_SRC} FOUND ~~~")

    !exists($${JPEG_SRC}) {
        USE_LOCAL_JPEG_LIB = False
        # message("~~~ JPEG LIBRARY $${JPEG_SRC} NOT FOUND ~~~")
    } else:message("~~~ JPEG LIBRARY $${JPEG_SRC} FOUND ~~~")

    win32-msvc* {
        !exists($${ZLIB_SRC}) {
            USE_LOCAL_ZLIB_LIB = False
            # message("~~~ Z LIBRARY $${ZLIB_SRC} NOT FOUND ~~~")
        } else:message("~~~ Z LIBRARY $${ZLIB_SRC} FOUND ~~~")
    } else {
        USE_LOCAL_MINIZIP_LIB = False
    }

    # Copy libraries from LDView (Disabled)
    contains(COPY_LDV_LIBS,True) {
        message("~~~ ENABLE COPY LDVIEW LIBRARIES TO: $$system_path( $$LDVLIBRARY ) ~~~ ")

        win32-msvc* {
            # destination paths
            LDLIB_DST        = $$system_path( $${LDVLIBRARY}/LDLib.lib )
            LDEXPORTER_DST   = $$system_path( $${LDVLIBRARY}/LDExporter.lib )
            LDLOADER_DST     = $$system_path( $${LDVLIBRARY}/LDLoader.lib )
            TRE_DST          = $$system_path( $${LDVLIBRARY}/TRE.lib )
            TCFOUNDATION_DST = $$system_path( $${LDVLIBRARY}/TCFoundation.lib )

            GL2PS_DST        = $$system_path( $${LDVLIBRARY}/gl2ps.lib )
            TINYXML_DST      = $$system_path( $${LDVLIBRARY}/tinyxml_STL.lib )
            3DS_DST          = $$system_path( $${LDVLIBRARY}/lib3ds-$${VSVER}.lib )
            MINIZIP_DST      = $$system_path( $${LDVLIBRARY}/unzip32-$${VSVER}.lib )
            PNG_DST          = $$system_path( $${LDVLIBRARY}/libpng16-$${VSVER}.lib )
            JPEG_DST         = $$system_path( $${LDVLIBRARY}/libjpeg-$${VSVER}.lib )
            ZLIB_DST         = $$system_path( $${LDVLIBRARY}/zlib-$${VSVER}.lib )
        } else {
            # destination paths
            LDLIB_DST        = $$system_path( $${LDVLIBRARY}/libLDraw-osmesa.a )
            LDEXPORTER_DST   = $$system_path( $${LDVLIBRARY}/libLDExporter-osmesa.a )
            LDLOADER_DST     = $$system_path( $${LDVLIBRARY}/libLDLoader-osmesa.a )
            TRE_DST          = $$system_path( $${LDVLIBRARY}/libTRE-osmesa.a )
            TCFOUNDATION_DST = $$system_path( $${LDVLIBRARY}/libTCFoundation-osmesa.a )

            GL2PS_DST        = $$system_path( $${LDVLIBRARY}/libgl2ps.a )
            TINYXML_DST      = $$system_path( $${LDVLIBRARY}/libtinyxml.a )
            3DS_DST          = $$system_path( $${LDVLIBRARY}/lib3ds.a )
            macx:ZIP_LIB     = $$system_path( $${LDVLIBRARY}/libminizip.a )
            else:ZIP_LIB     =
            PNG_DST          = $$system_path( $${LDVLIBRARY}/libpng16.a )
            JPEG_DST         = $$system_path( $${LDVLIBRARY}/libjpeg.a )
        }

        LDLIB_LIB_cmd             = $$COPY_CMD $${LDLIB_SRC} $${LDVLIBRARY}
        LDEXPORTER_LIB_cmd        = $$COPY_CMD $${LDEXPORTER_SRC} $${LDVLIBRARY}
        LDLOADER_LIB_cmd          = $$COPY_CMD $${LDLOADER_SRC} $${LDVLIBRARY}
        TRE_LIB_cmd               = $$COPY_CMD $${TRE_SRC} $${LDVLIBRARY}
        TCFOUNDATION_LIB_cmd      = $$COPY_CMD $${TCFOUNDATION_SRC} $${LDVLIBRARY}

        TINYXML_LIB_cmd           = $$COPY_CMD $${TINYXML_SRC} $${LDVLIBRARY}
        GL2PS_LIB_cmd             = $$COPY_CMD $${GL2PS_SRC} $${LDVLIBRARY}
        3DS_LIB_cmd               = $$COPY_CMD $${3DS_SRC} $${LDVLIBRARY}
        MINIZIP_LIB_cmd           = $$COPY_CMD $${MINIZIP_SRC} $${LDVLIBRARY}
        PNG_LIB_cmd               = $$COPY_CMD $${PNG_SRC} $${LDVLIBRARY}
        JPEG_LIB_cmd              = $$COPY_CMD $${JPEG_SRC} $${LDVLIBRARY}
        ZLIB_LIB_cmd              = $$COPY_CMD $${ZLIB_SRC} $${LDVLIBRARY}

        LDLib_lib.target          = $$LDLIB_DST
        LDLib_lib.commands        = $$LDLIB_LIB_cmd
        LDLib_lib.depends         = LDLib_lib_msg LDExporter_lib
        LDLib_lib_msg.commands    = @echo Copying LDLib library...

        LDExporter_lib.target     = $$LDEXPORTER_DST
        LDExporter_lib.commands   = $$LDEXPORTER_LIB_cmd
        LDExporter_lib.depends    = LDExporter_lib_msg LDLoader_lib
        LDExporter_lib_msg.commands = @echo Copying LDExporter library...

        LDLoader_lib.target       = $$LDLOADER_DST
        LDLoader_lib.commands     = $$LDLOADER_LIB_cmd
        LDLoader_lib.depends      = LDLoader_lib_msg TRE_lib
        LDLoader_lib_msg.commands = @echo Copying LDLoader library...

        TRE_lib.target            = $$TRE_DST
        TRE_lib.commands          = $$TRE_LIB_cmd
        TRE_lib.depends           = TRE_lib_msg TCFoundation_lib
        TRE_lib_msg.commands      = @echo Copying TRE library...

        TCFoundation_lib.target   = $$TCFOUNDATION_DST
        TCFoundation_lib.commands = $$TCFOUNDATION_LIB_cmd
        TCFoundation_lib.depends  = TCFoundation_lib_msg
        TCFoundation_lib_msg.commands = @echo Copying TCFoundation library...

        QMAKE_EXTRA_TARGETS += \
            LDLib_lib LDLib_lib_msg \
            LDExporter_lib LDExporter_lib_msg \
            LDLoader_lib LDLoader_lib_msg \
            TRE_lib TRE_lib_msg \
            TCFoundation_lib TCFoundation_lib_msg

        if (!contains(USE_LOCAL_TINYXML_LIB,False)) {
            Tinyxml_lib.target       = $$TINYXML_DST
            Tinyxml_lib.commands     = $$TINYXML_LIB_cmd
            Tinyxml_lib.depends      = Tinyxml_lib_msg
            Tinyxml_lib_msg.commands = @echo Copying Tinyxml library...
            QMAKE_EXTRA_TARGETS     += Tinyxml_lib Tinyxml_lib_msg
        }

        if (!contains(USE_LOCAL_GL2PS_LIB,False)) {
            Gl2ps_lib.target       = $$GL2PS_DST
            Gl2ps_lib.commands     = $$GL2PS_LIB_cmd
            Gl2ps_lib.depends      = Gl2ps_lib_msg
            Gl2ps_lib_msg.commands = @echo Copying Gl2ps library...
            QMAKE_EXTRA_TARGETS   += Gl2ps_lib Gl2ps_lib_msg
        }

        if (!contains(USE_LOCAL_3DS_LIB,False)) {
            3ds_lib.target         = $$3DS_DST
            3ds_lib.commands       = $$3DS_LIB_cmd
            3ds_lib.depends        = 3ds_lib_msg
            3ds_lib_msg.commands   = @echo Copying 3ds library...
            QMAKE_EXTRA_TARGETS   += 3ds_lib 3ds_lib_msg
        }

        if (!contains(USE_LOCAL_MINIZIP_LIB,False):!contains(IS_LINUX,True)) {
            Minizip_lib.target       = $$MINIZIP_DST
            Minizip_lib.commands     = $$MINIZIP_LIB_cmd
            Minizip_lib.depends      = Minizip_lib_msg
            Minizip_lib_msg.commands = @echo Copying Minizip library...
            QMAKE_EXTRA_TARGETS     += Minizip_lib Minizip_lib_msg
        }

        if (!contains(USE_LOCAL_PNG_LIB,False)) {
            Png_lib.target       = $$PNG_DST
            Png_lib.commands     = $$PNG_LIB_cmd
            Png_lib.depends      = Png_lib_msg
            Png_lib_msg.commands = @echo Copying Png library...
            QMAKE_EXTRA_TARGETS += Png_lib Png_lib_msg
        }

        if (!contains(USE_LOCAL_JPEG_LIB,False)) {
            Jpeg_lib.target       = $$JPEG_DST
            Jpeg_lib.commands     = $$JPEG_LIB_cmd
            Jpeg_lib.depends      = Jpeg_lib_msg
            Jpeg_lib_msg.commands = @echo Copying Jpeg library...
            QMAKE_EXTRA_TARGETS  += Jpeg_lib Jpeg_lib_msg
        }

        if (!contains(USE_LOCAL_ZLIB_LIB,False)) {
            Zlib_lib.target       = $$ZLIB_DST
            Zlib_lib.commands     = $$ZLIB_LIB_cmd
            Zlib_lib.depends      = Zlib_lib_msg
            Zlib_lib_msg.commands = @echo Copying Zlib library...
            QMAKE_EXTRA_TARGETS  += Zlib_lib Zlib_lib_msg
        }

        PRE_TARGETDEPS += \
            $${LDLIB_DST} \
            $${LDEXPORTER_DST} \
            $${LDLOADER_DST} \
            $${TRE_DST} \
            $${TCFOUNDATION_DST}

        QMAKE_CLEAN += \
            $${LDLIB_DST} \
            $${LDEXPORTER_DST} \
            $${LDLOADER_DST} \
            $${TRE_DST} \
            $${TCFOUNDATION_DST}
    } else {
        message("~~~ ADD LDVIEW LIBRARIES PATH: $$system_path( $$LDVLIBDIR ) ~~~ ")
        LIBS        += -L$${LDVLIBDIR}
    }
    # End copy libraries from LDView

    LIBS += \
        $${LDLIB_LIB} \
        $${LDEXPORTER_LIB} \
        $${LDLOADER_LIB} \
        $${TRE_LIB} \
        $${TCFOUNDATION_LIB}

    # 3rd party libraries should come after main libraries

    if (contains(USE_LOCAL_GL2PS_LIB,False)) {
        macx:LIBS          += /usr/local/lib/libgl2ps.dylib
        else:LIBS          += -lgl2ps
    } else {
        LIBS               += $$GL2PS_LIB
        contains(COPY_LDV_LIBS,True) {
            PRE_TARGETDEPS += $$GL2PS_DST
            QMAKE_CLEAN    += $$GL2PS_DST
        }
    }

    if (contains(USE_LOCAL_TINYXML_LIB,False)) {
        macx:LIBS          += /usr/local/lib/libtinyxml.dylib
        else:LIBS          += -ltinyxml
    } else {
        LIBS               += $$TINYXML_LIB
        contains(COPY_LDV_LIBS,True) {
            PRE_TARGETDEPS += $$TINYXML_DST
            QMAKE_CLEAN    += $$TINYXML_DST
        }
    }

    if (contains(USE_LOCAL_PNG_LIB,False)) {
        macx:LIBS          += /usr/local/lib/libpng.dylib
        else:LIBS          += -lpng
    } else {
        LIBS               += $$PNG_LIB
        contains(COPY_LDV_LIBS,True) {
            PRE_TARGETDEPS += $$PNG_DST
            QMAKE_CLEAN    += $$PNG_DST
        }
    }

    if (contains(USE_LOCAL_JPEG_LIB,False)) {
        macx:LIBS          += /usr/local/lib/libjpeg.dylib
        else:LIBS          += -ljpeg
    } else {
        LIBS               += $$JPEG_LIB
        contains(COPY_LDV_LIBS,True) {
            PRE_TARGETDEPS += $$JPEG_DST
            QMAKE_CLEAN    += $$JPEG_DST
        }
    }

    if (contains(USE_LOCAL_MINIZIP_LIB,False)) {
        macx:LIBS          += /usr/local/lib/libminizip.dylib
    } else:!contains(IS_LINUX,True) {
        LIBS               += $$ZIP_LIB
        contains(COPY_LDV_LIBS,True) {
            PRE_TARGETDEPS += $$MINIZIP_DST
            QMAKE_CLEAN    += $$MINIZIP_DST
        }
    }

    if (contains(USE_LOCAL_3DS_LIB,False)) {
        macx:LIBS          += /usr/local/lib/lib3ds.dylib
        else:LIBS          += -l3ds
    } else {
        LIBS               += $$3DS_LIB
        contains(COPY_LDV_LIBS,True) {
            PRE_TARGETDEPS += $$3DS_DST
            QMAKE_CLEAN    += $$3DS_DST
        }
    }

    if (!contains(USE_LOCAL_ZLIB_LIB,False)) {
        LIBS               += $$ZLIB_LIB
        contains(COPY_LDV_LIBS,True) {
            PRE_TARGETDEPS += $$ZLIB_DST
            QMAKE_CLEAN    += $$ZLIB_DST
        }
    }

#~~ Merge ldv messages ini files and move to extras dir ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    isEmpty(LDVMESSAGESINI_DEP): \
    LDVMESSAGESINI_DEP = $$system_path( $$absolute_path( $$_PRO_FILE_PWD_/extras/$$LDVMESSAGESINI ) )
    LDVRESDIR          = $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/resources )

#    message("~~~ lib$${TARGET} Messages ini path: $$system_path( $$LDVMESSAGESINI_DEP ) ~~~ ")
#    message("~~~ lib$${TARGET} Messages ini source: $$system_path( $$LDVRESDIR ) ~~~ ")

    win32 {
        PLUS_CMD      = +
        REDIRECT_CMD  =
    } else:unix {
        COPY_CMD      = cat
        PLUS_CMD      =
        REDIRECT_CMD  = >
    }

    LDVMSGINI_COPY_CMD = \
    $$COPY_CMD \
    $$system_path( $${LDVRESDIR}/LDViewMessages.ini ) $$PLUS_CMD \
    $$system_path( $${LDVRESDIR}/LDExportMessages.ini ) $$PLUS_CMD \
    $$system_path( $$PWD/LDVWidgetMessages.ini ) $$REDIRECT_CMD \
    $$system_path( $$LDVMESSAGESINI_DEP )
    # When compiling from QtCreator, add ldvMessages.ini to destination directory extras folder - except for macOS
    contains(DEVL_LDV_MESSAGES_INI,True) {
        LDVMSGINI_COPY_CMD += \
        $$escape_expand(\n\t) \
        $$COPY_CMD \
        $$system_path( $$LDVMESSAGESINI_DEP $$OUT_PWD/$$DESTDIR/extras/ )
    }

    ldvmsg_copy.target       = $$LDVMESSAGESINI_DEP
    ldvmsg_copy.commands     = $$LDVMSGINI_COPY_CMD
    ldvmsg_copy.depends      = $${LDVRESDIR}/LDViewMessages.ini \
                               $${LDVRESDIR}/LDExportMessages.ini \
                               $$PWD/LDVWidgetMessages.ini \
                               ldvmsg_copy_msg
    ldvmsg_copy_msg.commands = @echo Creating $${LDVMESSAGESINI}...

    QMAKE_EXTRA_TARGETS += ldvmsg_copy ldvmsg_copy_msg
    PRE_TARGETDEPS      += $$LDVMESSAGESINI_DEP
    QMAKE_CLEAN         += $$LDVMESSAGESINI_DEP
}
