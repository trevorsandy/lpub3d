/****************************************************************************
**
** Copyright (C) 2016 - 2019 Trevor SANDY. All rights reserved.
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

#include <QDebug>
#include <QGraphicsItem>
#include <QPoint>
#include <QPainter>
#include <QPen>
#include <QGraphicsSceneMouseEvent>
#include "lgraphicsscene.h"

#include "lpubalert.h"

LGraphicsScene::LGraphicsScene(QObject *parent)
  : QGraphicsScene(parent),
    guidePen(QPen(Qt::darkGray, 1)),
    mPageGuides(false),
    mObjCornerSet(false),
    mQGItem(nullptr),
    mQGPixmapItem(nullptr),
    mQGTextItem(nullptr),
    mQGRectItem(nullptr),
    mQGLineItem(nullptr),
    mQGPolyItem(nullptr)
{}

const QString LGraphicsScene::GItemNames[LGraphicsScene::GNumItems] =
{
  "Graphic Pixmap Item",     //00
  "Graphic Text Item",       //01
  "Graphic Rectangle Item",  //02
  "Graphic Line Item",       //03
  "Graphic Polygon Item",    //04
  "Graphic Item",            //05
}; //NumItems

void LGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
  updateGuidePos(event->scenePos());
  QGraphicsScene::mouseMoveEvent(event);
}

void LGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    mObjCornerSet = false;
  QGraphicsScene::mouseReleaseEvent(event);
}

void LGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  if (event->button() == Qt::LeftButton){
      mObjCornerSet = setObjCornerPos(event->scenePos());
    }
  QGraphicsScene::mousePressEvent(event);
}

void LGraphicsScene::drawForeground(QPainter *painter, const QRectF &rect){
  if (! mPageGuides || ! mObjCornerSet)
    return;

  painter->setClipRect(rect);
  painter->setRenderHints(QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing);
  painter->setPen(guidePen);
  painter->drawLine(mGuidePos.x(), rect.top(), mGuidePos.x(), rect.bottom());
  painter->drawLine(rect.left(), mGuidePos.y(), rect.right(), mGuidePos.y());
  update();
}

void LGraphicsScene::updateGuidePos(const QPointF &scenePos){
  if (mPageGuides) {
      if (mObjCornerSet) {
          // top, right bounding rect, Qt Y-axis increases going down so use '-' to go up to top
          mGuidePos = QPointF((scenePos.x() - mAdjustX),(scenePos.y() - mAdjustY));

#ifdef QT_DEBUG_MODE
//          qDebug() << "LG Scene Position Adjusted."
//                   << "\nscenePos  [" << QPointF(scenePos) << "]"
//                   << "\nmGuidePos [" << mGuidePos << "]"
//                   << "\nscenePos.x(" << scenePos.x() << ") - mAdjustX(" << mAdjustX << ") = mGuidePos.x[" << mGuidePos.x() << "]"
//                   << "\nscenePos.y(" << scenePos.y() << ") - mAdjustY(" << mAdjustY << ") = mGuidePos.y[" << mGuidePos.y() << "]"
//                      ;
#endif

        }
      invalidate(this->sceneRect(),QGraphicsScene::ForegroundLayer);
      update();
    }
}

bool LGraphicsScene::setObjCornerPos(const QPointF &scenePos){
  QRectF bB;  // Bounding Rectangle
  ObjItem objItem = setObjItem(scenePos);
  switch(objItem)
    {
    case GPixmapItem:
      bB = mQGPixmapItem->sceneBoundingRect();
      break;
    case GTextItem:
      bB = mQGTextItem->sceneBoundingRect();
      break;
    case GRectItem:
      bB = mQGRectItem->sceneBoundingRect();
      break;
    case GLineItem:
      bB = mQGLineItem->sceneBoundingRect();
      break;
    case GPolyItem:
      bB = mQGPolyItem->sceneBoundingRect();
      break;
    case GBaseItem:
      bB = mQGItem->sceneBoundingRect();
      break;
    default:
      return false;
    }

  mAdjustX = bB.width()/2;     // Adjustment Point X
  mAdjustY = bB.height()/2;    // Adjustment Point Y

  updateGuidePos(scenePos);

#ifdef QT_DEBUG_MODE
//  qDebug() << "LG Scene Positon At Cursor [" << scenePos << "]"
//           << "\nX Adjustment: ObjType (" << GItemNames[objItem] << "), Width  (" << bB.width() << "/2) = [" << mAdjustX << "]"
//           << "\nY Adjustment: ObjType (" << GItemNames[objItem] << "), Height (" << bB.height() << "/2) = [" << mAdjustY << "]"
//              ;
#endif

  return true;
}

LGraphicsScene::ObjItem LGraphicsScene::setObjItem(const QPointF &scenePos){
  mQGItem = itemAt(scenePos, QTransform());
  mQGPixmapItem = qgraphicsitem_cast<QGraphicsPixmapItem *>(mQGItem);
  if(mQGPixmapItem)
    return GPixmapItem;

  mQGTextItem = qgraphicsitem_cast<QGraphicsTextItem *>(mQGItem);
  if(mQGTextItem)
    return GTextItem;

  mQGRectItem = qgraphicsitem_cast<QGraphicsRectItem *>(mQGItem);
  if(mQGRectItem)
    return GRectItem;

  mQGLineItem = qgraphicsitem_cast<QGraphicsLineItem *>(mQGItem);
  if(mQGLineItem)
    return GLineItem;

  mQGPolyItem = qgraphicsitem_cast<QGraphicsPolygonItem *>(mQGItem);
  if(mQGPolyItem)
    return GPolyItem;

  if(mQGItem)
    return GBaseItem;

  return NoGItem;
}
