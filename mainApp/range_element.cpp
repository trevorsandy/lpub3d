
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
 * This is an abstract class used to represent things contained within a
 * range.  Some derived classes include step, and reserve.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "ranges_element.h"
#include "range_element.h"
#include "ranges.h"
#include "callout.h"
#include "lpub.h"

Steps *AbstractRangeElement::grandparent()
{
  return parent->grandparent();
}

Callout *AbstractRangeElement::callout()
{
  return dynamic_cast<Callout *>(grandparent());
}

const Where &AbstractRangeElement::topOfStep()
{
  return top;
}
    
const Where &AbstractRangeElement::bottomOfStep()
{
  return parent->bottomOfStep(this);
}

const Where &AbstractRangeElement::topOfRange()
{
  return parent->topOfRange();
}

const Where &AbstractRangeElement::bottomOfRange()
{
  return parent->bottomOfRange();
}

AbstractRangeElement *AbstractRangeElement::nextElement(
  const AbstractRangeElement *me)
{
  return parent->nextElement(me);
}

const Where &AbstractRangeElement::topOfSteps()
{
  return parent->topOfSteps();
}

const Where &AbstractRangeElement::bottomOfSteps()
{
  return parent->bottomOfSteps();
}
const Where &AbstractRangeElement::topOfCallout()
{
  Callout *co = callout();
  if (co) {
    return co->topOfCallout();
  }
  static Where empty;
  return empty;
}
const Where &AbstractRangeElement::bottomOfCallout()
{
  Callout *co = callout();
  if (co) {
    return co->bottomOfCallout();
  }
  static Where empty;
  return empty;
}

AllocMeta &AbstractRangeElement::allocMeta()
{
  return parent->allocMeta();
}

Boundary AbstractRangeElement::boundary()
{
  return parent->boundary(this);
}

QString AbstractRangeElement::path()
{
  return parent->path();
}

QString AbstractRangeElement::csiName()
{
  return parent->csiName();
}

QStringList AbstractRangeElement::submodelStack()
{
  return parent->submodelStack();
}

bool AbstractRangeElement::onlyChild()
{
  int numSteps = gui->numSteps(top.modelName);
  return numSteps < 2;
}
