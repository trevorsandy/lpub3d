/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.
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
#include <QDesktopServices>
#include <QSettings>
#include <QFileInfo>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QDate>
#include <JlCompress.h>
#include <LDVQt/LDVWidget.h>

#include "lpub_preferences.h"
#include "ui_preferences.h"
#include "preferencesdialog.h"
#include "updatecheck.h"
#include "resolution.h"
#include "render.h"
#include "pli.h"
#include "version.h"
#include "declarations.h"
#include "paths.h"
#include "ldrawcolourparts.h"
#include "application.h"
#include "lpub_qtcompat.h"
#include "messageboxresizable.h"
#include "lpub_object.h"

#include "lc_library.h"
#include "lc_profile.h"
#include "lc_view.h"

Preferences preferences;

QHash<QString, int> rendererMap;

const QString studStyleNames[StudStyleEnc::StyleCount] =
{
    "Plain",             // 0
    "Thin Lines Logo",   // 1
    "Outline Logo",      // 2
    "Sharp Top Logo",    // 3
    "Rounded Top Logo",  // 4
    "Flattened Logo",    // 5
    "High Contrast",     // 6
    "High Contrast Logo" // 7
};

const QString rendererNames[NUM_RENDERERS] =
{
  "Native",  // RENDERER_NATIVE
  "LDView",  // RENDERER_LDVIEW
  "LDGLite", // RENDERER_LDGLITE
  "POVRay"   // RENDERER_POVRAY
};

const QString MsgKeys[Preferences::NumKeys] =
{
    "ShowLineParseErrors",    // ParseErrors
    "ShowInsertErrors",       // InsertErrors
    "ShowBuildModErrors",     // BuildModErrors
    "ShowIncludeLineErrors",  // IncludeFileErrors
    "ShowAnnotationErrors"    // AnnotationErrors
};

const QString msgKeyTypes [][2] = {
   // Message Title,      Type Description
   {"Command",            "command message"},       //ParseErrors
   {"Insert",             "insert message"},        //InsertErrors
   {"Build Modification", "build modification"},    //BuildModErrors
   {"Include File",       "include file message" }, //IncludeFileErrors
   {"Annoatation",        "annotation message"}     //AnnotationErrors
};

Preferences::ThemeSettings Preferences::defaultThemeColors[THEME_NUM_COLORS] =
{   //KEY                                                COLOR      LABEL                                          // THEME_DEFAULT
    {"ThemeDefaultSceneBackgroundColor",                 "#AEADAC", "Scene Background"                          }, // THEME_DEFAULT_SCENE_BACKGROUND_COLOR
    {"ThemeDefaultGridPen",                              "#1E1B18", "Grid Pen"                                  }, // THEME_DEFAULT_GRID_PEN
    {"ThemeDefaultRulerPen",                             "#1E1B18", "Ruler Pen (uses Grid Pen)"                 }, // THEME_DEFAULT_RULER_PEN [NOT USED]
    {"ThemeDefaultRulerTickPen",                         "#35322F", "Ruler Tick Pen"                            }, // THEME_DEFAULT_RULER_TICK_PEN
    {"ThemeDefaultRulerTrackPen",                        "#00FF00", "Ruler Track Pen"                           }, // THEME_DEFAULT_RULER_TRACK_PEN
    {"ThemeDefaultGuidePen",                             "#AA0000", "Guide Pen"                                 }, // THEME_DEFAULT_GUIDE_PEN
    {"ThemeDefaultTransPageBorder",                      "#535559", "Page Hidden Background Border"             }, // THEME_DEFAULT_TRANS_PAGE_BORDER

    {"ThemeDefaultPaletteLight",                         "#AEADAC", "Palette Light"                             }, // THEME_DEFAULT_PALETTE_LIGHT

    {"ThemeDefaultViewerBackgroundColor",                "#FFFFFF", "Viewer Background"                         }, // THEME_DEFAULT_VIEWER_BACKGROUND_COLOR                      255, 255, 255, 255 LC_PROFILE_BACKGROUND_COLOR
    {"ThemeDefaultViewerGradientColorTop",               "#36485F", "Viewer Gradient Top"                       }, // THEME_DEFAULT_VIEWER_GRADIENT_COLOR_TOP                     54,  72,  95, 255 LC_PROFILE_GRADIENT_COLOR_TOP
    {"ThemeDefaultViewerGradientColorBottom",            "#313437", "Viewer Gradient Bottom"                    }, // THEME_DEFAULT_VIEWER_GRADIENT_COLOR_BOTTOM                  49,  52,  55, 255 LC_PROFILE_GRADIENT_COLOR_BOTTOM
    {"ThemeDefaultAxesColor",                            "#000000", "Viewer Axes"                               }, // THEME_DEFAULT_AXES_COLOR                                     0,   0,   0, 255 LC_PROFILE_AXES_COLOR
    {"ThemeDefaultOverlayColor",                         "#000000", "Viewer Overlay"                            }, // THEME_DEFAULT_OVERLAY_COLOR                                  0,   0,   0, 255 LC_PROFILE_OVERLAY_COLOR
    {"ThemeDefaultMarqueeBorderColor",                   "#4040FF", "Viewer Marquee Border"                     }, // THEME_DEFAULT_MARQUEE_BORDER_COLOR                          64,  64, 255, 255 LC_PROFILE_MARQUEE_BORDER_COLOR
    {"ThemeDefaultMarqueeFillColor",                     "#4040FF", "Viewer Marquee Fill"                       }, // THEME_DEFAULT_MARQUEE_FILL_COLOR                            64,  64, 255,  64 LC_PROFILE_MARQUEE_FILL_COLOR
    {"ThemeDefaultInactiveViewColor",                    "#454545", "Viewer Inactive View"                      }, // THEME_DEFAULT_INACTIVE_VIEW_COLOR                           69,  69,  69, 255 LC_PROFILE_INACTIVE_VIEW_COLOR
    {"ThemeDefaultActiveViewColor",                      "#FF0000", "Viewer Active View"                        }, // THEME_DEFAULT_ACTIVE_VIEW_COLOR                            255,   0,   0, 255 LC_PROFILE_ACTIVE_VIEW_COLOR
    {"ThemeDefaultGridStudColor",                        "#404040", "Viewer Grid Stud"                          }, // THEME_DEFAULT_GRID_STUD_COLOR                               64,  64,  64, 192 LC_PROFILE_GRID_STUD_COLOR alpha(192)
    {"ThemeDefaultGridLineColor",                        "#000000", "Viewer Grid Line"                          }, // THEME_DEFAULT_GRID_LINE_COLOR                                0,   0,   0, 255 LC_PROFILE_GRID_LINE_COLOR
    {"ThemeDefaultTextColor",                            "#000000", "Viewer Text"                               }, // THEME_DEFAULT_TEXT_COLOR                                     0,   0,   0, 255 LC_PROFILE_TEXT_COLOR
    {"ThemeDefaultViewSphereColor",                      "#FFFFFF", "Viewer View Sphere"                        }, // THEME_DEFAULT_VIEW_SPHERE_COLOR                            255, 255, 255, 255 LC_PROFILE_VIEW_SPHERE_COLOR
    {"ThemeDefaultViewSphereTextColor",                  "#646464", "Viewer View Sphere Text"                   }, // THEME_DEFAULT_VIEW_SPHERE_TEXT_COLOR                       100, 100, 100, 255 LC_PROFILE_VIEW_SPHERE_TEXT_COLOR
    {"ThemeDefaultViewSphereHlightColor",                "#FF0000", "Viewer View Sphere Hlight"                 }, // THEME_DEFAULT_VIEW_SPHERE_HLIGHT_COLOR                     255,   0,   0, 255 LC_PROFILE_VIEW_SPHERE_HIGHLIGHT_COLOR

    {"ThemeDefaultObjectSelectedColor",                  "#E54C66", "Object Selected Color"                     }, // THEME_DEFAULT_OBJECT_SELECTED_COLOR                        229,  76, 102, 255 LC_PROFILE_OBJECT_SELECTED_COLOR
    {"ThemeDefaultObjectFocusedColor",                   "#664CE5", "Object Focused Color "                     }, // THEME_DEFAULT_OBJECT_FOCUSED_COLOR                         102,  76, 229, 255 LC_PROFILE_OBJECT_FOCUSED_COLOR
    {"ThemeDefaultCameraColor",                          "#80CC80", "Camera Color"                              }, // THEME_DEFAULT_CAMERA_COLOR                                 128, 204, 128, 255 LC_PROFILE_CAMERA_COLOR
    {"ThemeDefaultLightColor",                           "#80CC80", "Light Color"                               }, // THEME_DEFAULT_LIGHT_COLOR                                  128, 204, 128, 255 LC_PROFILE_LIGHT_COLOR
    {"ThemeDefaultControlPointColor",                    "#80CC80", "Control Point Color"                       }, // THEME_DEFAULT_CONTROL_POINT_COLOR                          128, 204, 128, 128 LC_PROFILE_CONTROL_POINT_COLOR
    {"ThemeDefaultControlPointFocusedColor",             "#664CE5", "Control Point Focused Color"               }, // THEME_DEFAULT_CONTROL_POINT_FOCUSED_COLOR                  102,  76, 229, 128 LC_PROFILE_CONTROL_POINT_FOCUSED_COLOR
    {"ThemeDefaultBMObjectSelectedColor",                "#79D879", "Build Modification Object Selected Color"  }, // THEME_DEFAULT_BM_OBJECT_SELECTED_COLOR                     121, 216, 121, 255 LC_PROFILE_BM_OBJECT_SELECTED_COLOR

    {"ThemeDefaultDecorateLDrawComments",                "#006325", "Decorate LDraw Comments"                   }, // THEME_DEFAULT_DECORATE_LDRAW_COMMENTS                     br01
    {"ThemeDefaultDecorateLDrawHeader",                  "#4F97BA", "Decorate LDraw Header"                     }, // THEME_DEFAULT_DECORATE_LDRAW_HEADER                       br02
    {"ThemeDefaultDecorateLDrawBody",                    "#005AF6", "Decorate LDraw Body"                       }, // THEME_DEFAULT_DECORATE_LDRAW_BODY                         br03
    {"ThemeDefaultDecorateLPub3DLocal",                  "#609CFF", "Decorate LPub3D Local"                     }, // THEME_DEFAULT_DECORATE_LPUB3D_LOCAL                       br04
    {"ThemeDefaultDecorateLPub3DGlobal",                 "#57649B", "Decorate LPub3D Global"                    }, // THEME_DEFAULT_DECORATE_LPUB3D_GLOBAL                      br05

    {"ThemeDefaultDecorateLDrawLineType1",               "#816288", "Decorate LDraw Line Type 1"                }, // THEME_DEFAULT_DECORATE_LDRAW_LINE_TYPE_1                  br06
    {"ThemeDefaultDecorateLDrawColourCode",              "#F46105", "Decorate LDraw Colour Code"                }, // THEME_DEFAULT_DECORATE_LDRAW_COLOUR_CODE                  br07
    {"ThemeDefaultDecorateLDrawPartPosition",            "#ED3D63", "Decorate LDraw Part Position"              }, // THEME_DEFAULT_DECORATE_LDRAW_PART_POSITION                br08
    {"ThemeDefaultDecorateLDrawPartTransform1",          "#8BC3F6", "Decorate LDraw Part Transform 1"           }, // THEME_DEFAULT_DECORATE_LDRAW_PART_TRANSFORM_1             br09
    {"ThemeDefaultDecorateLDrawPartTransform2",          "#51ACFF", "Decorate LDraw Part Transform 2"           }, // THEME_DEFAULT_DECORATE_LDRAW_PART_TRANSFORM_2             br10
    {"ThemeDefaultDecorateLDrawPartTransform3",          "#2F86FF", "Decorate LDraw Part Transform 3"           }, // THEME_DEFAULT_DECORATE_LDRAW_PART_TRANSFORM_3             br11
    {"ThemeDefaultDecorateLDrawPartFile",                "#084575", "Decorate LDraw Part File"                  }, // THEME_DEFAULT_DECORATE_LDRAW_PART_FILE                    br12
    {"ThemeDefaultDecorateLDrawLineTypes2To5",           "#7D5E18", "Decorate LDraw Line Types 2 To 5"          }, // THEME_DEFAULT_DECORATE_LDRAW_LINE_TYPES_2_TO_5            br13
    {"ThemeDefaultDecorateLPub3DNumber",                 "#FC9B14", "Decorate LPub3D Number"                    }, // THEME_DEFAULT_DECORATE_LPUB3D_NUMBER                      br14
    {"ThemeDefaultDecorateLPub3DHexNumber",              "#B87620", "Decorate LPub3D Hex Number"                }, // THEME_DEFAULT_DECORATE_LPUB3D_HEX_NUMBER                  br15
    {"ThemeDefaultDecorateLPub3DPageSize",               "#C15317", "Decorate LPub3D Page Size"                 }, // THEME_DEFAULT_DECORATE_LPUB3D_PAGE_SIZE                   br16

    {"ThemeDefaultDecorateLeoCAD",                       "#14148C", "Decorate LeoCAD"                           }, // THEME_DEFAULT_DECORATE_LEOCAD                             br17
    {"ThemeDefaultDecorateLSynth",                       "#D882F6", "Decorate LSynth"                           }, // THEME_DEFAULT_DECORATE_LSYNTH                             br18
    {"ThemeDefaultDecorateLDCad",                        "#FF9900", "Decorate LDCad"                            }, // THEME_DEFAULT_DECORATE_LDCAD                              br19
    {"ThemeDefaultDecorateMLCad",                        "#0079CF", "Decorate MLCad"                            }, // THEME_DEFAULT_DECORATE_MLCAD                              br20
    {"ThemeDefaultDecorateMLCadBody",                    "#9421A6", "Decorate MLCad Body"                       }, // THEME_DEFAULT_DECORATE_MLCAD_BODY                         br21
    {"ThemeDefaultDecorateLPub3DFalse",                  "#FF1818", "Decorate LPub3D False"                     }, // THEME_DEFAULT_DECORATE_LPUB3D_FALSE                       br22
    {"ThemeDefaultDecorateLPub3DTrue",                   "#008000", "Decorate LPub3D True"                      }, // THEME_DEFAULT_DECORATE_LPUB3D_TRUE                        br23
    {"ThemeDefaultDecorateLPub3D",                       "#CC5980", "Decorate LPub3D"                           }, // THEME_DEFAULT_DECORATE_LPUB3D                             br24
    {"ThemeDefaultDecorateLPub3DBody",                   "#AA0000", "Decorate LPub3D Body"                      }, // THEME_DEFAULT_DECORATE_LPUB3D_BODY                        br25
    {"ThemeDefaultDecorateLDrawHeaderValue",             "#545454", "Decorate LDraw Header Value"               }, // THEME_DEFAULT_DECORATE_LDRAW_HEADER_VALUE                 br26
    {"ThemeDefaultDecorateLPub3DQuotedText",             "#231AFF", "Decorate LPub3D Quoted Text"               }, // THEME_DEFAULT_DECORATE_LPUB3D_QUOTED_TEXT                 br27
    {"ThemeDefaultDecorateLDrawLineType0FirstCharacter", "#000000", "Decorate LDraw Line Type 0 First Character"}, // THEME_DEFAULT_DECORATE_LDRAW_LINE_TYPE_0_FIRST_CHARACTER  br28
    {"ThemeDefaultDecorateLDCadGroupDefine",             "#BD4900", "Decorate LDCad Group Define"               }, // THEME_DEFAULT_DECORATE_LDCAD_GROUP_DEFINE                 br29

    {"ThemeDefaultDecorateIniFileComment",               "#006325", "Decorate Ini File Comment"                 }, // THEME_DEFAULT_DECORATE_INI_FILE_COMMENT                   br01
    {"ThemeDefaultDecorateIniFileHeader",                "#14148C", "Decorate Ini File Header"                  }, // THEME_DEFAULT_DECORATE_INI_FILE_HEADER                    br02
    {"ThemeDefaultDecorateIniFileEqual",                 "#0057FF", "Decorate Ini File Equal"                   }, // THEME_DEFAULT_DECORATE_INI_FILE_EQUAL                     br03
    {"ThemeDefaultDecorateIniFileValue",                 "#B25A2F", "Decorate Ini File Value"                   }, // THEME_DEFAULT_DECORATE_INI_FILE_VALUE                     br04

    {"ThemeDefaultDecorateParameterFilePartId",          "#084575", "Decorate Parameter File Part Id"           }, // THEME_DEFAULT_DECORATE_PARAMETER_FILE_PART_ID             br05
    {"ThemeDefaultDecorateParameterFilePartControl",     "#9421A6", "Decorate Parameter File Part Control"      }, // THEME_DEFAULT_DECORATE_PARAMETER_FILE_PART_CONTROL        br06
    {"ThemeDefaultDecorateParameterFilePartDescription", "#AA0000", "Decorate Parameter File Part Description"  }, // THEME_DEFAULT_DECORATE_PARAMETER_FILE_PART_DESCRIPTION    br07

    {"ThemeDefaultLineSelect",                           "#AA0000", "Editor Line Text Select"                   }, // THEME_DEFAULT_LINE_SELECT                                 alpha(30)
    {"ThemeDefaultLineHighlight",                        "#CCCCFF", "Editor Line Highlight"                     }, // THEME_DEFAULT_LINE_HIGHLIGHT
    {"ThemeDefaultLineError",                            "#FFCCCC", "Editor Line Error"                         }, // THEME_DEFAULT_LINE_ERROR
    {"ThemeDefaultLineHighlightEditorSelect",            "#00FFFF", "Editor Line Highlight Editor Select"       }, // THEME_DEFAULT_LINE_HIGHLIGHT_EDITOR_SELECT                lighter(180) alpha(100)
    {"ThemeDefaultLineHighlightViewerSelect",            "#00FF00", "Editor Line Highlight Viewer Select"       }, // THEME_DEFAULT_LINE_HIGHLIGHT_VIEWER_SELECT (180 alpha)    lighter(180) alpha(100)

    //KEY                                                COLOR      LABEL                                          // THEME_DARK
    {"ThemeDarkSceneBackgroundColor",                    "#31363B", "Scene Background"                          }, // THEME_DARK_SCENE_BACKGROUND_COLOR
    {"ThemeDarkGridPen",                                 "#FFFFFF", "Grid Pen"                                  }, // THEME_DARK_GRID_PEN
    {"ThemeDarkRulerPen",                                "#FFFFFF", "Ruler Pen (uses Grid Pen)"                 }, // THEME_DARK_RULER_PEN [NOT USED]
    {"ThemeDarkRulerTickPen",                            "#EFF0F1", "Ruler Tick Pen"                            }, // THEME_DARK_RULER_TICK_PEN
    {"ThemeDarkRulerTrackPen",                           "#00FF00", "Ruler Track Pen"                           }, // THEME_DARK_RULER_TRACK_PEN
    {"ThemeDarkGuidePen",                                "#AA0000", "Guide Pen"                                 }, // THEME_DARK_GUIDE_PEN
    {"ThemeDarkTransPageBorder",                         "#AEADAC", "Page Hidden Background Border"             }, // THEME_DARK_TRANS_PAGE_BORDER
    {"ThemeDarkEditMargin",                              "#ABB2B9", "Edit Margin"                               }, // THEME_DARK_EDIT_MARGIN

    {"ThemeDarkDecorateLDrawComments",                   "#17C723", "Decorate LDraw Comments"                   }, // THEME_DARK_DECORATE_LDRAW_COMMENTS                        br01
    {"ThemeDarkDecorateLDrawHeader",                     "#FB743E", "Decorate LDraw Header"                     }, // THEME_DARK_DECORATE_LDRAW_HEADER                          br02
    {"ThemeDarkDecorateLDrawBody",                       "#14AAFF", "Decorate LDraw Body"                       }, // THEME_DARK_DECORATE_LDRAW_BODY                            br03
    {"ThemeDarkDecorateLPub3DLocal",                     "#609CFF", "Decorate LPub3D Local"                     }, // THEME_DARK_DECORATE_LPUB3D_LOCAL                          br04
    {"ThemeDarkDecorateLPub3DGlobal",                    "#A0B2E2", "Decorate LPub3D Global"                    }, // THEME_DARK_DECORATE_LPUB3D_GLOBAL                         br05

    {"ThemeDarkDecorateLDrawLineType1",                  "#C2A4C0", "Decorate LDraw Line Type 1"                }, // THEME_DARK_DECORATE_LDRAW_LINE_TYPE_1                     br06
    {"ThemeDarkDecorateLDrawColourCode",                 "#F46105", "Decorate LDraw Colour Code"                }, // THEME_DARK_DECORATE_LDRAW_COLOUR_CODE                     br07
    {"ThemeDarkDecorateLDrawPartPosition",               "#46B5D1", "Decorate LDraw Part Position"              }, // THEME_DARK_DECORATE_LDRAW_PART_POSITION                   br08
    {"ThemeDarkDecorateLDrawPartTransform1",             "#DDFFEF", "Decorate LDraw Part Transform 1"           }, // THEME_DARK_DECORATE_LDRAW_PART_TRANSFORM_1                br09
    {"ThemeDarkDecorateLDrawPartTransform2",             "#A2FFD2", "Decorate LDraw Part Transform 2"           }, // THEME_DARK_DECORATE_LDRAW_PART_TRANSFORM_2                br10
    {"ThemeDarkDecorateLDrawPartTransform3",             "#5CFFB0", "Decorate LDraw Part Transform 3"           }, // THEME_DARK_DECORATE_LDRAW_PART_TRANSFORM_3                br11
    {"ThemeDarkDecorateLDrawPartFile",                   "#C3F6FE", "Decorate LDraw Part File"                  }, // THEME_DARK_DECORATE_LDRAW_PART_FILE                       br12
    {"ThemeDarkDecorateLDrawLineTypes2To5",              "#7D5E18", "Decorate LDraw Line Types 2 To 5"          }, // THEME_DARK_DECORATE_LDRAW_LINE_TYPES_2_TO_5               br13
    {"ThemeDarkDecorateLPub3DNumber",                    "#FC9B14", "Decorate LPub3D Number"                    }, // THEME_DARK_DECORATE_LPUB3D_NUMBER                         br14
    {"ThemeDarkDecorateLPub3DHexNumber",                 "#B87620", "Decorate LPub3D Hex Number"                }, // THEME_DARK_DECORATE_LPUB3D_HEX_NUMBER                     br15
    {"ThemeDarkDecorateLPub3DPageSize",                  "#C15317", "Decorate LPub3D Page Size"                 }, // THEME_DARK_DECORATE_LPUB3D_PAGE_SIZE                      br16

    {"ThemeDarkDecorateLeoCAD",                          "#0079CF", "Decorate LeoCAD"                           }, // THEME_DARK_DECORATE_LEOCAD                                br17
    {"ThemeDarkDecorateLSynth",                          "#FF9900", "Decorate LSynth"                           }, // THEME_DARK_DECORATE_LSYNTH                                br18
    {"ThemeDarkDecorateLDCad",                           "#FF9900", "Decorate LDCad"                            }, // THEME_DARK_DECORATE_LDCAD                                 br19
    {"ThemeDarkDecorateMLCad",                           "#0079CF", "Decorate MLCad"                            }, // THEME_DARK_DECORATE_MLCAD                                 br20
    {"ThemeDarkDecorateMLCadBody",                       "#9421A6", "Decorate MLCad Body"                       }, // THEME_DARK_DECORATE_MLCAD_BODY                            br21
    {"ThemeDarkDecorateLPub3DFalse",                     "#FF8080", "Decorate LPub3D False"                     }, // THEME_DARK_DECORATE_LPUB3D_FALSE                          br22
    {"ThemeDarkDecorateLPub3DTrue",                      "#ADFF2F", "Decorate LPub3D True"                      }, // THEME_DARK_DECORATE_LPUB3D_TRUE                           br23
    {"ThemeDarkDecorateLPub3D",                          "#F638DC", "Decorate LPub3D"                           }, // THEME_DARK_DECORATE_LPUB3D                                br24
    {"ThemeDarkDecorateLPub3DBody",                      "#FF3366", "Decorate LPub3D Body"                      }, // THEME_DARK_DECORATE_LPUB3D_BODY                           br25
    {"ThemeDarkDecorateLDrawHeaderValue",                "#AEAEAE", "Decorate LDraw Header Value"               }, // THEME_DARK_DECORATE_LDRAW_HEADER_VALUE                    br26
    {"ThemeDarkDecorateLPub3DQuotedText",                "#81D4FA", "Decorate LPub3D Quoted Text"               }, // THEME_DARK_DECORATE_LPUB3D_QUOTED_TEXT                    br27
    {"ThemeDarkDecorateLDrawLineType0FirstCharacter",    "#FFFFFF", "Decorate LDraw Line Type 0 First Character"}, // THEME_DARK_DECORATE_LDRAW_LINE_TYPE_0_FIRST_CHARACTER     br28
    {"ThemeDarkDecorateLDCadGroupDefine",                "#BD4900", "Decorate LDCad Group Define"               }, // THEME_DARK_DECORATE_LDCAD_GROUP_DEFINE                    br29

    {"ThemeDarkDecorateIniFileComment",                  "#17C723", "Decorate Ini File Comment"                 }, // THEME_DARK_DECORATE_INI_FILE_COMMENT                      br01
    {"ThemeDarkDecorateIniFileHeader",                   "#0079CF", "Decorate Ini File Header"                  }, // THEME_DARK_DECORATE_INI_FILE_HEADER                       br02
    {"ThemeDarkDecorateIniFileEqual",                    "#FF5227", "Decorate Ini File Equal"                   }, // THEME_DARK_DECORATE_INI_FILE_EQUAL                        br03
    {"ThemeDarkDecorateIniFileValue",                    "#BBBFFF", "Decorate Ini File Value"                   }, // THEME_DARK_DECORATE_INI_FILE_VALUE                        br04

    {"ThemeDarkDecorateParameterFilePartId",             "#C3F6FE", "Decorate Parameter File Part Id"           }, // THEME_DARK_DECORATE_PARAMETER_FILE_PART_ID                br05
    {"ThemeDarkDecorateParameterFilePartControl",        "#9421A6", "Decorate Parameter File Part Control"      }, // THEME_DARK_DECORATE_PARAMETER_FILE_PART_CONTROL           br06
    {"ThemeDarkDecorateParameterFilePartDescription",    "#AA0000", "Decorate Parameter File Part Description"  }, // THEME_DARK_DECORATE_PARAMETER_FILE_PART_DESCRIPTION       br07

    {"ThemeDarkLineSelect",                              "#E8E8E8", "Editor Line Select"                        }, // THEME_DARK_LINE_SELECT                                    alpha(30)
    {"ThemeDarkLineHighlight",                           "#5D5B59", "Editor Line Highlight"                     }, // THEME_DARK_LINE_HIGHLIGHT
    {"ThemeDarkLineError",                               "#FF0000", "Editor Line Error"                         }, // THEME_DARK_LINE_ERROR                                     lighter(180)
    {"ThemeDarkLineHighlightEditorSelect",               "#00FFFF", "Editor Line Highlight Editor Select"       }, // THEME_DARK_LINE_HIGHLIGHT_EDITOR_SELECT                   lighter(180) alpha(100)
    {"ThemeDarkLineHighlightViewerSelect",               "#00FF00", "Editor Line Highlight Viewer Select"       }, // THEME_DARK_LINE_HIGHLIGHT_VIEWER_SELECT (180 alpha)       lighter(180) alpha(100)

    {"ThemeDarkPaletteWindow",                           "#313437", "Palette Window"                            }, // THEME_DARK_PALETTE_WINDOW                                   49,  52,  55, 255
    {"ThemeDarkPaletteWindowText",                       "#F0F0F0", "Palette Window Text"                       }, // THEME_DARK_PALETTE_WINDOW_TEXT                             240, 240, 240, 255
    {"ThemeDarkPaletteBase",                             "#232629", "Palette Base"                              }, // THEME_DARK_PALETTE_BASE                                     35,  38,  41, 255
    {"ThemeDarkPaletteAltBase",                          "#2C2F32", "Palette Alt Base"                          }, // THEME_DARK_PALETTE_ALT_BASE                                 44,  47,  50, 255
    {"ThemeDarkPaletteTipBase",                          "#E0E0F4", "Palette Tip Base"                          }, // THEME_DARK_PALETTE_TIP_BASE                                224, 224, 244, 255
    {"ThemeDarkPaletteTipText",                          "#3A3A3A", "Palette Tip Text"                          }, // THEME_DARK_PALETTE_TIP_TEXT                                 58,  58,  58, 255

    {"ThemeDarkPalettePholderText",                      "#646464", "Palette Pholder Text"                      }, // THEME_DARK_PALETTE_PHOLDER_TEXT                            100, 100, 100, 255

    {"ThemeDarkPaletteText",                             "#E0E0E0", "Palette Text"                              }, // THEME_DARK_PALETTE_TEXT                                    224, 224, 224, 255
    {"ThemeDarkPaletteButton",                           "#2D3033", "Palette Button"                            }, // THEME_DARK_PALETTE_BUTTON                                   45,  48,  51, 255
    {"ThemeDarkPaletteButtonText",                       "#E0E0F4", "Palette Button Text"                       }, // THEME_DARK_PALETTE_BUTTON_TEXT                             224, 224, 244, 255
    {"ThemeDarkPaletteLight",                            "#414141", "Palette Light"                             }, // THEME_DARK_PALETTE_LIGHT                                    65,  65,  65, 255
    {"ThemeDarkPaletteMidlight",                         "#3E3E3E", "Palette Midlight"                          }, // THEME_DARK_PALETTE_MIDLIGHT                                 62,  62,  62, 255
    {"ThemeDarkPaletteDark",                             "#232323", "Palette Dark"                              }, // THEME_DARK_PALETTE_DARK                                     35,  35,  35, 255
    {"ThemeDarkPaletteMid",                              "#323232", "Palette Mid"                               }, // THEME_DARK_PALETTE_MID                                      50,  50,  50, 255
    {"ThemeDarkPaletteShadow",                           "#141414", "Palette Shadow"                            }, // THEME_DARK_PALETTE_SHADOW                                   20,  20,  20, 255
    {"ThemeDarkPaletteHilight",                          "#2980B9", "Palette Hilight"                           }, // THEME_DARK_PALETTE_HILIGHT                                  41, 128, 185, 255
    {"ThemeDarkPaletteHilightText",                      "#E8E8E8", "Palette Hilight Text"                      }, // THEME_DARK_PALETTE_HILIGHT_TEXT                            232, 232, 232, 255
    {"ThemeDarkPaletteLink",                             "#2980B9", "Palette Link"                              }, // THEME_DARK_PALETTE_LINK                                     41, 128, 185, 255
    {"ThemeDarkPaletteDisabledText",                     "#808080", "Palette Disabled Text"                     }, // THEME_DARK_PALETTE_DISABLED_TEXT                           128, 128, 128, 255

    {"ThemeDarkViewerBackgroundColor",                   "#313437", "Viewer Background"                         }, // THEME_DARK_VIEWER_BACKGROUND_COLOR                          49,  52,  55, 255 LC_PROFILE_BACKGROUND_COLOR
    {"ThemeDarkViewerGradientColorTop",                  "#0000BF", "Viewer Gradient Top"                       }, // THEME_DARK_VIEWER_GRADIENT_COLOR_TOP                         0,   0, 191, 255 LC_PROFILE_GRADIENT_COLOR_TOP
    {"ThemeDarkViewerGradientColorBottom",               "#FFFFFF", "Viewer Gradient Bottom"                    }, // THEME_DARK_VIEWER_GRADIENT_COLOR_BOTTOM                    255, 255, 255, 255 LC_PROFILE_GRADIENT_COLOR_BOTTOM
    {"ThemeDarkAxesColor",                               "#A0A0A0", "Viewer Axes"                               }, // THEME_DARK_AXES_COLOR                                      160, 160, 160, 255 LC_PROFILE_AXES_COLOR
    {"ThemeDarkOverlayColor",                            "#E0E0E0", "Viewer Overlay"                            }, // THEME_DARK_OVERLAY_COLOR                                   224, 224, 224, 255 LC_PROFILE_OVERLAY_COLOR
    {"ThemeDarkMarqueeBorderColor",                      "#4040FF", "Viewer Marquee Border"                     }, // THEME_DARK_MARQUEE_BORDER_COLOR                             64,  64, 255, 255 LC_PROFILE_MARQUEE_BORDER_COLOR
    {"ThemeDarkMarqueeFillColor",                        "#4040FF", "Viewer Marquee Fill"                       }, // THEME_DARK_MARQUEE_FILL_COLOR                               64,  64, 255,  64 LC_PROFILE_MARQUEE_FILL_COLOR
    {"ThemeDarkInactiveViewColor",                       "#454545", "Viewer Inactive View"                      }, // THEME_DARK_INACTIVE_VIEW_COLOR                              69,  69,  69, 255 LC_PROFILE_INACTIVE_VIEW_COLOR
    {"ThemeDarkActiveViewColor",                         "#2980B9", "Viewer Active View"                        }, // THEME_DARK_ACTIVE_VIEW_COLOR                                41, 128, 185, 255 LC_PROFILE_ACTIVE_VIEW_COLOR
    {"ThemeDarkGridStudColor",                           "#181818", "Viewer Grid Stud (192 alpha)"              }, // THEME_DARK_GRID_STUD_COLOR                                  24,  24,  24, 192 LC_PROFILE_GRID_STUD_COLOR alpha(192)
    {"ThemeDarkGridLineColor",                           "#181818", "Viewer Grid Line"                          }, // THEME_DARK_GRID_LINE_COLOR                                  24,  24,  24, 255 LC_PROFILE_GRID_LINE_COLOR
    {"ThemeDarkTextColor",                               "#A0A0A0", "Viewer Text"                               }, // THEME_DARK_TEXT_COLOR                                      160, 160, 160, 255 LC_PROFILE_TEXT_COLOR
    {"ThemeDarkViewSphereColor",                         "#232629", "Viewer View Sphere"                        }, // THEME_DARK_VIEW_SPHERE_COLOR                                35,  38,  41, 255 LC_PROFILE_VIEW_SPHERE_COLOR
    {"ThemeDarkViewSphereTextColor",                     "#E0E0E0", "Viewer View Sphere Text"                   }, // THEME_DARK_VIEW_SPHERE_TEXT_COLOR                          224, 224, 224, 255 LC_PROFILE_VIEW_SPHERE_TEXT_COLOR
    {"ThemeDarkViewSphereHlightColor",                   "#2980B9", "Viewer View Sphere Hlight"                 }, // THEME_DARK_VIEW_SPHERE_HLIGHT_COLOR                         41, 128, 185, 255 LC_PROFILE_VIEW_SPHERE_HIGHLIGHT_COLOR

    {"ThemeDarkObjectSelectedColor",                     "#E54C66", "Object Selected Color"                     }, // THEME_DARK_OBJECT_SELECTED_COLOR                           229,  76, 102, 255 LC_PROFILE_OBJECT_SELECTED_COLOR
    {"ThemeDarkObjectFocusedColor",                      "#664CE5", "Object Focused Color "                     }, // THEME_DARK_OBJECT_FOCUSED_COLOR                            102,  76, 229, 255 LC_PROFILE_OBJECT_FOCUSED_COLOR
    {"ThemeDarkCameraColor",                             "#80CC80", "Camera Color"                              }, // THEME_DARK_CAMERA_COLOR                                    128, 204, 128, 255 LC_PROFILE_CAMERA_COLOR
    {"ThemeDarkLightColor",                              "#80CC80", "Light Color"                               }, // THEME_DARK_LIGHT_COLOR                                     128, 204, 128, 255 LC_PROFILE_LIGHT_COLOR
    {"ThemeDarkControlPointColor",                       "#80CC80", "Control Point Color"                       }, // THEME_DARK_CONTROL_POINT_COLOR                             128, 204, 128, 128 LC_PROFILE_CONTROL_POINT_COLOR
    {"ThemeDarkControlPointFocusedColor",                "#664CE5", "Control Point Focused Color"               }, // THEME_DARK_CONTROL_POINT_FOCUSED_COLOR                     102,  76, 229, 128 LC_PROFILE_CONTROL_POINT_FOCUSED_COLOR
    {"ThemeDarkBmObjectSelectedColor",                   "#79D879", "Build Modification Object Selected Color"  }, // THEME_DARK_BM_OBJECT_SELECTED_COLOR                        121, 216, 121, 255 LC_PROFILE_BM_OBJECT_SELECTED_COLOR

    {"ThemeDarkGraphicsviewBorderColor",                 "#999999", "Graphicsview Border"                       }  // THEME_DARK_GRAPHICSVIEW_BORDER_COLOR                       153, 153, 153, 255
                                                                                                                   // THEME_NUM_COLORS
};

QString Preferences::themeColors[THEME_NUM_COLORS];
QMap<QString, QKeySequence> Preferences::keyboardShortcuts;

QString Preferences::lpub3dAppName              = EMPTY_STRING_DEFAULT;
QString Preferences::ldrawLibPath               = EMPTY_STRING_DEFAULT;
QString Preferences::altLDConfigPath            = EMPTY_STRING_DEFAULT;
QString Preferences::lpub3dLibFile              = EMPTY_STRING_DEFAULT;
QString Preferences::lgeoPath;
QString Preferences::lpub3dPath                 = DOT_PATH_DEFAULT;
QString Preferences::lpub3dCachePath            = DOT_PATH_DEFAULT;
QString Preferences::lpub3dExtrasResourcePath   = DOT_PATH_DEFAULT;
QString Preferences::lpub3dDocsResourcePath     = DOT_PATH_DEFAULT;
QString Preferences::lpub3d3rdPartyConfigDir    = DOT_PATH_DEFAULT;
QString Preferences::lpub3d3rdPartyAppDir       = DOT_PATH_DEFAULT;
QString Preferences::lpub3d3rdPartyAppExeDir    = DOT_PATH_DEFAULT;
QString Preferences::lpub3dLDVConfigDir         = DOT_PATH_DEFAULT;
QString Preferences::lpubDataPath               = DOT_PATH_DEFAULT;
QString Preferences::lpubConfigPath             = DOT_PATH_DEFAULT;
QString Preferences::lpubExtrasPath             = DOT_PATH_DEFAULT;
QString Preferences::ldgliteExe;
QString Preferences::ldviewExe;
QString Preferences::povrayConf;
QString Preferences::povrayIni;
QString Preferences::ldgliteIni;
QString Preferences::ldviewIni;
QString Preferences::ldviewPOVIni;
QString Preferences::nativeExportIni;
QString Preferences::povrayIniPath;
QString Preferences::povrayIncPath;
QString Preferences::povrayScenePath;
QString Preferences::povrayExe;
QString Preferences::optPrefix                  = VER_OPT_PREFIX_STR;
QString Preferences::highlightStepColour        = HIGHLIGHT_COLOUR_DEFAULT;
QString Preferences::ldrawiniFile;
QString Preferences::moduleVersion              = qApp->applicationVersion();
QString Preferences::availableVersions;
QString Preferences::ldgliteSearchDirs;
QString Preferences::loggingLevel               = LOGGING_LEVEL_DEFAULT;
QString Preferences::logPath;
QString Preferences::dataLocation;
QString Preferences::systemEditor;
QString Preferences::currentLibrarySave         = EMPTY_STRING_DEFAULT;

QString Preferences::blenderVersion;
QString Preferences::blenderRenderConfigFile;
QString Preferences::blenderDocumentConfigFile;
QString Preferences::blenderExe;

QStringList Preferences::ldSearchDirs;
QStringList Preferences::ldgliteParms;
QStringList Preferences::messagesNotShown;

//Dynamic page attributes
QString Preferences::defaultAuthor;
QString Preferences::publishDescription;
QString Preferences::defaultURL;
QString Preferences::defaultEmail;
QString Preferences::documentLogoFile;
//Static page attributes
QString Preferences::disclaimer                 = QString(QObject::trUtf8("LEGO® is a registered trademark of the LEGO Group, \n"
                                                                          "which does not sponsor, endorse, or authorize these \n"
                                                                          "instructions or the model they depict."));
QString Preferences::copyright                  = QString(QObject::trUtf8("Copyright © %1").arg(QDate::currentDate().toString("yyyy")));
QString Preferences::plugImage                  = QString(":/resources/LPub64.png");
QString Preferences::plug                       = QString(QObject::trUtf8("Instructions configured and generated using %1 %2 \n Download %1 at %3")
                                                          .arg(QString::fromLatin1(VER_PRODUCTNAME_STR),
                                                               QString::fromLatin1(VER_FILEVERSION_STR).replace("\"",""),
                                                               QString::fromLatin1(VER_HOMEPAGE_GITHUB_STR)));
QString Preferences::displayTheme               = THEME_DEFAULT;

QString Preferences::titleAnnotationsFile;
QString Preferences::freeformAnnotationsFile;
QString Preferences::annotationStyleFile;
QString Preferences::pliSubstitutePartsFile;
QString Preferences::excludedPartsFile;
QString Preferences::stickerPartsFile;
QString Preferences::pliControlFile;
QString Preferences::ldrawColourPartsFile;

// Dynamic file name keyed on LDraw library
QString Preferences::validTitleAnnotations      = VER_LEGO_TITLE_ANNOTATIONS_FILE;
QString Preferences::validFreeFormAnnotations   = VER_LEGO_FREEFROM_ANNOTATIONS_FILE;
QString Preferences::validAnnotationStyles      = VER_LEGO_ANNOTATION_STYLE_FILE;
QString Preferences::validPliSubstituteParts    = VER_LEGO_PLI_SUBSTITUTE_FILE;
QString Preferences::validExcludedPliParts      = VER_LEGO_PLI_EXCLUDED_FILE;
QString Preferences::validStickerPliParts       = VER_LEGO_PLI_STICKER_FILE;
QString Preferences::validPliControl            = VER_LEGO_PLI_CONTROL_FILE;

QString Preferences::validLDrawLibrary          = LEGO_LIBRARY;            // the currently loaded library
QString Preferences::validLDrawLibraryChange    = LEGO_LIBRARY;            // the result of a library test - initialized to the currently loaded library
QString Preferences::validFadeStepsColour       = LEGO_FADE_COLOUR_DEFAULT;
QString Preferences::validLDrawDir              = VER_LEGO_LDRAWDIR_STR;
QString Preferences::validLDrawPart             = VER_LEGO_LDRAWPART_STR;
QString Preferences::validLDrawPartsArchive     = VER_LDRAW_OFFICIAL_ARCHIVE;
QString Preferences::validLDrawCustomArchive    = VER_LPUB3D_UNOFFICIAL_ARCHIVE;
QString Preferences::validLDrawColorParts       = VER_LPUB3D_LEGO_COLOR_PARTS;
QString Preferences::validLDrawPartsLibrary     = LEGO_LIBRARY "® Parts";

QString Preferences::fadeStepsColourKey         = LEGO_FADE_COLOUR_KEY;
QString Preferences::ldrawSearchDirsKey         = LEGO_SEARCH_DIR_KEY;
QString Preferences::ldrawLibPathKey            = LEGO_LDRAW_LIB_PATH_KEY;

QString Preferences::blCodesFile                = VER_LPUB3D_BLCODES_FILE;
QString Preferences::legoElementsFile           = VER_LPUB3D_LEGOELEMENTS_FILE;
QString Preferences::blColorsFile               = VER_LPUB3D_BLCOLORS_FILE;
QString Preferences::ld2blColorsXRefFile        = VER_LPUB3D_LD2BLCOLORSXREF_FILE;
QString Preferences::ld2blCodesXRefFile         = VER_LPUB3D_LD2BLCODESXREF_FILE;
QString Preferences::ld2rbColorsXRefFile        = VER_LPUB3D_LD2RBCOLORSXREF_FILE;
QString Preferences::ld2rbCodesXRefFile         = VER_LPUB3D_LD2RBCODESXREF_FILE;

QString Preferences::sceneBackgroundColor       = defaultThemeColors[THEME_DEFAULT_SCENE_BACKGROUND_COLOR].color;
QString Preferences::sceneGridColor             = defaultThemeColors[THEME_DEFAULT_GRID_PEN].color;
QString Preferences::sceneRulerTickColor        = defaultThemeColors[THEME_DEFAULT_RULER_TICK_PEN].color;
QString Preferences::sceneRulerTrackingColor    = defaultThemeColors[THEME_DEFAULT_RULER_TRACK_PEN].color;
QString Preferences::sceneGuideColor            = defaultThemeColors[THEME_DEFAULT_GUIDE_PEN].color;

#ifdef Q_OS_MAC
QString Preferences::editorFont                 = DEFAULT_EDITOR_FONT_MACOS;
#elif defined Q_OS_LINUX
QString Preferences::editorFont                 = DEFAULT_EDITOR_FONT_LINUX;
#elif defined Q_OS_WIN
QString Preferences::editorFont                 = DEFAULT_EDITOR_FONT_WINDOWS;
#endif

bool    Preferences::usingDefaultLibrary        = true;
bool    Preferences::perspectiveProjection      = true;
bool    Preferences::saveOnRedraw               = true;
bool    Preferences::saveOnUpdate               = true;
bool    Preferences::useNativePovGenerator      = true;

bool    Preferences::applyCALocally             = true;
bool    Preferences::modeGUI                    = true;
bool    Preferences::showAllNotifications       = true;
bool    Preferences::showUpdateNotifications    = true;
bool    Preferences::enableDownloader           = true;

bool    Preferences::lineParseErrors            = true;
bool    Preferences::showInsertErrors           = true;
bool    Preferences::showBuildModErrors         = true;
bool    Preferences::showIncludeFileErrors      = true;
bool    Preferences::showAnnotationErrors       = true;

bool    Preferences::showSaveOnRedraw           = true;
bool    Preferences::showSaveOnUpdate           = true;
bool    Preferences::blenderIs28OrLater         = true;
bool    Preferences::finalModelEnabled          = true;
bool    Preferences::editorHighlightLines       = true;
bool    Preferences::editorLoadSelectionStep    = true;
bool    Preferences::editorPreviewOnDoubleClick = true;
bool    Preferences::inlineNativeContent        = true;

bool    Preferences::ldgliteInstalled           = false;
bool    Preferences::ldviewInstalled            = false;
bool    Preferences::povRayInstalled            = false;
bool    Preferences::blenderInstalled           = false;

bool    Preferences::portableDistribution       = false;
bool    Preferences::lgeoStlLib                 = false;
bool    Preferences::lpub3dLoaded               = false;
bool    Preferences::enableDocumentLogo         = false;
bool    Preferences::enableLDViewSingleCall     = false;
bool    Preferences::enableLDViewSnaphsotList   = false;
bool    Preferences::displayAllAttributes       = false;
bool    Preferences::generateCoverPages         = false;
bool    Preferences::printDocumentTOC           = false;
bool    Preferences::doNotShowPageProcessDlg    = false;
bool    Preferences::autoUpdateChangeLog        = false;

bool    Preferences::includeLogLevel            = false;
bool    Preferences::includeTimestamp           = false;
bool    Preferences::includeLineNumber          = false;
bool    Preferences::includeFileName            = false;
bool    Preferences::includeFunction            = false;
bool    Preferences::addLSynthSearchDir         = false;
bool    Preferences::archiveLSynthParts         = false;
bool    Preferences::skipPartsArchive           = false;
bool    Preferences::loadLastOpenedFile         = false;
bool    Preferences::extendedSubfileSearch      = false;
bool    Preferences::cycleEachPage              = false;

bool    Preferences::usingNPP                   = false;
bool    Preferences::pdfPageImage               = false;
bool    Preferences::ignoreMixedPageSizesMsg    = false;

bool    Preferences::debugLevel                 = false;
bool    Preferences::traceLevel                 = false;
bool    Preferences::noticeLevel                = false;
bool    Preferences::infoLevel                  = false;
bool    Preferences::statusLevel                = false;
bool    Preferences::warningLevel               = false;
bool    Preferences::errorLevel                 = false;
bool    Preferences::fatalLevel                 = false;

bool    Preferences::showSubmodels              = false;
bool    Preferences::showTopModel               = false;
bool    Preferences::showSubmodelInCallout      = false;
bool    Preferences::showInstanceCount          = false;

bool    Preferences::includeAllLogAttributes    = false;
bool    Preferences::allLogLevels               = false;

bool    Preferences::logLevel                   = false;   // logging level (combo box)
bool    Preferences::logging                    = false;   // logging on/off offLevel (grp box)
bool    Preferences::logLevels                  = false;   // individual logging levels (grp box)

bool    Preferences::preferCentimeters          = false;   // default is false, to use DPI
bool    Preferences::showDownloadRedirects      = false;
bool    Preferences::ldrawiniFound              = false;
bool    Preferences::povrayDisplay              = false;
bool    Preferences::povrayAutoCrop             = false;
bool    Preferences::isAppImagePayload          = false;

bool    Preferences::buildModEnabled            = false;
bool    Preferences::enableFadeSteps            = false;
bool    Preferences::fadeStepsUseColour         = false;
bool    Preferences::enableHighlightStep        = false;
bool    Preferences::enableImageMatting         = false;

bool    Preferences::sceneRuler                 = false;
bool    Preferences::sceneGuides                = false;
bool    Preferences::snapToGrid                 = false;
bool    Preferences::hidePageBackground         = false;
bool    Preferences::showGuidesCoordinates      = false;
bool    Preferences::showTrackingCoordinates    = false;
bool    Preferences::suppressStdOutToLog        = false;
bool    Preferences::archivePartsOnLaunch       = false;
bool    Preferences::highlightFirstStep         = false;

bool    Preferences::customSceneBackgroundColor = false;
bool    Preferences::customSceneGridColor       = false;
bool    Preferences::customSceneRulerTickColor  = false;
bool    Preferences::customSceneRulerTrackingColor = false;
bool    Preferences::customSceneGuideColor      = false;

bool    Preferences::debugLogging               = false;

bool    Preferences::defaultBlendFile           = false;
bool    Preferences::useSystemEditor            = false;
bool    Preferences::removeBuildModFormat       = false;
bool    Preferences::removeChildSubmodelFormat  = false;
bool    Preferences::editorBufferedPaging       = false;

#ifdef Q_OS_MAC
bool    Preferences::missingRendererLibs        = false;
#endif

bool    Preferences::loadTheme                  = false;
bool    Preferences::setSceneTheme              = false;
bool    Preferences::reloadPage                 = false;
bool    Preferences::reloadFile                 = false;
bool    Preferences::resetCustomCache           = false;
bool    Preferences::restartApplication         = false;
bool    Preferences::libraryChangeRestart       = false;

int     Preferences::preferredRenderer          = 0; // RENDERER_NATIVE;
int     Preferences::ldrawFilesLoadMsgs         = 0; // NEVER_SHOW;
int     Preferences::sceneRulerTracking         = 2; // TRACKING_NONE;
int     Preferences::sceneGuidesPosition        = 0; // GUIDES_TOP_LEFT;
int     Preferences::sceneGuidesLine            = SCENE_GUIDES_LINE_DEFAULT;
int     Preferences::povrayRenderQuality        = POVRAY_RENDER_QUALITY_DEFAULT;
int     Preferences::fadeStepsOpacity           = FADE_OPACITY_DEFAULT;              //Default = 50 percent (half opacity)
int     Preferences::highlightStepLineWidth     = HIGHLIGHT_LINE_WIDTH_DEFAULT;      //Default = 1

int     Preferences::checkUpdateFrequency       = UPDATE_CHECK_FREQUENCY_DEFAULT;    //0=Never,1=Daily,2=Weekly,3=Monthly

int     Preferences::gridSizeIndex              = GRID_SIZE_INDEX_DEFAULT;
int     Preferences::pageHeight                 = PAGE_HEIGHT_DEFAULT;
int     Preferences::pageWidth                  = PAGE_WIDTH_DEFAULT;
int     Preferences::rendererTimeout            = RENDERER_TIMEOUT_DEFAULT;          // measured in seconds
int     Preferences::pageDisplayPause           = PAGE_DISPLAY_PAUSE_DEFAULT;        // measured in seconds
int     Preferences::nativeImageCameraFoVAdjust = NATIVE_IMAGE_CAMERA_FOV_ADJUST;

int     Preferences::maxOpenWithPrograms        = MAX_OPEN_WITH_PROGRAMS_DEFAULT;

int     Preferences::editorLinesPerPage         = EDITOR_MIN_LINES_DEFAULT;
int     Preferences::editorDecoration           = EDITOR_DECORATION_DEFAULT;

int     Preferences::assemblyCameraLatitude     = DEFAULT_ASSEM_CAMERA_LATITUDE;
int     Preferences::assemblyCameraLongitude    = DEFAULT_ASSEM_CAMERA_LONGITUDE;
int     Preferences::partCameraLatitude         = DEFAULT_PART_CAMERA_LATITUDE;
int     Preferences::partCameraLongitude        = DEFAULT_PART_CAMERA_LONGITUDE;
int     Preferences::submodelCameraLatitude     = DEFAULT_SUBMODEL_CAMERA_LATITUDE;
int     Preferences::submodelCameraLongitude    = DEFAULT_SUBMODEL_CAMERA_LONGITUDE;

#ifdef Q_OS_MAC
int Preferences::editorFontSize                 = DEFAULT_EDITOR_FONT_SIZE_MACOS;
#elif defined Q_OS_LINUX
int Preferences::editorFontSize                 = DEFAULT_EDITOR_FONT_SIZE_LINUX;
#elif defined Q_OS_WIN
int Preferences::editorFontSize                 = DEFAULT_EDITOR_FONT_SIZE_WINDOWS;
#endif

bool    Preferences::initEnableFadeSteps        = false;
bool    Preferences::initFadeStepsUseColour     = false;
int     Preferences::initFadeStepsOpacity       = FADE_OPACITY_DEFAULT;
QString Preferences::initValidFadeStepsColour   = LEGO_FADE_COLOUR_DEFAULT;

bool    Preferences::initEnableHighlightStep    = false;
int     Preferences::initHighlightStepLineWidth = HIGHLIGHT_LINE_WIDTH_DEFAULT;
QString Preferences::initHighlightStepColour    = HIGHLIGHT_COLOUR_DEFAULT;

int     Preferences::initPreferredRenderer      = 0; //RENDERER_NATIVE;
int     Preferences::fileLoadWaitTime           = FILE_LOAD_WAIT_TIME;

// Native POV file generation settings
QString Preferences::xmlMapPath                 = EMPTY_STRING_DEFAULT;

/*
 * [DATA PATHS]
 * dataLocation            - the data location at install
 * lpubDataPath            - the application user data location after install
 * lpub3d3rdPartyConfigDir - 3rdParty folder at application user data location
 * lpub3d3rdPartyAppDir    - 3rdParty folder at install location
 * lpubExtrasPath          - not used
 */

Preferences::Preferences()
{
}

void Preferences::setStdOutToLogPreference(bool option)
{
    suppressStdOutToLog = option;
}

bool Preferences::checkLDrawLibrary(const QString &libPath) {

    QStringList validLDrawLibs = QStringList() << LEGO_LIBRARY << TENTE_LIBRARY << VEXIQ_LIBRARY;
    QStringList validLDrawParts = QStringList() << VER_LEGO_LDRAWPART_STR << VER_TENTE_LDRAWPART_STR << VER_VEXIQ_LDRAWPART_STR;

    for ( int i = 0; i < NumLibs; i++ )
    {
       if (QFileInfo(QString("%1%2").arg(libPath).arg(validLDrawParts[i])).exists()) {
           validLDrawLibraryChange = validLDrawLibs[i];
           return true;
       }
    }
    return false;
}

void Preferences::setLPub3DAltLibPreferences(const QString &library)
{
    QSettings Settings;
    if (! library.isEmpty()) {
        validLDrawLibrary = library;
        const QString libraryCompare = Settings.value(QString("%1/%2").arg(SETTINGS,"LDrawLibrary")).toString();
        if (libraryCompare != library) {
            currentLibrarySave = libraryCompare;
        }
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDrawLibrary"),validLDrawLibrary);
    } else {
        if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"LDrawLibrary"))) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDrawLibrary"),validLDrawLibrary);
        } else {
            validLDrawLibrary = Settings.value(QString("%1/%2").arg(SETTINGS,"LDrawLibrary")).toString();
        }
    }

    usingDefaultLibrary          = validLDrawLibrary ==  LEGO_LIBRARY;

    if (validLDrawLibrary ==  LEGO_LIBRARY) {
        validFadeStepsColour     = LEGO_FADE_COLOUR_DEFAULT;
        validLDrawPartsLibrary   = LEGO_LIBRARY "® Parts";
        validLDrawLibraryChange  = LEGO_LIBRARY;
        validLDrawDir            = VER_LEGO_LDRAWDIR_STR;
        validLDrawPart           = VER_LEGO_LDRAWPART_STR;
        validLDrawPartsArchive   = VER_LDRAW_OFFICIAL_ARCHIVE;
        validLDrawColorParts     = VER_LPUB3D_LEGO_COLOR_PARTS;
        validLDrawCustomArchive  = VER_LPUB3D_UNOFFICIAL_ARCHIVE;
        validPliControl          = VER_LEGO_PLI_CONTROL_FILE;
        validTitleAnnotations    = VER_LEGO_TITLE_ANNOTATIONS_FILE;
        validFreeFormAnnotations = VER_LEGO_FREEFROM_ANNOTATIONS_FILE;
        validAnnotationStyles    = VER_LEGO_ANNOTATION_STYLE_FILE;
        validPliSubstituteParts  = VER_LEGO_PLI_SUBSTITUTE_FILE;
        validExcludedPliParts    = VER_LEGO_PLI_EXCLUDED_FILE;
        validStickerPliParts     = VER_LEGO_PLI_STICKER_FILE;

        fadeStepsColourKey       = LEGO_FADE_COLOUR_KEY;
        ldrawSearchDirsKey       = LEGO_SEARCH_DIR_KEY;
        ldrawLibPathKey          = LEGO_LDRAW_LIB_PATH_KEY;
    }
    else
    if (validLDrawLibrary == TENTE_LIBRARY) {
        validFadeStepsColour     = TENTE_FADE_COLOUR_DEFAULT;
        validLDrawPartsLibrary   = TENTE_LIBRARY "® Construction Parts";
        validLDrawLibraryChange  = TENTE_LIBRARY;
        validLDrawDir            = VER_TENTE_LDRAWDIR_STR;
        validLDrawPart           = VER_TENTE_LDRAWPART_STR;
        validLDrawPartsArchive   = VER_LPUB3D_TENTE_ARCHIVE;
        validLDrawColorParts     = VER_LPUB3D_TENTE_COLOR_PARTS;
        validLDrawCustomArchive  = VER_LPUB3D_TENTE_CUSTOM_ARCHIVE;
        validPliControl          = VER_TENTE_PLI_CONTROL_FILE;
        validTitleAnnotations    = VER_TENTE_TITLE_ANNOTATIONS_FILE;
        validFreeFormAnnotations = VER_TENTE_FREEFROM_ANNOTATIONS_FILE;
        validAnnotationStyles    = VER_TENTE_ANNOTATION_STYLE_FILE;
        validPliSubstituteParts  = VER_TENTE_PLI_SUBSTITUTE_FILE;
        validExcludedPliParts    = VER_TENTE_PLI_EXCLUDED_FILE;
        validStickerPliParts     = VER_TENTE_PLI_STICKER_FILE;

        fadeStepsColourKey       = TENTE_FADE_COLOUR_KEY;
        ldrawSearchDirsKey       = TENTE_SEARCH_DIR_KEY;
        ldrawLibPathKey          = TENTE_LDRAW_LIB_PATH_KEY;
    }
    else
    if (validLDrawLibrary == VEXIQ_LIBRARY) {
        validFadeStepsColour     = VEXIQ_FADE_COLOUR_DEFAULT;
        validLDrawPartsLibrary   = VEXIQ_LIBRARY "® Parts";
        validLDrawLibraryChange  = VEXIQ_LIBRARY;
        validLDrawDir            = VER_VEXIQ_LDRAWDIR_STR;
        validLDrawPart           = VER_VEXIQ_LDRAWPART_STR;
        validLDrawPartsArchive   = VER_LPUB3D_VEXIQ_ARCHIVE;
        validLDrawColorParts     = VER_LPUB3D_VEXIQ_COLOR_PARTS;
        validLDrawCustomArchive  = VER_LPUB3D_VEXIQ_CUSTOM_ARCHIVE;
        validPliControl          = VER_VEXIQ_PLI_CONTROL_FILE;
        validTitleAnnotations    = VER_VEXIQ_TITLE_ANNOTATIONS_FILE;
        validFreeFormAnnotations = VER_VEXIQ_FREEFROM_ANNOTATIONS_FILE;
        validAnnotationStyles    = VER_VEXIQ_ANNOTATION_STYLE_FILE;
        validPliSubstituteParts  = VER_VEXIQ_PLI_SUBSTITUTE_FILE;
        validExcludedPliParts    = VER_VEXIQ_PLI_EXCLUDED_FILE;
        validStickerPliParts     = VER_VEXIQ_PLI_STICKER_FILE;

        fadeStepsColourKey       = VEXIQ_FADE_COLOUR_KEY;
        ldrawSearchDirsKey       = VEXIQ_SEARCH_DIR_KEY;
        ldrawLibPathKey          = VEXIQ_LDRAW_LIB_PATH_KEY;
    }
}

void Preferences::setDistribution(){
#ifdef Q_OS_WIN
    if ((portableDistribution = QDir(QCoreApplication::applicationDirPath()+"/extras").exists())){
        QDir configDir(QCoreApplication::applicationDirPath()+"/config");
        if(!QDir(configDir).exists())
            configDir.mkpath(".");
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, configDir.absolutePath());
    }
#endif
}

#ifdef Q_OS_MAC
bool Preferences::validLib(const QString &libName, const QString &libVersion) {

    int waitTime = 60000 ; // 60 secs
    QString scriptFile, scriptCommand;
    QTemporaryDir tempDir;
    if (tempDir.isValid()) {
        scriptFile =  QString("%1/ver.sh").arg(tempDir.path());
        //logDebug() << QString("Script file: [%1]").arg(scriptFile);

        if (libName != "xquartz")
            scriptCommand = QString("echo $(brew info " + libName + ") | sed \"s/^.*stable \\([^(]*\\).*/\\1/\"");
        else
            scriptCommand = QString("echo $(xdpyinfo | grep 'version number') | sed \"s/^.*\\:[ \\t]*\\(.*\\)$/\\1/\" && osascript -e 'quit app \"XQuartz\"'");

        QFile file(scriptFile);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << "#!/bin/bash" << lpub_endl;
            stream << scriptCommand << lpub_endl;
            file.close();
        } else {
           logError() << QString("Cannot write library check script file [%1] %2.")
                                 .arg(file.fileName())
                                 .arg(file.errorString());
           return false;
        }
    } else {
       logError() << QString("Cannot create library check temp path.");
       return false;
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QStringList envList = env.toStringList();
    envList.replaceInStrings(QRegularExpression("^(?i)PATH=(.*)"), "PATH=/usr/local/Homebrew/bin:/opt/local/bin:/usr/local/bin/:/opt/x11/bin:$HOME/bin:\\1");

    //logDebug() << "SystemEnvironment:  " << envList.join(" ");

    QProcess pr;
    pr.setEnvironment(envList);
    pr.start("/bin/sh",QStringList() << scriptFile);

    if (! pr.waitForStarted()) {
        logError() << QString("Cannot start library check process.");
        return false;
    }

    if (! pr.waitForFinished(waitTime)) {
        if (pr.exitCode() != 0) {
            QByteArray status = pr.readAll();
            QString str;
            str.append(status);
            logError() << QString("Library check process failed with code %1 %2")
                          .arg(pr.exitCode())
                          .arg(str);
            return false;
        }
    }

    QString p_stderr = pr.readAllStandardError();
    if (!p_stderr.isEmpty()) {
        /*
         * This 'whitelist' log spam is some sort of XCode 9 bug (r.33758979)
         * that seems unique to High Sierra.
         * Until I figure out a better way to override it, I'll just put
         * in place this hack
         */
        if (libName == "xquartz" && !p_stderr.contains("MessageTracer: Falling back to default whitelist")) {
            logError() << "Library check for xquartx returned error: " << p_stderr;
            return false;
        }
    }

    QString val1 = libVersion;
    QString val2 = pr.readAllStandardOutput().trimmed();

    // Compare v1 with v2 and return an integer:
    // Return -1 when v1 < v2
    // Return  0 when v1 = v2
    // Return  1 when v1 > v2

    auto compareLibVersion = [&val1, &val2] ()
    {
        int result = 0;
        int v1 = 0,v2 = 0;
        QString _val1 = val1;
        QString _val2 = val2;
        bool good = false, ok = false;
        if (_val1 == LIBJPEG_MACOS_VERSION) {
            _val1.chop(val1.size() - 1);
            _val2.chop(val2.size() - 1);
            v1 = _val1.toInt(&good);
            v2 = _val2.toInt(&ok);
            good &= ok;
            if (good) {
                if (v1 < v2)
                    return -1;
                else if (v1 == v2)
                    return 0;
                else
                    return 1;
            } else {
                return QString::compare(_val1, _val2, Qt::CaseInsensitive);
            }
        } else {
            QStringList _vals1 = _val1.split(".");
            QStringList _vals2 = _val2.split(".");
            if (_vals1.size() == _vals2.size()) {
                bool good = false, ok = false;
                for(int i = 0; i < _vals1.size(); i++){
                    v1 = _vals1.at(i).toInt(&good);
                    v2 = _vals2.at(i).toInt(&ok);
                    good &= ok;
                    if (good) {
                        if (v1 < v2)
                            result = -1;
                        else if (v1 == v2)
                            result = 0;
                        else
                            result = 1;
                        // if value is not 0 (equal), return result (-1, or 1)
                        if (result)
                            return result;
                    } else {
                       return QString::compare(_val1, _val2, Qt::CaseInsensitive);
                    }
                }
            } else {
                return QString::compare(_val1, _val2, Qt::CaseInsensitive);
            }
        }
        return result;
    };

    int vc =  compareLibVersion(); // QString::compare(val1, val2, Qt::CaseInsensitive);

    logInfo() << QString("Library version check - [%1] minimum :[%2] installed:[%3]"/* vc(%4): %5]" */)
                         .arg(libName).arg(val1).arg(val2)/* .arg(vc).arg(vc < 0 ? "v1 < v2" : vc == 0 ? "v1 = v2" : "v1 > v2") */;

    if (vc > 0) {
        logTrace() << QString("Library %1 version [%2] is less than required version [%3]")
                              .arg(libName).arg(val2).arg(libVersion);
        return false;
    }

    return true;
}
#endif

void Preferences::lpubPreferences()
{
    lpub3dAppName = QCoreApplication::applicationName();
    modeGUI = Application::instance()->modeGUI();
    QDir cwd(QCoreApplication::applicationDirPath());
#ifdef QT_DEBUG_MODE
    qDebug() << "";
    qDebug() << "--------------------------";
#else
    fprintf(stdout, "\n");
    fprintf(stdout, "--------------------------\n");
#endif
#ifdef Q_OS_MAC

    //qDebug() << QString("macOS Binary Directory (%1), AbsPath (%2)").arg(cwd.dirName()).arg(cwd.absolutePath());
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("macOS Binary Directory.......(%1)").arg(cwd.dirName()));
#else
    fprintf(stdout, "%s\n", QString(QString("macOS Binary Directory.......(%1)").arg(cwd.dirName())).toLatin1().constData());
#endif
    if (cwd.dirName() == "MacOS") {   // MacOS/         (app bundle executable folder)
        cwd.cdUp();                   // Contents/      (app bundle contents folder)
        cwd.cdUp();                   // LPub3D.app/    (app bundle folder)
        cwd.cdUp();                   // Applications/  (app bundle installation folder)
    }
    //qDebug() << QString("macOS Base Directory (%1), AbsPath (%2)").arg(cwd.dirName()).arg(cwd.absolutePath());
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("macOS Base Directory.........(%1)").arg(cwd.dirName()));
#else
    fprintf(stdout, "%s\n", QString(QString("macOS Base Directory.........(%1)").arg(cwd.dirName())).toLatin1().constData());
#endif

    lpub3dExtrasResourcePath = QString("%1/%2.app/Contents/Resources").arg(cwd.absolutePath(),lpub3dAppName);
    lpub3dDocsResourcePath   = lpub3dExtrasResourcePath;

    if (QCoreApplication::applicationName() != QString(VER_PRODUCTNAME_STR))
    {
#ifdef QT_DEBUG_MODE
        qDebug() << QString(QString("macOS Info.plist update......(%1)").arg(lpub3dAppName));
#else
        fprintf(stdout, "%s\n", QString(QString("macOS Info.plist update......(%1)").arg(lpub3dAppName)).toLatin1().constData());
#endif
        QFileInfo plbInfo("/usr/libexec/PlistBuddy");
        QString plistCmd = QString("%1 -c").arg(plbInfo.absoluteFilePath());
        QString infoPlistFile = QString("%1/%2.app/Contents/Info.plist").arg(cwd.absolutePath(),lpub3dAppName);
        if (plbInfo.exists())
        {
            QProcess::execute(QString("%1 \"Set :CFBundleExecutable %2\" \"%3\"").arg(plistCmd,lpub3dAppName,infoPlistFile));
            QProcess::execute(QString("%1 \"Set :CFBundleName %2\" \"%3\"").arg(plistCmd,lpub3dAppName,infoPlistFile));
            QProcess::execute(QString("%1 \"Set :CFBundleDisplayName %2\" \"%3\"").arg(plistCmd,lpub3dAppName,infoPlistFile));
            QProcess::execute(QString("%1 \"Set :CFBundleIdentifier com.trevorsandy.%2\" \"%3\"").arg(plistCmd,lpub3dAppName.toLower(),infoPlistFile));
            QProcess::execute(QString("%1 \"Set :UTExportedTypeDeclarations:0:UTTypeIdentifier com.trevorsandy.%2\" \"%3\"").arg(plistCmd,lpub3dAppName.toLower(),infoPlistFile));
            QProcess::execute(QString("%1 \"Set :UTExportedTypeDeclarations:0:UTTypeIdentifier com.trevorsandy.%2\" \"%3\"").arg(plistCmd,lpub3dAppName.toLower(),infoPlistFile));
            QProcess::execute(QString("%1 \"Set :UTExportedTypeDeclarations:0:UTTypeIdentifier com.trevorsandy.%2\" \"%3\"").arg(plistCmd,lpub3dAppName.toLower(),infoPlistFile));
        } else {
#ifdef QT_DEBUG_MODE
            qDebug() << QString(QString("ERROR - %1 not found, cannot update Info.Plist").arg(plbInfo.absoluteFilePath()));
#else
            fprintf(stdout, "%s\n", QString(QString("ERROR - %1 not found, cannot update Info.Plist").arg(plbInfo.absoluteFilePath())).toLatin1().constData());
#endif
        }
    }

#elif defined Q_OS_LINUX

    // Check if running as AppImage payload
    // Check for the presence of $ORIGIN/../AppRun *or* $ORIGIN/../../AppRun
    // (can be a file or a symlink) (with $ORIGIN being the application itself)
    QDir appRunDir(QString("%1/../../").arg(cwd.absolutePath()));
    QFileInfo appRunInfo(QString("%1/AppRun").arg(appRunDir.absolutePath()));
    if (appRunInfo.exists() && (appRunInfo.isFile() || appRunInfo.isSymLink())) {
        isAppImagePayload = true;
    } else {
        appRunDir.setPath(QString("%1/../").arg(cwd.absolutePath()));
        appRunInfo.setFile(QString("%1/AppRun").arg(appRunDir.absolutePath()));
        if (appRunInfo.exists() && (appRunInfo.isFile() || appRunInfo.isSymLink()))
            isAppImagePayload = true;
    }

    QDir appDir(QString("%1/../share").arg(cwd.absolutePath()));

    // This is a shameless hack until I figure out a better way to get the application name folder
    QStringList fileFilters;
    fileFilters << "lpub3d*";

    QDir contentsDir(appDir.absolutePath() + "/");
    QStringList shareContents = contentsDir.entryList(fileFilters);

    if (shareContents.size() > 0)
    {
        // Because the QCoreApplication::applicationName() is not the same as the LPub3D
        // executable name in an AppImage and the executable name is not the same as the
        // application folder, we set 'lpub3dAppName' to the value of the lpub3d application folder.
        // The application folder value is set with the DIST_TARGET variable in mainApp.pro
        lpub3dAppName = shareContents.at(0);
#ifdef QT_DEBUG_MODE
        qDebug() << QString(QString("LPub3D Application Folder....(%1)").arg(lpub3dAppName));
#else
        fprintf(stdout, "%s\n", QString(QString("LPub3D Application Folder....(%1)").arg(lpub3dAppName)).toLatin1().constData());
#endif
    } else {
#ifdef QT_DEBUG_MODE
        qDebug() << QString(QString("ERROR - Application Folder Not Found."));
#else
        fprintf(stdout, "%s\n", QString(QString("ERROR - Application Folder Not Found.")).toLatin1().constData());
#endif
    }

#ifdef X11_BINARY_BUILD                                               // Standard User Rights Install

    lpub3dDocsResourcePath   = QString("doc");

#else                                                                 // Elevated User Rights Install

    lpub3dDocsResourcePath   = QString("%1/doc/%2").arg(appDir.absolutePath(),lpub3dAppName);
    lpub3dExtrasResourcePath = QString("%1/%2").arg(appDir.absolutePath(),lpub3dAppName);

#endif

#elif defined Q_OS_WIN

    lpub3dDocsResourcePath   = QString("docs");

#endif

    lpub3dPath = cwd.absolutePath();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    // Default user data path
    QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
    lpubDataPath = dataPathList.first();

    // Default cache path
    QStringList cachePathList = QStandardPaths::standardLocations(QStandardPaths::CacheLocation);
    lpub3dCachePath = cachePathList.first();

    // Default configuration path
    QStringList configPathList = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    lpubConfigPath = configPathList.first();

    // DEBUG
#if defined (LP3D_FLATPACK) || defined(LP3D_SNAP)
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("LPub3D Default App Data Path.(%1)").arg(lpubDataPath));
    qDebug() << QString(QString("LPub3D Default Config Path.. (%1)").arg(lpubConfigPath));
    qDebug() << QString(QString("LPub3D Default 3D Parts Path.(%1)").arg(lpub3dCachePath));
#else
    fprintf(stdout, "%s\n", QString(QString("LPub3D Default App Data Path.(%1)").arg(lpubDataPath)).toLatin1().constData());
    fprintf(stdout, "%s\n", QString(QString("LPub3D Default Config Path.. (%1)").arg(lpubConfigPath)).toLatin1().constData());
    fprintf(stdout, "%s\n", QString(QString("LPub3D Default 3D Parts Path.(%1)").arg(lpub3dCachePath)).toLatin1().constData());
#endif
#endif // LP3D_FLATPACK or LP3D_SNAP
#endif // (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))

#ifdef Q_OS_WIN //... Windows portable or installed
    QSettings Settings;

    if (portableDistribution) { // we have a portable distribution

        bool programFolder = QCoreApplication::applicationDirPath().contains("Program Files") ||
                QCoreApplication::applicationDirPath().contains("Program Files (x86)");

        if (programFolder) {                     // ...installed in Program Folder directory

            if (modeGUI) {
                // Get the application icon as a pixmap
                QPixmap _icon = QPixmap(":/icons/lpub96.png");
                QMessageBoxResizable box;
                box.setWindowIcon(QIcon());
                box.setIconPixmap (_icon);
                box.setTextFormat (Qt::RichText);
                box.setWindowTitle(QMessageBox::tr ("Installation"));
                box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

                QString header  = "<b> " + QMessageBox::tr ("Data directory installation folder.") + "</b>";
                QString body = QMessageBox::tr ("Would you like to create a folder outside the Program Files / (x86) directory? \n"
                                                "If you choose No, the data directory will automatically be created in the user's AppData directory.");
                QString detail = QMessageBox::tr ("It looks like this installation is a portable or packaged (i.e. AIOI) distribution \n"
                                                  "of LPub3D installed under the system's Program Files/(x86) directory.\n\n"
                                                  "Updatable data will not be able to be written to unless you modify\n"
                                                  "user account access for this folder which is not recommended.\n\n"
                                                  "You should consider changing the installation folder or placing\n"
                                                  "the updatable data folder outside the Program Files/(x86) directory\n\n"
                                                  "Choose yes to continue and select a data folder outside Program Files/(x86).\n\n"
                                                  "If you choose No, the data directory will automatically be created in the user's AppData directory.");
                box.setText (header);
                box.setInformativeText (body);
                box.setDetailedText(detail);
                box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
                box.setDefaultButton   (QMessageBox::Yes);

                QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
                lpubDataPath = dataPathList.first();

                if (box.exec() == QMessageBox::Yes) {   // capture user's choice for user data directory

                    QString result = QFileDialog::getExistingDirectory(nullptr,
                                                                       QFileDialog::tr("Select Directory"),
                                                                       lpubDataPath,
                                                                       QFileDialog::ShowDirsOnly |
                                                                       QFileDialog::DontResolveSymlinks);
                    if (! result.isEmpty()) {
                        lpubDataPath = QDir::toNativeSeparators(result);
                    }
                }

            } else {                                          // console mode create automatically

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
                QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
                lpubDataPath = dataPathList.first();
#endif
            }

        } else {                                 // ...installed outside Program Folder directory

            lpubDataPath = lpub3dPath;
        }

        // Cache path
        lpub3dCachePath = QString("%1/cache").arg(lpub3dPath);

    } else {                    // we have an installed distribution

        QString const LPub3DDataPathKey("LPub3DDataPath");

        if (Settings.contains(QString("%1/%2").arg(SETTINGS,LPub3DDataPathKey))) {

            lpubDataPath = Settings.value(QString("%1/%2").arg(SETTINGS,LPub3DDataPathKey)).toString();

        } else {

            Settings.setValue(QString("%1/%2").arg(SETTINGS, LPub3DDataPathKey), lpubDataPath);
        }
    }

#else  // Q_OS_LINUX or Q_OS_MAC

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    const int dataIndex = lpubDataPath.indexOf("/share/");
    QString dataPrefixReplace = dataIndex > -1 ? lpubDataPath.left(dataIndex) : QString();

    const int cacheIndex = lpub3dCachePath.indexOf("/.cache/");
    QString cachePrefixReplace = cacheIndex > -1 ? lpub3dCachePath.left(cacheIndex) : QString();

    const int configIndex = lpubConfigPath.indexOf("/.config");
    QString configPrefixReplace = configIndex > -1 ? lpubConfigPath.left(configIndex) : QString();

    QString dataPrefixNew, cachePrefixNew, configPrefixNew;
#if defined(LP3D_SNAP)
    // Example         : /home/<user>/snap/<Snap name>/<Snap revision>/...
    char* snapUserDataPath = getenv("SNAP_USER_DATA");
    if (snapUserDataPath && snapUserDataPath[0])
        dataPrefixNew = QString(snapUserDataPath);

    char* snapUserCommonPath = getenv("SNAP_USER_COMMON");
    if (snapUserCommonPath && snapUserCommonPath[0])
        cachePrefixNew = configPrefixNew = QString(snapUserCommonPath);
#elif defined (LP3D_FLATPACK)
    // Example         : /home/<user>/.var/app/<Flapak name>/...
    char* flatpakUserDataPath = getenv("XDG_DATA_HOME");
    if (flatpakUserDataPath && flatpakUserDataPath[0]) {
        dataPrefixReplace.append("/share");
        dataPrefixNew = QString(flatpakUserDataPath);
    }

    char* flatpakUserCachePath = getenv("XDG_CACHE_HOME");
    if (flatpakUserCachePath && flatpakUserCachePath[0]) {
        cachePrefixReplace.append("/.cache");
        cachePrefixNew = QString(flatpakUserCachePath);
    }

    char* flatpakUserConfigPath = getenv("XDG_CONFIG_HOME");
    if (flatpakUserConfigPath && flatpakUserConfigPath[0]) {
        configPrefixReplace.append("/.config");
        configPrefixNew = QString(flatpakUserConfigPath);
    }

    // For Flatpak build check, unpack the user data to /app
    char* flatpakBuildCheck = getenv("FLATPAK_BUILD_CHECK");
    if (flatpakBuildCheck)
        dataPrefixNew = cachePrefixNew = configPrefixNew = "/app";
#endif // LP3D_SNAP, LP3D_FLATPACK
     // Linux example   : /home/<user>/.local/share/lpub3d/...
     // AppImage example: /home/<user>/.local/share/<AppImage name>/...
     if (! dataPrefixNew.isEmpty() && ! dataPrefixReplace.isEmpty())
        lpubDataPath = lpubDataPath.replace(dataPrefixReplace, dataPrefixNew);

     if (! cachePrefixNew.isEmpty() && ! cachePrefixReplace.isEmpty())
        lpub3dCachePath = lpub3dCachePath.replace(cachePrefixReplace, cachePrefixNew);

     if (! configPrefixNew.isEmpty() && ! configPrefixReplace.isEmpty())
        lpubConfigPath = lpubConfigPath.replace(configPrefixReplace, configPrefixNew);

#if defined (LP3D_FLATPACK) || defined(LP3D_SNAP)
     QDir configDir(lpubConfigPath);
     if(! QDir(configDir).exists())
         configDir.mkpath(".");
     QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, configDir.absolutePath());
#endif // LP3D_FLATPACK or LP3D_SNAP
#endif // (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#endif // Q_OS_WIN, Q_OS_LINUX or Q_OS_MAC

    // applications paths:
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("LPub3D App Data Path.........(%1)").arg(lpubDataPath));
#else
    fprintf(stdout, "%s\n", QString(QString("LPub3D App Data Path.........(%1)").arg(lpubDataPath)).toLatin1().constData());
#endif
#ifdef Q_OS_MAC
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("LPub3D Bundle App Path.......(%1)").arg(lpub3dPath));
#else
    fprintf(stdout, "%s\n", QString(QString("LPub3D Bundle App Path.......(%1)").arg(lpub3dPath)).toLatin1().constData());
#endif
#else // Q_OS_LINUX or Q_OS_WIN
    QString _logPath = QString("%1/logs/%2Log.txt").arg(lpubDataPath).arg(VER_PRODUCTNAME_STR);
#ifdef Q_OS_LINUX
    _logPath = _logPath.toLower();
#endif
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("LPub3D Executable Path.......(%1)").arg(lpub3dPath));
    qDebug() << QString(QString("LPub3D Log Path..............(%1)").arg(_logPath));
#else
    fprintf(stdout, "%s\n", QString(QString("LPub3D Executable Path.......(%1)").arg(lpub3dPath)).toLatin1().constData());
    fprintf(stdout, "%s\n", QString(QString("LPub3D Log Path..............(%1)").arg(_logPath)).toLatin1().constData());
#endif
#endif // Q_OS_WIN, Q_OS_LINUX or Q_OS_MAC

#ifdef Q_OS_WIN
    QString dataDir = "data";
    QString dataPath = lpub3dPath;
    if (portableDistribution) {
        dataDir = "extras";
#ifdef QT_DEBUG_MODE
        qDebug() << QString("LPub3D Portable Distribution.(Yes)");
#else
        fprintf(stdout, "%s\n", QString("LPub3D Portable Distribution.(Yes)").toLatin1().constData());
#endif
    // On Windows installer 'dataLocation' folder defaults to LPub3D install path but can be set with 'DataLocation' reg key
    } else if (Settings.contains(QString("%1/%2").arg(SETTINGS,"DataLocation"))) {
        QString validDataPath = Settings.value(QString("%1/%2").arg(SETTINGS,"DataLocation")).toString();
        QDir validDataDir(QString("%1/%2/").arg(validDataPath,dataDir));
        if(QDir(validDataDir).exists()) {
           dataPath = validDataPath;
#ifdef QT_DEBUG_MODE
           qDebug() << QString(QString("LPub3D Data Location.........(%1)").arg(validDataDir.absolutePath()));
#else
           fprintf(stdout, "%s\n", QString(QString("LPub3D Data Location.........(%1)").arg(validDataDir.absolutePath())).toLatin1().constData());
#endif
        }
    }
    dataLocation = QString("%1/%2/").arg(dataPath,dataDir);

#else // Q_OS_LINUX or Q_OS_MAC
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("LPub3D Extras Resource Path..(%1)").arg(lpub3dExtrasResourcePath));
#else
    fprintf(stdout, "%s\n", QString(QString("LPub3D Extras Resource Path..(%1)").arg(lpub3dExtrasResourcePath)).toLatin1().constData());
#endif
    // On Linux 'dataLocation' folder is /usr/share/lpub3d
    // On macOS 'dataLocation' folder is /Applications/LPub3D.app/Contents/Resources
    dataLocation = QString("%1/").arg(lpub3dExtrasResourcePath);
#if defined Q_OS_LINUX
    QDir rendererDir(QString("%1/../../%2/%3").arg(lpub3dPath)
                                              .arg(optPrefix.isEmpty() ? "opt" : optPrefix+"/opt")
                                              .arg(lpub3dAppName));
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("LPub3D Renderers Exe Path....(%1/3rdParty)").arg(rendererDir.absolutePath()));
#else
    fprintf(stdout, "%s\n", QString(QString("LPub3D Renderers Exe Path....(%1/3rdParty)").arg(rendererDir.absolutePath())).toLatin1().constData());
#endif
#endif // Q_OS_LINUX
#endif // Q_OS_WIN, Q_OS_LINUX or Q_OS_MAC

#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("LPub3D Config File Path......(%1)").arg(lpubConfigPath));
    qDebug() << QString(QString("LPub3D 3D Editor Cache Path..(%1)").arg(lpub3dCachePath));
#else
    fprintf(stdout, "%s\n", QString(QString("LPub3D Config File Path......(%1)").arg(lpubConfigPath)).toLatin1().constData());
    fprintf(stdout, "%s\n", QString(QString("LPub3D 3D Editor Cache Path..(%1)").arg(lpub3dCachePath)).toLatin1().constData());
#endif

    QDir extrasDir(lpubDataPath + QDir::separator() + "extras");
    if(!QDir(extrasDir).exists())
        extrasDir.mkpath(".");

    QFileInfo paramFile(QString("%1/%2").arg(extrasDir.absolutePath(), validLDrawColorParts));
    QFileInfo dataFile(dataLocation + paramFile.fileName());
    if (!paramFile.exists()) {
        const QString fileName = dataFile.exists() ? paramFile.fileName() : VER_LPUB3D_COLOR_PARTS;
        QFile::copy(dataLocation + fileName, paramFile.absoluteFilePath());
    }
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_FREEFOM_ANNOTATIONS_FILE));
    if (!paramFile.exists())
        QFile::copy(dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_EXTRAS_LDCONFIG_FILE));
    if (!paramFile.exists())
        QFile::copy(dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_PDFPRINT_IMAGE_FILE));
    if (!paramFile.exists())
        QFile::copy(dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_PLI_CONTROL_FILE));
    if (!paramFile.exists())
        QFile::copy(dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_PLI_SUBSTITUTE_PARTS_FILE));
    if (!paramFile.exists())
        QFile::copy(dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_TITLE_ANNOTATIONS_FILE));
    if (!paramFile.exists())
        QFile::copy(dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_EXCLUDED_PARTS_FILE));
    if (!paramFile.exists())
        QFile::copy(dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_STICKER_PARTS_FILE));
    if (!paramFile.exists())
        QFile::copy(dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("LPub3D Loaded LDraw Library..(%1)").arg(validLDrawPartsLibrary));
    qDebug() << "--------------------------";
//    qDebug() << "";
#else
    fprintf(stdout, "%s\n", QString(QString("LPub3D Loaded LDraw Library..(%1 Parts)").arg(validLDrawLibrary)).toLatin1().constData());
    fprintf(stdout, "--------------------------\n");
//    fprintf(stdout, "\n");
    fflush(stdout);
#endif
}

void Preferences::loggingPreferences()
{
    // define log path
    QDir logDir(lpubDataPath+"/logs");
    if(!QDir(logDir).exists())
        logDir.mkpath(".");
    logPath = QDir(logDir).filePath(QString("%1Log.txt").arg(VER_PRODUCTNAME_STR));

    QSettings Settings;
    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeLogLevel"))) {
        QVariant uValue(true);
        includeLogLevel = true;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeLogLevel"),uValue);
    } else {
        includeLogLevel = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeLogLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeTimestamp"))) {
        QVariant uValue(false);
        includeTimestamp = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeTimestamp"),uValue);
    } else {
        includeTimestamp = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeTimestamp")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeFileName"))) {
        QVariant uValue(false);
        includeFileName = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeFileName"),uValue);
    } else {
        includeFileName = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeFileName")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeLineNumber"))) {
        QVariant uValue(true);
        includeLineNumber = true;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeLineNumber"),uValue);
    } else {
        includeLineNumber = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeLineNumber")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeFunction"))) {
        QVariant uValue(true);
        includeFunction = true;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeFunction"),uValue);
    } else {
        includeFunction = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeFunction")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeAllLogAttributes"))) {
        QVariant uValue(false);
        includeAllLogAttributes = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeAllLogAttributes"),uValue);
    } else {
        includeAllLogAttributes = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeAllLogAttributes")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"Logging"))) {
        QVariant uValue(true);
        logging = true;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"Logging"),uValue);
    } else {
        logging = Settings.value(QString("%1/%2").arg(LOGGING,"Logging")).toBool();
    }

    // log levels combo
    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"LoggingLevel"))) {
        QVariant uValue(LOGGING_LEVEL_DEFAULT);
        loggingLevel = LOGGING_LEVEL_DEFAULT;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"LoggingLevel"),uValue);
    } else {
        loggingLevel = Settings.value(QString("%1/%2").arg(LOGGING,"LoggingLevel")).toString();
    }
    // log levels group box
    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"LogLevel"))) {
        QVariant uValue(true);
        logLevel = true;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"LogLevel"),uValue);
    } else {
        logLevel = Settings.value(QString("%1/%2").arg(LOGGING,"LogLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"LogLevels"))) {
        QVariant uValue(false);
        logLevels = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"LogLevels"),uValue);
    } else {
        logLevels = Settings.value(QString("%1/%2").arg(LOGGING,"LogLevels")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"DebugLevel"))) {
        QVariant uValue(false);
        debugLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"DebugLevel"),uValue);
    } else {
        debugLevel = Settings.value(QString("%1/%2").arg(LOGGING,"DebugLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"TraceLevel"))) {
        QVariant uValue(false);
        traceLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"TraceLevel"),uValue);
    } else {
        traceLevel = Settings.value(QString("%1/%2").arg(LOGGING,"TraceLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"NoticeLevel"))) {
        QVariant uValue(false);
        noticeLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"NoticeLevel"),uValue);
    } else {
        noticeLevel = Settings.value(QString("%1/%2").arg(LOGGING,"NoticeLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"InfoLevel"))) {
        QVariant uValue(false);
        infoLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"InfoLevel"),uValue);
    } else {
        infoLevel = Settings.value(QString("%1/%2").arg(LOGGING,"InfoLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"StatusLevel"))) {
        QVariant uValue(false);
        statusLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"StatusLevel"),uValue);
    } else {
        statusLevel = Settings.value(QString("%1/%2").arg(LOGGING,"StatusLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"WarningLevel"))) {
        QVariant uValue(false);
        warningLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"WarningLevel"),uValue);
    } else {
        warningLevel = Settings.value(QString("%1/%2").arg(LOGGING,"WarningLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"ErrorLevel"))) {
        QVariant uValue(false);
        errorLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"ErrorLevel"),uValue);
    } else {
        errorLevel = Settings.value(QString("%1/%2").arg(LOGGING,"ErrorLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"FatalLevel"))) {
        QVariant uValue(false);
        fatalLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"FatalLevel"),uValue);
    } else {
        fatalLevel = Settings.value(QString("%1/%2").arg(LOGGING,"FatalLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"AllLogLevels"))) {
        QVariant uValue(false);
        allLogLevels = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"AllLogLevels"),uValue);
    } else {
        allLogLevels = Settings.value(QString("%1/%2").arg(LOGGING,"AllLogLevels")).toBool();
    }

}

void Preferences::lpub3dLibPreferences(bool browse)
{
    if (modeGUI && ! lpub3dLoaded)
        emit Application::instance()->splashMsgSig("5% - Locate LDraw archive libraries...");

#ifdef Q_OS_WIN
    QString filter(QFileDialog::tr("Archive (*.zip *.bin);;All Files (*.*)"));
#else
    QString filter(QFileDialog::tr("All Files (*.*)"));
#endif

    QFileInfo fileInfo;
    QSettings Settings;

    // check if archive parts on launch enabled
    QString const archivePartsOnLaunchKey("ArchivePartsOnLaunch");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,archivePartsOnLaunchKey))) {
        QVariant uValue(archivePartsOnLaunch);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,archivePartsOnLaunchKey),uValue);
    } else {
        archivePartsOnLaunch = Settings.value(QString("%1/%2").arg(SETTINGS,archivePartsOnLaunchKey)).toBool();
    }

    // check if skip parts archive selected - used by refresh LDraw parts routine
    if (Settings.contains(QString("%1/%2").arg(DEFAULTS,SAVE_SKIP_PARTS_ARCHIVE_KEY))) {
        skipPartsArchive = Settings.value(QString("%1/%2").arg(DEFAULTS,SAVE_SKIP_PARTS_ARCHIVE_KEY)).toBool();
        Settings.remove(QString("%1/%2").arg(DEFAULTS,SAVE_SKIP_PARTS_ARCHIVE_KEY));
    }

    QString const PartsLibraryKey("PartsLibrary");
    lpub3dLibFile = Settings.value(QString("%1/%2").arg(SETTINGS,PartsLibraryKey)).toString();

    // Set archive library path, also check alternate location (e.g. AIOI uses C:\Users\Public\Documents\LDraw)
    QString validFile = QString("%1/libraries/%2").arg(lpubDataPath, validLDrawPartsArchive);

    bool altLibLocation = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,"AltLibLocation")))
        altLibLocation = Settings.value(QString("%1/%2").arg(SETTINGS,"AltLibLocation")).toBool();

    // Start by checking the registry value, if not exist set to valid file path
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,PartsLibraryKey))) {
        lpub3dLibFile = Settings.value(QString("%1/%2").arg(SETTINGS,PartsLibraryKey)).toString();
    } else {
        lpub3dLibFile = QDir::toNativeSeparators(validFile);
        Settings.setValue(QString("%1/%2").arg(SETTINGS, PartsLibraryKey), lpub3dLibFile);
    }

    // Confirm archive library location is valid
    if (lpub3dLibFile != validFile && !altLibLocation) {
        lpub3dLibFile = QDir::toNativeSeparators(validFile);
        Settings.setValue(QString("%1/%2").arg(SETTINGS, PartsLibraryKey), lpub3dLibFile);
    }

    // If we have a valid library archive file, update the Visual Editor parts_library variable, else clear the registry value
    if (! lpub3dLibFile.isEmpty() && ! browse) {
        fileInfo.setFile(lpub3dLibFile);

        if (fileInfo.exists()) {
            QString partsLibrary = Settings.value(QString("%1/%2").arg(SETTINGS,PartsLibraryKey)).toString();
            return;
        }
        else {
            lpub3dLibFile.clear();
            Settings.remove(QString("%1/%2").arg(SETTINGS, PartsLibraryKey));
        }
    }

    // Request to open a dialogue to select library path
    if (/*! lpub3dLibFile.isEmpty() && */ browse){

        QString result = QFileDialog::getOpenFileName(nullptr,
                                                      QFileDialog::tr("Select LDraw Library Archive"),
                                                      lpub3dLibFile.isEmpty() ? "." : lpub3dLibFile,
                                                      filter);
        if (! result.isEmpty())
            lpub3dLibFile = QDir::toNativeSeparators(result);
    }

    // Archive library exist, so set registry value
    if (! lpub3dLibFile.isEmpty()) {

        Settings.setValue(QString("%1/%2").arg(SETTINGS, PartsLibraryKey), lpub3dLibFile);

    } else {

        // lets go look for the archive files...

        fileInfo.setFile(dataLocation + validLDrawPartsArchive);
        bool archivesExist = fileInfo.exists();

        // DEBUG DEBUG DEBUG
        //        QMessageBox::information(nullptr,
        //                                 QMessageBox::tr("LPub3D"),
        //                                 QMessageBox::tr("lpub3dPath: (%1)\n\n"
        //                                                 "lpub3dExtrasResourcePath [dataLocation]: (%2)\n\n"
        //                                                 "ArchivesExist: (%3)\n\n"
        //                                                 "Full Archives Path: (%4)")
        //                                                 .arg(lpub3dPath)
        //                                                 .arg(lpub3dExtrasResourcePath)
        //                                                 .arg(archivesExist ? "Yes" : "No")
        //                                                 .arg(fileInfo.absoluteFilePath()));

        if (archivesExist) {  // This condition should always fire as archive files are deposited at installation

            emit Application::instance()->splashMsgSig("10% - Copying archive libraries...");

            QDir libraryDir(QString("%1/%2").arg(lpubDataPath).arg(Paths::libraryDir));
            if (!QDir(libraryDir).exists())
                libraryDir.mkpath(".");

            fileInfo.setFile(QString("%1/%2").arg(libraryDir.absolutePath(), validLDrawPartsArchive));
            if (!fileInfo.exists())
                QFile::copy(dataLocation + fileInfo.fileName(), fileInfo.absoluteFilePath());

            lpub3dLibFile = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
            Settings.setValue(QString("%1/%2").arg(SETTINGS, PartsLibraryKey), lpub3dLibFile);

            if (usingDefaultLibrary)
                fileInfo.setFile(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LPUB3D_UNOFFICIAL_ARCHIVE));
            if (usingDefaultLibrary && !fileInfo.exists())
                QFile::copy(dataLocation + fileInfo.fileName(), fileInfo.absoluteFilePath());

        } else if (modeGUI) { // This condition should never fire - left over old code that offers to select or download the archive libraries (in case the user removes them)

#ifdef Q_OS_MAC
            if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isVisible())
                Application::instance()->splash->hide();
#endif
            QPixmap _icon = QPixmap(":/icons/lpub96.png");
            QMessageBoxResizable box;
            box.setWindowIcon(QIcon());
            box.setIconPixmap (_icon);
            box.setTextFormat (Qt::RichText);
            box.setWindowTitle(QMessageBox::tr ("Library Selection"));
            box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

            QAbstractButton* downloadButton = box.addButton(QMessageBox::tr("Download"),QMessageBox::YesRole);
            QAbstractButton* selectButton   = box.addButton(QMessageBox::tr("Select"),QMessageBox::YesRole);

            QString header = "<b> " + QMessageBox::tr ("No LDraw library archive defined!") + "</b>";
            QString body = QMessageBox::tr ("Note: The LDraw library archives are not provided and <u>must be downloaded</u> - or selected.\n"
                                            "Would you like to download or select the library archives?");
            QString detail = QMessageBox::tr ("You must select or create your LDraw library archive files.\n"
                                              "The location of your official archive file (%1) should "
                                              "also have the unofficial archive file (%2).\n"
                                              "LDraw library archive files can be copied, downloaded or selected to your '%1/%2/' folder now.")
                                              .arg(validLDrawPartsArchive)
                                              .arg(usingDefaultLibrary ? VER_LPUB3D_UNOFFICIAL_ARCHIVE : "")
                                              .arg(lpubDataPath, "libraries");
            box.setText (header);
            box.setInformativeText (body);
            box.setDetailedText(detail);
            box.setStandardButtons (QMessageBox::Cancel);
            box.exec();

            if (box.clickedButton()==selectButton) {
                emit Application::instance()->splashMsgSig("10% - Selecting archive libraries...");

                QFileDialog dlgGetFileName(nullptr,QFileDialog::tr("Select LDraw Library Archive "));
                dlgGetFileName.setDirectory(lpubDataPath);
                dlgGetFileName.setNameFilter(filter);

                if (dlgGetFileName.exec() == QFileDialog::Accepted) {
                    lpub3dLibFile = QDir::toNativeSeparators(dlgGetFileName.selectedFiles().at(0));
                    Settings.setValue(QString("%1/%2").arg(SETTINGS, PartsLibraryKey), lpub3dLibFile);

                } else {

                    Settings.remove(QString("%1/%2").arg(SETTINGS, PartsLibraryKey));
                    body = QMessageBox::tr ("<u>Selection cancelled</u>.\n"
                                            "The application will terminate.");
                    box.removeButton(selectButton);
                    box.removeButton(downloadButton);
                    box.setStandardButtons (QMessageBox::Close);
                    box.setText(header);
                    box.setInformativeText(body);

                    box.exec();
                    lpub3dLibFile.clear();

                    exit(-1);
                }

            } else
                if (box.clickedButton()==downloadButton) {

#ifdef Q_OS_MAC
                    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isHidden())
                        Application::instance()->splash->show();
#endif
                    emit Application::instance()->splashMsgSig("10% - Downloading archive libraries...");

                    QDir libraryDir(QString("%1/%2").arg(lpubDataPath, "libraries"));
                    if (!QDir(libraryDir).exists())
                        libraryDir.mkpath(".");

                    UpdateCheck *libraryDownload;
                    QEventLoop  *wait = new QEventLoop();
                    libraryDownload  = new UpdateCheck(nullptr, (void*)LDrawOfficialLibraryDirectDownload);
                    wait->connect(libraryDownload, SIGNAL(downloadFinished(QString,QString)), wait, SLOT(quit()));
                    wait->connect(libraryDownload, SIGNAL(cancel()),                          wait, SLOT(quit()));
                    libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), libraryDir.absolutePath());
                    wait->exec();
                    if (libraryDownload->getCancelled()) {
                        body = QMessageBox::tr ("<u>Official LDraw archive download cancelled</u>.\n"
                                                "The application will terminate.");
                        box.removeButton(selectButton);
                        box.removeButton(downloadButton);
                        box.setStandardButtons (QMessageBox::Close);
                        box.setText(header);
                        box.setInformativeText(body);
                        box.exec();

                        exit(-1);
                    }
                    libraryDownload  = new UpdateCheck(nullptr, (void*)LDrawUnofficialLibraryDirectDownload);
                    wait->connect(libraryDownload, SIGNAL(downloadFinished(QString,QString)), wait, SLOT(quit()));
                    wait->connect(libraryDownload, SIGNAL(cancel()),                          wait, SLOT(quit()));
                    libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), libraryDir.absolutePath());
                    wait->exec();
                    if (libraryDownload->getCancelled()) {
                        body = QMessageBox::tr ("<u>Unofficial LDraw archive download cancelled</u>.\n"
                                                "The application will continue; however, your LDraw archives\n"
                                                "will not contain unofficial LDraw parts or primitives.");
                        box.removeButton(selectButton);
                        box.removeButton(downloadButton);
                        box.setStandardButtons (QMessageBox::Close);
                        box.setText(header);
                        box.setInformativeText(body);
                        box.exec();
                    }

                    // validate downloaded files
                    lpub3dLibFile = QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), validLDrawPartsArchive));
                    fileInfo.setFile(lpub3dLibFile);
                    if (!fileInfo.exists()) {

                        Settings.remove(QString("%1/%2").arg(SETTINGS, PartsLibraryKey));
                        if (usingDefaultLibrary)
                            fileInfo.setFile(QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LDRAW_UNOFFICIAL_ARCHIVE)));
                        if (usingDefaultLibrary && !fileInfo.exists()) {
                            body = QMessageBox::tr ("Required archive files\n%1\n%2\ndoes not exist.").arg(lpub3dLibFile, fileInfo.absoluteFilePath());
                        } else {
                            body = QMessageBox::tr ("Required archive file %1 does not exist.").arg(lpub3dLibFile);
                        }
                        body.append("\nThe application will terminate.");

                        box.setStandardButtons (QMessageBox::Close);
                        box.setText(header);
                        box.setInformativeText(body);

                        box.exec();
                        lpub3dLibFile.clear();

                        exit(-1);
                    }

                    // extract downloaded files
                    extractLDrawLib();

                } else {

                    exit(-1);

                }
#ifdef Q_OS_MAC
            if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isHidden())
                Application::instance()->splash->show();
#endif
        } else {              // If we get here, inform the user that required archive libraries do not exist (performing build check or they were probably removed)

            QString officialArchive = fileInfo.absoluteFilePath();
            if (usingDefaultLibrary)
                fileInfo.setFile(dataLocation + VER_LPUB3D_UNOFFICIAL_ARCHIVE);
            if (usingDefaultLibrary && !fileInfo.exists()) {
                fprintf(stderr, "Required archive files\n%s\n%s\ndoes not exist.\n", officialArchive.toLatin1().constData(), fileInfo.absoluteFilePath().toLatin1().constData());
            } else {
                fprintf(stderr, "Required archive file %s does not exist.\n", lpub3dLibFile.toLatin1().constData());
            }
            fprintf(stderr, "Launching %s in GUI mode offers a dialogue to download, and extract archive libraries.\n",lpub3dAppName.toLatin1().constData());
        }
    }
}

void Preferences::ldrawPreferences(bool browse)
{
    if (modeGUI && ! lpub3dLoaded)
        emit Application::instance()->splashMsgSig("10% - Locate LDraw directory...");

    QSettings Settings;
    ldrawLibPath = Settings.value(QString("%1/%2").arg(SETTINGS,ldrawLibPathKey)).toString();

    QDir ldrawDir(ldrawLibPath);
    if (! QFileInfo(ldrawDir.absolutePath()+validLDrawPart).exists() || browse) {      // first check

        QString returnMessage = QString();

        if (! browse ) {                                                               // second check - no browse
            ldrawLibPath.clear();
            Settings.remove(QString("%1/%2").arg(SETTINGS,ldrawLibPathKey));

            char* EnvPath = getenv("LDRAWDIR");                                       // check environment variable LDRAWDIR

            if (EnvPath && EnvPath[0]) {
                ldrawLibPath = QString(EnvPath);
                ldrawDir.setPath(ldrawLibPath);
            }


            if (portableDistribution &&
               (ldrawLibPath.isEmpty() ||
               ! QFileInfo(ldrawDir.absolutePath()+validLDrawPart).exists())){       // third check - no browse
                ldrawLibPath = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath).arg(validLDrawDir));
                ldrawDir.setPath(ldrawLibPath);
            }
        }

        if (! QFileInfo(ldrawDir.absolutePath()+validLDrawPart).exists() || browse) {  // fourth check - browse & no browse
            if (! browse) {                                                            // fourth check - no browse
                ldrawLibPath.clear();

                /* Path Checks */
                QString homePath,userLocalDataPath,userDocumentsPath;
                QStringList dataPathList;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
                dataPathList = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
                homePath = dataPathList.first();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
                dataPathList = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
#else
                dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
#endif
                userLocalDataPath = dataPathList.first();
                dataPathList = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
                userDocumentsPath = dataPathList.first();
#endif
                ldrawLibPath = QDir::toNativeSeparators(QString("%1/%2").arg(homePath).arg(validLDrawDir));

                if ( ! QFileInfo(ldrawLibPath+validLDrawPart).exists()) {     // check user documents path

                    ldrawLibPath = QDir::toNativeSeparators(QString("%1/%2").arg(userDocumentsPath).arg(validLDrawDir));

                    if ( ! QFileInfo(ldrawLibPath+validLDrawPart).exists()) { // check system data path

                        dataPathList = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

#if defined Q_OS_WIN || defined Q_OS_MAC
                        ldrawLibPath = QDir::toNativeSeparators(QString("%1/%2").arg(dataPathList.at(1)).arg(validLDrawDir)); /* C:/Users/<user>/AppData/Local/LPub3D Software/LPub3Dd/<LDraw library>,  ~/Library/Application Support/LPub3D Software/LPub3D/<LDraw library> */
#else
                        ldrawLibPath = QDir::toNativeSeparators(QString("%1/%2").arg(dataPathList.at(2)).arg(validLDrawDir)); /* ~/.local/share/LPub3D Software/lpub3d<ver_suffix>/<LDraw library>" */
#endif

                        if ( ! QFileInfo(ldrawLibPath+validLDrawPart).exists()) {     // check user data path

                            ldrawLibPath = QDir::toNativeSeparators(QString("%1/%2").arg(userLocalDataPath).arg(validLDrawDir));

                            QString message = QString("The %1 LDraw library was not found.").arg(validLDrawLibrary);

                            if ( ! QFileInfo(ldrawLibPath+validLDrawPart).exists()) { // manual prompt for LDraw Library location
                                ldrawLibPath.clear();

                                const QString searchDetail = QMessageBox::tr ("\t%1\n\t%2\n\t%3\n\t%4")
                                        .arg(portableDistribution ?
                                                 QString("%1\n\t%2")
                                                 .arg(QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath).arg(validLDrawDir)))
                                                 .arg(QDir::toNativeSeparators(QString("%1/%2").arg(homePath).arg(validLDrawDir))) :
                                                 QDir::toNativeSeparators(QString("%1/%2").arg(homePath).arg(validLDrawDir)))
                                        .arg(QDir::toNativeSeparators(QString("%1/%2").arg(userDocumentsPath).arg(validLDrawDir)))
#if defined Q_OS_WIN || defined Q_OS_MAC
                                        .arg(QDir::toNativeSeparators(QString("%1/%2").arg(dataPathList.at(1)).arg(validLDrawDir)))
#else
                                        .arg(QDir::toNativeSeparators(QString("%1/%2").arg(dataPathList.at(2)).arg(validLDrawDir)))
#endif
                                        .arg(QDir::toNativeSeparators(QString("%1/%2").arg(userLocalDataPath).arg(validLDrawDir)));

// For AppImage, Flatpak or Snap, automatically install LDraw library (user notified if install failed)
#if defined (LP3D_FLATPACK) || defined(LP3D_SNAP) || defined (LP3D_APPIMAGE)
                            if ( ! QFileInfo(ldrawLibPath+validLDrawPart).exists()) {
                                ldrawLibPath.clear();
                                if (!extractLDrawLib())
                                    message = QString("%1 LDraw library was not found and bundled archive parts failed to extract.").arg(validLDrawLibrary);
                                else
                                    message = QString("%1 LDraw library was not found but the bundled archive parts were extracted.").arg(validLDrawLibrary);
                                message += QString("\nThe following locations were searched for the LDraw library:\n%2.\n").arg(validLDrawLibrary).arg(searchDetail);
                                fprintf(stdout,"%s\n",message.toLatin1().constData());
                                fflush(stdout);
                            }
#endif

                                if (modeGUI) {
#ifdef Q_OS_MAC
                                    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isVisible())
                                        Application::instance()->splash->hide();
#endif
                                    QPixmap _icon = QPixmap(":/icons/lpub96.png");
                                    QMessageBoxResizable box;
                                    box.setWindowIcon(QIcon());
                                    box.setIconPixmap (_icon);
                                    box.setTextFormat (Qt::RichText);
                                    box.setWindowTitle(QMessageBox::tr ("LDraw Library"));
                                    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

                                    QAbstractButton* extractButton = box.addButton(QMessageBox::tr("Extract Archive"),QMessageBox::YesRole);
                                    QAbstractButton* selectButton  = box.addButton(QMessageBox::tr("Select Folder"),QMessageBox::YesRole);

                                    const QString header = "<b> " + message + "</b>";
                                    const QString body = QMessageBox::tr ("You may select your LDraw folder or extract it from the bundled %1 %2.\n"
                                                                           "Would you like to extract the library or select the LDraw folder?")
                                                                           .arg(validLDrawLibrary).arg(usingDefaultLibrary ? "archives" : "archive");
                                    const QString detail = QMessageBox::tr ("The following locations were searched for the LDraw library:\n%1\n"
                                                                            "You must select an LDraw library folder or extract the library.\n"
                                                                            "It is possible to create your library folder from the %2 file (%3) "
                                                                            "and the %4 parts archive file %5. The extracted library folder will "
                                                                            "be located at '%6'").arg(searchDetail)
                                                                            .arg(usingDefaultLibrary ? "official LDraw LEGO archive" : "LDraw " + validLDrawLibrary + " archive.")
                                                                            .arg(validLDrawPartsArchive).arg(usingDefaultLibrary ? "unofficial" : "custom")
                                                                            .arg(validLDrawCustomArchive).arg(ldrawLibPath);
                                    box.setText (header);
                                    box.setInformativeText (body);
                                    box.setDetailedText(detail);
                                    box.setStandardButtons (QMessageBox::Cancel);
                                    box.exec();

                                    if (box.clickedButton()==selectButton) {
                                        emit Application::instance()->splashMsgSig("10% - Selecting LDraw folder...");

                                        ldrawLibPath = QFileDialog::getExistingDirectory(nullptr,
                                                                                         QFileDialog::tr("Select LDraw library folder"),
                                                                                         "/",
                                                                                         QFileDialog::ShowDirsOnly |
                                                                                         QFileDialog::DontResolveSymlinks);

                                        if (! ldrawLibPath.isEmpty()) {
                                            if (checkLDrawLibrary(ldrawLibPath)) {
                                                Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawLibPathKey),ldrawLibPath);
                                            } else {
                                                ldrawLibPath.clear();
                                                returnMessage = QMessageBox::tr ("The selected path [%1] does not "
                                                                                     "appear to be a valid LDraw Library.")
                                                        .arg(ldrawLibPath);
                                            }
                                        }

                                    } else {
                                        if (box.clickedButton()==extractButton) {
                                            extractLDrawLib();
                                        } else {
                                            ldrawLibPath.clear();
                                        }
                                    }
                                } else {                  // Console mode so extract and install LDraw Library automatically if not exist in searched paths.
                                    const bool okToExtract = !message.endsWith("failed to extract.");
                                    message += QString("\nThe following locations were searched for the LDraw library:\n%2.\n").arg(searchDetail);
                                    fprintf(stdout,"%s\n",message.toLatin1().constData());
                                    if (okToExtract && extractLDrawLib()) {
                                        message = QString("The bundled %1 LDraw library archives were extracted to:\n%2\n"
                                                          "You can edit the library path in the Preferences dialogue.\n").arg(validLDrawLibrary).arg(ldrawLibPath);
                                        fprintf(stdout,"%s\n",message.toLatin1().constData());
                                    }
                                    fflush(stdout);
                                }
                            }
                        }
                    }
                }
            }

            if (/* ! ldrawLibPath.isEmpty() && */ browse && modeGUI) { // fourth check - browse
#ifdef Q_OS_MAC
                if (! lpub3dLoaded && Application::instance()->splash->isVisible())
                    Application::instance()->splash->hide();
#endif
                QString result = QFileDialog::getExistingDirectory(nullptr,
                                                                   QFileDialog::tr("Select LDraw Directory"),
                                                                   ldrawLibPath.isEmpty() ? "." : ldrawLibPath,
                                                                   QFileDialog::ShowDirsOnly |
                                                                   QFileDialog::DontResolveSymlinks);
                if (! result.isEmpty()) {

                    if (checkLDrawLibrary(result)) {
                        ldrawLibPath = QDir::toNativeSeparators(result);
                    } else {
                        returnMessage = QMessageBox::tr ("The specified path is not a valid LPub3D-supported LDraw Library.\n"
                                                          "%1").arg(ldrawLibPath);
                    }
                }
            }

            if (! ldrawLibPath.isEmpty()) { // third or fourth check successful - return

                Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawLibPathKey),ldrawLibPath);

            } else {

                if (modeGUI) {
#ifdef Q_OS_MAC
                    if (! lpub3dLoaded && Application::instance()->splash->isVisible())
                        Application::instance()->splash->hide();
#endif
                    QPixmap _icon = QPixmap(":/icons/lpub96.png");
                    QMessageBoxResizable box;
                    box.setWindowIcon(QIcon());
                    box.setIconPixmap (_icon);
                    box.setTextFormat (Qt::RichText);
                    box.setWindowTitle(QMessageBox::tr ("LDraw Directory"));
                    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

                    if (returnMessage.isEmpty())
                        returnMessage = "You must enter your LDraw library path.";
                    QString header = "<b> " + QMessageBox::tr ("No LDraw library defined!") + "</b>";
                    QString body = QMessageBox::tr ("%1\nDo you wish to continue?")
                                                    .arg(returnMessage);
                    QString detail = QMessageBox::tr ("The LDraw library is required by the LPub3D renderer(s). "
                                                      "If an LDraw library is not defined, the renderer will not "
                                                      "be able to find the parts and primitives needed to render images.");

                    box.setText (header);
                    box.setInformativeText (body);
                    box.setDetailedText(detail);
                    box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);

                    if (box.exec() != QMessageBox::Yes) {

                        exit(-1);

                    } else {
                        ldrawPreferences(false);
                    }

                } else {
                    fprintf(stderr, "No LDraw library defined! The application will terminate.\n");
                    fflush(stdout);
                    exit(-1);
                }
            } // final - retry or exit
        } else {                                 // second check successful - return
            Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawLibPathKey),ldrawLibPath);
        }
    }                                             // first check successful - return


    // Check for and set alternate LDConfig file if specified
    QString const altLDConfigPathKey("AltLDConfigPath");

    if (Settings.contains(QString("%1/%2").arg(SETTINGS,altLDConfigPathKey))) {
        altLDConfigPath = Settings.value(QString("%1/%2").arg(SETTINGS,altLDConfigPathKey)).toString();
    }

    if (! altLDConfigPath.isEmpty()) {

        QFileInfo altLDConfigFile(altLDConfigPath);

        if (altLDConfigFile.exists()) {
            return;
        } else {

            if (modeGUI) {
#ifdef Q_OS_MAC
                if (! lpub3dLoaded && Application::instance()->splash->isVisible())
                    Application::instance()->splash->hide();
#endif
                QPixmap _icon = QPixmap(":/icons/lpub96.png");
                QMessageBoxResizable box;
                box.setWindowIcon(QIcon());
                box.setIconPixmap (_icon);
                box.setTextFormat (Qt::RichText);
                box.setWindowTitle(QMessageBox::tr ("Alternate LDraw LDConfig"));
                box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

                QAbstractButton* selectButton  = box.addButton(QMessageBox::tr("Select File"),QMessageBox::YesRole);

                QString header = "<b> " + QMessageBox::tr ("Alternate LDraw LDConfig file not detected!") + "</b>";
                QString body = QMessageBox::tr ("The alternate LDraw LDConfig file:\n"
                                                "%1 does not exist.\n"
                                                "Would you like to select the alternate LDConfig file?").arg(altLDConfigFile.absoluteFilePath());;
                QString detail = QMessageBox::tr ("The alternate LDraw LDConfig file is optional.\n"
                                                  "You can use 'Select File' to select or 'Cancel' to\n"
                                                  "abandon the alternate LDraw LDConfig file.\n");
                box.setText (header);
                box.setInformativeText (body);
                box.setDetailedText(detail);
                box.setStandardButtons (QMessageBox::Cancel);
                box.exec();

                if (box.clickedButton()==selectButton) {
#ifdef Q_OS_WIN
                    QString filter(QMessageBox::tr("LDraw Files (*.ldr);;All Files (*.*)"));
#else
                    QString filter(QMessageBox::tr("All Files (*.*)"));
#endif
                    altLDConfigPath = QFileDialog::getOpenFileName(nullptr,
                                                                   QFileDialog::tr("Select LDRaw LDConfig file"),
                                                                   ldrawLibPath.isEmpty() ? "." : ldrawLibPath,
                                                                   filter);

                    if (! altLDConfigPath.isEmpty()) {

                        Settings.setValue(QString("%1/%2").arg(SETTINGS,"AltLDConfigPath"),altLDConfigPath);

                    }

                } else {
                    altLDConfigPath.clear();
                }

            } else {
                fprintf(stdout, "The alternate LDraw LDConfig file %s does not exist. Setting ignored.\n", altLDConfigFile.absoluteFilePath().toLatin1().constData());
                fflush(stdout);
            }
        }
    }

    // LSynth settings
    QString const addLSynthSearchDirKey("AddLSynthSearchDir");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,addLSynthSearchDirKey))) {
        QVariant uValue(false);
        addLSynthSearchDir = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,addLSynthSearchDirKey),uValue);
    } else {
        addLSynthSearchDir = Settings.value(QString("%1/%2").arg(SETTINGS,addLSynthSearchDirKey)).toBool();
    }

    QString const archiveLSynthPartsKey("ArchiveLSynthParts");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,archiveLSynthPartsKey))) {
        QVariant uValue(false);
        archiveLSynthParts = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,archiveLSynthPartsKey),uValue);
    } else {
        archiveLSynthParts = Settings.value(QString("%1/%2").arg(SETTINGS,archiveLSynthPartsKey)).toBool();
    }

#ifdef Q_OS_MAC
    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isHidden())
        Application::instance()->splash->show();
#endif
}

void Preferences::lpub3dUpdatePreferences(){

    emit Application::instance()->splashMsgSig("15% - Selecting update settings...");

    QSettings Settings;

    moduleVersion = qApp->applicationVersion();

    if ( ! Settings.contains(QString("%1/%2").arg(UPDATES,"CheckUpdateFrequency"))) {
        checkUpdateFrequency = UPDATE_CHECK_FREQUENCY_DEFAULT;
        Settings.setValue(QString("%1/%2").arg(UPDATES,"CheckUpdateFrequency"),checkUpdateFrequency);
    } else {
        checkUpdateFrequency = Settings.value(QString("%1/%2").arg(UPDATES,"CheckUpdateFrequency")).toInt();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(UPDATES,"ShowUpdateNotifications"))) {
        QVariant pValue(true);
        showUpdateNotifications = true;
        Settings.setValue(QString("%1/%2").arg(UPDATES,"ShowUpdateNotifications"),pValue);
    } else {
        showUpdateNotifications = Settings.value(QString("%1/%2").arg(UPDATES,"ShowUpdateNotifications")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(UPDATES,"EnableDownloader"))) {
        QVariant pValue(true);
        enableDownloader = true;
        Settings.setValue(QString("%1/%2").arg(UPDATES,"EnableDownloader"),pValue);
    } else {
        enableDownloader = Settings.value(QString("%1/%2").arg(UPDATES,"EnableDownloader")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"ShowDownloadRedirects"))) {
        QVariant uValue(true);
        showDownloadRedirects = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"ShowDownloadRedirects"),uValue);
    } else {
        showDownloadRedirects = Settings.value(QString("%1/%2").arg(SETTINGS,"ShowDownloadRedirects")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(UPDATES,"ShowAllNotifications"))) {
        QVariant pValue(true);
        showAllNotifications = true;
        Settings.setValue(QString("%1/%2").arg(UPDATES,"ShowAllNotifications"),pValue);
    } else {
        showAllNotifications = Settings.value(QString("%1/%2").arg(UPDATES,"ShowAllNotifications")).toBool();
    }
}

void Preferences::lgeoPreferences()
{
    logInfo() << QString("LGEO library status...");
    QSettings Settings;
    QString const lgeoDirKey("LGEOPath");
    QString lgeoDir = "";
    if (Settings.contains(QString("%1/%2").arg(POVRAY,lgeoDirKey))){
        lgeoDir = Settings.value(QString("%1/%2").arg(POVRAY,lgeoDirKey)).toString();
    } else { // check in ldraw directory path for lgeo
        lgeoDir = QDir::toNativeSeparators(ldrawLibPath + "/lgeo");
    }
    QDir lgeoDirInfo(lgeoDir);
    if (lgeoDirInfo.exists()) {
        lgeoPath = lgeoDir;
        logInfo() << QString("LGEO library path  : %1").arg(lgeoDirInfo.absolutePath());
        /* Durat's lgeo stl library Check */
        QDir lgeoStlLibInfo(QDir::toNativeSeparators(lgeoPath + "/stl"));
        lgeoStlLib = lgeoStlLibInfo.exists();
        if (lgeoStlLib)
            logInfo() << QString("Durat's Stl library: %1").arg(lgeoStlLibInfo.absolutePath());
    } else {
        Settings.remove(QString("%1/%2").arg(POVRAY,lgeoDirKey));
        logInfo() << QString("LGEO library path  : Not found");
        lgeoPath.clear();
    }
}

void Preferences::fadestepPreferences(bool persist)
{
    QSettings Settings;
    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"EnableFadeSteps")) || persist) {
        QVariant eValue(enableFadeSteps);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableFadeSteps"),eValue);
    } else {
        enableFadeSteps = Settings.value(QString("%1/%2").arg(SETTINGS,"EnableFadeSteps")).toBool();
    }

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"FadeStepsUseColour")) || persist) {
        QVariant eValue(fadeStepsUseColour);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepsUseColour"),eValue);
    } else {
        fadeStepsUseColour = Settings.value(QString("%1/%2").arg(SETTINGS,"FadeStepsUseColour")).toBool();
    }

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,fadeStepsColourKey)) || persist) {
        QVariant cValue(validFadeStepsColour);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,fadeStepsColourKey),cValue);
    } else {
        validFadeStepsColour = Settings.value(QString("%1/%2").arg(SETTINGS,fadeStepsColourKey)).toString();
        if (validFadeStepsColour.isEmpty()) {
            Settings.remove(QString("%1/%2").arg(SETTINGS,fadeStepsColourKey));
            validFadeStepsColour = LEGO_FADE_COLOUR_DEFAULT;
        }
    }

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"FadeStepsOpacity")) || persist) {
        QVariant cValue(fadeStepsOpacity);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepsOpacity"),cValue);
    } else {
        fadeStepsOpacity = Settings.value(QString("%1/%2").arg(SETTINGS,"FadeStepsOpacity")).toInt();
    }

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"LDrawColourPartsFile")) || persist) {
        if (! persist)
            ldrawColourPartsFile = QString("%1/extras/%2").arg(Preferences::lpubDataPath,VER_LPUB3D_COLOR_PARTS);
        QFileInfo ldrawColorFileInfo(ldrawColourPartsFile);
        if (! ldrawColorFileInfo.exists()) {
            ldrawColourPartsFile.clear();
        } else {
            QVariant cValue(ldrawColourPartsFile);
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDrawColourPartsFile"),cValue);
        }
    } else {
        ldrawColourPartsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"LDrawColourPartsFile")).toString();
        QFileInfo ldrawColorFileInfo(ldrawColourPartsFile);
        if (! ldrawColorFileInfo.exists()) {
            ldrawColourPartsFile = QString("%1/extras/%2").arg(Preferences::lpubDataPath,validLDrawColorParts);
            ldrawColorFileInfo.setFile(ldrawColourPartsFile);
            if (! ldrawColorFileInfo.exists()) {
               ldrawColourPartsFile.clear();
               Settings.remove(QString("%1/%2").arg(SETTINGS,"LDrawColourPartsFile"));
            } else {
               QVariant cValue(ldrawColourPartsFile);
               Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDrawColourPartsFile"),cValue);
            }
        }
    }

}

void Preferences::highlightstepPreferences(bool persist)
{
    QSettings Settings;
    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"EnableHighlightStep")) || persist) {
        QVariant eValue(enableHighlightStep);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableHighlightStep"),eValue);
    } else {
        enableHighlightStep = Settings.value(QString("%1/%2").arg(SETTINGS,"EnableHighlightStep")).toBool();
    }

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"HighlightStepColor")) || persist) {
        QVariant cValue(highlightStepColour);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"HighlightStepColor"),cValue);
    } else {
        highlightStepColour = Settings.value(QString("%1/%2").arg(SETTINGS,"HighlightStepColor")).toString();
        if (highlightStepColour.isEmpty()) {
            Settings.remove(QString("%1/%2").arg(SETTINGS,"HighlightStepColor"));
            highlightStepColour = HIGHLIGHT_COLOUR_DEFAULT;
        }
    }

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"HighlightStepLineWidth")) || persist) {
        QVariant uValue(highlightStepLineWidth);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"HighlightStepLineWidth"),uValue);
    } else {
        highlightStepLineWidth = Settings.value(QString("%1/%2").arg(SETTINGS,"HighlightStepLineWidth")).toInt();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"HighlightFirstStep")) || persist) {
        QVariant eValue(highlightFirstStep);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"HighlightFirstStep"),eValue);
    } else {
        highlightFirstStep = Settings.value(QString("%1/%2").arg(SETTINGS,"HighlightFirstStep")).toBool();
    }

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"LDrawColourPartsFile")) || persist) {
        if (! persist)
            ldrawColourPartsFile = QString("%1/extras/%2").arg(Preferences::lpubDataPath,VER_LPUB3D_COLOR_PARTS);
        QFileInfo ldrawColorFileInfo(ldrawColourPartsFile);
        if (! ldrawColorFileInfo.exists()) {
            ldrawColourPartsFile.clear();
        } else {
            QVariant cValue(ldrawColourPartsFile);
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDrawColourPartsFile"),cValue);
        }
    } else {
        ldrawColourPartsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"LDrawColourPartsFile")).toString();
        QFileInfo ldrawColorFileInfo(ldrawColourPartsFile);
        if (! ldrawColorFileInfo.exists()) {
            ldrawColourPartsFile = QString("%1/extras/%2").arg(Preferences::lpubDataPath,validLDrawColorParts);
            ldrawColorFileInfo.setFile(ldrawColourPartsFile);
            if (! ldrawColorFileInfo.exists()) {
               ldrawColourPartsFile.clear();
               Settings.remove(QString("%1/%2").arg(SETTINGS,"LDrawColourPartsFile"));
            } else {
               QVariant cValue(ldrawColourPartsFile);
               Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDrawColourPartsFile"),cValue);
            }
        }
    }
}

void Preferences::preferredRendererPreferences(bool persist)
{
    if (rendererMap.size() == 0) {
        rendererMap[rendererNames[RENDERER_NATIVE]]  = RENDERER_NATIVE;
        rendererMap[rendererNames[RENDERER_LDVIEW]]  = RENDERER_LDVIEW;
        rendererMap[rendererNames[RENDERER_LDGLITE]] = RENDERER_LDGLITE;
        rendererMap[rendererNames[RENDERER_POVRAY]]  = RENDERER_POVRAY;
    }

    QSettings Settings;

    /* Do we have a valid preferred renderer */

    // Get preferred renderer from Registry
    QString const preferredRendererKey("PreferredRenderer");

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,preferredRendererKey)) || persist) {
        if (! persist)
            preferredRenderer = RENDERER_NATIVE; // No persisted setting so set renderer to Native
        QVariant cValue(rendererNames[preferredRenderer]);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,preferredRendererKey),cValue);
    } else {
        preferredRenderer = rendererMap[Settings.value(QString("%1/%2").arg(SETTINGS,preferredRendererKey)).toString()];
        bool clearPreferredRenderer = false;
        if (preferredRenderer == RENDERER_LDGLITE) {
            clearPreferredRenderer = !ldgliteInstalled;
        } else if (preferredRenderer == RENDERER_LDVIEW) {
            clearPreferredRenderer = !ldviewInstalled;
        } else if (preferredRenderer == RENDERER_POVRAY) {
            clearPreferredRenderer = !povRayInstalled;
        }
        if (clearPreferredRenderer) {
            preferredRenderer = RENDERER_INVALID;
            Settings.remove(QString("%1/%2").arg(SETTINGS,preferredRendererKey));
        }
    }

    // Default projection
    QString const perspectiveProjectionKey("PerspectiveProjection");

    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,perspectiveProjectionKey)) || persist) {
        QVariant uValue(perspectiveProjection);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,perspectiveProjectionKey),uValue);
    } else {
        perspectiveProjection = Settings.value(QString("%1/%2").arg(SETTINGS,perspectiveProjectionKey)).toBool();
    }

    // LDView multiple files single call rendering
    QString const enableLDViewSingleCallKey("EnableLDViewSingleCall");

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,enableLDViewSingleCallKey)) || persist) {
        QVariant eValue(enableLDViewSingleCall);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,enableLDViewSingleCallKey),eValue);
    } else {
        enableLDViewSingleCall = Settings.value(QString("%1/%2").arg(SETTINGS,enableLDViewSingleCallKey)).toBool();
    }

    //  LDView single call snapshot list
    QString const enableLDViewSnapshotsListKey("EnableLDViewSnapshotsList");

    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,enableLDViewSnapshotsListKey)) || persist) {
        QVariant eValue(enableLDViewSnaphsotList);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,enableLDViewSnapshotsListKey),eValue);
    } else {
        enableLDViewSnaphsotList = Settings.value(QString("%1/%2").arg(SETTINGS,enableLDViewSnapshotsListKey)).toBool();
    }

    // povray generation renderer
    QString const useNativePovGeneratorKey("UseNativePovGenerator");

    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,useNativePovGeneratorKey)) || persist) {
        QVariant eValue(useNativePovGenerator);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,useNativePovGeneratorKey),eValue);
    } else {
        useNativePovGenerator = Settings.value(QString("%1/%2").arg(SETTINGS,useNativePovGeneratorKey)).toBool();
    }

    // Apply latitude and longitude camera angles locally
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"ApplyCALocally"))) {
        applyCALocally = !(perspectiveProjection && preferredRenderer == RENDERER_LDVIEW);
        QVariant uValue(applyCALocally);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"ApplyCALocally"),uValue);
    } else {
        applyCALocally = Settings.value(QString("%1/%2").arg(SETTINGS,"ApplyCALocally")).toBool();
    }

    // set LDView ini
    if (Preferences::preferredRenderer == RENDERER_POVRAY) {
        if (Preferences::useNativePovGenerator)
            TCUserDefaults::setIniFile(Preferences::nativeExportIni.toLatin1().constData());
        else
            TCUserDefaults::setIniFile(Preferences::ldviewPOVIni.toLatin1().constData());
    } else if (Preferences::preferredRenderer == RENDERER_LDVIEW) {
        TCUserDefaults::setIniFile(Preferences::ldviewIni.toLatin1().constData());
    }

    // Inline Native renderer prject content
    QString const inlineNativeContentKey("InlineNativeContent");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,inlineNativeContentKey))) {
        inlineNativeContent = true;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,inlineNativeContentKey),inlineNativeContent);
    } else {
        inlineNativeContent = Settings.value(QString("%1/%2").arg(SETTINGS,inlineNativeContentKey)).toBool();
    }
}

void Preferences::rendererPreferences()
{
    QSettings Settings;

    /* Set 3rdParty application locations */

    logInfo() << QString("Image renderers...");
#ifdef Q_OS_WIN
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,"RendererApplicationDir"))) {
        lpub3d3rdPartyAppDir = Settings.value(QString("%1/%2").arg(SETTINGS,"RendererApplicationDir")).toString();;
    } else {
        lpub3d3rdPartyAppDir = QString("%1/3rdParty").arg(lpub3dPath);
    }

    QFileInfo ldgliteInfo(QString("%1/%2/bin/ldglite.exe").arg(lpub3d3rdPartyAppDir, VER_LDGLITE_STR));
#if defined __i386__ || defined _M_IX86
    QFileInfo ldviewInfo(QString("%1/%2/bin/LDView.exe").arg(lpub3d3rdPartyAppDir, VER_LDVIEW_STR));
    QFileInfo povrayInfo(QString("%1/%2/bin/lpub3d_trace_cui32.exe").arg(lpub3d3rdPartyAppDir, VER_POVRAY_STR));
#elif defined __x86_64__ || defined _M_X64
    QFileInfo ldviewInfo(QString("%1/%2/bin/LDView64.exe").arg(lpub3d3rdPartyAppDir, VER_LDVIEW_STR));
    QFileInfo povrayInfo(QString("%1/%2/bin/lpub3d_trace_cui64.exe").arg(lpub3d3rdPartyAppDir, VER_POVRAY_STR));
#endif
#elif defined Q_OS_MAC
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,"RendererApplicationDir"))) {
        lpub3d3rdPartyAppDir = Settings.value(QString("%1/%2").arg(SETTINGS,"RendererApplicationDir")).toString();;
    } else {
        lpub3d3rdPartyAppDir = QString("%1/%2.app/Contents/3rdParty").arg(lpub3dPath).arg(lpub3dAppName);
    }

    QFileInfo ldgliteInfo(QString("%1/%2/bin/ldglite").arg(lpub3d3rdPartyAppDir, VER_LDGLITE_STR));
    QFileInfo ldviewInfo(QString("%1/%2/bin/LDView").arg(lpub3d3rdPartyAppDir, VER_LDVIEW_STR));
    QFileInfo povrayInfo(QString("%1/%2/bin/lpub3d_trace_cui").arg(lpub3d3rdPartyAppDir, VER_POVRAY_STR));
#else // Q_OS_LINUX
    QDir appDir;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,"RendererApplicationDir"))) {
        lpub3d3rdPartyAppDir = Settings.value(QString("%1/%2").arg(SETTINGS,"RendererApplicationDir")).toString();;
    } else {
        appDir.setPath(QString("%1/../share").arg(lpub3dPath));
        lpub3d3rdPartyAppDir = QString("%1/%2/3rdParty").arg(appDir.absolutePath(), lpub3dAppName);
    }

    if (Settings.contains(QString("%1/%2").arg(SETTINGS,"RendererExecutableDir"))) {
        lpub3d3rdPartyAppExeDir = Settings.value(QString("%1/%2").arg(SETTINGS,"RendererExecutableDir")).toString();;
    } else {
        appDir.setPath(QString("%1/../../%2").arg(lpub3dPath).arg(optPrefix.isEmpty() ? "opt" : optPrefix+"/opt"));
        lpub3d3rdPartyAppExeDir = QString("%1/%2/3rdParty").arg(appDir.absolutePath(), lpub3dAppName);
    }

    QFileInfo ldgliteInfo(QString("%1/%2/bin/ldglite").arg(lpub3d3rdPartyAppExeDir, VER_LDGLITE_STR));
    QFileInfo ldviewInfo(QString("%1/%2/bin/ldview").arg(lpub3d3rdPartyAppExeDir, VER_LDVIEW_STR));
    QFileInfo povrayInfo(QString("%1/%2/bin/lpub3d_trace_cui").arg(lpub3d3rdPartyAppExeDir, VER_POVRAY_STR));
#endif

    /* 3rd Party application installation status */

    // LDGLite EXE
    if (ldgliteInfo.exists()) {
        ldgliteInstalled = true;
        ldgliteExe = QDir::toNativeSeparators(ldgliteInfo.absoluteFilePath());
        logInfo() << QString("LDGLite : %1").arg(ldgliteExe);
    } else {
        logNotice() << QString("LDGLite : %1 not installed").arg(ldgliteInfo.absoluteFilePath());
    }

#ifdef Q_OS_MAC
    emit Application::instance()->splashMsgSig(QString("25% - %1 macOS Required Library Check...").arg(VER_PRODUCTNAME_STR));
    QStringList missingLibs;
    QPixmap _icon = QPixmap(":/icons/lpub96.png");
    QMessageBoxResizable box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (_icon);
    box.setTextFormat (Qt::RichText);
    box.setWindowTitle(QMessageBox::tr ("Missing Libraries"));
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    box.setStandardButtons (QMessageBox::Close);
#endif

    if (ldviewInfo.exists()) {
        ldviewInstalled = true;
        ldviewExe = QDir::toNativeSeparators(ldviewInfo.absoluteFilePath());
        logInfo() << QString("LDView  : %1").arg(ldviewExe);

#ifdef Q_OS_MAC
// Check macOS LDView Libraries

        if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"LDViewMissingLibs"))) {
            missingRendererLibs = true;
            QVariant eValue(missingRendererLibs);
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDViewMissingLibs"),eValue);
        } else {
            missingRendererLibs = Settings.value(QString("%1/%2").arg(SETTINGS,"LDViewMissingLibs")).toBool();
        }

        if (missingRendererLibs) {
            missingLibs.clear();
            QFileInfo libInfo("/opt/X11/lib/libOSMesa.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                QString xquartz = "xquartz";
                if (!validLib(xquartz, LIBXQUARTZ_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/libpng/lib/libpng.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                if (!validLib(libInfo.completeBaseName(), LIBPNG_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/gl2ps/lib/libgl2ps.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                if (!validLib(libInfo.completeBaseName().replace("lib",""), LIBGL2PS_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/jpeg/lib/libjpeg.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                if (!validLib(libInfo.completeBaseName().replace("lib",""), LIBJPEG_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/tinyxml/lib/libtinyxml.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                if (!validLib(libInfo.completeBaseName().replace("lib",""), LIBXML_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/minizip/lib/libminizip.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                if (!validLib(libInfo.completeBaseName().replace("lib",""), LIBMINIZIP_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }

            missingRendererLibs = missingLibs.size() > 0;
            QVariant eValue(missingRendererLibs);
            if (!missingRendererLibs) {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDViewMissingLibs"),eValue);
            }
            else
            {
                QString header = QMessageBox::tr ("<b>Required libraries were not found!</b>");
                QString body = QMessageBox::tr ("The following LDView libraries were not found:%2%2-%1%2%2"
                                                "See /Applications/LPub3D.app/Contents/Resources/README_macOS.txt for details.")
                                                .arg(missingLibs.join("\n -").arg(modeGUI ? "<br>" : "\n"));
                box.setText (header);
                box.setInformativeText (body);

                if (modeGUI) {
                    if (! lpub3dLoaded && Application::instance()->splash->isVisible())
                        Application::instance()->splash->hide();
                    if (box.exec() == QMessageBox::Close) {
                        if (! lpub3dLoaded && Application::instance()->splash->isHidden())
                            Application::instance()->splash->show();
                    }
                } else {
                    fprintf(stdout,"%s\n",body.toLatin1().constData());
                    fflush(stdout);

                    logDebug() << QString("LDView Missing Libs: %1").arg(missingLibs.join("\n -"));
                }
            }
        }
#endif
    } else {
        logNotice() << QString("LDView  : %1 not installed").arg(ldviewInfo.absoluteFilePath());
    }

    if (povrayInfo.exists()) {
        povRayInstalled = true;
        povrayExe = QDir::toNativeSeparators(povrayInfo.absoluteFilePath());
        logInfo() << QString("POVRay  : %1").arg(povrayExe);

#ifdef Q_OS_MAC
// Check POVRay libraries on macOS

        if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"POVRayMissingLibs"))) {
          missingRendererLibs = true;
          QVariant eValue(missingRendererLibs);
          Settings.setValue(QString("%1/%2").arg(SETTINGS,"POVRayMissingLibs"),eValue);
        } else {
          missingRendererLibs = Settings.value(QString("%1/%2").arg(SETTINGS,"POVRayMissingLibs")).toBool();
        }

        if (missingRendererLibs) {
            missingLibs.clear();
            QFileInfo libInfo("/opt/X11/lib/libX11.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                QString xquartz = "xquartz";
                if (!validLib(xquartz, LIBXQUARTZ_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/libtiff/lib/libtiff.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                if (!validLib(libInfo.completeBaseName(), LIBTIFF_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/openexr/lib/libIlmImf.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                QString openexr = "openexr";
                if (!validLib(openexr, LIBOPENEXR_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/ilmbase/lib/libHalf.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                QString ilmbase = "ilmbase";
                if (!validLib(ilmbase, LIBILMBASE_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/sdl2/lib/libSDL2.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                if (!validLib(libInfo.completeBaseName().replace("lib","").toLower(), LIBSDL_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }

            missingRendererLibs = missingLibs.size() > 0;
            QVariant eValue(missingRendererLibs);
            if (!missingRendererLibs) {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"POVRayMissingLibs"),eValue);
            }
            else
            {
              QString header = QMessageBox::tr ("<b>Required libraries were not found!</b>");
              QString body = QMessageBox::tr ("The following required POVRay libraries were not found:%2%2-%1%2%2 "
                                              "See /Applications/LPub3D.app/Contents/Resources/README_macOS.txt for details.")
                                              .arg(missingLibs.join("\n -").arg(modeGUI ? "<br>" : "\n"));
              box.setText (header);
              box.setInformativeText (body);

              if (modeGUI) {
                if (!lpub3dLoaded && Application::instance()->splash->isVisible())
                  Application::instance()->splash->hide();
                if (box.exec() == QMessageBox::Close) {
                    if (! lpub3dLoaded && Application::instance()->splash->isHidden())
                      Application::instance()->splash->show();
                }
              } else {
                QString message = body.replace("\n", " ");
                fprintf(stdout,"%s\n",message.toLatin1().constData());
                fflush(stdout);

                logDebug() << QString("POVRay Missing Libraries: %1").arg(missingLibs.join("\n -"));
              }
           }
        }
        emit Application::instance()->splashMsgSig(QString("25% - %1 window defaults loading...").arg(VER_PRODUCTNAME_STR));
#endif
    } else {
        logNotice() << QString("POVRay  : %1 not installed").arg(povrayInfo.absoluteFilePath());
    }

    // Set valid preferred renderer preferences
    preferredRendererPreferences();

    // Renderer timeout
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"RendererTimeout"))) {
        rendererTimeout = RENDERER_TIMEOUT_DEFAULT;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"RendererTimeout"),rendererTimeout);
    } else {
        rendererTimeout = Settings.value(QString("%1/%2").arg(SETTINGS,"RendererTimeout")).toInt();
    }

    // Native camera fov adjustment for image generation
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"NativeImageCameraFoVAdjust"))) {
        nativeImageCameraFoVAdjust = NATIVE_IMAGE_CAMERA_FOV_ADJUST;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"NativeImageCameraFoVAdjust"),rendererTimeout);
    } else {
        nativeImageCameraFoVAdjust = Settings.value(QString("%1/%2").arg(SETTINGS,"NativeImageCameraFoVAdjust")).toInt();
    }

    // Image matting [future use]
    QString const enableImageMattingKey("EnableImageMatting");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,enableImageMattingKey))) {
        QVariant uValue(enableImageMatting);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,enableImageMattingKey),uValue);
    } else {
        enableImageMatting = Settings.value(QString("%1/%2").arg(SETTINGS,enableImageMattingKey)).toBool();
    }

    // Write config files
    logInfo() << QString("Processing renderer configuration files...");

    lpub3d3rdPartyConfigDir = QString("%1/3rdParty").arg(lpubDataPath);
    lpub3dLDVConfigDir      = QString("%1/ldv").arg(lpubDataPath);

    setLDGLiteIniParams();
    updateLDVExportIniFile(SkipExisting);
    updateLDViewIniFile(SkipExisting);
    updateLDViewPOVIniFile(SkipExisting);
    updatePOVRayConfFile(SkipExisting);
    updatePOVRayIniFile(SkipExisting);

    // Blender config files
    QString const blenderConfigDir = QString("%1/Blender/config").arg(lpub3d3rdPartyConfigDir);

    // Individual render config file
    QString const blenderRenderConfigFileKey("BlenderConfigFile");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,blenderRenderConfigFileKey))) {
        blenderRenderConfigFile = QString("%1/%2").arg(blenderConfigDir,VER_BLENDER_RENDER_CONFIG_FILE);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,blenderRenderConfigFileKey),QVariant(blenderRenderConfigFile));
    } else {
        blenderRenderConfigFile = Settings.value(QString("%1/%2").arg(SETTINGS,blenderRenderConfigFileKey)).toString();
        if (!QFileInfo(blenderRenderConfigFile).exists()) {
            blenderRenderConfigFile =  QString("%1/%2").arg(blenderConfigDir,VER_BLENDER_RENDER_CONFIG_FILE);
            Settings.setValue(QString("%1/%2").arg(SETTINGS,blenderRenderConfigFileKey),QVariant(blenderRenderConfigFile));
        }
    }

    // Document render config file
    QString const blenderDocumentConfigFileKey("BlenderConfigFile");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,blenderDocumentConfigFileKey))) {
        blenderDocumentConfigFile = QString("%1/%2").arg(blenderConfigDir,VER_BLENDER_DOCUMENT_CONFIG_FILE);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,blenderDocumentConfigFileKey),QVariant(blenderDocumentConfigFile));
    } else {
        blenderDocumentConfigFile = Settings.value(QString("%1/%2").arg(SETTINGS,blenderDocumentConfigFileKey)).toString();
        if (!QFileInfo(blenderDocumentConfigFile).exists()) {
            blenderDocumentConfigFile =  QString("%1/%2").arg(blenderConfigDir,VER_BLENDER_DOCUMENT_CONFIG_FILE);
            Settings.setValue(QString("%1/%2").arg(SETTINGS,blenderDocumentConfigFileKey),QVariant(blenderDocumentConfigFile));
        }
    }

    // Blender executable
    QString const blenderExeKey("BlenderExeFile");
    blenderExe = Settings.value(QString("%1/%2").arg(SETTINGS,blenderExeKey)).toString();
    QFileInfo blenderFileInfo(blenderExe);
    if (blenderFileInfo.exists()) {
        blenderInstalled = true;
        blenderExe = QDir::toNativeSeparators(blenderFileInfo.absoluteFilePath());
        logInfo() << QString("Blender : %1").arg(blenderExe);
        QFileInfo blendFileInfo(QString("%1/%2").arg(blenderConfigDir).arg(VER_BLENDER_DEFAULT_BLEND_FILE));
        if ((defaultBlendFile = blendFileInfo.exists())) {
            logInfo() << QString("Blendfile (default): %1").arg(blendFileInfo.absoluteFilePath());
        } else {
            logNotice() << QString("Default blendfile does not exist: %1").arg(blendFileInfo.absoluteFilePath());
        }
    } else {
        if (!blenderExe.isEmpty())
            logNotice() << QString("Blender : %1 not installed").arg(blenderExe);
        Settings.remove(QString("%1/%2").arg(SETTINGS,blenderExeKey));
        blenderExe = QString();
    }

    // Blender version
    QString const blenderVersionKey("BlenderVersion");
    if (blenderInstalled){
        if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,blenderVersionKey))) {
            blenderVersion = "2.81";
            Settings.setValue(QString("%1/%2").arg(SETTINGS,blenderVersionKey),QVariant(blenderVersion));
        } else {
            blenderVersion = Settings.value(QString("%1/%2").arg(SETTINGS,blenderVersionKey)).toString();
        }
    } else if (Settings.contains(QString("%1/%2").arg(SETTINGS,blenderVersionKey))) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,blenderExeKey));
        blenderVersion = QString();
    }

    // Populate POVRay Library paths
    if (!povRayInstalled)
        return;

    // Display povray image during rendering [experimental]
    QString const povrayDisplayKey("POVRayDisplay");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,povrayDisplayKey))) {
        QVariant uValue(false);
        povrayDisplay = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,povrayDisplayKey),uValue);
    } else {
        povrayDisplay = Settings.value(QString("%1/%2").arg(SETTINGS,povrayDisplayKey)).toBool();
    }

    // Set POV-Ray render quality
    QString const povrayRenderQualityKey("PovrayRenderQuality");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,povrayRenderQualityKey))) {
        povrayRenderQuality = POVRAY_RENDER_QUALITY_DEFAULT;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,povrayRenderQualityKey),povrayRenderQuality);
    } else {
        povrayRenderQuality = Settings.value(QString("%1/%2").arg(SETTINGS,povrayRenderQualityKey)).toInt();
    }

    // Automatically crop POV-Ray rendered images
    QString const povrayAutoCropKey("PovrayAutoCrop");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,povrayAutoCropKey))) {
        QVariant uValue(true);
        povrayAutoCrop = true;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,povrayAutoCropKey),uValue);
    } else {
        povrayAutoCrop = Settings.value(QString("%1/%2").arg(SETTINGS,povrayAutoCropKey)).toBool();
    }

    QFileInfo resourceFile;
    resourceFile.setFile(QString("%1/%2/resources/ini/%3").arg(lpub3d3rdPartyAppDir, VER_POVRAY_STR, VER_POVRAY_INI_FILE));
    if (resourceFile.exists())
        povrayIniPath = resourceFile.absolutePath();
    logInfo() << QString("POVRay ini path    : %1").arg(povrayIniPath.isEmpty() ? "Not found" : povrayIniPath);

    resourceFile.setFile(QString("%1/%2/resources/include/%3").arg(lpub3d3rdPartyAppDir, VER_POVRAY_STR, VER_POVRAY_INC_FILE));
    if (resourceFile.exists())
        povrayIncPath = resourceFile.absolutePath();
    logInfo() << QString("POVRay include path: %1").arg(povrayIncPath.isEmpty() ? "Not found" : povrayIncPath);

    resourceFile.setFile(QString("%1/%2/resources/scenes/%3").arg(lpub3d3rdPartyAppDir, VER_POVRAY_STR, VER_POVRAY_SCENE_FILE));
    if (resourceFile.exists())
        povrayScenePath = resourceFile.absolutePath();
    logInfo() << QString("POVRay scene path  : %1").arg(povrayScenePath.isEmpty() ? "Not found" : povrayScenePath);

    logInfo() << QString("Renderer is %1%2.")
                         .arg(rendererNames[preferredRenderer])
                         .arg(preferredRenderer == RENDERER_POVRAY ? QString(" (POV file generator is %1)")
                                                                             .arg(useNativePovGenerator ? rendererNames[RENDERER_NATIVE] : rendererNames[RENDERER_LDVIEW]) :
                              preferredRenderer == RENDERER_LDVIEW ? enableLDViewSingleCall ?
                                                                     enableLDViewSnaphsotList ? QString(" (Single Call using Export File List)") :
                                                                                                QString(" (Single Call)") :
                                                                                                QString() : QString());

    // default camera preferences;
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"AssemblyCameraLatitude"))) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"AssemblyCameraLatitude"),assemblyCameraLatitude);
    } else {
        assemblyCameraLatitude = Settings.value(QString("%1/%2").arg(SETTINGS,"AssemblyCameraLatitude")).toInt();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"AssemblyCameraLongitude"))) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"AssemblyCameraLongitude"),assemblyCameraLongitude);
    } else {
        assemblyCameraLongitude = Settings.value(QString("%1/%2").arg(SETTINGS,"AssemblyCameraLongitude")).toInt();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"PartCameraLatitude"))) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"PartCameraLatitude"),partCameraLatitude);
    } else {
        partCameraLatitude = Settings.value(QString("%1/%2").arg(SETTINGS,"PartCameraLatitude")).toInt();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"PartCameraLongitude"))) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"PartCameraLongitude"),partCameraLongitude);
    } else {
        partCameraLongitude = Settings.value(QString("%1/%2").arg(SETTINGS,"PartCameraLongitude")).toInt();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"SubmodelCameraLatitude"))) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"SubmodelCameraLatitude"),submodelCameraLatitude);
    } else {
        submodelCameraLatitude = Settings.value(QString("%1/%2").arg(SETTINGS,"SubmodelCameraLatitude")).toInt();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"SubmodelCameraLongitude"))) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"SubmodelCameraLongitude"),submodelCameraLongitude);
    } else {
        submodelCameraLongitude = Settings.value(QString("%1/%2").arg(SETTINGS,"SubmodelCameraLongitude")).toInt();
    }
}

void Preferences::setLDGLiteIniParams()
{
    if (!ldgliteInstalled)
        return;

    QString inFileName;
    QFileInfo resourceFile;
    QFile confFileIn, confFileOut;

    resourceFile.setFile(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDGLITE_STR, VER_LDGLITE_INI_FILE));
    if (!resourceFile.exists()) {
        logInfo() << QString("Initializing %1...").arg(resourceFile.absoluteFilePath());
        inFileName = QString("%1/%2").arg(dataLocation, resourceFile.fileName());
        if (!resourceFile.absoluteDir().exists())
            resourceFile.absoluteDir().mkpath(".");
        confFileIn.setFileName(QDir::toNativeSeparators(inFileName));;
        confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDGLITE_STR, resourceFile.fileName()));
        if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly | QIODevice::Text)){
            QTextStream input(&confFileIn);
            QTextStream output(&confFileOut);
            while (!input.atEnd())
            {
                QString line = input.readLine();
                // Remove Template note from used instance
                if (line.contains(QRegExp("^__NOTE:"))){
                    continue;
                }
                output << line << lpub_endl;
            }
            confFileIn.close();
            confFileOut.close();
        } else {
            QString confFileError;
            if (!confFileIn.errorString().isEmpty())
                confFileError.append(QString(" confFileInError: %1").arg(confFileIn.errorString()));
            if (!confFileOut.errorString().isEmpty())
                confFileError.append(QString(" confFileOutError: %1").arg(confFileOut.errorString()));
            logError() << QString("Could not open ldglite.ini input or output file: %1").arg(confFileError);
        }
    }
    confFileIn.setFileName(resourceFile.absoluteFilePath());
    if (confFileIn.open(QIODevice::ReadOnly))
    {
        ldgliteParms.clear();
        QTextStream input(&confFileIn);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            if (line.contains(QRegExp("^-.*")))
            {
                //logDebug() << QString("Line PARAM: %1").arg(line);
                ldgliteParms << line;
            }
        }
        confFileIn.close();
    } else {
        QString confFileError;
        if (!confFileIn.errorString().isEmpty())
            confFileError.append(QString(" confFileInError: %1").arg(confFileIn.errorString()));
        logError() << QString("Could not open ldglite.ini input file: %1").arg(confFileError);
    }
    if (preferredRenderer == RENDERER_LDGLITE)
        logInfo() << QString("LDGLite Parameters :%1").arg((ldgliteParms.isEmpty() ? "No parameters" : ldgliteParms.join(" ")));
    if (resourceFile.exists())
        ldgliteIni = resourceFile.absoluteFilePath(); // populate ldglite ini file
    logInfo() << QString("LDGLite.ini file   : %1").arg(ldgliteIni.isEmpty() ? "Not found" : ldgliteIni);
}

void Preferences::updateLDVExportIniFile(UpdateFlag updateFlag)
{
    QString inFileName;
    QFileInfo resourceFile;
    QFile confFileIn, confFileOut, oldFile;
    QDateTime timeStamp = QDateTime::currentDateTime();

    resourceFile.setFile(QString("%1/%2").arg(lpub3dLDVConfigDir, VER_NATIVE_EXPORT_INI_FILE));
    if (resourceFile.exists())
    {
        if (updateFlag == SkipExisting) {
           nativeExportIni = resourceFile.absoluteFilePath(); // populate Native Export file file
           logInfo() << QString("Native Export file : %1").arg(nativeExportIni);
           return;
        }
        logInfo() << QString("Updating %1...").arg(resourceFile.absoluteFilePath());
        inFileName = QString("%1.%2").arg(resourceFile.absoluteFilePath(),timeStamp.toString("ddMMyyhhmmss"));
        oldFile.setFileName(resourceFile.absoluteFilePath());
        oldFile.rename(inFileName);
    } else {
       logInfo() << QString("Initializing %1...").arg(resourceFile.absoluteFilePath());
       inFileName = QString("%1/%2").arg(dataLocation, resourceFile.fileName());
       if (!resourceFile.absoluteDir().exists())
           resourceFile.absoluteDir().mkpath(".");
    }
    confFileIn.setFileName(QDir::toNativeSeparators(inFileName));
    confFileOut.setFileName(QString("%1/%2").arg(lpub3dLDVConfigDir, resourceFile.fileName()));
    if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream input(&confFileIn);
        QTextStream output(&confFileOut);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            // Remove Template note from used instance
            if (line.contains(QRegExp("^__NOTE:"))){
                continue;
            }
            // strip EdgeThickness because set in renderer parameters
            if (line.contains(QRegExp("^EdgeThickness="))){
                continue;
            }
            //logDebug() << QString("Line INPUT: %1").arg(line);
            // set ldraw dir
            if (line.contains(QRegExp("^LDrawDir=")))
            {
                line.clear();
                line = QString("LDrawDir=%1").arg(QDir::toNativeSeparators(ldrawLibPath));
            }
            // set lgeo paths as required
            if (line.contains(QRegExp("^XmlMapPath=")))
            {
                line.clear();
                if (lgeoPath.isEmpty())
                {
                    line = QString("XmlMapPath=");
                } else {
                    line = QString("XmlMapPath=%1").arg(QDir::toNativeSeparators(QString("%1/%2").arg(lgeoPath).arg(VER_LGEO_XML_FILE)));
                }
            }
            logInfo() << QString("NativePOV.ini OUT: %1").arg(line);
            output << line << lpub_endl;
        }
        confFileIn.close();
        confFileOut.close();
    } else {
        QString confFileError;
        if (!confFileIn.errorString().isEmpty())
            confFileError.append(QString(" confFileInError: %1\n").arg(confFileIn.errorString()));
        if (!confFileOut.errorString().isEmpty())
            confFileError.append(QString(" confFileOutError: %1").arg(confFileOut.errorString()));
        logError() << QString("Could not open NativePOV.ini input or output file: %1").arg(confFileError);
    }
    if (resourceFile.exists())
        nativeExportIni = resourceFile.absoluteFilePath(); // populate native POV ini file
    if (oldFile.exists())
        oldFile.remove();                               // delete old file
    logInfo() << QString("NativePOV ini file : %1").arg(nativeExportIni.isEmpty() ? "Not found" : nativeExportIni);
}

void Preferences::updateLDViewIniFile(UpdateFlag updateFlag)
{
    if (!ldviewInstalled)
        return;

    QString inFileName;
    QFileInfo resourceFile;
    QFile confFileIn, confFileOut, oldFile;
    QDateTime timeStamp = QDateTime::currentDateTime();

    resourceFile.setFile(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDVIEW_STR, VER_LDVIEW_INI_FILE));
    if (resourceFile.exists())
    {
       if (updateFlag == SkipExisting) {
           ldviewIni = QDir::toNativeSeparators(resourceFile.absoluteFilePath()); // populate ldview ini file
           logInfo() << QString("LDView ini file    : %1").arg(ldviewIni);
           return;
       }
       logInfo() << QString("Updating %1...").arg(resourceFile.absoluteFilePath());
       inFileName = QString("%1.%2").arg(resourceFile.absoluteFilePath(),timeStamp.toString("ddMMyyhhmmss"));
       oldFile.setFileName(resourceFile.absoluteFilePath());
       oldFile.rename(inFileName);
    } else {
       logInfo() << QString("Initializing %1...").arg(resourceFile.absoluteFilePath());
       inFileName = QString("%1/%2/resources/config/%3").arg(lpub3d3rdPartyAppDir, VER_LDVIEW_STR, resourceFile.fileName());
       if (!resourceFile.absoluteDir().exists())
           resourceFile.absoluteDir().mkpath(".");
    }
    confFileIn.setFileName(QDir::toNativeSeparators(inFileName));
    confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDVIEW_STR, resourceFile.fileName()));
    if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream input(&confFileIn);
        QTextStream output(&confFileOut);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            // strip EdgeThickness because set in renderer parameters
            if (line.contains(QRegExp("^EdgeThickness="))){
                continue;
            }
            //logDebug() << QString("Line INPUT: %1").arg(line);
            // set ldraw dir
            if (line.contains(QRegExp("^LDrawDir=")))
            {
                line.clear();
                line = QString("LDrawDir=%1").arg(QDir::toNativeSeparators(ldrawLibPath));
            }
            // set AutoCrop=0
//            if (line.contains(QRegExp("^AutoCrop="))) {
//                line.clear();
//                line = QString("AutoCrop=%1").arg((enableFadeSteps && enableImageMatting) ? 0 : 1);
//            }
            logInfo() << QString("LDView.ini OUT: %1").arg(line);
            output << line << lpub_endl;
        }
        confFileIn.close();
        confFileOut.close();
    } else {
        QString confFileError;
        if (!confFileIn.errorString().isEmpty())
            confFileError.append(QString(" confFileInError: %1\n").arg(confFileIn.errorString()));
        if (!confFileOut.errorString().isEmpty())
            confFileError.append(QString(" confFileOutError: %1").arg(confFileOut.errorString()));
        logError() << QString("Could not open LDView.ini input or output file: %1").arg(confFileError);
    }
    if (resourceFile.exists())
        ldviewIni = QDir::toNativeSeparators(resourceFile.absoluteFilePath()); // populate ldview ini file
    if (oldFile.exists())
        oldFile.remove();                            // delete old file
    logInfo() << QString("LDView ini file    : %1").arg(ldviewIni.isEmpty() ? "Not found" : ldviewIni);
}

void Preferences::updateLDViewPOVIniFile(UpdateFlag updateFlag)
{
     if (!ldviewInstalled || !povRayInstalled)
         return;

    QString inFileName;
    QFileInfo resourceFile;
    QFile confFileIn, confFileOut, oldFile;
    QDateTime timeStamp = QDateTime::currentDateTime();

    resourceFile.setFile(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDVIEW_STR, VER_LDVIEW_POV_INI_FILE));
    if (resourceFile.exists())
    {
        if (updateFlag == SkipExisting) {
           ldviewPOVIni = resourceFile.absoluteFilePath(); // populate ldview POV ini file
           logInfo() << QString("LDViewPOV ini file : %1").arg(ldviewPOVIni);
           return;
        }
        logInfo() << QString("Updating %1...").arg(resourceFile.absoluteFilePath());
        inFileName = QString("%1.%2").arg(resourceFile.absoluteFilePath(),timeStamp.toString("ddMMyyhhmmss"));
        oldFile.setFileName(resourceFile.absoluteFilePath());
        oldFile.rename(inFileName);
    } else {
       logInfo() << QString("Initializing %1...").arg(resourceFile.absoluteFilePath());
       inFileName = QString("%1/%2/resources/config/%3").arg(lpub3d3rdPartyAppDir, VER_LDVIEW_STR, resourceFile.fileName());
       if (!resourceFile.absoluteDir().exists())
           resourceFile.absoluteDir().mkpath(".");
    }
    confFileIn.setFileName(QDir::toNativeSeparators(inFileName));
    confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDVIEW_STR, resourceFile.fileName()));
    if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream input(&confFileIn);
        QTextStream output(&confFileOut);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            // strip EdgeThickness because set in renderer parameters
            if (line.contains(QRegExp("^EdgeThickness="))){
              continue;
            }
            //logDebug() << QString("Line INPUT: %1").arg(line);
            // set ldraw dir
            if (line.contains(QRegExp("^LDrawDir=")))
            {
                line.clear();
                line = QString("LDrawDir=%1").arg(QDir::toNativeSeparators(ldrawLibPath));
            }
            // set lgeo paths as required
            if (line.contains(QRegExp("^XmlMapPath=")))
            {
                line.clear();
                if (lgeoPath.isEmpty())
                {
                    line = QString("XmlMapPath=");
                } else {
                    line = QString("XmlMapPath=%1").arg(QDir::toNativeSeparators(QString("%1/%2").arg(lgeoPath).arg(VER_LGEO_XML_FILE)));
                }
            }
            logInfo() << QString("LDViewPOV.ini OUT: %1").arg(line);
            output << line << lpub_endl;
        }
        confFileIn.close();
        confFileOut.close();
    } else {
        QString confFileError;
        if (!confFileIn.errorString().isEmpty())
            confFileError.append(QString(" confFileInError: %1\n").arg(confFileIn.errorString()));
        if (!confFileOut.errorString().isEmpty())
            confFileError.append(QString(" confFileOutError: %1").arg(confFileOut.errorString()));
        logError() << QString("Could not open LDViewPOV.ini input or output file: %1").arg(confFileError);
    }
    if (resourceFile.exists())
        ldviewPOVIni = resourceFile.absoluteFilePath(); // populate ldview POV ini file
    if (oldFile.exists())
        oldFile.remove();                               // delete old file
    logInfo() << QString("LDViewPOV ini file : %1").arg(ldviewPOVIni.isEmpty() ? "Not found" : ldviewPOVIni);
}

void Preferences::updatePOVRayConfFile(UpdateFlag updateFlag)
{
    if (!povRayInstalled)
        return;

    QString inFileName;
    QFileInfo resourceFile;
    QFile confFileIn, confFileOut, oldFile;
    QDateTime timeStamp = QDateTime::currentDateTime();

    // POV-Ray Conf
    resourceFile.setFile(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_POVRAY_STR ,VER_POVRAY_CONF_FILE));
    if (resourceFile.exists())
    {
        if (updateFlag == SkipExisting) {
           povrayConf = resourceFile.absoluteFilePath();  // populate povray conf file
           logInfo() << QString("POVRay conf file   : %1").arg(povrayConf);
           updatePOVRayConfigFiles();
           return;
        }
        logInfo() << QString("Updating %1...").arg(resourceFile.absoluteFilePath());
        inFileName = QString("%1.%2").arg(resourceFile.absoluteFilePath(),timeStamp.toString("ddMMyyhhmmss"));
        oldFile.setFileName(resourceFile.absoluteFilePath());
        oldFile.rename(inFileName);
    } else {
       logInfo() << QString("Initializing %1...").arg(resourceFile.absoluteFilePath());
       inFileName = QString("%1/%2/resources/config/%3").arg(lpub3d3rdPartyAppDir, VER_POVRAY_STR, resourceFile.fileName());
       if (!resourceFile.absoluteDir().exists())
           resourceFile.absoluteDir().mkpath(".");
    }
    confFileIn.setFileName(QDir::toNativeSeparators(inFileName));
    confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_POVRAY_STR, resourceFile.fileName()));
    if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream input(&confFileIn);
        QTextStream output(&confFileOut);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            //logDebug() << QString("Line INPUT: %1").arg(line);
            if (line.contains(QRegExp("^read* =")) && oldFile.exists())
            {
                if (lgeoPath != "")
                {
                    if (line.contains(QRegExp("[\\/|\\\\]ar")))
                    {
                        line.clear();
                        line = QString("read* = \"%1\"").arg(QDir::toNativeSeparators(QString("%1/ar").arg(lgeoPath)));
                    }
                    if (line.contains(QRegExp("[\\/|\\\\]lg")))
                    {
                        line.clear();
                        line = QString("read* = \"%1\"").arg(QDir::toNativeSeparators(QString("%1/lg").arg(lgeoPath)));
                    }
                    if (lgeoStlLib && line.contains(QRegExp("[\\/|\\\\]stl")))
                    {
                        line.clear();
                        line = QString("read* = \"%1\"").arg(QDir::toNativeSeparators(QString("%1/stl").arg(lgeoPath)));
                    }
                }
            } else {
                QString locations = QString("You can use %HOME%, %INSTALLDIR% and the working directory (e.g. %1) as the origin to define permitted paths:")
                                            .arg(QDir::toNativeSeparators(QDir::homePath()+"/MOCs/myModel"));
                QString homedir = QString("%HOME% is hard-coded to the %1 environment variable (%2).")
#if defined Q_OS_WIN
                                          .arg("%USERPROFILE%")
#else
                                          .arg("$USER")
#endif
                                          .arg(QDir::toNativeSeparators(QDir::homePath()));
                QString workingdir = QString("The working directory (e.g. %1) is where LPub3D-Trace is called from.")
                                             .arg(QDir::toNativeSeparators(QDir::homePath()+"/MOCs/myModel"));

                // set application 3rd party renderers path
                line.replace(QString("__USEFUL_LOCATIONS_COMMENT__"),locations);
                line.replace(QString("__HOMEDIR_COMMENT__"),homedir);
                line.replace(QString("__WORKINGDIR_COMMENT__"),workingdir);
                line.replace(QString("__POVSYSDIR__"), QDir::toNativeSeparators(QString("%1/3rdParty/%2").arg(lpub3dPath, VER_POVRAY_STR)));
                // set lgeo paths as required
                if (lgeoPath != "")
                {
                    line.replace(QString("; read* = \"__LGEOARDIR__"), QString("read* = \"%1").arg(QDir::toNativeSeparators(lgeoPath)));
                    line.replace(QString("; read* = \"__LGEOLGDIR__"), QString("read* = \"%1").arg(QDir::toNativeSeparators(lgeoPath)));
                    if (lgeoStlLib){
                        line.replace(QString("; read* = \"__LGEOSTLDIR__"), QString("read* = \"%1").arg(QDir::toNativeSeparators(lgeoPath)));
                    }
                }
            }
            logInfo() << QString("POV-Ray.conf OUT: %1").arg(line);
            output << line << lpub_endl;
        }
        confFileIn.close();
        confFileOut.close();
    } else {
        QString confFileError;
        if (!confFileIn.errorString().isEmpty())
            confFileError.append(QString(" confFileInError: %1\n").arg(confFileIn.errorString()));
        if (!confFileOut.errorString().isEmpty())
            confFileError.append(QString(" confFileOutError: %1").arg(confFileOut.errorString()));
        logError() << QString("Could not open POVRay.conf input or output file: %1").arg(confFileError);
    }
    if (resourceFile.exists())
        povrayConf = resourceFile.absoluteFilePath();  // populate povray conf file
    if (oldFile.exists())
        oldFile.remove();                              // delete old file
    logInfo() << QString("POVRay conf file   : %1").arg(povrayConf.isEmpty() ? "Not found" : povrayConf);

    updatePOVRayConfigFiles();
}

void Preferences::updatePOVRayConfigFiles(){
#if defined Q_OS_WIN
    if (preferredRenderer == RENDERER_POVRAY) {
        QString targetFolder, destFolder, dataPath, targetFile, destFile, saveFile;
        QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
        QFileInfo fileInfo, saveFileInfo;
        dataPath     = dataPathList.first();
        destFolder   = QString("%1/%2").arg(dataPath).arg("3rdParty/" VER_POVRAY_STR "/config");
        targetFolder = QString(lpub3dPath +"/3rdParty/" VER_POVRAY_STR "/config");
        if (portableDistribution) {
            targetFile = QString("%1/povray.conf").arg(targetFolder);
            destFile   = QString("%1/povray.conf").arg(destFolder);
            saveFile   = QString("%1/povray_install.conf").arg(destFolder);
            fileInfo.setFile(destFile);
            saveFileInfo.setFile(saveFile);
            if (! saveFileInfo.exists()) {
                if (fileInfo.exists()) {
                    QFile file(fileInfo.absoluteFilePath());
                    if (! file.rename(saveFile) || ! QFile::copy(targetFile,destFile)) {
                        logError() << QString("Could not copy %1 to %2").arg(targetFile).arg(destFile);
                    } else {
                        logInfo() << QString("Copied 'portable' POVRay conf file %1").arg(destFile);
                    }
                } else if (! QFile::copy(targetFile,destFile)) {
                    logError() << QString("Could not copy %1 to %2").arg(targetFile).arg(destFile);
                }
            }
            targetFile = QString("%1/povray.ini").arg(targetFolder);
            destFile   = QString("%1/povray.ini").arg(destFolder);
            saveFile   = QString("%1/povray_install.ini").arg(destFolder);
            fileInfo.setFile(destFile);
            saveFileInfo.setFile(saveFile);
            if (! saveFileInfo.exists()) {
                if (fileInfo.exists()) {
                    QFile file(fileInfo.absoluteFilePath());
                    if (! file.rename(saveFile) || ! QFile::copy(targetFile,destFile)) {
                        logError() << QString("Could not copy %1 to %2").arg(targetFile).arg(destFile);
                    } else {
                        logInfo() << QString("Copied 'portable' POVRay ini file %1").arg(destFile);
                    }
                } else if (! QFile::copy(targetFile,destFile)) {
                    logError() << QString("Could not copy %1 to %2").arg(targetFile).arg(destFile);
                }
            }
        } else {
            saveFile = QString("%1/povray_install.conf").arg(destFolder);
            saveFileInfo.setFile(saveFile);
            if (saveFileInfo.exists()) {
                destFile = QString("%1/povray.conf").arg(destFolder);
                fileInfo.setFile(destFile);
                QFile file2(saveFileInfo.absoluteFilePath());
                if (fileInfo.exists()) {
                    QFile file(fileInfo.absoluteFilePath());
                    if (! file.remove(destFile) || ! file2.rename(destFile)) {
                        logError() << QString("Could not rename %1 to %2").arg(targetFile).arg(destFile);
                    } else {
                        logInfo() << QString("Renamed 'installed' POVRay conf file %1").arg(destFile);
                    }
                } else if (! file2.rename(destFile)) {
                    logError() << QString("Could not rename %1 to %2").arg(targetFile).arg(destFile);
                }
            }
            saveFile = QString("%1/povray_install.ini").arg(destFolder);
            saveFileInfo.setFile(saveFile);
            if (saveFileInfo.exists()) {
                destFile = QString("%1/povray.ini").arg(destFolder);
                fileInfo.setFile(destFile);
                QFile file2(saveFileInfo.absoluteFilePath());
                if (fileInfo.exists()) {
                    QFile file(fileInfo.absoluteFilePath());
                    if (! file.remove(destFile) || ! file2.rename(destFile)) {
                        logError() << QString("Could not rename %1 to %2").arg(targetFile).arg(destFile);
                    } else {
                        logInfo() << QString("Renamed 'installed' POVRay ini file %1").arg(destFile);
                    }
                } else if (! file2.rename(destFile)) {
                    logError() << QString("Could not rename %1 to %2").arg(targetFile).arg(destFile);
                }
            }

        }
    }
#else
    return;
#endif
}

void Preferences::updatePOVRayIniFile(UpdateFlag updateFlag)
{
    if (!povRayInstalled)
        return;

    QString inFileName;
    QFileInfo resourceFile;
    QFile confFileIn, confFileOut, oldFile;
    QDateTime timeStamp = QDateTime::currentDateTime();

    resourceFile.setFile(QString("%1/%2/%3").arg(lpub3d3rdPartyConfigDir, VER_POVRAY_STR "/config" ,VER_POVRAY_INI_FILE));
    if (resourceFile.exists())
    {
        if (updateFlag == SkipExisting) {
            povrayIni = resourceFile.absoluteFilePath();     // populate povray ini file
            logInfo() << QString("POVRay ini file    : %1").arg(povrayIni);
           return;
        }
        logInfo() << QString("Updating %1...").arg(resourceFile.absoluteFilePath());
        inFileName = QString("%1.%2").arg(resourceFile.absoluteFilePath(),timeStamp.toString("ddMMyyhhmmss"));
        oldFile.setFileName(resourceFile.absoluteFilePath());
        oldFile.rename(inFileName);
    } else {
       logInfo() << QString("Initializing %1...").arg(resourceFile.absoluteFilePath());
       inFileName = QString("%1/%2/resources/config/%3").arg(lpub3d3rdPartyAppDir, VER_POVRAY_STR, resourceFile.fileName());
       if (!resourceFile.absoluteDir().exists())
           resourceFile.absoluteDir().mkpath(".");
    }
    confFileIn.setFileName(QDir::toNativeSeparators(inFileName));
    confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_POVRAY_STR, resourceFile.fileName()));
    if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream input(&confFileIn);
        QTextStream output(&confFileOut);
        while (!input.atEnd())
        {
            // set the application 3rd party renderers path
            QString line = input.readLine();
            if (! oldFile.exists())
              line.replace(QString("__POVSYSDIR__"), QDir::toNativeSeparators(QString("%1/3rdParty/%2").arg(lpub3dPath, VER_POVRAY_STR)));
            logInfo() << QString("POV-Ray.ini OUT: %1").arg(line);
            output << line << lpub_endl;
        }
        confFileIn.close();
        confFileOut.close();
    } else {
        QString confFileError;
        if (!confFileIn.errorString().isEmpty())
            confFileError.append(QString(" confFileInError: %1\n").arg(confFileIn.errorString()));
        if (!confFileOut.errorString().isEmpty())
            confFileError.append(QString(" confFileOutError: %1").arg(confFileOut.errorString()));
        logError() << QString("Could not open POVRay.ini input or output file: %1").arg(confFileError);
    }
    if (resourceFile.exists())
        povrayIni = resourceFile.absoluteFilePath();     // populate povray ini file
    if (oldFile.exists())
        oldFile.remove();                              // delete old file
    logInfo() << QString("POVRay ini file    : %1").arg(povrayIni.isEmpty() ? "Not found" : povrayIni);
}

void Preferences::unitsPreferences()
{
    QSettings Settings;
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"Centimeters"))) {
        QVariant uValue(false);
        preferCentimeters = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"Centimeters"),uValue);
    } else {
        preferCentimeters = Settings.value(QString("%1/%2").arg(SETTINGS,"Centimeters")).toBool();
    }
}

void Preferences::editorPreferences()
{
    QSettings Settings;

    //  LDraw editor font
    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"EditorFont"))) {
        editorFont = qApp->font().family();
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"EditorFont"),editorFont);
    } else {
        editorFont = Settings.value(QString("%1/%2").arg(DEFAULTS,"EditorFont")).toString();
    }

    //  LDraw editor font size
    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"EditorFontSize"))) {
        editorFontSize = qApp->font().pointSize();
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"EditorFontSize"),editorFontSize);
    } else {
        editorFontSize = Settings.value(QString("%1/%2").arg(DEFAULTS,"EditorFontSize")).toInt();
    }

    //  LDraw editor buffered page read
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"EditorBufferedPaging"))) {
        QVariant uValue(editorBufferedPaging);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorBufferedPaging"),uValue);
    } else {
        editorBufferedPaging = Settings.value(QString("%1/%2").arg(SETTINGS,"EditorBufferedPaging")).toBool();
    }

    // Number of LDraw editor lines per paged buffer read
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"EditorLinesPerPage"))) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorLinesPerPage"),editorLinesPerPage);
    } else {
        editorLinesPerPage = Settings.value(QString("%1/%2").arg(SETTINGS,"EditorLinesPerPage")).toInt();
    }

    // LDraw editor text decoration
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"EditorDecoration"))) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorDecoration"),editorDecoration);
    } else {
        editorDecoration = Settings.value(QString("%1/%2").arg(SETTINGS,"EditorDecoration")).toInt();
    }

    // Highlight selected lines when clicked in Editor
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"EditorHighlightLines"))) {
        QVariant uValue(editorHighlightLines);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorHighlightLines"),uValue);
    } else {
        editorHighlightLines = Settings.value(QString("%1/%2").arg(SETTINGS,"EditorHighlightLines")).toBool();
    }

    // Load the first step (on multi-line select) of selected lines in the Visual Editor
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"EditorLoadSelectionStep"))) {
        QVariant uValue(editorLoadSelectionStep);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorLoadSelectionStep"),uValue);
    } else {
        editorLoadSelectionStep = Settings.value(QString("%1/%2").arg(SETTINGS,"EditorLoadSelectionStep")).toBool();
    }

    // Launch floating preview window on valid line double click
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"EditorPreviewOnDoubleClick"))) {
        QVariant uValue(editorPreviewOnDoubleClick);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorPreviewOnDoubleClick"),uValue);
    } else {
        editorPreviewOnDoubleClick = Settings.value(QString("%1/%2").arg(SETTINGS,"EditorPreviewOnDoubleClick")).toBool();
    }
}

void Preferences::themePreferences()
{
    QSettings Settings;


    QString const displayThemeKey("DisplayTheme");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,displayThemeKey))) {
            displayTheme = THEME_DEFAULT;
            Settings.setValue(QString("%1/%2").arg(SETTINGS,displayThemeKey),displayTheme);
    } else {
            displayTheme = Settings.value(QString("%1/%2").arg(SETTINGS,displayThemeKey)).toString();
    }

    for (int i = 0; i < THEME_NUM_COLORS; i++) {
        const QString themeKey(defaultThemeColors[i].key);
        if ( ! Settings.contains(QString("%1/%2").arg(THEMECOLORS,themeKey))) {
            themeColors[i] = defaultThemeColors[i].color;
            Settings.setValue(QString("%1/%2").arg(THEMECOLORS,themeKey),themeColors[i]);
        } else {
            themeColors[i] = Settings.value(QString("%1/%2").arg(THEMECOLORS,themeKey)).toString().toUpper();
        }
    }
}

void Preferences::userInterfacePreferences()
{
  QSettings Settings;
  QString const sceneRulerKey("SceneRuler");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneRulerKey))) {
          QVariant uValue(false);
          sceneRuler = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerKey),uValue);
  } else {
          sceneRuler = Settings.value(QString("%1/%2").arg(SETTINGS,sceneRulerKey)).toBool();
  }

  QString const sceneRulerTrackingKey("SceneRulerTracking");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingKey))) {
          sceneRulerTracking = TRACKING_NONE;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingKey),sceneRulerTracking);
  } else {
          sceneRulerTracking = Settings.value(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingKey)).toInt();
  }

  QString const sceneGuidesKey("SceneGuides");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneGuidesKey))) {
          QVariant uValue(false);
          sceneGuides = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuidesKey),uValue);
  } else {
          sceneGuides = Settings.value(QString("%1/%2").arg(SETTINGS,sceneGuidesKey)).toBool();
  }

  QString const sceneGuidesLineKey("SceneGuidesLine");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneGuidesLineKey))) {
          sceneGuidesLine = int(Qt::DashLine);
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuidesLineKey),sceneGuidesLine);
  } else {
          sceneGuidesLine = Settings.value(QString("%1/%2").arg(SETTINGS,sceneGuidesLineKey)).toInt();
  }

  QString const sceneGuidesPositionKey("SceneGuidesPosition");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneGuidesPositionKey))) {
      sceneGuidesPosition = int(GUIDES_TOP_LEFT);
      Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuidesPositionKey),sceneGuidesPosition);
  } else {
      sceneGuidesPosition = Settings.value(QString("%1/%2").arg(SETTINGS,sceneGuidesPositionKey)).toInt();
  }

  QString const customSceneBackgroundColorKey("CustomSceneBackgroundColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,customSceneBackgroundColorKey))) {
          QVariant uValue(false);
          customSceneBackgroundColor = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,customSceneBackgroundColorKey),uValue);
  } else {
          customSceneBackgroundColor = Settings.value(QString("%1/%2").arg(SETTINGS,customSceneBackgroundColorKey)).toBool();
  }

  QString const customSceneGridColorKey("CustomSceneGridColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,customSceneGridColorKey))) {
          QVariant uValue(false);
          customSceneGridColor = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,customSceneGridColorKey),uValue);
  } else {
          customSceneGridColor = Settings.value(QString("%1/%2").arg(SETTINGS,customSceneGridColorKey)).toBool();
  }

  QString const customSceneRulerTickColorKey("CustomSceneRulerTickColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,customSceneRulerTickColorKey))) {
          QVariant uValue(false);
          customSceneRulerTickColor = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,customSceneRulerTickColorKey),uValue);
  } else {
          customSceneRulerTickColor = Settings.value(QString("%1/%2").arg(SETTINGS,customSceneRulerTickColorKey)).toBool();
  }

  QString const customSceneRulerTrackingColorKey("CustomSceneRulerTrackingColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,customSceneRulerTrackingColorKey))) {
          QVariant uValue(false);
          customSceneRulerTrackingColor = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,customSceneRulerTrackingColorKey),uValue);
  } else {
          customSceneRulerTrackingColor = Settings.value(QString("%1/%2").arg(SETTINGS,customSceneRulerTrackingColorKey)).toBool();
  }

  QString const customSceneGuideColorKey("CustomSceneGuideColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,customSceneGuideColorKey))) {
          QVariant uValue(false);
          customSceneGuideColor = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,customSceneGuideColorKey),uValue);
  } else {
          customSceneGuideColor = Settings.value(QString("%1/%2").arg(SETTINGS,customSceneGuideColorKey)).toBool();
  }

  QString const sceneBackgroundColorKey("SceneBackgroundColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneBackgroundColorKey))) {
          displayTheme == THEME_DARK ?
                  sceneBackgroundColor = themeColors[THEME_DARK_SCENE_BACKGROUND_COLOR] :
                  sceneBackgroundColor = themeColors[THEME_DEFAULT_SCENE_BACKGROUND_COLOR];
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneBackgroundColorKey),sceneBackgroundColor);
  } else {
          sceneBackgroundColor = Settings.value(QString("%1/%2").arg(SETTINGS,sceneBackgroundColorKey)).toString().toUpper();
  }

  QString const sceneGridColorKey("SceneGridColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneGridColorKey))) {
          displayTheme == THEME_DARK ?
                  sceneGridColor = themeColors[THEME_DARK_GRID_PEN] :
                  sceneGridColor = themeColors[THEME_DEFAULT_GRID_PEN];
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGridColorKey),sceneGridColor);
  } else {
          sceneGridColor = Settings.value(QString("%1/%2").arg(SETTINGS,sceneGridColorKey)).toString().toUpper();
  }

  QString const sceneRulerTickColorKey("SceneRulerTickColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneRulerTickColorKey))) {
          displayTheme == THEME_DARK ?
                  sceneRulerTickColor = themeColors[THEME_DARK_RULER_TICK_PEN] :
                  sceneRulerTickColor = themeColors[THEME_DEFAULT_RULER_TICK_PEN];
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerTickColorKey),sceneRulerTickColor);
  } else {
          sceneRulerTickColor = Settings.value(QString("%1/%2").arg(SETTINGS,sceneRulerTickColorKey)).toString().toUpper();
  }

  QString const sceneRulerTrackingColorKey("SceneRulerTrackingColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingColorKey))) {
          displayTheme == THEME_DARK ?
                  sceneRulerTrackingColor = themeColors[THEME_DARK_RULER_TRACK_PEN] :
                  sceneRulerTrackingColor = themeColors[THEME_DEFAULT_RULER_TRACK_PEN];
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingColorKey),sceneRulerTrackingColor);
  } else {
          sceneRulerTrackingColor = Settings.value(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingColorKey)).toString().toUpper();
  }

  QString const sceneGuideColorKey("SceneGuideColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneGuideColorKey))) {
          displayTheme == THEME_DARK ?
                  sceneGuideColor = themeColors[THEME_DARK_GUIDE_PEN] :
                  sceneGuideColor = themeColors[THEME_DEFAULT_GUIDE_PEN];
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuideColorKey),sceneGuideColor);
  } else {
          sceneGuideColor = Settings.value(QString("%1/%2").arg(SETTINGS,sceneGuideColorKey)).toString().toUpper();
  }

  QString const snapToGridKey("SnapToGrid");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,snapToGridKey))) {
          QVariant uValue(false);
          snapToGrid = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,snapToGridKey),uValue);
  } else {
          snapToGrid = Settings.value(QString("%1/%2").arg(SETTINGS,snapToGridKey)).toBool();
  }

  QString const hidePageBackgroundKey("HidePageBackground");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,hidePageBackgroundKey))) {
          QVariant uValue(false);
          hidePageBackground = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,hidePageBackgroundKey),uValue);
  } else {
          hidePageBackground = Settings.value(QString("%1/%2").arg(SETTINGS,hidePageBackgroundKey)).toBool();
  }

  QString const showGuidesCoordinatesKey("ShowGuidesCoordinates");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,showGuidesCoordinatesKey))) {
          QVariant uValue(false);
          showGuidesCoordinates = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,showGuidesCoordinatesKey),uValue);
  } else {
          showGuidesCoordinates = Settings.value(QString("%1/%2").arg(SETTINGS,showGuidesCoordinatesKey)).toBool();
  }

  QString const showTrackingCoordinatesKey("ShowTrackingCoordinates");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,showTrackingCoordinatesKey))) {
          QVariant uValue(false);
          showTrackingCoordinates = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,showTrackingCoordinatesKey),uValue);
  } else {
          showTrackingCoordinates = Settings.value(QString("%1/%2").arg(SETTINGS,showTrackingCoordinatesKey)).toBool();
  }

  QString const gridSizeIndexKey("GridSizeIndex");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,gridSizeIndexKey))) {
      gridSizeIndex = GRID_SIZE_INDEX_DEFAULT;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,gridSizeIndexKey),gridSizeIndex);
  } else {
      gridSizeIndex = Settings.value(QString("%1/%2").arg(SETTINGS,gridSizeIndexKey)).toInt();
  }

  QString const lineParseErrorsKey("ShowLineParseErrors");
  if ( ! Settings.contains(QString("%1/%2").arg(MESSAGES,lineParseErrorsKey))) {
          QVariant uValue(true);
          lineParseErrors = true;
          Settings.setValue(QString("%1/%2").arg(MESSAGES,lineParseErrorsKey),uValue);
  } else {
          lineParseErrors = Settings.value(QString("%1/%2").arg(MESSAGES,lineParseErrorsKey)).toBool();
  }

  QString const insertErrorsKey("ShowInsertErrors");
  if ( ! Settings.contains(QString("%1/%2").arg(MESSAGES,insertErrorsKey))) {
      QVariant uValue(showInsertErrors);
      Settings.setValue(QString("%1/%2").arg(MESSAGES,insertErrorsKey),uValue);
  } else {
      showInsertErrors = Settings.value(QString("%1/%2").arg(MESSAGES,insertErrorsKey)).toBool();
  }

  QString const showIncludeFileErrorsKey("ShowIncludeFileErrors");
  if ( ! Settings.contains(QString("%1/%2").arg(MESSAGES,showIncludeFileErrorsKey))) {
          QVariant uValue(true);
          showIncludeFileErrors = true;
          Settings.setValue(QString("%1/%2").arg(MESSAGES,showIncludeFileErrorsKey),uValue);
  } else {
          showIncludeFileErrors = Settings.value(QString("%1/%2").arg(MESSAGES,showIncludeFileErrorsKey)).toBool();
  }

  QString const showBuildModErrorsKey("ShowBuildModErrors");
  if ( ! Settings.contains(QString("%1/%2").arg(MESSAGES,showBuildModErrorsKey))) {
          QVariant uValue(true);
          showBuildModErrors = true;
          Settings.setValue(QString("%1/%2").arg(MESSAGES,showBuildModErrorsKey),uValue);
  } else {
          showBuildModErrors = Settings.value(QString("%1/%2").arg(MESSAGES,showBuildModErrorsKey)).toBool();
  }

  QString const showAnnotationErrorsKey("ShowAnnotationErrors");
  if ( ! Settings.contains(QString("%1/%2").arg(MESSAGES,showAnnotationErrorsKey))) {
          QVariant uValue(true);
          showAnnotationErrors = true;
          Settings.setValue(QString("%1/%2").arg(MESSAGES,showAnnotationErrorsKey),uValue);
  } else {
          showAnnotationErrors = Settings.value(QString("%1/%2").arg(MESSAGES,showAnnotationErrorsKey)).toBool();
  }

  QString const showSaveOnRedrawKey("ShowSaveOnRedraw");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,showSaveOnRedrawKey))) {
          QVariant uValue(true);
          showSaveOnRedraw = true;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,showSaveOnRedrawKey),uValue);
  } else {
          showSaveOnRedraw = Settings.value(QString("%1/%2").arg(SETTINGS,showSaveOnRedrawKey)).toBool();
  }

  QString const showSaveOnUpdateKey("ShowSaveOnUpdate");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,showSaveOnUpdateKey))) {
          QVariant uValue(true);
          showSaveOnUpdate = true;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,showSaveOnUpdateKey),uValue);
  } else {
          showSaveOnUpdate = Settings.value(QString("%1/%2").arg(SETTINGS,showSaveOnUpdateKey)).toBool();
  }

  QString const showSubmodelsKey("ShowSubmodels");
  if (Settings.contains(QString("%1/%2").arg(SETTINGS,showSubmodelsKey))) {
      showSubmodels = Settings.value(QString("%1/%2").arg(SETTINGS,showSubmodelsKey)).toBool();
  }

  QString const showTopModelKey("ShowTopModel");
  if (Settings.contains(QString("%1/%2").arg(SETTINGS,showTopModelKey))) {
      showTopModel = Settings.value(QString("%1/%2").arg(SETTINGS,showTopModelKey)).toBool();
  }

  QString const showSubmodelInCalloutKey("ShowSubmodelInCallout");
  if (Settings.contains(QString("%1/%2").arg(SETTINGS,showSubmodelInCalloutKey))) {
      showSubmodelInCallout = Settings.value(QString("%1/%2").arg(SETTINGS,showSubmodelInCalloutKey)).toBool();
  }

  QString const showInstanceCountKey("ShowInstanceCount");
  if (Settings.contains(QString("%1/%2").arg(SETTINGS,showInstanceCountKey))) {
      showInstanceCount = Settings.value(QString("%1/%2").arg(SETTINGS,showInstanceCountKey)).toBool();
  }

  QString const loadLastOpenedFileKey("LoadLastOpenedFile");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,loadLastOpenedFileKey))) {
      QVariant uValue(false);
      loadLastOpenedFile = false;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,loadLastOpenedFileKey),uValue);
  } else {
      loadLastOpenedFile = Settings.value(QString("%1/%2").arg(SETTINGS,loadLastOpenedFileKey)).toBool();
  }

  QString const extendedSubfileSearchKey("ExtendedSubfileSearch");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,extendedSubfileSearchKey))) {
      QVariant uValue(false);
      extendedSubfileSearch = false;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,extendedSubfileSearchKey),uValue);
  } else {
      extendedSubfileSearch = Settings.value(QString("%1/%2").arg(SETTINGS,extendedSubfileSearchKey)).toBool();
  }

  QString const ldrawFilesLoadMsgsKey("LDrawFilesLoadMsgs");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,ldrawFilesLoadMsgsKey))) {
      ldrawFilesLoadMsgs = NEVER_SHOW;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawFilesLoadMsgsKey),ldrawFilesLoadMsgs);
  } else {
      ldrawFilesLoadMsgs = Settings.value(QString("%1/%2").arg(SETTINGS,ldrawFilesLoadMsgsKey)).toInt();
  }

  QString const saveOnRedrawKey("SaveOnRedraw");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,saveOnRedrawKey))) {
      QVariant uValue(true);
      saveOnRedraw = true;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,saveOnRedrawKey),uValue);
  } else {
      saveOnRedraw = Settings.value(QString("%1/%2").arg(SETTINGS,saveOnRedrawKey)).toBool();
  }

  QString const saveOnUpdateKey("SaveOnUpdate");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,saveOnUpdateKey))) {
      QVariant uValue(true);
      saveOnUpdate = true;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,saveOnUpdateKey),uValue);
  } else {
      saveOnUpdate = Settings.value(QString("%1/%2").arg(SETTINGS,saveOnUpdateKey)).toBool();
  }

  QString const maxOpenWithProgramsKey("MaxOpenWithPrograms");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,maxOpenWithProgramsKey))) {
      QVariant uValue(MAX_OPEN_WITH_PROGRAMS_DEFAULT);
      maxOpenWithPrograms = MAX_OPEN_WITH_PROGRAMS_DEFAULT;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,maxOpenWithProgramsKey),uValue);
  } else {
      maxOpenWithPrograms = Settings.value(QString("%1/%2").arg(SETTINGS,maxOpenWithProgramsKey)).toInt();
  }

  QString const useSystemEditorKey("UseSystemEditor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,useSystemEditorKey))) {
      useSystemEditor = false;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,useSystemEditorKey),useSystemEditor);
  } else {
      useSystemEditor = Settings.value(QString("%1/%2").arg(SETTINGS,useSystemEditorKey)).toBool();
  }

  QString const removeBuildModFormatKey("RemoveBuildModFormat");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,removeBuildModFormatKey))) {
      removeBuildModFormat = false;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,removeBuildModFormatKey),removeBuildModFormat);
  } else {
      removeBuildModFormat = Settings.value(QString("%1/%2").arg(SETTINGS,removeBuildModFormatKey)).toBool();
  }

  QString const removeChildSubmodelFormatKey("RemoveChildSubmodelFormat");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,removeChildSubmodelFormatKey))) {
      removeChildSubmodelFormat = false;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,removeChildSubmodelFormatKey),removeChildSubmodelFormat);
  } else {
      removeChildSubmodelFormat = Settings.value(QString("%1/%2").arg(SETTINGS,removeChildSubmodelFormatKey)).toBool();
  }

  QString const systemEditorKey("SystemEditor");
  systemEditor = Settings.value(QString("%1/%2").arg(SETTINGS,systemEditorKey)).toString();
  QFileInfo systemEditorInfo(systemEditor);
  usingNPP = systemEditorInfo.fileName() == QFileInfo(WINDOWS_NPP).fileName();
  if (!systemEditorInfo.exists() || !systemEditorInfo.isFile()) {
      bool found = false;
      QString command = "which";
      QStringList arguments;
#ifdef Q_OS_MAC
      arguments << MACOS_SYS_EDITOR;
#elif defined Q_OS_LINUX
      arguments << LINUX_SYS_EDITOR;
#elif defined Q_OS_WIN
      const QString systemDrive = QProcessEnvironment::systemEnvironment().value("SYSTEMDRIVE", "C:");
      if((found = QFileInfo(systemDrive + "\\" + WINDOWS_NPP_X64).exists())) {
        systemEditor = systemDrive + "\\" + WINDOWS_NPP_X64;
      } else if ((found = QFileInfo(systemDrive + "\\" + WINDOWS_NPP).exists())) {
        systemEditor = systemDrive + "\\" + WINDOWS_NPP;
      }
      usingNPP = found;
      if (found) {
        systemEditorInfo.setFile(systemEditor);
      } else {
        command = "where";
        arguments << WINDOWS_SYS_EDITOR;
      }
#endif
      if (!found) {
        QProcess findProcess;
        findProcess.start(command, arguments);
        findProcess.setReadChannel(QProcess::ProcessChannel::StandardOutput);
        if(findProcess.waitForFinished()) {
          systemEditor = QString(findProcess.readAll()).split(QRegExp("\n|\r\n|\r")).first().trimmed();
          systemEditorInfo.setFile(systemEditor);
        }
      }
      if ((found = systemEditorInfo.isFile())) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,systemEditorKey),systemEditor);
      }
      if (!found) {
        systemEditor.clear();
        Settings.remove(QString("%1/%2").arg(SETTINGS,systemEditorKey));
      }
  }

  // check if cycle each step when navigating forward by more than one step is enabled
  QString const cycleEachPageKey("CycleEachPage");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,cycleEachPageKey))) {
      cycleEachPage = false;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,cycleEachPageKey),cycleEachPage);
  } else {
      cycleEachPage = Settings.value(QString("%1/%2").arg(SETTINGS,cycleEachPageKey)).toBool();
  }

  QString const fileLoadWaitTimeKey("FileLoadWaitInMSec");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,fileLoadWaitTimeKey))) {
      fileLoadWaitTime = FILE_LOAD_WAIT_TIME;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,fileLoadWaitTimeKey),fileLoadWaitTime);
  } else {
      fileLoadWaitTime = Settings.value(QString("%1/%2").arg(SETTINGS,fileLoadWaitTimeKey)).toInt();
  }

  QString const autoUpdateChangeLogKey("AutoUpdateChangeLog");
  if (Settings.contains(QString("%1/%2").arg(SETTINGS,autoUpdateChangeLogKey))) {
      autoUpdateChangeLog = Settings.value(QString("%1/%2").arg(SETTINGS,autoUpdateChangeLogKey)).toBool();
  }
}

void Preferences::updateViewerInterfaceColors()
{
#define LC_RGBA_RED(rgba)   ((quint8)(((rgba) >>  0) & 0xff))
#define LC_RGBA_GREEN(rgba) ((quint8)(((rgba) >>  8) & 0xff))
#define LC_RGBA_BLUE(rgba)  ((quint8)(((rgba) >> 16) & 0xff))
#define LC_RGBA_ALPHA(rgba) ((quint8)(((rgba) >> 24) & 0xff))

    auto setInterfaceColor = [] (const ThemeColorType t, const quint32 & color)
    {
        QSettings Settings;
        themeColors[t] = QColor(LC_RGBA_RED(color), LC_RGBA_GREEN(color), LC_RGBA_BLUE(color), LC_RGBA_ALPHA(color)).name().toUpper();
        const QString themeKey(defaultThemeColors[t].key);
        Settings.setValue(QString("%1/%2").arg(THEMECOLORS,themeKey),themeColors[t]);
    };

    if (displayTheme == THEME_DARK) {
        setInterfaceColor(THEME_DARK_VIEWER_BACKGROUND_COLOR, lcGetProfileInt(LC_PROFILE_BACKGROUND_COLOR));
        setInterfaceColor(THEME_DARK_VIEWER_GRADIENT_COLOR_TOP, lcGetProfileInt(LC_PROFILE_GRADIENT_COLOR_TOP));
        setInterfaceColor(THEME_DARK_VIEWER_GRADIENT_COLOR_BOTTOM, lcGetProfileInt(LC_PROFILE_GRADIENT_COLOR_BOTTOM));
        setInterfaceColor(THEME_DARK_AXES_COLOR, lcGetProfileInt(LC_PROFILE_AXES_COLOR));
        setInterfaceColor(THEME_DARK_OVERLAY_COLOR, lcGetProfileInt(LC_PROFILE_OVERLAY_COLOR));
        setInterfaceColor(THEME_DARK_ACTIVE_VIEW_COLOR, lcGetProfileInt(LC_PROFILE_ACTIVE_VIEW_COLOR));
        setInterfaceColor(THEME_DARK_GRID_STUD_COLOR, lcGetProfileInt(LC_PROFILE_GRID_STUD_COLOR));
        setInterfaceColor(THEME_DARK_GRID_LINE_COLOR, lcGetProfileInt(LC_PROFILE_GRID_LINE_COLOR));
        setInterfaceColor(THEME_DARK_TEXT_COLOR, lcGetProfileInt(LC_PROFILE_TEXT_COLOR));
        setInterfaceColor(THEME_DARK_VIEW_SPHERE_COLOR, lcGetProfileInt(LC_PROFILE_VIEW_SPHERE_COLOR));
        setInterfaceColor(THEME_DARK_VIEW_SPHERE_TEXT_COLOR, lcGetProfileInt(LC_PROFILE_VIEW_SPHERE_TEXT_COLOR));
        setInterfaceColor(THEME_DARK_VIEW_SPHERE_HLIGHT_COLOR, lcGetProfileInt(LC_PROFILE_VIEW_SPHERE_HIGHLIGHT_COLOR));
        setInterfaceColor(THEME_DARK_OBJECT_SELECTED_COLOR, lcGetProfileUInt(LC_PROFILE_OBJECT_SELECTED_COLOR));
        setInterfaceColor(THEME_DARK_OBJECT_FOCUSED_COLOR, lcGetProfileUInt(LC_PROFILE_OBJECT_FOCUSED_COLOR));
        setInterfaceColor(THEME_DARK_CAMERA_COLOR, lcGetProfileUInt(LC_PROFILE_CAMERA_COLOR));
        setInterfaceColor(THEME_DARK_LIGHT_COLOR, lcGetProfileUInt(LC_PROFILE_LIGHT_COLOR));
        setInterfaceColor(THEME_DARK_CONTROL_POINT_COLOR, lcGetProfileUInt(LC_PROFILE_CONTROL_POINT_COLOR));
        setInterfaceColor(THEME_DARK_CONTROL_POINT_FOCUSED_COLOR, lcGetProfileUInt(LC_PROFILE_CONTROL_POINT_FOCUSED_COLOR));
        setInterfaceColor(THEME_DARK_BM_OBJECT_SELECTED_COLOR, lcGetProfileUInt(LC_PROFILE_BM_OBJECT_SELECTED_COLOR));
    } else {
        setInterfaceColor(THEME_DEFAULT_VIEWER_BACKGROUND_COLOR, lcGetProfileInt(LC_PROFILE_BACKGROUND_COLOR));
        setInterfaceColor(THEME_DEFAULT_VIEWER_GRADIENT_COLOR_TOP, lcGetProfileInt(LC_PROFILE_GRADIENT_COLOR_TOP));
        setInterfaceColor(THEME_DEFAULT_VIEWER_GRADIENT_COLOR_BOTTOM, lcGetProfileInt(LC_PROFILE_GRADIENT_COLOR_BOTTOM));
        setInterfaceColor(THEME_DEFAULT_AXES_COLOR, lcGetProfileInt(LC_PROFILE_AXES_COLOR));
        setInterfaceColor(THEME_DEFAULT_OVERLAY_COLOR, lcGetProfileInt(LC_PROFILE_OVERLAY_COLOR));
        setInterfaceColor(THEME_DEFAULT_ACTIVE_VIEW_COLOR, lcGetProfileInt(LC_PROFILE_ACTIVE_VIEW_COLOR));
        setInterfaceColor(THEME_DEFAULT_GRID_STUD_COLOR, lcGetProfileInt(LC_PROFILE_GRID_STUD_COLOR));
        setInterfaceColor(THEME_DEFAULT_GRID_LINE_COLOR, lcGetProfileInt(LC_PROFILE_GRID_LINE_COLOR));
        setInterfaceColor(THEME_DEFAULT_TEXT_COLOR, lcGetProfileInt(LC_PROFILE_TEXT_COLOR));
        setInterfaceColor(THEME_DEFAULT_VIEW_SPHERE_COLOR, lcGetProfileInt(LC_PROFILE_VIEW_SPHERE_COLOR));
        setInterfaceColor(THEME_DEFAULT_VIEW_SPHERE_TEXT_COLOR, lcGetProfileInt(LC_PROFILE_VIEW_SPHERE_TEXT_COLOR));
        setInterfaceColor(THEME_DEFAULT_VIEW_SPHERE_HLIGHT_COLOR, lcGetProfileInt(LC_PROFILE_VIEW_SPHERE_HIGHLIGHT_COLOR));
        setInterfaceColor(THEME_DEFAULT_OBJECT_SELECTED_COLOR, lcGetProfileUInt(LC_PROFILE_OBJECT_SELECTED_COLOR));
        setInterfaceColor(THEME_DEFAULT_OBJECT_FOCUSED_COLOR, lcGetProfileUInt(LC_PROFILE_OBJECT_FOCUSED_COLOR));
        setInterfaceColor(THEME_DEFAULT_CAMERA_COLOR, lcGetProfileUInt(LC_PROFILE_CAMERA_COLOR));
        setInterfaceColor(THEME_DEFAULT_LIGHT_COLOR, lcGetProfileUInt(LC_PROFILE_LIGHT_COLOR));
        setInterfaceColor(THEME_DEFAULT_CONTROL_POINT_COLOR, lcGetProfileUInt(LC_PROFILE_CONTROL_POINT_COLOR));
        setInterfaceColor(THEME_DEFAULT_CONTROL_POINT_FOCUSED_COLOR, lcGetProfileUInt(LC_PROFILE_CONTROL_POINT_FOCUSED_COLOR));
        setInterfaceColor(THEME_DEFAULT_BM_OBJECT_SELECTED_COLOR, lcGetProfileUInt(LC_PROFILE_BM_OBJECT_SELECTED_COLOR));
    }
}

bool Preferences::getShowMessagePreference(MsgKey key)
{
    bool result = true;
    QSettings Settings;
    QString const showMessageKey(MsgKeys[key]);
    if ( ! Settings.contains(QString("%1/%2").arg(MESSAGES,showMessageKey))) {
        QVariant uValue(result);
        Settings.setValue(QString("%1/%2").arg(MESSAGES,showMessageKey),uValue);
    } else {
        result = Settings.value(QString("%1/%2").arg(MESSAGES,showMessageKey)).toBool();
    }
    switch(key){
    case ParseErrors:
        lineParseErrors = result;
        break;
    case InsertErrors:
        showInsertErrors = result;
        break;
    case BuildModErrors:
        showBuildModErrors = result;
        break;
    case IncludeFileErrors:
        showIncludeFileErrors = result;
        break;
    case AnnotationErrors:
        showAnnotationErrors = result;
        break;
    default:
        break;
    }
    return result;
}

int  Preferences::showMessage(Preferences::MsgID msgID,
        const QString &message,
        const QString &title,
        const QString &type,
        bool option  /*OkCancel=false*/,
        bool override/*false*/,
        int icon/*Critical*/)
{
    for (QString &messageNotShown : messagesNotShown)
        if (messageNotShown.startsWith(msgID.toString()))
            return QMessageBox::Ok;

    QString msgTitle = title.isEmpty() ? msgKeyTypes[msgID.msgKey][0] : title;
    QString msgType  = type.isEmpty()  ? msgKeyTypes[msgID.msgKey][1] : type;

    if (static_cast<QMessageBox::Icon>(icon) == QMessageBox::Icon::Critical)
        LPub::loadBanner(ERROR_ENCOUNTERED);

    QMessageBoxResizable box;
    box.setWindowTitle(QString("%1 %2").arg(VER_PRODUCTNAME_STR).arg(msgTitle));
    box.setText(message);
    box.setIcon(static_cast<QMessageBox::Icon>(icon));
    box.setStandardButtons (option ? QMessageBox::Ok | QMessageBox::Cancel : QMessageBox::Ok);
    box.setDefaultButton   (option ? QMessageBox::Cancel : QMessageBox::Ok);
    if (!override) {
        QCheckBox *cb = new QCheckBox(QString("Do not show this %1 again.").arg(msgType));
        box.setCheckBox(cb);
        QObject::connect(cb, &QCheckBox::stateChanged, [&message, &msgID](int state) {
            if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked)
                messagesNotShown.append(QString(msgID.toString() + "|" + message));
        });
    }
    box.adjustSize();

    return box.exec();
}

void Preferences::setShowSaveOnRedrawPreference(bool b)
{
  QSettings Settings;
  showSaveOnRedraw = b;
  QVariant uValue(b);
  QString const showSaveOnRedrawKey("ShowSaveOnRedraw");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,showSaveOnRedrawKey),uValue);
}

void Preferences::setShowSaveOnUpdatePreference(bool b)
{
  QSettings Settings;
  showSaveOnUpdate = b;
  QVariant uValue(b);
  QString const showSaveOnUpdateKey("ShowSaveOnUpdate");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,showSaveOnUpdateKey),uValue);
}

void Preferences::setSnapToGridPreference(bool b)
{
  QSettings Settings;
  snapToGrid = b;
  QVariant uValue(b);
  QString const snapToGridKey("SnapToGrid");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,snapToGridKey),uValue);
}

void Preferences::setHidePageBackgroundPreference(bool b)
{
  QSettings Settings;
  hidePageBackground = b;
  QVariant uValue(b);
  QString const hidePageBackgroundKey("HidePageBackground");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,hidePageBackgroundKey),uValue);
}

void Preferences::setShowGuidesCoordinatesPreference(bool b)
{
  QSettings Settings;
  showGuidesCoordinates = b;
  QVariant uValue(b);
  QString const showGuidesCoordinatesKey("ShowGuidesCoordinates");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,showGuidesCoordinatesKey),uValue);
}

void Preferences::setShowTrackingCoordinatesPreference(bool b)
{
  QSettings Settings;
  showTrackingCoordinates = b;
  QVariant uValue(b);
  QString const showTrackingCoordinatesKey("ShowTrackingCoordinates");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,showTrackingCoordinatesKey),uValue);
}

void Preferences::setGridSizeIndexPreference(int i)
{
  QSettings Settings;
  gridSizeIndex = i;
  QVariant uValue(i);
  QString const gridSizeIndex("GridSizeIndex");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,gridSizeIndex),uValue);
}

void Preferences::setSceneGuidesPreference(bool b)
{
  QSettings Settings;
  sceneGuides = b;
  QVariant uValue(b);
  QString const sceneGuidesKey("SceneGuides");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuidesKey),uValue);
}

void Preferences::setSceneGuidesLinePreference(int i)
{
  QSettings Settings;
  sceneGuidesLine = i;
  QVariant uValue(i);
  QString const sceneGuidesLineKey("SceneGuidesLine");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuidesLineKey),uValue);
}

void Preferences::setSceneGuidesPositionPreference(int i)
{
  QSettings Settings;
  sceneGuidesPosition = i;
  QVariant uValue(i);
  QString const sceneGuidesPositionKey("SceneGuidesPosition");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuidesPositionKey),uValue);
}

void Preferences::setSceneRulerPreference(bool b)
{
  QSettings Settings;
  sceneRuler = b;
  QVariant uValue(b);
  QString const sceneRulerKey("SceneRuler");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerKey),uValue);
}

void Preferences::setSceneRulerTrackingPreference(int i)
{
  QSettings Settings;
  sceneRulerTracking = i;
  QString const sceneRulerTrackingKey("SceneRulerTracking");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingKey),sceneRulerTracking);
}

void Preferences::setCustomSceneGridColorPreference(bool b)
{
  QSettings Settings;
  customSceneGridColor = b;
  QVariant uValue(b);
  QString const sceneGridColorKey("CustomSceneGridColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGridColorKey),uValue);
}

void Preferences::setCustomSceneRulerTickColorPreference(bool b)
{
  QSettings Settings;
  customSceneRulerTickColor = b;
  QVariant uValue(b);
  QString const sceneRulerTickColorKey("CustomSceneRulerTickColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerTickColorKey),uValue);
}

void Preferences::setCustomSceneRulerTrackingColorPreference(bool b)
{
  QSettings Settings;
  customSceneRulerTrackingColor = b;
  QVariant uValue(b);
  QString const sceneRulerTrackingColorKey("CustomSceneRulerTrackingColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingColorKey),uValue);
}

void Preferences::setCustomSceneGuideColorPreference(bool b)
{
  QSettings Settings;
  customSceneGuideColor = b;
  QVariant uValue(b);
  QString const sceneGuideColorKey("CustomSceneGuideColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuideColorKey),uValue);
}

void Preferences::setSceneBackgroundColorPreference(QString s)
{
  QSettings Settings;
  sceneBackgroundColor = s;
  QVariant uValue(s);
  QString const sceneBackgroundColorKey("SceneBackgroundColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneBackgroundColorKey),uValue);
}

void Preferences::setCustomSceneBackgroundColorPreference(bool b)
{
  QSettings Settings;
  customSceneBackgroundColor = b;
  QVariant uValue(b);
  QString const sceneBackgroundColorKey("CustomSceneBackgroundColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneBackgroundColorKey),uValue);
}

void Preferences::setSceneGridColorPreference(QString s)
{
  QSettings Settings;
  sceneGridColor = s;
  QVariant uValue(s);
  QString const sceneGridColorKey("SceneGridColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGridColorKey),uValue);
}

void Preferences::setSceneRulerTickColorPreference(QString s)
{
  QSettings Settings;
  sceneRulerTickColor = s;
  QVariant uValue(s);
  QString const sceneRulerTickColorKey("SceneRulerTickColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerTickColorKey),uValue);
}

void Preferences::setSceneRulerTrackingColorPreference(QString s)
{
  QSettings Settings;
  sceneRulerTrackingColor = s;
  QVariant uValue(s);
  QString const sceneRulerTrackingColorKey("SceneRulerTrackingColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingColorKey),uValue);
}

void Preferences::setSceneGuideColorPreference(QString s)
{
  QSettings Settings;
  sceneGuideColor = s;
  QVariant uValue(s);
  QString const sceneGuideColorKey("SceneGuideColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuideColorKey),uValue);
}

void Preferences::setBlenderExePathPreference(QString s)
{
  QSettings Settings;
  blenderExe = s;
  QVariant uValue(s);
  QString const blenderExeKey("BlenderExeFile");
  if (blenderExe.isEmpty())
      Settings.remove(QString("%1/%2").arg(SETTINGS,blenderExeKey));
  else
      Settings.setValue(QString("%1/%2").arg(SETTINGS,blenderExeKey),uValue);
}

void Preferences::setBlenderVersionPreference(QString s)
{
    QSettings Settings;
    blenderVersion = s;
    QVariant uValue(s);
    QString const blenderVersionKey("BlenderVersion");
    if (blenderVersion.isEmpty())
        Settings.remove(QString("%1/%2").arg(SETTINGS,blenderVersionKey));
    else
        Settings.setValue(QString("%1/%2").arg(SETTINGS,blenderVersionKey),uValue);
}

void Preferences::removeBuildModFormatPreference(bool i)
{
  QSettings Settings;
  removeBuildModFormat = i;
  QVariant uValue(i);
  QString const settingsKey("RemoveBuildModFormat");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,settingsKey),uValue);
}

void Preferences::removeChildSubmodelFormatPreference(bool i)
{
  QSettings Settings;
  removeChildSubmodelFormat = i;
  QVariant uValue(i);
  QString const settingsKey("RemoveChildSubmodelFormat");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,settingsKey),uValue);
}

void Preferences::useSystemEditorPreference(bool i)
{
  QSettings Settings;
  useSystemEditor = i;
  QVariant uValue(i);
  QString const useSystemEditorKey("UseSystemEditor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,useSystemEditorKey),uValue);
}

bool Preferences::isBlender28OrLater()
{
    QStringList items = blenderVersion.split(" ");
    QString vesionString = items.size() ? items.first() : QString();
    bool ok;
    qreal version = vesionString.toDouble(&ok);
    if (ok)
        return version > 2.79;
    return true;
}

void Preferences::setDebugLogging(bool b){
    debugLogging = b;
}

void Preferences::annotationPreferences()
{
    QFileInfo annInfo;
    QSettings Settings;
    enum A_OK {A_01, A_02, A_03, A_04, A_05, A_06, A_07, A_08, A_09, A_10, NUM_OK};
    bool annOk[NUM_OK] = { true, true, true, true, true, true, true, true, true, true };

    QString const titleAnnotationKey(    "TitleAnnotationFile");
    QString const freeFormAnnotationsKey("FreeFormAnnotationsFile");
    QString const annotationStyleKey(    "AnnotationStyleFile");
    QString const blCodesKey(            "BLCodesFile");
    QString const legoElementsKey(       "LEGOElementsFile");
    QString const blColorsKey(           "BLColorsFile");
    QString const ld2blColorsXRefKey(    "LD2BLColorsXRefFile");
    QString const ld2blCodesXRefKey(     "LD2BLCodesXRefFile");
    QString const ld2rbColorsXRefKey(    "LD2RBColorsXRefFile");
    QString const ld2rbCodesXRefKey(     "LD2RBCodesXRefFile");

     titleAnnotationsFile = QString("%1/extras/%2").arg(lpubDataPath,VER_TITLE_ANNOTATIONS_FILE);
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,titleAnnotationKey)))
        titleAnnotationsFile = Settings.value(QString("%1/%2").arg(SETTINGS,titleAnnotationKey)).toString();
    annInfo.setFile(titleAnnotationsFile);
    if (! annInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,titleAnnotationKey));
        annOk[A_01] = false;
    }

    freeformAnnotationsFile = QString("%1/extras/%2").arg(lpubDataPath,VER_FREEFOM_ANNOTATIONS_FILE);
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,freeFormAnnotationsKey)))
        freeformAnnotationsFile = Settings.value(QString("%1/%2").arg(SETTINGS,freeFormAnnotationsKey)).toString();
    annInfo.setFile(freeformAnnotationsFile);
    if (! annInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,freeFormAnnotationsKey));
        annOk[A_02] = false;
    }

    annotationStyleFile = QString("%1/extras/%2").arg(lpubDataPath,VER_ANNOTATION_STYLE_FILE);
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,annotationStyleKey)))
        annotationStyleFile = Settings.value(QString("%1/%2").arg(SETTINGS,annotationStyleKey)).toString();
    annInfo.setFile(annotationStyleFile);
    if (! annInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,annotationStyleKey));
        annOk[A_03] = false;
    }

    if ((annOk[A_04] = Settings.contains(QString("%1/%2").arg(SETTINGS,blCodesKey)))) {
        blCodesFile = Settings.value(QString("%1/%2").arg(SETTINGS,blCodesKey)).toString();
        annInfo.setFile(blCodesFile);
        if (! annInfo.exists()) {
            Settings.remove(QString("%1/%2").arg(SETTINGS,blCodesKey));
            annOk[A_04] = false;
        }
    }

    if ((annOk[A_05] = Settings.contains(QString("%1/%2").arg(SETTINGS,legoElementsKey)))) {
        legoElementsFile = Settings.value(QString("%1/%2").arg(SETTINGS,legoElementsKey)).toString();
        annInfo.setFile(legoElementsFile);
        if (! annInfo.exists()) {
            Settings.remove(QString("%1/%2").arg(SETTINGS,legoElementsKey));
            annOk[A_05] = false;
        }
    }

    if ((annOk[A_06] = Settings.contains(QString("%1/%2").arg(SETTINGS,blColorsKey)))) {
        blColorsFile = Settings.value(QString("%1/%2").arg(SETTINGS,blColorsKey)).toString();
        annInfo.setFile(blColorsFile);
        if (! annInfo.exists()) {
            Settings.remove(QString("%1/%2").arg(SETTINGS,blColorsKey));
            annOk[A_06] = false;
        }
    }

    if ((annOk[A_07] = Settings.contains(QString("%1/%2").arg(SETTINGS,ld2blColorsXRefKey)))) {
        ld2blColorsXRefFile = Settings.value(QString("%1/%2").arg(SETTINGS,ld2blColorsXRefKey)).toString();
        annInfo.setFile(ld2blColorsXRefFile);
        if (! annInfo.exists()) {
            Settings.remove(QString("%1/%2").arg(SETTINGS,ld2blColorsXRefKey));
            annOk[A_07] = false;
        }
    }

    if ((annOk[A_08] = Settings.contains(QString("%1/%2").arg(SETTINGS,ld2blCodesXRefKey)))) {
        ld2blCodesXRefFile = Settings.value(QString("%1/%2").arg(SETTINGS,ld2blCodesXRefKey)).toString();
        annInfo.setFile(ld2blCodesXRefFile);
        if (! annInfo.exists()) {
            Settings.remove(QString("%1/%2").arg(SETTINGS,ld2blCodesXRefKey));
            annOk[A_08] = false;
        }
    }

    if ((annOk[A_09] = Settings.contains(QString("%1/%2").arg(SETTINGS,ld2rbColorsXRefKey)))) {
        ld2rbColorsXRefFile = Settings.value(QString("%1/%2").arg(SETTINGS,ld2rbColorsXRefKey)).toString();
        annInfo.setFile(ld2rbColorsXRefFile);
        if (! annInfo.exists()) {
            Settings.remove(QString("%1/%2").arg(SETTINGS,ld2rbColorsXRefKey));
            annOk[A_09] = false;
        }
    }

    if ((annOk[A_10] = Settings.contains(QString("%1/%2").arg(SETTINGS,ld2rbCodesXRefKey)))) {
        ld2rbCodesXRefFile = Settings.value(QString("%1/%2").arg(SETTINGS,ld2rbCodesXRefKey)).toString();
        annInfo.setFile(ld2rbCodesXRefFile);
        if (! annInfo.exists()) {
            Settings.remove(QString("%1/%2").arg(SETTINGS,ld2rbCodesXRefKey));
            annOk[A_10] = false;
        }
    }

    if (annOk[A_01] && annOk[A_02] && annOk[A_03] && annOk[A_04] &&
        annOk[A_05] && annOk[A_06] && annOk[A_07] && annOk[A_08] &&
        annOk[A_09] && annOk[A_10])
        return;

    // Set Preference empty if file not found to trigger internal load or skip
    if (! annOk[A_04]) {
        blCodesFile = QString("%1/extras/%2").arg(lpubDataPath,VER_LPUB3D_BLCODES_FILE);
        annInfo.setFile(blCodesFile);
        if (annInfo.exists())
            Settings.setValue(QString("%1/%2").arg(SETTINGS,blCodesKey),blCodesFile);
        else
            blCodesFile.clear();
    }

    if (! annOk[A_05]) {
        legoElementsFile = QString("%1/extras/%2").arg(lpubDataPath,VER_LPUB3D_LEGOELEMENTS_FILE);
        annInfo.setFile(legoElementsFile);
        if (annInfo.exists())
            Settings.setValue(QString("%1/%2").arg(SETTINGS,legoElementsKey),legoElementsFile);
        else
            legoElementsFile.clear();
    }

    if (! annOk[A_06]) {
        blColorsFile = QString("%1/extras/%2").arg(lpubDataPath,VER_LPUB3D_BLCOLORS_FILE);
        annInfo.setFile(blColorsFile);
        if (annInfo.exists())
            Settings.setValue(QString("%1/%2").arg(SETTINGS,blColorsKey),blColorsFile);
        else
            blColorsFile.clear();
    }

    if (! annOk[A_07]) {
        ld2blColorsXRefFile = QString("%1/extras/%2").arg(lpubDataPath,VER_LPUB3D_LD2BLCOLORSXREF_FILE);
        annInfo.setFile(ld2blColorsXRefFile);
        if (annInfo.exists())
            Settings.setValue(QString("%1/%2").arg(SETTINGS,ld2blColorsXRefKey),ld2blColorsXRefFile);
        else
            ld2blColorsXRefFile.clear();
    }

    if (! annOk[A_08]) {
        ld2blCodesXRefFile = QString("%1/extras/%2").arg(lpubDataPath,VER_LPUB3D_LD2BLCODESXREF_FILE);
        annInfo.setFile(ld2blCodesXRefFile);
        if (annInfo.exists())
            Settings.setValue(QString("%1/%2").arg(SETTINGS,ld2blCodesXRefKey),ld2blCodesXRefFile);
        else
            ld2blCodesXRefFile.clear();
    }

    if (! annOk[A_09]) {
        ld2rbColorsXRefFile = QString("%1/extras/%2").arg(lpubDataPath,VER_LPUB3D_LD2RBCOLORSXREF_FILE);
        annInfo.setFile(ld2rbColorsXRefFile);
        if (annInfo.exists())
            Settings.setValue(QString("%1/%2").arg(SETTINGS,ld2rbColorsXRefKey),ld2rbColorsXRefFile);
        else
            ld2rbColorsXRefFile.clear();
    }

    if (! annOk[A_10]) {
        ld2rbCodesXRefFile = QString("%1/extras/%2").arg(lpubDataPath,VER_LPUB3D_LD2RBCODESXREF_FILE);
        annInfo.setFile(ld2rbCodesXRefFile);
        if (annInfo.exists())
            Settings.setValue(QString("%1/%2").arg(SETTINGS,ld2rbCodesXRefKey),ld2rbCodesXRefFile);
        else
            ld2rbCodesXRefFile.clear();
    }

    if (annOk[A_01] && annOk[A_02] && annOk[A_03])
        return;

    if (! annOk[A_01]) {
        titleAnnotationsFile = QString("%1/extras/%2").arg(lpubDataPath,validTitleAnnotations);
        annInfo.setFile(titleAnnotationsFile);
        if (annInfo.exists())
            Settings.setValue(QString("%1/%2").arg(SETTINGS,titleAnnotationKey),titleAnnotationsFile);
        else
            titleAnnotationsFile.clear();
    }

    if (! annOk[A_02]) {
        freeformAnnotationsFile = QString("%1/extras/%2").arg(lpubDataPath,validFreeFormAnnotations);
        annInfo.setFile(freeformAnnotationsFile);
        if (annInfo.exists())
            Settings.setValue(QString("%1/%2").arg(SETTINGS,freeFormAnnotationsKey),freeformAnnotationsFile);
        else
            freeformAnnotationsFile.clear();
    }

    if (! annOk[A_03]) {
        annotationStyleFile = QString("%1/extras/%2").arg(lpubDataPath,validAnnotationStyles);
        annInfo.setFile(annotationStyleFile);
        if (annInfo.exists())
            Settings.setValue(QString("%1/%2").arg(SETTINGS,annotationStyleKey),annotationStyleFile);
        else
            annotationStyleFile.clear();
    }
}

void Preferences::pliPreferences()
{
    QFileInfo pliInfo;
    QSettings Settings;
    enum P_OK { P_SUB, P_EXC, P_STK, P_CTL, NUM_OK };
    bool pliOk[NUM_OK] = { true, true, true, true };

    QString const pliSubstitutePartsKey("PliSubstitutePartsFile");
    QString const excludedPartsKey(     "ExcludedPartsFile");
    QString const stickerPartsKey(      "StickerPartsFile");
    QString const pliControlKey(        "PliControlFile");

    pliSubstitutePartsFile = QString("%1/extras/%2").arg(lpubDataPath,VER_PLI_SUBSTITUTE_PARTS_FILE);
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,pliSubstitutePartsKey)))
        pliSubstitutePartsFile = Settings.value(QString("%1/%2").arg(SETTINGS,pliSubstitutePartsKey)).toString();
    pliInfo.setFile(pliSubstitutePartsFile);
    if (! pliInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,pliSubstitutePartsKey));
        pliOk[P_SUB] = false;
    }

    excludedPartsFile = QString("%1/extras/%2").arg(lpubDataPath,VER_EXCLUDED_PARTS_FILE);
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,excludedPartsKey)))
        excludedPartsFile = Settings.value(QString("%1/%2").arg(SETTINGS,excludedPartsKey)).toString();
    pliInfo.setFile(excludedPartsFile);
    if (! pliInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,excludedPartsKey));
        pliOk[P_EXC] = false;
    }

    stickerPartsFile = QString("%1/extras/%2").arg(Preferences::lpubDataPath,VER_STICKER_PARTS_FILE);
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,stickerPartsKey)))
        stickerPartsFile = Settings.value(QString("%1/%2").arg(SETTINGS,stickerPartsKey)).toString();
    pliInfo.setFile(stickerPartsFile);
    if (! pliInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,stickerPartsKey));
        pliOk[P_STK] = false;
    }

    pliControlFile = QString("%1/extras/%2").arg(lpubDataPath,VER_PLI_CONTROL_FILE);
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,pliControlKey)))
        pliControlFile = Settings.value(QString("%1/%2").arg(SETTINGS,pliControlKey)).toString();
    pliInfo.setFile(pliControlFile);
    if (! pliInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,pliControlKey));
        pliOk[P_CTL] = false;
    }

    if (pliOk[P_SUB] && pliOk[P_EXC] && pliOk[P_STK] && pliOk[P_CTL])
        return;

    // Set Preference empty if file not found to trigger internal load or skip
    if (! pliOk[P_SUB]) {
        pliSubstitutePartsFile = QString("%1/extras/%2").arg(lpubDataPath,validPliSubstituteParts);
        pliInfo.setFile(pliSubstitutePartsFile);
        if (pliInfo.exists())
            Settings.setValue(QString("%1/%2").arg(SETTINGS,pliSubstitutePartsKey),pliSubstitutePartsFile);
        else
            pliSubstitutePartsFile.clear();
    }

    if (! pliOk[P_EXC]) {
        excludedPartsFile = QString("%1/extras/%2").arg(lpubDataPath,validExcludedPliParts);
        pliInfo.setFile(excludedPartsFile);
        if (pliInfo.exists())
            Settings.setValue(QString("%1/%2").arg(SETTINGS,excludedPartsKey),excludedPartsFile);
        else
            excludedPartsFile.clear();
    }

    if (! pliOk[P_STK]) {
        stickerPartsFile = QString("%1/extras/%2").arg(lpubDataPath,validStickerPliParts);
        pliInfo.setFile(stickerPartsFile);
        if (pliInfo.exists())
            Settings.setValue(QString("%1/%2").arg(SETTINGS,stickerPartsKey),stickerPartsFile);
        else
            stickerPartsFile.clear();
    }

    if (! pliOk[P_CTL]) {
        pliControlFile = QString("%1/extras/%2").arg(lpubDataPath,validPliControl);
        pliInfo.setFile(pliControlFile);
        if (pliInfo.exists())
            Settings.setValue(QString("%1/%2").arg(SETTINGS,pliControlKey),pliControlFile);
        else
            pliControlFile.clear();
    }
}

void Preferences::exportPreferences()
{
    QSettings Settings;
    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"IgnoreMixedPageSizesMsg"))) {
      QVariant uValue(false);
      ignoreMixedPageSizesMsg = false;
      Settings.setValue(QString("%1/%2").arg(DEFAULTS,"IgnoreMixedPageSizesMsg"),uValue);
    } else {
      if (modeGUI)
        ignoreMixedPageSizesMsg = Settings.value(QString("%1/%2").arg(DEFAULTS,"IgnoreMixedPageSizesMsg")).toBool();
      else
        ignoreMixedPageSizesMsg = false;
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"PdfPageImage"))) {
      QVariant uValue(false);
      pdfPageImage = false;
      Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PdfPageImage"),uValue);
    } else {
      pdfPageImage = Settings.value(QString("%1/%2").arg(DEFAULTS,"PdfPageImage")).toBool();
    }
}

void Preferences::publishingPreferences()
{
    QSettings Settings;

    //Page Display Pause
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"PageDisplayPause"))) {
        pageDisplayPause = PAGE_DISPLAY_PAUSE_DEFAULT;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"PageDisplayPause"),pageDisplayPause);
    } else {
        pageDisplayPause = Settings.value(QString("%1/%2").arg(SETTINGS,"PageDisplayPause")).toInt();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"DoNotShowPageProcessDlg"))) {
        QVariant pValue(false);
        doNotShowPageProcessDlg = false;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DoNotShowPageProcessDlg"),pValue);
    } else {
        if (modeGUI)
          doNotShowPageProcessDlg = Settings.value(QString("%1/%2").arg(DEFAULTS,"DoNotShowPageProcessDlg")).toBool();
        else
          doNotShowPageProcessDlg = true;
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"DisplayAllAttributes"))) {
        QVariant pValue(false);
        displayAllAttributes = false;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DisplayAllAttributes"),pValue);
    } else {
        displayAllAttributes = Settings.value(QString("%1/%2").arg(DEFAULTS,"DisplayAllAttributes")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"GenerateCoverPages"))) {
        QVariant pValue(false);
        generateCoverPages = false;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"GenerateCoverPages"),pValue);
    } else {
        generateCoverPages = Settings.value(QString("%1/%2").arg(DEFAULTS,"GenerateCoverPages")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"PrintDocumentTOC"))) {
        QVariant pValue(false);
        printDocumentTOC = false;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PrintDocumentTOC"),pValue);
    } else {
        printDocumentTOC = Settings.value(QString("%1/%2").arg(DEFAULTS,"PrintDocumentTOC")).toBool();
    }

    if (Settings.contains(QString("%1/%2").arg(DEFAULTS,"Email"))) {
        defaultEmail = Settings.value(QString("%1/%2").arg(DEFAULTS,"Email")).toString();
    }

    if (Settings.contains(QString("%1/%2").arg(DEFAULTS,"URL"))) {
        defaultURL = Settings.value(QString("%1/%2").arg(DEFAULTS,"URL")).toString();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"Author"))) {
        QVariant eValue(VER_PRODUCTNAME_STR);
        defaultAuthor = VER_PRODUCTNAME_STR;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"Author"),eValue);
    } else {
        defaultAuthor = Settings.value(QString("%1/%2").arg(DEFAULTS,"Author")).toString();
        if (defaultAuthor.isEmpty())
            defaultAuthor = VER_PRODUCTNAME_STR;
    }

    if (! Settings.contains(QString("%1/%2").arg(DEFAULTS,"PublishDescription"))) {
        QVariant eValue(PUBLISH_DESCRIPTION_DEFAULT);
        publishDescription = PUBLISH_DESCRIPTION_DEFAULT;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PublishDescription"),eValue);
    } else {
        publishDescription = Settings.value(QString("%1/%2").arg(DEFAULTS,"PublishDescription")).toString();
        if (publishDescription.isEmpty())
            publishDescription = PUBLISH_DESCRIPTION_DEFAULT;
    }

    if (Settings.contains(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile"))) {
        documentLogoFile = Settings.value(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile")).toString();
        QFileInfo fileInfo(documentLogoFile);
        if (!fileInfo.exists()) {
            Settings.remove(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile"));
            documentLogoFile.clear();
        }
    }
}

void Preferences::viewerPreferences()
{
    QSettings Settings;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,"ProjectsPath")))
        lcSetProfileString(LC_PROFILE_PROJECTS_PATH, Settings.value(QString("%1/%2").arg(SETTINGS,"ProjectsPath")).toString());

    if (povRayInstalled)
        lcSetProfileString(LC_PROFILE_POVRAY_PATH, povrayExe);

    if (!lgeoPath.isEmpty())
        lcSetProfileString(LC_PROFILE_POVRAY_LGEO_PATH, lgeoPath);

    if (!altLDConfigPath.isEmpty())
        lcSetProfileString(LC_PROFILE_COLOR_CONFIG, Preferences::altLDConfigPath);

    if (!defaultAuthor.isEmpty())
        lcSetProfileString(LC_PROFILE_DEFAULT_AUTHOR_NAME, defaultAuthor);

    if (!lpub3dLibFile.isEmpty())
        lcSetProfileString(LC_PROFILE_PARTS_LIBRARY, lpub3dLibFile);

    lcSetProfileInt(LC_PROFILE_NATIVE_PROJECTION, perspectiveProjection ? 0 : 1);

    lcSetProfileInt(LC_PROFILE_DRAW_AXES, 1);

    lcSetProfileInt(LC_PROFILE_GRID_LINES, 0);

    lcSetProfileInt(LC_PROFILE_GRID_STUDS, 0);

    lcSetProfileInt(LC_PROFILE_CHECK_UPDATES, 0);
}

void Preferences::keyboardShortcutPreferences()
{
    QSettings Settings;
    Settings.beginGroup(KEYBOARDSHORTCUTS);
    foreach (const QString &objectName, Settings.childKeys()) {
        QKeySequence keySequence = Settings.value(objectName, "").value<QKeySequence>();
        if (objectName.isEmpty())
            continue;
        keyboardShortcuts.insert(objectName, keySequence);
    }
    Settings.endGroup();
}

bool Preferences::getPreferences()
{
#ifdef Q_OS_MAC
    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isVisible())
        Application::instance()->splash->hide();
#endif

    const int fadeStepsOpacityCompare            = fadeStepsOpacity;
    const int  highlightStepLineWidthCompare     = highlightStepLineWidth;
    const bool povFileGeneratorCompare           = useNativePovGenerator;
    const int preferredRendererCompare           = preferredRenderer;
    const int povrayRenderQualityCompare         = povrayRenderQuality;
    const QString fadeStepsColourCompare         = validFadeStepsColour;
    const QString highlightStepColourCompare     = highlightStepColour;
    const QString ldrawPathCompare               = ldrawLibPath;
    const QString lgeoPathCompare                = lgeoPath;
    const QString sceneBackgroundColorCompare    = sceneBackgroundColor;
    const QString sceneGridColorCompare          = sceneGridColor;
    const QString sceneRulerTickColorCompare     = sceneRulerTickColor;
    const QString sceneRulerTrackingColorCompare = sceneRulerTrackingColor;
    const QString sceneGuideColorCompare         = sceneGuideColor;

    lcLibRenderOptions Options;
    int CurrentAASamples = lcGetProfileInt(LC_PROFILE_ANTIALIASING_SAMPLES);
    lcStudStyle CurrentStudStyle = lcGetPiecesLibrary()->GetStudStyle();

    Options.Preferences = gApplication->mPreferences;
    Options.KeyboardShortcuts = keyboardShortcuts;

    Options.AASamples = CurrentAASamples;
    Options.StudStyle = CurrentStudStyle;

    Options.Preferences.mShadingMode   = (lcShadingMode)lcGetProfileInt(LC_PROFILE_SHADING_MODE);
    Options.Preferences.mDrawEdgeLines = lcGetProfileInt(LC_PROFILE_DRAW_EDGE_LINES);
    Options.Preferences.mLineWidth	   = lcGetProfileFloat(LC_PROFILE_LINE_WIDTH);

    Options.Preferences.mNativeViewpoint = lcGetProfileInt(LC_PROFILE_NATIVE_VIEWPOINT);
    Options.Preferences.mNativeProjection = lcGetProfileInt(LC_PROFILE_NATIVE_PROJECTION);

    Options.Preferences.mLPubTrueFade = lcGetProfileInt(LC_PROFILE_LPUB_TRUE_FADE);
    Options.Preferences.mDrawConditionalLines = lcGetProfileInt(LC_PROFILE_DRAW_CONDITIONAL_LINES);

    Options.KeyboardShortcutsModified = false;
    Options.KeyboardShortcutsDefault = false;

    bool returnResult = true;
    bool suspendFileDisplay = false;
    bool updateLDViewConfigFiles = false;

    PreferencesDialog *dialog = lpub->preferencesDialog;

    dialog->setOptions(&Options);

    // 'LDView INI settings
    if (preferredRenderer == RENDERER_POVRAY) {
        if (useNativePovGenerator)
            TCUserDefaults::setIniFile(nativeExportIni.toLatin1().constData());
        else
            TCUserDefaults::setIniFile(ldviewPOVIni.toLatin1().constData());
    } else if (preferredRenderer == RENDERER_LDVIEW) {
        TCUserDefaults::setIniFile(ldviewIni.toLatin1().constData());
    }

    if (dialog->exec() == QDialog::Accepted) {

        QElapsedTimer timer;
        timer.start();

        QMessageBox box;
        box.setMinimumSize(40,20);
        box.setIcon (QMessageBox::Information);
        box.setDefaultButton   (QMessageBox::Ok);
        box.setStandardButtons (QMessageBox::Ok);

        bool ldrawPathChanged     = false;
        bool PerspectiveProjectionChanged = false;

        QSettings Settings;

        // library paths
        if ((ldrawPathChanged = ldrawLibPath.toLower() != dialog->ldrawLibPath().toLower())) {
            ldrawLibPath = dialog->ldrawLibPath();
            if (ldrawLibPath.isEmpty()) {
                Settings.remove(QString("%1/%2").arg(SETTINGS,ldrawLibPathKey));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawLibPathKey),ldrawLibPath);
            }
            // update LDView ini files
            updateLDVExportIniFile(UpdateExisting);
            updateLDViewIniFile(UpdateExisting);       //ldraw path changed
            updateLDViewPOVIniFile(UpdateExisting);    //ldraw or lgeo paths changed
            updateLDViewConfigFiles = true;            //set flag to true

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("LDraw Library path changed from %1 to %2")
                            .arg(ldrawPathCompare)
                            .arg(ldrawLibPath));
            if (validLDrawLibrary != validLDrawLibraryChange) {
                libraryChangeRestart = true;
                emit lpub->messageSig(LOG_INFO,QMessageBox::tr("LDraw parts library changed from %1 to %2")
                                      .arg(validLDrawLibrary)
                                      .arg(validLDrawLibraryChange));
                box.setText (QMessageBox::tr("%1 will restart to properly load the %2 parts library.")
                                             .arg(VER_PRODUCTNAME_STR).arg(validLDrawLibraryChange));
                box.exec();
            }
        }

        if (autoUpdateChangeLog != dialog->autoUpdateChangeLog())
        {
            autoUpdateChangeLog = dialog->autoUpdateChangeLog();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"AutoUpdateChangeLog"),autoUpdateChangeLog);
        }

        if ((reloadFile = altLDConfigPath != dialog->altLDConfigPath()))
        {
            libraryChangeRestart = true;
            altLDConfigPath = dialog->altLDConfigPath();
            if (altLDConfigPath.isEmpty()) {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"AltLDConfigPath"));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"AltLDConfigPath"),altLDConfigPath);
            }

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Use Alternate LDConfig (Restart Required) %1.").arg(altLDConfigPath));
            box.setText (QMessageBox::tr("%1 will restart to properly load the alternate LDConfig file.").arg(VER_PRODUCTNAME_STR));
            box.exec();
        }

        if (pliControlFile != dialog->pliControlFile()) {
            pliControlFile = dialog->pliControlFile();
            if (pliControlFile.isEmpty()) {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"PliControlFile"));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"PliControlFile"),pliControlFile);
            }
        }

        if ((reloadFile |= preferredRenderer != dialog->preferredRenderer())) {
            preferredRenderer = dialog->preferredRenderer();
            initPreferredRenderer = preferredRenderer;
            if (preferredRenderer == RENDERER_INVALID) {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"PreferredRenderer"));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"PreferredRenderer"),rendererNames[preferredRenderer]);
            }

            Render::setRenderer(preferredRenderer);
            if (preferredRenderer == RENDERER_LDGLITE)
                lpub->partWorkerLDSearchDirs().populateLdgLiteSearchDirs();

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Renderer preference changed from %1 to %2%3")
                            .arg(preferredRendererCompare)
                            .arg(preferredRenderer)
                            .arg(preferredRenderer == RENDERER_POVRAY ? QMessageBox::tr(" (POV file generator is %1)")
                                                                                             .arg(useNativePovGenerator ? RENDERER_NATIVE : RENDERER_LDVIEW) :
                                 preferredRenderer == RENDERER_LDVIEW ? enableLDViewSingleCall ?
                                                                                     enableLDViewSnaphsotList ? QMessageBox::tr(" (Single Call using Export File List)") :
                                                                                                                             QMessageBox::tr(" (Single Call)") :
                                                                                                                             QString() : QString()));
        }

        if (enableLDViewSingleCall != dialog->enableLDViewSingleCall()) {
            enableLDViewSingleCall = dialog->enableLDViewSingleCall();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableLDViewSingleCall"),enableLDViewSingleCall);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Enable LDView Single Call is %1")
                                  .arg(enableLDViewSingleCall ? "ON" : "OFF"));

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Enable LDView Snapshots List is %1")
                                  .arg(enableLDViewSnaphsotList ? "ON" : "OFF"));
        }

        if (enableLDViewSnaphsotList != dialog->enableLDViewSnaphsotList()) {
            enableLDViewSnaphsotList = dialog->enableLDViewSnaphsotList();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableLDViewSnapshotsList"),enableLDViewSnaphsotList);
        }

        if (lgeoPath != dialog->lgeoPath()) {
            lgeoPath = dialog->lgeoPath();
            if(lgeoPath.isEmpty()) {
                Settings.remove(QString("%1/%2").arg(POVRAY,"LGEOPath"));
            } else {
                Settings.setValue(QString("%1/%2").arg(POVRAY,"LGEOPath"),lgeoPath);
            }
            // update LDView ini files
            if (!updateLDViewConfigFiles) {
                updateLDViewPOVIniFile(UpdateExisting);    //ldraw or lgeo paths changed
            }
            updatePOVRayConfFile(UpdateExisting);          //lgeo path changed

            if (!ldrawPathChanged)
                emit lpub->messageSig(LOG_INFO,QMessageBox::tr("LGEO path preference changed from %1 to %2")
                                .arg(lgeoPathCompare)
                                .arg(lgeoPath));
        }

        if (useNativePovGenerator != dialog->useNativePovGenerator())
        {
            useNativePovGenerator = dialog->useNativePovGenerator();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"UseNativePovGenerator"),useNativePovGenerator);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("POV file generation renderer changed from %1 to %2")
                                             .arg(povFileGeneratorCompare ? RENDERER_NATIVE : RENDERER_LDVIEW)
                                             .arg(useNativePovGenerator ? RENDERER_NATIVE : RENDERER_LDVIEW));
        }

        if (povrayDisplay != dialog->povrayDisplay())
        {
            povrayDisplay = dialog->povrayDisplay();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"POVRayDisplay"),povrayDisplay);
        }

        if (povrayRenderQuality != dialog->povrayRenderQuality())
        {
            povrayRenderQuality = dialog->povrayRenderQuality();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"PovrayRenderQuality"),povrayRenderQuality);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Povray Render Quality changed from %1 to %2")
                            .arg(povrayRenderQualityCompare == 0 ? "High" :
                                 povrayRenderQualityCompare == 1 ? "Medium" : "Low")
                            .arg(povrayRenderQuality == 0 ? "High" :
                                 povrayRenderQuality == 1 ? "Medium" : "Low"));

            reloadFile |= preferredRenderer == RENDERER_POVRAY;
        }

        if (povrayAutoCrop != dialog->povrayAutoCrop())
        {
            povrayAutoCrop = dialog->povrayAutoCrop();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"PovrayAutoCrop"),povrayAutoCrop);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Povray AutoCrop is %1")
                                  .arg(povrayAutoCrop ? "ON" : "OFF"));
        }

        if (rendererTimeout != dialog->rendererTimeout()) {
            rendererTimeout = dialog->rendererTimeout();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"RendererTimeout"),rendererTimeout);
        }

        if (pageDisplayPause != dialog->pageDisplayPause()) {
            pageDisplayPause = dialog->pageDisplayPause();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"PageDisplayPause"),pageDisplayPause);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Continuous process page display pause changed from %1 to %2")
                            .arg(highlightStepLineWidthCompare)
                            .arg(pageDisplayPause));
        }

        if (!dialog->documentLogoFile().isEmpty()) {
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile"),dialog->documentLogoFile());
            if (documentLogoFile.isEmpty()) {
                documentLogoFile = dialog->documentLogoFile();
            }
        }

        if (!dialog->defaultAuthor().isEmpty()) {
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"Author"),dialog->defaultAuthor());
            if (defaultAuthor.isEmpty()) {
                defaultAuthor = dialog->defaultAuthor();
            }
        }

        if (!dialog->defaultURL().isEmpty()) {
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"URL"),dialog->defaultURL());
            if (!defaultURL.isEmpty()) {
                defaultURL = dialog->defaultURL();
            }
        }

        if (!dialog->defaultEmail().isEmpty()) {
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"Email"),dialog->defaultEmail());
            if (defaultEmail.isEmpty()) {
                defaultEmail = dialog->defaultEmail();
            }
        }

        if (!dialog->publishDescription().isEmpty()) {
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PublishDescription"),dialog->publishDescription());
            if (publishDescription.isEmpty()) {
                publishDescription = dialog->publishDescription();
            }
        }

        if (lineParseErrors != dialog->showLineParseErrors())
        {
            lineParseErrors = dialog->showLineParseErrors();
            Settings.setValue(QString("%1/%2").arg(MESSAGES,"ShowLineParseErrors"),lineParseErrors);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Show Parse Errors is %1")
                                  .arg(lineParseErrors? "ON" : "OFF"));
        }

        if (showInsertErrors != dialog->showInsertErrors())
        {
            showInsertErrors = dialog->showInsertErrors();
            Settings.setValue(QString("%1/%2").arg(MESSAGES,"ShowInsertErrors"),showInsertErrors);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Show Insert Errors is %1")
                                  .arg(showInsertErrors    ? "ON" : "OFF"));
        }

        if (showBuildModErrors != dialog->showBuildModErrors())
        {
            showBuildModErrors = dialog->showBuildModErrors();
            Settings.setValue(QString("%1/%2").arg(MESSAGES,"ShowBuildModErrors"),showBuildModErrors);
        }

        if (showIncludeFileErrors != dialog->showIncludeFileErrors())
        {
            showIncludeFileErrors = dialog->showIncludeFileErrors();
            Settings.setValue(QString("%1/%2").arg(MESSAGES,"ShowIncludeFileErrors"),showIncludeFileErrors);
        }

        if (showAnnotationErrors != dialog->showAnnotationErrors())
        {
            showAnnotationErrors = dialog->showAnnotationErrors();
            Settings.setValue(QString("%1/%2").arg(MESSAGES,"ShowAnnotationErrors"),showAnnotationErrors);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Show Parse Errors is %1")
                                  .arg(showAnnotationErrors? "ON" : "OFF"));
        }

        if (showSaveOnRedraw != dialog->showSaveOnRedraw())
        {
            showSaveOnRedraw = dialog->showSaveOnRedraw();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ShowSaveOnRedraw"),showSaveOnRedraw);
        }

        if (showSaveOnUpdate != dialog->showSaveOnUpdate())
        {
            showSaveOnUpdate = dialog->showSaveOnUpdate();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ShowSaveOnUpdate"),showSaveOnUpdate);
        }

        // fade previous steps and highlight current step
        bool fadeStepsOpacityChanged = false;
        if ((fadeStepsOpacityChanged = fadeStepsOpacity != dialog->fadeStepsOpacity()))
        {
            fadeStepsOpacity = dialog->fadeStepsOpacity();
            initFadeStepsOpacity = fadeStepsOpacity;
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepsOpacity"),fadeStepsOpacity);

            if (dialog->enableFadeSteps())
                emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Fade Steps Transparency changed from %1 to %2 percent")
                                .arg(fadeStepsOpacityCompare)
                                .arg(fadeStepsOpacity));
        }

        bool fadeStepsUseColourChanged = false;
        if ((fadeStepsUseColourChanged = fadeStepsUseColour != dialog->fadeStepsUseColour()))
        {
            fadeStepsUseColour = dialog->fadeStepsUseColour();
            initFadeStepsUseColour = fadeStepsUseColour;
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepsUseColour"),fadeStepsUseColour);

            if (dialog->enableFadeSteps())
                emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Use Global Fade Color is %1").arg(fadeStepsUseColour ? "ON" : "OFF"));
        }

        bool fadeStepsColourChanged = false;
        if ((fadeStepsColourChanged = validFadeStepsColour != dialog->fadeStepsColour()))
        {
            validFadeStepsColour = dialog->fadeStepsColour();
            initValidFadeStepsColour = validFadeStepsColour;
            Settings.setValue(QString("%1/%2").arg(SETTINGS,fadeStepsColourKey),validFadeStepsColour);

            if (dialog->enableFadeSteps() && fadeStepsUseColour)
                emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Fade Steps Color preference changed from %1 to %2")
                                      .arg(QString(fadeStepsColourCompare).replace("_"," "))
                                      .arg(QString(validFadeStepsColour).replace("_"," ")));
        }

        bool highlightStepColorChanged = false;
        if ((highlightStepColorChanged = highlightStepColour != dialog->highlightStepColour()))
        {
            highlightStepColour = dialog->highlightStepColour();
            initHighlightStepColour = highlightStepColour;
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"HighlightStepColor"),highlightStepColour);

            if (dialog->enableHighlightStep())
                emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Highlight Step Color preference changed from %1 to %2")
                                .arg(highlightStepColourCompare)
                                .arg(highlightStepColour));
        }

        bool highlightStepLineWidthChanged = false;
        if (dialog->enableHighlightStep() && (highlightStepLineWidth != dialog->highlightStepLineWidth()))
        {
            highlightStepLineWidthChanged = true;
            highlightStepLineWidth = dialog->highlightStepLineWidth();
            initHighlightStepLineWidth = highlightStepLineWidth;
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"HighlightStepLineWidth"),highlightStepLineWidth);

            if (dialog->enableHighlightStep())
                emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Highlight Step line width changed from %1 to %2")
                                .arg(highlightStepLineWidthCompare)
                                .arg(highlightStepLineWidth));
        }

        if ((reloadFile |= highlightFirstStep != dialog->highlightFirstStep()))
        {
            highlightFirstStep = dialog->highlightFirstStep();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"HighlightFirstStep"),highlightFirstStep);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Highlight First Step is %1")
                                  .arg(highlightFirstStep ? "ON" : "OFF"));
        }

        if ((reloadFile |= enableImageMatting != dialog->enableImageMatting()))
        {
            enableImageMatting = dialog->enableImageMatting();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableImageMatting"),enableImageMatting);
            if (enableImageMatting)
                updateLDViewIniFile(UpdateExisting);       // strip AutoCrop [disabled]

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Enable image matting is %1")
                                  .arg(enableImageMatting ? "ON" : "OFF"));
        }

        if ((reloadFile |= preferCentimeters != dialog->centimeters()))
        {
            preferCentimeters = dialog->centimeters();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"Centimeters"),preferCentimeters);
            defaultResolutionType(preferCentimeters);

            emit lpub->messageSig(LOG_INFO,QString("Default units changed to %1")
                                  .arg(preferCentimeters? "Centimetres" : "Inches"));
        }

        bool addLSynthSearchDirChanged = false;
        if ((addLSynthSearchDirChanged = addLSynthSearchDir != dialog->addLSynthSearchDir()))
        {
            addLSynthSearchDir = dialog->addLSynthSearchDir();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"AddLSynthSearchDir"),addLSynthSearchDir);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Add LSynth Search Directory is %1")
                                  .arg(addLSynthSearchDir? "ON" : "OFF"));
        }

        bool archiveLSynthPartsChanged = false;
        if ((archiveLSynthPartsChanged = archiveLSynthParts != dialog->archiveLSynthParts()))
        {
            archiveLSynthParts = dialog->archiveLSynthParts();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ArchiveLSynthParts"),archiveLSynthParts);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Archive LSynth Parts is %1")
                                  .arg(archiveLSynthParts? "ON" : "OFF"));
        }

        if ((addLSynthSearchDirChanged || archiveLSynthPartsChanged) && archiveLSynthParts)
            lpub->mi.loadLDSearchDirParts();

        if (applyCALocally != dialog->applyCALocally())
        {
            applyCALocally = dialog->applyCALocally();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ApplyCALocally"),applyCALocally);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Apply camera angles locally is %1")
                                  .arg(applyCALocally ? "ON" : "OFF"));
        }

        if (enableDownloader != dialog->enableDownloader()) {
            enableDownloader = dialog->enableDownloader();
            Settings.setValue(QString("%1/%2").arg(UPDATES,"EnableDownloader"),enableDownloader);
        }

        if (showDownloadRedirects != dialog->showDownloadRedirects()) {
            showDownloadRedirects = dialog->showDownloadRedirects();
            Settings.setValue(QString("%1/%2").arg(UPDATES,"ShowDownloadRedirects"),showDownloadRedirects);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Show download redirects is %1")
                                  .arg(showDownloadRedirects? "ON" : "OFF"));
        }

        if (showUpdateNotifications != dialog->showUpdateNotifications()) {
            showUpdateNotifications = dialog->showUpdateNotifications();
            Settings.setValue(QString("%1/%2").arg(UPDATES,"ShowUpdateNotifications"),showUpdateNotifications);
        }

        if (showAllNotifications != dialog->showAllNotifications()) {
            showAllNotifications = dialog->showAllNotifications();
            Settings.setValue(QString("%1/%2").arg(UPDATES,"ShowAllNotifications"),showAllNotifications);
        }

        if (checkUpdateFrequency != dialog->checkUpdateFrequency()) {
            checkUpdateFrequency = dialog->checkUpdateFrequency();
            Settings.setValue(QString("%1/%2").arg(UPDATES,"CheckUpdateFrequency"),checkUpdateFrequency);
        }

        if (displayAllAttributes != dialog->displayAllAttributes()) {
            displayAllAttributes = dialog->displayAllAttributes();
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DisplayAllAttributes"),displayAllAttributes);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Display Attributes is %1")
                                  .arg(displayAllAttributes? "ON" : "OFF"));
        }

        if ((reloadFile |= generateCoverPages != dialog->generateCoverPages())) {
            generateCoverPages = dialog->generateCoverPages();
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"GenerateCoverPages"),generateCoverPages);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Generate Cover Pages preference is %1")
                                  .arg(generateCoverPages ? "ON" : "OFF"));
        }

        if (printDocumentTOC != dialog->printDocumentTOC()) {
            printDocumentTOC = dialog->printDocumentTOC();
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PrintDocumentTOC"),printDocumentTOC);
        }

        if (doNotShowPageProcessDlg != dialog->doNotShowPageProcessDlg()) {
            doNotShowPageProcessDlg = dialog->doNotShowPageProcessDlg();
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DoNotShowPageProcessDlg"),doNotShowPageProcessDlg);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Show continuous page process options dialog is %1.")
                                  .arg(doNotShowPageProcessDlg ? "ON" : "OFF"));
        }

        bool darkTheme = displayTheme == THEME_DARK;
        bool displayThemeChanged = false;
        if ((displayThemeChanged = displayTheme != dialog->displayTheme())){
            displayTheme = dialog->displayTheme();
            darkTheme  = displayTheme == THEME_DARK;
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"DisplayTheme"),displayTheme);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Display Theme changed from %1 to %2.")
                                  .arg(darkTheme ? "Dark Theme" : "Default Theme")
                                  .arg(darkTheme ? "Default Theme" : "Dark Theme"));
        }

        bool displayThemeColorsChanged = false;
        if ((displayThemeColorsChanged = dialog->themeColours().size())) {

            QMap<int, QString>::ConstIterator i = dialog->themeColours().constBegin();
            bool textDecorationColorChanged = false;
            while (i != dialog->themeColours().constEnd()) {
                if (!textDecorationColorChanged)
                    if ((darkTheme && i.key() >= THEME_DEFAULT_DECORATE_LDRAW_COMMENTS && i.key() < THEME_DARK_SCENE_BACKGROUND_COLOR) ||
                                     (i.key() >= THEME_DARK_DECORATE_LDRAW_COMMENTS && i.key() < THEME_DARK_PALETTE_WINDOW))
                        reloadFile = textDecorationColorChanged = true;
                themeColors[i.key()] = i.value().toUpper();
                const QString themeKey(defaultThemeColors[i.key()].key);
                Settings.setValue(QString("%1/%2").arg(THEMECOLORS,themeKey),themeColors[i.key()]);
                ++i;
            }

            emit lpub->messageSig(LOG_INFO,QString("Display theme colors have changed"));
            if (textDecorationColorChanged)
                emit lpub->messageSig(LOG_INFO,QString("Text Decoration color have changed"));
        }

        bool sceneBackgroundColorChanged = false;
        if ((sceneBackgroundColorChanged = sceneBackgroundColor != dialog->sceneBackgroundColor())){
            sceneBackgroundColor = dialog->sceneBackgroundColor();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneBackgroundColor"),sceneBackgroundColor);
            bool customColor = (sceneGuideColor != (darkTheme ? themeColors[THEME_DARK_SCENE_BACKGROUND_COLOR] :
                                                                themeColors[THEME_DEFAULT_SCENE_BACKGROUND_COLOR]));
            setCustomSceneBackgroundColorPreference(customColor);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Scene Background Color changed from %1 to %2")
                            .arg(sceneBackgroundColorCompare)
                            .arg(sceneBackgroundColor));
        }

        bool sceneGridColorChanged = false;
        if ((reloadPage = sceneGridColor != dialog->sceneGridColor())) {
            sceneGridColorChanged = true;
            sceneGridColor = dialog->sceneGridColor();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneGridColor"),sceneGridColor);
            bool customColor = (sceneGridColor != (darkTheme ? themeColors[THEME_DARK_GRID_PEN] :
                                                               themeColors[THEME_DEFAULT_GRID_PEN]));
            setCustomSceneGridColorPreference(customColor);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Scene Grid Color changed from %1 to %2")
                                  .arg(sceneGridColorCompare)
                                  .arg(sceneGridColor));
        }

        bool sceneRulerTickColorChanged = false;
        if ((sceneRulerTickColorChanged = sceneRulerTickColor != dialog->sceneRulerTickColor())) {
            sceneRulerTickColor = dialog->sceneRulerTickColor();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneRulerTickColor"),sceneRulerTickColor);
            bool customColor = (sceneRulerTickColor != (darkTheme ? themeColors[THEME_DARK_RULER_TICK_PEN] :
                                                                    themeColors[THEME_DEFAULT_RULER_TICK_PEN]));
            setCustomSceneRulerTickColorPreference(customColor);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Scene Ruler Tick Color changed from %1 to %2")
                                  .arg(sceneRulerTickColorCompare)
                                  .arg(sceneRulerTickColor));
        }

        if (sceneRulerTrackingColor != dialog->sceneRulerTrackingColor()) {
            sceneRulerTrackingColor = dialog->sceneRulerTrackingColor();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneRulerTrackingColor"),sceneRulerTrackingColor);
            bool customColor = (sceneRulerTrackingColor != (darkTheme ? themeColors[THEME_DARK_RULER_TRACK_PEN] :
                                                                        themeColors[THEME_DEFAULT_RULER_TRACK_PEN]));
            setCustomSceneRulerTrackingColorPreference(customColor);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Scene Ruler Tracking Color changed from %1 to %2")
                                  .arg(sceneRulerTrackingColorCompare)
                                  .arg(sceneRulerTrackingColor));
        }

        bool sceneGuideColorChanged = false;
        if ((sceneGuideColorChanged = sceneGuideColor != dialog->sceneGuideColor())){
            sceneGuideColor = dialog->sceneGuideColor();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneGuideColor"),sceneGuideColor);
            bool customColor = (sceneGuideColor != (darkTheme ? themeColors[THEME_DARK_GUIDE_PEN] :
                                                                themeColors[THEME_DEFAULT_GUIDE_PEN]));
            setCustomSceneGuideColorPreference(customColor);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Scene Guide Color changed from %1 to %2")
                            .arg(sceneGuideColorCompare)
                            .arg(sceneGuideColor));
        }

        if (dialog->resetSceneColors()) {
            if (darkTheme) {
                sceneBackgroundColor    = defaultThemeColors[THEME_DARK_SCENE_BACKGROUND_COLOR].color;
                sceneGridColor          = defaultThemeColors[THEME_DARK_GRID_PEN].color;
                sceneRulerTickColor     = defaultThemeColors[THEME_DARK_RULER_TICK_PEN].color;
                sceneRulerTrackingColor = defaultThemeColors[THEME_DARK_RULER_TRACK_PEN].color;
                sceneGuideColor         = defaultThemeColors[THEME_DARK_GUIDE_PEN].color;
            } else {
                sceneBackgroundColor    = defaultThemeColors[THEME_DEFAULT_SCENE_BACKGROUND_COLOR].color;
                sceneGridColor          = defaultThemeColors[THEME_DEFAULT_GRID_PEN].color;
                sceneRulerTickColor     = defaultThemeColors[THEME_DEFAULT_RULER_TICK_PEN].color;
                sceneRulerTrackingColor = defaultThemeColors[THEME_DEFAULT_RULER_TRACK_PEN].color;
                sceneGuideColor         = defaultThemeColors[THEME_DEFAULT_GUIDE_PEN].color;
            }
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneBackgroundColor"),sceneBackgroundColor);
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneGridColor"),sceneGridColor);
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneRulerTickColor"),sceneRulerTickColor);
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneRulerTrackingColor"),sceneRulerTrackingColor);
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneGuideColor"),sceneGuideColor);
        }

        if (moduleVersion != dialog->moduleVersion()){
            moduleVersion = dialog->moduleVersion();
        }

        if (includeLogLevel != dialog->includeLogLevel())
        {
            includeLogLevel = dialog->includeLogLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeLogLevel"),includeLogLevel);
        }

        if (includeTimestamp != dialog->includeTimestamp())
        {
            includeTimestamp = dialog->includeTimestamp();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeTimestamp"),includeTimestamp);
        }

        if (includeLineNumber != dialog->includeLineNumber())
        {
            includeLineNumber = dialog->includeLineNumber();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeLineNumber"),includeLineNumber);
        }

        if (includeFileName != dialog->includeFileName())
        {
            includeFileName = dialog->includeFileName();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeFileName"),includeFileName);
        }

        if (includeAllLogAttributes != dialog->includeAllLogAttrib())
        {
            includeAllLogAttributes = dialog->includeAllLogAttrib();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeAllLogAttributes"),includeAllLogAttributes);
        }

        bool loggingChanged = false;
        if ((loggingChanged |= logging != dialog->loggingGrpBox()))
        {
            logging = dialog->loggingGrpBox();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"Logging"),logging);
        }

        if ((loggingChanged |= loggingLevel != dialog->logLevelCombo()))
        {
            loggingLevel = dialog->logLevelCombo();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"LoggingLevel"),loggingLevel);
        }

        if ((loggingChanged |= logLevel != dialog->logLevelGrpBox()))
        {
            logLevel = dialog->logLevelGrpBox();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"LogLevel"),logLevel);
        }

        if ((loggingChanged |= logLevels != dialog->logLevelsGrpBox()))
        {
            logLevels = dialog->logLevelsGrpBox();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"LogLevels"),logLevels);
        }

        if ((loggingChanged |= debugLevel != dialog->debugLevel()))
        {
            debugLevel = dialog->debugLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"DebugLevel"),debugLevel);
        }

        if ((loggingChanged |= traceLevel != dialog->traceLevel()))
        {
            traceLevel = dialog->traceLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"TraceLevel"),traceLevel);
        }

        if ((loggingChanged |= noticeLevel != dialog->noticeLevel()))
        {
            noticeLevel = dialog->noticeLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"NoticeLevel"),noticeLevel);
        }

        if ((loggingChanged |= infoLevel != dialog->infoLevel()))
        {
            infoLevel = dialog->infoLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"InfoLevel"),infoLevel);
        }

        if ((loggingChanged |= statusLevel != dialog->statusLevel()))
        {
            statusLevel = dialog->statusLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"StatusLevel"),statusLevel);
        }

        if ((loggingChanged |= warningLevel != dialog->warningLevel()))
        {
            warningLevel = dialog->warningLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"WarningLevel"),warningLevel);
        }

        if ((loggingChanged |= errorLevel != dialog->errorLevel()))
        {
            errorLevel = dialog->errorLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"ErrorLevel"),errorLevel);
        }

        if ((loggingChanged |= fatalLevel != dialog->fatalLevel()))
        {
            fatalLevel = dialog->fatalLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"FatalLevel"),fatalLevel);
        }

        if ((loggingChanged |= allLogLevels != dialog->allLogLevels()))
        {
            allLogLevels = dialog->allLogLevels();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"AllLogLevels"),allLogLevels);
        }

        if (loggingChanged) {
            using namespace QsLogging;
            Logger& logger = Logger::instance();
            if (logging) {
                if (logLevels) {
                    logger.setLoggingLevels();
                    logger.setDebugLevel(debugLevel);
                    logger.setTraceLevel(traceLevel);
                    logger.setNoticeLevel(noticeLevel);
                    logger.setInfoLevel(infoLevel);
                    logger.setStatusLevel(statusLevel);
                    logger.setWarningLevel(warningLevel);
                    logger.setErrorLevel(errorLevel);
                    logger.setFatalLevel(fatalLevel);
                } else if (logLevel) {
                    bool ok;
                    Level logLevel = logger.fromLevelString(loggingLevel,&ok);
                    if (!ok) {
                        QString Message = QMessageBox::tr("Failed to set log level %1.\nLogging is off - level set to OffLevel").arg(loggingLevel);
                        if (modeGUI)
                            QMessageBox::critical(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR), Message);
                        else
                            fprintf(stderr, "%s", Message.toLatin1().constData());
                    }
                    logger.setLoggingLevel(logLevel);
                }
                logger.setIncludeLogLevel(includeLogLevel);
                logger.setIncludeTimestamp(includeTimestamp);
                logger.setIncludeLineNumber(includeLineNumber);
                logger.setIncludeFileName(includeFileName);
                logger.setIncludeFunctionInfo(includeFunction);
            } else {
                logger.setLoggingLevel(OffLevel);
            }
        }

        if ((reloadFile |= perspectiveProjection != dialog->perspectiveProjection()))
        {
            PerspectiveProjectionChanged = true;
            perspectiveProjection = dialog->perspectiveProjection();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"PerspectiveProjection"),perspectiveProjection);

            lcSetProfileInt(LC_PROFILE_NATIVE_PROJECTION, perspectiveProjection ? 0 : 1); /*0,1 refers to is Orthographic*/

            gApplication->mPreferences.mNativeProjection = lcGetProfileInt(LC_PROFILE_NATIVE_PROJECTION);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Projection set to %1")
                                  .arg(perspectiveProjection ? "Perspective" : "Orthographic"));
        }

        if (saveOnRedraw != dialog->saveOnRedraw())
        {
            saveOnRedraw = dialog->saveOnRedraw();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SaveOnRedraw"),saveOnRedraw);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Save On Redraw is %1")
                                  .arg(saveOnRedraw? "ON" : "OFF"));
        }

        if (saveOnUpdate != dialog->saveOnUpdate())
        {
            saveOnUpdate = dialog->saveOnUpdate();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SaveOnUpdate"),saveOnUpdate);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Save On Update is %1")
                                  .arg(saveOnUpdate? "ON" : "OFF"));
        }

        if (loadLastOpenedFile != dialog->loadLastOpenedFile())
        {
            loadLastOpenedFile = dialog->loadLastOpenedFile();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"LoadLastOpenedFile"),loadLastOpenedFile);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Load Last Opened File is %1")
                                  .arg(loadLastOpenedFile ? "ON" : "OFF"));
        }

        if (extendedSubfileSearch != dialog->extendedSubfileSearch())
        {
            extendedSubfileSearch = dialog->extendedSubfileSearch();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ExtendedSubfileSearch"),extendedSubfileSearch);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Extended Subfile Search is %1")
                                  .arg(extendedSubfileSearch ? "ON" : "OFF"));
        }

        if (ldrawFilesLoadMsgs != dialog->ldrawFilesLoadMsgs())
        {
            ldrawFilesLoadMsgs = dialog->ldrawFilesLoadMsgs();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDrawFilesLoadMsgs"),ldrawFilesLoadMsgs);

            emit lpub->messageSig(LOG_INFO,QString("LDraw file load status dialogue set to %1").arg(
                ldrawFilesLoadMsgs == NEVER_SHOW ? "Never Show" :
                ldrawFilesLoadMsgs == SHOW_ERROR ? "Show Error" :
                ldrawFilesLoadMsgs == SHOW_WARNING ? "Show Warning" :
                ldrawFilesLoadMsgs == SHOW_MESSAGE ? "Show Message" :
                "Always Show"));
        }

        if (inlineNativeContent != dialog->inlineNativeContent())
        {
            inlineNativeContent = dialog->inlineNativeContent();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"InlineNativeContent"),inlineNativeContent);

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Inline Native Render Content is %1")
                                  .arg(inlineNativeContent? "ON" : "OFF"));
        }

        // Shortcuts
        if (Options.KeyboardShortcutsModified) {

            if (Options.KeyboardShortcutsDefault) {

                Settings.beginGroup(KEYBOARDSHORTCUTS);
                Settings.remove("");
                Settings.endGroup();

                keyboardShortcuts.clear();

                lpub->setDefaultKeyboardShortcuts();

            } else {
                keyboardShortcuts = Options.KeyboardShortcuts;

                Settings.beginGroup(KEYBOARDSHORTCUTS);
                QMap<QString, QKeySequence>::const_iterator it = keyboardShortcuts.constBegin();
                while (it != keyboardShortcuts.constEnd()) {
                    Settings.setValue(it.key(), it.value());
                    ++it;
                }
                Settings.endGroup();
            }

            lpub->setKeyboardShortcuts();
        }

        // LcLib Preferences
        bool AAChanged = false;
        if ((AAChanged = CurrentAASamples != Options.AASamples))
            lcSetProfileInt(LC_PROFILE_ANTIALIASING_SAMPLES, Options.AASamples);

        bool StudStyleChanged = false;
        if ((StudStyleChanged = CurrentStudStyle != Options.StudStyle))
            lcSetProfileInt(LC_PROFILE_STUD_STYLE, static_cast<int>(Options.StudStyle));

        bool AutomateEdgeColorChanged = false;
        if ((AutomateEdgeColorChanged = Options.Preferences.mAutomateEdgeColor != gApplication->mPreferences.mAutomateEdgeColor))
            lcSetProfileInt(LC_PROFILE_AUTOMATE_EDGE_COLOR, Options.Preferences.mAutomateEdgeColor);

        if ((AutomateEdgeColorChanged |= Options.Preferences.mStudCylinderColor != gApplication->mPreferences.mStudCylinderColor))
            lcSetProfileInt(LC_PROFILE_STUD_CYLINDER_COLOR, Options.Preferences.mStudCylinderColor);

        if ((AutomateEdgeColorChanged |= Options.Preferences.mPartEdgeColor != gApplication->mPreferences.mPartEdgeColor))
            lcSetProfileInt(LC_PROFILE_PART_EDGE_COLOR, Options.Preferences.mPartEdgeColor);

        if ((AutomateEdgeColorChanged |= Options.Preferences.mBlackEdgeColor != gApplication->mPreferences.mBlackEdgeColor))
            lcSetProfileInt(LC_PROFILE_BLACK_EDGE_COLOR, Options.Preferences.mBlackEdgeColor);

        if ((AutomateEdgeColorChanged |= Options.Preferences.mDarkEdgeColor != gApplication->mPreferences.mDarkEdgeColor))
            lcSetProfileInt(LC_PROFILE_DARK_EDGE_COLOR, Options.Preferences.mDarkEdgeColor);

        if ((AutomateEdgeColorChanged |= Options.Preferences.mPartEdgeContrast != gApplication->mPreferences.mPartEdgeContrast))
            lcSetProfileFloat(LC_PROFILE_PART_EDGE_CONTRAST, Options.Preferences.mPartEdgeContrast);
        ;
        if ((AutomateEdgeColorChanged |= Options.Preferences.mPartColorValueLDIndex != gApplication->mPreferences.mPartColorValueLDIndex))
            lcSetProfileFloat(LC_PROFILE_PART_COLOR_VALUE_LD_INDEX, Options.Preferences.mPartColorValueLDIndex);;

        bool drawEdgeLinesChanged = false;
        if ((drawEdgeLinesChanged = Options.Preferences.mDrawEdgeLines != gApplication->mPreferences.mDrawEdgeLines))
            lcSetProfileInt(LC_PROFILE_DRAW_EDGE_LINES, Options.Preferences.mDrawEdgeLines);

        bool shadingModeChanged = false;
        if ((shadingModeChanged = Options.Preferences.mShadingMode != gApplication->mPreferences.mShadingMode))
            lcSetProfileInt(LC_PROFILE_SHADING_MODE, static_cast<int>(Options.Preferences.mShadingMode));

        bool lineWidthChanged = false;
        if ((Options.Preferences.mLineWidth != gApplication->mPreferences.mLineWidth))
            lcSetProfileFloat(LC_PROFILE_LINE_WIDTH, Options.Preferences.mLineWidth);

        bool NativeViewpointChanged = false;
        if ((NativeViewpointChanged = Options.Preferences.mNativeViewpoint != gApplication->mPreferences.mNativeViewpoint))
            lcSetProfileInt(LC_PROFILE_NATIVE_VIEWPOINT, Options.Preferences.mNativeViewpoint);

        bool NativeProjectionChanged = false;
        if (!PerspectiveProjectionChanged) {
            if ((NativeProjectionChanged = Options.Preferences.mNativeProjection != gApplication->mPreferences.mNativeProjection))
                lcSetProfileInt(LC_PROFILE_NATIVE_PROJECTION, Options.Preferences.mNativeProjection);
        }

        bool LPubTrueFadeChanged = false;
        if ((LPubTrueFadeChanged = Options.Preferences.mLPubTrueFade != gApplication->mPreferences.mLPubTrueFade))
            lcSetProfileInt(LC_PROFILE_LPUB_TRUE_FADE, Options.Preferences.mLPubTrueFade);

        bool DrawConditionalLinesChanged = false;
        if ((DrawConditionalLinesChanged = Options.Preferences.mDrawConditionalLines != gApplication->mPreferences.mDrawConditionalLines))
            lcSetProfileInt(LC_PROFILE_DRAW_CONDITIONAL_LINES, Options.Preferences.mDrawConditionalLines);

        bool DefaultCameraChanged = false;
        if ((Options.Preferences.mDDF != gApplication->mPreferences.mDDF))
             lcSetProfileFloat(LC_PROFILE_DEFAULT_DISTANCE_FACTOR, Options.Preferences.mDDF);

        if ((DefaultCameraChanged |= Options.Preferences.mCDP != gApplication->mPreferences.mCDP))
            lcSetProfileFloat(LC_PROFILE_CAMERA_DEFAULT_POSITION, Options.Preferences.mCDP);

        if ((DefaultCameraChanged |= Options.Preferences.mCFoV != gApplication->mPreferences.mCFoV))
             lcSetProfileFloat(LC_PROFILE_CAMERA_FOV, Options.Preferences.mCFoV);

        if ((DefaultCameraChanged |= Options.Preferences.mCNear != gApplication->mPreferences.mCNear))
             lcSetProfileFloat(LC_PROFILE_CAMERA_NEAR_PLANE, Options.Preferences.mCNear);

        if ((DefaultCameraChanged |= Options.Preferences.mCFar != gApplication->mPreferences.mCFar))
             lcSetProfileFloat(LC_PROFILE_CAMERA_FAR_PLANE, Options.Preferences.mCFar);

        gApplication->mPreferences = Options.Preferences;

        box.setIcon (QMessageBox::Question);
        box.setDefaultButton   (QMessageBox::Ok);
        box.setStandardButtons (QMessageBox::Ok | QMessageBox::Cancel);

        if (AAChanged) {
            QString thisChange =  QMessageBox::tr("Anti-aliasing");
            box.setText (QMessageBox::tr("You must close and restart %1 to enable %2 change.")
                                         .arg(QLatin1String(VER_PRODUCTNAME_STR))
                                         .arg(thisChange));
            box.setInformativeText (QMessageBox::tr("Click \"OK\" to close and restart %1 or \"Cancel\" to continue.\n\n")
                                                    .arg(QLatin1String(VER_PRODUCTNAME_STR)));
            if (box.exec() == QMessageBox::Ok) {
                restartApplication = true;
            }
        }
        if ((LPubTrueFadeChanged  ||
             DefaultCameraChanged ||
             DrawConditionalLinesChanged) && !restartApplication && !reloadFile)
            reloadPage = true;

        if ((LPubTrueFadeChanged  ||
             DefaultCameraChanged ||
             DrawConditionalLinesChanged) && !restartApplication && !reloadFile)
            reloadPage = true;

        if (preferredRenderer == RENDERER_NATIVE && !restartApplication)
        {
            if (shadingModeChanged     ||
                drawEdgeLinesChanged   ||
                lineWidthChanged       ||
                NativeViewpointChanged ||
                NativeProjectionChanged)
            {
                reloadFile = true;

                QString oldShadingMode, newShadingMode;
                switch (int(Options.Preferences.mShadingMode))
                {
                case int(lcShadingMode::Flat):
                    newShadingMode = "flat";
                    break;
                case int(lcShadingMode::DefaultLights):
                    newShadingMode = "default lights";
                    break;
                case int(lcShadingMode::Full):
                    newShadingMode = "full";
                    break;
                case int(lcShadingMode::Wireframe):
                    newShadingMode = "wire frame";
                    break;
                default:
                    newShadingMode = "unknown";
                }

                switch (lcGetProfileInt(LC_PROFILE_SHADING_MODE))
                {
                case int(lcShadingMode::Flat):
                    oldShadingMode = "flat";
                    break;
                case int(lcShadingMode::DefaultLights):
                    oldShadingMode = "default lights";
                    break;
                case int(lcShadingMode::Full):
                    oldShadingMode = "full";
                    break;
                case int(lcShadingMode::Wireframe):
                    oldShadingMode = "wire frame";
                    break;
                default:
                    oldShadingMode = "unknown";
                }

                if (shadingModeChanged)
                    emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Shading mode changed from %1 to %2.")
                                          .arg(oldShadingMode)
                                          .arg(newShadingMode));
                if (lineWidthChanged)
                    emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Edge line width changed from %1 to %2.")
                                          .arg(double(lcGetProfileFloat(LC_PROFILE_LINE_WIDTH)))
                                          .arg(double(Options.Preferences.mLineWidth)));
                if (drawEdgeLinesChanged)
                    emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Draw edge lines is %1.")
                                          .arg(Options.Preferences.mDrawEdgeLines ? "ON" : "OFF"));

                if (NativeViewpointChanged) {
                    QString Viewpoint;
                    switch (lcGetProfileInt(LC_PROFILE_NATIVE_VIEWPOINT))
                    {
                    case 0:
                        Viewpoint = "Front";
                        break;
                    case 1:
                        Viewpoint = "Back";
                        break;
                    case 2:
                        Viewpoint = "Top";
                        break;
                    case 3:
                        Viewpoint = "Bottom";
                        break;
                    case 4:
                        Viewpoint = "Left";
                        break;
                    case 5:
                        Viewpoint = "Right";
                        break;
                    case 6:
                        Viewpoint = "Home";
                        break;
                    default:
                        Viewpoint = "Front";
                    }

                    emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Native Viewport changed to '%1'.")
                                          .arg(Viewpoint.toUpper()));
                }

                if (NativeProjectionChanged) {
                    QVariant uValue(true);
                    QString Projection;
                    switch (lcGetProfileInt(LC_PROFILE_NATIVE_PROJECTION))
                    {
                    case 0:
                        Projection = "Perscpective";
                        break;
                    case 1:
                        Projection = "Ortographic";
                        uValue = false;
                        break;
                    default:
                        Projection = "Perscpective";
                        break;
                    }

                    if (preferredRenderer == RENDERER_NATIVE) {
                        Settings.setValue(QString("%1/%2").arg(SETTINGS,"PerspectiveProjection"),uValue);
                    }

                    emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Native Projection changed to '%1'.")
                                          .arg(Projection.toUpper()));
                }
            }
        }

        if (StudStyleChanged)
        {
            lcGetPiecesLibrary()->SetStudStyle(Options.StudStyle, true);
            reloadFile = !restartApplication;

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Stud style changed from '%1' to '%2'.")
                                  .arg(studStyleNames[static_cast<int>(CurrentStudStyle)])
                                  .arg(studStyleNames[static_cast<int>(Options.StudStyle)]));
        }
        else if (AutomateEdgeColorChanged)
        {
            lcGetPiecesLibrary()->LoadColors();
            reloadPage = !restartApplication && !reloadFile;

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Automate edge color changed"));
        }

        if (shadingModeChanged)
            lpub->SetShadingMode(Options.Preferences.mShadingMode);

        // fade previous steps and highlight current step
        const bool enableHighlightStepChanged = enableHighlightStep != dialog->enableHighlightStep();
        const bool enableFadeStepsChanged = enableFadeSteps != dialog->enableFadeSteps();

        if ((((fadeStepsColourChanged && fadeStepsUseColour)  ||
               fadeStepsUseColourChanged || fadeStepsOpacityChanged)       &&
               dialog->enableFadeSteps() && !enableFadeStepsChanged) ||
             ((highlightStepColorChanged || highlightStepLineWidthChanged) &&
               dialog->enableHighlightStep() && !enableHighlightStepChanged)) {
            reloadFile = resetCustomCache = true;
        }

        if (enableFadeStepsChanged || enableHighlightStepChanged) {

            if (enableFadeStepsChanged) {
                initEnableFadeSteps = dialog->enableFadeSteps();
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableFadeSteps"),enableFadeSteps);
            }

            if (enableHighlightStepChanged) {
                initEnableHighlightStep = dialog->enableHighlightStep();
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableHighlightStep"),enableHighlightStep);
            }

            suspendFileDisplay = restartApplication || reloadFile || reloadPage;

            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Fade Previous Steps is %1.")
                                  .arg(dialog->enableFadeSteps() ? "ON" : "OFF"));
            emit lpub->messageSig(LOG_INFO,QMessageBox::tr("Highlight Current Step is %1.")
                                  .arg(dialog->enableHighlightStep() ? "ON" : "OFF"));

            if (dialog->enableFadeSteps() && !LDrawColourParts::ldrawColorPartsIsLoaded()) {
                QString result;
                if (!LDrawColourParts::LDrawColorPartsLoad(result)) {
                    emit lpub->messageSig(LOG_ERROR, QMessageBox::tr("Could not open %1 LDraw color parts file [%2], Error: %3")
                                          .arg(validLDrawLibrary).arg(ldrawColourPartsFile).arg(result));
                }
            }

            if (dialog->enableFadeSteps() || dialog->enableHighlightStep())
                lpub->ldrawFile.clearPrevStepPositions();

            lpub->mi.clearCustomPartCache();
            lpub->mi.clearCsiCache();

            // fade steps or highlight step exist so remove final model
            if (enableFadeSteps || enableHighlightStep) {
                enableFadeSteps = dialog->enableFadeSteps();
                enableHighlightStep = dialog->enableHighlightStep();
                lpub->mi.setSuspendFileDisplayFlag(suspendFileDisplay);
                lpub->mi.deleteFinalModelStep();
            }
            // neither fade step nor highlight step exist and dialog option(s) set so create final model
            else if (!enableFadeSteps || !enableHighlightStep) {
                enableFadeSteps = dialog->enableFadeSteps();
                enableHighlightStep = dialog->enableHighlightStep();
                lpub->mi.setSuspendFileDisplayFlag(suspendFileDisplay);
                lpub->mi.insertFinalModelStep();
            }

            lpub->mi.setSuspendFileDisplayFlag(false);
        }
        // end fade previous steps and highlight current step

        setSceneTheme =
                displayThemeChanged         ||
                sceneBackgroundColorChanged ||
                sceneGridColorChanged       ||
                sceneRulerTickColorChanged  ||
                sceneGuideColorChanged;

        if (displayThemeColorsChanged || setSceneTheme) {
            loadTheme = displayThemeChanged || displayThemeColorsChanged;
        }

        emit lpub->messageSig(LOG_INFO_STATUS,QMessageBox::tr("Preferences updated. %1")
                              .arg(lpub->elapsedTime(timer.elapsed())));

    } else {

        returnResult = false;
    }

#ifdef Q_OS_MAC
    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isHidden())
        Application::instance()->splash->show();
#endif
    return returnResult;
}

void Preferences::resetPreferenceFlags()
{
    loadTheme            = false;
    setSceneTheme        = false;
    reloadPage           = false;
    reloadFile           = false;
    resetCustomCache     = false;
    restartApplication   = false;
    libraryChangeRestart = false;
}

void Preferences::getRequireds()
{
    // this call will most likely not get past the preferredRenderer == "" statement as
    // the preferred renderer is set on application launch before getRequireds() is called.
    if (preferredRenderer == RENDERER_INVALID && ! getPreferences()) {
        exit (-1);
    }
}

void Preferences::setInitFadeSteps()
{
    initEnableFadeSteps      = enableFadeSteps;
    initFadeStepsUseColour   = fadeStepsUseColour;
    initFadeStepsOpacity     = fadeStepsOpacity;
    initValidFadeStepsColour = validFadeStepsColour;
}

void Preferences::setInitHighlightStep()
{
    initEnableHighlightStep    = enableHighlightStep;
    initHighlightStepLineWidth = highlightStepLineWidth;
    initHighlightStepColour    = highlightStepColour;
}

void Preferences::setInitPreferredRenderer()
{
    initPreferredRenderer = preferredRenderer;
}

// source reset calls
void Preferences::resetFadeSteps()
{
    if (enableFadeSteps      != initEnableFadeSteps)
        enableFadeSteps       = initEnableFadeSteps;
    if (fadeStepsUseColour   != initFadeStepsUseColour)
        fadeStepsUseColour    = initFadeStepsUseColour;
    if (fadeStepsOpacity     != initFadeStepsOpacity)
        fadeStepsOpacity      = initFadeStepsOpacity;
    if (validFadeStepsColour != initValidFadeStepsColour)
        validFadeStepsColour  = initValidFadeStepsColour;

    fadestepPreferences(true/*persist*/);
}

void Preferences::resetHighlightStep()
{
    if (enableHighlightStep    != initEnableHighlightStep)
        enableHighlightStep     = initEnableHighlightStep;
    if (highlightStepLineWidth != initHighlightStepLineWidth)
        highlightStepLineWidth  = initHighlightStepLineWidth;
    if (highlightStepColour    != initHighlightStepColour)
        highlightStepColour     = initHighlightStepColour;

    highlightstepPreferences(true/*persist*/);
}

void Preferences::resetPreferredRenderer()
{
    if (preferredRenderer != initPreferredRenderer)
        preferredRenderer = initPreferredRenderer;

    preferredRendererPreferences(true/*persist*/);
}

void Preferences::unsetBuildModifications(){
    buildModEnabled = false;
}

void Preferences::setLPub3DLoaded(){
    lpub3dLoaded = true;
}

/*
 * Set the LDView extra search directories. This function is called on the following occasions
 * - LPub3D Initialize: Gui::initialize() -> Preferences::setLDViewExtraSearchDirs(...)
 * - Preference Dialogue if search directories change: Preferences::getPreferences()
 * - Archiving Custom Color parts where Custom dirs were not in search dirs list: PartWorker::processCustomColourParts()
 */
bool Preferences::setLDViewExtraSearchDirs(const QString &iniFile) {
    if (!ldviewInstalled)
        return true;

    bool retVal = true;
    QFile confFile(iniFile);
    QFileInfo confFileInfo(iniFile);
    QStringList contentList;
    QRegExp prefSetRx("^(Native POV|Native STL|Native 3DS|Native Part List|POV-Ray Render)",Qt::CaseInsensitive);
    if (preferredRenderer == RENDERER_LDVIEW)
      logInfo() << QString("Updating ExtraSearchDirs in %1").arg(iniFile);
    if (confFile.open(QIODevice::ReadOnly))
    {
        int dirNum = 0;
        bool foundExtraSearchDirs = false;
        bool inExtraSearchDirsSection = false;
        QString prefSet;
        QTextStream input(&confFile);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            if (line.left(17) == "[ExtraSearchDirs]") {
                foundExtraSearchDirs = true;
                inExtraSearchDirsSection = true;
                if (!contentList.contains(line,Qt::CaseInsensitive))
                    contentList.append( line);
            } else if (inExtraSearchDirsSection) {                  // in ExtraSearchDirs section
                if (line.left(1) == "[" || line.isEmpty()) {        // at next section or empty line, insert search dirs
                    dirNum = 0;
                    QString nativePath;
                    Q_FOREACH (QString searchDir, ldSearchDirs) {
                       dirNum++;
                       if (dirNum <= ldSearchDirs.count()) {
#ifdef Q_OS_WIN
                          nativePath = searchDir.replace("\\","\\\\");
#else
                          nativePath = QDir::toNativeSeparators(searchDir);
#endif
                          if (!contentList.contains(nativePath, Qt::CaseInsensitive)) {
                              QString formattedSearchDir = QString("Dir%1=%2").arg(dirNum, 3, 10, QChar('0')).arg(nativePath);
                              contentList += formattedSearchDir;
//                              if (preferredRenderer == RENDERER_LDVIEW || confFileInfo.completeBaseName().toLower() == "ldvexport")
//                                  logInfo() << QString("ExtraSearchDirs OUT: %1").arg(formattedSearchDir);
                          }
                       }
                    }
                    if ( !line.isEmpty())
                        contentList.append( line);
                    inExtraSearchDirsSection = false;
                }
            } else if (line.contains(prefSetRx)) {               // session preference set
                if (line.contains("_SessionPlaceholder")) {      // insert search dirs before session placeholder
                    dirNum = 0;
                    QString nativePath;
                    Q_FOREACH (QString searchDir, ldSearchDirs) {
                       dirNum++;
                       if (dirNum <= ldSearchDirs.count()) {
#ifdef Q_OS_WIN
                          nativePath = searchDir.replace("\\","\\\\");
#else
                          nativePath = QDir::toNativeSeparators(searchDir);
#endif
                          if (!contentList.contains(nativePath, Qt::CaseInsensitive)) {
                              QString formattedSearchDir = QString("%1/ExtraSearchDirs/Dir%2=%3").arg(prefSetRx.cap(1)).arg(dirNum, 3, 10, QChar('0')).arg(nativePath);
                              contentList += formattedSearchDir;
//                              if (preferredRenderer == RENDERER_LDVIEW || confFileInfo.completeBaseName().toLower() == "ldvexport")
//                                  logInfo() << QString("ExtraSearchDirs OUT: %1").arg(formattedSearchDir);
                          }
                       }
                    }
                    contentList.append( line);
                } else if ( !line.isEmpty() && !line.contains("/ExtraSearchDirs/",Qt::CaseInsensitive)) {   // remove old ExtraSearchDirs lines
                    contentList.append( line);
                }
            } else if (!contentList.contains(line,Qt::CaseInsensitive) && !line.isEmpty()) {
                contentList.append( line);
            }
        }  // atEnd
        confFile.close();
        if (!foundExtraSearchDirs) {
            logError() << QString("Did not find [ExtraSearchDirs] section in %1 "
                                  "The inf file %1 may be malformed or corrupt.").arg(confFile.fileName());
            retVal = false;
         }
    } else {
        QString confFileError;
        if (!confFile.errorString().isEmpty())
            confFileError.append(QString(" confFileInError: %1").arg(confFile.errorString()));
        logError() << QString("Could not open input: %1").arg(confFileError);
        retVal = false;
    }
    // write search dir to ini files
    if (confFile.open(QIODevice::WriteOnly))
    {
        QTextStream output(&confFile);
        Q_FOREACH (QString line, contentList) {
           output << line << lpub_endl;
        }
        confFile.flush();
        confFile.close();
    } else {
        QString confFileError;
        if (!confFile.errorString().isEmpty())
            confFileError.append(QString(" confFileOutError: %1").arg(confFile.errorString()));
        logError() << QString("Could not open input or output file: %1").arg(confFileError);
        retVal = false;
    }
    return retVal;
}

bool Preferences::extractLDrawLib() {

#ifdef Q_OS_MAC
    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isHidden())
        Application::instance()->splash->show();
#endif

    QSettings Settings;
    QFileInfo fileInfo;
    QString message;
    bool r = true;

    message = QMessageBox::tr("Extracting %1 LDraw library, please wait...").arg(validLDrawLibrary);

    emit Application::instance()->splashMsgSig("10% - " + message);

    if (!modeGUI) {
      fprintf(stdout,"%s\n",message.toLatin1().constData());
      fflush(stdout);
    }

    // if ldraw directory path is empty use the default location (datapath)
    if (ldrawLibPath.isEmpty()) {
        ldrawLibPath = QString("%1/%2").arg(lpubDataPath, validLDrawDir);
    }
    // set ldraw parent directory to extract archive.zip
    bool parentDirNotValid = false;
    QDir ldrawDir(ldrawLibPath);
    //logInfo() << QString("LDraw directory: %1").arg(ldrawDir.absolutePath()));
    logDebug() << QString("LDraw directory: %1").arg(ldrawDir.absolutePath());
    if (ldrawDir.dirName().toLower() != validLDrawDir.toLower())
        parentDirNotValid = true;
    if (!ldrawDir.isRoot())
        ldrawDir.cdUp();            // ldraw path parent directory
    //logInfo() << QString("LDraw parent directory (%1), AbsPath (%2)").arg(ldrawDir.dirName()).arg(ldrawDir.absolutePath())));

    // set the archive library path
    QDir libraryDir(QString("%1/%2").arg(lpubDataPath, "libraries"));
    fileInfo.setFile(QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), validLDrawPartsArchive)));

    // archive library exist so let's proceed...
    if (fileInfo.exists()) {

        // extract archive.zip
        QString destination = ldrawDir.absolutePath();
        QStringList result = JlCompress::extractDir(fileInfo.absoluteFilePath(),destination);
        if (result.isEmpty()){
            logError() << QString("Failed to extract %1 to %2/%3").arg(fileInfo.absoluteFilePath()).arg(destination).arg(validLDrawDir);
            r = false;
         } else {
            if (! usingDefaultLibrary) {
                // Rename extracted library to ldraw for compatibility with lc_library and renderers
                QFile library(QString("%1/%2").arg(destination).arg(validLDrawDir));
                QFile extract(QString("%1/ldraw").arg(destination));

                if (! library.exists() || library.remove()) {
                    if (! extract.rename(library.fileName())) {
                        logError() << QString("Failed to rename %1 to %2").arg(extract.fileName()).arg(library.fileName());
                    } else {
                        message = QMessageBox::tr("%1 %2 Library files extracted to %3/%4")
                                .arg(result.size()).arg(validLDrawLibrary).arg(destination).arg(validLDrawDir);
                        logInfo() << QString(message);
                    }
                }  else {
                    message = QMessageBox::tr("Could not remove old library %1").arg(library.remove());
                    logError() << QString(message);
                }
            } else {
             message = QMessageBox::tr("%1 Official Library files extracted to %2/%3").arg(result.size()).arg(destination).arg(validLDrawDir);
             logInfo() << QString(message);
            }
         }

        // extract lpub3dldrawunf.zip - for LEGO library only
        if (usingDefaultLibrary) {
            fileInfo.setFile(QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(),validLDrawCustomArchive)));
            if (fileInfo.exists()) {
                QString destination = QString("%1/unofficial").arg(ldrawLibPath);
                QStringList result = JlCompress::extractDir(fileInfo.absoluteFilePath(),destination);
                if (result.isEmpty()){
                    logError() << QString("Failed to extract %1 to %2").arg(fileInfo.absoluteFilePath()).arg(destination);
                } else {
                    message = QMessageBox::tr("%1 Unofficial Library files extracted to %2").arg(result.size()).arg(destination);
                    logInfo() << QString(message);
                }
            } else {
                message = QMessageBox::tr ("Unofficial Library archive file %1 does not exist.").arg(fileInfo.absoluteFilePath());
                logError() << QString(message);
            }
        }

        // copy extracted contents to ldraw directory and delete extract dir if needed
        if (parentDirNotValid) {
            QDir extractDir(QString("%1/%2").arg(ldrawDir.absolutePath()).arg(validLDrawDir));
            if (!copyRecursively(extractDir.absolutePath(),ldrawLibPath)) {
                message = QMessageBox::tr("Unable to copy %1 to %2").arg(extractDir.absolutePath(),ldrawLibPath);
                logInfo() << QString(message);
                r = false;
            }
            extractDir.removeRecursively();
        }

        // if no errors, (re)set ldrawLibPath in registry
        if (r)
            Settings.setValue(QString("%1/%2").arg(SETTINGS, ldrawLibPathKey),ldrawLibPath);

    } else {

        QString body;
        if (usingDefaultLibrary)
            fileInfo.setFile(QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LDRAW_UNOFFICIAL_ARCHIVE)));
        if (usingDefaultLibrary && !fileInfo.exists()) {
            body = QMessageBox::tr ("LPub3D attempted to extract the LDraw library however the required archive files\n%1\n%2\ndoes not exist.\n").arg(lpub3dLibFile, fileInfo.absoluteFilePath());
        } else {
            body = QMessageBox::tr ("LPub3D attempted to extract the LDraw library however the required archive file\n%1\ndoes not exist.\n").arg(lpub3dLibFile);
        }

        if (modeGUI) {
            QPixmap _icon = QPixmap(":/icons/lpub96.png");
            QMessageBox box;
            box.setWindowIcon(QIcon());
            box.setIconPixmap (_icon);
            box.setTextFormat (Qt::RichText);
            box.setWindowTitle(QMessageBox::tr ("LDraw Library"));
            box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
            QString header = "<b> " + QMessageBox::tr ("LDraw library archive not found!") + "</b>";
            box.setStandardButtons (QMessageBox::Close);
            box.setText(header);
            box.setInformativeText(body);
            box.exec();
        } else {
            fprintf(stderr,"%s",body.toLatin1().constData());
        }

        r = false;

        // remove registry setting and clear ldrawLibPath
        fileInfo.setFile(QDir::toNativeSeparators(QString("%1%2").arg(ldrawLibPath).arg(validLDrawPart)));
        if (!fileInfo.exists()) {
            Settings.remove(QString("%1/%2").arg(SETTINGS,ldrawLibPathKey));
            ldrawLibPath.clear();
        }

    }

    return r;
}

bool Preferences::copyRecursively(const QString &srcFilePath,
                            const QString &tgtFilePath)
{
    QFileInfo srcFileInfo(srcFilePath);
    if (srcFileInfo.isDir()) {
        QDir targetDir(tgtFilePath);
        targetDir.cdUp();
        if (!targetDir.mkdir(QFileInfo(tgtFilePath).fileName()))
            return false;
        QDir sourceDir(srcFilePath);
        QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        Q_FOREACH (const QString &fileName, fileNames) {
            const QString newSrcFilePath
                    = srcFilePath + QLatin1Char('/') + fileName;
            const QString newTgtFilePath
                    = tgtFilePath + QLatin1Char('/') + fileName;
            if (!copyRecursively(newSrcFilePath, newTgtFilePath))
                return false;
        }
    } else {
        if (!QFile::copy(srcFilePath, tgtFilePath))
            return false;
    }
    return true;
}

void Preferences::addKeyboardShortcut(const QString &objectName, const QKeySequence &keySequence)
{
    if (objectName.isEmpty()) return;
    keyboardShortcuts.insert(objectName, keySequence);
}

bool Preferences::hasKeyboardShortcut(const QString &objectName)
{
    return keyboardShortcuts.contains(objectName);
}

QKeySequence Preferences::keyboardShortcut(const QString &objectName)
{
    return keyboardShortcuts.value(objectName);
}
