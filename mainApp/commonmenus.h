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

#endif
