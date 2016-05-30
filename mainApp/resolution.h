 
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
 * This file encapsulates a few simple mechanisms used for implementing
 * dots per inch and dots per centimeter.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef resolution_h
#define resolution_h

#include <QString>

enum ResolutionType {
  DPI,
  DPCM,
};

float resolution();
void setResolution(float);
ResolutionType resolutionType();
void setResolutionType(ResolutionType);
void  defaultResolutionType(bool centimeters);
float inches2centimeters(float inches);
float centimeters2inches(float centimeters);
float toInches(ResolutionType,float);
float toPixels(float,ResolutionType);

QString units2name();
QString units2abbrev();

#endif
