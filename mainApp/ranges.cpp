
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
 * This class contains one or more individual range's.
 * By itself, this class represents step groups.  Callouts are derived
 * from ranges.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "ranges.h"
#include "ranges_item.h"
#include "range_element.h"
#include "range.h"
#include "step.h"
#include "meta.h"
#include "callout.h"
#include "lpub.h"
#include "dividerdialog.h"

#include "render.h"
#include "resize.h"
#include "submodelitem.h"
#include "rotateiconitem.h"
#include "pointer.h"
#include "pagepointeritem.h"

Steps::Steps()
{
  relativeType   = SingleStepType;
  pli.steps      = this;
  subModel.steps = this;
  isMirrored     = false;
}

Steps::Steps(Meta &_meta,QGraphicsView *_view)
{
  meta = _meta;
  view = _view;
}

Steps::~Steps()
{
  for (int i = 0; i < list.size(); i++) {
    delete list[i];
  }
  list.clear();
}

QString Steps::modelName()
{
  if (list.size()) {
    return list[0]->topOfRange().modelName;
  } else {
    return "Problems";
  }
}

QString Steps::path()
{
  QString thePath;
  for (int i = 0; i < meta.submodelStack.size(); i++) {
    thePath += "/" + QFileInfo(meta.submodelStack[i].modelName).completeBaseName();
  }
  thePath += "/" + QFileInfo(modelName()).completeBaseName();
  return thePath;
}

QString Steps::csiName()
{
  QString thePath;

  for (int i = 0; i < meta.submodelStack.size(); i++) {
    QString lineNum = QString("%1") .arg(meta.submodelStack[i].stepNumber);
    thePath += QFileInfo(meta.submodelStack[i].modelName).completeBaseName() + "_" + lineNum + "_";
  } 
  thePath += QFileInfo(modelName()).completeBaseName();

  return thePath;
}

const Where &Steps::bottomOfStep(
  AbstractStepsElement *me)
{
  for (int i = 0; i < list.size(); i++) {
    if (list[i] == me) {
      if (i < list.size()-1) {
        return list[i+1]->topOfRange();
      }
    }
  }
  return bottom;
}

AbstractStepsElement *Steps::nextRange(
  const AbstractStepsElement *me)
{
  int size = list.size();
  
  for (int i = 0; i < size; i++) {
    if (list[i] == me) {
      if (i < size - 1) {
        return list[i+1];
      } else {
        return nullptr;
      }
    }
  }
  return nullptr;
}
  

const Where &Steps::topOfSteps()
{
  if (list.size() == 0) {
    return top;
  } else {
    return list[0]->topOfRange();
  }
}
const Where &Steps::bottomOfSteps()
{
  return bottom;
}
void Steps::setTopOfSteps(const Where &tos)
{
  top = tos;
}
void Steps::setBottomOfSteps(const Where &bos)
{
  bottom = bos;
}

QStringList Steps::submodelStack()
{
  QStringList submodelStack;
  SubmodelStack filename;
  foreach (filename,meta.submodelStack) {
    submodelStack << filename.modelName;
  }
  return submodelStack;
}

void Steps::append(AbstractStepsElement *re)
{
  list.append(re);
}

AllocEnc Steps::allocType()
{
  return meta.LPub.multiStep.alloc.value();
}

AllocMeta &Steps::allocMeta()
{
  return meta.LPub.multiStep.alloc;
}
/*********************************************
 *
 * ranges.h function
 *
 * ranges.h
 *
 * ranges are used for two reasons
 *   1) single step/multi step pages
 *   2) callouts
 * in both cases, ranges contain a list of individual ranges (Range *)
 *
 ********************************************/

/* This destroys everything in its list, but not itself. */

void Steps::freeSteps()
{
  for (int i = 0; i < list.size(); i++) {
    AbstractStepsElement *re = list[i];
    delete re;
  }
  list.clear();
  textItemList.clear();
  relativeType = SingleStepType;
  relativeToList.clear();
}

void Steps::sizeIt(void)
{
  FreeFormData freeFormData;
  AllocEnc     allocEnc;
  if (relativeType == CalloutType) {
    freeFormData = meta.LPub.callout.freeform.value();
    allocEnc = meta.LPub.callout.alloc.value();
  } else {
    freeFormData = meta.LPub.multiStep.freeform.value();
    allocEnc = meta.LPub.multiStep.alloc.value();
  }

  if (freeFormData.mode) {
    if (allocEnc == Vertical) {
      sizeitFreeform(XX,YY);
    } else {
      sizeitFreeform(YY,XX);
    }
  } else {
    if (allocEnc == Vertical) {
      sizeit(allocEnc,XX,YY);
    } else {
      sizeit(allocEnc,YY,XX);
    }
  }
}

/*
 * This provides Vertical packing
 */

void Steps::sizeit(AllocEnc allocEnc, int x, int y)
{
  /* if a single step, then skip the step number */
  
  if (relativeType == CalloutType && list.size() == 1) {
    if (list[0]->relativeType == RangeType) {
      Range *range = dynamic_cast<Range *>(list[0]);
      if (range && range->list.size() == 1) {
        if (range->list[0]->relativeType == StepType) {
          Step *step = dynamic_cast<Step *>(range->list[0]);
          step->stepNumber.number = -1;
        }
      }
    }
  }

  /*
   * Size each range, determining its width and height
   * accumulating its width, and finding the maximum height
   */

  loc[XX] = 0;
  loc[YY] = 0;
  size[XX] = 0;
  size[YY] = 0;
  
  /* foreach range */

  for (int i = 0; i < list.size(); i++) {
    if (list[i]->relativeType == RangeType) {
      Range *range = dynamic_cast<Range *>(list[i]);
      if (range) {

        if (allocEnc == Vertical) {
          range->sizeitVert();
        } else {
          range->sizeitHoriz();
        }
        
        SepData divider = range->sepMeta.valuePixels();

        /* find the tallest (if vertical alloc) / widest (if horizontal alloc) range */

        bool usePageSize = false;
        bool useCustomSize = false;
        if (range->size[y] > size[y]) {
          int i = range->list.size() - 1; // check last step in range fro RangeDivider
          Step *lastStep = dynamic_cast<Step *>(range->list[i]);
          if (lastStep && lastStep->dividerType == RangeDivider)  {// divider found
              usePageSize = divider.type == SepData::LenPage;
              useCustomSize = divider.type == SepData::LenCustom;
          }
          if (usePageSize)
            size[y] = pageSize(y,true);
          else if (useCustomSize)
            size[y] = int(divider.length);
          else
            size[y] = range->size[y];
        }

        /* add range divider size adjustment */

        if (! usePageSize && !useCustomSize)
          size[y] += range->sizeRangeDividers(y);

        /* place each range Horizontally (if vertical alloc) / Vertically (if horizontal alloc) */

        range->loc[y] = 0;
        range->loc[x] = size[x];

        size[x] += range->size[x];

        if (i + 1 < list.size()) {

          /* accumulate total width (if vertical alloc) / height (if horizontal alloc) of ranges */

          size[x] += 2*divider.margin[x] + divider.thickness;
        }
      }
    }
  }

  /*
   * Each range is placed, but now we need to
   * evenly space the steps within a range
   */

  for (int i = 0; i < list.size(); i++) {
    if (list[i]->relativeType == RangeType) {
      Range *range = dynamic_cast<Range *>(list[i]);
      if (range) {

        /* space steps within column based on tallest column */

        range->placeit(size[y],x,y);
      }
    }
  }

  /*
   * So now that we know our size, we can consider the callout
   * that are placed relative to us
   */

  setBoundingSize();

  if (relativeType == CalloutType) {
    int newLoc[2] = { 0, 0 };
    for (int i = 0; i < list.size(); i++) {
      if (list[i]->relativeType == RangeType) {
        Range *range = dynamic_cast<Range *>(list[i]);
        if (range) {
          for (int j = 0; j < range->list.size(); j++) {
            Step *step = dynamic_cast<Step *>(range->list[j]);
            if (step) {
              int nCallouts = step->list.size();
              for (int k = 0; k < nCallouts; k++) {
                Callout *callout = dynamic_cast<Callout *>(step->list[k]);
                if (callout) {
                  if (callout->placement.value().relativeTo == CalloutType) {
                    // We need to place this callout relative to the
                    // current bounding box, and eventually figure out a
                    // new bounding box.
                    int margins[2] = { step->margin.valuePixels(XX) +
                                       range->margin.valuePixels(XX),
                                       step->margin.valuePixels(YY) +
                                       range->margin.valuePixels(YY)};
                    placeRelative(callout,margins);
                    if (callout->loc[XX] < newLoc[XX]) {
                      newLoc[XX] = callout->loc[XX] - callout->margin.valuePixels(XX);
                    }
                    if (callout->loc[YY] < newLoc[YY]) {
                      newLoc[YY] = callout->loc[YY] - callout->margin.valuePixels(YY);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    if (newLoc[XX] < 0 || newLoc[YY] < 0) {
      for (int i = 0; i < list.size(); i++) {
        if (list[i]->relativeType == RangeType) {
          Range *range = dynamic_cast<Range *>(list[i]);
          if (range) {
            range->loc[XX] -= newLoc[XX];
            range->loc[YY] -= newLoc[YY];
            for (int j = 0; j < range->list.size(); j++) {
              Step *step = dynamic_cast<Step *>(range->list[j]);
              if (step) {
                int nCallouts = step->list.size();
                for (int k = 0; k < nCallouts; k++) {
                  Callout *callout = dynamic_cast<Callout *>(step->list[k]);
                  if (callout) {
                    if (callout->placement.value().relativeTo == CalloutType) {
                      callout->loc[XX] -= newLoc[XX];
                      callout->loc[YY] -= newLoc[YY];
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    size[0] = boundingSize[0];
    size[1] = boundingSize[1];
  }
}

void Steps::sizeitFreeform(
  int xx,
  int yy)
{
  if (relativeType == CalloutType && list.size() == 1) {
    if (list[0]->relativeType == RangeType) {
      Range *range = dynamic_cast<Range *>(list[0]);
      if (range && range->list.size() == 1) {
        if (range->list[0]->relativeType == StepType) {
          Step *step = dynamic_cast<Step *>(range->list[0]);
          step->stepNumber.number = -1;
        }
      }
    }
  }

  /*
   * Size each range, determining its width and height
   * accumulating its width, and finding the maximum height
   */

  loc[XX] = 0;
  loc[YY] = 0;
  size[xx] = 0;
  size[yy] = 0;

  int lastMargin = 0;

  /* foreach range */

  for (int i = 0; i < list.size(); i++) {
    if (list[i]->relativeType == RangeType) {
      Range *range = dynamic_cast<Range *>(list[i]);
      if (range) {

//      range->sizeitFreeform(xx,yy,freeform.base,freeform.justification);

        /* find the tallest range */

        if (range->size[yy] > size[yy]) {
          size[yy] = range->size[yy];
        }

        /* place each range Horizontally */

        range->loc[yy] = 0;
        range->loc[xx] = size[xx];

        if (relativeType == StepGroupType) {
          Range *realRange = dynamic_cast<Range *>(range);
          realRange->loc[xx] += realRange->leftAdjust;
        }

        /* accumulate total width of ranges */

        lastMargin = range->margin.valuePixels(xx);
        size[xx] += range->size[xx] + lastMargin;
      }
    }
  }
  size[xx] -= lastMargin;

  /*
   * Each range is placed, but now we need to
   * evenly space the steps within a range
   */
  for (int i = 0; i < list.size(); i++) {
    if (list[i]->relativeType == RangeType) {
      Range *range = dynamic_cast<Range *>(list[i]);
      if (range) {

        /* space steps within column based on tallest column */

//      range->placeitFreeform(xx,yy,size[yy],freeform.justification);
      }
    }
  }
}

/*
 * This is used only by step groups
 */

void Steps::addGraphicsItems(
  int ox,
  int oy,
  QGraphicsItem *parent /*pageBg*/)
{
  QGraphicsItem *backDrop;
  QRectF rect = QRectF(ox + loc[XX], oy + loc[YY],size[XX],size[YY]);
  backDrop = new MultiStepRangesBackgroundItem(this,rect,parent,&meta);

  AllocEnc allocEnc;

  if (relativeType == CalloutType) {
    allocEnc = meta.LPub.callout.alloc.value();
  } else {
    allocEnc = meta.LPub.multiStep.alloc.value();
  }

  addGraphicsItems(allocEnc,ox,oy,backDrop);
}

/*
 * For MultiStep                Callout
 *       TransparentBackground    ColorBackground/Border
 * 
 *     Range
 *       TransparentRect
 */

void Steps::addGraphicsItems(
  AllocEnc allocEnc,
  int offsetX,
  int offsetY,
  QGraphicsItem *parent/*backdrop*/)
{
  // SN
  if (groupStepNumber.number) {
    groupStepNumber.addStepNumber(dynamic_cast<Page *>(this), parent);
  }

  // PLI
  if (pli.tsize() && ! pli.bom) {
    pli.addPli(meta.submodelStack.size(), parent);
  }

  // SM
  if (subModel.tsize()) {
    subModel.addSubModel(meta.submodelStack.size(), parent);
  }

//#ifdef QT_DEBUG_MODE
//  logDebug() << "\nSTEPS AddGraphicsItems OFFSET"
//             << " \nOffxetX [" << offsetX << "]"
//             << " \nOffxetY [" << offsetY << "]"
//                 ;
//#endif

  for (int i = 0; i < list.size(); i++) {
    if (list[i]->relativeType == RangeType) {
      Range *range = dynamic_cast<Range *>(list[i]);
      if (range) {
        if (relativeType == StepGroupType) {
          new MultiStepRangeBackgroundItem(this,range,&meta,
                    offsetX + loc[XX],
                    offsetY + loc[YY],
                    parent);
        }

        range->addGraphicsItems(
          offsetX + loc[XX], offsetY + loc[YY], &meta, relativeType, parent);

        // add range divider if exist here
        if (list.size() > 1 && i < list.size() - 1) {
          int size = range->list.size();
          if (size) {
            Step *step = dynamic_cast<Step *>(range->list[size-1]);
            if (step && step->dividerType == RangeDivider) {
              int oX = offsetX + loc[XX] + range->loc[XX];
              int oY = offsetY + loc[YY] + range->loc[YY];
//#ifdef QT_DEBUG_MODE
//                logDebug() << "\nRange Divider offset before alloc adjust for Step [" << step->stepNumber.number << "]:"
//                           << "\noX (pre Alloc) [" << oX << "] = offsetX" << offsetX << "+ loc[XX]" << loc[XX]  << "+ range->loc[XX]"  << range->loc[XX]
//                           << "\noY (pre Alloc) [" << oY << "] = offsetY" << offsetY << "+ loc[YY]" << loc[YY] << "+ range->loc[YY]" << range->loc[YY]
//                              ;
//#endif
              if (allocEnc == Vertical) {
//                int dbg_oX = oX;
                oX += range->size[XX];
//#ifdef QT_DEBUG_MODE
//                logDebug() << "\nRange Divider Vertical alloc (Left/Right view) offset adjust for Step [" << step->stepNumber.number << "]:"
//                           << "\noX Vertical  [" << oX << "] = oX" << dbg_oX << "+= range->size[XX]" << range->size[XX]
//                              ;
//#endif
              } else {
//                int dbg_oY = oY;
                oY += range->size[YY];
//#ifdef QT_DEBUG_MODE
//                logDebug() << "\nRange Divider Horizontal alloc (Top/Bottom view) offset adjust for Step [" << step->stepNumber.number << "]:"
//                           << "\noY Horizontal[" << oY << "] = oX" << dbg_oY << "+= range->size[YY]" << range->size[YY]
//                              ;
//#endif
              }
//#ifdef QT_DEBUG_MODE
//              logDebug() << "\nRange Divider - Ranges and Range Dimensions for Step [" << step->stepNumber.number << "]:"
//                         << "\nPage Size XX W        [" << gui->pageSize(meta.LPub.page,XX) << "]"
//                         << "\nPage Size YY H        [" << gui->pageSize(meta.LPub.page,YY) << "]"
//                         << "\nRanges::loc XX        [" << Steps::loc[XX] << "]"
//                         << "\nRanges::loc YY        [" << Steps::loc[YY] << "]"
//                         << "\nRanges::size XX W     [" << Steps::size[XX] << "]"
//                         << "\nRanges::size YY H     [" << Steps::size[YY] << "]"
//                         << "\nRange[" << i << "]::loc XX    [" << range->loc[XX] << "]"
//                         << "\nRange[" << i << "]::loc YY    [" << range->loc[YY] << "]"
//                         << "\nRange[" << i << "]::size XX W [" << range->size[XX] << "]"
//                         << "\nRange[" << i << "]::size YY H [" << range->size[YY] << "]"
//                         << "\noffsetX W             [" << offsetX << "] offsetX"
//                         << "\noffsetY H             [" << offsetY << "] offsetY"
//                         << "\noX (Divider)          [" << oX << "] = offsetX" << offsetX << " + ranges->loc[XX]" << Steps::loc[XX] << " + range->loc[XX]" << range->loc[XX]
//                         << "\noY (Divider)          [" << oY << "] = offsetY" << offsetY << " + ranges->loc[YY]" << Steps::loc[YY] << " + range->loc[YY]" << range->loc[YY]
//                         << "\n----------------"
//                         << "\nDivider::oX           [" << oX << "] = offsetX" << offsetX << " + ranges->loc[XX]" << Steps::loc[XX] << " + step->loc[XX]" << step->loc[XX]
//                         << "\nDivider::oY           [" << oY << "] = offsetY" << offsetY << " + ranges->loc[YY]" << Steps::loc[YY] << " + range->loc[YY]" << range->loc[YY]
//                            ;
//#endif
              DividerItem *divider = new DividerItem(step,&meta,oX,oY);
              divider->setParentItem(parent);
//#ifdef QT_DEBUG_MODE
//              logDebug() << "\nRange Divider - Dimensions for Step [" << step->stepNumber.number << "]:"
//                         << "\nDivider::loc XX    [" << divider->loc[XX] << "]"
//                         << "\nDivider::loc YY    [" << divider->loc[YY] << "]"
//                         << "\nDivider::size XX W [" << divider->size[XX] << "]"
//                         << "\nDivider::size YY H [" << divider->size[YY] << "]"
//                            ;
//#endif
              //   add divider pointers (if any) to the graphics scene
              for (int j = 0; j < range->rangeDividerPointerList.size(); j++) {
                  Pointer *pointer = range->rangeDividerPointerList[j];
                  divider->addGraphicsPointerItem(pointer,range->view);
              }
            }
          }
        }             // range divider

        // check steps for step divider
        for (int i = 0; i < range->list.size(); i++) {
          if (range->list[i]->relativeType == StepType) {
            Step *step = dynamic_cast<Step *>(range->list[i]);
            if (step && step->dividerType == StepDivider) {
              int oX,oY;
              if (allocEnc == Vertical) {
                oX = offsetX + loc[XX] + range->loc[XX];
                oY = offsetY + loc[YY] + step->loc[YY];
              } else {
                oX = offsetX + loc[XX] + step->loc[XX];
                oY = offsetY + loc[YY] + range->loc[YY];
              }
//#ifdef QT_DEBUG_MODE //int which  = allocType() == Vertical ? YY : XX;  // " <<  << " // " <<
//              logDebug() << "\nStep Divider - Ranges and Range Dimensions for Step [" << step->stepNumber.number << "]:"
//                         << "\nPage Size XX W       [" << gui->pageSize(meta.LPub.page,XX) << "]"
//                         << "\nPage Size YY H       [" << gui->pageSize(meta.LPub.page,YY) << "]"
//                         << "\nRanges::loc XX       [" << Steps::loc[XX] << "]"
//                         << "\nRanges::loc YY       [" << Steps::loc[YY] << "]"
//                         << "\nRanges::size XX W    [" << Steps::size[XX] << "]"
//                         << "\nRanges::size YY H    [" << Steps::size[YY] << "]"
//                         << (allocEnc == Vertical ? "\nStep[" : "\nRange[")
//                         << (allocEnc == Vertical ? i : step->stepNumber.number) << "]::loc XX   ["
//                         << (allocEnc == Vertical ? step->loc[XX] : range->loc[XX]) << "]"
//                         << (allocEnc == Vertical ? "\nStep[" : "\nRange[")
//                         << (allocEnc == Vertical ? i : step->stepNumber.number) << "]::loc YY   ["
//                         << (allocEnc == Vertical ? step->loc[YY] : range->loc[YY])  << "]"
//                         << "\nStep[" << step->stepNumber.number << "]::size XX W [" << step->size[XX] << "]"
//                         << "\nStep[" << step->stepNumber.number << "]::size YY H [" << step->size[YY] << "]"
//                         << "\noffsetX              [" << offsetX << "] offsetX"
//                         << "\noffsetY              [" << offsetY << "] offsetY"
//                         << "\nDivider::oX          [" << oX << "] = offsetX" << offsetX << "+ ranges->loc[XX]" << Steps::loc[XX] << "+ step->loc[XX]" << step->loc[XX]
//                         << "\nDivider::oY          [" << oY << "] = offsetY" << offsetY << "+ ranges->loc[YY]" << Steps::loc[YY] << "+ range->loc[YY]" << range->loc[YY]
//                            ;
//#endif
              DividerItem *divider = new DividerItem(step,&meta,oX,oY);
              divider->setParentItem(parent);
//#ifdef QT_DEBUG_MODE
//              logDebug() << "\nStep Divider - Dimensions for Step [" << step->stepNumber.number << "]:"
//                         << "\nDivider::loc XX    [" << divider->loc[XX] << "]"
//                         << "\nDivider::loc YY    [" << divider->loc[YY] << "]"
//                         << "\nDivider::size XX W [" << divider->size[XX] << "]"
//                         << "\nDivider::size YY H [" << divider->size[YY] << "]"
//                            ;
//#endif
              //   add divider pointers (if any) to the graphics scene
              for (int j = 0; j < range->stepDividerPointerList.size(); j++) {
                Pointer *pointer = range->stepDividerPointerList[j];
                if (pointer->stepNum == step->stepNumber.number)
                  divider->addGraphicsPointerItem(pointer,range->view);
              }
            }
          }
        }                        // step divider
      }                          // range
    }                            // rangeType
  }
}

/*
 * This is used to return the best adjusted size of a divider
 */

int Steps::pageSize(int axis, bool adjusted/*false*/, bool forDivider/*true*/){
  int which  = forDivider ? allocType() == Vertical ? YY : XX : axis;
  int pageSizeAdjust = gui->pageSize(meta.LPub.page,which);
  if (adjusted) {
   /*
    * TODO - get all items that intersect divider
    * and subtract top/bottom, left,right from retVal midpoint
    * see http://doc.qt.io/qt-5/qgraphicsview.html#items-4
    for (int i = 0; i < list.size(); i++) {
      if (list[i]->relativeType == RangeType) {
        Range *range = dynamic_cast<Range *>(list[i]);
        if (range && range->stepDividerPointerList.size()){
          break;
        }
      }
    } // for
    */
    int pageHeader        = meta.LPub.page.pageHeader.size.valuePixels(YY);
    int pageFooter        = meta.LPub.page.pageFooter.size.valuePixels(YY);
    int topBottomAdjust   = which == YY ? (pageHeader+pageFooter) : 0;
    BorderData pageBorder = meta.LPub.page.border.value();
    pageSizeAdjust       -= (2*pageBorder.margin[axis]+pageBorder.thickness+topBottomAdjust);
  }
  return pageSizeAdjust;
}

Boundary Steps::boundary(AbstractStepsElement *me)
{
  if (list.size() == 1) {
    return StartAndEndOfSteps;
  } else {
    if (list[0] == me) {
      return StartOfSteps;
    }
    if (list[list.size()-1] == me) {
      return EndOfSteps;
    }
  }
  return Middle;
}

/******************************************************************************
 *Place the multistep and callout CSI annotation metas - Not Used
 *****************************************************************************/

void Steps::setCsiAnnotationMetas(bool force)
{
  if (! meta.LPub.assem.annotation.display.value() ||
      gui->exportingObjects())
      return;

  MetaItem mi;
  QStringList parts;
  Where undefined,rangeStart,rangeEnd;
  Where walk,fromHere,toHere;

  bool calledout = relativeType == CalloutType;

  // get the model file lines to process
  rangeStart = topOfSteps();
  rangeEnd   = bottomOfSteps();

  if (rangeStart == undefined) {
    QString topOf = calledout ? "topOfCallout" : "topOfSteps";
    emit gui->messageSig(LOG_ERROR, QString("CSI annotations could not retrieve %1 location").arg(topOf));
    return;
  }

  for (int i = 0; i < list.size(); i++) {
    if (list[i]->relativeType == RangeType) {
      Range *range = dynamic_cast<Range *>(list[i]);
      if (range) {
        // process annotations for each step
        for (int j = 0; j < range->list.size(); j++) {
          if (range->list[j]->relativeType == StepType) {
            Step *step = dynamic_cast<Step *>(range->list[j]);
            if (step) {

              // Sometimes we may already have annotations for the
              // step defined - such as after printing or exporting
              // In such cases we do not need to set metas again.
              if (step->csiAnnotations.size() && ! force) {
                continue;
              }

              QHash<QString, PliPart*> pliParts;

              step->pli.getParts(pliParts);

              if (! pliParts.size())
                continue;

              QString savePartIds,partIds,lineNumbers;

              fromHere = step->topOfStep();
              toHere   = step->bottomOfStep();
              if (toHere == undefined)
                  toHere = fromHere;
              if (toHere == fromHere) {
                  mi.scanForward(toHere,StepMask);
              }

              if (fromHere == undefined) {
                  emit gui->messageSig(LOG_ERROR, QString("CSI annotations could not retrieve topOfStep location"));
                  continue;
              }

              walk = fromHere;

              for (; walk.lineNumber < toHere.lineNumber; ++walk) {
                QString line = gui->readLine(walk);
                QStringList argv;
                split(line,argv);

                if (argv.size() == 15 && argv[0] == "1") {
                  QString key = QString("%1_%2").arg(QFileInfo(argv[14]).completeBaseName()).arg(argv[1]);
                  PliPart *part = pliParts[key];

                  if (! part)
                    continue;

                  if (part->type != argv[14])
                    continue;

                  if (part->annotateText) {
                    QString typeName = QFileInfo(part->type).completeBaseName();
                    QString pattern = QString("^\\s*0\\s+(\\!*LPUB ASSEM ANNOTATION ICON).*("+typeName+"|HIDDEN|HIDE).*$");
                    QRegExp rx(pattern);
                    Where nextLine = walk;
                    line = gui->readLine(++nextLine); // check next line - skip if meta exist [we may have to extend this check to the end of the Step]
                    if ((line.contains(rx) && typeName == rx.cap(2)) ||
                       ((rx.cap(2) == "HIDDEN" || rx.cap(2) == "HIDE") && !force))
                       continue;

                    bool display = false;
                    AnnotationCategory annotationCategory = AnnotationCategory(Annotations::getAnnotationCategory(part->type));
                    switch (annotationCategory)
                    {
                    case AnnotationCategory::axle:
                        display = meta.LPub.assem.annotation.axleDisplay.value();
                        break;
                    case AnnotationCategory::beam:
                        display = meta.LPub.assem.annotation.beamDisplay.value();
                        break;
                    case AnnotationCategory::cable:
                        display = meta.LPub.assem.annotation.cableDisplay.value();
                        break;
                    case AnnotationCategory::connector:
                        display = meta.LPub.assem.annotation.connectorDisplay.value();
                        break;
                    case AnnotationCategory::hose:
                        display = meta.LPub.assem.annotation.hoseDisplay.value();
                        break;
                    case AnnotationCategory::panel:
                        display = meta.LPub.assem.annotation.panelDisplay.value();
                        break;
                    default:
                        display = meta.LPub.assem.annotation.extendedDisplay.value();
                        break;
                    }
                    if (display) {
                      // Pack parts type, partIds and instance line(s) into stringlist - do not reorder
                      for (int i = 0; i < part->instances.size(); ++i) {
                        savePartIds = typeName+";"+part->color+";"+part->instances[i].modelName;
                        if (partIds == savePartIds) {
                          lineNumbers += QString("%1;").arg(part->instances[i].lineNumber);
                        } else {
                          partIds     = savePartIds;
                          lineNumbers = QString("%1;").arg(part->instances[i].lineNumber);
                          parts.append(partIds+"@"+lineNumbers);
                        }
                      }
                    }
                  }
                }
              }
            }
          } // step
        } // foreach step in range - [if gereater than 1 then can be multistep]
      }
    }
  }
  // add annotation metas for the process list of parts
  if (parts.size()){
      mi.writeCsiAnnotationMeta(parts,fromHere,toHere,&meta,force);
  }
}
