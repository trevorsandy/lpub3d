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
  public:
    Page                       *page;
    PlacementType               parentRelativeType;
    BorderMeta                  border;
    BoolMeta                    displayPicture;
    FloatMeta                   picScale;
    BoolMeta                    display;
    QString                     name;
    int                         fillMode;

    PageAttributePixmapItem(
      Page                     *page,
      QPixmap                  &_pixmapPic,
      PageAttributePictureMeta &_papMeta,
      QGraphicsItem            *parent);

    void adjustImage(
      QPixmap                  *pixmap,
      PageAttributePictureMeta &_papMeta);

    virtual void change();
protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    QPixmap * pixmapPic;
};

#endif // PAGEATTRIBUTEPIXMAPITEM_H
