 
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

/****************************************************************************
 *
 * This file represents the page background and is derived from the generic
 * background class described in background.(h,cpp)
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "pagebackgrounditem.h"
#include <QAction>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include <QFileDialog>
#include "commonmenus.h"
#include "ranges_element.h"
#include "range.h"
#include "range_element.h"
#include "step.h"
#include "lpub.h"
#include "lpub_preferences.h"

PageBackgroundItem::PageBackgroundItem(
  Page   *_page,
  int     width,
  int     height,
  bool    _exporting)
{
  page = _page;

  relativeType = page->relativeType;

  pixmap = new QPixmap(width,height);

  QString toolTip("Page background - right-click to modify");

  setBackground(pixmap,
                PageType,
               &page->meta,
                page->meta.LPub.page.background,
                page->meta.LPub.page.border,
                page->meta.LPub.page.margin,
                page->meta.LPub.page.subModelColor,
                page->meta.submodelStack.size(),
                toolTip,
                _exporting);

  setPixmap(*pixmap);
  setFlag(QGraphicsItem::ItemIsSelectable,false);
  setFlag(QGraphicsItem::ItemIsMovable,false);
  setData(ObjectId, PageBackgroundObj);
  setZValue(PAGEBACKGROUND_ZVALUE_DEFAULT);

  delete pixmap;
}

void PageBackgroundItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QString name = "Page";
  bool fullContextMenu = page->list.size() && ! page->modelDisplayOnlyStep;

  // figure out if first step step number is greater than 1

  QAction *addNextStepAction    = nullptr;
  QAction *addNextStepsAction   = nullptr;
  QAction *addPrevStepAction    = nullptr;
  QAction *calloutAction        = nullptr;
  QAction *assembledAction      = nullptr;
  QAction *ignoreAction         = nullptr;
  QAction *partAction           = nullptr;
  QAction *perStepAction        = nullptr;
  QAction *clearPageCacheAction = nullptr;

  QAction *borderAction         = nullptr;
  QAction *backgroundAction     = nullptr;
  QAction *subModelColorAction  = nullptr;
  QAction *imageAction          = nullptr;
  QAction *displayImageAction   = nullptr;

  QAction *sizeAndOrientationAction = nullptr;

  Step    *lastStep = nullptr;
  Step    *firstStep = nullptr;

  int maxSteps = 0;
  int lastStepNumber = 0;
  if (fullContextMenu) {
      AbstractStepsElement *range = page->list[page->list.size()-1];
      if (range->relativeType == RangeType) {
          AbstractRangeElement *rangeElement = range->list[range->list.size()-1];
          if (rangeElement->relativeType == StepType) {
              lastStep = dynamic_cast<Step *> (rangeElement);
              lastStepNumber = lastStep->stepNumber.number;
              maxSteps = numSteps(lastStep->topOfStep().modelName);
              if (lastStepNumber != maxSteps) {
                  addNextStepAction = menu.addAction("Add Next Step");
                  addNextStepAction->setIcon(QIcon(":/resources/nextstep.png"));
                  addNextStepAction->setWhatsThis("Add Next Step:\n Add the first step of the next page to this page\n");
                }
              if ((maxSteps - lastStepNumber) >= 2) {
                  addNextStepsAction = menu.addAction("Add Next Steps");
                  addNextStepsAction->setIcon(QIcon(":/resources/nextsteps.png"));
                  addNextStepsAction->setWhatsThis("Add Next Steps:\n Add a specified number of next steps to this page\n");
              }
            }
        }

      // figure out if first step step number is greater than 1

      range = page->list[0];
      if (range->relativeType == RangeType) {
          AbstractRangeElement *rangeElement = range->list[0];
          if (rangeElement->relativeType == StepType) {
              firstStep = dynamic_cast<Step *> (rangeElement);
              if (firstStep->stepNumber.number > 1) {
                  addPrevStepAction = menu.addAction("Add Previous Step");
                  addPrevStepAction->setIcon(QIcon(":/resources/previousstep.png"));
                  addPrevStepAction->setWhatsThis("Add Previous Step:\n Add the last step of the previous page to this page\n");
                }
            }
        }

      if (page->meta.submodelStack.size() > 0) {
          calloutAction = menu.addAction("Convert to Callout");
          calloutAction->setIcon(QIcon(":/resources/convertcallout.png"));
          calloutAction->setWhatsThis("Convert to Callout:\n"
                                      "  A callout shows how to build these steps in a picture next\n"
                                      "  to where it is added to the set you are building");

          // FIXME: don't allow this if it already got an assembled.
          if (canConvertToCallout(&page->meta)) {
              assembledAction = menu.addAction("Add Assembled Image to Parent Page");
              assembledAction->setIcon(QIcon(":/resources/addassembledimage.png"));
              assembledAction->setWhatsThis("Add Assembled Image to Parent Page\n"
                                            "  A callout like image is added to the page where this submodel\n"
                                            "  is added to the set you are building");
            }

          ignoreAction = menu.addAction("Ignore this submodel");
          ignoreAction->setIcon(QIcon(":/resources/ignoresubmodel.png"));
          ignoreAction->setWhatsThis("Stops these steps from showing up in your instructions");

          partAction = menu.addAction("Treat as Part");
          partAction->setIcon(QIcon(":/resources/treataspart.png"));
          partAction->setWhatsThis("Treating this submodel as a part means these steps go away, "
                                   "and the submodel is displayed as a part in the parent step's "
                                   "part list image.");
        }
      clearPageCacheAction = menu.addAction("Reset Page Assembly Image Cache");
      clearPageCacheAction->setIcon(QIcon(":/resources/clearpagecache.png"));
      clearPageCacheAction->setWhatsThis("Reset the CSI image and ldr cache files for this page.");
    }

  if (page->instances > 1) {
      QString m_name = "Step";
      if (page->meta.LPub.stepPli.perStep.value()) {
          perStepAction = commonMenus.noPartsList(menu,m_name);
        } else {
          perStepAction = commonMenus.partsList(menu,m_name);
        }
    }

  backgroundAction    = commonMenus.backgroundMenu(menu,name);
  subModelColorAction = commonMenus.subModelColorMenu(menu,name);

  if (page->frontCover) {

      if (page->meta.LPub.page.coverImage.file.value().isEmpty()){
          imageAction     = commonMenus.changeImageMenu(menu,"Add Cover " + name + " Image");
      } else if (!page->meta.LPub.page.coverImage.display.value()) {
          displayImageAction = commonMenus.displayMenu(menu,name);
          displayImageAction->setText("Show Cover " + name + " Image");
      }
  }

  sizeAndOrientationAction = menu.addAction("Change Page Size or Orientation");
  sizeAndOrientationAction->setIcon(QIcon(":/resources/pagesizeandorientation.png"));
  sizeAndOrientationAction->setWhatsThis("Change the page size and orientation");

  if(page->meta.LPub.page.background.value().type == BackgroundData::BgTransparent) {
      borderAction = commonMenus.borderMenu(menu,name);
    }

  QAction *selectedAction     = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
      return;
    }

  bool useTop = relativeType == SingleStepType;

  if (page->meta.LPub.page.background.value().gsize[0] == 0.0 &&
      page->meta.LPub.page.background.value().gsize[1] == 0.0) {

      page->meta.LPub.page.background.value().gsize[0] = Preferences::pageHeight;
      page->meta.LPub.page.background.value().gsize[1] = Preferences::pageWidth;

      QSize gSize(int(page->meta.LPub.page.background.value().gsize[0]),
                  int(page->meta.LPub.page.background.value().gsize[1]));
      int h_off = gSize.width() / 10;
      int v_off = gSize.height() / 8;
      page->meta.LPub.page.background.value().gpoints << QPointF(gSize.width() / 2, gSize.height() / 2)
                                                      << QPointF(gSize.width() / 2 - h_off, gSize.height() / 2 - v_off);

    }

  if (fullContextMenu){
      if (selectedAction == calloutAction) {
          convertToCallout(&page->meta, page->bottom.modelName, page->isMirrored, false);
        } else if (selectedAction == assembledAction) {
          convertToCallout(&page->meta, page->bottom.modelName, page->isMirrored, true);
        } else if (selectedAction == ignoreAction) {
          convertToIgnore(&page->meta);
        } else if (selectedAction == partAction) {
          convertToPart(&page->meta);
        } else if (selectedAction == addNextStepAction) {
          addNextMultiStep(lastStep->topOfSteps(),lastStep->bottomOfSteps());
        } else if (selectedAction == addNextStepsAction) {
          bool ok;
          int maxNextSteps = maxSteps - lastStepNumber;
          int numOfSteps = QInputDialog::getInt(gui,"Next Steps","Number of next steps",maxNextSteps,1,maxNextSteps,1,&ok);
          if (ok)
              addNextStepsMultiStep(lastStep->topOfSteps(),lastStep->bottomOfSteps(),numOfSteps);
        } else if (selectedAction == addPrevStepAction) {
          addPrevMultiStep(firstStep->topOfSteps(),firstStep->bottomOfSteps());
        } else if (selectedAction == clearPageCacheAction){
          gui->clearPageCSICache(relativeType,page);
        }
    }

  if (selectedAction == perStepAction) {
      changeBool(page->top,
                 page->bottom,
                 &page->meta.LPub.stepPli.perStep,true,0,false,false);
    } else if (selectedAction == borderAction) {
      changeBorder("Border",
                   page->top,
                   page->bottom,
                   &page->meta.LPub.page.border);
    } else if (selectedAction == backgroundAction) {
      changeBackground("Page Background",
                       page->top,
                       page->bottom,
                       &page->meta.LPub.page.background, useTop);
    } else if (selectedAction == imageAction) {

      changeImage("Add Cover" + name + " Image",
                   page->top,
                   page->bottom,
                   &page->meta.LPub.page.coverImage.file,
                    true,1,true/*allowLocal*/,false/*askLocal*/);
    } else if (selectedAction == subModelColorAction) {
      changeSubModelColor("Submodel Color",
                          page->top,
                          page->bottom,
                          &page->meta.LPub.page.subModelColor,useTop);

    } else if (selectedAction == sizeAndOrientationAction) {
      changeSizeAndOrientation("Size and Orientation",
                               page->top,
                               page->bottom,
                               &page->meta.LPub.page.size,
                               &page->meta.LPub.page.orientation, useTop);
    } else if (selectedAction == displayImageAction){
      changeBool(page->top,
                 page->bottom,
                &page->meta.LPub.page.coverImage.display,
                 true,1,true/*allowLocal*/,false/*askLocal*/);

  }

}
