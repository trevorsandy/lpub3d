
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
 * This file describes a dialogue that the users can use to change the
 * displayed Submodel rotation values.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef ROTSTEPDIALOG_H
#define ROTSTEPDIALOG_H

#include <QDialog>
#include "meta.h"

class RotStepGui;
class RotStepDialog : public QDialog
{
  Q_OBJECT
public:

  RotStepDialog(
    RotStepData  &goods,
    QString      name);

  virtual ~RotStepDialog()
  {
  }

  static bool getRotStep(
    RotStepData  &goods,
    QString      name);

  RotStepMeta   meta;

public slots:
  void accept();
  void cancel();

private:
  RotStepGui *rotStep;
};
#endif // ROTSTEPDIALOG_H
