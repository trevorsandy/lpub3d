
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2016 - 2019 Trevor SANDY. All rights reserved.
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
 * This class implements the graphical pointers that extend from callouts to
 * assembly images as visual indicators to the builder as to where to 
 * add the completed submodel into partially assembled final model.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "callout.h"
#include "calloutpointeritem.h"
#include "calloutbackgrounditem.h"
#include "step.h"
#include "range.h"

//---------------------------------------------------------------------------

/*
 * This is the constructor of a graphical pointer
 */

CalloutPointerItem::CalloutPointerItem(
  Callout       *co,
  Pointer       *_pointer,
  QGraphicsItem *parent,
  QGraphicsView *_view)

  : PointerItem(parent)
{
  meta              = &co->meta;
  view              = _view;
  pointer           = *_pointer;
  callout           = co;
  pointerTop        = co->topOfCallout();
  pointerBottom     = co->bottomOfCallout();
  pointerParentType = CalloutType;
  resizeRequested   = false;

  PlacementData calloutPlacement = co->meta.LPub.callout.placement.value();

  PointerData pointerData = pointer.pointerMeta.value();

  placement       = pointerData.placement;

  baseX           = co->size[XX];
  baseY           = co->size[YY];

  if (pointerData.segments == OneSegment) {
      int cX = callout->parentStep->csiItem->loc[XX];
      int cY = callout->parentStep->csiItem->loc[YY];
      int dX = pointerData.x1*callout->parentStep->csiItem->size[XX];
      int dY = pointerData.y1*callout->parentStep->csiItem->size[YY];

      if (callout->placement.value().relativeTo == CalloutType) {
          cX += callout->parentStep->loc[XX];
          cY += callout->parentStep->loc[YY];
          points[Tip] = QPoint(cX + dX - callout->loc[XX], cY + dY - callout->loc[YY]);
      } else {
          points[Tip] = QPoint(cX + dX - callout->loc[XX], cY + dY - callout->loc[YY]);
      }
       /*
       * What does it take to convert csiItem->loc[] and size[] to the position of
       * the tip in these cases:
       *   single step
       *     callout relative to csi
       *     callout relative to stepNumber
       *     callout relative to pli
       *     callout relative to page
       *
       *   step group
       *     callout relative to csi
       *     callout relative to stepNumber
       *     callout relative to pli
       *     callout relative to page
       *     callout relative to stepGroup
       */
      if ( ! callout->parentStep->onlyChild()) {
          switch (calloutPlacement.relativeTo) {
          case PageType:
          case StepGroupType:
              points[Tip] += QPoint(callout->parentStep->grandparent()->loc[XX],
                                    callout->parentStep->grandparent()->loc[YY]);
              points[Tip] += QPoint(callout->parentStep->range()->loc[XX],
                                    callout->parentStep->range()->loc[YY]);
              points[Tip] += QPoint(callout->parentStep->loc[XX],
                                    callout->parentStep->loc[YY]);
              break;
          default:
              break;
          }
      }
  } else {
      points[Tip] = QPointF(pointerData.x1, pointerData.y1);
  }

  points[MidBase] = QPointF(pointerData.x3,pointerData.y3);
  points[MidTip]  = QPointF(pointerData.x4,pointerData.y4);

  PointerAttribData *pad = &pointer.pointerAttrib.valuePixels();

  QColor shaftPenColor(pad->lineData.color);
  QColor brushColor(pad->lineData.color);

  QPen shaftPen(shaftPenColor);
  shaftPen.setWidth(pad->lineData.thickness);
  shaftPen.setCapStyle(Qt::RoundCap);
  shaftPen.setJoinStyle(Qt::RoundJoin);
  if (pad->lineData.line == BorderData::BdrLnSolid){
      shaftPen.setStyle(Qt::SolidLine);
  }
  else if (pad->lineData.line == BorderData::BdrLnDash){
      shaftPen.setStyle(Qt::DashLine);
  }
  else if (pad->lineData.line == BorderData::BdrLnDot){
      shaftPen.setStyle(Qt::DotLine);
  }
  else if (pad->lineData.line == BorderData::BdrLnDashDot){
      shaftPen.setStyle(Qt::DashDotLine);
  }
  else if (pad->lineData.line == BorderData::BdrLnDashDotDot){
      shaftPen.setStyle(Qt::DashDotDotLine);
  }

  // shaft segments
  for (int i = 0; i < pointerData.segments; i++) {
      QLineF linef;
      shaft = new BorderedLineItem(linef,pad,this);
      shaft->setPen(shaftPen);
      shaft->setFlag(QGraphicsItem::ItemIsSelectable,false);
      shaft->setToolTip(QString("Pointer segment %1 - drag to move; right click to modify").arg(i+1));
      shaftSegments.append(shaft);
      addToGroup(shaft);
  }

  autoLocFromTip();

  QPolygonF poly;
  
  QColor tipPenColor(pad->lineData.color);
  if (! pad->borderData.useDefault)
      tipPenColor.setNamedColor(pad->borderData.color);

  QPen tipPen(tipPenColor);
  tipPen.setWidth(pad->borderData.thickness);
  tipPen.setCapStyle(Qt::RoundCap);
  tipPen.setJoinStyle(Qt::RoundJoin);
  if (pad->borderData.line == BorderData::BdrLnSolid){
      tipPen.setStyle(Qt::SolidLine);
  }
  else if (pad->borderData.line == BorderData::BdrLnDash){
      tipPen.setStyle(Qt::DashLine);
  }
  else if (pad->borderData.line == BorderData::BdrLnDot){
      tipPen.setStyle(Qt::DotLine);
  }
  else if (pad->borderData.line == BorderData::BdrLnDashDot){
      tipPen.setStyle(Qt::DashDotLine);
  }
  else if (pad->borderData.line == BorderData::BdrLnDashDotDot){
      tipPen.setStyle(Qt::DashDotDotLine);
  }

  head = new QGraphicsPolygonItem(poly, this);
  head->setPen(tipPen);
  head->setBrush(brushColor);
  head->setFlag(QGraphicsItem::ItemIsSelectable,false);
  head->setToolTip("Pointer head - drag to move");
  addToGroup(head);  
  
  for (int i = 0; i < NumPointerGrabbers; i++) {
    grabbers[i] = nullptr;
  }

  setFlag(QGraphicsItem::ItemIsFocusable,true);
  setData(ObjectId, CalloutPointerObj);
  setZValue(meta->LPub.page.scene.calloutPointer.zValue());

  drawPointerPoly();
}

/*
 * Given the location of the Tip (as dragged around by the user)
 * calculate a reasonable placement and Loc for Base or MidTip points.
 */

bool CalloutPointerItem::autoLocFromTip()
{        
    int width = callout->size[XX];
    int height = callout->size[YY];
    int left = 0;
    int right = width;
    int top = 0;
    int bottom = height;

    QPoint intersect;
    int tx,ty;

    tx = points[Tip].x();
    ty = points[Tip].y();

    if (segments() != OneSegment) {
        PointerData pointerData = pointer.pointerMeta.value();
        placement = pointerData.placement;

        if (!resizeRequested)
            points[Base] = QPointF(pointerData.x2,pointerData.y2);

        if (segments() == ThreeSegments){
            int mtx = pointerData.x3;
            points[MidTip].setY(ty);
            points[MidTip].setX(mtx);
        }
    } else {
        /* Figure out which corner */
        BorderData borderData = callout->background->border.valuePixels();
        int radius = (int) borderData.radius;

        if (ty >= top+radius && ty <= bottom-radius) {
            if (tx < left) {
                intersect = QPoint(left,ty);
                points[Tip].setY(ty);
                placement = Left;
            } else if (tx > right) {
                intersect = QPoint(right,ty);
                points[Tip].setY(ty);
                placement = Right;
            } else {
                // inside
                placement = Center;
            }
        } else if (tx >= left+radius && tx <= right-radius) {
            if (ty < top) {
                intersect = QPoint(tx,top);
                points[Tip].setX(tx);
                placement = Top;
            } else if (ty > bottom) {
                intersect = QPoint(tx,bottom);
                points[Tip].setX(tx);
                placement = Bottom;
            } else {
                // inside
                placement = Center;
            }
        } else if (tx < radius) {  // left?
            if (ty < radius) {
                intersect = QPoint(left+radius,top+radius);
                placement = TopLeft;
            } else {
                intersect = QPoint(radius,height-radius);
                placement = BottomLeft;
            }
        } else { // right!
            if (ty < radius) {
                intersect = QPoint(width-radius,radius);
                placement = TopRight;
            } else {
                intersect = QPoint(width-radius,height-radius);
                placement = BottomRight;
            }
        }

        points[Base] = intersect;
    }

    return true;
}

/*
 * Given the location of the MidBase point (as dragged around by the user)
 * calculate a reasonable placement and Loc for Base point.
 */

bool CalloutPointerItem::autoLocFromMidBase()
{
        int width = callout->size[XX];
        int height = callout->size[YY];
        int left = 0;
        int right = width;
        int top = 0;
        int bottom = height;

        QPoint intersect;
        int tx,ty;

        tx = points[MidBase].x();
        ty = points[MidBase].y();

        /* Figure out which corner */
        BorderData borderData = callout->background->border.valuePixels();
        int radius = (int) borderData.radius;

        if (ty >= top+radius && ty <= bottom-radius) {
            if (tx < left) {
                intersect = QPoint(left,ty);
                points[MidBase].setY(ty);
                placement = Left;
            } else if (tx > right) {
                intersect = QPoint(right,ty);
                points[MidBase].setY(ty);
                placement = Right;
            } else {
                // inside
                placement = Center;
            }
        } else if (tx >= left+radius && tx <= right-radius) {
            if (ty < top) {
                intersect = QPoint(tx,top);
                points[MidBase].setX(tx);
                placement = Top;
            } else if (ty > bottom) {
                intersect = QPoint(tx,bottom);
                points[MidBase].setX(tx);
                placement = Bottom;
            } else {
                // inside
                placement = Center;
            }
        } else if (tx < radius) {  // left?
            if (ty < radius) {
                intersect = QPoint(left+radius,top+radius);
                placement = TopLeft;
            } else {
                intersect = QPoint(radius,height-radius);
                placement = BottomLeft;
            }
        } else { // right!
            if (ty < radius) {
                intersect = QPoint(width-radius,radius);
                placement = TopRight;
            } else {
                intersect = QPoint(width-radius,height-radius);
                placement = BottomRight;
            }
        }

        points[MidTip].setX(tx);
        points[Base] = intersect;

    return true;
}

void CalloutPointerItem::defaultPointer()
{
  points[Tip] = QPointF(callout->parentStep->csiItem->loc[XX]+
                        callout->parentStep->csiItem->size[XX]/2,
                        callout->parentStep->csiItem->loc[YY]+
                        callout->parentStep->csiItem->size[YY]/2);

  if ( ! callout->parentStep->onlyChild()) {
    PlacementData calloutPlacement = callout->placement.value();
    switch (calloutPlacement.relativeTo) {
      case PageType:
      case StepGroupType:
        points[Tip] += QPoint(callout->parentStep->grandparent()->loc[XX],
                              callout->parentStep->grandparent()->loc[YY]);
        points[Tip] += QPoint(callout->parentStep->range()->loc[XX],
                              callout->parentStep->range()->loc[YY]);
        points[Tip] += QPoint(callout->parentStep->loc[XX],
                              callout->parentStep->loc[YY]);
        points[Tip] -= QPoint(callout->loc[XX],callout->loc[YY]);
      break;
      default:
      break;
    }
  }
  autoLocFromTip();
  drawPointerPoly();
  calculatePointerMeta();
  addPointerMeta();
}

void CalloutPointerItem::calculatePointerMetaLoc()
{
  float loc = 0;

  switch (placement) {
    case TopLeft:
    case TopRight:
    case BottomLeft:
    case BottomRight:
      loc = 0;
    break;
    case Top:
    case Bottom:
    {
      if (segments() == OneSegment)
         loc = points[Base].x()/callout->size[XX];
      else
         loc = points[Base].x();
    }
    break;
    case Left:
    case Right:
    {
      if (segments() == OneSegment)
         loc = points[Base].y()/callout->size[YY];
      else
         loc = points[Base].y();
    }
    break;
    default:
    break;
  }
  PointerData pointerData = pointer.pointerMeta.value();
  pointer.pointerMeta.setValue(
    placement,
    loc,
    0,
    segments(),
    pointerData.x1,
    pointerData.y1,
    pointerData.x2,
    pointerData.y2,
    pointerData.x3,
    pointerData.y3,
    pointerData.x4,
    pointerData.y4);
}

void CalloutPointerItem::calculatePointerMeta()
{
  calculatePointerMetaLoc();

  PointerData pointerData = pointer.pointerMeta.value();
  if (segments() == OneSegment) {
      if (callout->parentStep->onlyChild()) {
          points[Tip] += QPoint(callout->loc[XX],callout->loc[YY]);
      } else {
          PlacementData calloutPlacement = callout->meta.LPub.callout.placement.value();

          switch (calloutPlacement.relativeTo) {
          case CsiType:
          case PartsListType:
          case StepNumberType:
              points[Tip] += QPoint(callout->loc[XX],callout->loc[YY]);
              break;
          case PageType:
          case StepGroupType:
              points[Tip] -= QPoint(callout->parentStep->grandparent()->loc[XX],
                                    callout->parentStep->grandparent()->loc[YY]);
              points[Tip] -= QPoint(callout->parentStep->loc[XX],
                                    callout->parentStep->loc[YY]);
              points[Tip] += QPoint(callout->loc[XX],callout->loc[YY]);
              break;
          default:
              break;
          }
      }

      if (callout->placement.value().relativeTo == CalloutType) {
          points[Tip] -= QPoint(callout->parentStep->loc[XX],
                                callout->parentStep->loc[YY]);
      }

      pointerData.x1 = float(points[Tip].x() - callout->parentStep->csiItem->loc[XX])/callout->parentStep->csiItem->size[XX];
      pointerData.y1 = float(points[Tip].y() - callout->parentStep->csiItem->loc[YY])/callout->parentStep->csiItem->size[YY];
  } else {
      pointerData.x1 = float(points[Tip].x());
      pointerData.y1 = float(points[Tip].y());
      pointerData.x2 = float(points[Base].x());
      pointerData.y2 = float(points[Base].y());
      pointerData.x3 = float(points[MidBase].x());
      pointerData.y3 = float(points[MidBase].y());
      pointerData.x4 = float(points[MidTip].x());
      pointerData.y4 = float(points[MidTip].y());
  }
  pointer.pointerMeta.setValue(
    placement,
    pointerData.loc,
    0,
    segments(),
    pointerData.x1,
    pointerData.y1,
    pointerData.x2,
    pointerData.y2,
    pointerData.x3,
    pointerData.y3,
    pointerData.x4,
    pointerData.y4);
}
