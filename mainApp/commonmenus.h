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
    /*************************************
     *  PREFERENCES ENTRIES
     *************************************/
    WT_PREFERENCES,
    WT_PREFERENCES_DIRECTORIES,
    WT_PREFERENCES_GENERAL,
    WT_PREFERENCES_LDGLITE,
    WT_PREFERENCES_LDVIEW,
    WT_PREFERENCES_LOGGING,
    WT_PREFERENCES_NATIVE,
    WT_PREFERENCES_OTHER,
    WT_PREFERENCES_POVRAY,
    WT_PREFERENCES_PUBLISHING,
    WT_PREFERENCES_RENDERERS,
    WT_PREFERENCES_SHORTCUTS,
    WT_PREFERENCES_UPDATES,
    WT_VISUAL_PREFS,
    WT_VISUAL_PREFS_CATEGORIES,
    WT_VISUAL_PREFS_COLORS,
    WT_VISUAL_PREFS_GENERAL,
    WT_VISUAL_PREFS_KEYBOARD,
    WT_VISUAL_PREFS_MOUSE,
    WT_VISUAL_PREFS_RENDERING,
    WT_VISUAL_PREFS_USER_INTERFACE,
    WT_VISUAL_PREFS_VIEWS,
    /*************************************
     *  GLOBAL SETUP ENTRIES
     *************************************/
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
    /*************************************
     *  EDITOR ENTRIES
     *************************************/
    WT_COMMAND_EDIT,
    WT_COMMAND_EDIT_DET,
    WT_COMMAND_PREFS,
    WT_COMMANDS,
    WT_COMMANDS_EDIT,
    WT_COMMANDS_VIEW,
    WT_PARAMS_EDIT,
    WT_SHORTCUTS,
    WT_SNIPPETS_EDIT,
    WT_SNIPPETS_VIEW,
    WT_TEXT_EDIT,
    WT_VISUAL_EDIT,
    /*************************************
     *  DIALOG ENTRIES
     *************************************/
    WT_DIALOG_BACKGROUND,
    WT_DIALOG_BOM_OPTION,
    WT_DIALOG_BORDER,
    WT_DIALOG_CAMERA_ANGLES,
    WT_DIALOG_CAMERA_FOV,
    WT_DIALOG_CONSTRAIN,
    WT_DIALOG_EXPORT_PAGES,
    WT_DIALOG_DIVIDER,
    WT_DIALOG_DOUBLE_SPIN,
    WT_DIALOG_FADE_HIGHLIGHT,
    WT_DIALOG_FIND_REPLACE,
    WT_DIALOG_FLOAT,
    WT_DIALOG_FLOAT_PAIR,
    WT_DIALOG_GLOBAL_HIGHLIGHT_STEP,
    WT_DIALOG_GRADIENT,
    WT_DIALOG_LDRAW_COLOR,
    WT_DIALOG_LDRAW_FILES_LOAD,
    WT_DIALOG_LDRAW_SEARCH_DIR,
    WT_DIALOG_LOCAL,
    WT_DIALOG_OPTION,
    WT_DIALOG_PAGE_ORIENTATION,
    WT_DIALOG_PAGE_SIZE,
    WT_DIALOG_PLACEMENT,
    WT_DIALOG_PLI_ANNOTATION,
    WT_DIALOG_PLI_SORT,
    WT_DIALOG_POINTER_ATTRIB,
    WT_DIALOG_POINTER_PLACEMENT,
    WT_DIALOG_PREFERRED_RENDERER,
    WT_DIALOG_RENDER,
    WT_DIALOG_ROTATE_ICON_SIZE,
    WT_DIALOG_ROT_STEP,
    WT_DIALOG_SIZE_ANDORIENTATION,
    WT_DIALOG_SUBMODEL_COLOR,
    WT_DIALOG_SUBSTITUTE_PART,
    WT_DIALOG_UNITS,
    /*************************************
     *  METAGUI ENTRIES
     *************************************/
    WT_GUI_SIZE_ORIENTATION,
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

  QAction *clearImageCacheMenu(
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
