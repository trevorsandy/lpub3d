/****************************************************************************
**
** Copyright (C) 2019 Trevor SANDY. All rights reserved.
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
 * This class provides the options object passed between LPub3D, the Native
 * renderer and the 3DViewer.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef OPTIONS_H
#define OPTIONS_H

#include <QString>
#include <QStringList>

namespace Options
{
    enum Mt { PLI, CSI ,SMP};
}

class ViewerOptions
{

public:
  ViewerOptions()
  {
    ImageType      = Options::CSI;
    UsingViewpoint = false;
    ImageWidth     = 800 ;
    ImageHeight    = 600;
  }
  QString ViewerCsiKey;
  Options::Mt ImageType;
  QString ImageFileName;
  int ImageWidth;
  int ImageHeight;
  float CameraDistance;
  float FoV;
  float ZNear;
  float ZFar;
  float Latitude;
  float Longitude;
  bool UsingViewpoint;
};

class NativeOptions
{
public:
  NativeOptions()
  {
    ImageType         = Options::CSI;
    TransBackground   = true;
    HighlightNewParts = false;
    UsingViewpoint    = false;
    LineWidth         = 1.0;
    ExportMode        = -1; //NONE
    IniFlag           = -1; //NONE
  }
  QStringList ExportArgs;
  QString InputFileName;
  QString OutputFileName;
  QString ExportFileName;
  Options::Mt ImageType;
  int ExportMode;
  int IniFlag;
  float ImageWidth;
  float ImageHeight;
  float FoV;
  float ZNear;
  float ZFar;
  float Latitude;
  float Longitude;
  float CameraDistance;
  float LineWidth;
  bool HighlightNewParts;
  bool TransBackground;
  bool UsingViewpoint;
};

#endif // OPTIONS_H
