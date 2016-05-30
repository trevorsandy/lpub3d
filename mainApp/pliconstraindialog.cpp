 

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



#include "pliconstraindialog.h"

#include <QVBoxLayout>

#include <QGroupBox>

#include <QDialogButtonBox>

#include "metagui.h"



ConstrainDialog::ConstrainDialog(

  ConstrainData &goods,
  QString       _name,
  QWidget       *parent)

  : QDialog(parent)

{

  setWindowTitle(_name);

  meta.setValueUnit(goods);

  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);
  QGroupBox *box = new QGroupBox(_name,this);
  layout->addWidget(box);

  constraint = new ConstrainGui("",&meta,box);

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));
  layout->addWidget(buttonBox);

  setModal(true);

}

ConstrainDialog::~ConstrainDialog()
{
}

bool ConstrainDialog::getConstraint(
  ConstrainData &goods,
  QString         name,
  QWidget        *parent)

{
  ConstrainDialog *dialog = new ConstrainDialog(goods,name,parent);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    goods = dialog->meta.value();
  }
  return ok;
}

void ConstrainDialog::accept()

{
  if (constraint->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void ConstrainDialog::cancel()
{
  QDialog::reject();
}

