# install 3rd party executables, documents and resources

copy3rdexe {

    # renderer executables
    CONFIG(release, debug|release) {
            ldglite_ins.files += $$LDGLITE_INS_EXE
            ldglite_ins.path = $$LDGLITE_INS_DIR

            ldview_ins.files += $$LDVIEW_INS_EXE
            ldview_ins.path = $$LDVIEW_INS_DIR

            raytracer_ins.files += $$RAYTRACE_INS_EXE
            raytracer_ins.path = $$RAYTRACE_INS_DIR

            INSTALLS += \
            ldglite_ins \
            ldview_ins \
            raytracer_ins

            message("~~~ INSTALL LDGLITE_INS_EXE $$join(ARCH,,,bit) ~~~")
            message("~~~ INSTALL LDVIEW_INS_EXE $$join(ARCH,,,bit) ~~~")
            message("~~~ INSTALL RAYTRACE_INS_EXE $$join(ARCH,,,bit) ~~~")
    }
    CONFIG(debug, debug|release) {
            message("~~~ DEBUG BUILD - 3RD PARTY EXE'S NOT INSTALLED ~~~")
    }
}

copy3rdfiles {

    # ldglite content
    ldglite_ins_docs.files += \
             $$LDGLITE_INS_DOC/README.TXT \
             $$LDGLITE_INS_DOC/LICENCE
    ldglite_ins_docs.path = $$LDGLITE_INS_DOC_DIR

    ldglite_ins_resources.files += \
             $$LDGLITE_INS_RES/set-ldrawdir.command
    ldglite_ins_resources.path = $$LDGLITE_INS_RES_DIR

    # ldview contnet
    ldview_ins_docs.files += \
             $$LDVIEW_INS_DOC/ChangeHistory.html \
             $$LDVIEW_INS_DOC/Help.html \
             $$LDVIEW_INS_DOC/License.txt \
             $$LDVIEW_INS_DOC/Readme.txt
    ldview_ins_docs.path = $$LDVIEW_INS_DOC_DIR

    ldview_ins_resources.files += \
            $$LDVIEW_INS_RES/LDViewCustomIni \
            $$LDVIEW_INS_RES/8464.mpd \
            $$LDVIEW_INS_RES/m6459.ldr
    ldview_ins_resources.path = $$LDVIEW_INS_RES_DIR

    # raytracer (povray) content
    raytracer_ins_docs.files += \
            $$RAYTRACE_INS_DOC/help/povray37.chm \
            $$RAYTRACE_INS_DOC/AUTHORS.txt \
            $$RAYTRACE_INS_DOC/ChangeLog.txt \
            $$RAYTRACE_INS_DOC/CUI_README.txt \
            $$RAYTRACE_INS_DOC/LICENSE.txt
    raytracer_ins_docs.path = $$RAYTRACE_INS_DOC_DIR

    raytracer_resources_conf.files += \
            $$RAYTRACE_INS_RES/conf/povray.conf \
            $$RAYTRACE_INS_RES/conf/povray.ini
    raytracer_ins_resources_conf.path = $$RAYTRACE_INS_RES_DIR\conf

    raytracer_ins_resources_ini.files += \
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
    raytracer_ins_resources_ini.path = $$RAYTRACE_INS_RES_DIR\ini

    raytracer_ins_resources_include.files += \
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
    raytracer_ins_resources_include.path = $$RAYTRACE_INS_RES_DIR\include

    INSTALLS += \
    ldglite_ins_docs \
    ldglite_ins_resources \
    ldview_ins_docs \
    ldview_ins_resources \
    raytracer_ins_docs \
    raytracer_ins_resources_conf \
    raytracer_ins_resources_ini \
    raytracer_ins_resources_include
}
