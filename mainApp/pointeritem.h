/****************************************************************************
**
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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
 * This class implements the graphical pointers that extend from base objects
 * (e.g. a Callout) to assembly images as visual indicators to the builder
 * as to where to add the completed submodel into partially assembled final model.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef POINTERITEMH
#define POINTERITEMH

#include "pointer.h"
#include "borderedlineitem.h"
#include "metaitem.h"
#include "resize.h"

class QGraphicsPolygonItem;
class QGraphicsItemGroup;
class QGraphicsView;
class PointerHeadItem;

class PointerItem : public QGraphicsItemGroup, public MetaItem, public AbstractResize 
{
public:
  PointerItem(QGraphicsItem *parent = nullptr);
  virtual ~PointerItem();

  enum SelectedPoint { Tip, Base, MidBase, MidTip, NumPointerGrabbers };
  enum ShaftSegments { OneSegment = 1, TwoSegments, ThreeSegments};

  QGraphicsView             *view;

  Meta                      *meta;

  PlacementType              pointerParentType;

  int                        stepNumber;
  Where                      pointerTop, pointerBottom;

  Pointer                    pointer;
  PlacementEnc               placement;
  bool                       positionChanged;
  bool                       resizeRequested;
  float                      thickness;       // only used by autoLocFromBase()
  int                        baseX;
  int                        baseY;
  BorderedLineItem          *shaft;
  PointerHeadItem           *head;
  QList<BorderedLineItem *>  shaftSegments;

  Grabber                   *grabbers[NumPointerGrabbers];
  QPointF                    points[NumPointerGrabbers];  // the max points on the inside polygon
  
  virtual void placeGrabbers();
  virtual void resize(QPointF);
  virtual void change();
  virtual QGraphicsItem *myParentItem()
  {
    return parentItem();
  }

  /*
   *   +--------------------------------------------++
   *   |                                             |
   *   | . +-------------------------------------+   |
   *   |   |                                     | . |
   *   |   |                                     |   |
   *
   *
   *  base object size defines the outside edge of the base object.
   *  When there is a border, the inside rectangle starts
   *  at +thickness,+thickness, and ends at size-thickness,
   *  size-tickness.
   *
   *  Using round end cap caps the ends of the lines that
   *  intersect the base object are at +- tickness/2.  I'm not
   *  sure the affect of thickness is even vs. odd.
   *
   *  Loc should be calculated on the inside rectangle?
   *  The triangles have to go to the edge of the inner
   *  rectangle to obscure the border.
   *
   */
  
  /* When the user "Add Pointer", we need to give a default/
     reasonable pointer */

  virtual void defaultPointer() = 0;
  
  /* When we drag the pointer tip, we move the tip relative
   * to the CSI rect, so we must recalculate the offset
   * into the CSI */

  void updatePointer(QPoint &delta);

  void drawTip(QPoint delta, int type = 0);
  
  /* Drag the tip of the pointer, and calculate a good
   * location for the pointer to connect to the base object. */

    virtual bool autoLocFromTip() = 0;

  /* Drag the MidBase point of the pointer, and calculate a good
   * location for the pointer to connect to the base object. */

    virtual bool autoLocFromMidBase() = 0;

  /* When we drag the CSI or the pointer's base object, we
   * need recalculate the Location portion of the pointer
   * meta, but the offset remains unchanged.
   * When we have more than one segment we calculate
   * from the Tip to the segment point and from the
   * Tip to the base when we have one segment (default) */

   virtual void calculatePointerMetaLoc() = 0;

   virtual void calculatePointerMeta() = 0;

  /* When using menu to add a new pointer, we need to add
   * a new line to the LDraw file. */

   virtual void addPointerMeta();

  /* Drag the MidTip point of the pointer, and calculate a good
   * location for the MidBase segment to follow. */

    bool autoMidBaseFromMidTip();
	
  /* When we drag the base object or CSI, we need to recalculate
   * the pointer.
   * When we have more than one segment we calculate
   * from the CSI to the segment point and from the
   * CSI to the base when we have one segment (default) */

  void drawPointerPoly();
  bool autoLocFromBase(QPoint loc);

  /* Add shaft segment and control point grabber */
  void addShaftSegment();

  /* Remove shaft segment and control point grabber */
  void removeShaftSegment();

  int segments(){return shaftSegments.size();}

  static bool rectLineIntersect(QPoint        tip,
                                QPoint        loc,
                                QRect         rect,
                                int           base,
                                QPoint       &intersect,
                                PlacementEnc &placement);

protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
};

class PointerHeadItem : public QGraphicsPolygonItem
{
public:
  int stepNumber;
  Where top,bottom;
  PointerHeadItem(const QPolygonF &poly,
                  QGraphicsItem *parent = nullptr)
      :QGraphicsPolygonItem(poly, parent){}
};

#endif
