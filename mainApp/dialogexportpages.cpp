/****************************************************************************
**
** Copyright (C) 2015 - 2017 Trevor SANDY. All rights reserved.
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

#include "dialogexportpages.h"
#include "ui_dialogexportpages.h"
#include "lpub_preferences.h"
#include "lpub.h"

DialogExportPages::DialogExportPages(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogExportPages)
{
  ui->setupUi(this);

  bool preview = gui->m_previewDialog;

  if (! preview || gui->exportType != EXPORT_PDF) {
      ui->groupBoxMixedPageSizes->setVisible(false);
      this->resize(350,260);
      ui->line->setGeometry(10,203,331,20);
      ui->buttonBoxExportPages->setGeometry(10,220,331,32);
    } else {
      ui->checkBoxIgnoreMixedPageSizes->setChecked(Preferences::ignoreMixedPageSizesMsg);
    }
  ui->radioButtonAllPages->setChecked(true);
  ui->checkBoxResetCache->setChecked(false);
  ui->labelAllPages->setText(QString("1 to %1").arg(gui->maxPages));
  ui->labelCurrentPage->setText(QString("%1").arg(gui->displayPageNum));

  switch(gui->exportType){
    case EXPORT_PDF:      
      setWindowTitle(tr("%1 pdf").arg(preview ? "Preview":"Export to"));
      ui->groupBoxPrintOptions->setTitle(tr("%1 options").arg(preview ? "Preview":"Export to pdf"));
      ui->checkBoxResetCache->setText(tr("Reset all caches before %1 pdf").arg(preview ? "previewing":"exporting"));
      ui->checkBoxResetCache->setToolTip(tr("Check to clear all caches before %1 pdf").arg(preview ? "previewing":"exporting"));
      break;
    case EXPORT_PNG:
      setWindowTitle(tr("Export as png"));
      ui->groupBoxPrintOptions->setTitle("Export as png options");
      ui->checkBoxResetCache->setText(tr("Reset all caches before export to png"));
      ui->checkBoxResetCache->setToolTip(tr("Check to clear all caches before export to png"));
      break;
    case EXPORT_JPG:
      setWindowTitle(tr("Export as jpg"));
      ui->groupBoxPrintOptions->setTitle("Export as jpg options");
      ui->checkBoxResetCache->setText(tr("Reset all caches before export to jpg"));
      ui->checkBoxResetCache->setToolTip(tr("Check to clear all caches before export to jpg"));
      break;
    case EXPORT_BMP:
      setWindowTitle(tr("Export as bmp"));
      ui->groupBoxPrintOptions->setTitle("Export as bmp options");
      ui->checkBoxResetCache->setText(tr("Reset all caches before content export to bmp"));
      ui->checkBoxResetCache->setToolTip(tr("Check to clear all caches before export to bmp"));
      break;
    }
}

DialogExportPages::~DialogExportPages()
{
  delete ui;
}

QString const DialogExportPages::pageRangeText(){
  return ui->lineEditPageRange->text();
}

bool DialogExportPages::allPages(){
  return ui->radioButtonAllPages->isChecked();
}

bool DialogExportPages::currentPage(){
  return ui->radioButtonCurrentPage->isChecked();
}

bool DialogExportPages::pageRange(){
  return ui->radioButtonPageRange->isChecked();
}

bool DialogExportPages::resetCache(){
  return ui->checkBoxResetCache->isChecked();
}

bool DialogExportPages::ignoreMixedPageSizesMsg(){
  return ui->checkBoxIgnoreMixedPageSizes->isChecked();
}

void DialogExportPages::on_lineEditPageRange_textChanged(const QString &arg1)
{
    ui->radioButtonPageRange->setChecked(true);
}

