/****************************************************************************
**
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
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

#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>

#include <QColor>
#include <QPixmap>
#include <QAction>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>

#include "pageattributeitem.h"
#include "metatypes.h"
#include "color.h"
#include "name.h"
#include "placementdialog.h"
#include "commonmenus.h"

#include "ranges.h"
#include "step.h"

void PageAttributeItem::setAttributes(
  PlacementType              _relativeType,
  PlacementType              _parentRelativeType,
  Page                      *_page,
  Meta                      *_meta,
  PageAttributeTextMeta     &_pageAttributeText,
  PageAttributePictureMeta  &_pageAttributePicture,
  bool                       _isPicture,
  QString                   &toolTip,
  QGraphicsItem             *_parent)
{
    page               =  _page;
    meta               =  _meta;
    relativeType       =  _relativeType;
    parentRelativeType =  _parentRelativeType;

    if (_isPicture){
        // manage picture attributes here
        PageAttributePictureData _Picture = _pageAttributePicture.value();
        picScale       =  _Picture.picScale;
        displayPic     = &_Picture.display;

//        pageAttributePixmap.load(_Picture.string);
//        setPixmap(pageAttributePixmap);
//        setParentItem(_parent);
//        setTransformationMode(Qt::SmoothTransformation);

//        size[0] = pageAttributePixmap.width() *picScale;
//        size[1] = pageAttributePixmap.height() *picScale;

//        setFlag(QGraphicsItem::ItemIsSelectable,true);
//        setFlag(QGraphicsItem::ItemIsMovable,true);
//        setZValue(500);

        //PageAttributePixmapItem *pixmap = new PageAttributePixmapItem(qpixmap,_pageAttributePicture,_parent);
        //page->addPageAttributePixmap(pixmap);
        //pixmap->setTransformationMode(Qt::SmoothTransformation);
        //pixmap->scale(picScale,picScale);

    } else {
        // manage text attributes here
        textFont           = &_pageAttributeText.textFont;
        textColor          = &_pageAttributeText.textColor;
        margin             = &_pageAttributeText.margin;
        alignment          = &_pageAttributeText.alignment;
        displayText        = &_pageAttributeText.display;
        content            = &_pageAttributeText.content;

        QFont qfont;
        qfont.fromString(_pageAttributeText.textFont.valueFoo());
        setFont(qfont);

        QString foo;
        foo = content->value();
        setPlainText(foo);
        setDefaultTextColor(LDrawColor::color(textColor->value()));
    }

  setToolTip(toolTip);
  setParentItem(_parent);
}

PageAttributeItem::PageAttributeItem()
{
  relativeType  = PageAttributeType;    // should be from PageAttributeData
  meta          = NULL;
  textFont      = NULL;
  textColor     = NULL;
  margin        = NULL;
  alignment     = NULL;
  content       = NULL;
  picScale      = 0.0;
}

PageAttributeItem::PageAttributeItem(
  PlacementType              _relativeType,
  PlacementType              _parentRelativeType,
  Page                      *_page,
  Meta                      *_meta,
  PageAttributeTextMeta     &_pageAttributeText,
  PageAttributePictureMeta  &_pageAttributePicture,
  bool                       _isPicture,
  QString                   &_toolTip,
  QGraphicsItem             *_parent)
{
  setAttributes(_relativeType,
                _parentRelativeType,
                _page,
                _meta,
                _pageAttributeText,
                _pageAttributePicture,
                _isPicture,
                _toolTip,
                _parent);
}

PagePageAttributeItem::PagePageAttributeItem(
  Page                      *_page,
  PageAttributeTextMeta     &_pageAttributeText,
  PageAttributePictureMeta  &_pageAttributePicture,
  bool                      *_isPicture,
  QGraphicsItem             *_parent)
{
  page          = _page;
  QString       toolTip;
  PlacementType pageAttributeType;
  switch(_pageAttributeText.type)
  {
  case PageAttributeTextMeta::PageTitleType:
      pageAttributeType = PageTitleType;
      toolTip           = tr("Title text - right-click to modify");
      break;
  case PageAttributeTextMeta::PageModelNameType:
      pageAttributeType = PageModelNameType;
      toolTip           = tr("Model Name text - right-click to modify");
      break;
  case PageAttributeTextMeta::PageAuthorType:
      pageAttributeType = PageAuthorType;
      toolTip           = tr("Author text - right-click to modify");
      break;
  case PageAttributeTextMeta::PageURLType:
      pageAttributeType = PageURLType;
      toolTip           = tr("URL text - right-click to modify");
      break;
  case PageAttributeTextMeta::PageModelDescType:
      pageAttributeType = PageModelDescType;
      toolTip           = tr("Model Description text - right-click to modify");
      break;
  case PageAttributeTextMeta::PagePublishDescType:
      pageAttributeType = PagePublishDescType;
      toolTip           = tr("Publish Description text - right-click to modify");
      break;
  case PageAttributeTextMeta::PageCopyrightType:
      pageAttributeType = PageCopyrightType;
      toolTip           = tr("Copyright text - right-click to modify");
      break;
  case PageAttributeTextMeta::PageEmailType:
      pageAttributeType = PageEmailType;
      toolTip           = tr("Email text - right-click to modify");
      break;
  case PageAttributeTextMeta::PageDisclaimerType:
      pageAttributeType = PageDisclaimerType;
      toolTip           = tr("Disclaimer text - right-click to modify");
      break;
  case PageAttributeTextMeta::PagePiecesType:
      pageAttributeType = PagePiecesType;
      toolTip           = tr("Pieces text - right-click to modify");
      break;
  case PageAttributeTextMeta::PagePlugType:
      pageAttributeType = PagePlugType;
      toolTip           = tr("Plug text - right-click to modify");
      break;
  case PageAttributeTextMeta::PageCategoryType:
      pageAttributeType = PageCategoryType;
      toolTip           = tr("Category text - right-click to modify");
      break;
  }

  switch(_pageAttributePicture.value().type)
  {
  case PageAttributePictureData::PageDocumentLogoType:
      pageAttributeType = PageDocumentLogoType;
      toolTip           = tr("Logo image - right-click to modify");
      break;
  case PageAttributePictureData::PageCoverImageType:
      pageAttributeType = PageCoverImageType;
      toolTip           = tr("Cover image - right-click to modify");
      break;
  case PageAttributePictureData::PagePlugImageType:
      pageAttributeType = PagePlugImageType;
      toolTip           = tr("Plug image - right-click to modify");
      break;
  }

  setAttributes(pageAttributeType,
                SingleStepType,
                _page,
                meta,
                _pageAttributeText,
                _pageAttributePicture,
                _isPicture,
                 toolTip,
                _parent);
}

void PagePageAttributeItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  PlacementData placementData = placement.value();
  QString name = "Move Page Attribute";
  QAction *placementAction  = menu.addAction(name);
  QAction *marginAction     = menu.addAction("Change Page Attribute Margins");
  placementAction->setWhatsThis(
    commonMenus.naturalLanguagePlacementWhatsThis(PageAttributeType,placementData,name));

  Where topOfSteps      = page->topOfSteps();
  Where bottomOfSteps   = page->bottomOfSteps();
  Where begin           = topOfSteps;

  QAction *selectedAction   = menu.exec(event->screenPos());

  if (selectedAction == placementAction) {

    changePlacement(PageType,
                    PageAttributeType,
                    "Move Page Attribute",
                    topOfSteps,
                    bottomOfSteps,
                    &placement);

  } else if (selectedAction == marginAction) {

      changeMargins("Page Attribute Margins",
                    topOfSteps,bottomOfSteps,margin);
  }

  switch(relativeType)
  {
  case PageAttributeTextMeta::PageTitleType:
  case PageAttributeTextMeta::PageModelNameType:
  case PageAttributeTextMeta::PageAuthorType:
  case PageAttributeTextMeta::PageURLType:
  case PageAttributeTextMeta::PageModelDescType:
  case PageAttributeTextMeta::PagePublishDescType:
  case PageAttributeTextMeta::PageCopyrightType:
  case PageAttributeTextMeta::PageEmailType:
  case PageAttributeTextMeta::PageDisclaimerType:
  case PageAttributeTextMeta::PagePiecesType:
  case PageAttributeTextMeta::PagePlugType:
  case PageAttributeTextMeta::PageCategoryType:
      //text
      QAction *fontAction       = menu.addAction("Change Page Attribute Font");
      QAction *colorAction      = menu.addAction("Change Page Attribute Color");


      //text
      fontAction->setWhatsThis("You can change the textFont or the size of the page pageAttributeText");
      colorAction->setWhatsThis("You can change the textColor of the page pageAttributeText");
      marginAction->setWhatsThis("You can change how much empty space their is around the page pageAttributeText");

      if (selectedAction == fontAction) {

        changeFont(topOfSteps,bottomOfSteps,textFont);

      } else if (selectedAction == colorAction) {

        changeColor(topOfSteps,bottomOfSteps,textColor);

      }
      break;
  }

  switch(relativeType)
  {
  case PageAttributePictureData::PageDocumentLogoType:
  case PageAttributePictureData::PageCoverImageType:
  case PageAttributePictureData::PagePlugImageType:
      //picture
      QAction *scaleAction      = menu.addAction("Change Image Scale");

      //picture
      scaleAction->setWhatsThis("You can change the size of this image using the scale dialog (window).");

      if (selectedAction == scaleAction) {

          bool allowLocal = parentRelativeType != StepGroupType &&
                            parentRelativeType != CalloutType;
          PageAttributePictureData Picture = meta->LPub.page.documentLogo.value();
          FloatMeta picScale;
          picScale.setValue(Picture.picScale);
          // implement swithc to handle all image types
          // finish process (e.g. implement change method see csiitem.cpp line 382)
          changeFloatSpin("Image",
                          "Image Size",
                          begin,
                          topOfSteps,
                          &picScale,     //investigate
                          1,allowLocal);
      }
      break;
  }
}

void PagePageAttributeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

    QPointF newPosition;

    // back annotate the movement of the PLI into the LDraw file.
    newPosition = pos() - position;

    if (newPosition.x() || newPosition.y()) {
      positionChanged = true;

      PlacementData placementData = placement.value();

      placementData.offsets[0] += newPosition.x()/relativeToSize[0];
      placementData.offsets[1] += newPosition.y()/relativeToSize[1];

      placement.setValue(placementData);

      changePlacementOffset(page->bottomOfSteps(),&placement,PageAttributeType);

    }
  }
}

void PagePageAttributeItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mousePressEvent(event);
  positionChanged = false;
  position = pos();
}

void PagePageAttributeItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseMoveEvent(event);
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    positionChanged = true;
  }
}

void PagePageAttributeItem::focusInEvent(QFocusEvent *event)
{
  textValueChanged = false;
  QGraphicsTextItem::focusInEvent(event);
}

void PagePageAttributeItem::focusOutEvent(QFocusEvent *event)
{
  QGraphicsTextItem::focusOutEvent(event);

  // change meta

//  if (textChanged) {
//    InsertData insertData = meta.value();
//    QStringList list = toPlainText().split("\n");
//    insertData.text = list.join("\\n");
//    meta.setValue(insertData);

//    beginMacro(QString("Edit"));
//    changeInsertOffset(&meta);    // Create New one to accept PageAttributeItewm
//    endMacro();
//  }
}

void PagePageAttributeItem::keyPressEvent(QKeyEvent *event)
{
  textValueChanged = true;
  QGraphicsTextItem::keyPressEvent(event);
}
void PagePageAttributeItem::keyReleaseEvent(QKeyEvent *event)
{
  textValueChanged = true;
  QGraphicsTextItem::keyReleaseEvent(event);
}
