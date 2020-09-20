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

#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "submodelcolordialog.h"
#include "metagui.h"

SubModelColorDialog::SubModelColorDialog(
  StringListMeta &goods,
  QString     _name,
  QWidget    *parent)
  : QDialog(parent)
{
  setWindowTitle(_name);

  meta.setValue(goods.value());

  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox("Submodel Level Colors",this);
  layout->addWidget(box);
  subModelColor = new SubModelColorGui(&meta,box);

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);

  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(300,20);
}

SubModelColorDialog::~SubModelColorDialog()
{
}

bool SubModelColorDialog::getSubModelColor(
  StringListMeta &goods,
  QString         name,
  QWidget        *parent)
{
  SubModelColorDialog *dialog = new SubModelColorDialog(goods,name,parent);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    goods.setValue(dialog->meta.value());
  }
  return ok;
}

void SubModelColorDialog::accept()
{
  if (subModelColor->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void SubModelColorDialog::cancel()
{
  QDialog::reject();
}
