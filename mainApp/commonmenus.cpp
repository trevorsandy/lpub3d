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

#include <QObject>
#include <QMenu>
#include <QAction>
#include <QString>

#include "commonmenus.h"
#include "placement.h"
#include "metatypes.h"
#include "version.h"

CommonMenus::CommonMenus(void)
{
}

CommonMenus::~CommonMenus(void)
{
}

CommonMenus commonMenus;

QAction* CommonMenus::addAction(
              QMenu   &menu,
        const QIcon   &icon,
        const QString &text)
{
  QAction *action = menu.addAction(QIcon(icon),text);
  action->setParent(menu.parent());
  action->setProperty("name", DEF_ITEM);

  return action;
}

QAction* CommonMenus::addNextStepMenu(
          QMenu   &menu,
    const QString &)
{
  QAction *action;

  QString formatted = QObject::QObject::tr("Add Next Step");
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("addNextStepAction.1");
  action->setProperty("name", DEF_ITEM);
  action->setIcon(QIcon(":/resources/addnext.png"));

  formatted = QObject::tr("Add the first step of the next page to this page");
  action->setStatusTip(formatted);

  formatted = QObject::tr("Add Next Step:\n"
                          "  You can add the first step of the next page to this page.\n");
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::addNextStepsMenu(
          QMenu   &menu,
    const QString &)
{
  QAction *action;

  QString formatted = QObject::QObject::tr("Add Next Steps...");
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("addNextStepsAction.1");
  action->setProperty("name", DEF_ITEM);
  action->setIcon(QIcon(":/resources/nextsteps.png"));

  formatted = QObject::tr("Add a specified number of next steps to this page");
  action->setStatusTip(formatted);

  formatted = QObject::tr("Add Next Steps:\n"
                          "  You can add a specified number of next steps to this page.\n");
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::addPrevStepMenu(
          QMenu   &menu,
    const QString &)
{
  QAction *action;

  QString formatted = QObject::QObject::tr("Add Previous Step");
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("addPrevStepAction.1");
  action->setProperty("name", DEF_ITEM);
  action->setIcon(QIcon(":/resources/previousstep.png"));

  formatted = QObject::tr("Add the last step of the previous page to this page");
  action->setStatusTip(formatted);

  formatted = QObject::tr("Add Previous Step:\n"
                          "  You can add the last step of the previous page to this page.\n");
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::removeStepMenu(
          QMenu   &menu,
    const QString &)
{
  QAction *action;

  QString formatted = QObject::QObject::tr("Remove this Step");
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("removeStepAction.1");
  action->setProperty("name", DEF_ITEM);
  action->setIcon(QIcon(":/resources/remove.png"));

  formatted = QObject::tr("Move this step from this page to the next page");
  action->setStatusTip(formatted);

  formatted = QObject::tr("Remove this Step:\n"
                          "  You can move this step from this page to the next page.\n");
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::addMenu(
          QMenu   &menu,
    const QString &name)
{
  QAction *action;

  QString formatted = QObject::QObject::tr("Add %1") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("addAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/add.png"));

  formatted = QObject::tr("Add %1 to the page") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Add %1:\n"
                          "  You can add %2 to the page.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::annotationMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("%1 Annotation Options...") .arg(name);

  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("annotationAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/pliannotation.png"));

  formatted = QObject::tr("Set the %1 annotation option to title, free form or both or none") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("%1 Annotation Options:\n"
                          "  You can set the %2 annotation option to title,\n"
                          "  free form or both or none.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::resetViewerImageMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Reset %1 Viewer Image") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("resetViewerImageAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/resetviewerimage.png"));

  formatted = QObject::tr("Reset %1 viewer image to its original position") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Reset %1 Viewer Image:\n"
                          "  You can reset %2 viewer image to its original position.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::copyToClipboardMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Copy %1 Image Path To Clipboard") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("copyToClipboardAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/copytoclipboard.png"));

  formatted = QObject::tr("Copy %1 image path to the clipboard") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Copy %1 Image Path To Clipboard:\n"
                          "  You can copy %2 image path to the clipboard.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::fontMenu(
        QMenu   &menu, 
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Change %1 Font...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("fontAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/font.png"));

  formatted = QObject::tr("Change the font or edit the size for this %1") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Change %1 Font:\n"
                          "  You can change the font or edit the size for this %2.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::constrainMenu(
          QMenu   &menu,
    const QString &name)
{
  QAction *action;
  QString formatted = QObject::tr("Change %1 Shape...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("constrainAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/changeshape.png"));

  formatted = QObject::tr("Change the shape of this %1") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Change %1 Shape:\n"
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

QAction* CommonMenus::textMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Edit %1...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("textAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/textedit.png"));

  formatted = QObject::tr("Edit %1 text with this dialog") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Edit %1:\n"
                          "  You can edit %2 text with this dialog.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::colorMenu(
        QMenu   &menu, 
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Change %1 Color...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("colorAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/fontColour.png"));

  formatted = QObject::tr("Change the color of this %1") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Change %1 Color:\n"
                          "  You can change the color of this %2.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::changeImageMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Change %1 Image...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("changeImageAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/editimage.png"));

  formatted = QObject::tr("Can change the %1 image") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Change %1 Image:\n"
                          "  You can change the %2 image.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::stretchImageMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Stretch %1 Image") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("stretchImageAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/stretchimage.png"));

  formatted = QObject::tr("Stretch the %1 image to the size of the page") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Stretch %1 Image:\n"
                          "  You can stretch the %2 image to the size of the page.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::tileImageMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Tile %1 Image") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("tileImageAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/tileimage.png"));

  formatted = QObject::tr("Tile the %1 image across the page") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Tile %1 Image:\n"
                          "  You can tile the %2 image across the page.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::marginMenu(
        QMenu   &menu,
  const QString &name,
  const QString &whatsThis)
{
  QAction *action;

  QString formatted = QObject::tr("Change %1 Margins...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("marginAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/margins.png"));

  formatted = QObject::tr("Change how much empty space there is around this %1") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Change %1 Margins:\n"
                          "  You can change how much empty space there is\n"
                          "  around this %2.\n") .arg(name, name.toLower());
  action->setWhatsThis(whatsThis.isEmpty() ? formatted : whatsThis);

  return action;
}

QAction* CommonMenus::deleteMenu(
        QMenu   &menu,
  const QString &name,
  const QString &objectName,
  const QString &iconImage,
  const QString &itemText,
  const QString &whatsThis)
{
  QAction *action;

  QString formatted = QObject::tr("Delete %1") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName(objectName.isEmpty() ? "deleteAction.1" : objectName);
  action->setProperty("name", name);
  action->setIcon(QIcon(iconImage.isEmpty() ? ":/resources/delete.png" : ":/resources/" + iconImage));

  const QString item = itemText.isEmpty() ? QObject::tr("build instructions") : itemText;

  formatted = QObject::tr("Remove the %1 from the %2") .arg(name.toLower(), item);
  action->setStatusTip(formatted);

  formatted = QObject::tr("Delete %1:\n"
                          "  You can remove this %2 from the %3.\n") .arg(name, name.toLower(), item);
  action->setWhatsThis(whatsThis.isEmpty() ? formatted : whatsThis);

  return action;
}

QAction* CommonMenus::displayMenu(
        QMenu   &menu,
  const QString &name,
  const QString &objectName)
{
  QAction *action;

  QString formatted = QObject::tr("Hide %1") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName(objectName.isEmpty() ? "displayAction.1" : objectName);
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/display.png"));

  formatted = QObject::tr("Turn off and on %1 display") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Hide %1:\n"
                          "  You can turn off and on %2 display.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::displayRowsMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Display %1 As Rows") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("displayRowsAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/displayrow.png"));

  formatted = QObject::tr("Display %1 as rows on the page") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Display %1 as Rows:\n"
                          "  Change this whole set of %1 from columns of %1\n"
                          "  to rows of %2.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::displayColumnsMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Display %1 As Columns") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("displayColumnsAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/displaycolumn.png"));

  formatted = QObject::tr("Display %1 as columns on the page") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Display %1 as Columns:\n"
                          "  Change this whole set of %1 from rows of %1\n"
                          "  to columns of %2.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::partsListMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Parts List Per %1") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("partsListAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/partslist.png"));

  formatted = QObject::tr("Show parts list per %1.") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Parts List Per %1:\n"
                          "  You can choose to show the parts list per %2.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::noPartsListMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("No Parts List Per %1") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("noPartsListAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/nopartslist.png"));

  formatted = QObject::tr("Do not show parts list per %1") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("No Parts List Per %1:\n"
                          "  You can choose to not show parts list per %2.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::placementMenu(
        QMenu   &menu,
  const QString &name,
  const QString &whatsThis)
{
  QAction *action;
  QString formatted = QObject::tr("Change %1 Placement...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("placementAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/placement.png"));

  formatted = QObject::tr("Change the initial %1 placement relative to options provided in the placement dialog") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Change %1 Placement\n"
                          "  You can change the initial %2 placement relative to\n"
                          "  options provided in the placement dialog.\n") .arg(name, name.toLower());
  action->setWhatsThis(whatsThis.isEmpty() ? formatted : whatsThis);

  return action;
}

QAction* CommonMenus::sizeMenu(
        QMenu   &menu,
  const QString &name,
  const QString &whatsThis)
{
  QAction *action;

  QString formatted = QObject::tr("Change %1 Size...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("sizeAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/scale.png"));

  formatted = QObject::tr("Change the size of this %1 using the size dialog").arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Change %1 Size:\n"
                          "  You can change the size of this %2 using the\n"
                          "  size dialog.\n").arg(name, name.toLower());
  action->setWhatsThis(whatsThis.isEmpty() ? formatted : whatsThis);

  return action;
}

QAction* CommonMenus::subModelColorMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Change %1 Submodel Color...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("subModelColorAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/submodelcolor.png"));

  formatted = QObject::tr("Change the %1 submodel level background color") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Change %1 Submodel Color:\n"
                          "  You can change the %2 Submodel level background color.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::cameraDistFactorMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Change %1 Camera Distance...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("cameraDistFactorAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/cameradistfactor.png"));

  formatted = QObject::tr("Change the %1 native camera distance factor") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Change %1 Camera Distance:\n"
                          "  You can change the %2 native camera distance factor.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::rotStepMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Change %1 Rotation...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("rotStepAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/rotateicon.png"));

  formatted = QObject::tr("Change the %1 rotation angles and transform") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Change %1 Rotation:\n"
                          "  You can change the %2 Rotation angles and Transform.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::backgroundMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Change %1 Background...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("backgroundAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/background.png"));

  formatted = QObject::tr("Change the color or use a picture for the %1 background") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Change %1 Background:\n"
                          "  You can change the color or use a picture for\n"
                          "  the %2 background.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::borderMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Change %1 Border...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("borderAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/border.png"));

  formatted = QObject::tr("Set a square corner border, round corner border or no border at all for this %1") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Change %1 Border:\n"
                          "  You can set a square corner border, round corner border\n"
                          "  or no border at all for this %2.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::removeMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Remove %1") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("removeAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/remove.png"));

  formatted = QObject::tr("Remove %1 from the page") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Remove %1:\n"
                          "  You can remove %2 from the page.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::scaleMenu(
        QMenu   &menu,
  const QString &name,
  const QString &whatsThis)
{
  QAction *action;

  QString formatted = QObject::tr("Change %1 Scale...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("scaleAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/scale.png"));

  formatted = QObject::tr("Change the size of this %1 using the scale dialog") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = whatsThis.isEmpty() ? QObject::tr("Change %1 Scale:\n"
                                                "  You can change the size of this %2 using the scale dialog.\n"
                                                "  A scale of 1.0 is true size. A scale of 2.0 doubles\n"
                                                "  the size of your model.\n"
                                                "  A scale of 0.5 makes your model half real size\n").arg(name, name.toLower()) : whatsThis;
  action->setWhatsThis(whatsThis.isEmpty() ? formatted : whatsThis);

  return action;
}

QAction* CommonMenus::cameraFoVMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Change %1 Camera FOV Angle...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("cameraFoVAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/camerafov.png"));

  formatted = QObject::tr("Change the camera field of view angle in degrees for this %1") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Change %1 Camera FOV Angle:\n"
                          "  You can change the camera field of view angle\n"
                          "  in degrees for this %2.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::cameraAnglesMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Change %1 Camera Angles...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("cameraAnglesAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/cameraviewangles.png"));

  formatted = QObject::tr("Change the camera longitude and latitude view angles for this %1") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Change %1 Camera Angles:\n"
                          "  You can change the camera longitude and latitude\n"
                          "  view angles for this %2.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::clearImageCacheMenu(
        QMenu   &menu,
  const QString &name,
  const QString &objectName,
  const QString &iconImage,
  const PlacementType type)
{
  QAction *action;

  QString formatted = QObject::tr("Reset %1 Image Cache") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName(objectName.isEmpty() ? "clearImageCacheAction.1" : objectName);
  action->setProperty("name", name);
  action->setIcon(QIcon(iconImage.isEmpty() ? ":/resources/clearpagecache.png" : ":/resources/" + iconImage));

  const QString imagelabel = type != CsiType && type != SubModelType ? QObject::tr("images") : QObject::tr("image");

  formatted = QObject::tr("Clear the %1 %2 and working LDraw (.ldr) cache files") .arg(name.toLower(),imagelabel);
  action->setStatusTip(formatted);

  formatted = QObject::tr("Reset %1 Image Cache:\n"
                          "  You can clear this %2 %3 and working LDraw (.ldr)\n"
                          "  cache files.\n") .arg(name, name.toLower(), imagelabel);
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::sortMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Sort %1...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("sortAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/sortPli.png"));

  formatted = QObject::tr("Sort the %1 by part color, part category or image size") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Sort %1:\n"
                          "  You can sort the %2 by part color, part category\n"
                          "  or image size.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::partGroupsOnMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Turn On Movable %1 Group") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("partGroupsOnAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/enablegroupmove.png"));

  formatted = QObject::tr("Set %1 groups, image, annotaiton and instance count, movable") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Turn On Movable %1 Group:\n"
                          "  You can set %2 groups, part mage, annotaiton and\n"
                          "  instance count, movable.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::partGroupsOffMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Turn Off Movable %1 Group") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("partGroupsOffAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/disablegroupmove.png"));

  formatted = QObject::tr("Set %1 groups, image, annotaiton and instance count, non-movable") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Turn Off Movable %1 Group:\n"
                          "  You can set %2 groups, image, annotaiton and instance count,\n"
                          "  non-movable.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::substitutePartMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Substitute %1...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("substitutePartAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/substitutepart.png"));

  formatted = QObject::tr("Replace this %1 with a substitute") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Substitute %1:\n"
                          "  You can replace this %2 with a substitute.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::overrideCountMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Override %1") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("overrideCountAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/submodelsetup.png"));

  formatted = QObject::tr("Replace this %1 instance count with a manually entered value.") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Override %1:\n"
                          "  You can replace this %2 instance count with a\n"
                          "  manually entered value.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::restoreCountMenu( 
        QMenu   &menu, 
  const QString &name)
{ 
  QAction *action; 
 
  QString formatted = QObject::tr("Restore %1") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("restoreCountAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/clearsubmodelcache.png"));
 
  formatted = QObject::tr("Restore the default %1 instance count.") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Restore %1:\n"
                          "  You can restore the default %2 instance count.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted); 

  return action;
} 

QAction* CommonMenus::changeSubstitutePartMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Change Substitute %1") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("changeSubstitutePartAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/substitutepart.png"));

  formatted = QObject::tr("Change this substitute %1 with another part") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Change Substitute %1:\n"
                          "  You can change this substitute %2 with another part.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::removeSubstitutePartMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Remove Substitute %1") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("removeSubstitutePartAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/removesubstitutepart.png"));

  formatted = QObject::tr("Remove this substitute %1 with the original part") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Remove Substitute %1:\n"
                          "  You can remove this substitute %2 with the\n"
                          "  original part.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::resetPartGroupMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Reset %1 Group") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("resetPartGroupAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/disablegroupmove.png"));

  formatted = QObject::tr("Reset this %1 group - remove LPub meta command") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Reset %1 Group:\n"
                          "  You can reset this %2 group - remove LPub meta command.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::previewPartMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Preview %1...") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("previewPartAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/previewpart.png"));

  formatted = QObject::tr("Display the %1 in a popup 3D Viewer") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Preview %1:\n"
                          "  You can display the %2 in a popup viewer.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::hideMenu(
        QMenu   &menu,
  const QString &name,
  const QString &objectName,
  const QString &iconImage)
{
  QAction *action;

  QString formatted = QObject::tr("Hide %1") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName(objectName.isEmpty() ? "hideAction.1" : objectName);
  action->setProperty("name", name);
  action->setIcon(QIcon(iconImage.isEmpty() ? ":/resources/display.png" : ":/resources/" + iconImage));

  formatted = QObject::tr("Hide %1 in the page display") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Hide %1:\n"
                          "  You can hide %2 in the page display.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::hidePliPartMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Hide %1") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("hidePliPartAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/display.png"));

  formatted = QObject::tr("Hide %1 in the Parts List") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Hide %1:\n"
                          "  You can hide %2 in the Parts List.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::preferredRendererMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Change %1 Renderer") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("preferredRendererAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/preferredrenderer.png"));

  formatted = QObject::tr("Select the %1 image renderer and settings from available renderers") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Change %1 Renderer:\n"
                          "  You can select the %2 image renderer and settings\n"
                          "  from available renderers.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::fadeStepsMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Fade Previous %1 Steps") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("fadeStepsAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/fadesteps.png"));

  formatted = QObject::tr("Turn on or off fade previous steps for this %1 image") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Fade Previous %1 Steps:\n"
                          "  You can turn on or off fade previous steps for\n"
                          "  this %2 image.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::highlightStepMenu(
        QMenu   &menu,
  const QString &name)
{
  QAction *action;

  QString formatted = QObject::tr("Highlight Current %1 Step") .arg(name);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName("highlightStepAction.1");
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/highlightstep.png"));

  formatted = QObject::tr("Turn on or off highlight current step for this %1 image") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Highlight Current %1 Step:\n"
                          "  You can turn on or off highlight current step for\n"
                          "  this %2 image.\n") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::rendererArgumentsMenu(
        QMenu   &menu,
  const QString &name,
  const QString &objectName)
{
  QAction *action;

  const bool povGenerator = Preferences::preferredRenderer == RENDERER_POVRAY && !objectName.isEmpty();
  const QString rendererName = povGenerator ? Preferences::useNativePovGenerator ? rendererNames[RENDERER_NATIVE] : rendererNames[RENDERER_LDVIEW] :
                                           rendererNames[Preferences::preferredRenderer];
  const QString renderer = QString("%1 %2 %3").arg(rendererName, name, povGenerator ? QObject::tr("Generator") : QObject::tr("Renderer"));

  QString formatted = QObject::tr("Add %1 Arguments") .arg(renderer);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName(objectName.isEmpty() ? "renderArgumentsAction.1" : objectName);
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/display.png"));

  formatted = QObject::tr("Add %1 arguments") .arg(renderer.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Add %1 Arguments:\n"
                          "  You can add %2 arguments for this %3.\n") .arg(name, renderer, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

void CommonMenus::setName(
        QAction *action,
  const QString &name)
{
  const QString pname    = action->property("name").value<QString>();
  const QString pnameLow = pname.toLower();
  const QString nameLow  = name.toLower();

  action->setProperty("name", name);
  action->setText(action->text().replace(pname, name));
  action->setStatusTip(action->statusTip().replace(pnameLow, nameLow));
  if (!action->whatsThis().isEmpty())
    action->setWhatsThis(action->whatsThis().replace(pname, name).replace(pnameLow, nameLow));
}

void CommonMenus::addAction(
              QAction *action,
              QMenu   &menu,
        const QString &name)
{
  if (!name.isEmpty())
    setName(action,name);

  menu.addAction(action);
}

QString placement2english(PlacementEnc placement)
{
    switch (placement)
    {

        case TopLeft:
            return QObject::tr("top left");
            break;

        case TopRight:
            return QObject::tr("top right");
            break;

        case BottomRight:
            return QObject::tr("bottom right");
            break;

        case BottomLeft:
            return QObject::tr("bottom left");
            break;

        case Top:
            return QObject::tr("top");
            break;

        case Bottom:
            return QObject::tr("bottom");
            break;

        case Left:
            return QObject::tr("left side");
            break;

        case Right:
            return QObject::tr("right side");
            break;

        case Center:
            return QObject::tr("center");
            break;

        default:
            return "";
            break;
    }
}

QString type2english(PlacementType type)
{
    switch (type)
    {
        case PageType:
            return QObject::tr("page");
            break;

        case CsiType:
            return QObject::tr("step assembly image");
            break;

        case StepGroupType:
            return QObject::tr("group of steps");
            break;

        case StepNumberType:
            return QObject::tr("step number");
            break;

        case PartsListType:
            return QObject::tr("parts list");
            break;

        case SubModelType:
            return QObject::tr("submodel");
            break;

        case CalloutType:
            return QObject::tr("callout");
            break;

        case PageNumberType:
            return QObject::tr("page number");
            break;

        case PageTitleType:
            return QObject::tr("document title");
            break;

        case PageModelNameType:
            return QObject::tr("model id");
            break;

        case PageAuthorType:
            return QObject::tr("author");
            break;

        case PageURLType:
            return QObject::tr("publisher url");
            break;

        case PageDocumentLogoType:
            return QObject::tr("publisher logo");
            break;

        case PageModelDescType:
            return QObject::tr("model description");
            break;

        case PagePublishDescType:
            return QObject::tr("publish description");
            break;

        case PageCopyrightType:
            return QObject::tr("copyright notice");
            break;

        case PageEmailType:
            return QObject::tr("publisher email");
            break;

        case PageDisclaimerType:
            return QObject::tr("LEGO © disclaimer");
            break;

        case PagePartsType:
            return QObject::tr("number of parts");
            break;

        case PagePlugType:
            return QObject::tr("application plug");
            break;

        case PageCategoryType:
            return QObject::tr("model category");
            break;

        case PagePlugImageType:
            return QObject::tr("application logo");
            break;

        default:
            return "";
            break;
    }
}

QString type2english2(PlacementType type)
{
    switch (type)
    {
        case PageType:
            return QObject::tr("this page");
            break;

        case CsiType:
            return QObject::tr("a step assembly image");
            break;

        case StepGroupType:
            return QObject::tr("a group of steps");
            break;

        case StepNumberType:
            return QObject::tr("a step number");
            break;

        case PartsListType:
            return QObject::tr("a parts list");
            break;

        case SubModelType:
            return QObject::tr("a submodel");
            break;

        case CalloutType:
            return QObject::tr("a callout");
            break;

        case PageNumberType:
            return QObject::tr("the page number");
            break;

        case PageTitleType:
            return QObject::tr("the document title");
            break;

        case PageModelNameType:
            return QObject::tr("the model id");
            break;

        case PageAuthorType:
            return QObject::tr("the author");
            break;

        case PageURLType:
            return QObject::tr("the publisher url");
            break;

        case PageDocumentLogoType:
            return QObject::tr("the publisher logo");
            break;

        case PageModelDescType:
            return QObject::tr("the model description");
            break;

        case PagePublishDescType:
            return QObject::tr("the publish description");
            break;

        case PageCopyrightType:
            return QObject::tr("the copyright notice");
            break;

        case PageEmailType:
            return QObject::tr("the publisher email");
            break;

        case PageDisclaimerType:
            return QObject::tr("the LEGO © disclaimer");
            break;

        case PagePartsType:
            return QObject::tr("the number of parts");
            break;

        case PagePlugType:
            return QObject::tr("the application plug");
            break;

        case PageCategoryType:
            return QObject::tr("the model category");
            break;

        case PagePlugImageType:
            return QObject::tr("the application logo");
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
    QString preposition;
    const QString type          = type2english(myType);
    const QString relativeTo    = type2english2(placementData.relativeTo);
    const QString placement     = placement2english(placementData.placement);
    const QString justification = placement2english(placementData.justification);

    if (placementData.preposition == Inside) {
        preposition = QObject::tr("inside");
    } else {
        preposition = QObject::tr("outside");
    }

    switch (placementData.placement)
    {
        case TopLeft:
        case TopRight:
        case BottomRight:
        case BottomLeft:
            english = QObject::tr("This %1 is placed in the %2 %3 corner of %4.")
                                  .arg(type, placement, preposition, relativeTo);
            break;
        case Top:
        case Bottom:
        case Left:
        case Right:
            if (placementData.preposition == Inside) {
              english = QObject::tr("this %1 is placed inside %2 against the center of the %3.")
                                    .arg(type, relativeTo, placement);
            } else {
              english = QObject::tr("this %1 is placed outside %2 on the %3, slid to the %4.")
                                    .arg(type, relativeTo, placement, justification);
            }
            break;
        case Center:
            english = QObject::tr("this %1 is placed in the center of %2.")
                                  .arg(type, relativeTo);
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
  const QString       &name)
{
    return englishPlacementWhatsThis(type,placement,name);
}

QString CommonMenus::englishPlacementWhatsThis(
        PlacementType  type,
        PlacementData &placementData,
  const QString       &name)
{
    QString whatsThis;

    whatsThis  = QObject::tr("%1:\n").arg(name);
    whatsThis += QObject::tr("%1 lets you put things next to other things.  In this case,\n %2\n\n"
                 "Clicking \"%3\" pops up a window with a list of things you can put this %4 next to.\n"
                 "It also lets you describe how it is placed next to the thing you choose.\n\n")
                 .arg(VER_PRODUCTNAME_STR, englishPlacement(type,placementData), name.toLower(), type2english(type));

    if (placementData.offsets[XX] != 0 || placementData.offsets[YY] != 0)
    {
        whatsThis += QObject::tr("You've already discovered that you can click and drag\n"
                                 "this %1 around.  Great job, but there's more.\n\n")
                                 .arg(type2english(type));
    } else {
        whatsThis += QObject::tr("Did you know that you can click and drag this %1"
                                 " around using the mouse?\n\n")
                                 .arg(type2english(type));
    }

    return whatsThis;
}
