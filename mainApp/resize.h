 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
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
#include <QGraphicsTextItem>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QSize>
#include <QRect>
#include <QPen>
#include <QBrush>
#include <QsLog.h>

#include "placement.h"
#include "metatypes.h"
#include "resolution.h"
#include "meta.h"
#include "name.h"

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
    bool         grabbersVisible;
    AbstractResize()
    {
      sizeChanged = false;
      grabbersVisible = false;
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
      setZValue(GRABBER_ZVALUE_DEFAULT);

      QPen pen(Qt::black);
      setPen(pen);
      setBrush(Qt::black);
      setRect(0,0,resized->grabSize(),resized->grabSize());
      setAcceptHoverEvents(true);
    }
  protected:
    void mousePressEvent(QGraphicsSceneMouseEvent * /* event */) override
    {
      resized->whichGrabber(which);
    }
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override
    {
      resized->resize(event->scenePos());
    }
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *  event ) override
    {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
        if (event->button() == Qt::LeftButton){
            resized->change();
        }
    }
    virtual void hoverEnterEvent (QGraphicsSceneHoverEvent *event) override
    {
      QGraphicsItem::hoverEnterEvent(event);
      QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    }
    virtual void hoverLeaveEvent (QGraphicsSceneHoverEvent *event) override
    {
      QGraphicsItem::hoverLeaveEvent(event);
      QApplication::setOverrideCursor(Qt::ArrowCursor);
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
        grabbers[i] = nullptr;  
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

//------------------PixMap-------------------------//

class ResizePixmapItem: public QGraphicsPixmapItem, 
                        public MetaItem, 
                        public ResizeConstAspectRect
{
  public:
    QPointF  position;
    bool     positionChanged;

    ResizePixmapItem();

    virtual void change() = 0;
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

//------------------Text-------------------------//

class ResizeTextItem: public QGraphicsTextItem,
                      public MetaItem,
                      public ResizeConstAspectRect
{
  public:
    QPointF  position;
    bool     positionChanged;

    ResizeTextItem(QGraphicsItem *parent = nullptr);

    virtual void change() = 0;
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

//-----------------------------------------------------------
//-----------------------------------------------------------

class InsertPixmapItem : public ResizePixmapItem
{

  public:

    InsertMeta meta;
    //qreal      relativeToLoc[2];
    //qreal      relativeToSize[2];

    InsertPixmapItem(
      QPixmap       &pixmap,
      InsertMeta insertMeta,
      QGraphicsItem *parent);

    virtual void change();
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

#endif
