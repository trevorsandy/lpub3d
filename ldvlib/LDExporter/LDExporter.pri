INCLUDEPATH += $$PWD $$PWD/../tinyxml
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/LDExporter.h \
    $$PWD/LDExporterSetting.h \
    $$PWD/LDPovExporter.h

SOURCES += \
    $$PWD/LDExporter.cpp \
    $$PWD/LDExporterSetting.cpp \
    $$PWD/LDPovExporter.cpp

OTHER_FILES += \
    $$PWD/LDExportMessages.ini \
    $$PWD/LGEO.xml
