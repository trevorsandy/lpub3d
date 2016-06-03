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

#include "commonmenus.h"
#include "placement.h"
#include "metatypes.h"

#include <QMenu>
#include <QAction>
#include <QString>
#include "version.h"

CommonMenus::CommonMenus(void)
{
}

CommonMenus::~CommonMenus(void)
{
}

CommonMenus commonMenus;

QAction* CommonMenus::addMenu(
    QMenu   &menu,
    const QString  name)
{
  QAction *action;

  QString formatted = QString("Add %1") .arg(name);
  action = menu.addAction(formatted);
  action->setIcon(QIcon(":/resources/add.png"));

  formatted = QString("Add %1 to the page") .arg(name);
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::annotationMenu(
        QMenu   &menu,
  const QString  name)
{
  QAction *action;

  QString formatted = QString("%1 Annotation Options") .arg(name);
  action = menu.addAction(formatted);
  action->setIcon(QIcon(":/resources/pliannotation.png"));

  formatted = QString("You can set the %1 annotation option to title, free form or both or none") .arg(name);
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::fontMenu(
        QMenu   &menu, 
  const QString  name)
{
  QAction *action;

  QString formatted = QString("Change %1 Font") .arg(name);
  action = menu.addAction(formatted);
  action->setIcon(QIcon(":/resources/font.png"));

  formatted = QString(
   "You can change the font or edit the size for this %1") .arg(name);

  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::constrainMenu(
    QMenu   &menu,
    const QString  name)
{
  QAction *action;
  QString formatted = QString("Change %1 shape") .arg(name);
  action = menu.addAction(formatted);
  action->setIcon(QIcon(":/resources/changeshape.png"));

  formatted = QString("Change %1 Shape:\n"
                              "  You can change the shape of this parts list.  One way, is\n"
                              "  is to ask the computer to make the parts list as small as\n"
                              "  possible (area). Another way is to ask the computer to\n"
                              "  make it as close to square as possible.  You can also pick\n"
                              "  how wide you want it, and the computer will make it as\n"
                              "  tall as is needed.  Another way is to pick how tall you\n"
                              "  and it, and the computer will make it as wide as it needs.\n"
                              "  The last way is to tell the computer how many columns it\n"
                              "  can have, and then it will try to make all the columns the\n"
                              "  same height\n") .arg(name);
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::colorMenu(
        QMenu   &menu, 
  const QString  name)
{
  QAction *action;

  QString formatted = QString("Change %1 Color") .arg(name);
  action = menu.addAction(formatted);
  action->setIcon(QIcon(":/resources/fontColour.png"));

  formatted = QString("You can change the color of this %1") .arg(name);
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::marginMenu(
        QMenu   &menu, 
  const QString  name)
{
  QAction *action;

  QString formatted = QString("Change %1 Margins") .arg(name);
  action = menu.addAction(formatted);
  action->setIcon(QIcon(":/resources/margins.png"));

  formatted = QString("You can change how much empty space their is around this %1") .arg(name);
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::displayMenu(
      QMenu   &menu,
  const QString  name)
{
QAction *action;

QString formatted = QString("Hide %1") .arg(name);
action = menu.addAction(formatted);
action->setIcon(QIcon(":/resources/display.png"));

formatted = QString("You can turn off and on the display of %1") .arg(name);
action->setWhatsThis(formatted);

return action;
}

QAction* CommonMenus::displayRowsMenu(
      QMenu   &menu,
  const QString  name)
{
QAction *action;

QString formatted = QString("Display %1 as Rows") .arg(name);
action = menu.addAction(formatted);
action->setIcon(QIcon(":/resources/displayrow.png"));

formatted = QString("Display %1 as Rows:\n"
      "  Change this whole set of %1 from columns of %1\n"
      "  to rows of %1.") .arg(name);
action->setWhatsThis(formatted);

return action;
}

QAction* CommonMenus::displayColumnsMenu(
      QMenu   &menu,
  const QString  name)
{
QAction *action;

QString formatted = QString("Display %1 as Columns") .arg(name);
action = menu.addAction(formatted);
action->setIcon(QIcon(":/resources/displaycolumn.png"));

formatted = QString("Display %1 as Columns:\n"
      "  Change this whole set of %1 from rows of %1\n"
      "  to columns of %1.") .arg(name);
action->setWhatsThis(formatted);

return action;
}

QAction* CommonMenus::partsList(
      QMenu   &menu,
  const QString  name)
{
QAction *action;

QString formatted = QString("Parts List per %1") .arg(name);
action = menu.addAction(formatted);
action->setIcon(QIcon(":/resources/partslist.png"));

formatted = QString("Show Parts List per %1.") .arg(name);
action->setWhatsThis(formatted);

return action;
}

QAction* CommonMenus::noPartsList(
      QMenu   &menu,
  const QString  name)
{
QAction *action;

QString formatted = QString("No Parts List per %1") .arg(name);
action = menu.addAction(formatted);
action->setIcon(QIcon(":/resources/nopartslist.png"));

formatted = QString("Do not show Parts List per %1.") .arg(name);
action->setWhatsThis(formatted);

return action;
}

QAction* CommonMenus::placementMenu(
        QMenu   &menu,
  const QString  name,
  const QString  whatsThis)
{
  QAction *action;
  QString formatted = QString("Move %1") .arg(name);

  action = menu.addAction(formatted);
  action->setWhatsThis(whatsThis);
  action->setIcon(QIcon(":/resources/placement.png"));

  return action;
}

QAction* CommonMenus::backgroundMenu(
        QMenu   &menu,
  const QString  name)
{
  QAction *action;

  QString formatted = QString("Change %1 Background") .arg(name);
  action = menu.addAction(formatted);
  action->setIcon(QIcon(":/resources/background.png"));

  formatted = QString("You can change the color or use a picture for the %1 background") .arg(name);
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::borderMenu(
        QMenu   &menu,
  const QString  name)
{
  QAction *action;
  QString formatted = QString("Change %1 Border") .arg(name);

  action = menu.addAction(formatted);
  action->setWhatsThis("You can pick a square corner border, round corner border or no border at all");
  action->setIcon(QIcon(":/resources/border.png"));

  return action;
}

QAction* CommonMenus::removeMenu(
      QMenu   &menu,
  const QString  name)
{
QAction *action;

QString formatted = QString("Remove %1") .arg(name);
action = menu.addAction(formatted);
action->setIcon(QIcon(":/resources/remove.png"));

formatted = QString("Remove %1 from the page") .arg(name);
action->setWhatsThis(formatted);

return action;
}

QAction* CommonMenus::scaleMenu(
        QMenu   &menu,
  const QString  name)
{
  QString formatted = QString("Change %1 Scale") .arg(name);

  QAction *action;
  action = menu.addAction(formatted);
  action->setIcon(QIcon(":/resources/scale.png"));

  action->setWhatsThis(

    "You can make %1look bigger or smaller on the page by changing the scale."

    "A scale of 1 is true size.  A scale of 2 is double true size\n");

  return action;
}

QAction* CommonMenus::sortMenu(
        QMenu   &menu,
  const QString  name)
{
  QAction *action;

  QString formatted = QString("Sort %1") .arg(name);
  action = menu.addAction(formatted);
  action->setIcon(QIcon(":/resources/sortPli.png"));

  formatted = QString("You can sort the %1 by part colour, part category or image size") .arg(name);
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::renderParmsMenu(
        QMenu   &menu,
  const QString  name)
{
  QString formatted = QString("Change %1 Render Parameters") .arg(name);
  QAction *action;

  action = menu.addAction(formatted);
  action->setEnabled(false);

  action->setWhatsThis("This is not implemented yet");

  return action;
}

QString placement2english(PlacementEnc placement)

{

  switch (placement) {

    case TopLeft:

      return "top left";

    break;

    case TopRight:

      return "top right";

    break;

    case BottomRight:

      return "bottom right";

    break;

    case BottomLeft:

      return "bottom left";

    break;

    case Top:

      return "top";

    break;

    case Bottom:

      return "bottom";

    break;

    case Left:

      return "left side";

    break;

    case Right:

      return "right side";

    break;

    case Center:

      return "center";

    break;

    default:

      return "";

    break;

  }

}



QString type2english(PlacementType type)

{

    switch (type) {

    case PageType:

        return "page";

        break;

    case CsiType:

        return "step assembly image";

        break;

    case StepGroupType:

        return "group of steps";

        break;

    case StepNumberType:

        return "step number";

        break;

    case PartsListType:

        return "parts list";

        break;

    case CalloutType:

        return "callout";

        break;

    case PageNumberType:

        return "page number";

        break;

    case PageTitleType:

        return "document title";

        break;

    case PageModelNameType:

        return "model id";

        break;

    case PageAuthorType:

        return "author";

        break;

    case PageURLType:

        return "publisher url";

        break;

    case PageDocumentLogoType:

        return "publisher logo";

        break;

    case PageModelDescType:

        return "model description";

        break;

    case PagePublishDescType:

        return "publish description";

        break;

    case PageCopyrightType:

        return "copyright notice";

        break;

    case PageEmailType:

        return "publisher email";

        break;

    case PageDisclaimerType:

        return "LEGO © disclaimer";

        break;

    case PagePiecesType:

        return "number of pieces";

        break;

    case PagePlugType:

        return  "application plug";

        break;

    case PageCategoryType:

        return "model category";

        break;

    case PagePlugImageType:

        return "application logo";

        break;

    default:

        return "";

        break;

    }

}



QString type2english2(PlacementType type)

{
  switch (type) {
    case PageType:
      return "this page";

    break;

    case CsiType:

      return "a step assembly image";

    break;

    case StepGroupType:

      return "a group of steps";

    break;

    case StepNumberType:

      return "a step number";

    break;

    case PartsListType:

      return "a parts list";

    break;

    case CalloutType:

      return "a callout";

    break;

    case PageNumberType:

      return "the page number";

    break;

  case PageTitleType:

      return "the document title";

      break;

  case PageModelNameType:

      return "the model id";

      break;

  case PageAuthorType:

      return "the author";

      break;

  case PageURLType:

      return "the publisher url";

      break;

  case PageDocumentLogoType:

      return "the publisher logo";

      break;

  case PageModelDescType:

      return "the model description";

      break;

  case PagePublishDescType:

      return "the publish description";

      break;

  case PageCopyrightType:

      return "the copyright notice";

      break;

  case PageEmailType:

      return "the publisher email";

      break;

  case PageDisclaimerType:

      return "the LEGO © disclaimer";

      break;

  case PagePiecesType:

      return "the number of pieces";

      break;

  case PagePlugType:

      return  "the application plug";

      break;

  case PageCategoryType:

      return "the model category";

      break;

  case PagePlugImageType:

      return "the application logo";

      break;

  default:
      return "";
      break;
  }
}

QString CommonMenus::englishPlacement(
  PlacementType  myType,
  PlacementData &placementData)
{
  QString english;
  QString type          = type2english(myType);
  QString relativeTo    = type2english2(placementData.relativeTo);
  QString placement     = placement2english(placementData.placement);
  QString justification = placement2english(placementData.justification);
  QString preposition;

  if (placementData.preposition == Inside) {

    preposition = "inside";

  } else {

    preposition = "outside";

  }



  switch (placementData.placement) {

    case TopLeft:
    case TopRight:
    case BottomRight:
    case BottomLeft:
      english = QString("This %1 is placed in the %2 %3 corner of %4.")
                .arg(type) .arg(placement) .arg(preposition) .arg(relativeTo);

    break;
    case Top:
    case Bottom:
    case Left:
    case Right:
      if (placementData.preposition == Inside) {

        english = QString("this %1 is placed inside %2 against the center of the %3.")
                  .arg(type) .arg(relativeTo) .arg(placement);

      } else {

        english = QString("this %1 is placed outside %2 on the %3, slid to the %4.")
                  .arg(type) .arg(relativeTo) .arg(placement) .arg(justification);

      }
    break;
    case Center:
      english = QString("this %1 is placed in the center of %2.")
                .arg(type) .arg(relativeTo);
    break;
    default:
    break;
  }

  return english;

}

QString CommonMenus::naturalLanguagePlacement(
  PlacementType  type,
  PlacementData &placement)
{

  return englishPlacement(type,placement);

}

QString CommonMenus::naturalLanguagePlacementWhatsThis(
        PlacementType  type,
        PlacementData &placement,
  const QString        name)
{

  return englishPlacementWhatsThis(type,placement,name);

}

QString CommonMenus::englishPlacementWhatsThis(
        PlacementType  type,
        PlacementData &placementData,
  const QString        name)
{
  QString whatsThis;

  whatsThis = name + ":\n";
  whatsThis  += "LPub lets you put things next to other things.  In this case,\n" +
                englishPlacement(type,placementData) + "\n\n" + 
                "Clicking \"" + name + "\" pops up a window with a list of\n"
                "things you can put this " + type2english(type) + " next to.  It also\n" + 
                "lets you describe how it is placed next to the thing you choose.\n\n";

  if (placementData.offsets[XX] != 0 || placementData.offsets[YY] != 0) {

    whatsThis += "You've already discovered that you can click and drag\n"
                 "this " + type2english(type) + "around.  Great job, but there's more.\n\n";
  } else {

    whatsThis += "Did you know that you can click and drag this " + type2english(type) + 
                 " around using the mouse?\n\n";
  }

  return whatsThis;
} 

