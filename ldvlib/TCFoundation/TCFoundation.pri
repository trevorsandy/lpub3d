INCLUDEPATH += $$PWD $$PWD/../libpng $$PWD/../libjpeg
DEPENDPATH += $$PWD

include(../LDVQt/LDVQt.pri)
include(../stb/stb.pri)

HEADERS += \
	$$PWD/ConvertUTF.h \
	$$PWD/mystring.h \
	$$PWD/TCAlert.h \
	$$PWD/TCAlertManager.h \
	$$PWD/TCAlertSender.h \
	$$PWD/TCArray.h \
	$$PWD/TCAutoreleasePool.h \
	$$PWD/TCBmpImageFormat.h \
	$$PWD/TCDefines.h \
	$$PWD/TCDictionary.h \
	$$PWD/TCImage.h \
	$$PWD/TCImageFormat.h \
	$$PWD/TCImageOptions.h \
	$$PWD/TCJpegImageFormat.h \
	$$PWD/TCJpegOptions.h \
	$$PWD/TCLocalStrings.h \
	$$PWD/TCMacros.h \
	$$PWD/TCObject.h \
	$$PWD/TCObjectArray.h \
	$$PWD/TCPngImageFormat.h \
	$$PWD/TCProgressAlert.h \
	$$PWD/TCSortedStringArray.h \
	$$PWD/TCStlIncludes.h \
	$$PWD/TCStringArray.h \
	$$PWD/TCTypedDictionary.h \
	$$PWD/TCTypedObjectArray.h \
	$$PWD/TCTypedPointerArray.h \
	$$PWD/TCTypedValueArray.h \
	$$PWD/TCUserDefaults.h \
    $$PWD/TCVector.h

SOURCES += \
	$$PWD/ConvertUTF.c \
    $$PWD/mystring.cpp \
	$$PWD/TCAlert.cpp \
	$$PWD/TCAlertManager.cpp \
	$$PWD/TCArray.cpp \
	$$PWD/TCAutoreleasePool.cpp \
	$$PWD/TCBmpImageFormat.cpp \
	$$PWD/TCDictionary.cpp \
	$$PWD/TCImage.cpp \
	$$PWD/TCImageFormat.cpp \
	$$PWD/TCImageOptions.cpp \
	$$PWD/TCJpegImageFormat.cpp \
	$$PWD/TCJpegOptions.cpp \
	$$PWD/TCLocalStrings.cpp \
	$$PWD/TCObject.cpp \
	$$PWD/TCObjectArray.cpp \
	$$PWD/TCPngImageFormat.cpp \
	$$PWD/TCProgressAlert.cpp \
	$$PWD/TCSortedStringArray.cpp \
	$$PWD/TCStringArray.cpp \
	$$PWD/TCUserDefaults.cpp \
    $$PWD/TCVector.cpp

