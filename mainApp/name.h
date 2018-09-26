 
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

class LDVWidget;
extern LDVWidget *ldvWidget;

enum PartType { FADE_PART, HIGHLIGHT_PART };
enum LogType { LOG_STATUS, LOG_INFO, LOG_TRACE, LOG_DEBUG, LOG_NOTICE, LOG_ERROR, LOG_FATAL, LOG_QWARNING, LOG_QDEBUG };
enum IniFlag { NativePOVIni, LDViewPOVIni, LDViewIni };
enum Theme { ThemeDark, ThemeDefault };

// registry sections
#define DEFAULTS                "Defaults"
#define POVRAY                  "POVRay"
#define SETTINGS                "Settings"
#define MAINWINDOW              "MainWindow"
#define PARMSWINDOW             "ParmsWindow"
#define UPDATES                 "Updates"
#define LOGGING                 "Logging"
#define SAVE_DISPLAY_PAGE_NUM   "SaveDisplayPageNum"


#define MAX_NUM_POV_GEN_ARGS (128)

#define DOT_PATH_DEFAULT                        "."
#define EMPTY_STRING_DEFAULT                    ""

#define LOGGING_LEVEL_DEFAULT                   "STATUS"

#define LIBPNG_MACOS_VERSION                    "1.6.35"

#define CAMERA_FOV_DEFAULT                      0.01f    // LPub3D (L3P) default
#define CAMERA_ZNEAR_DEFAULT                    10.0f    // LPub3D (L3P) default
#define CAMERA_ZFAR_DEFAULT                     4000.0f  // LPub3D (L3P) default

#define CAMERA_FOV_NATIVE_DEFAULT               30.0f    // Native (LeoCAD) defaults
#define CAMERA_ZNEAR_NATIVE_DEFAULT             25.0f    // Native (LeoCAD) defaults
#define CAMERA_ZFAR_NATIVE_DEFAULT              50000.0f // Native (LeoCAD) defaults

#define UPDATE_CHECK_FREQUENCY_DEFAULT          0        //0=Never,1=Daily,2=Weekly,3=Monthly

#define PAGE_HEIGHT_DEFAULT                     800
#define PAGE_WIDTH_DEFAULT                      600

// filenames
#define URL_LDRAW_UNOFFICIAL_ARCHIVE            "http://www.ldraw.org/library/unofficial/ldrawunf.zip"
#define URL_LDRAW_OFFICIAL_ARCHIVE              "http://www.ldraw.org/library/updates/complete.zip"

#define FILE_LDRAW_OFFICIAL_ARCHIVE             "complete.zip"
#define FILE_LDRAW_UNOFFICIAL_ARCHIVE           "ldrawunf.zip"
#define FILE_LPUB3D_UNOFFICIAL_ARCHIVE          "lpub3dldrawunf.zip"

#define FILE_LDRAW_LDCONFIG                     "ldconfig.ldr"
#define DURAT_LGEO_STL_LIB_INFO                 "LGEO Stl library is available"

#define SPLASH_FONT_COLOUR                      "#aa0000" // Maroon

// Renderers
#define RENDERER_POVRAY                         "POVRay"
#define RENDERER_LDGLITE                        "LDGLite"
#define RENDERER_LDVIEW                         "LDView"
#define RENDERER_NATIVE                         "Native"

#define CAMERA_DISTANCE_FACTOR_NATIVE_DEFAULT   260  // Native camera distance factor for A4 portrait; factor is about 450 for A4 landscape

#define RENDERER_TIMEOUT_DEFAULT                6    // measured in seconds

#define PAGE_DISPLAY_PAUSE_DEFAULT              3    // measured in seconds

// Internal common material colours
#define LDRAW_EDGE_MATERIAL_COLOUR              "24"
#define LDRAW_MAIN_MATERIAL_COLOUR              "16"

#define FADE_COLOUR_DEFAULT                     "Very_Light_Bluish_Grey"
#define FADE_OPACITY_DEFAULT                    50         // 50 percent transarency

#define LPUB3D_COLOUR_FADE_PREFIX               "100"
#define LPUB3D_COLOUR_HIGHLIGHT_PREFIX          "110"
#define LPUB3D_COLOUR_TITLE_PREFIX              "LPub3D_"
#define LPUB3D_COLOUR_FILE_PREFIX               "colours_" //Not used

#define LPUB3D_IM_BASE_LDR_EXT                  "base.ldr"    // Image Matte LDraw working file extension
#define LPUB3D_IM_OVERLAY_LDR_EXT               "overlay.ldr" // Image Matte LDraw working file extension
#define LPUB3D_IM_BASE_PNG_EXT                  "base.png"    // Image Matte Png working file extension
#define LPUB3D_IM_OVERLAY_PNG_EXT               "overlay.png" // Image Matte Png working file extension

#define HIGHLIGHT_COLOUR_DEFAULT                "#FFFF00"  // Bright Yellow
#define HIGHLIGHT_LINE_WIDTH_DEFAULT            1.0        // line width 1 - 5

// Native pov file generation defaults
#define SEAM_WIDTH_DEFAULT                      0.5
#define QUALITY_EXPORT_DEFAULT                  3
#define SELECTED_ASPECT_RATIO_DEFAULT          -1          // ASPECT_RATIO_0 "Automatic"
#define CUSTOM_ASPECT_RATIO_DEFAULT             1.5f
#define EDGE_RADIUS_DEFAULT                     0.15f
#define AMBIENT_DEFAULT                         0.4f
#define DIFFUSE_DEFAULT                         0.4f
#define REFLECTION_DEFAULT                      0.08f
#define PHONG_DEFAULT                           0.5f
#define PHONG_SIZE_DEFAULT                      40.0f
#define TRANS_REFLECTION_DEFAULT                0.2f
#define TRANS_FILTER_DEFAULT                    0.85f
#define TRANS_IOR_DEFAULT                       1.25f
#define RUBBER_REFLECTION_DEFAULT               0.0f
#define RUBBER_PHONG_DEFAULT                    0.1f
#define RUBBER_PHONG_SIZE_DEFAULT               10.0f
#define CHROME_REFLECTION_DEFAULT               0.85f
#define CHROME_BRILLIANCE_DEFAULT               5.0f
#define CHROME_SPECULAR_DEFAULT                 0.8F
#define CHROME_ROUGHNESS_DEFAULT                0.01f
#define FILE_VERSION_DEFAULT                    3.6f

#define XML_MAP_PATH_DEFAULT                    EMPTY_STRING_DEFAULT
#define TOP_INCLUDE_DEFAULT                     EMPTY_STRING_DEFAULT
#define BOTTOM_INCLUDE_DEFAULT                  EMPTY_STRING_DEFAULT

#define LIGHT_01                                "1. Latitude 45.0 Longitude 0.0"     // 0
#define LIGHT_02                                "2. Latitude 30.0 Longitude 120.0"   // 1
#define LIGHT_03                                "3. Latitude 60.0 Longitude -120.0"  // 2
#define LIGHTS_COMBO_DEFAULT                    LIGHT_01 "," LIGHT_02 "," LIGHT_03

#define QUALITY_01                              "1. Bounding boxes only"              // 0
#define QUALITY_02                              "2. No refraction"                    // 1
#define QUALITY_03                              "3. Normal"                           // 2
#define QUALITY_04                              "4. Include stud logo"                // 3
#define QUALITY_COMBO_DEFAULT                   QUALITY_01 "," QUALITY_02 "," QUALITY_03 "," QUALITY_04

#define POV_FILE_VER_01                         "3.6"                                 // 0
#define POV_FILE_VER_02                         "3.7"                                 // 1
#define POV_FILE_VER_03                         "3.8"                                 // 2
#define POV_FILE_VERSION_COMBO_DEFAULT          POV_FILE_VER_01 "," POV_FILE_VER_02 "," POV_FILE_VER_03

#define ASPECT_RATIO_0                          "Automatic"                           // -1
#define ASPECT_RATIO_1                          "5:4"                                 //  0
#define ASPECT_RATIO_2                          "3:2"                                 //  2
#define ASPECT_RATIO_3                          "5:3"                                 //  3
#define ASPECT_RATIO_4                          "16:9"                                //  4
#define ASPECT_RATIO_5                          "2.35:1"                              //  5
#define ASPECT_RATIO_6                          "3DViewer Aspect Ratio"               //  6
#define ASPECT_RATIO_7                          "Custom"                              //  7
#define ASPECT_RATIO_8                          "4:3"                                 //  8
#define SELECTED_ASPECT_RATIO_COMBO_DEFAULT     ASPECT_RATIO_0 "," ASPECT_RATIO_1 "," ASPECT_RATIO_2 "," \
                                                ASPECT_RATIO_3 "," ASPECT_RATIO_4 "," ASPECT_RATIO_5 "," \
                                                ASPECT_RATIO_6 "," ASPECT_RATIO_7 "," ASPECT_RATIO_8

// Team colour suppliments
#define THEME_DEFAULT                   "Default" // Default Theme
#define THEME_TICK_PEN_DEFAULT          "#35322f" // Qt Dark gray
#define THEME_NML_PEN_DEFAULT           "#1e1b18" // Qt Double dark gray
#define THEME_GUIDE_PEN_DEFAULT         "#1e1b18" // Qt Double dark gray
#define THEME_MAIN_BGCOLOR_DEFAULT      "#aeadac" // Qt Light gray
#define THEME_VIEWER_BGCOLOR_DEFAULT    "#ffffff" // Qt White

#define THEME_HIGHLIGHT_01_DEFAULT      "#006325" // Qt 01 Dark green
#define THEME_HIGHLIGHT_02_DEFAULT      "#14148c" // Qt 02 Dark blue
#define THEME_HIGHLIGHT_03_DEFAULT      "#800000" // Qt 03 Dark Red
#define THEME_HIGHLIGHT_04_DEFAULT      "#0000FF" // Qt 04 Blue
#define THEME_HIGHLIGHT_05_DEFAULT      "#b40000" // Qt 05 Red

#define THEME_DARK                      "Dark"    // Dark Theme
#define THEME_TICK_PEN_DARK             "#eff0f1" // Custom pale grey
#define THEME_NML_PEN_DARK              "#ffffff" // Qt White
#define THEME_GUIDEL_PEN_DARK           "#5d5b59" // Qt Medium gray
#define THEME_MAIN_BGCOLOR_DARK         "#31363b" // Custom dark dark grey
#define THEME_VIEWER_BGCOLOR_DARK       "#808B96" // Custom grey
#define THEME_EDIT_MARGIN_DARK          "#ABB2B9" // Custom gray

#define THEME_HIGHLIGHT_01_DARK         "#17c723" // 01 Custom green
#define THEME_HIGHLIGHT_02_DARK         "#0079cf" // 02 Custom blue
#define THEME_HIGHLIGHT_03_DARK         "#d20117" // 03 Custom Red
#define THEME_HIGHLIGHT_04_DARK         "#14aaff" // 04 Qt Blue
#define THEME_HIGHLIGHT_05_DARK         "#ff3300" // 05 Custom Red

#define THEME_EDITWINDOW_LINE_DARK      "#5d5b59" // 06 Qt Medium gray
#endif
