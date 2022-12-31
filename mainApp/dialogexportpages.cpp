/****************************************************************************
**
** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.
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
#include "commonmenus.h"

DialogExportPages::DialogExportPages(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogExportPages)
{
    ui->setupUi(this); 

    setWhatsThis(lpubWT(WT_DIALOG_EXPORT_PAGES,windowTitle()));

    static const QString directionNames[] =
    {
      tr("Unspecified"), //DIRECTION_NOT_SET,
      tr("Next"),        //PAGE_NEXT,
      tr("Forward"),     //PAGE_JUMP_FORWARD,
      tr("Previous"),    //PAGE_PREVIOUS,
      tr("Backward")     //PAGE_JUMP_BACKWARD
    };

    bool ok[2] = {false, false};
    bool preview = gui->m_previewDialog;
    setLineEditResetAct = ui->lineEditPageRange->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
    setLineEditResetAct->setEnabled(false);
    connect(ui->lineEditPageRange, SIGNAL(textEdited(const QString &)), this, SLOT(enableLineEditPageRangeReset(const QString &)));
    connect(setLineEditResetAct, SIGNAL(triggered()), this, SLOT(lineEditPageRangeReset()));

    QRegExp rx("^(\\d+)(?:[^0-9]*|[a-zA-Z\\s]*)(\\d+)?$", Qt::CaseInsensitive);
    if (gui->setPageLineEdit->displayText().trimmed().contains(rx)) {
        rangeMin = rx.cap(1).toInt(&ok[0]);
        rangeMax = rx.cap(2).toInt(&ok[1]);
    }

    if (Gui::pageDirection < PAGE_BACKWARD) {
        if (ok[0] && rangeMin > Gui::displayPageNum && rangeMax == 0) {
            rangeMax = rangeMin;
            rangeMin = Gui::displayPageNum;
            ok[1]    = true;
        }
    } else {
        if (ok[0] && rangeMin < Gui::displayPageNum && rangeMax == 0) {
            rangeMax = Gui::displayPageNum;
            ok[1]    = true;
        }
    }

    if (!ok[0] || !ok[1] || rangeMin < 1 + Gui::pa || rangeMax > Gui::maxPages)
        QMessageBox::critical(this,tr("Invalid Range"),
                              tr("The page range '%1' is invaid. Valid format is <number>[ <characters> <number>].")
                              .arg(gui->setPageLineEdit->displayText()));

    if (Gui::m_exportMode != PAGE_PROCESS) {
        ui->doNotShowPageProcessDlgChk->hide();
        ui->pageProcessingContinuousGrpBox->hide();
    }

    ui->displayPageProcessingErrorsChk->setChecked(Preferences::displayPageProcessingErrors);
    connect(ui->displayPageProcessingErrorsChk, SIGNAL(clicked(bool)),
            this,                               SLOT(displayPageProcessingErrors(bool)));

    if ((! preview) &&
        (Gui::m_exportMode == EXPORT_POVRAY ||
         Gui::m_exportMode == EXPORT_STL    ||
         Gui::m_exportMode == EXPORT_3DS_MAX)){
        flag = Gui::m_exportMode == EXPORT_POVRAY ? NativePOVIni :
               Gui::m_exportMode == EXPORT_STL ? NativeSTLIni : Native3DSIni;
//        flag = Gui::m_exportMode == EXPORT_POVRAY ? NativePOVIni : NativeSTLIni;
        ui->gropuBoxLDVExport->setTitle(tr("%1 Export Settings")
                                           .arg(iniFlagNames[IniFlag(flag)]));
    } else {
        ui->gropuBoxLDVExport->hide();
    }

    if (! preview && Gui::m_exportMode != EXPORT_PDF) {
        ui->groupBoxMixedPageSizes->hide();
    } else {
        ui->checkBoxIgnoreMixedPageSizes->setChecked(Preferences::ignoreMixedPageSizesMsg);
    }

    ui->spinPixelRatio->setValue(gui->exportPixelRatio);
    ui->spinPixelRatio->setToolTip(tr("Change the export DPI pixel ratio. When not 1.0, Use Page Image is automatically is selected."));

    ui->checkBoxResetCache->setChecked(false);

    int pages = 1 + Gui::pa;
    if (Gui::pageDirection < PAGE_BACKWARD) {
        if (Gui::m_exportMode == PAGE_PROCESS) {
            pages = Gui::displayPageNum < Gui::maxPages ? Gui::displayPageNum + 1 : Gui::displayPageNum;
            if (rangeMin == Gui::displayPageNum && rangeMin < Gui::maxPages)
                rangeMin = rangeMin + 1;
        }
        ui->labelAllPages->setText(QString("%1 to %2").arg(pages).arg(Gui::maxPages));
        ui->lineEditPageRange->setText(QString("%1 - %2").arg(rangeMin).arg(rangeMax));
        allPagesInRange = pages == 1 + Gui::pa;
    } else {
        pages = Gui::maxPages;
        if (Gui::m_exportMode == PAGE_PROCESS) {
            pages = Gui::displayPageNum > 1 ? Gui::displayPageNum - 1 : Gui::displayPageNum;
            if (rangeMax == Gui::displayPageNum && rangeMax > 1)
                rangeMax = rangeMax - 1;
        }
        ui->labelAllPages->setText(QString("%1 to %2").arg(pages).arg(1 + Gui::pa));
        ui->lineEditPageRange->setText(QString("%1 - %2").arg(rangeMax).arg(rangeMin));
        allPagesInRange = pages == Gui::maxPages;
    }
    ui->labelCurrentPage->setText(QString("%1").arg(Gui::displayPageNum));

    if (Gui::m_exportMode > PAGE_PROCESS && Gui::m_exportMode < POVRAY_RENDER)
        ui->radioButtonAllPages->setChecked(true);
    else
        ui->radioButtonPageRange->setChecked(true);

    switch(Gui::m_exportMode){
    case PRINT_FILE:
        setWindowTitle(preview ? tr("Print Preview File") : tr("Print To File"));
        ui->groupBoxPrintOptions->setTitle(preview ? tr("Print preview options") : tr("Print to file options"));
        ui->checkBoxResetCache->setText(preview ? tr("Reset all caches before previewing file") : tr("Reset all caches before exporting file"));
        ui->checkBoxResetCache->setToolTip(preview ? tr("Check to reset all caches before previewing file"): tr("Check to reset all caches before exporting file"));
        ui->displayPageProcessingErrorsChk->setToolTip(tr("Display print to file warnings and errors in a message box, messages will also be logged."));
        break;
    case EXPORT_PDF:
        setWindowTitle(preview ? tr("Export Preview PDF") : tr("Export To PDF"));
        ui->groupBoxPrintOptions->setTitle(preview ? tr("Export preview options") : tr("Export to pdf options"));
        ui->checkBoxResetCache->setText(preview ? tr("Reset all caches before previewing pdf") : tr("Reset all caches before exporting pdf"));
        ui->checkBoxResetCache->setToolTip(preview ? tr("Check to reset all caches before previewing pdf"): tr("Check to reset all caches before exporting pdf"));
        ui->displayPageProcessingErrorsChk->setToolTip(tr("Display export to PDF warnings and errors in a message box, messages will also be logged."));
        break;
    case EXPORT_PNG:
        setWindowTitle(tr("Export As PNG"));
        ui->groupBoxPrintOptions->setTitle(tr("Export as PNG options"));
        ui->checkBoxResetCache->setText(tr("Reset all caches before export to PNG"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to PNG"));
        ui->displayPageProcessingErrorsChk->setToolTip(tr("Display export to PNG warnings and errors in a message box, messages will also be logged."));
        break;
    case EXPORT_JPG:
        setWindowTitle(tr("Export As JPG"));
        ui->groupBoxPrintOptions->setTitle(tr("Export as JPG options"));
        ui->checkBoxResetCache->setText(tr("Reset all caches before export to JPG"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to JPG"));
        ui->displayPageProcessingErrorsChk->setToolTip(tr("Display export to JPG warnings and errors in a message box, messages will also be logged."));
        break;
    case EXPORT_BMP:
        setWindowTitle(tr("Export As BMP"));
        ui->groupBoxPrintOptions->setTitle(tr("Export as BMP options"));
        ui->checkBoxResetCache->setText(tr("Reset all caches before content export to BMP"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to BMP"));
        ui->displayPageProcessingErrorsChk->setToolTip(tr("Display export to BMP warnings and errors in a message box, messages will also be logged."));
        break;

    case EXPORT_3DS_MAX:
        setWindowTitle(tr("Export As 3DS"));
        ui->groupBoxPrintOptions->setTitle(tr("Export as 3DS options"));
        ui->checkBoxResetCache->setText(tr("Reset all caches before content export to 3DS"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to 3DS"));
        ui->displayPageProcessingErrorsChk->setToolTip(tr("Display export to 3DS warnings and errors in a message box, messages will also be logged."));
        break;

    case EXPORT_COLLADA:
        setWindowTitle(tr("Export as DAE"));
        ui->groupBoxPrintOptions->setTitle(tr("Export as DAE options"));
        ui->checkBoxResetCache->setText(tr("Reset all caches before content export to DAE"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to DAE"));
        ui->displayPageProcessingErrorsChk->setToolTip(tr("Display export to DAE warnings and errors in a message box, messages will also be logged."));
        break;

    case EXPORT_WAVEFRONT:
        setWindowTitle(tr("Export As OBJ"));
        ui->groupBoxPrintOptions->setTitle(tr("Export as OBJ options"));
        ui->checkBoxResetCache->setText(tr("Reset all caches before content export to OBJ"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to OBJ"));
        ui->displayPageProcessingErrorsChk->setToolTip(tr("Display export to OBJ warnings and errors in a message box, messages will also be logged."));
        break;

    case EXPORT_STL:
        setWindowTitle(tr("Export As STL"));
        ui->groupBoxPrintOptions->setTitle(tr("Export as STL options"));
        ui->checkBoxResetCache->setText(tr("Reset all caches before content export to STL"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to STL"));
        ui->displayPageProcessingErrorsChk->setToolTip(tr("Display export to STL warnings and errors in a message box, messages will also be logged."));
        break;

    case EXPORT_POVRAY:
        setWindowTitle(tr("Export As POV"));
        ui->groupBoxPrintOptions->setTitle(tr("Export as POV options"));
        ui->checkBoxResetCache->setText(tr("Reset all caches before content export to POV"));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before export to POV"));
        ui->displayPageProcessingErrorsChk->setToolTip(tr("Display export to POV warnings and errors in a message box, messages will also be logged."));
        break;

    case PAGE_PROCESS:
        QString direction = directionNames[Gui::pageDirection];
        setWindowTitle(tr("%1 Page Continuous Processing").arg(direction));
        ui->groupBoxPrintOptions->setTitle(tr("Page Processing Options"));
        ui->checkBoxResetCache->setText(tr("Reset all caches before %1 page processing").arg(direction.toLower()));
        ui->checkBoxResetCache->setToolTip(tr("Check to reset all caches before %1 page processing").arg(direction.toLower()));
        ui->displayPageProcessingErrorsChk->setToolTip(tr("Display page processing warnings and errors in a message box, messages will also be logged."));

        ui->pageProcessingContinuousGrpBox->show();
        ui->doNotShowPageProcessDlgChk->show();
        ui->doNotShowPageProcessDlgChk->setEnabled(true);
        ui->labelCurrentPage->hide();
        ui->radioButtonCurrentPage->hide();
        ui->groupBoxMixedPageSizes->hide();
        break;
    }

    switch(Gui::m_exportMode) {
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

void DialogExportPages::displayPageProcessingErrors(bool b)
{
    Q_UNUSED(b)
    QSettings Settings;
    if (Preferences::displayPageProcessingErrors != ui->displayPageProcessingErrorsChk->isChecked()) {
        Preferences::displayPageProcessingErrors = ui->displayPageProcessingErrorsChk->isChecked();
        QString const displayPageProcessingErrorsKey("DisplayPageProcessingErrors");
        QVariant uValue(Preferences::displayPageProcessingErrors);
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,displayPageProcessingErrorsKey),uValue);
    }
}

void DialogExportPages::lineEditPageRangeReset()
{
    if (ui->lineEditPageRange) {
        setLineEditResetAct->setEnabled(false);
        ui->lineEditPageRange->setText(Gui::pageDirection < PAGE_BACKWARD ?
                                           QString("%1 - %2").arg(rangeMin).arg(rangeMax) :
                                           QString("%1 - %2").arg(rangeMax).arg(rangeMin));
    }
}

void DialogExportPages::enableLineEditPageRangeReset(const QString &displayText)
{
    if (ui->lineEditPageRange)
        setLineEditResetAct->setEnabled(Gui::pageDirection < PAGE_BACKWARD ?
                                            displayText != QString("%1 - %2").arg(rangeMin).arg(rangeMax) :
                                            displayText != QString("%1 - %2").arg(rangeMax).arg(rangeMin));
}

void DialogExportPages::groupBoxPixelRatio(bool show)
{
    ui->groupBoxPixelRatio->setVisible(show);
}

void DialogExportPages::getPixelRatioMsg(double value)
{
    const int dpi = value*int(resolution());
    QString exportedPixels = tr("DPI %1px").arg(dpi);
    QString pixelUnits = tr("Dots Per Inch (DPI) %1 pixels").arg(dpi);
    if (resolutionType() == DPCM) {
        const int dpcm = inches2centimeters(dpi);
        exportedPixels = tr("DPCM %1px").arg(dpcm);
        pixelUnits = tr("Dots Per Centimetre (DPCM) %1 pixels").arg(dpcm);
    }
    ui->labelPixelRatioFactor->setText(tr("Exported %1").arg(exportedPixels));
    ui->labelPixelRatioFactor->setToolTip(tr("The exported build instruction document %1.").arg(pixelUnits));
    ui->checkBoxPdfPageImage->setVisible(Gui::m_exportMode == EXPORT_PDF);
    ui->checkBoxPdfPageImage->setChecked(Preferences::pdfPageImage || value > 1.0 || value < 1.0);
    ui->checkBoxPdfPageImage->setEnabled(value == 1.0);
}

QString const DialogExportPages::pageRangeText(){
    return ui->lineEditPageRange->text();
}

QString const DialogExportPages::allPagesRangeText(){
    return ui->labelAllPages->text().replace("to", "-");
}

bool DialogExportPages::allPages(){
    return ui->radioButtonAllPages->isChecked();
}

bool DialogExportPages::allPagesRange(){
    return !allPagesInRange;
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
    const QString allCompare = ui->labelAllPages->text().replace("to","-").replace(" ","");
    const QString rangeCompare = Gui::pageDirection < PAGE_BACKWARD ? QString("%1-%2").arg(rangeMin).arg(rangeMax) :
                                                                      QString("%1-%2").arg(rangeMax).arg(rangeMin);
//    qDebug() << "AllPages: ["
//             << qPrintable(allCompare.trimmed())
//             << "] compared to PageRange: ["
//             << qPrintable(rangeCompare.trimmed())
//             << "]"
//             << "";
    if (allCompare.trimmed() != rangeCompare.trimmed())
        ui->radioButtonPageRange->setChecked(true);

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
