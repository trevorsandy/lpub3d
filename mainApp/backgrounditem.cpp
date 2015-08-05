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

  bool useGradient = false;

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
    case BackgroundData::BgGradient:
      useGradient = true;
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

  if (borderData.type == BorderData::BdrNone) {
      penColor = Qt::transparent;
    } else {
      penColor =  LDrawColor::color(borderData.color);
    }

  QPen pen;
  pen.setColor(penColor);
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::RoundJoin);
  if (borderData.line == BorderData::BdrLnSolid){
        pen.setStyle(Qt::SolidLine);
    }
  else if (borderData.line == BorderData::BdrLnDash){
      pen.setStyle(Qt::DashLine);
    }
  else if (borderData.line == BorderData::BdrLnDot){
      pen.setStyle(Qt::DotLine);
    }
  else if (borderData.line == BorderData::BdrLnDashDot){
      pen.setStyle(Qt::DashDotLine);
    }
  else if (borderData.line == BorderData::BdrLnDashDotDot){
      pen.setStyle(Qt::DashDotDotLine);
    }
  pen.setWidth(bt);

  painter.setPen(pen);
  useGradient ? painter.setBrush(QBrush(setGradient())) :
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

QGradient BackgroundItem::setGradient(){

  BackgroundData backgroundData = background.value();

  QGradient g;
  QPolygonF pts;
  QGradientStops stops;
  QGradient::Spread spread;
  QGradient::CoordinateMode mode;

  QSize gSize(backgroundData.gsize[0],backgroundData.gsize[1]);

//  int h_off = gSize.width() / 10;
//  int v_off = gSize.height() / 8;
//  pts << QPointF(gSize.width() / 2, gSize.height() / 2)
//      << QPointF(gSize.width() / 2 - h_off, gSize.height() / 2 - v_off);

  for (int i=0; i<backgroundData.gpoints.size(); i++)
    pts.append(backgroundData.gpoints.at(i));

  switch (backgroundData.gmode){
    case BackgroundData::LogicalMode:
      mode = QGradient::LogicalMode;
    break;
    case BackgroundData::StretchToDeviceMode:
      mode = QGradient::StretchToDeviceMode;
    break;
    case BackgroundData::ObjectBoundingMode:
      mode = QGradient::ObjectBoundingMode;
    break;
    }

  switch (backgroundData.gspread){
    case BackgroundData::PadSpread:
      spread = QGradient::PadSpread;
    break;
    case BackgroundData::RepeatSpread:
      spread = QGradient::RepeatSpread;
    break;
    case BackgroundData::ReflectSpread:
      spread = QGradient::ReflectSpread;
    break;
    }

  switch (backgroundData.gtype){
    case BackgroundData::LinearGradient:
      g = QLinearGradient(pts.at(0), pts.at(1));
    break;
    case BackgroundData::RadialGradient:
      {
        QLineF line(pts[0], pts[1]);
        if (line.length() > 132){
            line.setLength(132);
          }
        g = QRadialGradient(line.p1(),  qMin(gSize.width(), gSize.height()) / 3.0, line.p2());
      }
    break;
    case BackgroundData::ConicalGradient:
      {
//        QLineF l(pts.at(0), pts.at(1));
//        qreal angle = l.angle(QLineF(0, 0, 1, 0));
//        if (l.dy() > 0)
//          angle = 360 - angle;
        qreal angle = backgroundData.gangle;
        g = QConicalGradient(pts.at(0), angle);
      }
    break;
    case BackgroundData::NoGradient:
    break;
    }

  for (int i=0; i<backgroundData.gstops.size(); ++i) {
      stops.append(backgroundData.gstops.at(i));
    }

  g.setStops(stops);
  g.setSpread(spread);
  g.setCoordinateMode(mode);

  return g;
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
