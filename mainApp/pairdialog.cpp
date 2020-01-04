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
 * This file implements a few dialogs that contain textually editable
 * versions of number that describe configuration of LDraw files.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "pairdialog.h"
#include <QLineEdit>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QDoubleValidator>

FloatDialog::FloatDialog(
  QString  title,
  QString  _label0,
  FloatMeta *leaf)
{
  QString string = QString("%1") .arg(leaf->value(),
                                      leaf->_fieldWidth,
                                      'f',
                                      leaf->_precision);

  float0 = new QLineEdit(string,this);
  float0->setMaxLength(leaf->_fieldWidth);
  float0->setInputMask(leaf->_inputMask);
  float0->setAlignment(Qt::AlignRight);
  
  label0 = new QLabel(_label0);

  grid = new QGridLayout;

  setWindowTitle(title);

  grid->addWidget(label0,0,0);
  grid->addWidget(float0,0,1);

  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()), SLOT(reject()));

  grid->addWidget(buttonBox,2,0,2,2);

  setLayout(grid);

  setModal(true);
  setMinimumSize(40,20);
}

FloatDialog::~FloatDialog()
{
}

bool FloatDialog::getFloat(
  QString  title,
  QString  heading,
  FloatMeta *leaf,
  float   &try0)
{
  FloatDialog *dialog = new FloatDialog(title,heading,leaf);
  if (dialog->exec() == QDialog::Accepted) {
    bool ok0;
    try0 = dialog->float0->displayText().toFloat(&ok0);
    return ok0;
  } else {
    return false;
  }
}

#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "backgrounddialog.h"
#include "metagui.h"

FloatPairDialog::FloatPairDialog(
  QString  _name,
  QString  _heading1,
  QString  _heading2,
  FloatPairMeta *leaf,
  QWidget *parent)
  : QDialog(parent)
{
  setWindowTitle(_name);

  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox(_name,this);
  layout->addWidget(box);

  meta = leaf;

  floats = new FloatsGui(_heading1,_heading2,leaf,box);

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

FloatPairDialog::~FloatPairDialog()
{
}

bool FloatPairDialog::getFloatPair(
  QString       _name,
  QString       _heading1,
  QString       _heading2,
  FloatPairMeta *leaf,
  float         values[],
  QWidget       *parent)
{
  FloatPairDialog *dialog = new FloatPairDialog(_name,_heading1,_heading2,leaf,parent);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    values[0] = dialog->meta->value(0);
    values[1] = dialog->meta->value(1);
  }
  return ok;
}

void FloatPairDialog::accept()
{
  if (floats->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void FloatPairDialog::cancel()
{
  QDialog::reject();
}

DoubleSpinDialog::DoubleSpinDialog(
  QString   _name,
  QString   _heading,
  FloatMeta *floatMeta,
  float     step,  
  QWidget *parent)
  : QDialog(parent)
{
  setParent(parent);
  setWindowTitle(_name);
  QVBoxLayout *layout = new QVBoxLayout;
  setLayout(layout);

  QGroupBox *box = new QGroupBox(_name);
  layout->addWidget(box);

  meta = floatMeta;

  spin = new DoubleSpinGui(
               _heading,
               meta,
               meta->_min,
               meta->_max,
               step,
               box);
  
  QDialogButtonBox *buttonBox = new QDialogButtonBox;

  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(40,20);
}

bool DoubleSpinDialog::getFloat(
  QString   title,
  QString   heading,
  FloatMeta *floatMeta,
  float     &try0,
  float     step,
  QWidget *parent)
{
  DoubleSpinDialog *dialog = new DoubleSpinDialog(title,heading,floatMeta,step,parent);
  if (dialog->exec() == QDialog::Accepted) {
    try0 = dialog->meta->value();
    return true;
  } else {
    return false;
  }
}

void DoubleSpinDialog::accept()
{
  if (spin->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void DoubleSpinDialog::cancel()
{
  QDialog::reject();
}


LocalDialog::LocalDialog(
  QString  title,
  QString  question,
  QWidget *parent)
{
  QLabel *label = new QLabel(question,parent);

  QVBoxLayout *grid = new QVBoxLayout(parent);

  setWindowTitle(title);

  grid->addWidget(label);

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(parent);
  buttonBox->addButton(QDialogButtonBox::Yes);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::No);
  connect(buttonBox,SIGNAL(rejected()), SLOT(reject()));

  grid->addWidget(buttonBox);

  setLayout(grid);

  setModal(true);
  setMinimumSize(40,20);
}

LocalDialog::~LocalDialog()
{
}

bool LocalDialog::getLocal(
  QString  title,
  QString  question,
  QWidget *parent)
{
  LocalDialog *dialog = new LocalDialog(title,question,parent);
  if (dialog->exec() == QDialog::Accepted) {
    return true;
  } else {
    return false;
  }
}
