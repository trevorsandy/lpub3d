 
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
 * This class provides a simple mechanism for displaying arbitrary text
 * on the page.  
 *
 ***************************************************************************/

#ifndef textItemH
#define textItemH

#include <QGraphicsTextItem>
#include <QFont>
#include "placement.h"
#include "meta.h"
#include "metaitem.h"
#include "lpub.h"

class TextItem : public QGraphicsTextItem, public Placement, public MetaItem
{
public:
  InsertMeta meta;
  bool positionChanged;
  QPointF position;
  bool textChanged;

  TextItem()
  {
  }
  TextItem(
    InsertMeta meta,
    QGraphicsItem *parent) :  meta(meta)
  {
    InsertData data = meta.value();
    setParentItem(parent);

    QString fontString = data.textFont;
    if (fontString.length() == 0) {
      fontString = "Arial,48,-1,255,75,0,0,0,0,0";
    }

    QFont font;
    font.fromString(fontString);
    setFont(font);

    QColor color(data.textColor);
    setDefaultTextColor(color);

    QRegExp rx("\\\\n");
    QStringList list = data.text.split(rx);
    QString string = list.join("\n");

    QRegExp rx2("\\\\""");
    QStringList list2 = string.split(rx2);
    QString string2 = list2.join("""");

    setPlainText(string2);

    setTextInteractionFlags(Qt::TextEditorInteraction);

    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setZValue(1000);
    margin.setValues(0.0,0.0);
  }

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent * /* event */);
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent * /* event */);
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
  void focusInEvent(QFocusEvent *event);
  void focusOutEvent(QFocusEvent *event);
  void keyPressEvent(QKeyEvent *event);
  void keyReleaseEvent(QKeyEvent *event);
};

#endif
