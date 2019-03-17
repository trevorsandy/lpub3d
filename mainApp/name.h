 
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

enum PartType { FADE_PART, HIGHLIGHT_PART, NORMAL_PART, NUM_PART_TYPES };
enum PliType { PART, SUBMODEL, BOM, NUM_PLI_TYPES };
enum LogType { LOG_STATUS, LOG_INFO, LOG_TRACE, LOG_DEBUG, LOG_NOTICE, LOG_ERROR, LOG_INFO_STATUS, LOG_FATAL, LOG_QWARNING, LOG_QDEBUG };
enum IniFlag { NativePOVIni, NativeSTLIni, Native3DSIni, NativePartList, LDViewPOVIni, LDViewIni, NumIniFiles };
enum DividerType { StepDivider, RangeDivider, NoDivider };
enum LibType { LibLEGO, LibTENTE, LibVEXIQ, NumLibs };
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
#define VIEW_PARTS_WIDGET       "ViewPartsWidget"


#define MAX_NUM_POV_GEN_ARGS (128)

#define DOT_PATH_DEFAULT                        "."
#define EMPTY_STRING_DEFAULT                    ""

#define LOGGING_LEVEL_DEFAULT                   "STATUS"

#define LIBXQUARTZ_MACOS_VERSION                "11.0"   // base 11.0
#define LIBPNG_MACOS_VERSION                    "1.6.35" // base 1.6.34
#define LIBGL2PS_MACOS_VERSION                  "1.4.0"  // base 1.3.5
#define LIBJPEG_MACOS_VERSION                   "9c"     // base 8b
#define LIBXML_MACOS_VERSION                    "2.6.2"  // base 2.5.2
#define LIBMINIZIP_MACOS_VERSION                "1.2.11" // base 1.1.0

#define LIBTIFF_MACOS_VERSION                   "4.0.10" // base 3.6.1
#define LIBSDL_MACOS_VERSION                    "2.0.9"  // base 2.0.2
#define LIBOPENEXR_MACOS_VERSION                "2.2.0"  // base 2.2.0
#define LIBILMBASE_MACOS_VERSION                "2.2.1"  // base 2.2.1

#define CAMERA_FOV_DEFAULT                      0.01f    // LPub3D (L3P) default [Orthographic]
#define CAMERA_ZNEAR_DEFAULT                    10.0f    // LPub3D (L3P) default
#define CAMERA_ZFAR_DEFAULT                     4000.0f  // LPub3D (L3P) default

#define CAMERA_FOV_NATIVE_DEFAULT               30.0f    // Native (LeoCAD) defaults
#define CAMERA_ZNEAR_NATIVE_DEFAULT             25.0f    // Native (LeoCAD) defaults
#define CAMERA_ZFAR_NATIVE_DEFAULT              50000.0f // Native (LeoCAD) defaults

#define UPDATE_CHECK_FREQUENCY_DEFAULT          0        //0=Never,1=Daily,2=Weekly,3=Monthly

#define PAGE_HEIGHT_DEFAULT                     800
#define PAGE_WIDTH_DEFAULT                      600
#define LINE_WRAP_WIDTH                         160      //Text browser line width (change log)

#define PLAINTEXT_CHANGE_LOG_CUTOFF_VERSION    "2.2.2"   // last version of LPub3D to use html change notes

#define DURAT_LGEO_STL_LIB_INFO                 "LGEO Stl library is available"

#define SPLASH_FONT_COLOUR                      "#aa0000" // LPub3D Maroon

#define DISPLAY_SUBMODEL_SUFFIX                 "@Display"

// Renderers
#define RENDERER_POVRAY                         "POVRay"
#define RENDERER_LDGLITE                        "LDGLite"
#define RENDERER_LDVIEW                         "LDView"
#define RENDERER_NATIVE                         "Native"

#define LEGO_LIBRARY                            "LEGO"
#define TENTE_LIBRARY                           "TENTE"
#define VEXIQ_LIBRARY                           "VEXIQ"

#define LEGO_SEARCH_DIR_KEY                     "LEGOLDSearchDirs"
#define TENTE_SEARCH_DIR_KEY                    "TENTELDSearchDirs"
#define VEXIQ_SEARCH_DIR_KEY                    "VEXIQLDSearchDirs"

#define LEGO_FADE_COLOUR_KEY                    "LEGOFadeStepColour"
#define TENTE_FADE_COLOUR_KEY                   "TENTEFadeStepColour"
#define VEXIQ_FADE_COLOUR_KEY                   "VEXIQFadeStepColour"

#define LEGO_LDRAW_LIB_PATH_KEY                 "LEGOLDrawLibPath"
#define TENTE_LDRAW_LIB_PATH_KEY                "TENTELDrawLibPath"
#define VEXIQ_LDRAW_LIB_PATH_KEY                "VEXIQLDrawLibPath"

#define LEGO_FADE_COLOUR_DEFAULT                "Light_Blue"
#define TENTE_FADE_COLOUR_DEFAULT               "Light_Orange"
#define VEXIQ_FADE_COLOUR_DEFAULT               "VEX_Sky_Blue"

#define CAMERA_DISTANCE_FACTOR_NATIVE_DEFAULT   -260  // Native camera distance factor for A4 portrait; factor is about -450 for A4 landscape

#define RENDERER_TIMEOUT_DEFAULT                6    // measured in seconds

#define PAGE_DISPLAY_PAUSE_DEFAULT              3    // measured in seconds

// Internal common material colours
#define LDRAW_EDGE_MATERIAL_COLOUR              "24"
#define LDRAW_MAIN_MATERIAL_COLOUR              "16"

#define FADE_OPACITY_DEFAULT                    50         // 50 percent transparency

#define SUBMODEL_ICON_INDEX_BASE                1000
#define LPUB3D_COLOUR_FADE_SUFFIX               "fade"
#define LPUB3D_COLOUR_FADE_PREFIX               "100"
#define LPUB3D_COLOUR_HIGHLIGHT_SUFFIX          "highlight"
#define LPUB3D_COLOUR_HIGHLIGHT_PREFIX          "110"
#define LPUB3D_COLOUR_TITLE_PREFIX              "LPub3D_"
#define LPUB3D_COLOUR_FILE_PREFIX               "colours_" //Not used

#define LPUB3D_IM_BASE_LDR_EXT                  "base.ldr"    // Image Matte LDraw working file extension
#define LPUB3D_IM_OVERLAY_LDR_EXT               "overlay.ldr" // Image Matte LDraw working file extension
#define LPUB3D_IM_BASE_PNG_EXT                  "base.png"    // Image Matte Png working file extension
#define LPUB3D_IM_OVERLAY_PNG_EXT               "overlay.png" // Image Matte Png working file extension

#define HIGHLIGHT_COLOUR_DEFAULT                "#FFFF00"  // Bright Yellow
#define HIGHLIGHT_LINE_WIDTH_DEFAULT            1.0        // line width 1 - 5

// Team color supplements
#define THEME_DEFAULT                   "Default" // Default Theme
#define THEME_TICK_PEN_DEFAULT          "#35322f" // Qt Dark Grey
#define THEME_NML_PEN_DEFAULT           "#1e1b18" // Qt Double Dark Grey
#define THEME_GUIDE_PEN_DEFAULT         "#1e1b18" // Qt Double Dark Grey
#define THEME_MAIN_BGCOLOR_DEFAULT      "#aeadac" // Qt Light Gray
#define THEME_VIEWER_BGCOLOR_DEFAULT    "#ffffff" // Qt White

#define THEME_HIGHLIGHT_01_DEFAULT      "#006325" // 01 [LDraw Comments]
#define THEME_HIGHLIGHT_02_DEFAULT      "#4f97ba" // 02 [LDraw Header]
#define THEME_HIGHLIGHT_03_DEFAULT      "#005af6" // 03 [LDraw Body]
#define THEME_HIGHLIGHT_04_DEFAULT      "#609cff" // 04 [LPub3D Local]
#define THEME_HIGHLIGHT_05_DEFAULT      "#57649b" // 05 [LPub3D Global]

#define THEME_HIGHLIGHT_06_DEFAULT      "#816288" // 06 [LDraw Line type 1]
#define THEME_HIGHLIGHT_07_DEFAULT      "#f46105" // 07 [LDraw Colour code]
#define THEME_HIGHLIGHT_08_DEFAULT      "#ed3d63" // 08 [LDraw Part Position]
#define THEME_HIGHLIGHT_09_DEFAULT      "#8bc3f6" // 09 [LDraw Part Transform1]
#define THEME_HIGHLIGHT_10_DEFAULT      "#51acff" // 10 [LDraw Part Transform2]
#define THEME_HIGHLIGHT_11_DEFAULT      "#2f86ff" // 11 [LDraw Part Transform3]
#define THEME_HIGHLIGHT_12_DEFAULT      "#084575" // 12 [LDraw Part File]
#define THEME_HIGHLIGHT_13_DEFAULT      "#7d5e18" // 13 [LDraw Line Types 2_5]
#define THEME_HIGHLIGHT_14_DEFAULT      "#fc9b14" // 14 [LPub3D Number]
#define THEME_HIGHLIGHT_15_DEFAULT      "#b87620" // 15 [LPub3D Hex Number]
#define THEME_HIGHLIGHT_16_DEFAULT      "#c15317" // 16 [LPub3D Page Size]

#define THEME_HIGHLIGHT_17_DEFAULT      "#14148c" // 17 [LeoCAD]
#define THEME_HIGHLIGHT_18_DEFAULT      "#d882f6" // 18 [LSynth]
#define THEME_HIGHLIGHT_19_DEFAULT      "#ff9900" // 19 [LDCad]
#define THEME_HIGHLIGHT_20_DEFAULT      "#0079cf" // 20 [MLCad]
#define THEME_HIGHLIGHT_21_DEFAULT      "#9421a6" // 21 [MLCad Body]
#define THEME_HIGHLIGHT_22_DEFAULT      "#ff1818" // 22 [LPub3D False]
#define THEME_HIGHLIGHT_23_DEFAULT      "#008000" // 23 [LPub3D True]
#define THEME_HIGHLIGHT_24_DEFAULT      "#cc5980" // 24 [LPub3D]
#define THEME_HIGHLIGHT_25_DEFAULT      "#aa0000" // 25 [LPub3D Body]
#define THEME_HIGHLIGHT_26_DEFAULT      "#545454" // 26 [LDraw Header Value]
#define THEME_HIGHLIGHT_27_DEFAULT      "#231aff" // 27 [LPub3D Quoted Text]
#define THEME_HIGHLIGHT_28_DEFAULT      "#000000" // 28 [LDraw Line Type 0 First Character]

#define THEME_HIGHLIGHT_A_DEFAULT       "#006325" // Qt
#define THEME_HIGHLIGHT_B_DEFAULT       "#14148c" // Qt
#define THEME_HIGHLIGHT_C_DEFAULT       "#0057ff" // 06
#define THEME_HIGHLIGHT_D_DEFAULT       "#b25a2f" // 07

#define THEME_HIGHLIGHT_E_DEFAULT       "#084575" // br05 Part ID
#define THEME_HIGHLIGHT_F_DEFAULT       "#9421a6" // br06 Part Control
#define THEME_HIGHLIGHT_G_DEFAULT       "#aa0000" // br07 Part Description

// -----------------------------------------------//

#define THEME_DARK                      "Dark"    // Dark Theme
#define THEME_TICK_PEN_DARK             "#eff0f1" // Custom Pale Grey
#define THEME_NML_PEN_DARK              "#ffffff" // Qt White
#define THEME_GUIDEL_PEN_DARK           "#5d5b59" // Qt Medium Gray
#define THEME_MAIN_BGCOLOR_DARK         "#31363b" // Custom Dark Grey
#define THEME_VIEWER_BGCOLOR_DARK       "#808B96" // Custom Grey
#define THEME_EDIT_MARGIN_DARK          "#ABB2B9" // Custom Gray

#define THEME_HIGHLIGHT_01_DARK         "#17c723" // 01 [LDraw Comment]
#define THEME_HIGHLIGHT_02_DARK         "#fb743e" // 02 [LDraw Header]
#define THEME_HIGHLIGHT_03_DARK         "#14aaff" // 03 [LDraw Body]
#define THEME_HIGHLIGHT_04_DARK         "#609cff" // 04 [LPub3D Local]
#define THEME_HIGHLIGHT_05_DARK         "#a0b2e2" // 05 [LPub3D Global]

#define THEME_HIGHLIGHT_06_DARK         "#c2a4c0" // 06 [LDraw Line Type 1]
#define THEME_HIGHLIGHT_07_DARK         "#f46105" // 07 [LDraw Colour Code]
#define THEME_HIGHLIGHT_08_DARK         "#ed3d63" // 08 [LDraw Part Position]
#define THEME_HIGHLIGHT_09_DARK         "#ddffef" // 09 [LDraw Part Transform1]
#define THEME_HIGHLIGHT_10_DARK         "#a2ffd2" // 10 [LDraw Part Transform2]
#define THEME_HIGHLIGHT_11_DARK         "#5cffb0" // 11 [LDraw Part Transform3]
#define THEME_HIGHLIGHT_12_DARK         "#c3f6fe" // 12 [LDraw Part File]
#define THEME_HIGHLIGHT_13_DARK         "#7d5e18" // 13 [LDraw Line Types 2_5]
#define THEME_HIGHLIGHT_14_DARK         "#fc9b14" // 14 [LPub3D Number]
#define THEME_HIGHLIGHT_15_DARK         "#b87620" // 15 [LPub3D Hex Number]
#define THEME_HIGHLIGHT_16_DARK         "#c15317" // 16 [LPub3D Page Size]

#define THEME_HIGHLIGHT_17_DARK         "#0079cf" // 17 [LeoCAD]
#define THEME_HIGHLIGHT_18_DARK         "#ff9900" // 18 [LSynth]
#define THEME_HIGHLIGHT_19_DARK         "#ff9900" // 19 [LDCad]
#define THEME_HIGHLIGHT_20_DARK         "#0079cf" // 20 [MLCad]
#define THEME_HIGHLIGHT_21_DARK         "#9421a6" // 21 [MLCad Body]
#define THEME_HIGHLIGHT_22_DARK         "#ff1818" // 22 [LPub3D False]
#define THEME_HIGHLIGHT_23_DARK         "#adff2f" // 23 [LPub3D True]
#define THEME_HIGHLIGHT_24_DARK         "#ff3366" // 24 [LPub3D]
#define THEME_HIGHLIGHT_25_DARK         "#aa0000" // 25 [LPub3D Body]
#define THEME_HIGHLIGHT_26_DARK         "#aeaeae" // 26 [LDraw Header Value]
#define THEME_HIGHLIGHT_27_DARK         "#81d4fa" // 27 [LPub3D Quoted text]
#define THEME_HIGHLIGHT_28_DARK         "#ffffff" // 28 [LDraw Line type 0 First Character]

#define THEME_HIGHLIGHT_A_DARK          "#17c723" // 01 Custom Green
#define THEME_HIGHLIGHT_B_DARK          "#0079cf" // 02 Custom Blue
#define THEME_HIGHLIGHT_C_DARK          "#ff5227" // 06 Custom Orange
#define THEME_HIGHLIGHT_D_DARK          "#bbbfff" // 07 Custom Violet

#define THEME_HIGHLIGHT_E_DARK          "#c3f6fe" // br05 Part ID
#define THEME_HIGHLIGHT_F_DARK          "#9421a6" // br06 Part Control
#define THEME_HIGHLIGHT_G_DARK          "#aa0000" // br07 Part Description

#define THEME_EDITWINDOW_LINE_DARK      "#5d5b59" // 06 Qt Medium Gray
#endif
