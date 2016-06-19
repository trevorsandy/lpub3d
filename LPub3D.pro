TEMPLATE=subdirs
CONFIG += ordered # This tells Qt to compile the following SUBDIRS in order
SUBDIRS += \
     $$PWD/quazip \
     $$PWD/ldrawini \
     $$PWD/mainApp
	 
quazip.subdir = $$PWD/quazip
quazip.target = sub-quazip
quazip.depends =

ldrawini.subdir = $$PWD/ldrawini
ldrawini.target = sub-ldrawini
ldrawini.depends =

mainApp.subdir = $$PWD/mainApp
mainApp.target = sub-mainApp
mainApp.depends = quazip
mainApp.depends = ldrawini

RESOURCES += \
    res/su_resources.qrc

