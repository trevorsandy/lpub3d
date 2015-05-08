#ifndef VERSION_H
#define VERSION_H

#include "build.h"

#define VER_MAJOR                   1
#define VER_MINOR                   0
#define VER_PATCH                   0
#define VER_TEXT                    "1.0.0"
#define VER_REVISION                _REVISION
#define VER_BUILD                   _BUILD

#define VER_FILEVERSION             1,0,0
#define VER_FILEVERSION_STR         "1.0.0\0"
#define VER_PRODUCTVERSION          1,0,0
#define VER_PRODUCTVERSION_STR      "1.0.0\0"

#define VER_COMPANYNAME_STR         "LPub Software"
#define VER_FILEDESCRIPTION_STR     "An LDraw Building Instruction Editor"
#define VER_INTERNALNAME_STR        "LegacyOne"
#define VER_LEGALCOPYRIGHT_STR      "Copyright (c) 2015 Trevor Sandy"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR    VER_LEGALTRADEMARKS1_STR
#define VER_ORIGINALFILENAME_STR    "LPub3D.exe"
#define VER_PRODUCTNAME_STR         "LPub3D"
#define VER_PUBLISHER_STR           "Trevor SANDY"
#define VER_PUBLISHER_EMAIL         "trevor.sandy@gmail.com"
#define VER_PUBLISHER_SUPPORT_EMAIL "mailto:trevor.sandy@gmail.com?subject=LPub3D " VER_TEXT ", Rev " _REVISION ", Build " _BUILD
//#define VER_UPDATE_CHECK_URL      "http://lpub3d.sourceforge.net/latest.txt"
#define VER_UPDATE_CHECK_URL        "http://www.leocad.org/updates.txt"
#define VER_UPDATE_URL              "http://lpub3d.sourceforge.net/"
#define VER_COMPANYDOMAIN_STR       "http://lpub3d.sourceforge.net"

#endif // VERSION_H
