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

/****************************************************************************
 *
 * This file implements a dialog that allows the user to change the submodel
 * level color
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef SUBMODELCOLORDIALOG_H
#define SUBMODELCOLORDIALOG_H

#include <QDialog>
#include "meta.h"

class SubModelColorGui;
class SubModelColorDialog : public QDialog
{
  Q_OBJECT
public:

  SubModelColorDialog(
    StringListMeta  &goods,
    QString          name,
    QWidget         *parent);

  virtual ~SubModelColorDialog();

  static bool getSubModelColor(
    StringListMeta &goods,
    QString      name,
    QWidget     *parent);

  StringListMeta meta;

public slots:
  void accept();
  void cancel();

private:
  SubModelColorGui *subModelColor;
};
#endif
