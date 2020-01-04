
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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
 * This file describes a dialog that the users can use to change the
 * displayed Submodel rotation values.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "rotstepdialog.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include "metagui.h"

RotStepDialog::RotStepDialog(
  RotStepData &goods,
  QString     _name)
{
  setWindowTitle(_name);

  meta.setValue(goods);
  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox("Rotate Submodel",this);
  layout->addWidget(box);
  rotStep = new RotStepGui(&meta,box);

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);

  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(40,20);
}

bool RotStepDialog::getRotStep(
  RotStepData &goods,
  QString     name)
{
  RotStepDialog *dialog = new RotStepDialog(goods,name);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    goods = dialog->meta.value();
  }
  return ok;
}

void RotStepDialog::accept()
{
  if (rotStep->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void RotStepDialog::cancel()
{
  QDialog::reject();
}
