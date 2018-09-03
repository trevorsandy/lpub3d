 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef NAME_H
#define NAME_H

class Gui;

extern Gui *gui;
enum PartType { FADE_PART, HIGHLIGHT_PART };

// registry sections
#define DEFAULTS     "Defaults"
#define POVRAY       "POVRay"
#define SETTINGS     "Settings"
#define MAINWINDOW   "MainWindow"
#define PARMSWINDOW  "ParmsWindow"
#define UPDATES      "Updates"
#define LOGGING      "Logging"

#define LOGGING_LEVEL_DEFAULT             "STATUS"

#define UPDATE_CHECK_FREQUENCY_DEFAULT    0     //0=Never,1=Daily,2=Weekly,3=Monthly

#define PAGE_HEIGHT_DEFAULT               800
#define PAGE_WIDTH_DEFAULT                600

// filenames
#define URL_LDRAW_UNOFFICIAL_ARCHIVE      "http://www.ldraw.org/library/unofficial/ldrawunf.zip"
#define URL_LDRAW_OFFICIAL_ARCHIVE        "http://www.ldraw.org/library/updates/complete.zip"

#define FILE_LDRAW_OFFICIAL_ARCHIVE       "complete.zip"
#define FILE_LDRAW_UNOFFICIAL_ARCHIVE     "ldrawunf.zip"
#define FILE_LPUB3D_UNOFFICIAL_ARCHIVE    "lpub3dldrawunf.zip"

#define FILE_LDRAW_LDCONFIG               "ldconfig.ldr"
#define DURAT_LGEO_STL_LIB_INFO           "LGEO Stl library is available"

#define SPLASH_FONT_COLOUR                "#aa0000" // Maroon

// Renderers
#define RENDERER_POVRAY                   "POVRay"
#define RENDERER_LDGLITE                  "LDGLite"
#define RENDERER_LDVIEW                   "LDView"

#define RENDERER_TIMEOUT_DEFAULT          6    // measured in seconds

#define PAGE_DISPLAY_PAUSE_DEFAULT        3    // measured in seconds

// Internal common material colours
#define LDRAW_EDGE_MATERIAL_COLOUR        "24"
#define LDRAW_MAIN_MATERIAL_COLOUR        "16"

#define FADE_COLOUR_DEFAULT               "Very_Light_Bluish_Grey"
#define FADE_OPACITY_DEFAULT              100

#define LPUB3D_COLOUR_FADE_PREFIX         "100"
#define LPUB3D_COLOUR_HIGHLIGHT_PREFIX    "110"
#define LPUB3D_COLOUR_TITLE_PREFIX        "LPub3D_"
#define LPUB3D_COLOUR_FILE_PREFIX         "colours_"  //Not used

#define HIGHLIGHT_COLOUR_DEFAULT          "#FFFF00"      // Bright Yellow
#define HIGHLIGHT_LINE_WIDTH_DEFAULT      1              // line width 1 - 5

#endif
