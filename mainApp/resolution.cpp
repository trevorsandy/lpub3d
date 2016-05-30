 
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

#include "resolution.h"

static ResolutionType _resolutionType = DPI;
static float          _resolution = 150;  // presumably always inches

float inches2centimeters(float inches)
{
  return inches*2.54;
}

float centimeters2inches(float centimeters)
{
  return centimeters * 0.3937;
}

float resolution()
{
  return _resolution;
}

void setResolution(float res)
{
  if (_resolutionType == DPCM) {
    _resolution = centimeters2inches(res);
  } else {
    _resolution = res;
  }
}

ResolutionType resolutionType()
{
  return _resolutionType;
}

void setResolutionType(ResolutionType type)
{
  _resolutionType = type;
}

void defaultResolutionType(
  bool centimeters)
{
  if (centimeters) {
    _resolutionType = DPCM;
    _resolution = 150;
  } else {
    _resolutionType = DPI;
    _resolution = 150;
  }
}

float toInches(
  ResolutionType type,
  float          value)
{
  switch (type) {
    case DPI:
      return value;
    default:
      return centimeters2inches(value);
  }
}

float toPixels(
  float          value,
  ResolutionType type)
{
  if (type == resolutionType()) {
    if (type == DPCM) {
      value = inches2centimeters(value);
    } else {
      value = centimeters2inches(value);
    }
  }
  return value*resolution();
}

QString units2abbrev()
{
  switch (resolutionType()) {
    case DPI:
      return "in";
    default:
      return "cm";
  }
}

QString units2name()
{
  switch (resolutionType()) {
    case DPI:
      return "inches";
    default:
      return "centimeters";
  }
}
