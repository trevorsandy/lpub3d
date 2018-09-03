/****************************************************************************
**
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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
  linePageRange  = gui->setPageLineEdit->displayText().replace("of","to");

  if (gui->exportType != PAGE_PROCESS) {
      ui->doNotShowPageProcessDlgChk->hide();
      this->resize(350,318);
      ui->buttonBoxExportPages->setGeometry(10,277,331,32);
    }

  if (! preview && gui->exportType != EXPORT_PDF) {
      ui->groupBoxMixedPageSizes->hide();
      this->resize(350,260);
      ui->buttonBoxExportPages->setGeometry(10,220,331,32);
    } else {
      ui->checkBoxIgnoreMixedPageSizes->setChecked(Preferences::ignoreMixedPageSizesMsg);
    }

  ui->radioButtonAllPages->setChecked(true);
  ui->checkBoxResetCache->setChecked(false);
  ui->labelAllPages->setText(QString("1 to %1").arg(gui->maxPages));
  ui->labelCurrentPage->setText(QString("%1").arg(gui->displayPageNum));
  ui->lineEditPageRange->setText(QString(linePageRange).replace("to","-").replace(" ",""));

  switch(gui->exportType){
    case EXPORT_PDF:      
      setWindowTitle(tr("%1 pdf").arg(preview ? "Preview":"Export to"));
      ui->groupBoxPrintOptions->setTitle(tr("%1 options").arg(preview ? "Preview":"Export to pdf"));
      ui->checkBoxResetCache->setText(tr("Reset all caches before %1 pdf").arg(preview ? "previewing":"exporting"));
      ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before %1 pdf").arg(preview ? "previewing":"exporting"));
      break;
    case EXPORT_PNG:
      setWindowTitle(tr("Export as png"));
      ui->groupBoxPrintOptions->setTitle("Export as png options");
      ui->checkBoxResetCache->setText(tr("Reset all caches before export to png"));
      ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to png"));
      break;
    case EXPORT_JPG:
      setWindowTitle(tr("Export as jpg"));
      ui->groupBoxPrintOptions->setTitle("Export as jpg options");
      ui->checkBoxResetCache->setText(tr("Reset all caches before export to jpg"));
      ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to jpg"));
      break;
    case EXPORT_BMP:
      setWindowTitle(tr("Export as bmp"));
      ui->groupBoxPrintOptions->setTitle("Export as bmp options");
      ui->checkBoxResetCache->setText(tr("Reset all caches before content export to bmp"));
      ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to bmp"));
      break;
    case PAGE_PROCESS:
      QString direction = gui->pageDirection == PAGE_NEXT ? "Next" : "Previous";
      setWindowTitle(tr("%1 page continuous processing").arg(direction));
      ui->groupBoxPrintOptions->setTitle("Page processing options");
      ui->checkBoxResetCache->setText(tr("Reset all caches before %1 page processing").arg(direction.toLower()));
      ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before %1 page processing").arg(direction.toLower()));

      ui->doNotShowPageProcessDlgChk->show();
      ui->doNotShowPageProcessDlgChk->setEnabled(true);
      ui->labelCurrentPage->hide();
      ui->radioButtonCurrentPage->hide();
      ui->groupBoxMixedPageSizes->hide();
      ui->radioButtonPageRange->setGeometry(20,63,111,20);
      ui->lineEditPageRange->setGeometry(134,63,181,22);
      ui->groupBoxPrintOptions->setGeometry(10,17,331,111);
      ui->groupBoxExportCache->setGeometry(10,136,331,51);
      ui->doNotShowPageProcessDlgChk->setGeometry(30,195,251,20);
      ui->buttonBoxExportPages->setGeometry(10,230,331,32);
      this->resize(350,273);
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

bool DialogExportPages::doNotShowPageProcessDlg(){
  return ui->doNotShowPageProcessDlgChk->isChecked();
}

void DialogExportPages::on_lineEditPageRange_textChanged(const QString &arg1)
{
  Q_UNUSED(arg1)
    // ignore automatically setting check if range is same as all pages
    if (linePageRange != ui->labelAllPages->text())
        ui->radioButtonPageRange->setChecked(true);
}

