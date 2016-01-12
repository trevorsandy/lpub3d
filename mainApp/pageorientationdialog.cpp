/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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
 * pageOrientation.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "pageOrientationdialog.h"
#include "metagui.h"

PageOrientationDialog::PageOrientationDialog(
  OrientationEnc  &goods,
  QString         _name,
  QWidget         *parent)
  : QDialog(parent)
{
  setWindowTitle(_name);

  meta.setValue(goods);

  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox("PageOrientation",this);
  layout->addWidget(box);

  pageOrientation = new PageOrientationGui("",&meta,box);

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);

  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
}

PageOrientationDialog::~PageOrientationDialog()
{
}

bool PageOrientationDialog::getPageOrientation(
  OrientationEnc &goods,
  QString         name,
  QWidget        *parent)
{
  PageOrientationDialog *dialog = new PageOrientationDialog(goods,name,parent);

  bool ok = dialog->exec() == QDialog::Accepted;

  if (ok) {

    goods = dialog->meta.value();

  }
  return ok;
}

void PageOrientationDialog::accept()
{
  if (pageOrientation->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void PageOrientationDialog::cancel()
{
  QDialog::reject();
}
