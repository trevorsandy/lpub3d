 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2024 Trevor SANDY. All rights reserved.
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

  QString toolTip(QObject::tr("Page background [%1 x %2 px] - right-click to modify")
                  .arg(width)
                  .arg(height));

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
  // Note: we can precise this more if needed with DisplayType
  bool fullContextMenu = page->list.size() && page->displayPage == DT_DEFAULT;
  QString name = "Page";

  // figure out if first step step number is greater than 1

  Step *lastStep     = nullptr;
  Step *firstStep    = nullptr;
  int maxSteps       = 0;
  int lastStepNumber = 0;

  QAction *addNextStepAction         = nullptr;
  QAction *addNextStepsAction        = nullptr;
  QAction *addPrevStepAction         = nullptr;
  QAction *calloutAction             = nullptr;
  QAction *calloutNoPointAction      = nullptr;
  QAction *assembledAction           = nullptr;
  QAction *ignoreAction              = nullptr;
  QAction *partAction                = nullptr;
  QAction *refreshPageCacheAction    = nullptr;
  if (fullContextMenu) {
      AbstractStepsElement *range = page->list[page->list.size()-1];
      if (range->relativeType == RangeType) {
          AbstractRangeElement *rangeElement = range->list[range->list.size()-1];
          if (rangeElement->relativeType == StepType) {
              lastStep = dynamic_cast<Step *> (rangeElement);
              lastStepNumber = lastStep->stepNumber.number;
              maxSteps = numSteps(lastStep->topOfStep().modelName);
              if (lastStepNumber != maxSteps) {
                  addNextStepAction  = lpub->getAct("addNextStepAction.1");
                  commonMenus.addAction(addNextStepAction,menu);
              }
              if ((maxSteps - lastStepNumber) >= 2) {
                  addNextStepsAction = lpub->getAct("addNextStepsAction.1");
                  commonMenus.addAction(addNextStepsAction,menu);
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
                  addPrevStepAction  = lpub->getAct("addPrevStepAction.1");
                  commonMenus.addAction(addPrevStepAction,menu);
              }
          }
      }

      if (page->meta.submodelStack.size() > 0) {
          calloutAction              = lpub->getAct("calloutAction.1");
          commonMenus.addAction(calloutAction,menu);

          calloutNoPointAction       = lpub->getAct("calloutNoPointAction.1");
          commonMenus.addAction(calloutNoPointAction,menu);

          // FIXME: don't allow this if it already got an assembled.
          if (canConvertToCallout(&page->meta)) {
              assembledAction        = lpub->getAct("assembledAction.1");
              commonMenus.addAction(assembledAction,menu);
          }

          ignoreAction               = lpub->getAct("ignoreAction.1");
          commonMenus.addAction(ignoreAction,menu);

          partAction                 = lpub->getAct("partAction.1");
          commonMenus.addAction(partAction,menu);
      }
  }

  QAction *perStepAction             = nullptr;
  if (page->instances > 1) {
      // present the opposite action
      if (!page->meta.LPub.stepPli.perStep.value()) {
          perStepAction              = lpub->getAct("partsListAction.1");
      } else {
          perStepAction              = lpub->getAct("noPartsListAction.1");
      }
      commonMenus.addAction(perStepAction,menu,name);
  }

  QAction *borderAction              = nullptr;
  if(page->meta.LPub.page.background.value().type == BackgroundData::BgTransparent) {
      borderAction                   = lpub->getAct("borderAction.1");
      commonMenus.addAction(borderAction,menu,name);
  }

  QAction *changeImageAction         = nullptr;
  QAction *displayImageAction        = nullptr;
  if (page->frontCover) {
      if (page->meta.LPub.page.coverImage.file.value().isEmpty()) {
          changeImageAction          = lpub->getAct("changeImageAction.1");
          changeImageAction->setText(QObject::tr("Add Cover Page Image"));
          commonMenus.addAction(changeImageAction,menu,name);
      } else if (!page->meta.LPub.page.coverImage.display.value()) {
          displayImageAction         = lpub->getAct("displayImageAction.1");
          displayImageAction->setText(QObject::tr("Show Cover Page Image"));
          commonMenus.addAction(displayImageAction,menu,name);
      }
  }

  QAction *backgroundAction          = lpub->getAct("backgroundAction.1");
  commonMenus.addAction(backgroundAction,menu,name);

  QAction *subModelColorAction       = lpub->getAct("subModelColorAction.1");
  commonMenus.addAction(subModelColorAction,menu,name);

  QAction *sizeAndOrientationAction  = lpub->getAct("sizeAndOrientationAction.1");
  commonMenus.addAction(sizeAndOrientationAction,menu);

  if (fullContextMenu) {
      refreshPageCacheAction           = lpub->getAct("refreshPageCacheAction.1");
      commonMenus.addAction(refreshPageCacheAction,menu,name);
  }

  QAction *selectedAction            = menu.exec(event->screenPos());

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

  if (fullContextMenu) {
      if (selectedAction == calloutAction) {
        convertToCallout(&page->meta, page->bottom.modelName, page->isMirrored, false);
      } else if (selectedAction == calloutNoPointAction) {
        convertToCallout(&page->meta, page->bottom.modelName, page->isMirrored, false, true);
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
        int numOfSteps = QInputDialog::getInt(gui,QObject::tr("Next Steps"),QObject::tr("Number of next steps"),maxNextSteps,1,maxNextSteps,1,&ok);
        if (ok) {
            if (numOfSteps > 1)
                addNextStepsMultiStep(lastStep->topOfSteps(),lastStep->bottomOfSteps(),numOfSteps);
            else
                addNextMultiStep(lastStep->topOfSteps(),lastStep->bottomOfSteps());
        }
      } else if (selectedAction == addPrevStepAction) {
        addPrevMultiStep(firstStep->topOfSteps(),firstStep->bottomOfSteps());
      } else if (selectedAction == refreshPageCacheAction) {
        clearPageCache(relativeType,page,Options::MON/*Clear All Page Images*/);
      }
  }

  if (selectedAction == perStepAction) {
      changeBool(page->top,
                 page->bottom,
                 &page->meta.LPub.stepPli.perStep,true,0,false,false);
    } else if (selectedAction == borderAction) {
      changeBorder(QObject::tr("Border"),
                   page->top,
                   page->bottom,
                   &page->meta.LPub.page.border);
    } else if (selectedAction == backgroundAction) {
      changeBackground(QObject::tr("Page Background"),
                       page->top,
                       page->bottom,
                       &page->meta.LPub.page.background, useTop);
    } else if (selectedAction == changeImageAction) {

      changeImage(QObject::tr("Add Cover Page Image"),
                   page->top,
                   page->bottom,
                   &page->meta.LPub.page.coverImage.file,
                    true,1,true/*allowLocal*/,false/*askLocal*/);
    } else if (selectedAction == subModelColorAction) {
      changeSubModelColor(QObject::tr("Submodel Color"),
                          page->top,
                          page->bottom,
                          &page->meta.LPub.page.subModelColor,useTop);

    } else if (selectedAction == sizeAndOrientationAction) {
      changeSizeAndOrientation(QObject::tr("Size and Orientation"),
                               page->top,
                               page->bottom,
                               &page->meta.LPub.page.size,
                               &page->meta.LPub.page.orientation, useTop);
    } else if (selectedAction == displayImageAction) {
      changeBool(page->top,
                 page->bottom,
                &page->meta.LPub.page.coverImage.display,
                 true,1,true/*allowLocal*/,false/*askLocal*/);
    }
}
