/****************************************************************************
**
** Copyright (C) 2020 - 2024 Trevor SANDY. All rights reserved.
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
 * This file implements a dialog that allows the user to select BOM insert
 * options.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QRadioButton>

#include "bomoptionsdialog.h"
#include "commonmenus.h"
#include "version.h"

const BomOptionDialog::ButtonProps BomOptionDialog::buttonProperties[BomOptionDialog::BomOptions] =
{
  // Label                        ToolTip
  { "At end of model"            ,"Append BOM in a new step at the end of the main model"       , "Remove BOM formatting at at the end of the main model" },
  { "At current page"            ,"Append BOM in the last step of the current page"             , "Remove BOM formatting at the last step of the current page" },
  { "At end of current submodel" ,"Append BOM in a new step at the end of the current submodel" , "Remove BOM formatting at the end of the current submodel" },
  { "For the complete model"     ,"Include BOM for the complete document model"                 , "Remove BOM formatting for the complete document model" },
  { "For the current submodel"   ,"Include BOM parts for the current submodel"                  , "Remove BOM parts formatting for the current submodel" }
};

BomOptionDialog::BomOptionDialog(int modelIndex, bool remove, QWidget *parent)
{
  setParent(parent);

  setWindowTitle(tr("BOM Option"));

  setWhatsThis(lpubWT(WT_DIALOG_BOM_OPTION,windowTitle()));

  QVBoxLayout *dialogLayout = new QVBoxLayout(this);

  QString optionGroupTitle = QString(remove ? tr("Remove Bill Of Materials Format") : tr("Add Bill Of Materials"));

  QGroupBox *optionGroupBox = new QGroupBox(optionGroupTitle, this);

  dialogLayout->addWidget(optionGroupBox);

  QVBoxLayout *boxLayout = new QVBoxLayout(optionGroupBox);

  optionGroupBox->setLayout(boxLayout);

  subModelOptions = modelIndex;  //if true (> 0), page is not on the top model

  bool createContentBox = true;

  QRadioButton* optionsRadio = nullptr;

  for (int i = 0; i < BomOptions; ++i) {
    if (i < BomAtSubmodel) {
      optionsRadio = new QRadioButton(buttonProperties[i].label,optionGroupBox);
      optionsRadio->setToolTip(remove ? buttonProperties[i].removeToolTip : buttonProperties[i].addToolTip);
      optionsRadio->setChecked(i == BomAtModel);
      QObject::connect(optionsRadio, SIGNAL(clicked(bool)),
                       this,         SLOT(buttonClicked(bool)));
      boxLayout->addWidget(optionsRadio);
      buttonList.append(optionsRadio);
    } else {
      if (subModelOptions) {
        if (i < BomForModel) {
          optionsRadio = new QRadioButton(buttonProperties[i].label,optionGroupBox);
          optionsRadio->setToolTip(remove ? buttonProperties[i].removeToolTip : buttonProperties[i].addToolTip);
          QObject::connect(optionsRadio, SIGNAL(clicked(bool)),
                           this,         SLOT(buttonClicked(bool)));
          boxLayout->addWidget(optionsRadio);
          buttonList.append(optionsRadio);
        } else {
          if (createContentBox) {
            createContentBox = false;
            optionGroupTitle = QString(remove ? tr("Remove Bill Of Materials Parts Format") : tr("Include Bill Of Materials Parts"));
            optionGroupBox = new QGroupBox(optionGroupTitle, this);
            dialogLayout->addWidget(optionGroupBox);
            boxLayout = new QVBoxLayout(optionGroupBox);
            optionGroupBox->setLayout(boxLayout);
          }
          optionsRadio = new QRadioButton(buttonProperties[i].label,optionGroupBox);
          optionsRadio->setToolTip(remove ? buttonProperties[i].removeToolTip : buttonProperties[i].addToolTip);
          optionsRadio->setChecked(i == BomForModel);
          if (i == BomForSubmodel)
              optionsRadio->setEnabled(!buttonList[BomAtModel]->isChecked());
          QObject::connect(optionsRadio, SIGNAL(clicked(bool)),
                           this,         SLOT(buttonClicked(bool)));
          boxLayout->addWidget(optionsRadio);
          buttonList.append(optionsRadio);
        }
      }
    }
  }

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()), SLOT(reject()));

  dialogLayout->addWidget(buttonBox);

  setLayout(dialogLayout);

  setModal(true);

  setMinimumSize(40,20);
}

BomOptionDialog::~BomOptionDialog()
{
}

void BomOptionDialog::buttonClicked(bool enable)
{
  Q_UNUSED(enable)
  if (subModelOptions) {
    buttonList[BomForSubmodel]->setEnabled(!buttonList[BomAtModel]->isChecked());
    if (sender() == buttonList[BomAtModel]) {
      if (buttonList[BomForSubmodel]->isChecked()) {
        buttonList[BomForSubmodel]->setChecked(false);
        buttonList[BomForModel]->setChecked(true);
      }
    } else if (sender() != buttonList[BomAtModel]) {
      if (buttonList[BomForModel]->isChecked()) {
        buttonList[BomForModel]->setChecked(false);
        buttonList[BomForSubmodel]->setChecked(true);
      }
    } else if (sender() == buttonList[BomForSubmodel]) {
      if (buttonList[BomAtModel]->isChecked()) {
        buttonList[BomAtModel]->setChecked(false);
        buttonList[BomAtPage]->setChecked(true);
      }
    }
  }
}

int BomOptionDialog::getOption(bool &fullBom, int modelIndex, QWidget *parent, bool remove)
{
  fullBom = true;
  int result = 0;
  BomOptionDialog *dialog = new BomOptionDialog(modelIndex, remove, parent);
  if (dialog->exec() == QDialog::Accepted) {
    for (int i = 0; i < dialog->buttonList.size(); i++) {
      if (i < BomAtSubmodel) {
        if (dialog->buttonList.at(i)->isChecked())
          result = i + 1; // adjust for 0 index where 0 is No Action
      } else if (modelIndex) {
        if (i < BomForModel)
          if (dialog->buttonList.at(i)->isChecked())
            result = i + 1;
        if (dialog->buttonList.at(BomForSubmodel)->isChecked())
          fullBom = false;
      }
    }
  }
  return result;
}
