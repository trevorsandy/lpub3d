win32 {
    BUILD_WORKER_VERSION = $$(LP3D_VSVERSION)
    isEmpty(BUILD_WORKER_VERSION): BUILD_WORKER_VERSION = 2019
    !contains(LOAD_LDV_LIBS,True): \
    message("~~~ lib$${TARGET} build worker: Visual Studio $$BUILD_WORKER_VERSION ~~~")
    equals(BUILD_WORKER_VERSION, 2019) {
        contains(QT_ARCH,i386): VSVER=vs2017
        else: VSVER=vs2019
    } else {
        VSVER=vs2015
    }
    !contains(LOAD_LDV_LIBS,True): \
    message("~~~ lib$${TARGET} $$upper($$QT_ARCH) MSVS library version: $$VSVER ~~~")

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

    VER_LDVIEW = ldview-4.5
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # Header soruce path
    equals(VER_USE_LDVIEW_DEV,True) {
        isEmpty(LDVHDRDIR):LDVHDRDIR = $$system_path( $${VER_LDVIEW_DEV_REPOSITORY}/include )
        isEmpty(LDV3RDHDRDIR):LDV3RDHDRDIR = $$system_path( $$LDVHDRDIR )
        isEmpty(LDV3RDHDR):LDV3RDHDR = $$system_path( $${VER_LDVIEW_DEV_REPOSITORY}/3rdParty)
    } else {
        isEmpty(LDVHDRDIR):LDVHDRDIR = $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/include )
        isEmpty(LDV3RDHDRDIR):LDV3RDHDRDIR = $$system_path( $${LDVHDRDIR}/3rdParty )
        isEmpty(LDV3RDHDR):LDV3RDHDR = $$system_path( $${LDV3RDHDRDIR} )
    }

    message("~~~ ADD LDVIEW 3RDPARTY HEADERS TO INCLUDEPATH: $$system_path( $$LDVHDRDIR ) ~~~ ")

    DEPENDPATH  += $${LDVHDRDIR}
    INCLUDEPATH += $${LDVHDRDIR}
    INCLUDEPATH += $${LDV3RDHDRDIR}
    if (contains(UNIX_ARM_BUILD_ARCH,True)) {
        if(!contains(DEFINES,ARM_SKIP_GL_HEADERS)) {
            system( mkdir -p ./include/GL && touch ./include/GL/glext.h )
            INCLUDEPATH += $$system_path( ./include/GL )
            message("~~~ ADD FAKE LDVIEW GLEXT HEADER TO INCLUDEPATH: $$system_path( ./include/GL ) ~~~ ")
        }
        exists ($$system_path( $${LDVHDRDIR}/GL )): \
        system(rm -rf $$system_path( $${LDVHDRDIR}/GL ))
        !exists ($$system_path( $${LDVHDRDIR}/GL )): \
        message("~~~ REMOVED LDVIEW GL HEADERS FOR $$upper($$QT_ARCH) BUILD ~~~ ")
        message("~~~ USING SYSTEM GL HEADERS FOR $$upper($$QT_ARCH) BUILD ~~~ ")
    } else {
        if (!contains(SKIP_LDV_GL_HEADERS,True)) {
            INCLUDEPATH += $$system_path( $${LDVHDRDIR}/GL )
            message("~~~ ADD LDVIEW GL HEADERS TO INCLUDEPATH: $$system_path( $$LDVHDRDIR/GL ) ~~~ ")
        }
    }

    if (unix:exists(/usr/include/tinyxml.h)|exists($${SYSTEM_PREFIX}/include/tinyxml.h)) {
        message("~~~ lib$${TARGET} system library tinyxml found ~~~")
    } else:exists($${LDV3RDHDRDIR}/tinyxml.h) {
        message("~~~ lib$${TARGET} local library header for tinyxml found ~~~")
    } else {
        message("~~~ ERROR: Library header for tinyxml not found ~~~")
    }
    if (unix:exists(/usr/include/gl2ps.h)|exists($${SYSTEM_PREFIX}/include/gl2ps.h)) {
        message("~~~ lib$${TARGET} system library gl2ps found ~~~")
    } else:exists($${LDV3RDHDRDIR}/gl2ps.h) {
        message("~~~ lib$${TARGET} local library header for gl2ps found ~~~")
    } else {
        message("~~~ ERROR: Library header for gl2ps not found, using local ~~~")
    }
    if (unix:exists(/usr/include/lib3ds.h)|exists($${SYSTEM_PREFIX}/include/lib3ds.h)){
        message("~~~ lib$${TARGET} system library 3ds found ~~~")
    } else:exists($${LDV3RDHDRDIR}/lib3ds.h) {
        message("~~~ lib$${TARGET} local library header for 3ds found ~~~")
    } else {
        message("~~~ ERROR: Library header for 3ds not found ~~~")
    }
    if (unix:macx:exists(/usr/include/minizip/unzip.h)|exists($${SYSTEM_PREFIX}/include/minizip/unzip.h)) {
        message("~~~ lib$${TARGET} system library minizip found ~~~")
        DEFINES += HAVE_MINIZIP
    } else:exists($${LDV3RDHDR}/minizip/zip.h) {
        message("~~~ lib$${TARGET} local library header for minizip found ~~~")
        INCLUDEPATH += $${LDV3RDHDR}
        DEFINES += HAVE_MINIZIP
    } else {
        message("~~~ ERROR: Library header for minizip not found ~~~")
    }
    if (unix:exists(/usr/include/png.h)|exists($${SYSTEM_PREFIX}/include/png.h)) {
        message("~~~ lib$${TARGET} system library png found ~~~")
    } else:exists($${LDV3RDHDRDIR}/png.h) {
        message("~~~ lib$${TARGET} local library header for png found ~~~")
    } else {
        message("~~~ ERROR: Library header for png not found ~~~")
    }
    if (unix:exists(/usr/include/jpeglib.h)|exists($${SYSTEM_PREFIX}/include/jpeglib.h)) {
        message("~~~ lib$${TARGET} system library jpeg found ~~~")
    } else:exists($${LDV3RDHDRDIR}/gl2ps.h) {
        message("~~~ lib$${TARGET} local library header for jpeg found ~~~")
    } else {
        message("~~~ ERROR: Library header for jpeg not found ~~~")
    }
} # LOAD_LDV_HEADERS,True

# This block is executed by LPub3D mainApp to enable linking the LDVlib
contains(LOAD_LDV_LIBS,True) {
    win32-msvc*:CONFIG(debug, debug|release) {
        equals(VER_USE_LDVIEW_DEV,True) {
            LDVLIBDIR = $$system_path( $${VER_LDVIEW_DEV_REPOSITORY}/Build/Debug$$LIB_ARCH )
            LDV3RDLIBDIR = $$system_path( $${VER_LDVIEW_DEV_REPOSITORY}/lib )
        } else {
            LDVLIBDIR = $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/Build/Debug$$LIB_ARCH )
            LDV3RDLIBDIR = $$system_path( $$LDVLIBDIR )
        }
    } else {
        LDVLIBDIR = $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/bin/$$QT_ARCH )
        LDV3RDLIBDIR = $$system_path( $$LDVLIBDIR )
    }

    message("~~~ ADD LDVIEW LIBRARIES PATH TO LIBS: $$system_path( $$LDVLIBDIR ) ~~~ ")
    LIBS        += -L$${LDVLIBDIR}

    message("~~~ ADD LDVIEW 3RDPARTY LIBRARIES PATH TO LIBS: $$system_path( $$LDV3RDLIBDIR ) ~~~ ")
    LIBS        += -L$${LDV3RDLIBDIR}

#    isEmpty(LDVLIBRARY):LDVLIBRARY = $$system_path( $$absolute_path( $$OUT_PWD/../ldvlib/LDVQt/$$DESTDIR ) )
#    message("~~~ lib$${TARGET} Library path: $$LDVLIBRARY ~~~ ")

    # Set library names, source paths and local paths
    win32-msvc* {
        # library name
        LDLIB_LIB        = -lLDLib
        LDEXPORTER_LIB   = -lLDExporter
        LDLOADER_LIB     = -lLDLoader
        TRE_LIB          = -lTRE
        TCFOUNDATION_LIB = -lTCFoundation

        GL2PS_LIB        = -lgl2ps
        TINYXML_LIB      = -ltinyxml_STL
        ZIP_LIB          = -lminizip
        3DS_LIB          = -llib3ds-$${VSVER}
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
        MINIZIP_SRC      = $$system_path( $${LDVLIBDIR}/minizip.lib )
        3DS_SRC          = $$system_path( $${LDV3RDLIBDIR}/lib3ds-$${VSVER}.lib )
        PNG_SRC          = $$system_path( $${LDV3RDLIBDIR}/libpng16-$${VSVER}.lib )
        JPEG_SRC         = $$system_path( $${LDV3RDLIBDIR}/libjpeg-$${VSVER}.lib )
        ZLIB_SRC         = $$system_path( $${LDV3RDLIBDIR}/zlib-$${VSVER}.lib )
    } else {
        # library name
        LDLIB_LIB        = -lLDraw-osmesa
        LDEXPORTER_LIB   = -lLDExporter-osmesa
        LDLOADER_LIB     = -lLDLoader-osmesa
        TRE_LIB          = -lTRE-osmesa
        TCFOUNDATION_LIB = -lTCFoundation-osmesa

        GL2PS_LIB        = -lgl2ps
        TINYXML_LIB      = -ltinyxml
        3DS_LIB          = -l3ds
        ZIP_LIB          = -lminizip
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
        MINIZIP_SRC      = $$system_path( $${LDVLIBDIR}/libminizip.a )
        3DS_SRC          = $$system_path( $${LDV3RDLIBDIR}/lib3ds.a )
        PNG_SRC          = $$system_path( $${LDV3RDLIBDIR}/libpng16.a )
        JPEG_SRC         = $$system_path( $${LDV3RDLIBDIR}/libjpeg.a )
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

    !exists($${MINIZIP_SRC}) {
        USE_LOCAL_MINIZIP_LIB = False
        # message("~~~ MINIZIP LIBRARY $${MINIZIP_SRC} NOT FOUND ~~~")
    } else: message("~~~ MINIZIP LIBRARY $${MINIZIP_SRC} FOUND ~~~")

    win32-msvc* {
        !exists($${ZLIB_SRC}) {
            USE_LOCAL_ZLIB_LIB = False
            # message("~~~ Z LIBRARY $${ZLIB_SRC} NOT FOUND ~~~")
        } else:message("~~~ Z LIBRARY $${ZLIB_SRC} FOUND ~~~")
    }

    LIBS += \
        $${LDLIB_LIB} \
        $${LDEXPORTER_LIB} \
        $${LDLOADER_LIB} \
        $${TRE_LIB} \
        $${TCFOUNDATION_LIB}

    # 3rd party libraries should come after main libraries

    if (contains(USE_LOCAL_GL2PS_LIB,False)) {
        macx:LIBS          += $${SYSTEM_PREFIX}/lib/libgl2ps.dylib
        else:LIBS          += -lgl2ps
    } else {
        LIBS               += $$GL2PS_LIB
    }

    if (contains(USE_LOCAL_TINYXML_LIB,False)) {
        macx:LIBS          += $${SYSTEM_PREFIX}/lib/libtinyxml.dylib
        else:LIBS          += -ltinyxml
    } else {
        LIBS               += $$TINYXML_LIB
    }

    if (contains(USE_LOCAL_PNG_LIB,False)) {
        macx:LIBS          += $${SYSTEM_PREFIX}/lib/libpng.dylib
        else:LIBS          += -lpng
    } else {
        LIBS               += $$PNG_LIB
    }

    if (contains(USE_LOCAL_JPEG_LIB,False)) {
        macx:LIBS          += $${SYSTEM_PREFIX}/lib/libjpeg.dylib
        else:LIBS          += -ljpeg
    } else {
        LIBS               += $$JPEG_LIB
    }

    if (contains(USE_LOCAL_MINIZIP_LIB,False)) {
        macx:LIBS          += $${SYSTEM_PREFIX}/lib/libminizip.dylib
        else:LIBS          += -lminizip
    } else {
        LIBS               += $$ZIP_LIB
    }

    if (contains(USE_LOCAL_3DS_LIB,False)) {
        macx:LIBS          += $${SYSTEM_PREFIX}/lib/lib3ds.dylib
        else:LIBS          += -l3ds
    } else {
        LIBS               += $$3DS_LIB
    }

    if (!contains(USE_LOCAL_ZLIB_LIB,False)) {
        LIBS               += $$ZLIB_LIB
    }

    # message("~~~ INFO - LDVQt LIBRARIES: $${LIBS} ~~~")

#~~ Merge ldv messages ini files and move to extras dir ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    win32 {
        PLUS_CMD     = +
        REDIRECT_CMD =
    } else:unix {
        COPY_CMD     = cat
        PLUS_CMD     =
        REDIRECT_CMD = >
    }

    #LDVMESSAGESINI defined in mainApp.pro
    isEmpty(LDVMESSAGESINI_DEP): \
    LDVMESSAGESINI_DEP = $$system_path( $$absolute_path( $$_PRO_FILE_PWD_/extras/$$LDVMESSAGESINI ) )
    LDVRESDIR          = $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/resources )
    #message("~~~ lib$${TARGET} Messages ini path: $$system_path( $$LDVMESSAGESINI_DEP ) ~~~ ")
    #message("~~~ lib$${TARGET} Messages ini source: $$system_path( $$LDVRESDIR ) ~~~ ")
    LDVMSGINI_COPY_CMD = \
    $$COPY_CMD \
    $$system_path( $${LDVRESDIR}/LDViewMessages.ini ) $$PLUS_CMD \
    $$system_path( $${LDVRESDIR}/LDExportMessages.ini ) $$PLUS_CMD \
    $$system_path( $$PWD/LDVWidgetMessages.ini ) $$REDIRECT_CMD \
    $$system_path( $$LDVMESSAGESINI_DEP )
    # When compiling from QtCreator, add ldvMessages.ini to destination directory extras folder - except for macOS
    contains(DEVL_LDV_MESSAGES_INI,True) {
        unix:COPY_CMD = cp -f
        LDVMESSAGESINI_DEVL = $$system_path( $$OUT_PWD/$$DESTDIR/extras/$$LDVMESSAGESINI )
        message("~~~ COPY LDVMESSAGES.INI TO: $$LDVMESSAGESINI_DEVL) ~~~ ")
        LDVMSGINI_COPY_CMD += \
        $$escape_expand(\n\t) \
        $$COPY_CMD \
        $$system_path( $$LDVMESSAGESINI_DEP $$LDVMESSAGESINI_DEVL )
    }

    ldvmsg_copy.target       = $$LDVMESSAGESINI_DEP
    ldvmsg_copy.commands     = $$LDVMSGINI_COPY_CMD
    ldvmsg_copy.depends      = $${LDVRESDIR}/LDViewMessages.ini \
                               $${LDVRESDIR}/LDExportMessages.ini \
                               $$PWD/LDVWidgetMessages.ini \
                               ldvmsg_copy_msg
    ldvmsg_copy_msg.commands = @echo Project MESSAGE: ~~~ Creating $${TARGET} $${LDVMESSAGESINI}... ~~~

    QMAKE_EXTRA_TARGETS += ldvmsg_copy ldvmsg_copy_msg
    PRE_TARGETDEPS      += $$LDVMESSAGESINI_DEP
    QMAKE_CLEAN         += $$LDVMESSAGESINI_DEP
} # LOAD_LDV_LIBS,True
