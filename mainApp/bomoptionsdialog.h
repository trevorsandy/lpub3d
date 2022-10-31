
/****************************************************************************
**
** Copyright (C) 2020 - 2022 Trevor SANDY. All rights reserved.
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
 * This file implements a dialog that allows the user to select BOM insert
 * options.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef BOMOPTIONSDIALOG_H
#define BOMOPTIONSDIALOG_H

#include <QDialog>

class QRadioButton;
class BomOptionDialog : public QDialog
{

  Q_OBJECT

public:
  BomOptionDialog(int modelIndex, bool remove, QWidget *parent);
 ~BomOptionDialog();
  static int getOption(bool &fullBom, int modelIndex, QWidget *parent = nullptr, bool remove = false);
private slots:
  void buttonClicked(bool enable);
private:
  enum { BomAtModel,
         BomAtPage,
         BomAtSubmodel,
         BomForModel,
         BomForSubmodel,
         BomOptions
       };
  struct ButtonProps {
      QString label;
      QString addToolTip;
      QString removeToolTip;
  };
  static const ButtonProps buttonProperties[];
  QList<QRadioButton*> buttonList;
  bool subModelOptions;
};

#endif // BOMOPTIONSDIALOG_H
