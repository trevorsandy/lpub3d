/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2024 Trevor SANDY. All rights reserved.
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

#define WT_ARRAY_COUNT(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define WT_ARRAY_SIZE_CHECK(a,s) static_assert(WT_ARRAY_COUNT(a) == static_cast<int>(s), QT_STRINGIFY(a) " size mismatch.")

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

  formatted = QObject::tr("Add Next Step:<br>"
                          "  You can add the first step of the next page to this page.<br>");
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

  formatted = QObject::tr("Add Next Steps:<br>"
                          "  You can add a specified number of next steps to this page.<br>");
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

  formatted = QObject::tr("Add Previous Step:<br>"
                          "  You can add the last step of the previous page to this page.<br>");
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

  formatted = QObject::tr("Remove this Step:<br>"
                          "  You can move this step from this page to the next page.<br>");
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

  formatted = QObject::tr("Add %1:<br>"
                          "  You can add %2 to the page.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("%1 Annotation Options:<br>"
                          "  You can set the %2 annotation option to title,<br>"
                          "  free form or both or none.<br>") .arg(name, name.toLower());
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
  action->setIcon(QIcon(":/resources/reset_viewer_image.png"));

  formatted = QObject::tr("Reset %1 viewer image to its original position") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Reset %1 Viewer Image:<br>"
                          "  You can reset %2 viewer image to its original position.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Copy %1 Image Path To Clipboard:<br>"
                          "  You can copy %2 image path to the clipboard.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Change %1 Font:<br>"
                          "  You can change the font or edit the size for this %2.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Change %1 Shape:<br>"
                          "  You can change the shape of this parts list.  One way, is<br>"
                          "  is to ask the computer to make the parts list as small as<br>"
                          "  possible (area). Another way is to ask the computer to<br>"
                          "  make it as close to square as possible.  You can also pick<br>"
                          "  how wide you want it, and the computer will make it as<br>"
                          "  tall as is needed.  Another way is to pick how tall you<br>"
                          "  and it, and the computer will make it as wide as it needs.<br>"
                          "  The last way is to tell the computer how many columns it<br>"
                          "  can have, and then it will try to make all the columns the<br>"
                          "  same height<br>") .arg(name);
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

  formatted = QObject::tr("Edit %1:<br>"
                          "  You can edit %2 text with this dialog.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Change %1 Color:<br>"
                          "  You can change the color of this %2.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Change %1 Image:<br>"
                          "  You can change the %2 image.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Stretch %1 Image:<br>"
                          "  You can stretch the %2 image to the size of the page.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Tile %1 Image:<br>"
                          "  You can tile the %2 image across the page.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Change %1 Margins:<br>"
                          "  You can change how much empty space there is<br>"
                          "  around this %2.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Delete %1:<br>"
                          "  You can remove this %2 from the %3.<br>") .arg(name, name.toLower(), item);
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

  formatted = QObject::tr("Hide %1:<br>"
                          "  You can turn off and on %2 display.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Display %1 as Rows:<br>"
                          "  Change this whole set of %1 from columns of %1<br>"
                          "  to rows of %2.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Display %1 as Columns:<br>"
                          "  Change this whole set of %1 from rows of %1<br>"
                          "  to columns of %2.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Parts List Per %1:<br>"
                          "  You can choose to show the parts list per %2.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("No Parts List Per %1:<br>"
                          "  You can choose to not show parts list per %2.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Change %1 Placement<br>"
                          "  You can change the initial %2 placement relative to<br>"
                          "  options provided in the placement dialog.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Change %1 Size:<br>"
                          "  You can change the size of this %2 using the<br>"
                          "  size dialog.<br>").arg(name, name.toLower());
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

  formatted = QObject::tr("Change %1 Submodel Color:<br>"
                          "  You can change the %2 Submodel level background color.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Change %1 Camera Distance:<br>"
                          "  You can change the %2 native camera distance factor.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Change %1 Rotation:<br>"
                          "  You can change the %2 Rotation angles and Transform.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Change %1 Background:<br>"
                          "  You can change the color or use a picture for<br>"
                          "  the %2 background.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Change %1 Border:<br>"
                          "  You can set a square corner border, round corner border<br>"
                          "  or no border at all for this %2.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Remove %1:<br>"
                          "  You can remove %2 from the page.<br>") .arg(name, name.toLower());
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

  formatted = whatsThis.isEmpty() ? QObject::tr("Change %1 Scale:<br>"
                                                "  You can change the size of this %2 using the scale dialog.<br>"
                                                "  A scale of 1.0 is true size. A scale of 2.0 doubles<br>"
                                                "  the size of your model.<br>"
                                                "  A scale of 0.5 makes your model half real size<br>").arg(name, name.toLower()) : whatsThis;
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

  formatted = QObject::tr("Change %1 Camera FOV Angle:<br>"
                          "  You can change the camera field of view angle<br>"
                          "  in degrees for this %2.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Change %1 Camera Angles:<br>"
                          "  You can change the camera longitude and latitude<br>"
                          "  view angles for this %2.<br>") .arg(name, name.toLower());
  action->setWhatsThis(formatted);

  return action;
}

QAction* CommonMenus::refreshImageCacheMenu(
        QMenu   &menu,
  const QString &name,
  const QString &objectName,
  const QString &iconImage,
  const PlacementType type)
{
  QAction *action;

  const QString imagelabel = type != CsiType && type != SubModelType ? QObject::tr("Images") : QObject::tr("Image");

  QString formatted = QObject::tr("Refresh %1 %2") .arg(name,imagelabel);
  action = menu.addAction(formatted);
  action->setParent(menu.parent());
  action->setObjectName(objectName.isEmpty() ? "clearImageCacheAction.1" : objectName);
  action->setProperty("name", name);
  action->setIcon(QIcon(iconImage.isEmpty() ? ":/resources/refreshpagecache.png" : ":/resources/" + iconImage));

  formatted = QObject::tr("Clear the %1 %2 and working LDraw (.ldr) cache files") .arg(name.toLower(),imagelabel.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Refresh %1 %2:<br>"
                          "  You can clear this %3 %4 and working LDraw (.ldr)<br>"
                          "  cache files.<br>") .arg(name, imagelabel, name.toLower(), imagelabel.toLower());
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

  formatted = QObject::tr("Sort %1:<br>"
                          "  You can sort the %2 by part color, part category<br>"
                          "  or image size.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Turn On Movable %1 Group:<br>"
                          "  You can set %2 groups, part mage, annotaiton and<br>"
                          "  instance count, movable.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Turn Off Movable %1 Group:<br>"
                          "  You can set %2 groups, image, annotaiton and instance count,<br>"
                          "  non-movable.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Substitute %1:<br>"
                          "  You can replace this %2 with a substitute.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Override %1:<br>"
                          "  You can replace this %2 instance count with a<br>"
                          "  manually entered value.<br>") .arg(name, name.toLower());
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
  action->setIcon(QIcon(":/resources/refreshsubmodelcache.png"));

  formatted = QObject::tr("Restore the default %1 instance count.") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Restore %1:<br>"
                          "  You can restore the default %2 instance count.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Change Substitute %1:<br>"
                          "  You can change this substitute %2 with another part.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Remove Substitute %1:<br>"
                          "  You can remove this substitute %2 with the<br>"
                          "  original part.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Reset this %1 group - remove LPUB meta command") .arg(name.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Reset %1 Group:<br>"
                          "  You can reset this %2 group - remove LPUB meta command.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Preview %1:<br>"
                          "  You can display the %2 in a popup viewer.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Hide %1:<br>"
                          "  You can hide %2 in the page display.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Hide %1:<br>"
                          "  You can hide %2 in the Parts List.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Change %1 Renderer:<br>"
                          "  You can select the %2 image renderer and settings<br>"
                          "  from available renderers.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Fade Previous %1 Steps:<br>"
                          "  You can turn on or off fade previous steps for<br>"
                          "  this %2 image.<br>") .arg(name, name.toLower());
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

  formatted = QObject::tr("Highlight Current %1 Step:<br>"
                          "  You can turn on or off highlight current step for<br>"
                          "  this %2 image.<br>") .arg(name, name.toLower());
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
  action->setObjectName(objectName.isEmpty() ? "rendererArgumentsAction.1" : objectName);
  action->setProperty("name", name);
  action->setIcon(QIcon(":/resources/rendererarguments.png"));

  formatted = QObject::tr("Add %1 arguments") .arg(renderer.toLower());
  action->setStatusTip(formatted);

  formatted = QObject::tr("Add %1 Arguments:<br>"
                          "  You can add %2 arguments for this %3.<br>") .arg(name, renderer, name.toLower());
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

        case TextType:
            return QObject::tr("plain or rich text");
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

    case TextType:
            return QObject::tr("plain or rich text");
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

    whatsThis  = QObject::tr("%1:<br>").arg(name);
    whatsThis += QObject::tr("%1 lets you put things next to other things.  In this case,<br> %2<br><br>"
                 "Clicking \"%3\" pops up a window with a list of things you can put this %4 next to.<br>"
                 "It also lets you describe how it is placed next to the thing you choose.<br><br>")
                 .arg(VER_PRODUCTNAME_STR, englishPlacement(type,placementData), name.toLower(), type2english(type));

    if (placementData.offsets[XX] != 0 || placementData.offsets[YY] != 0)
    {
        whatsThis += QObject::tr("You've already discovered that you can click and drag<br>"
                                 "this %1 around.  Great job, but there's more.<br><br>")
                                 .arg(type2english(type));
    } else {
        whatsThis += QObject::tr("Did you know that you can click and drag this %1"
                                 " around using the mouse?<br><br>")
                                 .arg(type2english(type));
    }

    return whatsThis;
}

/****************************
 * What's This Text
 ****************************/
CommonMenus::WT_Text CommonMenus::WT[WT_NUM_ENTRIES];

void CommonMenus::setWhatsThis()
{
    WT_Text WTData[] =
    {
        //************************************
        //*  PREFERENCES ENTRIES
        //************************************
        // WT_LPUB3D_PREFERENCES
        {
            QObject::tr(
            "  Configure and edit your %1<br>"
            "  application settings and preferences.<br>"
            "  Settings configured here are global - this is<br>"
            "  to say they apply to any open model instruction<br>"
            "  document.<br><br>"
            "  To specify settings for a specific<br>"
            "  instruction document, use the global settings<br>"
            "  at Instruction, Build Instruction Setup.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LPUB3D_PREFERENCES")
#else
                 ""
#endif
                )
        },
        // WT_LPUB3D_PREFERENCES_DARK_THEME_COLORS
        {
            QObject::tr(
            "  Configure the %1 dark theme colors using the<br>"
            "  color picker dialog.<br><br>"
            "  Use the reset button accompanying each color<br>"
            "  setting to restore the default color properties.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LPUB3D_PREFERENCES_DARK_THEME_COLORS")
#else
                 ""
#endif
                )
        },
        // WT_LPUB3D_PREFERENCES_DEFAULT_THEME_COLORS
        {
            QObject::tr(
            "  Configure the %1 default theme colors using the<br>"
            "  color picker dialog.<br><br>"
            "  Use the reset button accompanying each color<br>"
            "  setting to restore the default color properties.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LPUB3D_PREFERENCES_DEFAULT_THEME_COLORS")
#else
                 ""
#endif
                )
        },
        // WT_LPUB3D_PREFERENCES_GENERAL
        {
            QObject::tr(
            "  Configure %1 LDraw library path, alternative LDConfig<br>"
            "  model file load, model step fade and highlight options,<br>"
            "  part list instance (PLI) setings, and default options<br>"
            "  for units, camara angles and dialog messages.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LPUB3D_PREFERENCES_GENERAL")
#else
                 ""
#endif
                )
        },
        // WT_LPUB3D_PREFERENCES_KEYBOARD_SHORTCUTS
        {
            QObject::tr(
            "  Configure and edit %1 keyboard shortcuts.<br>"
            "  All editable keyboard shortcuts are presented<br>"
            "  in the view table grouped by their parent dialog<br>"
            "  menu and action category.<br><br>"
            "  You can also export, import and reset your keyboard<br>"
            "  shortcuts from this dialog.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LPUB3D_PREFERENCES_KEYBOARD_SHORTCUTS")
#else
                 ""
#endif
                )
        },
        // WT_LPUB3D_PREFERENCES_LDGLITE
        {
            QObject::tr(
            "  View %1 LDGLite renderer setting.<br>"
            "  By default LDGLite and automatically configured<br>"
            "  at application startup using the %1 default<br>"
            "  options.<br><br>"
            "  When LDGLite is installed, the application path<br>"
            "  is provided.<br><br>"
            "  You can customize some LDGLite renderer options<br>"
            "  from its INI configuration file using the application<br>"
            "  menu action at Configuration, Edit Parameter Files...<br><br>"
            "  Note that some settings will be overridden by LPUB<br>"
            "  meta commands in your loaded model file.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LPUB3D_PREFERENCES_LDGLITE")
#else
                 ""
#endif
                )
        },
        // WT_LPUB3D_PREFERENCES_LDVIEW
        {
            QObject::tr(
            "  Configure %1 LDView renderer settings.<br>"
            "  By default LDView and automatically configured<br>"
            "  at application startup using the %1 default<br>"
            "  options.<br><br>"
            "  When LDView is installed, the application path<br>"
            "  is provided.<br><br>"
            "  You can customize additional LDView renderer options<br>"
            "  from its INI configuration file using the application<br>"
            "  menu action at Configuration, Edit Parameter Files...<br><br>"
            "  Note that some settings will be overridden by LPUB<br>"
            "  meta commands in your loaded model file.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LPUB3D_PREFERENCES_LDVIEW")
#else
                 ""
#endif
                )
        },
        // WT_LPUB3D_PREFERENCES_MESSAGES
        {
            QObject::tr(
            "  Configure and edit %1 message categories.<br>"
            "  Messages are generaged at various pints of<br>"
            "  parsing the LDraw model file as the editor<br>"
            "  transitions from page to page.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LPUB3D_PREFERENCES_MESSAGES")
#else
                 ""
#endif
                )
        },
        // WT_LPUB3D_PREFERENCES_NATIVE
        {
            QObject::tr(
            "  Configure %1 Native renderer rendering and default<br>"
            "  camera settings.<br><br>"
            "  The %1 Native renderer is a customized instance<br>"
            "  of the popular LDraw model editor LeoCAD that has<br>"
            "  been integrated into %1; hence, the similarity of<br>"
            "  this preferences dialog.<br><br>"
            "  The %1 3DViewer is a customized and repackaged<br>"
            "  instance LeoCAD so renderer settings set here can<br>"
            "  have an effect on the 3DViewer display also.<br><br>"
            "  You can configure additional Native renderer settings<br>"
            "  from the 3DViewer application menu action Preferences.<br><br>"
            "  Note that some settings will be overridden by LPUB<br>"
            "  meta commands in your loaded model file.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LPUB3D_PREFERENCES_NATIVE")
#else
                 ""
#endif
                )
        },
        // WT_LPUB3D_PREFERENCES_OTHER
        {
            QObject::tr(
            "  Configure %1 application update settings and<br>"
            "  view the current list of LDraw part and submodel<br>"
            "  search directories.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LPUB3D_PREFERENCES_OTHER")
#else
                 ""
#endif
                )
        },
        // WT_LPUB3D_PREFERENCES_POVRAY
        {
            QObject::tr(
            "  Configure %1 POV-Ray renderer settings.<br>"
            "  By default POV-Ray and automatically configured<br>"
            "  at application startup using the %1 default<br>"
            "  options.<br><br>"
            "  POV-Ray Quality options High, Medium and Low allows<br>"
            "  you to select the level of tracing intensity to apply.<br>"
            "  You can toggle on or off automatically cropping rendered<br>"
            "  images<br>"
            "  It is also possible to display a progress dialog that<br>"
            "  show the "
            "  When POV-Ray is installed, the application path<br>"
            "  is provided.<br><br>"
            "  Additionally, when the LGEO library is available, the<br>"
            "  library path is provided.<br><br>"
            "  You can customize additional POV-Ray renderer options<br>"
            "  from its INI configuration file using the application<br>"
            "  menu action at Configuration, Edit Parameter Files...<br><br>"
            "  Note that some settings will be overridden by LPUB<br>"
            "  meta commands in your loaded model file.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LPUB3D_PREFERENCES_POVRAY")
#else
                 ""
#endif
                )
        },
        // WT_LPUB3D_PREFERENCES_PUBLISHING
        {
            QObject::tr(
            "  Configure and edit %1 instruction document settings.<br>"
            "  Set continuous page settings, page attribute display,<br>"
            "  and page attribute values.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LPUB3D_PREFERENCES_PUBLISHING")
#else
                 ""
#endif
                )
        },
        // WT_LPUB3D_PREFERENCES_RENDERERS
        {
            QObject::tr(
            "  Configure and edit %1 built-in renderer settings.<br>"
            "  - Native: %1 'native' renderer and 3D viewer adapted<br>"
            "    from LeoCAD. The Native renderer is performant and<br>"
            "    provides 'true fade' rendering which enable powerful,<br>"
            "    high quality and complex model rendering options.<br><br>"
            "  - LDView: This is a custom instance of LDView specially<br>"
            "    adapted for %1 using the latest code base.<br>"
            "    LDView offers high quality, rendering plus POV scene<br>"
            "    file generation.<br><br>"
            "  - POV-Ray: %1 ray tracing renderer offering feature<br>"
            "    rich, photo-realistic rendering ideal for document<br>"
            "    covers and final model or assembly displays.<br><br>"
            "  - LDGLite: %1 legacy renderer written in 'c' offering<br>"
            "    fast image rendering ideal for proofing.<br><br>"
            "  - Blender: %1 addon module to enable LDraw import,<br>"
            "    export and image rendering using Blender 2.80 or<br>"
            "    later.<br>"
            "    The Blender LDraw addon is not configured by default.<br>"
            "    To enable, you must install Blender and 2.80 or later<br>"
            "    and set the Blender path to trigger the %1 automated<br>"
            "    addon configuration.<br>"
            "    Once configured, you can use this dialog to cutomize<br>"
            "    your Blender LDraw import, and image render settings.<br><br>"
            "  The Renderers tab also present the preferred renderer,<br>"
            "  projection and render process timeout settings.<br>"
            "  Additonally, LSynth options for configurable parts are<br>"
            "  configurable from this location.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LPUB3D_PREFERENCES_RENDERERS")
#else
                 ""
#endif
                )
        },
        // WT_LPUB3D_PREFERENCES_SEARCH_DIRECTORIES
        {
            QObject::tr(
            "  View current %1 LDraw content search directories.<br>"
            "  This dialog is read only. You can edit the search<br>"
            "  directories from the Configuration menu using<br>"
            "  the LDraw Search Directories action.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LPUB3D_PREFERENCES_SEARCH_DIRECTORIES")
#else
                 ""
#endif
                )
        },
        // WT_LPUB3D_PREFERENCES_THEME_AND_LOGGING
        {
            QObject::tr(
            "  Configure and edit %1 theme and logging settins.<br><br>"
            "  - Themes: Set the preferred theme and edit the<br>"
            "    application theme and view scene (the view that<br>"
            "    displays the generated instruction document page)<br>"
            "    colors.<br><br>"
            "  - Logging: Select the level of logging, the displayed<br>"
            "    log attributes and the individual logging level as<br>"
            "    desired.<br><br>"
            "  Logging can be helpful in troubleshooting unexptected<br>"
            "  application behaviour.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LPUB3D_PREFERENCES_THEME_AND_LOGGING")
#else
                 ""
#endif
                )
        },
        // WT_LPUB3D_PREFERENCES_THEME_COLORS
        {
            QObject::tr(
            "  Configure and reset %1 dark and default<br>"
            "  theme colors.<br><br>"
            "  Use the Default button to display the<br>"
            "  hexadecimal color code for each default<br>"
            "  color.<br><br>"
            "  Use the Reset button to reset all the theme<br>"
            "  colors to their default setting.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LPUB3D_PREFERENCES_THEME_COLORS")
#else
                 ""
#endif
                )
        },
        // WT_LPUB3D_PREFERENCES_UPDATES
        {
            QObject::tr(
            "  Configure and edit %1 application update settings.<br><br>"
            "  Set check for update and application change log<br>"
            "  preferences<br><br>"
            "  View %1 release change details.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LPUB3D_PREFERENCES_UPDATES")
#else
                 ""
#endif
                )
        },
        // WT_VISUAL_PREFERENCES
        {
            QObject::tr(
            "  Configure and edit the 3D viewer preferences<br>"
            "  and settings.<br><br>"
            "  The %1 3DViewer is a customized and repackaged<br>"
            "  instance of LeoCAD also used as the %1 Native<br>"
            "  renderer so settings configured here can have an<br>"
            "  effect on rendered images when the preferred renderer.<br>"
            "  is set to Native.<br>"
            "  Note that some settings will be overridden by LPUB<br>"
            "  meta commands in your loaded model file.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_VISUAL_PREFERENCES")
#else
                 ""
#endif
                )
        },
        // WT_VISUAL_PREFERENCES_CATEGORIES
        {
            QObject::tr(
            "  Configure and edit 3D viewer part categories used"
            "  in the parts lookup dialog.<br><br>"
            "  You can also export, import and reset part categories<br>"
            "  from this dialog.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_VISUAL_PREFERENCES_CATEGORIES")
#else
                 ""
#endif
                )
        },
        // WT_VISUAL_PREFERENCES_COLORS
        {
            QObject::tr(
            "  Configure and reset the 3D viewer default<br>"
            "  backgroud, displayed objects, interface, base<br>"
            "  grid and view sphere colors.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_VISUAL_PREFERENCES_COLORS")
#else
                 ""
#endif
                )
        },
        // WT_VISUAL_PREFERENCES_GENERAL
        {
            QObject::tr(
            "  Configure the 3D viewer mouse sensitivity, timeline,<br>"
            "  icons, page redraw prompt and official parts settings.<br><br>"
            "  Additionally, you can view paths, theme and author<br>"
            "  settings shared with %1.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_VISUAL_PREFERENCES_GENERAL")
#else
                 ""
#endif
                )
        },
        // WT_VISUAL_PREFERENCES_KEYBOARD_SHORTCUTS
        {
            QObject::tr(
            "  Configure keyboard shortcuts.<br>"
            "  This tab should not be visible.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_VISUAL_PREFERENCES_KEYBOARD_SHORTCUTS")
#else
                 ""
#endif
                )
        },
        // WT_VISUAL_PREFERENCES_MOUSE
        {
            QObject::tr(
            "  Configure mouse shortcuts.<br>"
            "  This tab should not be visible.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_VISUAL_PREFERENCES_MOUSE")
#else
                 ""
#endif
                )
        },
        // WT_VISUAL_PREFERENCES_RENDERING
        {
            QObject::tr(
            "  Configure the 3D viewer rendering settings.<br><br>"
            "  Renderer settings affect both the 3D viewer and<br>"
            "  the %1 Native renderer.<br><br>"
            "  Note that some settings will be overridden by LPUB<br>"
            "  meta commands in your loaded model file.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_VISUAL_PREFERENCES_RENDERING")
#else
                 ""
#endif
                )
        },
        // WT_VISUAL_PREFERENCES_USER_INTERFACE
        {
            QObject::tr(
            "  Configure the 3D viewer user interface, base grid<br>"
            "  model view and part preview settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_VISUAL_PREFERENCES_USER_INTERFACE")
#else
                 ""
#endif
                )
        },
        // WT_VISUAL_PREFERENCES_VIEWS
        {
            QObject::tr(
            "  Configure the 3D viewer default user interface and<br>"
            "  color settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_VISUAL_PREFERENCES_VIEWS")
#else
                 ""
#endif
                )
        },
        // WT_LDVIEW_PREFERENCES
        {
            QObject::tr(
            "  Configure or reset the LDView renderer default<br>"
            "  settings.<br><br>"
            "  Settings are grouped into five dialogs, general,<br>"
            "  gemometry, effects, primitives, updates and<br>"
            "  preference set.<br><br>"
            "  - Reset: This resets all the General preferences to<br>"
            "    their default values. As with all other preference<br>"
            "    changes, you must hit OK or Apply for the reset<br>"
            "    values to go into effect.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LDVIEW_PREFERENCES")
#else
                 ""
#endif
                )
        },
        // WT_LDVIEW_PREFERENCES_GENERAL
        {
            QObject::tr(
            "  Configure or reset the LDView antialiasing, color,<br>"
            "  miscellaneous options,default save directories, and<br>"
            "  custom LDConfig file.<br><br>"
            "  As the LDView renderer is used in multiple functions,<br>"
            "  - i.e. step renderer, html parts list generator and<br>"
            "  POV file generator -, there are miltiple configuration<br>"
            "  INI files, each defined to configure LDView for the<br>"
            "  function specified<br> The current INI file is displayed<br>"
            "  at the bottom of this dialog.<br><br>"
            "  - Reset: This resets all the General preferences to<br>"
            "    their default values. As with all other preference<br>"
            "    changes, you must hit OK or Apply for the reset<br>"
            "    values to go into effect.<br><br>"
            "  Note it is possible to configure settings that violate<br>"
            "  the parameters needed to successfully render %1<br>"
            "  images. Use caution when setting unfamiliar preferences.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LDVIEW_PREFERENCES_GENERAL")
#else
                 ""
#endif
                )
        },
        // WT_LDVIEW_PREFERENCES_LDRAW
        {
            QObject::tr(
            "  Configure the LDView LDraw archive parts library,<br>"
            "  parts library directory and extra search directories.<br><br>"
            "  - LDraw Zip: This allows you to manually select a zip<br>"
            "    file containing the LDraw library to be used in<br>"
            "    preference to the LDraw Folder below. Each parts<br>"
            "    release from ldraw.org contains a file named complete.zip<br>"
            "    that you can use here. Additionally, if a file named<br>"
            "    lpub3dldrawunf.zip or ldrawunf.zip exists alongside this<br>"
            "    zip file, it will automatically be scanned for unofficial<br>"
            "    parts.<br><br>"
            "    You can enter a path, or hit the Browse... button.<br>"
            "    If you enter a path that doesn't resolve to a zip file,<br>"
            "    you will get an error when you apply your preferences<br>"
            "    changes.<br><br>"
            "  - LDraw Folder: Allows you to manually select the LDraw<br>"
            "    folder. You can enter a path, or hit the Browse... button.<br>"
            "    If you type in a path that isn't an LDraw folder, you will<br>"
            "    get an error when you apply your preferences changes.<br><br>"
            "    Note that if you specify an LDraw Zip, the LDraw Folder is<br>"
            "    no longer required to have parts and p subdirectories.<br>"
            "    It still must be valid, though, since that is where<br>"
            "    automatically downloaded unofficial parts will be placed.<br><br>"
            "  - Extra Search Folders/Directories: Allows you to specify<br>"
            "    and organize any number of extra search directories. When<br>"
            "    LDView loads a model, and the model references a part or<br>"
            "    sub-model, it searches for that part or sub-model in a<br>"
            "    number of locations. No matter what you enter here, it<br>"
            "    will search the directory the model is located in, as well<br>"
            "    as the P, PARTS, and MODELS directories inside the LDraw<br>"
            "    directory (in that order). If it doesn't find the file in<br>"
            "    any of those directories, it will then search all the<br>"
            "    directories listed in the Extra Search Dirs dialog accessed<br>"
            "    via this menu item. It searches them in the order they are<br>"
            "    listed in the box, so you can move directories in the box<br>"
            "    up and down to get them in the order you want them to<br>"
            "    be in.<br><br>"
            "  - Extra Search Folders/Directories: Allows you to specify<br>"
            "    and organize any number of extra search directories.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LDVIEW_PREFERENCES_LDRAW")
#else
                 ""
#endif
                )
        },
        // WT_LDVIEW_PREFERENCES_GEOMETRY
        {
            QObject::tr(
            "  Configure or reset the LDView model, wireframe, edge<br>"
            "  lines and back face culling (BFC) settings.<br><br>"
            "  - Reset: This resets all the General preferences to<br>"
            "    their default values. As with all other preference<br>"
            "    changes, you must hit OK or Apply for the reset<br>"
            "    values to go into effect.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LDVIEW_PREFERENCES_GEOMETRY")
#else
                 ""
#endif
                )
        },
        // WT_LDVIEW_PREFERENCES_EFFECTS
        {
            QObject::tr(
            "  Configure or reset the LDView lighting, stereo,<br>"
            "  cutaway, transparency, shading and curve settings.<br><br>"
            "  - Reset: This resets all the General preferences to<br>"
            "    their default values. As with all other preference<br>"
            "    changes, you must hit OK or Apply for the reset<br>"
            "    values to go into effect.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LDVIEW_PREFERENCES_EFFECTS")
#else
                 ""
#endif
                )
        },
        // WT_LDVIEW_PREFERENCES_PRIMITIVES
        {
            QObject::tr(
            "  Configure or reset the LDView primitive substitution<br>"
            "  textures, stud style, edge color, stud quality and<br>"
            "  high-resulution primitives settings.<br><br>"
            "  - Reset: This resets all the General preferences to<br>"
            "    their default values. As with all other preference<br>"
            "    changes, you must hit OK or Apply for the reset<br>"
            "    values to go into effect.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LDVIEW_PREFERENCES_PRIMITIVES")
#else
                 ""
#endif
                )
        },
        // WT_LDVIEW_PREFERENCES_UPDATES
        {
            QObject::tr(
            "  Configure or reset the LDView internet proxy, missing parts<br>"
            "  download settings and check for LDraw library updates.<br><br>"
            "  - Internet Proxy: Configure your connection to the Internet<br>"
            "    with options to use a proxy server.<br><br>."
            "  - Missing Parts: Check LDraw.org for missing parts.<br><br>"
            "  - Check for Library Updates: Checks to see if there are any<br>"
            "    LDraw parts library updates on LDraw.org that aren't yet<br>"
            "    installed, and downloads and installs them if any are found.<br><br>"
            "  - Reset: This resets all the General preferences to<br>"
            "    their default values. As with all other preference<br>"
            "    changes, you must hit OK or Apply for the reset<br>"
            "    values to go into effect.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LDVIEW_PREFERENCES_UPDATES")
#else
                 ""
#endif
                )
        },
        // WT_LDVIEW_PREFERENCES_PREFERENCES_SET
        {
            QObject::tr(
            "  View the LDView current preference set.<br><br>"
            "  Note that %1 automatically loads the preference set<br>"
            "  configured for the specified function. For example,<br>"
            "  only the Default preference set is loaded when<br>"
            "  LDView is used as the preferred renderer while all<br>"
            "  the pre-configured preference sets are loaded when<br>"
            "  accessing the LDView preferences while POV-Ray is<br>"
            "  speified as the preferred renderer.<br><br>"
            "  Use caution when creating new preference sets which<br>"
            "  is to say you should possess sufficient understanding<br>"
            "  of this functionality and how it is used by %1 before<br>"
            "  changing this configuration.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_LDVIEW_PREFERENCES_PREFERENCES_SET")
#else
                 ""
#endif
                )
        },
        //*************************************
        //*  GLOBAL SETUP ENTRIES
        //*************************************
        // WT_SETUP_ASSEM
        {
            QObject::tr(
            "  Configure and edit your instructions document<br>"
            "  assembly / Current Step Instance (CSI)<br>"
            "  settings.<br><br>"
            "  You can configure your assembly content<br>"
            "  and display.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_ASSEM")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_ASSEM_ANNOTATION
        {
            QObject::tr(
            "  The following configuration setup are available for<br>"
            "  assembly part annotations:<br>"
            "  - Display Assembly (CSI) Part Annotation: turn on or<br>"
            "    off CSI part annotation.<br><br>"
            "  - CSI Part Annotation Placement: change CSI part<br>"
            "    annotation placement.<br>"
            "    You can also drag part annotations from their<br>"
            "    placed location to your desired location.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_ASSEM_ANNOTATION")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_ASSEM_ASSEMBLIES
        {
            QObject::tr(
            "  Configure your instruction document assembly<br>"
            "  images default scale, margins, camera and<br>"
            "  step number settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_ASSEM_ASSEMBLIES")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_ASSEM_CONTENTS
        {
            QObject::tr(
            "  Configure your instruction document assembly sizing,<br>"
            "  image camera orientation and step number settings.<br><br>"
            "  You can also configure the %1 renderer<br>"
            "  additional parameters and environment variables<br>"
            "  and, only for assemblies, additional parameters and<br>"
            "  environment variables for Blender photo-realistic<br>"
            "  image rendering.%2<br>")
            .arg(MetaDefaults::getPreferredRenderer())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_ASSEM_CONTENTS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_ASSEM_DISPLAY
        {
            QObject::tr(
            "  Configure your instruction document assembly<br>"
            "  step number display, part annotation display,<br>"
            "  and stud style and automate edge color settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_ASSEM_DISPLAY")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_ASSEM_DISPLAY_STEP
        {
            QObject::tr(
            "  The following configuration setup is available for<br>"
            "  assembly step:<br>"
            "  - Step Number: turn or off the step number display.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_ASSEM_DISPLAY_STEP")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_ASSEM_MORE_OPTIONS
        {
            QObject::tr(
            "  Configure your instruction document<br>"
            "  %1 default additional arguments.%2<br>")
            .arg(MetaDefaults::getPreferredRenderer())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_ASSEM_MORE_OPTIONS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_CALLOUT
        {
            QObject::tr(
            "  Configure and edit the callout settings<br>"
            "  for your instructions document.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_CALLOUT")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_CALLOUT_ASSEMBLY
        {
            QObject::tr(
            "  Configure your instruction document callout<br>"
            "  assembly image, assembly orientation, assembly<br>"
            "  margins and assembly stud style and automated<br>"
            "  edge colors settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_CALLOUT_ASSEMBLY")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_CALLOUT_BACKGROUND_BORDER
        {
            QObject::tr(
            "  Configure your instruction document callout<br>"
            "  background, border, margins, and divider<br>"
            "  settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_CALLOUT_BACKGROUND_BORDER")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_CALLOUT_CALLOUT
        {
            QObject::tr(
            "  Configure your instruction document callout<br>"
            "  parts list, step justification, submodel preview,<br>"
            "  step number and times used settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_CALLOUT_CALLOUT")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_CALLOUT_CALLOUT_PARTS_LIST
        {
            QObject::tr(
            "  The following configuration setup is available for<br>"
            "  callout parts list:<br>"
            "  - Per Step: set the parts list instance (PLI) per step<br>"
            "    display. When unchecked, the PLI is set to per<br>"
            "    callout.<br><br>"
            "  - Margins: set the parts list instance (PLI) margins<br>"
            "    L/R(Left/Right) width and T/B(Top/Bottom) height.<br>"
            "    Enter margin values using a decimal number<br>"
            "    in %1.<br>"
            "    Dot units are defined in Project Global Setup.%2<br>")
            .arg(MetaDefaults::getPreferredUnits())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_CALLOUT_CALLOUT_PARTS_LIST")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_CALLOUT_CALLOUT_SUBMODEL_PREVIEW
        {
            QObject::tr(
            "  The following configuration setup is available for<br>"
            "  callout submodel preview:<br>"
            "  - Show Submodel Preview At First Step: turn on or off<br>"
            "    displaying the called out submodel at the first<br>"
            "    step in the callout. This option is only available<br>"
            "    for unassembled callouts.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_CALLOUT_CALLOUT_SUBMODEL_PREVIEW")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_CALLOUT_CALLOUT_POINTERS
        {
            QObject::tr(
            "  Configure your instruction document callout<br>"
            "  pointers:<br>"
            "  ° Configure the pointer border.<br>"
            "  ° Configure the pointer line.<br>"
            "  ° Configure the pointer tip.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_CALLOUT_CALLOUT_POINTERS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_CALLOUT_CONTENTS
        {
            QObject::tr(
            "  The following configuration setup are available in<br>"
            "  the callout Content dialog:<br>"
            "  - Configure the callout properties.<br><br>"
            "  - Configure the callout assembly properties.<br>"
            "  - Configure the callout pointers.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_CALLOUT_CONTENTS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_CALLOUT_DIVIDER_POINTERS
        {
            QObject::tr(
            "  Configure your instruction document callout<br>"
            "  divider pointers:<br>"
            "  ° Configure the pointer border.<br>"
            "  ° Configure the pointer line.<br>"
            "  ° Configure the pointer tip.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_CALLOUT_DIVIDER_POINTERS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_CALLOUT_POINTERS
        {
            QObject::tr(
            "  The following configuration setup are available in<br>"
            "  the callout Pointers dialog:<br>"
            "  ° Configure the Callout Pointer properties.<br>"
            "  ° Configure the Divider Pointer properties.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_CALLOUT_POINTERS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_FADE_STEPS
        {
            QObject::tr(
            "  Configure and edit your instructions document<br>"
            "  fade previous steps or step parts settings.<br><br>"
            "  You can configure your fade previous steps or step<br>"
            "  parts display, setup and final model step settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_FADE_STEPS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_HIGHLIGHT_STEP
        {
            QObject::tr(
            "  Configure and edit your instructions document<br>"
            "  highlight current step or step parts settings.<br><br>"
            "  You can configure your highlight current step<br>"
            "  or step parts display, setup and final model<br>"
            "  step settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_HIGHLIGHT_STEP")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_MULTI_STEP
        {
            QObject::tr(
            "  Configure and edit the step group (multi step page)<br>"
            "  settings for your instructions document.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_MULTI_STEP")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_MULTI_STEP_ADJUST_STEP
        {
            QObject::tr(
            "  The following configuration setup is available for<br>"
            "  step group callout and rotate icon:<br>"
            "  - Adjust Step Row Or Column: turn on or off adjust<br>"
            "    step row or column when callout or rotate icon<br>"
            "    is dragged.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_MULTI_STEP_ADJUST_STEP")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_MULTI_STEP_ASSEMBLY
        {
            QObject::tr(
            "  Configure your instruction document step group<br>"
            "  assembly image, assembly orientation, assembly<br>"
            "  margins and assembly stud style and automated<br>"
            "  edge colors settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_MULTI_STEP_ASSEMBLY")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_MULTI_STEP_CONTENTS
        {
            QObject::tr(
            "  Configure your instruction document step group<br>"
            "  margins, step justification, callout and rotate<br>"
            "  icon adjustment and step number settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_MULTI_STEP_CONTENTS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_MULTI_STEP_DIVIDER
        {
            QObject::tr(
            "  Configure your instruction document step group<br>"
            "  divider, and divider pointer line and tip<br>"
            "  settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_MULTI_STEP_DIVIDER")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_MULTI_STEP_DISPLAY
        {
            QObject::tr(
            "  Configure your instruction document step group<br>"
            "  parts list and submodel preview settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_MULTI_STEP_DISPLAY")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_MULTI_STEP_DISPLAY_PARTS_LIST
        {
            QObject::tr(
            "  The following configuration setup is available for<br>"
            "  step group parts list:<br>"
            "  - Per Step: set the parts list instance (PLI) per step<br>"
            "    display. When unchecked, the PLI is set to per<br>"
            "    page.<br><br>"
            "  - Show Group Step Number: turn on show group step<br>"
            "    number when parts list instance per page is enabled.<br>"
            "    This setting enables to PLI to be placed relative to<br>"
            "    the step number.<br><br>"
            "  - Count Group Steps: turn on count step group as a<br>"
            "    single step when parts list instance per page is<br>"
            "    enabled.<br><br>"
            "  - Margins: set the parts list instance (PLI) margins<br>"
            "    L/R(Left/Right) width and T/B(Top/Bottom) height.<br>"
            "    Enter margin values using a decimal number<br>"
            "    in %1.<br>"
            "    Dot units are defined in Project Global Setup.%2<br>")
            .arg(MetaDefaults::getPreferredUnits())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_MULTI_STEP_DISPLAY_PARTS_LIST")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_MULTI_STEP_DISPLAY_SUBMODEL_PREVIEW
        {
            QObject::tr(
            "  The following configuration setup is available for<br>"
            "  step group submodel preview:<br>"
            "  - Show Submodel Preview At First Step: turn on or off<br>"
            "    displaying the current submodel at the first<br>"
            "    step in the step group.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_MULTI_STEP_DISPLAY_SUBMODEL_PREVIEW")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_MULTI_STEP_JUSTIFY_Y_AXIS_OUTSIDE_PLACEMENT
        {
            QObject::tr(
            "  The following configuration setup is available for<br>"
            "  Step number justify vertical axis outside placement:<br>"
            "  - Enable step numberjustify vertical 'y' axis outside<br>"
            "    placement for multiple ranges pages - i.e. pages with<br>"
            "    dividers.<br><br>"
            "  Multi-step pages with page dividers have traditionally<br>"
            "  justified step numbers set outside and relative to the<br>"
            "  step's Parts List Instance (PLI) horizontal alignment<br>"
            "  so that all step numbers in the step group horizontal<br>"
            "  row display at the same vertical 'Y' axis.<br><br>"
            "  With this setting checked, step numbers are justified<br>"
            "  to he margin distance away from its respective PLI.<br>"
            "  Therefore, step numbers map display at different 'Y'<br>"
            "  axes according to the height of the PLI<br><br>"
            "  By default this setting is disabled so that the<br>"
            "  behaviour is set as the default.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_MULTI_STEP_JUSTIFY_Y_AXIS_OUTSIDE_PLACEMENT")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE
        {
            QObject::tr(
            "  Configure and edit your instructions document<br>"
            "  page settings.<br><br>"
            "  Use the Section drop-down dialog to select between<br>"
            "  front and back cover pages, and content page header<br>"
            "  or footer.<br><br>"
            "  Most settings will be automatically extracted from<br>"
            "  the open model file or setting defined in Preferences.<br><br>"
            "  Regardless, you can change pre-populated settings with<br>"
            "  this dialog.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_AUTHOR_EMAIL
        {
            QObject::tr(
            "  Enable and configure the author and email address.<br>"
            "  - Section (Author): select the front cover, back cover,<br>"
            "    page header, or page footer to place the author.<br><br>"
            "  - Section (Email Address): select the back cover,<br>"
            "    page header, or page footer to place the email address.<br><br>"
            "  - Content: enter or update the document author and<br>"
            "    email address.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_AUTHOR_EMAIL")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_LPUB3D_LOGO
        {
            QObject::tr(
            "  Enable and configure the %1 logo image.<br>"
            "  - Section: only the back cover %1 logo is configurable<br><br>"
            "  Enable and configure the %1 logo border.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_LPUB3D_LOGO")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_COVER_IMAGE
        {
            QObject::tr(
            "  Enable and configure the front cover image.<br>"
            "  - Section: only the front cover image is configurable<br><br>"
            "  Enable and configure the front cover image border.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_COVER_IMAGE")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_DESCRIPTION
        {
            QObject::tr(
            "  Enable and configure the model description.<br>"
            "  - Section: only the front cover model description<br>"
            "    is configurable.<br><br>"
            "  - Content: enter or update the model description.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_DESCRIPTION")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_DOCUMENT_LOGO
        {
            QObject::tr(
            "  Enable and configure the document logo image.<br>"
            "  - Section: select the front cover, back cover,<br>"
            "    to place the document<br><br>"
            "  Enable and configure the document logo border.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_DOCUMENT_LOGO")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_LEGO_DISCLAIMER
        {
            QObject::tr(
            "  Enable and configure the LEGO disclaimer.<br>"
            "  - Section: only the back cover LEGO disclaimer<br>"
            "    is configurable.<br><br>"
            "  - Content: enter or update the LEGO disclaimer.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_LEGO_DISCLAIMER")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_MODEL_PARTS_ID
        {
            QObject::tr(
            "  Enable and configure the model ID and parts count.<br>"
            "  - Section: only the front cover model id and<br>"
            "    parts count is configurable.<br><br>"
            "  - Content: enter or update the document<br>"
            "    model id and parts count.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_MODEL_PARTS_ID")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_LPUB3D_PLUG
        {
            QObject::tr(
            "  Enable and configure the %1 plug.<br>"
            "  - Section: only the back cover %1 plug<br>"
            "    is configurable.<br><br>"
            "  - Content: The %1 plug is not editable.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_LPUB3D_PLUG")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_TITLE
        {
            QObject::tr(
            "  Enable and configure the document title.<br>"
            "  - Section: select the front or back cover page.<br><br>"
            "  - Content: enter or update the document title.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_TITLE")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_PUBLISH_DESCRIPTION
        {
            QObject::tr(
            "  Enable and configure a publisher description.<br>"
            "  - Section: select the front cover page,<br>"
            "    to place the publisher description.<br><br>"
            "  - Content: enter or update the document<br>"
            "    publisher description.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_PUBLISH_DESCRIPTION")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_WEBSITE_URL_COPYRIGHT
        {
            QObject::tr(
            "  Enable and configure website URL and document copyright.<br>"
            "  - Section (Website URL): select the back cover,<br>"
            "    page header, or page footer to place the website URL.<br><br>"
            "  - Section (Copyright): select the front cover, back cover,<br>"
            "    page header, or page footer to place the document<br>"
            "    copyright.<br><br>"
            "  - Content: enter or update the document website URL<br>"
            "    and document copyright.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_WEBSITE_URL_COPYRIGHT")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_VIEW
        {
            QObject::tr(
            "  The following configuration setup are available for<br>"
            "  document pages:<br>"
            "  ° Configure the page size and orientation settings.<br><br>"
            "  ° Configure the page background.<br><br>"
            "  ° Enable and configure the page border.<br><br>"
            "  ° Configure the page margins.<br><br>"
            "  ° Configure the page header height.<br><br>"
            "  ° Configure the page footer height.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_VIEW")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_DISCLAIMER
        {
            QObject::tr(
            "  Configure your instruction document disclaimer"
            "  and display the %1 logo and 'plug' statement.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_DISCLAIMER")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_MODEL
        {
            QObject::tr(
            "  Configure your instruction document page title,<br>"
            "  cover image, description, model id, parts and<br>"
            "  submodel level color.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_MODEL")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_NUMBER_TEXT
        {
            QObject::tr(
            "  Enable and configure the page number display,<br>"
            "  look, and placement.<br><br>"
            "  Enable and configure default plain and<br>"
            "  rich text placement.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_NUMBER_TEXT")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_NUMBER_DISPLAY
        {
            QObject::tr(
            "  The following configuration setup is available for<br>"
            "  page number display:<br>"
            "  - Display Page Number: turn or off the page number display.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_NUMBER_DISPLAY")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_NUMBER_PLACEMENT
        {
            QObject::tr(
            "  The following configuration setup is available for<br>"
            "  page number placement:<br>"
            "  - Alternate Corners: place the page number on alternate<br>"
            "    corners - like books.<br>"
            "  - Page Number Always in Same Place: page number placed<br>"
            "    at fixed position.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_NUMBER_PLACEMENT")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_TEXT_PLACEMENT
        {
            QObject::tr(
            "  The following configuration setup is available for<br>"
            "  page number display:<br>"
            "  - Enable Text Placement: turn or off display<br>"
            "    placement dialog when inserting text.<br><br>"
            "  - Text Placement: set default text placement<br>"
            "    using the placement dialog.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_TEXT_PLACEMENT")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PAGE_PUBLISH
        {
            QObject::tr(
            "  Configure your instruction document<br>"
            "  author, email address, Universal Resource Locator (URL)<br>"
            "  publisher description, copyright, and image logo.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PAGE_PUBLISH")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_BOM
        {
            QObject::tr(
            "  Configure and edit the Bill Of Material (BOM)<br>"
            "  settings for your instructions document.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_BOM")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_PLI
        {
            QObject::tr(
            "  Configure and edit the Part List Instance (PLI)<br>"
            "  settings for your instructions document.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_PLI")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_BACKGROUND_BORDER
        {
            QObject::tr(
            "  Configure your instruction document part list<br>"
            "  display, placement, background, border, margins<br>"
            "  and size constrain settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_BACKGROUND_BORDER")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_BACKGROUND_BORDER_PARTS_LIST
        {
            QObject::tr(
            "  The following configuration setup are available for<br>"
            "  part list:<br>"
            "  - Show Parts List: turn or off the parts list display.<br><br>"
            "  - Parts List Placement: change the parts list placement.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_BACKGROUND_BORDER_PARTS_LIST")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_CONTENTS
        {
            QObject::tr(
            "  The following configuration setup are available in<br>"
            "  the Part List Content dialog:<br>"
            "  - Configure Parts properties including part images.<br>"
            "    part camera orientation, groups and count<br>"
            "  - Configure More part properties including sort<br>"
            "    order and direction, stud style, submodel PLI<br>"
            "    display and show individual parts.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_CONTENTS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_PARTS
        {
            QObject::tr(
            "  Configure your instruction document part images<br>"
            "  default part orientaion, part groups, and part<br>"
            "  count settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_PARTS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_PARTS_ORIENTATION
        {
            QObject::tr(
            "  The following configuration setup are available for<br>"
            "  part orientation.<br>"
            "  - Configure the camera field of view.<br><br>%1"
            "  - Configure the camera angles or viewpoint.%2<br>")
            .arg(MetaDefaults::getPreferredRenderer() == QLatin1String("Native")
            ? QObject::tr("<br>"
            "  - Configure the camera near plane.<br><br>"
            "  - Configure the camera far z plane.<br>")
            : QObject::tr("<br>"
            "  - Camera near and far z planes are disabled<br>"
            "    when the preferred renderer is not Native.<br>"))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_PARTS_ORIENTATION")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_PARTS_MOVABLE_GROUPS_BOM
        {
            QObject::tr(
            "  Set part image, instance count, annotation and<br>"
            "  element id movable. This means you can drag these<br>"
            "  items to your desired position.<br><br>"
            "  The following configuration setup are available for<br>"
            "  Bill of Material (BOM) part groups:<br>"
            "  - Movable Part Groups: turn or off movable property.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_PARTS_MOVABLE_GROUPS_BOM")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_PARTS_MOVABLE_GROUPS_PLI
        {
            QObject::tr(
            "  Set part image, instance count and part annotation<br>"
            "  movable. This means you can drag these items to your<br>"
            "  desired position.<br><br>"
            "  The following configuration setup is available for<br>"
            "  Part List Instance (PLI) part groups:<br>"
            "  - Movable Part Groups: turn or off movable property.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_PARTS_MOVABLE_GROUPS_PLI")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_MORE_OPTIONS
        {
            QObject::tr(
            "  Configure your instruction document part sort<br>"
            "  order, sort direction, stud style, edge color<br>"
            "  automation and submodel display settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_MORE_OPTIONS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_MORE_OPTIONS_SHOW_SUBMODELS
        {
            QObject::tr(
            "  The following configuration setup is available for<br>"
            "  submodels:<br>"
            "  - Show in Parts List: show submodel in the parts<br>"
            "    list instance (PLI).%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_MORE_OPTIONS_SHOW_SUBMODELS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_MORE_OPTIONS_SHOW_INDIVIDUAL_BOM_PARTS
        {
            QObject::tr(
            "  When checked, individual parts will be displayed in<br>"
            "  the Bill Of Materials. This means parts will not show<br>"
            "  an instance (Nx) value. All other BOM behaviour will<br>"
            "  be preserved.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_MORE_OPTIONS_SHOW_INDIVIDUAL_BOM_PARTS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_MORE_OPTIONS_SHOW_INDIVIDUAL_PLI_PARTS
        {
            QObject::tr(
            "  When checked, individual parts will be displayed in<br>"
            "  the Parts List Instance. This means parts will not<br>"
            "  show an instance (Nx) value. All other PLI behaviour<br>"
            "  will be preserved.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_MORE_OPTIONS_SHOW_INDIVIDUAL_PLI_PARTS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_ANNOTATION
        {
            QObject::tr(
            "  The following configuration setup are available for<br>"
            "  part list annotations:<br>"
            "  - Configure the Annotation Options properties.<br><br>"
            "  - Configure the Annotation Styles properties.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_ANNOTATION")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_ANNOTATION_OPTIONS
        {
            QObject::tr(
            "  Configure your instruction document part annotation<br>"
            "  display, enable annotation types and annotation text<br>"
            "  format settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_ANNOTATION_OPTIONS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_ANNOTATION_OPTIONS_WITH_TEXT_FORMAT
        {
            QObject::tr(
            "  Part annotation options.<br>"
            "  - Display PartList (PLI) Annotation: select<br>"
            "    annotation source from Title, Free Form or<br>"
            "    Fixed annotations.<br><br>"
            "  - Enable Annotation Type: enable or disable<br>"
            "    annotation for axles, beams, cables, connectors,<br>"
            "    hoses, panels or custom annotations.<br><br>"
            "  - Default Text Format: configure the default<br>"
            "    annotation text font color and margins.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_ANNOTATION_OPTIONS_WITH_TEXT_FORMAT")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_ANNOTATION_STYLE_PLI
        {
            QObject::tr(
            "  Configure your instruction document part list instance<br>"
            "  part annotation display, enable annotation types,<br>"
            "  and annotation text format settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_ANNOTATION_STYLE_PLI")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_ANNOTATION_STYLE_BOM
        {
            QObject::tr(
            "  Configure your instruction document bill of materials<br>"
            "  part annotation display, annotation types,<br>"
            "  element identifiers and annotation text format<br>"
            "  settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_ANNOTATION_STYLE_BOM")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_ANNOTATION_STYLE_SELECT_BOM
        {
            QObject::tr(
            "  Edit your instruction document bill of materials (BOM)<br>"
            "  part annotation styles, annotation background,<br>"
            "  element identifiers and annotation text format<br>"
            "  settings.<br><br>"
            "  Edit annotation style:<br>"
            "  Select the style to edit by clicking the radio<br>"
            "  button of the following styles:<br>"
            "  - None: No style selected.<br><br>"
            "  - Square: square style used by beams, cables,<br>"
            "    connectors, and hoses.<br><br>"
            "  - Circle: circle style used by axles and panels.<br><br>"
            "  - Rectangle: rectangular sytle used by extended<br>"
            "    title and/or free form part annotations.<br><br>"
            "  - Element: rectangular sytle used by element<br>"
            "    identifiers.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_ANNOTATION_STYLE_SELECT_BOM")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PART_ANNOTATION_STYLE_SELECT_PLI
        {
            QObject::tr(
            "  Edit your instruction document part list instance (PLI)<br>"
            "  part annotation styles, annotation background,<br>"
            "  annotation border, annotation text format and<br>"
            "  annotation with fixed size settings.<br><br>"
            "  Edit annotation style:<br>"
            "  Select the style to edit by clicking the radio<br>"
            "  button of the following styles:<br>"
            "  - None: No style selected.<br><br>"
            "  - Square: square style used by beams, cables,<br>"
            "    connectors, and hoses.<br><br>"
            "  - Circle: circle style used by axles and panels.<br><br>"
            "  - Rectangle: rectangular sytle used by extended<br>"
            "    title and/or free form part annotations.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PART_ANNOTATION_STYLE_SELECT_PLI")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PROJECT
        {
            QObject::tr(
            "  Configure and edit project settings for your<br>"
            "  instructions document.<br><br>"
            "  The following configuration setup are available in<br>"
            "  the project global setup:<br>"
            "  - Render options<br>"
            "  - Parse options%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PROJECT")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PROJECT_RENDERER_OPTIONS
        {
            QObject::tr(
            "  Configure and edit renderer settings<br>"
            "  for your instructions document.<br><br>"
            "  Edit your instruction document preferred<br>"
            "  renderer, resolution, stud style and viewer<br>"
            "  behaviour on cover page settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PROJECT_RENDERER_OPTIONS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PROJECT_PARSE_OPTIONS
        {
            QObject::tr(
            "  Configure and edit parse LDraw file settings<br>"
            "  for your instructions document.<br><br>"
            "  Edit your instruction document build modifications,<br>"
            "  buffer exchange, submodel instances behaviour, step<br>"
            "  number, unofficial parts behaviour and start numbers<br>"
            "  behaviour.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PROJECT_PARSE_OPTIONS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PROJECT_PARSE_BUFFER_EXCHANGE
        {
            QObject::tr(
            "  The following configuration setup is available for<br>"
            "  buffer exchange:<br>"
            "  - Parse Single Step With NOSTEP Or BUFEXCHG Commands: <br>"
            "    Parse single steps containing NOSTEP or BUFEXCHG<br>"
            "    commands. Multi-step groups are automatically parsed.<br><br>"
            "    Typically, a step with a NOSTEP command is not parsed.<br>"
            "    However, bufexchg commands are not properly parsed if<br>"
            "    the bufexchg command shares a step with the nostep<br>"
            "    command and the step is not parsed.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PROJECT_PARSE_BUFFER_EXCHANGE")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PROJECT_START_NUMBERS
        {
            QObject::tr(
            "  The following configuration setup are available<br>"
            "  for your loaded instruction document start number:<br>"
            "  - Step Number: set the start number for the first<br>"
            "    step of the loaded instruction document.<br><br>"
            "  - Page Number: set the start number for the first<br>"
            "    page of the loaded instruction document.<br><br>"
            "  The mininimum allowed spin box value is 0 and<br>"
            "  the maximum is 10000.<br>"
            "  Spin box values are incremented by 1 step unit.<br><br>"
            "  Setting the start number is useful when editing<br>"
            "  large model instruction documents. You can use this<br>"
            "  feature to split the instruction document into more<br>"
            "  than one book - starting each book at it's<br>"
            "  corresponding step and/or page number.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PROJECT_START_NUMBERS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PROJECT_SUBMODEL_OPTIONS
        {
            QObject::tr(
            "  Configure LDraw submodel file settings for your<br>"
            "  instructions document.<br><br>"
            "  Edit your instruction document submodel instance<br>"
            "  consolidation, instance count, and set submodel<br>"
            "  substitute part as an unofficial part settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PROJECT_SUBMODEL_OPTIONS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_PROJECT_SUBMODEL_SUBSTITUTE_PART
        {
            QObject::tr(
            "  The following configuration setup is available for<br>"
            "  submodels used as a substitute part:<br>"
            "  - Set As Unofficial Part: Check to automatically set<br>"
            "  submodel substitute parts as unofficial part.<br>"
            "  This setting will automatically set subfiles designated<br>"
            "  as a submodel, which is the case when there is no<br>"
            "  !LDRAW_ORG part type file header, as an unofficial part.<br>"
            "  If left unchecked, you will be prompted with a warning<br>"
            "  message to giving you the options to consider adding a<br>"
            "  part type header to the file or to dynamically set the<br>"
            "  submodel as an unofficial part and proceed with the page<br>"
            "  load.<br><br>"
            "  By default this setting is disabled (unchecked).%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_PROJECT_SUBMODEL_SUBSTITUTE_PART")
#else
                ""
#endif
                )
        },
        // WT_SETUP_SUBMODEL
        {
            QObject::tr(
            "  Configure and edit the submodel preview<br>"
            "  settings for your instructions document.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_SUBMODEL")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_SUBMODEL_SUBMODEL
        {
            QObject::tr(
            "  The following configuration setup are available in<br>"
            "  the Submodel dialog:<br>"
            "  - Configure the Preview properties.<br>"
            "  - Configure the Settings properties.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_SUBMODEL_SUBMODEL")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_SUBMODEL_PREVIEW
        {
            QObject::tr(
            "  Configure your instruction document submodel<br>"
            "  preview options.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_SUBMODEL_PREVIEW")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_SUBMODEL_BACKGROUND
        {
            QObject::tr(
            "  Configure your instruction document submodel<br>"
            "  background, border and margins settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_SUBMODEL_BACKGROUND")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_SUBMODEL_CONTENTS
        {
            QObject::tr(
            "  The following configuration setup are available in<br>"
            "  the Contents dialog:<br>"
            "  - Configure the Image properties.<br>"
            "  - Configure the More properties.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_SUBMODEL_CONTENTS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_SUBMODEL_IMAGE
        {
            QObject::tr(
            "  Configure your instruction document submodel<br>"
            "  image, submodel constrain, submodel orientation,<br>"
            "  submodel and default step rotation settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_SUBMODEL_IMAGE")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_SUBMODEL_INSTANCE_COUNT
        {
            QObject::tr(
            "  Configure your instruction document submodel<br>"
            "  instance count and stud style and automate<br>"
            "  edge color settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_SUBMODEL_INSTANCE_COUNT")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_SHARED_IMAGE_SIZING
        {
            QObject::tr(
            "  The following configuration setup are available<br>"
            "  for image sizing:<br>"
            "  - Configure the image scale.<br><br>"
            "  - Configure the image margins.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_SHARED_IMAGE_SIZING")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_SHARED_MODEL_ORIENTATION
        {
            QObject::tr(
            "  The following configuration setup are available for<br>"
            "  assembly orientation:<br>"
            "  - Configure the camera field of view.<br>%1"
            "  - Configure the camera angles or viewpoint.%2<br>")
            .arg(MetaDefaults::getPreferredRenderer() == QLatin1String("Native")
            ? QObject::tr("<br>"
            "  - Configure the camera near z plane.<br><br>"
            "  - Configure the camera far z plane.<br>")
            : QObject::tr("<br>"
            "  - Camera near and far z planes are disabled<br><br>"
            "    when the preferred renderer is not Native.<br>"))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_SHARED_IMAGE_SIZING")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_SHARED_POINTERS
        {
            QObject::tr(
            "  The following configuration setup are available for<br>"
            "  pointers:<br>"
            "  - Configure the pointer border.<br>"
            "  - Configure the pointer line.<br>"
            "  - Configure the pointer tip.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_SHARED_POINTERS")
#else
                 ""
#endif
                )
        },
        // WT_SETUP_SHARED_SUBMODEL_LEVEL_COLORS
        {
            QObject::tr(
            "  Configure the submodel level background color.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_SETUP_SHARED_SUBMODEL_LEVEL_COLORS")
#else
                 ""
#endif
                )
        },
        //*************************************
        //*  DIALOG ENTRIES
        //*************************************
        // WT_DIALOG_APPEND_PAGE_OPTION **
        {
            QObject::tr(
            "  Select the append page option.<br><br>"
            "  Append page options determine where a new page will<br>"
            "  be added to the instruction document.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_APPEND_PAGE_OPTION")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_BACKGROUND
        {
            QObject::tr(
            "  Configure the background type, color,<br>"
            "  image and image fill settings when available.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_BACKGROUND")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_BOM_OPTION
        {
            QObject::tr(
            "  Select the Bill of Material option.<br><br>"
            "  Use item toolTip to see additional description.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_BOM_OPTION")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_BORDER
        {
            QObject::tr(
            "  Configure the border type, line, line width, line color,<br>"
            "  and margin when available.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_BORDER")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_BUILD_MODIFICATIONS
        {
            QObject::tr(
            "  Edit build modifications.<br><br>"
            "  You can apply, remove, change or delete<br>"
            "  build modifications from this dialog.<br><br>"
            "  Select the desired build modification and<br>"
            "  click OK to perform the respective apply,"
            "  remove, or change (load) action.<br>"
            "  Do not apply a build modification action that<br>"
            "  is the same as the last action of the build<br>"
            "  modification. Likewise, if you are adding<br>"
            "  the first action after creating the build<br>"
            "  modification, then the this action should<br>"
            "  be the remove action.<br><br>"
            "  If you are loading a build modification, you<br>"
            "  will be able to edit the loaded modification<br>"
            "  in the visual editor.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_BUILD_MODIFICATIONS")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_CAMERA_ANGLES **
        {
            QObject::tr(
            "  Configure camera angle latitude, longitude<br>"
            "  or camera viewpoint settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_CAMERA_ANGLES")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_CAMERA_FOV **
        {
            QObject::tr(
            "  Configure camera field of view (FOV), near and<br>"
            "  far plane settings.<br><br>"
            "  Camera near and far plane settings are only enabled<br>"
            "  when the preferred renderer is set to Native.<br><br>"
            "  The current preferred renderer is %1%2<br>")
            .arg(MetaDefaults::getPreferredRenderer())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_CAMERA_FOV")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_CAMERA_LAT_LON
        {
            QObject::tr(
            "  Configure the camera angles.<br>"
            "  - Latitude: set the camera angle x-axis in degrees.<br><br>"
            "  - Longitude: set the camera angle y-axis in degrees.<br><br>"
            "  - Distance Factor: optionally, you can check this<br>"
            "    to enable the default distance factor spin dialog<br>"
            "    where you can change the default camera distance.<br>"
            "  The Distance dialog is a read-only display of the<br>"
            "  calculated camera distance. The calculated distance<br>"
            "  uses the distance factor, default position, page width<br>"
            "  image resolution, image scale and LDraw Unit (LDU).%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_CAMERA_LAT_LON")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_CONSTRAIN
        {
            QObject::tr(
            "  Configure the size of an item using its constraint<br>"
            "  type - area, square, width, height or columns.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_CONSTRAIN")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_CYCLE **
        {
            QObject::tr(
            "  Cycle each page when navigating forward by more than <br>"
            "  one page.<br><br>"
            "  Select Yes to perform the cycle behaviour across the<br>"
            "  specified pages<br>"
            "  Select No to perform a direct jump to the the specified page.<br>"
            "  Select Cancel to abort the navigation action altogether.<br><br>"
            "  If you do not want to display this dialog at each Update,<br>"
            "  you may elect to not show this dialog. When the cycle<br>"
            "  each page dialog is presented upon clicking Update,<br>"
            "  an additional dialog to Remember your choice and do<br>"
            "  not show this message again is presented.<br>"
            "  When Build Modifications is enabled, the Cycle Pages dialog<br>"
            "  is not displayed and the Preferences setting is disabled<br>"
            "  because cycle pages is alway on when buildMods is enabled.<br>"
            "  You may reset the dialog display from the Command Editor<br>"
            "  dialog.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_CYCLE")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_EDITOR_PREFERENCES
        {
            QObject::tr(
            "  Configure the meta command editor font, text decoration<br>"
            "  buffered paging and selected item preferences.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_EDITOR_PREFERENCES")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_EXPORT_PAGES
        {
            QObject::tr(
            "  Configure the instruction document export options or<br>"
            "  continuous page processing options.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_EXPORT_PAGES")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_DIVIDER
        {
            QObject::tr(
            "  Configure and edit a placed divider width, length,<br>"
            "  type, length value, color and margins.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_DIVIDER")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_DOUBLE_SPIN **
        {
            QObject::tr(
            "  Set the value for this item using a decimal number.<br><br>"
            "  Select 'What's this' Help on this check box's<br>"
            "  parent group box to see additional details<br>"
            "  about the behaviour enabled by this control.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_DOUBLE_SPIN")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_FADE_HIGHLIGHT
        {
            QObject::tr(
            "  Enable or disable fade previous steps and highlight<br>"
            "  current step.<br><br>"
            "  This dialog is only available when Fade Previous Steps<br>"
            "  Setup and/or Highlight Current Step Setup is enabled.<br>"
            "  Fade and highlight step Setup is configured from the<br>"
            "  Configuration, Build Instruction Setup menu using<br>"
            "  Fade Steps Setup and/or Highlight Step Setup menu<br>"
            "  actions.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_FADE_HIGHLIGHT")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_FIND_REPLACE
        {
            QObject::tr(
            "  Find and replace text in the currently loaded<br>"
            "  LDraw model file.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_FIND_REPLACE")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_FLOAT **
        {
            QObject::tr(
            "  Set the value for this item using a decimal number.<br><br>"
            "  Select 'What's this' Help on this check box's<br>"
            "  parent group box to see additional details<br>"
            "  about the behaviour enabled by this control.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_FLOAT")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_FLOAT_PAIR **
        {
            QObject::tr(
            "  Set the value pair for this item using decimal<br>"
            "  numbers.<br><br>"
            "  Select 'What's this' Help on this check box's<br>"
            "  parent group box to see additional details<br>"
            "  about the behaviour enabled by this control.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_FLOAT_PAIR")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_GRADIENT
        {
            QObject::tr(
            "  Configure background gradient color, type and spread<br>"
            "  method settings.<br><br>"
            "  Alternatively, you can chose from the three pre-defined<br>"
            "  default gradient configurations.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_GRADIENT")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_LDRAW_COLOR
        {
            QObject::tr(
            "  Set or change the LDraw color for the currently<br>"
            "  selected item.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_LDRAW_COLOR")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_LDRAW_FILES_LOAD
        {
            QObject::tr(
            "  View the file load status for the currently loaded<br>"
            "  LDraw model file.<br><br>"
            "  - Total validated parts: the total number of parts<br>"
            "    validated during the model file load.<br>"
            "  - Unique validated parts: the number of unique parts<br>"
            "    validated during the file load.<br>"
            "  - Submodels: number of loaded MPD submodels.<br>"
            "  - Subfiles: number of loaded LDR subfiles.<br>"
            "  - Inline parts: parts defined in the loaded model file.<br>"
            "    These are usually unofficial parts.<br>"
            "  - Inline generated parts: LDCad generated parts defined<br>"
            "    in the loaded model file.<br>"
            "  - Inline primitives: primitives defined in the loaded<br>"
            "    model file.<br>"
            "  - Inline subparts: sublevel parts defined in the loaded<br>"
            "    model file.<br>"
            "  - Include files: LPub meta commands include file.<br>"
            "    model file.<br>"
            "  - Loaded steps: number of steps detected in the loaded<br>"
            "    model file.<br>"
            "  - Loaded subfiles: submodels, inline parts and primitives<br>"
            "    defined in the loaded model file.<br>"
            "  - Loaded lines: number of lines loaded into the model<br>"
            "    repository.<br>"
            "  - Error - Missing Parts: parts encountered during the<br>"
            "    file load that were not found in the official or<br>"
            "    unofficial LDraw archive library, the currently<br>"
            "    specified LDraw disc library or any of the specified<br>"
            "  - Warning - Empty Submodels: empty submodels detected.<br>"
            "    These submodels are not added to the model repository<br>"
            "    so they will not be visible in %1.<br>"
            "  - Warning - Include file issues: include file anomalies<br>"
            "    such as including part (type 1 - 5) lines.<br>"
            "  - Elapsed time: the time taken to load the model file.<br><br>"
            "  Part list view: list of loaded items as described above.<br>"
            "  Items are grouped with a header description and number<br>"
            "  of items detected. Item rows present the item name such<br>"
            "  as a part id or submodel name and description.<br>"
            "  description (if not missing).%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_LDRAW_FILES_LOAD")
#else
                 ""
#endif
                )
            /* Do not add these into the load status dialogue because they are not loaded in the LDrawFile.subfiles
            "  - Warning - Primitive Parts: primitive part encountered<br>"
            "    primitive parts are not normally defined at top level.<br><br>"
            "  - Warning - Sublevel Parts: sublevel part encountered<br>"
            "    sublevel parts are not normally defined at top level.<br><br>"
            */
        },
        // WT_DIALOG_LDRAW_SEARCH_DIRECTORIES
        {
            QObject::tr(
            "  Configure %1 LDraw content search directories.<br><br>"
            "  You can edit, add, open, position and reset<br>"
            "  search directory entries using this dialog.<br><br>"
            "  These directories are searched for LDraw part and<br>"
            "  model files during file load and image rendering.<br><br>"
            "  Directories enterend here are shared with the current<br>"
            "  %1 preferred renderer.<br><br>"
            "  For the native renderer and 3D viewer, parts in these<br>"
            "  locations are can be automatically loaded into the %1<br>"
            "  unofficial LDraw archive file if this option is enabled<br>"
            "  at the Configuration menu using the Archive Unofficial<br>"
            "  Parts On Launch menu action.<br><br>"
            "  Alternatively, you can interctively update the<br>"
            "  unofficial archive library from the Configuration menu<br>"
            "  using the Archive Unofficial Parts menu action.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_LDRAW_SEARCH_DIRECTORIES")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_LDVIEW_HTML_PART_LIST_EXPORT
        {
            QObject::tr(
            "  Configure html export display, part reference, and<br>"
            "  column order settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_LDVIEW_HTML_PART_LIST_EXPORT")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_LDVIEW_POV_EXPORT_OPTIONS
        {
            QObject::tr(
            "  Configure POV file generation general, geometry,<br>"
            "  pov geometry, lighting, pov lights, material<br>"
            "  properties, transparent material properties,<br>"
            "  rubber material properties, and chrome material<br>"
            "  properties settings.<br><br>"
            "  - Reset: This resets all the POV file generation<br>"
            "    preferences to their default values. As with all other<br>"
            "    preference changes, you must hit OK or Apply for the<br>"
            "    reset values to go into effect.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_LDVIEW_POV_EXPORT_OPTIONS")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_LOCAL **
        {
            QObject::tr(
            "  Select local or global scope for currently selected<br>"
            "  meta command.<br><br>"
            "  Selecting yes will constrain the current meta command<br>"
            "  to the current step.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_LOCAL")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_LOOK_AT_TARGET_AND_STEP_ROTATION
        {
            QObject::tr(
            "  Configure the default 'look at' target<br>"
            "  position and step rotation angles.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_LOOK_AT_TARGET_AND_STEP_ROTATION")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_META_COMMANDS
        {
            QObject::tr(
            "  Edit %1 meta command description and create command<br>"
            "  snippets.<br><br>"
            "  Use the command filter to efficiently filter the<br>"
            "  command or snippet list.<br><br>"
            "  You can insert/autocomplete individual meta commands<br>"
            "  using Ctrl+E to select the desired command.<br><br>"
            "  You can insert/autocomplete snippets using<br>"
            "  Ctrl+Space to select the desired command or to<br>"
            "  automatically insert the command after typing<br>"
            "  the 'trigger.'%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_META_COMMANDS")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_PAGE_ORIENTATION
        {
            QObject::tr(
            "  Configure page orientation choosing between<br>"
            "  landscape and portrait.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_PAGE_ORIENTATION")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_PAGE_SIZE
        {
            QObject::tr(
            "  Configure page width and height size settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_PAGE_SIZE")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_PLACEMENT
        {
            QObject::tr(
            "  Configure the desired placement setting for this item.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_PLACEMENT")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_PLI_ANNOTATION
        {
            QObject::tr(
            "  Configure dispaly annotation, annotation source and<br>"
            "  annotation types to display.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_PLI_ANNOTATION")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_PART_SORT_ORDER
        {
            QObject::tr(
            "  Configure the part sort order and direction<br>"
            "  settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_PART_SORT_ORDER")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_POINTER_ATTRIB
        {
            QObject::tr(
            "  Configure the pointer border, line and<br>"
            "  optionally pointer tip if available.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_POINTER_ATTRIB")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_PREFERRED_RENDERER
        {
            QObject::tr(
            "  Configure the default preferred step image<br>"
            "  renderer.<br>"
            "  %1 has 4 built-in step image renderers. They<br>"
            "  are Native, LDGLite, LDView and POV-Ray.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_PREFERRED_RENDERER")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_BLENDER_RENDER
        {
            QObject::tr(
            "  Render the current step image usng Blender.<br><br>"
            "  Configure the Blender single image render<br>"
            "  settings and output.<br>"
            "  - Browse: Change the name and or location of<br>"
            "    the rendered image.<br>"
            "  - Render: Render the image.<br>"
            "  - Settings: launch the Blender settings dialog.<br>"
            "  - Output: process output is not available for<br>"
            "    the Blender render process.<br><br>"
            "  If you reset the rendered image, the default settings<br>"
            "  (except image width and height), will also be reset.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_BLENDER_RENDER")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_BLENDER_RENDER_SETTINGS
        { //-------------------------------------------------------//
            QObject::tr(
            "  You can configure the Blender LDraw addon settings.<br>"
            "  - Blender Executable: set Blender path. On entry,<br>"
            "    %1 will automatically apply the setting and<br>"
            "    attempt the configure the LDraw addon.<br>"
            "  - %1 Blender LDraw Addon: you can update the LDraw<br>"
            "    addon which will downlod the latest addon or apply<br>"
            "    the current addon if the version is the same or newer<br>"
            "    than the online version.<br>"
            "    You can view the standard output log for the update.<br><br>"
            "  - Enabled Addon Modules: check the desired import module.<br>"
            "    The LDraw Import TN import module is the long-standing<br>"
            "    Blender LDraw import addon, while the LDraw Import MM<br>"
            "    addon was recently introduced. The latter addon also<br>"
            "    offer LDraw export functionality.<br>"
            "    The %1 3D Image Render addon is mandatory but if<br>"
            "    no import module is enabled, none of the modules<br>"
            "    will be enabled in Blender so it will not be possible<br>"
            "    to perform an LDraw model import or render.<br><br>"
            "  - LDraw Import Addon Paths: addon paths are specific<br>"
            "    to the enabled addon import module.<br>"
            "  - LDraw Import Addon Settings: addon settings are<br>"
            "    specific to the enabled addon import module.<br>"
            "  - Apply: apply the addon path and setting preferences.<br>"
            "  - Show/Hide Paths: show or hide the addon paths<br>"
            "    display box.<br>"
            "  - Reset: reset the addon path and setting preferences.<br>"
            "    You can select how to reset addon settings.<br>"
            "    The choice is since last apply or system default.<br><br>"
            "  You can see the specific description of each setting<br>"
            "  if you hover over the setting to display its tooltip.<br><br>"
            "  Image Width, Image Height and Render Percentage are<br>"
            "  always updated from the current step model when the<br>"
            "  this dialog is opened. These settngs can be manually<br>"
            "  overridden, Also, when Crop Image is checked the<br>"
            "  current step cropped image width and height is<br>"
            "  calculated and and used<br><br>"
            "  Use the dialogue window scroll bar to access the<br>"
            "  complete selection of addon settings.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_BLENDER_RENDER_SETTINGS")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_POVRAY_RENDER
        {
            QObject::tr(
            "  Render the current step image using POV-Ray.<br><br>"
            "  Configure the POV-Ray single image render<br>"
            "  settings and output.<br>"
            "  - Browse: Change the name and or location of<br>"
            "    the rendered image.<br>"
            "  - Render: Render the image.<br>"
            "  - Settings: launch the POV-Ray settings dialog.<br>"
            "  - Output: view the render process standard output log.<br><br>"
            "  If you reset the rendered image, the default settings<br>"
            "  (except image width and height), will also be reset.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_POVRAY_RENDER")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_POVRAY_RENDER_SETTINGS
        {
            QObject::tr(
            "  Configure the POV-Ray single image render<br>"
            "  settings.<br><br>"
            "  Use the individual setting's tooltip to see<br>"
            "  setting description.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_POVRAY_RENDER_SETTINGS")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_ROTATE_ICON_SIZE
        {
            QObject::tr(
            "  Configure the rotate icon width and height<br>"
            "  in %1.%2<br>")
            .arg(MetaDefaults::getPreferredUnits())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_ROTATE_ICON_SIZE")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_STEP_ROTATION
        {
            QObject::tr(
            "  Configure the default step rotation.<br>"
            "  Set the x-angle, y-angle and z-angle, which are <br>"
            "  the individual rotation angles for the different<br>"
            "  axes in degree (-360 to 360).<br><br>"
            "  Set the step transformation using one of the three<br>"
            "  options - relative, absolute or additive.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_STEP_ROTATION")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_SIZE_AND_ORIENTATION
        {
            QObject::tr(
            "  Configure the page size and orientation settings.<br><br>"
            "  Page size settings use standard page sizes or<br>"
            "  page height and width values.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_SIZE_AND_ORIENTATION")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_SUBMODEL_LEVEL_COLORS
        {
            QObject::tr(
            "  Configure the submodel level page background color.<br><br>"
            "  These colors help differentiate the current submode<br>"
            "  level being edited.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_SUBMODEL_LEVEL_COLORS")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_SUBSTITUTE_PART
        {
            QObject::tr(
            "  Configure substitute part attributes<br>"
            "  current part, primary part substitution<br>"
            "  arguments, and extended part substitution<br>"
            "  arguments.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_SUBSTITUTE_PART")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_TEXT_EDIT
        {
            QObject::tr(
            "  Create and edit instruction document text entries.<br>"
            "  - Plain text: plain text format.<br>"
            "  - Rich text: hyper text markup language (html)<br>"
            "    text format.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_TEXT_EDIT")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_UNITS
        {
            QObject::tr(
            "  Configure the dot unit values using a decimal<br>"
            "  number in %1.<br><br>"
            "  Dot unit options are dots per inch (DPI) or<br>"
            "  dots per centimetre (DPC)<br><br>"
            "  Dot units are defined in Project Global Setup.%2<br>")
            .arg(MetaDefaults::getPreferredUnits())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_UNITS")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_VISUAL_CATEGORY_EDIT
        {
            QObject::tr(
            "  Edit the selected LDraw part category.<br><br>"
            "  Part category attributes are:<br>"
            "  - Name: you can change the category name.<br>"
            "  - Keywords: add or edit category keyword(s).<br><br>"
            "    Category keywords are taken from the LDraw<br>"
            "    part description and are prefixed with '^%',<br>"
            "    for example ^%Door.<br><br>"
            "    When specifying multiple keyowrds, each keyword<br>"
            "    entry must be delimited by ' | ', do observe the<br>"
            "    single space padding so and example would be<br>"
            "    ^%Door | ^%Window.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_VISUAL_CATEGORY_EDIT")
#else
                 ""
#endif
                )
        },
        // WT_DIALOG_VISUAL_CATEGORY_NEW
        {
            QObject::tr(
            "  Add an new LDraw part category.<br><br>"
            "  Part category attributes are:<br>"
            "  - Name: create a new category name.<br>"
            "  - Keywords: add category keyword(s).<br><br>"
            "    Category keywords are taken from the LDraw<br>"
            "    part description and are prefixed with '^%',<br>"
            "    for example ^%Door.<br><br>"
            "    When specifying multiple keyowrds, each keyword<br>"
            "    entry must be delimited by ' | ', do observe the<br>"
            "    single space padding so and example would be<br>"
            "    ^%Door | ^%Window.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_DIALOG_VISUAL_CATEGORY_NEW")
#else
                 ""
#endif
                )
        },
        //*************************************
        //*  CONTROL ENTRIES
        //*************************************
        // WT_CONTROL_COMMANDS_EDIT
        {
            QObject::tr(
            "  You can edit each meta command description to reflect<br>"
            "  content important to you. The default description is<br>"
            "  presented to give you a starting point.<br><br>"
            "  The following buttons will complete your command edit:<br>"
            "  - Ok: accept all changes in the currnt session.<br><br>"
            "  - Cancel: discard all changes in the current session.<br><br>"
            "  Edited commands are saved to an external file which is<br>"
            "  loaded at application startup.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_COMMANDS_EDIT")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_COMMANDS_FILTER
        {
            QObject::tr(
            "  You can filter the command list to efficiently<br>"
            "  find your desired entry.<br><br>"
            "  The filter dialog supports the following actions.<br>"
            "  - Auto-complete: keyword suggenstions are presented<br>"
            "    after the third entered character.<br>"
            "    This option is on by default.<br><br>"
            "  - Case Sensitive: respect upper and lower case entries<br>"
            "    as unique.<br><br>"
            "  - Fixed String: present entries matching the entered<br>"
            "    string. This option is on by default.<br><br>"
            "  - Regular Expression: present entries matching the<br>"
            "    entered regular expression.<br><br>"
            "  - Wildcard: present entries that satisfy the pattern<br>"
            "    created with the wildcard symbol '*'.<br><br>"
            "  Click the filter icon to expose the filer actions menu<br>"
            "  which allow you to change the default filter actions.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_COMMANDS_FILTER")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_COMMANDS_RESET
        {
            QObject::tr(
            "  Return to the default meta command description.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_COMMANDS_RESET")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_COMMANDS_UPDATE
        {
            QObject::tr(
            "  Use this button to enable the editor.<br>"
            "  Your changes are automatically saved to the current<br>"
            "  session and permanently saved when you accept your<br>"
            "  changes by clicking the OK button.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_COMMANDS_UPDATE")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_COMMANDS_VIEW
        {
            QObject::tr(
            "  This table view presents all %1 meta commands.<br><br>"
            "  The description of a selected meta command is<br>"
            "  displayed in the editor dialog.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_COMMANDS_VIEW")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_ALTERNATE_LDCONFIG
        {
            QObject::tr(
            "  You can define custom LDraw colors in an LDConfig file<br>"
            "  and enter the path to that file here.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_ALTERNATE_LDCONFIG")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_APPLY_CAMERA_ANGLES
        {
            QObject::tr(
            "  When parts in an assembly, submodel or part list are<br>"
            "  submitted to the renderer, the default latitude<br>"
            "  and longitude camera angles are applied to properly<br>"
            "  orient the rendered image.<br><br>"
            "  You can choose to have the default camera angles<br>"
            "  applied as follows:<br><br>"
            "  - Locally: camera angles applied by %1 when the parts<br>"
            "    are rotated and then submitted to the preferred<br>"
            "    renderer.<br><br>"
            "  - Renderer: camera angles applied by the the current<br>"
            "    preferred renderer %2. This is to say the angles<br>"
            "    are supplied to the renderer along with the parts.%3<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR),
                 MetaDefaults::getPreferredRenderer())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_APPLY_CAMERA_ANGLES")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_ATTRIBUTES
        {
            QObject::tr(
            "  Configure document attributes and cover page display.<br><br>"
            "  - Display all page attributes: when checked all %1<br>"
            "    page attributes are enabled for display.<br><br>"
            "    Page attributes are automatically configured from<br>"
            "    entries in preferences and from the loaded LDraw<br>"
            "    file.<br>"
            "    Alternativly, you can interctively configure page<br>"
            "    attributes from the Configuration menu using the<br>"
            "    Build Instructions Setup menu action.<br><br>"
            "    Page attributes are grouped by the the type of page<br>"
            "    they are allowed to be displayed on. The two<br>"
            "    types of pages are cover and content pages.<br><br>"
            "    Page attributes (and default placement) by page type<br>"
            "    are:<br><br>"
            "    - Front Cover: <br>"
            "      ° Logo (Bottom of page header)<br>"
            "      ° Model Name (Top Left of Title)<br>"
            "      ° Title (Left of Page - anchor attribute)<br>"
            "      ° Author (Bottom Left of Title)<br>"
            "      ° Part Count (Bottom Left of Title)<br>"
            "      ° Model Description (Bottom Left of Parts)<br>"
            "      ° Publisher Description (Bottom Left of<br>"
            "          Model Description)<br>"
            "      ° Cover Image (Center of page)<br><br>"
            "    - Content Page: <br>"
            "      ° URL (Top Left of page)<br>"
            "      ° Email Address (Top Right of page)<br>"
            "      ° Copyright (Bottom Left of page)<br>"
            "      ° Author (Left Bottom of Page Number)<br><br>"
            "    - Back Cover: <br>"
            "      ° Logo (Bottom of page header)<br>"
            "      ° Title (Center of page)<br>"
            "      ° Author (Bottom of Title)<br>"
            "      ° Copyright (Bottom of Author)<br>"
            "      ° URL (Bottom of Copyright)<br>"
            "      ° Email (Bottom of URL)<br>"
            "      ° LEGO Disclaimer (Bottom of Email)<br>"
            "      ° %1 Plug Text (Bottom of LEGO Disclaimer)<br>"
            "      ° %1 Plug Logo (Bottom of Plug Text)<br><br>"
            "  - Generage conver pages: if you have not manually<br>"
            "    added cover page meta commands to your LDraw file,<br>"
            "    you can automatically create front and back cover<br>"
            "    pages by enabling this item.<br><br>"
            "  - Generage PDF Table of Contents: this option is not<br>"
            "    currently enabled in %1.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_ATTRIBUTES")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_CAMERA_DEFAULTS
        {
            QObject::tr(
            "  Configure the Native renderer and 3DViewer default<br>"
            "  camera settings.<br><br>"
            "  - Display properties: show or hide camera settings in<br>"
            "    the 3D viewer Properties window. An item's properties<br>"
            "    are displayed when it is selected.<br><br>"
            "  - Field of View: the default Native renderer and<br>"
            "    3D viewer FOV value.<br>"
            "    Set the camera field of view in degrees.<br><br>"
            "  - Near Plane: the default Native renderer and 3D viewer<br>"
            "    near Z plane value.<br>"
            "    Set the camera near plane using a decimal number.<br><br>"
            "  - Far Plane: the default Native renderer and 3D viewer<br>"
            "    far Z plane value.<br>"
            "    Set the camera far plane using a decimal number.<br><br>"
            "  - Default Distance Factor: used to adjust the camera<br>"
            "    position nearer to or farther away from the position<br>"
            "    of the assembly or part.<br>"
            "    Set the distance factor using a decimal number.<br><br>"
            "  - Default Camera Position: sets the initial position<br>"
            "    of the camera relative to the part or assembly.<br>"
            "    Set the default position using a decimal number.<br><br>"
            "  - Camera Distance Factor: this is the calculated value<br>"
            "    of the Default Camera Position divided by (Default<br>"
            "    Distance Factor multiplied by negative one).<br><br>"
            "  You can manipulate the Camera Distance Factor to set<br>"
            "  your desired default 'Zoom' for Native rendered items<br>"
            "  and items displayed in the 3D viewer.<br><br>"
            "  Use the reset button (...) to restore the camera<br>"
            "  defaults.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_CAMERA_DEFAULTS")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_CHANGE_LOG
        {
            QObject::tr(
            "  The change log allows you to view %1 release change<br>"
            "  details stored online in the popular software<br>"
            "  development and integration website GitHub.<br><br>"
            "  Change log options are as follows:<br>"
            "  - Download change log at startup: you can set %1 to<br>"
            "    download the online instance of the change log. The<br>"
            "    online log may have updates since after the installed<br>"
            "    log which is loaded by default.<br>"
            "    Note that there may be a material impact on the<br>"
            "    application load time, depending on your internet<br>"
            "    connect performance, when this option is enabled.<br>"
            "  - Update Chnage Log: interctively trigger %1 to<br>"
            "    download the latest online change log.<br><br>"
            "  With an appropriate GitHub account, you can contribute<br>"
            "  or ask questions relating to changes displayed in the<br>"
            "  change log.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_CHANGE_LOG")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_DEFAULTS
        {
            QObject::tr(
            "  Configure %1 units, camera angles and LDraw file<br>"
            "  parse message defaults.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_DEFAULTS")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_EDITOR
        {
            QObject::tr(
            "  Configure the default publisher page attributes.<br>"
            "  - Default Author: set the instruction document author<br>"
            "  - Default URL: set the universal resource locator (URL).<br>"
            "  - Default Email: set the desired email address.<br><br>"
            "  These attributes are part of the instruction document<br>"
            "  page attributes.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_EDITOR")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_EXTENDED_SEARCH
        {
            QObject::tr(
            "  Extended search paths used during initial LDraw model file load.<br>"
            "  These paths are searched for parts, primitives, and textures<br>"
            "  encountered in your model file during the initial file load<br>"
            "  but are not in the model file or archive parts library.<br><br>"
            "  Select among the available options, the search paths<br>"
            "  to include in your extended model file search paths list.<br>"
            "  This list is accessed when the extended search paths<br>"
            "  setting is enabled.<br>"
            "  If no items are selected %1 will only search the path of<br>"
            "  the current subfile being loaded.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_EXTENDED_SEARCH")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_FADE_STEPS
        {
            QObject::tr(
            "  Configure %1 fade previous steps default settings.<br><br>"
            "  - Enable Fade Previous Steps: turn on or off this<br>"
            "    feature.<br><br>"
            "  - Use Parts Global Fade Color: turn on or off using the<br>"
            "    specified fade color for all faded items.<br>"
            "    Meta commands using LDraw colors will display the LDraw<br>"
            "    color safe name (spaces replaced with '_') versus a<br>"
            "    hex [A]RGB value as there are multiple LDraw colours<br>"
            "    that use the same hex [A]RGB value.<br><br>"
            "    You can also manually enter any color using the<br>"
            "    (#|0x)([AA]RRGGBB) hex [A]RGB value syntax.<br>"
            "    When this option is enabled, use the dropdown list<br>"
            "    to select your desired global fade color.<br><br>"
            "    When this option is not enabled, %1 faded parts<br>"
            "    will display their assigned part color faded to the<br>"
            "    specified opacity.<br><br>"
            "    When the LDraw LEGO parts library is in use, the<br>"
            "    global fade color list is loaded from the LDraw<br>"
            "    parts LDConfig file.<br><br>"
            "  - Fade Percent: sets the fade color transparency<br>"
            "    between 0 and 100 percent where 0 is fully<br>"
            "    transparent and 100 is fully opaque.<br>"
            "    This value is translated to the fade color<br>"
            "    opacity and alpha channel - for example, 60% fade<br>"
            "    is 40% opacity, 102/255 alpha channel and 66 hex.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_FADE_STEPS")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_GENERAL_SETTINGS
        {
            QObject::tr(
            "  Configure the following %1 general settings.<br><br>"
            "  - Load the last opened file on startup: as the title<br>"
            "    states, the last file opened in %1 is automatically<br>"
            "    loaded when %1 is launched.<br><br>"
            "  - Load the last displayed page: this setting will load<br>"
            "    the last opened page of the last opened file.<br>"
            "    This behaviour will occur on automatic application<br>"
            "    restart after an abnormal end, normal application<br>"
            "    restart after setting change or when load last opened<br>"
            "    file on startup is enabled.<br><br>"
            "  - Show load status: set %1 to display a load status<br>"
            "    dialog upon completion of LDraw file load.<br>"
            "    The load status dialog can be set to display on the<br>"
            "    following triggers:<br><br>"
            "    * Never: do not display the load status dialog.<br><br>"
            "    * On Error: display dialog when an error condition,<br>"
            "      for example, when an invalid (missing) part is<br>"
            "      encountered<br><br>"
            "    * On Warning: display dialog when a warning condition,<br>"
            "      for example, when a subpart or primitive part is<br>"
            "      encountered.<br><br>"
            "    * On Message: display dialog when either a warning or<br>"
            "      an error condition is encountered.<br><br>"
            "    * Always: display dialog each time an LDraw file is<br>"
            "      loaded.<br><br>"
            "  - Extended search paths: search the current folder,<br>"
            "    LDraw search directories and LDraw disc library<br>"
            "    subfolders for model subfile references.<br><br>"
            "  - Save on Update: save the current model file when<br>"
            "    the command editor update action is triggered.<br><br>"
            "  - Save on Redraw: save the current model file when<br>"
            "    the command editor redraw or application reset<br>"
            "    cache menu action is triggered.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_GENERAL_SETTINGS")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_HIGHLIGHT_STEP
        {
            QObject::tr(
            "  Configure %1 highlight current step default settings.<br><br>"
            "  - Enable Highlight Current Step: turn on or off this<br>"
            "    feature.<br><br>"
            "  - Highlight first step: turn on or off highlighting the<br>"
            "    first step in the in the instruction document.<br><br>"
            "  - Highlight Color: use the dropdown list<br>"
            "    select your desired highlight color using the Color<br>"
            "    picker.%2%3<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(MetaDefaults::getPreferredRenderer() == QLatin1String("LDGLite")
            ? QObject::tr("<br>"
            "  - Highlight line width: set the highlight line width<br>"
            "    using an integer number.<br>")
            : "")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_HIGHLIGHT_STEP")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_IMAGE_MATTE
        {
            QObject::tr(
            "  This feature is not curently available.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_IMAGE_MATTE")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_KEYBOARD_SHORTCUT
        {
            QObject::tr(
            "  You can configure a keyboard shortcut for the selected<br>"
            "  menu action.<br><br>"
            "  - Key sequence: enter your desired shortcut key<br>"
            "    sequence.<br>"
            "  - Assign: click this button to add our custom defined<br>"
            "    shortcut to the selected menu action.<br><br>"
            "  - Remove: You can remove an assigned custom shortcut using<br>"
            "    the Remove button.<br><br>"
            "  - Use the Reset button in the key sequence dialog to<br>"
            "    restore the default menu action keyboard shortuct.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_KEYBOARD_SHORTCUT")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_KEYBOARD_SHORTCUT_TABLE
        {
            QObject::tr(
            "  View and select %1 menu action keyboard shortcuts.<br><br>"
            "  - Table: all editable keyboard shortcuts are presented<br>"
            "    in the view table grouped by their parent dialog<br>"
            "    menu and action category.<br><br>"
            "  - Filter: filter the shortcut table using menu action<br>"
            "    name. The auto-complete functionality will<br>"
            "    provide suggestions based on the characters you<br>"
            "    enter.<br><br>"
            "  - Import: import custom menu action shortucts.<br><br>"
            "  - Export: export the entire list of shortcuts to<br>"
            "    an external file.<br><br>"
            "  - Reset: reload all menu actions and their default<br>"
            "    keyboard shortcut.<br><br>"
            "  Your shortcut changes are accepted when you click the<br>"
            "  Ok button, conversely, clicking the Cancel button will<br>"
            "  discard any custom keyboard shortcuts created during<br>"
            "  session.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_KEYBOARD_SHORTCUT_TABLE")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LDGLITE
        {
            QObject::tr(
            "  View the LDGLite renderer application path when this<br>"
            "  renderer is installed. The LDGLite renderer is packaged<br>"
            "  with %1 and is automatically installed.<br><br>"
            "  By default LDGLite and automatically configured<br>"
            "  at application startup using the %1 default<br>"
            "  options.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_LDGLITE")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LDGLITE_INSTALL
        {
            QObject::tr(
            "  In the event LDGLite is not installed or have been<br>"
            "  removed. %1 will display this dialog to download and<br>"
            "  install the LDGLite renderer.<br><br>"
            "  Use the Install button to initiate the download and<br>"
            "  installation.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_LDGLITE_INSTALL")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LDRAW_LIB_PATH
        {
            QObject::tr(
            "  Configure the path to the LDraw parts disc library.<br><br>"
            "  %1 primarilly uses the official and unofficial LDraw<br>"
            "  archive part libraries; however, as the LDView and<br>"
            "  LDGLite 3rd party renderers do not recognize the<br>"
            "  archive libraries, the traditional disc library is<br>"
            "  also required.<br><br>"
            "  You can use the Browse button to select the disc<br>"
            "  library or interactively enter the path in the line<br>"
            "  edit dialog.<br><br>"
            "  %1 will attempt to validate the specified LDraw parts<br>"
            "  library returning a 'Suspicious LDraw Directory message<br>"
            "  when the library path is deemed to be invalid.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_LDRAW_LIB_PATH")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LDVIEW
        {
            QObject::tr(
            "  Configure the LDView renderer settings.<br>"
            "  - Use Mulitple File Single Call: submit all the ldraw<br>"
            "    files (parts, assembly, submodel preview) for the<br>"
            "    current page in a single LDView render request<br>"
            "    instead of individually at each step.<br><br>"
            "  - Use Snapshots List File: this option extends and<br>"
            "    requires LDView Single Call whereby a list files<br>"
            "    for ldraw part or assembly files are created and<br>"
            "    submitted to LDView.<br><br>"
            "    As only one submodel preview per page is generated,<br>"
            "    these options are ignored for submodel preview images.<br>"
            "    Additionally, when there is less than 2 image files<br>"
            "    for a page's parts list or step group, these options<br>"
            "    are ignored.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_LDVIEW")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LDVIEW_INSTALL
        {
            QObject::tr(
            "  In the event LDView is not installed or have been<br>"
            "  removed. %1 will display this dialog to download and<br>"
            "  install the LDView renderer.<br><br>"
            "  Use the Install button to initiate the download and<br>"
            "  installation.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_LDVIEW_INSTALL")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LDVIEW_SETTINGS
        {
            QObject::tr(
            "  Configure LDView renderer general, gemometry, effects,<br>"
            "  primitives, updates and preference set settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_LDVIEW_SETTINGS")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LOGGING
        {
            QObject::tr(
            "  Configure %1 logging settings.<br>"
            "  - Path: you can configure the log file path and<br>"
            "    file name.<br>"
            "  - Line Attributes: select the log line attributes you<br>"
            "    wish to see for log entry lines.<br>"
            "  - Logging Level: set the logging level.<br>"
            "  - Individual Log Level Messages: you can select your<br>"
            "    desired log level messages.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_LOGGING")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LOG_LEVEL
        {
            QObject::tr(
            "  Select your desired %1 log messages.<br><br>"
            "  - STATUS: display status messages (default level).<br>"
            "  - INFO: log information messages.<br>"
            "  - TRACE: log trace messages.<br>"
            "  - DEBUG: log debug messages.<br>"
            "  - NOTICE: log notice messages.<br>"
            "  - ERROR: log error messages.<br>"
            "  - FATAL: log fatal messages.<br>"
            "  - OFF: do not log any messages.<br><br>"
            "  Levels at and below the selected level are written to<br>"
            "  the log file. For example, setting the level to DEBUG<br>"
            "  will write DEBUG, NOTICE, ERROR, and FATAL messages to<br>"
            "  the log file.<br><br>"
            "  When this setting is configured. Individual Log<br>"
            "  Level Messages settings are disabled.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_LOG_LEVEL")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LOG_LEVEL_MESSAGES
        {
            QObject::tr(
            "  You can configure your selection of individual log<br>"
            "  level messages that will be written to the log file.<br><br>"
            "  Select the log message(s) you wish to be written to<br>"
            "  the log file.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_LOG_LEVEL_MESSAGES")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LOG_LINE_ATTRIBUTES
        {
            QObject::tr(
            "  Configure the attributes you wish to see for log entry<br>"
            "  lines.<br><br>"
            "  - Logging Level: show log level, Fatal, Error, Status,<br>"
            "    Trace, Debug, and Information.<br><br>"
            "  - Timestamp: show the YYYY-MM-DDTH:M:S:MS timestamp.<br><br>"
            "  - Line Number: show the file line number '@ln NNN'.<br><br>"
            "  - Class Name: show the class name where the log entry<br>"
            "    was triggered.<br><br>"
            "  - Function: show the function where the log entry was<br>"
            "    triggered.<br><br>"
            "  - All attributes: show logging level, timestamp, line<br>"
            "    number, class name, and function.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_LOG_LINE_ATTRIBUTES")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LOG_PATH
        {
            QObject::tr(
            "  You can configure the log file path and file name.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_LOG_PATH")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_PRINT_STANDARD_OUT
        {
            QObject::tr(
            "  You can configure printing output messages to<br>"
            "  standard output<br>"
            "  To prevent double log entries, This setting is<br>"
            "  usually enabled on Windows and disabled on Unix<br>"
            "  when logging is enabled or when executing in<br>"
            "  console mode.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_PRINT_STANDARD_OUT")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_MESSAGES
        {
            QObject::tr(
            "  Error, warning and information messages are displayed<br>"
            "  here.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_MESSAGES")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_PAGE_PROCESSING_CONTINUOUS
        {
            QObject::tr(
            "  Configure %1 continuous page processing settings.<br><br>"
            "  - Display page for n seconds: set the duration to<br>"
            "    display each page when processing, or exporting<br>"
            "    instruction document pages.<br><br>"
            "  - Display options dialog: set and reset displaying<br>"
            "    the continuous page processing and export dialog.<br><br>"
            "    The continuous page processing and export dialog<br>"
            "    offers the option to turn off displaying it.<br>"
            "    When display is turned off, you can use this setting<br>"
            "    to reset (restore) the continuous page processing and<br>"
            "    export dialog display.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_PAGE_PROCESSING_CONTINUOUS")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_PARSE_ERROR
        {
            QObject::tr(
            "  Select and clear %1 file parse messages.<br><br>"
            "  When an LDraw file is being processed, encountered<br>"
            "  parse errors are displayed to give the editor an<br>"
            "  opportunity to perform correction.<br><br>"
            "  Parse error messages are grouped by the type of error.<br><br>"
            "  You can toggle on and off displaying each type of<br>"
            "  parse error message.<br><br>"
            "  When a message is displayed, the editor can elect to<br>"
            "  not display the message in the future.<br>"
            "  Messages set to not be displayed can be reset here by<br>"
            "  clicking the enabled 'Clear' button.<br>"
            "  You can clear individual message categories or clear<br>"
            "  all message categories with a single action.<br>"
            "  The 'Clear' button is enabled when there are messages<br>"
            "  to be cleared.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_PARSE_ERROR")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_PLI_CONTROL
        {
            QObject::tr(
            "  Configure the part list instance (PLI) control file.<br>"
            "  You can toggle on and off use of the PLI control file<br>"
            "  and you can also specify its path path and file name.<br><br>"
            "  The PLI control file is used to specify the default<br>"
            "  orientation and size of parts in the Parts List and<br>"
            "  Bill Of Material instances.<br><br>"
            "  You can add and edit entries in the PLI control file<br>"
            "  from the Configuration menu then Edit Parameter<br>"
            "  Files... menu and the PLI Parts Control File action.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_PLI_CONTROL")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_POVRAY
        {
            QObject::tr(
            "  Configure %1 POV-Ray renderer settings.<br>"
            "  - Quality: you can set render quality options High,<br>"
            "    Medium and Low.<br><br>"
            "  - Auto-crop Image: You can toggle on or off<br>"
            "    automatically cropping rendered images.<br><br>"
            "  - Display Image Progeress: show a progress dialog that<br>"
            "    presents the rendered image at its state of<br>"
            "    completeness.<br><br>"
            "  - Path: show the POV-Ray application path and file name.<br><br>"
            "  - POV-File Generation Renderer: select between Native or<br>"
            "    LDView.<br><br>"
            "  - POV File Generation Settings: configure POV generation<br>"
            "    options and LDView preferences to fine tune the POV-Ray<br>"
            "    rendered image.<br><br>"
            "  - LGEO Is Available: when the LEGO Geometrical Equivalent<br>"
            "    Objects (LGEO) library is available, this attribute is<br>"
            "    enabled and the library path is provided.<br>"
            "  - LGEO Stereolithography (STL) library is available: is<br>"
            "    displayed when the LGEO STL library from Darat is<br>"
            "    present.<br><br>"
            "  You can customize additional POV-Ray renderer options<br>"
            "  from its INI configuration file using the application<br>"
            "  menu action at Configuration, Edit Parameter Files...<br><br>"
            "  Note that some settings will be overridden by LPUB<br>"
            "  meta commands in your loaded model file.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_POVRAY")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_INSTALL
        {
            QObject::tr(
            "  In the event POV-Ray is not installed or have been<br>"
            "  removed. %1 will display this dialog to download and<br>"
            "  install the POV-Ray renderer.<br><br>"
            "  Use the Install button to initiate the download and<br>"
            "  installation.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_INSTALL")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_LDVIEW_POV_GENERATION
        {
            QObject::tr(
            "  You can choose between Native and LDView POV file<br>"
            "  generation.<br><br>"
            "  - Native: this is %1's integrated instance of the<br>"
            "    LDView POV file generation behaviour.<br> This option<br>"
            "    offers a performance benefit over using the LDView<br>"
            "    instance.<br><br>"
            "  - LDView: %1 will pass the POV generation parameters to<br>"
            "    the third party LDView instance. The LDView renderer<br>"
            "    will then generate the POV file.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_LDVIEW_POV_GENERATION")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_LGEO
        {
            QObject::tr(
            "  Configure the LEGO® Geometrical Equivalent Objects<br>"
            "  (LGEO) part library created by Lutz Uhlmann (El Lutzo),<br>"
            "  updated by Owen Burgoyne (C3POwen) and extended with<br>"
            "  Stereolithography STL object files by<br>"
            "  Damien Roux (Darats).<br><br>"
            "  - LGEO Is Available: when the LGEO part library is<br>"
            "    available, this attribute is enabled.<br><br>"
            "  - LGEO Stereolithography (STL) library is available<br>"
            "    label is displayed when the STL library from<br>"
            "    Damien Roux (Darats) is present.<br><br>"
            "  - Path: when the LGEO part library is detected,<br>"
            "    the library path is provided.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_LGEO")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_NATIVE_POV_GENERATION
        {
            QObject::tr(
            "  Configure the POV generation and LDView renderer<br>"
            "  preference settings for POV file generation.<br><br>"
            "  POV generation options are grouped into general,<br>"
            "  geometry, pov geometry, lighting, pov lights, material<br>"
            "  properties, transparent material properties,<br>"
            "  rubber material properties, and chrome material<br>"
            "  properties settings.<br><br>"
            "  Configure or reset the LDView renderer settings for<br>"
            "  POV file generation.<br> Settings are grouped into<br>"
            "  general, gemometry, effects, primitives,<br>"
            "  updates and preference set.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_NATIVE_POV_GENERATION")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_PUBLISH_DESCRIPTION
        {
            QObject::tr(
            "  Configure the default publisher description page<br>"
            "  attribute.<br><br>"
            "  - Default Publish Description: set the instruction<br>"
            "    document publish description. This value is<br>"
            "    automatically converted from the loaded LDraw<br>"
            "    document description when this information is<br>"
            "    available.<br>"
            "    Updates make here will, of course, override the<br>"
            "    automatically configured value.<br><br>"
            "  This attribute is part of the instruction document<br>"
            "  page attributes.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_PUBLISH_DESCRIPTION")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_PUBLISH_LOGO
        {
            QObject::tr(
            "  Configure the default publisher logo page attribute.<br><br>"
            "  - Default Document Logo: set the instruction document<br>"
            "    logo.<br><br>"
            "  This attribute is  part of the instruction document<br>"
            "  page attributes.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_PUBLISH_LOGO")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_RENDERERS_AND_PROJECTION
        {
            QObject::tr(
            "  Configure %1 preferred renderer, projection and<br>"
            "  image processing timeout settings:<br><br>"
            "  - Preferred Renderer:<br>"
            "    * POVRay: %1 ray tracing renderer offering feature<br>"
            "      rich, photo-realistic rendering ideal for document<br>"
            "      covers and final model or assembly displays.<br><br>"
            "    * LDGLite: %1 legacy renderer written in 'c' offering<br>"
            "      fast image rendering ideal for proofing.<br><br>"
            "    * LDView: This is a custom instance of LDView specially<br>"
            "      adapted for %1 using the latest code base.<br>"
            "      LDView offers high quality, rendering plus POV scene<br>"
            "      file generation.<br><br>"
            "    * Native: %1 'native' renderer and 3D viewer adapted<br>"
            "      from LeoCAD. The Native renderer is performant and<br>"
            "      provides 'true fade' rendering which enable powerful,<br>"
            "      high quality and complex model rendering options.<br><br>"
            "  - Projection:<br><br>"
            "    * Perspective: linear project where distant objects<br>"
            "      appear smaller.<br><br>"
            "    * Orthographic: parallel projection, where all<br>"
            "      projection lines are orthogonal to the projection<br>"
            "      plane.<br><br>"
            "  - Timeout: set the time in minutes before a render<br>"
            "    process will terminate if the process has not been<br>"
            "    completed by the renderer<br>"
            "    The default value is 6 minutes.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_RENDERERS_AND_PROJECTION")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_RENDERERS_SEARCH_PATHS
        {
            QObject::tr(
            "  Configure what to do with the LDraw MODELS, LSynth<br>"
            "  and Helper parts paths in reference to including<br>"
            "  these paths in the LDraw Content Search Directories.<br>"
            "  Note that, by default, both helper and LSynth parts<br>"
            "  are included in the %1 unofficial parts zip archive.<br><br>"
            "  - Exclude MODELS Path: Exclude MODELS folder path<br>"
            "    from the LDraw search parths list. This path is<br>"
            "    hard-coded to the root of the LDraw folder<br>"
            "    e.g. ./LDraw/models.<br>"
            "    It could be helpful performance-wise to uncheck<br>"
            "    this if you are not using the LDraw MODELS folder.<br><br>"
            "  - Add LSynth Path: Add LSynth folder path to LDraw search<br>"
            "    paths if not already included in search paths.<br>"
            "    This path is hard-coded the root of the unofficial folder<br>"
            "    e.g. ./LDraw/unofficial/LSynth.<br><br>"
            "  - Add Helper Path: Add Helper folder path to LDraw search<br>"
            "    paths. This path is hard-coded to the root of the<br>"
            "    unofficial folder e.g. ./LDraw/unofficial/helper.<br>"
            "    If you wish to export your custom helper parts with<br>"
            "    the renderers and/or Blender addon, then<br><br>"
            "    check this setting.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_RENDERERS_SEARCH_PATHS")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_RENDERERS_MESSAGE
        {
            QObject::tr(
            "  Renderer error, warning and information messages<br>"
            "  are displayed here.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_RENDERERS_MESSAGE")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_RENDERING
        {
            QObject::tr(
            "  Configure the Native renderer and 3DViewer default<br>"
            "  rendering settings.<br><br>"
            "  - Anti-aliasing: toggle on or off and select between<br>"
            "    2x, 4x, and 8x using the drop-down menu.<br><br>"
            "  - Edge Lines: toggle on or off edge lines and select<br>"
            "    edge line width using the slide bar.<br><br>"
            "  - Mesh LOD: toggle on or off the mesh level of detaul<br>"
            "    and select the LOD value between 0 and 1500 using the<br>"
            "    slide bar.<br><br>"
            "  - Shading Mode: select the shading mode from default<br>"
            "    lights, wireframe, or flat shading.<br><br>"
            "  - Stud Style: stud style decorate studs with one of<br>"
            "    seven (7) LEGO inscribed styles.<br><br>"
            "    High contrast stud styles paint stud cylinder<br>"
            "    and part edge colors.<br><br>"
            "    Available stud styles:<br>"
            "    ° 0 Plain<br>"
            "    ° 1 Thin Line Logo<br>"
            "    ° 2 Outline Logo<br>"
            "    ° 3 Sharp Top Logo<br>"
            "    ° 4 Rounded Top Logo<br>"
            "    ° 5 Flattened Logo<br>"
            "    ° 6 High Contrast<br>"
            "    ° 7 High Contrast With Logo<br><br>"
            "    High Contrast Stud And Edge Color Setttings allow<br>"
            "    you to configure part edge and stud cylinder<br>"
            "    color settings.<br><br>"
            "  - LPub Fade and Highlight: toggle on or off rendering<br>"
            "    fade and highlight parts using %1 behaviour.<br>"
            "    When enabled, the back faces of faded parts will<br>"
            "    not be drawn. Also, when enabled, Highlight New<br>"
            "    Parts and Fade Previous Steps are disabled.<br><br>"
            "  - Draw Conditional Lines: toggle on or off drawing<br>"
            "    conditional lines.<br><br>"
            "  - Fade Parts: toggle on or off fade parts using the<br>"
            "    Visual Editor/Native Renderer fade behaviour.<br><br>"
            "  - Highlight Parts: toggle on or off highlight parts<br>"
            "    using the Visual Editor/Native highlight behavoiur.<br><br>"
            "    When Fade or Highlight Parts is enabled, LPub Fade<br>"
            "    and Highlight is disabled. Also, These settings are<br>"
            "    applied when exporting HTML steps.<br><br>"
            "  - Automate Edge Color: automatically adjust part edge<br>"
            "    colors based on the following configured settings:<br>"
            "    * Contrast: the amount of contrast.<br><br>"
            "    * Saturation: the amount of edge color tint<br>"
            "      or shade.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_RENDERING")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_SEARCH_DIRECTORIES
        {
            QObject::tr(
            "  View %1 LDraw content search directories.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_SEARCH_DIRECTORIES")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_THEME_SCENE_COLORS
        {
            QObject::tr(
            "  Configure and reset %1 scene colors.<br><br>"
            "  - Background: set or change the scene background color<br>"
            "    using the color picker.<br><br>"
            "  - Ruler: set or change the scene ruler color using the<br>"
            "    color picker.<br><br>"
            "  - Guide: set or change the scene guide bar color using<br>"
            "    the color picker.<br><br>"
            "  - Grid: set or change the scene grid color using the<br>"
            "    color picker.<br><br>"
            "  - Tracking: set or change the scene tracking bar color<br>"
            "    using the color picker.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_THEME_SCENE_COLORS")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_THEME_SELECT_AND_COLORS
        {
            QObject::tr(
            "  You can select the %1 theme from Default, Dark and,<br>"
            "  on Windows, System. Selecting System will set the %1<br>"
            "  theme to the operating system color palette when<br>"
            "  %1 starts. To override this behaviour, change the<br>"
            "  theme setting.<br><br>"
            "  Additionally, you can set and reset individual theme<br>"
            "  colors using the Theme Colors... button.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_THEME_SELECT_AND_COLORS")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_UNITS
        {
            QObject::tr(
            "  Select %1 default unit measurement from centimeters<br>"
            "  or inches.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_UNITS")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_UPDATES
        {
            QObject::tr(
            "  Configure %1 check for updates settings.<br>"
            "  - Frequency: select from never, once a day, once a week<br>"
            "    or once a month.<br><br>"
            "  - Show Progress Details: toggle on or off showing<br>"
            "    progress details while checking for updates.<br><br>"
            "  - Show Update Available: toggle on or off automatic<br>"
            "    notification when an update is available.<br><br>"
            "  - Version Installed: the version installed dialog<br>"
            "    will automatically display the currently installed<br>"
            "    version of %1.<br><br>"
            "  - Enable Downloader: download the available update<br>"
            "    directly instead of opening the download link<br>"
            "    through a browser.<br><br>"
            "  - Show Redirects: show the download URL redirect accept<br>"
            "    message prompt when redirected.<br><br>"
            "  - Check For Updates...: interactively trigger an update<br>"
            "    check.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_UPDATES")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_VIEWPOINT_AND_PROJECTION
        {
            QObject::tr(
            "  Configure the Native renderer and 3DViewer default<br>"
            "  viewpoint, projection and zoom settings.<br><br>"
            "  - Viewpoint: set the pre-defined camera angle latitude<br>"
            "    and longitude settings which offer the following<br>"
            "    options in degrees:<br>"
            "    * Front: set the camera view at 0 lat, 0 lon.<br>"
            "    * Back: set the camera view at 0 lat, 180 lon.<br>"
            "    * Top: set the camera view at 90 lat, 0 lon.<br>"
            "    * Bottom: set the camera view at -90 lat, 0 lon.<br>"
            "    * Left: set the camera view at 0 lat, 90 lon.<br>"
            "    * Right: set the camera view at 0 lat, -90 lon.<br>"
            "    * Home: set the camera view at 30 lat, 45 lon.<br>"
            "    * Latitude/Loingitude: when set here, the camera<br>"
            "      view will be set to 23 lat, 45 lon.<br>"
            "    * Default: do not set a camera view, instead<br>"
            "      use the camera projection setting.<br>"
            "    Note that setting the camera viewpoint will override<br>"
            "    the camera projection setting.<br><br>"
            "  - Projection: set the default projection from:<br><br>"
            "    * Perspective: linear project where distant objects<br>"
            "      appear smaller.<br>"
            "    * Orthographic: parallel projection, where all<br>"
            "      projection lines are orthogonal to the projection<br>"
            "      plane.<br>"
            "    * Default: do not apply camera projection, instead<br>"
            "      use the camera viewpoint setting.<br><br>"
            "    Note that when the projection setting in the<br>"
            "    Preferred Renderer dialog is changed, this setting<br>"
            "    is also automatically updated to the new setting.<br><br>"
            "    Also note that when the camera viewpoint is set,<br>"
            "    the camera projection is automatically set to<br>"
            "    default and vice versa.<br><br>"
            "  - Zoom Extents: set the automatic camera zoom to<br>"
            "    optimize the current image in the view port.<br>"
            "    You can select from the following options:<br>"
            "    * Default: do not apply zoom extents.<br>"
            "    * On Orthographic: automatically apply zoom extents<br>"
            "      when camera projection mode is orthographic.<br>"
            "    * Always: always apply zoom extents.<br><br>"
            "  Settings configured here will be overridden by meta<br>"
            "  commands with the same settings in loaded LDraw file.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LPUB3D_PREFERENCES_VIEWPOINT_AND_PROJECTION")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_APPLICATION
        {
            QObject::tr(
            "  View the 3D viewer renderer color theme.<br>"
            "  This setting is read only and is configured from the<br>"
            "  %1 theme selection.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_APPLICATION")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_AUTHOR
        {
            QObject::tr(
            "  View the %1 default author.<br>"
            "  This setting is read only and is configured from the<br>"
            "  %1 publish author setting.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_AUTHOR")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_BACKGROUND_COLOR
        {
            QObject::tr(
            "  Configure the 3D viewer background color using<br>"
            "  the color picker.<br><br>"
            "  You can select between a solid color or color gradient.<br>"
            "  For color gradient, you can configure the top and<br>"
            "  bottom colors.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_BACKGROUND_COLOR")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_BASE_GRID
        {
            QObject::tr(
            "  Configure the 3D viewer base grid attributes.<br>"
            "  - Draw Studs: toggle on and off drawing studs on the<br>"
            "    base grid.<br>"
            "  - Draw Lines Every n Studs: toggle on or off drawing<br>"
            "    grid lines evey n studs, by default n is 5 studs.<br>"
            "  - Draw Origin Lines: draw X and Y axis guidelines that<br>"
            "    intersect at the selected part or assembly origin.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_BASE_GRID")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_BASE_GRID_COLOR
        {
            QObject::tr(
            "  Configure the 3D viewer base grid line and stud color<br>"
            "  using the color picker.<br>"
            "  - Lines: grid line color.<br>"
            "  - Studs: drawn studs color.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_BASE_GRID_COLOR")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_CATEGORY
        {
            QObject::tr(
            "  View, import, export or reset LDraw part categories.<br>"
            "  - Categories: Part categories are taken from the<br>"
            "    part description.<br>"
            "    You can select a part category in the category column<br>"
            "    and the all parts under this category will displayin<br>"
            "    the right column. Parts are displayed by their name<br>"
            "    number attributes.<br><br>"
            "  You can perform the following actions on<br>"
            "  part categories.<br><br>"
            "  - Import: import custom part categories.<br><br>"
            "  - Export: export the entire list of part categories to<br>"
            "    an external file.<br><br>"
            "  - Reset: reload all part categories and their default<br>"
            "    definitions.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_CATEGORY")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_CATEGORY_EDIT
        {
            QObject::tr(
            "  Create, edit or delete LDraw part categories.<br>"
            "  - New: create a new part category.<br>"
            "  - Edit: edit the selected part category.<br>"
            "  - Delete: remove the selected part category.<br>"
            "    Edited and removed part categories can b restored<br>"
            "    using the Reset button in the Parts Library<br>"
            "    Categories dialog.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_CATEGORY_EDIT")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_GENERAL_SETTINGS
        {
            QObject::tr(
            "  Configure the 3D viewer and Native renderer general<br>"
            "  settings.<br><br>"
            "  - Prompt On Redraw LPub Document Page: toggle on or off<br>"
            "    a display message asking to redraw the current page<br>"
            "    when a preference setting that affects the rendered<br>"
            "    image is changed.<br><br>"
            "  - Prefer Official Parts: toggle on or off ignoring<br>"
            "    unofficial LDraw parts that also appear in the<br>"
            "    official parts archive library.<br><br>"
            "    There may be cases where you may wish to use an<br>"
            "    'altered' instance of an official part in which<br>"
            "    case this setting can be toggled off to enable<br>"
            "    this behaviour.<br><br>"
            "    Keep in mind custom parts, or any unofficial part,<br>"
            "    exposed to %1 using LDraw Search Directories, will<br>"
            "    ultimately be added to the LDraw unofficial parts<br>"
            "    archive library.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_GENERAL_SETTINGS")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_INTERFACE
        {
            QObject::tr(
            "  Configure the 3D viewer XYZ axis icon settings and<br>"
            "  select the axis icon location within the 3D viewer<br>"
            "  viewport.<br><br>"
            "  - Axis Icon: toggle on or off the axis icon display.<br>"
            "  - Location: XYZ axis icon location options<br>"
            "    ° Top Left<br>"
            "    ° Top Rightn<br>"
            "    ° Bottom Left<br>"
            "    ° Bottom Right%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_INTERFACE")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_INTERFACE_COLOR
        {
            QObject::tr(
            "  Configure the 3D viewer interface object colors<br>"
            "  using the color picker.<br><br>"
            "  - Axis Labels: axis icon X,Y,Z labels.<br>"
            "  - Overlay Color: object overlay.<br>"
            "  - Marquee Border: selection area border.<br>"
            "  - Marquee Fill: selection area fill.<br>"
            "  - Active View: active 3D viewwer viewport border.<br>"
            "  - Inactive View: inactive 3D viewwer viewport border.<br>"
            "  - Camera Name: custom camera label.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_INTERFACE_COLOR")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_KEYBOARD_SHORTCUTS_TABLE
        {
            QObject::tr(
            "  This setting is not available.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_KEYBOARD_SHORTCUTS_TABLE")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_MOUSE
        {
            QObject::tr(
            "  Set the 3D viewer mouse sensitivity when performing<br>"
            "  tasks such as dragging and item selection.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_MOUSE")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_MOUSE_SHORTCUTS
        {
            QObject::tr(
            "  This setting is not available.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_MOUSE_SHORTCUTS")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_MOUSE_SHORTCUTS_TABLE
        {
            QObject::tr(
            "  This setting is not available.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_MOUSE_SHORTCUTS_TABLE")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_OBJECTS_COLOR
        {
            QObject::tr(
            "  Configure the 3D viewer model object colors.<br>"
            "  using the color picker.<br><br>"
            "  - Focused: clicken on item color.<br>"
            "  - Selected: selection item color.<br>"
            "  - Camera: drawn camera color.<br>"
            "  - Control Point: flexible parts control point color.<br>"
            "  - Control Point Focused: clicked on flexible parts<br>"
            "    control point color.<br>"
            "  - Build Mod Selected: selected item when Build<br>"
            "    Modifications is enabled color.<br>"
            "  - Light: drawn light color.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_OBJECTS_COLOR")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_PART_PREVIEW
        {
            QObject::tr(
            "  Configure the 3D viewer part preview settings.<br><br>"
            "  - Preview Size: size options<br>"
            "    * Docked: the part preview window is docked as a tab<br>"
            "      in the 3D viewer.<br>"
            "    * Small: 300 pixels.<br>"
            "    * Medium: 400 pixels.<br>"
            "  - Location: location options<br>"
            "    ° Top Right<br>"
            "    ° Tob Left<br>"
            "    ° Bottom Right<br>"
            "    ° Bottom Left<br><br>"
            "  - Position: position options<br>"
            "    * Floating: the part preview windows is floating.<br>"
            "    * Docked: the part preview window is docked as a tab<br>"
            "      in the 3D viewer.<br><br>"
            "  - View Sphere Size: size options<br>"
            "    * Disabled: the view sphere is not dislayed.<br>"
            "    * Small: 50 pixels.<br>"
            "    * Medium: 100 pixels.<br>"
            "    * Large: 200 pixels.<br><br>"
            "  - View Sphere Location: location options<br>"
            "    ° Top Right<br>"
            "    ° Tob Left<br>"
            "    ° Bottom Right<br>"
            "    ° Bottom Left<br><br>"
            "  - Axis Icon: toggle on or off the XYZ axis icon display.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_PART_PREVIEW")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_PATHS
        {
            QObject::tr(
            "  View the 3D viewer and Native renderer paths.<br>"
            "  These setting are read only and are configured from<br>"
            "  %1.<br><br>"
            "  - Parts Library: the official LDraw archive parts<br>"
            "    library.<br>"
            "    This location should also contain the unofficial<br>"
            "    LDraw archive parts library.<br>"
            "  - Color Configuration: when an alternate LDConfig<br>"
            "    LDraw color configuration file is specified in %1<br>"
            "    Preferences, that file path is reflected here.<br>"
            "  - POV-Ray Executable: here we have the loction of the<br>"
            "    %1 POV-Ray renderer.<br>"
            "  - LGEO Library: when the LEGO Geometrical Equivalent<br>"
            "    Objects (LGEO) library is available, the library path<br>"
            "    is specified here.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_PATHS")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_SHORTCUT
        {
            QObject::tr(
            "  This setting is not available.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_SHORTCUT")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_TIMELINE
        {
            QObject::tr(
            "  Configure the 3D viewer timeline settings.<br>"
            "  - Show Timeline Piece Icons: display timeline icon as<br>"
            "    a scaled image of the timeline part or assembly.<br><br>"
            "  - Show Timeline Color Icons: display timeline icon as<br>"
            "    a circular image colored the same as the part<br>"
            "    represents. Assemblies are assigned the LDraw default<br>"
            "    color.<br><br>"
            "    By default, timeline icons is set to Show Timeline<br>"
            "    Color Icons. This setting allows you to set<br>"
            "    Show Timeline Piece Icons.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_TIMELINE")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_VIEW_SPHERE
        {
            QObject::tr(
            "  Configure the 3D viewer view sphere size and location.<br><br>"
            "  - Size: size options<br>"
            "    * Disabled: the view sphere is not dislayed.<br>"
            "    * Small: 50 pixels.<br>"
            "    * Medium: 100 pixels.<br>"
            "    * Large: 200 pixels.<br><br>"
            "  - Location: location options<br>"
            "    ° Top Right<br>"
            "    ° Tob Left<br>"
            "    ° Bottom Right<br>"
            "    ° Bottom Left%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_VIEW_SPHERE")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_VISUAL_PREFERENCES_VIEW_SPHERE_COLOR
        {
            QObject::tr(
            "  Configure the 3D viewer view sphere colors<br>"
            "  using the color picker.<br><br>"
            "  - Color: view sphere background color.<br>"
            "  - Text Color: view plane text color.<br>"
            "  - Highlight Color: view plane hover highlight color.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_VISUAL_PREFERENCES_VIEW_SPHERE_COLOR")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_ANTIALIASING
        {
            QObject::tr(
            "  Configure LDView antialiasing settings.<br><br>"
            "  - FSAA: This allows you to select what level of<br>"
            "    full-scene antialiasing to use.<br>"
            "    Note that if your video card does<br>"
            "    not support full-scene antialiasing, then None will<br>"
            "    be the only available item.<br>"
            "    If your video card does support full-scene<br>"
            "    antialiasing, then all available FSAA modes should be<br>"
            "    available to choose from. If you have an nVIDIA®<br>"
            "    video card with Quincunx support, then 2x Enhanced<br>"
            "    and 4x Enhanced should show up as options. These use<br>"
            "    nVIDIA's advanced FSAA filter.<br><br>"
            "  - Antialiased lines: Enabling this option causes lines<br>"
            "    to be drawn using antialiasing. This does not effect<br>"
            "    wireframe mode. Line antialiasing can significantly<br>"
            "    improve the visual quality of lines, but it can also<br>"
            "    significantly impact the frame rate.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_ANTIALIASING")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_COLORS
        {
            QObject::tr(
            "  Configure LDView color settings.<br><br>"
            "  - Background: This allows you to set the color used to<br>"
            "    draw the background behind the model.<br><br>"
            "  - Default Color: This allows you to set the default<br>"
            "    color.<br>"
            "    Note that edge lines on default-colored objects will<br>"
            "    be black.<br>"
            "    Resetting the General preferences also resets these<br>"
            "    colors.<br><br>"
            "  - Transparent: Enabling this option causes<br>"
            "    default-colored items to be transparent.<br><br>"
            "  - Random colors: Enabling this option causes the colors<br>"
            "    used in the model to be randomized.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_COLORS")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_CUSTOM_LDCONFIG
        {
            QObject::tr(
            "  This text field allows you to enter or paste the full path to<br>"
            "  an LDraw file that you want to use for custom config. It is<br>"
            "  expected that a custom config file would normally contain<br>"
            "  custom color definitions that get loaded after the standard<br>"
            "  ones in LDConfig.ldr (if that is enabled). It can thus override<br>"
            "  any colors that are defined in LDConfig.ldr, as well as define<br>"
            "  new colors.<br>"
            "  The file can contain any valid LDraw statements, including<br>"
            "  geometry, but any geometry specified by this file will be drawn<br>"
            "  in addition to any file that is loaded. The geometry will not<br>"
            "  be included in model size calculations, so it may be cropped.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_CUSTOM_LDCONFIG")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_INI_FILE
        {
            QObject::tr(
            "  View the LDView INI configuration file and preference<br>"
            "  set specified for the current function. Functions<br>"
            "  include, step image rendering, POV file generation<br>"
            "  and HTML parts list export.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_INI_FILE")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_MISC
        {
            QObject::tr(
            "  Configure LDView general miscellaneous settings.<br><br>"
            "  - Auto-crop rendered image: Enabling this option will<br>"
            "    automatically remove unnecessary negative space<br>"
            "    from the rendered image.<br><br>"
            "  - Rendered image background transparent: Enabling this<br>"
            "    option causes the rendered image background to be<br>"
            "    transparent.<br><br>"
            "  - Don't use pixel buffer: Enabling this option will not<br>"
            "    use the pixel buffer when rendering images.<br>"
            "    This setting is only available on Windows.<br><br>"
            "  - Field of view: Enter a number in this field to<br>"
            "    control the field of view (in degrees).<br>"
            "    Setting this to the minimum value of 0.1 will produce<br>"
            "    an image that is essentially orthographic.<br>"
            "    Setting it to the maximum value of 90 will produce a<br>"
            "    wide-angle view.<br><br>"
            "  - Memory usage: This allows you to have some control<br>"
            "    over how much memory LDView uses. Normally, setting<br>"
            "    this to High will result in the best performance.<br>"
            "    However, if you load a large model that uses more<br>"
            "    memory than you have available, setting this to<br>"
            "    Medium or Low may improve performance.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_MISC")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_SAVE_DIRECTORIES
        {
            QObject::tr(
            "  The default save directories box allows you to decide<br>"
            "  what directory to use when saving files. Each save<br>"
            "  operation has three possible options:<br><br>"
            "  - Model: The default directory is the same directory<br>"
            "    as the currently loaded model.<br>"
            "  - Last Saved: The default directory is whatever<br>"
            "    directory was used last for this file type. (The<br>"
            "    first time you save a file of a given type, the<br>"
            "    default directory is the model's directory.)<br><br>"
            "  Save directory can be set for the following:<br>"
            "  - Renderings: LDView rendered images.<br>"
            "  - Part Lists: HTML generated part list.<br>"
            "  - POV Exports: POV file generation used by the POV-Ray<br>"
            "    renderer.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_SAVE_DIRECTORIES")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_MODEL
        {
            QObject::tr(
            "  Configure LDView model settings.<br><br>"
            "  - Seam width: Enabling this option allows you to set<br>"
            "    the width of the seams between parts using the spin<br>"
            "    box up and down arrows to the right. The field then<br>"
            "    displays the current seam width. Use the spin box<br>"
            "    arrows to set the seam width to a range of numbers<br>"
            "    between 0.00 and 5.00.<br><br>"
            "  - Part bounding boxes only: Enabling this option makes<br>"
            "    it so that all parts in the model are drawn as just<br>"
            "    bounding boxes. A bounding box is the minimum<br>"
            "    axis-aligned box that can completely contain the part.<br>"
            "    Geometry that isn't detected as being inside a part is<br>"
            "    drawn normally.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_MODEL")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_WIREFRAME
        {
            QObject::tr(
            "  Configure LDView wire frame settings.<br><br>"
            "  - Wireframe: Enabling this option causes the model to<br>"
            "    be drawn in wireframe. Note that with many 3D<br>"
            "    accelerators, this actually runs slower than when<br>"
            "    drawing as a solid.<br><br>"
            "  - Fog: This option is only enabled if wireframe drawing<br>"
            "    is enabled. Enabling it causes fog to be used to make<br>"
            "    it so that the lines that are farther away are drawn<br>"
            "    more dimly.<br><br>"
            "  - Remove hidden lines: This option is only available if<br>"
            "    wireframe drawing is enabled. Enabling it causes the<br>"
            "    solid (non-transparent) portions of the model to hide<br>"
            "    lines that are behind them, even though these portions<br>"
            "    themselves aren't actually drawn.<br><br>"
            "  - Thickness: This option is only available if wireframe<br>"
            "    drawing is enabled. It adjusts the thickness of the<br>"
            "    lines used to draw the model in wireframe mode.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_WIREFRAME")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_BFC
        {
            QObject::tr(
            "  Configure LDView Back-face Culling (BFC) settings.<br><br>"
            "  - BFC: Enabling this option turns on back-face culling.<br>"
            "    BFC is a technique that can improve rendering<br>"
            "    erformance by not drawing the back faces of polygons.<br>"
            "    (Only BFC-certified parts have their back faces<br>"
            "    removed.)<br><br>"
            "  - Red back faces: This option is only available while<br>"
            "    BFC is enabled. Enabling it causes the back faces of<br>"
            "    polygons to be drawn in red. This is mainly useful for<br>"
            "    part authors.<br><br>"
            "  - Green front faces: This option is only available while<br>"
            "    BFC is enabled. Enabling it causes the front faces of<br>"
            "    polygons to be drawn in green. This is mainly useful<br>"
            "    for part authors.<br><br>"
            "  - Blue neutral faces: This option is only available<br>"
            "    while BFC is enabled. Enabling it causes the faces of<br>"
            "    non BFC certified polygons to be drawn in blue. This<br>"
            "    is mainly useful for part authors.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_BFC")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_EDGE_LINES
        {
            QObject::tr(
            "  Configure LDView edge lines settings.<br><br>"
            "  - Edge Lines: Enabling this option causes the edge<br>"
            "    lines that outline all the pieces to be drawn.<br><br>"
            "  - Show edges only: This option is only available if<br>"
            "    edge lines are enabled. Enabling it causes LDView<br>"
            "    to only show edge lines. Unless you also select the<br>"
            "    Wireframe option, the portions of the edge lines that<br>"
            "    are hidden by the model will still be hidden, even<br>"
            "    thought the model isn't being shown.<br><br>"
            "  - Conditional lines: This option is only available if<br>"
            "    edge lines are enabled. Enabling it causes conditional<br>"
            "    edge lines to be computed and drawn. Conditional edge<br>"
            "    lines show up as a silhouette of curved surfaces; they<br>"
            "    are only drawn when they are visually at the edge of<br>"
            "    the surface from the current viewing angle.<br><br>"
            "  - Show all: This option is only available if edge lines<br>"
            "    and conditional lines are enabled. Enabling it causes<br>"
            "    all conditional edge lines to be drawn, whether or not<br>"
            "    they are on a silhouette.<br><br>"
            "  - Show control pts: This option is only available if<br>"
            "    edge lines and conditional lines are enabled.<br>"
            "    Enabling it causes lines to be drawn from the first<br>"
            "    point of each conditional line to each of its two<br>"
            "    control points.<br><br>"
            "  - High quality: This option is only available if edge<br>"
            "    lines are enabled. Enabling it causes edge lines to be<br>"
            "    drawn slightly closer to the eye than everything else,<br>"
            "    resulting in much better looking lines.<br><br>"
            "  - Always black: This option is only available if edge<br>"
            "    lines are enabled. Enabling it causes all edge lines<br>"
            "    to be drawn in black or dark gray.<br><br>"
            "    If Process LDConfig.ldr is checked in the General<br>"
            "    Preferences, LDView will look for a entries in<br>"
            "    LDConfig.ldr with color names of Black_Edge and<br>"
            "    Dark_Gray_Edge. If it finds either entry, it will use<br>"
            "    that color instead of black or dark gray.<br><br>"
            "  - Thickness: This option is only available if edge lines<br>"
            "    are enabled. It adjusts the thickness of the lines<br>"
            "    used to draw the edges around parts.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_EDGE_LINES")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_LIGHTING
        {
            QObject::tr(
            "  Configure LDView lighting settings.<br><br>"
            " - Lighting: Enabling this option enables lighting.<br>"
            "   If you disable lighting, it is strongly recommended<br>"
            "   that you enable edge lines, unless you are in<br>"
            "   wireframe mode.<br><br>"
            " - High quality: This option is only available if<br>"
            "   lighting is enabled. Selecting this enables high<br>"
            "   quality lighting. Enabling this can significantly<br>"
            "   decrease performance, especially with large models.<br><br>"
            " - Subdued: This option is only available if lighting is<br>"
            "   enabled. Selecting this sets the lighting to a subdued<br>"
            "   level. Subdued lighting is about halfway in between<br>"
            "   standard lighting and no lighting at all.<br><br>"
            " - Specular highlight: This option is only available if<br>"
            "   lighting is enabled. Selecting this turns on the<br>"
            "   specular highlight. This causes the model to appear<br>"
            "   shiny.<br><br>"
            " - Alternate setup: This option is only available if<br>"
            "   lighting is enabled. Selecting this causes LDView to<br>"
            "   use an alternate lighting setup, which may improve<br>"
            "   performance on some video cards. The alternate setup<br>"
            "   is used automatically when specular highlight<br>"
            "   is enabled.<br><br>"
            " - Light direction: Set the direction the light comes from<br>"
            "   to a pre-set angle.<br><br>"
            " - Use LIGHT.DAT lights: This option is only available if<br>"
            "   lighting is enabled. Selecting this causes LDView to<br>"
            "   put point light sources in the scene in the locations<br>"
            "   where LIGHT.DAT is used as a part.<br><br>"
            " - Replace standard light: This option is only available<br>"
            "   if lighting is enabled and Use LIGHT.DAT lights is<br>"
            "   checked. Selecting this causes LDView to not use its<br>"
            "   standard light source any time a model is loaded that<br>"
            "   contains one or more LIGHT.DAT parts. Models that<br>"
            "   don't contain any LIGHT.DAT parts will still be<br>"
            "   illuminated using LDView's standard light source.<br><br>"
            " - Hide LIGHT.DAT geometry: Selecting this causes LDView<br>"
            "   to not display the geometry from LIGHT.DAT parts.<br>"
            "   Since the geometry is a small set of 3D coordinate<br>"
            "   axes, you often don't want to see it.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_LIGHTING")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_STEREO
        {
            QObject::tr(
            "  Configure LDView stereo settings.<br><br>"
            " - Stereo: Enabling this option turns on stereo mode.<br>"
            "   Stereo mode draws two images, a slightly different one<br>"
            "   for each eye.<br><br>"
            " - Cross-eyed: This option is only available if stereo is<br>"
            "   enabled. Selecting this sets the stereo mode to<br>"
            "   cross-eyed stereo. The two images are drawn side by<br>"
            "   side, with the image for the right eye on the left and<br>"
            "   the image for the left eye on the right.<br><br>"
            " - Parallel: This option is only available if stereo is<br>"
            "   enabled. Selecting this sets the stereo mode to<br>"
            "   parallel stereo. The two images are drawn side by<br>"
            "   side, with the image for the right eye on the right<br>"
            "   and the image for the left eye on the left.<br><br>"
            " - Amount: This option is only available if stereo is<br>"
            "   enabled. The slider controls the stereo spacing.<br>"
            "   A small value will result in a very subtle stereoscopic<br>"
            "   effect, as if you were looking at a very large object<br>"
            "   from a very long distance. (The minimum value results<br>"
            "   in no stereoscopic effect at all.) A large value will<br>"
            "   result in a pronounced stereoscopic effect, as if you<br>"
            "   were looking at a small object from a very short<br>"
            "   distance.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_STEREO")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_WIREFRAME_CUTAWAY
        {
            QObject::tr(
            "  Configure LDView wireframe cutaway settings.<br><br>"
            " - Wireframe cutaway: Enabling this option causes the<br>"
            "   cutaway portion of the model to be drawn in<br>"
            "   wireframe.<br><br>"
            " - Color: This option is only available if wireframe<br>"
            "   cutaway is enabled. Selecting it causes the wireframe<br>"
            "   cutaway to be drawn in color.<br><br>"
            " - Monochrome: This option is only available if wireframe<br>"
            "   cutaway is enabled. Selecting it causes the wireframe<br>"
            "   cutaway to be drawn in monochrome.<br><br>"
            " - Opacity: This option is only available if wireframe<br>"
            "   cutaway is enabled. The slider controls the opacity<br>"
            "   of the wireframe cutaway.<br><br>"
            " - Thickness: This option is only available if wireframe<br>"
            "   cutaway is enabled. The slider controls the thickness<br>"
            "   of the lines in the wireframe cutaway.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_WIREFRAME_CUTAWAY")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_TRANSPARENCY
        {
            QObject::tr(
            "  Configure LDView transparency settings.<br><br>"
            " - Sort transparent polygons: Enabling this option causes<br>"
            "   transparent shapes to be drawn in back-to-front order<br>"
            "   in order to produce a correct result.<br>"
            "   This option is mutually exclusive with the use stipple<br>"
            "   pattern option.<br><br>"
            " - Use stipple pattern: Enabling this option causes<br>"
            "   transparent shapes to be drawn using a stipple<br>"
            "   (checkerboard) pattern.<br>"
            "   This option is mutually exclusive with the sort<br>"
            "   transparent polygons option.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_TRANSPARENCY")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_MISC
        {
            QObject::tr(
            "  Configure LDView effects miscellaneous settings.<br><br>"
            " - Flat shading: Enabling this option causes flat shading<br>"
            "   to be used. This option is mutually exclusive with the<br>"
            "   smooth curves option.<br><br>"
            " - Smooth curves: Enabling this option causes curved<br>"
            "   surfaces to be shaded as curved surfaces instead of<br>"
            "   as facets.<br>"
            "   This option is mutually exclusive with the flat<br>"
            "   shading option.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_MISC")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_PRIMITIVE_SUBSTITUTION
        {
            QObject::tr(
            "  Configure LDView primitive substitution settings.<br><br>"
            " - Primitive substitution: Enabling this option causes<br>"
            "   LDView to use its own optimized version of many of the<br>"
            "   primitives (such as cylinders, cones, etc). As well as<br>"
            "   providing enhanced performance, this also provides<br>"
            "   improved visual quality, since smooth shaded primitives<br>"
            "   are generated where appropriate.<br><br>"
            " - Curve quality: This option is only available if<br>"
            "   primitive substitution is enabled. It adjusts the<br>"
            "   quality of substituted primitives by changing the<br>"
            "   number of facets used to draw curved primitives.<br>"
            "   Changing this from its default value can lead to holes<br>"
            "   in the model between the substitute primitives and<br>"
            "   other geometry. Be very careful with this setting.<br>"
            "   Setting it too high can lead to huge slow-downs.<br>"
            "   The default setting of 2 is fine for most models.<br>"
            "   Note: Setting this to the minimum value will result<br>"
            "   in incorrect rendering of n-16 primitives.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_PRIMITIVE_SUBSTITUTION")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_PRIMITIVE_TEXTURES
        {
            QObject::tr(
            "  Configure LDView texture settings.<br><br>"
            " - Use texture mapping: Enabling this options causes<br>"
            "   LDView to display texture maps referenced in LDraw<br>"
            "   files.<br><br>"
            " - Offset: This option is only available if texture<br>"
            "   mapping is enabled. It controls how far textures are<br>"
            "   pulled toward the camera before being drawn.<br><br>"
            " - Texture studs: This option is only available if<br>"
            "   primitive substitution is enabled. Enabling it causes<br>"
            "   a LEGO® logo to be textured onto all the standard<br>"
            "   studs.<br><br>"
            " - Nearest filtering: This option is only available if<br>"
            "   primitive substitution and texture studs are both<br>"
            "   enabled. Selecting it causes nearest texel texture<br>"
            "   filtering to be used. This is the lowest quality<br>"
            "   texture filtering.<br><br>"
            " - Bilinear filtering: This option is only available if<br>"
            "   primitive substitution and texture studs are both<br>"
            "   enabled. Selecting it causes bilinear texture filtering<br>"
            "   to be used. This is the middle quality texture<br>"
            "   filtering.<br><br>"
            " - Trilinear filtering: This option is only available if<br>"
            "   primitive substitution and texture studs are both<br>"
            "   enabled. Selecting it causes trilinear texture<br>"
            "   filtering to be used. This is the highest quality<br>"
            "   texture filtering.<br><br>"
            " - Anisotropic filtering: This option is only available<br>"
            "   if primitive substitution and texture studs are both<br>"
            "   enabled, and your video card supports it. Selecting it<br>"
            "   causes anisotropic texture filtering to be used.<br>"
            "   This is the highest quality texture filtering.<br><br>"
            " - Anisotropic filtering level: This option is only<br>"
            "   available if primitive substitution and texture studs<br>"
            "   are both enabled, and anisotropic filtering is selected<br>"
            "   as the filtering type. It adjusts the anisotropic<br>"
            "   filtering level. Higher numbers produce higher quality<br>"
            "   filtering.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_PRIMITIVE_TEXTURES")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_PRIMITIVE_MISC
        {
            QObject::tr(
            "  Configure LDView primitives miscellaneous settings.<br><br>"
            "  - Stud Style: stud style decorate studs with one of<br>"
            "    seven (7) LEGO inscribed styles.<br><br>"
            "    High contrast stud styles paint stud cylinder<br>"
            "    and part edge colors.<br><br>"
            "    Available stud styles:<br>"
            "    ° 0 Plain<br>"
            "    ° 1 Thin Line Logo<br>"
            "    ° 2 Outline Logo<br>"
            "    ° 3 Sharp Top Logo<br>"
            "    ° 4 Rounded Top Logo<br>"
            "    ° 5 Flattened Logo<br>"
            "    ° 6 High Contrast<br>"
            "    ° 7 High Contrast With Logo<br><br>"
            "    High Contrast Stud And Edge Color Setttings allow<br>"
            "    you to configure part edge and stud cylinder<br>"
            "    color settings.<br><br>"
            "  - Automate Edge Color: automatically adjust part edge<br>"
            "    colors based on the following configured settings:<br>"
            "    * Contrast: the amount of contrast.<br><br>"
            "    * Saturation: the amount of edge color tint<br>"
            "      or shade.<br>"
            " - Low quality studs: Enabling this option switches to<br>"
            "   using the low-resolution stud files instead of the<br>"
            "   normal-resolution stud files. This can produce<br>"
            "   significantly faster rendering.<br><br>"
            " - Use hi-res primitives when available: Enabling this<br>"
            "   option causes LDView to attempt to load hi-res versions<br>"
            "   of primitives before loading the standard ones. The<br>"
            "   hi-res primitives are otherwise known as 48-sided<br>"
            "   primitives, because they approximate circular features<br>"
            "   with 48-sided features. Since not all primitives have<br>"
            "   hi-res versions, enabling this can create gaps between<br>"
            "   the standard resolution primitives and the hi-res ones.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_PRIMITIVE_MISC")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_UPDATES_PROXY
        {
            QObject::tr(
            "  Configure LDView internet proxy settings.<br><br>"
            "  - Direct connection to the Internet: Select this if you<br>"
            "    have a direct connection to the Internet and don't want<br>"
            "    to use a proxy server.<br><br>"
            "  - Use simple Windows proxy configuration: Select this if<br>"
            "    you want to use the HTTP proxy setting that you<br>"
            "    configured in Windows, Settings, Network & Internet,<br>"
            "    Proxy.<br>"
            "    This only works if you have a proxy server manually<br>"
            "    configured. Automatic proxy configurations are not<br>"
            "    supported.<br>"
            "    This option is only available in the Windows version<br>"
            "    of LDView.<br><br>"
            "  - Manual proxy configuration: Select this to manually<br>"
            "    configure your HTTP proxy server in the space provided<br>"
            "    below.<br><br>"
            "  - Proxy: Set this to the name or IP address of your HTTP<br>"
            "    proxy server. This can only be entered if Manual proxy<br>"
            "    configuration is selected above.<br><br>"
            "  - Port: Set this to the port of your HTTP proxy server.<br>"
            "    This can only be entered if Manual proxy configuration<br>"
            "    is selected above.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_UPDATES_PROXY")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_UPDATES_MISSING_PART
        {
            QObject::tr(
            "  Configure LDView internet proxy, missing parts and<br>"
            "  check for LDraw library updates settings.<br><br>"
            "  - Automatically check LDraw.org for missing parts: <br>"
            "    Enabling this causes LDView to automatically check the<br>"
            "    LDraw.org Parts Tracker when it can't find a file. If<br>"
            "    the part exists on the Parts Tracker, it will<br>"
            "    automatically be downloaded and used.<br><br>"
            "    If LDView fails to connect to the LDraw.org server<br>"
            "    while checking for an unofficial part, it will<br>"
            "    automatically disable this setting. You have to<br>"
            "    manually re-enable it once you have a working Internet<br>"
            "    connection again.<br><br>"
            "  - Days between missing part checks: Set this to the<br>"
            "    number of days you want LDView to wait between checks<br>"
            "    to the LDraw.org Parts Tracker when a part cannot be<br>"
            "    found. This number must be greater than zero.<br><br>"
            "  - Days between updated part checks: Set this to the<br>"
            "    number of days you want LDView to wait between checks<br>"
            "    for updates to an unofficial part that it has<br>"
            "    successfully downloaded from the LDraw.org Parts<br>"
            "    Tracker. Since unofficial parts can update on the<br>"
            "    Parts Tracker, LDView will check for updates with the<br>"
            "    specified frequency when it encounters unofficial parts<br>"
            "    in a model. This number must be greater than zero.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_UPDATES_MISSING_PART")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_UPDATES_LDRAW_SEARCH_DIRECTORIES
        {
            QObject::tr(
            "  Allows you to specify and organize any number of extra<br>"
            "  LDraw parts search directories. When LDView loads a model,<br>"
            "  and the model references a part or sub-model, it searches<br>"
            "  for that part or sub-model in a number of locations. No<br>"
            "  matter what you enter here, it will search the directory<br>"
            "  the model is located in, as well as the P, PARTS, and<br>"
            "  MODELS directories inside the LDraw directory (in that<br>"
            "  order). If it doesn't find the file in any of those<br>"
            "  directories, it will then search all the directories<br>"
            "  listed in the Extra Search Dirs dialog accessed via this<br>"
            "  menu item. It searches them in the order they are listed<br>"
            "  in the box, so you can move directories in the box up and<br>"
            "  down to get them in the order you want them to be in."
            "  - Add button: Add a search directory to the list.<br><br>"
            "  - Delete button: Remove a search directory from the list.<br><br>"
            "  - Up button: Move a search directory up in the list.<br><br>"
            "  - Down button: Move a search directory down in the list.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_UPDATES_LDRAW_SEARCH_DIRECTORIES")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_UPDATES_LDRAW_LIBRARY
        {
            QObject::tr(
            "  Checks to see if there are any LDraw parts library updates on<br>"
            "  LDraw.org that aren't yet installed, and download and<br>"
            "  install them if any are found.<br><br>"
            "  - Browse... button: Allows you to launch the download and<br>"
            "    update dialog which shows the update progress.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_UPDATES_LDRAW_LIBRARY")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_PREFERENCE_SET
        {
            QObject::tr(
            "  Configure LDView missing parts settings.<br><br>"
            "  - Use the following preference set: This list shows all<br>"
            "    of the available preference sets. Selecting a set from<br>"
            "    this list will cause LDView to use the preferences<br>"
            "    stored in that set after you hit Apply.<br><br>"
            "    If you have made any changes to preferences on other<br>"
            "    tabs and select a new preference set, it will prompt<br>"
            "    you whether you want to apply those changes, abandon<br>"
            "    them, or cancel your selection.<br><br>"
            "  - Hot Key: This allows you to assign a hot key to the<br>"
            "    currently selected preference set. There are 10<br>"
            "    possible hot keys: Ctrl + Shift + 1 through<br>"
            "    Ctrl + Shift + 0. Once a hot key has been assigned to<br>"
            "    a preference set, you can use it when viewing a model<br>"
            "    to immediately switch to that preference set.<br><br>"
            "  - Delete: This deletes the currently selected preference<br>"
            "    set.<br><br>"
            "  - New: This creates a new preference set that will<br>"
            "    initially contain all the current settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_PREFERENCES_PREFERENCE_SET")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_HTML_PART_LIST_EXPORT_COLUMN_ORDER
        {
            QObject::tr(
            "  Configure LDView HTML part list column order settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_HTML_PART_LIST_EXPORT_COLUMN_ORDER")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_HTML_PART_LIST_EXPORT_DISPLAY_OPTIONS
        {
            QObject::tr(
            "  Configure LDView HTML part list display options<br>"
            "  settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_HTML_PART_LIST_EXPORT_DISPLAY_OPTIONS")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_HTML_PART_LIST_EXPORT_PART_REFERENCE
        {
            QObject::tr(
            "  Configure LDView HTML part list part reference<br>"
            "  settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_HTML_PART_LIST_EXPORT_PART_REFERENCE")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_CHROME_MATERIAL
        {
            QObject::tr(
            "  Configure LDView POV export chrome material settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_CHROME_MATERIAL")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_GENERAL
        {
            QObject::tr(
            "  Configure LDView POV export general settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_GENERAL")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_GEOMETRY
        {
            QObject::tr(
            "  Configure LDView POV export geometry settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_GEOMETRY")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_INI_FILE
        {
            QObject::tr(
            "  View the LDView INI configuration file and preference<br>"
            "  set specified for POV export. The INI file for POV<br>"
            "  generation is ldvExport.ini, the preference set for POV<br>"
            "  generation is POV-Ray Render.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_INI_FILE")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_LIGHTING
        {
            QObject::tr(
            "  Configure LDView POV export lighting settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_LIGHTING")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_MATERIAL
        {
            QObject::tr(
            "  Configure LDView POV export material settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_MATERIAL")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_POV_GEOMETRY
        {
            QObject::tr(
            "  Configure LDView POV export POV geometry settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_POV_GEOMETRY")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_POVRAY_LIGHTS
        {
            QObject::tr(
            "  Configure LDView POV export POVRay lights settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_POVRAY_LIGHTS")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_RUBBER_MATERIAL
        {
            QObject::tr(
            "  Configure LDView POV export rubber material settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_RUBBER_MATERIAL")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_SCROLL_AREA
        {
            QObject::tr(
            "  Configure POV file generation general, geometry,<br>"
            "  pov geometry, lighting, pov lights, material<br>"
            "  properties, transparent material properties,<br>"
            "  rubber material properties, and chrome material<br>"
            "  properties settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_SCROLL_AREA")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_TRANSPARENT_MATERIAL
        {
            QObject::tr(
            "  Configure LDView POV export transparent material<br>"
            "  settings.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_TRANSPARENT_MATERIAL")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_LOOK_AT_TARGET
        {
            QObject::tr(
            "  Configure the default target position.<br>"
            "  - Target: using integers set the the 'Look At'<br>"
            "    position using the x-axes, y-axes and z-axes.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_LOOK_AT_TARGET")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_SNIPPETS_ADD
        {
            QObject::tr(
            "  Use this button to add a command snippet.<br><br>"
            "  When the Add button is clicked, a new snippet line<br>"
            "  entry is created in the snippets table and the<br>"
            "  snippet editor is enabled allowing you to edit<br>"
            "  the commands that will make up this snippet.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_SNIPPETS_ADD")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_SNIPPETS_EDIT
        {
            QObject::tr(
            "  Add meta commands to your snippet as desired.<br><br>"
            "  To apply a snippet in the command editor, after typing<br>"
            "  the snippet shortcut - e.g. 'msb' for MULTI_STEP BEGIN,<br>"
            "  use 'CTRL+Space' to trigger the snippet insert dialog.<br>"
            "  When the desired snippet is highlighted, hit ENTER to<br>"
            "  insert the snippet.<br><br>"
            "  Use '$|' to indicate where the cursor will be<br>"
            "  positioned when the snippet is inserted into your<br>"
            "  instruction document.<br><br>"
            "  Any combination of meta commands can be added; however,<br>"
            "  it would not be wise to add command values that are<br>"
            "  subject to change. For example if you wish to add a<br>"
            "  command that defines the default page size, use the<br>"
            "  page size type (A4, Letter etc...) versus the width<br>"
            "  and height.<br> Perhaps a better example is the OFFSET<br>"
            "  command. Unless you are certain the offset value will<br>"
            "  remain constant across the snippet's use cases, it<br>"
            "  might be better to not include the offset command.<br><br>"
            "  Multi-line snippets are supported.<br><br>"
            "  The following buttons will complete your snippet edit:<br>"
            "  - Ok: accept all changes in the currnt session.<br><br>"
            "  - Cancel: discard all changes in the current session.<br><br>"
            "  User defined snippets are saved to an external file<br>"
            "  which is loaded at application startup.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_SNIPPETS_EDIT")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_SNIPPETS_LIST
        {
            QObject::tr(
            "  This table view lists the defined snippets currently<br>"
            "  loaded.<br><br>"
            "  There are two types of snippets, built-in and<br>"
            "  user-defined.<br><br>"
            "  Built-in snippets are read-only.<br>"
            "  This is to say, the Remove button is not enabled<br>"
            "  when you select a built-in snippet.<br><br>"
            "  User-defined snippets are created using the Add<br>"
            "  button which also enables the snippet editor.<br><br>"
            "  When a new snippet is created, you must edit the<br>"
            "  snippet trigger and description in this table.<br><br>"
            "  - Trigger: a short name that uniquely identifies<br>"
            "    the user-defined snippet.<br><br>"
            "  - Description: a concise description of the snippet.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_SNIPPETS_LIST")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_SNIPPETS_REMOVE
        {
            QObject::tr(
            "  Remove the selected user-defined command snippet.<br><br>"
            "  The remove button is disabled when a built-in command<br>"
            "  snippet is selected. You cannot remove built-in<br>"
            "  snippets.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_SNIPPETS_REMOVE")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_SUBSTITUTE_CURRENT_PART
        {
            QObject::tr(
            "  This dialogue displays the current part<br>"
            "  and its title description. These dialogues<br>"
            "  are read-only.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_SUBSTITUTE_CURRENT_PART")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_SUBSTITUTE_PRIMARY_ARGUMENTS
        {
            QObject::tr(
            "  This dialogue presents the main editable<br>"
            "  substitute part attributes.<br>"
            "  - Substitute: enter your desired substitute<br>"
            "    part or use the Lookup button to select a<br>"
            "    part from the part lookup dialogue.<br>"
            "    The substitute part title will display in<br>"
            "    Substitute Title.<br><br>"
            "  - Color: select your desired part color or<br>"
            "    use the Change button to select the part<br>"
            "    color using the color picker.<br><br>"
            "  - LDraw Part: if the substitute part is a<br>"
            "    generated part, you can optionally use this<br>"
            "    dialogue to display the appropriate LDraw part<br>"
            "    name and part title description.<br><br>"
            "    This option is useful when you wish to show<br>"
            "    a custom representation of the part in the<br>"
            "    part list.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_SUBSTITUTE_PRIMARY_ARGUMENTS")
#else
                 ""
#endif
                )
        },
        // WT_CONTROL_SUBSTITUTE_EXTENDED_ARGUMENTS
        {
            QObject::tr(
            "  This dialogue presents additional editable<br>"
            "  substitute part attributes.<br>"
            "  - Scale: resize the displayed substitute part.<br><br>"
            "  - Field of View: edit the camera FOV for the<br>"
            "    substitute part.<br><br>"
            "  - Camera Latitude: set the camera angle x-axis<br>"
            "    in degrees.<br><br>"
            "  - Camera Longitude: set the camera angle y-axis<br>"
            "    in degrees.<br><br>"
            "  - Target: using integers set the the 'Look At'<br>"
            "    position using the x-axes, y-axes and z-axes.<br><br>"
            "  - Rotation: rotate the substitute part using<br>"
            "    x-angle, y-angle and z-angle.<br>"
            "    These are the individual rotation angles for<br>"
            "    the different axes in degrees (-360 to 360).<br>"
            "  - Transform: set the step transformation using<br>"
            "    options - relative, absolute or additive.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_CONTROL_SUBSTITUTE_EXTENDED_ARGUMENTS")
#else
                 ""
#endif
                )
        },
        //*************************************
        //*  METAGUI ENTRIES
        //*************************************
        // WT_GUI_SIZE_AND_ORIENTATION
        {
            QObject::tr(
            "  Configure the page size and orientation settings.<br>"
            "  - Size: set the page size using standard sizes like<br>"
            "    A4, Letter etc... You can also specify the 'Custom'<br>"
            "    size options along with its corresponding height and<br>"
            "    width values in %1.<br><br>"
            "    The page size custom height and width unit values<br>"
            "    are determined by the resolution units specified<br>"
            "    in Project Global Setup.<br>"
            "    Dot unit options are dots per inch (DPI) or<br>"
            "    dots per centimetre (DPC)<br>"
            "  - Orientation: set the page orientation to<br>"
            "    portrait or landscape.%2<br>")
            .arg(MetaDefaults::getPreferredUnits())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_SIZE_AND_ORIENTATION")
#else
                 ""
#endif
                )
        },
        // WT_GUI_BACKGROUND
        {
            QObject::tr(
            "  Configure the background.<br>"
            "  - Background: set the background option from<br>"
            "    the drop-down control:<br>"
            "    - None (Transparent)<br>"
            "    - Solid Color<br>"
            "    - Gradient<br>"
            "    - Picture<br>"
            "    - Submodel Level Color.<br><br>"
            "  - Change: select the background color from the<br>"
            "    color picker if Solid Color option selected.<br><br>"
            "  - Browse: set or select the image path if background<br>"
            "    image option selected.<br><br>"
            "  - Image Fill: select stretch or tile of the front<br>"
            "    cover image.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_BACKGROUND")
#else
                 ""
#endif
                )
        },
        // WT_GUI_BACKGROUND_NO_IMAGE
        {
            QObject::tr(
            "  Configure the background.<br>"
            "  - Background: set the background option from the<br>"
            "    drop-down control:<br>"
            "    ° None (Transparent)<br>"
            "    ° Solid Color<br>"
            "    ° Gradient<br>"
            "    ° Submodel Level Color.<br><br>"
            "  - Change: select the background color from the<br>"
            "    color picker if Solid Color option selected.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_BACKGROUND_NO_IMAGE")
#else
                 ""
#endif
                )
        },
        // WT_GUI_BORDER
        {
            QObject::tr(
            "  Configure the border.<br>"
            "  - Border: set the border corner square, round or<br>"
            "    borderless.<br><br>"
            "  - Line: set the border line type.<br><br>"
            "  - Color: select the border line color.<br><br>"
            "  - Margins: set the border margins width and height.<br>"
            "    Enter margin values using a decimal number<br>"
            "    in %1.<br>"
            "    Dot units are defined in Project Global Setup.%2<br>")
            .arg(MetaDefaults::getPreferredUnits())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_BORDER")
#else
                 ""
#endif
                )
        },
        // WT_GUI_CAMERA_ANGLES
        {
            QObject::tr(
            "  Configure the camera angles.<br>"
            "  - Latitude: set the camera angle x-axis in degrees.<br><br>"
            "  - Longitude: set the camera angle y-axis in degrees.<br><br>"
            "  - Camera Viewpoint: pre-defined camera angle latitude<br>"
            "    and longitude settings which offer the following<br>"
            "    options in degrees:<br>"
            "    * Front: set the camera view at 0 lat, 0 lon.<br>"
            "    * Back: set the camera view at 0 lat, 180 lon.<br>"
            "    * Top: set the camera view at 90 lat, 0 lon.<br>"
            "    * Bottom: set the camera view at -90 lat, 0 lon.<br>"
            "    * Left: set the camera view at 0 lat, 90 lon.<br>"
            "    * Right: set the camera view at 0 lat, -90 lon.<br>"
            "    * Home: set the camera view at 30 lat, 45 lon.<br>"
            "      You can change the latitude and longitude angles<br>"
            "      for Home camera view.<br>"
            "    * Latitude/Loingitude: set the latitude and longitude<br>"
            "      angles. This setting is equivalent to selecting Use<br>"
            "      Latitude And Longitude Angles to allow Home camera<br>"
            "      view changes.<br>"
            "    * Default: do not use Camera Viewpoint.<br><br>"
            "  - Use Latitude And Longitude Angles: enable the<br>"
            "    Latitude and Longitude dialogues to allow Home<br>"
            "    camera viewpoint angle changes. This dialog is only<br>"
            "    enabled when camera viewpoint Home is selected.<br><br>"
            "    Camera angle value range is -360.0 to 360.0<br><br>"
            "    Precision is one decimal place.<br><br>"
            "    Selecting a Camera Viewpoint other that Default<br>"
            "    will disable the individual Latitude and Longitude<br>"
            "    camera angle dialogues.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_CAMERA_ANGLES")
#else
                 ""
#endif
                )
        },
        // WT_GUI_CAMERA_DEFAULT_DISTANCE_FACTOR
        {
            QObject::tr(
            "  Configure the camera default distance factor:<br>"
            "  - Default Distance Factor: set the distance<br>"
            "    factor using a decimal number.<br><br>"
            "    The default distance factor value is used to<br>"
            "    adjust the camera position nearer to or farther<br>"
            "    away from the position of the assembly or part.<br>"
            "    It is effectively a zoom capability intended to<br>"
            "    allow the editor to tune the image size as desired.<br><br>"
            "    The primary use case is to maintain the same size<br>"
            "    image when changing your renderer to Native and  you<br>"
            "    wish to maintain the image sizing from the previous<br>"
            "    renderer.<br><br>"
            "    This setting is ignored when the preferred renderer is<br>"
            "    not Native. The current renderer is %1.<br><br>"
            "    Camera distance factor value range is 1.0 to 100.0<br><br>"
            "    Precision is two decimal places.%2<br>")
            .arg(MetaDefaults::getPreferredRenderer())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_CAMERA_DEFAULT_DISTANCE_FACTOR")
#else
                 ""
#endif
                )
        },
        // WT_GUI_CAMERA_FIELD_OF_VIEW
        {
            QObject::tr(
            "  Configure the camera field of view:<br>"
            "  - Camera FOV: set the camera field of view in degrees.<br>"
            "    %1%2%3<br>")
            .arg(MetaDefaults::getPreferredRenderer() == QLatin1String("Native")
            ? QObject::tr("<br>"
            "  ° Configure the camera near plane.<br><br>"
            "  ° Configure the camera far z plane.<br>")
            : QObject::tr("<br>"
            "  ° Camera near and far z planes are disabled<br>"
            "    when the preferred renderer is not Native.<br>")
            , QObject::tr("<br>"
            "    The field of view value range is determined by the<br>"
            "    current preferred renderer which is %1.<br>"
            "    The current FOV range is %2 to %3<br>")
            .arg(MetaDefaults::getPreferredRenderer())
            .arg(MetaDefaults::getFOVMinRange())
            .arg(MetaDefaults::getFOVMaxRange()))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_CAMERA_FIELD_OF_VIEW")
#else
                 ""
#endif
                )
        },
        // WT_GUI_CAMERA_NEAR_PLANE
        {
            QObject::tr(
            "  Configure the camera near Z plane:<br>"
            "  - Camera Near Plane: set the camera near plane<br>"
            "    using a decimal number.<br><br>"
            "    The near Z plane dialog is only enabled for the<br>"
            "    Native preferred renderer. The current renderer<br>"
            "    is %1.%2<br>")
            .arg(MetaDefaults::getPreferredRenderer())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_CAMERA_NEAR_PLANE")
#else
                 ""
#endif
                )
        },
        // WT_GUI_CAMERA_FAR_PLANE
        {
            QObject::tr(
            "  Configure the camera far plane:<br>"
            "  - Camera Far Plane: set the camera far plane<br>"
            "    using a decimal number.<br><br>"
            "    The far Z plane dialog is only enabled for the<br>"
            "    Native preferred renderer. The current renderer<br>"
            "    is %1.%2<br>")
            .arg(MetaDefaults::getPreferredRenderer())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_CAMERA_FAR_PLANE")
#else
                 ""
#endif
                )
        },
        // WT_GUI_IMAGE_SCALE
        {
            QObject::tr(
            "  The following configuration setup are available<br>"
            "  for image:<br>"
            "  - Scale: set the image scale using a decimal<br>"
            "    number.<br><br>"
            "    The mininimum allowed spin box value is -10000.0<br>"
            "    and the maximum is 10000.0.<br>"
            "    Spin box values are incremented by 1 step unit.<br>"
            "    Precision is two decimal places%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_IMAGE_SCALE")
#else
                 ""
#endif
                )
        },
        // WT_GUI_UNITS_MARGIN
        {
            QObject::tr(
            "  Configure the margins.<br>"
            "  - L/R(Left/Right): set the margin width.<br><br>"
            "  - T/B(Top/Bottom): set the margin height.<br><br>"
            "  Enter margin values using a decimal number<br>"
            "  in %1.<br>"
            "  Dot units are defined in Project Global Setup.%2<br>")
            .arg(MetaDefaults::getPreferredUnits())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_UNITS_MARGIN")
#else
                 ""
#endif
                )
        },
        // WT_GUI_HEADER_HEIGHT
        {
            QObject::tr(
            "  Configure the page header height using the<br>"
            "  specified dot units.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_HEADER_HEIGHT")
#else
                 ""
#endif
                )
        },
        // WT_GUI_FOOTER_HEIGHT
        {
            QObject::tr(
            "  Configure the page footer height using the<br>"
            "  specified dot units.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_FOOTER_HEIGHT")
#else
                 ""
#endif
                )
        },
        // WT_GUI_POINTER_BORDER
        {
            QObject::tr(
            "  Configure the pointer border.<br>"
            "  - Type: set the pointer border line type.<br>"
            "  - Width: set the pointer border line width.<br><br>"
            "  - Color: set the pointer border line color<br>"
            "    using the color picker.<br>"
            "  Enter Width using a decimal number in %1.<br>"
            "  Dot units are defined in Project Global Setup.%2<br>")
            .arg(MetaDefaults::getPreferredUnits())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_POINTER_BORDER")
#else
                 ""
#endif
                )
        },
        // WT_GUI_POINTER_LINE
        {
            QObject::tr(
            "  Configure the pointer line.<br>"
            "  - Type: set the pointer line type.<br>"
            "  - Width: set the pointer line width.<br>"
            "  - Color: set the pointer line color<br>"
            "    using the color picker.<br>"
            "  - Tip Visibility: set checked to show the<br>"
            "    pointer tip.<br><br>"
            "  Enter Width using a decimal number in %1.<br>"
            "  Dot units are defined in Project Global Setup.%2<br>")
            .arg(MetaDefaults::getPreferredUnits())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_POINTER_LINE")
#else
                 ""
#endif
                )
        },
        // WT_GUI_POINTER_TIP
        {
            QObject::tr(
            "  Configure the pointer tip.<br>"
            "  - Tip Width: set the visible tip width.<br>"
            "  - Tip Height: set the visible tip height.<br><br>"
            "  Enter Width and Height using a decimal number<br>"
            "  in %1.<br>"
            "  Dot units are defined in Project Global Setup.%2<br>")
            .arg(MetaDefaults::getPreferredUnits())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_POINTER_TIP")
#else
                 ""
#endif
                )
        },
        // WT_GUI_PAGE_ATTRIBUTE_TEXT
        {
            QObject::tr(
            "  Configure the document text properties.<br>"
            "  - Section: select the available location to place<br>"
            "    the text. Available locations may be cover page<br>"
            "    front or cover page back, content page header<br>"
            "    or content page footer. The selection<br>"
            "    drop-down list will display availabe location(s)<br>"
            "    for the selected text attribute.<br><br>"
            "  - Placement: select where on the page to place<br>"
            "    the text string.<br><br>"
            "  - Font: select the text font.<br><br>"
            "  - Color: select the text font color.<br><br>"
            "  - Margins: set the text margin area width/height.<br>"
            "    Enter margin values using a decimal number<br>"
            "    in %1.<br>"
            "    Dot units are defined in Project Global Setup.<br><br>"
            "  - Content: enter or update the text.%2<br>")
            .arg(MetaDefaults::getPreferredUnits())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_PAGE_ATTRIBUTE_TEXT")
#else
                 ""
#endif
                )
        },
        // WT_GUI_PAGE_ATTRIBUTE_IMAGE
        {
            QObject::tr(
            "  Configure the document image properties.<br>"
            "  - Section: select the available location to place<br>"
            "    the image attributethe. Available locations may<br>"
            "    be cover page ront or cover page back, content<br>"
            "    page header or content page footer. The selection<br>"
            "    drop-down list will display availabe location(s)<br>"
            "    for the selected image.<br><br>"
            "  - Placement: select where on the page to place<br>"
            "    the image.<br><br>"
            "  - Margins: set the width(Left/Right), height(Top/Bottom)<br>"
            "    image margins.<br>"
            "    Enter margin values using a decimal number<br>"
            "    in %1.<br>"
            "    Dot units are defined in Project Global Setup.<br><br>"
            "  - Browse: set or select the image path.<br><br>"
            "  - Scale: check to enable and configure scaling the image.<br><br>"
            "  - Image Fill: select aspect (resize on image aspect ratio),<br>"
            "    stretch or tile the image across the page background.%2<br>")
            .arg(MetaDefaults::getPreferredUnits())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_PAGE_ATTRIBUTE_IMAGE")
#else
                 ""
#endif
                )
        },
        // WT_GUI_SUBMODEL_LEVEL_COLORS
        {
            QObject::tr(
            "  Configure the submodel level page background color.<br><br>"
            "  These colors help differentiate the current submode<br>"
            "  level being edited.<br><br>"
            "  There are four default submodel levels, each with<br>"
            "  its own default colour.<br>"
            "  - Change: select the desired color using the<br>"
            "    color picker.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_SUBMODEL_LEVEL_COLORS")
#else
                 ""
#endif
                )
        },
        // WT_GUI_CHECK_BOX
        {
            QObject::tr(
            "  Check to enable property and uncheck to disable.<br><br>"
            "  Select 'What's this' Help on this check box's<br>"
            "  parent group box to see additional details<br>"
            "  about the behaviour enabled by this control.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_CHECK_BOX")
#else
                 ""
#endif
                )
        },
        // WT_GUI_UNITS
        {
            QObject::tr(
            "  Configure the unit pair.<br>"
            "  - Unit Value 0: set the first unit value.<br><br>"
            "  - Unit Value 1: set the second unit value.<br><br>"
            "    Enter margin values using a decimal number<br>"
            "    in %1.<br>"
            "    Dot units are defined in Project Global Setup.<br><br>"
            "  Select 'What's this' Help on this check box's<br>"
            "  parent group box to see additional details<br>"
            "  about the behaviour enabled by this control%2<br>")
            .arg(MetaDefaults::getPreferredUnits())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_UNITS")
#else
                 ""
#endif
                )
        },
        // WT_GUI_NUMBER
        {
            QObject::tr(
            "  Configure the number properties.<br>"
            "  - Font: select the number font.<br><br>"
            "  - Color: select the number font color.<br><br>"
            "  - Margins: set the number margin area width/height.<br><br>"
            "    Enter margin values using a decimal number<br>"
            "    in %1.<br>"
            "    Dot units are defined in Project Global Setup.<br><br>"
            "  Select 'What's this' Help on this check box's<br>"
            "  parent group box to see additional details<br>"
            "  about the behaviour enabled by this control%2<br>")
            .arg(MetaDefaults::getPreferredUnits())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_NUMBER")
#else
                 ""
#endif
                )
        },
        // WT_GUI_BOOL_RADIO_BUTTON
        {
            QObject::tr(
            "  Select between the pair of option alternatives.<br>"
            "  The selected option is set to true while the<br>"
            "  unselected option is false.<br><br>"
            "  Select 'What's this' Help on this check box's<br>"
            "  parent group box to see additional details<br>"
            "  about the behaviour enabled by this control%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_BOOL_RADIO_BUTTON")
#else
                 ""
#endif
                )
        },
        // WT_GUI_PAGE_SIZE
        {
            QObject::tr(
            "PageSizeGui SHOULD NOT BE USED<br>"
            "  - Use SizeAndOrientationGui.<br><br>"
            "  Configure the page size settings.<br>"
            "  - Size: set the page size using standard sizes like<br>"
            "    A4, Letter etc... You can also specify the 'Custom'<br>"
            "    size options along with its corresponding height and<br>"
            "    width values.<br><br>"
            "    The page size custom height and width unit values<br>"
            "    are determined by the resolution units specified<br>"
            "    in Project Global Setup.<br>"
            "    Dot unit options are dots per inch (DPI) or<br>"
            "    dots per centimetre (DPC).%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_PAGE_SIZE")
#else
                 ""
#endif
                )
        },
        // WT_GUI_PAGE_ORIENTATION
        {
            QObject::tr(
            "PageOrientationGui SHOULD NOT BE USED.<br>"
            "  - Use SizeAndOrientationGui.<br><br>"
            "  Configure the page orientation settings.<br>"
            "  - Portrait: set the page orientation to<br>"
            "    portrait.<br><br>"
            "  - Landscape: set the page orientation to<br>"
            "    landscape.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_PAGE_ORIENTATION")
#else
                 ""
#endif
                )
        },
        // WT_GUI_CONSTRAIN
        {
            QObject::tr(
            "  Configure the area constrain properties.<br>"
            "  Select the the constrain type from the<br>"
            "  drop-down list and use the line edit dialog<br>"
            "  to enter the corresponding constraint type units.<br><br>"
            "  Available types are:<br>"
            "  - Area: Apply constraint using specified area.<br><br>"
            "  - Square: Apply constraint using quadrilateral<br>"
            "    of equal-length sides.<br><br>"
            "  - Width: Apply constraint on specified width.<br><br>"
            "  - Height: Apply constraint on specified height.<br><br>"
            "  - Columns: Apply constraint on specified number<br>"
            "    of comumns.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_CONSTRAIN")
#else
                 ""
#endif
                )
        },
        // WT_GUI_DOUBLE_SPIN
        {
            QObject::tr(
            "  Configure a floating point number with double<br>"
            "  precision accuracy using a spinbox control.<br>"
            "  Available input properties are:<br>"
            "  - Range: the minimum and maximum integer values <br>"
            "    accepted as input.<br><br>"
            "  - SingleStep: the step by which the integer value is<br>"
            "    increased and decreased.<br><br>"
            "  - Decimal Places: the precision of the double spinbox<br>"
            "    in decimal format.<br><br>"
            "  - Value: the current double precision floating point<br>"
            "    number accepted as input.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_DOUBLE_SPIN")
#else
                 ""
#endif
                )
        },
        // WT_GUI_SPIN
        {
            QObject::tr(
            "  Configure an integer number using a spinbox control.<br>"
            "  Available input properties are:<br>"
            "  - Range: the minimum and maximum integer values <br>"
            "    accepted as input.<br><br>"
            "  - SingleStep: the step by which the integer value is<br>"
            "    increased and decreased.<br><br>"
            "  - Value: the current integer number accepted as input<br><br>"
            "  Select 'What's this' Help on this check box's<br>"
            "  parent group box to see additional details<br>"
            "  about the behaviour enabled by this control.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_SPIN")
#else
                 ""
#endif
                )
        },
        // WT_GUI_FLOATS
        {
            QObject::tr(
            "  Configure a pair of floating point (decimal) numbers<br>"
            "  with double precision.<br><br>"
            "  Three decimal places (e.g. 1.012) is a good example.<br><br>"
            "  Select 'What's this' Help on this check box's<br>"
            "  parent group box to see additional details<br>"
            "  about the behaviour enabled by this control%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_FLOATS")
#else
                 ""
#endif
                )
        },
        // WT_GUI_PART_SORT
        {
            QObject::tr(
            "PliSort SHOULD NOT BE USED.<br>"
            "  - Use PliSortOrderGui.<br><br>"
            "  - Sort Attributes: part size, colorcategory.<br><br>"
            "  - Direction: direction are ascending - from small<br>"
            "    to large<br><br>"
            "  Select 'What's this' Help on this check box's<br>"
            "  parent group box to see additional details<br>"
            "  about the behaviour enabled by this control.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_PART_SORT")
#else
                 ""
#endif
                )
        },
        // WT_GUI_PART_SORT_ORDER
        {
            QObject::tr(
            "  Configure the sort order and direction properties.<br>"
            "  - Sort Attributes: part color, category<br>"
            "    size and no-sort are available in each<br>"
            "    of the three ordered drop-down controls.<br><br>"
            "  - Sort Order: primary (first), secondary (second)<br>"
            "    and tertiary (third) precedence in which the<br>"
            "    attributes will be sorted.<br><br>"
            "  - Direction: direction are ascending - from small<br>"
            "    to large and descending from large to small.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_PART_SORT_ORDER")
#else
                 ""
#endif
                )
        },
        // WT_GUI_STUD_STYLE_AUTOMATE_EDGE_COLOR
        {
            QObject::tr(
            "  Configure the stud style and automate<br>"
            "  edge color settings.<br><br>"
            "  Stud style decorate studs with one of<br>"
            "  seven (7) LEGO inscribed styles.<br><br>"
            "  High contrast stud styles paint stud cylinder<br>"
            "  and part edge colors.<br><br>"
            "  Available stud styles:<br>"
            "  ° 0 Plain<br>"
            "  ° 1 Thin Line Logo<br>"
            "  ° 2 Outline Logo<br>"
            "  ° 3 Sharp Top Logo<br>"
            "  ° 4 Rounded Top Logo<br>"
            "  ° 5 Flattened Logo<br>"
            "  ° 6 High Contrast<br>"
            "  ° 7 High Contrast With Logo<br><br>"
            "  Light/Dark Value triggers the part edge color<br>"
            "  update for dark parts.<br><br>"
            "  High Contrast styles enable part edge and<br>"
            "  stud cylinder color settings.<br><br>"
            "  Automate edge color settings allow %1 to<br>"
            "  automatically adjust part edge colors.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_STUD_STYLE_AUTOMATE_EDGE_COLOR")
#else
                 ""
#endif
                )
        },
        // WT_GUI_HIGH_CONTRAST_COLOR_SETTINGS
        {
            QObject::tr(
            "  High Contrast styles exposes the Light/Dark Value<br>"
            "  edge color setting.<br><br>"
            "  Light/Dark Value triggers the part edge color<br>"
            "  update for dark parts.<br><br>"
            "  High Contrast styles also exposes the following<br>"
            "  part edge and stud cylinder color settings:<br>"
            "  - Stud Cylinder Color: the applied high contrast<br>"
            "    stud cylinder color<br><br>"
            "  - Parts Edge Color: the default high contrast parts<br>"
            "    edge color.<br><br>"
            "  - Black Parts Edge Color: the applied color for<br>"
            "    black parts.<br><br>"
            "  - Dark Parts Edge Color: the applied edge color for<br>"
            "    dark parts.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_HIGH_CONTRAST_COLOR_SETTINGS")
#else
                 ""
#endif
                )
        },
        // WT_GUI_AUTOMATE_EDGE_COLOR_SETTINGS
        {
            QObject::tr(
            "  Automate edge color settings allow %1 to<br>"
            "  automatically adjust part edge colors based<br>"
            "  on the following configured settings:<br>"
            "  - Contrast: the amount of contrast.<br><br>"
            "  - Saturation: the amount of edge color tint<br>"
            "    or shade.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_AUTOMATE_EDGE_COLOR_SETTINGS")
#else
                 ""
#endif
                )
        },
        // WT_GUI_LINE_WIDTH_MAX_GRANULARITY
        {
            QObject::tr(
            "  Set the maximum line width smoothing granularity and range.<br>"
            "  The granularity setting is automatically specified by your<br>"
            "  platform's graphics system; however, there are cases where<br>"
            "  the specified value is too granular making it difficult to<br>"
            "  manually set your desired line width in the specified range.<br>"
            "  Conversely, the specified range may not be enough to allow<br>"
            "  you to specify your desired line width.<br><br>"
            "  In these cases, use this setting to enable a range with the<br>"
            "  granularity that allows you to better set your line width.<br><br>"
            "  The smoothing range calculation will use the maximum granularity<br>"
            "  value measured between this setting and the value specified<br>"
            "  by your graphics system. So if you wish to use the system<br>"
            "  specified granularity, simply set this value to 0.0.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_LINE_WIDTH_MAX_GRANULARITY")
#else
                 ""
#endif
                )
        },
        // WT_GUI_PART_ANNOTATION_OPTIONS
        {
            QObject::tr(
            "  Part annotation options.<br>"
            "  - Display PartList (PLI) Annotation: select<br>"
            "    annotation source from Title, Free Form or<br>"
            "    Fixed annotations.<br><br>"
            "  - Enable Annotation Type: enable or disable<br>"
            "    annotation for axles, beams, cables, connectors,<br>"
            "    hoses, panels or custom annotations.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_PART_ANNOTATION_OPTIONS")
#else
                 ""
#endif
                )
        },
        // WT_GUI_PART_ANNOTATIONS_STYLES
        {
            QObject::tr(
            "  Part annotation styles.<br>"
            "  - Select Style to Edit: enable the corresponding<br>"
            "    background, border, text and size for editing.<br>"
            "    Choices include square, circle or rectangle.<br>"
            "    This setting defaults to None when the dialogue<br>"
            "    is initially opened.<br><br>"
            "  - Background: edit the annotation background<br><br>"
            "  - Border: edit the annotation border attributes<br><br>"
            "  - Annotation Text Format: edit the annotation text<br>"
            "    attributes<br><br>"
            "  - Size: edit the annotation size attributes.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_PART_ANNOTATIONS_STYLES")
#else
                 ""
#endif
                )
        },
        // WT_GUI_PART_ANNOTATIONS_SOURCE
        {
            QObject::tr(
            "  Enable part annotation:<br>"
            "  - Display: show part annotations.<br><br>"
            "  Select part annotation source:<br>"
            "  - Title: use the title annotation source file.<br>"
            "    Title annotations are derived from part<br>"
            "    descriptions taken the LDraw parts library.<br><br>"
            "  - Free Form: use the free form annotation source<br>"
            "    file.<br>"
            "    The above options allows the editor to define<br>"
            "    custom part annotations and use the 'Extended'<br>"
            "    annotation type.<br><br>"
            "  - Fixed: use the built-in annotations with fixed size<br>"
            "    properties. While the fixed size for square and<br>"
            "    circle styles cannot be modified from the LDraw<br>"
            "    file editor or main window, it is possible to change<br>"
            "    the fixed size properties in the 'Styles' section of<br>"
            "    this dialog.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_PART_ANNOTATIONS_SOURCE")
#else
                 ""
#endif
                )
        },
        // WT_GUI_PART_ANNOTATIONS_TYPE_BOM
        {
            QObject::tr(
            "  Enable annotation type:<br>"
            "  Select the bill of materials (BOM) annotations<br>"
            "  types to display in the instruction document.<br><br>"
            "  Available fixed annotation types for bill of<br>"
            "  materials (BOM) parts are:<br>"
            "  - Axles: annotation on circle background.<br><br>"
            "  - Beams: annotation on square background.<br><br>"
            "  - Cables: annotation on square background.<br><br>"
            "  - Connections: annotation on square background.<br><br>"
            "  - Hoses: annotation on square background.<br><br>"
            "  - Panels: annotation on circle background.<br><br>"
            "  - Element: LEGO element identifier on rectangular<br>"
            "    background.<br><br>"
            "  Available user configurable annotation types for<br>"
            "  bill of materials (BOM) parts are:<br>"
            "  - Extended: title and/or free form part annotations<br>"
            "    on rectangular background.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_PART_ANNOTATIONS_TYPE_BOM")
#else
                 ""
#endif
                )
        },
        // WT_GUI_PART_ANNOTATIONS_TYPE_PLI
        {
            QObject::tr(
            "  Enable the display of part annotation:<br>"
            "  Select the part list instance (PLI) annotation<br>"
            "  types to display in the instruction document.<br><br>"
            "  Available fixed annotation types for part list<br>"
            "  instance (PLI) parts are:<br>"
            "  - Axles: annotation on circle background.<br>"
            "  - Beams: annotation on square background.<br>"
            "  - Cables: annotation on square background.<br>"
            "  - Connections: annotation on square background.<br>"
            "  - Hoses: annotation on square background.<br>"
            "  - Panels: annotation on circle background.<br><br>"
            "  Available user configurable annotation types for<br>"
            "  part list instance (PLI) parts are:<br>"
            "  - Extended: title and/or free form part annotations<br>"
            "    on rectangular background.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_PART_ANNOTATIONS_TYPE_PLI")
#else
                 ""
#endif
                )
        },
        // WT_GUI_PART_ELEMENTS_BOM
        {
            QObject::tr(
            "  Enable and configure part element annotation.<br>"
            "  Bill of material part element identifiers can be sourced from one of two files:<br>"
            "  - Bricklink Codes: codes.txt is the tab-delimited Bricklink file downloaded from<br>"
            "    https://www.bricklink.com/catalogDownload.asp %1<br>"
            "    will automatically download a stored copy of this file if it is not found<br>"
            "    in the 'extras' user data folder.<br><br>"
            "  - User Defined Elements: userelements.lst (formerly legoelements.lst)<br>"
            "    is a tab-delimited user-defined text file that must be placed in the %1<br>"
            "   'extras' user data folder and include part id, color id and the user<br>"
            "    defined element id (whatever id you want to display) delimited by tabs.<br><br>"
            "  You can set %1 to use either LEGO Element or Bricklink ID - a combination<br>"
            "  of the Bricklink Item No and Color ID delimited with '-', for example 11145-87.<br>"
            "  - LDraw Key: allows you to specify what key format to use when storing and<br>"
            "    retrieving user-defined part elements.<br>"
            "    The default option is LDraw Part Type+Color. If you prefer to use the<br>"
            "    Bricklink Item No+Color, uncheck this setting.<br><br>"
            "  The three options availale to display part element annotations are<br>"
            "  therefore:<br>"
            "  - Bricklink: use Bricklink part element identifier sourced from Bricklink codes.txt.<br><br>"
            "  - LEGO: use LEGO part element identifiers sourced from Bricklink codes.txt.<br><br>"
            "  - User Defined: use the part element identifier sourced from the<br>"
            "    userelements.lst file using either LDraw or Bricklink key formats.<br><br>"
            "  As with the other %1 configuration files, you can edit the local instance<br>"
            "  of these files to update add or remove part elements.<br>"
            "  If a local file does not exist, it will be created when you select the<br>"
            "  respective Configuration, Edit Parameter Files menu item.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_PART_ELEMENTS_BOM")
#else
                 ""
#endif
                )
        },
        // WT_GUI_BUILD_MODIFICATIONS
        {
            QObject::tr(
            "  Turn on or off the build modifications feature.<br><br>"
            "  Build modifications allow you to modify your build<br>"
            "  instructions steps to show additional 'instruction aids'<br>"
            "  or to automate the positioning and camera angle of<br>"
            "  existing parts within a step. This feature is a substitute<br>"
            "  for the legacy MLCad BUFEXCHG framework.<br><br>"
            "  You can add a modification command in any step with parts<br>"
            "  and remove (or re-apply) the modification in any other<br>"
            "  subsequent step - regardless of the subsequent step's<br>"
            "  submodel or if the step is in a called out submodel.<br><br>"
            "  Build modification commands can include any number or<br>"
            "  configuration of existing or additional parts desired<br>"
            "  to demonstrate the editor's build instruction intent.<br><br>"
            "  Check the box to enable this feature or uncheck to disable.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_BUILD_MODIFICATIONS")
#else
                 ""
#endif
                )
        },
        // WT_GUI_CONTINUOUS_STEP_NUMBERS
        {
            QObject::tr(
            "  Turn on or off continuous step numbers feature.<br><br>"
            "  Continuous step numbers allow you to continuously<br>"
            "  number your instruction steps across submodels and<br>"
            "  unassembled callouts.<br><br>"
            "  When not enabled, %1 will restart step numbers for<br>"
            "  submodels and unassembled callouts.<br> Numbering in<br>"
            "  the parent submodel is conontinued after the callout<br>"
            "  or submodel steps are completed.<br><br>"
            "  Check the box to enable this feature or uncheck to<br>"
            "  disable.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_CONTINUOUS_STEP_NUMBERS")
#else
                 ""
#endif
                )
        },
        // WT_GUI_CONSOLIDATE_SUBMODEL_INSTANCE_COUNT
        {
            QObject::tr(
            "  Turn on or off the consolidate submodel instance<br>"
            "  count feature.<br><br>"
            "  Consolidate submodel instance count will calculate<br>"
            "  the number of instances at the first occurrence of<br>"
            "  the submodel.<br><br>"
            "  Check the box to enable this feature or uncheck to<br>"
            "  disable.<br><br>"
            "  Select where to consolidate the submodel instance<br>"
            "  count from the radio-button options.<br><br>"
            "  You can consolidate the submodel instance count with<br>"
            "  the following options:<br>"
            "  - At Top: display count at last step page of first<br>"
            "    occurrence in the entire model file. This is the<br>"
            "    LPub instance count default behaviour. However the<br>"
            "    default %1 count behaviour is At Model.<br><br>"
            "  - At Model: display count at last step page of first<br>"
            "    occurrence in the parent model. This is the default<br>"
            "    behaviour.<br><br>"
            "  - At Step: display count at step page of first<br>"
            "    occurrence in the respective step.<br><br>"
            "  - Count By Colour Code: Count unique instances for<br>"
            "    submodels that have the same name but unique colour<br>"
            "    code. This option is enabled (checked) by default.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_CONSOLIDATE_SUBMODEL_INSTANCE_COUNT")
#else
                 ""
#endif
                )
        },
        // WT_GUI_MODEL_VIEW_ON_COVER_PAGE
        {
            QObject::tr(
            "  Turn on or off the view top model in the visual viewer<br>"
            "  when the current page is a cover page feature.<br><br>"
            "  You can set %1 to display the complete instruction<br>"
            "  document model in the visual editor when the main<br>"
            "  window is currently displaying a front or back cover<br><br>"
            "  page.<br><br>"
            "  Check the box to enable this feature or uncheck to<br>"
            "  disable.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_MODEL_VIEW_ON_COVER_PAGE")
#else
                 ""
#endif
                )
        },
        // WT_GUI_LOAD_UNOFFICIAL_PART_IN_EDITOR
        {
            QObject::tr(
            "  Turn on or off loading unofficial parts in the command<br>"
            "  editor.<br><br>"
            "  You can choose to load and edit unofficial parts in the<br>"
            "  command editor - effectively treating the part like a<br>"
            "  submodel.<br><br>"
            "  This feature is useful when you have inline unofficial<br>"
            "  parts in you model file and would like to view and or<br>"
            "  edit the part's content.<br><br>"
            "  Check the box to enable this feature or uncheck to<br>"
            "  disable.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_LOAD_UNOFFICIAL_PART_IN_EDITOR")
#else
                 ""
#endif
                )
        },
        // WT_GUI_PREFERRED_RENDERER
        {
            QObject::tr(
            "  Select the default preferred step image renderer.<br>"
            "  You can use the drop-down control to select from<br>"
            "  the following %1 packaged renderer options:<br>"
            "  - Native: %1 'native' renderer and 3D Viewer,<br>"
            "    offering performant, 'true fade' image rendering.<br><br>"
            "  - LDGLite: fast image rendering ideal for proofing.<br><br>"
            "  - LDView: high quality image rendering plus POV<br>"
            "    scene file generation.<br><br>"
            "  - POV-Ray: ray tracing, photo-realistic, feature rich<br>"
            "    image rendering.<br><br>"
            "  Selecting the LDView renderer enables the following<br>"
            "  additional performance-oriented options:<br>"
            "  - Use LDView Single Call: submit all the ldraw files<br>"
            "    (parts, assembly, submodel preview) for the current<br>"
            "    page in a single LDView render request instead of<br>"
            "    individually at each step.<br><br>"
            "  - Use LDView Snapshot List: this option extends and<br>"
            "    requires LDView Single Call whereby a list files<br>"
            "    for ldraw part or assembly files are created and<br>"
            "    submitted to LDView.<br><br>"
            "    As only one submodel preview per page is generated,<br>"
            "    these options are ignored for submodel preview images.<br>"
            "    Additionally, when there is less than 2 image files<br>"
            "    for a page's parts list or step group, these options<br>"
            "    are ignored.<br><br>"
            "  The POV-Ray renderer requires a POV input file which is<br>"
            "  generated by the following selected option:<br>"
            "  - Native: an %1 imbeded module which is built<br>"
            "    from LDView source and performs POV file generation<br>"
            "    among some other features.<br><br>"
            "  - LDView: POV file generation is submitted to the %1<br>"
            "    packaged instance of LDView just as normal image files.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_PREFERRED_RENDERER")
#else
                 ""
#endif
                )
        },
        // WT_GUI_DOCUMENT_RESOLUTION
        {
            QObject::tr(
            "  Set the default instruction document dot resolution.<br><br>"
            "  You can configure the document dot resolution with the<br>"
            "  following unit options:<br>"
            "  - Dots Per Inch: set the Units drop-down to this<br>"
            "    value if you would like to use inches.<br><br>"
            "  - Dots Per Centimetre: set the Units drop-down to this<br>"
            "    value if you would like to use centimetres.<br><br>"
            "  Use the dot resulution unit value dialog to set the<br>"
            "  number of units. The default for Dots Per Inch is 150<br>"
            "  which automatically changes to 381 when the units<br>"
            "  drop-down is switched to Dots Per Centimetre.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_DOCUMENT_RESOLUTION")
#else
                 ""
#endif
                )
        },
        // WT_GUI_ASSEM_ANNOTATION_TYPE
        {
            QObject::tr(
            "  Enable the display of part annotation:<br>"
            "  Select the Current Step Instance (CSI)<br>"
            "  annotation types to display in the instruction<br>"
            "  document.<br><br>"
            "  Available fixed annotation types for CSI parts are:<br>"
            "  - Axles: annotation on circle background.<br><br>"
            "  - Beams: annotation on square background.<br><br>"
            "  - Cables: annotation on square background.<br><br>"
            "  - Connections: annotation on square background.<br><br>"
            "  - Hoses: annotation on square background.<br><br>"
            "  - Panels: annotation on circle background.<br><br>"
            "  Available user configurable annotation types for<br>"
            "  current step instance (CSI) parts are:<br>"
            "  - Extended: title and/or free form part annotations<br>"
            "    on rectangular background.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_ASSEM_ANNOTATION_TYPE")
#else
                 ""
#endif
                )
        },
        // WT_GUI_FADE_STEPS
        {
            QObject::tr(
            "  Turn on or off fade previous steps or step parts globally.<br><br>"
            "  You can fade step parts locally using the !FADE command.<br>"
            "  You can configure fade previous steps or step parts with<br>"
            "  the following options:<br>"
            "  - Use LPub Fade Steps: this option will generate faded<br>"
            "    submodels and modify part lines color value before<br>"
            "    submitting them to the renderer. When not using the<br>"
            "    Native renderer, this option is obligatory.<br>"
            "    When unchecked, %1 will use the Native renderer fade<br>"
            "    behaviour. Parts and submodels are not manipulated, so<br>"
            "    when unchecked, step navigation is more performant.<br><br>"
            "  - Setup LPub Fade Steps: Turn on or off the capability<br>"
            "    to define fade previous steps or step parts locally<br>"
            "    (versus globally). This is to say, you can apply fade<br>"
            "    previous steps from the step where the LPUB meta command<br>"
            "    is placed to enable this feature to the step where it<br>"
            "    is placed to disable it.<br>"
            "    For display model steps, this command is automatically<br>"
            "    disabled at the end of the step.<br>"
            "    This option requires Use LPub Fade Steps and is<br>"
            "    disabled if Enable Fade Steps is checked.<br><br>"
            "  - Use Fade Color: all faded parts are assigned the color<br>"
            "    specified for fade color as described above.<br>"
            "    When not checked, faded parts will display their<br>"
            "    assigned part color faded to the specified opacity.<br><br>"
            "  - Fade Color: set the fade color using the LDraw<br>"
            "    colors drop-down control.<br>"
            "    Fade color selection defaults to standard LDraw colors.<br>"
            "    However, you can manually enter any color using the<br>"
            "    (#|0x)([AA]RRGGBB) hex [A]RGB value syntax.<br>"
            "    Meta commands using LDraw colors will display the LDraw<br>"
            "    color safe name (spaces replaced with '_') versus a<br>"
            "    hex [A]RGB value as there are multiple LDraw colours<br>"
            "    that use the same hex [A]RGB value.<br><br>"
            "  - Fade Percent: sets the fade color transparency<br>"
            "    between 0 and 100 percent where 0 is fully<br>"
            "    transparent and 100 is fully opaque.<br>"
            "    This value is translated to the fade color<br>"
            "    opacity and alpha channel - for example, 60% fade<br>"
            "    is 40% opacity, 102/255 alpha channel and 66 hex.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_FADE_STEPS")
#else
                 ""
#endif
                )
        },
        // WT_GUI_HIGHLIGHT_CURRENT_STEP
        {
            QObject::tr(
            "  Turn on or off highlight current step or step parts<br>"
            "  globally.<br><br>"
            "  You can highlight step parts locally using the<br>"
            "  !SILHOUETTE command.<br>"
            "  You can configure highlight current step or step parts<br>"
            "  with the following options:<br>"
            "  - Use LPub Highlight Step: this option will generate<br>"
            "    highlighted submodels and modify part lines color<br>"
            "    value before submitting them to the renderer. When<br>"
            "    not using the Native renderer, this option is<br>"
            "    obligatory. When unchecked, %1 will use the Native<br>"
            "    renderer highlight behaviour. Parts and submodels<br>"
            "    are not manipulated, so when unchecked, step<br>"
            "    navigation is more performant.<br><br>"
            "  - Setup LPub Highlight Step: Turn on or off the<br>"
            "    capability to define highlight current step or step<br>"
            "    parts locally (versus globally). This is to say, you<br>"
            "    can apply highlight step from the step where the LPub<br>"
            "    meta command is placed to enable this feature to the<br>"
            "    step where it is placed to disable it.<br>"
            "    For display model steps, this command is automatically<br>"
            "    disabled at the end of the step.<br>"
            "    This option requires Use LPub Highlight Step and is<br>"
            "    disabled if Enable Highlight Step is checked.<br><br>"
            "  - Highlight Color: set the highlight color using<br>"
            "    the color picker.<br>"
            "    Any color can be selected for the highlight color.<br><br>"
            "  - Line Width (LDGLite renderer only): set the<br>"
            "    highlight line width.<br>"
            "    The mininimum allowed spin box value is 0<br>"
            "    and the maximum is 10.<br>"
            "    Spin box values are incremented by 1 step<br>"
            "    unit.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_HIGHLIGHT_CURRENT_STEP")
#else
                 ""
#endif
                )
        },
        // WT_GUI_FINAL_FADE_HIGHLIGHT_MODEL_STEP
        {
            QObject::tr(
            "  Turn on or off auto generating a final step displaying<br>"
            "  the final model without highlight or fade content.<br><br>"
            "  The 6 auto-generated command lines that define this step<br>"
            "  are not persisted to your saved ldraw model file.<br><br>"
            "  If you manually save the command 0 !LPUB INSERT MODEL<br>"
            "  to your model file, when you subsequently open the file,<br>"
            "  if neither fade previous steps nor highlight current step<br>"
            "  is enabled, %1 will remove this command, and any<br>"
            "  adjacent content containing the auto-generted commands<br>"
            "  from the loaded working document.<br><br>"
            "  Saving your content will consequently remove the final<br>"
            "  model lines from your ldraw model file.<br><br>"
            "  If you prefer to persist the final model step - i.e override<br>"
            "  this behaviour - you can remove the auto-generated<br>"
            "  comment lines and replace 0 !LPUB INSERT MODEL with:<br><br>"
            "  0 !LPUB INSERT DISPLAY_MODEL<br><br>"
            "  Check the box to enable this feature or uncheck to disable.%2<br>")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_FINAL_FADE_HIGHLIGHT_MODEL_STEP")
#else
                 ""
#endif
                )
        },
        // WT_GUI_SEPARATOR
        {
            QObject::tr(
            "  Configure the separator.<br>"
            "  - Width: set the separator width as a floating<br>"
            "    point number (decimal).<br><br>"
            "  - Length: set the separator length value as required<br>"
            "    by the following length options:<br>"
            "    - Default: length is automatically calculated -<br>"
            "      length value entry is disabled.<br><br>"
            "    - Page: length is fixed to the page height minus<br>"
            "      the page margins, header and footer - length<br>"
            "      value entry is disabled.<br><br>"
            "    - Custom: specify your desired spearator length<br>"
            "      using a floating point (decimal) number.<br><br>"
            "  - Color: select the separator line color using<br>"
            "    the Change button to launch the color picker.<br><br>"
            "  - Margins: set the border margins width<br>"
            "    and height.<br>"
            "    Enter width, length and margin values using<br>"
            "    a decimal number in %1.<br>"
            "    Dot units are defined in Project Global Setup.<br>"
            "    Precision is four decimal places.%2<br>")
            .arg(MetaDefaults::getPreferredUnits())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_SEPARATOR")
#else
                 ""
#endif
                )
        },
        // WT_GUI_STEP_JUSTIFICATION
        {
            QObject::tr(
            "  Configure the step justification.<br>"
            "  - Set Step Justification: - set the step justification<br>"
            "    using the drop-down control to select from the<br>"
            "    following options:<br>"
            "    - Center: center the step horizontally and vertically.<br>"
            "    - Center Horizontal: center the step horizontally.<br>"
            "    - Center Vertical: center the step vertically.<br>"
            "    - Left: position the step at the left border.<br>"
            "      This is the default positioning.<br><br>"
            "  - Spacing: set the spacing, using a floating point<br>"
            "    number (decimal), between items when the step<br>"
            "    is center justified.<br>"
            "    Enter spacing value using a decimal number<br>"
            "    in %1.<br>"
            "    Dot units are defined in Project Global Setup.<br>"
            "    Precision is four decimal places.%2<br>")
            .arg(MetaDefaults::getPreferredUnits())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_STEP_JUSTIFICATION")
#else
                 ""
#endif
                )
        },
        // WT_GUI_STEP_ROTATION
        {
            QObject::tr(
            "  Configure the default step rotation.<br>"
            "  - Rotation: set the x-angle, y-angle and z-angle,<br>"
            "    which are the individual rotation angles for the<br>"
            "    different axes in degree (-360 to 360), using<br>"
            "    decimals.<br>"
            "    Spin box values are incremented by 1 step unit.<br>"
            "    Precision is automatically calculated based on the.<br>"
            "    input values.<br><br>"
            "  - Transform: there are three transform options.<br>"
            "    - REL: relative rotation steps are based on the<br>"
            "      actual angles of the individual viewing areas.<br>"
            "      The model will rotated by the specified rotation<br>"
            "      angles relative to the current view angle.<br><br>"
            "    - ABS: absolute rotation steps is similar to relative<br>"
            "      rotation steps, but it ignores the current view<br>"
            "      angles so that after executing this command the<br>"
            "      model will be rotated from a starting angle of 0.<br><br>"
            "    - ADD: additive rotation step turns the model by<br>"
            "      the specified angles, taking the current view<br>"
            "      angle into account.<br>The command can be used to<br>"
            "      continuously rotate the model by a specific angle.<br>"
            "      For example if this command is executed four times<br>"
            "      on a front view, and the model is rotated<br>"
            "      90° clockwise on the y-axle then you will see the<br>"
            "      model from each of the four sides.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_STEP_ROTATION")
#else
                 ""
#endif
                )
        },
        // WT_GUI_SUBMODEL_PREVIEW_DISPLAY
        {
            QObject::tr(
            "  Configure the default submodel preview display<br>"
            "  settings.<br>"
            "  - Show submodel at first step: turn on or off<br>"
            "    displaying a complete submodel image at the<br>"
            "    first step of the submodel.<br><br>"
            "  For each preview display option, you can select<br>"
            "  one or both of the following persist settings:<br>"
            "  - Set In Preferences: set the preview display option<br>"
            "    in Preferences so that this option will apply<br>"
            "    globally for all instruction documents.<br><br>"
            "  - Add LPub Meta Command: add the corresponding<br>"
            "    LPUB meta command to instruction document.<br><br>"
            "  Checking the show submodel at first step option<br>"
            "  will enable the following submodel preview options.<br>"
            "  - Show main model at first step: show the top model<br>"
            "    the first step. This option is not typically<br>"
            "    selected as the displayed model will be the same<br>"
            "    as that on the instruction cover page.<br><br>"
            "  - Show submodel in callout: show the called out<br>"
            "    submodel in the first step of the callout. This<br>"
            "    option is only enabled for unassembled callouts.<br><br>"
            "  - Show submodel instance count: add the instance<br>"
            "    count to the submodel preview display.<br><br>"
            "  The submodel preview default placement can be set<br>"
            "  using the following setting:<br>"
            "  - Submodel Placement: change the default submodel<br>"
            "    placement using the placement dialog.%1<br>")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_SUBMODEL_PREVIEW_DISPLAY")
#else
                 ""
#endif
                )
        },
        // WT_GUI_ROTATE_ICON_SIZE
        {
            QObject::tr(
            "  Configure rotate icon size.<br>"
            "  - Width: set the rotate icon  width in %1.<br><br>"
            "  - Height: set the rotate icon hieght in %1.<br><br>"
            "    Dot units are defined in Project Global Setup.%2<br>")
            .arg(MetaDefaults::getPreferredUnits())
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_ROTATE_ICON_SIZE")
#else
                 ""
#endif
                )
        },
        // WT_GUI_RENDERER_PARAMETERS
        {
            QObject::tr(
            "  Configure your instruction document %1<br>"
            "  additional parameters and environment variables.<br><br>"
            "  Both %1 parameters and environment variables<br>"
            "  must be space delimited and can be either single ' or<br>"
            "  double \" qouted. So if you want to use a value that<br>"
            "  contains at least one space, this value must be quoted.<br><br>"
            "  Environment variables must respect the system environment<br>"
            "  format which usually is key, value separated by an<br>"
            "  equal sign.%2%3<br>")
            .arg(MetaDefaults::getPreferredRenderer())
            .arg(MetaDefaults::getPreferredRenderer() ==
                 QLatin1String("POVRay") && !Preferences::useNativePovGenerator
                 ? QObject::tr("<br><br>"
            "  The current configuration is using %1 as the POV file<br>"
            "  generator so dialogues for %1 additional parameters<br>"
            "  and environment variables are present.<br>"
            "  These configuration settings should only target the<br>"
            "  POV file generator parameters and environment dialogues.<br>")
            .arg(MetaDefaults::getPreferredRenderer(RENDERER_LDVIEW))
            : "")
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_RENDERER_PARAMETERS")
#else
                 ""
#endif
                )
        },
        // WT_GUI_RENDERER_PARAMETERS_BLENDER
        {
            QObject::tr(
            "  Configure your %1 photo-realistic image render<br>"
            "  additional parameters and environment variables.<br><br>"
            "  Both %1 parameters and environment variables<br>"
            "  must be space delimited and can be either single ' or<br>"
            "  double \" qouted. So if you want to use a value that<br>"
            "  contains at least one space, this value must be quoted.<br><br>"
            "  Environment variables must respect the system environment<br>"
            "  format which usually is key, value separated by an<br>"
            "  equal sign.%2<br>")
            .arg(MetaDefaults::getPreferredRenderer(RENDERER_BLENDER))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_RENDERER_PARAMETERS_BLENDER")
#else
                 ""
#endif
                )
        },
        // WT_GUI_RENDERER_PARAMETERS_POVRAY
        {
            QObject::tr(
            "  Configure your %1 photo-realistic image render<br>"
            "  additional parameters and environment variables.<br><br>"
            "  Both %1 parameters and environment variables<br>"
            "  must be space delimited and can be either single ' or<br>"
            "  double \" qouted. So if you want to use a value that<br>"
            "  contains at least one space, this value must be quoted.<br><br>"
            "  Environment variables must respect the system environment<br>"
            "  format which usually is key, value separated by an<br>"
            "  equal sign.%2<br>")
            .arg(MetaDefaults::getPreferredRenderer(RENDERER_POVRAY))
            .arg(
#ifdef QT_DEBUG_MODE
                 QLatin1String("<br><br>  WT_GUI_RENDERER_PARAMETERS_POVRAY")
#else
                 ""
#endif
                )
        }
        // WT_NUM_ENTRIES
    };

    std::copy(std::begin(WTData), std::end(WTData), std::begin(CommonMenus::WT));

    WT_ARRAY_SIZE_CHECK(CommonMenus::WT, WT_NUM_ENTRIES);
}
