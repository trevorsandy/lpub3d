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

#include "rotateicon.h"
#include <QMenu>
#include <QAction>
#include <QGraphicsSceneContextMenuEvent>
#include "commonmenus.h"

int RotateIcon::addBackground(
    int            submodelLevel,
    QGraphicsItem *parent){

  background     =
      new RotateIconBackgroundItem(
        this,
        size[0],
        size[1],
        parentRelativeType,
        submodelLevel,
        parent);

  if (! background){
      return -1;
    }

  background->size[0] = size[0];
  background->size[1] = size[1];
  background->setPos(this->loc[XX],this->loc[YY]);
  background->setFlag(QGraphicsItem::ItemIsMovable,true);
  background->setFlag(QGraphicsItem::ItemIsSelectable,true);

  return 0;
}

RotateIconBackgroundItem::RotateIconBackgroundItem(
  RotateIcon    *_rotateIcon,
  int             width,
  int             height,
  PlacementType  _parentRelativeType,
  int             submodelLevel,
  QGraphicsItem  *parent)
{
  rotateIcon  = _rotateIcon;

  parentRelativeType = _parentRelativeType;

  QPixmap *pixmap = new QPixmap(width,height);

  QString toolTip;

  toolTip = "Rotate Icon - right-click to modify";

  placement = rotateIcon->rotateIconMeta.placement;

  setBackground( pixmap,
                 PartsListType,
                 rotateIcon->meta,
                 rotateIcon->rotateIconMeta.background,
                 rotateIcon->rotateIconMeta.border,
                 rotateIcon->rotateIconMeta.margin,
                 rotateIcon->rotateIconMeta.subModelColor,
                 submodelLevel,
                 toolTip);

  setPixmap(*pixmap);
  setParentItem(parent);
  setFlag(QGraphicsItem::ItemIsMovable,true);
  setFlag(QGraphicsItem::ItemIsSelectable,true);
}

void RotateIconBackgroundItem::contextMenuEvent(
  QGraphicsSceneContextMenuEvent *event)
{
  if (rotateIcon) {
    QMenu menu;

    PlacementData placementData = rotateIcon->placement.value();

    QString pl = "Rotate Icon";
    QAction *placementAction  = commonMenus.placementMenu(menu,pl,
                                commonMenus.naturalLanguagePlacementWhatsThis(SingleStepType,placementData,pl));
    QAction *backgroundAction = commonMenus.backgroundMenu(menu,pl);
    QAction *borderAction     = commonMenus.borderMenu(menu,pl);
    QAction *marginAction     = commonMenus.marginMenu(menu,pl);

  QAction *editArrowAction = menu.addAction("Edit Arrow");
  editArrowAction->setWhatsThis("Edit this rotation icon arrows");

  QAction *deleteRotateIconAction = menu.addAction("Delete " +pl);
  deleteRotateIconAction->setWhatsThis("Delete this rotate icon");
  deleteRotateIconAction->setIcon(QIcon(":/resources/delete.png"));

  QAction *selectedAction  = menu.exec(event->screenPos());

  if (selectedAction == NULL) {
      return;
    }

    Where here = rotateIcon->insMeta.here();

    if (selectedAction == placementAction) {
        changePlacement(parentRelativeType,
                        SingleStepType,			//change
                        pl+" Placement",
                        here,
                        here,
                       &rotateIcon->rotateIconMeta.placement,true,1,0,false);
    } else if (selectedAction == backgroundAction) {
      changeBackground(pl+" Background",
                       here,
                       here,
                       &rotateIcon->rotateIconMeta.background);
    } else if (selectedAction == borderAction) {
      changeBorder(pl+" Border",
                   here,
                   here,
                   &rotateIcon->rotateIconMeta.border);
    } else if (selectedAction == marginAction) {
      changeMargins(pl+" Margins",
                    here,
                    here,
                    &rotateIcon->rotateIconMeta.margin);
    } else if (selectedAction == editArrowAction) {

      //TODO
    } else if (selectedAction == deleteRotateIconAction) {
      beginMacro("DeleteRotateIcon");
      deleteMeta(here);
      endMacro();
    }
   }
 }

void RotateIconBackgroundItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  position = pos();
  positionChanged = false;
  QGraphicsItem::mousePressEvent(event);
}

void RotateIconBackgroundItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  positionChanged = true;
  QGraphicsItem::mouseMoveEvent(event);
}

void RotateIconBackgroundItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable) && positionChanged) {

    InsertData insertData = rotateIcon->insMeta.value();

    qreal topLeft[2] = { sceneBoundingRect().left(),  sceneBoundingRect().top() };
    qreal size[2]    = { sceneBoundingRect().width(), sceneBoundingRect().height() };

    PlacementData pld;

    pld.placement     = TopLeft;
    pld.justification = Center;
    pld.relativeTo    = PageType;
    pld.preposition   = Inside;

    calcOffsets(pld,insertData.offsets,topLeft,size);

    beginMacro(QString("MoveRotateIcon"));

    changeInsertOffset(&rotateIcon->insMeta);

    endMacro();
  }
}
