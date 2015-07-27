 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
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

#include <QMenu>
#include <QAction>
#include "resize.h"
#include "metaitem.h"
#include "scaledialog.h"
#include "step.h"
#include "ranges.h"
#include "color.h"
#include "commonmenus.h"

qreal AbstractResize::grabsize = 0.125; // inches

RectPlacement constAspectRatioResizePlacement[] =
{ TopLeftOutsideCorner, 
  TopRightOutsideCorner, 
  BottomRightOutsideCorner, 
  BottomLeftOutsideCorner };

void ResizeRect::placeGrabbers()
{
  QRectF rect = currentRect();
  int    left = rect.left();
  int    top  = rect.top();
  int    width = rect.width();
  int    height = rect.height();
  
  points[0] = QPointF(left,top);
  points[1] = QPointF(left + width, top);
  points[2] = QPointF(left + width, top + height);
  points[3] = QPointF(left, top + height);
  
  if (grabbers[0] == NULL) {
    for (int i = 0; i < numGrabbers; i++) {
      grabbers[i] = new Grabber(constAspectRatioResizePlacement[i],
                               this,myParentItem());
    }
  }
  for (int i = 0; i < numGrabbers; i++) {
    grabbers[i]->setPos(points[i].x()-grabSize()/2,points[i].y()-grabSize()/2);
  }
}

void ResizeConstAspectRect::resize(QPointF grabbed)
{
  // recalculate corners Y

  switch (selectedGrabber) {
    case TopLeftOutsideCorner:
    case TopRightOutsideCorner:
      points[0].setY(grabbed.y());
      points[1].setY(grabbed.y());
    break;
    case BottomRightOutsideCorner:
    case BottomLeftOutsideCorner:
      points[2].setY(grabbed.y());
      points[3].setY(grabbed.y());
    break;
    default:
    break;
  }

  qreal  rawWidth, rawHeight;

  switch (selectedGrabber) {
    case TopLeftOutsideCorner:
    case BottomLeftOutsideCorner:
      points[0].setX(grabbed.x());
      points[3].setX(grabbed.y());
    break;
    case TopRightOutsideCorner:
    case BottomRightOutsideCorner:
      points[1].setX(grabbed.x());
      points[2].setX(grabbed.x());
    break;
    default:
    break;
  }
 
  switch (selectedGrabber) {
    case TopLeftOutsideCorner:
      rawWidth = points[2].x() - grabbed.x();
      rawHeight = points[2].y() - grabbed.y();
    break;
    case TopRightOutsideCorner:
      rawWidth = grabbed.x()-points[3].x();
      rawHeight = points[3].y() - grabbed.y();
    break;
    case BottomRightOutsideCorner:
      rawWidth = grabbed.x() - points[0].x();
      rawHeight = grabbed.y() - points[0].y();
    break;
    default:
      rawWidth = points[1].x() - grabbed.x();
      rawHeight = grabbed.y() - points[1].y();
    break;
  }
  
  if (rawWidth > 0 && rawHeight > 0) {

    // Force aspect ratio to match original aspect ratio of picture
    // ratio = width/height
    // width = height * ratio
    
    qreal width = rawHeight * size[0]/size[1];
    qreal height = rawWidth * size[1]/size[0];
    
    if (width * rawHeight < rawWidth * height) {
      height = rawHeight;
    } else {
      width = rawWidth;
    }
    
    // Place the scaled box

    switch (selectedGrabber) {
      case TopLeftOutsideCorner:
        setNewPos(points[2].x()-width,points[2].y()-height);
      break;
      case TopRightOutsideCorner:
        setNewPos(points[3].x(),points[3].y()-height);
      break;
      case BottomRightOutsideCorner:
        setNewPos(points[0].x(),points[0].y());
      break;
      default:
        setNewPos(points[1].x()-width,points[1].y());
      break;
    }
    
    // Calculate corners
    
    points[0] = currentPos();
    points[1] = QPointF(currentPos().x() + width,currentPos().y());
    points[2] = currentPos() + QPointF(width,height);
    points[3] = QPointF(currentPos().x(),currentPos().y()+height);
  
    for (int i = 0; i < 4; i++) {
      grabbers[i]->setPos(points[i].x()-grabSize()/2,points[i].y()-grabSize()/2);
    }
    
    // Unscale from last time

    setScale(1.0/oldScale,1.0/oldScale);
    
    // Scale it to the new scale
    
    oldScale = width/size[0];
    setScale(oldScale,oldScale);
    sizeChanged = true;
  }
}

//---------------------------------------------------------------

ResizePixmapItem::ResizePixmapItem()
{
  setTransformationMode(Qt::SmoothTransformation);
}

QRectF ResizePixmapItem::currentRect()
{
  return sceneBoundingRect();
}

QPointF ResizePixmapItem::currentPos()
{
  return pos();
}

void ResizePixmapItem::setNewPos(qreal x, qreal y)
{
  setPos(x,y);
}

void ResizePixmapItem::setScale(qreal x, qreal y)
{
  scale(x,y);
}

void ResizePixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{     
  position = pos();
  positionChanged = false;
  QGraphicsItem::mousePressEvent(event);
  placeGrabbers();
} 
  
void ResizePixmapItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{ 
  positionChanged = true;
  QGraphicsItem::mouseMoveEvent(event);
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    placeGrabbers();
  }
}

void ResizePixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    change();
  }
}

QVariant ResizePixmapItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
  if (grabbers[0] && change == ItemSelectedChange) {
    for (int i = 0; i < numGrabbers; i++) {
      grabbers[i]->setVisible(value.toBool());
    }
  }
  return QGraphicsItem::itemChange(change,value);
}

//-----------------------------------------------------------

InsertPixmapItem::InsertPixmapItem(
  QPixmap    &pixmap,
  InsertMeta &insertMeta,
  QGraphicsItem *parent)

  : insertMeta(insertMeta)
{
  setPixmap(pixmap);
  setParentItem(parent);
  
  size[0] = pixmap.width() *insertMeta.value().picScale;
  size[1] = pixmap.height()*insertMeta.value().picScale;

  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsMovable,true);
  setZValue(500);

  margin.setValues(0.0,0.0);
}

void InsertPixmapItem::change()
{
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    if (positionChanged || sizeChanged) {

      beginMacro(QString("Resize"));
      
      InsertData insertData = insertMeta.value();
      
      qreal topLeft[2] = { sceneBoundingRect().left(),  sceneBoundingRect().top() };
      qreal size[2]    = { sceneBoundingRect().width(), sceneBoundingRect().height() };

      PlacementData pld;

      pld.placement    = TopLeft;
      pld.justification    = Center;
      pld.relativeTo      = PageType;
      pld.preposition   = Inside;

      calcOffsets(pld,insertData.offsets,topLeft,size);

      insertData.picScale *= oldScale;
      insertMeta.setValue(insertData);
      
      changeInsertOffset(&insertMeta);
      
      endMacro();
    }
  }
}

void InsertPixmapItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  QAction *deleteAction = menu.addAction("Delete this Picture");
  deleteAction->setWhatsThis("Delete this picture");

  QAction *selectedAction   = menu.exec(event->screenPos());

  if (selectedAction == NULL) {
    return;
  }

  if (selectedAction == deleteAction) {
    MetaItem mi;
    mi.deleteMeta(insertMeta.here());
  }
}

//-----------------------------------------------------------

void ResizePixmapItem::setRotateIconPixmap(QPixmap         &pixmap,
                                            RotateIconMeta &_rotateIconMeta)
{
   rotateIconMeta = _rotateIconMeta;

  // set pixmap to transparent
  QPixmap transform(pixmap.size());
  transform.fill(Qt::transparent);

  QPainter painter(&transform);
  painter.setOpacity(0.2);
  painter.drawPixmap(0, 0, pixmap);
  painter.end();

  // set border and background parameters
  BorderData     borderData     = rotateIconMeta.border.valuePixels();
  BackgroundData backgroundData = rotateIconMeta.background.value();

  // set icon size and demensions parameters
  int ibt = int(borderData.thickness);
  QRectF irect(ibt/2,ibt/2,pixmap.width()-ibt,pixmap.height()-ibt);

  qreal aw = irect.width();
  qreal ah = irect.height() / 2.0;
  float inset = 6.0;

  float ix    = inset * 1.8;
  float iy    = inset * 2.5;

  // set arrow parts (head, tips etc...)
  qreal arrowTipLength = 9.0;
  qreal arrowTipHeight = 4.0;
  QPolygonF arrowHead;
  arrowHead << QPointF()
            << QPointF(arrowTipLength + 2.5, -arrowTipHeight)
            << QPointF(arrowTipLength      , 0.0)
            << QPointF(arrowTipLength + 2.5,  arrowTipHeight);

  // set default arrow pen and transfer to working arrow pen
  QColor arrowPenColor;
  arrowPenColor = LDrawColor::color(rotateIconMeta.arrowColour.value());
  QPen defaultArrowPen;
  defaultArrowPen.setColor(arrowPenColor);
  defaultArrowPen.setCapStyle(Qt::SquareCap);
  defaultArrowPen.setJoinStyle(Qt::MiterJoin);
  defaultArrowPen.setStyle(Qt::SolidLine);
  defaultArrowPen.setWidth(0);
  QPen arrowPen = defaultArrowPen;

  // set painter (initialized with pixmap) and set render hints, pen and brush
//  QPainter painter;
//  painter.begin(pixmap);

  painter.begin(&pixmap);
  painter.setRenderHints(QPainter::Antialiasing,false);
  painter.setPen(arrowPen);
  painter.setBrush(Qt::transparent);

  // draw upper and lower arrow arcs
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

  // draw upper and lower arrow heads
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

  // set icon brush, pen and render hints
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

  // set icon background colour
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

  // set icon border attribute
  if (borderData.type == BorderData::BdrNone) {
      penColor = Qt::transparent;
    } else {
      penColor =  LDrawColor::color(borderData.color);
    }

  // set icon border demensions
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

  // draw icon rectangle
  if (borderData.type == BorderData::BdrRound) {
      painter.drawRoundRect(irect,int(rx),int(ry));
    } else {
      painter.drawRect(irect);
    }

  painter.end();
}
