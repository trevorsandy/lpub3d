 
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
#include "placementdialog.h"
#include "pairdialog.h"

TextItem::TextItem(InsertMeta meta,
  int onPageType,
  bool placement,
  QGraphicsItem *parent)
    : meta(meta),
      onPageType( onPageType),
      pagePlaced( false),
      textPlacement(placement),
      textChanged(false),
      isHovered(  false),
      mouseIsDown(false)
{
  if (meta.value().placementCommand) {
      QString line = gui->readLine(meta.here());
      emit gui->messageSig(LOG_ERROR, QString("Text placement command must come after an 'Add Text' command.<br>Line: %1")
                           .arg(QString("%1 %2%3").arg(meta.here().lineNumber).arg(meta.here().modelName).arg(line.isEmpty() ? "" : line)));
      return;
  }

  InsertData data    = meta.value();
  richText           = data.type == InsertData::InsertRichText;
  setParentItem(parent);

  QString fontString = data.textFont;
  if (fontString.length() == 0) {
    fontString = "Arial,24,-1,255,75,0,0,0,0,0";
  }

  QFont font;
  font.fromString(fontString);
  setFont(font);

  QColor color(data.textColor);
  setDefaultTextColor(color);

  // unformat text - remove quote escapte
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

    if (richText)
      setHtml(list2.join("\n"));
    else
      setPlainText(list2.join("\n"));
  }

  margin.setValues(0.0,0.0);

  setTextInteractionFlags(Qt::TextEditorInteraction);
  setAcceptHoverEvents(true);
  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIsSelectable);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setData(ObjectId, InsertTextObj);
  setZValue(INSERTTEXT_ZVALUE_DEFAULT);
}

void TextItem::formatText(const QString &input, QString &output)
{
    QStringList list = input.split("\n");

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

   output = list2.join("\\n");
}

void TextItem::contextMenuEvent(
  QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QString pl = "Text";

  InsertData data = meta.value();

  PlacementData placementData = placement.value();

  QAction *editTextAction   = commonMenus.textMenu(menu,pl);
  QAction *placementAction = nullptr;

  if (textPlacement) {
      placementAction = commonMenus.placementMenu(menu,pl,
                        commonMenus.naturalLanguagePlacementWhatsThis(TextType,placementData,pl));

      // remove offset from insertData if textPlacement enabled
      data.offsets[XX] = 0.0f;
      data.offsets[YY] = 0.0f;
  }

  QAction *editFontAction  = nullptr;
  QAction *editColorAction = nullptr;
  if (!richText){
    editFontAction  = commonMenus.fontMenu(menu,pl);
    editColorAction = commonMenus.colorMenu(menu,pl);
  }

  QAction *deleteTextAction = menu.addAction("Delete This Text");
  deleteTextAction->setIcon(QIcon(":/resources/textDelete.png"));
  deleteTextAction->setWhatsThis("Delete this text");

  QAction *selectedAction  = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
    return;
  }

  if (selectedAction == editTextAction) {

    bool multiStep = parentRelativeType == StepGroupType;

    updateText(meta.here(),
               data.text,
               data.textFont,
               data.textColor,
               data.offsets[XX],
               data.offsets[YY],
               parentRelativeType,
               richText,
               false/*append*/);

  } else if (selectedAction == editFontAction) {

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

    QColor color(data.textColor);

    color = QColorDialog::getColor(color);

    if (color.isValid())
        data.textColor = color.name();

    meta.setValue(data);
    beginMacro("UpdateColor");
    replaceMeta(meta.here(),meta.format(false,false));
    endMacro();

  } else if (selectedAction == placementAction) {
      placement.preamble = QString("0 !LPUB INSERT %1 PLACEMENT ")
                                   .arg(richText ? "RICH_TEXT" : "TEXT");
      PlacementData placementData = placement.value();
      bool ok;
      ok = PlacementDialog
           ::getPlacement(parentRelativeType,relativeType,placementData,"Placement",onPageType);
      if (ok) {
        placement.setValue(placementData);
        QString line = gui->readLine(meta.here());
        if (line.contains(placement.preamble)) {
           line = placement.format(true,meta.global);
           replaceMeta(meta.here(),line);
        } else {
           Where walkFwd = meta.here() + 1;
           line = gui->readLine(walkFwd);
           if (line.contains(placement.preamble)) {
              line = placement.format(true,meta.global);
              replaceMeta(walkFwd,line);
           } else {
              bool local = LocalDialog::getLocal(VER_PRODUCTNAME_STR, "Change only this step?",nullptr);
              line = placement.format(local,false);
              insertMeta(walkFwd,line);
           }
        }
      }
  } else if (selectedAction == deleteTextAction) {
    Where walkFwd = meta.here() + 1;
    QString placement = QString("0 !LPUB INSERT %1 PLACEMENT ")
                                .arg(richText ? "RICH_TEXT" : "TEXT");
    QString line = gui->readLine(walkFwd);
    beginMacro("DeleteText");
    if (line.contains(placement))
        deleteMeta(walkFwd);
    deleteMeta(meta.here());
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
    // remove offset from insertData if textPlacement enabled
    if (textPlacement){
        insertData.offsets[XX] = 0.0f;
        insertData.offsets[YY] = 0.0f;
    }

    QString input,output;
    if (richText)
       input = toHtml();
    else
       input = toPlainText();

    formatText(input, output);

    insertData.text = output;
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

void TextItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  positionChanged = true;
  QGraphicsItem::mouseMoveEvent(event);
}

void TextItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = !this->isSelected() && !mouseIsDown;
    QGraphicsItem::hoverEnterEvent(event);
}

void TextItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void TextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  position = pos();
  positionChanged = false;
  mouseIsDown = true;
  QGraphicsItem::mousePressEvent(event);
//  update();
}

void TextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  mouseIsDown = false;
  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable) && positionChanged) {

    InsertData insertData = meta.value();

    qreal topLeft[2] = { sceneBoundingRect().left(),  sceneBoundingRect().top() };
    qreal size[2]    = { sceneBoundingRect().width(), sceneBoundingRect().height() };

    if (textPlacement)
        placement.preamble = QString("0 !LPUB INSERT %1 PLACEMENT ")
                                     .arg(richText ? "RICH_TEXT" : "TEXT");

    PlacementData pld = placement.value();

    calcOffsets(pld,insertData.offsets,topLeft,size);

    if (textPlacement) {
        // apply offset to placementData
        pld.offsets[XX] = insertData.offsets[XX];
        pld.offsets[YY] = insertData.offsets[YY];

        placement.setValue(pld);

        bool canReplace = false;
        Where walk = meta.here();
        QString line = gui->readLine(walk);
        if ((canReplace = line.contains(placement.preamble))) {
           line = placement.format(true,meta.global);
        } else {
           walk++;
           line = gui->readLine(walk);
           if ((canReplace = line.contains(placement.preamble))) {
              line = placement.format(true,meta.global);
           }
        }
        if (canReplace) {
          beginMacro(QString("MoveTextPlacement"));

          replaceMeta(walk,line);

          endMacro();
        }
    } else {
        QString input,output;
        if (richText)
           input = toHtml();
        else
           input = toPlainText();

        formatText(input, output);

        insertData.text = output;
        meta.setValue(insertData);

        beginMacro(QString("MoveText"));

        changeInsertOffset(&meta);

        endMacro();
    }
  }
}

void TextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  QPen pen;
  pen.setColor(isHovered ? QColor(Preferences::sceneGuideColor) : Qt::black);
  pen.setWidth(0/*cosmetic*/);
  pen.setStyle(isHovered ? Qt::PenStyle(Preferences::sceneGuidesLine) : Qt::NoPen);
  painter->setPen(pen);
  painter->setBrush(Qt::transparent);
  painter->drawRect(this->boundingRect());
  QGraphicsTextItem::paint(painter,option,widget);
}
