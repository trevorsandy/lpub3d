macx {

    ICON = lpub3d.icns
    QMAKE_INFO_PLIST = Info.plist

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

    # renderers
#    CONFIG(release, debug|release) {
#        ldglite.files += $$DESTDIR/../../ldglite/release/ldglite.app
#        ldglite.path = Contents/3rdParty/bin

#        ldview.files += ../builds/3rdParty/bin/macx/ldview-4.3/ldview.app
#        ldview.path = Contents/3rdParty/bin
#    }

#    ldglite_docs.files += ../ldglite/doc/ldglite.1 ../ldglite/doc/README.TXT ../ldglite/doc/LICENCE
#    ldglite_docs.path = Contents/3rdParty/docs/ldglite

#    liglite_resources.files += ../ldglite/ldglite_osxwrapper.sh
#    ldglite_resources.path = Contents/3rdParty/resources/ldglite

#    ldview_docs.files += \
#         ../builds/3rdParty/docs/ldview-4.3/HELP.HTML \
#         ../builds/3rdParty/docs/ldview-4.3/README.TXT \
#         ../builds/3rdParty/docs/ldview-4.3/LICENSE.TXT \
#         ../builds/3rdParty/docs/ldview-4.3/CHANGEHISTORY.HTML
#    ldview_docs.path = Contents/3rdParty/docs/ldvieW

#    ldview_resources.files += \
#          ../builds/3rdParty/resources/ldview-4.3/LDExportMessages.ini \
#          ../builds/3rdParty/resources/ldview-4.3/LDViewMessages.ini \
#          ../builds/3rdParty/resources/ldview-4.3/SansSerif.fnt \
#          ../builds/3rdParty/resources/ldview-4.3/ldviewrc \
#          ../builds/3rdParty/resources/ldview-4.3/StudLogo.png \
#          ../builds/3rdParty/resources/ldview-4.3/StudLogo.psd \
#          ../builds/3rdParty/resources/ldview-4.3/8464.mpd \
#          ../builds/3rdParty/resources/ldview-4.3/m6459.ldr
#    ldview_resources.path = Contents/3rdParty/resources/ldview

    # libraries
    CONFIG(release, debug|release) {
        libquazip.files += \
            $$DESTDIR/../../quazip/release/libQuaZIP.0.dylib
        libquazip.path = Contents/Libs

        libldrawini.files += \
            $$DESTDIR/../../ldrawini/release/libLDrawIni.16.dylib
        libldrawini.path = Contents/Libs
    }

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
        ldraw_official_library #\
#        ldview_docs \
#        ldview_resources \
#        ldglite_docs \
#        ldglite_resources
    CONFIG(release, debug|release) {
    QMAKE_BUNDLE_DATA += \
        libquazip \
        libldrawini #\
#        ldglite \
#        ldview
     }
}
