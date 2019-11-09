 
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
 * This file describes the data structure that represents an LPub pagepointer
 * (the steps of a submodel packed together and displayed next to the
 * assembly where the submodel is used).  
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef PAGEPOINTERH
#define PAGEPOINTERH

#include "ranges.h"
#include "range.h"
#include "placement.h"
#include "where.h"
#include "numberitem.h"

#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsPolygonItem>
#include "borderedlineitem.h"
#include <QGraphicsItemGroup>

class Step;
class AbstractRangeElement;
class Pointer;
class PagePointerItem;
class PagePointerBackgroundItem;
class QGraphicsView;
/*
 * There can be more than one pagepointer per step, so we add
 * the ability to be in a list here
 */

class PagePointer : public Steps {
  public:
    Step                      *parentStep;
    PlacementType              parentRelativeType;

    QList<Pointer *>           pointerList;             /* Pointers and pointer tips data */
    QList<PagePointerItem *>   graphicsPagePointerList; /* Pointer and pointer tips graphics */

    PagePointerBackgroundItem *background;
    Where  		               topPage,bottomPage;

    Where &topOfPagePointer()
    {
      return topPage;
    }
    Where &bottomOfPagePointer()
    {
      return bottomPage;
    }
    void setTopOfPagePointer(const Where &topOfPagePointer)
    {
      topPage = topOfPagePointer;
    }
    void setBottomOfPagePointer(const Where &bottomOfPagePointer)
    {
      bottomPage = bottomOfPagePointer;
    }

    PagePointer(
      Meta                 *_meta,
      QGraphicsView        *_view);

    virtual ~PagePointer();

    virtual void appendPointer(const Where &here, PointerMeta &pointerMeta, PointerAttribMeta &pointerAttrib);

    virtual void sizeIt();

    void addGraphicsItems(int offsetX, int offsetY, QGraphicsItem *parent, bool movable);

    virtual void addGraphicsPointerItem(Pointer *pointer);

    virtual void drawTips(QPoint &delta, QGraphicsItem *target, int type = 0);

    virtual void updatePointers(QPoint &delta, QGraphicsItem *target);
};


#endif
