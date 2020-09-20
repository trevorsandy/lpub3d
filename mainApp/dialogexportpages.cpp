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
        (gui->exportMode == EXPORT_POVRAY ||
         gui->exportMode == EXPORT_STL    ||
         gui->exportMode == EXPORT_3DS_MAX)){
        flag = gui->exportMode == EXPORT_POVRAY ? NativePOVIni :
               gui->exportMode == EXPORT_STL ? NativeSTLIni : Native3DSIni;
//        flag = gui->exportMode == EXPORT_POVRAY ? NativePOVIni : NativeSTLIni;
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

    ui->spinPixelRatio->setValue(gui->exportPixelRatio);
    ui->spinPixelRatio->setToolTip(QString("Change the export DPI pixel ratio."));

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
        setWindowTitle(tr("Export as PNG"));
        ui->groupBoxPrintOptions->setTitle("Export as PNG options");
        ui->checkBoxResetCache->setText(tr("Reset all caches before export to PNG"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to PNG"));
        break;
    case EXPORT_JPG:
        setWindowTitle(tr("Export as JPG"));
        ui->groupBoxPrintOptions->setTitle("Export as JPG options");
        ui->checkBoxResetCache->setText(tr("Reset all caches before export to JPG"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to JPG"));
        break;
    case EXPORT_BMP:
        setWindowTitle(tr("Export as BMP"));
        ui->groupBoxPrintOptions->setTitle("Export as BMP options");
        ui->checkBoxResetCache->setText(tr("Reset all caches before content export to BMP"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to BMP"));
        break;

    case EXPORT_3DS_MAX:
        setWindowTitle(tr("Export as 3DS"));
        ui->groupBoxPrintOptions->setTitle("Export as 3DS options");
        ui->checkBoxResetCache->setText(tr("Reset all caches before content export to 3DS"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to 3DS"));
        break;

    case EXPORT_COLLADA:
        setWindowTitle(tr("Export as DAE"));
        ui->groupBoxPrintOptions->setTitle("Export as DAE options");
        ui->checkBoxResetCache->setText(tr("Reset all caches before content export to DAE"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to DAE"));
        break;

    case EXPORT_WAVEFRONT:
        setWindowTitle(tr("Export as OBJ"));
        ui->groupBoxPrintOptions->setTitle("Export as OBJ options");
        ui->checkBoxResetCache->setText(tr("Reset all caches before content export to OBJ"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to OBJ"));
        break;

    case EXPORT_STL:
        setWindowTitle(tr("Export as STL"));
        ui->groupBoxPrintOptions->setTitle("Export as STL options");
        ui->checkBoxResetCache->setText(tr("Reset all caches before content export to STL"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to STL"));
        break;

    case EXPORT_POVRAY:
        setWindowTitle(tr("Export as POV"));
        ui->groupBoxPrintOptions->setTitle("Export as POV options");
        ui->checkBoxResetCache->setText(tr("Reset all caches before content export to POV"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to POV"));
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

    switch(gui->exportMode) {
    case EXPORT_PDF:
    case EXPORT_PNG:
    case EXPORT_JPG:
    case EXPORT_BMP:
        getPixelRatioMsg(gui->exportPixelRatio);
        groupBoxPixelRatio(true);
    break;
    default:
        groupBoxPixelRatio(false);
    break;
    }

    connect(new FocusWatcher(ui->lineEditPageRange), SIGNAL(focusChanged(bool)),
            this,                                    SLOT(  lineEditPageRangeFocusChanged(bool)));

    setMinimumSize(40,20);
    adjustSize();
}

DialogExportPages::~DialogExportPages()
{
    delete ui;
}

void DialogExportPages::groupBoxPixelRatio(bool show)
{
    ui->groupBoxPixelRatio->setVisible(show);
}

void DialogExportPages::getPixelRatioMsg(double value)
{
    ui->labelPixelRatioFactor->setText(QString("Export DPI %1px")
                                       .arg(value*int(resolution())));
    ui->checkBoxPdfPageImage->setVisible(gui->exportMode == EXPORT_PDF);
    ui->checkBoxPdfPageImage->setChecked(Preferences::pdfPageImage || value > 1.0 || value < 1.0);
    ui->checkBoxPdfPageImage->setEnabled(value == 1.0);
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

bool DialogExportPages::pdfPageImage()
{
    bool ok = ui->spinPixelRatio->value() > 1.0 || ui->spinPixelRatio->value() < 1.0 ? Preferences::pdfPageImage : true;
    return ui->checkBoxPdfPageImage->isChecked() && ok;
}

qreal DialogExportPages::exportPixelRatio(){
    return ui->spinPixelRatio->value();
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

void DialogExportPages::lineEditPageRangeFocusChanged(bool focus)
{
    // if line takes focus, set the page range radio button
    if (focus)
        ui->radioButtonPageRange->setChecked(focus);
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

void DialogExportPages::on_spinPixelRatio_valueChanged(double value)
{
   getPixelRatioMsg(value);
}
