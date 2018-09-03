# Stage executables, documentation and resources as source files in the LPub3D source.
win32: stagewindistcontent {

    # stage lpub3d executables, documents and resources

    # 32bit and 64bit
    target.path = $$LPUB3D_STG_PATH

    lpub3d_plugin_bearer.files += \
        $$[QT_INSTALL_PLUGINS]/bearer/qgenericbearer.dll
    lpub3d_plugin_bearer.path = $$LPUB3D_STG_PATH/bearer

    lpub3d_plugin_iconengines.files += \
        $$[QT_INSTALL_PLUGINS]/iconengines/qsvgicon.dll
    lpub3d_plugin_iconengines.path = $$LPUB3D_STG_PATH/iconengines

    lpub3d_plugin_imageformats.files += \
        $$[QT_INSTALL_PLUGINS]/imageformats/qgif.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qicns.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qico.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qjpeg.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qsvg.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qtga.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qtiff.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qwbmp.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qwebp.dll
    lpub3d_plugin_imageformats.path = $$LPUB3D_STG_PATH/imageformats

    lpub3d_plugin_platforms.files += \
        $$[QT_INSTALL_PLUGINS]/platforms/qwindows.dll
    lpub3d_plugin_platforms.path = $$LPUB3D_STG_PATH/platforms

    lpub3d_plugin_printsupport.files += \
        $$[QT_INSTALL_PLUGINS]/printsupport/windowsprintersupport.dll
    lpub3d_plugin_printsupport.path = $$LPUB3D_STG_PATH/printsupport

    lpub3d_plugin_styles.files += \
        $$[QT_INSTALL_PLUGINS]/styles/qwindowsvistastyle.dll
    lpub3d_plugin_styles.path = $$LPUB3D_STG_PATH/styles

    lpub3d_bins_qtlibs.files += \
        $$[QT_INSTALL_BINS]/Qt5Concurrent.dll \
        $$[QT_INSTALL_BINS]/Qt5Core.dll \
        $$[QT_INSTALL_BINS]/Qt5Gui.dll \
        $$[QT_INSTALL_BINS]/Qt5Network.dll \
        $$[QT_INSTALL_BINS]/Qt5OpenGL.dll \
        $$[QT_INSTALL_BINS]/Qt5PrintSupport.dll \
        $$[QT_INSTALL_BINS]/Qt5Svg.dll \
        $$[QT_INSTALL_BINS]/Qt5Widgets.dll \
        $$[QT_INSTALL_BINS]/Qt5Xml.dll
    lpub3d_bins_qtlibs.path = $$LPUB3D_STG_PATH

    lpub3d_bins_additional_libs.files += \
        $$[QT_INSTALL_BINS]/D3Dcompiler_47.dll \
        $$[QT_INSTALL_BINS]/libEGL.dll \
        $$[QT_INSTALL_BINS]/libGLESV2.dll \
        $$[QT_INSTALL_BINS]/opengl32sw.dll
    lpub3d_bins_additional_libs.path = $$LPUB3D_STG_PATH

    lpub3d_extras.files += \
        $$LPUB3D_STG_EXTRAS/excludedParts.lst \
        $$LPUB3D_STG_EXTRAS/fadeStepColorParts.lst \
        $$LPUB3D_STG_EXTRAS/freeformAnnotations.lst \
        $$LPUB3D_STG_EXTRAS/LDConfig.ldr \
        $$LPUB3D_STG_EXTRAS/ldglite.ini \
        $$LPUB3D_STG_EXTRAS/nativePOV.ini \
        $$LPUB3D_STG_EXTRAS/PDFPrint.jpg \
        $$LPUB3D_STG_EXTRAS/pli.mpd \
        $$LPUB3D_STG_EXTRAS/pliSubstituteParts.lst \
        $$LPUB3D_STG_EXTRAS/povray.ini \
        $$LPUB3D_STG_EXTRAS/titleAnnotations.lst \
        $$LPUB3D_STG_EXTRAS/$$LDVMESSAGESINI
    lpub3d_extras.path = $$LPUB3D_STG_PATH/extras

    lpub3d_docs.files += \
        $$LPUB3D_STG_DOCS/COPYING_BRIEF \
        $$LPUB3D_STG_DOCS/CREDITS.txt \
        $$LPUB3D_STG_DOCS/README.txt
    lpub3d_docs.path = $$LPUB3D_STG_PATH/docs

    INSTALLS += \
        target \
        lpub3d_plugin_bearer \
        lpub3d_plugin_iconengines \
        lpub3d_plugin_imageformats \
        lpub3d_plugin_platforms \
        lpub3d_plugin_printsupport \
        lpub3d_plugin_styles \
        lpub3d_bins_qtlibs \
        lpub3d_bins_additional_libs \
        lpub3d_extras \
        lpub3d_docs

    # stage 3rd party executables, documents and resources

    stage3rdexe {
        # renderer executables
        CONFIG(release, debug|release) {
            release: message("~~~ RELEASE BUILD - 3RD PARTY EXEs STAGED TO: $$LPUB3D_STG_3RD_PATH ~~~")
            ldglite_stg.files += $$LDGLITE_INS_EXE
            ldglite_stg.path = $$LDGLITE_STG_DIR

            ldview_stg.files += $$LDVIEW_INS_EXE
            ldview_stg.path = $$LDVIEW_STG_DIR

            raytracer_stg.files += $$RAYTRACE_INS_EXE
            raytracer_stg.path = $$RAYTRACE_STG_DIR

            INSTALLS += \
            ldglite_stg \
            ldview_stg \
            raytracer_stg

        }
        CONFIG(debug, debug|release): message("~~~ 3RD PARTY EXECUTABLES WILL NOT BE STAGED ~~~")
    }

    stage3rdexeconfig {
        message("~~~ 3RD PARTY EXECUTABLE CONFIG FILES WILL BE STAGED ~~~")
        # renderer configuration
        raytracer_stg_resources_config.files += \
            $$RAYTRACE_INS_RES/config/$$QT_ARCH/povray.conf \
            $$RAYTRACE_INS_RES/config/$$QT_ARCH/povray.ini
        raytracer_stg_resources_config.path = $$RAYTRACE_STG_RES_DIR/config

        ldview_stg_resources_config.files += \
            $$LDVIEW_INS_RES/config/ldview.ini \
            $$LDVIEW_INS_RES/config/ldviewPOV.ini \
            $$LDVIEW_INS_RES/config/LDViewCustomini
        ldview_stg_resources_config.path = $$LDVIEW_STG_RES_DIR/config

        INSTALLS += \
        raytracer_stg_resources_config \
        ldview_stg_resources_config
    }

    stage3rdcontent {
        message("~~~ 3RD PARTY RESOURCES AND DOCUMENTATION WILL BE STAGED ~~~")
        # renderer content and documentation

        # ldglite content
        ldglite_stg_docs.files += \
            $$LDGLITE_INS_DOC/README.TXT \
            $$LDGLITE_INS_DOC/LICENCE
        ldglite_stg_docs.path = $$LDGLITE_STG_DOC_DIR

        ldglite_stg_resources.files += \
            $$LDGLITE_INS_RES/set-ldrawdir.command
        ldglite_stg_resources.path = $$LDGLITE_STG_RES_DIR

        # ldview contnet
        ldview_stg_docs.files += \
            $$LDVIEW_INS_DOC/ChangeHistory.html \
            $$LDVIEW_INS_DOC/Help.html \
            $$LDVIEW_INS_DOC/License.txt \
            $$LDVIEW_INS_DOC/Readme.txt
        ldview_stg_docs.path = $$LDVIEW_STG_DOC_DIR

        ldview_stg_resources.files += \
            $$LDVIEW_INS_RES/8464.mpd \
            $$LDVIEW_INS_RES/m6459.ldr \
            $$LDVIEW_INS_RES/LGEO.xml
        ldview_stg_resources.path = $$LDVIEW_STG_RES_DIR

        raytracer_stg_docs.files += \
            $$RAYTRACE_INS_DOC/help/povray37.chm \
            $$RAYTRACE_INS_DOC/AUTHORS.txt \
            $$RAYTRACE_INS_DOC/ChangeLog.txt \
            $$RAYTRACE_INS_DOC/CUI_README.txt \
            $$RAYTRACE_INS_DOC/LICENSE.txt
        raytracer_stg_docs.path = $$RAYTRACE_STG_DOC_DIR

        raytracer_stg_resources_ini.files += \
            $$RAYTRACE_INS_RES/ini/allanim.ini \
            $$RAYTRACE_INS_RES/ini/allstill.ini \
            $$RAYTRACE_INS_RES/ini/low.ini \
            $$RAYTRACE_INS_RES/ini/pngflc.ini \
            $$RAYTRACE_INS_RES/ini/pngfli.ini \
            $$RAYTRACE_INS_RES/ini/povray.ini \
            $$RAYTRACE_INS_RES/ini/quickres.ini \
            $$RAYTRACE_INS_RES/ini/res120.ini \
            $$RAYTRACE_INS_RES/ini/res1k.ini \
            $$RAYTRACE_INS_RES/ini/res320.ini \
            $$RAYTRACE_INS_RES/ini/res640.ini \
            $$RAYTRACE_INS_RES/ini/res800.ini \
            $$RAYTRACE_INS_RES/ini/slow.ini \
            $$RAYTRACE_INS_RES/ini/tgaflc.ini \
            $$RAYTRACE_INS_RES/ini/tgafli.ini \
            $$RAYTRACE_INS_RES/ini/zipflc.ini \
            $$RAYTRACE_INS_RES/ini/zipfli.ini
        raytracer_stg_resources_ini.path = $$RAYTRACE_STG_RES_DIR/ini

        raytracer_stg_resources_include.files += \
            $$RAYTRACE_INS_RES/include/arrays.inc \
            $$RAYTRACE_INS_RES/include/ash.map \
            $$RAYTRACE_INS_RES/include/benediti.map \
            $$RAYTRACE_INS_RES/include/bubinga.map \
            $$RAYTRACE_INS_RES/include/bumpmap_.png \
            $$RAYTRACE_INS_RES/include/cedar.map \
            $$RAYTRACE_INS_RES/include/chars.inc \
            $$RAYTRACE_INS_RES/include/colors.inc \
            $$RAYTRACE_INS_RES/include/colors_ral.inc \
            $$RAYTRACE_INS_RES/include/consts.inc \
            $$RAYTRACE_INS_RES/include/crystal.ttf \
            $$RAYTRACE_INS_RES/include/cyrvetic.ttf \
            $$RAYTRACE_INS_RES/include/debug.inc \
            $$RAYTRACE_INS_RES/include/finish.inc \
            $$RAYTRACE_INS_RES/include/fract003.png \
            $$RAYTRACE_INS_RES/include/functions.inc \
            $$RAYTRACE_INS_RES/include/glass.inc \
            $$RAYTRACE_INS_RES/include/glass_old.inc \
            $$RAYTRACE_INS_RES/include/golds.inc \
            $$RAYTRACE_INS_RES/include/ior.inc \
            $$RAYTRACE_INS_RES/include/logo.inc \
            $$RAYTRACE_INS_RES/include/makegrass.inc \
            $$RAYTRACE_INS_RES/include/marbteal.map \
            $$RAYTRACE_INS_RES/include/math.inc \
            $$RAYTRACE_INS_RES/include/meshmaker.inc \
            $$RAYTRACE_INS_RES/include/metals.inc \
            $$RAYTRACE_INS_RES/include/Mount1.png \
            $$RAYTRACE_INS_RES/include/Mount2.png \
            $$RAYTRACE_INS_RES/include/mtmand.pot \
            $$RAYTRACE_INS_RES/include/mtmandj.png \
            $$RAYTRACE_INS_RES/include/orngwood.map \
            $$RAYTRACE_INS_RES/include/pinkmarb.map \
            $$RAYTRACE_INS_RES/include/plasma2.png \
            $$RAYTRACE_INS_RES/include/plasma3.png \
            $$RAYTRACE_INS_RES/include/povlogo.ttf \
            $$RAYTRACE_INS_RES/include/povmap.png \
            $$RAYTRACE_INS_RES/include/rad_def.inc \
            $$RAYTRACE_INS_RES/include/rand.inc \
            $$RAYTRACE_INS_RES/include/rdgranit.map \
            $$RAYTRACE_INS_RES/include/screen.inc \
            $$RAYTRACE_INS_RES/include/shapes.inc \
            $$RAYTRACE_INS_RES/include/shapes2.inc \
            $$RAYTRACE_INS_RES/include/shapes3.inc \
            $$RAYTRACE_INS_RES/include/shapesq.inc \
            $$RAYTRACE_INS_RES/include/shapes_old.inc \
            $$RAYTRACE_INS_RES/include/skies.inc \
            $$RAYTRACE_INS_RES/include/spiral.df3 \
            $$RAYTRACE_INS_RES/include/stage1.inc \
            $$RAYTRACE_INS_RES/include/stars.inc \
            $$RAYTRACE_INS_RES/include/stdcam.inc \
            $$RAYTRACE_INS_RES/include/stdinc.inc \
            $$RAYTRACE_INS_RES/include/stoneold.inc \
            $$RAYTRACE_INS_RES/include/stones.inc \
            $$RAYTRACE_INS_RES/include/stones1.inc \
            $$RAYTRACE_INS_RES/include/stones2.inc \
            $$RAYTRACE_INS_RES/include/strings.inc \
            $$RAYTRACE_INS_RES/include/sunpos.inc \
            $$RAYTRACE_INS_RES/include/teak.map \
            $$RAYTRACE_INS_RES/include/test.png \
            $$RAYTRACE_INS_RES/include/textures.inc \
            $$RAYTRACE_INS_RES/include/timrom.ttf \
            $$RAYTRACE_INS_RES/include/transforms.inc \
            $$RAYTRACE_INS_RES/include/ttffonts.cat \
            $$RAYTRACE_INS_RES/include/whiteash.map \
            $$RAYTRACE_INS_RES/include/woodmaps.inc \
            $$RAYTRACE_INS_RES/include/woods.inc
        raytracer_stg_resources_include.path = $$RAYTRACE_STG_RES_DIR/include

        INSTALLS += \
        ldglite_stg_docs \
        ldglite_stg_resources \
        ldview_stg_docs \
        ldview_stg_resources \
        raytracer_stg_docs \
        raytracer_stg_resources_ini \
        raytracer_stg_resources_include
    }
}
