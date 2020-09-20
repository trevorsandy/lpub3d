/****************************************************************************
**
** Copyright (C) 2018 - 2020 Trevor SANDY. All rights reserved.
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
 * This file implements a dialog that allows the user to change the page
 * size and/or orientation.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef ROTATEICONSIZEDIALOG_H
#define ROTATEICONSIZEDIALOG_H

#include <QDialog>
#include "meta.h"

class RotateIconSizeGui;
class RotateIconSizeDialog : public QDialog
{
  Q_OBJECT
public:

  RotateIconSizeDialog(
    float           goods[],
    QString          name,
    QWidget         *parent);

  virtual ~RotateIconSizeDialog();

  static bool getRotateIconSize(
    float          goods[],
    QString        name,
    QWidget       *parent = 0);

    UnitsMeta      meta;

public slots:
  void accept();
  void cancel();

private:
  RotateIconSizeGui *rotateIconSize;
};

#endif // ROTATEICONSIZEDIALOG_H
