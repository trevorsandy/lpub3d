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

#include <QMenu>
#include <QAction>
#include <QGraphicsRectItem>
#include <QGraphicsSceneContextMenuEvent>
#include "rotateiconitem.h"
#include "commonmenus.h"
#include "step.h"
#include "ranges.h"
#include "color.h"


RotateIconItem::RotateIconItem(
  Page          *_page,
  QPixmap        &pixmap,
  InsertMeta     _insMeta,
  QGraphicsItem  *parent)
{
  page               = _page;
  insMeta            = _insMeta;
  parentRelativeType = page->relativeType;
  rotateIconMeta     = page->meta.LPub.rotateIconMeta;


//  pixmap = new QPixmap(50,50);

//  QPixmap *pixmap = new QPixmap(
//        rotateIconMeta.size.valuePixels(0),
//        rotateIconMeta.size.valuePixels(1));

  QString toolTip("Rotate Icon - right-click to modify");

  margin             = rotateIconMeta.margin;
  size[0]            = pixmap.width() *rotateIconMeta.size.valuePixels(0);
  size[1]            = pixmap.height()*rotateIconMeta.size.valuePixels(1);
  placement          = rotateIconMeta.placement;
  relativeType       = RotateIconType;

  BorderData     borderData     = rotateIconMeta.border.valuePixels();
  BackgroundData backgroundData = rotateIconMeta.background.value();

  int ibt = int(borderData.thickness);
  QRectF arect(ibt/2,ibt/2,pixmap.width()-ibt,pixmap.height()-ibt);

  pixmap.setAlphaChannel(pixmap);
  pixmap.fill(Qt::transparent);

  qreal arrowTipLength = 9.0;
  qreal arrowTipHeight = 4.0;
  QPolygonF arrowHead;
  arrowHead << QPointF()
            << QPointF(arrowTipLength + 2.5, -arrowTipHeight)
            << QPointF(arrowTipLength      , 0.0)
            << QPointF(arrowTipLength + 2.5,  arrowTipHeight);

  QColor arrowPenColor;
  arrowPenColor = LDrawColor::color(rotateIconMeta.arrowColour.value());
  QPen defaultArrowPen;
  defaultArrowPen.setColor(arrowPenColor);
  defaultArrowPen.setCapStyle(Qt::SquareCap);
  defaultArrowPen.setJoinStyle(Qt::MiterJoin);
  defaultArrowPen.setStyle(Qt::SolidLine);
  defaultArrowPen.setWidth(0);

  QPen arrowPen = defaultArrowPen;

  QPainter painter(&pixmap);
  painter.setRenderHints(QPainter::Antialiasing,false);
  painter.setPen(arrowPen);
  painter.setBrush(Qt::transparent);

  qreal aw = arect.width();
  qreal ah = arect.height() / 2.0;
  float inset = 6.0;

  float ix    = inset * 1.8;
  float iy    = inset * 2.5;

  QPainterPath path;

  QPointF start(     inset, ah - inset);
  QPointF end  (aw - inset, ah - inset);
  path.moveTo(start);
  path.cubicTo(start + QPointF( ix, -iy),   end + QPointF(-ix, -iy),end);

  start += QPointF(0, inset + inset);
  end   += QPointF(0, inset + inset);
  path.moveTo(end);
  path.cubicTo(end   + QPointF(-ix,  iy), start + QPointF( ix,  iy),start);

  painter.drawPath(path);

  painter.setBrush(arrowPen.color());

  painter.save();
  painter.translate(aw - inset, ah - inset);
  painter.rotate(-135);
  painter.drawPolygon(arrowHead);
  painter.restore();

  painter.save();
  painter.translate(inset, ah + inset);
  painter.rotate(45);
  painter.drawPolygon(arrowHead);
  painter.restore();

  QColor penColor,brushColor;
  QPen backgroundPen;
  backgroundPen.setColor(penColor);
  backgroundPen.setCapStyle(Qt::RoundCap);
  backgroundPen.setJoinStyle(Qt::RoundJoin);
  backgroundPen.setStyle(Qt::SolidLine);
  backgroundPen.setWidth(ibt);

  painter.setPen(backgroundPen);
  painter.setBrush(brushColor);

  painter.setRenderHints(QPainter::HighQualityAntialiasing,true);
  painter.setRenderHints(QPainter::Antialiasing,true);

  switch(backgroundData.type) {
    case BackgroundData::BgTransparent:
      brushColor = Qt::transparent;
      break;
    case BackgroundData::BgColor:
    case BackgroundData::BgSubmodelColor:
      if (backgroundData.type == BackgroundData::BgColor) {
          brushColor = LDrawColor::color(backgroundData.string);
        } else {
          brushColor = LDrawColor::color(rotateIconMeta.subModelColor.value(0));
        }
      break;
    }

  if (borderData.type == BorderData::BdrNone) {
      penColor = Qt::transparent;
    } else {
      penColor =  LDrawColor::color(borderData.color);
    }

  qreal rx = borderData.radius;
  qreal ry = borderData.radius;
  qreal dx = pixmap.width();
  qreal dy = pixmap.height();

  if (dx && dy) {
      if (dx > dy) {
          rx *= dy;
          rx /= dx;
        } else {
          ry *= dx;
          ry /= dy;
        }
    }

  if (borderData.type == BorderData::BdrRound) {
      painter.drawRoundRect(arect,int(rx),int(ry));
    } else {
      painter.drawRect(arect);
    }

  setZValue(500);
//    *pixmap = pixmap->scaled(rotateIconMeta.size.valuePixels(0),
//                             rotateIconMeta.size.valuePixels(1),
//                             Qt::KeepAspectRatio,
//                             Qt::SmoothTransformation);

  setToolTip(toolTip);
  setPixmap(pixmap);
  setParentItem(parent);
  setFlag(QGraphicsItem::ItemIsMovable,true);
  setFlag(QGraphicsItem::ItemIsSelectable,true);
  delete &pixmap;
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
                      SingleStepType,         //not using RotateIconType intentionally
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
                 &rotateIconMeta.display);
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
  QGraphicsItem::mousePressEvent(event);
  positionChanged = false;
  position = pos();
}

void RotateIconItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseMoveEvent(event);
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
      positionChanged = true;
    }
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


