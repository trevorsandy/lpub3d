
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
 * This class implements the graphical pointers that extend from pages to
 * assembly images as visual indicators to the builder as to where to
 * add the completed submodel into partially assembled final model.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "pagepointeritem.h"
#include "pagepointer.h"
#include "pagepointerbackgrounditem.h"
#include "step.h"
#include "range.h"

//---------------------------------------------------------------------------

/*
 * This is the constructor of a graphical pointer
 */

PagePointerItem::PagePointerItem(
  PagePointer   *pp,
  Pointer       *_pointer,
  QGraphicsItem *parent,
  QGraphicsView *_view)

  : PointerItem(parent)
{
  meta              = &pp->meta;
  view              = _view;
  pointer           = *_pointer;
  pagePointer       = pp;
  stepNumber        = pp->parentStep->stepNumber.number;
  pointerTop        = pp->topOfPagePointer();
  pointerBottom     = pp->bottomOfPagePointer();
  pointerParentType = PagePointerType;
  resizeRequested   = false;

  PointerData pointerData = pointer.pointerMeta.value();

  placement       = pointerData.placement;

  baseX           = pp->size[XX];
  baseY           = pp->size[YY];

  if (pointerData.segments == OneSegment) {
      int cX = pagePointer->parentStep->csiItem->loc[XX];
      int cY = pagePointer->parentStep->csiItem->loc[YY];
      int dX = pointerData.x1*pagePointer->parentStep->csiItem->size[XX];
      int dY = pointerData.y1*pagePointer->parentStep->csiItem->size[YY];

      if (pagePointer->placement.value().relativeTo == CalloutType) {
          cX += pagePointer->parentStep->loc[XX];
          cY += pagePointer->parentStep->loc[YY];
          points[Tip] = QPoint(cX + dX - pagePointer->loc[XX], cY + dY - pagePointer->loc[YY]);
      } else {
          points[Tip] = QPoint(cX + dX - pagePointer->loc[XX], cY + dY - pagePointer->loc[YY]);
      }

      if ( ! pagePointer->parentStep->onlyChild()) {
          switch (pagePointer->parentRelativeType) {
          case StepType:
              points[Tip] += QPoint(pagePointer->parentStep->range()->loc[XX],
                                    pagePointer->parentStep->range()->loc[YY]);
              points[Tip] += QPoint(pagePointer->parentStep->loc[XX],
                                    pagePointer->parentStep->loc[YY]);
              break;
          case StepGroupType:
                points[Tip] += QPoint(pagePointer->parentStep->grandparent()->loc[XX],
                                      pagePointer->parentStep->grandparent()->loc[YY]);
                points[Tip] += QPoint(pagePointer->parentStep->range()->loc[XX],
                                      pagePointer->parentStep->range()->loc[YY]);
                points[Tip] += QPoint(pagePointer->parentStep->loc[XX],
                                      pagePointer->parentStep->loc[YY]);
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

  PointerAttribData *pad  = &pointer.pointerAttrib.valuePixels();

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

  if (pad->lineData.hideArrows) {
      brushColor  = QColor(Qt::transparent);
      tipPenColor = brushColor;
  }

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
  setData(ObjectId, PagePointerObj);
  setZValue(PAGEPOINTER_ZVALUE_DEFAULT);

  drawPointerPoly();
}

PagePointerItem::~PagePointerItem(){
    shaftSegments.clear();
}

/*
 * Given the location of the Tip (as dragged around by the user)
 * calculate a reasonable placement and Loc for Base or MidTip points.
 */

bool PagePointerItem::autoLocFromTip()
{
    int width  = pagePointer->size[XX];
    int height = pagePointer->size[YY];
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

        if (segments() == ThreeSegments) {
            int mtx = pointerData.x3;
            points[MidTip].setY(ty);
            points[MidTip].setX(mtx);
        }
    } else {
        /* Figure out which corner */
        BorderData borderData = pagePointer->background->border->valuePixels();
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

bool PagePointerItem::autoLocFromMidBase()
{
        int width = pagePointer->size[XX];
        int height = pagePointer->size[YY];
        int left = 0;
        int right = width;
        int top = 0;
        int bottom = height;

        QPoint intersect;
        int tx,ty;

        tx = points[MidBase].x();
        ty = points[MidBase].y();

        /* Figure out which corner */
        BorderData borderData = pagePointer->background->border->valuePixels();
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

void PagePointerItem::defaultPointer()
{

  points[Tip] = QPointF(pagePointer->parentStep->csiItem->loc[XX]+
                        pagePointer->parentStep->csiItem->size[XX]/2,
                        pagePointer->parentStep->csiItem->loc[YY]+
                        pagePointer->parentStep->csiItem->size[YY]/2);

  if ( ! pagePointer->parentStep->onlyChild()) {
    PlacementData pagePointerPlacement = pagePointer->placement.value();
    switch (pagePointerPlacement.relativeTo) {
      case PageType:
      case StepGroupType:
        points[Tip] += QPoint(pagePointer->parentStep->grandparent()->loc[XX],
                              pagePointer->parentStep->grandparent()->loc[YY]);
        points[Tip] += QPoint(pagePointer->parentStep->range()->loc[XX],
                              pagePointer->parentStep->range()->loc[YY]);
        points[Tip] += QPoint(pagePointer->parentStep->loc[XX],
                              pagePointer->parentStep->loc[YY]);
        points[Tip] -= QPoint(pagePointer->loc[XX],pagePointer->loc[YY]);
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

void PagePointerItem::calculatePointerMetaLoc()
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
         loc = points[Base].x()/pagePointer->size[XX];
      else
         loc = points[Base].x();
    }
    break;
    case Left:
    case Right:
    {
      if (segments() == OneSegment)
         loc = points[Base].y()/pagePointer->size[YY];
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

void PagePointerItem::calculatePointerMeta()
{
  calculatePointerMetaLoc();

  PointerData pointerData = pointer.pointerMeta.value();

  if (segments() == OneSegment) {
      if (pagePointer->parentStep->onlyChild()) {
          points[Tip] += QPoint(pagePointer->loc[XX],pagePointer->loc[YY]);
      } else {
          switch (pagePointer->parentRelativeType) {
          case StepType:
              points[Tip] -= QPoint(pagePointer->parentStep->loc[XX],
                                    pagePointer->parentStep->loc[YY]);
              points[Tip] += QPoint(pagePointer->loc[XX],pagePointer->loc[YY]);
               break;
          case StepGroupType:
            {
              points[Tip] -= QPoint(pagePointer->parentStep->grandparent()->loc[XX],
                                    pagePointer->parentStep->grandparent()->loc[YY]);
              points[Tip] -= QPoint(pagePointer->parentStep->loc[XX],
                                    pagePointer->parentStep->loc[YY]);
              points[Tip] += QPoint(pagePointer->loc[XX],pagePointer->loc[YY]);
            }
              break;
          default:
              break;
          }
      }

//      if (pagePointer->placement.value().relativeTo == PageType ||
//          pagePointer->placement.value().relativeTo == StepGroupType ) {
//          points[Tip] -= QPoint(pagePointer->parentStep->loc[XX],
//                                pagePointer->parentStep->loc[YY]);
//      }

      pointerData.x1 = float((points[Tip].x() - pagePointer->parentStep->csiItem->loc[XX])/pagePointer->parentStep->csiItem->size[XX]);
      pointerData.y1 = float((points[Tip].y() - pagePointer->parentStep->csiItem->loc[YY])/pagePointer->parentStep->csiItem->size[YY]);
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

//  qDebug()<< "\nPAGE POINTER DATA (Formatted)"
//          << " \nPlacement:             "   << PlacNames[placement]     << " (" << placement << ")"
//          << " \nLoc(fraction of side): "   << pointerData.loc
//          << " \nx1 (Tip.x):            "   << pointerData.x1
//          << " \ny1 (Tip.y):            "   << pointerData.y1
//          << " \nx2 (Base.x):           "   << pointerData.x2
//          << " \ny2 (Base.y):           "   << pointerData.y2
//          << " \nx3 (MidBase.x):        "   << pointerData.x3
//          << " \ny3 (MidBase.y):        "   << pointerData.y3
//          << " \nx4 (MidTip.x):         "   << pointerData.x4
//          << " \ny4 (MidTip.y):         "   << pointerData.y4
//          << " \nBase:                  "   << 0
//          << " \nSegments:              "   << segments()
//          << " \nPagePointer Rect:      "   << RectNames[pagePointer->placement.value().rectPlacement]  << " (" << pagePointer->placement.value().rectPlacement << ")";
//              ;
  pointer.pointerMeta.setValuePage(
    pagePointer->placement.value().rectPlacement,
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
