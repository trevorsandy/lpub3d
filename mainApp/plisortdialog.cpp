/****************************************************************************
**
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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
 * This class implements a simple dialog that lets the user specify sort
 * options for the parts list images.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "plisortdialog.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include "metagui.h"

PliSortDialog::PliSortDialog(
 PliSortOrderMeta &goods,
  QString       _name,
  QWidget       *parent)
  : QDialog(parent)
{
  setWindowTitle(_name);

  meta.primary.setValue(goods.primary.value());
  meta.secondary.setValue(goods.secondary.value());
  meta.tertiary.setValue(goods.tertiary.value());
  meta.primaryDirection.setValue(goods.primaryDirection.value());
  meta.secondaryDirection.setValue(goods.secondaryDirection.value());
  meta.tertiaryDirection.setValue(goods.tertiaryDirection.value());

  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox(_name,this);
  layout->addWidget(box);
  bool bom  = _name.toUpper().contains("BOM");
  pliSortOrder = new PliSortOrderGui("",&meta,box,bom);

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

PliSortDialog::~PliSortDialog()
{
}

bool PliSortDialog::getPliSortOption(
  PliSortOrderMeta &goods,
  QString         name,
  QWidget        *parent)
{
  PliSortDialog *dialog = new PliSortDialog(goods,name,parent);

  bool ok = dialog->exec() == QDialog::Accepted;

  if (ok) {
      goods = dialog->meta;
  }
  return ok;
}

void PliSortDialog::accept()
{
  if (pliSortOrder->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void PliSortDialog::cancel()
{
  QDialog::reject();
}

