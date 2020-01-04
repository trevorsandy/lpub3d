/****************************************************************************
**
** Copyright (C) 2016 - 2020 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
 *
 * This class implements a dialog that lets the user pick the placement for
 * page pointers placing them relative to the page.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef POINTERPLACEMENTDIALOG_H
#define POINTERPLACEMENTDIALOG_H

#include <QDialog>
#include <QLabel>
#include "metatypes.h"

class QGridLayout;
class QPushButton;
class QFrame;
class QComboBox;
class QDialogButtonBox;

/*
 *   +-----------+
 *   |T/L  T  T/R|
 *   |L    C    R|
 *   |B/L  B  B/R|
 *   +-----------+
 */

class PointerPlacementDialog : public QDialog
{
    Q_OBJECT

public:

  enum PointerPlacementType {   //  placement dialog codes:
    PageType,                   // 0 page
    PointerNumRelatives         // 1
  };

  enum PrepositionMask { InsideOk = 128 };

  enum RelativeTos { page };

  PointerPlacementDialog(
    PlacementType   parentType,
    PlacementType   placedType,
    PlacementData  &goods,
    QString         title,
    QWidget        *parent);

 ~PointerPlacementDialog();

  static bool getPointerPlacement(
    PlacementType   parentType,
    PlacementType   placedType,
    PlacementData  &goods,          // the type in here provides default value
    QString         title,
    QWidget        *parent = nullptr);

  static QString relativeToName(int relativeTo);

private slots:

  void topLeftI();
  void top();
  void topRightI();

  void left();
  void center();
  void right();

  void bottomLeftI();
  void bottom();
  void bottomRightI();

  void relativeToChanged(int);

private:
  void setEnabled(int okPrepositions);
  void highlightPlacement(PlacementData *goods);

  PlacementType      _parentType;
  PlacementData     *goods;
  QComboBox         *combo;
  QPushButton       *buttons[3][3];
  QGridLayout       *outsideGrid;
  QFrame            *horizontalLine;
  QLabel            *lblRelativeTo;
  QDialogButtonBox  *buttonBox;

  static const QString labels[][3];
  static const QString relativeNames[];
  static const QList<int> relativeToOks[];
  static const int prepositionOks[];
};

#endif // POINTERPLACEMENTDIALOG_H
