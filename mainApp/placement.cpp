
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
 * This class implements a fundamental class for placing things relative to
 * other things.  This concept is the cornerstone of LPub's meta commands
 * for describing what building instructions should look like without having
 * to specify inches, centimeters or pixels.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "placement.h"
#include "ranges.h"
#include "callout.h"
#include "range.h"
#include "step.h"
#include "csiannotation.h"

// for debugging tables
QString tblNames[] = {
    "TblCo0 - 0 C0",   // Callout relative to StepNumber
    "TblSn0 - 5 S0",   // StepNumber relative to CSI
    "TblCo1 - 2 C1",   // Callout relative to RotateIcon
    "TblRi0 - 3 R0",   // RotateIcon relative to CSI
    "TblCo2 - 4 C2",   // Callout relative to Submodel
    "TblSm0 - 1 M0",   // Submodel relative to CSI
    "TblCo3 - 6 C3",   // Callout relative to PLI
    "TblPli0 - 7 P0",  // Pli relative to CSI
    "TblCo4 - 8 C4",   // Callout relative to CSI
    "TblCsi - 9 A ",   // CSI
    "TblCo5 - 10 C5",  // Callout relative to CSI
    "TblPli1 - 11 P1", // Pli relative to CSI
    "TblCo6 - 12 C6",  // Callout relative to PLI
    "TblSm1 - 13 M1",  // Submodel relative to CSI
    "TblCo7 - 14 C7",  // Callout relative to Submodel
    "TblRi1 - 15 R1",  // RotateIcon relative to CSI
    "TblCo8 - 16 C8",  // Callout relative to RotateIcon
    "TblSn1 - 17 S1",  // StepNumber relative to CSI
    "TblCo9 - 18 C9",  // Callout relative to StepNumber
    "NumPlaces - 19"   //
};

void PlacementNum::sizeit()
{
  sizeit("%1");
}

void PlacementNum::sizeit(QString format)
{
  if (number < 1) {
    size[0] = 0;
    size[1] = 0;
  } else {
    QString string = QString(format) .arg(number);
    QGraphicsTextItem gti(string);
    QFont   f;
    f.fromString(font);
    gti.setFont(f);
    size[XX] = int(gti.document()->size().width());
    size[YY] = int(gti.document()->size().height());
  }
}

int PlacementNum::addStepNumber(
    Page          *page,
    QGraphicsItem *parent)
{
    stepNumber =
        new GroupStepNumberItem(
          page,
          page->groupStepMeta.LPub.multiStep.stepNum,
          "%d",
          page->groupStepNumber.number,
          parent);

    if (!stepNumber)
        return -1;

    stepNumber->relativeType = StepNumberType;
    stepNumber->size[XX]     = int(stepNumber->document()->size().width());
    stepNumber->size[YY]     = int(stepNumber->document()->size().height());

    return 0;
}

/* add a placement element (ranges, range, setep, callout, pli ...) to the list */

void Placement::appendRelativeTo(Placement *element)
{
  if (element->relativeType != PageType) {
    // check if element already added to list
    for (int i = 0; i < relativeToList.size(); i++) {
      if (relativeToList[i] == element) {
        return;
      }
    }
    // add element to list
    if (relativeToList.size() < 100) {
      relativeToList.append(element);
      element->relativeToParent = this;
    }
  }
}

/*
 * we start with a page, and ranges, and
 * we walk through the ranges,range,steps,
 * looking for things that are relative to page.
 * We put these in the p_head list.
 *
 * foreach thing relative to page, we make a list
 * of things that are relative to them.
 */
int Placement::relativeTo(
  Step *step)
{
  int rc = 0;

  if (step) {
    /*
     * step = a range of ranges where ranges by itself are multi-step
     * relativeTo = item is placed relative to what?
     *        e.g. relativeTo PageType [Page]
     * relativeType = this placement type is what?
     *        e.g. this placementType is PageType [Page]
     */
    PlacementType stepRelativeTo;
    /* pageHeader */
    stepRelativeTo = step->plPageHeader.placement.value().relativeTo;
    if (stepRelativeTo == relativeType) {
      placeRelative(&step->plPageHeader);
      appendRelativeTo(&step->plPageHeader);
    }
    /* csiItem (Assembly) */
    stepRelativeTo = step->csiItem->placement.value().relativeTo;
    if (stepRelativeTo == relativeType) {
      placeRelative(step->csiItem);
      appendRelativeTo(step->csiItem);
    }
    /* pli (Parts List) */
    stepRelativeTo = step->pli.placement.value().relativeTo;
    if (stepRelativeTo == relativeType) {
      placeRelative(&step->pli);
      appendRelativeTo(&step->pli);
    }
    /* subModel (Submodel) */
    stepRelativeTo = step->subModel.placement.value().relativeTo;
    if (stepRelativeTo == relativeType) {
      placeRelative(&step->subModel);
      appendRelativeTo(&step->subModel);
    }
    /* stepNumber */
    stepRelativeTo = step->stepNumber.placement.value().relativeTo;
    if (stepRelativeTo == relativeType) {
      placeRelative(&step->stepNumber);
      appendRelativeTo(&step->stepNumber);
    }
    /* rotateIcon */
    stepRelativeTo = step->rotateIcon.placement.value().relativeTo;
    if (stepRelativeTo == relativeType) {
      placeRelative(&step->rotateIcon);
      appendRelativeTo(&step->rotateIcon);
    }
    /* pageFooter */
    stepRelativeTo = step->plPageFooter.placement.value().relativeTo;
    if (stepRelativeTo == relativeType) {
      placeRelative(&step->plPageFooter);
      appendRelativeTo(&step->plPageFooter);
    }
    /* callouts */
    for (int i = 0; i < step->list.size(); i++) {
      if (step->list[i]->relativeType == CalloutType) {
        Callout *callout = step->list[i];
        stepRelativeTo = callout->placement.value().relativeTo;
        if (stepRelativeTo == relativeType) {
          placeRelative(callout);
          appendRelativeTo(callout);
        }
      }
    } // callouts
    /* pagePointers */
    for (auto i : step->parent->parent->pagePointers.keys()) {
        if (step->parent->parent->pagePointers[i]->relativeType == PagePointerType) {
            PagePointer *pagePointer = step->parent->parent->pagePointers[i];
            stepRelativeTo = pagePointer->placement.value().relativeTo;
            if (stepRelativeTo == relativeType) {
                int size[2]   = {pagePointer->size[0],pagePointer->size[1]};
                int margin[2] = {pagePointer->margin.valuePixels(0),pagePointer->margin.valuePixels(1)};
                placeRelative(pagePointer, size, margin);
                appendRelativeTo(pagePointer);
              }
          }
      } // pagePointers
    /* textItem KO*/
    for (int i = 0; i < step->parent->parent->textItemList.size(); i++) {
        TextItem *textItem = step->parent->parent->textItemList[i];
        if (textItem->pagePlaced)
            continue;
        stepRelativeTo = textItem->placement.value().relativeTo;
        if (stepRelativeTo == relativeType) {
            int size[2]   = {textItem->size[0],textItem->size[1]};
            int margin[2] = {textItem->margin.valuePixels(0),textItem->margin.valuePixels(1)};
            placeRelative(textItem, size, margin);
            appendRelativeTo(textItem);
        }
    } // textItem
    // Everything placed
  } // if step

  /* try to find relation for things relative to us */

  int limit = relativeToList.size();
  for (int i = 0; i < limit; i++) {
    rc = relativeToList[i]->relativeTo(step);
    if (rc) {
      break;
    }
  }

  return rc;
}

// RelativeTo Step Group [Multi-step]
int Placement::relativeToSg(
  Steps *steps)
{
  if (steps) {
    // PLI
    if (steps->pli.tsize() &&
        steps->pli.placement.value().relativeTo == relativeType) {
      placeRelative(&steps->pli);
      appendRelativeTo(&steps->pli);
    }

    // SM
    if (steps->subModel.tsize() &&
        steps->subModel.placement.value().relativeTo == relativeType) {
      placeRelative(&steps->subModel);
      appendRelativeTo(&steps->subModel);
    }

    // Callout
    for (int j = 0; j < steps->list.size(); j++) {
      /* range (Steps) */
      if (steps->list[j]->relativeType == RangeType) {
        Range *range = dynamic_cast<Range *>(steps->list[j]);
        for (int i = 0; i < range->list.size(); i++) {
          /* step */
          if (range->list[i]->relativeType == StepType) {
            Step *step = dynamic_cast<Step *>(range->list[i]);
            /* callouts */
            for (int i = 0; i < step->list.size(); i++) {
              if (step->list[i]->relativeType == CalloutType) {
                Callout *callout = dynamic_cast<Callout *>(step->list[i]);
                PlacementData placementData = callout->placement.value();
                if (placementData.relativeTo == relativeType) {
                  placeRelative(callout);
                  steps->appendRelativeTo(callout);
                }
              }
            } // callouts
          } // step
        }
      } // range (Steps)
    }

    // Page Pointer
    for (auto i : steps->pagePointers.keys()) {
      PagePointer *pagePointer = dynamic_cast<PagePointer *>(steps->pagePointers[i]);
      PlacementData placementData = pagePointer->placement.value();
      if (placementData.relativeTo == relativeType) {
        int size[2]   = { pagePointer->size[0], pagePointer->size[1] };
        int margin[2] = { pagePointer->margin.valuePixels(0), pagePointer->margin.valuePixels(1) };
        placeRelative(pagePointer, size, margin);
        steps->appendRelativeTo(pagePointer);
      }
    } // pagePointerss

    // textItem KO
    for (int i = 0; i < steps->textItemList.size(); i++) {
      TextItem *textItem = dynamic_cast<TextItem *>(steps->textItemList[i]);
      if (textItem->pagePlaced)
          continue;
      PlacementData placementData = textItem->placement.value();
      if (placementData.relativeTo == relativeType) {
        int size[2]   = {textItem->size[0],textItem->size[1]};
        int margin[2] = {textItem->margin.valuePixels(0),textItem->margin.valuePixels(1)};
        placeRelative(textItem, size, margin);
        steps->appendRelativeTo(textItem);
      }
    } // textItem

    /* try to find relation for things relative to us */

    return 0;
  } else {
    return -1;
  }
}

/*
 * This recursive function is the center piece of the whole concept of
 * placing items relative to other items.  At the topmost level, this is of
 * type page, and items can be of types pageHeader, csiItem, pli, subModel,
 * stepNumber, rotateIcon, pageFooter, callouts, pagePointers, or step group...
 *
 * As we recurse, an item could be a step group or csi. If this->relativeType
 * is step-group then callouts could be placed relative to us.  If
 * this->relativeType is csi, then step number, rotateIcons, submodels, PLIs,
 * or callouts could be placed relative to us.
 */

void Placement::placeRelative(
  Placement *them)
{
  int lmargin[2] = { them->margin.valuePixels(XX), them->margin.valuePixels(YY) };
  int margin2[2] = { margin.valuePixels(XX), margin.valuePixels(YY) };
  
  for (int i = 0; i < 2; i++) {  
    lmargin[i] = margin2[i] > lmargin[i] ? margin2[i] : lmargin[i];
  }
  
  placeRelative(them,them->size,lmargin);

  for (int i = 0; i < 2; i++) {  
    them->boundingLoc[i] = them->loc[i];
  
    int top, bottom, height;

    // calculate changes in our size due to neighbours
    // if neighbour is left or top, calculate bounding top left
    // corner

    top = them->loc[i];
    height = boundingLoc[i] - top;
    if (height > 0) {
      boundingLoc[i]  -= height;
      boundingSize[i] += height;
    }     
    
    bottom = top + them->size[i];
    height = bottom - (boundingLoc[i] + boundingSize[i]);
    if (height > 0) {
      boundingSize[i] += height;
    }
  }
}

void Placement::placeRelative(
  Placement   *them,
  int          margin[2])
{
  int lmargin[2] = { them->margin.valuePixels(XX), them->margin.valuePixels(YY) };
  
  for (int i = 0; i < 2; i++) {  
    lmargin[i] = margin[i] > lmargin[i] ? margin[i] : lmargin[i];
  }
  
  placeRelative(them,them->size,lmargin);

  for (int i = 0; i < 2; i++) {  
    them->boundingLoc[i] = them->loc[i];
  
    int top, bottom, height;

    top = them->loc[i];
    height = boundingLoc[i] - top;
    if (height > 0) {
      boundingLoc[i]  -= height;
      boundingSize[i] += height;
    }     
    bottom = top + them->size[i];
    height = bottom - boundingLoc[i] - boundingSize[i];
    if (height > 0) {
      boundingSize[i] += height;
    }
  }
}

void Placement::placeRelative(
  Placement *them,
  int   them_size[2],
  int   lmargin[2])
{
  PlacementData placementData = them->placement.value();

  them->relativeToLoc[0] = loc[0];
  them->relativeToLoc[1] = loc[1];
  them->relativeToSize[0] = size[XX];
  them->relativeToSize[1] = size[YY];

  if (placementData.preposition == Outside) {
    switch (placementData.placement) {
      case TopLeft:
      case Left:
      case BottomLeft:
        them->loc[XX] = loc[XX] - (them_size[XX] + lmargin[XX]);
      break;
      case TopRight:
      case Right:
      case BottomRight:
        them->loc[XX] = loc[XX] + size[XX] + lmargin[XX];
      break;
      case Top:
      case Bottom:
        them->loc[XX] = loc[XX];
        switch (placementData.justification) {
          case Center:
            them->loc[XX] += (size[XX] - them_size[XX])/2;
          break;
          case Right:
            them->loc[XX] += size[XX] - them_size[XX];
          break;
          default:
          break;
        }
      break;
      case Center:
        them->loc[XX] = loc[XX];
      break;
      default:
      break;
    }
    switch (placementData.placement) {
      case TopLeft:
      case Top:
      case TopRight:
        them->loc[YY] = loc[YY] - (them_size[YY] + lmargin[YY]);
      break;
      case BottomLeft:
      case Bottom:
      case BottomRight:
        them->loc[YY] = loc[YY] + size[YY] + lmargin[YY];
      break;
      case Left:
      case Right:
        them->loc[YY] = loc[YY];
        switch(placementData.justification) {
          case Center:
            them->loc[YY] += (size[YY] - them_size[YY])/2;
          break;
          case Bottom:
            them->loc[YY] += size[YY] - them_size[YY];
          break;
          default:
          break;
        }
      break;
      default:
      break;
    }
  } else {                  //inside
    switch (placementData.placement) {
      case TopLeft:
      case Left:
      case BottomLeft:
        them->loc[XX] = loc[XX] + lmargin[XX];
      break;
      case Top:
      case Center:
      case Bottom:
        them->loc[XX] = loc[XX] + (size[XX] - them_size[XX])/2;
      break;
      case TopRight:
      case Right:
      case BottomRight:
        them->loc[XX] = loc[XX] +  size[XX] - them_size[XX] - lmargin[XX];
      break;
      default:
      break;
    }

    switch (placementData.placement) {
      case TopLeft:
      case Top:
      case TopRight:
        them->loc[YY] = loc[YY] + lmargin[YY];
      break;
      case Left:
      case Center:
      case Right:
        them->loc[YY] = loc[YY] + (size[YY] - them_size[YY])/2;
      break;
      case BottomLeft:
      case Bottom:
      case BottomRight:
        them->loc[YY] = loc[YY] +  size[YY] - them_size[YY] - lmargin[YY];
      break;
      default:
      break;
    }
  }
  them->loc[XX] += int(size[XX] * them->placement.value().offsets[XX]);
  them->loc[YY] += int(size[YY] * them->placement.value().offsets[YY]);
}

void Placement::placeRelativeBounding(
  Placement *them)
{
  int lmargin[2] = { them->margin.valuePixels(XX), them->margin.valuePixels(YY) };
  int margin2[2] = { margin.valuePixels(XX), margin.valuePixels(YY) };

  for (int i = 0; i < 2; i++) {
    lmargin[i] = margin2[i] > lmargin[i] ? margin2[i] : lmargin[i];
  }

  int bias[2];

  bias[XX] = them->loc[XX] - them->boundingLoc[XX];
  bias[YY] = them->loc[YY] - them->boundingLoc[YY];

  placeRelative(them, them->boundingSize, lmargin);

  them->loc[XX] += bias[XX];
  them->loc[YY] += bias[YY];
}

void Placement::justifyX(
  int          origin,
  int          height)
{
  switch (placement.value().placement) {
    case Top:
    case Bottom:
      switch (placement.value().justification) {
        case Left:
          loc[XX] = origin;
        break;
        case Center:
          loc[XX] = origin + (height - size[XX])/2;
        break;
        case Right:
          loc[XX] = origin + height - size[XX];
        break;
        default:
        break;
      }
    break;
    default:
    break;
  }
}

void Placement::justifyY(
  int          origin,
  int          height)
{
  switch (placement.value().placement) {
    case Left:
    case Right:
      switch (placement.value().justification) {
        case Top:
          loc[YY] = origin;
        break;
        case Center:
          loc[YY] = origin + (height - size[YY])/2;
        break;
        case Bottom:
          loc[YY] = origin + height - size[YY];
        break;
        default:
        break;
      }
    break;
    default:
    break;
  }
}

void Placement::calcOffsets(
  PlacementData &placementData,
  float offset[2],
  qreal topLeft[2],
  qreal size[2])
{
  topLeft[0] -= relativeToLoc[0];
  topLeft[1] -= relativeToLoc[1];
  
  if (placementData.preposition == Inside) {
    switch (placementData.placement) {
      case TopLeft:
      case Left:
      case BottomLeft:
        offset[0] = float(topLeft[0]);
      break;
      case TopRight:
      case Right:
      case BottomRight:
        offset[0] = float((topLeft[0]+size[0]) - relativeToSize[0]);
      break;
      default:
        offset[0] = float(topLeft[0] + size[0]/2 - relativeToSize[0]/2);
      break;
    }
    switch (placementData.placement) {
      case TopLeft:
      case Top:
      case TopRight:
        offset[1] = float(topLeft[1]);
      break;
      case BottomLeft:
      case Bottom:
      case BottomRight:
        offset[1] = float((topLeft[1] + size[1]) - relativeToSize[1]);
      break;
      default:
        offset[1] = float(topLeft[1] + size[1]/2 - relativeToSize[1]/2);
      break;
    }
  } else {
    switch (placementData.placement) {
      case TopLeft:
      case Left:
      case BottomLeft:
        offset[0] = float(topLeft[0] + size[0]);
      break;
      case TopRight:
      case Right:
      case BottomRight:
        offset[0] = float(topLeft[0] - relativeToSize[0]);
      break;
      default:
        offset[0] = float(topLeft[0] + size[0]/2 - relativeToSize[0]/2);
      break;
    }
    switch (placementData.placement) {
      case TopLeft:
      case Top:
      case TopRight:
        offset[1] = float(topLeft[1] + size[1]);
      break;
      case BottomLeft:
      case Bottom:
      case BottomRight:
        offset[1] = float(topLeft[1] - relativeToSize[1]);
      break;
      default:
        offset[1] = float(topLeft[1] + size[1]/2 - relativeToSize[0]/2);
      break;
    }
  }
  offset[0] /= relativeToSize[0];
  offset[1] /= relativeToSize[1];
}
