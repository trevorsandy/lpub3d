#ifndef VERSION_H
#define VERSION_H

// ~~~~~ local includes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
#include "build.h"

// ~~~~~ predefines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
#define STR(x)   #x
#define STRING(x)  STR(x)

// ~~~~~ Version info ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
#define VER_MAJOR                           1
#define VER_MINOR                           0
#define VER_SP                              0

#define VER_BUILD_STR                   	_BUILD_NUMBER
#define VER_REVISION_STR                    _BUILD_REVISION
#define VER_BUILDDATE_STR                   _BUILD_DATE

#define VER_FILEVERSION             		VER_MAJOR,VER_MINOR,VER_SP,VER_REVISION_STR,VER_BUILD_STR
#define VER_FILEVERSION_STR         		STRING(VER_MAJOR) "." STRING(VER_MINOR) "." STRING(VER_SP) "." VER_REVISION_STR "." VER_BUILD_STR "\0"
#define VER_PRODUCTVERSION          		VER_MAJOR,VER_MINOR,VER_SP
#define VER_PRODUCTVERSION_STR      		STRING(VER_MAJOR) "." STRING(VER_MINOR) "." STRING(VER_SP) "\0"

#define VER_PRODUCTNAME_STR         		"LPub3D"
#define VER_COMPANYNAME_STR         		"LPub3D Software"
#define VER_FILEDESCRIPTION_STR     		"LPub3D - An LDraw Building Instruction Editor"
#define VER_INTERNALNAME_STR        		"bociphus"
#define VER_LEGALCOPYRIGHT_STR      		"Copyright &copy; 2015 by Trevor SANDY"
#define VER_LEGALTRADEMARKS1_STR    		"All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR    		VER_LEGALTRADEMARKS1_STR
#define VER_ORIGINALFILENAME_STR    		"LPub3D.exe"
#define VER_PUBLISHER_STR           		"Trevor SANDY"
#define VER_PUBLISHER_EMAIL_STR     		"trevor.sandy@gmail.com"
#define VER_PUBLISHER_SUPPORT_EMAIL_STR		"mailto:trevor.sandy@gmail.com?subject=LPub3D Version " VER_FILEVERSION_STR
#define VER_UPDATE_CHECK_URL        		"http://lpub3d.sourceforge.net/latest.txt"
#define VER_CHANGE_LOG_URL                  "http://lpub3d.sourceforge.net/change_log.txt"
#define VER_DOWNLOAD_URL                    "http://lpub3d.sourceforge.net/" VER_PRODUCTNAME_STR "-UpdateMaster.exe"
#define VER_SOURCE_URL              		"https://sourceforge.net/p/lpub3d/code/ref/master/"
#define VER_COMPANYDOMAIN_STR       		"https://sourceforge.net/projects/lpub3d/"

#define VER_COMPILED_ON             		"MinGW (i686-4.9.2-x64) Windows"
#define VER_COMPILED_FOR            		"x86_64-w64-mingw32"
#define VER_COMPILED_WITH                   "x86_64-w64-mingw32-g++ (posix-sjlj-rev1) 4.9.2"
#define VER_IDE                             "Qt Creator 3.5 b1 on Qt 5.5.0 (MSVC 2013 x32)"

#endif // VERSION_H
