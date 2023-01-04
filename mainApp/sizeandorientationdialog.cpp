/****************************************************************************
**
** Copyright (C) 2015 - 2023 Trevor SANDY. All rights reserved.
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
#include "commonmenus.h"
#include "metagui.h"
#include "lpub.h"

SizeAndOrientationDialog::SizeAndOrientationDialog(
  PageSizeData    &goodsS,
  OrientationEnc  &goodsO,
  QString         _name,
  QWidget         *parent)
  : QDialog(parent)
{
  setWindowTitle(_name);

  setWhatsThis(lpubWT(WT_DIALOG_SIZE_AND_ORIENTATION,windowTitle()));

  metaS.setValue(goodsS);
  metaO.setValue(goodsO);
  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  bool dpi = lpub->page.meta.LPub.resolution.type() == DPI;
  QString header = (dpi ? "Size and Orientation (Inches) " : "Size and Orientation (Centimeters)");

  QGroupBox *box = new QGroupBox(header,this);
  layout->addWidget(box);

  sizeAndOrientation = new SizeAndOrientationGui("",&metaS,&metaO,box);

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
  PageSizeData   &goodsS,
  OrientationEnc &goodsO,
  QString         name,
  QWidget        *parent)
{
  SizeAndOrientationDialog *dialog = new SizeAndOrientationDialog(goodsS,goodsO,name,parent);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    goodsS = dialog->metaS.value();
    goodsO = dialog->metaO.value();
  }
  return ok;
}

void SizeAndOrientationDialog::accept()
{
  bool modified = sizeAndOrientation->orientationModified ||
                  sizeAndOrientation->sizeIDModified ||
                  sizeAndOrientation->sizeModified ||
                  sizeAndOrientation->modified;
  if (modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void SizeAndOrientationDialog::cancel()
{
  QDialog::reject();
}

