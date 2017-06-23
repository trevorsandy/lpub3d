/****************************************************************************
**
** Copyright (C) 2015 - 2017 Trevor SANDY. All rights reserved.
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

// ~~~~~ predefines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
#define STR(x)   #x
#define STRING(x)  STR(x)

// ~~~~~ Generic Version info ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#define VER_COMPILE_DATE_STR                DATE_YY " " DATE_MM " " DATE_DD " " BUILD_TIME
#define VER_PRODUCTVERSION_STR              STRING(VER_MAJOR) "." STRING(VER_MINOR) "." STRING(VER_PATCH)
#define VER_PRODUCTNAME_STR                 "LPub3D"
#define VER_COMPANYNAME_BLD_STR             "LPub3D Software"
#define VER_FILEDESCRIPTION_STR             "LPub3D - An LDraw Building Instruction Editor"
#define VER_LEGALCOPYRIGHT_STR              "Copyright &copy; 2015 - 2017 by Trevor SANDY"
#define VER_LEGALTRADEMARKS1_STR            "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR            VER_LEGALTRADEMARKS1_STR
#define VER_PUBLISHER_STR                   "Trevor SANDY"
#define VER_PUBLISHER_EMAIL_STR             "trevor.sandy@gmail.com"
#define VER_FILEVERSION_STR                 STRING(VER_MAJOR) "." STRING(VER_MINOR) "." STRING(VER_PATCH) "." VER_REVISION_STR "." VER_BUILD_STR
#define VER_PUBLISHER_SUPPORT_EMAIL_STR     "mailto:trevor.sandy@gmail.com?subject=LPub3D Version " VER_FILEVERSION_STR
#ifdef QT_DEBUG_MODE
  #define COMPANYNAME_STR                   "LPub3D Software Maint"
#else
  #define COMPANYNAME_STR                   "LPub3D Software"
#endif
#define VER_COMPANYNAME_STR                 COMPANYNAME_STR

// ~~~~~~ Parameters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#define VER_LOGGING_LEVELS_STR              "STATUS,INFO,TRACE,DEBUG,NOTICE,ERROR,FATAL,OFF"

#define VER_FADESTEP_COLORPARTS_FILE        "fadeStepColorParts.lst"
#define VER_FREEFOM_ANNOTATIONS_FILE        "freeformAnnotations.lst"
#define VER_EXTRAS_LDCONFIG_FILE            "LDConfig.ldr"
#define VER_PDFPRINT_IMAGE_FILE             "PDFPrint.jpg"
#define VER_PLI_MPD_FILE                    "pli.mpd"
#define VER_PLI_SUBSTITUTE_PARTS_FILE       "pliSubstituteParts.lst"
#define VER_TITLE_ANNOTATIONS_FILE          "titleAnnotations.lst"
#define VER_EXCLUDED_PARTS_FILE             "excludedParts.lst"

#define VER_LDRAW_OFFICIAL_ARCHIVE          "complete.zip"
#define VER_LDRAW_UNOFFICIAL_ARCHIVE        "ldrawunf.zip"
#define VER_LPUB3D_UNOFFICIAL_ARCHIVE       "lpub3dldrawunf.zip"

#define VER_UPDATE_CHECK_JSON_URL           "http://lpub3d.sourceforge.net/lpub3dupdates.json"
#define VER_OFFICIAL_LIBRARY_JSON_URL       "http://lpub3d.sourceforge.net/complete.json"
#define VER_UNOFFICIAL_LIBRARY_JSON_URL     "http://lpub3d.sourceforge.net/lpub3dldrawunf.json"
#define VER_CHANGE_LOG_URL                  "http://lpub3d.sourceforge.net/change_log.txt"

#define VER_SOURCE_SF_URL                   "http://sourceforge.net/p/lpub3d/code/"
#define VER_COMPANYDOMAIN_STR               "http://sourceforge.net/projects/lpub3d/"
#define VER_SOURCE_GITHUB_URL               "https://github.com/trevorsandy/lpub3d/"

#ifdef __GNUC__
  #ifdef __MINGW64__
    #define VER_COMPILED_ON             "MinGW (i686-6.1.0 64bit) Windows"
    #define VER_COMPILED_FOR            "MS Windows 64bit"
    #define VER_COMPILED_WITH           "Qt qtver (MinGW 64bit)"
    #define VER_IDE                     "Qt Creator 4.1.0 on Qt 5.7.0 (MSVC 2013, x32)"
  #elif __MINGW32__
    #define VER_COMPILED_ON             "MinGW (i686-5.3.0 32bit) Windows"
    #define VER_COMPILED_FOR            "MS Windows 32bit, 64bit"
    #define VER_COMPILED_WITH           "Qt qtver (MinGW 32bit)"
    #define VER_IDE                     "Qt Creator 4.1.0 on Qt 5.7.0 (MSVC 2013, x32)"
  #elif _MSC_VER
    #define VER_COMPILED_ON             "MSVC 2015"
    #define VER_COMPILED_FOR            "MS Windows 32bit, 64bit"
    #define VER_COMPILED_WITH           "(MSVC 2015 x86, x64)"
    #define VER_IDE                     "Visual Studio 2015 v14.0.25424.00 Update 3"
  #elif __APPLE__
    #define VER_COMPILED_ON             "Clang (x86 64bit) Apple"
    #define VER_COMPILED_FOR            "Apple OSX"
    #define VER_COMPILED_WITH           "Qt qtver (Clang 64bit)"
    #define VER_IDE                     "Qt Creator 4.1.0 on Qt 5.7.0 (Clang 7.0, x64)"
  #elif __linux__
    #define VER_COMPILED_ON             "GCC (4.5.8 20150623)"
    #define VER_COMPILED_FOR            "Linux"
    #define VER_COMPILED_WITH           "Qt qtver (GCC)"
    #define VER_IDE                     "Qt Creator 4.1.0 on Qt 5.7.0"
  #elif defined(_POSIX_VERSION)
    #define VER_COMPILED_ON             "POSIX"
    #define VER_COMPILED_FOR            "POSIX"
    #define VER_COMPILED_WITH           "Qt qtver (GCC)"
    #define VER_IDE                     "Qt Creator 4.1.0 on Qt 5.7.0"
  #else
  #   error "Unknown compiler"
  #endif
#endif

#endif // VERSION_H
