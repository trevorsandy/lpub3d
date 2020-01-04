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

#ifndef DIVIDERPOINTERITEM_H
#define DIVIDERPOINTERITEM_H

#include "pointeritem.h"
#include "metaitem.h"

class QGraphicsPolygonItem;
class QGraphicsLineItem;
class QGraphicsItemGroup;
class DividerItem;

class DividerPointerItem  : public PointerItem
{
public:
  DividerPointerItem(DividerItem   *divider,
                     Pointer       *pointer,
                     QGraphicsItem *parent,
                     QGraphicsView *view);

  virtual ~DividerPointerItem();

private:
    DividerItem     *divider;

  /*
   *   +--------------------------------------------++
   *   |                                             |
   *   | . +-------------------------------------+   |
   *   |   |                                     | . |
   *   |   |                                     |   |
   *
   *  divider size defines the outside edge of the divider.
   *  When there is a border, the inside rectangle starts
   *  at +thickness,+thickness, and ends at size-thickness,
   *  size-thickness.
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

#endif // DIVIDERPOINTERITEM_H
