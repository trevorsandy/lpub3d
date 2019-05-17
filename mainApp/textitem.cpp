 
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

#include "textitem.h"
#include <QMenu>
#include <QAction>
#include <QGraphicsSceneContextMenuEvent>
#include <QFontDialog>
#include <QColor>
#include <QColorDialog>
#include "commonmenus.h"
#include "pagebackgrounditem.h"
#include "lpub.h"

TextItem::TextItem(
  InsertMeta meta,
  QGraphicsItem *parent)
    : meta(meta)
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

  QString string;

  QStringList list = data.text.split("\\n");

  QStringList list2;
  foreach (QString string, list){
    string = string.trimmed();
    QRegExp rx2("\"");
    int pos = 0;
    QChar esc('\\');
    while ((pos = rx2.indexIn(string, pos)) != -1) {
      if (pos < string.size()) {
        QChar ch = string.at(pos-1);
        if (ch == esc) {
          string.remove(pos-1,1);
          pos += rx2.matchedLength() - 1;
        }
      }
    }
    // if last character is \, append space ' ' so not to escape closing string double quote
    if (string.at(string.size()-1) == QChar('\\'))
      string.append(QChar(' '));
    list2 << string;
  }

  setPlainText(list2.join("\n"));

  setTextInteractionFlags(Qt::TextEditorInteraction);

  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIsSelectable);
  setData(ObjectId, InsertTextObj);
  margin.setValues(0.0,0.0);
}

void TextItem::contextMenuEvent(
  QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QString pl = "Text";

  QAction *editFontAction  = commonMenus.fontMenu(menu,pl);
  QAction *editColorAction = commonMenus.colorMenu(menu,pl);

  QAction *deleteTextAction = menu.addAction("Delete This Text");
  deleteTextAction->setIcon(QIcon(":/resources/textDelete.png"));
  deleteTextAction->setWhatsThis("Delete this text");

  QAction *bringToFrontAction = nullptr;
  QAction *sendToBackBackAction = nullptr;
  if (gui->pagescene()->showContextAction()) {
      if (!gui->pagescene()->isSelectedItemOnTop())
          bringToFrontAction = commonMenus.bringToFrontMenu(menu, pl);
      if (!gui->pagescene()->isSelectedItemOnBottom())
          sendToBackBackAction  = commonMenus.sendToBackMenu(menu, pl);
  }

  QAction *selectedAction  = menu.exec(event->screenPos());

  Where here = meta.here();

  if (selectedAction == nullptr) {
    return;
  }

  if (selectedAction == editFontAction) {

    InsertData data = meta.value();
    FontMeta font;
    font.setValuePoints(data.textFont);

    QFont _font;
    QString fontName = font.valueFoo();
    _font.fromString(fontName);

    bool ok;

    _font = QFontDialog::getFont(&ok,_font);

    if (ok) {

        data.textFont = _font.toString();
        meta.setValue(data);

        beginMacro("UpdateFont");
        replaceMeta(meta.here(),meta.format(false,false));
        endMacro();

    } else {
        gui->displayPage();
    }

  } else if (selectedAction == editColorAction) {
    InsertData data = meta.value();

    QColor color(data.textColor);

    color = QColorDialog::getColor(color);

    data.textColor = color.name();
    meta.setValue(data);
    beginMacro("UpdateColor");
    replaceMeta(meta.here(),meta.format(false,false));
    endMacro();

  } else if (selectedAction == deleteTextAction) {

    beginMacro("DeleteText");
    deleteMeta(here);
    endMacro();
  } else if (selectedAction == bringToFrontAction) {
      PageBackgroundItem * pageBgItem = qgraphicsitem_cast<PageBackgroundItem *>(parentObject());
      setSelectedItemZValue(here,
                            here,
                            BringToFront,
                            &pageBgItem->meta->LPub.page.scene.insertText);
  } else if (selectedAction == sendToBackBackAction) {
      PageBackgroundItem * pageBgItem = qgraphicsitem_cast<PageBackgroundItem *>(parentObject());
      setSelectedItemZValue(here,
                            here,
                            SendToBack,
                            &pageBgItem->meta->LPub.page.scene.insertText);
  }
}

void TextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  position = pos();
  positionChanged = false;
  QGraphicsItem::mousePressEvent(event);
}

void TextItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  positionChanged = true;
  QGraphicsItem::mouseMoveEvent(event);
}

void TextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable) && positionChanged) {

    InsertData insertData = meta.value();

    qreal topLeft[2] = { sceneBoundingRect().left(),  sceneBoundingRect().top() };
    qreal size[2]    = { sceneBoundingRect().width(), sceneBoundingRect().height() };

    PlacementData pld;

    pld.placement    = TopLeft;
    pld.justification    = Center;
    pld.relativeTo      = PageType;
    pld.preposition   = Inside;

    calcOffsets(pld,insertData.offsets,topLeft,size);

    QStringList list = toPlainText().split("\n");

    QStringList list2;
    foreach (QString string, list){
      string = string.trimmed();
      QRegExp rx2("\"");
      int pos = 0;
      QChar esc('\\');
      while ((pos = rx2.indexIn(string, pos)) != -1) {
        pos += rx2.matchedLength();
        if (pos < string.size()) {
          QChar ch = string.at(pos-1);
          if (ch != esc) {
            string.insert(pos-1,&esc,1);
            pos++;
          }
        }
      }
      // if last character is \, append space ' ' so not to escape closing string double quote
      if (string.at(string.size()-1) == QChar('\\'))
        string.append(QChar(' '));
      list2 << string;
    }

    insertData.text = list2.join("\\n");
    meta.setValue(insertData);

    beginMacro(QString("MoveText"));

    changeInsertOffset(&meta);

    endMacro();
  }
}

void TextItem::focusInEvent(QFocusEvent *event)
{
  textChanged = false;
  QGraphicsTextItem::focusInEvent(event);
}

void TextItem::focusOutEvent(QFocusEvent *event)
{
  QGraphicsTextItem::focusOutEvent(event);
  if (textChanged) {
    InsertData insertData = meta.value();

    QStringList list = toPlainText().split("\n");

    QStringList list2;
    foreach (QString string, list){
      string = string.trimmed();
      QRegExp rx2("\"");
      int pos = 0;
      QChar esc('\\');
      while ((pos = rx2.indexIn(string, pos)) != -1) {
        pos += rx2.matchedLength();
        if (pos < string.size()) {
          QChar ch = string.at(pos-1);
          if (ch != esc) {
            string.insert(pos-1,&esc,1);
            pos++;
          }
        }
      }
      // if last character is \, append space ' ' so not to escape closing string double quote
      if (string.at(string.size()-1) == QChar('\\'))
        string.append(QChar(' '));
      list2 << string;
    }

    insertData.text = list2.join("\\n");
    meta.setValue(insertData);

    beginMacro(QString("EditText"));
    changeInsertOffset(&meta);
    endMacro();
  }
}

void TextItem::keyPressEvent(QKeyEvent *event)
{
  textChanged = true;
  QGraphicsTextItem::keyPressEvent(event);
}

void TextItem::keyReleaseEvent(QKeyEvent *event)
{
  textChanged = true;
  QGraphicsTextItem::keyReleaseEvent(event);
}
