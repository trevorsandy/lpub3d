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

/****************************************************************************
 *
 * This file describes a class that implements an LPub background.
 * Page, Parts Lists and Callouts have backgrounds.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "backgrounditem.h"
#include "meta.h"
#include <QColor>
#include <QPixmap>
#include <QBitmap>
#include <QAction>
#include <QMenu>
#include <QPainter>
#include <QFile>
#include "color.h"

#include "QsLog.h"

void BackgroundItem::setBackground(
    QPixmap          *pixmap,
    PlacementType    _parentRelativeType,
    Meta            *_meta,
    BackgroundMeta  &_background,
    BorderMeta      &_border,
    MarginsMeta     &_margin,
    StringListMeta  &_subModel,
    int              _submodelLevel,
    QString          &toolTip)
{
  meta          =  _meta;
  background    =  _background;
  border        =  _border;
  margin        =  _margin;
  subModelColor =  _subModel;
  submodelLevel =  _submodelLevel;
  parentRelativeType = _parentRelativeType;

  BorderData     borderData     = _border.valuePixels();
  BackgroundData backgroundData = _background.value();

  int bt = int(borderData.thickness);

  QColor penColor,brushColor;
  QRectF prect(bt/2,bt/2,pixmap->width()-bt,pixmap->height()-bt); // was -1-bt


  pixmap->setAlphaChannel(*pixmap);
  pixmap->fill(Qt::transparent);

  QPainter painter(pixmap);

  switch(backgroundData.type) {
    case BackgroundData::BgImage:
      {
        QString image_name = backgroundData.string;
        QFile file(image_name);

        if ( ! file.exists()) {
            return;
          }

        QImage image(image_name);
        if (backgroundData.stretch) {
            QSize psize = pixmap->size();
            QSize isize = image.size();
            qreal sx = psize.width();
            qreal sy = psize.height();
            sx /= isize.width();
            sy /= isize.height();
            painter.scale(sx,sy);
            painter.drawImage(0,0,image);
          } else {
            for (int y = 0; y < pixmap->height(); y += image.height()) {
                for (int x = 0; x < pixmap->width(); x += image.width()) {
                    painter.drawImage(x,y,image);
                  }
              }
          }
        brushColor = Qt::transparent;
      }
      break;
    case BackgroundData::BgTransparent:
      brushColor = Qt::transparent;
      break;
    case BackgroundData::BgColor:
    case BackgroundData::BgSubmodelColor:
      if (backgroundData.type == BackgroundData::BgColor) {
          brushColor = LDrawColor::color(backgroundData.string);
        } else {
          brushColor = LDrawColor::color(_subModel.value(submodelLevel));
        }
      break;
    }

  qreal rx = borderData.radius;
  qreal ry = borderData.radius;
  qreal dx = pixmap->width();
  qreal dy = pixmap->height();

  if (dx && dy) {
      if (dx > dy) {
          // the rx is going to appear larger that ry, so decrease rx based on ratio
          rx *= dy;
          rx /= dx;
        } else {
          ry *= dx;
          ry /= dy;
        }
    }

    if (isRotationIcon){
        QRectF arect(0,0,50,50);
        qreal arrowTipLength = 9.0;
        qreal arrowTipHeight = 4.0;
        QPolygonF arrowHead;
        arrowHead << QPointF()
                  << QPointF(arrowTipLength + 2.5, -arrowTipHeight)
                  << QPointF(arrowTipLength      , 0.0)
                  << QPointF(arrowTipLength + 2.5,  arrowTipHeight);

        QColor arrowPenColor;
        arrowPenColor = LDrawColor::color(arrowColour.value());
        QPen defaultArrowPen;
        defaultArrowPen.setColor(arrowPenColor);
        defaultArrowPen.setCapStyle(Qt::SquareCap);
        defaultArrowPen.setJoinStyle(Qt::MiterJoin);
        defaultArrowPen.setStyle(Qt::SolidLine);
        defaultArrowPen.setWidth(3);

        QPen arrowPen = defaultArrowPen;
        painter.setRenderHints(QPainter::Antialiasing,false);
        painter.setPen(arrowPen);
        painter.setBrush(Qt::transparent);

//        qreal aw = prect.width();
//        qreal ah = prect.height() / 2.0;
        qreal aw = arect.width();
        qreal ah = arect.height() / 2.0;
        float inset = 6.0;

        // 1.8 & 2.5 entirely qualitative, for a 'nice' curve
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

      }

  if (borderData.type == BorderData::BdrNone) {
      penColor = Qt::transparent;
    } else {
      penColor =  LDrawColor::color(borderData.color);
    }

  QPen pen;
  pen.setColor(penColor);
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::RoundJoin);
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(bt);

  painter.setPen(pen);
  painter.setBrush(brushColor);

  painter.setRenderHints(QPainter::HighQualityAntialiasing,true);
  painter.setRenderHints(QPainter::Antialiasing,true);

  if (borderData.type == BorderData::BdrRound) {
      painter.drawRoundRect(prect,int(rx),int(ry));
    } else {
      painter.drawRect(prect);
    }

  setToolTip(toolTip);
  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsMovable,true);
}

void BackgroundItem::setRotateIconBackground(
    QPixmap          *pixmap,
    PlacementType    _parentRelativeType,
    Meta            *_meta,
    StringMeta      &_arrowColour,
    BackgroundMeta  &_background,
    BorderMeta      &_border,
    MarginsMeta     &_margin,
    StringListMeta  &_subModel,
    int              _submodelLevel,
    QString          &toolTip)
{
  arrowColour    = _arrowColour;
  isRotationIcon = true;

  BackgroundItem::setBackground( pixmap,
                                _parentRelativeType,
                                _meta,
                                _background,
                                _border,
                                _margin,
                                _subModel,
                                _submodelLevel,
                                toolTip);
}

void PlacementBackgroundItem::setBackground(
    QPixmap                 *pixmap,
    PlacementType           _relativeType,
    PlacementType           _parentRelativeType,
    PlacementMeta           &_placement,
    BackgroundMeta          &_background,
    BorderMeta              &_border,
    MarginsMeta             &_margin,
    StringListMeta          &_subModel,
    int                      _submodelLevel,
    QString                 &toolTip)
{
  relativeType = _relativeType;
  placement    = _placement;

  BackgroundItem::setBackground(pixmap,
                                _parentRelativeType,
                                meta,
                                _background,
                                _border,
                                _margin,
                                _subModel,
                                _submodelLevel,
                                toolTip);

}

void PlacementBackgroundItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mousePressEvent(event);
  positionChanged = false;
  position = pos();
}

void PlacementBackgroundItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseMoveEvent(event);
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
      positionChanged = true;
    }
}

void PlacementBackgroundItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable) && positionChanged) {
      QPointF newPosition;

      // back annotate the movement of the PLI into the LDraw file.
      newPosition = pos() - position;
      PlacementData placementData = placement.value();
      placementData.offsets[0] += newPosition.x()/meta->LPub.page.size.valuePixels(0);
      placementData.offsets[1] += newPosition.y()/meta->LPub.page.size.valuePixels(1);
      placement.setValue(placementData);
    }
}
