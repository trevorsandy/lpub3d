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

#include "dialogexportpages.h"
#include "ui_dialogexportpages.h"

DialogExportPages::DialogExportPages(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogExportPages)
{
  ui->setupUi(this);

  ui->radioButtonAllPages->setChecked(true);
  ui->checkBoxResetCache->setChecked(false);
  ui->labelAllPages->setText(QString("1 to %1").arg(gui->maxPages));
  ui->labelCurrentPage->setText(QString("%1").arg(gui->displayPageNum));

  bool preview = gui->m_previewDialog;

  switch(gui->exportType){
    case EXPORT_PDF:      
      setWindowTitle(tr("%1 pdf").arg(preview ? "Preview":"Print to"));
      ui->groupBoxPrintOptions->setTitle(tr("%1 options").arg(preview ? "Preview":"Print to pdf"));
      ui->checkBoxResetCache->setText(tr("Reset all caches before %1 pdf").arg(preview ? "previewing":"printing"));
      ui->checkBoxResetCache->setToolTip(tr("Check to clear all caches before %1 pdf").arg(preview ? "previewing":"printing"));
      ui->groupBoxMixedPageSize->setEnabled(! preview);
      ui->checkBoxMixedPageSize->setChecked(! preview);
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

bool DialogExportPages::mixedPageSize(){
    return ui->checkBoxMixedPageSize->isChecked();
}

