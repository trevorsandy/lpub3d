/****************************************************************************
**
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
 *
 * This class provides a simple mechanism for displaying a rotation icon
 * on the page.
 *
 ***************************************************************************/

#include "rotateiconitem.h"
#include <QMenu>
#include <QAction>
#include <QGraphicsSceneContextMenuEvent>
#include "commonmenus.h"

RotateIconItem::RotateIconItem(
  Page          *_page,
  InsertMeta     _insMeta,
  QGraphicsItem  *parent)
{
  page               = _page;
  insMeta            = _insMeta;
  parentRelativeType = _page->relativeType;
  rotateIconMeta     = _page->meta.LPub.rotateIconMeta;
  display            = _page->meta.LPub.rotateIconMeta.display;

  QPixmap *pixmap = new QPixmap(
        rotateIconMeta.size.valuePixels(0),
        rotateIconMeta.size.valuePixels(1));

  QString toolTip;
  toolTip = "Rotate Icon - right-click to modify";

  placement = rotateIconMeta.placement;

  int submodelLevel = 0;
  setBackground( pixmap,
                 SingleStepType,
                 meta,
                 rotateIconMeta.background,
                 rotateIconMeta.border,
                 rotateIconMeta.margin,
                 rotateIconMeta.subModelColor,
                 submodelLevel,
                 toolTip);

  setPixmap(*pixmap);
  setParentItem(parent);
  setFlag(QGraphicsItem::ItemIsMovable,true);
  setFlag(QGraphicsItem::ItemIsSelectable,true);
}

void RotateIconItem::contextMenuEvent(
    QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  PlacementData placementData = placement.value();

  QString pl = "Rotate Icon";
  QAction *placementAction  = commonMenus.placementMenu(menu,pl,
                                                        commonMenus.naturalLanguagePlacementWhatsThis(SingleStepType,placementData,pl));
  QAction *backgroundAction = commonMenus.backgroundMenu(menu,pl);
  QAction *borderAction     = commonMenus.borderMenu(menu,pl);
  QAction *marginAction     = commonMenus.marginMenu(menu,pl);
  QAction *displayAction    = commonMenus.displayMenu(menu,pl);

  QAction *editArrowAction = menu.addAction("Edit Arrow");
  editArrowAction->setWhatsThis("Edit this rotation icon arrows");

  QAction *deleteRotateIconAction = menu.addAction("Delete " +pl);
  deleteRotateIconAction->setWhatsThis("Delete this rotate icon");
  deleteRotateIconAction->setIcon(QIcon(":/resources/delete.png"));

  QAction *selectedAction  = menu.exec(event->screenPos());

  if (selectedAction == NULL) {
      return;
    }

  Where here          = insMeta.here();
  Where topOfSteps    = page->topOfSteps();
  Where bottomOfSteps = page->bottomOfSteps();

  if (selectedAction == placementAction) {
      changePlacement(parentRelativeType,
                      SingleStepType,
                      pl+" Placement",
                      topOfSteps,
                      bottomOfSteps,
                      &rotateIconMeta.placement,true,1,0,false);
    } else if (selectedAction == backgroundAction) {
      changeBackground(pl+" Background",
                       topOfSteps,
                       bottomOfSteps,
                       &rotateIconMeta.background);
    } else if (selectedAction == borderAction) {
      changeBorder(pl+" Border",
                   topOfSteps,
                   bottomOfSteps,
                   &rotateIconMeta.border);
    } else if (selectedAction == marginAction) {
      changeMargins(pl+" Margins",
                    topOfSteps,
                    bottomOfSteps,
                    &rotateIconMeta.margin);
    } else if (selectedAction == displayAction){
      changeBool(topOfSteps,
                 bottomOfSteps,
                 &display);
    } else if (selectedAction == editArrowAction) {

      //TODO
    } else if (selectedAction == deleteRotateIconAction) {
      beginMacro("DeleteRotateIcon");
      deleteMeta(here);
      endMacro();
    }
}

void RotateIconItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  position        = pos();
  positionChanged = false;
  QGraphicsItem::mousePressEvent(event);
}

void RotateIconItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  positionChanged = true;
  QGraphicsItem::mouseMoveEvent(event);
}

void RotateIconItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable) && positionChanged) {

    InsertData insertData = insMeta.value();

    qreal topLeft[2] = { sceneBoundingRect().left(),  sceneBoundingRect().top() };
    qreal size[2]    = { sceneBoundingRect().width(), sceneBoundingRect().height() };

    PlacementData pld;

    pld.placement    = Center;
    pld.justification= Center;
    pld.relativeTo   = PageType;
    pld.preposition  = Inside;

    calcOffsets(pld,insertData.offsets,topLeft,size);

    beginMacro(QString("MoveRotateIcon"));

    changeInsertOffset(&insMeta);

    endMacro();
  }
}
