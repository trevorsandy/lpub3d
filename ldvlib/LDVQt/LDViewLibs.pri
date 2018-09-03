# Discard STDERR so get path to NULL device
win32: NULL_DEVICE = NUL # Windows doesn't have /dev/null but has NUL
else:  NULL_DEVICE = /dev/null

# LDView source path components
win32 {
    COPY_CMD = COPY /y /a /b
} else:unix {
    COPY_CMD = cp -f
}

contains(LOAD_LDVHEADERS,True) {
    isEmpty(LDVINCLUDE):LDVINCLUDE = $$system_path( $$absolute_path( $$PWD/include ) )

    VER_LDVIEW                = ldview-4.3
    unix:!macx: DIST_DIR      = lpub3d_linux_3rdparty
    else:macx:  DIST_DIR      = lpub3d_macos_3rdparty
    else:win32: DIST_DIR      = lpub3d_windows_3rdparty
    THIRD_PARTY_DIST_DIR_PATH = $$(LP3D_DIST_DIR_PATH)
    !exists($$THIRD_PARTY_DIST_DIR_PATH): \
    THIRD_PARTY_DIST_DIR_PATH = $$system_path( $$absolute_path( $$PWD/../../../$$DIST_DIR ) )
    win32 {
        DELETE_HDR_DIRS_CMD = IF EXIST $$LDVINCLUDE RMDIR /S /Q $$LDVINCLUDE
        CREATE_HDR_DIRS_CMD = MKDIR $$LDVINCLUDE\LDLib $$LDVINCLUDE\LDExporter $$LDVINCLUDE\LDLoader $$LDVINCLUDE\TRE $$LDVINCLUDE\TCFoundation $$LDVINCLUDE\3rdParty $$LDVINCLUDE\GL
    } else {
        DELETE_HDR_DIRS_CMD = if test -e $$LDVINCLUDE; then rm -rf $$LDVINCLUDE; fi
        CREATE_HDR_DIRS_CMD = mkdir -p $$LDVINCLUDE/LDExporter $$LDVINCLUDE/LDLib \
                              $$LDVINCLUDE/LDLoader $$LDVINCLUDE/TRE $$LDVINCLUDE/TCFoundation \
                              $$LDVINCLUDE/3rdParty $$LDVINCLUDE/GL
    }

    LDVHDRDIR = $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/include )

#    message("~~~ LDVQt Headers path: $$LDVINCLUDE ~~~ ")
#    message("~~~ LDVQt Headers source: $$LDVHDRDIR ~~~ ")

    !exists($$THIRD_PARTY_DIST_DIR_PATH): \
    message("~~~ ERROR - THIRD PARTY DIST DIR $$THIRD_PARTY_DIST_DIR_PATH NOT DETECTED! ~~~")

    # Initialize local header folders
    system( $$DELETE_HDR_DIRS_CMD )
    system( $$CREATE_HDR_DIRS_CMD )

    # Copy headers from LDView LDVHDRDIR
    system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/LDLib/*.h) $$system_path( $${LDVINCLUDE}/LDLib/ ) )
    system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/LDExporter/*.h) $$system_path( $${LDVINCLUDE}/LDExporter/ ) )
    system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/LDLoader/*.h) $$system_path( $${LDVINCLUDE}/LDLoader/) )
    system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/TRE/*.h) $$system_path( $${LDVINCLUDE}/TRE/) )
    system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/TCFoundation/*.h) $$system_path( $${LDVINCLUDE}/TCFoundation/ ) )
    system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/3rdParty/*.h) $$system_path( $${LDVINCLUDE}/3rdParty/ ) )
    if (contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64):!contains(DEFINES,_OPENSUSE_1320_ARM)) {
        message("~~~ $$upper($$QT_ARCH) build - skip local GL content ~~~")
        system( touch $$system_path( $${LDVHDRDIR}/GL/glext.h) )
    } else {
        system( $$COPY_CMD $$system_path( $${LDVHDRDIR}/GL/*.h) $$system_path( $${LDVINCLUDE}/GL/ ) )
    }

    exists($$system_path( $$LDVINCLUDE/TCFoundation/TCObject.h )): \
    message("~~~ LDVQt Headers copied to $${LDVINCLUDE} ~~~")
}

contains(LOAD_LDVLIBS,True) {

    isEmpty(LDVLIBRARY):LDVLIBRARY = $$system_path( $$absolute_path( $$OUT_PWD/../ldvlib/LDVQt/$$DESTDIR ) )
    win32-msvc*:CONFIG(debug, debug|release): \
    LDVLIBDIR = $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/Build/Debug$$LIB_ARCH )
    else:CONFIG(release, debug|release): \
    LDVLIBDIR = $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/bin/$$QT_ARCH )

#    message("~~~ LDVQt Library path: $$LDVLIBRARY ~~~ ")
#    message("~~~ LDVQt Library source: $$LDVLIBDIR ~~~ ")

    # Set source and local paths and library names
    win32-msvc* {
        _LDLIB_DEP        = $$system_path( $${LDVLIBDIR}/LDLib.lib )
        _LDEXPORTER_DEP   = $$system_path( $${LDVLIBDIR}/LDExporter.lib )
        _LDLOADER_DEP     = $$system_path( $${LDVLIBDIR}/LDLoader.lib )
        _TRE_DEP          = $$system_path( $${LDVLIBDIR}/TRE.lib )
        _TCFOUNDATION_DEP = $$system_path( $${LDVLIBDIR}/TCFoundation.lib )

        _GL2PS_DEP        = $$system_path( $${LDVLIBDIR}/gl2ps.lib )
        _TINYXML_DEP      = $$system_path( $${LDVLIBDIR}/tinyxml_STL.lib )
        _3DS_DEP          = $$system_path( $${LDVLIBDIR}/lib3ds-vs2015.lib )
        _MINIZIP_DEP      = $$system_path( $${LDVLIBDIR}/unzip32-vs2015.lib )
        _PNG_DEP          = $$system_path( $${LDVLIBDIR}/libpng16-vs2015.lib )
        _JPEG_DEP         = $$system_path( $${LDVLIBDIR}/libjpeg-vs2015.lib )
        _ZLIB_DEP         = $$system_path( $${LDVLIBDIR}/zlib-vs2015.lib )

        # library name
        LDLIB_LIB         = -lLDLib
        LDEXPORTER_LIB 	  = -lLDExporter
        LDLOADER_LIB      = -lLDLoader
        TRE_LIB           = -lTRE
        TCFOUNDATION_LIB  = -lTCFoundation

        GL2PS_LIB         = -lgl2ps
        TINYXML_LIB       = -ltinyxml_STL
        3DS_LIB           = -llib3ds-vs2015
        ZIP_LIB           = -lunzip32-vs2015
        PNG_LIB           = -llibpng16-vs2015
        JPEG_LIB          = -llibjpeg-vs2015
        ZLIB_LIB          = -lzlib-vs2015

        # local library paths
        LDLIB_DEP         = $$system_path( $${LDVLIBRARY}/LDLib.lib )
        LDEXPORTER_DEP 	  = $$system_path( $${LDVLIBRARY}/LDExporter.lib )
        LDLOADER_DEP      = $$system_path( $${LDVLIBRARY}/LDLoader.lib )
        TRE_DEP           = $$system_path( $${LDVLIBRARY}/TRE.lib )
        TCFOUNDATION_DEP  = $$system_path( $${LDVLIBRARY}/TCFoundation.lib )

        GL2PS_DEP         = $$system_path( $${LDVLIBRARY}/gl2ps.lib )
        TINYXML_DEP       = $$system_path( $${LDVLIBRARY}/tinyxml_STL.lib )
        3DS_DEP           = $$system_path( $${LDVLIBRARY}/lib3ds-vs2015.lib )
        MINIZIP_DEP       = $$system_path( $${LDVLIBRARY}/unzip32-vs2015.lib )
        PNG_DEP           = $$system_path( $${LDVLIBRARY}/libpng16-vs2015.lib )
        JPEG_DEP          = $$system_path( $${LDVLIBRARY}/libjpeg-vs2015.lib )
        ZLIB_DEP          = $$system_path( $${LDVLIBRARY}/zlib-vs2015.lib )
    } else {
        # source paths
        _LDLIB_DEP        = $$system_path( $${LDVLIBDIR}/libLDraw-osmesa.a )
        _LDEXPORTER_DEP   = $$system_path( $${LDVLIBDIR}/libLDExporter-osmesa.a )
        _LDLOADER_DEP     = $$system_path( $${LDVLIBDIR}/libLDLoader-osmesa.a )
        _TRE_DEP          = $$system_path( $${LDVLIBDIR}/libTRE-osmesa.a )
        _TCFOUNDATION_DEP = $$system_path( $${LDVLIBDIR}/libTCFoundation-osmesa.a )

        _GL2PS_DEP        = $$system_path( $${LDVLIBDIR}/libgl2ps.a )
        _TINYXML_DEP      = $$system_path( $${LDVLIBDIR}/libtinyxml.a )
        _3DS_DEP          = $$system_path( $${LDVLIBDIR}/lib3ds.a )
        _PNG_DEP          = $$system_path( $${LDVLIBDIR}/libpng.a )
        _JPEG_DEP         = $$system_path( $${LDVLIBDIR}/libjpeg.a )
        macx {
            _MINIZIP_DEP  = $$system_path( $${LDVLIBDIR}/libminizip.a )
        } else {
            _MINIZIP_DEP  =
            IS_LINUX      = True
        }

        # library name
        LDLIB_LIB        = -lLDraw-osmesa
        LDEXPORTER_LIB 	 = -lLDExporter-osmesa
        LDLOADER_LIB     = -lLDLoader-osmesa
        TRE_LIB          = -lTRE-osmesa
        TCFOUNDATION_LIB = -lTCFoundation-osmesa

        GL2PS_LIB        = -lgl2ps
        TINYXML_LIB      = -ltinyxml
        3DS_LIB          = -l3ds
        macx:ZIP_LIB     = -lminizip
        else:ZIP_LIB     =
        PNG_LIB          = -llibpng
        JPEG_LIB         = -llibjpeg

        # local paths
        LDLIB_DEP        = $$system_path( $${LDVLIBRARY}/libLDraw-osmesa.a )
        LDEXPORTER_DEP 	 = $$system_path( $${LDVLIBRARY}/libLDExporter-osmesa.a )
        LDLOADER_DEP     = $$system_path( $${LDVLIBRARY}/libLDLoader-osmesa.a )
        TRE_DEP          = $$system_path( $${LDVLIBRARY}/libTRE-osmesa.a )
        TCFOUNDATION_DEP = $$system_path( $${LDVLIBRARY}/libTCFoundation-osmesa.a )

        GL2PS_DEP        = $$system_path( $${LDVLIBRARY}/libgl2ps.a )
        TINYXML_DEP      = $$system_path( $${LDVLIBRARY}/libtinyxml.a )
        3DS_DEP          = $$system_path( $${LDVLIBRARY}/lib3ds.a )
        macx:ZIP_LIB     = $$system_path( $${LDVLIBRARY}/libminizip.a )
        else:ZIP_LIB     =
        PNG_DEP          = $$system_path( $${LDVLIBRARY}/libpng.a )
        JPEG_DEP         = $$system_path( $${LDVLIBRARY}/libjpeg.a )
    }

    # Set 'use local' flags
    !exists($${_GL2PS_DEP}): USE_LOCAL_GL2PS_LIB = False
#    else:message("~~~ Local gl2ps library $${_GL2PS_DEP} detected ~~~")

    !exists($${_TINYXML_DEP}): USE_LOCAL_TINYXML_LIB = False
#    else:message("~~~ Local tinyxml library $${_TINYXML_DEP} detected ~~~")

    !exists($${_3DS_DEP}): USE_LOCAL_3DS_LIB = False
#    else:message("~~~ Local 3ds library $${_3DS_DEP} detected ~~~")

    !exists($${_MINIZIP_DEP}): USE_LOCAL_MINIZIP_LIB = False
#    else:message("~~~ Local minizip library $${_MINIZIP_DEP} detected ~~~")

    !exists($${_PNG_DEP}): USE_LOCAL_PNG_LIB = False
#    else:message("~~~ Local png library $${_PNG_DEP} detected ~~~")

    !exists($${_JPEG_DEP}): USE_LOCAL_JPEG_LIB = False
#    else:message("~~~ Local jpeg library $${_JPEG_DEP} detected ~~~")

    !exists($${_ZLIB_DEP}):  USE_LOCAL_ZLIB_LIB = False
#    else:message("~~~ Local z library $${_ZLIB_DEP} detected ~~~")

    # Copy libraries from LDView
    LDLIB_LIB_cmd             = $$COPY_CMD $${_LDLIB_DEP} $${LDVLIBRARY}
    LDEXPORTER_LIB_cmd        = $$COPY_CMD $${_LDEXPORTER_DEP} $${LDVLIBRARY}
    LDLOADER_LIB_cmd          = $$COPY_CMD $${_LDLOADER_DEP} $${LDVLIBRARY}
    TRE_LIB_cmd               = $$COPY_CMD $${_TRE_DEP} $${LDVLIBRARY}
    TCFOUNDATION_LIB_cmd      = $$COPY_CMD $${_TCFOUNDATION_DEP} $${LDVLIBRARY}

    TINYXML_LIB_cmd           = $$COPY_CMD $${_TINYXML_DEP} $${LDVLIBRARY}
    GL2PS_LIB_cmd             = $$COPY_CMD $${_GL2PS_DEP} $${LDVLIBRARY}
    3DS_LIB_cmd               = $$COPY_CMD $${_3DS_DEP} $${LDVLIBRARY}
    MINIZIP_LIB_cmd           = $$COPY_CMD $${_MINIZIP_DEP} $${LDVLIBRARY}
    PNG_LIB_cmd               = $$COPY_CMD $${_PNG_DEP} $${LDVLIBRARY}
    JPEG_LIB_cmd              = $$COPY_CMD $${_JPEG_DEP} $${LDVLIBRARY}
    ZLIB_LIB_cmd              = $$COPY_CMD $${_ZLIB_DEP} $${LDVLIBRARY}

    LDLib_lib.target          = $$LDLIB_DEP
    LDLib_lib.commands        = $$LDLIB_LIB_cmd
    LDLib_lib.depends         = LDLib_lib_msg LDExporter_lib
    LDLib_lib_msg.commands    = @echo Copying LDLib library...

    LDExporter_lib.target     = $$LDEXPORTER_DEP
    LDExporter_lib.commands   = $$LDEXPORTER_LIB_cmd
    LDExporter_lib.depends    = LDExporter_lib_msg LDLoader_lib
    LDExporter_lib_msg.commands = @echo Copying LDExporter library...

    LDLoader_lib.target       = $$LDLOADER_DEP
    LDLoader_lib.commands     = $$LDLOADER_LIB_cmd
    LDLoader_lib.depends      = LDLoader_lib_msg TRE_lib
    LDLoader_lib_msg.commands = @echo Copying LDLoader library...

    TRE_lib.target            = $$TRE_DEP
    TRE_lib.commands          = $$TRE_LIB_cmd
    TRE_lib.depends           = TRE_lib_msg TCFoundation_lib
    TRE_lib_msg.commands      = @echo Copying TRE library...

    TCFoundation_lib.target   = $$TCFOUNDATION_DEP
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
      Tinyxml_lib.target       = $$TINYXML_DEP
      Tinyxml_lib.commands     = $$TINYXML_LIB_cmd
      Tinyxml_lib.depends      = Tinyxml_lib_msg
      Tinyxml_lib_msg.commands = @echo Copying Tinyxml library...
      QMAKE_EXTRA_TARGETS     += Tinyxml_lib Tinyxml_lib_msg
    }

    if (!contains(USE_LOCAL_GL2PS_LIB,False)) {
      Gl2ps_lib.target       = $$GL2PS_DEP
      Gl2ps_lib.commands     = $$GL2PS_LIB_cmd
      Gl2ps_lib.depends      = Gl2ps_lib_msg
      Gl2ps_lib_msg.commands = @echo Copying Gl2ps library...
      QMAKE_EXTRA_TARGETS   += Gl2ps_lib Gl2ps_lib_msg
    }

    if (!contains(USE_LOCAL_3DS_LIB,False)) {
      3ds_lib.target       = $$3DS_DEP
      3ds_lib.commands     = $$3DS_LIB_cmd
      3ds_lib.depends      = 3ds_lib_msg
      3ds_lib_msg.commands = @echo Copying 3ds library...
      QMAKE_EXTRA_TARGETS += 3ds_lib 3ds_lib_msg
    }

    if (!contains(USE_LOCAL_MINIZIP_LIB,False):!contains(IS_LINUX,True)) {
      Minizip_lib.target       = $$MINIZIP_DEP
      Minizip_lib.commands     = $$MINIZIP_LIB_cmd
      Minizip_lib.depends      = Minizip_lib_msg
      Minizip_lib_msg.commands = @echo Copying Minizip library...
      QMAKE_EXTRA_TARGETS     += Minizip_lib Minizip_lib_msg
    }

    if (!contains(USE_LOCAL_PNG_LIB,False)) {
      Png_lib.target       = $$PNG_DEP
      Png_lib.commands     = $$PNG_LIB_cmd
      Png_lib.depends      = Png_lib_msg
      Png_lib_msg.commands = @echo Copying Png library...
      QMAKE_EXTRA_TARGETS += Png_lib Png_lib_msg
    }

    if (!contains(USE_LOCAL_JPEG_LIB,False)) {
      Jpeg_lib.target       = $$JPEG_DEP
      Jpeg_lib.commands     = $$JPEG_LIB_cmd
      Jpeg_lib.depends      = Jpeg_lib_msg
      Jpeg_lib_msg.commands = @echo Copying Jpeg library...
      QMAKE_EXTRA_TARGETS  += Jpeg_lib Jpeg_lib_msg
    }

    if (!contains(USE_LOCAL_ZLIB_LIB,False)) {
      Zlib_lib.target       = $$ZLIB_DEP
      Zlib_lib.commands     = $$ZLIB_LIB_cmd
      Zlib_lib.depends      = Zlib_lib_msg
      Zlib_lib_msg.commands = @echo Copying Zlib library...
      QMAKE_EXTRA_TARGETS  += Zlib_lib Zlib_lib_msg
    }

    LIBS += \
        $${LDLIB_LIB} \
        $${LDEXPORTER_LIB} \
        $${LDLOADER_LIB} \
        $${TRE_LIB} \
        $${TCFOUNDATION_LIB}

    PRE_TARGETDEPS += \
        $${LDLIB_DEP} \
        $${LDEXPORTER_DEP} \
        $${LDLOADER_DEP} \
        $${TRE_DEP} \
        $${TCFOUNDATION_DEP}

    QMAKE_CLEAN += \
        $${LDLIB_DEP} \
        $${LDEXPORTER_DEP} \
        $${LDLOADER_DEP} \
        $${TRE_DEP} \
        $${TCFOUNDATION_DEP}

    # 3rd party libraries should come after main libraries

    if (contains(USE_LOCAL_GL2PS_LIB,False)) {
        macx:LIBS += /usr/local/lib/libgl2ps.dylib
        else:LIBS += -lgl2ps
    } else {
        LIBS           += $$GL2PS_LIB
        PRE_TARGETDEPS += $$GL2PS_DEP
        QMAKE_CLEAN    += $$GL2PS_DEP
    }

    if (contains(USE_LOCAL_TINYXML_LIB,False)) {
        macx:LIBS += /usr/local/lib/libtinyxml.dylib
        else:LIBS += -ltinyxml
    } else {
        LIBS           += $$TINYXML_LIB
        PRE_TARGETDEPS += $$TINYXML_DEP
        QMAKE_CLEAN    += $$TINYXML_DEP
    }

    if (contains(USE_LOCAL_PNG_LIB,False)) {
        if (contains(HOST, Ubuntu):contains(HOST, 14.04.5)) {
            LIBS           += $$PNG_LIB
            PRE_TARGETDEPS += $$PNG_DEP
            QMAKE_CLEAN    += $$PNG_DEP
        } else {
            macx:LIBS += /usr/local/lib/libpng.dylib
            else:LIBS += -lpng
        }
    } else {
        LIBS           += $$PNG_LIB
        PRE_TARGETDEPS += $$PNG_DEP
        QMAKE_CLEAN    += $$PNG_DEP
    }

    if (contains(USE_LOCAL_JPEG_LIB,False)) {
        macx:LIBS += /usr/local/lib/libjpeg.dylib
        else:LIBS += -ljpeg
    } else {
        LIBS           += $$JPEG_LIB
        PRE_TARGETDEPS += $$JPEG_DEP
        QMAKE_CLEAN    += $$JPEG_DEP
    }

    if (contains(USE_LOCAL_MINIZIP_LIB,False)) {
        macx:LIBS += /usr/local/lib/libminizip.dylib
    } else:!contains(IS_LINUX,True) {
        LIBS           += $$ZIP_LIB
        PRE_TARGETDEPS += $$MINIZIP_DEP
        QMAKE_CLEAN    += $$MINIZIP_DEP
    }

    if (contains(USE_LOCAL_3DS_LIB,False)) {
        macx:LIBS += /usr/local/lib/lib3ds.dylib
        else:LIBS += -l3ds
    } else {
        LIBS           += $$3DS_LIB
        PRE_TARGETDEPS += $$3DS_DEP
        QMAKE_CLEAN    += $$3DS_DEP
    }

    if (!contains(USE_LOCAL_ZLIB_LIB,False)) {
        LIBS           += $$ZLIB_LIB
        PRE_TARGETDEPS += $$ZLIB_DEP
        QMAKE_CLEAN    += $$ZLIB_DEP
    }

#~~ Merge ldv messages ini files and move to extras dir ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    isEmpty(LDVMESSAGESINI_DEP): \
    LDVMESSAGESINI_DEP = $$system_path( $$absolute_path( $$_PRO_FILE_PWD_/extras/$$LDVMESSAGESINI ) )
    LDVRESDIR          = $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/resources )

#    message("~~~ LDVQt Messages ini path: $$system_path( $$LDVMESSAGESINI_DEP ) ~~~ ")
#    message("~~~ LDVQt Messages ini source: $$system_path( $$LDVRESDIR ) ~~~ ")

    win32 {
        PLUS_CMD      = +
        REDIRECT_CMD  =
    } else:unix {
        COPY_CMD      = cat
        PLUS_CMD      =
        REDIRECT_CMD  = >
    }

    ldvmsg_copy_cmd = \
    $$COPY_CMD \
    $$system_path( $${LDVRESDIR}/LDViewMessages.ini ) $$PLUS_CMD \
    $$system_path( $${LDVRESDIR}/LDExportMessages.ini ) $$REDIRECT_CMD \
    $$system_path( $$LDVMESSAGESINI_DEP )

    ldvmsg_copy.target         = $$LDVMESSAGESINI_DEP
    ldvmsg_copy.commands       = $$ldvmsg_copy_cmd
    ldvmsg_copy.depends        = $${LDVRESDIR}/LDViewMessages.ini \
                                 $${LDVRESDIR}/LDExportMessages.ini \
                                 ldvmsg_copy_msg
    ldvmsg_copy_msg.commands   = @echo Copying $${LDVMESSAGESINI}...

    QMAKE_EXTRA_TARGETS += ldvmsg_copy ldvmsg_copy_msg
    PRE_TARGETDEPS      += $$LDVMESSAGESINI_DEP
    QMAKE_CLEAN         += $$LDVMESSAGESINI_DEP
}
