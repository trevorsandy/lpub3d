/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef VERSION_H
#define VERSION_H
// ~~~~~ local includes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
#include "build.h"

// ~~~~~ predefines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
#define STR(x)   #x
#define STRING(x)  STR(x)

// ~~~~~ Version info ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
#define VER_MAJOR                           2
#define VER_MINOR                           0
#define VER_SP                              2

#define VER_BUILD_STR                       _BUILD_NUMBER
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
#define VER_LEGALCOPYRIGHT_STR      		"Copyright &copy; 2016 by Trevor SANDY"
#define VER_LEGALTRADEMARKS1_STR    		"All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR    		VER_LEGALTRADEMARKS1_STR
#define VER_ORIGINALFILENAME_STR    		"LPub3D.exe"
#define VER_PUBLISHER_STR           		"Trevor SANDY"
#define VER_PUBLISHER_EMAIL_STR     		"trevor.sandy@gmail.com"
#define VER_PUBLISHER_SUPPORT_EMAIL_STR		"mailto:trevor.sandy@gmail.com?subject=LPub3D Version " VER_FILEVERSION_STR

//obsolete
#define VER_UPDATE_CHECK_URL        		"http://lpub3d.sourceforge.net/latest.txt"
#define VER_DOWNLOAD_URL                    "http://lpub3d.sourceforge.net/" VER_PRODUCTNAME_STR "-UpdateMaster.exe"

#define VER_LDRAW_OFFICIAL_ARCHIVE          "complete.zip"
#define VER_LDRAW_UNOFFICIAL_ARCHIVE        "ldrawunf.zip"
#define VER_LPUB3D_UNOFFICIAL_ARCHIVE       "lpub3dldrawunf.zip"
#define VER_UPDATE_CHECK_JSON_URL        	"http://lpub3d.sourceforge.net/lpub3dupdates.json"
#define VER_OFFICIAL_LIBRARY_JSON_URL       "http://lpub3d.sourceforge.net/complete.json"
#define VER_UNOFFICIAL_LIBRARY_JSON_URL     "http://lpub3d.sourceforge.net/lpub3dldrawunf.json"
//#define VER_OFFICIAL_LIBRARY_JSON_URL       "http://lpub3d.sourceforge.net/testfile01.json"
//#define VER_UNOFFICIAL_LIBRARY_JSON_URL     "http://lpub3d.sourceforge.net/testfile02.json"
#define VER_CHANGE_LOG_URL                  "http://lpub3d.sourceforge.net/change_log.txt"

#define VER_SOURCE_URL              		"http://sourceforge.net/p/lpub3d/code/"
#define VER_COMPANYDOMAIN_STR       		"http://sourceforge.net/projects/lpub3d/"

#ifdef WIN32
#ifdef _MSC_VER
#define VER_COMPILED_ON             		"MSVC 2015"
#define VER_COMPILED_FOR            		"MS Windows 32 bit, 64 bit"
#define VER_COMPILED_WITH                   "Qt 5.6.1 (MSVC 2015 x86, x64)"
#define VER_IDE                             "Visual Studio 2015 v14.0.25123.00 Update 2"
#else
#define VER_COMPILED_ON             		"MinGW (i686-4.9.2 32bit) Windows"
#define VER_COMPILED_FOR            		"MS Windows 32 bit, 64 bit"
#define VER_COMPILED_WITH                   "Qt 5.5.1 (MinGW 32bit)"
#define VER_IDE                             "Qt Creator 4.0.1 on Qt 5.6.1 (MSVC 2013, x32)"
#endif
#else
 //MAC/Linux stuff
#endif

#endif // VERSION_H
