/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2023 Trevor SANDY. All rights reserved.
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

#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QSettings>
#include <QPainter>
#include <QPrinter>
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QString>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QErrorMessage>
#include <algorithm>

#include "paths.h"
#include "lpub.h"
#include "progress_dialog.h"
#include "dialogexportpages.h"
#include "messageboxresizable.h"

#include <TCFoundation/TCUserDefaults.h>
#include <LDLib/LDUserDefaultsKeys.h>

// Set to enable print/export trace logging
#ifndef PAGE_PRINT_DEBUG
//#define PAGE_PRINT_DEBUG
#endif

static bool exportPdf    = true;
static bool exportPreview = true;
static QString exportTime;

// Compare two variants.
bool lessThan(const int &v1, const int &v2)
{
    return v1 < v2;
}

QPageLayout Gui::getPageLayout(bool nextPage){

  int pageNum = displayPageNum;
  if (nextPage && displayPageNum < maxPages){
      pageNum = displayPageNum + 1;            //next page
    }


  QMap<int,PageSizeData>::iterator i = pageSizes.find(pageNum);
  if (i != pageSizes.end()){

      // determine page orientation
      bool  ls = i.value().orientation == Landscape;
      QPageLayout::Orientation layoutOrientation = ls ? QPageLayout::Landscape : QPageLayout::Portrait;
      // determine the page size in inches. only concerned with inches because resolution() reports DPI
      qreal pageWidthIn, pageHeightIn;
      pageWidthIn  = i.value().sizeW;
      pageHeightIn = i.value().sizeH;

      // we always want to send portrait width x height to the printer
      QPageSize pageSize(QSizeF(pageWidthIn,pageHeightIn),QPageSize::Inch,"",QPageSize::FuzzyMatch);

#ifdef PAGE_PRINT_DEBUG
      logDebug() << QString("         PAGE %5 SIZE LAYOUT - WidthIn: %1 x HeightIn: %2 Orientation: %3 DPx: %4 CurPage: %6")
                    .arg(QString::number(pageWidthIn,'f',4),
                         QString::number(pageHeightIn,'f',4))
                    .arg(layoutOrientation == QPageLayout::Landscape ? "Landscape":"Portrait")
                    .arg(page.meta.LPub.resolution.type() == DPI ? "DPI":"DPCM")
                    .arg(pageNum).arg(displayPageNum);
#endif
      return QPageLayout(pageSize, layoutOrientation, QMarginsF(0,0,0,0));
    }

  emit messageSig(LOG_ERROR,tr("Could not load page %1 layout parameters.").arg(pageNum));
  return QPageLayout();
}

void Gui::getExportPageSize(float &pageWidth, float &pageHeight,int d)
{
  QMap<int,PageSizeData>::iterator i = pageSizes.find(displayPageNum);   // this page
  if (i != pageSizes.end()){

      float pageWidthIn, pageHeightIn;

      // only concerned with inches because resolution() reports DPI
      pageWidthIn  = i.value().sizeW;
      pageHeightIn = i.value().sizeH;

      // flip orientation for exporting landscape
      if (i.value().orientation == Landscape){
          pageWidthIn  = i.value().sizeH;
          pageHeightIn = i.value().sizeW;
        }

      if (d == Pixels) {
          // pixel dimension are inches * pixels per inch
          pageWidth  = int((pageWidthIn * resolution()) + 0.5);
          pageHeight = int((pageHeightIn * resolution()) + 0.5);
        } else {
          pageWidth  = pageWidthIn;
          pageHeight = pageHeightIn;
        }
#ifdef PAGE_PRINT_DEBUG
      logDebug() << QString("  PAGE %6 SIZE PIXELS - WidthIn: %3 x HeightIn: %4 << WidthPx: %1 x HeightPx: %2 DPx: %5 CurPage: %6")
                    .arg(QString::number(pageWidth,'f',0),
                         QString::number(pageHeight,'f',0),
                         QString::number(pageWidthIn,'f',4),
                         QString::number(pageHeightIn,'f',4))
                    .arg(page.meta.LPub.resolution.type() == DPI ? "DPI":"DPCM")
                    .arg(displayPageNum);
#endif
    } else {
      emit messageSig(LOG_ERROR,tr("Could not load page %1 size parameters.").arg(displayPageNum));
    }
}

OrientationEnc Gui::getPageOrientation(bool nextPage)
{
  int pageNum = displayPageNum;
  if (nextPage && displayPageNum < maxPages){
      pageNum = displayPageNum + 1;            //next page
    }

  QMap<int,PageSizeData>::iterator i = pageSizes.find(pageNum);   // this page
  if (i != pageSizes.end()){

#ifdef PAGE_PRINT_DEBUG
      bool  ls = i.value().orientation == Landscape;
      logDebug() << QString("PAGE %2 ORIENTATION - %1 CurPage: %3")
                    .arg(ls ? "Landscape":"Portrait")
                    .arg(pageNum).arg(displayPageNum);
#endif
      return i.value().orientation;
    }
  emit messageSig(LOG_ERROR,tr("Could not load page %1 orientation parameter.").arg(pageNum));
  return InvalidOrientation;
}

void Gui::checkMixedPageSizeStatus(){

  QMessageBox box;
  QString title;
  QString text, text1, text2;

  text1 = tr ("Your document has page orientation changes (Portrait/Landscape)\n"
              "The export preview dialog will not correctly display\n"
              "orientation changes but will correctly\n"
              "apply them to your exported pdf document.\n\n");

  text2 = tr ("Your document has page size changes (A4, A5, Letter,...)\n"
              "The export preview dialog will not correctly display or\n"
              "export different pages sizes. This is a limitation\n"
              "of the underlying %1 development platform. You can export\n"
              "a mixed size and orientation document using the Export pdf\n"
              "or image (PNG, JPG, BMP) functions.\n").arg(VER_PRODUCTNAME_STR);

  float pageWidthIn          = pageSizes[DEF_SIZE].sizeW;
  float pageHeightIn         = pageSizes[DEF_SIZE].sizeH;
  QString sizeID             = pageSizes[DEF_SIZE].sizeID;
  OrientationEnc orientation = pageSizes[DEF_SIZE].orientation;
  bool orientation_warning   = false;
  bool size_warning          = false;
  bool double_warning        = false;
  int key;

  if (processOption == EXPORT_PAGE_RANGE){

      QStringList pageRanges = pageRangeText.split(",");
      QList<int> printPages;
      Q_FOREACH (QString ranges,pageRanges){
          if (ranges.contains("-")){
              QStringList range = ranges.split("-");
              int minPage = range[0].toInt();
              int maxPage = range[1].toInt();
              for(int i = minPage; i <= maxPage; i++){
                  printPages.append(i);
                }
            } else {
              printPages.append(ranges.toInt());
            }
        }

      if (printPages.size() > 1){

          bool defaultSizesUpdated = false;

          Q_FOREACH (key,printPages) {

              QMap<int,PageSizeData>::iterator i = pageSizes.find(key);   // this page
              if (i != pageSizes.end()){
#ifdef PAGE_PRINT_DEBUG
                  logDebug() << QString("%6 page %3 of %4, size(Inches) W %1 x H %2, ID %8, orientation %5 for range %7")
                                           .arg(pageSizes[key].sizeW)
                                           .arg(pageSizes[key].sizeH)
                                           .arg(key)
                                           .arg(printPages.count())
                                           .arg(pageSizes[key].orientation == Landscape ? "Landscape" : "Portrait")
                                           .arg("Checking")
                                           .arg(pageRanges.join(" "))
                                           .arg(pageSizes[key].sizeID);
#endif
                  if (! defaultSizesUpdated) {
                      pageWidthIn  = pageSizes[key].sizeW;
                      pageHeightIn = pageSizes[key].sizeH;
                      sizeID       = pageSizes[key].sizeID;
                      orientation  = pageSizes[key].orientation;
                      defaultSizesUpdated = true;
                      continue;
                    }

                  if (pageSizes[key].orientation != orientation && orientation_warning != true) {
                      orientation_warning = true;
                      text = text1;
                      title = "<b>" + tr ("Mixed page orientation detected.") + "</b>";
                      box.setIcon (QMessageBox::Information);
                    }

                  if ((pageSizes[key].sizeID != sizeID      ||
                      (pageSizes[key].sizeW  < pageWidthIn  ||
                       pageSizes[key].sizeW  > pageWidthIn) ||
                      (pageSizes[key].sizeH  < pageHeightIn ||
                       pageSizes[key].sizeH  > pageHeightIn)) && size_warning != true) {
                      size_warning = true;
                      text = text2;
                      title = "<b>" + tr ("Mixed page size detected.") + "</b>";
                      box.setIcon (QMessageBox::Warning);
                    }

                  if (orientation_warning && size_warning) {
                      double_warning = true;
                      text = text1 + text2;
                      title = "<b>" + tr ("Mixed page size and orientation detected.") + "</b>";
                      box.setIcon (QMessageBox::Warning);
                      break;
                    }
                }
            }
        }

    } else if (processOption == EXPORT_ALL_PAGES) {
#ifdef PAGE_PRINT_DEBUG
      logDebug() << tr("Default  page         size(Inches) W %1 x H %2, ID %3, orientation %4")
                       .arg(pageSizes[DEF_SIZE].sizeW)
                       .arg(pageSizes[DEF_SIZE].sizeH)
                       .arg(pageSizes[DEF_SIZE].sizeID)
                       .arg(pageSizes[DEF_SIZE].orientation == Landscape ? "Landscape" : "Portrait");
#endif
      Q_FOREACH (key,pageSizes.keys()){

          if (pageSizes[key].orientation != orientation && orientation_warning != true) {
              orientation_warning = true;
              text = text1;
              title = "<b>" + tr ("Mixed page orientation detected.") + "</b>";
              box.setIcon (QMessageBox::Information);
            }

          if ((pageSizes[key].sizeID != sizeID      ||
               pageSizes[key].sizeW  != pageWidthIn ||
               pageSizes[key].sizeH  != pageHeightIn) && size_warning != true) {
              size_warning = true;
              text = text2;
              title = "<b>" + tr ("Mixed page size detected.") + "</b>";
              box.setIcon (QMessageBox::Warning);
            }

          if (orientation_warning && size_warning) {
              double_warning = true;
              text = text1 + text2;
              title = "<b>" + tr ("Mixed page size and orientation detected.") + "</b>";
              box.setIcon (QMessageBox::Warning);
              break;
            }
        }
    }

  if (orientation_warning || size_warning || double_warning) {

      box.setTextFormat (Qt::RichText);
      box.setStandardButtons (QMessageBox::Close);
      box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      box.setWindowTitle(tr ("Export Preview"));

      box.setText (title);
      box.setInformativeText (text);

      if (Preferences::modeGUI)
        box.exec();
      else
        emit messageSig(LOG_STATUS,QString("%1").arg(text));
    }
}

bool Gui::validatePageRange() {

  QMessageBox box;
  box.setTextFormat (Qt::RichText);
  box.setIcon (QMessageBox::Information);
  box.setStandardButtons (QMessageBox::Close);
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  box.setWindowTitle(tr ("Page Range"));

  QString title;
  QString text;

  if(pageRangeText.isEmpty()){

      title = "<b>" + tr ("Empty page range.") + "</b>";
      text = tr ("You must enter a page range");

      box.setText (title);
      box.setInformativeText (text);

      if (Preferences::modeGUI)
        box.exec();
      else
        emit messageSig(LOG_STATUS,tr("Empty page range. %1").arg(text));

      return false;
    }

  bool validEntry = true;
  QString message;
  QStringList pageRanges = pageRangeText.split(",");
  for (QString const &ranges: pageRanges){
      if (ranges.contains("-")){
          bool ok[2];
          QStringList range = ranges.split("-");

          int startPage = QString(range[0].trimmed()).toInt(&ok[0]);
          int endPage = QString(range[1].trimmed()).toInt(&ok[1]);
          if (!ok[0] || !ok[1]){
              message = QString("%1-%2").arg(startPage).arg(endPage);
              validEntry = false;
              break;
          }
          if ((pageDirection == PAGE_NEXT) && (startPage > endPage)) {
              message = QString("%1-%2").arg(startPage).arg(endPage);
              validEntry = false;
              break;
          } else
            if ((pageDirection == PAGE_PREVIOUS) && (endPage > startPage)) {
                message = QString("%1-%2").arg(startPage).arg(endPage);
                validEntry = false;
                break;
            }
        } else {
          bool ok;
          int pageNum = ranges.toInt(&ok);
          if (!ok){
              message = QString("%1").arg(pageNum);
              validEntry = false;
              break;
            }
        }
    }

  if (! validEntry) {

      title = "<b>" + tr ("Invalid page number(s).") + "</b>";
      text = tr("Invalid page number(s) %1 detected. You must enter valid page number(s).").arg(message);

      box.setText (title);
      box.setInformativeText (text);

      if (Preferences::modeGUI)
        box.exec();
      else
        emit messageSig(LOG_STATUS,tr("Invalid page number(s). %1").arg(text));

      return false;
    }

  return true;
}

void Gui::consoleCommand(int mode, int* result)
{
    int savePause = Preferences::pageDisplayPause;
    if (mode == PAGE_PROCESS) {
        Preferences::pageDisplayPause = 1;
        continuousPageDialog(PAGE_NEXT);
    } else if (mode > PAGE_PROCESS && mode < EXPORT_BRICKLINK)
        exportAsDialog(ExportMode(mode));
    else if (mode == EXPORT_BRICKLINK)
        exportAsBricklinkXML();
    else if (mode == EXPORT_CSV)
        exportAsCsv();
    else if (mode == EXPORT_HTML_PARTS)
        exportAsHtml();
    else if (mode == EXPORT_HTML_STEPS)
        exportAsHtmlSteps();
    else
        exportAsDialog(EXPORT_PDF);
    Preferences::pageDisplayPause = savePause;

    *result = 0;
}

void Gui::consoleCommandCurrentThread(int mode, int *value)
{
    QThread *thread = QThread::currentThread();
    QThread *mainthread = this->thread();
    if(thread == mainthread)
        consoleCommand(mode, value);
    else
        emit consoleCommandFromOtherThreadSig(mode, value);
}

void Gui::exportAsPdfDialog(){
  exportAsDialog(EXPORT_PDF);
}

void Gui::exportAsPngDialog(){
  exportAsDialog(EXPORT_PNG);
}

void Gui::exportAsJpgDialog(){
  exportAsDialog(EXPORT_JPG);
}

void Gui::exportAsBmpDialog(){
  exportAsDialog(EXPORT_BMP);
}

void Gui::exportAsStlDialog(){
  exportAsDialog(EXPORT_STL);
}

void Gui::exportAs3dsDialog(){
  exportAsDialog(EXPORT_3DS_MAX);
}

void Gui::exportAsPovDialog(){
  exportAsDialog(EXPORT_POVRAY);
}

void Gui::exportAsColladaDialog(){
  exportAsDialog(EXPORT_COLLADA);
}

void Gui::exportAsObjDialog(){
  exportAsDialog(EXPORT_WAVEFRONT);
}

bool Gui::exportAsDialog(ExportMode m)
{
  messageList.clear();
  m_exportMode = m;

 if (m_exportMode != PAGE_PROCESS)
     pageDirection = PAGE_NEXT;

  if (Preferences::modeGUI) {
      DialogExportPages *dialog = new DialogExportPages();
      if (dialog->exec() == QDialog::Accepted) {
          if(dialog->allPages()){
              if (dialog->allPagesRange()) {
                processOption = EXPORT_PAGE_RANGE;
                pageRangeText = dialog->allPagesRangeText();
              } else {
                processOption = EXPORT_ALL_PAGES;
              }
            }
          else
          if(dialog->currentPage()){
              processOption = EXPORT_CURRENT_PAGE;
            }
          else
          if(dialog->pageRange()){
              processOption = EXPORT_PAGE_RANGE;
              pageRangeText = dialog->pageRangeText();
              if (! validatePageRange()){
                  return false;
                }
            }
        } else {
          return false;
        }

      exportPixelRatio = dialog->exportPixelRatio();

      resetCache = dialog->resetCache();

      QSettings Settings;
      if (Preferences::ignoreMixedPageSizesMsg != dialog->ignoreMixedPageSizesMsg())
      {
        Preferences::ignoreMixedPageSizesMsg = dialog->ignoreMixedPageSizesMsg();
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"IgnoreMixedPageSizesMsg "),Preferences::ignoreMixedPageSizesMsg);
      }

      if (m_exportMode == EXPORT_PDF && Preferences::pdfPageImage != dialog->pdfPageImage())
      {
        Preferences::pdfPageImage = dialog->pdfPageImage();
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PdfPageImage"),Preferences::pdfPageImage);
      }
    }

  if(resetCache)
      resetModelCache();

  if (! m_previewDialog){
      switch (m)
      {
      case EXPORT_PDF:
        // send signal to halt Visual Editor
        emit setExportingSig(true);
        exportAsPdf();
      break;
      case EXPORT_PNG:
        emit setExportingSig(true);
        exportAs(".png");
      break;
      case EXPORT_JPG:
        emit setExportingSig(true);
        exportAs(".jpg");
      break;
      case EXPORT_BMP:
        emit setExportingSig(true);
        exportAs(".bmp");
      break;
      case EXPORT_STL:
        emit setExportingObjectsSig(true);
        exportAs(".stl");
      break;
      case EXPORT_3DS_MAX:
        emit setExportingObjectsSig(true);
        exportAs(".3ds");
      break;
      case EXPORT_COLLADA:
        emit setExportingObjectsSig(true);
        exportAs(".dae");
      break;
      case EXPORT_WAVEFRONT:
        emit setExportingObjectsSig(true);
        exportAs(".obj");
      break;
      case EXPORT_POVRAY:
        setExportingObjectsSig(true);
        exportAs(".pov");
      break;
      default:
      break;
      }
    }

  return true;
}

void Gui::exportAsHtmlSteps()
{
    messageList.clear();
    lpub->Options                     = new NativeOptions();
    lpub->Options->ExportMode         = EXPORT_HTML_STEPS;
    lpub->Options->ImageType          = Options::CSI;
    lpub->Options->InputFileName      = curFile;
    if (!Gui::m_saveDirectoryName.isEmpty())
        lpub->Options->ExportFileName = QDir::toNativeSeparators(
                                        Gui::m_saveDirectoryName + "/" +
                                        Paths::htmlStepsDir);
    else
        lpub->Options->ExportFileName = QDir::toNativeSeparators(
                                        QFileInfo(curFile).absolutePath() + "/" +
                                        Paths::htmlStepsDir);

    bool saveFadeStepsFlag = Preferences::enableFadeSteps;
    if (saveFadeStepsFlag) {
        Preferences::enableFadeSteps = false;
        emit messageSig(LOG_INFO,tr("%1 Fade Previous Steps set to OFF during HTML Steps export.").arg(VER_PRODUCTNAME_STR));
    }
    if (! renderer->NativeExport(lpub->Options)) {
        emit messageSig(LOG_ERROR,QMessageBox::tr("HTML Steps export failed."));
    }
    if (saveFadeStepsFlag) {
        Preferences::enableFadeSteps = saveFadeStepsFlag;
        emit messageSig(LOG_INFO,tr("%1 Fade Previous Steps restored to ON following HTML Steps export.").arg(VER_PRODUCTNAME_STR));
    }

    lpub->Options = nullptr;
}
void Gui::exportAsHtml()
{
    messageList.clear();
    lpub->Options              = new NativeOptions();
    lpub->Options->ExportMode  = EXPORT_HTML_PARTS;
    // Visual Editor only
    lpub->Options->ImageType   = Options::CSI;
    if (!Gui::m_saveDirectoryName.isEmpty())
        lpub->Options->ExportFileName = QDir::toNativeSeparators(Gui::m_saveDirectoryName);
    else
        lpub->Options->ExportFileName = QDir::toNativeSeparators(QFileInfo(curFile).absolutePath());
    // LDV only
    lpub->Options->IniFlag            = NativePartList;

    // Capture the model's last CSI
    emit setExportingObjectsSig(true);

    // Switch to Native Renderer for fast processing
    setNativeRenderer();

    // store current display page number
    prevDisplayPageNum = displayPageNum;

    // start at the last page moving backward until we find a valid CSI
    Meta meta;
    MetaItem mi;
    DrawPageFlags dpFlags;
    m_partListCSIFile  = true;
    bool modelFound    = false;
    int pageNum        = maxPages;

    // start at the bottom of the page's last step
    Where pagePos = Gui::topOfPages[pageNum];
    pagePos.lineNumber = gui->subFileSize(pagePos.modelName);
    pagePos--;     //adjust to start at absolute bottom of file
    int numLines = gui->subFileSize(pagePos.modelName);
    QRegExp type15LineRx("^[1-5]\\s.*$");

    // traverse backwards until we find an inserted model or part line
    for (; pagePos < numLines && pagePos > 0 && ! modelFound; --pagePos) {
        QString line = readLine(pagePos);
        if(line.startsWith("0 "))
         {
            Rc rc = meta.parse(line,pagePos);
            switch (rc) {
              // if STEP_GROUP_END, scan backward to line before STEP_GROUP_BEGIN and adjust page number
              case StepGroupEndRc:
                mi.scanBackward(pagePos,StepGroupMask);
                pageNum--;
                break;
              // if STEP, scan backward to bottom of previous STEP and adjust page number
              case StepRc:
              case RotStepRc:
                mi.scanBackward(pagePos,StepMask);
                pageNum--;
                break;
              // if final MODEL or DISPLAY_MODEL
              case InsertDisplayModelRc:
              case InsertFinalModelRc:
                modelFound = true;
                break;
              default:
                break;
            }
        }
        else
        // if type 1 to 5 line
        if(line.contains(type15LineRx)) {
            break;
        }
    }

    displayPageNum = pageNum;

    // setup and export the last CSI on the page
    LGraphicsScene scene;
    LGraphicsView view(&scene);

    drawPage(&view,&scene,dpFlags);
    clearPage(&view,&scene);

    restorePreferredRenderer();
    emit setExportingSig(false);

    // reset meta
    meta = Meta();

    // return to whatever page we were viewing before capturing the CSI
    m_partListCSIFile = false;
    displayPageNum    = prevDisplayPageNum;
    displayPage();

    // create partList key
    bool noCA = Preferences::applyCALocally || meta.rotStep.value().type.toUpper() == QLatin1String("ABS");
    float partListModelScale = meta.LPub.bom.modelScale.value();
    bool suffix = QFileInfo(getCurFile()).suffix().contains(QRegExp("(dat|ldr|mpd)$",Qt::CaseInsensitive));
    QString partListKey = QString("%1_%2_%3_%4_%5_%6_%7.%8")
                                  .arg(lpub->pageSize(meta.LPub.page, 0))
                                  .arg(double(resolution()))
                                  .arg(resolutionType() == DPI ? "DPI" : "DPCM")
                                  .arg(double(partListModelScale))
                                  .arg(double(meta.LPub.bom.cameraFoV.value()))
                                  .arg(noCA ? double(0.0f) : double(meta.LPub.bom.cameraAngles.value(0)))
                                  .arg(noCA ? double(0.0f) : double(meta.LPub.bom.cameraAngles.value(1)))
                                  .arg(suffix ? QFileInfo(getCurFile()).suffix() : "ldr");

    // generate HTML parts list
    QString ldrBaseFile = QDir::currentPath()+QDir::separator()+Paths::tmpDir+QDir::separator()+QFileInfo(curFile).completeBaseName();
    QString partListFile = QDir::toNativeSeparators(ldrBaseFile+"_parts.ldr");
    if (! generateBOMPartsFile(partListFile))
        return;

    // setup snapshot image generation arguments
    QStringList arguments;
    QString snapshot    = QDir::toNativeSeparators(ldrBaseFile+"_snapshot.ldr");
    if (QFileInfo(snapshot).exists()) {
        // always use LDView settings regardless of preferred renderer
        noCA = Preferences::applyCALocally || meta.rotStep.value().type.toUpper() == QLatin1String("ABS");
        // setup camera globe (latitude, longitude) using LDView default camera distance
        QString cg = QString("-cg%1,%2") .arg(noCA ? double(0.0f) : double(meta.LPub.assem.cameraAngles.value(0)))
                                         .arg(noCA ? double(0.0f) : double(meta.LPub.assem.cameraAngles.value(1)));
        arguments << cg;
        arguments << QString("-Snapshot=\"%1\"").arg(snapshot);
        if (!Preferences::altLDConfigPath.isEmpty())
           arguments << QString("-LDConfig=\"%1\"").arg(QDir::toNativeSeparators(Preferences::altLDConfigPath));
    } else {
        emit messageSig(LOG_ERROR,QMessageBox::tr("HTML snapshot model file %1 was not found.").arg(snapshot));
    }

    // setup part list arguments
    arguments << QString("-LDrawDir=\"%1\"").arg(QDir::toNativeSeparators(Preferences::ldrawLibPath));
    arguments << QString("-PartlistKey=%1").arg(partListKey);
    arguments << QString("\"%1\"").arg(partListFile);

    lpub->Options->ExportArgs    = arguments;
    lpub->Options->InputFileName = partListFile;

    if (! renderer->NativeExport(lpub->Options)) {
        emit messageSig(LOG_ERROR,QMessageBox::tr("HTML parts list export failed."));
    }

    lpub->Options = nullptr;
}

void Gui::exportAsCsv()
{
    messageList.clear();
    lpub->Options             = new NativeOptions();
    lpub->Options->ImageType  = Options::CSI;
    lpub->Options->ExportMode = EXPORT_CSV;
    if (!saveFileName.isEmpty()) // command line user specified file name
        lpub->Options->OutputFileName = saveFileName;
    else
        lpub->Options->OutputFileName = QDir::toNativeSeparators(QDir::toNativeSeparators(QString(curFile).replace(QFileInfo(curFile).suffix(),"cvs")));
    if (Preferences::modeGUI) {
        // determine location for output file
        QFileInfo fileInfo(curFile);
        QString baseName = fileInfo.completeBaseName();
        QString fileName = QFileDialog::getSaveFileName(
              this,
              tr("Export File Name"),
              QDir::currentPath() + QDir::separator() + baseName,
              tr("CSV File (*.csv);;All Files (*.*)"));
        if (!fileName.isEmpty())
            lpub->Options->OutputFileName = fileName;
    }
    lpub->Options->InputFileName = QDir::toNativeSeparators(QDir::currentPath()+QDir::separator()+
                                   Paths::tmpDir+QDir::separator()+QFileInfo(curFile).completeBaseName()+"_parts.ldr");
    if (! generateBOMPartsFile(lpub->Options->InputFileName))
        return;
    if (! renderer->NativeExport(lpub->Options)) {
        emit messageSig(LOG_ERROR,QMessageBox::tr("CSV parts list export failed."));
    }

    lpub->Options = nullptr;
}

void Gui::exportAsBricklinkXML()
{
    messageList.clear();
    lpub->Options             = new NativeOptions();
    lpub->Options->ImageType  = Options::CSI;
    lpub->Options->ExportMode = EXPORT_BRICKLINK;
    if (!saveFileName.isEmpty()) // command line user specified file name
        lpub->Options->OutputFileName = saveFileName;
    else
        lpub->Options->OutputFileName = QDir::toNativeSeparators(QDir::toNativeSeparators(QString(curFile).replace(QFileInfo(curFile).suffix(),"xml")));
    if (Preferences::modeGUI) {
        // determine location for output file
        QFileInfo fileInfo(curFile);
        QString baseName = fileInfo.completeBaseName();
        QString fileName = QFileDialog::getSaveFileName(
              this,
              tr("Export File Name"),
              QDir::currentPath() + QDir::separator() + baseName,
              tr("XML File (*.xml)"));
        if (!fileName.isEmpty())
            lpub->Options->OutputFileName = fileName;
    }
    lpub->Options->InputFileName = QDir::toNativeSeparators(QDir::currentPath()+QDir::separator()+
                                   Paths::tmpDir+QDir::separator()+QFileInfo(curFile).completeBaseName()+"_parts.ldr");
    if (! generateBOMPartsFile(lpub->Options->InputFileName))
        return;
    if (! renderer->NativeExport(lpub->Options)) {
        emit messageSig(LOG_ERROR,QMessageBox::tr("Bricklink XML parts list export failed."));
    }

    lpub->Options = nullptr;
}

void Gui::exportAsPdf()
{
  // init drawPage flags
  DrawPageFlags dpFlags;

  // store current display page number
  prevDisplayPageNum = displayPageNum;

  // return to whatever page we were viewing before printing
  auto restoreCurrentPage = [&](bool restoreKpage = true)
  {
      displayPageNum = prevDisplayPageNum;
      if (abortProcess())
          restorePreviousPage();
      else if (restoreKpage)
          displayPage();
  };

  // add pixel ratio info to file name
  QString dpiInfo = QString("_%1_DPI").arg(int(resolution()));
  if (exportPixelRatio > 1.0 || exportPixelRatio < 1.0){
      dpiInfo += QString("_%1x").arg(exportPixelRatio);
  }

  // determine location for output file
  QFileInfo fileInfo(curFile);
  QString baseName = fileInfo.completeBaseName();
  baseName += dpiInfo;
  QString fileName = QDir::currentPath() + QDir::separator() + baseName;

  if (Preferences::modeGUI) {
      fileName = QFileDialog::getSaveFileName(
            this,
            tr("Export File Name"),
            QDir::currentPath() + QDir::separator() + baseName,
            tr("PDF File (*.pdf)"));

      if (fileName.isEmpty()) {
          // release Visual Editor
          emit setExportingSig(false);
          restoreCurrentPage(false);
          return;
      }
  } else if (!saveFileName.isEmpty()) { // command line user specified file name
      fileName = saveFileName;
  }

  // want info about output file now, not model file
  fileInfo.setFile(fileName);

  // append proper PDF file extension if needed
  QString suffix = fileInfo.suffix();
  if (suffix == "") {
      fileName += ".pdf";
    } else if (suffix != ".pdf" && suffix != ".PDF") {
      fileName = fileInfo.path() + QDir::separator() + fileInfo.completeBaseName() + ".pdf";
    }

  // QMessageBox box; // old
  QMessageBoxResizable box;
  box.setTextFormat (Qt::RichText);
  box.setIcon (QMessageBox::Critical);
  box.setStandardButtons (QMessageBox::Retry | QMessageBox::Abort);
  box.setDefaultButton   (QMessageBox::Retry);
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  box.setWindowTitle(tr ("%1 Export pdf").arg(VER_PRODUCTNAME_STR));

  // check if file open by another program
  QFile printFile(fileName);
  if ( ! printFile.open(QFile::ReadWrite)) {

      QString title = "<b>" + tr ("Cannot open file.") + "</b>";
      QString text = tr ("Cannot open %1 for writing:\n%2")
          .arg(QDir::toNativeSeparators(fileName))
          .arg(printFile.errorString());

      box.setText (title);
      box.setInformativeText (text);

      bool release = false;
      if (Preferences::modeGUI && (box.exec() == QMessageBox::Retry)) {
          if ( ! printFile.open(QFile::ReadWrite)) {
              release = true;
          }
      } else {
          release = true;
      }
      if (release) {
          emit messageSig(LOG_INFO_STATUS, tr("Cannot open file. %1").arg(text));
          // release Visual Editor
          emit setExportingSig(false);
          restoreCurrentPage(false);
          return;
      }
  }

  QElapsedTimer exportTimer;
  exportTimer.start();
  displayPageTimer.start();

  // determine size of output pages, in pixels
  float pageWidthPx = 0.0f, pageHeightPx = 0.0f;
  float pageWidthIn = 0.0f, pageHeightIn = 0.0f;
  int adjPageWidthPx = 0, adjPageHeightPx = 0;

  // create a PDF pdfWriter
  QPdfWriter pdfWriter(fileName);
  pdfWriter.setCreator(QString("%1 %2")
                       .arg(QString::fromLatin1(VER_PRODUCTNAME_STR))
                       .arg(QString::fromLatin1(VER_PRODUCTVERSION_STR)));

  // calculate device pixel ratio
  qreal dpr = exportPixelRatio;

  // set export page elements or image
  bool exportPdfElements = !Preferences::pdfPageImage && dpr == 1.0;

  QString messageIntro = exportPdfElements ? tr("Exporting page ") : tr("Step 1. Creating image for page ");

  // instantiate the scene and view
  LGraphicsScene scene;
  LGraphicsView view(&scene);

  // initialize page sizes
  displayPageNum = 0;
  dpFlags.printing = true;
  drawPage(&view,&scene,dpFlags);
  clearPage(&view,&scene);
  displayPageNum = prevDisplayPageNum;

  int _displayPageNum = 0;
  int _maxPages       = 0;

  QString message = tr("instructions to pdf...");

  // initialize progress bar dialog
  m_progressDialog->setWindowTitle("Export pdf");
  m_progressDialog->setLabelText(tr("Exporting %1").arg(message));
  if (Preferences::modeGUI) {
      m_progressDialog->setBtnToCancel();
      m_progressDialog->show();
      QApplication::setOverrideCursor(Qt::ArrowCursor);
      QCoreApplication::processEvents();
  }
  emit messageSig(LOG_INFO_STATUS,tr("Starting export %1").arg(message));

  // reset page indicators
  _displayPageNum = 0;
  _maxPages       = 0;

  // page store
  struct PdfPage {
      QImage image;
      float pageWidthIn;
      float pageHeightIn;
      QPageLayout pageLayout;
  };
  QMap<int, PdfPage> pages;

  if (processOption != EXPORT_PAGE_RANGE) {

      if(processOption == EXPORT_ALL_PAGES){
          _displayPageNum = 1 + pa;
          _maxPages = maxPages;
        }

      if (processOption == EXPORT_CURRENT_PAGE){
          _displayPageNum = displayPageNum;
          _maxPages       = displayPageNum;
        }

      message = tr("Export %1 pages to pdf").arg(_maxPages);
      emit messageSig(LOG_INFO_STATUS,message);

      if (Preferences::modeGUI) {
          m_progressDialog->setLabelText(message);
          m_progressDialog->setRange(_displayPageNum,_maxPages);
          QCoreApplication::processEvents();
      }

      // set displayPageNum so we can send the correct index to retrieve page size data
      displayPageNum = _displayPageNum;

      // set initial pdfWriter page layout
      pdfWriter.setPageLayout(getPageLayout());

      QPainter painter;
      if (exportPdfElements) {
         // initialize painter with pdfWriter
         painter.begin(&pdfWriter);
      }

      // step 1. generate page pixmaps
      for (displayPageNum = _displayPageNum; displayPageNum <= _maxPages; displayPageNum++) {

          if (! exporting()) {
              if (exportPdfElements)
                  painter.end();
              message = tr("Export to pdf terminated before completion. %1 pages of %2 processed%3.")
                           .arg(displayPageNum - 1).arg(_maxPages).arg(gui->elapsedTime(exportTimer.elapsed()));
              emit messageSig(LOG_INFO_STATUS,message);
              if (Preferences::modeGUI) {
                  QApplication::restoreOverrideCursor();
                  m_progressDialog->setBtnToClose();
                  m_progressDialog->setLabelText(message, true/*alert*/);
              }
              emit setExportingSig(false);
              restoreCurrentPage();
              return;
          }

          message = tr("%1 %2 of %3...")
                        .arg(messageIntro)
                        .arg(displayPageNum)
                        .arg(_maxPages);
          emit messageSig(LOG_INFO_STATUS,message);

          if (Preferences::modeGUI) {
              m_progressDialog->setLabelText(message);
              m_progressDialog->setValue(displayPageNum);
              QApplication::processEvents();
          }

          // get size of output image, in pixels
          getExportPageSize(pageWidthPx, pageHeightPx);
          adjPageWidthPx  = int(double(pageWidthPx)  * dpr);
          adjPageHeightPx = int(double(pageHeightPx) * dpr);

          bool  ls = getPageOrientation() == Landscape;

          message = tr("                  %8 %3 of %4, size(pixels) W %1 x H %2, orientation %5, DPI %6, pixel ratio %7...")
                       .arg(adjPageWidthPx)
                       .arg(adjPageHeightPx)
                       .arg(displayPageNum)
                       .arg(_maxPages)
                       .arg(ls ? tr("Landscape") : tr("Portrait"))
                       .arg(int(resolution()))
                       .arg(dpr)
                       .arg(messageIntro);
          emit messageSig(LOG_NOTICE,message);

          // initiialize the image
          QImage image(adjPageWidthPx, adjPageHeightPx, QImage::Format_ARGB32);
          image.setDevicePixelRatio(dpr);

          // set up the view - use unscaled page size
          QRectF boundingRect(0.0, 0.0, int(pageWidthPx),int(pageHeightPx));
          QRect bounding(0, 0, int(pageWidthPx),int(pageHeightPx));
          view.scale(1.0,1.0);
          view.setMinimumSize(int(pageWidthPx),int(pageHeightPx));
          view.setMaximumSize(int(pageWidthPx),int(pageHeightPx));
          view.setGeometry(bounding);
          view.setSceneRect(boundingRect);
          view.setRenderHints(
                QPainter::Antialiasing |
                QPainter::TextAntialiasing |
                QPainter::SmoothPixmapTransform);
          view.centerOn(boundingRect.center());
          clearPage(&view,&scene);

          // paint to the image the scene we view
          if (!exportPdfElements) {
              // initialize painter with image
              painter.begin(&image);
              // clear the pixels of the image
              image.fill(Qt::white);
          }

          // render this page
          dpFlags.printing = true;
          drawPage(&view,&scene,dpFlags);
          scene.setSceneRect(0.0,0.0,adjPageWidthPx,adjPageHeightPx);
          scene.render(&painter);
          clearPage(&view,&scene);

          if (exportPdfElements) {
              // prepare pdfWriter to render next page
              if(displayPageNum < _maxPages) {
                  bool nextPage = true;
                  pdfWriter.setPageLayout(getPageLayout(nextPage));
                  pdfWriter.newPage();
              }
          } else {
              // store the image and required page attributes
              getExportPageSize(pageWidthIn, pageHeightIn, Inches);
              PdfPage pdfPage;
              pdfPage.image       = image;
              pdfPage.pageWidthIn  = pageWidthIn;
              pdfPage.pageHeightIn = pageHeightIn;

              // store pdfWriter next page layout
              if(displayPageNum < _maxPages) {
                  bool nextPage = true;
                  pdfPage.pageLayout = getPageLayout(nextPage);
              }

              // store the rendered page
              QMap<int, PdfPage>::iterator i = pages.find(displayPageNum);
              if (i != pages.end())
                  pages.erase(i);
              pages.insert(displayPageNum,pdfPage);

              // wrap up paint to image
              painter.end();
          }
      } // end of step 1. generate page pixmaps

      if (Preferences::modeGUI)
          m_progressDialog->setValue(_maxPages);

      if (exportPdfElements) {
          // wrap up paint to pdfWriter
          painter.end();
      } else {
          // step 2. paint generated page pixmaps to the pdfWriter
          painter.begin(&pdfWriter);

          if (Preferences::modeGUI)
              m_progressDialog->setRange(1,pages.count());

          int page;
          Q_FOREACH (page, pages.keys()) {

              message = tr("Step 2. Exporting pdf document page %1 of %2").arg(page).arg(pages.count());
              emit messageSig(LOG_INFO_STATUS,message);

              if (Preferences::modeGUI) {
                  m_progressDialog->setLabelText(message);
                  m_progressDialog->setValue(page);
                  QApplication::processEvents();
              }

              if (! exporting()) {
                  painter.end();
                  message = tr("Export to pdf terminated before completion. %2 pages of %3 processed%4.")
                                .arg(page).arg(pages.count()).arg(gui->elapsedTime(exportTimer.elapsed()));
                  emit messageSig(LOG_INFO_STATUS,message);
                  if (Preferences::modeGUI) {
                      QApplication::restoreOverrideCursor();
                      m_progressDialog->setBtnToClose();
                      m_progressDialog->setLabelText(message, true/*alert*/);
                  }
                  emit setExportingSig(false);
                  restoreCurrentPage();
                  return;
              }

              // render this page's image to the pdfWriter
              painter.drawImage(QRect(0,0,
                                       int(pdfWriter.logicalDpiX()*pages[page].pageWidthIn),
                                       int(pdfWriter.logicalDpiY()*pages[page].pageHeightIn)),
                                       pages[page].image);

              // prepare to render next page
              if(page < pages.count()) {
                  pdfWriter.setPageLayout(pages[page].pageLayout);
                  pdfWriter.newPage();
              }
          }

          if (Preferences::modeGUI)
              m_progressDialog->setValue(pages.count());

          // wrap up paint to pdfWriter
          pages.clear();
          painter.end();
      }

  } else {

      QStringList pageRanges = pageRangeText.split(",");
      QList<int> printPages;
      Q_FOREACH (QString ranges,pageRanges){
          if (ranges.contains("-")){
              QStringList range = ranges.split("-");
              int minPage = range[0].toInt();
              int maxPage = range[1].toInt();
              for(int i = minPage; i <= maxPage; i++){
                  printPages.append(i);
                }
            } else {
              printPages.append(ranges.toInt());
            }
        }

      std::sort(printPages.begin(),printPages.end(),lessThan);

      message = tr("Export %1 pages to pdf").arg(printPages.count());
      emit messageSig(LOG_INFO_STATUS,message);

      if (Preferences::modeGUI) {
          m_progressDialog->setLabelText(message);
          m_progressDialog->setRange(1,printPages.count());
          QCoreApplication::processEvents();
      }

      int _pageCount = 0;

      // set displayPageNum so we can send the correct index to retrieve page size data
      displayPageNum = printPages.first();

      // set initial pdfWriter page layout
      pdfWriter.setPageLayout(getPageLayout());

      QPainter painter;
      if (exportPdfElements) {
         // initialize painter with pdfWriter
         painter.begin(&pdfWriter);
      }

      // step 1. generate page pixmaps
      Q_FOREACH (int printPage,printPages) {

          _pageCount++;

          if (! exporting()) {
              if (exportPdfElements)
                  painter.end();
              message = tr("Export to pdf terminated before completion. %2 pages of %3 processed%4.")
                            .arg(_pageCount).arg(printPages.count()).arg(gui->elapsedTime(exportTimer.elapsed()));
              emit messageSig(LOG_INFO_STATUS,message);
              if (Preferences::modeGUI) {
                  QApplication::restoreOverrideCursor();
                  m_progressDialog->setBtnToClose();
                  m_progressDialog->setLabelText(message, true/*alert*/);
              }
              emit setExportingSig(false);
              restoreCurrentPage();
              return;
          }

          displayPageNum = printPage;

          message = tr("%1 %2 (%3 of %4) from the range of %5...")
                       .arg(messageIntro)
                       .arg(displayPageNum)
                       .arg(_pageCount)
                       .arg(printPages.count())
                       .arg(pageRanges.join(" "));
          emit messageSig(LOG_INFO_STATUS,message);

          if (Preferences::modeGUI) {
              m_progressDialog->setLabelText(message);
              m_progressDialog->setValue(_pageCount);
              QApplication::processEvents();
          }

          // get size of output image, in pixels
          getExportPageSize(pageWidthPx, pageHeightPx);
          adjPageWidthPx  = int(double(pageWidthPx)  * dpr);
          adjPageHeightPx = int(double(pageHeightPx) * dpr);

          bool  ls = getPageOrientation() == Landscape;
          message = tr("%1 %2 (%3 of %4) from the range of %5, size(in pixels) W %6 x H %7, orientation %8, DPI %9")
                       .arg(messageIntro)                    //1
                       .arg(displayPageNum)                  //2
                       .arg(_pageCount)                      //3
                       .arg(printPages.count())              //4
                       .arg(pageRanges.join(" "))            //5
                       .arg(adjPageWidthPx)                  //6
                       .arg(adjPageHeightPx)                 //7
                       .arg(ls ? "Landscape" : "Portrait")   //8
                       .arg(int(resolution()));              //9
          emit messageSig(LOG_NOTICE,message);

          // initiialize the image
          QImage image(adjPageWidthPx, adjPageHeightPx, QImage::Format_ARGB32);
          image.setDevicePixelRatio(dpr);

          // set up the view - use unscaled page size
          QRectF boundingRect(0.0, 0.0, int(pageWidthPx),int(pageHeightPx));
          QRect bounding(0, 0, int(pageWidthPx),int(pageHeightPx));
          view.scale(1.0,1.0);
          view.setMinimumSize(int(pageWidthPx),int(pageHeightPx));
          view.setMaximumSize(int(pageWidthPx),int(pageHeightPx));
          view.setGeometry(bounding);
          view.setSceneRect(boundingRect);
          view.setRenderHints(
                QPainter::Antialiasing |
                QPainter::TextAntialiasing |
                QPainter::SmoothPixmapTransform);
          view.centerOn(boundingRect.center());
          clearPage(&view,&scene);

          // paint to the image the scene we view
          if (!exportPdfElements) {
              // initialize painter with image
              painter.begin(&image);
              // clear the pixels of the image
              image.fill(Qt::white);
          }

          // render this page
          dpFlags.printing = true;
          drawPage(&view,&scene,dpFlags);
          scene.setSceneRect(0.0,0.0,adjPageWidthPx,adjPageHeightPx);
          scene.render(&painter);
          clearPage(&view,&scene);

          if (exportPdfElements) {
              // prepare pdfWriter to render next page
              if(_pageCount < printPages.count()) {
                  bool nextPage = true;
                  pdfWriter.setPageLayout(getPageLayout(nextPage));
                  pdfWriter.newPage();
              }
          } else {
              // store the image and required page attributes
              getExportPageSize(pageWidthIn, pageHeightIn, Inches);
              PdfPage pdfPage;
              pdfPage.image       = image;
              pdfPage.pageWidthIn  = pageWidthIn;
              pdfPage.pageHeightIn = pageHeightIn;

              // store pdfWriter next page layout
              if(_pageCount < printPages.count()) {
                  bool nextPage = true;
                  pdfPage.pageLayout = getPageLayout(nextPage);
              }

              // store the rendered page
              QMap<int, PdfPage>::iterator i = pages.find(displayPageNum);
              if (i != pages.end())
                  pages.erase(i);
              pages.insert(displayPageNum,pdfPage);

              // wrap up
              painter.end();
          }
      } // end of step 1. generate page pixmaps

      if (Preferences::modeGUI)
          m_progressDialog->setValue(printPages.count());

      if (exportPdfElements) {
          // wrap up paint to pdfWriter
          painter.end();
      } else {
          // step 2. paint generated page pixmaps to the pdfWriter
          painter.begin(&pdfWriter);

          if (Preferences::modeGUI)
              m_progressDialog->setRange(1,pages.count());

          int page;
          Q_FOREACH (page, pages.keys()) {

              message = tr("Step 2. Exporting pdf document page %1 of %2").arg(page).arg(pages.count());
              emit messageSig(LOG_INFO_STATUS,message);

              if (Preferences::modeGUI) {
                  m_progressDialog->setLabelText(message);
                  m_progressDialog->setValue(page);
                  QApplication::processEvents();
              }

              if (! exporting()) {
                  painter.end();
                  message = tr("Export to pdf terminated before completion. %2 pages of %3 processed%4.")
                                .arg(page).arg(pages.count()).arg(gui->elapsedTime(exportTimer.elapsed()));
                  emit messageSig(LOG_INFO_STATUS,message);
                  if (Preferences::modeGUI) {
                      QApplication::restoreOverrideCursor();
                      m_progressDialog->setBtnToClose();
                      m_progressDialog->setLabelText(message, true/*alert*/);
                  }
                  emit setExportingSig(false);
                  restoreCurrentPage();
                  return;
              }

              // render this page's image to the pdfWriter
              painter.drawImage(QRect(0,0,
                                 int(pdfWriter.logicalDpiX()*pages[page].pageWidthIn),
                                 int(pdfWriter.logicalDpiY()*pages[page].pageHeightIn)),
                                 pages[page].image);

              // prepare to render next page
              if(page < pages.count()) {
                  pdfWriter.setPageLayout(pages[page].pageLayout);
                  pdfWriter.newPage();
              }
          }

          if (Preferences::modeGUI)
              m_progressDialog->setValue(pages.count());

          // wrap up paint to pdfWriter
          pages.clear();
          painter.end();
      } // end of step 2. paint generated page pixmaps to the pdfWriter
  }

  // hide progress bar
  if (Preferences::modeGUI) {
      QApplication::restoreOverrideCursor();
      m_progressDialog->hide();
  }

  // release Visual Editor
  emit setExportingSig(false);

  // set elapsed time
  exportTime = gui->elapsedTime(exportTimer.elapsed());

  // return to whatever page we were viewing before printing
  restoreCurrentPage();

  //display completion message
  if (Preferences::modeGUI) {

      message = tr("Your document export completed. %1").arg(exportTime);

      if (messageList.size()) {
          int errorSet = 0;
          int warnSet  = 0;
          QRegExp errorRx(">ERROR<");
          QRegExp fatalRx(">FATAL<");
          QRegExp warnRx(">WARNING<");
          for (const QString &item : messageList)
              if (item.contains(errorRx) || item.contains(fatalRx))
                  errorSet++;
              else if (! warnSet && item.contains(warnRx))
                  warnSet++;
          message.append(tr("<br><br>There %1 %2%3 :<br>%4")
                            .arg(messageList.size() == 1 ? tr("was") : tr("were"))
                            .arg(errorSet ? QString("%1 %2").arg(QString::number(errorSet)).arg(errorSet == 1 ? tr("error") : tr("errors")) : "")
                            .arg(warnSet  ? QString("%1%2 %3").arg(errorSet ? tr(" and ") : "").arg(QString::number(warnSet )).arg(warnSet  == 1 ? tr("warning") : tr("warnings")) : "")
                            .arg(messageList.join(" ")));
      }

      QPixmap _icon = QPixmap(":/icons/lpub96.png");
      box.setWindowIcon(QIcon());
      box.setIconPixmap (_icon);
      box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
      box.setDefaultButton   (QMessageBox::Yes);

      QString title = "<b>" + tr ("Export to pdf completed.") + "</b>";
      QString text = tr ("%1<br><br>Do you want to open this document ?<br><br>%2")
                         .arg(message).arg(QDir::toNativeSeparators(fileName));

      box.setText (title);
      box.setInformativeText (text);
  }

  if (Preferences::modeGUI && box.exec() == QMessageBox::Yes) {
      const QString CommandPath = fileName;
#ifdef Q_OS_WIN
      QDesktopServices::openUrl((QUrl("file:///"+CommandPath, QUrl::TolerantMode)));
#else
      QProcess *Process = new QProcess(this);
      Process->setWorkingDirectory(QFileInfo(CommandPath).absolutePath() + QDir::separator());
      QStringList arguments = QStringList() << CommandPath;
      Process->start(UNIX_SHELL, arguments);
      Process->waitForFinished();
      if (Process->exitStatus() != QProcess::NormalExit || Process->exitCode() != 0) {
          QErrorMessage *m = new QErrorMessage(this);
          m->showMessage(tr("Failed to launch PDF document.\n%1\n%2")
                             .arg(CommandPath).arg(QString(Process->readAllStandardError())));
      }
#endif
  } else {
      LogType logType = Preferences::modeGUI ? LOG_INFO_STATUS : LOG_INFO;
      emit messageSig(logType, tr("Export to pdf completed! %1").arg(exportTime));
  }
}

void Gui::exportAs(const QString &_suffix)
{
  QString suffix = QString(_suffix).replace(".","").toUpper();
  QString directoryName = m_saveDirectoryName.isEmpty() ? QDir::currentPath() : m_saveDirectoryName;

  QString type;
  if (suffix == "PNG" ||
      suffix == "JPG" ||
      suffix == "BMP") {
      type    = tr("page image");
  }
  else
  if (suffix == "STL" ||
      suffix == "3DS" ||
      suffix == "OBJ") {
      type    = tr("object");
  }
  else {
      // DAE
      // POV
      type    = tr("file");
  }

  // store current display page number
  prevDisplayPageNum = displayPageNum;

  // init drawPage flags
  DrawPageFlags dpFlags;

  // return to whatever page we were viewing before printing
  auto restoreCurrentPage = [&]()
  {
      displayPageNum = prevDisplayPageNum;
      if (abortProcess())
          restorePreviousPage();
      else
          displayPage();
  };

  // Switch to Native Renderer for fast processing
  if (exportingObjects()) {

      setNativeRenderer();

      if (m_saveDirectoryName.isEmpty()) {
          char *exportsDir = nullptr;
          exportsDir = TCUserDefaults::stringForKey(EXPORTS_DIR_KEY, nullptr,false);
          if (exportsDir) {
              stripTrailingPathSeparators(exportsDir);
              m_saveDirectoryName = exportsDir;
          } else {
              m_saveDirectoryName = directoryName;
          }
          delete[] exportsDir;
      }
  }

  // determine location to output images
  QFileInfo fileInfo(curFile);

  // add pixel ratio info to file name
  QString dpiInfo = QString("_%1_DPI").arg(int(resolution()));
  if (exportPixelRatio > 1.0 || exportPixelRatio < 1.0){
      dpiInfo += QString("_%1x").arg(exportPixelRatio);
  }

  LGraphicsScene scene;
  LGraphicsView view(&scene);

  float pageWidthPx = 0.0f, pageHeightPx = 0.0f;
  int adjPageWidthPx = 0, adjPageHeightPx = 0;

  int _displayPageNum = 0;
  int _maxPages       = 0;
  QStringList pageRanges;
  QList<int> printPages;

  // initialize page sizes
  displayPageNum = 0;
  dpFlags.printing = true;
  drawPage(&view,&scene,dpFlags);
  clearPage(&view,&scene);
  displayPageNum = prevDisplayPageNum;

  if (processOption != EXPORT_PAGE_RANGE) {

      if(processOption == EXPORT_ALL_PAGES) {
          _displayPageNum = 1 + pa;
          _maxPages = maxPages;
      }
      if (processOption == EXPORT_CURRENT_PAGE) {
          _displayPageNum = displayPageNum;
          _maxPages       = displayPageNum;
      }

  } else {

      pageRanges = pageRangeText.split(",");
      for (QString const &ranges : pageRanges) {
          if (ranges.contains("-")) {
              QStringList range = ranges.split("-");
              int minPage = range[0].toInt();
              int maxPage = range[1].toInt();
              for(int i = minPage; i <= maxPage; i++) {
                  printPages.append(i);
              }
          } else {
              printPages.append(ranges.toInt());
          }
      }

      std::sort(printPages.begin(),printPages.end(),lessThan);
  }

  // set the file name
  QString baseName = fileInfo.completeBaseName();
  baseName += dpiInfo;

  if (Preferences::modeGUI && m_saveDirectoryName.isEmpty()) {
      directoryName = QFileDialog::getExistingDirectory(
          this,
          tr("Save %1 %2 to folder").arg(suffix).arg(type),
          QDir::currentPath(),
          QFileDialog::ShowDirsOnly);
      if (directoryName == "") {
          // release Visual Editor
          emit setExportingSig(false);
          restoreCurrentPage();
          return;
      }
      m_saveDirectoryName = directoryName;
  } else
  if (!m_saveDirectoryName.isEmpty()) {
      directoryName = m_saveDirectoryName;
  }

  QElapsedTimer exportTimer;
  exportTimer.start();
  displayPageTimer.start();

  // Support transparency for formats that can handle it, but use white for those that can't.
  bool fillPng = suffix.compare("png", Qt::CaseInsensitive) == 0;

  // calculate device pixel ratio
  qreal dpr = exportPixelRatio;

  // initialize progress dialogue
  QString message = tr("instructions to %1 to %2...").arg(type).arg(suffix);

  m_progressDialog->setWindowTitle(tr("Export as %1 %2").arg(suffix).arg(type));
  m_progressDialog->setLabelText(tr("Exporting %1").arg(message));
  if (Preferences::modeGUI) {
      m_progressDialog->setBtnToCancel();
      m_progressDialog->show();
      QApplication::setOverrideCursor(Qt::ArrowCursor);
      QCoreApplication::processEvents();
  }
  emit messageSig(LOG_INFO_STATUS,tr("Starting export %1").arg(message));

  if (processOption != EXPORT_PAGE_RANGE) {

      message = tr("Export %1 %2 to %3").arg(_maxPages).arg(type).arg(suffix);
      emit messageSig(LOG_INFO_STATUS,message);

      if (Preferences::modeGUI) {
          m_progressDialog->setLabelText(message);
          m_progressDialog->setRange(_displayPageNum,_maxPages);
          QCoreApplication::processEvents();
      }

      for (displayPageNum = _displayPageNum; displayPageNum <= _maxPages; displayPageNum++) {

          if (! exporting()) {
              message = tr("Export to pdf terminated before completion. %1 %2 of %3 processed%%4.")
                            .arg(displayPageNum - 1).arg(_maxPages).arg(type).arg(gui->elapsedTime(exportTimer.elapsed()));
              emit messageSig(LOG_INFO_STATUS,message);
              if (Preferences::modeGUI) {
                  QApplication::restoreOverrideCursor();
                  m_progressDialog->setBtnToClose();
                  m_progressDialog->setLabelText(message, true/*alert*/);
              }
              emit setExportingSig(false);
              restoreCurrentPage();
              return;
          }

          message = tr("Exporting %1 %2: %3 of %4...")
                       .arg(suffix).arg(type)
                       .arg(displayPageNum)
                       .arg(_maxPages);
          emit messageSig(LOG_INFO_STATUS,message);

          if (Preferences::modeGUI) {
              m_progressDialog->setLabelText(message);
              m_progressDialog->setValue(displayPageNum);
              QApplication::processEvents();
          }

          if (exportingObjects()) {

              // execute drawPage to prperly process csiParts for content generation
              dpFlags.printing = false;
              drawPage(&view,&scene,dpFlags);
              clearPage(&view,&scene);

          } else {
              // determine size of output image, in pixels
              getExportPageSize(pageWidthPx, pageHeightPx);
              adjPageWidthPx = int(double(pageWidthPx) * dpr);
              adjPageHeightPx = int(double(pageHeightPx) * dpr);

              bool  ls = getPageOrientation() == Landscape;
              message = tr("Exporting %1 %2 %3 of %4, size(in pixels) W %5 x H %6, orientation %7, DPI %8, pixel ratio %9")
                           .arg(suffix)
                           .arg(type)
                           .arg(displayPageNum)
                           .arg(_maxPages)
                           .arg(adjPageWidthPx)
                           .arg(adjPageHeightPx)
                           .arg(ls ? "Landscape" : "Portrait")
                           .arg(int(resolution()))
                           .arg(dpr);
              emit messageSig(LOG_NOTICE,message);

              // paint to the image the scene we view
              QImage image(adjPageWidthPx, adjPageHeightPx, QImage::Format_ARGB32);
              image.setDevicePixelRatio(dpr);

              QPainter painter;
              painter.begin(&image);

              // set up the view
              QRectF boundingRect(0.0, 0.0, int(pageWidthPx),int(pageHeightPx));
              QRect bounding(0, 0, int(pageWidthPx),int(pageHeightPx));
              view.scale(1.0,1.0);
              view.setMinimumSize(int(pageWidthPx),int(pageHeightPx));
              view.setMaximumSize(int(pageWidthPx),int(pageHeightPx));
              view.setGeometry(bounding);
              view.setSceneRect(boundingRect);
              view.setRenderHints(
                    QPainter::Antialiasing |
                    QPainter::TextAntialiasing |
                    QPainter::SmoothPixmapTransform);
              view.centerOn(boundingRect.center());
              clearPage(&view,&scene);

              // clear the pixels of the image, just in case the background is
              // transparent or uses a PNG image with transparency. This will
              // prevent rendered pixels from each page layering on top of each
              // other.
              image.fill(fillPng ? Qt::transparent : Qt::white);

              // render this page
              // scene.render instead of view.render resolves "warm up" issue
              dpFlags.printing = true;
              drawPage(&view,&scene,dpFlags);
              scene.setSceneRect(0.0,0.0,image.width(),image.height());
              scene.render(&painter);
              clearPage(&view,&scene);

              // save the image to the selected directory
              // internationalization of "_page_"?
              QString pn = QString::number(displayPageNum);
              QString const imageFile = QDir::toNativeSeparators(directoryName + QDir::separator() + baseName + "_page_" + pn + "." + suffix.toLower());
              QImageWriter Writer(imageFile);
              if (Writer.format().isEmpty())
                  Writer.setFormat(qPrintable(suffix));
              if (!Writer.write(image)) {
                  message = QObject::tr("Failed to export %1 %2 file:<br>[%3].<br>Reason: %4.")
                                        .arg(suffix)
                                        .arg(type)
                                        .arg(imageFile)
                                        .arg(Writer.errorString());
                  emit messageSig(LOG_WARNING,message);
              }
              painter.end();
          }
      }

      if (Preferences::modeGUI)
          m_progressDialog->setValue(_maxPages);

    } else {

      if (Preferences::modeGUI)
          m_progressDialog->setRange(1,printPages.count());

      int _pageCount = 0;

      Q_FOREACH (int printPage,printPages){

          if (! exporting()) {
              message = tr("Export to pdf terminated before completion. %1 %2 of %3 processed%%4.")
                           .arg(_pageCount).arg(printPages.count()).arg(type).arg(gui->elapsedTime(exportTimer.elapsed()));
              emit messageSig(LOG_INFO_STATUS,message);
              if (Preferences::modeGUI) {
                  QApplication::restoreOverrideCursor();
                  m_progressDialog->setBtnToClose();
                  m_progressDialog->setLabelText(message, true/*alert*/);
              }
              emit setExportingSig(false);
              restoreCurrentPage();
              return;
          }

          displayPageNum = printPage;

          message = tr("Exporting %1 %2 %3 of range %4...")
                       .arg(suffix).arg(type)
                       .arg(_pageCount++)
                       .arg(pageRanges.join(" "));
          emit messageSig(LOG_INFO_STATUS,message);

          if (Preferences::modeGUI) {
              m_progressDialog->setLabelText(message);
              m_progressDialog->setValue(_pageCount++);
              QApplication::processEvents();
          }

          if (exportingObjects()) {

              // execute drawPage to prperly process csiParts for content generation
              dpFlags.printing = false;
              drawPage(&view,&scene,dpFlags);
              clearPage(&view,&scene);

          } else {
              // determine size of output image, in pixels
              getExportPageSize(pageWidthPx, pageHeightPx);
              adjPageWidthPx  = int(double(pageWidthPx) * dpr);
              adjPageHeightPx = int(double(pageHeightPx) * dpr);

              bool  ls = getPageOrientation() == Landscape;
              message = tr("Exporting %1 %2 %3 of %4, size(in pixels) W %5 x H %6, orientation %7, DPI %8, pixel ratio %9")
                            .arg(suffix)
                            .arg(type)
                            .arg(displayPageNum)
                            .arg(_maxPages)
                            .arg(adjPageWidthPx)
                            .arg(adjPageHeightPx)
                            .arg(ls ? "Landscape" : "Portrait")
                            .arg(int(resolution()))
                            .arg(dpr);
              emit messageSig(LOG_NOTICE,message);

              // paint to the image the scene we view
              QImage image(adjPageWidthPx, adjPageHeightPx, QImage::Format_ARGB32);
              image.setDevicePixelRatio(dpr);

              QPainter painter;
              painter.begin(&image);

              QRectF boundingRect(0.0, 0.0, int(pageWidthPx),int(pageHeightPx));
              QRect bounding(0, 0, int(pageWidthPx),int(pageHeightPx));
              view.scale(1.0,1.0);
              view.setMinimumSize(int(pageWidthPx),int(pageHeightPx));
              view.setMaximumSize(int(pageWidthPx),int(pageHeightPx));
              view.setGeometry(bounding);
              view.setSceneRect(boundingRect);
              view.setRenderHints(
                    QPainter::Antialiasing |
                    QPainter::TextAntialiasing |
                    QPainter::SmoothPixmapTransform);
              view.centerOn(boundingRect.center());
              clearPage(&view,&scene);

              // clear the pixels of the image, just in case the background is
              // transparent or uses a PNG image with transparency. This will
              // prevent rendered pixels from each page layering on top of each
              // other.
              image.fill(fillPng ? Qt::transparent : Qt::white);

              // render this page
              // scene.render instead of view.render resolves "warm up" issue
              dpFlags.printing = true;
              drawPage(&view,&scene,dpFlags);
              scene.setSceneRect(0.0,0.0,image.width(),image.height());
              scene.render(&painter);
              clearPage(&view,&scene);

              // save the image to the selected directory
              // internationalization of "_page_"?
              QString pn = QString::number(displayPageNum);
              QString const imageFile = QDir::toNativeSeparators(directoryName + QDir::separator() + baseName + "_page_" + pn + "." + suffix.toLower());
              QImageWriter Writer(imageFile);
              if (Writer.format().isEmpty())
                  Writer.setFormat(qPrintable(suffix));
              if (!Writer.write(image)) {
                  message = QObject::tr("Failed to export %1 %2 file:<br>[%3].<br>Reason: %4.")
                                        .arg(suffix)
                                        .arg(type)
                                        .arg(imageFile)
                                        .arg(Writer.errorString());
                  emit messageSig(LOG_WARNING,message);
              }
              painter.end();
          }
      }
      if (Preferences::modeGUI)
          m_progressDialog->setValue(printPages.count());
    }

    // hide progress bar
    if (Preferences::modeGUI) {
      QApplication::restoreOverrideCursor();
      m_progressDialog->hide();
    }

    // restore preferred renderer
    if (exportingObjects()) {
        restorePreferredRenderer();
    }

    // release Visual Editor
    emit setExportingSig(false);

    // set elapsed time
    exportTime = gui->elapsedTime(exportTimer.elapsed());

    // return to whatever page we were viewing before output
    restoreCurrentPage();

    //display completion message
    QMessageBoxResizable box;
    if (Preferences::modeGUI) {

      message = tr("Your %1 export completed. %2").arg(type).arg(exportTime);

      if (messageList.size()) {
          int errorSet = 0;
          int warnSet  = 0;
          QRegExp errorRx(">ERROR<");
          QRegExp fatalRx(">FATAL<");
          QRegExp warnRx(">WARNING<");
          for (const QString &item : messageList)
            if (item.contains(errorRx) || item.contains(fatalRx))
                errorSet++;
            else if (! warnSet && item.contains(warnRx))
                warnSet++;
          message.append(tr("<br><br>There %1 %2%3 :<br>%4")
                             .arg(messageList.size() == 1 ? tr("was") : tr("were"))
                             .arg(errorSet ? QString("%1 %2").arg(QString::number(errorSet)).arg(errorSet == 1 ? tr("error") : tr("errors")) : "")
                             .arg(warnSet  ? QString("%1%2 %3").arg(errorSet ? tr(" and ") : "").arg(QString::number(warnSet )).arg(warnSet  == 1 ? tr("warning") : tr("warnings")) : "")
                             .arg(messageList.join(" ")));
      }

        QPixmap _icon = QPixmap(":/icons/lpub96.png");
        box.setWindowIcon(QIcon());
        box.setIconPixmap (_icon);
        box.setTextFormat (Qt::RichText);
        box.setStandardButtons (QMessageBox::Yes| QMessageBox::No);
        box.setDefaultButton   (QMessageBox::Yes);
        box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        box.setWindowTitle(tr ("Export %1 %2").arg(suffix).arg(type));

      QString title = "<b>" + tr ("Export %1 %2 completed.").arg(suffix).arg(type) + "</b>";
      QString text = tr ("%1<br><br>Do you want to open the %2 folder ?<br><br>%3")
                         .arg(message).arg(type).arg(directoryName);

      box.setText (title);
      box.setInformativeText (text);
    }

    if (Preferences::modeGUI && box.exec() == QMessageBox::Yes) {
      openFolderSelect(directoryName  + "/" + type);
    } else {
      LogType logType = Preferences::modeGUI ? LOG_INFO_STATUS : LOG_INFO;
      emit messageSig(logType, tr("Export %1 %2 completed! %3")
                                  .arg(suffix)
                                  .arg(type)
                                  .arg(exportTime));
      emit messageSig(LOG_INFO, tr("Exported %1 %2 path: %3")
                                   .arg(suffix)
                                   .arg(type)
                                   .arg(directoryName));
    }

    m_saveDirectoryName.clear();
}

//-----------------PRINT FUNCTIONS------------------------//

void Gui::Print(QPrinter* Printer)
{
  bool preview = printPreview;
  QString mode = exportPdf ? "pdf" : tr("file");

  emit hidePreviewDialogSig();

  /*
   * Options:
   *
   */
  int DocCopies;
  int PageCopies;

  int PageCount = maxPages;
  prevDisplayPageNum = displayPageNum;

  // set drawPage flags
  DrawPageFlags dpFlags;

  // return to whatever page we were viewing before printing
  auto restoreCurrentPage = [&]()
  {
      displayPageNum = prevDisplayPageNum;
      if (abortProcess())
          restorePreviousPage();
      else
          displayPage();
  };

  if (Printer->collateCopies())
    {
      DocCopies = 1;
      PageCopies = Printer->supportsMultipleCopies() ? 1 : Printer->copyCount();
    }
  else
    {
      DocCopies = Printer->supportsMultipleCopies() ? 1 : Printer->copyCount();
      PageCopies = 1;
    }

  int FromPage = Printer->fromPage();
  int ToPage = Printer->toPage();
  bool Ascending = true;

  if (FromPage == 0 && ToPage == 0)
    {
      FromPage = 1 + pa;
      ToPage = PageCount;
    }

  FromPage = qMax(1, FromPage);
  ToPage = qMin(PageCount, ToPage);

  if (ToPage < FromPage) {
    restoreCurrentPage();
    return;
  }

  if (Printer->pageOrder() == QPrinter::LastPageFirst)
    {
      int Tmp = FromPage;
      FromPage = ToPage;
      ToPage = Tmp;
      Ascending = false;
    }

  // send signal to halt Visual Editor
  setExportingSig(true);

  LGraphicsScene scene;
  LGraphicsView view(&scene);

  // initialize page sizes
  displayPageNum = 0;
  dpFlags.printing = true;
  drawPage(&view,&scene,dpFlags);
  clearPage(&view,&scene);

  // check if mixed page size and orientation
  checkMixedPageSizeStatus();

  // export over the entire page
  Printer->setFullPage(true);

  // set displayPageNum so we can send the correct index to retrieve page size data
  displayPageNum = FromPage;

  // set initial page layout using first page as default
  Printer->setPageLayout(getPageLayout());

  // paint to the printer
  QPainter Painter(Printer);

  // initialize progress bar dialog
  m_progressDialog->setWindowTitle(preview ? tr("Preview %1").arg(mode) : exportPdf ? tr("Export %1").arg(mode) : tr("Print %1").arg(mode));  //Hack
  if (Preferences::modeGUI)
      m_progressDialog->show();
  m_progressDialog->setLabelText(preview ? tr("Generating preview %1...").arg(mode) : exportPdf ? tr("Exporting %1...").arg(mode) : tr("Printing %1...").arg(mode));

  for (int DocCopy = 0; DocCopy < DocCopies; DocCopy++)
    {
      int Page = FromPage;

      m_progressDialog->setRange(Page,ToPage);

      if (processOption != EXPORT_PAGE_RANGE){

          if (processOption == EXPORT_CURRENT_PAGE){
              // reset display page from saved page
              displayPageNum = prevDisplayPageNum;
              // reset page layout using display page
              Printer->setPageLayout(getPageLayout());
              // set range to display page
              Page   = displayPageNum;
              ToPage = displayPageNum;
            }

          for (displayPageNum = Page; displayPageNum <= ToPage; displayPageNum++)
            {
              if (Printer->printerState() == QPrinter::Aborted || Printer->printerState() == QPrinter::Error || ! exporting())
                {
                  if (Preferences::modeGUI)
                      m_progressDialog->hide();
                  emit messageSig(LOG_STATUS,tr("Export %1 terminated before completion.")
                                                .arg(preview ? tr("Preview %1").arg(mode) : exportPdf ? tr("Export %1").arg(mode) : tr("Print %1").arg(mode)));
                  if (preview){
                    m_previewDialog = false;
                    exportPdf = printPreview = true;
                  }
                  restoreCurrentPage();
                  return;
                }

              m_progressDialog->setLabelText(tr("%3 page %1 of %2...").arg(Page).arg(ToPage)
                                               .arg(preview ? tr("Preview %1").arg(mode) : exportPdf ? tr("Export %1").arg(mode) : tr("Print %1").arg(mode)));
              m_progressDialog->setValue(Page);
              QApplication::processEvents();

              // determine size of output image, in pixels. dimension are inches * pixels per inch
              float pageWidthPx = 0.0f, pageHeightPx = 0.0f;
              getExportPageSize(pageWidthPx, pageHeightPx);

              bool  ls = getPageOrientation() == Landscape;
              logNotice() << tr("%6 page %3 of %4, size(in pixels) W %1 x H %2, orientation %5")
                                .arg(double(pageWidthPx))
                                .arg(double(pageHeightPx))
                                .arg(displayPageNum)
                                .arg(ToPage)
                                .arg(ls ? tr("Landscape") : tr("Portrait"))
                                .arg(preview ? tr("Preview %1").arg(mode) : exportPdf ? tr("Export %1").arg(mode) : tr("Print %1").arg(mode));

              // set up the view
              QRectF boundingRect(0.0, 0.0, double(pageWidthPx), double(pageHeightPx));
              QRect bounding(0, 0, int(pageWidthPx), int(pageHeightPx));
              view.scale(1.0,1.0);
              view.setMinimumSize(int(pageWidthPx),int(pageHeightPx));
              view.setMaximumSize(int(pageWidthPx),int(pageHeightPx));
              view.setGeometry(bounding);
              view.setSceneRect(boundingRect);
              view.setRenderHints(
                    QPainter::Antialiasing |
                    QPainter::TextAntialiasing |
                    QPainter::SmoothPixmapTransform);
              view.centerOn(boundingRect.center());
              clearPage(&view,&scene);

              for (int PageCopy = 0; PageCopy < PageCopies; PageCopy++)
                {
                  if (Printer->printerState() == QPrinter::Aborted || Printer->printerState() == QPrinter::Error || ! exporting())
                    {
                      if (Preferences::modeGUI)
                          m_progressDialog->hide();
                      emit messageSig(LOG_STATUS,tr("%1 terminated before completion.")
                                                    .arg(preview ? tr("Preview %1").arg(mode) : exportPdf ? tr("Export %1").arg(mode) : tr("Print %1").arg(mode)));
                      if (preview){
                        m_previewDialog = false;
                        exportPdf = printPreview = false;
                      }
                      restoreCurrentPage();
                      return;
                    }

                  // render this page
                  dpFlags.printing = true;
                  drawPage(&view,&scene,dpFlags);
                  scene.setSceneRect(0.0,0.0,double(pageWidthPx),double(pageHeightPx));
                  scene.render(&Painter);
                  clearPage(&view,&scene);

                  // TODO: export header and footer

                  if (PageCopy < PageCopies - 1){
                      Printer->newPage();
                    }
                }

              if (Page == ToPage)
                break;

              if (Ascending)
                Page++;
              else
                Page--;

              bool nextPage = true;
              QPageLayout::Orientation pageOrientation = getPageOrientation(nextPage) == Landscape ? QPageLayout::Landscape : QPageLayout::Portrait;
              Printer->setPageOrientation(pageOrientation);
              Printer->newPage();
            }
        } else {
          // page range
          QStringList pageRanges = pageRangeText.split(",");
          QList<int> printPages;
          Q_FOREACH (QString ranges,pageRanges){
              if (ranges.contains("-")){
                  QStringList range = ranges.split("-");
                  int minPage = range[0].toInt();
                  int maxPage = range[1].toInt();
                  for(int i = minPage; i <= maxPage; i++){
                      printPages.append(i);
                    }
                } else {
                  printPages.append(ranges.toInt());
                }
            }

          std::sort(printPages.begin(),printPages.end(),lessThan);

          ToPage = printPages.last();

          m_progressDialog->setRange(1,printPages.count());

          int _pageCount = 0;

          Q_FOREACH (int printPage,printPages){

              if (Printer->printerState() == QPrinter::Aborted || Printer->printerState() == QPrinter::Error || ! exporting())
                {
                  if (Preferences::modeGUI)
                      m_progressDialog->hide();
                  emit messageSig(LOG_STATUS,tr("%1 terminated before completion.")
                                                .arg(exportPdf ? tr("Export %1").arg(mode) : tr("Print %1").arg(mode)));
                  if (preview){
                    m_previewDialog = false;
                    exportPdf = printPreview = false;
                  }
                  restoreCurrentPage();
                  return;
                }

              displayPageNum = Page = printPage;

              m_progressDialog->setLabelText(tr("%1 page %2 of %3 for range %4 ")
                                               .arg(preview ? tr("Preview %1").arg(mode) : exportPdf ? tr("Export %1").arg(mode) : tr("Print %1").arg(mode))
                                               .arg(Page)
                                               .arg(printPages.count())
                                               .arg(pageRanges.join(" ")));
              m_progressDialog->setValue(_pageCount++);
              QApplication::processEvents();

              // determine size of output image, in pixels. dimension are inches * pixels per inch
              float pageWidthPx = 0.0f, pageHeightPx = 0.0f;
              getExportPageSize(pageWidthPx, pageHeightPx);

              bool  ls = getPageOrientation() == Landscape;
              logNotice() << tr("%6 page %3 of %4, size(pixels) W %1 x H %2, orientation %5 for range %7")
                                .arg(double(pageWidthPx))
                                .arg(double(pageHeightPx))
                                .arg(Page)
                                .arg(printPages.count())
                                .arg(ls ? tr("Landscape") : tr("Portrait"))
                                .arg(preview ? tr("Preview %1").arg(mode) : exportPdf ? tr("Export %1").arg(mode) : tr("Print %1").arg(mode))
                                .arg(pageRanges.join(" "));

              // set up the view
              QRectF boundingRect(0.0, 0.0, double(pageWidthPx), double(pageHeightPx));
              QRect bounding(0, 0, int(pageWidthPx), int(pageHeightPx));
              view.scale(1.0,1.0);
              view.setMinimumSize(int(pageWidthPx),int(pageHeightPx));
              view.setMaximumSize(int(pageWidthPx),int(pageHeightPx));
              view.setGeometry(bounding);
              view.setSceneRect(boundingRect);
              view.setRenderHints(
                    QPainter::Antialiasing |
                    QPainter::TextAntialiasing |
                    QPainter::SmoothPixmapTransform);
              view.centerOn(boundingRect.center());
              clearPage(&view,&scene);

              for (int PageCopy = 0; PageCopy < PageCopies; PageCopy++)
                {
                  if (Printer->printerState() == QPrinter::Aborted || Printer->printerState() == QPrinter::Error || ! exporting())
                    {
                      if (Preferences::modeGUI)
                          m_progressDialog->hide();
                      emit messageSig(LOG_STATUS,tr("%1 terminated before completion.")
                                                 .arg(preview ? tr("Preview %1").arg(mode) : exportPdf ? tr("Export %1").arg(mode) : tr("Print %1").arg(mode)));
                      if (preview){
                        m_previewDialog = false;
                        exportPdf = printPreview = false;
                      }
                      restoreCurrentPage();
                      return;
                    }

                  // render this page
                  dpFlags.printing = true;
                  drawPage(&view,&scene,dpFlags);
                  scene.setSceneRect(0.0,0.0,int(pageWidthPx),int(pageHeightPx));
                  scene.render(&Painter);
                  clearPage(&view,&scene);

                  // TODO: export header and footer

                  if (PageCopy < PageCopies - 1){
                      Printer->newPage();
                    }
                }
              bool nextPage = true;
              QPageLayout::Orientation pageOrientation = getPageOrientation(nextPage) == Landscape ? QPageLayout::Landscape : QPageLayout::Portrait;
              Printer->setPageOrientation(pageOrientation);
              Printer->newPage();
            }
        }
      m_progressDialog->setValue(ToPage);

      if (DocCopy < DocCopies - 1) {
          Printer->newPage();
        }
    }

  // release Visual Editor
  setExportingSig(false);

  // return to whatever page we were viewing before output
  displayPageNum = prevDisplayPageNum;
  dpFlags.printing = false;
  drawPage(KpageView,KpageScene,dpFlags);

  // hide progress bar
  if (Preferences::modeGUI)
      m_progressDialog->hide();

  emit messageSig(LOG_STATUS,tr("%1 completed.")
                                .arg(preview ? tr("Preview %1").arg(mode) : exportPdf ? tr("Export to %1").arg(mode) : tr("Print to %1").arg(mode)));

  if (preview){
      m_previewDialog = false;
      exportPdf = printPreview = false;
    }
}

void Gui::showExportedFile(){

  if (! exportedFile.isEmpty() && Preferences::modeGUI ){

      //display completion message
      QPixmap _icon = QPixmap(":/icons/lpub96.png");
      QMessageBoxResizable box;
      box.setWindowIcon(QIcon());
      box.setIconPixmap (_icon);
      box.setTextFormat (Qt::RichText);
      box.setStandardButtons (QMessageBox::Close);
      box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      box.setWindowTitle(tr ("Export Status"));

      box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
      box.setDefaultButton   (QMessageBox::Yes);

      QString title = "<b>" + tr ("Document export completed.") + "</b>";
      QString text = tr ("Your instruction document has finished exporting.\n\n"
                         "Do you want to open this document ?\n\n%1").arg(exportedFile);

      box.setText (title);
      box.setInformativeText (text);

      if ((box.exec() == QMessageBox::Yes)) {
          const QString CommandPath = exportedFile;
#ifdef Q_OS_WIN
          QDesktopServices::openUrl((QUrl("file:///"+CommandPath, QUrl::TolerantMode)));
#else
          QProcess *Process = new QProcess(this);
          Process->setWorkingDirectory(QFileInfo(CommandPath).absolutePath() + QDir::separator());
          QStringList arguments = QStringList() << CommandPath;
          Process->start(UNIX_SHELL, arguments);
          Process->waitForFinished();
          if (Process->exitStatus() != QProcess::NormalExit || Process->exitCode() != 0) {
              QErrorMessage *m = new QErrorMessage(this);
              m->showMessage(tr("Failed to launch exported document.\n%1\n%2")
                                .arg(CommandPath).arg(QString(Process->readAllStandardError())));
          }
#endif
          return;
        } else {
          emit messageSig(LOG_STATUS, tr("Document export completed!"));
          return;

        }
    }
}

void Gui::ShowPrintDialog()
{
    int PageCount = maxPages;

    QPrinter Printer(QPrinter::HighResolution);
    Printer.setFromTo(1 + pa, PageCount + 1);

    QPrintDialog PrintDialog(&Printer, this);

    m_previewDialog = false;

    if (PrintDialog.exec() == QDialog::Accepted) {
        exportPdf = printPreview = false;
        Print(&Printer);
    }
}

void Gui::TogglePdfExportPreview()
{
    TogglePrintPreview(EXPORT_PDF);
}

void Gui::TogglePrintToFilePreview()
{
    TogglePrintPreview(PRINT_FILE);
}

void Gui::TogglePrintPreview(ExportMode m)
{
    m_previewDialog = true;

    if (! exportAsDialog(m)){
        m_previewDialog = false;
        return;
    }

    printPreview = true;
    exportPdf    = m == EXPORT_PDF;

    int PageCount = maxPages;

    QPrinter Printer(QPrinter::ScreenResolution);

    //set page parameters
    Printer.setFromTo(1 + pa, PageCount + 1);

    QPrintPreviewDialog Preview(&Printer, this);

    if (exportPdf) {
        // determine location for output file
        QFileInfo fileInfo(curFile);
        QString baseName = fileInfo.completeBaseName();
        QString fileName = QDir::currentPath() + "/" + baseName;
        fileInfo.setFile(fileName);
        QString suffix = fileInfo.suffix();
        if (suffix == "") {
            fileName += ".pdf";
        } else if (suffix != ".pdf" && suffix != ".PDF") {
            fileName = fileInfo.path() + "/" + fileInfo.completeBaseName() + ".pdf";
        }

        exportedFile = fileName;
        Printer.setOutputFileName(exportedFile);
    }

    connect(&Preview, SIGNAL(paintRequested(QPrinter*)),          SLOT(Print(QPrinter*)));
    connect(this,     SIGNAL(hidePreviewDialogSig()),   &Preview, SLOT(hide()));

    int rc = Preview.exec();

    if (exportPdf) {
        messageSig(LOG_STATUS, tr("PDF export preview result is %1")
                                  .arg(rc == 1 ? exportedFile + tr(" exported") : tr("preview only")));  // 0=preview only, 1=export output
        if (rc == 1) {
            showExportedFile();
        }
    }

    m_previewDialog = false;
}
