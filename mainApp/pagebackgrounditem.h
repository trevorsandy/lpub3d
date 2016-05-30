 
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
* sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
 *
 * This file represents the page background and is derived from the generic
 * background class described in background.(h,cpp)
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef BACKGROUNDITEM_H
#define BACKGROUNDITEM_H

#include <QGraphicsScene>
#include "backgrounditem.h"
#include "ranges.h"

class PageBackgroundItem : public BackgroundItem
{
  private:
    PlacementType  relativeType;
    QPixmap *pixmap;
    Page    *page;

  public:
        PageBackgroundItem(
          Page   *_page,
          int     width,
          int     height);

//    PageBackgroundItem(
//      Page   *_page,
//      int     width,
//      int     height)
//    {
//      page = _page;

//      relativeType = page->relativeType;
//#if 0
//      width = int(page->meta.LPub.page.size.valuePixels(0));
//      height= int(page->meta.LPub.page.size.valuePixels(1));
//#endif

//      pixmap = new QPixmap(width,height);

//      QString toolTip("Page background - right-click to modify");

//      setBackground(pixmap,
//                    PageType,
//                   &page->meta,
//                    page->meta.LPub.page.background,
//                    page->meta.LPub.page.border,
//                    page->meta.LPub.page.margin,
//                    page->meta.LPub.page.subModelColor,
//                    page->meta.submodelStack.size(),
//                    toolTip);

//      setPixmap(*pixmap);
//      setFlag(QGraphicsItem::ItemIsSelectable,false);
//      setFlag(QGraphicsItem::ItemIsMovable,false);

//      delete pixmap;
//    }
    ~PageBackgroundItem()
    {
//      delete pixmap;
    }
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

#endif
