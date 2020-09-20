/****************************************************************************
**
** Copyright (C) 2016 - 2020 Trevor SANDY. All rights reserved.
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
#include "ranges_item.h"
#include "step.h"

/*
 * This is the constructor of a graphical pointer
 */

DividerPointerItem::DividerPointerItem(
  DividerItem   *div,
  Pointer       *_pointer,
  QGraphicsItem *parent,
  QGraphicsView *_view)

  : PointerItem(parent)
{
  meta               = &div->meta;
  view               = _view;
  pointer            = *_pointer;
  divider            = div;

  pointerTop         = divider->parentStep->nextStep()->topOfStep();
  pointerBottom      = divider->parentStep->nextStep()->bottomOfStep();
  stepNumber         = divider->parentStep->stepNumber.number;
  pointerParentType  = DividerPointerType;
  resizeRequested    = false;

  PointerData pointerData = pointer.pointerMeta.value();

  placement       = pointerData.placement;

  baseX           = divider->size[XX];
  baseY           = divider->size[YY];

  if (pointerData.segments == OneSegment) {
      int cX = divider->parentStep->csiItem->loc[XX];
      int cY = divider->parentStep->csiItem->loc[YY];
      int dX = pointerData.x1*divider->parentStep->csiItem->size[XX];
      int dY = pointerData.y1*divider->parentStep->csiItem->size[YY];

      if (divider->placement.value().relativeTo == CalloutType) {
          cX += divider->parentStep->loc[XX];
          cY += divider->parentStep->loc[YY];
          points[Tip] = QPoint(cX + dX - divider->loc[XX], cY + dY - divider->loc[YY]);
      } else {
          points[Tip] = QPoint(cX + dX - divider->loc[XX], cY + dY - divider->loc[YY]);
      }

       /*
       * What does it take to convert csiItem->loc[] and size[] to the position of
       * the tip in these cases:
       *   single step
       *   step group
       */
      if ( ! divider->parentStep->onlyChild()) {
          switch (divider->parentRelativeType) {
          case PageType:
          case StepGroupType:
              points[Tip] += QPoint(divider->parentStep->grandparent()->loc[XX],
                                    divider->parentStep->grandparent()->loc[YY]);
              points[Tip] += QPoint(divider->parentStep->range()->loc[XX],
                                    divider->parentStep->range()->loc[YY]);
              points[Tip] += QPoint(divider->parentStep->loc[XX],
                                    divider->parentStep->loc[YY]);
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

  head = new PointerHeadItem(poly, this);
  head->setPen(tipPen);
  head->setBrush(brushColor);
  head->setFlag(QGraphicsItem::ItemIsSelectable,false);
  head->setToolTip("Pointer head - drag to move");
  addToGroup(head);

  for (int i = 0; i < NumPointerGrabbers; i++) {
    if (grabbers[i])
    grabbers[i] = nullptr;
  }

  setFlag(QGraphicsItem::ItemIsFocusable,true);
  setData(ObjectId, DividerPointerObj);
  setZValue(DIVIDERPOINTER_ZVALUE_DEFAULT);

  drawPointerPoly();
}

DividerPointerItem::~DividerPointerItem(){
    shaftSegments.clear();
}

/*
 * Given the location of the Tip (as dragged around by the user)
 * calculate a reasonable placement and Loc for Base or MidTip points.
 */
bool DividerPointerItem::autoLocFromTip(){
  int width = divider->size[XX];
  int height = divider->size[YY];
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
      BorderData borderData = divider->background->border->valuePixels();
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

bool DividerPointerItem::autoLocFromMidBase(){
  int width = divider->size[XX];
  int height = divider->size[YY];
  int left = 0;
  int right = width;
  int top = 0;
  int bottom = height;

  QPoint intersect;
  int tx,ty;

  tx = points[MidBase].x();
  ty = points[MidBase].y();

  /* Figure out which corner */
  BorderData borderData = divider->background->border->valuePixels();
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

void DividerPointerItem::defaultPointer(){
  points[Tip] = QPointF(divider->parentStep->csiItem->loc[XX]+
                        divider->parentStep->csiItem->size[XX]/2,
                        divider->parentStep->csiItem->loc[YY]+
                        divider->parentStep->csiItem->size[YY]/2);

  if ( ! divider->parentStep->onlyChild()) {
    PlacementData dividerPlacement = divider->placement.value();
    switch (dividerPlacement.relativeTo) {
      case PageType:
      case StepGroupType:
      case CalloutType:
        points[Tip] += QPoint(divider->parentStep->grandparent()->loc[XX],
                              divider->parentStep->grandparent()->loc[YY]);
        points[Tip] += QPoint(divider->parentStep->range()->loc[XX],
                              divider->parentStep->range()->loc[YY]);
        points[Tip] += QPoint(divider->parentStep->loc[XX],
                              divider->parentStep->loc[YY]);
        points[Tip] -= QPoint(divider->loc[XX],divider->loc[YY]);
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
         loc = points[Base].x()/divider->size[XX];
      else
         loc = points[Base].x();
    }
    break;
    case Left:
    case Right:
    {
      if (segments() == OneSegment)
         loc = points[Base].y()/divider->size[YY];
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

void DividerPointerItem::calculatePointerMeta()
{
  calculatePointerMetaLoc();

  auto calculateOffset = [this] (SelectedPoint selected, PointerData &pData) {
       float pointerData[2];
       if (segments() == OneSegment && selected == Tip) {
           if (divider->parentStep->onlyChild()) {
               points[selected] += QPoint(divider->loc[XX],divider->loc[YY]);
           } else {
               switch (divider->parentRelativeType) {
               case StepType:
                   points[selected] += QPoint(divider->loc[XX],divider->loc[YY]);
                    break;
               case PageType:
               case StepGroupType:
                   points[selected] -= QPoint(divider->parentStep->grandparent()->loc[XX],
                                         divider->parentStep->grandparent()->loc[YY]);
                   points[selected] -= QPoint(divider->parentStep->loc[XX],
                                         divider->parentStep->loc[YY]);
                   points[selected] += QPoint(divider->loc[XX],divider->loc[YY]);
                   break;
               default:
                   break;
               }
           }
           if (divider->placement.value().relativeTo == StepGroupType ||
               divider->placement.value().relativeTo == CalloutType) {
               points[selected] -= QPoint(divider->parentStep->loc[XX],
                                          divider->parentStep->loc[YY]);
           }
           pointerData[XX] = float(points[selected].x() - divider->parentStep->csiItem->loc[XX])/divider->parentStep->csiItem->size[XX];
           pointerData[YY] = float(points[selected].y() - divider->parentStep->csiItem->loc[YY])/divider->parentStep->csiItem->size[YY];
       } else {
           pointerData[XX] = float(points[selected].x());
           pointerData[YY] = float(points[selected].y());
       }
      switch (selected) {
       case Tip:
          pData.x1 = pointerData[XX];
          pData.y1 = pointerData[YY];
          break;
      case Base:
          pData.x2 = pointerData[XX];
          pData.y2 = pointerData[YY];
         break;
      case MidBase:
          pData.x3 = pointerData[XX];
          pData.y3 = pointerData[YY];
         break;
      case MidTip:
          pData.x4 = pointerData[XX];
          pData.y4 = pointerData[YY];
         break;
      default:
         break;
      }
  };

  PointerData pointerData = pointer.pointerMeta.value();
  for (int i = 0; i < NumPointerGrabbers; i++) {
      calculateOffset(SelectedPoint(i),pointerData);
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
