/****************************************************************************
**
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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
#define VER_PRODUCTVERSION_STR              VER_MAJOR "." VER_MINOR "." VER_PATCH
#define VER_PRODUCTNAME_STR                 "LPub3D"
#define VER_COMPANYNAME_BLD_STR             "LPub3D Software"
#define VER_APPLICATIONNAME_STR             VER_APPNAME
#define VER_FILEDESCRIPTION_STR             "LPub3D - An LDraw Building Instruction Editor"
#define VER_LEGALCOPYRIGHT_STR              "Copyright &copy; 2015 - 2018 by Trevor SANDY"
#define VER_LEGALTRADEMARKS1_STR            "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR            VER_LEGALTRADEMARKS1_STR
#define VER_PUBLISHER_STR                   "Trevor SANDY"
#define VER_FILEVERSION_STR                 STRING(VER_MAJOR) "." STRING(VER_MINOR) "." STRING(VER_PATCH) "." VER_REVISION_STR "." VER_BUILD_STR
#define VER_PUBLISHER_SUPPORT_STR           "https://github.com/trevorsandy/lpub3d/issues"
#define VER_LICENSE_INFO_STR                "http://www.gnu.org/licenses/gpl-3.0.html"
#ifdef QT_DEBUG_MODE
  #define COMPANYNAME_STR                   "LPub3D Software Maint"
#else
  #define COMPANYNAME_STR                   "LPub3D Software"
#endif
#define VER_COMPANYNAME_STR                 COMPANYNAME_STR
#define VER_BUILD_ARCH_STR                  VER_ARCH

// ~~~~~~ 3rdParty Applications ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
#define VER_LDVIEW_STR                      VER_LDVIEW
#define VER_LDGLITE_STR                     VER_LDGLITE
#define VER_POVRAY_STR                      VER_POVRAY

#define VER_LDVIEW_INI_FILE                 "ldview.ini"      // LDView renderer config settings
#define VER_LDVIEW_POV_INI_FILE             "ldviewPOV.ini"   // LDView POV file generation config settings
#define VER_POVRAY_CONF_FILE                "povray.conf"     // POV-Ray access control config settings
#define VER_POVRAY_INI_FILE                 "povray.ini"      // POV-Ray renderer config settings
#define VER_LDGLITE_INI_FILE                "ldglite.ini"     // LDGLite renderer config settings
#define VER_POVRAY_INC_FILE                 "consts.inc"      // Only used to validate directory
#define VER_POVRAY_SCENE_FILE               "index.htm"       // Only used to validate directory


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

#define VER_COMPANYDOMAIN_STR               "https://trevorsandy.github.io/lpub3d/"
#define VER_SOURCE_GITHUB_STR               "https://github.com/trevorsandy/lpub3d/"

#ifdef __GNUC__
  #ifdef __MINGW64__
    #define VER_COMPILED_ON             "MinGW (i686-7.2.0 64bit) Windows"
    #define VER_COMPILED_FOR            "MS Windows " VER_BUILD_ARCH_STR "bit"
    #define VER_COMPILED_WITH           "Qt qtver (MinGW " VER_BUILD_ARCH_STR "bit)"
    #define VER_IDE                     "Qt Creator 4.5.0 on Qt 5.10.0 (MSVC 2015, x32)"
  #elif __MINGW32__
    #define VER_COMPILED_ON             "MinGW (i686-5.3.0 32bit) Windows"
    #define VER_COMPILED_FOR            "MS Windows " VER_BUILD_ARCH_STR "bit"
    #define VER_COMPILED_WITH           "Qt qtver (MinGW " VER_BUILD_ARCH_STR "bit)"
    #define VER_IDE                     "Qt Creator 4.5.0 on Qt 5.10.0 (MSVC 2015, x32)"
  #elif _MSC_VER
    #define VER_COMPILED_ON             "MSVC 2015"
    #define VER_COMPILED_FOR            "MS Windows 32bit, 64bit"
    #define VER_COMPILED_WITH           "(MSVC 2015 x86, x64)"
    #define VER_IDE                     "Visual Studio 2015 v14.0.25424.00 Update 3"
  #elif __APPLE__
    #define VER_COMPILED_ON             "Clang (x86 64bit) Apple"
    #define VER_COMPILED_FOR            "macOS " VER_BUILD_ARCH_STR "bit"
    #define VER_COMPILED_WITH           "Qt qtver (Clang 64bit)"
    #define VER_IDE                     "Qt Creator 4.5.0 on Qt 5.10.0 (Clang 7.0, x64)"
  #elif __linux__
    #define VER_COMPILED_ON             "GCC (5.4.1 20160904)"
    #define VER_COMPILED_FOR            "Linux " VER_BUILD_ARCH_STR "bit"
    #define VER_COMPILED_WITH           "Qt qtver (GCC)"
    #define VER_IDE                     "Qt Creator 4.5.0 on Qt 5.10.0 (GCC 5.3.1, x64)"
  #elif defined(_POSIX_VERSION)
    #define VER_COMPILED_ON             "POSIX"
    #define VER_COMPILED_FOR            "POSIX " VER_BUILD_ARCH_STR "bit"
    #define VER_COMPILED_WITH           "Qt qtver (GCC)"
    #define VER_IDE                     "Qt Creator 4.5.0 on Qt 5.10.0 (GCC 5.3.1, x64)"
  #else
  #   error "Unknown compiler"
  #endif
#endif

#endif // VERSION_H
