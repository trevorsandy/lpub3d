 

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
 * The Where class described here is a fundamental class used for
 * backannotating user edits into the LDraw files.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

/*
 *  ranges.h (submodel, multistep, callout)
 *    top, bottom
 *  Range
 *    top, bottom
 *  step
 *    top, bottom
 *  divider
 *    parent(range)
 *  pli
 *  pointer
 *    parent(callout)
 */



#ifndef whereH
#define whereH

#include <QString>

class Where
{
  public:
    QString modelName;
    int     lineNumber;

    Where()
    {
      modelName     = "undefined";
      lineNumber    = 0;
    }

    Where(const Where &rhs)
    {
      modelName = rhs.modelName;
      lineNumber  = rhs.lineNumber;
    }

    Where operator=(const Where &rhs)
    {
      if (this != &rhs) {
        modelName = rhs.modelName;
        lineNumber = rhs.lineNumber;
      }
      return *this;
    }

    Where(
      QString _modelName,
      int     _lineNumber)
    {
      modelName     = _modelName;
      lineNumber    = _lineNumber;
    }

    Where(int _lineNumber)
    {
      lineNumber = _lineNumber;
    }

    const Where operator+(const int &where) const
    {
      Where foo = *this;
      foo.lineNumber += where;
      return foo;
    }

    const Where operator-(const int &where) const
    {
      Where foo = *this;
      foo.lineNumber -= where;
      return foo;
    }

    Where& operator++()
    {
      ++lineNumber;
      return *this;
    }

    Where operator++(int)
    {
      Where foo = *this;
      ++lineNumber;
      return foo;
    }

    Where& operator--()
    {
      --lineNumber;
      return *this;
    }

    Where operator--(int)
    {
      Where foo = *this;
      --lineNumber;
      return foo;
    }

    Where& operator=(const int value) {
      lineNumber = value;
      return *this;
    }

    bool operator<(const int _lineNumber) const
    {
      return lineNumber < _lineNumber;
    }

    bool operator<=(const int _lineNumber) const
    {
      return lineNumber <= _lineNumber;
    }

    bool operator>(const int _lineNumber) const
    {
      return lineNumber > _lineNumber;
    }

    bool operator>=(const int _lineNumber) const
    {
      return lineNumber >= _lineNumber;
    }

    bool operator==(const int other) const
    {
      return lineNumber == other;
    }

    bool operator!=(const int other) const
    {
      return lineNumber != other;
    }

    bool operator==(const Where &other) const
    {
      return modelName  == other.modelName &&
             lineNumber == other.lineNumber;
    }

    bool operator!=(const Where &other) const
    {
      return modelName  != other.modelName ||
             lineNumber != other.lineNumber;
    }

    ~Where()
    {
      modelName.clear();
    }

};

#endif



