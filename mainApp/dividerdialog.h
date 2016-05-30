 
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
 * This file describes a dialog the user can use to edit divider attributes
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef DIVIDERDIALOG
#define DIVIDERDIALOG

#include <QDialog>
#include "meta.h"

class SepGui;
class DividerDialog : public QDialog
{
  Q_OBJECT
public:
    
  DividerDialog(
    SepData &goods,
    QString      name,
    QWidget     *parent);

  virtual ~DividerDialog();

  static bool getDivider(
    SepData &goods,
    QString  name,
    QWidget *parent);

  SepMeta meta;

public slots:
  void accept();
  void cancel();
  
private:
  SepGui *seperator;
};

#endif
