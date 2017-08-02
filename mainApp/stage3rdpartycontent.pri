# Stage 3rd party executables, documentation and resources as source files in the LPub3D source.

# source directories
isEmpty(LDGLITE_DOC):LDGLITE_DOC   = $$THIRD_PARTY_SRC/docs/ldglite-1.3
isEmpty(LDVIEW_DOC):LDVIEW_DOC     = $$THIRD_PARTY_SRC/docs/LDView-4.3
isEmpty(RAYTRACE_DOC):RAYTRACE_DOC = $$THIRD_PARTY_SRC/docs/lpub3d_trace_cui-3.7.2

isEmpty(LDGLITE_RES):LDGLITE_RES   = $$THIRD_PARTY_SRC/resources/ldglite-1.3
isEmpty(LDVIEW_RES):LDVIEW_RES     = $$THIRD_PARTY_SRC/resources/LDView-4.3
isEmpty(RAYTRACE_RES):RAYTRACE_RES = $$THIRD_PARTY_SRC/resources/lpub3d_trace_cui-3.7.2

# stage directories
isEmpty(BIN_DIR):BIN_DIR                 = $$THIRD_PARTY_STG/bin
isEmpty(DOCS_DIR):DOCS_DIR               = $$THIRD_PARTY_STG/docs
isEmpty(RESOURCE_DIR):RESOURCE_DIR       = $$THIRD_PARTY_STG/resources

isEmpty(LDGLITE_DIR):LDGLITE_DIR           = $$BIN_DIR/ldglite-1.3/$$QT_ARCH
isEmpty(LDGLITE_DOC_DIR):LDGLITE_DOC_DIR   = $$DOCS_DIR/ldglite-1.3
isEmpty(LDGLITE_RES_DIR):LDGLITE_RES_DIR   = $$RESOURCE_DIR/ldglite-1.3
isEmpty(LDVIEW_DIR):LDVIEW_DIR             = $$BIN_DIR/ldview-4.3/$$QT_ARCH
isEmpty(LDVIEW_DOC_DIR):LDVIEW_DOC_DIR     = $$DOCS_DIR/ldview-4.3
isEmpty(LDVIEW_RES_DIR):LDVIEW_RES_DIR     = $$RESOURCE_DIR/ldview-4.3
isEmpty(RAYTRACE_DIR):RAYTRACE_DIR         = $$BIN_DIR/lpub3d_trace_cui-3.7.2/$$QT_ARCH
isEmpty(RAYTRACE_DOC_DIR):RAYTRACE_DOC_DIR = $$DOCS_DIR/lpub3d_trace_cui-3.7.2
isEmpty(RAYTRACE_RES_DIR):RAYTRACE_RES_DIR = $$RESOURCE_DIR/lpub3d_trace_cui-3.7.2

# renderer executables
CONFIG(release, debug|release) {
        ldglite.files += $$LDGLITE_EXE
        ldglite.path = $$LDGLITE_DIR

        ldview.files += $$LDVIEW_EXE
        ldview.path = $$LDVIEW_DIR

        raytracer.files += $$RAYTRACE_EXE
        raytracer.path = $$RAYTRACE_DIR

        INSTALLS += \
        ldglite \
        ldview \
        raytracer

        message("~~~ STAGE LDGLITE_EXE $$join(ARCH,,,bit) ~~~")
        message("~~~ STAGE LDVIEW_EXE $$join(ARCH,,,bit) ~~~")
        message("~~~ STAGE RAYTRACE_EXE $$join(ARCH,,,bit) ~~~")
}
CONFIG(debug, debug|release) {
        message("~~~ DEBUG BUILD - 3RD PARTY EXE'S NOT STAGED ~~~")
}

# ldglite content
ldglite_docs.files += \
         $$LDGLITE_DOC/README.TXT \
         $$LDGLITE_DOC/LICENCE
ldglite_docs.path = $$LDGLITE_DOC_DIR

ldglite_resources.files += \
         $$LDGLITE_RES/set-ldrawdir.command
ldglite_resources.path = $$LDGLITE_RES_DIR

# ldview contnet
ldview_docs.files += \
         $$LDVIEW_DOC/ChangeHistory.html \
         $$LDVIEW_DOC/Help.html \
         $$LDVIEW_DOC/License.txt \
         $$LDVIEW_DOC/Readme.txt
ldview_docs.path = $$LDVIEW_DOC_DIR

ldview_resources.files += \
        $$LDVIEW_RES/LDViewCustomIni \
        $$LDVIEW_RES/8464.mpd \
        $$LDVIEW_RES/m6459.ldr
ldview_resources.path = $$LDVIEW_RES_DIR

# raytracer (povray) content
raytracer_docs.files += \
        $$RAYTRACE_DOC/help/povray37.chm \
        $$RAYTRACE_DOC/AUTHORS.txt \
        $$RAYTRACE_DOC/ChangeLog.txt \
        $$RAYTRACE_DOC/CUI_README.txt \
        $$RAYTRACE_DOC/LICENSE.txt
raytracer_docs.path = $$RAYTRACE_DOC_DIR

raytracer_resources_conf.files += \
        $$RAYTRACE_RES/conf/povray.conf \
        $$RAYTRACE_RES/conf/povray.ini
raytracer_resources_conf.path = $$RAYTRACE_RES_DIR\conf

raytracer_resources_ini.files += \
        $$RAYTRACE_RES/ini/allanim.ini \
        $$RAYTRACE_RES/ini/allstill.ini \
        $$RAYTRACE_RES/ini/low.ini \
        $$RAYTRACE_RES/ini/pngflc.ini \
        $$RAYTRACE_RES/ini/pngfli.ini \
        $$RAYTRACE_RES/ini/povray.ini \
        $$RAYTRACE_RES/ini/quickres.ini \
        $$RAYTRACE_RES/ini/res120.ini \
        $$RAYTRACE_RES/ini/res1k.ini \
        $$RAYTRACE_RES/ini/res320.ini \
        $$RAYTRACE_RES/ini/res640.ini \
        $$RAYTRACE_RES/ini/res800.ini \
        $$RAYTRACE_RES/ini/slow.ini \
        $$RAYTRACE_RES/ini/tgaflc.ini \
        $$RAYTRACE_RES/ini/tgafli.ini \
        $$RAYTRACE_RES/ini/zipflc.ini \
        $$RAYTRACE_RES/ini/zipfli.ini
raytracer_resources_ini.path = $$RAYTRACE_RES_DIR\ini

raytracer_resources_include.files += \
        $$RAYTRACE_RES/include/arrays.inc \
        $$RAYTRACE_RES/include/ash.map \
        $$RAYTRACE_RES/include/benediti.map \
        $$RAYTRACE_RES/include/bubinga.map \
        $$RAYTRACE_RES/include/bumpmap_.png \
        $$RAYTRACE_RES/include/cedar.map \
        $$RAYTRACE_RES/include/chars.inc \
        $$RAYTRACE_RES/include/colors.inc \
        $$RAYTRACE_RES/include/colors_ral.inc \
        $$RAYTRACE_RES/include/consts.inc \
        $$RAYTRACE_RES/include/crystal.ttf \
        $$RAYTRACE_RES/include/cyrvetic.ttf \
        $$RAYTRACE_RES/include/debug.inc \
        $$RAYTRACE_RES/include/finish.inc \
        $$RAYTRACE_RES/include/fract003.png \
        $$RAYTRACE_RES/include/functions.inc \
        $$RAYTRACE_RES/include/glass.inc \
        $$RAYTRACE_RES/include/glass_old.inc \
        $$RAYTRACE_RES/include/golds.inc \
        $$RAYTRACE_RES/include/ior.inc \
        $$RAYTRACE_RES/include/logo.inc \
        $$RAYTRACE_RES/include/makegrass.inc \
        $$RAYTRACE_RES/include/marbteal.map \
        $$RAYTRACE_RES/include/math.inc \
        $$RAYTRACE_RES/include/meshmaker.inc \
        $$RAYTRACE_RES/include/metals.inc \
        $$RAYTRACE_RES/include/Mount1.png \
        $$RAYTRACE_RES/include/Mount2.png \
        $$RAYTRACE_RES/include/mtmand.pot \
        $$RAYTRACE_RES/include/mtmandj.png \
        $$RAYTRACE_RES/include/orngwood.map \
        $$RAYTRACE_RES/include/pinkmarb.map \
        $$RAYTRACE_RES/include/plasma2.png \
        $$RAYTRACE_RES/include/plasma3.png \
        $$RAYTRACE_RES/include/povlogo.ttf \
        $$RAYTRACE_RES/include/povmap.png \
        $$RAYTRACE_RES/include/rad_def.inc \
        $$RAYTRACE_RES/include/rand.inc \
        $$RAYTRACE_RES/include/rdgranit.map \
        $$RAYTRACE_RES/include/screen.inc \
        $$RAYTRACE_RES/include/shapes.inc \
        $$RAYTRACE_RES/include/shapes2.inc \
        $$RAYTRACE_RES/include/shapes3.inc \
        $$RAYTRACE_RES/include/shapesq.inc \
        $$RAYTRACE_RES/include/shapes_old.inc \
        $$RAYTRACE_RES/include/skies.inc \
        $$RAYTRACE_RES/include/spiral.df3 \
        $$RAYTRACE_RES/include/stage1.inc \
        $$RAYTRACE_RES/include/stars.inc \
        $$RAYTRACE_RES/include/stdcam.inc \
        $$RAYTRACE_RES/include/stdinc.inc \
        $$RAYTRACE_RES/include/stoneold.inc \
        $$RAYTRACE_RES/include/stones.inc \
        $$RAYTRACE_RES/include/stones1.inc \
        $$RAYTRACE_RES/include/stones2.inc \
        $$RAYTRACE_RES/include/strings.inc \
        $$RAYTRACE_RES/include/sunpos.inc \
        $$RAYTRACE_RES/include/teak.map \
        $$RAYTRACE_RES/include/test.png \
        $$RAYTRACE_RES/include/textures.inc \
        $$RAYTRACE_RES/include/timrom.ttf \
        $$RAYTRACE_RES/include/transforms.inc \
        $$RAYTRACE_RES/include/ttffonts.cat \
        $$RAYTRACE_RES/include/whiteash.map \
        $$RAYTRACE_RES/include/woodmaps.inc \
        $$RAYTRACE_RES/include/woods.inc
raytracer_resources_include.path = $$RAYTRACE_RES_DIR\include

INSTALLS += \
ldglite_docs \
ldglite_resources \
ldview_docs \
ldview_resources \
raytracer_docs \
raytracer_resources_conf \
raytracer_resources_ini \
raytracer_resources_include

