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
 * This class implements a dialog that lets the user pick the placement for
 * something relative to other things.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef PLACEMENTDIALOG_H
#define PLACEMENTDIALOG_H

#include <QDialog>
#include <QLabel>
#include "metatypes.h"

class QGridLayout;
class QPushButton;
class QFrame;
class QComboBox;
class QDialogButtonBox;

/*
 *   T/L|T:L T:C T:R|T/R
 *   ---+-----------+--
 *   L:T|T/L  T  T/R|R:T
 *   L:C|L    C    R|R:C
 *   L:B|B/L  B  B/R|R:B
 *   ---+-----------+---
 *   B/L|B:L B:C B:R|B/R
 *
 * TL,T-L=Top Left
 * BL,B-L=Bottom Left
 */

class PlacementDialog : public QDialog

{

  Q_OBJECT

public:

  enum PrepositionMask { InsideOk = 128, OutsideOk = 256 };

  enum RelativeTos 
    { Page = 1 ,   Csi = 2,     Ms  = 4,      Sn  = 8,      Pli  = 16,     Callout = 32,
      Pn  = 64,    tt  = 128,   mnt = 256,    at  = 512,    urlt = 1024,   mdt = 2048,
      pdt = 4096,  ct  = 8192,  et  = 16384,  dt  = 32768,  pt   = 65536,  plt = 131072,
      cat = 262144,dlt = 524288,cit = 1048576,pit = 2097152,ph   = 4194304,pf  = 8388608
    };

  RelativeTos currentRelativeType;

  PlacementDialog(
    PlacementType   parentType,
    PlacementType   placedType,
    PlacementData  &goods,
    QString         title,
    int             onPageType,
    QWidget        *parent,
    bool            pliPerStep);
 ~PlacementDialog();

  static bool getPlacement(
    PlacementType   parentType,
    PlacementType   placedType,
    PlacementData  &goods,  // the type in here provides default value
    QString         title,
    int             onPageType = ContentPage,
    QWidget        *parent = 0,
    bool            pliPerStep = false);

  static QString relativeToName(int relativeTo);

private slots:
  void topLeftO();
  void topleft();
  void topcenter();
  void topright();
  void topRightO();

  void lefttop();
  void topLeftI();
  void top();
  void topRightI();
  void righttop();

  void leftcenter();
  void left();
  void center();
  void right();
  void rightcenter();

  void leftbottom();
  void bottomLeftI();
  void bottom();
  void bottomRightI();
  void rightbottom();

  void bottomLeftO();
  void bottomleft();
  void bottomcenter();
  void bottomright();
  void bottomRightO();

  void relativeToChanged(int);

private:
  void setEnabled(int okPrepositions);
  void highlightPlacement(PlacementData *goods);

  PlacementData     *goods;
  QComboBox         *combo;
  QPushButton       *buttons[5][5];
  QGridLayout       *outsideGrid;
  QGridLayout       *insideGrid;
  QFrame            *insideFrame;
  QLabel            *lblRelativeTo;
  QDialogButtonBox  *buttonBox;

  static const int relativeToOks[];
  static const int prepositionOks[];
};

#endif
