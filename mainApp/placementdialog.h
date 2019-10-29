/****************************************************************************

**

** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.

** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.

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
 * This class implements a dialogue that lets the user pick the placement for
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
 *   TL,T-L=Top Left
 *   BL,B-L=Bottom Left
 */

class PlacementDialog : public QDialog

{

  Q_OBJECT

public:

  enum PrepositionMask { InsideOk = 128, OutsideOk = 256 };
  enum RelativeTos
    {
      /*  0 Page              */ Page,
      /*  1 Csi (Assem)       */ Csi,
      /*  2 MultiStep         */ Ms,
      /*  3 StepNum           */ Sn,
      /*  4 Pli               */ Pli,
      /*  5 Callout           */ Callout,
      /*  6 PageNum           */ Pn,

      /*  7 Title             */ Tt,
      /*  8 ModelName         */ Mnt,
      /*  9 Author            */ At,
      /* 10 Url               */ Urlt,
      /* 11 ModelDesc         */ Mdt,
      /* 12 PublishDesc       */ Pdt,
      /* 13 Copyright         */ Ct,
      /* 14 Email             */ Et,
      /* 15 Disclaimer        */ Dt,
      /* 16 Parts             */ Pt,
      /* 17 Plug              */ Plt,
      /* 18 SubmodelInsCount  */ Sic,
      /* 19 DocumentLogo      */ Dlt,
      /* 20 CoverImage        */ Cit,
      /* 21 PlugImage         */ Pit,
      /* 22 PageHeader        */ Ph,
      /* 23 PageFooter        */ Pf,
      /* 24 Category          */ Cat,
      /* 25 Submodel          */ Sm,
      /* 26 Rotate Icon       */ Ri,
      /* 27 Csi Part          */ Cp,
      /* 28 Step              */ Stp,
      /* 29 Range             */ Rng
      /* 30 Text              */ // plus 8 more...
      /* 31 Bom               */

      /* 32 PagePointer       */
      /* 33 SingleStep        */
      /* 34 Reserve           */
      /* 35 CoverPage         */
      /* 36 CsiAnnotationType */
      /* 37 DividerPointer    */

      /* 38 NumRelatives      */
    };

  PlacementDialog(
    PlacementType   parentType,
    PlacementType   placedType,
    PlacementData  &goods,
    QString         title,
    int             onPageType,
    bool            pliPerStep,
    QWidget        *parent);
 ~PlacementDialog();

  static bool getPlacement(
    PlacementType   parentType,
    PlacementType   placedType,
    PlacementData  &goods,  // the type in here provides default value
    QString         title,
    int             onPageType = ContentPage,
    bool            pliPerStep = false,
    QWidget        *parent = nullptr);

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
  QFrame            *horizontalLine;
  QLabel            *lblRelativeTo;
  QDialogButtonBox  *buttonBox;

  static const QString labels[][5];
  static const QString relativeNames[];
  static const QList<int> relativeToOks[];
  static const int prepositionOks[];
};

#endif
