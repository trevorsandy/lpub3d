
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

struct ActionAttributes
{
    const char* ID;
    const char* MenuName;
    const char* StatusText;
};

struct StepLines {
  int top;
  int bottom;
  StepLines() : top(0), bottom(0) {}
  StepLines( int _top, int _bottom) :
      top(_top), bottom(_bottom) {}
  bool isInScope(int value)
  { return value >= top && value <= bottom; }
};

struct TypeLine {
  int modelIndex;
  int lineIndex;
  TypeLine() : modelIndex(-1), lineIndex(-1) {}
  TypeLine( int _modelIndex, int _lineIndex) :
      modelIndex(_modelIndex), lineIndex(_lineIndex) {}
};

enum PartType { FADE_PART, HIGHLIGHT_PART, NORMAL_PART, NUM_PART_TYPES };
enum PliType { PART, SUBMODEL, BOM, NUM_PLI_TYPES };
enum LogType { LOG_STATUS, LOG_INFO, LOG_TRACE, LOG_DEBUG, LOG_NOTICE, LOG_ERROR, LOG_INFO_STATUS, LOG_FATAL, LOG_QWARNING, LOG_QDEBUG };
enum CamFlag { DefFoV, DefZNear, DefZFar };
enum IniFlag { NativePOVIni, NativeSTLIni, Native3DSIni, NativePartList, POVRayRender, LDViewPOVIni, LDViewIni, NumIniFiles };
enum DividerType { StepDivider, RangeDivider, NoDivider };
enum ShowLoadMsgType { NEVER_SHOW, SHOW_ERROR, SHOW_WARNING, SHOW_MESSAGE, ALWAYS_SHOW };
enum LoadMsgType { MISSING_LOAD_MSG, PRIMITIVE_LOAD_MSG, SUBPART_LOAD_MSG, VALID_LOAD_MSG, ALL_LOAD_MSG };
enum MissingHeader { NoneMissing, NameMissing, AuthorMissing, BothMissing };
enum LDrawFileRegExp { SOF_RX, EOF_RX, LDR_RX, AUT_RX, NAM_RX, CAT_RX, DES_RX, LDG_RX };
enum RulerTrackingType { TRACKING_TICK, TRACKING_LINE, TRACKING_NONE};
enum SceneGuidesPosType { GUIDES_TOP_LEFT, GUIDES_TOP_RIGHT, GUIDES_CENTRE, GUIDES_BOT_LEFT, GUIDES_BOT_RIGHT};
enum RemoveObjectsRC { RemovedPieceRC, RemovedCameraRC, RemovedLightRC };
enum LibType { LibLEGO, LibTENTE, LibVEXIQ, NumLibs };
enum Theme { ThemeDark, ThemeDefault };
enum SaveOnSender { SaveOnNone, SaveOnRedraw, SaveOnUpdate };
enum NativeType { NTypeDefault, NTypeCalledOut, NTypeMultiStep };
enum DisplayModelType { DM_FINAL_MODEL = -1, DM_DISPLAY_MODEL = - 2 };
enum ShowLineType { PS_POS, LINE_HIGHLIGHT = PS_POS, PS_LAST_POS, LINE_ERROR = PS_LAST_POS, PS_STEP_NUM };
enum ProcessType { PROC_NONE, PROC_WRITE_TO_TMP, PROC_FIND_PAGE, PROC_DRAW_PAGE, PROC_DISPLAY_PAGE, PROC_COUNT_PAGE };
enum SceneObjectInfo { ObjectId };


enum PartSource {
    NOT_FOUND          = -1,               //  -1
    OUT_OF_BOUNDS      = NOT_FOUND,        //  -1
    NEW_PART           = NOT_FOUND,        //  -1
    NEW_MODEL          = NOT_FOUND,        //  -1
    EDITOR_LINE,                           //   0
    VIEWER_NONE        = EDITOR_LINE,      //   0
    VIEWER_LINE,                           //   1
    VIEWER_MOD,                            //   2
    VIEWER_DEL,                            //   3
    VIEWER_SEL,                            //   4
    VIEWER_CLR                             //   5
};

enum BuildModFlags {
    BM_LAST_ACTION      = -1,              //  -1
    BM_INVALID_INDEX    = BM_LAST_ACTION,  //  -1
    BM_BEGIN,                              //   0 MOD LEVEL BEGIN
    BM_FIRST_INDEX      = BM_BEGIN,        //   0
    BM_MODEL_NAME       = BM_BEGIN,        //   0
    BM_STEP_MODEL_KEY   = BM_BEGIN,        //   0
    BM_BEGIN_LINE_NUM   = BM_BEGIN,        //   0 BUILD_MOD BEGIN   BuildModRange[0] Attributes[0]
    BM_END_MOD,                            //   1
    BM_STEP_LINE_KEY    = BM_END_MOD,      //   1
    BM_ACTION_LINE_NUM  = BM_END_MOD,      //   1 BUILD_MOD END_MOD BuildModRange[1] Attributes[1]
    BM_LINE_NUMBER      = BM_END_MOD,      //   1
    BM_END,                                //   2 MOD LEVEL END
    BM_END_LINE_NUM     = BM_END,          //   2 BUILD_MOD END                      Attributes[2]
    BM_STEP_NUM_KEY     = BM_END,          //   2
    BM_MODEL_INDEX      = BM_END,          //   2 MODEL_INDEX       BuildModRange[2]
    BM_STEP_KEYS,                          //   3
    BM_DISPLAY_PAGE_NUM = BM_STEP_KEYS,    //   3                                    Attributes[3]
    BM_STEP_PIECES,                        //   4                                    Attributes[4]
    BM_MODEL_NAME_INDEX,                   //   5                                    Attributes[5]
    BM_MODEL_LINE_NUM,                     //   6                                    Attributes[6]
    BM_MODEL_STEP_NUM,                     //   7                                    Attributes[7]
    BM_SINGLE_STEP,                        //   8
    BM_MULTI_STEP,                         //   9
    BM_CALLOUT_STEP,                       //  10
    BM_CHANGE,                             //  11
    BM_DELETE                              //  12
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
    NUM_GRID_SIZES,
    GRID_SIZE_LAST = NUM_GRID_SIZES
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

extern ActionAttributes sgCommands[NUM_GRID_SIZES];

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

// Theme colours

enum ThemeColorType {
    // THEME_DEFAULT
    THEME_DEFAULT_SCENE_BGCOLOR,                             // "#aeadac"
    THEME_DEFAULT_GRID_PEN,                                  // "#1e1b18"
    THEME_DEFAULT_RULER_TICK_PEN,                            // "#35322f"
    THEME_DEFAULT_RULER_TRACK_PEN,                           // "#00FF00"
    THEME_DEFAULT_GUIDE_PEN,                                 // "#aa0000"
    THEME_DEFAULT_TRANS_PAGE_BORDER,                         // "#535559"
    THEME_DEFAULT_NML_PEN,                                   // "#1e1b18"

    THEME_DEFAULT_PALETTE_LIGHT,                             // "#aeadac"

    THEME_DEFAULT_VIEWER_BACKGROUND_COLOR,                   // "#FFFFFF"
    THEME_DEFAULT_VIEWER_GRADIENT_COLOR_TOP,                 // "#36485f"
    THEME_DEFAULT_VIEWER_GRADIENT_COLOR_BOTTOM,              // "#313437"
    THEME_DEFAULT_AXES_COLOR,                                // "#000000"
    THEME_DEFAULT_OVERLAY_COLOR,                             // "#000000"
    THEME_DEFAULT_ACTIVE_VIEW_COLOR,                         // "#FF0000"
    THEME_DEFAULT_ACTIVE_PREVIEW_COLOR,                      // "#454545"
    THEME_DEFAULT_GRID_STUD_COLOR,                           // "#404040"
    THEME_DEFAULT_GRID_LINE_COLOR,                           // "#000000"
    THEME_DEFAULT_VIEW_SPHERE_COLOR,                         // "#FFFFFF"
    THEME_DEFAULT_VIEW_SPHERE_TEXT_COLOR,                    // "#646464"
    THEME_DEFAULT_VIEW_SPHERE_HLIGHT_COLOR,                  // "#FF0000"

    THEME_DEFAULT_DECORATE_LDRAW_COMMENTS,                   // "#006325"
    THEME_DEFAULT_DECORATE_LDRAW_HEADER,                     // "#4f97ba"
    THEME_DEFAULT_DECORATE_LDRAW_BODY,                       // "#005af6"
    THEME_DEFAULT_DECORATE_LPUB3D_LOCAL,                     // "#609cff"
    THEME_DEFAULT_DECORATE_LPUB3D_GLOBAL,                    // "#57649b"

    THEME_DEFAULT_DECORATE_LDRAW_LINE_TYPE_1,                // "#816288"
    THEME_DEFAULT_DECORATE_LDRAW_COLOUR_CODE,                // "#f46105"
    THEME_DEFAULT_DECORATE_LDRAW_PART_POSITION,              // "#ed3d63"
    THEME_DEFAULT_DECORATE_LDRAW_PART_TRANSFORM_1,           // "#8bc3f6"
    THEME_DEFAULT_DECORATE_LDRAW_PART_TRANSFORM_2,           // "#51acff"
    THEME_DEFAULT_DECORATE_LDRAW_PART_TRANSFORM_3,           // "#2f86ff"
    THEME_DEFAULT_DECORATE_LDRAW_PART_FILE,                  // "#084575"
    THEME_DEFAULT_DECORATE_LDRAW_LINE_TYPES_2_TO_5,          // "#7d5e18"
    THEME_DEFAULT_DECORATE_LPUB3D_NUMBER,                    // "#fc9b14"
    THEME_DEFAULT_DECORATE_LPUB3D_HEX_NUMBER,                // "#b87620"
    THEME_DEFAULT_DECORATE_LPUB3D_PAGE_SIZE,                 // "#c15317"

    THEME_DEFAULT_DECORATE_LEOCAD,                           // "#14148c"
    THEME_DEFAULT_DECORATE_LSYNTH,                           // "#d882f6"
    THEME_DEFAULT_DECORATE_LDCAD,                            // "#ff9900"
    THEME_DEFAULT_DECORATE_MLCAD,                            // "#0079cf"
    THEME_DEFAULT_DECORATE_MLCAD_BODY,                       // "#9421a6"
    THEME_DEFAULT_DECORATE_LPUB3D_FALSE,                     // "#ff1818"
    THEME_DEFAULT_DECORATE_LPUB3D_TRUE,                      // "#008000"
    THEME_DEFAULT_DECORATE_LPUB3D,                           // "#cc5980"
    THEME_DEFAULT_DECORATE_LPUB3D_BODY,                      // "#aa0000"
    THEME_DEFAULT_DECORATE_LDRAW_HEADER_VALUE,               // "#545454"
    THEME_DEFAULT_DECORATE_LPUB3D_QUOTED_TEXT,               // "#231aff"
    THEME_DEFAULT_DECORATE_LDRAW_LINE_TYPE_0_FIRST_CHARACTER,// "#000000"
    THEME_DEFAULT_DECORATE_LDCAD_GROUP_DEFINE,               // "#bd4900"

    THEME_DEFAULT_DECORATE_INI_FILE_COMMENT,                 // "#006325"
    THEME_DEFAULT_DECORATE_INI_FILE_HEADER,                  // "#14148c"
    THEME_DEFAULT_DECORATE_INI_FILE_EQUAL,                   // "#0057ff"
    THEME_DEFAULT_DECORATE_INI_FILE_VALUE,                   // "#b25a2f"

    THEME_DEFAULT_DECORATE_PARAMETER_FILE_PART_ID,           // "#084575"
    THEME_DEFAULT_DECORATE_PARAMETER_FILE_PART_CONTROL,      // "#9421a6"
    THEME_DEFAULT_DECORATE_PARAMETER_FILE_PART_DESCRIPTION,  // "#aa0000"

    THEME_DEFAULT_LINE_HIGHLIGHT,                            // "#ccccff"
    THEME_DEFAULT_LINE_ERROR,                                // "#ffcccc"

    // THEME_DARK
    THEME_DARK_SCENE_BGCOLOR,                                // "#31363b"
    THEME_DARK_GRID_PEN,                                     // "#ffffff"
    THEME_DARK_RULER_TICK_PEN,                               // "#eff0f1"
    THEME_DARK_RULER_TRACK_PEN,                              // "#00FF00"
    THEME_DARK_GUIDE_PEN,                                    // "#aa0000"
    THEME_DARK_TRANS_PAGE_BORDER,                            // "#aeadac"
    THEME_DARK_RULER_PEN,                                    // "#ffffff"
    THEME_DARK_EDIT_MARGIN,                                  // "#ABB2B9"

    THEME_DARK_DECORATE_LDRAW_COMMENTS,                      // "#17c723"
    THEME_DARK_DECORATE_LDRAW_HEADER,                        // "#fb743e"
    THEME_DARK_DECORATE_LDRAW_BODY,                          // "#14aaff"
    THEME_DARK_DECORATE_LPUB3D_LOCAL,                        // "#609cff"
    THEME_DARK_DECORATE_LPUB3D_GLOBAL,                       // "#a0b2e2"

    THEME_DARK_DECORATE_LDRAW_LINE_TYPE_1,                   // "#c2a4c0"
    THEME_DARK_DECORATE_LDRAW_COLOUR_CODE,                   // "#f46105"
    THEME_DARK_DECORATE_LDRAW_PART_POSITION,                 // "#46b5d1"
    THEME_DARK_DECORATE_LDRAW_PART_TRANSFORM_1,              // "#ddffef"
    THEME_DARK_DECORATE_LDRAW_PART_TRANSFORM_2,              // "#a2ffd2"
    THEME_DARK_DECORATE_LDRAW_PART_TRANSFORM_3,              // "#5cffb0"
    THEME_DARK_DECORATE_LDRAW_PART_FILE,                     // "#c3f6fe"
    THEME_DARK_DECORATE_LDRAW_LINE_TYPES_2_TO_5,             // "#7d5e18"
    THEME_DARK_DECORATE_LPUB3D_NUMBER,                       // "#fc9b14"
    THEME_DARK_DECORATE_LPUB3D_HEX_NUMBER,                   // "#b87620"
    THEME_DARK_DECORATE_LPUB3D_PAGE_SIZE,                    // "#c15317"

    THEME_DARK_DECORATE_LEOCAD,                              // "#0079cf"
    THEME_DARK_DECORATE_LSYNTH,                              // "#ff9900"
    THEME_DARK_DECORATE_LDCAD,                               // "#ff9900"
    THEME_DARK_DECORATE_MLCAD,                               // "#0079cf"
    THEME_DARK_DECORATE_MLCAD_BODY,                          // "#9421a6"
    THEME_DARK_DECORATE_LPUB3D_FALSE,                        // "#ff8080"
    THEME_DARK_DECORATE_LPUB3D_TRUE,                         // "#adff2f"
    THEME_DARK_DECORATE_LPUB3D,                              // "#f638dc"
    THEME_DARK_DECORATE_LPUB3D_BODY,                         // "#ff3366"
    THEME_DARK_DECORATE_LDRAW_HEADER_VALUE,                  // "#aeaeae"
    THEME_DARK_DECORATE_LPUB3D_QUOTED_TEXT,                  // "#81d4fa"
    THEME_DARK_DECORATE_LDRAW_LINE_TYPE_0_FIRST_CHARACTER,   // "#ffffff"
    THEME_DARK_DECORATE_LDCAD_GROUP_DEFINE,                  // "#bd4900"

    THEME_DARK_DECORATE_INI_FILE_COMMENT,                    // "#17c723"
    THEME_DARK_DECORATE_INI_FILE_HEADER,                     // "#0079cf"
    THEME_DARK_DECORATE_INI_FILE_EQUAL,                      // "#ff5227"
    THEME_DARK_DECORATE_INI_FILE_VALUE,                      // "#bbbfff"

    THEME_DARK_DECORATE_PARAMETER_FILE_PART_ID,              // "#c3f6fe"
    THEME_DARK_DECORATE_PARAMETER_FILE_PART_CONTROL,         // "#9421a6"
    THEME_DARK_DECORATE_PARAMETER_FILE_PART_DESCRIPTION,     // "#aa0000"

    THEME_DARK_LINE_HIGHLIGHT,                               // "#5d5b59"
    THEME_DARK_LINE_ERROR,                                   // "#ff0000"

    THEME_DARK_PALETTE_WINDOW,                               // "#313437"
    THEME_DARK_PALETTE_WINDOW_TEXT,                          // "#F0F0F0"
    THEME_DARK_PALETTE_BASE,                                 // "#232629"
    THEME_DARK_PALETTE_ALT_BASE,                             // "#2C2F32"
    THEME_DARK_PALETTE_TIP_BASE,                             // "#E0E0F4"
    THEME_DARK_PALETTE_TIP_TEXT,                             // "#3A3A3A"

    THEME_DARK_PALETTE_PHOLDER_TEXT,                         // "#646464"

    THEME_DARK_PALETTE_TEXT,                                 // "#E0E0E0"
    THEME_DARK_PALETTE_BUTTON,                               // "#2D3033"
    THEME_DARK_PALETTE_BUTTON_TEXT,                          // "#E0E0F4"
    THEME_DARK_PALETTE_LIGHT,                                // "#414141"
    THEME_DARK_PALETTE_MIDLIGHT,                             // "#3E3E3E"
    THEME_DARK_PALETTE_DARK,                                 // "#232323"
    THEME_DARK_PALETTE_MID,                                  // "#323232"
    THEME_DARK_PALETTE_SHADOW,                               // "#141414"
    THEME_DARK_PALETTE_HILIGHT,                              // "#2980B9"
    THEME_DARK_PALETTE_HILIGHT_TEXT,                         // "#E8E8E8"
    THEME_DARK_PALETTE_LINK,                                 // "#2980B9"
    THEME_DARK_PALETTE_DISABLED_TEXT,                        // "#808080"

    THEME_DARK_VIEWER_BACKGROUND_COLOR,                      // "#313437"
    THEME_DARK_VIEWER_GRADIENT_COLOR_TOP,                    // "#0000bf"
    THEME_DARK_VIEWER_GRADIENT_COLOR_BOTTOM,                 // "#FFFFFF"
    THEME_DARK_AXES_COLOR,                                   // "#000000"
    THEME_DARK_OVERLAY_COLOR,                                // "#000000"
    THEME_DARK_ACTIVE_VIEW_COLOR,                            // "#2980B9"
    THEME_DARK_ACTIVE_PREVIEW_COLOR,                         // "#999999"
    THEME_DARK_GRID_STUD_COLOR,                              // "#181818"
    THEME_DARK_GRID_LINE_COLOR,                              // "#181818"
    THEME_DARK_VIEW_SPHERE_COLOR,                            // "#232629"
    THEME_DARK_VIEW_SPHERE_TEXT_COLOR,                       // "#E0E0E0"
    THEME_DARK_VIEW_SPHERE_HLIGHT_COLOR,                     // "#2980B9"

    THEME_DARK_GRAPHICSVIEW_BORDER_COLOR,                    // "#999999"
    THEME_NUM_COLORS                                         // 136
};

// Color picker: https://www.w3schools.com/colors/colors_picker.asp

#define LPUB3D_DEFAULT_COLOUR                  "#aa0000" // 170,   0,   0, 255 LPub3D Maroon

#define LPUB3D_DISABLED_TEXT_COLOUR            "#808080" // 128, 128, 128, 255

#define SPLASH_FONT_COLOUR                      LPUB3D_DEFAULT_COLOUR

#define THEME_DEFAULT                          "Default" // Default Theme

#define THEME_DARK                             "Dark"    // Dark Theme



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

#define GLOBAL_META_RX                              "^\\s*0\\s+!LPUB\\s+.*GLOBAL"

// registry sections
#define DEFAULTS                               "Defaults"
#define POVRAY                                 "POVRay"
#define SETTINGS                               "Settings"
#define MESSAGES                               "Messages"
#define MAINWINDOW                             "MainWindow"
#define PARMSWINDOW                            "ParmsWindow"
#define EDITWINDOW                             "EditWindow"
#define FINDREPLACEWINDOW                      "FindReplaceWindow"
#define UPDATES                                "Updates"
#define LOGGING                                "Logging"
#define IMPORTLDRAW                            "importldraw"
#define IMPORTDEFAULT                          "DEFAULT"
#define THEMECOLORS                            "ThemeColors"

#define FADE_SFX                                "-fade"
#define HIGHLIGHT_SFX                           "-highlight"
#define SAVE_DISPLAY_PAGE_NUM_KEY               "SaveDisplayPageNum"
#define SAVE_SKIP_PARTS_ARCHIVE_KEY             "SaveSkipPartsArchive"
#define VIEW_EXPORT_TOOLBAR_KEY                 "ViewExportToolbar"
#define VIEW_CACHE_TOOLBAR_KEY                  "ViewCacheToolbar"
#define VIEW_SETUP_TOOLBAR_KEY                  "ViewSetupToolbar"
#define VIEW_EDIT_TOOLBAR_KEY                   "ViewEditToolbar"
#define VIEW_EDITPARAMS_TOOLBAR_KEY             "ViewEditParamsToolbar"

#define CASE_CHECK                              "CaseCheck"
#define WORD_CHECK                              "WordCheck"
#define REGEXP_CHECK                            "RegExpCheck"
#define DEFAULT                                 0
#define NEXT                                    1
#define PREVIOUS                                2


#define RENDER_IMAGE_MAX_SIZE                   32768 // pixels
#define PLACEHOLDER_IMAGE_WIDTH                 68    // pixels
#define PLACEHOLDER_IMAGE_HEIGHT                79

#define STYLE_SIZE_DEFAULT                      0.28f // annotation style width, height, diameter in inches

#define GRID_SIZE_INDEX_DEFAULT                 1 // 20

#define GHOST_META                              "0 GHOST"

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

#define CAMERA_FOV_NATIVE_MIN_DEFAULT            1.0f    // Native (LeoCAD) default
#define CAMERA_FOV_NATIVE_MAX_DEFAULT          359.0f    // Native (LeoCAD) default

#define UPDATE_CHECK_FREQUENCY_DEFAULT          0        //0=Never,1=Daily,2=Weekly,3=Monthly

#define PAGE_HEIGHT_DEFAULT                     800
#define PAGE_WIDTH_DEFAULT                      600
#define LINE_WRAP_WIDTH                         160      //Text browser line width (change log)

#define STEP_SPACING_DEFAULT                    0.05f    // Step group step default spacing when center justified

#define PLAINTEXT_CHANGE_LOG_CUTOFF_VERSION    "2.2.2"   // last version of LPub3D to use html change notes

#define DURAT_LGEO_STL_LIB_INFO                 "LGEO Stl library is available"

#define PREVIEW_SUBMODEL_SUFFIX                 "Preview"
#define PREVIEW_MODEL_DEFAULT                   "Preview.ldr"
#define VIEWER_MODEL_DEFAULT                    "Model.ldr"

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
#define MESSAGE_LINE_WIDTH_DEFAULT             80    // default width of message line in characters
// Internal common material colours
#define LDRAW_EDGE_MATERIAL_COLOUR              "24"
#define LDRAW_MAIN_MATERIAL_COLOUR              "16"
#define LDRAW_MATERIAL_COLOUR                    16

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

#define PUBLISH_DESCRIPTION_DEFAULT            "LDraw model"

#define EDITOR_MIN_LINES_DEFAULT                300           // minimum number of lines to capture at each data read
#define EDITOR_MAX_LINES_DEFAULT                10000         // maximum number of lines to capture at each data read
#define EDITOR_DECORATION_DEFAULT               1             // 0 = simple, 1 = fancy

#define MPD_COMBO_MIN_ITEMS_DEFAULT             25
#define GO_TO_PAGE_MIN_ITEMS_DEFAULT            10

#endif // NAME_H
