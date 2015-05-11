 

/****************************************************************************

**

** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.

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

#include "metatypes.h"



class QGridLayout;

class QPushButton;

class QFrame;

class QComboBox;

class QDialogButtonBox;





/*

 *    TL|T-L T-C T-R|TR

 *   ---+-----------+--

 *   L-T|TL   T   TR|R-T

 *   L-C|L    C    R|R-C

 *   L_B|BL   B   BR|R-B

 *   ---+-----------+---

 *    BL|B-L  B  B-R|BR

 */



class PlacementDialog : public QDialog

{

  Q_OBJECT

public:

  enum PrepositionMask { InsideOk = 128, OutsideOk = 256 };

  enum RelativeTos 

    { Page = 1 , Csi = 2, Ms = 4, Sn = 8, Pli = 16, Callout = 32, Pn = 64};

  RelativeTos currentRelativeType;

    

  PlacementDialog(

    PlacementType   parentType,

    PlacementType   placedType,

    PlacementData  &goods,

    QString         title,

    QWidget        *parent,

    bool            pliPerStep);



 ~PlacementDialog();



  static bool getPlacement(

    PlacementType   parentType,

    PlacementType   placedType,

    PlacementData  &goods,  // the type in here provides default value

    QString         title,

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



  PlacementData *goods;



  QComboBox   *combo;

  QPushButton *buttons[5][5];

  QGridLayout *outsideGrid;

  QGridLayout *insideGrid;

  QFrame      *insideFrame;

  QDialogButtonBox *buttonBox;



  static const int relativeToOks[];

  static const int prepositionOks[];

};

#endif

