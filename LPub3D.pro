TEMPLATE=subdirs
CONFIG += ordered # This tells Qt to compile the following SUBDIRS in order
SUBDIRS += \
     $$PWD/quazip \
     $$PWD/ldrawini \
     $$PWD/ldglite \
     $$PWD/mainApp
	 
quazip.subdir = $$PWD/quazip
quazip.target = sub-quazip
quazip.depends =

ldrawini.subdir = $$PWD/ldrawini
ldrawini.target = sub-ldrawini
ldrawini.depends =

ldglite.subdir = $$PWD/ldglite
ldglite.target = sub-ldglite
ldglite.depends =

mainApp.subdir = $$PWD/mainApp
mainApp.target = sub-mainApp
mainApp.depends = quazip
mainApp.depends = ldrawini
mainApp.depends = ldglite

RESOURCES += \
    qsimpleupdater/res/qsu_resources.qrc \
    mainApp/lpub3d.qrc

CONFIG(debug, debug|release) {
    message("~~~ LPUB3D DEBUG build ~~~")
} else {
    message("~~~ LPUB3D RELEASE build ~~~")
}
