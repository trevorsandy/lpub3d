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
    isEmpty(RESOURCE_DIR):RESOURCE_DIR     = $$SHARE_DIR/lpub3d

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
    ldraw_official_library

    DEFINES += LC_INSTALL_PREFIX=\\\"$$INSTALL_PREFIX\\\"

    !isEmpty(DISABLE_UPDATE_CHECK) {
            DEFINES += LC_DISABLE_UPDATE_CHECK=$$DISABLE_UPDATE_CHECK
    }

    !isEmpty(LDRAW_LIBRARY_PATH) {
            DEFINES += LC_LDRAW_LIBRARY_PATH=\\\"$$LDRAW_LIBRARY_PATH\\\"
    }

    # The package distribution settings below requires a specific dev env configuration.
    # Basically, if you create a projects folder e.g. c:\Users\<user>\Projects and
    # clone the listed GitHub repos under the Projects folder, your dev env will
    # conform to these settings.
    # GitHub clone list:
    # LPub3D           = https://github.com/trevorsandy/lpub3d [Branch: master]
    # 3rdParty Content = https://github.com/trevorsandy/lpub3d_linux_3rdparty [Branch: master]
    #
    # At the end, your Projects folder should look like this:
    # C:\Users\<user>\Projects\
    #                   |
    #                   ---\lpub3d_linux_3rdparty\
    #                   |
    #                   ---\LPub3D\
    #
    # Additionally, when using QtCreator be sure 'Shadow Build' is enabled.
    #
    # source path
    isEmpty(THIRD_PARTY_SRC):THIRD_PARTY_SRC = $$_PRO_FILE_PWD_/../../lpub3d_linux_3rdparty
    message("~~~ INSTALL FROM REPO $$THIRD_PARTY_SRC ~~~")

    # source executables - 3rd party components
    isEmpty(LDGLITE_INS_EXE):LDGLITE_INS_EXE   = $$THIRD_PARTY_SRC/$$VER_LDGLITE/bin/$$QT_ARCH/ldglite
    isEmpty(LDVIEW_INS_EXE):LDVIEW_INS_EXE     = $$THIRD_PARTY_SRC/$$VER_LDVIEW/bin/$$QT_ARCH/ldview
    isEmpty(RAYTRACE_INS_EXE):RAYTRACE_INS_EXE = $$THIRD_PARTY_SRC/$$VER_POVRAY/bin/$$QT_ARCH/lpub3d_trace_cui

    # source directories - 3rd party components
    isEmpty(LDGLITE_INS_DOC):LDGLITE_INS_DOC   = $$THIRD_PARTY_SRC/$$VER_LDGLITE/docs
    isEmpty(LDVIEW_INS_DOC):LDVIEW_INS_DOC     = $$THIRD_PARTY_SRC/$$VER_LDVIEW/docs
    isEmpty(RAYTRACE_INS_DOC):RAYTRACE_INS_DOC = $$THIRD_PARTY_SRC/$$VER_POVRAY/docs

    isEmpty(LDGLITE_INS_RES):LDGLITE_INS_RES   = $$THIRD_PARTY_SRC/$$VER_LDGLITE/resources
    isEmpty(LDVIEW_INS_RES):LDVIEW_INS_RES     = $$THIRD_PARTY_SRC/$$VER_LDVIEW/resources
    isEmpty(RAYTRACE_INS_RES):RAYTRACE_INS_RES = $$THIRD_PARTY_SRC/$$VER_POVRAY/resources

    # installed data directories - 3rd party components
    isEmpty(LDGLITE_INS_DIR):LDGLITE_INS_DIR           = $$SHARE_DIR/lpub3d/3rdParty/$$VER_LDGLITE/bin
    isEmpty(LDGLITE_INS_DOC_DIR):LDGLITE_INS_DOC_DIR   = $$SHARE_DIR/lpub3d/3rdParty/$$VER_LDGLITE/doc
    isEmpty(LDGLITE_INS_RES_DIR):LDGLITE_INS_RES_DIR   = $$SHARE_DIR/lpub3d/3rdParty/$$VER_LDGLITE/resources
    isEmpty(LDVIEW_INS_DIR):LDVIEW_INS_DIR             = $$SHARE_DIR/lpub3d/3rdParty/$$VER_LDVIEW/bin
    isEmpty(LDVIEW_INS_RES_DIR):LDVIEW_INS_RES_DIR     = $$SHARE_DIR/lpub3d/3rdParty/$$VER_LDVIEW/resources
    isEmpty(LDVIEW_INS_DOC_DIR):LDVIEW_INS_DOC_DIR     = $$SHARE_DIR/lpub3d/3rdParty/$$VER_LDVIEW/doc
    isEmpty(RAYTRACE_INS_DIR):RAYTRACE_INS_DIR         = $$SHARE_DIR/lpub3d/3rdParty/$$VER_POVRAY/bin
    isEmpty(RAYTRACE_INS_DOC_DIR):RAYTRACE_INS_DOC_DIR = $$SHARE_DIR/lpub3d/3rdParty/$$VER_POVRAY/doc
    isEmpty(RAYTRACE_INS_RES_DIR):RAYTRACE_INS_RES_DIR = $$SHARE_DIR/lpub3d/3rdParty/$$VER_POVRAY/resources

    # install 3rd party content
    include(install3rdpartycontent.pri)
}
