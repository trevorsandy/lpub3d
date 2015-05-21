TEMPLATE=subdirs
CONFIG += ordered # This tells Qt to compile the following SUBDIRS in order
SUBDIRS += \
     quazip \
     qslog \
     mainApp
	 
quazip.subdir = $$PWD/quazip
quazip.target = sub-quazip
quazip.depends =

qslog.subdir = $$PWD/qslog
qslog.target = sub-qslog
qslog.depends =

mainApp.subdir = $$PWD/mainApp
mainApp.target = sub-mainApp
mainApp.depends = quazip
mainApp.depends = qslog

