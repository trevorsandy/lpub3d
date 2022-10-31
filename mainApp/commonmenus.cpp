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

#define WT_ARRAY_COUNT(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define WT_ARRAY_SIZE_CHECK(a,s) static_assert(WT_ARRAY_COUNT(a) == static_cast<int>(s), QT_STRINGIFY(a) " size mismatch.")

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
  action->setObjectName(objectName.isEmpty() ? "rendererArgumentsAction.1" : objectName);
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
        // WT_PREFERENCES
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_PREFERENCES_DIRECTORIES
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_PREFERENCES_GENERAL
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_PREFERENCES_LDGLITE
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_PREFERENCES_LDVIEW
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_PREFERENCES_LOGGING
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_PREFERENCES_NATIVE
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_PREFERENCES_OTHER
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_PREFERENCES_POVRAY
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_PREFERENCES_PUBLISHING
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_PREFERENCES_RENDERERS
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_PREFERENCES_SHORTCUTS
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_PREFERENCES_UPDATES
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_VISUAL_PREFS
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_VISUAL_PREFS_CATEGORIES
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_VISUAL_PREFS_COLORS
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_VISUAL_PREFS_GENERAL
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_VISUAL_PREFS_KEYBOARD
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_VISUAL_PREFS_MOUSE
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_VISUAL_PREFS_RENDERING
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_VISUAL_PREFS_USER_INTERFACE
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_VISUAL_PREFS_VIEWS
        {
            QObject::tr("  To be defined...\n")
        },
        //*************************************
        //*  GLOBAL SETUP ENTRIES
        //*************************************
        // WT_SETUP_ASSEM
        {
            QObject::tr(
            "  Configure and edit your instructions document\n"
            "  assembly / Current Step Instance (CSI)\n"
            "  settings.\n\n"
            "  You can configure your assembly content\n"
            "  and display.\n")
        },
        // WT_SETUP_ASSEM_CONTENTS
        {
            QObject::tr(
            "  Configure your instruction document assembly\n"
            "  sizing,\n"
            "  assembly orientation and step number properties.\n")
        },
        // WT_SETUP_ASSEM_DISPLAY
        {
            QObject::tr(
            "  Configure your instruction document assembly\n"
            "  step number display, part annotation display,\n"
            "  and stud style and automate edge color settings.\n")
        },
        // WT_SETUP_ASSEM_DISPLAY_STEP
        {
            QObject::tr(
            "  The following configuration setup is available for\n"
            "  assembly step:\n"
            "  - Step Number: turn or off the step number display.\n")
        },
        // WT_SETUP_ASSEM_ANNOTATION
        {
            QObject::tr(
            "  The following configuration setup are available for\n"
            "  assembly part annotations:\n"
            "  - Display Assembly (CSI) Part Annotation: turn on or\n"
            "    off CSI part annotation.\n\n"
            "  - CSI Part Annotation Placement: change CSI part\n"
            "    annotation placement.\n"
            "    You can also drag part annotations from their\n"
            "    placed location to your desired location.\n")
        },
        // WT_SETUP_CALLOUT
        {
            QObject::tr(
            "  Configure and edit the callout settings\n"
            "  for your instructions document.\n")
        },
        // WT_SETUP_CALLOUT_ASSEMBLY
        {
            QObject::tr(
            "  Configure your instruction document callout\n"
            "  assembly image, assembly orientation, assembly\n"
            "  margins and assembly stud style and automated\n"
            "  edge colors settings.\n")
        },
        // WT_SETUP_CALLOUT_BACKGROUND_BORDER
        {
            QObject::tr(
            "  Configure your instruction document callout\n"
            "  background, border, margins, and divider\n"
            "  settings.\n")
        },
        // WT_SETUP_CALLOUT_CALLOUT
        {
            QObject::tr(
            "  Configure your instruction document callout\n"
            "  parts list, step justification, submodel preview,\n"
            "  step number and times used settings.\n")
        },
        // WT_SETUP_CALLOUT_CALLOUT_PARTS_LIST
        {
            QObject::tr(
            "  The following configuration setup is available for\n"
            "  callout parts list:\n"
            "  - Per Step: set the parts list instance (PLI) per step\n"
            "    display. When unchecked, the PLI is set to per\n"
            "    callout.\n\n"
            "  - Margins: set the parts list instance (PLI) margins\n"
            "    L/R(Left/Right) width and T/B(Top/Bottom) height.\n"
            "    Enter margin values using a decimal number\n"
            "    in %1.\n"
            "    Dot units are defined in Project Global Setup.\n")
            .arg(MetaDefaults::getPreferredUnits())
        },
        // WT_SETUP_CALLOUT_CALLOUT_SUBMODEL_PREVIEW
        {
            QObject::tr(
            "  The following configuration setup is available for\n"
            "  callout submodel preview:\n"
            "  - Show Submodel Preview At First Step: turn on or off\n"
            "    displaying the called out submodel at the first\n"
            "    step in the callout. This option is only available\n"
            "    for unassembled callouts.\n\n")
        },
        // WT_SETUP_CALLOUT_CALLOUT_POINTERS
        {
            QObject::tr(
            "  Configure your instruction document callout\n"
            "  pointers:\n"
            "  - Configure the pointer border.\n"
            "  - Configure the pointer line.\n"
            "  - Configure the pointer tip.\n")
        },
        // WT_SETUP_CALLOUT_CONTENTS
        {
            QObject::tr(
            "  The following configuration setup are available in\n"
            "  the callout Content dialog:\n"
            "  - Configure the callout properties.\n\n"
            "  - Configure the callout assembly properties.\n"
            "  - Configure the callout pointers.\n")
        },
        // WT_SETUP_CALLOUT_DIVIDER_POINTERS
        {
            QObject::tr(
            "  Configure your instruction document callout\n"
            "  divider pointers:\n"
            "  - Configure the pointer border.\n"
            "  - Configure the pointer line.\n"
            "  - Configure the pointer tip.\n")
        },
        // WT_SETUP_CALLOUT_POINTERS
        {
            QObject::tr(
            "  The following configuration setup are available in\n"
            "  the callout Pointers dialog:\n"
            "  - Configure the Callout Pointer properties.\n"
            "  - Configure the Divider Pointer properties.\n")
        },
        // WT_SETUP_FADE_STEPS
        {
            QObject::tr(
            "  Configure and edit your instructions document\n"
            "  fade previous steps settings.\n\n"
            "  You can configure your fade previous steps display,\n"
            "  setup and final model step settings.\n")
        },
        // WT_SETUP_FADE_STEPS_SETUP
        {
            QObject::tr(
            "  Turn on of off the capability to define fade previous\n"
            "  steps locally (versus globally). This is to say, you\n"
            "  can apply fade previous steps from the step where\n"
            "  the LPub meta command is defined to enable this \n"
            "  feature to the step where it is defined to disable it.\n\n"
            "  This setting is disabled if Enable Fade Previous Steps\n"
            "  is checked.\n")
        },
        // WT_SETUP_HIGHLIGHT_STEP
        {
            QObject::tr(
            "  Configure and edit your instructions document\n"
            "  highlight current step settings.\n\n"
            "  You can configure your highlight current step\n"
            "  display, setup and final model step settings.\n")
        },
        // WT_SETUP_HIGHLIGHT_STEP_SETUP
        {
            QObject::tr(
            "  Turn on of off the capability to define highlight\n"
            "  current step locally (versus globally). This is to\n"
            "  say, you can apply highlight current step from the\n"
            "  step where the LPub meta command is defined to enable\n"
            "  this feature to the step where it is defined to disable\n"
            "  it.\n\n"
            "  This setting is disabled if Enable Highlight Current\n"
            "  Step is checked.\n")
        },
        // WT_SETUP_MULTI_STEP
        {
            QObject::tr(
            "  Configure and edit the step group (multi step page)\n"
            "  settings for your instructions document.\n")
        },
        // WT_SETUP_MULTI_STEP_ADJUST_STEP
        {
            QObject::tr(
            "  The following configuration setup is available for\n"
            "  step group callout and rotate icon:\n"
            "  - Adjust Step Row Or Column: turn on or off adjust\n"
            "    step row or column when callout or rotate icon\n"
            "    is dragged.\n")
        },
        // WT_SETUP_MULTI_STEP_ASSEMBLY
        {
            QObject::tr(
            "  Configure your instruction document step group\n"
            "  assembly image, assembly orientation, assembly\n"
            "  margins and assembly stud style and automated\n"
            "  edge colors settings.\n")
        },
        // WT_SETUP_MULTI_STEP_CONTENTS
        {
            QObject::tr(
            "  Configure your instruction document step group\n"
            "  margins, step justification, callout and rotate\n"
            "  icon adjustment and step number settings.\n")
        },
        // WT_SETUP_MULTI_STEP_DIVIDER
        {
            QObject::tr(
            "  Configure your instruction document step group\n"
            "  divider, and divider pointer line and tip\n"
            "  settings.\n")
        },
        // WT_SETUP_MULTI_STEP_DISPLAY
        {
            QObject::tr(
            "  Configure your instruction document step group\n"
            "  parts list and submodel preview settings.\n")
        },
        // WT_SETUP_MULTI_STEP_DISPLAY_PARTS_LIST
        {
            QObject::tr(
            "  The following configuration setup is available for\n"
            "  step group parts list:\n"
            "  - Per Step: set the parts list instance (PLI) per step\n"
            "    display. When unchecked, the PLI is set to per\n"
            "    page.\n\n"
            "  - Show Group Step Number: turn on show group step\n"
            "    number when parts list instance per page is enabled.\n"
            "    This setting enables to PLI to be placed relative to\n"
            "    the step number.\n\n"
            "  - Count Group Steps: turn on count step group as a\n"
            "    single step when parts list instance per page is\n"
            "    enabled.\n\n"
            "  - Margins: set the parts list instance (PLI) margins\n"
            "    L/R(Left/Right) width and T/B(Top/Bottom) height.\n"
            "    Enter margin values using a decimal number\n"
            "    in %1.\n"
            "    Dot units are defined in Project Global Setup.\n")
            .arg(MetaDefaults::getPreferredUnits())
        },
        // WT_SETUP_MULTI_STEP_DISPLAY_SUBMODEL_PREVIEW
        {
            QObject::tr(
            "  The following configuration setup is available for\n"
            "  step group submodel preview:\n"
            "  - Show Submodel Preview At First Step: turn on or off\n"
            "    displaying the current submodel at the first\n"
            "    step in the step group.\n")
        },
        // WT_SETUP_PAGE
        {
            QObject::tr(
            "  Configure and edit your instructions document\n"
            "  page settings.\n\n"
            "  Use the Section drop-down dialog to select between\n"
            "  front and back cover pages, and content page header\n"
            "  or footer.\n\n"
            "  Most settings will be automatically extracted from\n"
            "  the open model file or setting defined in Preferences.\n\n"
            "  Regardless, you can change pre-populated settings with\n"
            "  this dialog.\n")
        },
        // WT_SETUP_PAGE_AUTHOR_EMAIL
        {
            QObject::tr(
            "  Enable and configure the author and email address.\n"
            "  - Section (Author): select the front cover, back cover,\n"
            "    page header, or page footer to place the author.\n\n"
            "  - Section (Email Address): select the back cover,\n"
            "    page header, or page footer to place the email address.\n\n"
            "  - Content: enter or update the document author and\n"
            "    email address.\n")
        },
        // WT_SETUP_PAGE_LPUB3D_LOGO
        {
            QObject::tr(
            "  Enable and configure the %1 logo image.\n"
            "  - Section: only the back cover %1 logo is configurable\n\n"
            "  Enable and configure the %1 logo border.\n")
            .arg(VER_PRODUCTNAME_STR)
        },
        // WT_SETUP_PAGE_COVER_IMAGE
        {
            QObject::tr(
            "  Enable and configure the front cover image.\n"
            "  - Section: only the front cover image is configurable\n\n"
            "  Enable and configure the front cover image border.\n")
        },
        // WT_SETUP_PAGE_DESCRIPTION
        {
            QObject::tr(
            "  Enable and configure the model description.\n"
            "  - Section: only the front cover model description\n"
            "    is configurable.\n\n"
            "  - Content: enter or update the model description.\n")
        },
        // WT_SETUP_PAGE_DOCUMENT_LOGO
        {
            QObject::tr(
            "  Enable and configure the document logo image.\n"
            "  - Section: select the front cover, back cover,\n"
            "    to place the document\n\n"
            "  Enable and configure the document logo border.\n")
        },
        // WT_SETUP_PAGE_LEGO_DISCLAIMER
        {
            QObject::tr(
            "  Enable and configure the LEGO disclaimer.\n"
            "  - Section: only the back cover LEGO disclaimer\n"
            "    is configurable.\n\n"
            "  - Content: enter or update the LEGO disclaimer.\n")
        },
        // WT_SETUP_PAGE_MODEL_PARTS_ID
        {
            QObject::tr(
            "  Enable and configure the model ID and parts count.\n"
            "  - Section: only the front cover model id and\n"
            "    parts count is configurable.\n\n"
            "  - Content: enter or update the document\n"
            "    model id and parts count.\n")
        },
        // WT_SETUP_PAGE_LPUB3D_PLUG
        {
            QObject::tr(
            "  Enable and configure the %1 plug.\n"
            "  - Section: only the back cover %1 plug\n"
            "    is configurable.\n\n"
            "  - Content: The %1 plug is not editable.\n").arg(VER_PRODUCTNAME_STR)
        },
        // WT_SETUP_PAGE_TITLE
        {
            QObject::tr(
            "  Enable and configure the document title.\n"
            "  - Section: select the front or back cover page.\n\n"
            "  - Content: enter or update the document title.\n")
        },
        // WT_SETUP_PAGE_PUBLISH_DESCRIPTION
        {
            QObject::tr(
            "  Enable and configure a publisher description.\n"
            "  - Section: select the front cover page,\n"
            "    to place the publisher description.\n\n"
            "  - Content: enter or update the document\n"
            "    publisher description.\n")
        },
        // WT_SETUP_PAGE_WEBSITE_URL_COPYRIGHT
        {
            QObject::tr(
            "  Enable and configure website URL and document copyright.\n"
            "  - Section (Website URL): select the back cover,\n"
            "    page header, or page footer to place the website URL.\n\n"
            "  - Section (Copyright): select the front cover, back cover,\n"
            "    page header, or page footer to place the document\n"
            "    copyright.\n\n"
            "  - Content: enter or update the document website URL\n"
            "    and document copyright.\n")
        },
        // WT_SETUP_PAGE_VIEW
        {
            QObject::tr(
            "  The following configuration setup are available for\n"
            "  document pages:\n"
            "  - Configure the page size and orientation settings.\n\n"
            "  - Configure the page background.\n\n"
            "  - Enable and configure the page border.\n\n"
            "  - Configure the page margins.\n\n"
            "  - Configure the page header height.\n\n"
            "  - Configure the page footer height.\n")
        },
        // WT_SETUP_PAGE_DISCLAIMER
        {
            QObject::tr(
            "  Configure your instruction document disclaimer"
            "  and display the %1 logo and 'plug' statement.\n")
            .arg(VER_PRODUCTNAME_STR)
        },
        // WT_SETUP_PAGE_MODEL
        {
            QObject::tr(
            "  Configure your instruction document page title,\n"
            "  cover image, description, model id, parts and\n"
            "  submodel level color.\n")
        },
        // WT_SETUP_PAGE_NUMBER_TEXT
        {
            QObject::tr(
            "  Enable and configure the page number display,\n"
            "  look, and placement.\n\n"
            "  Enable and configure default plain and\n"
            "  rich text placement.\n")
        },
        // WT_SETUP_PAGE_NUMBER_DISPLAY
        {
            QObject::tr(
            "  The following configuration setup is available for\n"
            "  page number display:\n"
            "  - Display Page Number: turn or off the page number display.\n")
        },
        // WT_SETUP_PAGE_NUMBER_PLACEMENT
        {
            QObject::tr(
            "  The following configuration setup is available for\n"
            "  page number placement:\n"
            "  - Alternate Corners: place the page number on alternate\n"
            "    corners - like books.\n"
            "  - Page Number Always in Same Place: page number placed\n"
            "    at fixed position.\n")
        },
        // WT_SETUP_PAGE_TEXT_PLACEMENT
        {
            QObject::tr(
            "  The following configuration setup is available for\n"
            "  page number display:\n"
            "  - Enable Text Placement: turn or off display\n"
            "    placement dialog when inserting text.\n\n"
            "  - Text Placement: set default text placement\n"
            "    using the placement dialog.\n")
        },
        // WT_SETUP_PAGE_PUBLISH
        {
            QObject::tr(
            "  Configure your instruction document\n"
            "  author, email address, Universal Resource Locator (URL)\n"
            "  publisher description, copyright, and image logo.\n")
        },
        // WT_SETUP_PART_BOM
        {
            QObject::tr(
            "  Configure and edit the Bill Of Material (BOM)\n"
            "  settings for your instructions document.\n"
            )
        },
        // WT_SETUP_PART_PLI
        {
            QObject::tr(
            "  Configure and edit the Part List Instance (PLI)\n"
            "  settings for your instructions document.\n")
        },
        // WT_SETUP_PART_BACKGROUND_BORDER
        {
            QObject::tr(
            "  Configure your instruction document part list\n"
            "  display, placement, background, border, margins\n"
            "  and size constrain settings.\n")
        },
        // WT_SETUP_PART_BACKGROUND_BORDER_PARTS_LIST
        {
            QObject::tr(
            "  The following configuration setup are available for\n"
            "  part list:\n"
            "  - Show Parts List: turn or off the parts list display.\n\n"
            "  - Parts List Placement: change the parts list placement.\n")
        },
        // WT_SETUP_PART_CONTENTS
        {
            QObject::tr(
            "  The following configuration setup are available in\n"
            "  the Part List Content dialog:\n"
            "  - Configure the Part properties.\n\n"
            "  - Configure part Sorting properties.\n")
        },
        // WT_SETUP_PART_PARTS
        {
            QObject::tr(
            "  Configure your instruction document part images\n"
            "  default part orientaion, part groups, and part\n"
            "  count settings.\n")
        },
        // WT_SETUP_PART_PARTS_ORIENTATION
        {
            QObject::tr(
            "  The following configuration setup are available for\n"
            "  part orientation.\n"
            "  - Configure the camera field of view.\n\n"
            "  - Configure the camera angles or viewpoint.\n")
            .arg(MetaDefaults::getPreferredRenderer() == QLatin1String("Native") ?
            QObject::tr("\n"
            "  - Configure the camera near plane.\n\n"
            "  - Configure the camera far z plane.\n") :
            QObject::tr("\n"
            "  - Camera near and far z planes are disabled\n"
            "    when the preferred renderer is not Native.\n"))
        },
        // WT_SETUP_PART_PARTS_MOVABLE_GROUPS_BOM
        {
            QObject::tr(
            "  Set part image, instance count, annotation and\n"
            "  element id movable. This means you can drag these\n"
            "  items to your desired position.\n\n"
            "  The following configuration setup are available for\n"
            "  Bill of Material (BOM) part groups:\n"
            "  - Movable Part Groups: turn or off movable property.\n")
        },
        // WT_SETUP_PART_PARTS_MOVABLE_GROUPS_PLI
        {
            QObject::tr(
            "  Set part image, instance count and part annotation\n"
            "  movable. This means you can drag these items to your\n"
            "  desired position.\n\n"
            "  The following configuration setup is available for\n"
            "  Part List Instance (PLI) part groups:\n"
            "  - Movable Part Groups: turn or off movable property.\n")
        },
        // WT_SETUP_PART_MORE_OPTIONS
        {
            QObject::tr(
            "  Configure your instruction document part sort\n"
            "  order, sort direction, stud style, edge color\n"
            "  automation and submodel display settings.\n")
        },
        // WT_SETUP_PART_MORE_OPTIONS_SHOW_SUBMODELS
        {
            QObject::tr(
            "  The following configuration setup is available for\n"
            "  submodels:\n"
            "  - Show in Parts List: show submodel in the parts\n"
            "    list instance (PLI).\n")
        },
        // WT_SETUP_PART_ANNOTATION
        {
            QObject::tr(
            "  The following configuration setup are available for\n"
            "  part list annotations:\n"
            "  - Configure the Annotation Options properties.\n\n"
            "  - Configure the Annotation Styles properties.\n")
        },
        // WT_SETUP_PART_ANNOTATION_OPTIONS
        {
            QObject::tr(
            "  Configure your instruction document part annotation\n"
            "  display, enable annotation types and annotation text\n"
            "  format settings.\n")
        },
        // WT_SETUP_PART_ANNOTATION_STYLE_BOM
        {
            QObject::tr(
            "  Configure your instruction document bill of materials\n"
            "  part annotation display, annotation types,\n"
            "  element identifiers and annotation text format\n"
            "  settings.\n")
        },
        // WT_SETUP_PART_ANNOTATION_STYLE_PLI
        {
            QObject::tr(
            "  Configure your instruction document part list instance\n"
            "  part annotation display, enable annotation types,\n"
            "  and annotation text format settings.\n")
        },
        // WT_SETUP_PART_ANNOTATION_STYLE_SELECT_BOM
        {
            QObject::tr(
            "  Edit your instruction document bill of materials (BOM)\n"
            "  part annotation styles, annotation background,\n"
            "  element identifiers and annotation text format\n"
            "  settings.\n\n"
            "  Edit annotation style:\n"
            "  Select the style to edit by clicking the radio\n"
            "  button of the following styles:\n"
            "  - None: No style selected.\n\n"
            "  - Square: square style used by beams, cables,\n"
            "    connectors, and hoses.\n\n"
            "  - Circle: circle style used by axles and panels.\n\n"
            "  - Rectangle: rectangular sytle used by extended\n"
            "    title and/or free form part annotations.\n\n"
            "  - Element: rectangular sytle used by element\n"
            "    identifiers.\n")
        },
        // WT_SETUP_PART_ANNOTATION_STYLE_SELECT_PLI
        {
            QObject::tr(
            "  Edit your instruction document part list instance (PLI)\n"
            "  part annotation styles, annotation background,\n"
            "  annotation border, annotation text format and\n"
            "  annotation with fixed size settings.\n\n"
            "  Edit annotation style:\n"
            "  Select the style to edit by clicking the radio\n"
            "  button of the following styles:\n"
            "  - None: No style selected.\n\n"
            "  - Square: square style used by beams, cables,\n"
            "    connectors, and hoses.\n\n"
            "  - Circle: circle style used by axles and panels.\n\n"
            "  - Rectangle: rectangular sytle used by extended\n"
            "    title and/or free form part annotations.\n")
        },
        // WT_SETUP_PROJECT
        {
            QObject::tr(
            "  Configure and edit project settings for your\n"
            "  instructions document.\n\n"
            "  The following configuration setup are available in\n"
            "  the project global setup:\n"
            "  - Render options\n"
            "  - Parse options\n")
        },
        // WT_SETUP_PROJECT_RENDERER_OPTIONS
        {
            QObject::tr(
            "  Configure and edit renderer settings\n"
            "  for your instructions document.\n\n"
            "  Edit your instruction document preferred\n"
            "  renderer, resolution, stud style and viewer\n"
            "  behaviour on cover page settings.\n")
        },
        // WT_SETUP_PROJECT_PARSE_OPTIONS
        {
            QObject::tr(
            "  Configure and edit parse LDraw file settings\n"
            "  for your instructions document.\n\n"
            "  Edit your instruction document build modifications,\n"
            "  buffer exchange, submodel instances behaviour, step\n"
            "  number, unofficial parts behaviour and start numbers\n"
            "  behaviour.\n")
        },
        // WT_SETUP_PROJECT_PARSE_BUFFER_EXCHANGE
        {
            QObject::tr(
            "  The following configuration setup is available for\n"
            "  buffer exchange:\n"
            "  - Parse Single Step With NOSTEP And BUFEXCHG Commands:\n"
            "    Parse single steps containing NOSTEP and BUFEXCHG\n"
            "    commands. Multi-step groups are automatically parsed.\n\n"
            "    Typically, a step with a NOSTEP command is not parsed.\n"
            "    However, bufexchg commands are not properly parsed if\n"
            "    the bufexchg command shares a step with the nostep\n"
            "    command and the step is not parsed.\n")
        },
        // WT_SETUP_PROJECT_START_NUMBERS
        {
            QObject::tr(
            "  The following configuration setup are available\n"
            "  for your loaded instruction document start number:\n"
            "  - Step Number: set the start number for the first\n"
            "    step of the loaded instruction document.\n\n"
            "  - Page Number: set the start number for the first\n"
            "    page of the loaded instruction document.\n\n"
            "  The mininimum allowed spin box value is 0 and\n"
            "  the maximum is 10000.\n"
            "  Spin box values are incremented by 1 step unit.\n\n"
            "  Setting the start number is useful when editing\n"
            "  large model instruction documents. You can use this\n"
            "  feature to split the instruction document into more\n"
            "  than one book - starting each book at it's\n"
            "  corresponding step and/or page number.\n")
        },
        // WT_SETUP_SUBMODEL
        {
            QObject::tr(
            "  Configure and edit the submodel preview\n"
            "  settings for your instructions document.\n")
        },
        // WT_SETUP_SUBMODEL_SUBMODEL
        {
            QObject::tr(
            "  The following configuration setup are available in\n"
            "  the Submodel dialog:\n"
            "  - Configure the Preview properties.\n"
            "  - Configure the Settings properties.\n")
        },
        // WT_SETUP_SUBMODEL_PREVIEW
        {
            QObject::tr(
            "  Configure your instruction document submodel\n"
            "  preview options.\n")
        },
        // WT_SETUP_SUBMODEL_BACKGROUND
        {
            QObject::tr(
            "  Configure your instruction document submodel\n"
            "  background, border and margins settings.\n")
        },
        // WT_SETUP_SUBMODEL_CONTENTS
        {
            QObject::tr(
            "  The following configuration setup are available in\n"
            "  the Contents dialog:\n"
            "  - Configure the Image properties.\n"
            "  - Configure the More properties.\n")
        },
        // WT_SETUP_SUBMODEL_IMAGE
        {
            QObject::tr(
            "  Configure your instruction document submodel\n"
            "  image, submodel constrain, submodel orientation,\n"
            "  submodel and default step rotation settings.\n")
        },
        // WT_SETUP_SUBMODEL_INSTANCE_COUNT
        {
            QObject::tr(
            "  Configure your instruction document submodel\n"
            "  instance count and stud style and automate\n"
            "  edge color settings.\n")
        },
        // WT_SETUP_SHARED_IMAGE_SIZING
        {
            QObject::tr(
            "  The following configuration setup are available\n"
            "  for image sizing:\n"
            "  - Configure the image scale.\n\n"
            "  - Configure the image margins.\n")
        },
        // WT_SETUP_SHARED_MODEL_ORIENTATION
        {
            QObject::tr(
            "  The following configuration setup are available for\n"
            "  assembly orientation:\n"
            "  - Configure the camera field of view.\n\n"
            "  - Configure the camera angles or viewpoint.\n")
            .arg(MetaDefaults::getPreferredRenderer() == QLatin1String("Native") ?
            QObject::tr("\n"
            "  - Configure the camera near plane.\n\n"
            "  - Configure the camera far z plane.\n") :
            QObject::tr("\n"
            "  - Camera near and far z planes are disabled\n"
            "    when the preferred renderer is not Native.\n"))
        },
        // WT_SETUP_SHARED_POINTERS
        {
            QObject::tr(
            "  The following configuration setup are available for\n"
            "  pointers:\n"
            "  - Configure the pointer border.\n"
            "  - Configure the pointer line.\n"
            "  - Configure the pointer tip.\n")
        },
        // WT_SETUP_SHARED_SUBMODEL_LEVEL_COLORS
        {
            QObject::tr(
            "  Configure the submodel level background color.\n")
        },
        /*************************************
         *  EDITOR ENTRIES
         *************************************/
        // WT_COMMAND_EDIT
        {
            QObject::tr("  Add or edit LPub commands.\n")
        },
        // WT_COMMAND_EDIT_DET
        {
            QObject::tr("  Add or edit LPub commands in detached editor.\n")
        },
        // WT_COMMAND_PREFS
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_COMMANDS
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_COMMANDS_EDIT
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_COMMANDS_VIEW
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_PARAMS_EDIT
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_SHORTCUTS
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_SNIPPETS_EDIT
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_SNIPPETS_VIEW
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_TEXT_EDIT
        {
            QObject::tr("  To be defined...\n")
        },
        // WT_VISUAL_EDIT
        {
            QObject::tr("  To be defined...\n")
        },
        //*************************************
        //*  DIALOG ENTRIES
        //*************************************
        // WT_DIALOG_BACKGROUND
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_BOM_OPTION
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_BORDER
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_CAMERA_ANGLES **
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_CAMERA_FOV **
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_CONSTRAIN
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_EXPORT_PAGES
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_DIVIDER
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_DOUBLE_SPIN **
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_FADE_HIGHLIGHT
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_FIND_REPLACE
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_FLOAT **
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_FLOAT_PAIR **
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_GLOBAL_HIGHLIGHT_STEP
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_GRADIENT
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_LDRAW_COLOR
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_LDRAW_FILES_LOAD
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_LDRAW_SEARCH_DIR
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_LOCAL **
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_OPTION **
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_PAGE_ORIENTATION
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_PAGE_SIZE
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_PLACEMENT
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_PLI_ANNOTATION
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_PLI_SORT
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_POINTER_ATTRIB
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_POINTER_PLACEMENT
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_PREFERRED_RENDERER
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_RENDER
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_ROTATE_ICON_SIZE
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_ROT_STEP
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_SIZE_ANDORIENTATION
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_SUBMODEL_COLOR
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_SUBSTITUTE_PART
        {
           QObject::tr("  To be defined...\n")
        },
        // WT_DIALOG_UNITS
        {
           QObject::tr(
           "  Configure the respective dot unit value using\n"
           "  a decimal number in %1.\n\n"
           "  Dot unit options are dots per inch (DPI) or\n"
           "  dots per centimetre (DPC)\n\n"
           "  Dot units are defined in Project Global Setup.\n")
           .arg(MetaDefaults::getPreferredUnits()
        },
        //*************************************
        //*  METAGUI ENTRIES
        //*************************************
        // WT_GUI_SIZE_ORIENTATION
        {
            QObject::tr(
            "  Configure the page size and orientation settings.\n"
            "  - Size: set the page size using standard sizes like\n"
            "    A4, Letter etc... You can also specify the 'Custom'\n"
            "    size options along with its corresponding height and\n"
            "    width values.\n\n"
            "    The page size custom height and width unit values\n"
            "    are determined by the resolution units specified\n"
            "    in Project Global Setup.\n"
            "    Dot unit options are dots per inch (DPI) or\n"
            "    dots per centimetre (DPC)\n"
            "  - Orientation: set the page orientation to\n"
            "    portrait or landscape.\n")
        },
        // WT_GUI_BACKGROUND
        {
            QObject::tr(
            "  Configure the background.\n"
            "  - Background: set the background option from\n"
            "    the drop-down control:\n"
            "    - None (Transparent)\n"
            "    - Solid Color\n"
            "    - Gradient\n"
            "    - Picture\n"
            "    - Submodel Level Color.\n\n"
            "  - Change: select the background color from the\n"
            "    color picker if Solid Color option selected.\n\n"
            "  - Browse: set or select the image path if background\n"
            "    image option selected.\n\n"
            "  - Image Fill: select stretch or tile of the front\n"
            "    cover image.\n")
        },
        // WT_GUI_BACKGROUND_NO_IMAGE
        {
            QObject::tr(
            "  Configure the background.\n"
            "  - Background: set the background option from\n"
            "    the drop-down control:\n"
            "    - None (Transparent)\n"
            "    - Solid Color\n"
            "    - Gradient\n"
            "    - Submodel Level Color.\n\n"
            "  - Change: select the background color from the\n"
            "    color picker if Solid Color option selected.\n")
        },
        // WT_GUI_BORDER
        {
            QObject::tr(
            "  Configure the border.\n"
            "  - Border: set the border corner\n"
            "    square, round or borderless.\n\n"
            "  - Line: set the border line type.\n\n"
            "  - Color: select the border line color.\n\n"
            "  - Margins: set the border margins width\n"
            "    and height.\n"
            "    Enter margin values using a decimal number\n"
            "    in %1.\n"
            "    Dot units are defined in Project Global Setup.\n")
            .arg(MetaDefaults::getPreferredUnits())
        },
        // WT_GUI_CAMERA_ANGLES
        {
            QObject::tr(
            "  Configure the camera angles.\n"
            "  - Latitude: set the camera angle x-axis\n"
            "    in degrees.\n\n"
            "  - Longitude: set the camera angle y-axis\n"
            "    in degrees.\n\n"
            "  - Camera Viewpoint: pre-defined camera angle latitude\n"
            "    and longitude settings which offer the following\n"
            "    options in degrees:\n"
            "    - Front: set the camera view at 0 lat, 0 lon.\n"
            "    - Back: set the camera view at 0 lat, 180 lon.\n"
            "    - Top: set the camera view at 90 lat, 0 lon.\n"
            "    - Bottom: set the camera view at -90 lat, 0 lon.\n"
            "    - Left: set the camera view at 0 lat, 90 lon.\n"
            "    - Right: set the camera view at 0 lat, -90 lon.\n"
            "    - Home: set the camera view at 30 lat, 45 lon.\n"
            "      You can change the latitude and longitude angles\n"
            "      for Home camera view.\n"
            "    - Default: do not use Camera Viewpoint.\n\n"
            "  - Use Latitude And Longitude Angles: enable the\n"
            "    Latitude and Longitude dialogs to allow Home\n"
            "    camera viewpoint angle changes. This dialog is only\n"
            "    enabled when camera viewpoint Home is selected.\n\n"
            "    Camera angle value range is -360.0 to 360.0\n\n"
            "    Precision is one decimal place.\n\n"
            "    Selecting a Camera Viewpoint other that Default\n"
            "    will disable the individual Latitude and Longitude\n"
            "    camera angle dialogs.\n")
        },
        // WT_GUI_CAMERA_DEFAULT_DISTANCE_FACTOR
        {
            QObject::tr(
            "  Configure the camera default distance factor:\n"
            "  - Default Distance Factor: set the distance\n"
            "    factor using a decimal number.\n\n"
            "    The default distance factor value is used to\n"
            "    adjust the camera position nearer to or farther\n"
            "    away from the position of the assembly or part.\n"
            "    It is effectively a zoom capability intended to\n"
            "    allow the editor to tune the image size as desired.\n\n"
            "    The primary use case is to maintain the same size\n"
            "    image when changing your renderer to Native and  you\n"
            "    wish to maintain the image sizing from the previous\n"
            "    renderer.\n\n"
            "    This setting is ignored when the preferred renderer is\n"
            "    not Native. The current renderer is %1.\n\n"
            "    Camera distance factor value range is 1.0 to 100.0\n\n"
            "    Precision is two decimal places.\n")
            .arg(MetaDefaults::getPreferredRenderer())
        },
        // WT_GUI_CAMERA_FIELD_OF_VIEW
        {
            QObject::tr(
            "  Configure the camera field of view:\n"
            "  - Camera FOV: set the camera field of view\n"
            "    in degrees.\n%1%2")
            .arg(MetaDefaults::getPreferredRenderer() == QLatin1String("Native") ? QObject::tr("\n"
            "  - Configure the camera near plane.\n\n"
            "  - Configure the camera far z plane.\n") : QObject::tr("\n"
            "  - Camera near and far z planes are disabled\n"
            "    when the preferred renderer is not Native.\n"), QObject::tr("\n"
            "    The field of view value range is determined by the\n"
            "    current preferred renderer which is %1.\n"
            "    The current FOV range is %2 to %3\n")
            .arg(MetaDefaults::getPreferredRenderer())
            .arg(MetaDefaults::getFOVMinRange())
            .arg(MetaDefaults::getFOVMaxRange()))
        },
        // WT_GUI_CAMERA_NEAR_PLANE
        {
            QObject::tr(
            "  Configure the camera near Z plane:\n"
            "  - Camera Near Plane: set the camera near plane\n"
            "    using a decimal number.\n\n"
            "    The near Z plane dialog is only enabled for the\n"
            "    Native preferred renderer. The current renderer\n"
            "    is %1.\n")
            .arg(MetaDefaults::getPreferredRenderer())
        },
        // WT_GUI_CAMERA_FAR_PLANE
        {
            QObject::tr(
            "  Configure the camera far plane:\n"
            "  - Camera Far Plane: set the camera far plane\n"
            "    using a decimal number.\n\n"
            "    The far Z plane dialog is only enabled for the\n"
            "    Native preferred renderer. The current renderer\n"
            "    is %1.\n")
            .arg(MetaDefaults::getPreferredRenderer())
        },
        // WT_GUI_IMAGE_SCALE
        {
            QObject::tr(
            "  The following configuration setup are available\n"
            "  for image:\n"
            "  - Scale: set the image scale using a decimal\n"
            "    number.\n\n"
            "    The mininimum allowed spin box value is -10000.0\n"
            "    and the maximum is 10000.0.\n"
            "    Spin box values are incremented by 1 step unit.\n"
            "    Precision is two decimal places\n")
        },
        // WT_GUI_UNITS_MARGIN
        {
            QObject::tr(
            "  Configure the margins.\n"
            "  - L/R(Left/Right): set the margin width.\n\n"
            "  - T/B(Top/Bottom): set the margin height.\n\n"
            "  Enter margin values using a decimal number\n"
            "  in %1.\n"
            "  Dot units are defined in Project Global Setup.\n")
            .arg(MetaDefaults::getPreferredUnits())
        },
        // WT_GUI_HEADER_HEIGHT
        {
            QObject::tr(
            "  Configure the page header height using the\n"
            "  specified dot units.\n")
        },
        // WT_GUI_FOOTER_HEIGHT
        {
            QObject::tr(
            "  Configure the page footer height using the\n"
            "  specified dot units.\n")
        },
        // WT_GUI_POINTER_BORDER
        {
            QObject::tr(
            "  Configure the pointer border.\n"
            "  - Type: set the pointer border line type.\n"
            "  - Width: set the pointer border line width.\n\n"
            "  - Color: set the pointer border line color\n"
            "    using the color picker.\n"
            "  Enter Width using a decimal number in %1.\n"
            "  Dot units are defined in Project Global Setup.\n")
            .arg(MetaDefaults::getPreferredUnits())
        },
        // WT_GUI_POINTER_LINE
        {
            QObject::tr(
            "  Configure the pointer line.\n"
            "  - Type: set the pointer line type.\n"
            "  - Width: set the pointer line width.\n"
            "  - Color: set the pointer line color\n"
            "    using the color picker.\n"
            "  - Tip Visibility: set checked to show the\n"
            "    pointer tip.\n\n"
            "  Enter Width using a decimal number in %1.\n"
            "  Dot units are defined in Project Global Setup.\n")
            .arg(MetaDefaults::getPreferredUnits())
        },
        // WT_GUI_POINTER_TIP
        {
            QObject::tr(
            "  Configure the pointer tip.\n"
            "  - Tip Width: set the visible tip width.\n"
            "  - Tip Height: set the visible tip height.\n\n"
            "  Enter Width and Height using a decimal number\n"
            "  in %1.\n"
            "  Dot units are defined in Project Global Setup.\n")
            .arg(MetaDefaults::getPreferredUnits())
        },
        // WT_GUI_PAGE_ATTRIBUTE_TEXT
        {
            QObject::tr(
            "  Configure the document text properties.\n"
            "  - Section: select the available location to place\n"
            "    the text. Available locations may be cover page\n"
            "    front or cover page back, content page header\n"
            "    or content page footer. The selection\n"
            "    drop-down list will display availabe location(s)\n"
            "    for the selected text attribute.\n\n"
            "  - Placement: select where on the page to place\n"
            "    the text string.\n\n"
            "  - Font: select the text font.\n\n"
            "  - Color: select the text font color.\n\n"
            "  - Margins: set the text margin area width/height.\n"
            "    Enter margin values using a decimal number\n"
            "    in %1.\n"
            "    Dot units are defined in Project Global Setup.\n\n"
            "  - Content: enter or update the text.\n")
            .arg(MetaDefaults::getPreferredUnits())
        },
        // WT_GUI_PAGE_ATTRIBUTE_IMAGE
        {
            QObject::tr(
            "  Configure the document image properties.\n"
            "  - Section: select the available location to place\n"
            "    the image attributethe. Available locations may\n"
            "    be cover page ront or cover page back, content\n"
            "    page header or content page footer. The selection\n"
            "    drop-down list will display availabe location(s)\n"
            "    for the selected image.\n\n"
            "  - Placement: select where on the page to place\n"
            "    the image.\n\n"
            "  - Margins: set the width(Left/Right), height(Top/Bottom)\n"
            "    image margins.\n"
            "    Enter margin values using a decimal number\n"
            "    in %1.\n"
            "    Dot units are defined in Project Global Setup.\n\n"
            "  - Browse: set or select the image path.\n\n"
            "  - Scale: check to enable and configure scaling the image.\n\n"
            "  - Image Fill: select aspect (resize on image aspect ratio),\n"
            "    stretch or tile the image across the page background.\n")
            .arg(MetaDefaults::getPreferredUnits())
        },
        // WT_GUI_SUBMODEL_LEVEL_COLORS
        {
            QObject::tr(
            "  Configure the submodel level page background color.\n\n"
            "  There are four default submodel levels, each with\n"
            "  its own default colour.\n"
            "  - Change: select the desired color using the\n"
            "    color picker.\n")
        },
        // WT_GUI_CHECK_BOX
        {
            QObject::tr(
            "  Check to enable property and uncheck to disable.\n")
        },
        // WT_GUI_UNITS
        {
            QObject::tr(
            "  Configure the unit pair.\n"
            "  - Unit Value 0: set the first unit value.\n\n"
            "  - Unit Value 1: set the second unit value.\n\n"
            "    Enter margin values using a decimal number\n"
            "    in %1.\n"
            "    Dot units are defined in Project Global Setup.\n")
            .arg(MetaDefaults::getPreferredUnits())
        },
        // WT_GUI_NUMBER
        {
            QObject::tr(
            "  Configure the number properties.\n"
            "  - Font: select the number font.\n\n"
            "  - Color: select the number font color.\n\n"
            "  - Margins: set the number margin area width/height.\n\n"
            "    Enter margin values using a decimal number\n"
            "    in %1.\n"
            "    Dot units are defined in Project Global Setup.\n")
            .arg(MetaDefaults::getPreferredUnits())
        },
        // WT_GUI_BOOL_RADIO_BUTTON
        {
            QObject::tr(
            "  Select between the pair of option alternatives.\n"
            "  The selected option is set to true while the\n"
            "  unselected option is false.\n")
        },
        // WT_GUI_PAGE_SIZE
        {
            QObject::tr(
            "PageSizeGui SHOULD NOT BE USED\n"
            "  - Use SizeAndOrientationGui.\n\n"
            "  Configure the page size settings.\n"
            "  - Size: set the page size using standard sizes like\n"
            "    A4, Letter etc... You can also specify the 'Custom'\n"
            "    size options along with its corresponding height and\n"
            "    width values.\n\n"
            "    The page size custom height and width unit values\n"
            "    are determined by the resolution units specified\n"
            "    in Project Global Setup.\n"
            "    Dot unit options are dots per inch (DPI) or\n"
            "    dots per centimetre (DPC)\n")
        },
        // WT_GUI_PAGE_ORIENTATION
        {
            QObject::tr(
            "PageOrientationGui SHOULD NOT BE USED.\n"
            "  - Use SizeAndOrientationGui.\n\n"
            "  Configure the page orientation settings.\n"
            "  - Portrait: set the page orientation to\n"
            "    portrait.\n\n"
            "  - Landscape: set the page orientation to\n"
            "    landscape.\n")
        },
        // WT_GUI_CONSTRAIN
        {
            QObject::tr(
            "  Configure the area constrain properties.\n"
            "  Select the the constrain type from the\n"
            "  drop-down list and use the line edit dialog\n"
            "  to enter the corresponding constraint type units.\n\n"
            "  Available types are:\n"
            "  - Area: Apply constraint using specified area.\n\n"
            "  - Square: Apply constraint using quadrilateral\n"
            "    of equal-length sides.\n\n"
            "  - Width: Apply constraint on specified width.\n\n"
            "  - Height: Apply constraint on specified height.\n\n"
            "  - Columns: Apply constraint on specified number\n"
            "    of comumns.\n")
        },
        // WT_GUI_DOUBLE_SPIN
        {
            QObject::tr(
            "  Configure a floating point number with double\n"
            "  precision accuracy using a spinbox control.\n"
            "  Available input properties are:\n"
            "  - Range: the minimum and maximum integer values \n"
            "    accepted as input.\n\n"
            "  - SingleStep: the step by which the integer value is\n"
            "    increased and decreased.\n\n"
            "  - Decimal Places: the precision of the double spinbox\n"
            "    in decimal format.\n\n"
            "  - Value: the current double precision floating point\n"
            "    number accepted as input.\n")
        },
        // WT_GUI_SPIN
        {
            QObject::tr(
            "  Configure an integer number using a spinbox control.\n"
            "  Available input properties are:\n"
            "  - Range: the minimum and maximum integer values \n"
            "    accepted as input.\n\n"
            "  - SingleStep: the step by which the integer value is\n"
            "    increased and decreased.\n\n"
            "  - Value: the current integer number accepted as input.\n")
        },
        // WT_GUI_FLOATS
        {
            QObject::tr(
            "  Configure a pair of floating point (decimal) numbers\n"
            "  with double precision.\n\n"
            "  Three decimal places (e.g. 1.012) is a good example.\n")
        },
        // WT_GUI_PART_SORT
        {
            QObject::tr(
            "PliSort SHOULD NOT BE USED.\n"
            "  - Use PliSortOrderGui.\n\n"
            "  - Sort Attributes: part size, colorcategory.\n\n"
            "  - Direction: direction are ascending - from small\n"
            "    to large\n")
        },
        // WT_GUI_PART_SORT_ORDER
        {
            QObject::tr(
            "  Configure the sort order and direction properties.\n"
            "  - Sort Attributes: part color, category\n"
            "    size and no-sort are available in each\n"
            "    of the three ordered drop-down controls.\n\n"
            "  - Sort Order: primary (first), secondary (second)\n"
            "    and tertiary (third) precedence in which the\n"
            "    attributes will be sorted.\n\n"
            "  - Direction: direction are ascending - from small\n"
            "    to large and descending from large to small.\n")
        },
        // WT_GUI_STUD_STYLE_AUTOMATE_EDGE_COLOR
        {
            QObject::tr(
            "  Configure the stud style and automate\n"
            "  edge color settings.\n\n"
            "  Stud style decorate studs with one of\n"
            "  seven (7) LEGO inscribed styles.\n\n"
            "  High contrast stud styles paint stud cylinder\n"
            "  and part edge colors.\n\n"
            "  Available stud styles:\n"
            "  - 0 Plain\n"
            "  - 1 Thin Line Logo\n"
            "  - 2 Outline Logo\n"
            "  - 3 Sharp Top Logo\n"
            "  - 4 Rounded Top Logo\n"
            "  - 5 Flattened Logo\n"
            "  - 6 High Contrast\n"
            "  - 7 High Contrast With Logo\n\n"
            "  Light/Dark Value triggers the part edge color\n"
            "  update for dark parts.\n\n"
            "  High Contrast styles enable part edge and\n"
            "  stud cylinder color settings.\n\n"
            "  Automate edge color settings allow %1 to\n"
            "  automatically adjust part edge colors.\n")
            .arg(VER_PRODUCTNAME_STR)
        },
        // WT_GUI_HIGH_CONTRAST_COLOR_SETTINGS
        {
            QObject::tr(
            "  High Contrast styles exposes the Light/Dark Value\n"
            "  edge color setting.\n\n"
            "  Light/Dark Value triggers the part edge color\n"
            "  update for dark parts.\n\n"
            "  High Contrast styles also exposes the following\n"
            "  part edge and stud cylinder color settings:\n"
            "  - Stud Cylinder Color: the applied high contrast\n"
            "    stud cylinder color\n\n"
            "  - Parts Edge Color: the default high contrast parts\n"
            "    edge color.\n\n"
            "  - Black Parts Edge Color: the applied color for\n"
            "    black parts.\n\n"
            "  - Dark Parts Edge Color: the applied edge color for\n"
            "    dark parts.\n")
        },
        // WT_GUI_AUTOMATE_EDGE_COLOR_SETTINGS
        {
            QObject::tr(
            "  Automate edge color settings allow %1 to\n"
            "  automatically adjust part edge colors based\n"
            "  on the following configured settings:\n"
            "  - Contrast: the amount of contrast.\n\n"
            "  - Saturation: the amount of edge color tint\n"
            "    or shade.\n")
            .arg(VER_PRODUCTNAME_STR)
        },
        // WT_GUI_PART_ANNOTATION_OPTIONS
        {
            QObject::tr(
            "  Part annotation options.\n"
            "  - Display PartList (PLI) Annotation: select\n"
            "    annotation source from Title, Free Form or\n"
            "    Fixed annotations.\n\n"
            "  - Enable Annotation Style: enable or disable\n"
            "    annotation for axles, beams, cables, connectors,\n"
            "    hoses, panels or custom annotations.\n\n"
            "  - Default Text Format: configure the default\n"
            "    annotation text font color and margins.\n")
        },
        // WT_GUI_PART_ANNOTATIONS_STYLES
        {
            QObject::tr(
            "  Part annotation styles.\n"
            "  - Select Style to Edit: display the corresponding\n"
            "    background, border, text and size for editing.\n"
            "    Choices include square, circle or rectangle.\n\n"
            "  - Background: edit the annotation background\n\n"
            "  - Border: edit the annotation border attributes\n\n"
            "  - Annotation Text Format: edit the annotation text\n"
            "    attributes\n\n"
            "  - Size: edit the annotation size attributes\n")
        },
        // WT_GUI_PART_ANNOTATIONS_SOURCE
        {
            QObject::tr(
            "  Enable part annotation:\n"
            "  - Display: show part annotations.\n\n"
            "  Select part annotation source:\n"
            "  - Title: use the title annotation source file.\n"
            "    Title annotations are derived from part\n"
            "    descriptions taken the LDraw parts library.\n\n"
            "  - Free Form: use the free form annotation source\n"
            "    file.\n"
            "    The above options allows the editor to define\n"
            "    custom part annotations and use the 'Extended'\n"
            "    annotation type.\n\n"
            "  - Fixed: use the built-in annotations with fixed size\n"
            "    properties. While the fixed size for square and\n"
            "    circle\n styles cannot be modified from the LDraw\n"
            "    file editor or main window, it is possible to change\n"
            "    the fixed size properties in the 'Styles' section of\n"
            "    this dialog.\n")
        },
        // WT_GUI_PART_ANNOTATIONS_TYPE_BOM
        {
            QObject::tr(
            "  Enable annotation type:\n"
            "  Select the bill of materials (BOM) annotations\n"
            "  types to display in the instruction document.\n\n"
            "  Available fixed annotation types for bill of\n"
            "  materials (BOM) parts are:\n"
            "  - Axles: annotation on circle background.\n\n"
            "  - Beams: annotation on square background.\n\n"
            "  - Cables: annotation on square background.\n\n"
            "  - Connections: annotation on square background.\n\n"
            "  - Hoses: annotation on square background.\n\n"
            "  - Panels: annotation on circle background.\n\n"
            "  - Element: LEGO element identifier on rectangular\n"
            "    background.\n\n"
            "  Available user configurable annotation types for\n"
            "  bill of materials (BOM) parts are:\n"
            "  - Extended: title and/or free form part annotations\n"
            "    on rectangular background.\n")
        },
        // WT_GUI_PART_ANNOTATIONS_TYPE_PLI
        {
            QObject::tr(
            "  Enable the display of part annotation:\n"
            "  Select the part list instance (PLI) annotation\n"
            "  types to display in the instruction document.\n\n"
            "  Available fixed annotation types for part list\n"
            "  instance (PLI) parts are:\n"
            "  - Axles: annotation on circle background.\n"
            "  - Beams: annotation on square background.\n"
            "  - Cables: annotation on square background.\n"
            "  - Connections: annotation on square background.\n"
            "  - Hoses: annotation on square background.\n"
            "  - Panels: annotation on circle background.\n\n"
            "  Available user configurable annotation types for\n"
            "  part list instance (PLI) parts are:\n"
            "  - Extended: title and/or free form part annotations\n"
            "    on rectangular background.\n")
        },
        // WT_GUI_PART_ELEMENTS_BOM
        {
            QObject::tr(
            "  Enable and configure part element annotation.\n"
            "  Bill of material part element identifiers can\n"
            "  be sourced from one of three files:\n"
            "  - Bricklink: brickline part element identifier\n"
            "    file.\n\n"
            "  - LEGO: Element identifiers sourced from the \n"
            "    LEGO group.\n\n"
            "  - Local: Local LEGO elements identifier file.\n")
        },
        // WT_GUI_BUILD_MODIFICATIONS
        {
            QObject::tr(
            "  Turn on or off the build modifications feature.\n\n"
            "  Build modifications allow you to modify your build\n"
            "  instructions steps to show additional 'instruction aids'\n"
            "  or to automate the positioning and camera angle of\n"
            "  existing parts within a step. This feature is a substitute\n"
            "  for the legacy MLCad BUFEXCHG framework.\n\n"
            "  You can add a modification command in any step with parts\n"
            "  and remove (or re-apply) the modification in any other\n"
            "  subsequent step - regardless of the subsequent step's\n"
            "  submodel or if the step is in a called out submodel.\n\n"
            "  Build modification commands can include any number or\n"
            "  configuration of existing or additional parts desired\n"
            "  to demonstrate the editor's build instruction intent.\n\n"
            "  Check the box to enable this feature or uncheck to disable.\n")
        },
        // WT_GUI_CONTINUOUS_STEP_NUMBERS
        {
            QObject::tr(
            "  Turn on or off continuous step numbers feature.\n\n"
            "  Continuous step numbers allow you to continuously\n"
            "  number your instruction steps across submodels and\n"
            "  unassembled callouts.\n\n"
            "  When not enabled, %1 will restart step numbers for\n"
            "  submodels and unassembled callouts.\n Numbering in\n"
            "  the parent submodel is conontinued after the callout\n"
            "  or submodel steps are completed.\n\n"
            "  Check the box to enable this feature or uncheck to\n"
            "  disable.\n")
        },
        // WT_GUI_CONSOLIDATE_SUBMODEL_INSTANCE_COUNT
        {
            QObject::tr(
            "  Turn on or off the consolidate submodel instance\n"
            "  count feature.\n\n"
            "  Consolidate submodel instance count will calculate\n"
            "  the number of instances at the first occurrence of\n"
            "  the submodel.\n\n"
            "  Check the box to enable this feature or uncheck to\n"
            "  disable.\n\n"
            "  Select where to consolidate the submodel instance\n"
            "  count.\n\n"
            "  You can consolidate the submodel instance count with\n"
            "  the following options:\n"
            "  - At Top: display count at last step page of first\n"
            "    occurrence in the entire model file.\n\n"
            "  - At Model: display count at last step page of first\n"
            "    occurrence in the parent model. This is the default\n"
            "    behaviour.\n\n"
            "  - At Step: display count at step page of first\n"
            "    occurrence in the respective step.\n")
        },
        // WT_GUI_MODEL_VIEW_ON_COVER_PAGE
        {
            QObject::tr(
            "  Turn on or off the view top model in the visual viewer\n"
            "  when the current page is a cover page feature.\n\n"
            "  You can set %1 to display the complete instruction\n"
            "  document model in the visual editor when the main\n"
            "  window is currently displaying a front or back cover\n\n"
            "  page.\n\n"
            "  Check the box to enable this feature or uncheck to\n"
            "  disable.\n")
        },
        // WT_GUI_LOAD_UNOFFICIAL_PART_IN_EDITOR
        {
            QObject::tr(
            "  Turn on or off loading unofficial parts in the command\n"
            "  editor.\n\n"
            "  You can choose to load and edit unofficial parts in the\n"
            "  command editor - effectively treating the part like a\n"
            "  submodel.\n\n"
            "  This feature is useful when you have inline unofficial\n"
            "  parts in you model file and would like to view and or\n"
            "  edit the part's content.\n\n"
            "  Check the box to enable this feature or uncheck to\n"
            "  disable.\n")
        },
        // WT_GUI_PREFERRED_RENDERER
        {
            QObject::tr(
            "  Select the default preferred image renderer.\n"
            "  You can use the drop-down control to select from\n"
            "  the following %1 packaged renderer options:\n"
            "  - Native: %1 'native' renderer and 3D Viewer,\n"
            "    offering performant, 'true fade' image rendering.\n\n"
            "  - LDGLite: fast image rendering ideal for proofing.\n\n"
            "  - LDView: high quality image rendering plus POV\n"
            "    scene file generation.\n\n"
            "  - POV-Ray: ray tracing, photo-realistic, feature rich\n"
            "    image rendering.\n\n"
            "  Selecting the LDView renderer enables the following\n"
            "  additional performance-oriented options:\n"
            "  - Use LDView Single Call: submit all the ldraw files\n"
            "    (parts, assembly, submodel preview) for the current\n"
            "    page in a single LDView render request instead of\n"
            "    individually at each step.\n\n"
            "  - Use LDView Snapshot List: this option extends and\n"
            "    requires LDView Single Call whereby a list files\n"
            "    for ldraw part or assembly files are created and\n"
            "    submitted to LDView.\n\n"
            "    As only one submodel preview per page is generated,\n"
            "    these options are ignored for submodel preview images.\n"
            "    Additionally, when there is less than 2 image files\n"
            "    for a page's parts list or step group, these options\n"
            "    are ignored.\n\n"
            "  The POV-Ray renderer requires a POV input file which is\n"
            "  generated by the following selected option:\n"
            "  - Native: an %1 imbeded module which is built\n"
            "    from LDView source and performs POV file generation\n"
            "    among some other features.\n\n"
            "  - LDView: POV file generation is submitted to the %1\n"
            "    packaged instance of LDView just as normal image files.\n")
            .arg(VER_PRODUCTNAME_STR)
        },
        // WT_GUI_DOCUMENT_RESOLUTION
        {
            QObject::tr(
            "  Set the default instruction document dot resolution.\n\n"
            "  You can configure the document dot resolution with the\n"
            "  following unit options:\n"
            "  - Dots Per Inch: set the Units drop-down to this\n"
            "    value if you would like to use inches.\n\n"
            "  - Dots Per Centimetre: set the Units drop-down to this\n"
            "    value if you would like to use centimetres.\n\n"
            "  Use the dot resulution unit value dialog to set the\n"
            "  number of units. The default for Dots Per Inch is 150\n"
            "  which automatically changes to 381 when the units\n"
            "  drop-down is switched to Dots Per Centimetre.\n")
        },
        // WT_GUI_ASSEM_ANNOTATION_TYPE
        {
            QObject::tr(
            "  Enable the display of part annotation:\n"
            "  Select the Current Step Instance (CSI)\n"
            "  annotation types to display in the instruction\n"
            "  document.\n\n"
            "  Available fixed annotation types for CSI parts are:\n"
            "  - Axles: annotation on circle background.\n\n"
            "  - Beams: annotation on square background.\n\n"
            "  - Cables: annotation on square background.\n\n"
            "  - Connections: annotation on square background.\n\n"
            "  - Hoses: annotation on square background.\n\n"
            "  - Panels: annotation on circle background.\n\n"
            "  Available user configurable annotation types for\n"
            "  current step instance (CSI) parts are:\n"
            "  - Extended: title and/or free form part annotations\n"
            "    on rectangular background.\n")
        },
        // WT_GUI_FADE_PREVIOUS_STEPS
        {
            QObject::tr(
            "  Turn on or off fade previous steps globally.\n\n"
            "  You can configure fade previous steps with the\n"
            "  following options:\n"
            "  - Fade Color: set the fade color using the color\n"
            "    picker.\n\n"
            "  - Use Fade Color: faded content will be applied\n"
            "    with the specified Fade Color above.\n"
            "    When not checked, each part in the faded assembly\n"
            "    will display its assigned part color.\n\n"
            "  - Fade Opacity: sets the fade color opaqueness\n"
            "    between 0 and 100 percent where 0 is fully\n"
            "    transparent and 100 is fully opaque.\n")
        },
        // WT_GUI_HIGHLIGHT_CURRENT_STEP
        {
            QObject::tr(
            "  Turn on or off highlight current step globally.\n\n"
            "  You can configure highlight current step with the\n"
            "  following options:\n"
            "  - Highlight Color: set the highlight color using\n"
            "    the color picker.\n\n"
            "  - Line Width (LDGLite renderer only): set the\n"
            "    highlight line width.\n"
            "    The mininimum allowed spin box value is 0\n"
            "    and the maximum is 10.\n"
            "    Spin box values are incremented by 1 step\n"
            "    unit.\n")
        },
        // WT_GUI_FINAL_FADE_HIGHLIGHT_MODEL_STEP
        {
            QObject::tr(
            "  Turn on or off auto generating a final step displaying\n"
            "  the final model without highlight or fade content.\n\n"
            "  The 6 auto-generated command lines that define this step\n"
            "  are not persisted to your saved ldraw model file.\n\n"
            "  If you manually save the command 0 !LPUB INSERT MODEL\n"
            "  to your model file, when you subsequently open the file,\n"
            "  if neither fade previous steps nor highlight current step\n"
            "  is enabled, %1 will remove this command, and any\n"
            "  adjacent content containing the auto-generted commands\n"
            "  from the loaded working document.\n\n"
            "  Saving your content will consequently remove the final\n"
            "  model lines from your ldraw model file.\n\n"
            "  If you prefer to persist the final model step - i.e override\n"
            "  this behaviour - you can remove the auto-generated\n"
            "  comment lines and replace 0 !LPUB INSERT MODEL with:\n\n"
            "  0 !LPUB INSERT DISPLAY_MODEL\n\n"
            "  Check the box to enable this feature or uncheck to disable.\n")
            .arg(VER_PRODUCTNAME_STR)
        },
        // WT_GUI_SEPARATOR
        {
            QObject::tr(
            "  Configure the separator.\n"
            "  - Width: set the separator width as a floating\n"
            "    point number (decimal).\n\n"
            "  - Length: set the separator length value as required\n"
            "    by the following length options:\n"
            "    - Default: length is automatically calculated -\n"
            "      length value entry is disabled.\n\n"
            "    - Page: length is fixed to the page height minus\n"
            "      the page margins, header and footer - length\n"
            "      value entry is disabled.\n\n"
            "    - Custom: specify your desired spearator length\n"
            "      using a floating point (decimal) number.\n\n"
            "  - Color: select the separator line color using\n"
            "    the Change button to launch the color picker.\n\n"
            "  - Margins: set the border margins width\n"
            "    and height.\n"
            "    Enter width, length and margin values using\n"
            "    a decimal number in %1.\n"
            "    Dot units are defined in Project Global Setup.\n"
            "    Precision is four decimal places.\n")
            .arg(MetaDefaults::getPreferredUnits())
        },
        // WT_GUI_STEP_JUSTIFICATION
        {
            QObject::tr(
            "  Configure the step justification.\n"
            "  - Set Step Justification: - set the step justification\n"
            "    using the drop-down control to select from the\n"
            "    following options:\n"
            "    - Center: center the step horizontally and vertically.\n"
            "    - Center Horizontal: center the step horizontally.\n"
            "    - Center Vertical: center the step vertically.\n"
            "    - Left: position the step at the left border.\n"
            "      This is the default positioning.\n\n"
            "  - Spacing: set the spacing, using a floating point\n"
            "    number (decimal), between items when the step\n"
            "    is center justified.\n"
            "    Enter spacing value using a decimal number\n"
            "    in %1.\n"
            "    Dot units are defined in Project Global Setup.\n"
            "    Precision is four decimal places.\n")
            .arg(MetaDefaults::getPreferredUnits())
        },
        // WT_GUI_STEP_ROTATION
        {
            QObject::tr(
            "  Configure the default step rotation.\n"
            "  - Rotation: set the X, Y, and Z spin box values using\n"
            "    a floating point number (decimal)\n"
            "    The mininimum allowed spin box value is 0 and\n"
            "    the maximum is 360.0.\n"
            "    Spin box values are incremented by 1 step unit.\n"
            "    Precision is automatically calculated based on the.\n"
            "    input values.\n\n"
            "  - Transform: there are three transform options.\n"
            "    - ABS: set the input transform as absolute\n"
            "      positioning.\n"
            "    - REL: set the input transform relative to the\n"
            "      existing positioning.\n"
            "    - ADD: add input values to the existing positioning.\n")
        },
        // WT_GUI_SUBMODEL_PREVIEW_DISPLAY
        {
            QObject::tr(
            "  Configure the default submodel preview display\n"
            "  settings.\n"
            "  - Show submodel at first step: turn on or off\n"
            "    displaying a complete submodel image at the\n"
            "    first step of the submodel.\n\n"
            "  For each preview display option, you can select\n"
            "  one or both of the following persist settings:\n"
            "  - Set In Preferences: set the preview display option\n"
            "    in Preferences so that this option will apply\n"
            "    globally for all instruction documents.\n\n"
            "  - Add LPub Meta Command: add the corresponding\n"
            "    LPub meta command to instruction document.\n\n"
            "  Checking the show submodel at first step option\n"
            "  will enable the following submodel preview options.\n"
            "  - Show main model at first step: show the top model\n"
            "    the first step. This option is not typically\n"
            "    selected as the displayed model will be the same\n"
            "    as that on the instruction cover page.\n\n"
            "  - Show submodel in callout: show the called out\n"
            "    submodel in the first step of the callout. This\n"
            "    option is only enabled for unassembled callouts.\n\n"
            "  - Show submodel instance count: add the instance\n"
            "    count to the submodel preview display.\n\n"
            "  The submodel preview default placement can be set\n"
            "  using the following setting:\n"
            "  - Submodel Placement: change the default submodel\n"
            "    placement using the placement dialog.\n")
        }
        // WT_NUM_ENTRIES
    };

    std::copy(std::begin(WTData), std::end(WTData), std::begin(CommonMenus::WT));

    WT_ARRAY_SIZE_CHECK(CommonMenus::WT, WT_NUM_ENTRIES);
}
