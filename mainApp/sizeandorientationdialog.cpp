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
 * size and/or orientation.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "sizeandorientationdialog.h"
#include "metagui.h"
#include "lpub.h"

SizeAndOrientationDialog::SizeAndOrientationDialog(
  float            sgoods[2],
  OrientationEnc  &ogoods,
  QString         _name,
  QWidget         *parent)
  : QDialog(parent)
{
  setWindowTitle(_name);

  smeta.setRange(1,1000);
  smeta.setFormats(6,4,"9.9999");

  smeta.setValue(0,sgoods[0]);
  smeta.setValue(1,sgoods[1]);
  ometa.setValue(ogoods);

  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  bool dpi = gui->page.meta.LPub.resolution.type() == DPI;
  QString header = (dpi ? "Size and Orientation (In) " : "Size and Orientation (Cm)" );

  QGroupBox *box = new QGroupBox(header,this);
  layout->addWidget(box);

  sizeAndOrientation = new SizeAndOrientationGui("",&smeta,&ometa,box);

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);

  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
}

SizeAndOrientationDialog::~SizeAndOrientationDialog()
{
}

bool SizeAndOrientationDialog::getSizeAndOrientation(
  float           sgoods[2],
  OrientationEnc &ogoods,
  QString         name,
  QWidget        *parent)
{
  SizeAndOrientationDialog *dialog = new SizeAndOrientationDialog(sgoods,ogoods,name,parent);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {

    sgoods[0] = dialog->smeta.value(0);
    sgoods[1] = dialog->smeta.value(1);
    ogoods = dialog->ometa.value();

  }
  return ok;
}

void SizeAndOrientationDialog::accept()
{
  if (sizeAndOrientation->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void SizeAndOrientationDialog::cancel()
{
  QDialog::reject();
}

