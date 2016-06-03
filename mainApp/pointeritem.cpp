
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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
 * add the completed submodel into partially assembeled final model.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/


#include "lpub.h"
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsLineItem>
#include <QPolygonF>
#include <QGraphicsSceneContextMenuEvent>
#include <math.h>
#include "pointeritem.h"
#include "metaitem.h"
#include "callout.h"
#include "calloutbackgrounditem.h"
#include "step.h"
#include "range.h"

//---------------------------------------------------------------------------

/* calculate the parameters for the equation of line from two points */

bool rectLineIntersect(
  QPoint  tip,
  QPoint  loc,
  QRect   rect, 
  int     base,
  QPoint &intersect,

  PlacementEnc &placement);

/*
 * This is the constructor of a graphical pointer
 */

CalloutPointerItem::CalloutPointerItem(
  Callout       *co,
  Meta          *meta,
  Pointer       *_pointer,
  QGraphicsItem *parent,
  QGraphicsView *_view)

  : QGraphicsItemGroup(parent)
{
  view          = _view;
  callout       = co;
  pointer       = *_pointer;
  QString color;

  PointerData pointerData = pointer.pointerMeta.value();
  BorderData  border = meta->LPub.callout.border.valuePixels();
  PlacementData calloutPlacement = meta->LPub.callout.placement.value();

  borderColor     = border.color;
  borderThickness = border.thickness;

  placement = pointerData.placement;

  int cX = callout->parentStep->csiItem->loc[XX];
  int cY = callout->parentStep->csiItem->loc[YY];
  int dX = pointerData.x*callout->parentStep->csiItem->size[XX];
  int dY = pointerData.y*callout->parentStep->csiItem->size[YY];

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
  
  autoLocFromTip();

  QLineF linef;

  //QColor qColor = LDrawColor::color(border.color);
  QColor qColor(border.color); 

  QPen pen(qColor);
  pen.setWidth(borderThickness);
  pen.setCapStyle(Qt::RoundCap);

  shaft = new QGraphicsLineItem(linef,this);
  shaft->setPen(pen);
  shaft->setZValue(-5);
  shaft->setFlag(QGraphicsItem::ItemIsSelectable,false);
  shaft->setToolTip("Arrow shaft");
  addToGroup(shaft);
  
  QPolygonF poly;
  
  head = new QGraphicsPolygonItem(poly, this);
  head->setPen(qColor);
  head->setBrush(qColor);
  head->setFlag(QGraphicsItem::ItemIsSelectable,false);
  head->setToolTip("Arrow head");
  addToGroup(head);  
  
  for (int i = 0; i < NumGrabbers; i++) {
    grabbers[i] = NULL;
  }

  drawPointerPoly();
  setFlag(QGraphicsItem::ItemIsFocusable,true);
}

void CalloutPointerItem::drawPointerPoly()
{
  QLineF linef = QLineF(points[Base],points[Tip]);

  removeFromGroup(shaft);
  shaft->setLine(linef);
  addToGroup(shaft);
  
  QPolygonF poly;
  
  poly << QPointF(-2*grabSize(), 0);
  poly << QPointF(-2*grabSize(),grabSize()/2);
  poly << QPointF(grabSize()/2,0);
  poly << QPointF(-2*grabSize(),-grabSize()/2);
  poly << QPointF(-2*grabSize(),0);
  
  removeFromGroup(head);
  head->setPolygon(poly);
  
  qreal x = points[Tip].x()-points[Base].x();
  qreal y = points[Tip].y()-points[Base].y();
  qreal h = sqrt(x*x+y*y);
  qreal angle = 180*acos(x/h);

  qreal pi = 22.0/7;

  if (x == 0) {
    if (y < 0) {
      angle = 270.0;
    } else {
      angle = 90.0;
    }
  } else if (y == 0) {
    if (x < 0) {
      angle = 180.0;
    } else {
      angle = 0.0;
    }
  } else {
    volatile qreal h = sqrt(x*x+y*y);
    if (x > 0) {
      if (y > 0) {
        angle = 180-180*acos(-x/h)/pi;
      } else {
        angle = 180+180*acos(-x/h)/pi;
      }
    } else {
      if (y > 0) {
        angle = 180*acos(x/h)/pi;
      } else {
        angle = -180*acos(x/h)/pi;
      }
    }
  }
  
  head->resetTransform();
  head->setRotation(rotation() + angle);
  head->setPos(points[Tip]);
  addToGroup(head);

  view->updateSceneRect(sceneBoundingRect());
}

/*
 * Given the location of the Tip (as dragged around by the user)
 * calculate a reasonable placement and Loc for points[Base]
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

  return true;
}

/*
 * Given the location of Loc (as dragged around by the user)
 * calculate a reasonable placement and Loc for points[Base]
 */

bool CalloutPointerItem::autoLocFromLoc(
  QPoint loc)
{
  QRect rect(0,
             0,
             callout->size[XX],
             callout->size[YY]);

  QPoint intersect;
  PlacementEnc tplacement;

  QPoint tip(points[Tip].x() + 0.5, points[Tip].y() + 0.5);

  if (rectLineIntersect(tip,
                        loc,
                        rect,
                        borderThickness,
                        intersect,
                        tplacement)) {
    placement = tplacement;
    points[Base] = intersect;
    return true;
  }    
  return false;
}

void CalloutPointerItem::placeGrabbers()
{
  if (grabbers[0] == NULL) {
    for (int i = 0; i < NumGrabbers; i++) {
      grabbers[i] = new Grabber(i,this,myParentItem());
    }
  }
  for (int i = 0; i < NumGrabbers; i++) {
    grabbers[i]->setPos(points[i].x() - grabSize()/2, points[i].y()-grabSize()/2);
  }
}
      /*
       *     c1 c2  c1 c2
       *    +--+------+--+
       *  c1|            | c1
       *    +            +
       *  c2|            | c2
       *    |            |
       *    |            |
       *  c1|            | c1
       *    +            +
       *  c2|            | c2
       *    +--+------+--+
       *     c1 c2  c1 c2
       *
       */

      /*     c1        c1
       *    +------------+
       *  c2|            | c2
       *    |            |
       *    |            |   
       *    |            |
       *    |            |
       *    |            |   
       *    |            |
       *  c2|            | c2
       *    +------------+
       *     c1        c1
       */
void CalloutPointerItem::mousePressEvent  (QGraphicsSceneMouseEvent * /* unused */)
{
  placeGrabbers();
}

/*
 * Mouse move.  What we do depends on what was selected by the mouse click.
 *
 * If pointer's tip is selected then allow the tip to move anywhere within the
 * CSI.
 *
 * If the pointer's location (where the pointer is attached to the callout)
 * is selected then allow the pointer to move to any valid edges or corner
 * (constrained by base).
 *
 * If the two outside corners are selected, then the user is trying to adjust the
 * base.
 */

void CalloutPointerItem::resize(QPointF grabbed)
{
  grabbed -= scenePos();
  
  QPoint intersect;

  bool changed = false;

  QRect rect(0,0,callout->size[XX],callout->size[YY]);

  switch (selectedGrabber) {
    /*
     * Loc must track along an edge.  If you draw a line from tip to the mouse
     * cursor, the line intersects with the edge of the callout one or two places.
     * The single intersection is at corners, and the dual intersection is not
     * on corners.  In the case of dual intersection, the one that is closest
     * to tip is the one used.
     * 
     * If the tip to mouse line does not intersect with the callout, do nothing.
     * If the top to mouse line only intersects with corner, then force corner
     * placement.
     *
     * If the the tip to mouse line intersects twice, then the closest intersect is
     * used.  if the distance between the intersect and the corner is less than
     * base, react as thought we were at corner.
     */

    case Base:
      {
        QPoint loc(grabbed.x(),grabbed.y());
      
        if (autoLocFromLoc(loc)) {
          changed = true;
        }
      }
    break;
    case Tip:
      points[Tip] = grabbed;
      if (autoLocFromTip()) {
        changed = true;
      }
    break;
    default:
    break;
  }
  if (changed) {
    drawPointerPoly();
    positionChanged = true;
  }
  grabbers[Base]->setPos(points[Base].x() - grabSize()/2, points[Base].y() - grabSize()/2);
  grabbers[Tip ]->setPos(points[Tip ].x() - grabSize()/2, points[Tip ].y() - grabSize()/2);
}

void CalloutPointerItem::change()
{
  calculatePointerMeta();

  // back annotate pointer shape back into the LDraw file.

  MetaItem::updatePointer(pointer.here, &pointer.pointerMeta);
}

/* Meta related stuff */

void CalloutPointerItem::drawTip(QPoint delta)
{
  points[Tip] += delta;
  autoLocFromTip();
  drawPointerPoly();
  points[Tip] -= delta;
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
      loc = points[Base].x()/callout->size[XX];
    break;
    case Left:
    case Right:
      loc = points[Base].y()/callout->size[YY];
    break;
    default:
    break;
  }

  PointerData pointerData = pointer.pointerMeta.value();
  pointer.pointerMeta.setValue(
    placement,
    loc,
    0,
    pointerData.x,
    pointerData.y);
}

void CalloutPointerItem::calculatePointerMeta()
{
  calculatePointerMetaLoc();

  PointerData pointerData = pointer.pointerMeta.value();

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

  pointerData.x = (points[Tip].x() - callout->parentStep->csiItem->loc[XX])/callout->parentStep->csiItem->size[XX];
  pointerData.y = (points[Tip].y() - callout->parentStep->csiItem->loc[YY])/callout->parentStep->csiItem->size[YY];

  pointer.pointerMeta.setValue(
    placement,
    pointerData.loc,
    0,
    pointerData.x,
    pointerData.y);
}

void CalloutPointerItem::updatePointer(
  QPoint &delta)
{
  points[Tip] += delta;
  autoLocFromTip();
  drawPointerPoly();
  calculatePointerMetaLoc();

  MetaItem::updatePointer(pointer.here, &pointer.pointerMeta);
}

void CalloutPointerItem::addPointerMeta()
{
  QString metaString = pointer.pointerMeta.format(false,false);
  Where here = pointer.here+1;
  insertMeta(here,metaString);
  gui->displayPage();
}

void CalloutPointerItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  QAction *removeAction = menu.addAction("Delete this Arrow");
  removeAction->setIcon(QIcon(":/resources/deletearrow.png"));
  removeAction->setWhatsThis(            "Delete this Arrow:\n"
                                         "Deletes this arrow from the callout");

  QAction *selectedAction   = menu.exec(event->screenPos());
  if (selectedAction == removeAction) {
    deletePointer(pointer.here);
  }
}


/* calculate the parameters for the equation of line from two points */

bool lineEquation(
  QPoint point[2],
  qreal &a,
  qreal &b,
  qreal &c)
{
  qreal xlk = point[0].x() - point[1].x();
  qreal ylk = point[0].y() - point[1].y();
  qreal rsq = xlk*xlk+ylk*ylk;
  if (rsq < 1) {
    return false;
  }
  qreal rinv = 1.0/sqrt(rsq);
  a = -ylk*rinv;
  b =  xlk*rinv;
  c = (point[1].x()*point[0].y() - point[0].x()*point[1].y())*rinv;
  return true;
}

/* calculate the intersection of two lines */

bool lineIntersect(
  qreal a1, qreal b1, qreal c1,
  qreal a2, qreal b2, qreal c2,
  QPoint &intersect)
{
  qreal det = a1*b2 - a2*b1;
  if (fabs(det) < 1e-6) {
    return false;
  } else {
    qreal dinv = 1.0/det;
    intersect.setX((b1*c2 - b2*c1)*dinv);
    intersect.setY((a2*c1 - a1*c2)*dinv);
    return true;
  }
}

/* determine if a line intersects a Horizontal line segment, if so return the intersection */

bool lineIntersectHorizSeg(
  qreal a1, qreal b1, qreal c1,
  QPoint seg[2], QPoint &intersect)
{
  qreal a2, b2, c2;
  if ( ! lineEquation(seg,a2,b2,c2)) {
    return false;
  }

  if ( ! lineIntersect(a1,b1,c1,a2,b2,c2,intersect)) {
    return false;
  }

  return intersect.x() >= seg[0].x() && intersect.x() <= seg[1].x();
}

/* determine if a line intersects a Vertical line segment, if so return the intersection */

bool lineIntersectVertSeg(
  qreal a1, qreal b1, qreal c1,
  QPoint seg[2], QPoint &intersect)
{
  qreal a2, b2, c2;
  if ( ! lineEquation(seg,a2,b2,c2)) {
    return false;
  }

  if ( ! lineIntersect(a1,b1,c1,a2,b2,c2,intersect)) {
    return false;
  }

  return intersect.y() >= seg[0].y() && intersect.y() <= seg[1].y();
}
/*
 * Given Tip and Loc that form a line and an rectangle in 2D space, calculate the
 * intersection of the line and rect that is closes to tip.
 *
 * iv_top && iv_left
 * iv_top && iv_bottom
 * iv_top && iv_right
 * iv_left && iv_bottom
 * iv_left && iv_right
 * iv_bottom && iv_right
 *
 * The line will always intersect two sides, unless it just hits the corners.
 *
 */

bool rectLineIntersect(
  QPoint        tip,
  QPoint        loc,
  QRect         rect, 
  int           base,
  QPoint       &intersect,
  PlacementEnc &placement)
{
  int width = rect.width();
  int height = rect.height();
  int tipX = tip.x();
  int tipY = tip.y();
  int tx,ty;

  QPoint seg[2];
  QPoint intersect_top,intersect_bottom,intersect_left,intersect_right;
  bool   iv_top, iv_bottom,iv_left,iv_right;

  /* calculate formula for line between tip and {C1,C2,Loc) */

  seg[0] = tip;
  seg[1] = loc;
  qreal a, b, c;
  if ( ! lineEquation(seg,a,b,c)) {
    return false;
  }

  /* top Horizontal line */

  seg[0].setX(0);
  seg[1].setX(rect.width());
  seg[0].setY(rect.top());
  seg[1].setY(rect.top());
  iv_top = lineIntersectHorizSeg(a,b,c,seg,intersect_top);

  tx = tipX - intersect_top.x();
  ty = tipY - intersect_top.y();
  int top_dist = tx*tx+ty*ty;

  /* bottom Horizontal line */

  seg[0].setY(rect.height());
  seg[1].setY(rect.height());

  iv_bottom = lineIntersectHorizSeg(a,b,c,seg,intersect_bottom);

  tx = tipX - intersect_bottom.x();
  ty = tipY - intersect_bottom.y();
  int bot_dist = tx*tx+ty*ty;

  /* left Vertical line */

  seg[0].setX(0);
  seg[1].setX(0);
  seg[0].setY(rect.top());
  seg[1].setY(rect.height());

  iv_left = lineIntersectVertSeg(a,b,c,seg,intersect_left);

  tx = tipX - intersect_left.x();
  ty = tipY - intersect_left.y();
  int left_dist = tx*tx+ty*ty;

  /* right Vertical line */

  seg[0].setX(rect.width());
  seg[1].setX(rect.width());

  iv_right = lineIntersectVertSeg(a,b,c,seg,intersect_right);

  tx = tipX - intersect_right.x();
  ty = tipY - intersect_right.y();
  int right_dist = tx*tx+ty*ty;

  if (iv_top && iv_bottom) {

    /* Is mouse tip closer to top or bottom? */

    if (top_dist < bot_dist) {

      /* tip closer to top */

      if (intersect_top.x() < base) {
        placement = TopLeft;
        intersect.setX(0);
      } else if (width - intersect_top.x() < base) {
        placement = TopRight;
        intersect.setX(width);
      } else {
        placement = Top;
        int x = intersect_top.x();
        x = x < base
              ? base
              : x > width - base
                  ? width - base
                  : x;
        intersect.setX(x);
      }
      intersect.setY(0);
    } else {
      if (intersect_bottom.x() < base) {
        placement = BottomLeft;
        intersect.setX(0);
      } else if (width - intersect_bottom.x() < base) {
        placement = BottomRight;
        intersect.setX(width);
      } else {
        placement = Bottom;
        int x = intersect_top.x();
        x = x < base
              ? base
              : x > width - base
                  ? width - base
                  : x;
        intersect.setY(intersect_bottom.y());
        intersect.setX(x);
      }
      intersect.setY(height);
    }
  } else if (iv_left && iv_right) {

    /* Is the tip closer to right or left? */

    if (left_dist < right_dist) {

      /* closer to left */

      if (intersect_left.y() < base) {
        placement = TopLeft;
        intersect.setY(0);
      } else if (height - intersect_left.y() < base) {
        placement = BottomLeft;
        intersect.setY(height);
      } else {
        placement = Left;
        int y = intersect_left.y();
        y = y < base
              ? base
              : y > height - base
                ? height - base
                : y;
        intersect.setY(intersect_left.y());
      }
      intersect.setX(0);
    } else {
      if (intersect_right.y() < base && tipY < 0) {
        placement = TopRight;
        intersect.setY(0);
      } else if (height - intersect_right.y() < base && tipY > height) {
        placement = BottomRight;
        intersect.setY(height);
      } else {
        placement = Right;
        int y = intersect_right.y();
        y = y < base
                ? base
                : y > height - base
                  ? height - base
                  : y;
        intersect.setY(y);
      }
      intersect.setX(width);
    }
  } else if (iv_top && iv_left) {

    if (top_dist <= left_dist) {

      /* tip above the rectangle - line going down/left */

      if (width - intersect_top.x() < base) {
        placement = TopRight;
        intersect.setX(width);
      } else {
        placement = Top; 
        intersect.setX(intersect_top.x());
        if (intersect.x() < base) {
          intersect.setX(base);
        }
      }
      intersect.setY(0);
    } else {

      /* tip to left of rectangle - line going up/right*/
     
      if (height - intersect_left.y() < base) {
        placement = BottomLeft;
        intersect.setY(height);
      } else {
        placement = Left;
        intersect.setY(intersect_left.y());
        if (intersect.y() < base) {
          intersect.setY(base);
        }
      }
      intersect.setX(0);
    }
  } else if (iv_top && iv_right) {

    if (top_dist <= right_dist) {

      /* tip above the rectangle - line going down/right */

      if (intersect_top.x() < base) {
        placement = TopLeft;
        intersect.setX(0);
      } else {
        placement = Top; 
        intersect.setX(intersect_top.x());
        if (width - intersect.x() < base) {
          intersect.setX(width - base);
        }
      }
      intersect.setY(0);
    } else {

      /* tip to right of rectangle - line going up/left*/
     
      if (height - intersect_left.y() < base) {
        placement = BottomRight;
        intersect.setY(height);
      } else {
        placement = Right;
        intersect.setY(intersect_right.y());
        if (intersect.y() < base) {
          intersect.setY(base);
        }
      }
      intersect.setX(width);
    }
  } else if (iv_bottom && iv_right) {

    if (bot_dist <= right_dist) {

      /* tip below the rectangle - line going up/right */

      if (intersect_bottom.x() < base) {
        placement = BottomLeft;
        intersect.setX(0);
      } else {
        placement = Bottom; 
        intersect.setX(intersect_bottom.x());
        if (width - intersect.x() < base) {
          intersect.setX(width - base);
        }
      }
      intersect.setY(height);
    } else {

      /* tip to right of rectangle - line going down/left */
     
      if (intersect_right.y() < base) {
        placement = TopRight;
        intersect.setY(0);
      } else {
        placement = Right;
        intersect.setY(intersect_right.y());
        if (height - intersect.y() < base) {
          intersect.setY(height - base);
        }
      }
      intersect.setX(width);
    }
  } else if (iv_bottom && iv_left) {

    if (bot_dist <= left_dist) {

      /* tip below the rectangle - line going up/left */

      if (width - intersect_bottom.x() < base) {
        placement = BottomRight;
        intersect.setX(width);
      } else {
        placement = Bottom; 
        intersect.setX(intersect_bottom.x());
        if (intersect.x() < base) {
          intersect.setX(base);
        }
      }
      intersect.setY(height);
    } else {

      /* tip left of the rectangle */
     
      if (intersect_left.y() < base) {
        placement = TopLeft;
        intersect.setY(0);
      } else {
        placement = Left;
        intersect.setY(intersect_left.y());
        if (height - intersect.y() < base) {
          intersect.setY(height - base);
        }
      }
      intersect.setX(0);
    }
  } else {
    /* Bah! The user isn't anywhere near the rectangle */
    return false;
  }
  return true;
}
