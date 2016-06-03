/**************************************************************************** 
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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

class QAction;
class QMenu;

class CommonMenus
{
public:
	CommonMenus(void);
	~CommonMenus(void);

  QAction *addMenu(
          QMenu   &menu,
          const QString  name = "");

  QAction *annotationMenu(
          QMenu   &menu,
    const QString  name = "");

  QAction *constrainMenu(
            QMenu   &menu,
      const QString  name = "");

  QAction *colorMenu(
          QMenu   &menu, 
    const QString  name = "");

  QAction *displayMenu(
          QMenu   &menu,
    const QString  name = "");

  QAction *displayRowsMenu(
          QMenu   &menu,
    const QString  name = "");

  QAction *displayColumnsMenu(
          QMenu   &menu,
    const QString  name = "");

  QAction *fontMenu(
          QMenu   &menu,
    const QString  name = "");

  QAction *marginMenu(
          QMenu   &menu, 
    const QString  name = "");

  QString naturalLanguagePlacement(
          PlacementType  type,
          PlacementData &placement);

  QString naturalLanguagePlacementWhatsThis(
          PlacementType  type,
          PlacementData &placement,
    const QString        name = "");

  QAction *noPartsList(
          QMenu   &menu,
    const QString  name = "");

  QAction *partsList(
          QMenu   &menu,
    const QString  name = "");

  QAction *placementMenu(
          QMenu   &menu, 
    const QString  name = "",
    const QString  whatsThis = "");

  QAction *backgroundMenu(
          QMenu   &menu, 
    const QString  name = "");

  QAction *borderMenu(
          QMenu   &menu, 
    const QString  name = "");

  QAction *scaleMenu(  
          QMenu   &menu, 
    const QString  name = "");

  QAction *removeMenu(
            QMenu   &menu,
      const QString  name = "");

  QAction *renderParmsMenu(  
          QMenu   &menu, 
    const QString  name = "");

  QAction *sortMenu(
          QMenu   &menu,
    const QString  name = "");

private:
  QString englishPlacement(
    PlacementType  type,
    PlacementData &placementData);

  QString englishPlacementWhatsThis(
    PlacementType  type,
    PlacementData &placement,
    QString        menuName);

};

extern CommonMenus commonMenus;

#endif
