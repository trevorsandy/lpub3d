    # These settings are used for package distribution
win32 {

    CONFIG(debug, debug|release) { DIST = debug } else { DIST = release }

    # The Windows package distribution settings below requires a specific dev env configuration.
    # Basically, if you create a projects folder e.g. c:\Users\<user>\Projects and
    # clone the listed GitHub repos under the Projects folder, your dev env will
    # conform to these settings.
    # GitHub clone list:
    # LPub3D           = https://github.com/trevorsandy/lpub3d [Branch: master]
    # 3rdParty Content = https://github.com/trevorsandy/lpub3d_windows_3rdparty [Branch: master]
    #
    # At the end, your Projects folder should look like this:
    # C:\Users\<user>\Projects\
    #                   |
    #                   ---\lpub3d_windows_3rdparty\
    #                   |
    #                   ---\LPub3D\
    #
    # Additionally, when using QtCreator be sure 'Shadow Build' is enabled.
    #
    # source path
    isEmpty(THIRD_PARTY_SRC):THIRD_PARTY_SRC        = $$THIRD_PARTY_DIST_DIR_PATH

    # source executables - 3rd party components
    isEmpty(LDGLITE_INS_EXE):LDGLITE_INS_EXE        = $$THIRD_PARTY_SRC/$$VER_LDGLITE/bin/$$QT_ARCH/LDGLite.exe
    isEmpty(RAYTRACE_INS_EXE):RAYTRACE_INS_EXE      = $$THIRD_PARTY_SRC/$$VER_POVRAY/bin/$$QT_ARCH/lpub3d_trace_cui$$join(ARCH,,,.exe)
    equals (ARCH, 64) {
        isEmpty(LDVIEW_INS_EXE):LDVIEW_INS_EXE      = $$THIRD_PARTY_SRC/$$VER_LDVIEW/bin/$$QT_ARCH/LDView$$join(ARCH,,,.exe)
    } else {
        isEmpty(LDVIEW_INS_EXE):LDVIEW_INS_EXE      = $$THIRD_PARTY_SRC/$$VER_LDVIEW/bin/$$QT_ARCH/LDView.exe
    }

    # source directories - 3rd party components
    isEmpty(LDGLITE_INS_DOC):LDGLITE_INS_DOC        = $$THIRD_PARTY_SRC/$$VER_LDGLITE/docs
    isEmpty(LDVIEW_INS_DOC):LDVIEW_INS_DOC          = $$THIRD_PARTY_SRC/$$VER_LDVIEW/docs
    isEmpty(RAYTRACE_INS_DOC):RAYTRACE_INS_DOC      = $$THIRD_PARTY_SRC/$$VER_POVRAY/docs

    isEmpty(LDGLITE_INS_RES):LDGLITE_INS_RES        = $$THIRD_PARTY_SRC/$$VER_LDGLITE/resources
    isEmpty(LDVIEW_INS_RES):LDVIEW_INS_RES          = $$THIRD_PARTY_SRC/$$VER_LDVIEW/resources
    isEmpty(RAYTRACE_INS_RES):RAYTRACE_INS_RES      = $$THIRD_PARTY_SRC/$$VER_POVRAY/resources

    # install paths
    isEmpty(INSTALL_PREFIX):INSTALL_PREFIX          = $$OUT_PWD
    isEmpty(LPUB3D_INS_CONTENT_PATH):LPUB3D_INS_CONTENT_PATH = $$INSTALL_PREFIX/$$join(ARCH,,,bit_$$DIST)/3rdParty

    # install data directories - 3rd party components
    isEmpty(LDGLITE_INS_DIR):LDGLITE_INS_DIR           = $$LPUB3D_INS_CONTENT_PATH/$$VER_LDGLITE/bin
    isEmpty(LDGLITE_INS_DOC_DIR):LDGLITE_INS_DOC_DIR   = $$LPUB3D_INS_CONTENT_PATH/$$VER_LDGLITE/doc
    isEmpty(LDGLITE_INS_RES_DIR):LDGLITE_INS_RES_DIR   = $$LPUB3D_INS_CONTENT_PATH/$$VER_LDGLITE/resources
    isEmpty(LDVIEW_INS_DIR):LDVIEW_INS_DIR             = $$LPUB3D_INS_CONTENT_PATH/$$VER_LDVIEW/bin
    isEmpty(LDVIEW_INS_DOC_DIR):LDVIEW_INS_DOC_DIR     = $$LPUB3D_INS_CONTENT_PATH/$$VER_LDVIEW/doc
    isEmpty(LDVIEW_INS_RES_DIR):LDVIEW_INS_RES_DIR     = $$LPUB3D_INS_CONTENT_PATH/$$VER_LDVIEW/resources
    isEmpty(RAYTRACE_INS_DIR):RAYTRACE_INS_DIR         = $$LPUB3D_INS_CONTENT_PATH/$$VER_POVRAY/bin
    isEmpty(RAYTRACE_INS_DOC_DIR):RAYTRACE_INS_DOC_DIR = $$LPUB3D_INS_CONTENT_PATH/$$VER_POVRAY/doc
    isEmpty(RAYTRACE_INS_RES_DIR):RAYTRACE_INS_RES_DIR = $$LPUB3D_INS_CONTENT_PATH/$$VER_POVRAY/resources

    # install 3rd party content
    include(install3rdpartycontent.pri)

    stagewindistcontent {

        # stage directory path components
        isEmpty(LPUB3D_STG_PRODUCT):LPUB3D_STG_PRODUCT = $$STG_TARGET
        isEmpty(LPUB3D_STG_VERSION):LPUB3D_STG_VERSION = $$VERSION
        isEmpty(LPUB3D_STG_FULLVER):LPUB3D_STG_FULLVER = $$VERSION"."$$VER_REVISION_STR"."$$VER_BUILD_STR"_"$$DATE_YY$$DATE_MM$$DATE_DD
        isEmpty(LPUB3D_STG_DIRNAME):LPUB3D_STG_DIRNAME = $$LPUB3D_STG_PRODUCT"-Any-"$$LPUB3D_STG_FULLVER

        # stage directories - lpub3d contentt
        isEmpty(LPUB3D_STAGE):LPUB3D_STAGE              = $$_PRO_FILE_PWD_/../builds/windows
        isEmpty(LPUB3D_STG_EXTRAS):LPUB3D_STG_EXTRAS    = $$_PRO_FILE_PWD_/extras
        isEmpty(LPUB3D_STG_DOCS):LPUB3D_STG_DOCS        = $$_PRO_FILE_PWD_/docs
        isEmpty(LPUB3D_STG_LIBS):LPUB3D_STG_LIBS        = $$_PRO_FILE_PWD_/..

        # stage directories
        isEmpty(LPUB3D_STG_PATH):LPUB3D_STG_PATH        = $$LPUB3D_STAGE/$$DIST/$$LPUB3D_STG_DIRNAME/$$LPUB3D_STG_PRODUCT"_"$$STG_ARCH
         message("~~~ WIN_DIST_STAGE_TO $$LPUB3D_STG_PATH ~~~")

        isEmpty(LPUB3D_STG_3RD_PATH):LPUB3D_STG_3RD_PATH= $$LPUB3D_STG_PATH/3rdParty
        message("~~~ 3RD_DIST_STAGE_TO $$LPUB3D_STG_3RD_PATH ~~~")

        # destination data directories - 3rd party components
        isEmpty(LDGLITE_STG_DIR):LDGLITE_STG_DIR           = $$LPUB3D_STG_3RD_PATH/$$VER_LDGLITE/bin
        isEmpty(LDGLITE_STG_DOC_DIR):LDGLITE_STG_DOC_DIR   = $$LPUB3D_STG_3RD_PATH/$$VER_LDGLITE/doc
        isEmpty(LDGLITE_STG_RES_DIR):LDGLITE_STG_RES_DIR   = $$LPUB3D_STG_3RD_PATH/$$VER_LDGLITE/resources
        isEmpty(LDVIEW_STG_DIR):LDVIEW_STG_DIR             = $$LPUB3D_STG_3RD_PATH/$$VER_LDVIEW/bin
        isEmpty(LDVIEW_STG_DOC_DIR):LDVIEW_STG_DOC_DIR     = $$LPUB3D_STG_3RD_PATH/$$VER_LDVIEW/doc
        isEmpty(LDVIEW_STG_RES_DIR):LDVIEW_STG_RES_DIR     = $$LPUB3D_STG_3RD_PATH/$$VER_LDVIEW/resources
        isEmpty(RAYTRACE_STG_DIR):RAYTRACE_STG_DIR         = $$LPUB3D_STG_3RD_PATH/$$VER_POVRAY/bin
        isEmpty(RAYTRACE_STG_DOC_DIR):RAYTRACE_STG_DOC_DIR = $$LPUB3D_STG_3RD_PATH/$$VER_POVRAY/doc
        isEmpty(RAYTRACE_STG_RES_DIR):RAYTRACE_STG_RES_DIR = $$LPUB3D_STG_3RD_PATH/$$VER_POVRAY/resources

        # stage Windows distribution content
        include(stagewindistcontent.pri)
    }
}
