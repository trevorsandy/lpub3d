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

/***************************************************************************
 *
 * This file invokes the traverse function to count pages, and to gather
 * the contents of a given page of your building instructions.  Once
 * gathered, the contents of the page are translated to graphical representation
 * and presented to the user for editing.
 *
 **************************************************************************/

#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QScrollBar>
#include <QPixmap>
#include <QBitmap>
#include <QColor>
#include "commonmenus.h"
#include "callout.h"
#include "lpub.h"
#include "ranges.h"
#include "range.h"
#include "step.h"
#include "meta.h"
#include "color.h"
#include "pagebackgrounditem.h"
#include "numberitem.h"
#include "pageattributetextitem.h"
#include "pageattributepixmapitem.h"
#include "csiitem.h"
#include "calloutbackgrounditem.h"
#include "textitem.h"
#include "rotateiconitem.h"
#include "paths.h"

/*
 * We need to draw page every time there is change to the LDraw file.
 *   Changes can come from Menu->dialogs, people editing the file.
 *
 * Gui tracks modified, so whenever things go modified, we need to
 * delete all the GraphicsItems and do a freeranges.h.
 */

void Gui::clearPage(
    LGraphicsView  *view,
    QGraphicsScene *scene)
{
  page.freePage();
  page.pli.clear();

  if (view->pageBackgroundItem) {
      delete view->pageBackgroundItem;
      view->pageBackgroundItem = NULL;
    }
  scene->clear();
}

/*********************************************
 *
 * given a ranges.h for a page, format the
 * entire page.
 *
 ********************************************/

class SubmodelInstanceCount : public NumberPlacementItem
{
  Page *page;
  
public:
  SubmodelInstanceCount(
      Page                *pageIn,
      NumberPlacementMeta &numberMetaIn,
      const char          *formatIn,
      int                  valueIn,
      QGraphicsItem       *parentIn)    {
    page = pageIn;
    QString toolTip("Times used - right-click to modify");
    setAttributes(SubmodelInstanceCountType,
                  page->relativeType,
                  numberMetaIn,
                  formatIn,
                  valueIn,
                  toolTip,
                  parentIn);
  }
protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

void SubmodelInstanceCount::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QString name = "Submodel Instance Count";

  QAction *fontAction       = commonMenus.fontMenu(menu,name);
  QAction *colorAction      = commonMenus.colorMenu(menu,name);
  QAction *marginAction     = commonMenus.marginMenu(menu,name);
  QAction *placementAction  = commonMenus.placementMenu(menu,name,"You can move this submodel count around.");

  QAction *selectedAction   = menu.exec(event->screenPos());

  Where topOfSteps          = page->topOfSteps();
  Where bottomOfSteps       = page->bottomOfSteps();
  bool  useTop              = parentRelativeType != StepGroupType;

  if (selectedAction == NULL) {
    return;
  } else if (selectedAction == fontAction) {

      changeFont(topOfSteps,
                 bottomOfSteps,
                &font,1,true,
                 useTop);

    } else if (selectedAction == colorAction) {

      changeColor(topOfSteps,
                  bottomOfSteps,
                 &color,1,true,
                  useTop);

    } else if (selectedAction == marginAction) {

      changeMargins(name + " Margins",
                    topOfSteps,
                    bottomOfSteps,
                   &margin,
                    useTop);

    } else if (selectedAction == placementAction) {
      changePlacement(parentRelativeType,
                      SubmodelInstanceCountType,
                      "Move " + name,
                      topOfSteps,
                      bottomOfSteps,
                      &placement,
                      useTop);
    }
}

void SubmodelInstanceCount::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

      QPointF newPosition;

      Where topOfSteps    = page->topOfSteps();
      Where bottomOfSteps = page->bottomOfSteps();
      bool  useTop        = parentRelativeType != StepGroupType;

      // back annotate the movement of the PLI into the LDraw file.
      newPosition = pos() - position;

      if (newPosition.x() || newPosition.y()) {
          positionChanged = true;

          PlacementData placementData = placement.value();

          placementData.offsets[0] += newPosition.x()/relativeToSize[0];
          placementData.offsets[1] += newPosition.y()/relativeToSize[1];

          placement.setValue(placementData);

          changePlacementOffset(useTop ? topOfSteps:bottomOfSteps,
                                &placement,StepNumberType);
        }
    }
}

/*********************************************************************************
 * addGraphicsPageItems - this function is given the page contents in tree
 * data structure form, with all the CSI and PLI images rendered.  This function
 * then creates Qt graphics view items for each of the components of the page.
 * Sinple things like page number, page background, inserts and the like are
 * handled here.  For things like callouts and step groups, there is a bunch
 * of packing and placing things relative to things that must go on, these
 * operations are handled elsewhere (step.cpp, steps.cpp, callout.cpp, placement.cpp
 * etc.)
 *
 * This function is used for on screen display, printing to PDF and exporting
 * to images per page.
 ********************************************************************************/

int Gui::addGraphicsPageItems(
    Steps          *steps,
    bool            coverPage,
    bool            endOfSubmodel,
    LGraphicsView  *view,
    QGraphicsScene *scene,
    bool            printing)
{

  Page                    *page = dynamic_cast<Page *>(steps);

  PageBackgroundItem      *pageBg;
  Placement                plPage;
  PlacementHeader          pageHeader;
  PlacementFooter          pageFooter;
  PageNumberItem          *pageNumber;
  SubmodelInstanceCount   *instanceCount;

  PageAttributeTextItem   *url;
  PageAttributeTextItem   *email;
  PageAttributeTextItem   *copyright;
  PageAttributeTextItem   *author;

  PageAttributeTextItem   *titleFront;
  PageAttributeTextItem   *modelNameFront;
  PageAttributeTextItem   *modelDescFront;
  PageAttributeTextItem   *publishDescFront;
  PageAttributeTextItem   *authorFront;
  PageAttributeTextItem   *piecesFront;
  //  PageAttributeTextItem   *categoryFront;
  PageAttributePixmapItem *pixmapLogoFront;
  PageAttributePixmapItem *pixmapCoverImageFront;

  PageAttributePixmapItem *pixmapLogoBack;
  PageAttributeTextItem   *titleBack;
  PageAttributeTextItem   *authorBack;
  PageAttributeTextItem   *copyrightBack;
  PageAttributeTextItem   *urlBack;
  PageAttributeTextItem   *emailBack;
  PageAttributeTextItem   *disclaimerBack;
  PageAttributeTextItem   *plugBack;
  PageAttributePixmapItem *pixmapPlugImageBack;

  /* There are issues with printing to PDF and its fixed page sizes, and
   * LPub's aribitrary page size controls.  So when printing, we have to
   * pick a PDF page size that is closest but not smaller than the size
   * defined by the user, and then fill that page's background to the
   * edges.  The code below gets most of this done (the print caller
   * maps LPub page size to PDF page size and sets view size, so we
   * work with that here.
   *
   * There still seem to be cases where we get little gaps on either the
   * right or bottom edge, and I just don't know what the problem is.
   */

  int pW, pH;

  if (printing) {
      pW = view->maximumWidth();
      pH = view->maximumHeight();
    } else {
      pW = pageSize(page->meta.LPub, 0);
      pH = pageSize(page->meta.LPub, 1);
    }

//  logDebug() << QString("  DRAW PAGE %3 SIZE PIXELS - WidthPx: %1 x HeightPx: %2 CurPage: %3")
//                .arg(QString::number(pW), QString::number(pH)).arg(stepPageNum);

  pageBg = new PageBackgroundItem(page, pW, pH);

  view->pageBackgroundItem = pageBg;
  pageBg->setPos(0,0);

  // Set up the placement aspects of the page in the Qt space

  plPage.relativeType = PageType;

  plPage.setSize(pW,pH);
  plPage.margin  = page->meta.LPub.page.margin;
  plPage.loc[XX] = 0;
  plPage.loc[YY] = 0;

  // Set up page footer

  pageFooter.relativeType   = PageFooterType;
  pageFooter.placement      = page->meta.LPub.page.pageFooter.placement;
  pageFooter.size[XX]       = page->meta.LPub.page.pageFooter.size.valuePixels(XX);
  pageFooter.size[YY]       = page->meta.LPub.page.pageFooter.size.valuePixels(YY);

  if (pageFooter.placement.value().relativeTo == plPage.relativeType) {
      plPage.placeRelative(&pageFooter);
      plPage.appendRelativeTo(&pageFooter);
    }
  pageFooter.setPos(pageFooter.loc[XX],pageFooter.loc[YY]);

  // Display a page number?

  if (page->meta.LPub.page.dpn.value() && ! coverPage) {

      // allocate QGraphicsTextItem for page number

      pageNumber =
          new PageNumberItem(
            page,
            page->meta.LPub.page.number,
            "%d",
            stepPageNum,
            pageBg);

      pageNumber->relativeType = PageNumberType;
      pageNumber->size[XX]     = (int) pageNumber->document()->size().width();
      pageNumber->size[YY]     = (int) pageNumber->document()->size().height();

      PlacementData  placementData;

      placementData = pageNumber->placement.value();

      if (page->meta.LPub.page.togglePnPlacement.value() && ! (stepPageNum & 1)) {
          switch (placementData.placement) {
            case TopLeft:
              placementData.placement = TopRight;
              break;
            case Top:
            case Bottom:
              switch (placementData.justification) {
                case Left:
                  placementData.justification = Right;
                  break;
                case Right:
                  placementData.justification = Left;
                  break;
                default:
                  break;
                }
              break;
            case TopRight:
              placementData.placement = TopLeft;
              break;
            case Left:
              placementData.placement = Right;
              break;
            case Right:
              placementData.placement = Left;
              break;
            case BottomLeft:
              placementData.placement = BottomRight;
              break;
            case BottomRight:
              placementData.placement = BottomLeft;
              break;
            default:
              break;
            }

          pageNumber->placement.setValue(placementData);
        }

      plPage.appendRelativeTo(pageNumber);
      plPage.placeRelative(pageNumber);
      pageNumber->setPos(pageNumber->loc[XX],pageNumber->loc[YY]);

      //initialize Author
      if (page->meta.LPub.page.author.display.value()) {
          author               = new PageAttributeTextItem(page,page->meta.LPub.page.author,pageBg);
          author->relativeType = PageAuthorType;
          author->size[XX]     = (int) author->document()->size().width();
          author->size[YY]     = (int) author->document()->size().height();
      }

      if (! page->coverPage)
        {

          // URL,
          if (page->meta.LPub.page.url.display.value()) {
              url               = new PageAttributeTextItem(page,page->meta.LPub.page.url,pageBg);
              url->relativeType = PageURLType;
              url->size[XX]     = (int) url->document()->size().width();
              url->size[YY]     = (int) url->document()->size().height();
          }

          //Email
          if (page->meta.LPub.page.email.display.value()){
              email               = new PageAttributeTextItem(page,page->meta.LPub.page.email,pageBg);
              email->relativeType = PageEmailType;
              email->size[XX]     = (int) email->document()->size().width();
              email->size[YY]     = (int) email->document()->size().height();
          }

          //Copyright
          if (page->meta.LPub.page.copyright.display.value()){
              copyright               = new PageAttributeTextItem(page,page->meta.LPub.page.copyright,pageBg);
              copyright->relativeType = PageCopyrightType;
              copyright->size[XX]     = (int) copyright->document()->size().width();
              copyright->size[YY]     = (int) copyright->document()->size().height();
          }

          // allocate QGraphicsTextItem for // url (Header/Footer) //~~~~~~~~~~~~~~~~
          if (page->meta.LPub.page.url.display.value()) {

              // URL
              PlacementData pld = url->placement.value();
             if (pld.relativeTo == PageHeaderType) {
                  pageHeader.placeRelative(url);
                } else if (pld.relativeTo == PageFooterType) {
                  pageFooter.placeRelative(url);
                } else if (pld.relativeTo == PageNumberType) {
                  pageNumber->placeRelative(url);
                } else if (pld.relativeTo == PageEmailType &&
                           page->meta.LPub.page.email.display.value()) {
                  email->placeRelative(url);
                } else if (pld.relativeTo == PageCopyrightType &&
                           page->meta.LPub.page.copyright.display.value()) {
                  copyright->placeRelative(url);
                } else if (pld.relativeTo == PageAuthorType &&
                           page->meta.LPub.page.author.display.value()) {
                  author->placeRelative(url);
                } else {
                  plPage.appendRelativeTo(url);
                  plPage.placeRelative(url);
                }
              url->setPos(url->loc[XX],url->loc[YY]);
            }

          // allocate QGraphicsTextItem for // email (Header/Footer) //~~~~~~~~~~~~~~~~
          if (page->meta.LPub.page.email.display.value()) {

              //Email
              PlacementData pld = email->placement.value();
              if (pld.relativeTo == PageHeaderType) {
                  pageHeader.placeRelative(email);
                } else if (pld.relativeTo == PageFooterType) {
                  pageFooter.placeRelative(email);
                } else if (pld.relativeTo == PageNumberType) {
                  pageNumber->placeRelative(email);
                } else if (pld.relativeTo == PageURLType &&
                           page->meta.LPub.page.url.display.value()) {
                  url->placeRelative(email);
                } else if (pld.relativeTo == PageCopyrightType &&
                           page->meta.LPub.page.copyright.display.value()) {
                  copyright->placeRelative(email);
                } else if (pld.relativeTo == PageAuthorType &&
                           page->meta.LPub.page.author.display.value()) {
                  author->placeRelative(email);
                } else {
                  plPage.appendRelativeTo(email);
                  plPage.placeRelative(email);
                }
              email->setPos(email->loc[XX],email->loc[YY]);
            }

          // allocate QGraphicsTextItem for // author (Header/Footer) //~~~~~~~~~~~~~~~~
          if (page->meta.LPub.page.author.display.value()) {

              // Author
              PlacementData pld = author->placement.value();
              if (pld.relativeTo == PageHeaderType) {
                  pageHeader.placeRelative(author);
                } else if (pld.relativeTo == PageFooterType) {
                  pageFooter.placeRelative(author);
                } else if (pld.relativeTo == PageNumberType) {
                  pageNumber->placeRelative(author);
                } else if (pld.relativeTo == PageURLType &&
                           page->meta.LPub.page.url.display.value()) {
                  url->placeRelative(author);
                } else if (pld.relativeTo == PageEmailType &&
                           page->meta.LPub.page.email.display.value()) {
                  email->placeRelative(author);
                } else if (pld.relativeTo == PageCopyrightType &&
                           page->meta.LPub.page.copyright.display.value()) {
                  copyright->placeRelative(author);
                } else {
                  plPage.appendRelativeTo(author);
                  plPage.placeRelative(author);
                }
              author->setPos(author->loc[XX],author->loc[YY]);
            }


          // allocate QGraphicsTextItem for // copyright (Header/Footer) //~~~~~~~~~~~~~~~~
          if (page->meta.LPub.page.copyright.display.value()) {

              //Copyright
              PlacementData pld = copyright->placement.value();
              if (pld.relativeTo == PageHeaderType) {
                  pageHeader.placeRelative(copyright);
                } else if (pld.relativeTo == PageFooterType) {
                  pageFooter.placeRelative(copyright);
                } else if (pld.relativeTo == PageNumberType) {
                  pageNumber->placeRelative(copyright);
                } else if (pld.relativeTo == PageURLType &&
                           page->meta.LPub.page.url.display.value()) {
                  url->placeRelative(copyright);
                } else if (pld.relativeTo == PageEmailType &&
                           page->meta.LPub.page.email.display.value()) {
                  email->placeRelative(copyright);
                } else if (pld.relativeTo == PageAuthorType &&
                           page->meta.LPub.page.author.display.value()) {
                  author->placeRelative(copyright);
                } else {
                  plPage.appendRelativeTo(copyright);
                  plPage.placeRelative(copyright);
                }
              copyright->setPos(copyright->loc[XX],copyright->loc[YY]);
            }
        }

      // if this page contains the last step of the page,
      // and instance is > 1 then display instance

      // allocate QGraphicsTextItem for instance number

      if (endOfSubmodel && page->instances > 1) {
          instanceCount = new SubmodelInstanceCount(
                page,
                page->meta.LPub.page.instanceCount,
                "x%d ",
                page->instances,
                pageBg);

          /*
       * To make mousemove always know how to calculate offset, I modified
       * SubmodelInstanceClass to be derived from Placement.  The relativeToSize
       * offset calculation are in Placement.
       *
       * The offset calculation works great, but we end up with a problem
       * SubmodelInstanceCount gets placement from NumberPlacementItem, and
       * placement from Placement.  To work around this, I had to hack (and I mean
       * ugly) SubmodelInstanceCount to Placement.
       */

          if (instanceCount) {
              instanceCount->setSize(int(instanceCount->document()->size().width()),
                                     int(instanceCount->document()->size().height()));
              instanceCount->loc[XX] = 0;
              instanceCount->loc[YY] = 0;
              instanceCount->tbl[0] = 0;
              instanceCount->tbl[1] = 0;

              instanceCount->placement = page->meta.LPub.page.instanceCount.placement;

//              logDebug() << "TogglePageNumber:" << page->meta.LPub.page.togglePnPlacement.value();
//              logDebug() << (page->meta.LPub.page.number.number % 2 ?
//                                 tr("Page %1 is Even").arg(stepPageNum) :
//                                 tr("Page %1 is Odd").arg(stepPageNum));
//              logDebug() << "Placement: " << RectNames[page->meta.LPub.page.instanceCount.placement.value().rectPlacement]
//                         << "(" << page->meta.LPub.page.instanceCount.placement.value().rectPlacement << ")" ;

              PlacementData placementData = instanceCount->placement.value();

              if (placementData.relativeTo == PageNumberType &&
                      page->meta.LPub.page.togglePnPlacement.value() &&
                      ! (stepPageNum % 2 /* if page is odd */)) {
                  switch (placementData.rectPlacement){
                  case (TopLeftOutsideCorner):
                      instanceCount->placement.setValue(TopRightOutsideCorner,PageNumberType);
                      break;
                  case (TopLeftOutside):
                      instanceCount->placement.setValue(TopRightOutSide,PageNumberType);
                      break;
                  case (LeftTopOutside):
                      instanceCount->placement.setValue(RightTopOutside,PageNumberType);
                      break;
                  case (LeftOutside):
                      instanceCount->placement.setValue(RightOutside,PageNumberType);
                      break;
                  case (LeftBottomOutside):
                      instanceCount->placement.setValue(RightBottomOutside,PageNumberType);
                      break;
                  case (BottomLeftOutsideCorner):
                      instanceCount->placement.setValue(BottomRightOutsideCorner,PageNumberType);
                      break;
                  case (BottomLeftOutside):
                      instanceCount->placement.setValue(BottomRightOutside,PageNumberType);
                      break;
                  default:
                      instanceCount->placement = page->meta.LPub.page.instanceCount.placement;
                      break;
                  }
              } else {
                  instanceCount->placement = page->meta.LPub.page.instanceCount.placement;
              }

              if (placementData.relativeTo == PageNumberType &&
                  page->meta.LPub.page.dpn.value()) {
                  pageNumber->placeRelative(instanceCount);
                } else {
                  plPage.placeRelative(instanceCount);
                }
              instanceCount->setPos(instanceCount->loc[XX],instanceCount->loc[YY]);
            }
        }
    }
  
  /* Create any graphics items in the insert list */
  
  int nInserts = page->inserts.size();
  
  if (nInserts) {
      QFileInfo fileInfo;
      for (int i = 0; i < nInserts; i++) {
          InsertData insert = page->inserts[i].value();

          switch (insert.type) {
            case InsertData::InsertPicture:
              {
                fileInfo.setFile(insert.picName);
                if (fileInfo.exists()) {

                    QPixmap qpixmap;
                    qpixmap.load(insert.picName);
                    InsertPixmapItem *pixmap = new InsertPixmapItem(qpixmap,page->inserts[i],pageBg);

                    page->addInsertPixmap(pixmap);
                    pixmap->setTransformationMode(Qt::SmoothTransformation);
                    pixmap->setScale(insert.picScale,insert.picScale);

                    PlacementData pld;

                    pld.placement      = TopLeft;
                    pld.justification  = Center;
                    pld.relativeTo     = PageType;
                    pld.preposition    = Inside;
                    pld.offsets[0]     = insert.offsets[0];
                    pld.offsets[1]     = insert.offsets[1];

                    pixmap->placement.setValue(pld);

                    int margin[2] = {0, 0};

                    plPage.placeRelative(pixmap, margin);
                    pixmap->setPos(pixmap->loc[XX],pixmap->loc[YY]);
                    pixmap->relativeToSize[0] = plPage.size[XX];
                    pixmap->relativeToSize[1] = plPage.size[YY];
                  }
              }
              break;
            case InsertData::InsertText:
              {
                TextItem *text = new TextItem(page->inserts[i],pageBg);

                PlacementData pld;

                pld.placement     = TopLeft;
                pld.justification = Center;
                pld.relativeTo    = PageType;
                pld.preposition   = Inside;
                pld.offsets[0]    = insert.offsets[0];
                pld.offsets[1]    = insert.offsets[1];

                text->placement.setValue(pld);

                int margin[2] = {0, 0};

                plPage.placeRelative(text, margin);
                text->setPos(text->loc[XX],text->loc[YY]);
                text->relativeToSize[0] = plPage.size[XX];
                text->relativeToSize[1] = plPage.size[YY];
              }
              break;
            case InsertData::InsertArrow:
              break;
            case InsertData::InsertBom:
              {
                Where current(ldrawFile.topLevelFile(),0);
                QStringList bomParts;
                QString addLine;
                getBOMParts(current,addLine,bomParts);
                page->pli.steps = steps;
                getBOMOccurrence(current);
                if (boms > 1){
                    page->pli.setParts(bomParts,page->meta,true,true); //Split BOM Parts
                  } else {
                    page->pli.setParts(bomParts,page->meta,true);
                  }
                bomParts.clear();
                page->pli.sizePli(&page->meta,page->relativeType,false);
                page->pli.relativeToSize[0] = plPage.size[XX];
                page->pli.relativeToSize[1] = plPage.size[YY];
              }
              break;
            case InsertData::InsertRotateIcon:
              break;
            }
        }
    }

  // Process Front and Back Cover Pages - if selected
  if (page->coverPage && page->frontCover) {

      bool enableTitleFront = page->meta.LPub.page.titleFront.display.value();
      bool breakTitleFrontRelativeTo = false;
      PlacementData titleFrontPld;
      if (enableTitleFront) {

          titleFront               = new PageAttributeTextItem(page,page->meta.LPub.page.titleFront,pageBg);
          titleFront->relativeType = PageTitleType;
          titleFront->size[XX]     = (int) titleFront->document()->size().width();
          titleFront->size[YY]     = (int) titleFront->document()->size().height();

          titleFrontPld = titleFront->placement.value();
          breakTitleFrontRelativeTo = titleFrontPld.relativeTo != PageType;
      }

      bool enableAuthorFront = page->meta.LPub.page.authorFront.display.value();
      bool breakAuthorFrontRelativeTo = false;
      PlacementData authorFrontPld;
      if (enableAuthorFront) {

          authorFront               = new PageAttributeTextItem(page,page->meta.LPub.page.authorFront,pageBg);
          authorFront->relativeType = PageAuthorType;
          authorFront->size[XX]     = (int) authorFront->document()->size().width();
          authorFront->size[YY]     = (int) authorFront->document()->size().height();

          authorFrontPld = authorFront->placement.value();
          breakAuthorFrontRelativeTo = authorFrontPld.relativeTo != PageTitleType;
      }

      bool enablePiecesFront = page->meta.LPub.page.pieces.display.value();
      bool breakPiecesFrontRelativeTo = false;
      PlacementData piecesFrontPld;
      if (enablePiecesFront) {

          piecesFront               = new PageAttributeTextItem(page,page->meta.LPub.page.pieces,pageBg);
          piecesFront->relativeType = PagePiecesType;
          piecesFront->size[XX]     = (int) piecesFront->document()->size().width();
          piecesFront->size[YY]     = (int) piecesFront->document()->size().height();

          piecesFrontPld = piecesFront->placement.value();
          breakPiecesFrontRelativeTo = piecesFrontPld.relativeTo != PageAuthorType;
      }

      bool enableModelDescFront = page->meta.LPub.page.modelDesc.display.value();
      bool breakModelDescFrontRelativeTo = false;
      PlacementData modelDescFrontPld;
      if (enableModelDescFront) {

          modelDescFront               = new PageAttributeTextItem(page,page->meta.LPub.page.modelDesc,pageBg);
          modelDescFront->relativeType = PageModelDescType;
          modelDescFront->size[XX]     = (int) modelDescFront->document()->size().width();
          modelDescFront->size[YY]     = (int) modelDescFront->document()->size().height();

          modelDescFrontPld = modelDescFront->placement.value();
          breakModelDescFrontRelativeTo = modelDescFrontPld.relativeTo != PagePiecesType;
      }

      // allocate QGraphicsTextItem for // page Header (Front Cover) //~~~~~~~~~~~~~~~~
      pageHeader.relativeType   = PageHeaderType;
      pageHeader.placement      = page->meta.LPub.page.pageHeader.placement;
      pageHeader.size[XX]       = page->meta.LPub.page.pageHeader.size.valuePixels(XX);
      pageHeader.size[YY]       = page->meta.LPub.page.pageHeader.size.valuePixels(YY);
      if (pageHeader.placement.value().relativeTo == plPage.relativeType) {
          plPage.placeRelative(&pageHeader);
          plPage.appendRelativeTo(&pageHeader);
        }
      pageHeader.setPos(pageHeader.loc[XX],pageHeader.loc[YY]);

      // allocate QGraphicsTextItem for // page Footer (Front Cover) //~~~~~~~~~~~~~~~~
      pageFooter.relativeType   = PageFooterType;
      pageFooter.placement      = page->meta.LPub.page.pageFooter.placement;
      pageFooter.size[XX]       = page->meta.LPub.page.pageFooter.size.valuePixels(XX);
      pageFooter.size[YY]       = page->meta.LPub.page.pageFooter.size.valuePixels(YY);
      if (pageFooter.placement.value().relativeTo == plPage.relativeType) {
          plPage.placeRelative(&pageFooter);
          plPage.appendRelativeTo(&pageFooter);
        }
      pageFooter.setPos(pageFooter.loc[XX],pageFooter.loc[YY]);

      // allocate QGraphicsTextItem for // title (Front Cover) //~~~~~~~~~~~~~~~~
      if (enableTitleFront) {

          plPage.appendRelativeTo(titleFront);
          plPage.placeRelative(titleFront);

          titleFront->setPos(titleFront->loc[XX],titleFront->loc[YY]);
      }

      // allocate QGraphicsTextItem for // modelName (Front Cover) //~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.modelName.display.value()) {

          modelNameFront               = new PageAttributeTextItem(page,page->meta.LPub.page.modelName,pageBg);
          modelNameFront->relativeType = PageModelNameType;
          modelNameFront->size[XX]     = (int) modelNameFront->document()->size().width();
          modelNameFront->size[YY]     = (int) modelNameFront->document()->size().height();

          PlacementData pld = modelNameFront->placement.value();
          if (breakTitleFrontRelativeTo && pld.relativeTo == PageTitleType) {
              modelNameFront->placement.setValue(RightInside,PageType);
              plPage.appendRelativeTo(modelNameFront);
              plPage.placeRelative(modelNameFront);
            } else if (pld.relativeTo == PageTitleType &&
                       page->meta.LPub.page.titleFront.display.value()) {
              titleFront->placeRelative(modelNameFront);
            } else {
              plPage.appendRelativeTo(modelNameFront);
              plPage.placeRelative(modelNameFront);
            }
          modelNameFront->setPos(modelNameFront->loc[XX],modelNameFront->loc[YY]);
        }

      // allocate QGraphicsTextItem for // author (Front Cover) //~~~~~~~~~~~~~~~~
      if (enableAuthorFront) {

          authorFrontPld = authorFront->placement.value();
          if (breakTitleFrontRelativeTo && authorFrontPld.relativeTo == PageTitleType) {
              authorFront->placement.setValue(RightInside,PageType);
              plPage.appendRelativeTo(authorFront);
              plPage.placeRelative(authorFront);
            } else if (authorFrontPld.relativeTo == PageTitleType &&
                       page->meta.LPub.page.titleFront.display.value()) {
              titleFront->placeRelative(authorFront);
            } else {
              plPage.appendRelativeTo(authorFront);
              plPage.placeRelative(authorFront);
            }
          authorFront->setPos(authorFront->loc[XX],authorFront->loc[YY]);
        }

      // allocate QGraphicsTextItem for // pieces (Front Cover) //~~~~~~~~~~~~~~~~
      if (enablePiecesFront) {

          piecesFrontPld = piecesFront->placement.value();
          if (breakAuthorFrontRelativeTo && piecesFrontPld.relativeTo == PageAuthorType) {
              piecesFront->placement.setValue(RightInside,PageType);
              plPage.appendRelativeTo(piecesFront);
              plPage.placeRelative(piecesFront);
            } else if (piecesFrontPld.relativeTo == PageAuthorType &&
                       page->meta.LPub.page.authorFront.display.value()) {
              authorFront->appendRelativeTo(piecesFront);
              authorFront->placeRelative(piecesFront);
            } else {
              plPage.appendRelativeTo(piecesFront);
              plPage.placeRelative(piecesFront);
            }
          piecesFront->setPos(piecesFront->loc[XX],piecesFront->loc[YY]);
        }
      // allocate QGraphicsTextItem for // modelDesc (Front Cover) //~~~~~~~~~~~~~~~~
      if (enableModelDescFront) {

          modelDescFrontPld = page->meta.LPub.page.modelDesc.placement.value();
          if (breakPiecesFrontRelativeTo && modelDescFrontPld.relativeTo == PagePiecesType) {
              modelDescFront->placement.setValue(RightInside,PageType);
              plPage.appendRelativeTo(modelDescFront);
              plPage.placeRelative(modelDescFront);
          } else if (modelDescFrontPld.relativeTo == PagePiecesType &&
                     page->meta.LPub.page.pieces.display.value()) {
              piecesFront->appendRelativeTo(modelDescFront);
              piecesFront->placeRelative(modelDescFront);
          } else {
              plPage.appendRelativeTo(modelDescFront);
              plPage.placeRelative(modelDescFront);
          }
          modelDescFront->setPos(modelDescFront->loc[XX],modelDescFront->loc[YY]);
        }

      // allocate QGraphicsTextItem for // publishDesc (Front Cover) //~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.publishDesc.display.value()) {

          publishDescFront               = new PageAttributeTextItem(page,page->meta.LPub.page.publishDesc,pageBg);
          publishDescFront->relativeType = PagePublishDescType;
          publishDescFront->size[XX]     = (int) publishDescFront->document()->size().width();
          publishDescFront->size[YY]     = (int) publishDescFront->document()->size().height();

          PlacementData pld = publishDescFront->placement.value();
          if (breakModelDescFrontRelativeTo && pld.relativeTo == PageModelDescType) {
              publishDescFront->placement.setValue(RightInside,PageType);
              plPage.appendRelativeTo(publishDescFront);
              plPage.placeRelative(publishDescFront);
            } else if (pld.relativeTo == PageModelDescType &&
                       page->meta.LPub.page.modelDesc.display.value()) {
              modelDescFront->appendRelativeTo(publishDescFront);
              modelDescFront->placeRelative(publishDescFront);
            } else {
              plPage.appendRelativeTo(publishDescFront);
              plPage.placeRelative(publishDescFront);
            }
          publishDescFront->setPos(publishDescFront->loc[XX],publishDescFront->loc[YY]);
        }

      // allocate QGraphicsTextItem for // category (Front Cover) //~~~~~~~~~~~~~~~~
      /* if (page->meta.LPub.page.category.display.value()) {

        //categoryFront               = new PageAttributeTextItem(page,page->meta.LPub.page.category,pageBg);
          categoryFront->relativeType = PageCopyrightType;
          categoryFront->size[XX]     = (int) categoryFront->document()->size().width();
          categoryFront->size[YY]     = (int) categoryFront->document()->size().height();

          PlacementData pld = categoryFront->placement.value();
          if (pld.relativeTo == PageType) {
              plPage.appendRelativeTo(categoryFront);
              plPage.placeRelative(categoryFront);
          } else if (pld.relativeTo == PagePiecesType) {
              piecesFront->placeRelative(categoryFront);
          } else {
              categoryFront->placement.setValue(LeftInside,PageType);
              plPage.appendRelativeTo(categoryFront);
              plPage.placeRelative(categoryFront);
          }
          categoryFront->setPos(categoryFront->loc[XX],categoryFront->loc[YY]);
      } */

      // allocate QGraphicsPixmapItem for // documentLogoFront (Front Cover) //~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.documentLogoFront.display.value()) {

          QFileInfo fileInfo;
          QString file = page->meta.LPub.page.documentLogoFront.file.value();
          qreal picScale   = page->meta.LPub.page.documentLogoFront.picScale.value();
          fileInfo.setFile(file);
          if (fileInfo.exists()) {

              QPixmap qpixmap;
              qpixmap.load(file);
              pixmapLogoFront
                  = new PageAttributePixmapItem(
                    page,
                    qpixmap,
                    page->meta.LPub.page.documentLogoFront,
                    pageBg);

              page->addPageAttributePixmap(pixmapLogoFront);
              pixmapLogoFront->setTransformationMode(Qt::SmoothTransformation);
              pixmapLogoFront->setScale(picScale,picScale);

              int margin[2] = {0, 0};

              PlacementData pld = pixmapLogoFront->placement.value();
              if (pld.relativeTo == PageHeaderType) {
                  pageHeader.placeRelative(pixmapLogoFront, margin);
                  pixmapLogoFront->relativeToSize[0] = pageHeader.size[XX];
                  pixmapLogoFront->relativeToSize[1] = pageHeader.size[YY];
                } else if (pld.relativeTo == PageFooterType) {
                  pageFooter.placeRelative(pixmapLogoFront, margin);
                  pixmapLogoFront->relativeToSize[0] = pageFooter.size[XX];
                  pixmapLogoFront->relativeToSize[1] = pageFooter.size[YY];
                } else {
                  plPage.placeRelative(pixmapLogoFront, margin);
                  pixmapLogoFront->relativeToSize[0] = plPage.size[XX];
                  pixmapLogoFront->relativeToSize[1] = plPage.size[YY];
                }
              pixmapLogoFront->setPos(pixmapLogoFront->loc[XX],pixmapLogoFront->loc[YY]);
            }
        }

      // allocate QGraphicsPixmapItem for // coverImage (Front Cover) //~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.coverImage.display.value()) {

          QFileInfo fileInfo;
          QString file = page->meta.LPub.page.coverImage.file.value();
          qreal picScale   = page->meta.LPub.page.coverImage.picScale.value();
          fileInfo.setFile(file);

          if (fileInfo.exists()) {

              QPixmap qpixmap;
              qpixmap.load(file);
              pixmapCoverImageFront
                  = new PageAttributePixmapItem(
                    page,
                    qpixmap,
                    page->meta.LPub.page.coverImage,
                    pageBg);

              page->addPageAttributePixmap(pixmapCoverImageFront);
              pixmapCoverImageFront->setTransformationMode(Qt::SmoothTransformation);
              pixmapCoverImageFront->setScale(picScale,picScale);

              int margin[2] = {0, 0};

              plPage.placeRelative(pixmapCoverImageFront, margin);

              pixmapCoverImageFront->relativeToSize[0] = plPage.size[XX];
              pixmapCoverImageFront->relativeToSize[1] = plPage.size[YY];
              pixmapCoverImageFront->setPos(pixmapCoverImageFront->loc[XX],pixmapCoverImageFront->loc[YY]);

            }
        }
    }

  if (page->coverPage && page->backCover){

      // reltaitve to checks
      bool enableTitleBack = page->meta.LPub.page.titleBack.display.value();
      bool breakTitleBackRelativeTo = false;
      PlacementData titleBackPld;
      if (enableTitleBack) {

          titleBack               = new PageAttributeTextItem(page,page->meta.LPub.page.titleBack,pageBg);
          titleBack->relativeType = PageTitleType;
          titleBack->size[XX]     = (int) titleBack->document()->size().width();
          titleBack->size[YY]     = (int) titleBack->document()->size().height();

          titleBackPld = titleBack->placement.value();
          breakTitleBackRelativeTo = titleBackPld.relativeTo != PageType;
      }

      bool enableAuthorBack = page->meta.LPub.page.authorBack.display.value();
      bool breakAuthorBackRelativeTo = false;
      PlacementData authorBackPld;
      if (enableAuthorBack) {

          authorBack               = new PageAttributeTextItem(page,page->meta.LPub.page.authorBack,pageBg);
          authorBack->relativeType = PageAuthorType;
          authorBack->size[XX]     = (int) authorBack->document()->size().width();
          authorBack->size[YY]     = (int) authorBack->document()->size().height();

          authorBackPld = authorBack->placement.value();
          breakAuthorBackRelativeTo = authorBackPld.relativeTo != PageTitleType;
      }

      bool enableCopyrightBack = page->meta.LPub.page.modelDesc.display.value();
      bool breakCopyrightBackRelativeTo = false;
      PlacementData copyrightBackPld;
      if (enableCopyrightBack) {

          copyrightBack               = new PageAttributeTextItem(page,page->meta.LPub.page.copyrightBack,pageBg);
          copyrightBack->relativeType = PageCopyrightType;
          copyrightBack->size[XX]     = (int) copyrightBack->document()->size().width();
          copyrightBack->size[YY]     = (int) copyrightBack->document()->size().height();

          copyrightBackPld = copyrightBack->placement.value();
          breakCopyrightBackRelativeTo = copyrightBackPld.relativeTo != PageAuthorType;
      }

      bool enableurlBack = page->meta.LPub.page.pieces.display.value();
      bool breakURLBackRelativeTo = false;
      PlacementData urlBackPld;
      if (enableurlBack) {

          urlBack               = new PageAttributeTextItem(page,page->meta.LPub.page.urlBack,pageBg);
          urlBack->relativeType = PageURLType;
          urlBack->size[XX]     = (int) urlBack->document()->size().width();
          urlBack->size[YY]     = (int) urlBack->document()->size().height();

          urlBackPld = urlBack->placement.value();
          breakURLBackRelativeTo = urlBackPld.relativeTo != PageCopyrightType;
      }

      bool enableEmailBack = page->meta.LPub.page.pieces.display.value();
      bool breakEmailBackRelativeTo = false;
      PlacementData emailBackPld;
      if (enableEmailBack) {

          emailBack               = new PageAttributeTextItem(page,page->meta.LPub.page.emailBack,pageBg);
          emailBack->relativeType = PageEmailType;
          emailBack->size[XX]     = (int) emailBack->document()->size().width();
          emailBack->size[YY]     = (int) emailBack->document()->size().height();

          emailBackPld = emailBack->placement.value();
          breakEmailBackRelativeTo = emailBackPld.relativeTo != PageURLType;
      }

      bool enableDisclaimerBack = page->meta.LPub.page.modelDesc.display.value();
      bool breakDisclaimerBackRelativeTo = false;
      PlacementData disclaimerBackPld;
      if (enableDisclaimerBack) {

          disclaimerBack               = new PageAttributeTextItem(page,page->meta.LPub.page.disclaimer,pageBg);
          disclaimerBack->relativeType = PageDisclaimerType;
          disclaimerBack->size[XX]     = (int) disclaimerBack->document()->size().width();
          disclaimerBack->size[YY]     = (int) disclaimerBack->document()->size().height();

          disclaimerBackPld = disclaimerBack->placement.value();
          breakDisclaimerBackRelativeTo = disclaimerBackPld.relativeTo != PageEmailType;
      }

      bool enablePlugBack = page->meta.LPub.page.plug.display.value();
      bool breakPlugBackRelativeTo = false;
      PlacementData plugBackPld;
      if (enablePlugBack) {

          plugBack               = new PageAttributeTextItem(page,page->meta.LPub.page.plug,pageBg);
          plugBack->relativeType = PagePlugType;
          plugBack->size[XX]     = (int) plugBack->document()->size().width();
          plugBack->size[YY]     = (int) plugBack->document()->size().height();

          plugBackPld = plugBack->placement.value();
          breakPlugBackRelativeTo = plugBackPld.relativeTo != PageDisclaimerType;
      }

      // allocate QGraphicsTextItem for // page Header (Back Cover) //~~~~~~~~~~~~~~~~
      pageHeader.relativeType   = PageHeaderType;
      pageHeader.placement      = page->meta.LPub.page.pageHeader.placement;
      pageHeader.size[XX]       = page->meta.LPub.page.pageHeader.size.valuePixels(XX);
      pageHeader.size[YY]       = page->meta.LPub.page.pageHeader.size.valuePixels(YY);
      if (pageHeader.placement.value().relativeTo == plPage.relativeType) {
          plPage.placeRelative(&pageHeader);
          plPage.appendRelativeTo(&pageHeader);
        }
      pageHeader.setPos(pageHeader.loc[XX],pageHeader.loc[YY]);

      // allocate QGraphicsTextItem for // page Footer (Back Cover) //~~~~~~~~~~~~~~~~
      pageFooter.relativeType   = PageFooterType;
      pageFooter.placement      = page->meta.LPub.page.pageFooter.placement;
      pageFooter.size[XX]       = page->meta.LPub.page.pageFooter.size.valuePixels(XX);
      pageFooter.size[YY]       = page->meta.LPub.page.pageFooter.size.valuePixels(YY);
      if (pageFooter.placement.value().relativeTo == plPage.relativeType) {
          plPage.placeRelative(&pageFooter);
          plPage.appendRelativeTo(&pageFooter);
        }
      pageFooter.setPos(pageFooter.loc[XX],pageFooter.loc[YY]);

      // allocate QGraphicsTextItem for // title (Back Cover) //~~~~~~~~~~~~~~~~
      if (enableTitleBack) {

          plPage.appendRelativeTo(titleBack);
          plPage.placeRelative(titleBack);

          titleBack->setPos(titleBack->loc[XX],titleBack->loc[YY]);
        }

      // allocate QGraphicsTextItem for // author (Back) //~~~~~~~~~~~~~~~~
      if (enableAuthorBack) {

          authorBackPld = authorBack->placement.value();
          if (breakTitleBackRelativeTo && authorBackPld.relativeTo == PageTitleType) {
              authorBack->placement.setValue(CenterCenter,PageType);
              plPage.appendRelativeTo(authorBack);
              plPage.placeRelative(authorBack);
            } else if (authorBackPld.relativeTo == PageTitleType &&
                       page->meta.LPub.page.titleBack.display.value()) {
              titleBack->appendRelativeTo(authorBack);
              titleBack->placeRelative(authorBack);
            } else {
              plPage.appendRelativeTo(authorBack);
              plPage.placeRelative(authorBack);
            }
          authorBack->setPos(authorBack->loc[XX],authorBack->loc[YY]);
        }

      // allocate QGraphicsTextItem for // copyright (Back) //~~~~~~~~~~~~~~~~
      if (enableCopyrightBack) {

          copyrightBackPld = copyrightBack->placement.value();
          if (breakAuthorBackRelativeTo && copyrightBackPld.relativeTo == PageAuthorType) {
              copyrightBack->placement.setValue(LeftInside,PageType);
              plPage.appendRelativeTo(copyrightBack);
              plPage.placeRelative(copyrightBack);
            } else if (copyrightBackPld.relativeTo == PageAuthorType &&
                       page->meta.LPub.page.authorBack.display.value()) {
              authorBack->appendRelativeTo(copyrightBack);
              authorBack->placeRelative(copyrightBack);
            } else {
              plPage.appendRelativeTo(copyrightBack);
              plPage.placeRelative(copyrightBack);
            }
          copyrightBack->setPos(copyrightBack->loc[XX],copyrightBack->loc[YY]);
        }

      // allocate QGraphicsTextItem for // url (Back) //~~~~~~~~~~~~~~~~
      if (enableurlBack) {

          urlBackPld = urlBack->placement.value();
          if (breakCopyrightBackRelativeTo && urlBackPld.relativeTo == PageCopyrightType) {
              urlBack->placement.setValue(LeftInside,PageType);
              plPage.appendRelativeTo(urlBack);
              plPage.placeRelative(urlBack);
            } else if (urlBackPld.relativeTo == PageCopyrightType &&
                       page->meta.LPub.page.copyrightBack.display.value()) {
              copyrightBack->appendRelativeTo(urlBack);
              copyrightBack->placeRelative(urlBack);
            } else {
              plPage.appendRelativeTo(urlBack);
              plPage.placeRelative(urlBack);
            }
          urlBack->setPos(urlBack->loc[XX],urlBack->loc[YY]);
        }

      // allocate QGraphicsTextItem for // emailBack (Back) //~~~~~~~~~~~~~~~~
      if (enableEmailBack) {

          emailBackPld = emailBack->placement.value();
          if (breakURLBackRelativeTo && emailBackPld.relativeTo == PageURLType) {
              emailBack->placement.setValue(RightInside,PageType);
              plPage.appendRelativeTo(emailBack);
              plPage.placeRelative(emailBack);
            } else if (emailBackPld.relativeTo == PageURLType &&
                       page->meta.LPub.page.urlBack.display.value()) {
              urlBack->appendRelativeTo(emailBack);
              urlBack->placeRelative(emailBack);
            } else {
              plPage.appendRelativeTo(emailBack);
              plPage.placeRelative(emailBack);
            }
          emailBack->setPos(emailBack->loc[XX],emailBack->loc[YY]);
        }

      // allocate QGraphicsTextItem for // disclaimer (Back Cover)//~~~~~~~~~~~~~~~~
      if (enableDisclaimerBack) {

          disclaimerBackPld = disclaimerBack->placement.value();
          if (breakEmailBackRelativeTo && disclaimerBackPld.relativeTo == PageEmailType) {
              disclaimerBack->placement.setValue(RightInside,PageType);
              plPage.appendRelativeTo(disclaimerBack);
              plPage.placeRelative(disclaimerBack);
            } else if (disclaimerBackPld.relativeTo == PageEmailType &&
                       page->meta.LPub.page.emailBack.display.value()) {
              emailBack->appendRelativeTo(disclaimerBack);
              emailBack->placeRelative(disclaimerBack);
            } else {
              plPage.appendRelativeTo(disclaimerBack);
              plPage.placeRelative(disclaimerBack);
            }
          disclaimerBack->setPos(disclaimerBack->loc[XX],disclaimerBack->loc[YY]);
        }

      // allocate QGraphicsTextItem for // plug (Back Cover)//~~~~~~~~~~~~~~~~
      if (enablePlugBack) {

          PlacementData plugBackPld = plugBack->placement.value();
          if (breakDisclaimerBackRelativeTo && plugBackPld.relativeTo == PageDisclaimerType) {
              plugBack->placement.setValue(RightInside,PageType);
              plPage.appendRelativeTo(plugBack);
              plPage.placeRelative(plugBack);
            } else if (plugBackPld.relativeTo == PageDisclaimerType &&
                       page->meta.LPub.page.disclaimer.display.value()) {
              disclaimerBack->appendRelativeTo(plugBack);
              disclaimerBack->placeRelative(plugBack);
            } else {
              plPage.appendRelativeTo(plugBack);
              plPage.placeRelative(plugBack);
            }
          plugBack->setPos(plugBack->loc[XX],plugBack->loc[YY]);
        }
      // allocate QGraphicsPixmapItem for // documentLogoBack (Back Cover) //~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.documentLogoBack.display.value()) {

          QFileInfo fileInfo;
          QString file = page->meta.LPub.page.documentLogoBack.file.value();
          qreal picScale   = page->meta.LPub.page.documentLogoBack.picScale.value();
          fileInfo.setFile(file);
          if (fileInfo.exists()) {

              QPixmap qpixmap;
              qpixmap.load(file);
              pixmapLogoBack =
                  new PageAttributePixmapItem(
                    page,
                    qpixmap,
                    page->meta.LPub.page.documentLogoBack,
                    pageBg);

              page->addPageAttributePixmap(pixmapLogoBack);
              pixmapLogoBack->setTransformationMode(Qt::SmoothTransformation);
              pixmapLogoBack->setScale(picScale,picScale);

              int margin[2] = {0, 0};

              PlacementData pld = pixmapLogoBack->placement.value();
              if (pld.relativeTo == PageHeaderType) {
                  pageHeader.placeRelative(pixmapLogoBack, margin);
                  pixmapLogoBack->relativeToSize[0] = pageHeader.size[XX];
                  pixmapLogoBack->relativeToSize[1] = pageHeader.size[YY];
                } else if (pld.relativeTo == PageFooterType) {
                  pageFooter.placeRelative(pixmapLogoBack, margin);
                  pixmapLogoBack->relativeToSize[0] = pageFooter.size[XX];
                  pixmapLogoBack->relativeToSize[1] = pageFooter.size[YY];
                } else {
                  plPage.placeRelative(pixmapLogoBack, margin);
                  pixmapLogoBack->relativeToSize[0] = plPage.size[XX];
                  pixmapLogoBack->relativeToSize[1] = plPage.size[YY];
                }
              pixmapLogoBack->setPos(pixmapLogoBack->loc[XX],pixmapLogoBack->loc[YY]);
            }
        }
      // allocate QGraphicsPixmapItem for // plugImage (Back Cover) //~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.plugImage.display.value()) {

          QFileInfo fileInfo;
          QString file     = page->meta.LPub.page.plugImage.file.value();
          qreal picScale   = page->meta.LPub.page.plugImage.picScale.value();
          fileInfo.setFile(file);
          if (fileInfo.exists()) {

              QPixmap qpixmap;
              qpixmap.load(file);
              pixmapPlugImageBack =
                  new PageAttributePixmapItem(
                    page,
                    qpixmap,
                    page->meta.LPub.page.plugImage,
                    pageBg);

              page->addPageAttributePixmap(pixmapPlugImageBack);;
              pixmapPlugImageBack->setTransformationMode(Qt::SmoothTransformation);
              pixmapPlugImageBack->setScale(picScale,picScale);

              int margin[2] = {0, 0};

              PlacementData pld = pixmapPlugImageBack->placement.value();
              if (breakPlugBackRelativeTo && pld.relativeTo == PagePlugType) {
                  plPage.placeRelative(pixmapPlugImageBack, margin);
                } else if (pld.relativeTo == PagePlugType) {
                  plugBack->placeRelative(pixmapPlugImageBack, margin);
                } else {
                  plPage.placeRelative(pixmapLogoBack, margin);
                }
              pixmapPlugImageBack->setPos(pixmapPlugImageBack->loc[XX],pixmapPlugImageBack->loc[YY]);
              pixmapPlugImageBack->relativeToSize[0] = plPage.size[XX];
              pixmapPlugImageBack->relativeToSize[1] = plPage.size[YY];
            }
        }
    }

  // If the page contains a single step then process it here
  if (page->relativeType == SingleStepType && page->list.size()) {
      if (page->list.size()) {
          Range *range = dynamic_cast<Range *>(page->list[0]);
          if (range->relativeType == RangeType) {
              Step *step = dynamic_cast<Step *>(range->list[0]);
              if (step && step->relativeType == StepType) {

                  // populate page pixmaps - of using LDView Single Call

                  if (renderer->useLDViewSCall()){
                      step->csiPixmap.load(step->pngName);
                      step->csiPlacement.size[0] = step->csiPixmap.width();
                      step->csiPlacement.size[1] = step->csiPixmap.height();
                      for (int k = 0; k < step->list.size(); k++) {
                          if (step->list[k]->relativeType == CalloutType) {
                              Callout *callout = dynamic_cast<Callout *>(step->list[k]);
                              if (callout) {
                                  for (int l = 0; l < callout->list.size(); l++){
                                      Range *range = dynamic_cast<Range *>(callout->list[l]);
                                      for (int m = 0; m < range->list.size(); m++){
                                          if (range->relativeType == RangeType) {
                                              Step *step = dynamic_cast<Step *>(range->list[m]);
                                              if (step){
                                                  step->csiPixmap.load(step->pngName);
                                                  step->csiPlacement.size[0] = step->csiPixmap.width();
                                                  step->csiPlacement.size[1] = step->csiPixmap.height();
                                                } // validate step (StepType) and process...
                                            } // validate RangeType - to cast step
                                        } // for each step within divided group...=>list[AbstractRangeElement]->StepType
                                    } // for each divided group within callout...=>list[AbstractStepsElement]->RangeType
                                } // validate callout
                            } // validate calloutType
                        } // for divided group within step...=>list[Steps]->CalloutType
                    }

                  // add the step number

                  if ( ! step->onlyChild() && step->showStepNumber) {
                      step->stepNumber.sizeit();
                    }

                  /* Size the callouts */
                  for (int i = 0; i < step->list.size(); i++) {
                      step->list[i]->sizeIt();
                    }

                  // add the assembly image to the scene

                  step->csiItem = new CsiItem(
                        step,
                        &page->meta,
                        step->csiPixmap,
                        step->submodelLevel,
                        pageBg,
                        page->relativeType);

                  if (step->csiItem == NULL) {
                      exit(-1);
                    }
                  step->csiItem->assign(&step->csiPlacement);
                  step->csiItem->boundingSize[XX] = step->csiItem->size[XX];
                  step->csiItem->boundingSize[YY] = step->csiItem->size[YY];

                  // Place the step relative to the page.

                  plPage.relativeTo(step);      // place everything

                  // center the csi's bounding box relative to the page

                  plPage.placeRelativeBounding(step->csiItem);

                  // place callouts relative to the csi bounding box

                  for (int i = 0; i < step->list.size(); i++) {

                      if (step->list[i]->relativeType == CalloutType) {
                          Callout *callout = dynamic_cast<Callout *>(step->list[i]);

                          PlacementData placementData = callout->placement.value();

                          if (placementData.relativeTo == CsiType) {
                              step->csiItem->placeRelativeBounding(callout);
                            }
                        } // if callout
                    } // callouts


                  if (step->pli.placement.value().relativeTo == CsiType) {
                      step->csiItem->placeRelative(&step->pli);
                    }

                  // place the CSI relative to the entire step's box
                  step->csiItem->setPos(step->csiItem->loc[XX],
                                        step->csiItem->loc[YY]);



                  // add the PLI graphically to the scene

                  step->pli.addPli(step->submodelLevel, pageBg);

                  // place the PLI relative to the entire step's box
                  step->pli.setPos(step->pli.loc[XX],
                                   step->pli.loc[YY]);

                  // allocate QGraphicsPixmapItem for rotate icon

                  if (step->placeRotateIcon && page->meta.LPub.rotateIcon.display.value()) {

                      step->rotateIcon.sizeit();
                      RotateIconItem *rotateIcon =
                          new RotateIconItem(
                            step,
                            page->relativeType,
                            page->meta.LPub.rotateIcon,
                            pageBg);
                      rotateIcon->setPos(step->rotateIcon.loc[XX],
                                         step->rotateIcon.loc[YY]);
                      rotateIcon->relativeToSize[0] = step->rotateIcon.relativeToSize[0];
                      rotateIcon->relativeToSize[1] = step->rotateIcon.relativeToSize[1];
                      rotateIcon->setFlag(QGraphicsItem::ItemIsMovable,true);
                    }

                  // allocate QGraphicsTextItem for step number

                  if ( ! step->onlyChild() && ! step->modelDisplayStep) {
                      StepNumberItem *stepNumber =
                          new StepNumberItem(step,
                                             page->relativeType,
                                             page->meta.LPub.stepNumber,
                                             "%d",
                                             step->stepNumber.number,
                                             pageBg);
                      stepNumber->setPos(step->stepNumber.loc[XX],
                                         step->stepNumber.loc[YY]);
                      stepNumber->relativeToSize[0] = step->stepNumber.relativeToSize[0];
                      stepNumber->relativeToSize[1] = step->stepNumber.relativeToSize[1];

                    } else if (step->pli.background) {
                      step->pli.background->setFlag(QGraphicsItem::ItemIsMovable,false);
                    }

                  // foreach callout

                  for (int i = 0; i < step->list.size(); i++) {


                      Callout *callout = step->list[i];
                      QRect    csiRect(step->csiItem->loc[XX]-callout->loc[XX],
                                       step->csiItem->loc[YY]-callout->loc[YY],
                                       step->csiItem->size[XX],
                                       step->csiItem->size[YY]);

                      // add the callout's graphics items to the scene

                      callout->addGraphicsItems(0,0,csiRect,pageBg,true);

                      // foreach pointer
                      //   add the pointer to the graphics scene

                      for (int i = 0; i < callout->pointerList.size(); i++) {
                          Pointer *pointer = callout->pointerList[i];
                          callout->addGraphicsPointerItem(pointer,callout->underpinnings);
                        }
                    }

                  // Place the Bill of Materials on the page along with single step

                  if (page->pli.tsize()) {
                      if (page->pli.bom) {
                          page->pli.addPli(0,pageBg);
                          page->pli.setFlag(QGraphicsItem::ItemIsSelectable,true);
                          page->pli.setFlag(QGraphicsItem::ItemIsMovable,true);

                          PlacementData pld;

                          pld = page->pli.pliMeta.placement.value();

                          page->pli.placement.setValue(pld);
                          if (pld.relativeTo == PageType) {
                              plPage.placeRelative(page->pli.background);
                            } else {
                              step->csiItem->placeRelative(page->pli.background);
                            }
                          page->pli.loc[XX] = page->pli.background->loc[XX];
                          page->pli.loc[YY] = page->pli.background->loc[YY];

                          page->pli.setPos(page->pli.loc[XX],page->pli.loc[YY]);
                        }
                    }
                }
            }
        }

    } else {

      // qDebug() << "List relative type: " << RelNames[range->relativeType];
      // We've got a page that contains step groups, so add it

      // LDView generate multistep pixamps
      if (renderer->useLDViewSCall() &&
              page->list.size()) {
          // Load images and set size
          for (int i = 0; i < page->list.size(); i++){
              Range *range = dynamic_cast<Range *>(page->list[i]);
              for (int j = 0; j < range->list.size(); j++){
                  if (range->relativeType == RangeType) {
                      Step *step = dynamic_cast<Step *>(range->list[j]);
                      if (step && step->relativeType == StepType){
                          step->csiPixmap.load(step->pngName);
                          step->csiPlacement.size[0] = step->csiPixmap.width();
                          step->csiPlacement.size[1] = step->csiPixmap.height();
                          for (int k = 0; k < step->list.size(); k++) {
                              if (step->list[k]->relativeType == CalloutType) {
                                  Callout *callout = dynamic_cast<Callout *>(step->list[k]);
                                  if (callout) {
                                      for (int l = 0; l < callout->list.size(); l++){
                                          Range *range = dynamic_cast<Range *>(callout->list[l]);
                                          for (int m = 0; m < range->list.size(); m++){
                                              if (range->relativeType == RangeType) {
                                                  Step *step = dynamic_cast<Step *>(range->list[m]);
                                                  if (step){
                                                      step->csiPixmap.load(step->pngName);
                                                      step->csiPlacement.size[0] = step->csiPixmap.width();
                                                      step->csiPlacement.size[1] = step->csiPixmap.height();
                                                  } // validate step (StepType) and process...
                                              } // validate RangeType - to cast step
                                          } // for each step within divided group...=>list[AbstractRangeElement]->StepType
                                      } // for each divided group within callout...=>list[AbstractStepsElement]->RangeType
                                  } // validate callout
                              } // validate calloutType
                          } // for divided group within step...=>list[Steps]->CalloutType
                      } // validate step (StepType) and process...
                  } // validate RangeType - to cast step
              } // for each step within divided group...=>list[AbstractRangeElement]->StepType
          } // for each divided group within page...=>list[AbstractStepsElement]->RangeType
      }

      PlacementData data = page->meta.LPub.multiStep.placement.value();
      page->placement.setValue(data);

      // place all the steps in the group relative to each other, including
      // any callouts placed relative to steps

      page->sizeIt();             // size multi-step

      plPage.relativeToSg(page);  // place callouts relative to PAGE
      plPage.placeRelative(page); // place multi-step relative to the page

      page->relativeToSg(page);   // compute bounding box of step group and callouts
      // placed relative to it.

      plPage.placeRelativeBounding(page); // center multi-step in page's bounding box

      page->relativeToSg(page);           // place callouts relative to MULTI_STEP

      page->addGraphicsItems(0,0,pageBg);

      // Place the Bill of materials on the page along with step group?????

      if (page->pli.tsize()) {
          if (page->pli.bom) {
              page->pli.addPli(0,pageBg);
              page->pli.setFlag(QGraphicsItem::ItemIsSelectable,true);
              page->pli.setFlag(QGraphicsItem::ItemIsMovable,true);

              PlacementData pld;
              pld = page->pli.pliMeta.placement.value();
              page->pli.placement.setValue(pld);
              if (pld.relativeTo == PageType) {
                  plPage.placeRelative(page->pli.background);
                } else {
                  page->placeRelative(page->pli.background);
                }
              page->pli.loc[XX] = page->pli.background->loc[XX];
              page->pli.loc[YY] = page->pli.background->loc[YY];
            }
          page->pli.setPos(page->pli.loc[XX],page->pli.loc[YY]);
        }
    }

  if ( ! printing) {

      QGraphicsDropShadowEffect *bodyShadow = new QGraphicsDropShadowEffect;
      bodyShadow->setBlurRadius(9.0);
      bodyShadow->setColor(QColor(0, 0, 0, 160));
      bodyShadow->setOffset(4.0);

      if (page->meta.LPub.page.background.value().type != BackgroundData::BgTransparent)
        pageBg->setGraphicsEffect(bodyShadow);

      view->horizontalScrollBar()->setRange(0,int(pageSize(page->meta.LPub, 0)));
      view->verticalScrollBar()->setRange(  0,int(pageSize(page->meta.LPub, 1)));

    }

  scene->addItem(pageBg);

  view->setSceneRect(pageBg->sceneBoundingRect());


  if (printing) {
      view->fitInView(0,0,pW,pH);
    } else if (fitMode == FitWidth) {
      fitWidth(view);
    } else if (fitMode == FitVisible) {
      fitVisible(view);
    }

  page->relativeType = SingleStepType;
  statusBarMsg("");
  return 0;
}
