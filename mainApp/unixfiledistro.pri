unix:!macx {

    # For compiled builds on unix set C++11 standard appropriately
    GCC_VERSION = $$system(g++ -dumpversion)
    greaterThan(GCC_VERSION, 4.6) {
        QMAKE_CXXFLAGS += -std=c++11
    } else {
        QMAKE_CXXFLAGS += -std=c++0x
    }

    binarybuild {
        # To build a binary distribution that will not require elevated rights to install,
        # pass CONFIG+=binarybuild to qmake (i.e. in QtCreator, set in qmake Additional Arguments)
        # The installer which uses Qt installer framework can be found in the "builds" source folder.
        # This macro is used to properly load parameter files on initial launch
        DEFINES += X11_BINARY_BUILD
        # Linker flag setting to properly direct LPub3D to ldrawini and quazip shared libraries.
        # This setting assumes dependent libraries are deposited at <exe location>/lib by the installer.
        QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/lib\'"

    }

    # These defines point LPub3D to the architecture appropriate content
    # when performing 'check for update' download and installation
    # Don't forget to set CONFIG+=<deb|rpm|pkg> accordingly if NOT using
    # the accompanying build scripts - CreateDeb.sh, CreateRpm.sh or CreatePkg.sh
    deb: PACKAGE_TYPE = DEB_DISTRO
    rpm: PACKAGE_TYPE = RPM_DISTRO
    pkg: PACKAGE_TYPE = PKG_DISTRO
    !isEmpty(PACKAGE_TYPE): DEFINES += $$PACKAGE_TYPE

    MAN_PAGE = lpub3d$$VER_MAJOR$$VER_MINOR
    MAN_PAGE = $$join(MAN_PAGE,,,.1)

    # These settings are used for package distributions that will require elevated rights to install
    isEmpty(INSTALL_PREFIX):INSTALL_PREFIX = /usr
    isEmpty(SHARE_DIR):SHARE_DIR = $$INSTALL_PREFIX/share

    isEmpty(BIN_DIR):BIN_DIR               = $$SHARE_DIR/bin
    isEmpty(DOCS_DIR):DOCS_DIR             = $$SHARE_DIR/doc/lpub3d
    isEmpty(ICON_DIR):ICON_DIR             = $$SHARE_DIR/pixmaps
    isEmpty(MAN_DIR):MAN_DIR               = $$SHARE_DIR/man/man1
    isEmpty(DESKTOP_DIR):DESKTOP_DIR       = $$SHARE_DIR/applications
    isEmpty(MIME_DIR):MIME_DIR             = $$SHARE_DIR/mime/packages
    isEmpty(MIME_ICON_DIR):MIME_ICON_DIR   = $$SHARE_DIR/icons/hicolor/scalable/mimetypes

    isEmpty(RESOURCE_DIR):RESOURCE_DIR       = $$SHARE_DIR/lpub3d
    isEmpty(LDGLITE_DIR):LDGLITE_DIR         = $$SHARE_DIR/lpub3d/3rdParty/bin
    isEmpty(LDGLITE_DOC_DIR):LDGLITE_DOC_DIR = $$SHARE_DIR/doc/lpub3d/3rdParty/ldglite
    isEmpty(LDGLITE_RES_DIR):LDGLITE_RES_DIR = $$SHARE_DIR/lpub3d/3rdParty/ldglite
    isEmpty(LDVIEW_DIR):LDVIEW_DIR           = $$SHARE_DIR/lpub3d/3rdParty/bin
    isEmpty(LDVIEW_RES_DIR):LDVIEW_RES_DIR   = $$SHARE_DIR/lpub3d/3rdParty/ldview
    isEmpty(LDVIEW_DOC_DIR):LDVIEW_DOC_DIR   = $$SHARE_DIR/doc/lpub3d/3rdParty/ldview

    target.path = $$BIN_DIR

    docs.files += docs/README.txt docs/CREDITS.txt docs/COPYING.txt
    docs.path = $$DOCS_DIR

    man.files += $$MAN_PAGE
    man.path = $$MAN_DIR

    desktop.files += lpub3d.desktop
    desktop.path = $$DESKTOP_DIR

    icon.files += images/lpub3d.png
    icon.path = $$ICON_DIR

    mime.files += lpub3d.xml
    mime.path = $$MIME_DIR

    mime_ldraw_icon.files += images/x-ldraw.svg
    mime_ldraw_icon.path = $$MIME_ICON_DIR

    mime_multi_part_ldraw_icon.files += images/x-multi-part-ldraw.svg
    mime_multi_part_ldraw_icon.path = $$MIME_ICON_DIR

    mime_multipart_ldraw_icon.files += images/x-multipart-ldraw.svg
    mime_multipart_ldraw_icon.path = $$MIME_ICON_DIR

    excluded_count_parts.files += extras/excludedParts.lst
    excluded_count_parts.path = $$RESOURCE_DIR

    fadestep_color_parts.files += extras/fadeStepColorParts.lst
    fadestep_color_parts.path = $$RESOURCE_DIR

    pli_freeform_annotations.files += extras/freeformAnnotations.lst
    pli_freeform_annotations.path = $$RESOURCE_DIR

    pli_title_annotations.files += extras/titleAnnotations.lst
    pli_title_annotations.path = $$RESOURCE_DIR

    pli_orientation.files += extras/pli.mpd
    pli_orientation.path = $$RESOURCE_DIR

    pli_substitution_parts.files += extras/pliSubstituteParts.lst
    pli_substitution_parts.path = $$RESOURCE_DIR

    ldraw_unofficial_library.files += extras/lpub3dldrawunf.zip
    ldraw_unofficial_library.path = $$RESOURCE_DIR

    ldraw_official_library.files += extras/complete.zip
    ldraw_official_library.path = $$RESOURCE_DIR

    # renderer executables
    CONFIG(release, debug|release) {
        ldglite.files += $$DESTDIR/../../ldglite/release/ldglite
        ldglite.path = $$LDGLITE_DIR

        ldview.files += ../builds/3rdParty/bin/macx/ldview/ldview
        ldview.path = $$LDVIEW_DIR
    }
    ldglite_docs.files += ../ldglite/ldglite.1 ../ldglite/doc/README.TXT ../ldglite/doc/LICENCE
    ldglite_docs.path = $$LDGLITE_DOC_DIR

    liglite_resources.files += ../ldglite/ldglite_osxwrapper.sh
    ldglite_resources.path = $$LDGLITE_RES_DIR

    ldview_docs.files += \
         ../builds/3rdParty/docs/ldview-4.3/HELP.HTML \
         ../builds/3rdParty/docs/ldview-4.3/README.TXT \
         ../builds/3rdParty/docs/ldview-4.3/LICENSE.TXT \
         ../builds/3rdParty/docs/ldview-4.3/CHANGEHISTORY.HTML
    ldview_docs.path = $$LDVIEW_DOC_DIR

    ldview_resources.files += \
#          ../builds/3rdParty/resources/ldview-4.3/LDExportMessages.ini \
#          ../builds/3rdParty/resources/ldview-4.3/LDViewMessages.ini \
          ../builds/3rdParty/resources/ldview-4.3/SansSerif.fnt \
          ../builds/3rdParty/resources/ldview-4.3/ldviewrc \
          ../builds/3rdParty/resources/ldview-4.3/StudLogo.png \
          ../builds/3rdParty/resources/ldview-4.3/StudLogo.psd \
          ../builds/3rdParty/resources/ldview-4.3/8464.mpd \
          ../builds/3rdParty/resources/ldview-4.3/m6459.ldr
    ldview_resources.path = $$LDVIEW_RES_DIR

    INSTALLS += \
    target \
    docs \
    man \
    desktop \
    icon\
    mime\
    mime_ldraw_icon \
    mime_multi_part_ldraw_icon \
    mime_multipart_ldraw_icon \
    excluded_count_parts \
    fadestep_color_parts \
    pli_freeform_annotations \
    pli_title_annotations \
    pli_orientation \
    pli_substitution_parts \
    ldraw_unofficial_library \
    ldraw_official_library \
    ldglite_docs \
    ldglite_resources \
    ldview_docs \
    ldview_resources

    CONFIG(release, debug|release) {
        INSTALLS += \
        ldglite \
        ldview
    }

    DEFINES += LC_INSTALL_PREFIX=\\\"$$INSTALL_PREFIX\\\"

    !isEmpty(DISABLE_UPDATE_CHECK) {
            DEFINES += LC_DISABLE_UPDATE_CHECK=$$DISABLE_UPDATE_CHECK
    }

    !isEmpty(LDRAW_LIBRARY_PATH) {
            DEFINES += LC_LDRAW_LIBRARY_PATH=\\\"$$LDRAW_LIBRARY_PATH\\\"
    }
}
