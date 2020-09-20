
/****************************************************************************
**
** Copyright (C) 2018 - 2020 Trevor SANDY. All rights reserved.
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

#ifndef PAGESIZES_H
#define PAGESIZES_H

#include <QString>

class PageSizes
{
public:
  PageSizes(){}
  static int numPageTypes();
  static QString pageTypeSizeID(int);
  static float pageWidthCm(int);
  static float pageHeightCm(int);
  static float pageWidthIn(int);
  static float pageHeightIn(int);
};

#endif // PAGESIZES_H
