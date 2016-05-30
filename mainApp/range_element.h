 
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

#ifndef range_elementH
#define range_elementH

#include "placement.h"
#include "where.h"

/*
 * This is the base class for step and reserve
 */

class Step;
class Steps;
class Callout;
class AbstractStepsElement;
class AllocMeta;

class AbstractRangeElement : public Placement {
  public:
    AbstractStepsElement *parent;
    Where   top;

    AbstractRangeElement()
    {
    }

    virtual ~AbstractRangeElement() {};

    const Where &topOfStep();
    void         setTopOfStep(Where &);
    const Where &bottomOfStep();
    const Where &topOfRange();
    const Where &bottomOfRange();
    const Where &topOfSteps();
    const Where &bottomOfSteps();
    const Where &topOfCallout();
    const Where &bottomOfCallout();
          AbstractRangeElement *nextElement(const AbstractRangeElement *);
    
    Steps   *grandparent();
    Callout *callout();
    
    AllocMeta &allocMeta();
    
    Boundary boundary();

    QString path();
    QString csiName();
    QStringList submodelStack();
    bool    onlyChild();
};

#endif
