/****************************************************************************
**
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
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

#ifndef PAGEATTRIBUTEPIXMAPITEM_H
#define PAGEATTRIBUTEPIXMAPITEM_H

#include <QGraphicsPixmapItem>
#include "resize.h"

class PageAttributePixmapItem : public ResizePixmapItem
{
  Page                          *page;
  public:
    PlacementType               parentRelativeType;
    BoolMeta                    displayPicture;
    FloatMeta                   picScale;
    QString                     name;
    BorderMeta                  border;
    int                         fillMode;

    PageAttributePixmapItem(
      Page                     *page,
      QPixmap                  &pixmap,
      PageAttributePictureMeta &pageAttributePictureMeta,
      QGraphicsItem            *parent);

    virtual void change();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    /* Highlight bounding rectangle on hover */
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    bool isHovered;
    bool mouseIsDown;
};

#endif // PAGEATTRIBUTEPIXMAPITEM_H
