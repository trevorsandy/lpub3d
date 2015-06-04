#ifndef VERSION_H
#define VERSION_H

#include "build.h"


#define VER_REVISION                _REVISION
#define VER_BUILD                   _BUILD
#define VER_COMPILED_ON             "x86_64-w64-mingw64"
#define VER_COMPILED_FOR            "x86_64-w64-mingw64"

#define VER_MAJOR                   1
#define VER_MINOR                   0
#define VER_PATCH                   0
#define VER_TEXT                    "1.0.0"
#define VER_FILEVERSION             1,0,0
#define VER_FILEVERSION_STR         "1.0.0\0"
#define VER_PRODUCTVERSION          1,0,0
#define VER_PRODUCTVERSION_STR      "1.0.0\0"

#define VER_COMPANYNAME_STR         "LPub Software"
#define VER_FILEDESCRIPTION_STR     "LPub3D - An LDraw Building Instruction Editor"
#define VER_INTERNALNAME_STR        "LegacyOne"
#define VER_LEGALCOPYRIGHT_STR      "Copyright (c) 2015 Trevor Sandy"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR    VER_LEGALTRADEMARKS1_STR
#define VER_ORIGINALFILENAME_STR    "LPub3D.exe"
#define VER_PRODUCTNAME_STR         "LPub3D"
#define VER_PUBLISHER_STR           "Trevor SANDY"
#define VER_PUBLISHER_EMAIL         "trevor.sandy@gmail.com"
#define VER_PUBLISHER_SUPPORT_EMAIL "mailto:trevor.sandy@gmail.com?subject=LPub3D " VER_TEXT ", Rev " _REVISION ", Build " _BUILD
#define VER_UPDATE_CHECK_URL        "http://lpub3d.sourceforge.net/latest.txt"
//#define VER_UPDATE_CHECK_URL       "http://www.leocad.org/updates.txt"
#define VER_UPDATE_URL              "http://lpub3d.sourceforge.net/"
#define VER_COMPANYDOMAIN_STR       "http://lpub3d.sourceforge.net"

#define VER_OPERATING_SYSTEM        "Windows 8.1 (build 9600), 64-bit edition"
#define VER_OS_VERSION              "6.3"
#define VER_PLATFORM                "64-bit system"
#define VER_LIBRARY                 "Qt 4.8.6"


// Example of __DATE__ string: "Jul 27 2012"
// Example of __TIME__ string: "21:06:19"

#define COMPUTE_BUILD_YEAR \
    ( \
        (__DATE__[ 7] - '0') * 1000 + \
        (__DATE__[ 8] - '0') *  100 + \
        (__DATE__[ 9] - '0') *   10 + \
        (__DATE__[10] - '0') \
    )

#define COMPUTE_BUILD_DAY \
    ( \
        ((__DATE__[4] >= '0') ? (__DATE__[4] - '0') * 10 : 0) + \
        (__DATE__[5] - '0') \
    )

#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')


#define COMPUTE_BUILD_MONTH \
    ( \
        (BUILD_MONTH_IS_JAN) ?  1 : \
        (BUILD_MONTH_IS_FEB) ?  2 : \
        (BUILD_MONTH_IS_MAR) ?  3 : \
        (BUILD_MONTH_IS_APR) ?  4 : \
        (BUILD_MONTH_IS_MAY) ?  5 : \
        (BUILD_MONTH_IS_JUN) ?  6 : \
        (BUILD_MONTH_IS_JUL) ?  7 : \
        (BUILD_MONTH_IS_AUG) ?  8 : \
        (BUILD_MONTH_IS_SEP) ?  9 : \
        (BUILD_MONTH_IS_OCT) ? 10 : \
        (BUILD_MONTH_IS_NOV) ? 11 : \
        (BUILD_MONTH_IS_DEC) ? 12 : \
        /* error default */  99 \
    )

#define COMPUTE_BUILD_HOUR ((__TIME__[0] - '0') * 10 + __TIME__[1] - '0')
#define COMPUTE_BUILD_MIN  ((__TIME__[3] - '0') * 10 + __TIME__[4] - '0')
#define COMPUTE_BUILD_SEC  ((__TIME__[6] - '0') * 10 + __TIME__[7] - '0')


#define BUILD_DATE_IS_BAD (__DATE__[0] == '?')

#define BUILD_YEAR  ((BUILD_DATE_IS_BAD) ? 99 : COMPUTE_BUILD_YEAR)
#define BUILD_MONTH ((BUILD_DATE_IS_BAD) ? 99 : COMPUTE_BUILD_MONTH)
#define BUILD_DAY   ((BUILD_DATE_IS_BAD) ? 99 : COMPUTE_BUILD_DAY)

#define BUILD_TIME_IS_BAD (__TIME__[0] == '?')

#define BUILD_HOUR  ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_HOUR)
#define BUILD_MIN   ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_MIN)
#define BUILD_SEC   ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_SEC)

const unsigned char completeVersion[] =
{
    VER_MAJOR,
    '.',
    VER_MINOR,
    '.',
    VER_PATCH,
    '_',
    static_cast<unsigned char>BUILD_YEAR,
    '-',
    BUILD_MONTH,
    '-',
    BUILD_DAY,
    '_',
    BUILD_HOUR,
    '-',
    BUILD_MIN,
    '-',
    BUILD_SEC,
    '\0'
};

#endif // VERSION_H
