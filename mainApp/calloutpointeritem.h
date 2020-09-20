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
 * This class implements the graphical pointers that extend from callouts to
 * assembly images as visual indicators to the builder as to where to 
 * add the completed submodel into partially assembled final model.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef CALLOUTPOINTERITEMH
#define CALLOUTPOINTERITEMH

#include "pointeritem.h"
#include "metaitem.h"

class QGraphicsPolygonItem;
class BorderedLineItem;
class QGraphicsItemGroup;
class Callout;

class CalloutPointerItem : public PointerItem
{
public:
  CalloutPointerItem(
    Callout             *co,
    Pointer             *pointer,
    QGraphicsItem       *parent,
    QGraphicsView       *view);

private:
  Callout              *callout;

  /*
   *   +--------------------------------------------++
   *   |                                             |
   *   | . +-------------------------------------+   |
   *   |   |                                     | . |
   *   |   |                                     |   |
   *
   *
   *  callout size defines the outside edge of the callout.
   *  When there is a border, the inside rectangle starts
   *  at +thickness,+thickness, and ends at size-thickness,
   *  size-tickness.
   *
   *  Using round end cap caps the ends of the lines that
   *  intersect the callout are at +- tickness/2.  I'm not
   *  sure the affect of thickness is even vs. odd.
   *
   *  Loc should be calculated on the inside rectangle?
   *  The triangles have to go to the edge of the inner
   *  rectangle to obscure the border.
   *
   */

public:

  /* When the user "Add Pointer", we need to give a default/
     reasonable pointer */

  virtual void defaultPointer();

private:
  /* Drag the tip of the pointer, and calculate a good
   * location for the pointer to connect to the callout. */

  virtual bool autoLocFromTip();

  /* Drag the MidBase point of the pointer, and calculate a good
   * location for the pointer to connect to the callout. */

  virtual bool autoLocFromMidBase();

  /* When we drag the CSI or the pointer's callout, we
   * need recalculate the Location portion of the pointer
   * meta, but the offset remains unchanged.
   * When we have more than one segment we calculate
   * from the Tip to the segment point and from the
   * Tip to the base when we have one segment (default) */

  virtual void calculatePointerMetaLoc();

  virtual void calculatePointerMeta();

};

#endif
