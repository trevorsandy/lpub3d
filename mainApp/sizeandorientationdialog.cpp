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
  PgSizeData    &sgoods,
  OrientationEnc  &ogoods,
  QString         _name,
  QWidget         *parent)
  : QDialog(parent)
{
  setWindowTitle(_name);

  smeta.setRange(1,1000);
  smeta.setFormats(6,4,"9.9999");
  smeta.setValue(0,sgoods.sizeW);
  smeta.setValue(1,sgoods.sizeH);
  smeta.setValueSizeID(sgoods.sizeID);

  ometa.setValue(ogoods);

  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  bool dpi = gui->page.meta.LPub.resolution.type() == DPI;
  QString header = (dpi ? "Size and Orientation (Inches) " : "Size and Orientation (Centimeters)");

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
  setMinimumSize(40,20);
}

SizeAndOrientationDialog::~SizeAndOrientationDialog()
{
}

bool SizeAndOrientationDialog::getSizeAndOrientation(
  PgSizeData   &sgoods,
  OrientationEnc &ogoods,
  QString         name,
  QWidget        *parent)
{
  SizeAndOrientationDialog *dialog = new SizeAndOrientationDialog(sgoods,ogoods,name,parent);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {

    sgoods.sizeW = dialog->smeta.value(0);
    sgoods.sizeH = dialog->smeta.value(1);
    sgoods.sizeID= dialog->smeta.valueSizeID(); // e.g. A4
    ogoods = dialog->ometa.value();
//    logDebug() << " SIZE TX(dialog return): Width: " << dialog->smeta.value(0) << " Height: " << dialog->smeta.value(1) << " SizeID: " << dialog->smeta.valueSizeID();

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

