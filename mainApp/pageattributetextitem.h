/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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

#ifndef PAGEATTRIBUTETextITEM_H
#define PAGEATTRIBUTETextITEM_H

#include <QGraphicsTextItem>
#include <QGraphicsPixmapItem>
#include "where.h"
#include "placement.h"
#include "metaitem.h"

class Where;
class Page;
class Step;

class PageAttributeTextItem : public QGraphicsTextItem, public MetaItem, public Placement
{
    Page                    *page;
public:   
    PlacementType          relativeType;
    PlacementType          parentRelativeType;
    FontMeta               textFont;
    StringMeta             textColor;
    StringMeta             content;
    BoolMeta               displayText;
    QString                name;

    bool                   textValueChanged;
    bool                   positionChanged;
    QPointF                position;
    MetaItem               mi;

    PageAttributeTextItem();

    PageAttributeTextItem(
            Page                       *page,
            PageAttributeTextMeta     &_pageAttributeText,
            QGraphicsItem              *parent);

    void setAttributes(
            PlacementType                relativeType,
            PlacementType                parentRelativeType,
            PageAttributeTextMeta       &pageAttributeText,
            QString                     &toolTip,
            QGraphicsItem               *parent,
            QString                     &name);

    void setFlags( GraphicsItemFlag flag, bool value)
    {
        QGraphicsTextItem::setFlag(flag,value);
    }
protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    virtual void focusInEvent(QFocusEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

};

#endif // PAGEATTRIBUTETextITEM_H
