    # These settings are used for package distribution
win32 {
    # The Windows package distribution settings below requires a specific dev env configuration.
    # Basically, if you create a projects folder e.g. c:\Users\<user>\Projects and
    # clone the listed GitHub repos under the Projects folder, your dev env will
    # conform to these settings.
    # GitHub clone list:
    # LPub3D  = https://github.com/trevorsandy/lpub3d [Branch: master]
    # LDView  = https://github.com/tcobbs/ldview      [Branch: master]
    # POV-Ray = https://github.com/trevorsandy/povray [Branch: lpub3d/lpub3d-windows-cui]
    # In the end your Projects folder should look like this
    # C:\Users\<user>\Projects\
    #                   |
    #                   ---\povray\
    #                   |
    #                   ---\LPub3D\
    #                   |
    #                   ---\ldview\
    # Additionally, when using QtCreator be sure 'Shadow Build' is enabled.
    #
    isEmpty(THIRD_PARTY):THIRD_PARTY   = $$_PRO_FILE_PWD_/../builds/3rdParty
    isEmpty(BIN_DIR):BIN_DIR           = $$THIRD_PARTY/bin
    isEmpty(DOCS_DIR):DOCS_DIR         = $$THIRD_PARTY/docs
    isEmpty(RESOURCE_DIR):RESOURCE_DIR = $$THIRD_PARTY/resources

    isEmpty(LDGLITE_DIR):LDGLITE_DIR           = $$BIN_DIR/windows/ldglite-1.3.3/$$join(ARCH,,,bit)
    isEmpty(LDGLITE_DOC_DIR):LDGLITE_DOC_DIR   = $$DOCS_DIR/ldglite-1.3.3
    isEmpty(LDVIEW_DIR):LDVIEW_DIR             = $$BIN_DIR/windows/ldview-4.3/$$join(ARCH,,,bit)
    isEmpty(LDVIEW_DOC_DIR):LDVIEW_DOC_DIR     = $$DOCS_DIR/ldview-4.3
    isEmpty(LDVIEW_RES_DIR):LDVIEW_RES_DIR     = $$RESOURCE_DIR/ldview-4.3
    isEmpty(RAYTRACE_DIR):RAYTRACE_DIR         = $$BIN_DIR/windows/lpub3d-trace-3.7.2/$$join(ARCH,,,bit)
    isEmpty(RAYTRACE_DOC_DIR):RAYTRACE_DOC_DIR = $$DOCS_DIR/lpub3d-trace-3.7.2
    isEmpty(RAYTRACE_RES_DIR):RAYTRACE_RES_DIR = $$RESOURCE_DIR/lpub3d-trace-3.7.2

    isEmpty(LDVIEW):LDVIEW = $$_PRO_FILE_PWD_/../../ldview
    isEmpty(LDGLITE):LDGLITE = $$_PRO_FILE_PWD_/../ldglite
    isEmpty(RAYTRACE):RAYTRACE = $$_PRO_FILE_PWD_/../../povray
    isEmpty(LDVIEW_DEV):LDVIEW_DEV = $$LDVIEW/Build
    isEmpty(RAYTRACE_DEV):RAYTRACE_DEV = $$RAYTRACE/windows/vs2015
    isEmpty(RAYTRACE_DIST):RAYTRACE_DIST = $$RAYTRACE/distribution

    equals (ARCH, 64) {
        isEmpty(LDVIEW_EXE):LDVIEW_EXE = $$LDVIEW_DEV/Release64/LDView64.exe
        isEmpty(RAYTRACE_EXE):RAYTRACE_EXE = $$RAYTRACE_DEV/bin64/lpub3d_trace_cui64.exe
    } else {
        isEmpty(LDVIEW_EXE):LDVIEW_EXE = $$LDVIEW_DEV/Release/LDView.exe
        isEmpty(RAYTRACE_EXE):RAYTRACE_EXE = $$RAYTRACE_DEV/bin32/lpub3d_trace_cui32.exe
    }

    # renderers executables
    CONFIG(release, debug|release) {
        ldglite.files += $$DESTDIR/../../ldglite/release/ldglite.exe
        ldglite.path = $$LDGLITE_DIR

        ldview.files += $$LDVIEW_EXE
        ldview.path = $$LDVIEW_DIR

        raytracer.files += $$RAYTRACE_EXE
        raytracer.path = $$RAYTRACE_DIR
    }

    docopyfiles {

        # ldglite content
        ldglite_docs.files += \
             $$LDGLITE/doc/README.TXT \
             $$LDGLITE/doc/LICENCE
        ldglite_docs.path = $$LDGLITE_DOC_DIR

        # ldview contnet
        ldview_docs.files += \
             $$LDVIEW/HELP.HTML \
             $$LDVIEW/README.TXT \
             $$LDVIEW/LICENSE.TXT \
             $$LDVIEW/CHANGEHISTORY.HTML
        ldview_docs.path = $$LDVIEW_DOC_DIR

        ldview_resources.files += \
            $$LDVIEW/LDExporter/LGEO.xml \
            $$LDVIEW/Textures/SansSerif.fnt \
            $$LDVIEW/Textures/StudLogo.png \
            $$LDVIEW/Textures/StudLogo.psd \
            $$LDVIEW/8464.mpd \
            $$LDVIEW/m6459.ldr
        ldview_resources.path = $$LDVIEW_RES_DIR

        # raytracer (povray) content
        raytracer_docs.files += \
            $$RAYTRACE/windows/CUI_README.txt \
            $$RAYTRACE/windows/README.md \
            $$RAYTRACE/changes.txt \
            $$RAYTRACE/revision.txt
        raytracer_docs.path = $$RAYTRACE_DOC_DIR

        raytracer_resources_ini.files += \
            $$RAYTRACE_DIST/ini/allanim.ini \
            $$RAYTRACE_DIST/ini/allstill.ini \
            $$RAYTRACE_DIST/ini/low.ini \
            $$RAYTRACE_DIST/ini/pngflc.ini \
            $$RAYTRACE_DIST/ini/pngfli.ini \
            $$RAYTRACE_DIST/ini/povray.ini \
            $$RAYTRACE_DIST/ini/quickres.ini \
            $$RAYTRACE_DIST/ini/res120.ini \
            $$RAYTRACE_DIST/ini/res1k.ini \
            $$RAYTRACE_DIST/ini/res320.ini \
            $$RAYTRACE_DIST/ini/res640.ini \
            $$RAYTRACE_DIST/ini/res800.ini \
            $$RAYTRACE_DIST/ini/slow.ini \
            $$RAYTRACE_DIST/ini/tgaflc.ini \
            $$RAYTRACE_DIST/ini/tgafli.ini \
            $$RAYTRACE_DIST/ini/zipflc.ini \
            $$RAYTRACE_DIST/ini/zipfli.ini
        raytracer_resources_ini.path = $$RAYTRACE_RES_DIR\ini

        raytracer_resources_include.files += \
            $$RAYTRACE_DIST/include/arrays.inc \
            $$RAYTRACE_DIST/include/ash.map \
            $$RAYTRACE_DIST/include/benediti.map \
            $$RAYTRACE_DIST/include/bubinga.map \
            $$RAYTRACE_DIST/include/bumpmap_.png \
            $$RAYTRACE_DIST/include/cedar.map \
            $$RAYTRACE_DIST/include/chars.inc \
            $$RAYTRACE_DIST/include/colors.inc \
            $$RAYTRACE_DIST/include/colors_ral.inc \
            $$RAYTRACE_DIST/include/consts.inc \
            $$RAYTRACE_DIST/include/crystal.ttf \
            $$RAYTRACE_DIST/include/cyrvetic.ttf \
            $$RAYTRACE_DIST/include/debug.inc \
            $$RAYTRACE_DIST/include/finish.inc \
            $$RAYTRACE_DIST/include/fract003.png \
            $$RAYTRACE_DIST/include/functions.inc \
            $$RAYTRACE_DIST/include/glass.inc \
            $$RAYTRACE_DIST/include/glass_old.inc \
            $$RAYTRACE_DIST/include/golds.inc \
            $$RAYTRACE_DIST/include/ior.inc \
            $$RAYTRACE_DIST/include/logo.inc \
            $$RAYTRACE_DIST/include/makegrass.inc \
            $$RAYTRACE_DIST/include/marbteal.map \
            $$RAYTRACE_DIST/include/math.inc \
            $$RAYTRACE_DIST/include/meshmaker.inc \
            $$RAYTRACE_DIST/include/metals.inc \
            $$RAYTRACE_DIST/include/Mount1.png \
            $$RAYTRACE_DIST/include/Mount2.png \
            $$RAYTRACE_DIST/include/mtmand.pot \
            $$RAYTRACE_DIST/include/mtmandj.png \
            $$RAYTRACE_DIST/include/orngwood.map \
            $$RAYTRACE_DIST/include/pinkmarb.map \
            $$RAYTRACE_DIST/include/plasma2.png \
            $$RAYTRACE_DIST/include/plasma3.png \
            $$RAYTRACE_DIST/include/povlogo.ttf \
            $$RAYTRACE_DIST/include/povmap.png \
            $$RAYTRACE_DIST/include/rad_def.inc \
            $$RAYTRACE_DIST/include/rand.inc \
            $$RAYTRACE_DIST/include/rdgranit.map \
            $$RAYTRACE_DIST/include/screen.inc \
            $$RAYTRACE_DIST/include/shapes.inc \
            $$RAYTRACE_DIST/include/shapes2.inc \
            $$RAYTRACE_DIST/include/shapes3.inc \
            $$RAYTRACE_DIST/include/shapesq.inc \
            $$RAYTRACE_DIST/include/shapes_old.inc \
            $$RAYTRACE_DIST/include/skies.inc \
            $$RAYTRACE_DIST/include/spiral.df3 \
            $$RAYTRACE_DIST/include/stage1.inc \
            $$RAYTRACE_DIST/include/stars.inc \
            $$RAYTRACE_DIST/include/stdcam.inc \
            $$RAYTRACE_DIST/include/stdinc.inc \
            $$RAYTRACE_DIST/include/stoneold.inc \
            $$RAYTRACE_DIST/include/stones.inc \
            $$RAYTRACE_DIST/include/stones1.inc \
            $$RAYTRACE_DIST/include/stones2.inc \
            $$RAYTRACE_DIST/include/strings.inc \
            $$RAYTRACE_DIST/include/sunpos.inc \
            $$RAYTRACE_DIST/include/teak.map \
            $$RAYTRACE_DIST/include/test.png \
            $$RAYTRACE_DIST/include/textures.inc \
            $$RAYTRACE_DIST/include/timrom.ttf \
            $$RAYTRACE_DIST/include/transforms.inc \
            $$RAYTRACE_DIST/include/ttffonts.cat \
            $$RAYTRACE_DIST/include/whiteash.map \
            $$RAYTRACE_DIST/include/woodmaps.inc \
            $$RAYTRACE_DIST/include/woods.inc
        raytracer_resources_include.path = $$RAYTRACE_RES_DIR\include

        INSTALLS += \
        ldglite_docs \
        ldview_docs \
        ldview_resources \
        raytracer_docs \
        raytracer_resources_ini \
        raytracer_resources_include
    }

    CONFIG(release, debug|release) {
        INSTALLS += \
        ldglite \
        ldview \
        raytracer

        message("~~~ COPIED LDGLITE_EXE $$join(ARCH,,,bit) ~~~")
        message("~~~ COPIED LDVIEW_EXE $$join(ARCH,,,bit) ~~~")
        message("~~~ COPIED RAYTRACE_EXE $$join(ARCH,,,bit) ~~~")
    }

    CONFIG(debug, debug|release) {
        message("~~~ DEBUG BUILD - EXE'S NOT COPIED ~~~")
    }
}
