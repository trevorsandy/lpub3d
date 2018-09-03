/****************************************************************************
**
** Copyright (C) 2016 - 2018 Trevor SANDY. All rights reserved.
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
 * This class implements the graphical pointers that extend from the divider to
 * as visual indicators to the builder as to where what the referenced
 * item is associated with on the page.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "dividerpointeritem.h"
#include "step.h"
#include "range.h"
#include "ranges.h"


DividerPointerItem::DividerPointerItem(
  DividerItem   *divider,
  Meta          *_meta,
  Pointer       *_pointer,
  QGraphicsItem *parent,
  QGraphicsView *_view)

  : PointerItem(parent)
{
  meta                = _meta;
  view                = _view;
  pointer             = *_pointer;
  step                = divider->step;
  steps               = divider->step->grandparent();
  dividerSize         = divider->boundingRect();
  sepData             = step->range()->sepMeta.valuePixels();
  parentRelativeType  = steps->placement.value().relativeTo;
  QLineF  dividerLine = divider->lineItem->line();
  dividerLoc          =  dividerLine.p1();

  PointerData pointerData = pointer.pointerMeta.value();
  PlacementData dividerPlacement;
  if (parentRelativeType == CalloutType){
      dividerPlacement = meta->LPub.callout.placement.value();
    } else {
      dividerPlacement = meta->LPub.multiStep.placement.value();
    }

  borderColor     = sepData.color;
  borderThickness = sepData.thickness;

  placement       = pointerData.placement;

  baseX           = dividerSize.x();
  baseY           = dividerSize.y();

  if (pointerData.segments == OneSegment) {
      int cX = step->csiItem->loc[XX];
      int cY = step->csiItem->loc[YY];
      int dX = pointerData.x1*step->csiItem->size[XX];
      int dY = pointerData.y1*step->csiItem->size[YY];

      if (steps->placement.value().relativeTo == CalloutType) {
          cX += step->loc[XX];
          cY += step->loc[YY];
          points[Tip] = QPoint(cX + dX - dividerLoc.x(), cY + dY - dividerLoc.y());
      } else {
          points[Tip] = QPoint(cX + dX - dividerLoc.x(), cY + dY - dividerLoc.y());
      }

       /*
       * What does it take to convert csiItem->loc[] and size[] to the position of
       * the tip in these cases:
       *   single step
       *   step group
       */
      if ( ! step->onlyChild()) {
          switch (dividerPlacement.relativeTo) {
          case PageType:
          case StepGroupType:
              points[Tip] += QPoint(step->grandparent()->loc[XX],
                                    step->grandparent()->loc[YY]);
              points[Tip] += QPoint(step->range()->loc[XX],
                                    step->range()->loc[YY]);
              points[Tip] += QPoint(step->loc[XX],
                                    step->loc[YY]);
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

  QColor qColor(borderColor);
  QPen pen(qColor);
  pen.setWidth(borderThickness);
  pen.setCapStyle(Qt::RoundCap);

  // shaft segments
  for (int i = 0; i < pointerData.segments; i++) {
      QLineF linef;
      shaft = new QGraphicsLineItem(linef,this);
      shaft->setPen(pen);
      shaft->setZValue(-5);
      shaft->setFlag(QGraphicsItem::ItemIsSelectable,false);
      shaft->setToolTip(QString("Pointer segment %1 - drag to move; right click to modify").arg(i+1));
      shaftSegments.append(shaft);
      addToGroup(shaft);
  }

  autoLocFromTip();

  QPolygonF poly;

  head = new QGraphicsPolygonItem(poly, this);
  head->setPen(qColor);
  head->setBrush(qColor);
  head->setFlag(QGraphicsItem::ItemIsSelectable,false);
  head->setToolTip("Pointer head - drag to move");
  addToGroup(head);

  for (int i = 0; i < NumGrabbers; i++) {
    grabbers[i] = NULL;
  }

  drawPointerPoly();
  setFlag(QGraphicsItem::ItemIsFocusable,true);
}

void DividerPointerItem::defaultPointer(){
  points[Tip] = QPointF(step->csiItem->loc[XX]+
                        step->csiItem->size[XX]/2,
                        step->csiItem->loc[YY]+
                        step->csiItem->size[YY]/2);

  if ( ! step->onlyChild()) {
      PlacementData dividerPlacement;
      if (parentRelativeType == CalloutType){
          dividerPlacement = meta->LPub.callout.placement.value();
        } else {
          dividerPlacement = meta->LPub.multiStep.placement.value();
        }
    switch (dividerPlacement.relativeTo) {
      case PageType:
      case StepGroupType:
        points[Tip] += QPoint(step->grandparent()->loc[XX],
                              step->grandparent()->loc[YY]);
        points[Tip] += QPoint(step->range()->loc[XX],
                              step->range()->loc[YY]);
        points[Tip] += QPoint(step->loc[XX],
                              step->loc[YY]);
        points[Tip] -= QPoint(dividerLoc.x(),dividerLoc.y());
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

/*
 * Given the location of the Tip (as dragged around by the user)
 * calculate a reasonable placement and Loc for Base or MidTip points.
 */
bool DividerPointerItem::autoLocFromTip(){
  int width = dividerSize.x();
  int height = dividerSize.y();
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
      int mx = pointerData.x3;
      points[Base] = QPointF(pointerData.x2,pointerData.y2);
      placement = pointerData.placement;

      if (segments() == ThreeSegments){
          points[MidTip].setY(ty);
          points[MidTip].setX(mx);
      }
  } else {
      /* Figure out which corner */
      int radius = 10;

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

bool DividerPointerItem::autoLocFromMidBase(){
  int width = dividerSize.x();
  int height = dividerSize.y();
  int left = 0;
  int right = width;
  int top = 0;
  int bottom = height;

  QPoint intersect;
  int tx,ty;

  tx = points[MidBase].x();
  ty = points[MidBase].y();

  /* Figure out which corner */
  int radius = 10;

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

void DividerPointerItem::calculatePointerMetaLoc(){
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
         loc = points[Base].x()/dividerSize.x();
      else
         loc = points[Base].x();
    }
    break;
    case Left:
    case Right:
    {
      if (segments() == OneSegment)
         loc = points[Base].y()/dividerSize.y();
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

void DividerPointerItem::calculatePointerMeta(){
  calculatePointerMetaLoc();

  PointerData pointerData = pointer.pointerMeta.value();
  if (segments() == OneSegment) {
      if (step->onlyChild()) {
          points[Tip] += QPoint(dividerLoc.x(),dividerLoc.y());
      } else {
          PlacementData dividerPlacement;
          if (parentRelativeType == CalloutType){
              dividerPlacement = meta->LPub.callout.placement.value();
            } else {
              dividerPlacement = meta->LPub.multiStep.placement.value();
            }
          switch (dividerPlacement.relativeTo) {
          case CsiType:
          case PartsListType:
          case StepNumberType:
              points[Tip] += QPoint(dividerLoc.x(),dividerLoc.y());
              break;
          case PageType:
          case StepGroupType:
              points[Tip] -= QPoint(step->grandparent()->loc[XX],
                                    step->grandparent()->loc[YY]);
              points[Tip] -= QPoint(step->loc[XX],
                                    step->loc[YY]);
              points[Tip] += QPoint(dividerLoc.x(),dividerLoc.y());
              break;
          default:
              break;
          }
      }

      if (parentRelativeType == CalloutType) {
          points[Tip] -= QPoint(step->loc[XX],
                                step->loc[YY]);
      }

      pointerData.x1 = (points[Tip].x() - step->csiItem->loc[XX])/step->csiItem->size[XX];
      pointerData.y1 = (points[Tip].y() - step->csiItem->loc[YY])/step->csiItem->size[YY];
  } else {
      pointerData.x1 = points[Tip].x();
      pointerData.y1 = points[Tip].y();
      pointerData.x2 = points[Base].x();
      pointerData.y2 = points[Base].y();
      pointerData.x3 = points[MidBase].x();
      pointerData.y3 = points[MidBase].y();
      pointerData.x4 = points[MidTip].x();
      pointerData.y4 = points[MidTip].y();
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
