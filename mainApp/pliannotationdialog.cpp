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
 * This class implements a simple dialog that lets the user specify annotation
 * options for the parts list images.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "pliannotationdialog.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include "metagui.h"

PliAnnotationDialog::PliAnnotationDialog(
  PliAnnotationMeta &goods,
  QString           _name,
  QWidget           *parent)
  : QDialog(parent)
{
  setWindowTitle(_name);

  meta.display.setValue(goods.display.value());
  meta.titleAnnotation.setValue(goods.titleAnnotation.value());
  meta.freeformAnnotation.setValue(goods.freeformAnnotation.value());
  meta.titleAndFreeformAnnotation.setValue(goods.titleAndFreeformAnnotation.value());

  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox(_name,this);
  layout->addWidget(box);
  annotation = new PliAnnotationGui("",&meta,box);

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
}

PliAnnotationDialog::~PliAnnotationDialog()
{
}

bool PliAnnotationDialog::getPliAnnotationOption(
  PliAnnotationMeta &goods,
  QString            name,
  QWidget           *parent)
{
  PliAnnotationDialog *dialog = new PliAnnotationDialog(goods,name,parent);

  bool ok = dialog->exec() == QDialog::Accepted;

  if (ok) {
    goods = dialog->meta;
  }
  return ok;
}

void PliAnnotationDialog::accept()
{
  if (annotation->titleModified ||
      annotation->freeformModified ||
      annotation->titleAndFreeformModified ||
      annotation->displayModified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void PliAnnotationDialog::cancel()
{
  QDialog::reject();
}

