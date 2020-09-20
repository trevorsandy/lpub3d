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
#include <QScrollBar>
#include <QPixmap>
#include <QBitmap>
#include <QColor>
#include "lgraphicsscene.h"
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
#include "pagepointer.h"
#include "lgraphicsscene.h"
#include "name.h"

#include "ranges_item.h"
#include "csiannotation.h"
#include "dividerpointeritem.h"
#include "calloutpointeritem.h"
#include "pagepointeritem.h"

/*
 * We need to draw page every time there is change to the LDraw file.
 *   Changes can come from Menu->dialogs, people editing the file.
 *
 * Gui tracks modified, so whenever things go modified, we need to
 * delete all the GraphicsItems and do a freeranges.h.
 */

void Gui::clearPage(
    LGraphicsView *view,
    LGraphicsScene *scene,
    bool clearViewPageBg)
{
  page.freePage();
  page.pli.clear();
  page.subModel.clear();

  if (clearViewPageBg) {
    //  Moved to end of GraphicsPageItems()
    if (view->pageBackgroundItem) {
      delete view->pageBackgroundItem;
      view->pageBackgroundItem = nullptr;
    }
    scene->clear();
  }
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
      QGraphicsItem       *parentIn) :
      isHovered(false),
      mouseIsDown(false) {
    page = pageIn;
    QString toolTip("Times used - right-click to modify");
    setAttributes(SubmodelInstanceCountType,
                  page->relativeType,
                  numberMetaIn,
                  formatIn,
                  valueIn,
                  toolTip,
                  parentIn);
    setData(ObjectId, SubmodelInstanceCountObj);
    setZValue(SUBMODELINSTANCECOUNT_ZVALUE_DEFAULT);
    setFlag(QGraphicsItem::ItemIsSelectable,true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setAcceptHoverEvents(true);
  }
protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
  virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
  virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  bool isHovered;
  bool mouseIsDown;
};

void SubmodelInstanceCount::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QString name = "Submodel Instance Count";

  QAction *fontAction          = commonMenus.fontMenu(menu,name);
  QAction *colorAction         = commonMenus.colorMenu(menu,name);
  QAction *marginAction        = commonMenus.marginMenu(menu,name);
  QAction *placementAction     = commonMenus.placementMenu(menu,name,"You can move this submodel count around.");
  QAction *overrideCountAction = commonMenus.overrideCountMenu(menu,name);
  QAction *restoreCountAction  = nullptr;
  if (page->meta.LPub.page.countInstanceOverride.value()) {
    restoreCountAction         = commonMenus.restoreCountMenu(menu,name);
  } 

  QAction *selectedAction   = menu.exec(event->screenPos());

  Where topOfSteps          = page->topOfSteps();
  Where bottomOfSteps       = page->bottomOfSteps();
  bool  useTop              = parentRelativeType != StepGroupType;

  if (selectedAction == nullptr) {
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
    } else if (selectedAction == overrideCountAction) {
      bool ok;
      int max = (1 + gui->getSubmodelInstances(page->meta.LPub.countInstance.here().modelName, false)) * 4;
      int override = QInputDialog::getInt(gui,"Submodel Instances","Submodel Instances",value,0,max,1,&ok);
      if (ok) {
          Where trueTopOfModel;
          trueTopOfModel = firstLine(topOfSteps.modelName);
          page->meta.LPub.page.countInstanceOverride.setValue(override);
          setMetaTopOf(trueTopOfModel,
                       bottomOfSteps,
                       &page->meta.LPub.page.countInstanceOverride,
                       1    /*append*/,
                       true /*local*/,
                       false/*askLocal*/);
      }
    } else if (selectedAction == restoreCountAction) {
      Where undefined, here;
      here = page->meta.LPub.page.countInstanceOverride.here();
      if (here == undefined)
        return;
      deleteMeta(here);
    }
}

void SubmodelInstanceCount::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = !this->isSelected() && !mouseIsDown;
    QGraphicsItem::hoverEnterEvent(event);
}

void SubmodelInstanceCount::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void SubmodelInstanceCount::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mouseIsDown = true;
    QGraphicsItem::mousePressEvent(event);
    update();
}

void SubmodelInstanceCount::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  mouseIsDown = false;
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

  update();
}

void SubmodelInstanceCount::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

/*********************************************************************************
 * addGraphicsPageItems - this function is given the page contents in tree
 * data structure form, with all the CSI and PLI images rendered.  This function
 * then creates Qt graphics view items for each of the components of the page.
 * Simple things like page number, page background, inserts and the like are
 * handled here. For things like callouts, page pointers and step groups, there is a bunch
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
    LGraphicsScene *scene,
    bool            printing)
{

  Page                    *page  = dynamic_cast<Page *>(steps);

  Placement                plPage;
  PlacementHeader         *pageHeader;
  PlacementFooter         *pageFooter;
  PageBackgroundItem      *pageBg;
  PageNumberItem          *pageNumber = nullptr;
  SubmodelInstanceCount   *instanceCount = nullptr;
  TextItem                *textItem = nullptr;

  // disable viewer menu on conver page
  gMainWindow->mToolsToolBar->setEnabled(!page->coverPage);
  gMainWindow->mTimelineToolBar->setEnabled(!page->coverPage);
  gMainWindow->mPropertiesToolBar->setEnabled(!page->coverPage);
  gMainWindow->mPartsToolBar->setEnabled(!page->coverPage);
  gMainWindow->mColorsToolBar->setEnabled(!page->coverPage);
  ViewerMenu->setEnabled(!page->coverPage);

  int pW, pH;

  if (printing) {
      pW = view->maximumWidth();
      pH = view->maximumHeight();
    } else {
      // Flip page size per orientation and return size in pixels
      pW = pageSize(page->meta.LPub.page, 0);
      pH = pageSize(page->meta.LPub.page, 1);
    }

  // set page type (SingleStep, MultiStep)

  bool SingleStepPage = page->relativeType == SingleStepType && page->list.size();

//  logDebug() << QString("  DRAW PAGE %3 SIZE PIXELS - WidthPx: %1 x HeightPx: %2 CurPage: %3")
//                .arg(QString::number(pW), QString::number(pH)).arg(stepPageNum);

  pageBg = new PageBackgroundItem(page, pW, pH, exporting());

//  Moved to end of GraphicsPageItems()
//  view->pageBackgroundItem = pageBg;
//  pageBg->setPos(0,0);

  bool textPlacement     = false;

  // Set up the placement aspects of the page in the Qt space

  plPage.relativeType = PageType;

  plPage.setSize(pW,pH);
  plPage.margin  = page->meta.LPub.page.margin;
  plPage.loc[XX] = 0;
  plPage.loc[YY] = 0;

  // Set up page header and footer //~~~~~~~~~~~~~~~~

  int which = page->meta.LPub.page.orientation.value() == Landscape ? 1 : 0;
  float _pW = page->meta.LPub.page.size.value(which);
  page->meta.LPub.page.pageHeader.size.setValue(0,_pW);
  page->meta.LPub.page.pageFooter.size.setValue(0,_pW);

  pageHeader = new PlacementHeader(page->meta.LPub.page.pageHeader,pageBg);
  if (pageHeader->placement.value().relativeTo == plPage.relativeType) {
      plPage.appendRelativeTo(pageHeader);
      plPage.placeRelative(pageHeader);
    }
  pageHeader->setPos(pageHeader->loc[XX],pageHeader->loc[YY]);

  pageFooter = new PlacementFooter(page->meta.LPub.page.pageFooter,pageBg);
  if (pageFooter->placement.value().relativeTo == plPage.relativeType) {
      plPage.appendRelativeTo(pageFooter);
      plPage.placeRelative(pageFooter);
    }
  pageFooter->setPos(pageFooter->loc[XX],pageFooter->loc[YY]);

  // Process Cover Page Attriutes

  addCoverPageAttributes(page,pageBg,pageHeader,pageFooter,plPage);

  // Display the page number and instance count

  bool displayPageNumber = page->meta.LPub.page.dpn.value();
  if (displayPageNumber && ! coverPage) {

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

      // if this page contains the last step of the submodel,
      // and instance is > 1 and merge instance count, then display instance

      // allocate QGraphicsTextItem for instance number

      if (endOfSubmodel && page->displayInstanceCount/*page->instances > 1*/) {

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

              if (placementData.relativeTo == PageNumberType) {

                  if (page->meta.LPub.page.togglePnPlacement.value() &&
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
                  pageNumber->placeRelative(instanceCount);
                } else {
                  instanceCount->placement.setValue(BottomRightInsideCorner,PageType);
                  plPage.placeRelative(instanceCount);
                }
              instanceCount->setPos(instanceCount->loc[XX],instanceCount->loc[YY]);
            }
        }

      // Process Content Page Attributes - with page number

      addContentPageAttributes(page,pageBg,pageHeader,pageFooter,pageNumber,plPage,false);

    } else {

      // Process Content Page Attributes - without page number and end of submodel

      addContentPageAttributes(page,pageBg,pageHeader,pageFooter,nullptr,plPage,endOfSubmodel);
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
                fileInfo.setFile(getFilePath(insert.picName));

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
                } else {
                  emit messageSig(LOG_ERROR, QString("Unable to locate picture %1. Be sure picture file "
                                                     "is located relative to model file or use an absolute path.")
                                                     .arg(fileInfo.absoluteFilePath()));
                }
              }
              break;
            case InsertData::InsertText:
            case InsertData::InsertRichText:
              {
                textPlacement = page->meta.LPub.page.textPlacement.value();
                QString insertPreamble = QString("0 !LPUB INSERT ");
                if (insert.placementCommand && page->textItemList.size()) {
                    // upate placement in last inserted text
                    textItem        = textPlacement && page->textItemList.size() ?
                                      page->textItemList.last() : nullptr;
                    insertPreamble += QString("%1 PLACEMENT ")
                                              .arg(insert.type == InsertData::InsertRichText ?
                                                   "RICH_TEXT" : "TEXT");
                } else {
                    // create a new text instance and insert into list
                    textItem = new TextItem(page->inserts[i],DefaultPage,textPlacement,pageBg);
                    textItem->setSize(int(textItem->document()->size().width()),
                                      int(textItem->document()->size().height()));
                    if (textPlacement)
                        page->textItemList.append(textItem);
                }

                if (textItem) {

                    textItem->relativeType       = insert.relativeType;
                    if (SingleStepPage) {
                        textItem->parentRelativeType = PageType;
                    } else {
                        textItem->parentRelativeType = StepGroupType;
                    }


                    if (textPlacement && insert.defaultPlacement)  {
                        textItem->placement = page->meta.LPub.page.textPlacementMeta;
                    } else {
                        PlacementData pld;

                        Where insertHere  = page->inserts[i].here();
                        int insertPushed  = page->inserts[i].pushed;
                        int insertGlobal  = page->inserts[i].global;
                        bool insertErrors = page->inserts[i].reportErrors;

                        textItem->placement.pushed   = insertPushed;
                        textItem->placement.global   = insertGlobal;
                        textItem->placement.preamble = insertPreamble;
                        textItem->placement.reportErrors = insertErrors;
                        textItem->placement._here[insertPushed] = insertHere;

                        pld.placement      = insert.placement;
                        pld.justification  = insert.justification;
                        pld.relativeTo     = insert.relativeTo;
                        pld.preposition    = insert.preposition;
                        pld.rectPlacement  = insert.rectPlacement;
                        pld.offsets[0]     = insert.offsets[0];
                        pld.offsets[1]     = insert.offsets[1];

                        textItem->placement.setValue(pld);
                    }

                    int margin[2] = {0, 0};

                    if (textPlacement) {

                        if ((textItem->pagePlaced = textItem->placement.value().relativeTo == PageType)) {
                            plPage.appendRelativeTo(textItem);
                            plPage.placeRelative(textItem, margin);
                        } else
                        if ((textItem->pagePlaced = textItem->placement.value().relativeTo == PageHeaderType)) {
                            pageHeader->appendRelativeTo(textItem);
                            pageHeader->placeRelative(textItem);
                        } else
                        if ((textItem->pagePlaced = textItem->placement.value().relativeTo == PageFooterType)) {
                            pageFooter->appendRelativeTo(textItem);
                            pageFooter->placeRelative(textItem);
                        }
                        if (textItem->pagePlaced) {
                            textItem->setPos(textItem->loc[XX],textItem->loc[YY]);
                        }

                    } else {
                        plPage.placeRelative(textItem, margin);
                        textItem->setPos(textItem->loc[XX],textItem->loc[YY]);
                        textItem->relativeToSize[0] = plPage.size[XX];
                        textItem->relativeToSize[1] = plPage.size[YY];
                    }
                }
              }
              break;
            case InsertData::InsertArrow:
              break;
            case InsertData::InsertBom:
              {
                Where where(insert.where.modelName, insert.where.lineNumber);
                Where current(ldrawFile.topLevelFile(),0);
                QList<PliPartGroupMeta> bomPartGroups;
                QStringList bomParts;
                QString addLine;
                getBOMParts(current,addLine,bomParts,bomPartGroups);
                page->pli.steps = steps;
                getBOMOccurrence(current);
                page->pli.setParts(bomParts,bomPartGroups,page->meta,true,(boms > 1/*Split BOM Parts*/));
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

  // If the page contains a single step then process it here
  if (SingleStepPage) {
      if (page->list.size()) {
          Range *range = dynamic_cast<Range *>(page->list[0]);
          if (range->relativeType == RangeType) {
              Step *step = dynamic_cast<Step *>(range->list[0]);
              if (step && step->relativeType == StepType) {

                  if (!step->onlyChild())
                      page->stepNumber = step->stepNumber.number;

                  // populate page pixmaps - when using LDView Single Call

                  if (renderer->useLDViewSCall()){
                      addStepImageGraphics(step);
                    }

                  // set PLI relativeType

                  if (!page->pli.bom)
                      step->pli.relativeType = PartsListType;

                  // if show step number

                  if (! step->onlyChild() && step->showStepNumber) {

                      // add the step number

                      step->stepNumber.sizeit();

                      // if Submodel and Pli relative to StepNumber

                      if (step->placeSubModel &&
                          step->subModel.placement.value().relativeTo == StepNumberType) {

                          // Redirect Pli relative to SubModel

                          if (step->pli.pliMeta.show.value() &&
                                  step->pli.placement.value().relativeTo == StepNumberType) {

                              step->pli.placement.setValue(BottomLeftOutside,SubModelType);
                              step->subModel.appendRelativeTo(&step->pli);
                              step->subModel.placeRelative(&step->pli);
                          }
                      }
                  }

                  // if no step number

                  else {

                      // if Submodel relative to StepNumber

                      if (step->placeSubModel &&
                          step->subModel.placement.value().relativeTo == StepNumberType) {

                          // Redirect Submodel relative to PageHeader

                          step->subModel.placement.setValue(BottomLeftOutside,PageHeaderType);
                          pageHeader->appendRelativeTo(&step->subModel);
                          pageHeader->placeRelative(&step->subModel);
                      }

                      // if Pli relative to StepNumber

                      if (step->pli.pliMeta.show.value() &&
                          step->pli.pliMeta.placement.value().relativeTo == StepNumberType) {

                          if (step->placeSubModel)
                          {
                              // Redirect Pli relative to SubModel

                              step->pli.placement.setValue(BottomLeftOutside,SubModelType);
                              step->subModel.appendRelativeTo(&step->pli);
                              step->subModel.placeRelative(&step->pli);
                          }
                      }
                  }

                  // size the callouts

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

                  if (step->csiItem == nullptr) {
                      exit(-1);
                    }
                  step->csiItem->assign(&step->csiPlacement);
                  step->csiItem->boundingSize[XX] = step->csiItem->size[XX];
                  step->csiItem->boundingSize[YY] = step->csiItem->size[YY];

                  // add the SM graphically to the scene

                  if (step->placeSubModel) {
                      step->subModel.addSubModel(step->submodelLevel, pageBg);
                  }

                  // add the RotateIcon graphically to the scene

                  RotateIconItem *rotateIcon = nullptr;
                  if (step->placeRotateIcon && page->meta.LPub.rotateIcon.display.value()) {
                      step->rotateIcon.sizeit();
                      rotateIcon =
                              new RotateIconItem(
                                  step,
                                  page->relativeType,
                                  step->rotateIconMeta,
                                  pageBg);
                  }

                  // add the PLI graphically to the scene

                  step->pli.addPli(step->submodelLevel, pageBg);

                  // Place the step relative to the page.

                  plPage.relativeTo(step);      // place everything - calculate placement for all page objects

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

                  // place the CSI relative to the entire step's box

                  step->csiItem->setPos(step->csiItem->loc[XX],
                                        step->csiItem->loc[YY]);

                  // place texts not relative to page

                  for (int i = 0; i < page->textItemList.size(); i++) {
                      textItem  = textPlacement ?
                                  page->textItemList[i] : nullptr;
                      if (textItem) {
                          if (!textItem->pagePlaced) {
                              if ((textItem->pagePlaced = textItem->placement.value().relativeTo == CsiType)) {
                                  step->csiItem->appendRelativeTo(textItem);
                                  step->csiItem->placeRelative(textItem);
                                  textItem->setPos(textItem->loc[XX],textItem->loc[YY]);
                              }
                          } // if text item not placed relative to page
                      } // if text item
                  } // text items

                  // place CSI annotations //

                  if (step->csiItem->assem->annotation.display.value() &&
                      ! gui->exportingObjects())
                      step->csiItem->placeCsiPartAnnotations();

                  // add the SM relative to the entire step's box

                  if (step->placeSubModel) {
                      step->subModel.setPos(step->subModel.loc[XX],
                                            step->subModel.loc[YY]);
                  }

                  // place the PLI relative to the entire step's box

                  step->pli.setPos(step->pli.loc[XX],
                                   step->pli.loc[YY]);

                  // place the RotateIcon

                  if (rotateIcon) {
                      PlacementData pld = step->rotateIcon.placement.value();

                      rotateIcon->setPos(qreal(pld.offsets[XX]) + step->rotateIcon.loc[XX],
                                         qreal(pld.offsets[YY]) + step->rotateIcon.loc[YY]);

                      if (pld.offsets[XX] != 0.0f || pld.offsets[YY] != 0.0f) {
                          rotateIcon->relativeToSize[0] = step->rotateIcon.relativeToSize[0];
                          rotateIcon->relativeToSize[1] = step->rotateIcon.relativeToSize[1];
                      } else {
                          rotateIcon->assign(&step->rotateIcon);
                          rotateIcon->boundingSize[XX] = step->rotateIcon.size[XX];
                          rotateIcon->boundingSize[YY] = step->rotateIcon.size[YY];
                      }

                      rotateIcon->setFlag(QGraphicsItem::ItemIsMovable,true);
                  }

                  // allocate QGraphicsTextItem for step number

                  if ((! step->onlyChild() && step->showStepNumber) || ! step->displayModelOnlyStep()) {
                      StepNumberItem *stepNumber =
                          new StepNumberItem(step,
                                             page->relativeType,
                                             page->meta.LPub.stepNumber,
                                             "%d",
                                             step->stepNumber.number,
                                             pageBg);;
                      stepNumber->setPos(step->stepNumber.loc[XX],
                                         step->stepNumber.loc[YY]);
                      stepNumber->relativeToSize[0] = step->stepNumber.relativeToSize[0];
                      stepNumber->relativeToSize[1] = step->stepNumber.relativeToSize[1];

                    }

                  // add callout pointer items to the scene

                  for (int i = 0; i < step->list.size(); i++) {

                      // foreach callout

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

                  // add page pointer base and pointers to the scene

                  for (auto i : page->pagePointers.keys()) {
                      PagePointer *pp = page->pagePointers[i];

                      // override the default location
                      if (pp->placement.value().placement == Left ||
                          pp->placement.value().placement == Right)
                          pp->loc[YY] = pp->loc[YY] - (plPage.size[YY]/2);
                      else
                      if (pp->placement.value().placement == Top ||
                          pp->placement.value().placement == Bottom)
                          pp->loc[XX] = pp->loc[XX] - (plPage.size[XX]/2);

                      // size the pagePointer origin (hidden base rectangle)
                      pp->sizeIt();

                      // add the pagePointer origin (hidden base rectangle) to the graphics scene
                      pp->addGraphicsItems(0,0,pageBg,true); // set true to make movable

                      //   add the pagePointer pointers to the graphics scene
                      for (int i = 0; i < pp->pointerList.size(); i++) {
                          Pointer *pointer = pp->pointerList[i];
                          pp->addGraphicsPointerItem(pointer);
                        }
                    }

                  // Place the Bill of Materials on the page along with single step

                  if (page->pli.tsize()) {
                      if (page->pli.bom) {
                          page->pli.relativeType = BomType;
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
      if (page->list.size()) {
          for (int i = 0; i < page->list.size(); i++){
              Range *range = dynamic_cast<Range *>(page->list[i]);
              for (int j = 0; j < range->list.size(); j++){
                  if (range->relativeType == RangeType) {
                      Step *step = dynamic_cast<Step *>(range->list[j]);
                      if (step && step->relativeType == StepType){
                          // // LDView single call load images and set size
                          if (renderer->useLDViewSCall())
                              addStepImageGraphics(step);
                          // set last step number
                          page->stepNumber = step->stepNumber.number;
                      } // 1.4 validate if relativeType is StepType - to add image, check for Callout
                  } // 1.3 validate if relativeType is RangeType - to cast as Step
              } // 1.2 for each list-item (Step) within a Range...=>list[AbstractRangeElement]->StepType
          } // 1.1 for each list-item (Range) within a Page...=>list[AbstractStepsElement]->RangeType
      } // 1.0 Page

      PlacementData data = page->meta.LPub.multiStep.placement.value();
      page->placement.setValue(data);

      // place all the steps in the group relative to each other, including
      // any callouts placed relative to steps

      // sizeIt() sequence
      // formatpage.cpp page->sizeIt();                      - size multi-step
      // void Steps::sizeIt(void)                            - size horiz or vertical (freeform/size)
      // void Steps::sizeit(AllocEnc allocEnc, int x, int y) - vertical packing
      // void Range::sizeitHoriz()                           - accumulate the tallest of the rows
      // int Step::sizeit()                                  - Size components within a step
      // range.cpp sizeMargins(cols,colsMargin,margins);     - size margins

      page->sizeIt();             // size multi-step

      plPage.relativeToSg(page);  // place callouts and page pointers relative to PAGE

      plPage.placeRelative(page); // place multi-step relative to the page

      page->relativeToSg(page);   // compute bounding box of step group and callouts
                                  // placed relative to it.

      plPage.placeRelativeBounding(page); // center multi-step in page's bounding box

      page->relativeToSg(page);           // place callouts relative to MULTI_STEP

      page->addGraphicsItems(0,0,pageBg); // place all multi-step graphics items (e.g. RotateIcon...)

      // add page pointers to the scene

      for (auto i : page->pagePointers.keys()) {
          PagePointer *pp = page->pagePointers[i];

          // override the default location
          if (pp->placement.value().placement == Left ||
              pp->placement.value().placement == Right)
              pp->loc[YY] = pp->loc[YY] - (plPage.size[YY]/2);
          else
          if (pp->placement.value().placement == Top ||
              pp->placement.value().placement == Bottom)
              pp->loc[XX] = pp->loc[XX] - (plPage.size[XX]/2);

          // size the pagePointer origin (hidden page rectangle)
          pp->sizeIt();

          // add the pagePointer origin (page rectangle) to the graphics scene
          pp->addGraphicsItems(0,0,pageBg,true); // set true to make movable

          //   add the pagePointer pointers to the graphics scene
          for (int i = 0; i < pp->pointerList.size(); i++) {
              Pointer *pointer = pp->pointerList[i];
              pp->addGraphicsPointerItem(pointer);
            }
        }

      // Place the Bill of materials on the page along with step group?????

      if (page->pli.tsize()) {
          if (page->pli.bom) {
              page->pli.relativeType = BomType;
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

  // Moved from clearPage() to reduce page update lag.
  if (view->pageBackgroundItem) {
      delete view->pageBackgroundItem;
      view->pageBackgroundItem = nullptr;
  }
  scene->clear();
  // Moved from beginning of GraphicsPageItems() to reduce page update lag
  view->pageBackgroundItem = pageBg;
  pageBg->setPos(0,0);

  if ( ! printing) {

      if (pageBg->background.value().type != BackgroundData::BgTransparent){

          QGraphicsDropShadowEffect *bodyShadow = new QGraphicsDropShadowEffect;
          bodyShadow->setBlurRadius(9.0);
          bodyShadow->setColor(QColor(0, 0, 0, 160));
          bodyShadow->setOffset(4.0);

          pageBg->setGraphicsEffect(bodyShadow);
      }

      view->horizontalScrollBar()->setRange(0,pW);
      view->verticalScrollBar()->setRange(  0,pH);

    }

  scene->addItem(pageBg);

  addPliPartGroupsToScene(page, scene);

  view->setSceneRect(pageBg->sceneBoundingRect());

  QRectF pageRect = QRectF(0,0,pW,pH);
  if (printing) {
      view->fitInView(pageRect);
    }
  else
  if (view->fitMode == FitWidth) {
      view->fitWidth(pageRect);
    }
  else
  if (view->fitMode == FitVisible) {
      view->fitVisible(pageRect);
    }

  if (page->selectedSceneItems.size()) {
      setSceneItemZValue(page, scene);
  }

  page->relativeType = SingleStepType;
  statusBarMsg("");
  return 0;
}

/*
 * Add step image graphics
 * This function recurses the step's model to add images.
 * Call only if using LDView Single Call (useLDViewsCall=true)
 */
int Gui::addStepImageGraphics(Step *step) {
  int retVal = 0;
  step->csiPixmap.load(step->pngName);
  step->csiPlacement.size[0] = step->csiPixmap.width();
  step->csiPlacement.size[1] = step->csiPixmap.height();
  step->viewerOptions.ImageWidth = step->csiPixmap.width();
  step->viewerOptions.ImageHeight = step->csiPixmap.height();
  // process callout step's image(s)
  for (int k = 0; k < step->list.size(); k++) {
      if (step->list[k]->relativeType == CalloutType) {
          Callout *callout = dynamic_cast<Callout *>(step->list[k]);
          for (int l = 0; l < callout->list.size(); l++){
              Range *range = dynamic_cast<Range *>(callout->list[l]);
              for (int m = 0; m < range->list.size(); m++){
                  if (range->relativeType == RangeType) {
                      Step *step = dynamic_cast<Step *>(range->list[m]);
                      if (step && step->relativeType == StepType){
                          addStepImageGraphics(step);
                      } // 1.6 validate if Step relativeType is StepType - to add image, check for Callout
                  } // 1.5 validate if Range relativeType is RangeType - to cast as Step
              } // 1.4 for each Step list-item within a Range...=>list[AbstractRangeElement]->StepType
          } // 1.3 for each Range list-item within a Callout...=>list[AbstractStepsElement]->RangeType
      } // 1.2 validate if relativeType is CalloutType - to cast as Callout...
  } // 1.1 for each Callout list-item within a Step...=>list[Steps]->CalloutType
  return retVal;
}

int Gui::addStepPliPartGroupsToScene(Step *step,LGraphicsScene *scene){
    int retVal = 0;
    // add Pli part group to scene
    QHash<QString, PliPart*> pliParts;
    PliPart *part;
    QString key;
    if (step->pli.pliMeta.enablePliPartGroup.value() &&
        step->pli.pliMeta.show.value() &&
        step->pli.tsize()) {
        step->pli.getParts(pliParts);
        if (pliParts.size()) {
            Where top = step->topOfStep();
            Where bottom = step->bottomOfStep();
            int lineNumber = step->stepNumber.number;
            foreach(key,pliParts.keys()) {
                part = pliParts[key];
                part->addPartGroupToScene(scene,top,bottom,lineNumber);
            }
        }
    }
    // process callout step's PLI(s)
    for (int k = 0; k < step->list.size(); k++) {
        if (step->list[k]->relativeType == CalloutType) {
            Callout *callout = dynamic_cast<Callout *>(step->list[k]);
            for (int l = 0; l < callout->list.size(); l++){
                Range *range = dynamic_cast<Range *>(callout->list[l]);
                for (int m = 0; m < range->list.size(); m++){
                    if (range->relativeType == RangeType) {
                        Step *step = dynamic_cast<Step *>(range->list[m]);
                        if (step && step->relativeType == StepType){
                            addStepPliPartGroupsToScene(step,scene);
                        }
                    }
                }
            }
        }
    }
    return retVal;
}

int Gui::addPliPartGroupsToScene(
        Page           *page,
        LGraphicsScene *scene)
{
    if (page->list.size()){
        // Single Step
        if (page->relativeType == SingleStepType) {
            if (page->list.size()) {
                Range *range = dynamic_cast<Range *>(page->list[0]);
                if (range->relativeType == RangeType) {
                    Step *step = dynamic_cast<Step *>(range->list[0]);
                    if (step && step->relativeType == StepType) {
                        addStepPliPartGroupsToScene(step,scene);
                    }
                }
            }
        }
        // Step Group
        else
        {
            for (int i = 0; i < page->list.size(); i++) {
                Range *range = dynamic_cast<Range *>(page->list[i]);
                for (int j = 0; j < range->list.size(); j++) {
                    if (range->relativeType == RangeType) {
                        Step *step = dynamic_cast<Step *>(range->list[j]);
                        if (step && step->relativeType == StepType) {
                            addStepPliPartGroupsToScene(step,scene);
                        }
                    }
                }
            }
        }
    }
    // BOM
    if (page->pli.tsize()) {
        if (page->pli.pliMeta.enablePliPartGroup.value() &&
            page->pli.bom) {
            // add Pli part group to scene
            QHash<QString, PliPart*> pliParts;
            PliPart *part;
            QString key;
            page->pli.getParts(pliParts);
            if (pliParts.size()) {
                Where top = page->pli.step ? page->pli.step->topOfStep() : page->topOfSteps();
                Where bottom = page->pli.step ? page->pli.step->bottomOfStep() : page->bottomOfSteps();
                int lineNumber = page->pli.step ? page->pli.step->stepNumber.number : 0;
                foreach(key,pliParts.keys()) {
                    part = pliParts[key];
                    part->addPartGroupToScene(scene,top,bottom,lineNumber);
                }
            }
        }
    }
    return 0;
}

bool Gui::getSceneObjectWhere(QGraphicsItem *selectedItem, Where &itemTop)
{
    int dummy;
    return getSceneObject(selectedItem, itemTop, dummy);
}
bool Gui::getSceneObjectStep(QGraphicsItem *selectedItem, int &stepNumber)
{
    Where dummy;
    return getSceneObject(selectedItem, dummy, stepNumber);
}

bool Gui::getSceneObject(QGraphicsItem *selectedItem, Where &itemTop, int &stepNumber)
{
    itemTop    = page.top;
    stepNumber = page.stepNumber;
    SceneObject itemObj = UndefinedObj;

    if (selectedItem)
        itemObj = SceneObject(selectedItem->data(ObjectId).toInt());
    else
        return false;

    switch (itemObj){
    case AssemAnnotationObj:
    {
        CsiAnnotationItem *csiAnnotationItem = dynamic_cast<CsiAnnotationItem *>(selectedItem);
        if (csiAnnotationItem){
            itemTop = csiAnnotationItem->topOf;
            stepNumber = csiAnnotationItem->stepNumber;
        }
    }
        break;
    case AssemAnnotationPartObj:
    {
        PlacementCsiPart *placementCsiPart = dynamic_cast<PlacementCsiPart *>(selectedItem);
        if (placementCsiPart){
            itemTop = placementCsiPart->top;
            stepNumber = placementCsiPart->stepNumber;
        }
    }
        break;
    case AssemObj:
    {
        CsiItem *csiItem = dynamic_cast<CsiItem *>(selectedItem);
        if (csiItem){
            itemTop = csiItem->step->topOfStep();
            stepNumber = csiItem->step->stepNumber.number;
        }
    }
        break;
    case CalloutUnderpinningObj:
    {
        UnderpinningsItem *UnderpinningsItem = dynamic_cast<class UnderpinningsItem *>(selectedItem);
        if (UnderpinningsItem){
            itemTop = UnderpinningsItem->top;
            stepNumber = UnderpinningsItem->stepNumber;
        }
    }
        break;
    case CalloutBackgroundObj:
    {
        CalloutBackgroundItem *calloutBackgroundItem = dynamic_cast<CalloutBackgroundItem *>(selectedItem);
        if (calloutBackgroundItem){
            itemTop = calloutBackgroundItem->callout->parentStep->topOfStep();
            stepNumber = calloutBackgroundItem->callout->parentStep->stepNumber.number;
        }
    }
        break;
    case CalloutPointerObj:
    {
        CalloutPointerItem *calloutPointerItem = dynamic_cast<CalloutPointerItem *>(selectedItem);
        if (calloutPointerItem){
            itemTop = calloutPointerItem->pointerTop;
            stepNumber = calloutPointerItem->stepNumber;
        }
    }
        break;
    case CalloutInstanceObj:
    {
        CalloutInstanceItem *calloutInstanceItem = dynamic_cast<CalloutInstanceItem *>(selectedItem);
        if (calloutInstanceItem){
            itemTop = calloutInstanceItem->instanceTop;
            stepNumber = calloutInstanceItem->stepNumber;
        }
    }
        break;
    case DividerBackgroundObj:
    {
        DividerBackgroundItem *dividerBackgroundItem = dynamic_cast<DividerBackgroundItem *>(selectedItem);
        if (dividerBackgroundItem){
            itemTop = dividerBackgroundItem->top;
            stepNumber = dividerBackgroundItem->stepNumber;
        }
    }
        break;
    case DividerObj:
    {
        DividerItem *dividerItem = dynamic_cast<DividerItem *>(selectedItem);
        if (dividerItem){
            itemTop = dividerItem->parentStep->topOfStep();
            stepNumber = dividerItem->parentStep->stepNumber.number;
        }
    }
        break;
    case DividerLineObj:
    {
        DividerLine *dividerLine = dynamic_cast<DividerLine *>(selectedItem);
        if (dividerLine){
            itemTop = dividerLine->top;
            stepNumber = dividerLine->stepNumber;;
        }
    }
        break;
    case DividerPointerObj:
    {
        DividerPointerItem *dividerPointerItem = dynamic_cast<DividerPointerItem *>(selectedItem);
        if (dividerPointerItem){
            itemTop = dividerPointerItem->pointerTop;
            stepNumber = dividerPointerItem->stepNumber;
        }
    }
        break;
    // for the moment, these all have the samve zValues
    case PointerGrabberObj:
    case PliGrabberObj:
    case SubmodelGrabberObj:
    {
        Grabber *grabberItem = dynamic_cast<Grabber *>(selectedItem);
        if (grabberItem){
            itemTop = grabberItem->top;
            stepNumber = grabberItem->stepNumber;
        }
    }
        break;
    case InsertPixmapObj:
    {
        InsertPixmapItem *insertPixmapItem = dynamic_cast<InsertPixmapItem *>(selectedItem);
        if (insertPixmapItem){
            itemTop = insertPixmapItem->meta.here();
            stepNumber = page.stepNumber;
        }
    }
        break;
    case InsertTextObj:
    {
        TextItem *textItem = dynamic_cast<TextItem *>(selectedItem);
        if (textItem){
            itemTop = textItem->meta.here();
            stepNumber = page.stepNumber;
        }
    }
        break;
    case PagePointerObj:
    {
        PagePointerItem *pagePointerItem = dynamic_cast<PagePointerItem *>(selectedItem);
        if (pagePointerItem){
            itemTop = pagePointerItem->pointerTop;
            stepNumber = pagePointerItem->stepNumber;
        }
    }
        break;
    case PartsListAnnotationObj:
    {
        AnnotateTextItem *annotateTextItem = dynamic_cast<AnnotateTextItem *>(selectedItem);
        if (annotateTextItem){
            itemTop = annotateTextItem->pli->top;
            stepNumber = annotateTextItem->pli->step->stepNumber.number;
        }
    }
        break;
    case PartsListBackgroundObj:
    {
        PliBackgroundItem *pliBackgroundItem = dynamic_cast<PliBackgroundItem *>(selectedItem);
        if (pliBackgroundItem){
            itemTop = pliBackgroundItem->pli->top;
            stepNumber = pliBackgroundItem->pli->step->stepNumber.number;
        }
    }
        break;
    case PartsListInstanceObj:
    {
        InstanceTextItem *instanceTextItem = dynamic_cast<InstanceTextItem *>(selectedItem);
        if (instanceTextItem){
            itemTop = instanceTextItem->pli->top;
            stepNumber = instanceTextItem->pli->step->stepNumber.number;
        }
    }
        break;
    case PointerHeadObj:
    {
        PointerHeadItem *pointerHeadItem = dynamic_cast<PointerHeadItem *>(selectedItem);
        if (pointerHeadItem){
            itemTop = pointerHeadItem->top;
            stepNumber = pointerHeadItem->stepNumber;
        }
    }
        break;
    // these all use the same base object which captures the appropriate zValue
    case PointerFirstSegObj:
    case PointerSecondSegObj:
    case PointerThirdSegObj:
    {
        BorderedLineItem *borderedLineItem = dynamic_cast<BorderedLineItem *>(selectedItem);
        if (borderedLineItem){
            itemTop = borderedLineItem->top;
            stepNumber = borderedLineItem->stepNumber;
        }
    }
        break;
    case RotateIconBackgroundObj:
    {
        RotateIconItem *rotateIconItem = dynamic_cast<RotateIconItem *>(selectedItem);
        if (rotateIconItem){
            itemTop = rotateIconItem->step->topOfStep();
            stepNumber = rotateIconItem->step->stepNumber.number;
        }
    }
        break;
    case StepNumberObj:
    {
        StepNumberItem *stepNumberItem = dynamic_cast<StepNumberItem *>(selectedItem);
        if (stepNumberItem){
            itemTop = stepNumberItem->top;
            stepNumber = stepNumberItem->value;
        }
    }
        break;
    case SubModelBackgroundObj:
    {
        SubModelBackgroundItem *subModelBackgroundItem = dynamic_cast<SubModelBackgroundItem *>(selectedItem);
        if (subModelBackgroundItem){
            itemTop = subModelBackgroundItem->subModel->step->topOfStep();
            stepNumber = subModelBackgroundItem->subModel->step->stepNumber.number;
        }
    }
        break;
    case SubModelInstanceObj:
    {
        SMInstanceTextItem *sMInstanceTextItem = dynamic_cast<SMInstanceTextItem *>(selectedItem);
        if (sMInstanceTextItem){
            itemTop = sMInstanceTextItem->subModel->step->topOfStep();
            stepNumber = sMInstanceTextItem->subModel->step->stepNumber.number;
        }
    }
        break;
    case PartsListPixmapObj:
    {
        PGraphicsPixmapItem *pGraphicsPixmapItem = dynamic_cast<PGraphicsPixmapItem *>(selectedItem);
        if (pGraphicsPixmapItem){
            itemTop = pGraphicsPixmapItem->pli->topOfStep();
            stepNumber = pGraphicsPixmapItem->pli->step->stepNumber.number;
        }
    }
        break;
    case PartsListGroupObj:
    {
        PartGroupItem *partGroupItem = dynamic_cast<PartGroupItem *>(selectedItem);
        if (partGroupItem){
            itemTop = partGroupItem->top;
            stepNumber = partGroupItem->stepNumber;
        }
    }
        break;
    case StepBackgroundObj:
    {
        MultiStepStepBackgroundItem *multiStepStepBackgroundItem = dynamic_cast<MultiStepStepBackgroundItem *>(selectedItem);
        if (multiStepStepBackgroundItem){
            itemTop = multiStepStepBackgroundItem->top;
            stepNumber = multiStepStepBackgroundItem->stepNumber;
        }
    }
        break;
    case PageAttributePixmapObj:
    case PageAttributeTextObj:
    case PageNumberObj:
    case SubmodelInstanceCountObj:
    case MultiStepBackgroundObj:
    case MultiStepsBackgroundObj:
    default:
        break;
    }

    return itemTop != Where();
}

void Gui::setSceneItemZValue(SceneObjectDirection direction)
{
    bool debugLogging = Preferences::debugLogging;
    QPointF scenePosition(KpageScene->mPos(XX),KpageScene->mPos(YY));
    QGraphicsItem *selectedItem = KpageScene->itemAt(scenePosition, QTransform());

    if (!selectedItem)
        return;

    selectedItemObj        = SceneObject(selectedItem->data(ObjectId).toInt());
    if (debugLogging)
        emit messageSig(LOG_DEBUG, QString("Start %2 (%3) %1 opration...")
                        .arg(direction == BringToFront ? "bring-to-front" : "send-to-back")
                        .arg(soMap[selectedItemObj]).arg(selectedItemObj));

    SceneObjectMeta *soMeta = dynamic_cast<SceneObjectMeta*>(
                              page.meta.LPub.page.scene.list.value(soMap[selectedItemObj]));

    if (!soMeta)
        return;

    Where itemTop;
    QMap<Where, SceneObjectData>::iterator i;
    for (i = page.selectedSceneItems.begin(); i != page.selectedSceneItems.end(); ++i) {
        if (i.value().itemObj == selectedItemObj) {
            itemTop = i.key();
            soMeta->setValue(i.value());
            break;
        }
    }

    bool newCommand = itemTop == Where();
    int  stepNumber = page.stepNumber;
    Where tempWhere;

    getSceneObject(selectedItem, tempWhere, stepNumber);

    if (newCommand)
        itemTop = tempWhere;

    SceneObjectData soData = soMeta->value();
    soData.direction       = direction;
    soData.scenePos[XX]    = float(scenePosition.x());
    soData.scenePos[YY]    = float(scenePosition.y());
    soMeta->setValue(soData);
    QString metaString = soMeta->format(true/*local*/,false/*global*/);

    if (Preferences::debugLogging)
        emit messageSig(LOG_DEBUG, QString("%1 %2 (%3) ITEM LINE NUMBER: %4 MODEL NAME: %5 STEP %6 %7")
                        .arg(direction == BringToFront ? "BRING TO FRONT" : "SEND TO BACK")
                        .arg(soMap[selectedItemObj]).arg(selectedItemObj)
                        .arg(itemTop.lineNumber).arg(itemTop.modelName)
                        .arg(stepNumber).arg(newCommand ? "NEW COMMAND" : ""));

    beginMacro("SceneItemZValue");

    MetaItem mi;
    mi.setMetaAlt(itemTop, metaString, newCommand);

    endMacro();
}

void Gui::setSceneItemZValue(Page *page, LGraphicsScene *scene)
{
    bool debugLogging = Preferences::debugLogging;
    QMap<Where, SceneObjectData>::iterator i;
    for (i = page->selectedSceneItems.begin(); i != page->selectedSceneItems.end(); ++i) {
//        if (debugLogging)
//            emit messageSig(LOG_DEBUG, QString("00 Data scene item %1 (%2), Selected scene item %3 (%4)...")
//                            .arg(soMap[SceneObject(i.value().itemObj)])
//                            .arg(i.value().itemObj)
//                            .arg(soMap[selectedItemObj])
//                            .arg(selectedItemObj));
        if (i.value().itemObj == selectedItemObj) {
            SceneObjectData soData = i.value();
            QPointF scenePosition  = QPointF(double(soData.scenePos[XX]),double(soData.scenePos[YY]));
            QGraphicsItem *selectedItem = scene->itemAt(scenePosition, QTransform());
            SceneObject itemObjS   = SceneObject(selectedItem->data(ObjectId).toInt());
            SceneObject itemObjD   = SceneObject(soData.itemObj);
            int itemStepNumber;
            if (!getSceneObjectStep(selectedItem, itemStepNumber))
                if (debugLogging)
                    emit messageSig(LOG_ERROR, QString("00 Failed to retrieve scene item %1 (%2) step number")
                                    .arg(soMap[itemObjS]).arg(itemObjS));

            if (debugLogging)
                emit messageSig(LOG_DEBUG, QString("01 Initial %1 scene item %2 (%3) initial ZValue %4 data item %5 (%6) step %7")
                                .arg(soData.direction == BringToFront ? "bring-to-front" : "send-to-back")
                                .arg(soMap[itemObjS]).arg(itemObjS)
                                .arg(selectedItem->zValue())
                                .arg(soMap[itemObjD]).arg(itemObjD)
                                .arg(itemStepNumber));

            if (!selectedItem)
                continue;

            QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();

            // in the case where the selected item's zvalue does not place it at topmost
            if (itemObjS != itemObjD) {
                selectedItem = nullptr;
                if (!overlapItems.size()) {
                    overlapItems = scene->items();
                    if (debugLogging)
                        emit messageSig(LOG_DEBUG, QString("01 Checking all scene items..."));
                } else {
                    if (debugLogging)
                        emit messageSig(LOG_DEBUG, QString("01 Checking colliding scene items..."));
                }
                foreach (QGraphicsItem *item, overlapItems) {
                    itemObjS = SceneObject(item->data(ObjectId).toInt());
                    if (itemObjS == itemObjD){
                        if (page->relativeType == StepGroupType){
                            int selectedItemStepNumber = 0;
                            if (getSceneObjectStep(item, selectedItemStepNumber)) {
                                if (debugLogging)
                                    emit messageSig(LOG_TRACE, QString("-- Selected scene item step number %1, data step number %2")
                                                    .arg(itemStepNumber).arg(itemStepNumber));
                                if (selectedItemStepNumber == itemStepNumber) {
                                    selectedItem = item;
                                    overlapItems = selectedItem->collidingItems();
                                    break;
                                }
                            } else {
                                if (debugLogging)
                                    emit messageSig(LOG_NOTICE, QString("-- Failed to retrieve selected scene item %1 (%2) step number")
                                                    .arg(soMap[itemObjS]).arg(itemObjS));
                            }
                        } else if (page->relativeType == SingleStepType){
                            selectedItem = item;
                            overlapItems = selectedItem->collidingItems();
                            break;
                        }
                    }
                }
            }

            if (!selectedItem)
                continue;

            // this lambda function captures related items. For example, a
            // callout pointer is related to the callout so the whole callout/pointer
            // assembly must be moved when a pointer move is triggered
            auto addRelatedItems = [this, &itemStepNumber, &debugLogging]
                    (QGraphicsItem *item){
                QList<QGraphicsItem *>relatedItems;
                int relatedItemStepNumber = 0;
                SceneObject itemObjR = SceneObject(item->data(ObjectId).toInt());
                if (getSceneObjectStep(item, relatedItemStepNumber)) {
                    if (debugLogging)
                        emit messageSig(LOG_TRACE, QString("-- Related scene item step number %1, data step number %2")
                                        .arg(itemStepNumber).arg(itemStepNumber));
                    if (relatedItemStepNumber == itemStepNumber) {
                        if (!relatedItems.contains(item)){
                            if (debugLogging)
                                emit messageSig(LOG_TRACE, QString("-- Add related scene item %1 (%2) for step (%3)...")
                                                .arg(soMap[itemObjR]).arg(itemObjR).arg(relatedItemStepNumber));
                            relatedItems.append(item);
                            foreach (QGraphicsItem *childItem, item->childItems()) {
                                if (!relatedItems.contains(childItem)){
                                    SceneObject itemObjC = SceneObject(childItem->data(ObjectId).toInt());
                                    if (debugLogging)
                                        emit messageSig(LOG_TRACE, QString("-- Add child scene item %1 (%2) added for step (%3)...")
                                                        .arg(soMap[itemObjC]).arg(itemObjC).arg(relatedItemStepNumber));
                                    relatedItems.append(childItem);
                                }
                            }
                        }
                    }
                } else {
                    if (debugLogging)
                        emit messageSig(LOG_NOTICE, QString("-- Failed to retrieve related scene item %1 (%2) step number")
                                        .arg(soMap[itemObjR]).arg(itemObjR));
                }
                return relatedItems;
            };

            // Check for and add related items
            // Callout Pointer
            QList<QGraphicsItem *>relatedItems;
            if (itemObjS == CalloutPointerObj){
                foreach (QGraphicsItem *item, scene->items()) {
                    SceneObject itemObjR = SceneObject(item->data(ObjectId).toInt());
                    if (debugLogging)
                        emit messageSig(LOG_DEBUG, QString("-- Checking related scene item %1 (%2)...")
                                        .arg(soMap[itemObjR]).arg(itemObjR));
                    if (itemObjR == CalloutUnderpinningObj)
                        relatedItems.append(addRelatedItems(item));
                    if (itemObjR == CalloutBackgroundObj)
                        relatedItems.append(addRelatedItems(item));
                }
            }
            // Callout
            if (itemObjS == CalloutBackgroundObj){
                foreach (QGraphicsItem *item, scene->items()) {
                    SceneObject itemObjR = SceneObject(item->data(ObjectId).toInt());
                    if (debugLogging)
                        emit messageSig(LOG_DEBUG, QString("-- Checking related scene item %1 (%2)...")
                                        .arg(soMap[itemObjR]).arg(itemObjR));
                    if (itemObjR == CalloutUnderpinningObj)
                        relatedItems.append(addRelatedItems(item));
                    if (itemObjR == CalloutPointerObj)
                        relatedItems.append(addRelatedItems(item));
                }
            }

            // Divider pointer
            if (itemObjS == DividerPointerObj){
                foreach (QGraphicsItem *item, scene->items()) {
                    SceneObject itemObjR = SceneObject(item->data(ObjectId).toInt());
                    if (debugLogging)
                        emit messageSig(LOG_DEBUG, QString("-- Checking related scene item %1 (%2)...")
                                        .arg(soMap[itemObjR]).arg(itemObjR));
                    if (itemObjR == DividerObj)
                        relatedItems.append(addRelatedItems(item));
                    if (itemObjR == DividerBackgroundObj)
                        relatedItems.append(addRelatedItems(item));
                }
            }
            // Divider
            if (itemObjS == DividerObj){
                foreach (QGraphicsItem *item, scene->items()) {
                    SceneObject itemObjR = SceneObject(item->data(ObjectId).toInt());
                    if (debugLogging)
                        emit messageSig(LOG_DEBUG, QString("-- Checking related scene item %1 (%2)...")
                                        .arg(soMap[itemObjR]).arg(itemObjR));
                    if (itemObjR == DividerBackgroundObj)
                        relatedItems.append(addRelatedItems(item));
                    if (itemObjR == DividerPointerObj)
                        relatedItems.append(addRelatedItems(item));
                }
            }

            if (debugLogging)
                emit messageSig(LOG_DEBUG, QString("02 Selected scene item %1 (%2) initial ZValue %3 data item %4 (%5) POS X (%6) POS Y (%7)")
                                .arg(soMap[itemObjS]).arg(itemObjS)
                                .arg(selectedItem->zValue())
                                .arg(soMap[itemObjD]).arg(itemObjD)
                                .arg(double(selectedItem->scenePos().x()))
                                .arg(double(selectedItem->scenePos().y())));

            qreal zValue = 0;
            SceneObjectDirection direction = soData.direction;
            foreach (QGraphicsItem *item, overlapItems) {
                qreal itemZValue = item->zValue() ;
                SceneObject itemObjO = SceneObject(item->data(ObjectId).toInt());
                bool itemIsUserSceneObject = isUserSceneObject(itemObjO);
                if (itemIsUserSceneObject) {
                    if (direction == SendToBack) {
                        if (itemZValue <= zValue)
                            zValue = item->zValue() - 1.0;
                    } else {
                        if (itemZValue >= zValue)
                            zValue = item->zValue() + 1.0;
                    }
                    if (debugLogging)
                        emit messageSig(LOG_DEBUG, QString("03 Overlap scene item %1 (%2) ZValue %3")
                                        .arg(soMap[itemObjO]).arg(itemObjO)
                                        .arg(item->zValue()));
                } else {
                    emit messageSig(LOG_TRACE, QString("Overlap scene item %1 (%2) ZValue %3 is not in the "
                                                       "LPub3D User Scene Object list")
                                    .arg(soMap[itemObjO]).arg(itemObjO)
                                    .arg(item->zValue()));
                }
            }

            if (relatedItems.size()){
                qreal zValueR = zValue;
                foreach (QGraphicsItem *item, relatedItems) {
                    qreal itemZValueR = item->zValue() ;
                    SceneObject itemObjR = SceneObject(item->data(ObjectId).toInt());
                    if (debugLogging)
                        emit messageSig(LOG_DEBUG, QString("03 Related scene item %1 (%2) ZValue %3")
                                        .arg(soMap[itemObjR]).arg(itemObjR)
                                        .arg(item->zValue()));
                    if (direction == SendToBack) {
                        if (itemZValueR >= zValue) {
                            zValueR = itemZValueR > 0 ?           /*positive*/
                                        -itemZValueR < zValue ?  /*less than zValue*/
                                            -itemZValueR : zValue :
                                                                 itemZValueR < zValue ?
                                                                    itemZValueR : zValue;
                            zValue = zValueR - 1;
                        }
                    } else {         /*BingToFront*/
                        if (itemZValueR <= zValue) {
                            zValueR = itemZValueR < 0 ?  /*negative*/
                                        -itemZValueR > zValue ?  /*greater than zValue*/
                                            -itemZValueR : zValue :
                                                                 itemZValueR > zValue ?
                                                                    itemZValueR : zValue;
                            zValue = zValueR + 1;
                        }
                    }
                    item->setZValue(zValueR);
                    if (debugLogging)
                        emit messageSig(LOG_TRACE, QString("03 Related scene item %1 (%2) adjusted ZValue %3")
                                        .arg(soMap[itemObjR]).arg(itemObjR)
                                        .arg(item->zValue()));
                }
            }

            selectedItem->setZValue(zValue);
            KpageScene->update();                  // TODO - Check when this is disabled

            if (debugLogging)
                emit messageSig(LOG_DEBUG, QString("04 Final %1 scene item %2 (%3) applied ZValue %4 POS X (%5) POS Y (%6)")
                                .arg(direction == BringToFront ? "bring-to-front" : "send-to-back")
                                .arg(soMap[itemObjS]).arg(itemObjS).arg(zValue)
                                .arg(double(selectedItem->scenePos().x()))
                                .arg(double(selectedItem->scenePos().y())));
            break;
        }
    }
//    selectedItemObj = UndefinedObj;
    if (debugLogging)
        emit messageSig(LOG_DEBUG, QString("----------------------------------"));
}

int Gui::addContentPageAttributes(
    Page                *page,
    PageBackgroundItem  *pageBg,
    PlacementHeader     *pageHeader,
    PlacementFooter     *pageFooter,
    PageNumberItem      *pageNumber,
    Placement           &plPage,
    bool                 endOfSubmodel)
{
  // Content Page Initializations and Placements...
  if (! page->coverPage)
    {
      // Initializations ...

      PageAttributeTextItem   *url       = new PageAttributeTextItem(page,page->meta.LPub.page.url,pageBg);
      PageAttributeTextItem   *email     = new PageAttributeTextItem(page,page->meta.LPub.page.email,pageBg);
      PageAttributeTextItem   *copyright = new PageAttributeTextItem(page,page->meta.LPub.page.copyright,pageBg);
      PageAttributeTextItem   *author    = new PageAttributeTextItem(page,page->meta.LPub.page.author,pageBg);
      bool displayPageNumber = page->meta.LPub.page.dpn.value();

      //  Content Page URL (Header/Footer) Initialization //~~~~~~~~~~~~~~~~
      bool displayURL         = page->meta.LPub.page.url.display.value();
      if (displayURL) {
          url->size[XX]       = int(url->document()->size().width());
          url->size[YY]       = int(url->document()->size().height());
      } else {
          delete url;
          url                 = nullptr;
      }
      //  Content Page Email (Header/Footer) Initialization //~~~~~~~~~~~~~~~~
      bool displayEmail       = page->meta.LPub.page.email.display.value();
      if (displayEmail){
          email->size[XX]     = int(email->document()->size().width());
          email->size[YY]     = int(email->document()->size().height());
      }else {
          delete email;
          email               = nullptr;
      }
      //  Content Page Copyright (Header/Footer) Initialization //~~~~~~~~~~~~~~~~
      bool displayCopyright   = page->meta.LPub.page.copyright.display.value();
      if (displayCopyright){
          copyright->size[XX] = int(copyright->document()->size().width());
          copyright->size[YY] = int(copyright->document()->size().height());
      }else {
          delete copyright;
          copyright           = nullptr;
      }
      //  Content Page Author (Header/Footer) Initialization //~~~~~~~~~~~~~~~~
      bool displayAuthor      = page->meta.LPub.page.author.display.value();
      if (displayAuthor) {
          author->size[XX]    = int(author->document()->size().width());
          author->size[YY]    = int(author->document()->size().height());
      }else {
          delete author;
          author              = nullptr;
      }

      // Placements...

      //  Content Page URL (Header/Footer) Placement //~~~~~~~~~~~~~~~~
      if (displayURL) {
          PlacementData pld = url->placement.value();
          if (pld.relativeTo == PageType) {
              plPage.appendRelativeTo(url);
              plPage.placeRelative(url);
          } else if (pld.relativeTo == PageHeaderType) {
              pageHeader->appendRelativeTo(url);
              pageHeader->placeRelative(url);
          } else if (pld.relativeTo == PageFooterType) {
              pageFooter->appendRelativeTo(url);
              pageFooter->placeRelative(url);
          } else if (displayPageNumber && pld.relativeTo == PageNumberType) {
              pageNumber->appendRelativeTo(url);
              pageNumber->placeRelative(url);
          } else if (displayEmail && pld.relativeTo == PageEmailType) {
              email->appendRelativeTo(url);
              email->placeRelative(url);
          } else if (displayCopyright && pld.relativeTo == PageCopyrightType) {
              copyright->appendRelativeTo(url);
              copyright->placeRelative(url);
          } else if (displayAuthor && pld.relativeTo == PageAuthorType) {
              author->appendRelativeTo(url);
              author->placeRelative(url);
          } else {
              plPage.appendRelativeTo(url);
              plPage.placeRelative(url);
          }
          url->setPos(url->loc[XX],url->loc[YY]);
      }

      //  Content Page Email (Header/Footer) Placement //~~~~~~~~~~~~~~~~
      if (displayEmail) {
          PlacementData pld = email->placement.value();
          if (pld.relativeTo == PageType) {
              plPage.appendRelativeTo(email);
              plPage.placeRelative(email);
          } else if (pld.relativeTo == PageHeaderType) {
              pageHeader->appendRelativeTo(email);
              pageHeader->placeRelative(email);
          } else if (pld.relativeTo == PageFooterType) {
              pageFooter->appendRelativeTo(email);
              pageFooter->placeRelative(email);
          } else if (displayPageNumber && pld.relativeTo == PageNumberType) {
              pageNumber->appendRelativeTo(email);
              pageNumber->placeRelative(email);
          } else if (displayURL && pld.relativeTo == PageURLType) {
              url->appendRelativeTo(email);
              url->placeRelative(email);
          } else if (displayCopyright && pld.relativeTo == PageCopyrightType) {
              copyright->appendRelativeTo(email);
              copyright->placeRelative(email);
          } else if (displayAuthor && pld.relativeTo == PageAuthorType) {
              author->appendRelativeTo(email);
              author->placeRelative(email);
          } else {
              plPage.appendRelativeTo(email);
              plPage.placeRelative(email);
          }
          email->setPos(email->loc[XX],email->loc[YY]);
      }

      //  Content Page Copyright (Header/Footer) Placement //~~~~~~~~~~~~~~~~
      if (displayCopyright) {
          PlacementData pld = copyright->placement.value();
          if (pld.relativeTo == PageType) {
              plPage.appendRelativeTo(copyright);
              plPage.placeRelative(copyright);
          } else if (pld.relativeTo == PageHeaderType) {
              pageHeader->appendRelativeTo(copyright);
              pageHeader->placeRelative(copyright);
          } else if (pld.relativeTo == PageFooterType) {
              pageFooter->appendRelativeTo(copyright);
              pageFooter->placeRelative(copyright);
          } else if (displayPageNumber && pld.relativeTo == PageNumberType) {
              pageNumber->appendRelativeTo(copyright);
              pageNumber->placeRelative(copyright);
          } else if (displayURL && pld.relativeTo == PageURLType) {
              url->appendRelativeTo(copyright);
              url->placeRelative(copyright);
          } else if (displayEmail && pld.relativeTo == PageEmailType) {
              email->appendRelativeTo(copyright);
              email->placeRelative(copyright);
          } else if (displayAuthor && pld.relativeTo == PageAuthorType) {
              author->appendRelativeTo(copyright);
              author->placeRelative(copyright);
          } else {
              plPage.appendRelativeTo(copyright);
              plPage.placeRelative(copyright);
          }
          copyright->setPos(copyright->loc[XX],copyright->loc[YY]);
      }

      //  Content Page Author (Header/Footer) Placement //~~~~~~~~~~~~~~~~
      if (displayAuthor) {
          PlacementData pld = author->placement.value();
          if (pld.relativeTo == PageType) {
              plPage.appendRelativeTo(author);
              plPage.placeRelative(author);
          } else if (pld.relativeTo == PageHeaderType) {
              pageHeader->appendRelativeTo(author);
              pageHeader->placeRelative(author);
          } else if (pld.relativeTo == PageFooterType) {
              pageFooter->appendRelativeTo(author);
              pageFooter->placeRelative(author);
          } else if (displayPageNumber && pld.relativeTo == PageNumberType) {
              pageNumber->appendRelativeTo(author);
              pageNumber->placeRelative(author);
          } else if (displayURL && pld.relativeTo == PageURLType) {
              url->appendRelativeTo(author);
              url->placeRelative(author);
          } else if (displayEmail && pld.relativeTo == PageEmailType) {
              email->appendRelativeTo(author);
              email->placeRelative(author);
          } else if (displayCopyright && pld.relativeTo == PageCopyrightType) {
              copyright->appendRelativeTo(author);
              copyright->placeRelative(author);
          } else if (displayPageNumber) {
              author->placement.setValue(LeftBottomOutside,PageNumberType);
              pageNumber->appendRelativeTo(author);
              pageNumber->placeRelative(author);
          } else {
              plPage.appendRelativeTo(author);
              plPage.placeRelative(author);
          }
          author->setPos(author->loc[XX],author->loc[YY]);
      }

      // Place insance count if end of submodel

      if (endOfSubmodel && page->displayInstanceCount/*page->instances > 1*/) {

          SubmodelInstanceCount   *instanceCount;

          instanceCount = new SubmodelInstanceCount(
                      page,
                      page->meta.LPub.page.instanceCount,
                      "x%d ",
                      page->instances,
                      pageBg);

          if (instanceCount) {
              instanceCount->setSize(int(instanceCount->document()->size().width()),
                                     int(instanceCount->document()->size().height()));
              instanceCount->loc[XX] = 0;
              instanceCount->loc[YY] = 0;
              instanceCount->tbl[0] = 0;
              instanceCount->tbl[1] = 0;

              instanceCount->placement = page->meta.LPub.page.instanceCount.placement;

              if (displayAuthor){
                  PlacementData pld = author->placement.value();
                  if ((pld.rectPlacement == TopLeftOutsideCorner     ||
                       pld.rectPlacement == TopLeftOutside           ||
                       pld.rectPlacement == TopOutside               ||
                       pld.rectPlacement == LeftTopOutside           ||
                       pld.rectPlacement == LeftOutside              ||
                       pld.rectPlacement == LeftBottomOutside       ) &&
                          ((pld.relativeTo    == PageNumberType          )||
                           (pld.rectPlacement == BottomRightInsideCorner  &&
                            pld.relativeTo    == PageType)))
                  {
                      instanceCount->placement.setValue(TopOutside,PageAuthorType);
                      author->appendRelativeTo(instanceCount);
                      author->placeRelative(instanceCount);

                  }
              }
              if (displayEmail){
                  PlacementData pld = email->placement.value();
                  if ((pld.rectPlacement == TopLeftOutsideCorner     ||
                       pld.rectPlacement == TopLeftOutside           ||
                       pld.rectPlacement == TopOutside               ||
                       pld.rectPlacement == LeftTopOutside           ||
                       pld.rectPlacement == LeftOutside              ||
                       pld.rectPlacement == LeftBottomOutside       ) &&
                          ((pld.relativeTo    == PageNumberType          )||
                           (pld.rectPlacement == BottomRightInsideCorner  &&
                            pld.relativeTo    == PageType)))
                  {
                      instanceCount->placement.setValue(TopOutside,PageEmailType);
                      email->appendRelativeTo(instanceCount);
                      email->placeRelative(instanceCount);
                  }
              }
              if (displayURL){
                  PlacementData pld = url->placement.value();
                  if ((pld.rectPlacement == TopLeftOutsideCorner     ||
                       pld.rectPlacement == TopLeftOutside           ||
                       pld.rectPlacement == TopOutside               ||
                       pld.rectPlacement == LeftTopOutside           ||
                       pld.rectPlacement == LeftOutside              ||
                       pld.rectPlacement == LeftBottomOutside       ) &&
                          ((pld.relativeTo    == PageNumberType          )||
                           (pld.rectPlacement == BottomRightInsideCorner  &&
                            pld.relativeTo    == PageType)))
                  {
                      instanceCount->placement.setValue(TopOutside,PageURLType);
                      url->appendRelativeTo(instanceCount);
                      url->placeRelative(instanceCount);
                  }
              }
              if (displayCopyright){
                  PlacementData pld = copyright->placement.value();
                  if ((pld.rectPlacement == TopLeftOutsideCorner     ||
                       pld.rectPlacement == TopLeftOutside           ||
                       pld.rectPlacement == TopOutside               ||
                       pld.rectPlacement == LeftTopOutside           ||
                       pld.rectPlacement == LeftOutside              ||
                       pld.rectPlacement == LeftBottomOutside       ) &&
                          ((pld.relativeTo    == PageNumberType          )||
                           (pld.rectPlacement == BottomRightInsideCorner  &&
                            pld.relativeTo    == PageType)))
                  {
                      instanceCount->placement.setValue(TopOutside,PageCopyrightType);
                      copyright->appendRelativeTo(instanceCount);
                      copyright->placeRelative(instanceCount);
                  }
              }
              else
              {
                  plPage.appendRelativeTo(instanceCount);
                  plPage.placeRelative(instanceCount);
              }
          }
          instanceCount->setPos(instanceCount->loc[XX],instanceCount->loc[YY]);
        }
    }
  return 0;
}

int Gui::addCoverPageAttributes(
    Page                *page,
    PageBackgroundItem  *pageBg,
    PlacementHeader     *pageHeader,
    PlacementFooter     *pageFooter,
    Placement           &plPage)
{
  // Front Cover Page Initializations and Placements...
  if (page->coverPage && page->frontCover) {

      // Initializations...
      PageAttributeTextItem   *titleFront            = new PageAttributeTextItem(page,page->meta.LPub.page.titleFront,pageBg);
      PageAttributeTextItem   *modelNameFront        = new PageAttributeTextItem(page,page->meta.LPub.page.modelName,pageBg);
      PageAttributeTextItem   *authorFront           = new PageAttributeTextItem(page,page->meta.LPub.page.authorFront,pageBg);
      PageAttributeTextItem   *partsFront            = new PageAttributeTextItem(page,page->meta.LPub.page.parts,pageBg);
      PageAttributeTextItem   *modelDescFront        = new PageAttributeTextItem(page,page->meta.LPub.page.modelDesc,pageBg);
      PageAttributeTextItem   *publishDescFront      = new PageAttributeTextItem(page,page->meta.LPub.page.publishDesc,pageBg);
      PageAttributePixmapItem *pixmapLogoFront;
      PageAttributePixmapItem *pixmapCoverImageFront;
      //PageAttributeTextItem   *categoryFront       = new PageAttributeTextItem(page,page->meta.LPub.page.category,pageBg);

      // Title (Front Cover) Initialization //~~~~~~~~~~~~~~~~
      bool displayTitleFront         = page->meta.LPub.page.titleFront.display.value();
      bool breakTitleFrontRelativeTo = !displayTitleFront;
      PlacementData titleFrontPld;
      if (displayTitleFront) {
          titleFront->size[XX]       = int(titleFront->document()->size().width());
          titleFront->size[YY]       = int(titleFront->document()->size().height());
          titleFrontPld = titleFront->placement.value();
          breakTitleFrontRelativeTo  = titleFrontPld.relativeTo != PageType;
      } else {
          delete titleFront;
          titleFront                 = nullptr;
      }

      // Model Name (Front Cover) Initialization //~~~~~~~~~~~~~~~~
      bool displayModelNameFront         = page->meta.LPub.page.modelName.display.value();
      bool breakModelNameFrontRelativeTo = !displayModelNameFront;
      PlacementData modelNameFrontPld;
      if (displayModelNameFront){
          modelNameFront->size[XX]       = int(modelNameFront->document()->size().width());
          modelNameFront->size[YY]       = int(modelNameFront->document()->size().height());
          modelNameFrontPld = modelNameFront->placement.value();
          breakModelNameFrontRelativeTo  = modelNameFrontPld.relativeTo != PageTitleType;
      } else {
          delete modelNameFront;
          modelNameFront                 = nullptr;
      }

       // Author (Front Cover) Initialization //~~~~~~~~~~~~~~~~
      bool displayAuthorFront         = page->meta.LPub.page.authorFront.display.value();
      bool breakAuthorFrontRelativeTo = !displayAuthorFront;
      PlacementData authorFrontPld;
      if (displayAuthorFront) {
          authorFront->size[XX]       = int(authorFront->document()->size().width());
          authorFront->size[YY]       = int(authorFront->document()->size().height());
          authorFrontPld = authorFront->placement.value();
          breakAuthorFrontRelativeTo  = authorFrontPld.relativeTo != PageTitleType;
      } else {
          delete authorFront;
          authorFront                 = nullptr;
      }

      // Parts Count (Front Cover) Initialization //~~~~~~~~~~~~~~~~
      bool displayPartsFront         = page->meta.LPub.page.parts.display.value();
      bool breakPartsFrontRelativeTo = !displayPartsFront;
      PlacementData PartsFrontPld;
      if (displayPartsFront) {
          partsFront->size[XX]       = int(partsFront->document()->size().width());
          partsFront->size[YY]       = int(partsFront->document()->size().height());
          PartsFrontPld = partsFront->placement.value();
          breakPartsFrontRelativeTo  = PartsFrontPld.relativeTo != PageAuthorType;
      } else {
          delete  partsFront;
          partsFront                 = nullptr;
      }

      // Model Description (Front Cover) Initialization //~~~~~~~~~~~~~~~~
      bool displayModelDescFront         = page->meta.LPub.page.modelDesc.display.value();
      bool breakModelDescFrontRelativeTo = !displayModelDescFront;
      PlacementData modelDescFrontPld;
      if (displayModelDescFront) {
          modelDescFront->size[XX]       = int(modelDescFront->document()->size().width());
          modelDescFront->size[YY]       = int(modelDescFront->document()->size().height());
          modelDescFrontPld = modelDescFront->placement.value();
          breakModelDescFrontRelativeTo  = modelDescFrontPld.relativeTo != PagePartsType;
      } else {
          delete modelDescFront;
          modelDescFront                 = nullptr;
      }

      // Publish Description (Front Cover) Initialization //~~~~~~~~~~~~~~~~
      bool displayPublishDescFront         = page->meta.LPub.page.publishDesc.display.value();
      bool breakPublishDescFrontRelativeTo = !displayPublishDescFront;
      PlacementData publishDescFrontPld;
      if (displayPublishDescFront) {
          publishDescFront->size[XX]      = int(publishDescFront->document()->size().width());
          publishDescFront->size[YY]      = int(publishDescFront->document()->size().height());
          publishDescFrontPld = publishDescFront->placement.value();
          breakPublishDescFrontRelativeTo = publishDescFrontPld.relativeTo != PageModelDescType;
      } else {
          delete publishDescFront;
          publishDescFront                = nullptr;
      }

      // Category (Front Cover) Initialization //~~~~~~~~~~~~~~~~
      /* bool displayCategoryFront      = page->meta.LPub.page.category.display.value();
      bool breakCategoryFrontRelativeTo = !displayCategoryFront;
      PlacementData categoryFrontPld;
      if (displayCategoryFront) {
          categoryFront->size[XX]       = int(categoryFront->document()->size().width());
          categoryFront->size[YY]       = int(categoryFront->document()->size().height());
          categoryFrontPld = categoryFront->placement.value();
          breakCategoryFrontRelativeTo  = categoryFrontPld.relativeTo != PagePartsType;
      } else {
          delete  categoryFront;
          categoryFront                 = nullptr;
      } */

      // Nothing is placed relative to the document logo or plug image so no break statement is defined.
      // Front Cover Document Logo
      bool displayDocumentLogoFront  = page->meta.LPub.page.documentLogoFront.display.value();

      // Front Cover Image
      bool displayCoverImage  = page->meta.LPub.page.coverImage.display.value();

      // Placements...

      // Title (Front Cover) //~~~~~~~~~~~~~~~~
      if (displayTitleFront) {
          plPage.appendRelativeTo(titleFront);
          plPage.placeRelative(titleFront);
          titleFront->setPos(titleFront->loc[XX],titleFront->loc[YY]);
      }

      // ModelName (Front Cover) Placement //~~~~~~~~~~~~~~~~
      if (displayModelNameFront) {                                   // display attribute On ?
          if (modelNameFrontPld.relativeTo == PageTitleType) {       // default relativeTo ?
              if (displayTitleFront && !breakTitleFrontRelativeTo){  // display and not break default relativeTo ?
                  titleFront->appendRelativeTo(modelNameFront);
                  titleFront->placeRelative(modelNameFront);
              } else {                                               // break or no display default relativeTo ?
                  titleFront->placement.setValue(RightInside,PageType);
                  plPage.appendRelativeTo(modelNameFront);
                  plPage.placeRelative(modelNameFront);
              }
          } else {                                                   // placement changed so place as is
              plPage.appendRelativeTo(modelNameFront);
              plPage.placeRelative(modelNameFront);
          }
          modelNameFront->setPos(modelNameFront->loc[XX],modelNameFront->loc[YY]);
      }

      // Author (Front Cover) Placement //~~~~~~~~~~~~~~~~
      if (displayAuthorFront) {
          if (authorFrontPld.relativeTo == PageTitleType) {
              if (displayTitleFront && !breakTitleFrontRelativeTo) {
                  titleFront->appendRelativeTo(authorFront);
                  titleFront->placeRelative(authorFront);
              } else {
                  authorFront->placement.setValue(LeftInside,PageType);
                  plPage.appendRelativeTo(authorFront);
                  plPage.placeRelative(authorFront);
              }
          } else {
              plPage.appendRelativeTo(authorFront);
              plPage.placeRelative(authorFront);
          }
          authorFront->setPos(authorFront->loc[XX],authorFront->loc[YY]);
      }

      // Parts (Front Cover) Placement //~~~~~~~~~~~~~~~~
      if (displayPartsFront) {
          if (PartsFrontPld.relativeTo == PageAuthorType) {
              if (displayAuthorFront && !breakAuthorFrontRelativeTo) {
                  authorFront->appendRelativeTo(partsFront);
                  authorFront->placeRelative(partsFront);
              } else {
                  partsFront->placement.setValue(RightInside,PageType);
                  plPage.appendRelativeTo(partsFront);
                  plPage.placeRelative(partsFront);
              }
          } else {
              plPage.appendRelativeTo(partsFront);
              plPage.placeRelative(partsFront);
          }
          partsFront->setPos(partsFront->loc[XX],partsFront->loc[YY]);
      }

      // ModelDesc (Front Cover) Placement //~~~~~~~~~~~~~~~~
      if (displayModelDescFront) {
          if (modelDescFrontPld.relativeTo == PagePartsType) {
              if (displayPartsFront && !breakPartsFrontRelativeTo) {
                  partsFront->appendRelativeTo(modelDescFront);
                  partsFront->placeRelative(modelDescFront);
              } else {
                  modelDescFront->placement.setValue(RightInside,PageType);
                  plPage.appendRelativeTo(modelDescFront);
                  plPage.placeRelative(modelDescFront);
              }
          } else {
              plPage.appendRelativeTo(modelDescFront);
              plPage.placeRelative(modelDescFront);
          }
          modelDescFront->setPos(modelDescFront->loc[XX],modelDescFront->loc[YY]);
      }

      // PublishDesc (Front Cover) Placement //~~~~~~~~~~~~~~~~
      if (displayPublishDescFront) {
          if (publishDescFrontPld.relativeTo == PageModelDescType) {
              if (displayModelDescFront && !breakModelDescFrontRelativeTo) {
                  modelDescFront->appendRelativeTo(publishDescFront);
                  modelDescFront->placeRelative(publishDescFront);
              } else {
                  publishDescFront->placement.setValue(RightInside,PageType);
                  plPage.appendRelativeTo(publishDescFront);
                  plPage.placeRelative(publishDescFront);
              }
          } else {
              plPage.appendRelativeTo(publishDescFront);
              plPage.placeRelative(publishDescFront);
          }
          publishDescFront->setPos(publishDescFront->loc[XX],publishDescFront->loc[YY]);
      }

      // Category (Front Cover) Placement //~~~~~~~~~~~~~~~~
      /*
      if (displayCategoryFront) {
          if (categoryFrontPld.relativeTo == PagePartsType) {
              if (displayPartsFront && !breakPartsFrontRelativeTo) {
                  partsFront->appendRelativeTo(categoryFront);
                  partsFront->placeRelative(categoryFront);
              } else  {
                  categoryFront->placement.setValue(RightInside,PageType);
                  plPage.appendRelativeTo(categoryFront);
                  plPage.placeRelative(categoryFront);
              }
          } else {
              plPage.appendRelativeTo(categoryFront);
              plPage.placeRelative(categoryFront);
          }
          categoryFront->setPos(categoryFront->loc[XX],categoryFront->loc[YY]);
      }
     */

      QFileInfo fileInfo;
      // DocumentLogo (Front Cover) //~~~~~~~~~~~~~~~~
      if (displayDocumentLogoFront) {
          qreal picScale = double(page->meta.LPub.page.documentLogoFront.picScale.value());
          fileInfo.setFile(getFilePath(page->meta.LPub.page.documentLogoFront.file.value()));
          if (fileInfo.exists()) {
              QPixmap qpixmap;
              qpixmap.load(fileInfo.absoluteFilePath());
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
                  pageHeader->appendRelativeTo(pixmapLogoFront);
                  pageHeader->placeRelative(pixmapLogoFront, margin);
                  pixmapLogoFront->relativeToSize[0] = pageHeader->size[XX];
                  pixmapLogoFront->relativeToSize[1] = pageHeader->size[YY];
              } else if (pld.relativeTo == PageFooterType) {
                  pageFooter->appendRelativeTo(pixmapLogoFront);
                  pageFooter->placeRelative(pixmapLogoFront, margin);
                  pixmapLogoFront->relativeToSize[0] = pageFooter->size[XX];
                  pixmapLogoFront->relativeToSize[1] = pageFooter->size[YY];
              } else {
                  plPage.appendRelativeTo(pixmapLogoFront);
                  plPage.placeRelative(pixmapLogoFront, margin);
                  pixmapLogoFront->relativeToSize[0] = plPage.size[XX];
                  pixmapLogoFront->relativeToSize[1] = plPage.size[YY];
              }
              pixmapLogoFront->setPos(pixmapLogoFront->loc[XX],pixmapLogoFront->loc[YY]);
          }
      }

      // CoverImage (Front Cover) //~~~~~~~~~~~~~~~~
      if (displayCoverImage) {
          qreal picScale   = double(page->meta.LPub.page.coverImage.picScale.value());
          fileInfo.setFile(getFilePath(page->meta.LPub.page.coverImage.file.value()));
          if (fileInfo.exists()) {
              QPixmap qpixmap;
              qpixmap.load(fileInfo.absoluteFilePath());
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
              pixmapCoverImageFront->setPos(pixmapCoverImageFront->loc[XX],pixmapCoverImageFront->loc[YY]);
              pixmapCoverImageFront->relativeToSize[0] = plPage.size[XX];
              pixmapCoverImageFront->relativeToSize[1] = plPage.size[YY];
          }
      }
  }

  // Back Cover Page Initializations and Placements...
  if (page->coverPage && page->backCover){

      // Initializations...
      PageAttributeTextItem   *titleBack      = new PageAttributeTextItem(page,page->meta.LPub.page.titleBack,pageBg);
      PageAttributeTextItem   *authorBack     = new PageAttributeTextItem(page,page->meta.LPub.page.authorBack,pageBg);
      PageAttributeTextItem   *copyrightBack  = new PageAttributeTextItem(page,page->meta.LPub.page.copyrightBack,pageBg);
      PageAttributeTextItem   *urlBack        = new PageAttributeTextItem(page,page->meta.LPub.page.urlBack,pageBg);
      PageAttributeTextItem   *emailBack      = new PageAttributeTextItem(page,page->meta.LPub.page.emailBack,pageBg);
      PageAttributeTextItem   *disclaimerBack = new PageAttributeTextItem(page,page->meta.LPub.page.disclaimer,pageBg);
      PageAttributeTextItem   *plugBack       = new PageAttributeTextItem(page,page->meta.LPub.page.plug,pageBg);;
      PageAttributePixmapItem *pixmapLogoBack;
      PageAttributePixmapItem *pixmapPlugImageBack;

      // Title (Back Cover) Initialization //~~~~~~~~~~~~~~~~
      bool displayTitleBack = page->meta.LPub.page.titleBack.display.value();
      bool breakTitleBackRelativeTo = !displayTitleBack;
      PlacementData titleBackPld;
      if (displayTitleBack) {
          titleBack->size[XX]      = int(titleBack->document()->size().width());
          titleBack->size[YY]      = int(titleBack->document()->size().height());
          titleBackPld = titleBack->placement.value();
          breakTitleBackRelativeTo = titleBackPld.relativeTo != PageType;
      } else {
          delete titleBack;
          titleBack                = nullptr;
      }

      // Author (Back Cover) Initialization //~~~~~~~~~~~~~~~~
      bool displayAuthorBack = page->meta.LPub.page.authorBack.display.value();
      bool breakAuthorBackRelativeTo = !displayAuthorBack;
      PlacementData authorBackPld;
      if (displayAuthorBack) {
          authorBack->size[XX]      = int(authorBack->document()->size().width());
          authorBack->size[YY]      = int(authorBack->document()->size().height());
          authorBackPld = authorBack->placement.value();
          breakAuthorBackRelativeTo = authorBackPld.relativeTo != PageTitleType;
      } else {
          delete authorBack;
          authorBack                = nullptr;
      }

      // Copyright (Back Cover) Initialization //~~~~~~~~~~~~~~~~
      bool displayCopyrightBack = page->meta.LPub.page.copyrightBack.display.value();
      bool breakCopyrightBackRelativeTo = !displayCopyrightBack;
      PlacementData copyrightBackPld;
      if (displayCopyrightBack) {
          copyrightBack->size[XX]      = int(copyrightBack->document()->size().width());
          copyrightBack->size[YY]      = int(copyrightBack->document()->size().height());
          copyrightBackPld = copyrightBack->placement.value();
          breakCopyrightBackRelativeTo = copyrightBackPld.relativeTo != PageAuthorType;
      } else {
          delete copyrightBack;
          copyrightBack                = nullptr;
      }

      // URL (Back Cover) Initialization //~~~~~~~~~~~~~~~~
      bool displayUrlBack = page->meta.LPub.page.urlBack.display.value();
      bool breakURLBackRelativeTo = !displayUrlBack;
      PlacementData urlBackPld;
      if (displayUrlBack) {
          urlBack->size[XX]      = int(urlBack->document()->size().width());
          urlBack->size[YY]      = int(urlBack->document()->size().height());
          urlBackPld = urlBack->placement.value();
          breakURLBackRelativeTo = urlBackPld.relativeTo != PageCopyrightType;
      } else {
          delete urlBack;
          urlBack                = nullptr;
      }

      // Email (Back Cover) Initialization //~~~~~~~~~~~~~~~~
      bool displayEmailBack = page->meta.LPub.page.emailBack.display.value();
      bool breakEmailBackRelativeTo = !displayEmailBack;
      PlacementData emailBackPld;
      if (displayEmailBack) {
          emailBack->size[XX]      = int(emailBack->document()->size().width());
          emailBack->size[YY]      = int(emailBack->document()->size().height());
          emailBackPld = emailBack->placement.value();
          breakEmailBackRelativeTo = emailBackPld.relativeTo != PageURLType;
      } else {
          delete emailBack;
          emailBack               = nullptr;
      }

      // Disclaimer (Back Cover) Display //~~~~~~~~~~~~~~~~
      bool displayDisclaimerBack = page->meta.LPub.page.disclaimer.display.value();
      bool breakDisclaimerBackRelativeTo = !displayDisclaimerBack;
      PlacementData disclaimerBackPld;
      if (displayDisclaimerBack) {
          disclaimerBack->size[XX]      = int(disclaimerBack->document()->size().width());
          disclaimerBack->size[YY]      = int(disclaimerBack->document()->size().height());
          disclaimerBackPld = disclaimerBack->placement.value();
          breakDisclaimerBackRelativeTo = disclaimerBackPld.relativeTo != PageEmailType;
      } else {
          delete disclaimerBack;
          disclaimerBack                = nullptr;
      }

      // Plug (Back Cover) Initialization //~~~~~~~~~~~~~~~~
      bool displayPlugBack = page->meta.LPub.page.plug.display.value();
      bool breakPlugBackRelativeTo = !displayPlugBack;
      PlacementData plugBackPld;
      if (displayPlugBack) {
          plugBack->size[XX]      = int(plugBack->document()->size().width());
          plugBack->size[YY]      = int(plugBack->document()->size().height());
          plugBackPld = plugBack->placement.value();
          breakPlugBackRelativeTo = plugBackPld.relativeTo != PageDisclaimerType;
      } else {
          delete plugBack;
          plugBack                = nullptr;
      }

      // Nothing is placed relative to the document logo or plug image so no break statement is defined.
      // Back Cover DocumentLogo
      bool displayDocumentLogoBack = page->meta.LPub.page.documentLogoBack.display.value();

      // Back Cover Plug Image
      bool displayPlugImageBack    = page->meta.LPub.page.plugImage.display.value();

      // Placements...

      // Title (Back Cover) Initialization //~~~~~~~~~~~~~~~~
      if (displayTitleBack) {
          plPage.appendRelativeTo(titleBack);
          plPage.placeRelative(titleBack);
          titleBack->setPos(titleBack->loc[XX],titleBack->loc[YY]);
        }

      // Author (Back Cover) Placement //~~~~~~~~~~~~~~~~
      if (displayAuthorBack) {
          if (authorBackPld.relativeTo == PageTitleType) {
              if (displayTitleBack && !breakTitleBackRelativeTo){
                  titleBack->appendRelativeTo(authorBack);
                  titleBack->placeRelative(authorBack);
              } else if (authorBackPld.relativeTo == PageTitleType) {
                  authorBack->placement.setValue(CenterCenter,PageType);
                  plPage.appendRelativeTo(authorBack);
                  plPage.placeRelative(authorBack);
              }
          } else {
              plPage.appendRelativeTo(authorBack);
              plPage.placeRelative(authorBack);
          }
          authorBack->setPos(authorBack->loc[XX],authorBack->loc[YY]);
      }

      // Copyright (Back Cover) Placement //~~~~~~~~~~~~~~~~
      if (displayCopyrightBack) {
          if (copyrightBackPld.relativeTo == PageAuthorType) {
              if (displayAuthorBack && !breakAuthorBackRelativeTo){
                  authorBack->appendRelativeTo(copyrightBack);
                  authorBack->placeRelative(copyrightBack);
              } else if (copyrightBackPld.relativeTo == PageAuthorType) {
                  copyrightBack->placement.setValue(LeftInside,PageType);
                  plPage.appendRelativeTo(copyrightBack);
                  plPage.placeRelative(copyrightBack);
              }
          } else {
              plPage.appendRelativeTo(copyrightBack);
              plPage.placeRelative(copyrightBack);
          }
          copyrightBack->setPos(copyrightBack->loc[XX],copyrightBack->loc[YY]);
      }

      // Url (Back Cover) Placement //~~~~~~~~~~~~~~~~
      if (displayUrlBack) {
          urlBackPld = urlBack->placement.value();
          if (urlBackPld.relativeTo == PageCopyrightType) {
              if (displayCopyrightBack && !breakCopyrightBackRelativeTo){
                  copyrightBack->appendRelativeTo(urlBack);
                  copyrightBack->placeRelative(urlBack);
              } else if (urlBackPld.relativeTo == PageCopyrightType) {
                  urlBack->placement.setValue(LeftInside,PageType);
                  plPage.appendRelativeTo(urlBack);
                  plPage.placeRelative(urlBack);
              }
          } else {
              plPage.appendRelativeTo(urlBack);
              plPage.placeRelative(urlBack);
          }
          urlBack->setPos(urlBack->loc[XX],urlBack->loc[YY]);
      }

      // Email (Back Cover) Placement //~~~~~~~~~~~~~~~~
      if (displayEmailBack) {
          emailBackPld = emailBack->placement.value();
          if (emailBackPld.relativeTo == PageURLType) {
              if (displayUrlBack && !breakURLBackRelativeTo){
                  urlBack->appendRelativeTo(emailBack);
                  urlBack->placeRelative(emailBack);
              } else if (emailBackPld.relativeTo == PageURLType) {
                  emailBack->placement.setValue(RightInside,PageType);
                  plPage.appendRelativeTo(emailBack);
                  plPage.placeRelative(emailBack);
              }
          } else {
              plPage.appendRelativeTo(emailBack);
              plPage.placeRelative(emailBack);
          }
          emailBack->setPos(emailBack->loc[XX],emailBack->loc[YY]);
      }

      // Disclaimer (Back Cover) Placement //~~~~~~~~~~~~~~~~
      if (displayDisclaimerBack) {                                 // display attribute On ?
          if (disclaimerBackPld.relativeTo == PageEmailType) {     // default relativeTo ?
              if (displayEmailBack && !breakEmailBackRelativeTo) { // display and not break default relativeTo ?
                  emailBack->appendRelativeTo(disclaimerBack);
                  emailBack->placeRelative(disclaimerBack);
              } else {                                             // break or no display default relativeTo ?
                  disclaimerBack->placement.setValue(RightInside,PageType);
                  plPage.appendRelativeTo(disclaimerBack);
                  plPage.placeRelative(disclaimerBack);
              }
          } else {                                                 // // placement changed so place as is
              plPage.appendRelativeTo(disclaimerBack);
              plPage.placeRelative(disclaimerBack);
          }
          disclaimerBack->setPos(disclaimerBack->loc[XX],disclaimerBack->loc[YY]);
      }

      // Plug (Back Cover) Placement //~~~~~~~~~~~~~~~~
      if (displayPlugBack) {
          if (plugBackPld.relativeTo == PageDisclaimerType) {
              if (displayDisclaimerBack && !breakDisclaimerBackRelativeTo){
                  disclaimerBack->appendRelativeTo(plugBack);
                  disclaimerBack->placeRelative(plugBack);
              } else {
                  plugBack->placement.setValue(RightInside,PageType);
                  plPage.appendRelativeTo(plugBack);
                  plPage.placeRelative(plugBack);
              }
          } else {
              plPage.appendRelativeTo(plugBack);
              plPage.placeRelative(plugBack);
          }
          plugBack->setPos(plugBack->loc[XX],plugBack->loc[YY]);
      }

      // DocumentLogoBack (Back Cover) Placement //~~~~~~~~~~~~~~~~
      QFileInfo fileInfo;
      if (displayDocumentLogoBack) {
          qreal picScale   = double(page->meta.LPub.page.documentLogoBack.picScale.value());
          fileInfo.setFile(getFilePath(page->meta.LPub.page.documentLogoBack.file.value()));
          if (fileInfo.exists()) {
              QPixmap qpixmap;
              qpixmap.load(fileInfo.absoluteFilePath());
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
              if (pld.relativeTo == PageHeaderType) {                              // Default placement
                  pageHeader->appendRelativeTo(pixmapLogoBack);
                  pageHeader->placeRelative(pixmapLogoBack, margin);
                  pixmapLogoBack->relativeToSize[0] = pageHeader->size[XX];
                  pixmapLogoBack->relativeToSize[1] = pageHeader->size[YY];
              } else if (pld.relativeTo == PageFooterType) {                       // Alternate placement
                  pageFooter->appendRelativeTo(pixmapLogoBack);
                  pageFooter->placeRelative(pixmapLogoBack, margin);
                  pixmapLogoBack->relativeToSize[0] = pageFooter->size[XX];
                  pixmapLogoBack->relativeToSize[1] = pageFooter->size[YY];
              } else {
                  plPage.appendRelativeTo(pixmapLogoBack);
                  plPage.placeRelative(pixmapLogoBack, margin);
                  pixmapLogoBack->relativeToSize[0] = plPage.size[XX];
                  pixmapLogoBack->relativeToSize[1] = plPage.size[YY];
              }
              pixmapLogoBack->setPos(pixmapLogoBack->loc[XX],pixmapLogoBack->loc[YY]);
          }
      }

      // PlugImage (Back Cover) Placement //~~~~~~~~~~~~~~~~
      if (displayPlugImageBack) {
          qreal picScale   = double(page->meta.LPub.page.plugImage.picScale.value());
          fileInfo.setFile(getFilePath(page->meta.LPub.page.plugImage.file.value()));
          if (fileInfo.exists()) {
              QPixmap qpixmap;
              qpixmap.load(fileInfo.absoluteFilePath());
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
              if (displayPlugBack && pld.relativeTo == PagePlugType) {
                  plugBack->appendRelativeTo(pixmapPlugImageBack);
                  plugBack->placeRelative(pixmapPlugImageBack, margin);
                  pixmapPlugImageBack->relativeToSize[0] = plugBack->size[XX];
                  pixmapPlugImageBack->relativeToSize[1] = plugBack->size[YY];
              } else if (pld.relativeTo == PageFooterType) {
                  pageFooter->appendRelativeTo(pixmapPlugImageBack);
                  pageFooter->placeRelative(pixmapPlugImageBack, margin);
                  pixmapPlugImageBack->relativeToSize[0] = pageFooter->size[XX];
                  pixmapPlugImageBack->relativeToSize[1] = pageFooter->size[YY];
              } else {
                  plPage.appendRelativeTo(pixmapPlugImageBack);
                  plPage.placeRelative(pixmapPlugImageBack, margin);
                  pixmapPlugImageBack->relativeToSize[0] = plPage.size[XX];
                  pixmapPlugImageBack->relativeToSize[1] = plPage.size[YY];
              }
              pixmapPlugImageBack->setPos(pixmapPlugImageBack->loc[XX],pixmapPlugImageBack->loc[YY]);
          }
      }
  }
  return 0;
}

QString Gui::getFilePath(const QString &fileName) const
{
    QFileInfo fileInfo(fileName);
    QString path;
    if (!gui->getCurFile().isEmpty())
        path = QFileInfo(gui->getCurFile()).absolutePath();
    // current path
    if (QFileInfo(path + QDir::separator() + fileInfo.fileName()).isFile()) {
        fileInfo = QFileInfo(path + QDir::separator() + fileInfo.fileName());
    } else
    // file path
    if (QFileInfo(fileInfo.filePath()).isFile()){
        fileInfo = QFileInfo(fileInfo.filePath());
    }
    return fileInfo.absoluteFilePath();
}
