/**************************************************************************** 
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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
#include <QColor>
#include "callout.h"
#include "lpub.h"
#include "ranges.h"
#include "range.h"
#include "step.h"
#include "meta.h"
#include "color.h"
#include "pagebackgrounditem.h"
#include "numberitem.h"
#include "pageattributeitem.h"
#include "csiitem.h"
#include "calloutbackgrounditem.h"
#include "textitem.h"

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
      QString toolTip("Times used - popup menu");
      setAttributes(PageNumberType,
                    SingleStepType,
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

  PlacementData placementData = placement.value();

  QAction *fontAction       = menu.addAction("Change Submodel Count Font");
  QAction *colorAction      = menu.addAction("Change Submodel Count Color");
  QAction *marginAction     = menu.addAction("Change Submodel Count Margins");
  QAction *placementAction  = menu.addAction("Move this number");

  fontAction->setWhatsThis("You can change the font or the size of the page number");
  colorAction->setWhatsThis("You can change the color of the page number");
  marginAction->setWhatsThis("You can change how much empty space their is around the page number");
  placementAction->setWhatsThis("You can move this submodel count around");

  QAction *selectedAction   = menu.exec(event->screenPos());

  if (selectedAction == fontAction) {

    changeFont(page->topOfSteps(),page->bottomOfSteps(),&font);

  } else if (selectedAction == colorAction) {

    changeColor(page->topOfSteps(),page->bottomOfSteps(),&color);

  } else if (selectedAction == marginAction) {

    changeMargins("Submodel Count Margins",
                  page->topOfSteps(),page->bottomOfSteps(),
                  &margin);
  } else if (selectedAction == placementAction) {
    changePlacement(PageType,
                    SubmodelInstanceCountType,
                    "Submodel Count Placement",
                    page->topOfSteps(),
                    page->bottomOfSteps(),
                    &placement);
  }
}

void SubmodelInstanceCount::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
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

      changePlacementOffset(page->topOfSteps(),&placement,PageType);
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
  int             instances,
  LGraphicsView  *view,
  QGraphicsScene *scene,
  bool            printing)
{
  Page *page = dynamic_cast<Page *>(steps);

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
    if (view->maximumWidth() < page->meta.LPub.page.size.valuePixels(0)) {
      pW = view->maximumWidth();
    } else {
      pW = page->meta.LPub.page.size.valuePixels(0);
    }
    if (view->maximumHeight() < page->meta.LPub.page.size.valuePixels(1)) {
      pH = view->maximumHeight();
    } else {
      pH = page->meta.LPub.page.size.valuePixels(1);
    }
  } else {
    pW = page->meta.LPub.page.size.valuePixels(0);
    pH = page->meta.LPub.page.size.valuePixels(1);
  }

   // pW = page->meta.LPub.page.size.valuePixels(0);
   // pH = page->meta.LPub.page.size.valuePixels(1);


  PageBackgroundItem *pageBg;
  pageBg = new PageBackgroundItem(page, pW, pH);

  view->pageBackgroundItem = pageBg;
  pageBg->setPos(0,0);

  // Set up the placement aspects of the page in the Qt space

  Placement plPage;
  plPage.relativeType = PageType;

  plPage.setSize(pW,pH);
  plPage.margin  = page->meta.LPub.page.margin;
  plPage.loc[XX] = 0;
  plPage.loc[YY] = 0;

  // Display a page number?

  if (page->meta.LPub.page.dpn.value() && ! coverPage) {

    // allocate QGraphicsTextItem for page number

    PageNumberItem  *pageNumber = 
      new PageNumberItem(
                      page,
                      page->meta.LPub.page.number, 
                     "%d", 
                     stepPageNum,
                     pageBg);
    
    pageNumber->relativeType = PageNumberType;
    pageNumber->size[XX]     = (int) pageNumber->document()->size().width();
    pageNumber->size[YY]     = (int) pageNumber->document()->size().height();
    
    PlacementData placementData = pageNumber->placement.value();
    
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

    // allocate QGraphicsTextItem for // author (Header) //~~~~~~~~~~~~~~~~
    if (page->meta.LPub.page.author.display.value() && ! coverPage) {

        PagePageAttributeItem *author =
                new PagePageAttributeItem(
                    page,
                    page->meta.LPub.page.author,
                    pageBg);

        author->relativeType = PageCopyrightType;
        author->size[XX]     = (int) author->document()->size().width();
        author->size[YY]     = (int) author->document()->size().height();

        plPage.appendRelativeTo(author);
        plPage.placeRelative(author);
        author->setPos(author->loc[XX],author->loc[YY]);
    }

    // allocate QGraphicsTextItem for // url (Header) //~~~~~~~~~~~~~~~~
    if (page->meta.LPub.page.url.display.value() && ! coverPage) {

        PagePageAttributeItem *url =
                new PagePageAttributeItem(
                    page,
                    page->meta.LPub.page.url,
                    pageBg);

        url->relativeType = PageCopyrightType;
        url->size[XX]     = (int) url->document()->size().width();
        url->size[YY]     = (int) url->document()->size().height();

        plPage.appendRelativeTo(url);
        plPage.placeRelative(url);
        url->setPos(url->loc[XX],url->loc[YY]);
    }

    // allocate QGraphicsTextItem for // email (Footer) //~~~~~~~~~~~~~~~~
    if (page->meta.LPub.page.email.display.value() && ! coverPage) {

        PagePageAttributeItem *email =
                new PagePageAttributeItem(
                    page,
                    page->meta.LPub.page.email,
                    pageBg);

        email->relativeType = PageCopyrightType;
        email->size[XX]     = (int) email->document()->size().width();
        email->size[YY]     = (int) email->document()->size().height();

        plPage.appendRelativeTo(email);
        plPage.placeRelative(email);
        email->setPos(email->loc[XX],email->loc[YY]);
    }

    // allocate QGraphicsTextItem for // copyright (Footer) //~~~~~~~~~~~~~~~~
    if (page->meta.LPub.page.copyright.display.value() && ! coverPage) {

        PagePageAttributeItem *copyright =
                new PagePageAttributeItem(
                    page,
                    page->meta.LPub.page.copyright,
                    pageBg);

        copyright->relativeType = PageCopyrightType;
        copyright->size[XX]     = (int) copyright->document()->size().width();
        copyright->size[YY]     = (int) copyright->document()->size().height();

        plPage.appendRelativeTo(copyright);
        plPage.placeRelative(copyright);
        copyright->setPos(copyright->loc[XX],copyright->loc[YY]);
    }

    // if this page contains the last step of the page, 
    // and instance is > 1 then display instance

    // allocate QGraphicsTextItem for instance number 
    
    SubmodelInstanceCount *instanceCount;
    
    if (endOfSubmodel && instances > 1) {
      instanceCount = new SubmodelInstanceCount(
        page,
        page->meta.LPub.page.instanceCount,
        "x%d ",
        instances,
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
                        
        PlacementData placementData = instanceCount->placement.value();
        
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
              pixmap->scale(insert.picScale,insert.picScale);
              
              PlacementData pld;
              
              pld.placement    = TopLeft;
              pld.justification    = Center;
              pld.relativeTo      = PageType;
              pld.preposition   = Inside;
              pld.offsets[0]    = insert.offsets[0];
              pld.offsets[1]    = insert.offsets[1];
              
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
            getBOMOccurrence(current);                                      //divide BOM Parts
            if (boms > 1){
                logTrace() << "BOMS: " << boms;
                QStringList dividedBOMParts;
                divideBOMParts(bomParts,dividedBOMParts);
                page->pli.setParts(dividedBOMParts,page->meta,true);
                dividedBOMParts.clear();
            } else {
                page->pli.setParts(bomParts,page->meta,true);
            }
            bomParts.clear();
            page->pli.sizePli(&page->meta,page->relativeType,false);
            page->pli.relativeToSize[0] = plPage.size[XX];
            page->pli.relativeToSize[1] = plPage.size[YY];
          }
        break;
      }
    }
  }

  // Process Front and Back Cover Pages - if selected
  if (page->frontCover) {

      // allocate QGraphicsTextItem for // title (Front Cover) //~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.title.display.value()) {

          PagePageAttributeItem *title =
                  new PagePageAttributeItem(
                      page,
                      page->meta.LPub.page.title,
                      pageBg);

          title->relativeType = PageCopyrightType;
          title->size[XX]     = (int) title->document()->size().width();
          title->size[YY]     = (int) title->document()->size().height();

          plPage.appendRelativeTo(title);
          plPage.placeRelative(title);
          title->setPos(title->loc[XX],title->loc[YY]);
      }

      // allocate QGraphicsTextItem for // modelName (Front Cover) //~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.modelName.display.value()) {

          PagePageAttributeItem *modelName =
                  new PagePageAttributeItem(
                      page,
                      page->meta.LPub.page.modelName,
                      pageBg);

          modelName->relativeType = PageCopyrightType;
          modelName->size[XX]     = (int) modelName->document()->size().width();
          modelName->size[YY]     = (int) modelName->document()->size().height();

          plPage.appendRelativeTo(modelName);
          plPage.placeRelative(modelName);
          modelName->setPos(modelName->loc[XX],modelName->loc[YY]);
      }

      // allocate QGraphicsTextItem for // modelDesc (Front Cover) //~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.modelDesc.display.value()) {

          PagePageAttributeItem *modelDesc =
                  new PagePageAttributeItem(
                      page,
                      page->meta.LPub.page.modelDesc,
                      pageBg);

          modelDesc->relativeType = PageCopyrightType;
          modelDesc->size[XX]     = (int) modelDesc->document()->size().width();
          modelDesc->size[YY]     = (int) modelDesc->document()->size().height();

          plPage.appendRelativeTo(modelDesc);
          plPage.placeRelative(modelDesc);
          modelDesc->setPos(modelDesc->loc[XX],modelDesc->loc[YY]);
      }

      // allocate QGraphicsTextItem for // publishDesc (Front Cover) //~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.publishDesc.display.value()) {

          PagePageAttributeItem *publishDesc =
                  new PagePageAttributeItem(
                      page,
                      page->meta.LPub.page.publishDesc,
                      pageBg);

          publishDesc->relativeType = PageCopyrightType;
          publishDesc->size[XX]     = (int) publishDesc->document()->size().width();
          publishDesc->size[YY]     = (int) publishDesc->document()->size().height();

          plPage.appendRelativeTo(publishDesc);
          plPage.placeRelative(publishDesc);
          publishDesc->setPos(publishDesc->loc[XX],publishDesc->loc[YY]);
      }

      // allocate QGraphicsTextItem for // author (Front Cover) //~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.author.display.value()) {

          PagePageAttributeItem *author =
                  new PagePageAttributeItem(
                      page,
                      page->meta.LPub.page.author,
                      pageBg);

          author->relativeType = PageCopyrightType;
          author->size[XX]     = (int) author->document()->size().width();
          author->size[YY]     = (int) author->document()->size().height();

          plPage.appendRelativeTo(author);
          plPage.placeRelative(author);
          author->setPos(author->loc[XX],author->loc[YY]);
      }

      // allocate QGraphicsTextItem for // pieces (Front Cover) //~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.pieces.display.value()) {

          PagePageAttributeItem *pieces =
                  new PagePageAttributeItem(
                      page,
                      page->meta.LPub.page.pieces,
                      pageBg);

          pieces->relativeType = PageCopyrightType;
          pieces->size[XX]     = (int) pieces->document()->size().width();
          pieces->size[YY]     = (int) pieces->document()->size().height();

          plPage.appendRelativeTo(pieces);
          plPage.placeRelative(pieces);
          pieces->setPos(pieces->loc[XX],pieces->loc[YY]);
      }

      // allocate QGraphicsTextItem for // category (Front Cover) //~~~~~~~~~~~~~~~~
      /* if (page->meta.LPub.page.category.display.value()) {

          PagePageAttributeItem *category =
                  new PagePageAttributeItem(
                      page,
                      page->meta.LPub.page.category,
                      pageBg);

          category->relativeType = PageCopyrightType;
          category->size[XX]     = (int) category->document()->size().width();
          category->size[YY]     = (int) category->document()->size().height();

          plPage.appendRelativeTo(category);
          plPage.placeRelative(category);
          category->setPos(category->loc[XX],category->loc[YY]);
      } */

      // allocate QGraphicsTextItem for // documentLogo (Front Cover) //~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.documentLogo.value().display) {

          QFileInfo fileInfo;
          PageAttributePictureMeta pictureMeta = page->meta.LPub.page.documentLogo;
          fileInfo.setFile(pictureMeta.value().string);
          if (fileInfo.exists()) {

              QPixmap qpixmap;
              qpixmap.load(pictureMeta.value().string);
              PageAttributePixmapItem *pixmap = new PageAttributePixmapItem(qpixmap,pictureMeta,pageBg);

              page->addPageAttributePixmap(pixmap);
              pixmap->setTransformationMode(Qt::SmoothTransformation);
              pixmap->scale(pictureMeta.value().picScale,pictureMeta.value().picScale);
              pixmap->placement.setValue(pictureMeta.value().placement);

              int margin[2] = {0, 0};

              plPage.placeRelative(pixmap, margin);
              pixmap->setPos(pixmap->loc[XX],pixmap->loc[YY]);
              pixmap->relativeToSize[0] = plPage.size[XX];
              pixmap->relativeToSize[1] = plPage.size[YY];
          }
      }

      // allocate QGraphicsTextItem for // coverImage (Front Cover) //~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.coverImage.value().display) {

          QFileInfo fileInfo;
          PageAttributePictureMeta pictureMeta = page->meta.LPub.page.coverImage;
          fileInfo.setFile(pictureMeta.value().string);
          if (fileInfo.exists()) {

              QPixmap qpixmap;
              qpixmap.load(pictureMeta.value().string);
              PageAttributePixmapItem *pixmap = new PageAttributePixmapItem(qpixmap,pictureMeta,pageBg);

              page->addPageAttributePixmap(pixmap);
              pixmap->setTransformationMode(Qt::SmoothTransformation);
              pixmap->scale(pictureMeta.value().picScale,pictureMeta.value().picScale);
              pixmap->placement.setValue(pictureMeta.value().placement);

              int margin[2] = {0, 0};

              plPage.placeRelative(pixmap, margin);
              pixmap->setPos(pixmap->loc[XX],pixmap->loc[YY]);
              pixmap->relativeToSize[0] = plPage.size[XX];
              pixmap->relativeToSize[1] = plPage.size[YY];
          }
      }
  }

  if (page->backCover){

      // allocate QGraphicsTextItem for // url (Back Cover)//~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.url.display.value() ) {

          PagePageAttributeItem *url =
                  new PagePageAttributeItem(
                      page,
                      page->meta.LPub.page.url,
                      pageBg);

          url->relativeType = PageCopyrightType;
          url->size[XX]     = (int) url->document()->size().width();
          url->size[YY]     = (int) url->document()->size().height();

          plPage.appendRelativeTo(url);
          plPage.placeRelative(url);
          url->setPos(url->loc[XX],url->loc[YY]);
      }

      // allocate QGraphicsTextItem for // disclaimer (Back Cover)//~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.disclaimer.display.value() ) {

          PagePageAttributeItem *disclaimer =
                  new PagePageAttributeItem(
                      page,
                      page->meta.LPub.page.disclaimer,
                      pageBg);

          disclaimer->relativeType = PageCopyrightType;
          disclaimer->size[XX]     = (int) disclaimer->document()->size().width();
          disclaimer->size[YY]     = (int) disclaimer->document()->size().height();

          plPage.appendRelativeTo(disclaimer);
          plPage.placeRelative(disclaimer);
          disclaimer->setPos(disclaimer->loc[XX],disclaimer->loc[YY]);
      }

      // allocate QGraphicsTextItem for // plug (Back Cover)//~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.plug.display.value() ) {

          PagePageAttributeItem *plug =
                  new PagePageAttributeItem(
                      page,
                      page->meta.LPub.page.plug,
                      pageBg);

          plug->relativeType = PageCopyrightType;
          plug->size[XX]     = (int) plug->document()->size().width();
          plug->size[YY]     = (int) plug->document()->size().height();

          plPage.appendRelativeTo(plug);
          plPage.placeRelative(plug);
          plug->setPos(plug->loc[XX],plug->loc[YY]);
      }

      // allocate QGraphicsTextItem for // plugImage (Back Cover) //~~~~~~~~~~~~~~~~
      if (page->meta.LPub.page.plugImage.value().display) {

          QFileInfo fileInfo;
          PageAttributePictureMeta pictureMeta = page->meta.LPub.page.plugImage;
          fileInfo.setFile(pictureMeta.value().string);
          if (fileInfo.exists()) {

              QPixmap qpixmap;
              qpixmap.load(pictureMeta.value().string);
              PageAttributePixmapItem *pixmap = new PageAttributePixmapItem(qpixmap,pictureMeta,pageBg);

              page->addPageAttributePixmap(pixmap);
              pixmap->setTransformationMode(Qt::SmoothTransformation);
              pixmap->scale(pictureMeta.value().picScale,pictureMeta.value().picScale);

              pixmap->placement.setValue(pictureMeta.value().placement);

              int margin[2] = {0, 0};

              plPage.placeRelative(pixmap, margin);
              pixmap->setPos(pixmap->loc[XX],pixmap->loc[YY]);
              pixmap->relativeToSize[0] = plPage.size[XX];
              pixmap->relativeToSize[1] = plPage.size[YY];
          }
      }
  }
  // If the page contains a single step then process it here

  if (page->relativeType == SingleStepType && page->list.size()) {
    if (page->list.size()) {
      Range *range = dynamic_cast<Range *>(page->list[0]);
      if (range->relativeType == RangeType) {        
        Step *step= dynamic_cast<Step *>(range->list[0]);
        if (step && step->relativeType == StepType) {
       
          if ( ! step->onlyChild() && step->showStepNumber) {
            step->stepNumber.sizeit();
          }

          // add the PLI graphically to the scene

          step->pli.addPli(step->submodelLevel, pageBg);
          
          /* Size the callouts */   
          for (int i = 0; i < step->list.size(); i++) {
            step->list[i]->sizeIt();
          }
          
          // add the assembly image to the scene

          step->csiItem = new CsiItem(step,
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

          // place the PLI relative to the entire step's box
          step->pli.setPos(step->pli.loc[XX],
                           step->pli.loc[YY]);

          // allocate QGraphicsTextItem for step number
      
          if ( ! step->onlyChild()) {
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

          // Place the Bill of materials on the page along with single step

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

    // We've got a page that contains step groups, so add it

    PlacementData data = page->meta.LPub.multiStep.placement.value();
    page->placement.setValue(data);

    // place all the steps in the group relative to each other, including
    // any callouts placed relative to steps

    page->sizeIt();             // multi-step

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

  scene->addItem(pageBg);
  
  view->setSceneRect(pageBg->sceneBoundingRect());

  if ( ! printing) {
    view->horizontalScrollBar()->setRange(0,int(page->meta.LPub.page.size.valuePixels(0)));
    view->verticalScrollBar()->setRange(0,int(page->meta.LPub.page.size.valuePixels(1)));
  }

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

int Gui::getBOMOccurrence(Where	current) {		// top of ldrawFile

    // traverse content to find the number and location of BOM pages
    // key=modelName_LineNumber, value=occurrence
    QHash<QString, int> bom_Occurrence;

    Meta meta;

    skipHeader(current);

    int numLines        = ldrawFile.size(current.modelName);
    int occurrenceNum   = 0;
    boms                = 0;
    bomOccurrence       = 0;

    Rc rc;

    for ( ;
          current.lineNumber < numLines;
          current.lineNumber++) {

        QString line = ldrawFile.readLine(current.modelName,current.lineNumber).trimmed();
        switch (line.toAscii()[0]) {
        case '1':
          {
            QStringList token;
            split(line,token);
            QString type = token[token.size()-1];

            if (ldrawFile.isSubmodel(type)) {
                Where current2(type,0);
                getBOMOccurrence(current2);
            }
            break;
          }
        case '0':
          {
            rc = meta.parse(line,current);
            switch (rc) {
            case InsertRc:
              {
                InsertData insertData = meta.LPub.insert.value();
                if (insertData.type == InsertData::InsertBom){

                    QString uniqueID = QString("%1_%2").arg(current.modelName).arg(current.lineNumber);
                    occurrenceNum++;
                    bom_Occurrence[uniqueID] = occurrenceNum;

                    logTrace()  << "BOM Occurrence: " << bom_Occurrence[uniqueID]
                                << " in Model: "      << current.modelName
                                << " on Line: "       << current.lineNumber
                                << " Number of BOMs: "<< bom_Occurrence.size()
                                << " Key: "           << uniqueID
                                   ;
                }
              }
                break;
            default:
                break;
            } // switch metas
            break;
          }  // switch line type
        }
    } // for every line

    if (occurrenceNum > 1) {
        // now set the bom occurrance based on our current position
        Where here = gui->topOfPages[gui->displayPageNum-1];
        for (++here; here.lineNumber < ldrawFile.size(here.modelName); here++) {
            QString line = gui->readLine(here);
            Meta meta;
            Rc rc;

            rc = meta.parse(line,here);
            if (rc == InsertRc) {

                InsertData insertData = meta.LPub.insert.value();
                if (insertData.type == InsertData::InsertBom) {

                    QString bomID   = QString("%1_%2").arg(here.modelName).arg(here.lineNumber);
                    bomOccurrence   = bom_Occurrence[bomID];
                    boms            = bom_Occurrence.size();

                    logInfo() << QString("BOM Occurrance: %1 Number of BOMs: %2").arg(bomOccurrence).arg(boms)
                              << QString(" BOM_ID %1_%2").arg(here.modelName).arg(here.lineNumber);

                    break;
                }
            }
        }
    }
    return 0;
}

int Gui::divideBOMParts(
        const QStringList   &bomParts,
              QStringList   &dividedBOMParts){

    int quotient    = bomParts.size() / boms;
    int remainder   = bomParts.size() % boms;
    int maxParts    = 0;
    int i           = 0;

    if (bomOccurrence == boms){
        maxParts = bomOccurrence * quotient + remainder;
        i = maxParts - quotient - remainder;
    } else {
        maxParts = bomOccurrence * quotient;
        i = maxParts - quotient;
    }

    for(; i < maxParts; i++){
        dividedBOMParts << bomParts[i];
    }

    logTrace() << "bomParts: " << bomParts.size();
    logTrace() << "quotient: " << quotient;
    logTrace() << "remainder: " << remainder;
    logTrace() << "dividedBOMParts: " << dividedBOMParts.size();

    return 0;
}
