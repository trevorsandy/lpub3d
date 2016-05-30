

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

 * These dialogs provide graphical user interfaces for the UnitMeta and

 * UnitsMeta.

 *

 * Please see lpub.h for an overall description of how the files in LPub

 * make up the LPub program.

 *

 ***************************************************************************/



#include "scaledialog.h"

#include <QVBoxLayout>

#include <QGroupBox>

#include <QDialogButtonBox>

#include "metagui.h"



UnitsDialog::UnitsDialog(

  float   values[2],

  QString _name,

  QWidget *parent)

  : QDialog(parent)

{

  setWindowTitle(_name);



  QVBoxLayout *layout = new QVBoxLayout(this);

  setLayout(layout);



  QGroupBox *box = new QGroupBox(_name,this);

  layout->addWidget(box);



  meta.setValue(values[0],values[1]);

  units = new UnitsGui("",&meta,box);



  QDialogButtonBox *buttonBox;

  

  buttonBox = new QDialogButtonBox(this);



  buttonBox->addButton(QDialogButtonBox::Ok);

  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));

  buttonBox->addButton(QDialogButtonBox::Cancel);

  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));



  layout->addWidget(buttonBox);



  setModal(true);

}



UnitsDialog::~UnitsDialog()

{

}



bool UnitsDialog::getUnits(

  float    values[2],

  QString  name,

  QWidget *parent)

{

  UnitsDialog *dialog = new UnitsDialog(values,name,parent);



  bool ok = dialog->exec() == QDialog::Accepted;

  if (ok) {

    values[0] = dialog->meta.value(0);

    values[1] = dialog->meta.value(1);

  }

  return ok;

}



void UnitsDialog::accept()

{

  if (units->modified) {

    QDialog::accept();

  } else {

    QDialog::reject();

  }

}



void UnitsDialog::cancel()

{

  QDialog::reject();

}

