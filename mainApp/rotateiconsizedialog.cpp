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

#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "rotateiconsizedialog.h"
#include "metagui.h"

RotateIconSizeDialog::RotateIconSizeDialog(
  float            goods[2],
  QString         _name,
  QWidget         *parent)
  : QDialog(parent)
{
  setWindowTitle(_name);

  meta.setValue(0,goods[0]);
  meta.setValue(1,goods[1]);

  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox(QString("Rotate Icon Size (in %1)")
                                 .arg(Preferences::preferCentimeters ? "centimetres" : "inches"),this);
  layout->addWidget(box);

  rotateIconSize = new RotateIconSizeGui("",&meta,box);

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

RotateIconSizeDialog::~RotateIconSizeDialog()
{
}

bool RotateIconSizeDialog::getRotateIconSize(
  float           goods[2],
  QString         name,
  QWidget        *parent)
{
  RotateIconSizeDialog *dialog = new RotateIconSizeDialog(goods,name,parent);

  bool ok = dialog->exec() == QDialog::Accepted;

  if (ok) {
    goods[0] = dialog->meta.value(0);
    goods[1] = dialog->meta.value(1);
  }
  return ok;
}

void RotateIconSizeDialog::accept()
{
  if (rotateIconSize->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void RotateIconSizeDialog::cancel()
{
  QDialog::reject();
}
