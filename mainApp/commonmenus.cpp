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
  action->setIcon(QIcon(":/resources/reset_viewer_image.png"));

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

  formatted = QObject::tr("Refresh %1 %2:\n"
                          "  You can clear this %3 %4 and working LDraw (.ldr)\n"
                          "  cache files.\n") .arg(name, imagelabel, name.toLower(), imagelabel.toLower());
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
  action->setIcon(QIcon(":/resources/refreshsubmodelcache.png"));

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
        // WT_LPUB3D_PREFERENCES
        {
            QObject::tr(
            "  Configure and edit your %1\n"
            "  application settings and preferences.\n"
            "  Settings configured here are global - this is\n"
            "  to say they apply to any open model instruction\n"
            "  document.\n\n"
            "  To specify settings for a specific\n"
            "  instruction document, use the global settings\n"
            "  at Instruction, Build Instruction Setup\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_LPUB3D_PREFERENCES_DARK_THEME_COLORS
        {
            QObject::tr(
            "  Configure the %1 dark theme colors using the\n"
            "  color picker dialog.\n\n"
            "  Use the reset button accompanying each color\n"
            "  setting to restore the default color properties.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_LPUB3D_PREFERENCES_DEFAULT_THEME_COLORS
        {
            QObject::tr(
            "  Configure the %1 default theme colors using the\n"
            "  color picker dialog.\n\n"
            "  Use the reset button accompanying each color\n"
            "  setting to restore the default color properties.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_LPUB3D_PREFERENCES_GENERAL
        {
            QObject::tr(
            "  Configure %1 LDraw library path, alternative LDConfig\n"
            "  model file load, model step fade and highlight options,\n"
            "  part list instance (PLI) setings, and default options\n"
            "  for units, camara angles and dialog messages.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_LPUB3D_PREFERENCES_KEYBOARD_SHORTCUTS
        {
            QObject::tr(
            "  Configure and edit %1 keyboard shortcuts.\n"
            "  All editable keyboard shortcuts are presented\n"
            "  in the view table grouped by their parent dialog\n"
            "  menu and action category.\n\n"
            "  You can also export, import and reset your keyboard\n"
            "  shortcuts from this dialog.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_LPUB3D_PREFERENCES_LDGLITE
        {
            QObject::tr(
            "  View %1 LDGLite renderer setting.\n"
            "  By default LDGLite and automatically configured\n"
            "  at application startup using the %1 default\n"
            "  options.\n\n"
            "  When LDGLite is installed, the application path\n"
            "  is provided.\n\n"
            "  You can customize some LDGLite renderer options\n"
            "  from its INI configuration file using the application\n"
            "  menu action at Configuration, Edit Parameter Files...\n\n"
            "  Note that some settings will be overridden by LPub\n"
            "  meta commands in your loaded model file.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_LPUB3D_PREFERENCES_LDVIEW
        {
            QObject::tr(
            "  Configure %1 LDView renderer settings.\n"
            "  By default LDView and automatically configured\n"
            "  at application startup using the %1 default\n"
            "  options.\n\n"
            "  When LDView is installed, the application path\n"
            "  is provided.\n\n"
            "  You can customize additional LDView renderer options\n"
            "  from its INI configuration file using the application\n"
            "  menu action at Configuration, Edit Parameter Files...\n\n"
            "  Note that some settings will be overridden by LPub\n"
            "  meta commands in your loaded model file.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_LPUB3D_PREFERENCES_MESSAGES
        {
            QObject::tr(
            "  Configure and edit %1 message categories.\n"
            "  Messages are generaged at various pints of\n"
            "  parsing the LDraw model file as the editor\n"
            "  transitions from page to page.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_LPUB3D_PREFERENCES_NATIVE
        {
            QObject::tr(
            "  Configure %1 Native renderer rendering and default\n"
            "  camera settings.\n\n"
            "  The %1 Native renderer is a customized instance\n"
            "  of the popular LDraw model editor LeoCAD that has\n"
            "  been integrated into %1; hence, the similarity of\n"
            "  this preferences dialog.\n\n"
            "  The %1 3DViewer is a customized and repackaged\n"
            "  instance LeoCAD so renderer settings set here can\n"
            "  have an effect on the 3DViewer display also.\n\n"
            "  You can configure additional Native renderer settings\n"
            "  from the 3DViewer application menu action Preferences.\n\n"
            "  Note that some settings will be overridden by LPub\n"
            "  meta commands in your loaded model file.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_LPUB3D_PREFERENCES_OTHER
        {
            QObject::tr(
            "  Configure %1 application update settings and\n"
            "  view the current list of LDraw part and submodel\n"
            "  search directories.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_LPUB3D_PREFERENCES_POVRAY
        {
            QObject::tr(
            "  Configure %1 POV-Ray renderer settings.\n"
            "  By default POV-Ray and automatically configured\n"
            "  at application startup using the %1 default\n"
            "  options.\n\n"
            "  POV-Ray Quality options High, Medium and Low allows\n"
            "  you to select the level of tracing intensity to apply.\n"
            "  You can toggle on or off automatically cropping rendered\n"
            "  images\n"
            "  It is also possible to display a progress dialog that\n"
            "  show the "
            "  When POV-Ray is installed, the application path\n"
            "  is provided.\n\n"
            "  Additionally, when the LGEO library is available, the\n"
            "  library path is provided.\n\n"
            "  You can customize additional POV-Ray renderer options\n"
            "  from its INI configuration file using the application\n"
            "  menu action at Configuration, Edit Parameter Files...\n\n"
            "  Note that some settings will be overridden by LPub\n"
            "  meta commands in your loaded model file.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_LPUB3D_PREFERENCES_PUBLISHING
        {
            QObject::tr(
            "  Configure and edit %1 instruction document settings.\n"
            "  Set continuous page settings, page attribute display,\n"
            "  and page attribute values.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_LPUB3D_PREFERENCES_RENDERERS
        {
            QObject::tr(
            "  Configure and edit %1 built-in renderer settings.\n"
            "  - Native: %1 'native' renderer and 3D viewer adapted\n"
            "    from LeoCAD. The Native renderer is performant and\n"
            "    provides 'true fade' rendering which enable powerful,\n"
            "    high quality and complex model rendering options.\n\n"
            "  - LDView: This is a custom instance of LDView specially\n"
            "    adapted for %1 using the latest code base.\n"
            "    LDView offers high quality, rendering plus POV scene\n"
            "    file generation.\n\n"
            "  - POV-Ray: %1 ray tracing renderer offering feature\n"
            "    rich, photo-realistic rendering ideal for document\n"
            "    covers and final model or assembly displays.\n\n"
            "  - LDGLite: %1 legacy renderer written in 'c' offering\n"
            "    fast image rendering ideal for proofing.\n\n"
            "  The Renderers tab also present the preferred renderer,\n"
            "  projection and render process timeout settings.\n"
            "  Additonally, LSynth options for configurable parts are\n"
            "  configurable from this location.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_LPUB3D_PREFERENCES_SEARCH_DIRECTORIES
        {
            QObject::tr(
            "  View current %1 LDraw content search directories.\n"
            "  This dialog is read only. You can edit the search\n"
            "  directories from the Configuration menu using\n"
            "  the LDraw Search Directories action.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_LPUB3D_PREFERENCES_THEME_AND_LOGGING
        {
            QObject::tr(
            "  Configure and edit %1 theme and logging settins.\n\n"
            "  - Themes: Set the preferred theme and edit the\n"
            "    application theme and view scene (the view that\n"
            "    displays the generated instruction document page)\n"
            "    colors.\n\n"
            "  - Logging: Select the level of logging, the displayed\n"
            "    log attributes and the individual logging level as\n"
            "    desired.\n\n"
            "  Logging can be helpful in troubleshooting unexptected\n"
            "  application behaviour.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_LPUB3D_PREFERENCES_THEME_COLORS
        {
            QObject::tr(
            "  Configure and reset %1 dark and default\n"
            "  theme colors.\n\n"
            "  Use the Default button to display the\n"
            "  hexadecimal color code for each default\n"
            "  color.\n\n"
            "  Use the Reset button to reset all the theme\n"
            "  colors to their default setting.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_LPUB3D_PREFERENCES_UPDATES
        {
            QObject::tr(
            "  Configure and edit %1 application update settings.\n\n"
            "  Set check for update and application change log\n"
            "  preferences\n\n"
            "  View %1 release change details.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_VISUAL_PREFERENCES
        {
            QObject::tr(
            "  Configure and edit the 3D viewer preferences\n"
            "  and settings.\n\n"
            "  The %1 3DViewer is a customized and repackaged\n"
            "  instance of LeoCAD also used as the %1 Native\n"
            "  renderer so settings configured here can have an\n"
            "  effect on rendered images when the preferred renderer.\n"
            "  is set to Native.\n"
            "  Note that some settings will be overridden by LPub\n"
            "  meta commands in your loaded model file.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_VISUAL_PREFERENCES_CATEGORIES
        {
            QObject::tr(
            "  Configure and edit 3D viewer part categories used"
            "  in the parts lookup dialog.\n\n"
            "  You can also export, import and reset part categories\n"
            "  from this dialog.\n")
        },
        // WT_VISUAL_PREFERENCES_COLORS
        {
            QObject::tr(
            "  Configure and reset the 3D viewer default\n"
            "  backgroud, displayed objects, interface, base\n"
            "  grid and view sphere colors.\n")
        },
        // WT_VISUAL_PREFERENCES_GENERAL
        {
            QObject::tr(
            "  Configure the 3D viewer mouse sensitivity, timeline,\n"
            "  icons, page redraw prompt and official parts settings.\n\n"
            "  Additionally, you can view paths, theme and author\n"
            "  settings shared with %1.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_VISUAL_PREFERENCES_KEYBOARD_SHORTCUTS
        {
            QObject::tr(
            "  Configure keyboard shortcuts.\n"
            "  This tab should not be visible.\n")
        },
        // WT_VISUAL_PREFERENCES_MOUSE
        {
            QObject::tr(
            "  Configure mouse shortcuts.\n"
            "  This tab should not be visible.\n")
        },
        // WT_VISUAL_PREFERENCES_RENDERING
        {
            QObject::tr(
            "  Configure the 3D viewer rendering settings.\n\n"
            "  Renderer settings affect both the 3D viewer and\n"
            "  the %1 Native renderer.\n\n"
            "  Note that some settings will be overridden by LPub\n"
            "  meta commands in your loaded model file.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_VISUAL_PREFERENCES_USER_INTERFACE
        {
            QObject::tr(
            "  Configure the 3D viewer user interface, base grid\n"
            "  model view and part preview settings.\n")
        },
        // WT_VISUAL_PREFERENCES_VIEWS
        {
            QObject::tr(
            "  Configure the 3D viewer default user interface and\n"
            "  color settings.\n")
        },
        // WT_LDVIEW_PREFERENCES
        {
            QObject::tr(
            "  Configure or reset the LDView renderer default\n"
            "  settings.\n\n"
            "  Settings are grouped into five dialogs, general,\n"
            "  gemometry, effects, primitives, updates and\n"
            "  preference set.\n\n"
            "  - Reset: This resets all the General preferences to\n"
            "    their default values. As with all other preference\n"
            "    changes, you must hit OK or Apply for the reset\n"
            "    values to go into effect.\n")
        },
        // WT_LDVIEW_PREFERENCES_GENERAL
        {
            QObject::tr(
            "  Configure or reset the LDView antialiasing, color,\n"
            "  miscellaneous options, and default save directories,\n\n"
            "  As the LDView renderer is used in multiple functions,\n"
            "  - i.e. step renderer, html parts list generator and\n"
            "  POV file generator -, there are miltiple configuration\n"
            "  INI files, each defined to configure LDView for the\n"
            "  function specified\n The current INI file is displayed\n"
            "  at the bottom of this dialog.\n\n"
            "  - Reset: This resets all the General preferences to\n"
            "    their default values. As with all other preference\n"
            "    changes, you must hit OK or Apply for the reset\n"
            "    values to go into effect.\n\n"
            "  Note it is possible to configure settings that violate\n"
            "  the parameters needed to successfully render %1\n"
            "  images. Use caution when setting unfamiliar preferences.\n")
        },
        // WT_LDVIEW_PREFERENCES_GEOMETRY
        {
            QObject::tr(
            "  Configure or reset the LDView model, wireframe, edge\n"
            "  lines and back face culling (BFC) settings.\n\n"
            "  - Reset: This resets all the General preferences to\n"
            "    their default values. As with all other preference\n"
            "    changes, you must hit OK or Apply for the reset\n"
            "    values to go into effect.\n")
        },
        // WT_LDVIEW_PREFERENCES_EFFECTS
        {
            QObject::tr(
            "  Configure or reset the LDView lighting, stereo,\n"
            "  cutaway, transparency, shading and curve settings.\n\n"
            "  - Reset: This resets all the General preferences to\n"
            "    their default values. As with all other preference\n"
            "    changes, you must hit OK or Apply for the reset\n"
            "    values to go into effect.\n")
        },
        // WT_LDVIEW_PREFERENCES_PRIMITIVES
        {
            QObject::tr(
            "  Configure or reset the LDView primitive substitution\n"
            "  textures, stud style, edge color, stud quality and\n"
            "  high-resulution primitives settings.\n\n"
            "  - Reset: This resets all the General preferences to\n"
            "    their default values. As with all other preference\n"
            "    changes, you must hit OK or Apply for the reset\n"
            "    values to go into effect.\n")
        },
        // WT_LDVIEW_PREFERENCES_UPDATES
        {
            QObject::tr(
            "  Configure or reset the LDView internet proxy and\n"
            "  missing parts download settings.\n\n"
            "  - Reset: This resets all the General preferences to\n"
            "    their default values. As with all other preference\n"
            "    changes, you must hit OK or Apply for the reset\n"
            "    values to go into effect.\n")
        },
        // WT_LDVIEW_PREFERENCES_PREFERENCES_SET
        {
            QObject::tr(
            "  View the LDView current preference set.\n\n"
            "  Note that %1 automatically loads the preference set\n"
            "  configured for the specified function. For example,\n"
            "  only the Default preference set is loaded when\n"
            "  LDView is used as the preferred renderer while all\n"
            "  the pre-configured preference sets are loaded when\n"
            "  accessing the LDView preferences while POV-Ray is\n"
            "  speified as the preferred renderer.\n\n"
            "  Use caution when creating new preference sets which\n"
            "  is to say you should possess sufficient understanding\n"
            "  of this functionality and how it is used by %1 before\n"
            "  changing this configuration.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
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
            "    for unassembled callouts.\n")
        },
        // WT_SETUP_CALLOUT_CALLOUT_POINTERS
        {
            QObject::tr(
            "  Configure your instruction document callout\n"
            "  pointers:\n"
            "  ° Configure the pointer border.\n"
            "  ° Configure the pointer line.\n"
            "  ° Configure the pointer tip.\n")
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
            "  ° Configure the pointer border.\n"
            "  ° Configure the pointer line.\n"
            "  ° Configure the pointer tip.\n")
        },
        // WT_SETUP_CALLOUT_POINTERS
        {
            QObject::tr(
            "  The following configuration setup are available in\n"
            "  the callout Pointers dialog:\n"
            "  ° Configure the Callout Pointer properties.\n"
            "  ° Configure the Divider Pointer properties.\n")
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
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
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
            "  - Content: The %1 plug is not editable.\n").arg(QLatin1String(VER_PRODUCTNAME_STR))
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
            "  ° Configure the page size and orientation settings.\n\n"
            "  ° Configure the page background.\n\n"
            "  ° Enable and configure the page border.\n\n"
            "  ° Configure the page margins.\n\n"
            "  ° Configure the page header height.\n\n"
            "  ° Configure the page footer height.\n")
        },
        // WT_SETUP_PAGE_DISCLAIMER
        {
            QObject::tr(
            "  Configure your instruction document disclaimer"
            "  and display the %1 logo and 'plug' statement.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
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
            "  - Configure the camera field of view.\n\n%1"
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
        // WT_SETUP_PART_ANNOTATION_OPTIONS_WITH_TEXT_FORMAT
        {
            QObject::tr(
            "  Part annotation options.\n"
            "  - Display PartList (PLI) Annotation: select\n"
            "    annotation source from Title, Free Form or\n"
            "    Fixed annotations.\n\n"
            "  - Enable Annotation Type: enable or disable\n"
            "    annotation for axles, beams, cables, connectors,\n"
            "    hoses, panels or custom annotations.\n\n"
            "  - Default Text Format: configure the default\n"
            "    annotation text font color and margins.\n")
        },
        // WT_SETUP_PART_ANNOTATION_STYLE_PLI
        {
            QObject::tr(
            "  Configure your instruction document part list instance\n"
            "  part annotation display, enable annotation types,\n"
            "  and annotation text format settings.\n")
        },
        // WT_SETUP_PART_ANNOTATION_STYLE_BOM
        {
            QObject::tr(
            "  Configure your instruction document bill of materials\n"
            "  part annotation display, annotation types,\n"
            "  element identifiers and annotation text format\n"
            "  settings.\n")
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
            "  - Parse Single Step With NOSTEP Or BUFEXCHG Commands: \n"
            "    Parse single steps containing NOSTEP or BUFEXCHG\n"
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
            "  - Configure the camera field of view.\n%1"
            "  - Configure the camera angles or viewpoint.\n")
            .arg(MetaDefaults::getPreferredRenderer() == QLatin1String("Native") ?
            QObject::tr("\n"
            "  - Configure the camera near z plane.\n\n"
            "  - Configure the camera far z plane.\n") :
            QObject::tr("\n"
            "  - Camera near and far z planes are disabled\n\n"
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
        //*************************************
        //*  DIALOG ENTRIES
        //*************************************
        // WT_DIALOG_APPEND_PAGE_OPTION **
        {
            QObject::tr(
            "  Select the append page option.\n\n"
            "  Append page options determine where a new page will\n"
            "  be added to the instruction document.\n")
        },
        // WT_DIALOG_BACKGROUND
        {
            QObject::tr(
            "  Configure the background type, color,\n"
            "  image and image fill settings when available.\n")
        },
        // WT_DIALOG_BOM_OPTION
        {
            QObject::tr(
            "  Select the Bill of Material option.\n\n"
            "  Use item toolTip to see additional description.\n")
        },
        // WT_DIALOG_BORDER
        {
            QObject::tr(
            "  Configure the border type, line, line width, line color,\n"
            "  and margin when available.\n")
        },
        // WT_DIALOG_BUILD_MODIFICATIONS
        {
            QObject::tr(
            "  Edit build modifications.\n\n"
            "  You can apply, remove, change or delete\n"
            "  build modifications from this dialog.\n\n"
            "  Select the desired build modification and\n"
            "  click OK to perform the respective apply,"
            "  remove, or change (load) action.\n"
            "  Do not apply a build modification action that\n"
            "  is the same as the last action of the build\n"
            "  modification. Likewise, if you are adding\n"
            "  the first action after creating the build\n"
            "  modification, then the this action should\n"
            "  be the remove action.\n\n"
            "  If you are loading a build modification, you\n"
            "  will be able to edit the loaded modification\n"
            "  in the visual editor.\n")
        },
        // WT_DIALOG_CAMERA_ANGLES **
        {
            QObject::tr(
            "  Configure camera angle latitude, longitude\n"
            "  or camera viewpoint settings.\n")
        },
        // WT_DIALOG_CAMERA_FOV **
        {
            QObject::tr(
            "  Configure camera field of view (FOV), near and\n"
            "  far plane settings.\n\n"
            "  Camera near and far plane settings are only enabled\n"
            "  when the preferred renderer is set to Native.\n\n"
            "  The current preferred renderer is %1\n")
            .arg(MetaDefaults::getPreferredRenderer())
        },
        // WT_DIALOG_CAMERA_LAT_LON
        {
            QObject::tr(
            "  Configure the camera angles.\n"
            "  - Latitude: set the camera angle x-axis in degrees.\n\n"
            "  - Longitude: set the camera angle y-axis in degrees.\n\n"
            "  - Distance Factor: optionally, you can check this\n"
            "    to enable the default distance factor spin dialog\n"
            "    where you can change the default camera distance.\n"
            "  The Distance dialog is a read-only display of the\n"
            "  calculated camera distance. The calculated distance\n"
            "  uses the distance factor, default position, page width\n"
            "  image resolution, image scale and LDraw Unit (LDU)."
            )
        },
        // WT_DIALOG_CONSTRAIN
        {
            QObject::tr(
            "  Configure the size of an item using its constraint\n"
            "  type - area, square, width, height or columns.\n")
        },
        // WT_DIALOG_EDITOR_PREFERENCES
        {
            QObject::tr(
            "  Configure the meta command editor font, text decoration\n"
            "  buffered paging and selected item preferences.\n")
        },
        // WT_DIALOG_EXPORT_PAGES
        {
            QObject::tr(
            "  Configure the instruction document export options or\n"
            "  continuous page processing options.\n")
        },
        // WT_DIALOG_DIVIDER
        {
            QObject::tr(
            "  Configure and edit a placed divider width, length,\n"
            "  type, length value, color and margins.\n")
        },
        // WT_DIALOG_DOUBLE_SPIN **
        {
            QObject::tr(
            "  Set the value for this item using a decimal number.\n")
        },
        // WT_DIALOG_FADE_HIGHLIGHT
        {
            QObject::tr(
            "  Enable or disable fade previous steps and highlight\n"
            "  current step.\n\n"
            "  This dialog is only available when Fade Previous Steps\n"
            "  Setup and/or Highlight Current Step Setup is enabled.\n"
            "  Fade and highlight step Setup is configured from the\n"
            "  Configuration, Build Instruction Setup menu using\n"
            "  Fade Steps Setup and/or Highlight Step Setup menu\n"
            "  actions.\n")
        },
        // WT_DIALOG_FIND_REPLACE
        {
            QObject::tr(
            "  Find and replace text in the currently loaded\n"
            "  LDraw model file.\n")
        },
        // WT_DIALOG_FLOAT **
        {
            QObject::tr(
            "  Set the value for this item using a decimal number.\n")
        },
        // WT_DIALOG_FLOAT_PAIR **
        {
            QObject::tr(
            "  Set the value pair for this item using decimal\n"
            "  numbers.\n")
        },
        // WT_DIALOG_GRADIENT
        {
            QObject::tr(
            "  Configure background gradient color, type and spread\n"
            "  method settings.\n\n"
            "  Alternatively, you can chose from the three pre-defined\n"
            "  default gradient configurations.\n")
        },
        // WT_DIALOG_LDRAW_COLOR
        {
            QObject::tr(
            "  Set or change the LDraw color for the currently\n"
            "  selected item.\n")
        },
        // WT_DIALOG_LDRAW_FILES_LOAD
        {
            QObject::tr(
            "  View the file load status for the currently loaded\n"
            "  LDraw model file.\n\n"
            "  - Validated parts: the total number of parts validated\n"
            "    during the file load.\n\n"
            "  - Unique validated parts: the number of unique parts\n"
            "    validated during the file load.\n\n"
            "  - Elapsed time: the time taken to load the model file.\n\n"
            "  - Error - Missing Parts: parts encountered during the\n"
            "    file load that were not found in the official or\n"
            "    unofficial LDraw archive library, the currently\n"
            "    specified LDraw disc library or any of the specified\n"
            "    LDraw content search directories.\n\n"
            "  - Warning - Primitive Parts: primitive part encountered\n"
            "    primitive parts are not normally defined at top level.\n\n"
            "  - Warning - Sublevel Parts: sublevel part encountered\n"
            "    sublevel parts are not normally defined at top level.\n\n"
            "  Part list view: list of loaded parts - part number and\n"
            "  description (if not missing).\n")
        },
        // WT_DIALOG_LDRAW_SEARCH_DIRECTORIES
        {
            QObject::tr(
            "  Configure %1 LDraw content search directories.\n\n"
            "  You can edit, add, reset and position search directory\n"
            "  entries using this dialog.\n\n"
            "  These directories are searched for LDraw part and\n"
            "  model files during file load and when when rendering\n"
            "  images.\n\n"
            "  Directories enterend here are shared with the current\n"
            "  %1 preferred renderer.\n\n"
            "  For the native renderer and 3D viewer, parts in these\n"
            "  locations are can be automatically loaded into the %1\n"
            "  unofficial LDraw archive file if this option is enabled\n"
            "  at the Configuration menu using the Archive Unofficial\n"
            "  Parts On Launch menu action.\n\n"
            "  Alternatively, you can interctively update the\n"
            "  unofficial archive library from the Configuration menu\n"
            "  using the Archive Unofficial Parts menu action.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_DIALOG_LDVIEW_HTML_PART_LIST_EXPORT
        {
            QObject::tr(
            "  Configure html export display, part reference, and\n"
            "  column order settings.\n")
        },
        // WT_DIALOG_LDVIEW_POV_EXPORT_OPTIONS
        {
            QObject::tr(
            "  Configure POV file generation general, geometry,\n"
            "  pov geometry, lighting, pov lights, material\n"
            "  properties, transparent material properties,\n"
            "  rubber material properties, and chrome material\n"
            "  properties settings.\n\n"
            "  - Reset: This resets all the POV file generation\n"
            "    preferences to their default values. As with all other\n"
            "    preference changes, you must hit OK or Apply for the\n"
            "    reset values to go into effect.\n")
        },
        // WT_DIALOG_LOCAL **
        {
            QObject::tr(
            "  Select local or global scope for currently selected\n"
            "  meta command.\n\n"
            "  Selecting yes will constrain the current meta command\n"
            "  to the current step.\n")
        },
        // WT_DIALOG_LOOK_AT_TARGET_AND_STEP_ROTATION
        {
            QObject::tr(
            "  Configure the default 'look at' target\n"
            "  position and step rotation angles.\n")
        },
        // WT_DIALOG_META_COMMANDS
        {
            QObject::tr(
            "  Edit %1 meta command description and create command\n"
            "  snippets.\n\n"
            "  Use the command filter to efficiently filter the\n"
            "  command or snippet list.\n\n"
            "  You can insert/autocomplete individual meta commands\n"
            "  using Ctrl+E to select the desired command.\n\n"
            "  You can insert/autocomplete snippets using\n"
            "  Ctrl+Space to select the desired command or to\n"
            "  automatically insert the command after typing\n"
            "  the 'trigger.'\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_DIALOG_PAGE_ORIENTATION
        {
            QObject::tr(
            "  Configure page orientation choosing between\n"
            "  landscape and portrait.\n")
        },
        // WT_DIALOG_PAGE_SIZE
        {
            QObject::tr(
            "  Configure page width and height size settings.\n")
        },
        // WT_DIALOG_PLACEMENT
        {
            QObject::tr(
            "  Configure the desired placement setting for this item.\n")
        },
        // WT_DIALOG_PLI_ANNOTATION
        {
            QObject::tr(
            "  Configure dispaly annotation, annotation source and\n"
            "  annotation types to display.\n")
        },
        // WT_DIALOG_PART_SORT_ORDER
        {
            QObject::tr(
            "  Configure the part sort order and direction\n"
            "  settings.\n")
        },
        // WT_DIALOG_POINTER_ATTRIB
        {
            QObject::tr(
            "  Configure the pointer border, line and\n"
            "  optionally pointer tip if available.\n")
        },
        // WT_DIALOG_PREFERRED_RENDERER
        {
            QObject::tr(
            "  Configure the default preferred step image\n"
            "  renderer.\n"
            "  %1 has 4 built-in step image renderers. They\n"
            "  are Native, LDGLite, LDView and POV-Ray.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_DIALOG_BLENDER_RENDER
        {
            QObject::tr(
            "  Render the current step image usng Blender.\n\n"
            "  Configure the Blender single image render\n"
            "  settings and output.\n"
            "  - Browse: Change the name and or location of\n"
            "    the rendered image.\n"
            "  - Render: Render the image.\n"
            "  - Settings: launch the Blender settings dialog.\n"
            "  - Output: process output is not available for\n"
            "    the Blender render process.\n\n"
            "  If you reset the rendered image, the default settings\n"
            "  (except image width and height), will also be reset.\n")
        },
        // WT_DIALOG_BLENDER_RENDER_SETTINGS
        {
            QObject::tr(
            "  Configure the Blender single image render\n"
            "  settings.\n"
            "  Use the individual setting's tooltip to see\n"
            "  setting description.\n")
        },
        // WT_DIALOG_POVRAY_RENDER
        {
            QObject::tr(
            "  Render the current step image using POV-Ray.\n\n"
            "  Configure the POV-Ray single image render\n"
            "  settings and output.\n"
            "  - Browse: Change the name and or location of\n"
            "    the rendered image.\n"
            "  - Render: Render the image.\n"
            "  - Settings: launch the POV-Ray settings dialog.\n"
            "  - Output: view the render process standard output log.\n\n"
            "  If you reset the rendered image, the default settings\n"
            "  (except image width and height), will also be reset.\n")
        },
        // WT_DIALOG_POVRAY_RENDER_SETTINGS
        {
            QObject::tr(
            "  Configure the POV-Ray single image render\n"
            "  settings.\n\n"
            "  Use the individual setting's tooltip to see\n"
            "  setting description.\n")
        },
        // WT_DIALOG_ROTATE_ICON_SIZE
        {
            QObject::tr(
            "  Configure the rotate icon width and height\n"
            "  in %1.\n")
            .arg(MetaDefaults::getPreferredUnits())
        },
        // WT_DIALOG_STEP_ROTATION
        {
            QObject::tr(
            "  Configure the default step rotation.\n"
            "  Set the x-angle, y-angle and z-angle, which are \n"
            "  the individual rotation angles for the different\n"
            "  axes in degree (-360 to 360).\n\n"
            "  Set the step transformation using one of the three\n"
            "  options - relative, absolute or additive.\n")
        },
        // WT_DIALOG_SIZE_AND_ORIENTATION
        {
            QObject::tr(
            "  Configure the page size and orientation settings.\n\n"
            "  Page size settings use standard page sizes or\n"
            "  page height and width values.\n")
        },
        // WT_DIALOG_SUBMODEL_LEVEL_COLORS
        {
            QObject::tr(
            "  Configure the submodel level page background color.\n\n"
            "  These colors help differentiate the current submode\n"
            "  level being edited.\n")
        },
        // WT_DIALOG_SUBSTITUTE_PART
        {
            QObject::tr(
            "  Configure substitute part attributes\n"
            "  current part, primary part substitution\n"
            "  arguments, and extended part substitution\n"
            "  arguments.\n")
        },
        // WT_DIALOG_TEXT_EDIT
        {
            QObject::tr(
            "  Create and edit instruction document text entries.\n"
            "  - Plain text: plain text format.\n"
            "  - Rich text: hyper text markup language (html)\n"
            "    text format.\n")
        },
        // WT_DIALOG_UNITS
        {
            QObject::tr(
            "  Configure the dot unit values using a decimal\n"
            "  number in %1.\n\n"
            "  Dot unit options are dots per inch (DPI) or\n"
            "  dots per centimetre (DPC)\n\n"
            "  Dot units are defined in Project Global Setup.\n")
            .arg(MetaDefaults::getPreferredUnits())
        },
        // WT_DIALOG_VISUAL_CATEGORY_EDIT
        {
            QObject::tr(
            "  Edit the selected LDraw part category.\n\n"
            "  Part category attributes are:\n"
            "  - Name: you can change the category name.\n"
            "  - Keywords: add or edit category keyword(s).\n\n"
            "    Category keywords are taken from the LDraw\n"
            "    part description and are prefixed with '^%',\n"
            "    for example ^%Door.\n\n"
            "    When specifying multiple keyowrds, each keyword\n"
            "    entry must be delimited by ' | ', do observe the\n"
            "    single space padding so and example would be\n"
            "    ^%Door | ^%Window.\n")
        },
        // WT_DIALOG_VISUAL_CATEGORY_NEW
        {
            QObject::tr(
            "  Add an new LDraw part category.\n\n"
            "  Part category attributes are:\n"
            "  - Name: create a new category name.\n"
            "  - Keywords: add category keyword(s).\n\n"
            "    Category keywords are taken from the LDraw\n"
            "    part description and are prefixed with '^%',\n"
            "    for example ^%Door.\n\n"
            "    When specifying multiple keyowrds, each keyword\n"
            "    entry must be delimited by ' | ', do observe the\n"
            "    single space padding so and example would be\n"
            "    ^%Door | ^%Window.\n")
        },
        //*************************************
        //*  CONTROL ENTRIES
        //*************************************
        // WT_CONTROL_COMMANDS_EDIT
        {
            QObject::tr(
            "  You can edit each meta command description to reflect\n"
            "  content important to you. The default description is\n"
            "  presented to give you a starting point.\n\n"
            "  The following buttons will complete your command edit:\n"
            "  - Ok: accept all changes in the currnt session.\n\n"
            "  - Cancel: discard all changes in the current session.\n\n"
            "  Edited commands are saved to an external file which is\n"
            "  loaded at application startup.\n")
        },
        // WT_CONTROL_COMMANDS_FILTER
        {
            QObject::tr(
            "  You can filter the command list to efficiently\n"
            "  find your desired entry.\n\n"
            "  The filter dialog supports the following actions.\n"
            "  - Auto-complete: keyword suggenstions are presented\n"
            "    after the third entered character.\n"
            "    This option is on by default.\n\n"
            "  - Case Sensitive: respect upper and lower case entries\n"
            "    as unique.\n\n"
            "  - Fixed String: present entries matching the entered\n"
            "    string. This option is on by default.\n\n"
            "  - Regular Expression: present entries matching the\n"
            "    entered regular expression.\n\n"
            "  - Wildcard: present entries that satisfy the pattern\n"
            "    created with the wildcard symbol '*'.\n\n"
            "  Click the filter icon to expose the filer actions menu\n"
            "  which allow you to change the default filter actions.\n")
        },
        // WT_CONTROL_COMMANDS_RESET
        {
            QObject::tr(
            "  Return to the default meta command description.\n")
        },
        // WT_CONTROL_COMMANDS_UPDATE
        {
            QObject::tr(
            "  Use this button to enable the editor.\n"
            "  Your changes are automatically saved to the current\n"
            "  session and permanently saved when you accept your\n"
            "  changes by clicking the OK button.\n")
        },
        // WT_CONTROL_COMMANDS_VIEW
        {
            QObject::tr(
            "  This table view presents all %1 meta commands.\n\n"
            "  The description of a selected meta command is\n"
            "  displayed in the editor dialog.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_ALTERNATE_LDCONFIG
        {
            QObject::tr(
            "  You can define custom LDraw colors in an LDConfig file\n"
            "  and enter the path to that file here.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_APPLY_CAMERA_ANGLES
        {
            QObject::tr(
            "  When parts in an assembly, submodel or part list are\n"
            "  submitted to the renderer, the default latitude\n"
            "  and longitude camera angles are applied to properly\n"
            "  orient the rendered image.\n\n"
            "  You can choose to have the default camera angles\n"
            "  applied as follows:\n\n"
            "  - Locally: camera angles applied by %1 when the parts\n"
            "    are rotated and then submitted to the preferred\n"
            "    renderer.\n\n"
            "  - Renderer: camera angles applied by the the current\n"
            "    preferred renderer %2. This is to say the angles\n"
            "    are supplied to the renderer along with the parts.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR),
                 MetaDefaults::getPreferredRenderer())
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_ATTRIBUTES
        {
            QObject::tr(
            "  Configure document attributes and cover page display.\n\n"
            "  - Display all page attributes: when checked all %1\n"
            "    page attributes are enabled for display.\n\n"
            "    Page attributes are automatically configured from\n"
            "    entries in preferences and from the loaded LDraw\n"
            "    file.\n"
            "    Alternativly, you can interctively configure page\n"
            "    attributes from the Configuration menu using the\n"
            "    Build Instructions Setup menu action.\n\n"
            "    Page attributes are grouped by the the type of page\n"
            "    they are allowed to be displayed on. The two\n"
            "    types of pages are cover and content pages.\n\n"
            "    Page attributes (and default placement) by page type\n"
            "    are:\n\n"
            "    - Front Cover: \n"
            "      ° Logo (Bottom of page header)\n"
            "      ° Model Name (Top Left of Title)\n"
            "      ° Title (Left of Page - anchor attribute)\n"
            "      ° Author (Bottom Left of Title)\n"
            "      ° Part Count (Bottom Left of Title)\n"
            "      ° Model Description (Bottom Left of Parts)\n"
            "      ° Publisher Description (Bottom Left of\n"
            "          Model Description)\n"
            "      ° Cover Image (Center of page)\n\n"
            "    - Content Page: \n"
            "      ° URL (Top Left of page)\n"
            "      ° Email Address (Top Right of page)\n"
            "      ° Copyright (Bottom Left of page)\n"
            "      ° Author (Left Bottom of Page Number)\n\n"
            "    - Back Cover: \n"
            "      ° Logo (Bottom of page header)\n"
            "      ° Title (Center of page)\n"
            "      ° Author (Bottom of Title)\n"
            "      ° Copyright (Bottom of Author)\n"
            "      ° URL (Bottom of Copyright)\n"
            "      ° Email (Bottom of URL)\n"
            "      ° LEGO Disclaimer (Bottom of Email)\n"
            "      ° %1 Plug Text (Bottom of LEGO Disclaimer)\n"
            "      ° %1 Plug Logo (Bottom of Plug Text)\n\n"
            "  - Generage conver pages: if you have not manually\n"
            "    added cover page meta commands to your LDraw file,\n"
            "    you can automatically create front and back cover\n"
            "    pages by enabling this item.\n\n"
            "  - Generage PDF Table of Contents: this option is not\n"
            "    currently enabled in %1.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_CAMERA_DEFAULTS
        {
            QObject::tr(
            "  Configure the Native renderer and 3DViewer default\n"
            "  camera settings.\n\n"
            "  - Display properties: show or hide camera settings in\n"
            "    the 3D viewer Properties window. An item's properties\n"
            "    are displayed when it is selected.\n\n"
            "  - Field of View: the default Native renderer and\n"
            "    3D viewer FOV value.\n"
            "    Set the camera field of view in degrees.\n\n"
            "  - Near Plane: the default Native renderer and 3D viewer\n"
            "    near Z plane value.\n"
            "    Set the camera near plane using a decimal number.\n\n"
            "  - Far Plane: the default Native renderer and 3D viewer\n"
            "    far Z plane value.\n"
            "    Set the camera far plane using a decimal number.\n\n"
            "  - Default Distance Factor: used to adjust the camera\n"
            "    position nearer to or farther away from the position\n"
            "    of the assembly or part.\n"
            "    Set the distance factor using a decimal number.\n\n"
            "  - Default Camera Position: sets the initial position\n"
            "    of the camera relative to the part or assembly.\n"
            "    Set the default position using a decimal number.\n\n"
            "  - Camera Distance Factor: this is the calculated value\n"
            "    of the Default Camera Position divided by (Default\n"
            "    Distance Factor multiplied by negative one).\n\n"
            "  You can manipulate the Camera Distance Factor to set\n"
            "  your desired default 'Zoom' for Native rendered items\n"
            "  and items displayed in the 3D viewer.\n\n"
            "  Use the reset button (...) to restore the camera\n"
            "  defaults.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_CHANGE_LOG
        {
            QObject::tr(
            "  The change log allows you to view %1 release change\n"
            "  details stored online in the popular software\n"
            "  development and integration website GitHub.\n\n"
            "  Change log options are as follows:\n"
            "  - Download change log at startup: you can set %1 to\n"
            "    download the online instance of the change log. The\n"
            "    online log may have updates since after the installed\n"
            "    log which is loaded by default.\n"
            "    Note that there may be a material impact on the\n"
            "    application load time, depending on your internet\n"
            "    connect performance, when this option is enabled.\n"
            "  - Update Chnage Log: interctively trigger %1 to\n"
            "    download the latest online change log.\n\n"
            "  With an appropriate GitHub account, you can contribute\n"
            "  or ask questions relating to changes displayed in the\n"
            "  change log.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_DEFAULTS
        {
            QObject::tr(
            "  Configure %1 units, camera angles and LDraw file\n"
            "  parse message defaults.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_EDITOR
        {
            QObject::tr(
            "  Configure the default publisher page attributes.\n"
            "  - Default Author: set the instruction document author\n"
            "  - Default URL: set the universal resource locator (URL).\n"
            "  - Default Email: set the desired email address.\n\n"
            "  These attributes are part of the instruction document\n"
            "  page attributes.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_FADE_STEP
        {
            QObject::tr(
            "  Configure %1 fade previous steps default settings.\n\n"
            "  - Enable Fade Previous Steps: turn on or off this\n"
            "    feature.\n\n"
            "  - Use Parts Global Fade Color: turn on or off using the\n"
            "    specified fade color for all faded items.\n"
            "    When this option is enabled, use the dropdown list\n"
            "    select your desired global fade color.\n\n"
            "    When this option is not enabled, %1 will fade the\n"
            "    part's assigned color.\n\n"
            "    When the LDraw LEGO parts library is in use, the\n"
            "    global fade color list is loaded from the LDraw\n"
            "    parts LDConfig file.\n\n"
            "  - Fade Percent: set the fade opacity percent where 0\n"
            "    is fully opaque and 100 is fully transparent.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_GENERAL_SETTINGS
        {
            QObject::tr(
            "  Configure the following %1 general settings.\n\n"
            "  - Load the last opened file on startup: as the title\n"
            "    states, the last file opened in %1 is automatically\n"
            "    loaded when %1 is launched.\n\n"
            "  - Show load messages: set %1 to display a load status\n"
            "    dialog upon completion of LDraw file load.\n"
            "    The load status dialog can be set to display on the\n"
            "    following triggers:\n\n"
            "    * Never: do not display the load status dialog.\n\n"
            "    * On Error: display dialog when an error condition,\n"
            "      for example, when an invalid (missing) part is\n"
            "      encountered\n\n"
            "    * On Warning: display dialog when a warning condition,\n"
            "      for example, when a subpart or primitive part is\n"
            "      encountered.\n\n"
            "    * On Message: display dialog when either a warning or\n"
            "      an error condition is encountered.\n\n"
            "    * Always: display dialog each time an LDraw file is\n"
            "      loaded.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_HIGHLIGHT_STEP
        {
            QObject::tr(
            "  Configure %1 highlight current step default settings.\n\n"
            "  - Enable Highlight Current Step: turn on or off this\n"
            "    feature.\n\n"
            "  - Highlight first step: turn on or off highlighting the\n"
            "    first step in the in the instruction document.\n\n"
            "  - Highlight Color: use the dropdown list\n"
            "    select your desired highlight color using the Color\n"
            "    picker.\n%2")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
            .arg(MetaDefaults::getPreferredRenderer() == QLatin1String("LDGLite") ?
            QObject::tr("\n"
            "  - Highlight line width: set the highlight line width\n"
            "    using an integer number.\n") : QString())
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_IMAGE_MATTE
        {
            QObject::tr(
            "  This feature is not curently available.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_KEYBOARD_SHORTCUT
        {
            QObject::tr(
            "  You can configure a keyboard shortcut for the selected\n"
            "  menu action.\n\n"
            "  - Key sequence: enter your desired shortcut key\n"
            "    sequence.\n"
            "  - Assign: click this button to add our custom defined\n"
            "    shortcut to the selected menu action.\n\n"
            "  - Remove: You can remove an assigned custom shortcut using\n"
            "    the Remove button.\n\n"
            "  - Use the Reset button in the key sequence dialog to\n"
            "    restore the default menu action keyboard shortuct.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_KEYBOARD_SHORTCUT_TABLE
        {
            QObject::tr(
            "  View and select %1 menu action keyboard shortcuts.\n\n"
            "  - Table: all editable keyboard shortcuts are presented\n"
            "    in the view table grouped by their parent dialog\n"
            "    menu and action category.\n\n"
            "  - Filter: filter the shortcut table using menu action\n"
            "    name. The auto-complete functionality will\n"
            "    provide suggestions based on the characters you\n"
            "    enter.\n\n"
            "  - Import: import custom menu action shortucts.\n\n"
            "  - Export: export the entire list of shortcuts to\n"
            "    an external file.\n\n"
            "  - Reset: reload all menu actions and their default\n"
            "    keyboard shortcut.\n\n"
            "  Your shortcut changes are accepted when you click the\n"
            "  Ok button, conversely, clicking the Cancel button will\n"
            "  discard any custom keyboard shortcuts created during\n"
            "  session.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LDGLITE
        {
            QObject::tr(
            "  View the LDGLite renderer application path when this\n"
            "  renderer is installed. The LDGLite renderer is packaged\n"
            "  with %1 and is automatically installed.\n\n"
            "  By default LDGLite and automatically configured\n"
            "  at application startup using the %1 default\n"
            "  options.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LDGLITE_INSTALL
        {
            QObject::tr(
            "  In the event LDGLite is not installed or have been\n"
            "  removed. %1 will display this dialog to download and\n"
            "  install the LDGLite renderer.\n\n"
            "  Use the Install button to initiate the download and\n"
            "  installation.")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LDRAW_LIB_PATH
        {
            QObject::tr(
            "  Configure the path to the LDraw parts disc library.\n\n"
            "  %1 primarilly uses the official and unofficial LDraw\n"
            "  archive part libraries; however, as the LDView and\n"
            "  LDGLite 3rd party renderers do not recognize the\n"
            "  archive libraries, the traditional disc library is\n"
            "  also required.\n\n"
            "  You can use the Browse button to select the disc\n"
            "  library or interactively enter the path in the line\n"
            "  edit dialog.\n\n"
            "  %1 will attempt to validate the specified LDraw parts\n"
            "  library returning a 'Suspicious LDraw Directory message\n"
            "  when the library path is deemed to be invalid.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LDVIEW
        {
            QObject::tr(
            "  Configure the LDView renderer settings.\n"
            "  - Use Mulitple File Single Call: submit all the ldraw\n"
            "    files (parts, assembly, submodel preview) for the\n"
            "    current page in a single LDView render request\n"
            "    instead of individually at each step.\n\n"
            "  - Use Snapshots List File: this option extends and\n"
            "    requires LDView Single Call whereby a list files\n"
            "    for ldraw part or assembly files are created and\n"
            "    submitted to LDView.\n\n"
            "    As only one submodel preview per page is generated,\n"
            "    these options are ignored for submodel preview images.\n"
            "    Additionally, when there is less than 2 image files\n"
            "    for a page's parts list or step group, these options\n"
            "    are ignored.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LDVIEW_INSTALL
        {
            QObject::tr(
            "  In the event LDView is not installed or have been\n"
            "  removed. %1 will display this dialog to download and\n"
            "  install the LDView renderer.\n\n"
            "  Use the Install button to initiate the download and\n"
            "  installation.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LDVIEW_SETTINGS
        {
            QObject::tr(
            "  Configure LDView renderer general, gemometry, effects,\n"
            "  primitives, updates and preference set settings.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LOGGING
        {
            QObject::tr(
            "  Configure %1 logging settings.\n"
            "  - Path: you can configure the log file path and\n"
            "    file name.\n"
            "  - Line Attributes: select the log line attributes you\n"
            "    wish to see for log entry lines.\n"
            "  - Logging Level: set the logging level.\n"
            "  - Individual Log Level Messages: you can select your\n"
            "    desired log level messages.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LOG_LEVEL
        {
            QObject::tr(
            "  Select your desired %1 log messages.\n\n"
            "  - STATUS: display status messages (default level).\n"
            "  - INFO: log information messages.\n"
            "  - TRACE: log trace messages.\n"
            "  - DEBUG: log debug messages.\n"
            "  - NOTICE: log notice messages.\n"
            "  - ERROR: log error messages.\n"
            "  - FATAL: log fatal messages.\n"
            "  - OFF: do not log any messages.\n\n"
            "  Levels at and below the selected level are written to\n"
            "  the log file. For example, setting the level to DEBUG\n"
            "  will write DEBUG, NOTICE, ERROR, and FATAL messages to\n"
            "  the log file.\n\n"
            "  When this setting is configured. Individual Log\n"
            "  Level Messages settings are disabled.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LOG_LEVEL_MESSAGES
        {
            QObject::tr(
            "  You can configure your selection of individual log\n"
            "  level messages that will be written to the log file.\n\n"
            "  Select the log message(s) you wish to be written to\n"
            "  the log file.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LOG_LINE_ATTRIBUTES
        {
            QObject::tr(
            "  Configure the attributes you wish to see for log entry\n"
            "  lines.\n\n"
            "  - Logging Level: show log level, Fatal, Error, Status,\n"
            "    Trace, Debug, and Information.\n\n"
            "  - Timestamp: show the YYYY-MM-DDTH:M:S:MS timestamp.\n\n"
            "  - Line Number: show the file line number '@ln NNN'.\n\n"
            "  - Class Name: show the class name where the log entry\n"
            "    was triggered.\n\n"
            "  - Function: show the function where the log entry was\n"
            "    triggered.\n\n"
            "  - All attributes: show logging level, timestamp, line\n"
            "    number, class name, and function\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_LOG_PATH
        {
            QObject::tr(
            "  You can configure the log file path and file name.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_PRINT_STANDARD_OUT
        {
            QObject::tr(
            "  You can configure printing output messages to\n"
            "  standard output\n"
            "  To prevent double log entries, This setting is\n"
            "  usually enabled on Windows and disabled on Unix\n"
            "  when logging is enabled or when executing in\n"
            "  console mode.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_MESSAGES
        {
            QObject::tr(
            "  Error, warning and information messages are displayed\n"
            "  here.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_PAGE_PROCESSING_CONTINUOUS
        {
            QObject::tr(
            "  Configure %1 continuous page processing settings.\n\n"
            "  - Display page for n seconds: set the duration to\n"
            "    display each page when processing, or exporting\n"
            "    instruction document pages.\n\n"
            "  - Display options dialog: set and reset displaying\n"
            "    the continuous page processing and export dialog.\n\n"
            "    The continuous page processing and export dialog\n"
            "    offers the option to turn off displaying it.\n"
            "    When display is turned off, you can use this setting\n"
            "    to reset (restore) the continuous page processing and\n"
            "    export dialog display.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_PARSE_ERROR
        {
            QObject::tr(
            "  Select and clear %1 file parse messages.\n\n"
            "  When an LDraw file is being processed, encountered\n"
            "  parse errors are displayed to give the editor an\n"
            "  opportunity to perform correction.\n\n"
            "  Parse error messages are grouped by the type of error.\n\n"
            "  You can toggle on and off displaying each type of\n"
            "  parse error message.\n\n"
            "  When a message is displayed, the editor can elect to\n"
            "  not display the message in the future.\n"
            "  Messages set to not be displayed can be reset here by\n"
            "  clicking the enabled 'Clear' button.\n"
            "  You can clear individual message categories or clear\n"
            "  all message categories with a single action.\n"
            "  The 'Clear' button is enabled when there are messages\n"
            "  to be cleared.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_PLI_CONTROL
        {
            QObject::tr(
            "  Configure the part list instance (PLI) control file.\n"
            "  You can toggle on and off use of the PLI control file\n"
            "  and you can also specify its path path and file name.\n\n"
            "  The PLI control file is used to specify the default\n"
            "  orientation and size of parts in the Parts List and\n"
            "  Bill Of Material instances.\n\n"
            "  You can add and edit entries in the PLI control file\n"
            "  from the Configuration menu then Edit Parameter\n"
            "  Files... menu and the PLI Parts Control File action.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_POVRAY
        {
            QObject::tr(
            "  Configure %1 POV-Ray renderer settings.\n"
            "  - Quality: you can set render quality options High,\n"
            "    Medium and Low.\n\n"
            "  - Auto-crop Image: You can toggle on or off\n"
            "    automatically cropping rendered images.\n\n"
            "  - Display Image Progeress: show a progress dialog that\n"
            "    presents the rendered image at its state of\n"
            "    completeness.\n\n"
            "  - Path: show the POV-Ray application path and file name.\n\n"
            "  - POV-File Generation Renderer: select between Native or\n"
            "    LDView.\n\n"
            "  - POV File Generation Settings: configure POV generation\n"
            "    options and LDView preferences to fine tune the POV-Ray\n"
            "    rendered image.\n\n"
            "  - LGEO Is Available: when the LEGO Geometrical Equivalent\n"
            "    Objects (LGEO) library is available, this attribute is\n"
            "    enabled and the library path is provided.\n"
            "  - LGEO Stereolithography (STL) library is available: is\n"
            "    displayed when the LGEO STL library from Darat is\n"
            "    present.\n\n"
            "  You can customize additional POV-Ray renderer options\n"
            "  from its INI configuration file using the application\n"
            "  menu action at Configuration, Edit Parameter Files...\n\n"
            "  Note that some settings will be overridden by LPub\n"
            "  meta commands in your loaded model file.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_INSTALL
        {
            QObject::tr(
            "  In the event POV-Ray is not installed or have been\n"
            "  removed. %1 will display this dialog to download and\n"
            "  install the POV-Ray renderer.\n\n"
            "  Use the Install button to initiate the download and\n"
            "  installation.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_LDVIEW_POV_GENERATION
        {
            QObject::tr(
            "  You can choose between Native and LDView POV file\n"
            "  generation.\n\n"
            "  - Native: this is %1's integrated instance of the\n"
            "    LDView POV file generation behaviour.\n This option\n"
            "    offers a performance benefit over using the LDView\n"
            "    instance.\n\n"
            "  - LDView: %1 will pass the POV generation parameters to\n"
            "    the third party LDView instance. The LDView renderer\n"
            "    will then generate the POV file.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_LGEO
        {
            QObject::tr(
            "  Configure the LEGO® Geometrical Equivalent Objects\n"
            "  (LGEO) part library created by Lutz Uhlmann (El Lutzo),\n"
            "  updated by Owen Burgoyne (C3POwen) and extended with\n"
            "  Stereolithography STL object files by\n"
            "  Damien Roux (Darats).\n\n"
            "  - LGEO Is Available: when the LGEO part library is\n"
            "    available, this attribute is enabled.\n\n"
            "  - LGEO Stereolithography (STL) library is available\n"
            "    label is displayed when the STL library from\n"
            "    Damien Roux (Darats) is present.\n\n"
            "  - Path: when the LGEO part library is detected,\n"
            "    the library path is provided.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_POVRAY_NATIVE_POV_GENERATION
        {
            QObject::tr(
            "  Configure the POV generation and LDView renderer\n"
            "  preference settings for POV file generation.\n\n"
            "  POV generation options are grouped into general,\n"
            "  geometry, pov geometry, lighting, pov lights, material\n"
            "  properties, transparent material properties,\n"
            "  rubber material properties, and chrome material\n"
            "  properties settings.\n\n"
            "  Configure or reset the LDView renderer settings for\n"
            "  POV file generation.\n Settings are grouped into\n"
            "  general, gemometry, effects, primitives,\n"
            "  updates and preference set.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_PUBLISH_DESCRIPTION
        {
            QObject::tr(
            "  Configure the default publisher description page\n"
            "  attribute.\n\n"
            "  - Default Publish Description: set the instruction\n"
            "    document publish description. This value is\n"
            "    automatically converted from the loaded LDraw\n"
            "    document description when this information is\n"
            "    available.\n"
            "    Updates make here will, of course, override the\n"
            "    automatically configured value.\n\n"
            "  This attribute is part of the instruction document\n"
            "  page attributes.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_PUBLISH_LOGO
        {
            QObject::tr(
            "  Configure the default publisher logo page attribute.\n\n"
            "  - Default Document Logo: set the instruction document\n"
            "    logo.\n\n"
            "  This attribute is  part of the instruction document\n"
            "  page attributes.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_RENDERERS_AND_PROJECTION
        {
            QObject::tr(
            "  Configure %1 preferred renderer, projection and\n"
            "  image processing timeout settings:\n\n"
            "  - Preferred Renderer:"
            "    * POVRay: %1 ray tracing renderer offering feature\n"
            "      rich, photo-realistic rendering ideal for document\n"
            "      covers and final model or assembly displays.\n\n"
            "    * LDGLite: %1 legacy renderer written in 'c' offering\n"
            "      fast image rendering ideal for proofing.\n\n"
            "    * LDView: This is a custom instance of LDView specially\n"
            "      adapted for %1 using the latest code base.\n"
            "      LDView offers high quality, rendering plus POV scene\n"
            "      file generation.\n\n"
            "    * Native: %1 'native' renderer and 3D viewer adapted\n"
            "      from LeoCAD. The Native renderer is performant and\n"
            "      provides 'true fade' rendering which enable powerful,\n"
            "      high quality and complex model rendering options.\n\n"
            "  - Projection:\n\n"
            "    * Perspective: linear project where distant objects\n"
            "      appear smaller.\n\n"
            "    * Orthographic: parallel projection, where all\n"
            "      projection lines are orthogonal to the projection\n"
            "      plane.\n\n"
            "  - Timeout: set the time in minutes before a render\n"
            "    process will terminate if the process has not been\n"
            "    completed by the renderer\n"
            "    The default value is 6 minutes.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_RENDERERS_LSYNTH_PARTS
        {
            QObject::tr(
            "  Configure LSynth settings.\n"
            "  - Archive LSynth Parts: Check if LSynth parts exist in\n"
            "    LDraw folder but not included in LDaw Content Search\n"
            "    Directories paths and add these parts to the %1\n"
            "    LDraw unofficial archive library.\n\n"
            "  - Add LSynth Library Path To LDraw Search Directories: \n"
            "    add library path to LDraw Content Search Directories\n"
            "    if not already included in search paths.\n\n"
            "    This option is enabled only whe Archive LSynth Parts\n"
            "    is enabled.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_RENDERERS_MESSAGE
        {
            QObject::tr(
            "  Renderer error, warning and information messages\n"
            "  are displayed here.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_RENDERING
        {
            QObject::tr(
            "  Configure the Native renderer and 3DViewer default\n"
            "  rendering settings.\n\n"
            "  - Anti-aliasing: toggle on or off and select between\n"
            "    2x, 4x, and 8x using the drop-down menu.\n\n"
            "  - Edge Lines: toggle on or off edge lines and select\n"
            "    edge line width using the slide bar.\n\n"
            "  - Mesh LOD: toggle on or off the mesh level of detaul\n"
            "    and select the LOD value between 0 and 1500 using the\n"
            "    slide bar.\n\n"
            "  - Shading Mode: select the shading mode from default\n"
            "    lights, wireframe, or flat shading.\n\n"
            "  - Stud Style: stud style decorate studs with one of\n"
            "    seven (7) LEGO inscribed styles.\n\n"
            "    High contrast stud styles paint stud cylinder\n"
            "    and part edge colors.\n\n"
            "    Available stud styles:\n"
            "    ° 0 Plain\n"
            "    ° 1 Thin Line Logo\n"
            "    ° 2 Outline Logo\n"
            "    ° 3 Sharp Top Logo\n"
            "    ° 4 Rounded Top Logo\n"
            "    ° 5 Flattened Logo\n"
            "    ° 6 High Contrast\n"
            "    ° 7 High Contrast With Logo\n\n"
            "    High Contrast Stud And Edge Color Setttings allow\n"
            "    you to configure part edge and stud cylinder\n"
            "    color settings.\n\n"
            "  - LPub True Fade: toggle on or off drawing the back\n"
            "    faces of faded parts.\n\n"
            "  - Draw Conditional Lines: toggle on or off drawing\n"
            "    conditional lines.\n\n"
            "  - HTML Export Fade Prefious Steps: toggle on or off\n"
            "    fade previous steps when exporting HTML steps.\n\n"
            "  - HTML Export Highlight New Parts: toggle on or off\n"
            "    highlight current step when exporting HTML steps.\n\n"
            "  - Automate Edge Color: automatically adjust part edge\n"
            "    colors based on the following configured settings:\n"
            "    * Contrast: the amount of contrast.\n\n"
            "    * Saturation: the amount of edge color tint\n"
            "      or shade.\n")
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_SEARCH_DIRECTORIES
        {
            QObject::tr(
            "  View %1 LDraw content search directories.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_THEME_SCENE_COLORS
        {
            QObject::tr(
            "  Configure and reset %1 scene colors.\n\n"
            "  - Background: set or change the scene background color\n"
            "    using the color picker.\n\n"
            "  - Ruler: set or change the scene ruler color using the\n"
            "    color picker.\n\n"
            "  - Guide: set or change the scene guide bar color using\n"
            "    the color picker.\n\n"
            "  - Grid: set or change the scene grid color using the\n"
            "    color picker.\n\n"
            "  - Tracking: set or change the scene tracking bar color\n"
            "    using the color picker.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_THEME_SELECT_AND_COLORS
        {
            QObject::tr(
            "  You can select the %1 theme from Default and Dark\n\n"
            "  Additionally, you can set and reset theme colors\n"
            "  using the Theme Colors... button.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_UNITS
        {
            QObject::tr(
            "  Select %1 default unit measurement from centimeters\n"
            "  or inches.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_UPDATES
        {
            QObject::tr(
            "  Configure %1 check for updates settings.\n"
            "  - Frequency: select from never, once a day, once a week\n"
            "    or once a month.\n\n"
            "  - Show Progress Details: toggle on or off showing\n"
            "    progress details while checking for updates.\n\n"
            "  - Show Update Available: toggle on or off automatic\n"
            "    notification when an update is available.\n\n"
            "  - Version Installed: the version installed dialog\n"
            "    will automatically display the currently installed\n"
            "    version of %1.\n\n"
            "  - Enable Downloader: download the available update\n"
            "    directly instead of opening the download link\n"
            "    through a browser.\n\n"
            "  - Show Redirects: show the download URL redirect accept\n"
            "    message prompt when redirected.\n\n"
            "  - Check For Updates...: interactively trigger an update\n"
            "    check.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_LPUB3D_PREFERENCES_VIEWPOINT_AND_PROJECTION
        {
            QObject::tr(
            "  Configure the Native renderer and 3DViewer default\n"
            "  viewpoint, projection and zoom settings.\n\n"
            "  - Viewpoint: set the pre-defined camera angle latitude\n"
            "    and longitude settings which offer the following\n"
            "    options in degrees:\n"
            "    * Default: do not set a camera view, instead\n"
            "      use the camera projection setting.\n"
            "    * Front: set the camera view at 0 lat, 0 lon.\n"
            "    * Back: set the camera view at 0 lat, 180 lon.\n"
            "    * Top: set the camera view at 90 lat, 0 lon.\n"
            "    * Bottom: set the camera view at -90 lat, 0 lon.\n"
            "    * Left: set the camera view at 0 lat, 90 lon.\n"
            "    * Right: set the camera view at 0 lat, -90 lon.\n"
            "    * Home: set the camera view at 30 lat, 45 lon.\n"
            "    Note that setting the camera viewpoint will override\n"
            "    the camera projection setting.\n\n"
            "  - Projection: set the default projection from:\n\n"
            "    * Perspective: linear project where distant objects\n"
            "      appear smaller.\n"
            "    * Orthographic: parallel projection, where all\n"
            "      projection lines are orthogonal to the projection\n"
            "      plane.\n"
            "    * Default: do not apply camera projection, instead\n"
            "      use the camera viewpoint setting.\n\n"
            "    Note that when the projection setting in the\n"
            "    Preferred Renderer dialog is changed, this setting\n"
            "    is also automatically updated to the new setting.\n\n"
            "    Also note that when the camera viewpoint is set,\n"
            "    the camera projection is automatically set to\n"
            "    default and vice versa.\n\n"
            "  - Zoom Extents: set the automatic camera zoom to\n"
            "    optimize the current image in the view port.\n"
            "    You can select from the following options:\n"
            "    * Default: do not apply zoom extents.\n"
            "    * On Orthographic: automatically apply zoom extents\n"
            "      when camera projection mode is orthographic.\n"
            "    * Always: always apply zoom extents.\n\n"
            "  Settings configured here will be overridden by meta\n"
            "  commands with the same settings in loaded LDraw file.\n")
        },
        // WT_CONTROL_VISUAL_PREFERENCES_APPLICATION
        {
            QObject::tr(
            "  View the 3D viewer renderer color theme.\n"
            "  This setting is read only and is configured from the\n"
            "  %1 theme selection.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_VISUAL_PREFERENCES_AUTHOR
        {
            QObject::tr(
            "  View the %1 default author.\n"
            "  This setting is read only and is configured from the\n"
            "  %1 publish author setting.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_VISUAL_PREFERENCES_BACKGROUND_COLOR
        {
            QObject::tr(
            "  Configure the 3D viewer background color using\n"
            "  the color picker.\n\n"
            "  You can select between a solid color or color gradient.\n"
            "  For color gradient, you can configure the top and\n"
            "  bottom colors.\n")
        },
        // WT_CONTROL_VISUAL_PREFERENCES_BASE_GRID
        {
            QObject::tr(
            "  Configure the 3D viewer base grid attributes.\n"
            "  - Draw Studs: toggle on and off drawing studs on the\n"
            "    base grid.\n"
            "  - Draw Lines Every n Studs: toggle on or off drawing\n"
            "    grid lines evey n studs, by default n is 5 studs.\n"
            "  - Draw Origin Lines: draw X and Y axis guidelines that\n"
            "    intersect at the selected part or assembly origin.\n")
        },
        // WT_CONTROL_VISUAL_PREFERENCES_BASE_GRID_COLOR
        {
            QObject::tr(
            "  Configure the 3D viewer base grid line and stud color\n"
            "  using the color picker.\n"
            "  - Lines: grid line color.\n"
            "  - Studs: drawn studs color.\n")
        },
        // WT_CONTROL_VISUAL_PREFERENCES_CATEGORY
        {
            QObject::tr(
            "  View, import, export or reset LDraw part categories.\n"
            "  - Categories: Part categories are taken from the\n"
            "    part description.\n"
            "    You can select a part category in the category column\n"
            "    and the all parts under this category will displayin\n"
            "    the right column. Parts are displayed by their name\n"
            "    number attributes.\n\n"
            "  You can perform the following actions on\n"
            "  part categories.\n\n"
            "  - Import: import custom part categories.\n\n"
            "  - Export: export the entire list of part categories to\n"
            "    an external file.\n\n"
            "  - Reset: reload all part categories and their default\n"
            "    definitions.\n")
        },
        // WT_CONTROL_VISUAL_PREFERENCES_CATEGORY_EDIT
        {
            QObject::tr(
            "  Create, edit or delete LDraw part categories.\n"
            "  - New: create a new part category.\n"
            "  - Edit: edit the selected part category.\n"
            "  - Delete: remove the selected part category.\n"
            "    Edited and removed part categories can b restored\n"
            "    using the Reset button in the Parts Library\n"
            "    Categories dialog.\n")
        },
        // WT_CONTROL_VISUAL_PREFERENCES_GENERAL_SETTINGS
        {
            QObject::tr(
            "  Configure the 3D viewer and Native renderer general\n"
            "  settings.\n\n"
            "  - Prompt On Redraw LPub Document Page: toggle on or off\n"
            "    a display message asking to redraw the current page\n"
            "    when a preference setting that affects the rendered\n"
            "    image is changed.\n\n"
            "  - Prefer Official Parts: toggle on or off ignoring\n"
            "    unofficial LDraw parts that also appear in the\n"
            "    official parts archive library.\n\n"
            "    There may be cases where you may wish to use an\n"
            "    'altered' instance of an official part in which\n"
            "    case this setting can be toggled off to enable\n"
            "    this behaviour.\n\n"
            "    Keep in mind custom parts, or any unofficial part,\n"
            "    exposed to %1 using LDraw Search Directories, will\n"
            "    ultimately be added to the LDraw unofficial parts\n"
            "    archive library.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_VISUAL_PREFERENCES_INTERFACE
        {
            QObject::tr(
            "  Configure the 3D viewer XYZ axis icon settings and\n"
            "  select the axis icon location within the 3D viewer\n"
            "  viewport.\n\n"
            "  - Axis Icon: toggle on or off the axis icon display.\n"
            "  - Location: XYZ axis icon location options\n"
            "    ° Top Left\n"
            "    ° Top Rightn\n"
            "    ° Bottom Left\n"
            "    ° Bottom Right\n")
        },
        // WT_CONTROL_VISUAL_PREFERENCES_INTERFACE_COLOR
        {
            QObject::tr(
            "  Configure the 3D viewer interface object colors\n"
            "  using the color picker.\n\n"
            "  - Axis Labels: axis icon X,Y,Z labels.\n"
            "  - Overlay Color: object overlay.\n"
            "  - Marquee Border: selection area border.\n"
            "  - Marquee Fill: selection area fill.\n"
            "  - Active View: active 3D viewwer viewport border.\n"
            "  - Inactive View: inactive 3D viewwer viewport border.\n"
            "  - Camera Name: custom camera label.\n")
        },
        // WT_CONTROL_VISUAL_PREFERENCES_KEYBOARD_SHORTCUTS_TABLE
        {
            QObject::tr(
            "  This setting is not available.\n")
        },
        // WT_CONTROL_VISUAL_PREFERENCES_MOUSE
        {
            QObject::tr(
            "  Set the 3D viewer mouse sensitivity when performing\n"
            "  tasks such as dragging and item selection.\n")
        },
        // WT_CONTROL_VISUAL_PREFERENCES_MOUSE_SHORTCUTS
        {
            QObject::tr(
            "  This setting is not available.\n")
        },
        // WT_CONTROL_VISUAL_PREFERENCES_MOUSE_SHORTCUTS_TABLE
        {
            QObject::tr(
            "  This setting is not available.\n")
        },
        // WT_CONTROL_VISUAL_PREFERENCES_OBJECTS_COLOR
        {
            QObject::tr(
            "  Configure the 3D viewer model object colors.\n"
            "  using the color picker.\n\n"
            "  - Focused: clicken on item color.\n"
            "  - Selected: selection item color.\n"
            "  - Camera: drawn camera color.\n"
            "  - Control Point: flexible parts control point color.\n"
            "  - Control Point Focused: clicked on flexible parts\n"
            "    control point color.\n"
            "  - Build Mod Selected: selected item when Build\n"
            "    Modifications is enabled color.\n"
            "  - Light: drawn light color.\n")
        },
        // WT_CONTROL_VISUAL_PREFERENCES_PART_PREVIEW
        {
            QObject::tr(
            "  Configure the 3D viewer part preview settings.\n\n"
            "  - Preview Size: size options\n"
            "    * Docked: the part preview window is docked as a tab\n"
            "      in the 3D viewer.\n"
            "    * Small: 300 pixels.\n"
            "    * Medium: 400 pixels.\n"
            "  - Location: location options\n"
            "    ° Top Right\n"
            "    ° Tob Left\n"
            "    ° Bottom Right\n"
            "    ° Bottom Left\n\n"
            "  - Position: position options\n"
            "    * Floating: the part preview windows is floating.\n"
            "    * Docked: the part preview window is docked as a tab\n"
            "      in the 3D viewer.\n\n"
            "  - View Sphere Size: size options\n"
            "    * Disabled: the view sphere is not dislayed.\n"
            "    * Small: 50 pixels.\n"
            "    * Medium: 100 pixels.\n"
            "    * Large: 200 pixels.\n\n"
            "  - View Sphere Location: location options\n"
            "    ° Top Right\n"
            "    ° Tob Left\n"
            "    ° Bottom Right\n"
            "    ° Bottom Left\n\n"
            "  - Axis Icon: toggle on or off the XYZ axis icon display.\n")
        },
        // WT_CONTROL_VISUAL_PREFERENCES_PATHS
        {
            QObject::tr(
            "  View the 3D viewer and Native renderer paths.\n"
            "  These setting are read only and are configured from\n"
            "  %1.\n\n"
            "  - Parts Library: the official LDraw archive parts\n"
            "    library.\n"
            "    This location should also contain the unofficial\n"
            "    LDraw archive parts library.\n"
            "  - Color Configuration: when an alternate LDConfig\n"
            "    LDraw color configuration file is specified in %1\n"
            "    Preferences, that file path is reflected here.\n"
            "  - POV-Ray Executable: here we have the loction of the\n"
            "    %1 POV-Ray renderer.\n"
            "  - LGEO Library: when the LEGO Geometrical Equivalent\n"
            "    Objects (LGEO) library is available, the library path\n"
            "    is specified here.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_CONTROL_VISUAL_PREFERENCES_SHORTCUT
        {
            QObject::tr(
            "  This setting is not available.\n")
        },
        // WT_CONTROL_VISUAL_PREFERENCES_TIMELINE
        {
            QObject::tr(
            "  Configure the 3D viewer timeline settings.\n"
            "  - Show Timeline Piece Icons: display timeline icon as\n"
            "    a scaled image of the timeline part or assembly.\n\n"
            "  - Show Timeline Color Icons: display timeline icon as\n"
            "    a circular image colored the same as the part\n"
            "    represents. Assemblies are assigned the LDraw default\n"
            "    color.\n\n"
            "    By default, timeline icons is set to Show Timeline\n"
            "    Color Icons. This setting allows you to set\n"
            "    Show Timeline Piece Icons.\n")
        },
        // WT_CONTROL_VISUAL_PREFERENCES_VIEW_SPHERE
        {
            QObject::tr(
            "  Configure the 3D viewer view sphere size and location.\n\n"
            "  - Size: size options\n"
            "    * Disabled: the view sphere is not dislayed.\n"
            "    * Small: 50 pixels.\n"
            "    * Medium: 100 pixels.\n"
            "    * Large: 200 pixels.\n\n"
            "  - Location: location options\n"
            "    ° Top Right\n"
            "    ° Tob Left\n"
            "    ° Bottom Right\n"
            "    ° Bottom Left\n")
        },
        // WT_CONTROL_VISUAL_PREFERENCES_VIEW_SPHERE_COLOR
        {
            QObject::tr(
            "  Configure the 3D viewer view sphere colors\n"
            "  using the color picker.\n\n"
            "  - Color: view sphere background color.\n"
            "  - Text Color: view plane text color.\n"
            "  - Highlight Color: view plane hover highlight color.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_ANTIALIASING
        {
            QObject::tr(
            "  Configure LDView antialiasing settings.\n\n"
            "  - FSAA: This allows you to select what level of\n"
            "    full-scene antialiasing to use.\n"
            "    Note that if your video card does\n"
            "    not support full-scene antialiasing, then None will\n"
            "    be the only available item.\n"
            "    If your video card does support full-scene\n"
            "    antialiasing, then all available FSAA modes should be\n"
            "    available to choose from. If you have an nVIDIA®\n"
            "    video card with Quincunx support, then 2x Enhanced\n"
            "    and 4x Enhanced should show up as options. These use\n"
            "    nVIDIA's advanced FSAA filter.\n\n"
            "  - Antialiased lines: Enabling this option causes lines\n"
            "    to be drawn using antialiasing. This does not effect\n"
            "    wireframe mode. Line antialiasing can significantly\n"
            "    improve the visual quality of lines, but it can also\n"
            "    significantly impact the frame rate.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_COLORS
        {
            QObject::tr(
            "  Configure LDView color settings.\n\n"
            "  - Background: This allows you to set the color used to\n"
            "    draw the background behind the model.\n\n"
            "  - Default Color: This allows you to set the default\n"
            "    color.\n"
            "    Note that edge lines on default-colored objects will\n"
            "    be black.\n"
            "    Resetting the General preferences also resets these\n"
            "    colors.\n\n"
            "  - Transparent: Enabling this option causes\n"
            "    default-colored items to be transparent.\n\n"
            "  - Random colors: Enabling this option causes the colors\n"
            "    used in the model to be randomized.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_INI_FILE
        {
            QObject::tr(
            "  View the LDView INI configuration file and preference\n"
            "  set specified for the current function. Functions\n"
            "  include, step image rendering, POV file generation\n"
            "  and HTML parts list export.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_MISC
        {
            QObject::tr(
            "  Configure LDView general miscellaneous settings.\n\n"
            "  - Auto-crop rendered image: Enabling this option will\n"
            "    automatically remove unnecessary negative space\n"
            "    from the rendered image.\n\n"
            "  - Rendered image background transparent: Enabling this\n"
            "    option causes the rendered image background to be\n"
            "    transparent.\n\n"
            "  - Don't use pixel buffer: Enabling this option will not\n"
            "    use the pixel buffer when rendering images.\n"
            "    This setting is only available on Windows.\n\n"
            "  - Field of view: Enter a number in this field to\n"
            "    control the field of view (in degrees).\n"
            "    Setting this to the minimum value of 0.1 will produce\n"
            "    an image that is essentially orthographic.\n"
            "    Setting it to the maximum value of 90 will produce a\n"
            "    wide-angle view.\n\n"
            "  - Memory usage: This allows you to have some control\n"
            "    over how much memory LDView uses. Normally, setting\n"
            "    this to High will result in the best performance.\n"
            "    However, if you load a large model that uses more\n"
            "    memory than you have available, setting this to\n"
            "    Medium or Low may improve performance.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GENERAL_SAVE_DIRECTORIES
        {
            QObject::tr(
            "  The default save directories box allows you to decide\n"
            "  what directory to use when saving files. Each save\n"
            "  operation has three possible options:\n\n"
            "  - Model: The default directory is the same directory\n"
            "    as the currently loaded model.\n"
            "  - Last Saved: The default directory is whatever\n"
            "    directory was used last for this file type. (The\n"
            "    first time you save a file of a given type, the\n"
            "    default directory is the model's directory.)\n\n"
            "  Save directory can be set for the following:\n"
            "  - Renderings: LDView rendered images.\n"
            "  - Part Lists: HTML generated part list.\n"
            "  - POV Exports: POV file generation used by the POV-Ray\n"
            "    renderer.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_MODEL
        {
            QObject::tr(
            "  Configure LDView model settings.\n\n"
            "  - Seam width: Enabling this option allows you to set\n"
            "    the width of the seams between parts using the spin\n"
            "    box up and down arrows to the right. The field then\n"
            "    displays the current seam width. Use the spin box\n"
            "    arrows to set the seam width to a range of numbers\n"
            "    between 0.00 and 5.00.\n\n"
            "  - Part bounding boxes only: Enabling this option makes\n"
            "    it so that all parts in the model are drawn as just\n"
            "    bounding boxes. A bounding box is the minimum\n"
            "    axis-aligned box that can completely contain the part.\n"
            "    Geometry that isn't detected as being inside a part is\n"
            "    drawn normally.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_WIREFRAME
        {
            QObject::tr(
            "  Configure LDView wire frame settings.\n\n"
            "  - Wireframe: Enabling this option causes the model to\n"
            "    be drawn in wireframe. Note that with many 3D\n"
            "    accelerators, this actually runs slower than when\n"
            "    drawing as a solid.\n\n"
            "  - Fog: This option is only enabled if wireframe drawing\n"
            "    is enabled. Enabling it causes fog to be used to make\n"
            "    it so that the lines that are farther away are drawn\n"
            "    more dimly.\n\n"
            "  - Remove hidden lines: This option is only available if\n"
            "    wireframe drawing is enabled. Enabling it causes the\n"
            "    solid (non-transparent) portions of the model to hide\n"
            "    lines that are behind them, even though these portions\n"
            "    themselves aren't actually drawn.\n\n"
            "  - Thickness: This option is only available if wireframe\n"
            "    drawing is enabled. It adjusts the thickness of the\n"
            "    lines used to draw the model in wireframe mode.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_BFC
        {
            QObject::tr(
            "  Configure LDView Back-face Culling (BFC) settings.\n\n"
            "  - BFC: Enabling this option turns on back-face culling.\n"
            "    BFC is a technique that can improve rendering\n"
            "    erformance by not drawing the back faces of polygons.\n"
            "    (Only BFC-certified parts have their back faces\n"
            "    removed.)\n\n"
            "  - Red back faces: This option is only available while\n"
            "    BFC is enabled. Enabling it causes the back faces of\n"
            "    polygons to be drawn in red. This is mainly useful for\n"
            "    part authors.\n\n"
            "  - Green front faces: This option is only available while\n"
            "    BFC is enabled. Enabling it causes the front faces of\n"
            "    polygons to be drawn in green. This is mainly useful\n"
            "    for part authors.\n\n"
            "  - Blue neutral faces: This option is only available\n"
            "    while BFC is enabled. Enabling it causes the faces of\n"
            "    non BFC certified polygons to be drawn in blue. This\n"
            "    is mainly useful for part authors.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_GEOMETRY_EDGE_LINES
        {
            QObject::tr(
            "  Configure LDView edge lines settings.\n\n"
            "  - Edge Lines: Enabling this option causes the edge\n"
            "    lines that outline all the pieces to be drawn.\n\n"
            "  - Show edges only: This option is only available if\n"
            "    edge lines are enabled. Enabling it causes LDView\n"
            "    to only show edge lines. Unless you also select the\n"
            "    Wireframe option, the portions of the edge lines that\n"
            "    are hidden by the model will still be hidden, even\n"
            "    thought the model isn't being shown.\n\n"
            "  - Conditional lines: This option is only available if\n"
            "    edge lines are enabled. Enabling it causes conditional\n"
            "    edge lines to be computed and drawn. Conditional edge\n"
            "    lines show up as a silhouette of curved surfaces; they\n"
            "    are only drawn when they are visually at the edge of\n"
            "    the surface from the current viewing angle.\n\n"
            "  - Show all: This option is only available if edge lines\n"
            "    and conditional lines are enabled. Enabling it causes\n"
            "    all conditional edge lines to be drawn, whether or not\n"
            "    they are on a silhouette.\n\n"
            "  - Show control pts: This option is only available if\n"
            "    edge lines and conditional lines are enabled.\n"
            "    Enabling it causes lines to be drawn from the first\n"
            "    point of each conditional line to each of its two\n"
            "    control points.\n\n"
            "  - High quality: This option is only available if edge\n"
            "    lines are enabled. Enabling it causes edge lines to be\n"
            "    drawn slightly closer to the eye than everything else,\n"
            "    resulting in much better looking lines.\n\n"
            "  - Always black: This option is only available if edge\n"
            "    lines are enabled. Enabling it causes all edge lines\n"
            "    to be drawn in black or dark gray.\n\n"
            "    If Process LDConfig.ldr is checked in the General\n"
            "    Preferences, LDView will look for a entries in\n"
            "    LDConfig.ldr with color names of Black_Edge and\n"
            "    Dark_Gray_Edge. If it finds either entry, it will use\n"
            "    that color instead of black or dark gray.\n\n"
            "  - Thickness: This option is only available if edge lines\n"
            "    are enabled. It adjusts the thickness of the lines\n"
            "    used to draw the edges around parts.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_LIGHTING
        {
            QObject::tr(
            "  Configure LDView lighting settings.\n\n"
            " - Lighting: Enabling this option enables lighting.\n"
            "   If you disable lighting, it is strongly recommended\n"
            "   that you enable edge lines, unless you are in\n"
            "   wireframe mode.\n\n"
            " - High quality: This option is only available if\n"
            "   lighting is enabled. Selecting this enables high\n"
            "   quality lighting. Enabling this can significantly\n"
            "   decrease performance, especially with large models.\n\n"
            " - Subdued: This option is only available if lighting is\n"
            "   enabled. Selecting this sets the lighting to a subdued\n"
            "   level. Subdued lighting is about halfway in between\n"
            "   standard lighting and no lighting at all.\n\n"
            " - Specular highlight: This option is only available if\n"
            "   lighting is enabled. Selecting this turns on the\n"
            "   specular highlight. This causes the model to appear\n"
            "   shiny.\n\n"
            " - Alternate setup: This option is only available if\n"
            "   lighting is enabled. Selecting this causes LDView to\n"
            "   use an alternate lighting setup, which may improve\n"
            "   performance on some video cards. The alternate setup\n"
            "   is used automatically when specular highlight\n"
            "   is enabled.\n\n"
            " - Light direction: Set the direction the light comes from\n"
            "   to a pre-set angle.\n\n"
            " - Use LIGHT.DAT lights: This option is only available if\n"
            "   lighting is enabled. Selecting this causes LDView to\n"
            "   put point light sources in the scene in the locations\n"
            "   where LIGHT.DAT is used as a part.\n\n"
            " - Replace standard light: This option is only available\n"
            "   if lighting is enabled and Use LIGHT.DAT lights is\n"
            "   checked. Selecting this causes LDView to not use its\n"
            "   standard light source any time a model is loaded that\n"
            "   contains one or more LIGHT.DAT parts. Models that\n"
            "   don't contain any LIGHT.DAT parts will still be\n"
            "   illuminated using LDView's standard light source.\n\n"
            " - Hide LIGHT.DAT geometry: Selecting this causes LDView\n"
            "   to not display the geometry from LIGHT.DAT parts.\n"
            "   Since the geometry is a small set of 3D coordinate\n"
            "   axes, you often don't want to see it.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_STEREO
        {
            QObject::tr(
            "  Configure LDView stereo settings.\n\n"
            " - Stereo: Enabling this option turns on stereo mode.\n"
            "   Stereo mode draws two images, a slightly different one\n"
            "   for each eye.\n\n"
            " - Cross-eyed: This option is only available if stereo is\n"
            "   enabled. Selecting this sets the stereo mode to\n"
            "   cross-eyed stereo. The two images are drawn side by\n"
            "   side, with the image for the right eye on the left and\n"
            "   the image for the left eye on the right.\n\n"
            " - Parallel: This option is only available if stereo is\n"
            "   enabled. Selecting this sets the stereo mode to\n"
            "   parallel stereo. The two images are drawn side by\n"
            "   side, with the image for the right eye on the right\n"
            "   and the image for the left eye on the left.\n\n"
            " - Amount: This option is only available if stereo is\n"
            "   enabled. The slider controls the stereo spacing.\n"
            "   A small value will result in a very subtle stereoscopic\n"
            "   effect, as if you were looking at a very large object\n"
            "   from a very long distance. (The minimum value results\n"
            "   in no stereoscopic effect at all.) A large value will\n"
            "   result in a pronounced stereoscopic effect, as if you\n"
            "   were looking at a small object from a very short\n"
            "   distance.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_WIREFRAME_CUTAWAY
        {
            QObject::tr(
            "  Configure LDView wireframe cutaway settings.\n\n"
            " - Wireframe cutaway: Enabling this option causes the\n"
            "   cutaway portion of the model to be drawn in\n"
            "   wireframe.\n\n"
            " - Color: This option is only available if wireframe\n"
            "   cutaway is enabled. Selecting it causes the wireframe\n"
            "   cutaway to be drawn in color.\n\n"
            " - Monochrome: This option is only available if wireframe\n"
            "   cutaway is enabled. Selecting it causes the wireframe\n"
            "   cutaway to be drawn in monochrome.\n\n"
            " - Opacity: This option is only available if wireframe\n"
            "   cutaway is enabled. The slider controls the opacity\n"
            "   of the wireframe cutaway.\n\n"
            " - Thickness: This option is only available if wireframe\n"
            "   cutaway is enabled. The slider controls the thickness\n"
            "   of the lines in the wireframe cutaway.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_TRANSPARENCY
        {
            QObject::tr(
            "  Configure LDView transparency settings.\n\n"
            " - Sort transparent polygons: Enabling this option causes\n"
            "   transparent shapes to be drawn in back-to-front order\n"
            "   in order to produce a correct result.\n"
            "   This option is mutually exclusive with the use stipple\n"
            "   pattern option.\n\n"
            " - Use stipple pattern: Enabling this option causes\n"
            "   transparent shapes to be drawn using a stipple\n"
            "   (checkerboard) pattern.\n"
            "   This option is mutually exclusive with the sort\n"
            "   transparent polygons option.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_EFFECTS_MISC
        {
            QObject::tr(
            "  Configure LDView effects miscellaneous settings.\n\n"
            " - Flat shading: Enabling this option causes flat shading\n"
            "   to be used. This option is mutually exclusive with the\n"
            "   smooth curves option.\n\n"
            " - Smooth curves: Enabling this option causes curved\n"
            "   surfaces to be shaded as curved surfaces instead of\n"
            "   as facets.\n"
            "   This option is mutually exclusive with the flat\n"
            "   shading option.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_PRIMITIVE_SUBSTITUTION
        {
            QObject::tr(
            "  Configure LDView primitive substitution settings.\n\n"
            " - Primitive substitution: Enabling this option causes\n"
            "   LDView to use its own optimized version of many of the\n"
            "   primitives (such as cylinders, cones, etc). As well as\n"
            "   providing enhanced performance, this also provides\n"
            "   improved visual quality, since smooth shaded primitives\n"
            "   are generated where appropriate.\n\n"
            " - Curve quality: This option is only available if\n"
            "   primitive substitution is enabled. It adjusts the\n"
            "   quality of substituted primitives by changing the\n"
            "   number of facets used to draw curved primitives.\n"
            "   Changing this from its default value can lead to holes\n"
            "   in the model between the substitute primitives and\n"
            "   other geometry. Be very careful with this setting.\n"
            "   Setting it too high can lead to huge slow-downs.\n"
            "   The default setting of 2 is fine for most models.\n"
            "   Note: Setting this to the minimum value will result\n"
            "   in incorrect rendering of n-16 primitives.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_PRIMITIVE_TEXTURES
        {
            QObject::tr(
            "  Configure LDView texture settings.\n\n"
            " - Use texture mapping: Enabling this options causes\n"
            "   LDView to display texture maps referenced in LDraw\n"
            "   files.\n\n"
            " - Offset: This option is only available if texture\n"
            "   mapping is enabled. It controls how far textures are\n"
            "   pulled toward the camera before being drawn.\n\n"
            " - Texture studs: This option is only available if\n"
            "   primitive substitution is enabled. Enabling it causes\n"
            "   a LEGO® logo to be textured onto all the standard\n"
            "   studs.\n\n"
            " - Nearest filtering: This option is only available if\n"
            "   primitive substitution and texture studs are both\n"
            "   enabled. Selecting it causes nearest texel texture\n"
            "   filtering to be used. This is the lowest quality\n"
            "   texture filtering.\n\n"
            " - Bilinear filtering: This option is only available if\n"
            "   primitive substitution and texture studs are both\n"
            "   enabled. Selecting it causes bilinear texture filtering\n"
            "   to be used. This is the middle quality texture\n"
            "   filtering.\n\n"
            " - Trilinear filtering: This option is only available if\n"
            "   primitive substitution and texture studs are both\n"
            "   enabled. Selecting it causes trilinear texture\n"
            "   filtering to be used. This is the highest quality\n"
            "   texture filtering.\n\n"
            " - Anisotropic filtering: This option is only available\n"
            "   if primitive substitution and texture studs are both\n"
            "   enabled, and your video card supports it. Selecting it\n"
            "   causes anisotropic texture filtering to be used.\n"
            "   This is the highest quality texture filtering.\n\n"
            " - Anisotropic filtering level: This option is only\n"
            "   available if primitive substitution and texture studs\n"
            "   are both enabled, and anisotropic filtering is selected\n"
            "   as the filtering type. It adjusts the anisotropic\n"
            "   filtering level. Higher numbers produce higher quality\n"
            "   filtering.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_PRIMITIVE_MISC
        {
            QObject::tr(
            "  Configure LDView primitives miscellaneous settings.\n\n"
            "  - Stud Style: stud style decorate studs with one of\n"
            "    seven (7) LEGO inscribed styles.\n\n"
            "    High contrast stud styles paint stud cylinder\n"
            "    and part edge colors.\n\n"
            "    Available stud styles:\n"
            "    ° 0 Plain\n"
            "    ° 1 Thin Line Logo\n"
            "    ° 2 Outline Logo\n"
            "    ° 3 Sharp Top Logo\n"
            "    ° 4 Rounded Top Logo\n"
            "    ° 5 Flattened Logo\n"
            "    ° 6 High Contrast\n"
            "    ° 7 High Contrast With Logo\n\n"
            "    High Contrast Stud And Edge Color Setttings allow\n"
            "    you to configure part edge and stud cylinder\n"
            "    color settings.\n\n"
            "  - Automate Edge Color: automatically adjust part edge\n"
            "    colors based on the following configured settings:\n"
            "    * Contrast: the amount of contrast.\n\n"
            "    * Saturation: the amount of edge color tint\n"
            "      or shade.\n"
            " - Low quality studs: Enabling this option switches to\n"
            "   using the low-resolution stud files instead of the\n"
            "   normal-resolution stud files. This can produce\n"
            "   significantly faster rendering.\n\n"
            " - Use hi-res primitives when available: Enabling this\n"
            "   option causes LDView to attempt to load hi-res versions\n"
            "   of primitives before loading the standard ones. The\n"
            "   hi-res primitives are otherwise known as 48-sided\n"
            "   primitives, because they approximate circular features\n"
            "   with 48-sided features. Since not all primitives have\n"
            "   hi-res versions, enabling this can create gaps between\n"
            "   the standard resolution primitives and the hi-res ones.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_UPDATES_PROXY
        {
            QObject::tr(
            "  Configure LDView internet proxy settings.\n\n"
            "  - Direct connection to the Internet: Select this if you\n"
            "    have a direct connection to the Internet and don't want\n"
            "    to use a proxy server.\n\n"
            "  - Use simple Windows proxy configuration: Select this if\n"
            "    you want to use the HTTP proxy setting that you\n"
            "    configured in Windows, Settings, Network & Internet,\n"
            "    Proxy.\n"
            "    This only works if you have a proxy server manually\n"
            "    configured. Automatic proxy configurations are not\n"
            "    supported.\n"
            "    This option is only available in the Windows version\n"
            "    of LDView.\n\n"
            "  - Manual proxy configuration: Select this to manually\n"
            "    configure your HTTP proxy server in the space provided\n"
            "    below.\n\n"
            "  - Proxy: Set this to the name or IP address of your HTTP\n"
            "    proxy server. This can only be entered if Manual proxy\n"
            "    configuration is selected above.\n\n"
            "  - Port: Set this to the port of your HTTP proxy server.\n"
            "    This can only be entered if Manual proxy configuration\n"
            "    is selected above.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_UPDATES_MISSING_PART
        {
            QObject::tr(
            "  Configure LDView preference sets settings.\n\n"
            "  - Automatically check LDraw.org for missing parts: \n"
            "    Enabling this causes LDView to automatically check the\n"
            "    LDraw.org Parts Tracker when it can't find a file. If\n"
            "    the part exists on the Parts Tracker, it will\n"
            "    automatically be downloaded and used.\n\n"
            "    If LDView fails to connect to the LDraw.org server\n"
            "    while checking for an unofficial part, it will\n"
            "    automatically disable this setting. You have to\n"
            "    manually re-enable it once you have a working Internet\n"
            "    connection again.\n\n"
            "  - Days between missing part checks: Set this to the\n"
            "    number of days you want LDView to wait between checks\n"
            "    to the LDraw.org Parts Tracker when a part cannot be\n"
            "    found. This number must be greater than zero.\n\n"
            "  - Days between updated part checks: Set this to the\n"
            "    number of days you want LDView to wait between checks\n"
            "    for updates to an unofficial part that it has\n"
            "    successfully downloaded from the LDraw.org Parts\n"
            "    Tracker. Since unofficial parts can update on the\n"
            "    Parts Tracker, LDView will check for updates with the\n"
            "    specified frequency when it encounters unofficial parts\n"
            "    in a model. This number must be greater than zero.\n")
        },
        // WT_CONTROL_LDVIEW_PREFERENCES_PREFERENCE_SET
        {
            QObject::tr(
            "  Configure LDView missing parts settings.\n\n"
            "  - Use the following preference set: This list shows all\n"
            "    of the available preference sets. Selecting a set from\n"
            "    this list will cause LDView to use the preferences\n"
            "    stored in that set after you hit Apply.\n\n"
            "    If you have made any changes to preferences on other\n"
            "    tabs and select a new preference set, it will prompt\n"
            "    you whether you want to apply those changes, abandon\n"
            "    them, or cancel your selection.\n\n"
            "  - Hot Key: This allows you to assign a hot key to the\n"
            "    currently selected preference set. There are 10\n"
            "    possible hot keys: Ctrl + Shift + 1 through\n"
            "    Ctrl + Shift + 0. Once a hot key has been assigned to\n"
            "    a preference set, you can use it when viewing a model\n"
            "    to immediately switch to that preference set.\n\n"
            "  - Delete: This deletes the currently selected preference\n"
            "    set.\n\n"
            "  - New: This creates a new preference set that will\n"
            "    initially contain all the current settings.\n")
        },
        // WT_CONTROL_LDVIEW_HTML_PART_LIST_EXPORT_COLUMN_ORDER
        {
            QObject::tr(
            "  Configure LDView HTML part list column order settings.\n")
        },
        // WT_CONTROL_LDVIEW_HTML_PART_LIST_EXPORT_DISPLAY_OPTIONS
        {
            QObject::tr(
            "  Configure LDView HTML part list display options\n"
            "  settings.\n")
        },
        // WT_CONTROL_LDVIEW_HTML_PART_LIST_EXPORT_PART_REFERENCE
        {
            QObject::tr(
            "  Configure LDView HTML part list part reference\n"
            "  settings.\n")
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_CHROME_MATERIAL
        {
            QObject::tr(
            "  Configure LDView POV export chrome material settings.\n")
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_GENERAL
        {
            QObject::tr(
            "  Configure LDView POV export general settings.\n")
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_GEOMETRY
        {
            QObject::tr(
            "  Configure LDView POV export geometry settings.\n")
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_INI_FILE
        {
            QObject::tr(
            "  View the LDView INI configuration file and preference\n"
            "  set specified for POV export. The INI file for POV\n"
            "  generation is ldvExport.ini, the preference set for POV\n"
            "  generation is POV-Ray Render.\n")
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_LIGHTING
        {
            QObject::tr(
            "  Configure LDView POV export lighting settings.\n")
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_MATERIAL
        {
            QObject::tr(
            "  Configure LDView POV export material settings.\n")
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_POV_GEOMETRY
        {
            QObject::tr(
            "  Configure LDView POV export POV geometry settings.\n")
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_POVRAY_LIGHTS
        {
            QObject::tr(
            "  Configure LDView POV export POVRay lights settings.\n")
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_RUBBER_MATERIAL
        {
            QObject::tr(
            "  Configure LDView POV export rubber material settings.\n")
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_SCROLL_AREA
        {
            QObject::tr(
            "  Configure POV file generation general, geometry,\n"
            "  pov geometry, lighting, pov lights, material\n"
            "  properties, transparent material properties,\n"
            "  rubber material properties, and chrome material\n"
            "  properties settings.\n")
        },
        // WT_CONTROL_LDVIEW_POV_EXPORT_OPTIONS_TRANSPARENT_MATERIAL
        {
            QObject::tr(
            "  Configure LDView POV export transparent material\n"
            "  settings.\n")
        },
        // WT_CONTROL_LOOK_AT_TARGET
        {
            QObject::tr(
            "  Configure the default target position.\n"
            "  - Target: using integers set the the 'Look At'\n"
            "    position using the x-axes, y-axes and z-axes.\n")
        },
        // WT_CONTROL_SNIPPETS_ADD
        {
            QObject::tr(
            "  Use this button to add a command snippet.\n\n"
            "  When the Add button is clicked, a new snippet line\n"
            "  entry is created in the snippets table and the\n"
            "  snippet editor is enabled allowing you to edit\n"
            "  the commands that will make up this snippet.\n")
        },
        // WT_CONTROL_SNIPPETS_EDIT
        {
            QObject::tr(
            "  Add meta commands to your snippet as desired.\n\n"
            "  Use '$|' to indicate where the cursor will be\n"
            "  positioned when the snippet is inserted into your\n"
            "  instruction document.\n\n"
            "  Any combination of meta commands can be added; however,\n"
            "  it would not be wise to add command values that are\n"
            "  subject to change. For example if you wish to add a\n"
            "  command that defines the default page size, use the\n"
            "  page size type (A4, Letter etc...) versus the width\n"
            "  and height.\n Perhaps a better example is the OFFSET\n"
            "  command. Unless you are certain the offset value will\n"
            "  remain constant across the snippet's use cases, it\n"
            "  might be better to not include the offset command.\n\n"
            "  Multi-line snippets are supported.\n\n"
            "  The following buttons will complete your snippet edit:\n"
            "  - Ok: accept all changes in the currnt session.\n\n"
            "  - Cancel: discard all changes in the current session.\n\n"
            "  User defined snippets are saved to an external file\n"
            "  which are loaded at application startup.\n")
        },
        // WT_CONTROL_SNIPPETS_LIST
        {
            QObject::tr(
            "  This table view lists the defined snippets currently\n"
            "  loaded.\n\n"
            "  There are two types of snippets, built-in and\n"
            "  user-defined.\n\n"
            "  Built-in snippets are read-only.\n"
            "  This is to say, the Remove button is not enabled\n"
            "  when you select a built-in snippet.\n\n"
            "  User-defined snippets are created using the Add\n"
            "  button which also enables the snippet editor.\n\n"
            "  When a new snippet is created, you must edit the\n"
            "  snippet trigger and description in this table.\n\n"
            "  - Trigger: a short name that uniquely identifies\n"
            "    the user-defined snippet.\n\n"
            "  - Description: a concise description of the snippet.\n")
        },
        // WT_CONTROL_SNIPPETS_REMOVE
        {
            QObject::tr(
            "  Remove the selected user-defined command snippet.\n\n"
            "  The remove button is disabled when a built-in command\n"
            "  snippet is selected. You cannot remove built-in\n"
            "  snippets.\n")
        },
        // WT_CONTROL_SUBSTITUTE_CURRENT_PART
        {
            QObject::tr(
            "  This dialogue displays the current part\n"
            "  and its title description. These dialogues\n"
            "  are read-only.\n")
        },
        // WT_CONTROL_SUBSTITUTE_PRIMARY_ARGUMENTS
        {
            QObject::tr(
            "  This dialogue presents the main editable\n"
            "  substitute part attributes.\n"
            "  - Substitute: enter your desired substitute\n"
            "    part or use the Lookup button to select a\n"
            "    part from the part lookup dialogue.\n"
            "    The substitute part title will display in\n"
            "    Substitute Title.\n\n"
            "  - Color: select your desired part color or\n"
            "    use the Change button to select the part\n"
            "    color using the color picker.\n\n"
            "  - LDraw Part: if the substitute part is a\n"
            "    generated part, you can optionally use this\n"
            "    dialogue to display the appropriate LDraw part\n"
            "    name and part title description.\n\n"
            "    This option is useful when you wish to show\n"
            "    a custom representation of the part in the\n"
            "    part list.\n")
        },
        // WT_CONTROL_SUBSTITUTE_EXTENDED_ARGUMENTS
        {
            QObject::tr(
            "  This dialogue presents additional editable\n"
            "  substitute part attributes.\n"
            "  - Scale: resize the displayed substitute part.\n\n"
            "  - Field of View: edit the camera FOV for the\n"
            "    substitute part.\n\n"
            "  - Camera Latitude: set the camera angle x-axis\n"
            "    in degrees.\n\n"
            "  - Camera Longitude: set the camera angle y-axis\n"
            "    in degrees.\n\n"
            "  - Target: using integers set the the 'Look At'\n"
            "    position using the x-axes, y-axes and z-axes.\n\n"
            "  - Rotation: rotate the substitute part using\n"
            "    x-angle, y-angle and z-angle.\n"
            "    These are the individual rotation angles for\n"
            "    the different axes in degrees (-360 to 360).\n"
            "  - Transform: set the step transformation using\n"
            "    options - relative, absolute or additive.\n")
        },
        //*************************************
        //*  METAGUI ENTRIES
        //*************************************
        // WT_GUI_SIZE_AND_ORIENTATION
        {
            QObject::tr(
            "  Configure the page size and orientation settings.\n"
            "  - Size: set the page size using standard sizes like\n"
            "    A4, Letter etc... You can also specify the 'Custom'\n"
            "    size options along with its corresponding height and\n"
            "    width values in %1.\n\n"
            "    The page size custom height and width unit values\n"
            "    are determined by the resolution units specified\n"
            "    in Project Global Setup.\n"
            "    Dot unit options are dots per inch (DPI) or\n"
            "    dots per centimetre (DPC)\n"
            "  - Orientation: set the page orientation to\n"
            "    portrait or landscape.\n")
            .arg(MetaDefaults::getPreferredUnits())
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
            "  - Background: set the background option from the\n"
            "    drop-down control:\n"
            "    ° None (Transparent)\n"
            "    ° Solid Color\n"
            "    ° Gradient\n"
            "    ° Submodel Level Color.\n\n"
            "  - Change: select the background color from the\n"
            "    color picker if Solid Color option selected.\n")
        },
        // WT_GUI_BORDER
        {
            QObject::tr(
            "  Configure the border.\n"
            "  - Border: set the border corner square, round or\n"
            "    borderless.\n\n"
            "  - Line: set the border line type.\n\n"
            "  - Color: select the border line color.\n\n"
            "  - Margins: set the border margins width and height.\n"
            "    Enter margin values using a decimal number\n"
            "    in %1.\n"
            "    Dot units are defined in Project Global Setup.\n")
            .arg(MetaDefaults::getPreferredUnits())
        },
        // WT_GUI_CAMERA_ANGLES
        {
            QObject::tr(
            "  Configure the camera angles.\n"
            "  - Latitude: set the camera angle x-axis in degrees.\n\n"
            "  - Longitude: set the camera angle y-axis in degrees.\n\n"
            "  - Camera Viewpoint: pre-defined camera angle latitude\n"
            "    and longitude settings which offer the following\n"
            "    options in degrees:\n"
            "    * Front: set the camera view at 0 lat, 0 lon.\n"
            "    * Back: set the camera view at 0 lat, 180 lon.\n"
            "    * Top: set the camera view at 90 lat, 0 lon.\n"
            "    * Bottom: set the camera view at -90 lat, 0 lon.\n"
            "    * Left: set the camera view at 0 lat, 90 lon.\n"
            "    * Right: set the camera view at 0 lat, -90 lon.\n"
            "    * Home: set the camera view at 30 lat, 45 lon.\n"
            "      You can change the latitude and longitude angles\n"
            "      for Home camera view.\n"
            "    * Latitude/Loingitude: set the latitude and longitude\n"
            "      angles. This setting is equivalent to selecting Use\n"
            "      Latitude And Longitude Angles to allow Home camera\n"
            "      view changes.\n"
            "    * Default: do not use Camera Viewpoint.\n\n"
            "  - Use Latitude And Longitude Angles: enable the\n"
            "    Latitude and Longitude dialogues to allow Home\n"
            "    camera viewpoint angle changes. This dialog is only\n"
            "    enabled when camera viewpoint Home is selected.\n\n"
            "    Camera angle value range is -360.0 to 360.0\n\n"
            "    Precision is one decimal place.\n\n"
            "    Selecting a Camera Viewpoint other that Default\n"
            "    will disable the individual Latitude and Longitude\n"
            "    camera angle dialogues.\n")
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
            "  - Camera FOV: set the camera field of view in degrees.\n"
            "    %1%2")
            .arg(MetaDefaults::getPreferredRenderer() == QLatin1String("Native") ? QObject::tr("\n"
            "  ° Configure the camera near plane.\n\n"
            "  ° Configure the camera far z plane.\n") : QObject::tr("\n"
            "  ° Camera near and far z planes are disabled\n"
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
            "  These colors help differentiate the current submode\n"
            "  level being edited.\n\n"
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
            "  ° 0 Plain\n"
            "  ° 1 Thin Line Logo\n"
            "  ° 2 Outline Logo\n"
            "  ° 3 Sharp Top Logo\n"
            "  ° 4 Rounded Top Logo\n"
            "  ° 5 Flattened Logo\n"
            "  ° 6 High Contrast\n"
            "  ° 7 High Contrast With Logo\n\n"
            "  Light/Dark Value triggers the part edge color\n"
            "  update for dark parts.\n\n"
            "  High Contrast styles enable part edge and\n"
            "  stud cylinder color settings.\n\n"
            "  Automate edge color settings allow %1 to\n"
            "  automatically adjust part edge colors.\n")
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
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
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
        },
        // WT_GUI_PART_ANNOTATION_OPTIONS
        {
            QObject::tr(
            "  Part annotation options.\n"
            "  - Display PartList (PLI) Annotation: select\n"
            "    annotation source from Title, Free Form or\n"
            "    Fixed annotations.\n\n"
            "  - Enable Annotation Type: enable or disable\n"
            "    annotation for axles, beams, cables, connectors,\n"
            "    hoses, panels or custom annotations.\n")
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
            "    circle styles cannot be modified from the LDraw\n"
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
            "  Select the default preferred step image renderer.\n"
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
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
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
            .arg(QLatin1String(VER_PRODUCTNAME_STR))
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
            "  - Rotation: set the x-angle, y-angle and z-angle,\n"
            "    which are the individual rotation angles for the\n"
            "    different axes in degree (-360 to 360), using\n"
            "    decimals.\n"
            "    Spin box values are incremented by 1 step unit.\n"
            "    Precision is automatically calculated based on the.\n"
            "    input values.\n\n"
            "  - Transform: there are three transform options.\n"
            "    - REL: relative rotation steps are based on the\n"
            "      actual angles of the individual viewing areas.\n"
            "      The model will rotated by the specified rotation\n"
            "      angles relative to the current view angle.\n\n"
            "    - ABS: absolute rotation steps is similar to relative\n"
            "      rotation steps, but it ignores the current view\n"
            "      angles so that after executing this command the\n"
            "      model will be rotated from a starting angle of 0.\n\n"
            "    - ADD: additive rotation step turns the model by\n"
            "      the specified angles, taking the current view\n"
            "      angle into account.\nThe command can be used to\n"
            "      continuously rotate the model by a specific angle.\n"
            "      For example if this command is executed four times\n"
            "      on a front view, and the model is rotated\n"
            "      90° clockwise on the y-axle then you will see the\n"
            "      model from each of the four sides.\n")
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
        },
        // WT_GUI_ROTATE_ICON_SIZE
        {
            QObject::tr(
            "  Configure rotate icon size.\n"
            "  - Width: set the rotate icon  width in %1.\n\n"
            "  - Height: set the rotate icon hieght in %1.\n\n"
            "    Dot units are defined in Project Global Setup.\n")
            .arg(MetaDefaults::getPreferredUnits())
        }
        // WT_NUM_ENTRIES
    };

    std::copy(std::begin(WTData), std::end(WTData), std::begin(CommonMenus::WT));

    WT_ARRAY_SIZE_CHECK(CommonMenus::WT, WT_NUM_ENTRIES);
}
