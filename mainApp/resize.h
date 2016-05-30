 
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
 * This class implements a fundamental class for placing things relative to
 * other things.  This concept is the cornerstone of LPub's meta commands
 * for describing what building instructions should look like without having
 * to specify inches, centimeters or pixels.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef resizeH
#define resizeH

#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QSize>
#include <QRect>
#include <QPen>
#include <QBrush>

#include "placement.h"
#include "metatypes.h"
#include "resolution.h"
#include "meta.h"

#include "QsLog.h"

class Grabber;
class QGraphicsItem;
class Page;

class AbstractResize
{
  public:
    static qreal grabsize;
    static qreal grabSize()
    {
      return grabsize*resolution();
    }
    int          selectedGrabber;
    bool         sizeChanged;
    AbstractResize()
    {
      sizeChanged = false;
    }

    virtual ~AbstractResize()
    {
    }
    virtual void  placeGrabbers() = 0;
    virtual void  whichGrabber(int which)
    {
      selectedGrabber = which;
    }
    virtual void  resize(QPointF) = 0;
    virtual void  change() = 0;
    virtual QGraphicsItem *myParentItem() = 0;
};

class Grabber : public QGraphicsRectItem
{
  public:
    int which;
    AbstractResize *resized;
    Grabber() {}
    
    Grabber(int which,
            AbstractResize *resized,
            QGraphicsItem *parentGraphicsItem)
          : which(which),
            resized(resized)
    {
      setParentItem(parentGraphicsItem);
      setFlag(QGraphicsItem::ItemIsSelectable,true);
      setFlag(QGraphicsItem::ItemIsMovable,true);
      setZValue(100);
      QPen pen(Qt::black);
      setPen(pen);
      setBrush(Qt::black);
      setRect(0,0,resized->grabSize(),resized->grabSize());
    }
  protected:
    void mousePressEvent(QGraphicsSceneMouseEvent * /* event */)
    {
      resized->whichGrabber(which);
    }
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
      resized->resize(event->scenePos());
    }
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * /* event */)
    {
      resized->change();
    }
};

class ResizeRect : public AbstractResize, public Placement
{
  public:
    static const int numGrabbers = 4;
    Grabber *grabbers[numGrabbers];
    QPointF  points[numGrabbers];
    ResizeRect()
    {
      for (int i = 0; i < numGrabbers; i++) {
        grabbers[i] = NULL;  
      }
    }
    virtual ~ResizeRect()
    {
    }
    virtual void placeGrabbers();
    virtual void resize(QPointF) = 0;
    virtual void change() = 0;
    virtual QRectF currentRect() = 0;
    virtual QPointF currentPos() = 0;
    virtual void    setNewPos(qreal,qreal) = 0;

};

class ResizeConstAspectRect : public ResizeRect
{
  public:
    qreal oldScale;
    ResizeConstAspectRect()
    {
      oldScale = 1.0;
    }
    virtual ~ResizeConstAspectRect() {}
    virtual void resize(QPointF);
    virtual void change() = 0;
    virtual QRectF currentRect() = 0;
    virtual QPointF currentPos() = 0;
    virtual void    setNewPos(qreal,qreal) = 0;
    virtual void    setScale(qreal,qreal) = 0;
};

class ResizePixmapItem: public QGraphicsPixmapItem, 
                        public MetaItem, 
                        public ResizeConstAspectRect
{
  public:
    QPointF  position;
    bool     positionChanged;

    ResizePixmapItem();

    virtual void change() {}
    QRectF currentRect();
    QPointF currentPos();
    virtual void setNewPos(qreal,qreal);
    virtual void setScale(qreal,qreal);
    virtual QGraphicsItem *myParentItem()
    {
      return parentItem();
    }
  protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
};

class InsertPixmapItem : public ResizePixmapItem
{
  public:

    InsertMeta insertMeta;
    qreal      relativeToLoc[2];
    qreal      relativeToSize[2];

    InsertPixmapItem(
      QPixmap       &pixmap,
      InsertMeta    &insertMeta,
      QGraphicsItem *parent = 0);

    virtual void change();
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

#endif
