 
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

void PlacementRotateIcon::setSize(
    UnitsMeta _size,
    float _borderThickness)
{
  iconImageSize   = _size;
  borderThickness =_borderThickness;
}

void PlacementRotateIcon::sizeit()
{
  size[0] = int(iconImageSize.valuePixels(0));
  size[1] = int(iconImageSize.valuePixels(1));

  size[0] += 2*int(borderThickness);
  size[1] += 2*int(borderThickness);
}


/* add a placement element (ranges, range, setep, callout, pli ...) to the list */

void Placement::appendRelativeTo(Placement *element)
{
  if (element->relativeType != PageType) {
    for (int i = 0; i < relativeToList.size(); i++) {
      if (relativeToList[i] == element) {
        return;
      }
    }
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
              e.g. relativeTo PageType(Page)
       relativeType = item's placement type is what?
              e.g. pageHeader's placementType is PageHeaderType */
    PlacementType stepRelativeTo;
    /* pageHeader */
    stepRelativeTo = step->pageHeader.placement.value().relativeTo;
    if (stepRelativeTo == relativeType) {
      placeRelative(&step->pageHeader);
      appendRelativeTo(&step->pageHeader);
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
    /* step number */
    stepRelativeTo = step->stepNumber.placement.value().relativeTo;
    if (stepRelativeTo == relativeType) {
      placeRelative(&step->stepNumber);
      appendRelativeTo(&step->stepNumber);
    }
    /* rotate icon */
    stepRelativeTo = step->rotateIcon.placement.value().relativeTo;
    if (stepRelativeTo == relativeType) {
      placeRelative(&step->rotateIcon);
      appendRelativeTo(&step->rotateIcon);
    }
    /* pageFooter */
    stepRelativeTo = step->pageFooter.placement.value().relativeTo;
    if (stepRelativeTo == relativeType) {
      placeRelative(&step->pageFooter);
      appendRelativeTo(&step->pageFooter);
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

    for (int j = 0; j < steps->list.size(); j++) {

      if (steps->list[j]->relativeType == RangeType) {
        Range *range = dynamic_cast<Range *>(steps->list[j]);
        for (int i = 0; i < range->list.size(); i++) {

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
              } // if callout
            } // callouts
          } // if step
        } // foreach step
      } // if range
    } // foreach range

    /* try to find relation for things relative to us */

    return 0;
  } else {
    return -1;
  }
}

/*
 * This recursive function is the center piece of the whole concept of
 * placing things relative to things.  At the topmost level, this is of
 * type page, and them can be of types page number, step number, csi,
 * pli callout, or step group.
 *
 * Later as we recurse them could be step group or csi. If this->relativeType
 * is step group then callouts could be placed relative to us.  If
 * this->relativeType is csi, then step number, PLIs, or callouts could
 * be placed relative to us.
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

    // calculate changes in our size due to neighbors
    // if neighbor is left or top, calculate bounding top left
    // corner

    top = them->loc[i];
    height = boundingLoc[i] - top;
    if (height > 0) {
      boundingLoc[i] -= height;
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
      boundingLoc[i] -= height;
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
  } else {
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
        offset[0] = topLeft[0];
      break;
      case TopRight:
      case Right:
      case BottomRight:
        offset[0] = (topLeft[0]+size[0]) - relativeToSize[0];
      break;
      default:
        offset[0] = topLeft[0] + size[0]/2 - relativeToSize[0]/2;
      break;
    }
    switch (placementData.placement) {
      case TopLeft:
      case Top:
      case TopRight:
        offset[1] = topLeft[1];
      break;
      case BottomLeft:
      case Bottom:
      case BottomRight:
        offset[1] = (topLeft[1] + size[1]) - relativeToSize[1];
      break;
      default:
        offset[1] = topLeft[1] + size[1]/2 - relativeToSize[1]/2;
      break;
    }
  } else {
    switch (placementData.placement) {
      case TopLeft:
      case Left:
      case BottomLeft:
        offset[0] = topLeft[0] + size[0];
      break;
      case TopRight:
      case Right:
      case BottomRight:
        offset[0] = topLeft[0] - relativeToSize[0];
      break;
      default:
        offset[0] = topLeft[0] + size[0]/2 - relativeToSize[0]/2;
      break;
    }
    switch (placementData.placement) {
      case TopLeft:
      case Top:
      case TopRight:
        offset[1] = topLeft[1] + size[1];
      break;
      case BottomLeft:
      case Bottom:
      case BottomRight:
        offset[1] = topLeft[1] - relativeToSize[1];
      break;
      default:
        offset[1] = topLeft[1] + size[1]/2 - relativeToSize[0]/2;
      break;
    }
  }
  offset[0] /= relativeToSize[0];
  offset[1] /= relativeToSize[1];
}






