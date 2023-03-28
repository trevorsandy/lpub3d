/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2023 Trevor SANDY. All rights reserved.
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

#ifndef commonmenus_h
#define commonmenus_h
#pragma once

#include <QString>
#include "metatypes.h"

#define DEF_ITEM "Item"

/****************************
 * What's This Text
 ****************************/

enum WT_Type
{
    //*************************************
    //*  PREFERENCES ENTRIES
    //*************************************
    WT_LPUB3D_PREFERENCES,
    WT_LPUB3D_PREFERENCES_DARK_THEME_COLORS,
    WT_LPUB3D_PREFERENCES_DEFAULT_THEME_COLORS,
    WT_LPUB3D_PREFERENCES_GENERAL,
    WT_LPUB3D_PREFERENCES_KEYBOARD_SHORTCUTS,
    WT_LPUB3D_PREFERENCES_LDGLITE,
    WT_LPUB3D_PREFERENCES_LDVIEW,
    WT_LPUB3D_PREFERENCES_MESSAGES,
    WT_LPUB3D_PREFERENCES_NATIVE,
    WT_LPUB3D_PREFERENCES_OTHER,
    WT_LPUB3D_PREFERENCES_POVRAY,
    WT_LPUB3D_PREFERENCES_PUBLISHING,
    WT_LPUB3D_PREFERENCES_RENDERERS,
    WT_LPUB3D_PREFERENCES_SEARCH_DIRECTORIES,
    WT_LPUB3D_PREFERENCES_THEME_AND_LOGGING,
    WT_LPUB3D_PREFERENCES_THEME_COLORS,
    WT_LPUB3D_PREFERENCES_UPDATES,
    WT_VISUAL_PREFERENCES,
    WT_VISUAL_PREFERENCES_CATEGORIES,
    WT_VISUAL_PREFERENCES_COLORS,
    WT_VISUAL_PREFERENCES_GENERAL,
    WT_VISUAL_PREFERENCES_KEYBOARD_SHORTCUTS,
    WT_VISUAL_PREFERENCES_MOUSE,
    WT_VISUAL_PREFERENCES_RENDERING,
    WT_VISUAL_PREFERENCES_USER_INTERFACE,
    WT_VISUAL_PREFERENCES_VIEWS,
    WT_LDVIEW_PREFERENCES,
    WT_LDVIEW_PREFERENCES_GENERAL,
    WT_LDVIEW_PREFERENCES_GEOMETRY,
    WT_LDVIEW_PREFERENCES_EFFECTS,
    WT_LDVIEW_PREFERENCES_PRIMITIVES,
    WT_LDVIEW_PREFERENCES_UPDATES,
    WT_LDVIEW_PREFERENCES_PREFERENCES_SET,
    //*************************************
    //*  GLOBAL SETUP ENTRIES
    //*************************************
    WT_SETUP_ASSEM,
    WT_SETUP_ASSEM_CONTENTS,
    WT_SETUP_ASSEM_DISPLAY,
    WT_SETUP_ASSEM_DISPLAY_STEP,
    WT_SETUP_ASSEM_ANNOTATION,
    WT_SETUP_CALLOUT,
    WT_SETUP_CALLOUT_ASSEMBLY,
    WT_SETUP_CALLOUT_BACKGROUND_BORDER,
    WT_SETUP_CALLOUT_CALLOUT,
    WT_SETUP_CALLOUT_CALLOUT_PARTS_LIST,
    WT_SETUP_CALLOUT_CALLOUT_SUBMODEL_PREVIEW,
    WT_SETUP_CALLOUT_CALLOUT_POINTERS,
    WT_SETUP_CALLOUT_CONTENTS,
    WT_SETUP_CALLOUT_DIVIDER_POINTERS,
    WT_SETUP_CALLOUT_POINTERS,
    WT_SETUP_FADE_STEPS,
    WT_SETUP_FADE_STEPS_SETUP,
    WT_SETUP_HIGHLIGHT_STEP,
    WT_SETUP_HIGHLIGHT_STEP_SETUP,
    WT_SETUP_MULTI_STEP,
    WT_SETUP_MULTI_STEP_ADJUST_STEP,
    WT_SETUP_MULTI_STEP_ASSEMBLY,
    WT_SETUP_MULTI_STEP_CONTENTS,
    WT_SETUP_MULTI_STEP_DIVIDER,
    WT_SETUP_MULTI_STEP_DISPLAY,
    WT_SETUP_MULTI_STEP_DISPLAY_PARTS_LIST,
    WT_SETUP_MULTI_STEP_DISPLAY_SUBMODEL_PREVIEW,
    WT_SETUP_PAGE,
    WT_SETUP_PAGE_AUTHOR_EMAIL,
    WT_SETUP_PAGE_LPUB3D_LOGO,
    WT_SETUP_PAGE_COVER_IMAGE,
    WT_SETUP_PAGE_DESCRIPTION,
    WT_SETUP_PAGE_DOCUMENT_LOGO,
    WT_SETUP_PAGE_LEGO_DISCLAIMER,
    WT_SETUP_PAGE_MODEL_PARTS_ID,
    WT_SETUP_PAGE_LPUB3D_PLUG,
    WT_SETUP_PAGE_TITLE,
    WT_SETUP_PAGE_PUBLISH_DESCRIPTION,
    WT_SETUP_PAGE_WEBSITE_URL_COPYRIGHT,
    WT_SETUP_PAGE_VIEW,
    WT_SETUP_PAGE_DISCLAIMER,
    WT_SETUP_PAGE_MODEL,
    WT_SETUP_PAGE_NUMBER_TEXT,
    WT_SETUP_PAGE_NUMBER_DISPLAY,
    WT_SETUP_PAGE_NUMBER_PLACEMENT,
    WT_SETUP_PAGE_TEXT_PLACEMENT,
    WT_SETUP_PAGE_PUBLISH,
    WT_SETUP_PART_BOM,
    WT_SETUP_PART_PLI,
    WT_SETUP_PART_BACKGROUND_BORDER,
    WT_SETUP_PART_BACKGROUND_BORDER_PARTS_LIST,
    WT_SETUP_PART_CONTENTS,
    WT_SETUP_PART_PARTS,
    WT_SETUP_PART_PARTS_ORIENTATION,
    WT_SETUP_PART_PARTS_MOVABLE_GROUPS_BOM,
    WT_SETUP_PART_PARTS_MOVABLE_GROUPS_PLI,
    WT_SETUP_PART_MORE_OPTIONS,
    WT_SETUP_PART_MORE_OPTIONS_SHOW_SUBMODELS,
    WT_SETUP_PART_ANNOTATION,
    WT_SETUP_PART_ANNOTATION_OPTIONS,
    WT_SETUP_PART_ANNOTATION_OPTIONS_WITH_TEXT_FORMAT,
    WT_SETUP_PART_ANNOTATION_STYLE_PLI,
    WT_SETUP_PART_ANNOTATION_STYLE_BOM,
    WT_SETUP_PART_ANNOTATION_STYLE_SELECT_BOM,
    WT_SETUP_PART_ANNOTATION_STYLE_SELECT_PLI,
    WT_SETUP_PROJECT,
    WT_SETUP_PROJECT_RENDERER_OPTIONS,
    WT_SETUP_PROJECT_PARSE_OPTIONS,
    WT_SETUP_PROJECT_PARSE_BUFFER_EXCHANGE,
    WT_SETUP_PROJECT_START_NUMBERS,
    WT_SETUP_SUBMODEL,
    WT_SETUP_SUBMODEL_SUBMODEL,
    WT_SETUP_SUBMODEL_PREVIEW,
    WT_SETUP_SUBMODEL_BACKGROUND,
    WT_SETUP_SUBMODEL_CONTENTS,
    WT_SETUP_SUBMODEL_IMAGE,
    WT_SETUP_SUBMODEL_INSTANCE_COUNT,
    WT_SETUP_SHARED_IMAGE_SIZING,
    WT_SETUP_SHARED_MODEL_ORIENTATION,
    WT_SETUP_SHARED_POINTERS,
    WT_SETUP_SHARED_SUBMODEL_LEVEL_COLORS,
    //*************************************
    //*  DIALOG ENTRIES
    //*************************************
    WT_DIALOG_APPEND_PAGE_OPTION,
    WT_DIALOG_BACKGROUND,
    WT_DIALOG_BOM_OPTION,
    WT_DIALOG_BORDER,
    WT_DIALOG_BUILD_MODIFICATIONS,
    WT_DIALOG_CAMERA_ANGLES,
    WT_DIALOG_CAMERA_FOV,
    WT_DIALOG_CAMERA_LAT_LON,
    WT_DIALOG_CONSTRAIN,
    WT_DIALOG_EDITOR_PREFERENCES,
    WT_DIALOG_EXPORT_PAGES,
    WT_DIALOG_DIVIDER,
    WT_DIALOG_DOUBLE_SPIN,
    WT_DIALOG_FADE_HIGHLIGHT,
    WT_DIALOG_FIND_REPLACE,
    WT_DIALOG_FLOAT,
    WT_DIALOG_FLOAT_PAIR,
    WT_DIALOG_GRADIENT,
    WT_DIALOG_LDRAW_COLOR,
    WT_DIALOG_LDRAW_FILES_LOAD,
    WT_DIALOG_LDRAW_SEARCH_DIRECTORIES,
    WT_DIALOG_LDVIEW_HTML_PART_LIST_EXPORT,
    WT_DIALOG_LDVIEW_POV_EXPORT_OPTIONS,
    WT_DIALOG_LOCAL,
    WT_DIALOG_LOOK_AT_TARGET_AND_STEP_ROTATION,
    WT_DIALOG_META_COMMANDS,
    WT_DIALOG_PAGE_ORIENTATION,
    WT_DIALOG_PAGE_SIZE,
    WT_DIALOG_PLACEMENT,
    WT_DIALOG_PLI_ANNOTATION,
    WT_DIALOG_PART_SORT_ORDER,
    WT_DIALOG_POINTER_ATTRIB,
    WT_DIALOG_PREFERRED_RENDERER,
    WT_DIALOG_BLENDER_RENDER,
    WT_DIALOG_BLENDER_RENDER_SETTINGS,
    WT_DIALOG_POVRAY_RENDER,
    WT_DIALOG_POVRAY_RENDER_SETTINGS,
    WT_DIALOG_ROTATE_ICON_SIZE,
    WT_DIALOG_STEP_ROTATION,
    WT_DIALOG_SIZE_AND_ORIENTATION,
    WT_DIALOG_SUBMODEL_LEVEL_COLORS,
    WT_DIALOG_SUBSTITUTE_PART,
    WT_DIALOG_TEXT_EDIT,
    WT_DIALOG_UNITS,
    WT_DIALOG_VISUAL_CATEGORY_EDIT,
    WT_DIALOG_VISUAL_CATEGORY_NEW,
    //*************************************
    //*  CONTROL ENTRIES
    //************************************* 
    WT_CONTROL_COMMANDS_EDIT,
    WT_CONTROL_COMMANDS_FILTER,
    WT_CONTROL_COMMANDS_RESET,
    WT_CONTROL_COMMANDS_UPDATE,
    WT_CONTROL_COMMANDS_VIEW,
    WT_CONTROL_LPUB3D_PREFERENCES_ALTERNATE_LDCONFIG,
    WT_CONTROL_LPUB3D_PREFERENCES_APPLY_CAMERA_ANGLES,
    WT_CONTROL_LPUB3D_PREFERENCES_ATTRIBUTES,
    WT_CONTROL_LPUB3D_PREFERENCES_CAMERA_DEFAULTS,
    WT_CONTROL_LPUB3D_PREFERENCES_CHANGE_LOG,
    WT_CONTROL_LPUB3D_PREFERENCES_DEFAULTS,
    WT_CONTROL_LPUB3D_PREFERENCES_EDITOR,
    WT_CONTROL_LPUB3D_PREFERENCES_FADE_STEPS,
    WT_CONTROL_LPUB3D_PREFERENCES_GENERAL_SETTINGS,
    WT_CONTROL_LPUB3D_PREFERENCES_HIGHLIGHT_STEP,
    WT_CONTROL_LPUB3D_PREFERENCES_IMAGE_MATTE,
    WT_CONTROL_LPUB3D_PREFERENCES_KEYBOARD_SHORTCUT,
    WT_CONTROL_LPUB3D_PREFERENCES_KEYBOARD_SHORTCUT_TABLE,
    WT_CONTROL_LPUB3D_PREFERENCES_LDGLITE,
    WT_CONTROL_LPUB3D_PREFERENCES_LDGLITE_INSTALL,
    WT_CONTROL_LPUB3D_PREFERENCES_LDRAW_LIB_PATH,
    WT_CONTROL_LPUB3D_PREFERENCES_LDVIEW,
    WT_CONTROL_LPUB3D_PREFERENCES_LDVIEW_INSTALL,
    WT_CONTROL_LPUB3D_PREFERENCES_LDVIEW_SETTINGS,
    WT_CONTROL_LPUB3D_PREFERENCES_LOGGING,
    WT_CONTROL_LPUB3D_PREFERENCES_LOG_LEVEL,
    WT_CONTROL_LPUB3D_PREFERENCES_LOG_LEVEL_MESSAGES,
    WT_CONTROL_LPUB3D_PREFERENCES_LOG_LINE_ATTRIBUTES,
    WT_CONTROL_LPUB3D_PREFERENCES_LOG_PATH,
    WT_CONTROL_LPUB3D_PREFERENCES_PRINT_STANDARD_OUT,
    WT_CONTROL_LPUB3D_PREFERENCES_MESSAGES,
    WT_CONTROL_LPUB3D_PREFERENCES_PAGE_PROCESSING_CONTINUOUS,
    WT_CONTROL_LPUB3D_PREFERENCES_PARSE_ERROR,
    WT_CONTROL_LPUB3D_PREFERENCES_PLI_CONTROL,
    WT_CONTROL_LPUB3D_PREFERENCES_POVRAY,
    WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_INSTALL,
    WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_LDVIEW_POV_GENERATION,
    WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_LGEO,
    WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_NATIVE_POV_GENERATION,
    WT_CONTROL_LPUB3D_PREFERENCES_PUBLISH_DESCRIPTION,
    WT_CONTROL_LPUB3D_PREFERENCES_PUBLISH_LOGO,
    WT_CONTROL_LPUB3D_PREFERENCES_RENDERERS_AND_PROJECTION,
    WT_CONTROL_LPUB3D_PREFERENCES_RENDERERS_LSYNTH_PARTS,
    WT_CONTROL_LPUB3D_PREFERENCES_RENDERERS_MESSAGE,
    WT_CONTROL_LPUB3D_PREFERENCES_RENDERING,
    WT_CONTROL_LPUB3D_PREFERENCES_SEARCH_DIRECTORIES,
    WT_CONTROL_LPUB3D_PREFERENCES_THEME_SCENE_COLORS,
    WT_CONTROL_LPUB3D_PREFERENCES_THEME_SELECT_AND_COLORS,
    WT_CONTROL_LPUB3D_PREFERENCES_UNITS,
    WT_CONTROL_LPUB3D_PREFERENCES_UPDATES,
    WT_CONTROL_LPUB3D_PREFERENCES_VIEWPOINT_AND_PROJECTION,
    WT_CONTROL_VISUAL_PREFERENCES_APPLICATION,
    WT_CONTROL_VISUAL_PREFERENCES_AUTHOR,
    WT_CONTROL_VISUAL_PREFERENCES_BACKGROUND_COLOR,
    WT_CONTROL_VISUAL_PREFERENCES_BASE_GRID,
    WT_CONTROL_VISUAL_PREFERENCES_BASE_GRID_COLOR,
    WT_CONTROL_VISUAL_PREFERENCES_CATEGORY,
    WT_CONTROL_VISUAL_PREFERENCES_CATEGORY_EDIT,
    WT_CONTROL_VISUAL_PREFERENCES_GENERAL_SETTINGS,
    WT_CONTROL_VISUAL_PREFERENCES_INTERFACE,
    WT_CONTROL_VISUAL_PREFERENCES_INTERFACE_COLOR,
    WT_CONTROL_VISUAL_PREFERENCES_KEYBOARD_SHORTCUTS_TABLE,
    WT_CONTROL_VISUAL_PREFERENCES_MOUSE,
    WT_CONTROL_VISUAL_PREFERENCES_MOUSE_SHORTCUTS,
    WT_CONTROL_VISUAL_PREFERENCES_MOUSE_SHORTCUTS_TABLE,
    WT_CONTROL_VISUAL_PREFERENCES_OBJECTS_COLOR,
    WT_CONTROL_VISUAL_PREFERENCES_PART_PREVIEW,
    WT_CONTROL_VISUAL_PREFERENCES_PATHS,
    WT_CONTROL_VISUAL_PREFERENCES_SHORTCUT,
    WT_CONTROL_VISUAL_PREFERENCES_TIMELINE,
    WT_CONTROL_VISUAL_PREFERENCES_VIEW_SPHERE,
    WT_CONTROL_VISUAL_PREFERENCES_VIEW_SPHERE_COLOR,
    WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_ANTIALIASING,
    WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_COLORS,
    WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_INI_FILE,
    WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_MISC,
    WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_SAVE_DIRECTORIES,
    WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_MODEL,
    WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_WIREFRAME,
    WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_BFC,
    WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_EDGE_LINES,
    WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_LIGHTING,
    WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_STEREO,
    WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_WIREFRAME_CUTAWAY,
    WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_TRANSPARENCY,
    WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_MISC,
    WT_CONTROL_LDVIEW_PREFERENCES_PRIMITIVE_SUBSTITUTION,
    WT_CONTROL_LDVIEW_PREFERENCES_PRIMITIVE_TEXTURES,
    WT_CONTROL_LDVIEW_PREFERENCES_PRIMITIVE_MISC,
    WT_CONTROL_LDVIEW_PREFERENCES_UPDATES_PROXY,
    WT_CONTROL_LDVIEW_PREFERENCES_UPDATES_MISSING_PART,
    WT_CONTROL_LDVIEW_PREFERENCES_PREFERENCE_SET,
    WT_CONTROL_LDVIEW_HTML_PART_LIST_EXPORT_COLUMN_ORDER,
    WT_CONTROL_LDVIEW_HTML_PART_LIST_EXPORT_DISPLAY_OPTIONS,
    WT_CONTROL_LDVIEW_HTML_PART_LIST_EXPORT_PART_REFERENCE,
    WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_CHROME_MATERIAL,
    WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_GENERAL,
    WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_GEOMETRY,
    WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_INI_FILE,
    WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_LIGHTING,
    WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_MATERIAL,
    WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_POV_GEOMETRY,
    WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_POVRAY_LIGHTS,
    WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_RUBBER_MATERIAL,
    WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_SCROLL_AREA,
    WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_TRANSPARENT_MATERIAL,
    WT_CONTROL_LOOK_AT_TARGET,
    WT_CONTROL_SNIPPETS_ADD,
    WT_CONTROL_SNIPPETS_EDIT,
    WT_CONTROL_SNIPPETS_LIST,
    WT_CONTROL_SNIPPETS_REMOVE,
    WT_CONTROL_SUBSTITUTE_CURRENT_PART,
    WT_CONTROL_SUBSTITUTE_PRIMARY_ARGUMENTS,
    WT_CONTROL_SUBSTITUTE_EXTENDED_ARGUMENTS,
    //*************************************
    //*  METAGUI ENTRIES
    //*************************************
    WT_GUI_SIZE_AND_ORIENTATION,
    WT_GUI_BACKGROUND,
    WT_GUI_BACKGROUND_NO_IMAGE,
    WT_GUI_BORDER,
    WT_GUI_CAMERA_ANGLES,
    WT_GUI_CAMERA_DEFAULT_DISTANCE_FACTOR,
    WT_GUI_CAMERA_FIELD_OF_VIEW,
    WT_GUI_CAMERA_NEAR_PLANE,
    WT_GUI_CAMERA_FAR_PLANE,
    WT_GUI_IMAGE_SCALE,
    WT_GUI_UNITS_MARGIN,
    WT_GUI_HEADER_HEIGHT,
    WT_GUI_FOOTER_HEIGHT,
    WT_GUI_POINTER_BORDER,
    WT_GUI_POINTER_LINE,
    WT_GUI_POINTER_TIP,
    WT_GUI_PAGE_ATTRIBUTE_TEXT,
    WT_GUI_PAGE_ATTRIBUTE_IMAGE,
    WT_GUI_SUBMODEL_LEVEL_COLORS,
    WT_GUI_CHECK_BOX,
    WT_GUI_UNITS,
    WT_GUI_NUMBER,
    WT_GUI_BOOL_RADIO_BUTTON,
    WT_GUI_PAGE_SIZE,
    WT_GUI_PAGE_ORIENTATION,
    WT_GUI_CONSTRAIN,
    WT_GUI_DOUBLE_SPIN,
    WT_GUI_SPIN,
    WT_GUI_FLOATS,
    WT_GUI_PART_SORT,
    WT_GUI_PART_SORT_ORDER,
    WT_GUI_STUD_STYLE_AUTOMATE_EDGE_COLOR,
    WT_GUI_HIGH_CONTRAST_COLOR_SETTINGS,
    WT_GUI_AUTOMATE_EDGE_COLOR_SETTINGS,
    WT_GUI_PART_ANNOTATION_OPTIONS,
    WT_GUI_PART_ANNOTATIONS_STYLES,
    WT_GUI_PART_ANNOTATIONS_SOURCE,
    WT_GUI_PART_ANNOTATIONS_TYPE_BOM,
    WT_GUI_PART_ANNOTATIONS_TYPE_PLI,
    WT_GUI_PART_ELEMENTS_BOM,
    WT_GUI_BUILD_MODIFICATIONS,
    WT_GUI_CONTINUOUS_STEP_NUMBERS,
    WT_GUI_CONSOLIDATE_SUBMODEL_INSTANCE_COUNT,
    WT_GUI_MODEL_VIEW_ON_COVER_PAGE,
    WT_GUI_LOAD_UNOFFICIAL_PART_IN_EDITOR,
    WT_GUI_PREFERRED_RENDERER,
    WT_GUI_DOCUMENT_RESOLUTION,
    WT_GUI_ASSEM_ANNOTATION_TYPE,
    WT_GUI_FADE_PREVIOUS_STEPS,
    WT_GUI_HIGHLIGHT_CURRENT_STEP,
    WT_GUI_FINAL_FADE_HIGHLIGHT_MODEL_STEP,
    WT_GUI_SEPARATOR,
    WT_GUI_STEP_JUSTIFICATION,
    WT_GUI_STEP_ROTATION,
    WT_GUI_SUBMODEL_PREVIEW_DISPLAY,
    WT_GUI_ROTATE_ICON_SIZE,
    WT_NUM_ENTRIES
};

class QAction;
class QMenu;

class CommonMenus
{
public:
  CommonMenus(void);
  ~CommonMenus(void);

  QAction *addAction(
                QMenu &menu,
          const QIcon &icon,
          const QString &text);

  QAction *addNextStepMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *addNextStepsMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *addPrevStepMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *removeStepMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *addMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *annotationMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *constrainMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *textMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *colorMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *changeImageMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *stretchImageMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *tileImageMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *deleteMenu(
                QMenu   &menu,
          const QString &name = "",
          const QString &objectName = "",
          const QString &iconImage = "",
          const QString &itemText = "",
          const QString &whatsThis = "");

  QAction *displayMenu(
                QMenu   &menu,
          const QString &name = "",
          const QString &objectName = "");

  QAction *displayRowsMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *displayColumnsMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *resetViewerImageMenu(
                  QMenu &menu,
          const QString &name = "");

  QAction *copyToClipboardMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *fontMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *marginMenu(
                QMenu   &menu,
          const QString &name = "",
          const QString &whatsThis = "");

  QAction *noPartsListMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *partsListMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *placementMenu(
                QMenu   &menu,
          const QString &name = "",
          const QString &whatsThis = "");

  QAction *sizeMenu(
                QMenu   &menu,
          const QString &name = "",
          const QString &whatsThis = "");

  QAction *subModelColorMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *cameraDistFactorMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *rotStepMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *backgroundMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *borderMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *scaleMenu(
                QMenu   &menu,
          const QString &name = "",
          const QString &whatsThis = "");

  QAction *cameraFoVMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *cameraAnglesMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *refreshImageCacheMenu(
                QMenu   &menu,
          const QString &name = "",
          const QString &objectName = "",
          const QString &iconImage = "",
          const PlacementType  type = PageType);

  QAction *removeMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *rendererArgumentsMenu(
                QMenu   &menu,
          const QString &name = "",
          const QString &objectName = "");

  QAction *sortMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *partGroupsOnMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *partGroupsOffMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *substitutePartMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *overrideCountMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *restoreCountMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *removeSubstitutePartMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *changeSubstitutePartMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *previewPartMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *resetPartGroupMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *hideMenu(
                QMenu   &menu,
          const QString &name = "",
          const QString &objectName = "",
          const QString &iconImage = "");

  QAction *hidePliPartMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *preferredRendererMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *fadeStepsMenu(
                QMenu   &menu,
          const QString &name = "");

  QAction *highlightStepMenu(
                QMenu   &menu,
          const QString &name = "");

  QString naturalLanguagePlacement(
                PlacementType  type,
                PlacementData &placement);

  QString naturalLanguagePlacementWhatsThis(
                PlacementType  type,
                PlacementData &placement,
          const QString       &name = "");

  void setName(
                QAction *action,
          const QString &name = "");

  void addAction(
                QAction *action,
                QMenu   &menu,
          const QString &name = "");

  void setWhatsThis();

  struct WT_Text
  {
      QString text;
  };

  static WT_Text WT[];

private:
  QString englishPlacement(
    PlacementType  type,
    PlacementData &placementData);

  QString englishPlacementWhatsThis(
    PlacementType  type,
    PlacementData &placement,
    const QString &menuName);
};

extern CommonMenus commonMenus;

inline QString lpubWT(WT_Type type, const QString &title)
{
    return QString("%1:\n%2").arg(title,CommonMenus::WT[type].text);
}

#endif
