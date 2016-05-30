 
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
 * This class implements a simple dialog that lets the user specify constraints
 * for how the parts in the parts list images should be arranged.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef PLICONSTRAINDIALOG_H
#define PLICONSTRAINDIALOG_H

#include <QDialog>
#include "meta.h"

class ConstrainGui;
class ConstrainDialog : public QDialog
{
  Q_OBJECT
public:
    
  ConstrainDialog(
    ConstrainData  &goods,
    QString          name,
    QWidget         *parent);

  virtual ~ConstrainDialog();

  static bool getConstraint(
    ConstrainData &goods,
    QString        name,
    QWidget       *parent);

  ConstrainMeta meta;

public slots:
  void accept();
  void cancel();
  
private:
  ConstrainGui *constraint;
};

#endif
