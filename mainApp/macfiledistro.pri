macx {

    ICON = lpub3d.icns
    QMAKE_INFO_PLIST = Info.plist

    # libraries
    CONFIG(release, debug|release) {
        libquazip.files += \
            $$DESTDIR/../../quazip/release/libQuaZIP.0.dylib
        libquazip.path = Contents/Libs

        libldrawini.files += \
            $$DESTDIR/../../ldrawini/release/libLDrawIni.16.dylib
        libldrawini.path = Contents/Libs

        QMAKE_BUNDLE_DATA += \
            libquazip \
            libldrawini
    }

    # documents and resources
    document_icon.files += ldraw_document.icns
    document_icon.path = Contents/Resources

    document_readme.files += docs/README.txt
    document_readme.path = Contents/Resources

    document_credits.files += docs/CREDITS.txt
    document_credits.path = Contents/Resources

    document_copying.files += docs/COPYING.txt
    document_copying.path = Contents/Resources

    excluded_count_parts.files += extras/excludedParts.lst
    excluded_count_parts.path = Contents/Resources

    fadestep_color_parts.files += extras/fadeStepColorParts.lst
    fadestep_color_parts.path = Contents/Resources

    pli_freeform_annotations.files += extras/freeformAnnotations.lst
    pli_freeform_annotations.path = Contents/Resources

    pli_title_annotations.files += extras/titleAnnotations.lst
    pli_title_annotations.path = Contents/Resources

    pli_orientation.files += extras/pli.mpd
    pli_orientation.path = Contents/Resources

    pli_substitution_parts.files += extras/pliSubstituteParts.lst
    pli_substitution_parts.path = Contents/Resources

    ldraw_unofficial_library.files += extras/lpub3dldrawunf.zip
    ldraw_unofficial_library.path = Contents/Resources

    ldraw_official_library.files += extras/complete.zip
    ldraw_official_library.path = Contents/Resources

    QMAKE_BUNDLE_DATA += \
        document_icon \
        document_readme \
        document_credits \
        document_copying \
        excluded_count_parts \
        fadestep_color_parts \
        pli_freeform_annotations \
        pli_title_annotations \
        pli_orientation \
        pli_substitution_parts \
        ldraw_unofficial_library \
        ldraw_official_library

    # The package distribution settings below requires a specific dev env configuration.
    # Basically, if you create a projects folder e.g. c:\Users\<user>\Projects and
    # clone the listed GitHub repos under the Projects folder, your dev env will
    # conform to these settings.
    # GitHub clone list:
    # LPub3D           = https://github.com/trevorsandy/lpub3d [Branch: master]
    # 3rdParty Content = https://github.com/trevorsandy/lpub3d_macos_3rdparty [Branch: master]
    #
    # At the end, your Projects folder should look like this:
    # C:\Users\<user>\Projects\
    #                   |
    #                   ---\lpub3d_macos_3rdparty\
    #                   |
    #                   ---\LPub3D\
    #
    # Additionally, when using QtCreator be sure 'Shadow Build' is enabled.
    #
    # source directories
    isEmpty(THIRD_PARTY_SRC):THIRD_PARTY_SRC = $$_PRO_FILE_PWD_/../../lpub3d_macos_3rdparty
    isEmpty(THIRD_PARTY_STG):THIRD_PARTY_STG = $$_PRO_FILE_PWD_/../builds/3rdParty

    stage3rdcontent {

        # source directories
        isEmpty(LDGLITE_EXE):LDGLITE_EXE   = $$THIRD_PARTY_SRC/bin/ldglite-1.3/$$QT_ARCH/ldglite
        isEmpty(LDVIEW_EXE):LDVIEW_EXE     = $$THIRD_PARTY_SRC/bin/ldview-4.3/$$QT_ARCH/ldview
        isEmpty(RAYTRACE_EXE):RAYTRACE_EXE = $$THIRD_PARTY_SRC/bin/lpub3d_trace_cui-3.7.2/$$QT_ARCH/lpub3d_trace_cui

        include(stage3rdpartycontent.pri)
    }

    install3rdfrmrepo {

        # source directories
        isEmpty(LDGLITE_INS_EXE):LDGLITE_INS_EXE   = $$THIRD_PARTY_SRC/bin/ldglite-1.3/$$QT_ARCH/ldglite
        isEmpty(LDVIEW_INS_EXE):LDVIEW_INS_EXE     = $$THIRD_PARTY_SRC/bin/ldview-4.3/$$QT_ARCH/ldview
        isEmpty(RAYTRACE_INS_EXE):RAYTRACE_INS_EXE = $$THIRD_PARTY_SRC/bin/lpub3d_trace_cui-3.7.2/$$QT_ARCH/lpub3d_trace_cui

        isEmpty(LDGLITE_INS_DOC):LDGLITE_INS_DOC   = $$THIRD_PARTY_SRC/docs/ldglite-1.3
        isEmpty(LDVIEW_INS_DOC):LDVIEW_INS_DOC     = $$THIRD_PARTY_SRC/docs/ldview-4.3
        isEmpty(RAYTRACE_INS_DOC):RAYTRACE_INS_DOC = $$THIRD_PARTY_SRC/docs/lpub3d_trace_cui-3.7.2

        isEmpty(LDGLITE_INS_RES):LDGLITE_INS_RES   = $$THIRD_PARTY_SRC/resources/ldglite-1.3
        isEmpty(LDVIEW_INS_RES):LDVIEW_INS_RES     = $$THIRD_PARTY_SRC/resources/ldview-4.3
        isEmpty(RAYTRACE_INS_RES):RAYTRACE_INS_RES = $$THIRD_PARTY_SRC/resources/lpub3d_trace_cui-3.7.2

    } else:install3rdfrmstage {

        # stage directories
        isEmpty(LDGLITE_INS_EXE):LDGLITE_INS_EXE   = $$THIRD_PARTY_STG/bin/ldglite-1.3/$$QT_ARCH/ldglite
        isEmpty(LDVIEW_INS_EXE):LDVIEW_INS_EXE     = $$THIRD_PARTY_STG/bin/ldview-4.3/$$QT_ARCH/ldview
        isEmpty(RAYTRACE_INS_EXE):RAYTRACE_INS_EXE = $$THIRD_PARTY_STG/bin/lpub3d_trace_cui-3.7.2/$$QT_ARCH/lpub3d_trace_cui

        isEmpty(LDGLITE_INS_DOC):LDGLITE_INS_DOC   = $$THIRD_PARTY_STG/docs/ldglite-1.3
        isEmpty(LDVIEW_INS_DOC):LDVIEW_INS_DOC     = $$THIRD_PARTY_STG/docs/ldview-4.3
        isEmpty(RAYTRACE_INS_DOC):RAYTRACE_INS_DOC = $$THIRD_PARTY_STG/docs/lpub3d_trace_cui-3.7.2

        isEmpty(LDGLITE_INS_RES):LDGLITE_INS_RES   = $$THIRD_PARTY_STG/resources/ldglite-1.3
        isEmpty(LDVIEW_INS_RES):LDVIEW_INS_RES     = $$THIRD_PARTY_STG/resources/ldview-4.3
        isEmpty(RAYTRACE_INS_RES):RAYTRACE_INS_RES = $$THIRD_PARTY_STG/resources/lpub3d_trace_cui-3.7.2

    }

    INSTALL = $$find(CONFIG, install3rdfrm.*)
    !isEmpty(INSTALL) {

        # install directories
        isEmpty(LDGLITE_DIR):LDGLITE_DIR           = Contents/3rdParty/bin
        isEmpty(LDGLITE_DOC_DIR):LDGLITE_DOC_DIR   = Contents/3rdParty/docs/ldglite-1.3
        isEmpty(LDGLITE_RES_DIR):LDGLITE_RES_DIR   = Contents/3rdParty/resources/ldglite-1.3
        isEmpty(LDVIEW_DIR):LDVIEW_DIR             = Contents/3rdParty/bin
        isEmpty(LDVIEW_RES_DIR):LDVIEW_RES_DIR     = Contents/3rdParty/resources/ldview-4.3
        isEmpty(LDVIEW_DOC_DIR):LDVIEW_DOC_DIR     = Contents/3rdParty/docs/ldview-4.3
        isEmpty(RAYTRACE_DIR):RAYTRACE_DIR         = Contents/3rdParty/bin
        isEmpty(RAYTRACE_DOC_DIR):RAYTRACE_DOC_DIR = Contents/3rdParty/docs/lpub3d_trace_cui-3.7.2
        isEmpty(RAYTRACE_RES_DIR):RAYTRACE_RES_DIR = Contents/3rdParty/resources/lpub3d_trace_cui-3.7.2

        include(install3rdpartycontent.pri)
    }
}
