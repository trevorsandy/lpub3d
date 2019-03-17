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
#include <LDVQt/LDVWidget.h>
#include "lpub.h"

DialogExportPages::DialogExportPages(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogExportPages)
{
    ui->setupUi(this);

    bool preview = gui->m_previewDialog;
    linePageRange  = gui->setPageLineEdit->displayText().replace("of","-").replace(" ","");

    if (gui->exportMode != PAGE_PROCESS) {
        ui->doNotShowPageProcessDlgChk->hide();
        ui->pageProcessingContinuousBox->hide();
    }

    if ((! preview) &&
        (gui->exportMode == EXPORT_POVRAY  ||
         gui->exportMode == EXPORT_STL /* ||
        gui->exportMode == EXPORT_3DS_MAX */)){
       /* flag = gui->exportMode == EXPORT_POVRAY ? NativePOVIni :
               gui->exportMode == EXPORT_STL ? NativeSTLIni : Native3DSIni; */
        flag = gui->exportMode == EXPORT_POVRAY ? NativePOVIni : NativeSTLIni ;
        ui->gropuBoxLDVExport->setTitle(QString("%1 Export Settings")
                                                .arg(iniFlagNames[IniFlag(flag)]));
    } else {
        ui->gropuBoxLDVExport->hide();
    }

    if (! preview && gui->exportMode != EXPORT_PDF) {
        ui->groupBoxMixedPageSizes->hide();
    } else {
        ui->checkBoxIgnoreMixedPageSizes->setChecked(Preferences::ignoreMixedPageSizesMsg);
    }

    ui->radioButtonAllPages->setChecked(true);
    ui->checkBoxResetCache->setChecked(false);

    if (gui->pageDirection == PAGE_NEXT){
        ui->labelAllPages->setText(QString("1 to %1").arg(gui->maxPages));
        ui->lineEditPageRange->setText(QString("%1").arg(linePageRange));
    } else {
        ui->labelAllPages->setText(QString("%1 to 1").arg(gui->maxPages));
        ui->lineEditPageRange->setText(QString("%1-%2").arg(linePageRange.section('-',1,1)).arg(linePageRange.section('-',0,0)));
    }
    ui->labelCurrentPage->setText(QString("%1").arg(gui->displayPageNum));

    switch(gui->exportMode){
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

    case EXPORT_3DS_MAX:
        setWindowTitle(tr("Export as 3ds"));
        ui->groupBoxPrintOptions->setTitle("Export as 3ds options");
        ui->checkBoxResetCache->setText(tr("Reset all caches before content export to 3ds"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to 3ds"));
        break;

    case EXPORT_COLLADA:
        setWindowTitle(tr("Export as dae"));
        ui->groupBoxPrintOptions->setTitle("Export as dae options");
        ui->checkBoxResetCache->setText(tr("Reset all caches before content export to dae"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to dae"));
        break;

    case EXPORT_WAVEFRONT:
        setWindowTitle(tr("Export as obj"));
        ui->groupBoxPrintOptions->setTitle("Export as obj options");
        ui->checkBoxResetCache->setText(tr("Reset all caches before content export to obj"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to obj"));
        break;

    case EXPORT_STL:
        setWindowTitle(tr("Export as stl"));
        ui->groupBoxPrintOptions->setTitle("Export as stl options");
        ui->checkBoxResetCache->setText(tr("Reset all caches before content export to stl"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to stl"));
        break;

    case EXPORT_POVRAY:
        setWindowTitle(tr("Export as pov"));
        ui->groupBoxPrintOptions->setTitle("Export as pov options");
        ui->checkBoxResetCache->setText(tr("Reset all caches before content export to pov"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to pov"));
        break;

    case PAGE_PROCESS:
        QString direction = gui->pageDirection == PAGE_NEXT ? "Next" : "Previous";
        setWindowTitle(tr("%1 page continuous processing").arg(direction));
        ui->groupBoxPrintOptions->setTitle("Page processing options");
        ui->checkBoxResetCache->setText(tr("Reset all caches before %1 page processing").arg(direction.toLower()));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before %1 page processing").arg(direction.toLower()));

        ui->pageProcessingContinuousBox->show();
        ui->doNotShowPageProcessDlgChk->show();
        ui->doNotShowPageProcessDlgChk->setEnabled(true);
        ui->labelCurrentPage->hide();
        ui->radioButtonCurrentPage->hide();
        ui->groupBoxMixedPageSizes->hide();
        break;
    }
    setMinimumSize(40,20);
    adjustSize();
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

int DialogExportPages::pageDisplayPause(){
    return ui->pageDisplayPauseSpin->value();
}

void DialogExportPages::on_lineEditPageRange_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)

    // ignore automatically setting check if range is same as all pages
    if (gui->pageDirection == PAGE_NEXT) {
        if (QString("%1 to %2")
                .arg(linePageRange.section('-',0,0))
                .arg(linePageRange.section('-',1,1)) != ui->labelAllPages->text() )
            ui->radioButtonPageRange->setChecked(true);
    } else {
        if (QString("%1 to %2")
                .arg(linePageRange.section('-',1,1))
                .arg(linePageRange.section('-',0,0)) != ui->labelAllPages->text() )
            ui->radioButtonPageRange->setChecked(true);
    }
}

void DialogExportPages::on_lineEditPageRange_selectionChanged()
{
    // if line selected, move radio to page range
    ui->radioButtonPageRange->setChecked(true);
}

void DialogExportPages::on_pushButtonExportSettings_clicked()
{
    ldvWidget = new LDVWidget(this,IniFlag(flag),true);
    ldvWidget->showLDVExportOptions();
}

void DialogExportPages::on_pushButtonExportPreferences_clicked()
{
    ldvWidget = new LDVWidget(this,IniFlag(flag),true);
    ldvWidget->showLDVPreferences();
}
