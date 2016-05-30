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

#ifndef PAGEATTRIBUTEPIXMAPITEM_H
#define PAGEATTRIBUTEPIXMAPITEM_H

#include <QGraphicsPixmapItem>
#include <QSize>
#include <QRect>
#include "meta.h"
#include "metaitem.h"
#include "resize.h"

#include "QsLog.h"

class PageAttributePixmapItem : public ResizePixmapItem
{
  Page                          *page;
  public:
    PlacementType               relativeType;
    PlacementType               parentRelativeType;
    BoolMeta                    displayPicture;
    FloatMeta                   picScale;

    qreal                       relativeToLoc[2];
    qreal                       relativeToSize[2];
    QString                     name;
    MetaItem                    mi;

    PageAttributePixmapItem(
      Page                     *page,
      QPixmap                  &pixmap,
      PageAttributePictureMeta &pageAttributePictureMeta,
      QGraphicsItem            *parent);

    virtual void change();
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

#endif // PAGEATTRIBUTEPIXMAPITEM_H
