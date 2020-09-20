
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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

struct SnapGridCommands
{
    const char* ID;
    const char* MenuName;
    const char* StatusText;
};

struct TypeLine {
  int modelIndex;
  int lineIndex;
  TypeLine(){}
  TypeLine(int _modelIndex,int _lineIndex) :
      modelIndex(_modelIndex),
      lineIndex(_lineIndex)
  {}
};

enum PartType { FADE_PART, HIGHLIGHT_PART, NORMAL_PART, NUM_PART_TYPES };
enum PliType { PART, SUBMODEL, BOM, NUM_PLI_TYPES };
enum LogType { LOG_STATUS, LOG_INFO, LOG_TRACE, LOG_DEBUG, LOG_NOTICE, LOG_ERROR, LOG_INFO_STATUS, LOG_FATAL, LOG_QWARNING, LOG_QDEBUG };
enum CamFlag { DefFoV, DefZNear, DefZFar };
enum IniFlag { NativePOVIni, NativeSTLIni, Native3DSIni, NativePartList, POVRayRender, LDViewPOVIni, LDViewIni, NumIniFiles };
enum DividerType { StepDivider, RangeDivider, NoDivider };
enum ShowLoadMsgType { NEVER_SHOW, SHOW_ERROR, SHOW_WARNING, SHOW_MESSAGE, ALWAYS_SHOW };
enum LoadMsgType { MISSING_LOAD_MSG, PRIMITIVE_LOAD_MSG, SUBPART_LOAD_MSG, VALID_LOAD_MSG, ALL_LOAD_MSG };
enum RulerTrackingType { TRACKING_TICK, TRACKING_LINE, TRACKING_NONE};
enum SceneGuidesPosType { GUIDES_TOP_LEFT, GUIDES_TOP_RIGHT, GUIDES_CENTRE, GUIDES_BOT_LEFT, GUIDES_BOT_RIGHT};
enum LibType { LibLEGO, LibTENTE, LibVEXIQ, NumLibs };
enum Theme { ThemeDark, ThemeDefault };
enum PartSource { EDITOR_LINE, VIEWER_NONE = EDITOR_LINE, VIEWER_LINE, VIEWER_MOD };
enum SaveOnSender { SaveOnNone, SaveOnRedraw, SaveOnUpdate };
enum NativeType { NTypeDefault, NTypeCalledOut, NTypeMultiStep };
enum SceneObjectInfo { ObjectId };
enum PartsAttrib {
    BM_BEGIN_LINE,
    BM_ACTION_LINE,
    BM_MODEL_INDEX = BM_ACTION_LINE,
    BM_END_LINE,
    BM_MODEL_NAME_INDEX
};
enum GridStepSize {
    GRID_SIZE_FIRST,
    SCENE_GRID_SIZE_S1 = GRID_SIZE_FIRST,
    SCENE_GRID_SIZE_S2,
    SCENE_GRID_SIZE_S3,
    SCENE_GRID_SIZE_S4,
    SCENE_GRID_SIZE_S5,
    SCENE_GRID_SIZE_S6,
    SCENE_GRID_SIZE_S7,
    SCENE_GRID_SIZE_S8,
    SCENE_GRID_SIZE_S9,
    GRID_SIZE_LAST = SCENE_GRID_SIZE_S9,
    NUM_GRID_SIZES
};
enum LDrawUnofficialFileType {
    UNOFFICIAL_SUBMODEL,
    UNOFFICIAL_PART,
    UNOFFICIAL_SUBPART,
    UNOFFICIAL_PRIMITIVE,
    UNOFFICIAL_OTHER
};
enum SubAttributes {
    sAdj = -2,
    sType = 0,        // 0  level 1   substitution
    sColorCode,       // 1  level 2   substitution
    sModelScale,      // 2  level 3   substitution
    sCameraFoV,       // 3  level 4   substitution
    sCameraAngleXX,   // 4  level 5   substitution
    sCameraAngleYY,   // 5  level 5   substitution
    sTargetX,         // 6  level 6/8 substitution
    sTargetY,         // 7  level 6/8 substitution
    sTargetZ,         // 8  level 6/8 substitution
    sRotX,            // 9  level 7/8 substitution
    sRotY,            // 10 level 7/8 substitution
    sRotZ,            // 11 level 7/8 substitution
    sTransform,       // 12 level 7/8 substitution
    sOriginalType,    // 13 Substituted part/Original LDraw part
    sSubstitute,      // 14
    sUpdate,          // 15
    sRemove           // 16
};
enum NameKeyAttributes {
    nType = 0,        // 0 Set to CSI for CSI type
    nColorCode,       // 1 This is Step Number for CSI type
    nPageWidth,       // 2
    nResolution,      // 3
    nResType,         // 4
    nModelScale,      // 5
    nCameraFoV,       // 6
    nCameraAngleXX,   // 7
    nCameraAngleYY,   // 8
    nBaseAttributes      = nCameraAngleYY,  // 8th element
    nTargetX,         // 9
    nRotX                = nTargetX,
    nTargetY,         // 10
    nRotY                = nTargetY,
    nTargetZ,         // 11
    nRotZ                = nTargetZ,
    nHasTarget           = nTargetZ,       // 11th element
    nRot_X,           // 12
    nRotTrans            = nRot_X,
    nHasRotstep          = nRotTrans,      // 12th element
    nRot_Y,           // 13
    nRot_Z,           // 14
    nRot_Trans,       // 15
    nHasTargetAndRotstep = nRot_Trans,     // 15th element
    nSub              // 16 Used by LDView single call
};
enum NameKeyAttributes2{
    K_STEPNUMBER = 0, // 0  not used
    K_IMAGEWIDTH,     // 1  not used
    K_RESOLUTION,     // 2
    K_RESOLUTIONTYPE, // 3
    K_MODELSCALE,     // 4
    K_FOV,            // 5  not used
    K_LATITUDE,       // 6
    K_LONGITUDE,      // 7
    K_TARGETX,        // 8
    K_TARGETY,        // 9
    K_TARGETZ,        // 10
    K_ROTSX,          // 11
    K_ROTSY,          // 12
    K_ROTSZ,          // 13
    K_ROTSTYPE        // 14
};

const int GridSizeTable[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90 };

extern SnapGridCommands sgCommands[NUM_GRID_SIZES];

/*
 * Scene Objects
 *
 * Synchronization:
 * Meta.h         - class SceneItemMeta
 * Meta.cpp       - SceneItemMeta::SceneItemMeta(), SceneItemMeta::init()
 * lpub.cpp       - void Gui::initialize() soMap
 * name.h         - SceneObject enum, ZValue defines, IncludedSceneObjects[]
 * traverse.cpp   - case SceneItemZValueDirectionRc
 * formatpage.cpp - bool Gui::getSceneObject(()
 */

enum SceneObject {
    UndefinedObj             = -1,
    AssemAnnotationObj       = 36, //  0 CsiAnnotationItem             / CsiAnnotationType
    AssemAnnotationPartObj   = 27, //  1 PlacementCsiPart              / CsiPartType
    AssemObj                 =  1, //  2 CsiItem                       / CsiType
    CalloutBackgroundObj     =  5, //  4 CalloutBackgroundItem         / CalloutType
    CalloutInstanceObj       =  8, //  5 CalloutInstanceItem           /
    CalloutPointerObj        =  9, //  6 CalloutPointerItem            /
    CalloutUnderpinningObj   = 10, //  7 UnderpinningsItem             /
    DividerBackgroundObj     = 11, //  8 DividerBackgroundItem         /
    DividerObj               = 12, //  9 DividerItem                   /
    DividerLineObj           = 13, // 10 DividerLine                   /
    DividerPointerObj        = 37, // 11 DividerPointerItem            / DividerPointerType
    PointerGrabberObj        = 14, // 12 Grabber                       /
    PliGrabberObj            = 41, // 13 Grabber                       /
    SubmodelGrabberObj       = 42, // 14 Grabber                       /
    InsertPixmapObj          = 15, // 15 InsertPixmapItem              /
    InsertTextObj            = 30, // 16 TextItem                      / TextType
    MultiStepBackgroundObj   =  2, // 17 MultiStepRangeBackgroundItem  / StepGroupType
    MultiStepsBackgroundObj  = 43, // 18 MultiStepRangesBackgroundItem /
    PageAttributePixmapObj   = 17, // 19 PageAttributePixmapItem       / PageDocumentLogoType, PageCoverImageType
    PageAttributeTextObj     = 19, // 20 PageAttributeTextItem         / PageTitleType ... PagePlugType
    PageBackgroundObj        =  0, // 21 PageBackgroundItem            / PageType
    PageNumberObj            =  6, // 22 PageNumberItem                / PageNumberType
    PagePointerObj           = 29, // 23 PagePointerItem               / PagePointerType
    PartsListAnnotationObj   = 20, // 24 AnnotateTextItem              /
    PartsListBackgroundObj   =  4, // 25 PliBackgroundItem             / PartsListType
    PartsListInstanceObj     = 21, // 26 InstanceTextItem              /
    PointerHeadObj           = 22, // 27 PointerHeadItem               /
    PointerFirstSegObj       = 23, // 28 BorderedLineItem              /
    PointerSecondSegObj      = 24, // 29 BorderedLineItem              /
    PointerThirdSegObj       = 28, // 30 BorderedLineItem              /
    RotateIconBackgroundObj  = 26, // 31 RotateIconItem                / RotateIconType
    StepNumberObj            =  3, // 32 StepNumberItem                / StepNumberType
    SubModelBackgroundObj    = 25, // 33 SubModelType                  /
    SubModelInstanceObj      = 16, // 34 SMInstanceTextItem            /
    SubmodelInstanceCountObj = 18, // 35 SubmodelInstanceCount         / SubmodelInstanceCountType
    PartsListPixmapObj       = 44, // 36 PGraphicsPixmapItem           /
    PartsListGroupObj        = 45, // 37 PartGroupItem                 /
    StepBackgroundObj        = 46  // 38 MultiStepStepBackgroundItem   / StepType
};

static const SceneObject IncludedSceneObjects[] =
{
    AssemAnnotationObj,
    AssemAnnotationPartObj,
    AssemObj,
    CalloutBackgroundObj,
    CalloutInstanceObj,
    CalloutPointerObj,
    CalloutUnderpinningObj,
    DividerBackgroundObj,
    DividerLineObj,
    DividerObj,
    DividerPointerObj,
    InsertPixmapObj,
    InsertTextObj,
    MultiStepBackgroundObj,
    MultiStepsBackgroundObj,
    PageAttributePixmapObj,
    PageAttributeTextObj,
    PageBackgroundObj,
    PageNumberObj,
    PagePointerObj,
    PartsListAnnotationObj,
    PartsListBackgroundObj,
    PartsListGroupObj,
    PartsListInstanceObj,
    PartsListPixmapObj,
    PliGrabberObj,
    PointerFirstSegObj,
    PointerGrabberObj,
    PointerHeadObj,
    PointerSecondSegObj,
    PointerThirdSegObj,
    RotateIconBackgroundObj,
    StepBackgroundObj,
    StepNumberObj,
    SubModelBackgroundObj,
    SubmodelGrabberObj,
    SubmodelInstanceCountObj,
    SubModelInstanceObj
};

// Exempted from detection - triggers invalid object
static const SceneObject ExemptSceneObjects[] =
{
    PageBackgroundObj,      //  0
    PointerFirstSegObj,     // 22
    PointerSecondSegObj,    // 24
    PointerThirdSegObj,     // 28

    PartsListPixmapObj,     // 44
    PartsListAnnotationObj, // 20
    PartsListInstanceObj    // 21
};

// Trigger scene guide on PliPartGroup object
static const SceneObject PliPartGroupSceneObjects[] =
{
    PartsListPixmapObj,     // 44
    PartsListAnnotationObj, // 20
    PartsListInstanceObj    // 21
};

#define Z_VALUE_DEFAULT                            0.0
#define GRABBER_ZVALUE_DEFAULT                     100.0
#define POINTER_ZVALUE_DEFAULT                    -1.0
#define ASSEM_ZVALUE_DEFAULT                       Z_VALUE_DEFAULT
#define ASSEMANNOTATION_ZVALUE_DEFAULT             Z_VALUE_DEFAULT
#define ASSEMANNOTATIONPART_ZVALUE_DEFAULT         Z_VALUE_DEFAULT
#define CALLOUTBACKGROUND_ZVALUE_DEFAULT          98.0
#define CALLOUTINSTANCE_ZVALUE_DEFAULT          1000.0
#define CALLOUTPOINTER_ZVALUE_DEFAULT             POINTER_ZVALUE_DEFAULT
#define CALLOUTUNDERPINNING_ZVALUE_DEFAULT        97.0
#define DIVIDER_ZVALUE_DEFAULT                   100.0
#define DIVIDERLINE_ZVALUE_DEFAULT                99.0
#define DIVIDERBACKGROUND_ZVALUE_DEFAULT          98
#define DIVIDERPOINTER_ZVALUE_DEFAULT              POINTER_ZVALUE_DEFAULT
#define INSERTPIXMAP_ZVALUE_DEFAULT              500.0
#define INSERTTEXT_ZVALUE_DEFAULT               1000.0
#define MULTISTEPBACKGROUND_ZVALUE_DEFAULT        -2.0
#define MULTISTEPSBACKGROUND_ZVALUE_DEFAULT       -2.0
#define PAGEATTRIBUTETEXT_ZVALUE_DEFAULT           1.0
#define PAGEATTRIBUTEPIXMAP_ZVALUE_DEFAULT         Z_VALUE_DEFAULT
#define PAGEBACKGROUND_ZVALUE_DEFAULT             -3.0
#define PAGENUMBER_ZVALUE_DEFAULT                  Z_VALUE_DEFAULT
#define PAGEPOINTER_ZVALUE_DEFAULT                 POINTER_ZVALUE_DEFAULT
#define PAGEPOINTERBACKGROUND_ZVALUE_DEFAULT       Z_VALUE_DEFAULT
#define PARTSLISTANNOTATION_ZVALUE_DEFAULT         Z_VALUE_DEFAULT
#define PARTSLISTBACKGROUND_ZVALUE_DEFAULT         Z_VALUE_DEFAULT
#define PARTSLISTINSTANCE_ZVALUE_DEFAULT           Z_VALUE_DEFAULT
#define PARTSLISTPARTGROUP_ZVALUE_DEFAULT          Z_VALUE_DEFAULT
#define PARTSLISTPARTPIXMAP_ZVALUE_DEFAULT         Z_VALUE_DEFAULT
#define PLIGRABBER_ZVALUE_DEFAULT                  GRABBER_ZVALUE_DEFAULT
#define POINTERGRABBER_ZVALUE_DEFAULT              GRABBER_ZVALUE_DEFAULT
#define POINTERFIRSTSEG_ZVALUE_DEFAULT            -2.0
#define POINTERSECONDSEG_ZVALUE_DEFAULT           -3.0
#define POINTERTHIRDSEG_ZVALUE_DEFAULT            -4.0
#define POINTERHEAD_ZVALUE_DEFAULT                -5.0
#define ROTATEICONBACKGROUND_ZVALUE_DEFAULT      101.0
#define STEP_BACKGROUND_ZVALUE_DEFAULT             Z_VALUE_DEFAULT
#define STEPNUMBER_ZVALUE_DEFAULT                  Z_VALUE_DEFAULT
#define SUBMODELBACKGROUND_ZVALUE_DEFAULT          Z_VALUE_DEFAULT
#define SUBMODELGRABBER_ZVALUE_DEFAULT             GRABBER_ZVALUE_DEFAULT
#define SUBMODELINSTANCE_ZVALUE_DEFAULT            Z_VALUE_DEFAULT
#define SUBMODELINSTANCECOUNT_ZVALUE_DEFAULT       Z_VALUE_DEFAULT
#define STEP_BACKGROUND_ZVALUE_DEFAULT             Z_VALUE_DEFAULT

#define GLOBAL_META_RX             "^\\s*0\\s+!LPUB\\s+.*GLOBAL"

// registry sections
#define DEFAULTS                   "Defaults"
#define POVRAY                     "POVRay"
#define SETTINGS                   "Settings"
#define MESSAGES                   "Messages"
#define MAINWINDOW                 "MainWindow"
#define PARMSWINDOW                "ParmsWindow"
#define EDITWINDOW                 "EditWindow"
#define FINDREPLACEWINDOW          "FindReplaceWindow"
#define UPDATES                    "Updates"
#define LOGGING                    "Logging"

#define FADE_SFX                    "-fade"
#define HIGHLIGHT_SFX               "-highlight"
#define SAVE_DISPLAY_PAGE_NUM_KEY   "SaveDisplayPageNum"
#define SAVE_SKIP_PARTS_ARCHIVE_KEY "SaveSkipPartsArchive"
#define VIEW_PARTS_WIDGET_KEY       "ViewPartsWidget"
#define VIEW_COLORS_WIDGET_KEY      "ViewColorsWidget"
#define VIEW_EXPORT_TOOLBAR_KEY     "ViewExportToolbar"
#define VIEW_CACHE_TOOLBAR_KEY      "ViewCacheToolbar"
#define VIEW_SETUP_TOOLBAR_KEY      "ViewSetupToolbar"
#define VIEW_EDIT_TOOLBAR_KEY       "ViewEditToolbar"
#define VIEW_EDITPARAMS_TOOLBAR_KEY "ViewEditParamsToolbar"

#define CASE_CHECK    "CaseCheck"
#define WORD_CHECK    "WordCheck"
#define REGEXP_CHECK  "RegExpCheck"
#define DEFAULT       0
#define NEXT          1
#define PREVIOUS      2

#define RENDER_IMAGE_MAX_SIZE                 32768 // pixels
#define PLACEHOLDER_IMAGE_WIDTH               68    // pixels
#define PLACEHOLDER_IMAGE_HEIGHT              79

#define GRID_SIZE_INDEX_DEFAULT               1 // 20

#define GHOST_META    "0 GHOST"

#define MAX_NUM_POV_GEN_ARGS (128)

#define DOT_PATH_DEFAULT                        "."
#define EMPTY_STRING_DEFAULT                    ""

#define LOGGING_LEVEL_DEFAULT                   "STATUS"

#define LIBXQUARTZ_MACOS_VERSION                "11.0"   // base 11.0
#define LIBPNG_MACOS_VERSION                    "1.6.37" // base 1.6.34
#define LIBGL2PS_MACOS_VERSION                  "1.4.0"  // base 1.3.5
#define LIBJPEG_MACOS_VERSION                   "9c"     // base 8b
#define LIBXML_MACOS_VERSION                    "2.6.2"  // base 2.5.2
#define LIBMINIZIP_MACOS_VERSION                "1.2.11" // base 1.1.0

#define LIBTIFF_MACOS_VERSION                   "4.0.10" // base 3.6.1
#define LIBSDL_MACOS_VERSION                    "2.0.10" // base 2.0.2
#define LIBOPENEXR_MACOS_VERSION                "2.3.0"  // base 2.2.0
#define LIBILMBASE_MACOS_VERSION                "2.3.0"  // base 2.2.1

#define CAMERA_FOV_LDVIEW_P_DEFAULT              0.007f  // LPub3D (LDViez) default [Mimic Perspective]
#define CAMERA_FOV_LDVIEW_P_MAX_DEFAULT         90.0f    // LPub3D (LDViez) default

#define CAMERA_FOV_DEFAULT                       0.01f   // LPub3D (L3P) default [Orthographic]
#define CAMERA_FOV_MIN_DEFAULT                   0.0f    // LPub3D (L3P) default
#define CAMERA_FOV_MAX_DEFAULT                 360.0f    // LPub3D (L3P) default

#define CAMERA_ZNEAR_DEFAULT                    10.0f    // LPub3D (L3P) default
#define CAMERA_ZFAR_DEFAULT                   4000.0f    // LPub3D (L3P) default

#define CAMERA_FOV_NATIVE_DEFAULT               30.0f    // Native (LeoCAD) defaults
#define CAMERA_FOV_NATIVE_MIN_DEFAULT            1.0f    // Native (LeoCAD) default
#define CAMERA_FOV_NATIVE_MAX_DEFAULT          359.0f    // Native (LeoCAD) default
#define CAMERA_ZNEAR_NATIVE_DEFAULT             25.0f    // Native (LeoCAD) defaults
#define CAMERA_ZFAR_NATIVE_DEFAULT           50000.0f    // Native (LeoCAD) defaults

#define UPDATE_CHECK_FREQUENCY_DEFAULT          0        //0=Never,1=Daily,2=Weekly,3=Monthly

#define PAGE_HEIGHT_DEFAULT                     800
#define PAGE_WIDTH_DEFAULT                      600
#define LINE_WRAP_WIDTH                         160      //Text browser line width (change log)

#define STEP_SPACING_DEFAULT                    0.05f    // Step group step default spacing when center justified

#define PLAINTEXT_CHANGE_LOG_CUTOFF_VERSION    "2.2.2"   // last version of LPub3D to use html change notes

#define DURAT_LGEO_STL_LIB_INFO                 "LGEO Stl library is available"

#define SPLASH_FONT_COLOUR                      "#aa0000" // LPub3D Maroon

#define PREVIEW_SUBMODEL_SUFFIX                 "Preview"

#define VIEWER_MODEL_DEFAULT                    "LPub3D_PlcHldr.ldr"

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

#define POVRAY_RENDER_QUALITY_DEFAULT           0    // 0=High, 1-Medium, 2=Low
#define RENDERER_TIMEOUT_DEFAULT                6    // measured in seconds

#define PAGE_DISPLAY_PAUSE_DEFAULT              3    // measured in seconds
#define MAX_OPEN_WITH_PROGRAMS_DEFAULT          3    // maximum open with programs entries

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

#define HIGHLIGHT_COLOUR_DEFAULT                "#FFFF00"     // Bright Yellow
#define HIGHLIGHT_LINE_WIDTH_DEFAULT            1             // line width 1 - 5

#define SCENE_GUIDES_LINE_DEFAULT               2             // PenStyle - DashLine

// Team color supplements
#define THEME_DEFAULT                   "Default" // Default Theme
#define THEME_SCENE_BGCOLOR_DEFAULT     "#aeadac" // Qt Light Gray
#define THEME_GRID_PEN_DEFAULT          "#1e1b18" // Qt Double Dark Grey
#define THEME_RULER_TICK_PEN_DEFAULT    "#35322f" // Qt Dark Grey
#define THEME_RULER_TRACK_PEN_DEFAULT   "#00FF00" // Custom Neon Green
#define THEME_GUIDE_PEN_DEFAULT         "#aa0000" // LPub3D Maroon
#define THEME_TRANS_PAGE_BORDER_DEFAULT "#535559" // Custom Gray
#define THEME_VIEWER_BGCOLOR_DEFAULT    "#ffffff" // Qt White
#define THEME_NML_PEN_DEFAULT           "#1e1b18" // Qt Double Dark Grey [NOT USED]

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
#define THEME_HIGHLIGHT_29_DEFAULT      "#bd4900" // 29 [LDCad Group Define]

#define THEME_HIGHLIGHT_A_DEFAULT       "#006325" // Qt
#define THEME_HIGHLIGHT_B_DEFAULT       "#14148c" // Qt
#define THEME_HIGHLIGHT_C_DEFAULT       "#0057ff" // 06
#define THEME_HIGHLIGHT_D_DEFAULT       "#b25a2f" // 07

#define THEME_HIGHLIGHT_E_DEFAULT       "#084575" // br05 Part ID
#define THEME_HIGHLIGHT_F_DEFAULT       "#9421a6" // br06 Part Control
#define THEME_HIGHLIGHT_G_DEFAULT       "#aa0000" // br07 Part Description

// -----------------------------------------------//

#define THEME_DARK                      "Dark"    // Dark Theme
#define THEME_SCENE_BGCOLOR_DARK        "#31363b" // Custom Dark Grey
#define THEME_GRID_PEN_DARK             "#ffffff" // Qt White
#define THEME_RULER_TICK_PEN_DARK       "#eff0f1" // Custom Pale Grey
#define THEME_RULER_TRACK_PEN_DARK      "#00FF00" // Custom Neon Green
#define THEME_GUIDE_PEN_DARK            "#aa0000" // LPub3D Maroon
#define THEME_VIEWER_BGCOLOR_DARK       "#808B96" // Custom Grey
#define THEME_EDIT_MARGIN_DARK          "#ABB2B9" // Custom Gray
#define THEME_TRANS_PAGE_BORDER_DARK    "#aeadac" // Qt Light Gray
#define THEME_NML_PEN_DARK              "#ffffff" // Qt White [NOT USED]

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
#define THEME_HIGHLIGHT_29_DARK         "#bd4900" // 29 [LDCad Group Define]

#define THEME_HIGHLIGHT_A_DARK          "#17c723" // 01 Custom Green
#define THEME_HIGHLIGHT_B_DARK          "#0079cf" // 02 Custom Blue
#define THEME_HIGHLIGHT_C_DARK          "#ff5227" // 06 Custom Orange
#define THEME_HIGHLIGHT_D_DARK          "#bbbfff" // 07 Custom Violet

#define THEME_HIGHLIGHT_E_DARK          "#c3f6fe" // br05 Part ID
#define THEME_HIGHLIGHT_F_DARK          "#9421a6" // br06 Part Control
#define THEME_HIGHLIGHT_G_DARK          "#aa0000" // br07 Part Description

#define THEME_EDITWINDOW_LINE_DARK      "#5d5b59" // 06 Qt Medium Gray
#endif
