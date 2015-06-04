#ifndef VERSION_H
#define VERSION_H

// ~~~~~ local includes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
#include "build.h"

// ~~~~~ predefines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
#define STR(x)   #x
#define STRING(x)  STR(x)

// ~~~~~ Version info ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
#define VER_MAJOR                   		1
#define VER_MINOR                   		0
#define VER_SVC_PACK                		0
#define VER_BUILDINCVERSION_STR     		"1.0.0"

#define VER_BUILD                   		_BUILD_NUMBER
#define VER_REVISION                		_BUILD_REVISION

#define VER_FILEVERSION             		STRING(VER_MAJOR),STRING(VER_MINOR),STRING(VER_SVC_PACK),VER_REVISION,VER_BUILD
#define VER_FILEVERSION_STR         		STRING(VER_MAJOR) "." STRING(VER_MINOR) "." STRING(VER_SVC_PACK) "." VER_REVISION "." VER_BUILD "\0"
#define VER_PRODUCTVERSION          		STRING(VER_MAJOR),STRING(VER_MINOR),STRING(VER_SVC_PACK)
#define VER_PRODUCTVERSION_STR      		STRING(VER_MAJOR) "." STRING(VER_MINOR) "." STRING(VER_SVC_PACK) "\0"

#define VER_COMPANYNAME_STR         		"LPub Software"
#define VER_FILEDESCRIPTION_STR     		"LPub3D - An LDraw Building Instruction Editor"
#define VER_INTERNALNAME_STR        		"LegacyOne"
#define VER_LEGALCOPYRIGHT_STR      		"Copyright (c) 2015 by Trevor Sandy"
#define VER_LEGALTRADEMARKS1_STR    		"All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR    		VER_LEGALTRADEMARKS1_STR
#define VER_ORIGINALFILENAME_STR    		"LPub3D.exe"
#define VER_PRODUCTNAME_STR         		"LPub3D"
#define VER_PUBLISHER_STR           		"Trevor SANDY"
#define VER_PUBLISHER_EMAIL_STR     		"trevor.sandy@gmail.com"
#define VER_PUBLISHER_SUPPORT_EMAIL_STR		"mailto:trevor.sandy@gmail.com?subject=LPub3D Version " VER_FILEVERSION_STR
#define VER_UPDATE_CHECK_URL        		"http://lpub3d.sourceforge.net/latest.txt"
//#define VER_UPDATE_CHECK_URL      		 "http://www.leocad.org/updates.txt"
#define VER_UPDATE_URL              		"http://lpub3d.sourceforge.net/"
#define VER_COMPANYDOMAIN_STR       		"http://lpub3d.sourceforge.net"

#define VER_COMPILED_ON             		"x86_64-w64-mingw64"
#define VER_COMPILED_FOR            		"x86_64-w64-mingw64"

#define VER_OPERATING_SYSTEM        		"Windows 8.1 (build 9600), 64-bit edition"
#define VER_OS_VERSION              		"6.3"
#define VER_PLATFORM                		"64-bit system"
#define VER_LIBRARY                 		"Qt 4.8.6"

#endif // VERSION_H
