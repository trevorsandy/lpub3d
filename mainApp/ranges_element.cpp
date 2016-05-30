 
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
 * This abstract class is used to represent things contained in a ranges.
 * Primarily this is range.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "range_element.h"
#include "ranges_element.h"
#include "ranges.h"

Steps *AbstractStepsElement::grandparent()
{
  return parent;
}

AllocEnc AbstractStepsElement::allocType()
{
  return parent->allocType();
}

AllocMeta &AbstractStepsElement::allocMeta()
{
  return parent->allocMeta();
}

/*
 * Each step only has its top of step.  To find bottom of step, we need
 * to find the next step's top.....
 */

const Where &AbstractStepsElement::bottomOfStep(
  AbstractRangeElement *me)
{
  int size = list.size();
  
  for (int i = 0; i < size; i++) {
    if (list[i] == me) {
      if (i < size - 1) {
        return list[i+1]->topOfStep();
      } else {
        return parent->bottomOfStep(this);
      }
    }
  }
  static Where nowhere;
  return nowhere;
}

const Where &AbstractStepsElement::topOfRange()
{
  if (list.size() && list[0]) {
    return list[0]->topOfStep();
  } else {
    static Where foo;
    return foo;
  }
}

const Where &AbstractStepsElement::bottomOfRange()
{
  return parent->bottomOfStep(this);
}

AbstractRangeElement *AbstractStepsElement::nextElement(
  const AbstractRangeElement *me)
{
  int size = list.size();
  
  for (int i = 0; i < size; i++) {
    if (list[i] == me) {
      if (i < size - 1) {
        return list[i+1];
      } else {
        const AbstractStepsElement *range;
        range = parent->nextRange(this);
        if (range) {
          return range->list[0];
        } else {
          return NULL;
        }
      }
    }
  }
  return NULL;
}


const Where &AbstractStepsElement::topOfSteps()
{
  return parent->topOfSteps();
}  

const Where &AbstractStepsElement::bottomOfSteps()
{
  return parent->bottomOfSteps();
}  


QString AbstractStepsElement::path()
{
  return parent->path();
}

QString AbstractStepsElement::csiName()
{
  return parent->csiName();
}

QStringList AbstractStepsElement::submodelStack()
{
  return parent->submodelStack();
}

QString AbstractStepsElement::modelName()
{
  return parent->modelName();
}

Boundary AbstractStepsElement::boundary(AbstractRangeElement *me)
{
  Boundary myBoundary = parent->boundary(this);

  switch (myBoundary) {
    case StartOfSteps:
      if (list.size() == 1) {
        return Boundary(StartOfSteps|StartOfRange|EndOfRange);
      }
      if (list[0] == me) {
        return Boundary(StartOfSteps|StartOfRange);
      }
    break;
    case EndOfSteps:
      if (list.size() == 1) {
        return Boundary(StartOfRange|EndOfRange|EndOfSteps);
      }
      if (list[list.size()-1] == me) {
        return Boundary(EndOfRange|EndOfSteps);
      }
    break;
    case StartAndEndOfSteps:
      if (list.size() == 1) {
        return StartAndEndOfSteps;
      } else if (list[0] == me) {
        return Boundary(StartOfSteps|StartOfRange);
      } else if (list[list.size()-1] == me) {
        return Boundary(EndOfRange|EndOfSteps);
      }
    break;
    default:
    break;
  }
  if (list.size() == 1) {
    return StartAndEndOfRange;
  }
  if (list[0] == me) {
    return StartOfRange;
  }
  if (list[list.size()-1] == me) {
    return EndOfRange;
  }
  return Middle;
}
