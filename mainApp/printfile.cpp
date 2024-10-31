/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2024 Trevor SANDY. All rights reserved.
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

#include <LDVQt/LDVWidget.h>

#include "paths.h"
#include "lpub.h"
#include "progress_dialog.h"
#include "dialogexportpages.h"
#include "messageboxresizable.h"

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

QPageLayout Gui::getPageLayout(bool nextPage) {

  int pageNum = Gui::displayPageNum;
  if (nextPage && (Gui::displayPageNum < Gui::maxPages)) {
      pageNum = Gui::displayPageNum + 1;            //next page
    }


  QMap<int,PageSizeData>::iterator i = Gui::pageSizes.find(pageNum);
  if (i != Gui::pageSizes.end()) {

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
                    .arg(lpub->page.meta.LPub.resolution.type() == DPI ? "DPI":"DPCM")
                    .arg(pageNum).arg(Gui::displayPageNum);
#endif
      return QPageLayout(pageSize, layoutOrientation, QMarginsF(0,0,0,0));
    }

  emit gui->messageSig(LOG_ERROR,tr("Could not load page %1 layout parameters.").arg(pageNum));
  return QPageLayout();
}

void Gui::getExportPageSize(float &pageWidth, float &pageHeight,int d)
{
  QMap<int,PageSizeData>::iterator i = Gui::pageSizes.find(Gui::displayPageNum);   // this page
  if (i != Gui::pageSizes.end()) {

      float pageWidthIn, pageHeightIn;

      // only concerned with inches because resolution() reports DPI
      pageWidthIn  = i.value().sizeW;
      pageHeightIn = i.value().sizeH;

      // flip orientation for exporting landscape
      if (i.value().orientation == Landscape) {
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
                    .arg(lpub->page.meta.LPub.resolution.type() == DPI ? "DPI":"DPCM")
                    .arg(displayPageNum);
#endif
    } else {
      emit gui->messageSig(LOG_ERROR,tr("Could not load page %1 size parameters.").arg(displayPageNum));
    }
}

OrientationEnc Gui::getPageOrientation(bool nextPage)
{
  int pageNum = Gui::displayPageNum;
  if (nextPage && (Gui::displayPageNum < Gui::maxPages)) {
      pageNum = Gui::displayPageNum + 1;            //next page
    }

  QMap<int,PageSizeData>::iterator i = Gui::pageSizes.find(pageNum);   // this page
  if (i != Gui::pageSizes.end()) {

#ifdef PAGE_PRINT_DEBUG
      bool  ls = i.value().orientation == Landscape;
      logDebug() << QString("PAGE %2 ORIENTATION - %1 CurPage: %3")
                    .arg(ls ? "Landscape":"Portrait")
                    .arg(pageNum).arg(Gui::displayPageNum);
#endif
      return i.value().orientation;
    }
  emit gui->messageSig(LOG_ERROR,tr("Could not load page %1 orientation parameter.").arg(pageNum));
  return InvalidOrientation;
}

void Gui::checkMixedPageSizeStatus() {

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

  float pageWidthIn          = Gui::pageSizes[DEF_SIZE].sizeW;
  float pageHeightIn         = Gui::pageSizes[DEF_SIZE].sizeH;
  QString sizeID             = Gui::pageSizes[DEF_SIZE].sizeID;
  OrientationEnc orientation = Gui::pageSizes[DEF_SIZE].orientation;
  bool orientation_warning   = false;
  bool size_warning          = false;
  bool double_warning        = false;

  if (Gui::processOption == EXPORT_PAGE_RANGE) {

      QStringList pageRanges = Gui::pageRangeText.split(",");
      QList<int> printPages;
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

      if (printPages.size() > 1) {

          bool defaultSizesUpdated = false;

          for (int key : printPages) {

              QMap<int,PageSizeData>::iterator i = Gui::pageSizes.find(key);   // this page
              if (i != Gui::pageSizes.end()) {
#ifdef PAGE_PRINT_DEBUG
                  logDebug() << QString("%6 page %3 of %4, size(Inches) W %1 x H %2, ID %8, orientation %5 for range %7")
                                           .arg(Gui::pageSizes[key].sizeW)
                                           .arg(Gui::pageSizes[key].sizeH)
                                           .arg(key)
                                           .arg(printPages.count())
                                           .arg(Gui::pageSizes[key].orientation == Landscape ? "Landscape" : "Portrait")
                                           .arg("Checking")
                                           .arg(pageRanges.join(" "))
                                           .arg(Gui::pageSizes[key].sizeID);
#endif
                  if (! defaultSizesUpdated) {
                      pageWidthIn  = Gui::pageSizes[key].sizeW;
                      pageHeightIn = Gui::pageSizes[key].sizeH;
                      sizeID       = Gui::pageSizes[key].sizeID;
                      orientation  = Gui::pageSizes[key].orientation;
                      defaultSizesUpdated = true;
                      continue;
                    }

                  if (Gui::pageSizes[key].orientation != orientation && orientation_warning != true) {
                      orientation_warning = true;
                      text = text1;
                      title = "<b>" + tr ("Mixed page orientation detected.") + "</b>";
                      box.setIcon (QMessageBox::Information);
                    }

                  if ((Gui::pageSizes[key].sizeID != sizeID      ||
                      (Gui::pageSizes[key].sizeW  < pageWidthIn  ||
                       Gui::pageSizes[key].sizeW  > pageWidthIn) ||
                      (Gui::pageSizes[key].sizeH  < pageHeightIn ||
                       Gui::pageSizes[key].sizeH  > pageHeightIn)) && size_warning != true) {
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

    } else if (Gui::processOption == EXPORT_ALL_PAGES) {
#ifdef PAGE_PRINT_DEBUG
      logDebug() << tr("Default  page         size(Inches) W %1 x H %2, ID %3, orientation %4")
                       .arg(Gui::pageSizes[DEF_SIZE].sizeW)
                       .arg(Gui::pageSizes[DEF_SIZE].sizeH)
                       .arg(Gui::pageSizes[DEF_SIZE].sizeID)
                       .arg(Gui::pageSizes[DEF_SIZE].orientation == Landscape ? "Landscape" : "Portrait");
#endif
      for (int key : Gui::pageSizes.keys()) {

          if (Gui::pageSizes[key].orientation != orientation && orientation_warning != true) {
              orientation_warning = true;
              text = text1;
              title = "<b>" + tr ("Mixed page orientation detected.") + "</b>";
              box.setIcon (QMessageBox::Information);
            }

          if ((Gui::pageSizes[key].sizeID != sizeID      ||
               Gui::pageSizes[key].sizeW  != pageWidthIn ||
               Gui::pageSizes[key].sizeH  != pageHeightIn) && size_warning != true) {
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
        emit gui->messageSig(LOG_STATUS,QString("%1").arg(text));
    }
}

bool Gui::validatePageRange()
{

  QMessageBox box;
  box.setTextFormat (Qt::RichText);
  box.setIcon (QMessageBox::Information);
  box.setStandardButtons (QMessageBox::Close);
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  box.setWindowTitle(tr ("Page Range"));

  QString title;
  QString text;

  if(Gui::pageRangeText.isEmpty()) {

      title = "<b>" + tr ("Empty page range.") + "</b>";
      text = tr ("You must enter a page range");

      box.setText (title);
      box.setInformativeText (text);

      if (Preferences::modeGUI)
        box.exec();
      else
        emit gui->messageSig(LOG_STATUS,tr("Empty page range. %1").arg(text));

      return false;
    }

  bool validEntry = true;
  QString message;
  QStringList pageRanges = Gui::pageRangeText.split(",");
  for (QString const &ranges: pageRanges) {
      if (ranges.contains("-")) {
          bool ok[2];
          QStringList range = ranges.split("-");

          int startPage = QString(range[0].trimmed()).toInt(&ok[0]);
          int endPage = QString(range[1].trimmed()).toInt(&ok[1]);
          if (!ok[0] || !ok[1]) {
              message = QString("%1-%2").arg(startPage).arg(endPage);
              validEntry = false;
              break;
          }
          if ((Gui::pageDirection == PAGE_NEXT) && (startPage > endPage)) {
              message = QString("%1-%2").arg(startPage).arg(endPage);
              validEntry = false;
              break;
          } else
            if ((Gui::pageDirection == PAGE_PREVIOUS) && (endPage > startPage)) {
                message = QString("%1-%2").arg(startPage).arg(endPage);
                validEntry = false;
                break;
            }
        } else {
          bool ok;
          int pageNum = ranges.toInt(&ok);
          if (!ok) {
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
        emit gui->messageSig(LOG_STATUS,tr("Invalid page number(s). %1").arg(text));

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
        gui->exportAsDialog(ExportMode(mode));
    else if (mode == EXPORT_BRICKLINK)
        gui->exportAsBricklinkXML();
    else if (mode == EXPORT_CSV)
        gui->exportAsCsv();
    else if (mode == EXPORT_HTML_PARTS)
        gui->exportAsHtml();
    else if (mode == EXPORT_HTML_STEPS)
        gui->exportAsHtmlSteps();
    else
        gui->exportAsDialog(EXPORT_PDF);
    Preferences::pageDisplayPause = savePause;

    *result = 0;
}

void Gui::consoleCommandCurrentThread(int mode, int *value)
{
    QThread *thread = QThread::currentThread();
    QThread *mainthread = gui->thread();
    if(thread == mainthread)
        gui->consoleCommand(mode, value);
    else
        emit gui->consoleCommandFromOtherThreadSig(mode, value);
}

void Gui::exportAsPdfDialog() {
  gui->exportAsDialog(EXPORT_PDF);
}

void Gui::exportAsPngDialog() {
  gui->exportAsDialog(EXPORT_PNG);
}

void Gui::exportAsJpgDialog() {
  gui->exportAsDialog(EXPORT_JPG);
}

void Gui::exportAsBmpDialog() {
  gui->exportAsDialog(EXPORT_BMP);
}

void Gui::exportAsStlDialog() {
  gui->exportAsDialog(EXPORT_STL);
}

void Gui::exportAs3dsDialog() {
  gui->exportAsDialog(EXPORT_3DS_MAX);
}

void Gui::exportAsPovDialog() {
  gui->exportAsDialog(EXPORT_POVRAY);
}

void Gui::exportAsColladaDialog() {
  gui->exportAsDialog(EXPORT_COLLADA);
}

void Gui::exportAsObjDialog() {
  gui->exportAsDialog(EXPORT_WAVEFRONT);
}

bool Gui::exportAsDialog(ExportMode m)
{
  Gui::messageList.clear();
  Gui::m_exportMode = m;

 if (Gui::m_exportMode != PAGE_PROCESS)
     Gui::pageDirection = PAGE_NEXT;

  if (Preferences::modeGUI) {
      DialogExportPages *dialog = new DialogExportPages();
      if (dialog->exec() == QDialog::Accepted) {
          if(dialog->allPages()) {
              if (dialog->allPagesRange()) {
                Gui::processOption = EXPORT_PAGE_RANGE;
                Gui::pageRangeText = dialog->allPagesRangeText();
              } else {
                Gui::processOption = EXPORT_ALL_PAGES;
              }
            }
          else
          if(dialog->currentPage()) {
              Gui::processOption = EXPORT_CURRENT_PAGE;
            }
          else
          if(dialog->pageRange()) {
              Gui::processOption = EXPORT_PAGE_RANGE;
              Gui::pageRangeText = dialog->pageRangeText();
              if (! gui->validatePageRange()) {
                  return false;
                }
            }
        } else {
          return false;
        }

      exportPixelRatio = dialog->exportPixelRatio();

      Gui::resetCache = dialog->resetCache();

      QSettings Settings;
      if (Preferences::ignoreMixedPageSizesMsg != dialog->ignoreMixedPageSizesMsg())
      {
        Preferences::ignoreMixedPageSizesMsg = dialog->ignoreMixedPageSizesMsg();
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"IgnoreMixedPageSizesMsg "),Preferences::ignoreMixedPageSizesMsg);
      }

      if (Gui::m_exportMode == EXPORT_PDF && Preferences::pdfPageImage != dialog->pdfPageImage())
      {
        Preferences::pdfPageImage = dialog->pdfPageImage();
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PdfPageImage"),Preferences::pdfPageImage);
      }
    }

  if(Gui::resetCache)
      gui->resetModelCache(QFileInfo(Gui::curFile).absoluteFilePath());

  if (! m_previewDialog) {
      switch (m)
      {
      case EXPORT_PDF:
        // send signal to halt Visual Editor
        emit gui->setExportingSig(true);
        gui->exportAsPdf();
      break;
      case EXPORT_PNG:
        emit gui->setExportingSig(true);
        gui->exportAs(".png");
      break;
      case EXPORT_JPG:
        emit gui->setExportingSig(true);
        gui->exportAs(".jpg");
      break;
      case EXPORT_BMP:
        emit gui->setExportingSig(true);
        gui->exportAs(".bmp");
      break;
      case EXPORT_STL:
        emit gui->setExportingObjectsSig(true);
        gui->exportAs(".stl");
      break;
      case EXPORT_3DS_MAX:
        emit gui->setExportingObjectsSig(true);
        gui->exportAs(".3ds");
      break;
      case EXPORT_COLLADA:
        emit gui->setExportingObjectsSig(true);
        gui->exportAs(".dae");
      break;
      case EXPORT_WAVEFRONT:
        emit gui->setExportingObjectsSig(true);
        gui->exportAs(".obj");
      break;
      case EXPORT_POVRAY:
        gui->setExportingObjectsSig(true);
        gui->exportAs(".pov");
      break;
      default:
      break;
      }
    }

  return true;
}

void Gui::exportAsHtmlSteps()
{
    Gui::messageList.clear();
    lpub->Options                     = new NativeOptions();
    lpub->Options->ExportMode         = EXPORT_HTML_STEPS;
    lpub->Options->ImageType          = Options::CSI;
    lpub->Options->InputFileName      = Gui::curFile;
    if (!Gui::m_saveDirectoryName.isEmpty())
        lpub->Options->ExportFileName = QDir::toNativeSeparators(
                                        Gui::m_saveDirectoryName + "/" +
                                        Paths::htmlStepsDir);
    else
        lpub->Options->ExportFileName = QDir::toNativeSeparators(
                                        QFileInfo(Gui::curFile).absolutePath() + "/" +
                                        Paths::htmlStepsDir);

    bool saveFadeStepsFlag = Preferences::enableFadeSteps;
    if (saveFadeStepsFlag) {
        Preferences::enableFadeSteps = false;
        emit gui->messageSig(LOG_INFO,tr("%1 Fade Steps set to OFF during HTML Steps export.").arg(VER_PRODUCTNAME_STR));
    }
    if (! renderer->NativeExport(lpub->Options)) {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("HTML Steps export failed."));
    }
    if (saveFadeStepsFlag) {
        Preferences::enableFadeSteps = saveFadeStepsFlag;
        emit gui->messageSig(LOG_INFO,tr("%1 Fade Steps restored to ON following HTML Steps export.").arg(VER_PRODUCTNAME_STR));
    }

    lpub->Options = nullptr;
}
void Gui::exportAsHtml()
{
    Gui::messageList.clear();
    lpub->Options              = new NativeOptions();
    lpub->Options->ExportMode  = EXPORT_HTML_PARTS;
    // Visual Editor only
    lpub->Options->ImageType   = Options::CSI;
    if (!Gui::m_saveDirectoryName.isEmpty())
        lpub->Options->ExportFileName = QDir::toNativeSeparators(Gui::m_saveDirectoryName);
    else
        lpub->Options->ExportFileName = QDir::toNativeSeparators(QFileInfo(Gui::curFile).absolutePath());
    // LDV only
    lpub->Options->IniFlag            = NativePartList;

    // Capture the model's last CSI
    emit gui->setExportingObjectsSig(true);

    // Switch to Native Renderer for fast processing
    Gui::setNativeRenderer();

    // store current display page number
    Gui::prevDisplayPageNum = Gui::displayPageNum;

    // start at the last page moving backward until we find a valid CSI
    Meta meta;
    MetaItem mi;
    DrawPageFlags dpFlags;
    Gui::m_partListCSIFile = true;
    bool modelFound    = false;
    int pageNum        = Gui::maxPages;

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

    Gui::displayPageNum = pageNum;

    // setup and export the last CSI on the page
    gui->KexportScene = LGraphicsScene(gui);
    gui->KexportView = LGraphicsView(&gui->KexportScene);

    drawPage(dpFlags);
    Gui::clearPage();

    Gui::restorePreferredRenderer();
    emit gui->setExportingSig(false);

    // reset meta
    meta = Meta();

    // return to whatever page we were viewing before capturing the CSI
    Gui::m_partListCSIFile = false;
    Gui::displayPageNum  = Gui::prevDisplayPageNum;
    Gui::displayPage();

    // create partList key
    bool noCA = Preferences::applyCALocally || meta.rotStep.value().type.toUpper() == QLatin1String("ABS");
    float partListModelScale = meta.LPub.bom.modelScale.value();
    bool suffix = QFileInfo(Gui::getCurFile()).suffix().contains(QRegExp("(dat|ldr|mpd)$",Qt::CaseInsensitive));
    QString partListKey = QString("%1_%2_%3_%4_%5_%6_%7.%8")
                                  .arg(lpub->pageSize(meta.LPub.page, 0))
                                  .arg(double(resolution()))
                                  .arg(resolutionType() == DPI ? "DPI" : "DPCM")
                                  .arg(double(partListModelScale))
                                  .arg(double(meta.LPub.bom.cameraFoV.value()))
                                  .arg(noCA ? double(0.0f) : double(meta.LPub.bom.cameraAngles.value(0)))
                                  .arg(noCA ? double(0.0f) : double(meta.LPub.bom.cameraAngles.value(1)))
                                  .arg(suffix ? QFileInfo(Gui::getCurFile()).suffix() : "ldr");

    // generate HTML parts list
    QString ldrBaseFile = QDir::currentPath()+QDir::separator()+Paths::tmpDir+QDir::separator()+QFileInfo(Gui::curFile).completeBaseName();
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
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("HTML snapshot model file %1 was not found.").arg(snapshot));
    }

    // setup part list arguments
    arguments << QString("-LDrawDir=\"%1\"").arg(QDir::toNativeSeparators(Preferences::ldrawLibPath));
    arguments << QString("-PartlistKey=%1").arg(partListKey);
    arguments << QString("\"%1\"").arg(partListFile);

    lpub->Options->ExportArgs    = arguments;
    lpub->Options->InputFileName = partListFile;

    if (! renderer->NativeExport(lpub->Options)) {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("HTML parts list export failed."));
    }

    lpub->Options = nullptr;
}

void Gui::exportAsCsv()
{
    Gui::messageList.clear();
    lpub->Options             = new NativeOptions();
    lpub->Options->ImageType  = Options::CSI;
    lpub->Options->ExportMode = EXPORT_CSV;
    if (!Gui::saveFileName.isEmpty()) // command line user specified file name
        lpub->Options->OutputFileName = Gui::saveFileName;
    else
        lpub->Options->OutputFileName = QDir::toNativeSeparators(QDir::toNativeSeparators(QString(Gui::curFile).replace(QFileInfo(Gui::curFile).suffix(),"cvs")));
    if (Preferences::modeGUI) {
        // determine location for output file
        QFileInfo fileInfo(Gui::curFile);
        QString baseName = fileInfo.completeBaseName();
        QString fileName = QFileDialog::getSaveFileName(
              gui,
              tr("Export File Name"),
              QDir::currentPath() + QDir::separator() + baseName,
              tr("CSV File (*.csv);;All Files (*.*)"));
        if (!fileName.isEmpty())
            lpub->Options->OutputFileName = fileName;
    }
    lpub->Options->InputFileName = QDir::toNativeSeparators(QDir::currentPath()+QDir::separator()+
                                   Paths::tmpDir+QDir::separator()+QFileInfo(Gui::curFile).completeBaseName()+"_parts.ldr");
    if (! generateBOMPartsFile(lpub->Options->InputFileName))
        return;
    if (! renderer->NativeExport(lpub->Options)) {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("CSV parts list export failed."));
    }

    lpub->Options = nullptr;
}

void Gui::exportAsBricklinkXML()
{
    Gui::messageList.clear();
    lpub->Options             = new NativeOptions();
    lpub->Options->ImageType  = Options::CSI;
    lpub->Options->ExportMode = EXPORT_BRICKLINK;
    if (!Gui::saveFileName.isEmpty()) // command line user specified file name
        lpub->Options->OutputFileName = Gui::saveFileName;
    else
        lpub->Options->OutputFileName = QDir::toNativeSeparators(QDir::toNativeSeparators(QString(Gui::curFile).replace(QFileInfo(Gui::curFile).suffix(),"xml")));
    if (Preferences::modeGUI) {
        // determine location for output file
        QFileInfo fileInfo(Gui::curFile);
        QString baseName = fileInfo.completeBaseName();
        QString fileName = QFileDialog::getSaveFileName(
              gui,
              tr("Export File Name"),
              QDir::currentPath() + QDir::separator() + baseName,
              tr("XML File (*.xml)"));
        if (!fileName.isEmpty())
            lpub->Options->OutputFileName = fileName;
    }
    lpub->Options->InputFileName = QDir::toNativeSeparators(QDir::currentPath()+QDir::separator()+
                                   Paths::tmpDir+QDir::separator()+QFileInfo(Gui::curFile).completeBaseName()+"_parts.ldr");
    if (! generateBOMPartsFile(lpub->Options->InputFileName))
        return;
    if (! renderer->NativeExport(lpub->Options)) {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Bricklink XML parts list export failed."));
    }

    lpub->Options = nullptr;
}

void Gui::exportAsPdf()
{
  // init drawPage flags
  DrawPageFlags dpFlags;

  // store current display page number
  Gui::prevDisplayPageNum = Gui::displayPageNum;

  // return to whatever page we were viewing before printing
  auto restoreCurrentPage = [&](bool restoreKpage = true)
  {
      Gui::displayPageNum = Gui::prevDisplayPageNum;
      if (Gui::abortProcess())
          gui->restorePreviousPage();
      else if (restoreKpage)
          Gui::displayPage();
  };

  // add pixel ratio info to file name
  QString dpiInfo = QString("_%1_DPI").arg(int(resolution()));
  if (exportPixelRatio > 1.0 || exportPixelRatio < 1.0) {
      dpiInfo += QString("_%1x").arg(exportPixelRatio);
  }

  // determine location for output file
  QFileInfo fileInfo(Gui::curFile);
  QString baseName = fileInfo.completeBaseName();
  baseName += dpiInfo;
  QString fileName = QDir::currentPath() + QDir::separator() + baseName;

  if (Preferences::modeGUI) {
      fileName = QFileDialog::getSaveFileName(
            gui,
            tr("Export File Name"),
            QDir::currentPath() + QDir::separator() + baseName,
            tr("PDF File (*.pdf)"));

      if (fileName.isEmpty()) {
          // release Visual Editor
          emit gui->setExportingSig(false);
          restoreCurrentPage(false);
          return;
      }
  } else if (!Gui::saveFileName.isEmpty()) { // command line user specified file name
      fileName = Gui::saveFileName;
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
          emit gui->messageSig(LOG_INFO_STATUS, tr("Cannot open file. %1").arg(text));
          // release Visual Editor
          emit gui->setExportingSig(false);
          restoreCurrentPage(false);
          return;
      }
  }

  QElapsedTimer exportTimer;
  exportTimer.start();
  gui->displayPageTimer.start();

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
  gui->KexportScene = LGraphicsScene(gui);
  gui->KexportView = LGraphicsView(&gui->KexportScene);

  // initialize page sizes
  Gui::displayPageNum = 0;
  dpFlags.printing = true;
  drawPage(dpFlags);
  Gui::clearPage();
  Gui::displayPageNum = Gui::prevDisplayPageNum;

  int _displayPageNum = 0;
  int _maxPages       = 0;

  QString message = tr("instructions to pdf...");

  // initialize progress bar dialog
  gui->m_progressDialog->setWindowTitle("Export pdf");
  gui->m_progressDialog->setLabelText(tr("Exporting %1").arg(message));
  if (Preferences::modeGUI) {
      gui->m_progressDialog->setBtnToCancel();
      gui->m_progressDialog->show();
      QApplication::setOverrideCursor(Qt::ArrowCursor);
      QCoreApplication::processEvents();
  }
  emit gui->messageSig(LOG_INFO_STATUS,tr("Starting export %1").arg(message));

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

  if (Gui::processOption != EXPORT_PAGE_RANGE) {

      if(Gui::processOption == EXPORT_ALL_PAGES) {
          _displayPageNum = 1 + Gui::pa;
          _maxPages = Gui::maxPages;
        }

      if (Gui::processOption == EXPORT_CURRENT_PAGE) {
          _displayPageNum = Gui::displayPageNum;
          _maxPages       = Gui::displayPageNum;
        }

      message = tr("Export %1 pages to pdf").arg(_maxPages);
      emit gui->messageSig(LOG_INFO_STATUS,message);

      if (Preferences::modeGUI) {
          gui->m_progressDialog->setLabelText(message);
          gui->m_progressDialog->setRange(_displayPageNum,_maxPages);
          QCoreApplication::processEvents();
      }

      // set displayPageNum so we can send the correct index to retrieve page size data
      Gui::displayPageNum = _displayPageNum;

      // set initial pdfWriter page layout
      pdfWriter.setPageLayout(getPageLayout());

      QPainter painter;
      if (exportPdfElements) {
         // initialize painter with pdfWriter
         painter.begin(&pdfWriter);
      }

      // step 1. generate page pixmaps
      for (Gui::displayPageNum = _displayPageNum; Gui::displayPageNum <= _maxPages; Gui::displayPageNum++) {

          if (! Gui::exporting()) {
              if (exportPdfElements)
                  painter.end();
              message = tr("Export to pdf terminated before completion. %1 pages of %2 processed%3.")
                           .arg(Gui::displayPageNum - 1).arg(_maxPages).arg(gui->elapsedTime(exportTimer.elapsed()));
              emit gui->messageSig(LOG_INFO_STATUS,message);
              if (Preferences::modeGUI) {
                  QApplication::restoreOverrideCursor();
                  gui->m_progressDialog->setBtnToClose();
                  gui->m_progressDialog->setLabelText(message, true/*alert*/);
              }
              emit gui->setExportingSig(false);
              restoreCurrentPage();
              return;
          }

          message = tr("%1 %2 of %3...")
                        .arg(messageIntro)
                        .arg(Gui::displayPageNum)
                        .arg(_maxPages);
          emit gui->messageSig(LOG_INFO_STATUS,message);

          if (Preferences::modeGUI) {
              gui->m_progressDialog->setLabelText(message);
              gui->m_progressDialog->setValue(Gui::displayPageNum);
              QApplication::processEvents();
          }

          // get size of output image, in pixels
          gui->getExportPageSize(pageWidthPx, pageHeightPx);
          adjPageWidthPx  = int(double(pageWidthPx)  * dpr);
          adjPageHeightPx = int(double(pageHeightPx) * dpr);

          bool  ls = getPageOrientation() == Landscape;

          message = tr("                  %8 %3 of %4, size(pixels) W %1 x H %2, orientation %5, DPI %6, pixel ratio %7...")
                       .arg(adjPageWidthPx)
                       .arg(adjPageHeightPx)
                       .arg(Gui::displayPageNum)
                       .arg(_maxPages)
                       .arg(ls ? tr("Landscape") : tr("Portrait"))
                       .arg(int(resolution()))
                       .arg(dpr)
                       .arg(messageIntro);
          emit gui->messageSig(LOG_NOTICE,message);

          // initiialize the image
          QImage image(adjPageWidthPx, adjPageHeightPx, QImage::Format_ARGB32);
          image.setDevicePixelRatio(dpr);

          // set up the view - use unscaled page size
          QRectF boundingRect(0.0, 0.0, int(pageWidthPx),int(pageHeightPx));
          QRect bounding(0, 0, int(pageWidthPx),int(pageHeightPx));
          gui->KexportView.scale(1.0,1.0);
          gui->KexportView.setMinimumSize(int(pageWidthPx),int(pageHeightPx));
          gui->KexportView.setMaximumSize(int(pageWidthPx),int(pageHeightPx));
          gui->KexportView.setGeometry(bounding);
          gui->KexportView.setSceneRect(boundingRect);
          gui->KexportView.setRenderHints(
                QPainter::Antialiasing |
                QPainter::TextAntialiasing |
                QPainter::SmoothPixmapTransform);
          gui->KexportView.centerOn(boundingRect.center());
          Gui::clearPage();

          // paint to the image the scene we view
          if (!exportPdfElements) {
              // initialize painter with image
              painter.begin(&image);
              // clear the pixels of the image
              image.fill(Qt::white);
          }

          // render this page
          dpFlags.printing = true;
          drawPage(dpFlags);
          gui->KexportScene.setSceneRect(0.0,0.0,adjPageWidthPx,adjPageHeightPx);
          gui->KexportScene.render(&painter);
          Gui::clearPage();

          if (exportPdfElements) {
              // prepare pdfWriter to render next page
              if(Gui::displayPageNum < _maxPages) {
                  bool nextPage = true;
                  pdfWriter.setPageLayout(getPageLayout(nextPage));
                  pdfWriter.newPage();
              }
          } else {
              // store the image and required page attributes
              gui->getExportPageSize(pageWidthIn, pageHeightIn, Inches);
              PdfPage pdfPage;
              pdfPage.image       = image;
              pdfPage.pageWidthIn  = pageWidthIn;
              pdfPage.pageHeightIn = pageHeightIn;

              // store pdfWriter next page layout
              if(Gui::displayPageNum < _maxPages) {
                  bool nextPage = true;
                  pdfPage.pageLayout = getPageLayout(nextPage);
              }

              // store the rendered page
              QMap<int, PdfPage>::iterator i = pages.find(Gui::displayPageNum);
              if (i != pages.end())
                  pages.erase(i);
              pages.insert(Gui::displayPageNum,pdfPage);

              // wrap up paint to image
              painter.end();
          }
      } // end of step 1. generate page pixmaps

      if (Preferences::modeGUI)
          gui->m_progressDialog->setValue(_maxPages);

      if (exportPdfElements) {
          // wrap up paint to pdfWriter
          painter.end();
      } else {
          // step 2. paint generated page pixmaps to the pdfWriter
          painter.begin(&pdfWriter);

          if (Preferences::modeGUI)
              gui->m_progressDialog->setRange(1,pages.count());

          for (int page : pages.keys()) {

              message = tr("Step 2. Exporting pdf document page %1 of %2").arg(page).arg(pages.count());
              emit gui->messageSig(LOG_INFO_STATUS,message);

              if (Preferences::modeGUI) {
                  gui->m_progressDialog->setLabelText(message);
                  gui->m_progressDialog->setValue(page);
                  QApplication::processEvents();
              }

              if (! Gui::exporting()) {
                  painter.end();
                  message = tr("Export to pdf terminated before completion. %2 pages of %3 processed%4.")
                                .arg(page).arg(pages.count()).arg(gui->elapsedTime(exportTimer.elapsed()));
                  emit gui->messageSig(LOG_INFO_STATUS,message);
                  if (Preferences::modeGUI) {
                      QApplication::restoreOverrideCursor();
                      gui->m_progressDialog->setBtnToClose();
                      gui->m_progressDialog->setLabelText(message, true/*alert*/);
                  }
                  emit gui->setExportingSig(false);
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
              gui->m_progressDialog->setValue(pages.count());

          // wrap up paint to pdfWriter
          pages.clear();
          painter.end();
      }

  } else {

      QStringList pageRanges = Gui::pageRangeText.split(",");
      QList<int> printPages;
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

      message = tr("Export %1 pages to pdf").arg(printPages.count());
      emit gui->messageSig(LOG_INFO_STATUS,message);

      if (Preferences::modeGUI) {
          gui->m_progressDialog->setLabelText(message);
          gui->m_progressDialog->setRange(1,printPages.count());
          QCoreApplication::processEvents();
      }

      int _pageCount = 0;

      // set displayPageNum so we can send the correct index to retrieve page size data
      Gui::displayPageNum = printPages.first();

      // set initial pdfWriter page layout
      pdfWriter.setPageLayout(getPageLayout());

      QPainter painter;
      if (exportPdfElements) {
         // initialize painter with pdfWriter
         painter.begin(&pdfWriter);
      }

      // step 1. generate page pixmaps
      for (int printPage : printPages) {

          _pageCount++;

          if (! Gui::exporting()) {
              if (exportPdfElements)
                  painter.end();
              message = tr("Export to pdf terminated before completion. %2 pages of %3 processed%4.")
                            .arg(_pageCount).arg(printPages.count()).arg(gui->elapsedTime(exportTimer.elapsed()));
              emit gui->messageSig(LOG_INFO_STATUS,message);
              if (Preferences::modeGUI) {
                  QApplication::restoreOverrideCursor();
                  gui->m_progressDialog->setBtnToClose();
                  gui->m_progressDialog->setLabelText(message, true/*alert*/);
              }
              emit gui->setExportingSig(false);
              restoreCurrentPage();
              return;
          }

          Gui::displayPageNum = printPage;

          message = tr("%1 %2 (%3 of %4) from the range of %5...")
                       .arg(messageIntro)
                       .arg(Gui::displayPageNum)
                       .arg(_pageCount)
                       .arg(printPages.count())
                       .arg(pageRanges.join(" "));
          emit gui->messageSig(LOG_INFO_STATUS,message);

          if (Preferences::modeGUI) {
              gui->m_progressDialog->setLabelText(message);
              gui->m_progressDialog->setValue(_pageCount);
              QApplication::processEvents();
          }

          // get size of output image, in pixels
          gui->getExportPageSize(pageWidthPx, pageHeightPx);
          adjPageWidthPx  = int(double(pageWidthPx)  * dpr);
          adjPageHeightPx = int(double(pageHeightPx) * dpr);

          bool  ls = getPageOrientation() == Landscape;
          message = tr("%1 %2 (%3 of %4) from the range of %5, size(in pixels) W %6 x H %7, orientation %8, DPI %9")
                       .arg(messageIntro)                    //1
                       .arg(Gui::displayPageNum)             //2
                       .arg(_pageCount)                      //3
                       .arg(printPages.count())              //4
                       .arg(pageRanges.join(" "))            //5
                       .arg(adjPageWidthPx)                  //6
                       .arg(adjPageHeightPx)                 //7
                       .arg(ls ? "Landscape" : "Portrait")   //8
                       .arg(int(resolution()));              //9
          emit gui->messageSig(LOG_NOTICE,message);

          // initiialize the image
          QImage image(adjPageWidthPx, adjPageHeightPx, QImage::Format_ARGB32);
          image.setDevicePixelRatio(dpr);

          // set up the view - use unscaled page size
          QRectF boundingRect(0.0, 0.0, int(pageWidthPx),int(pageHeightPx));
          QRect bounding(0, 0, int(pageWidthPx),int(pageHeightPx));
          gui->KexportView.scale(1.0,1.0);
          gui->KexportView.setMinimumSize(int(pageWidthPx),int(pageHeightPx));
          gui->KexportView.setMaximumSize(int(pageWidthPx),int(pageHeightPx));
          gui->KexportView.setGeometry(bounding);
          gui->KexportView.setSceneRect(boundingRect);
          gui->KexportView.setRenderHints(
                QPainter::Antialiasing |
                QPainter::TextAntialiasing |
                QPainter::SmoothPixmapTransform);
          gui->KexportView.centerOn(boundingRect.center());
          Gui::clearPage();

          // paint to the image the scene we view
          if (!exportPdfElements) {
              // initialize painter with image
              painter.begin(&image);
              // clear the pixels of the image
              image.fill(Qt::white);
          }

          // render this page
          dpFlags.printing = true;
          drawPage(dpFlags);
          gui->KexportScene.setSceneRect(0.0,0.0,adjPageWidthPx,adjPageHeightPx);
          gui->KexportScene.render(&painter);
          Gui::clearPage();

          if (exportPdfElements) {
              // prepare pdfWriter to render next page
              if(_pageCount < printPages.count()) {
                  bool nextPage = true;
                  pdfWriter.setPageLayout(getPageLayout(nextPage));
                  pdfWriter.newPage();
              }
          } else {
              // store the image and required page attributes
              gui->getExportPageSize(pageWidthIn, pageHeightIn, Inches);
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
              QMap<int, PdfPage>::iterator i = pages.find(Gui::displayPageNum);
              if (i != pages.end())
                  pages.erase(i);
              pages.insert(Gui::displayPageNum,pdfPage);

              // wrap up
              painter.end();
          }
      } // end of step 1. generate page pixmaps

      if (Preferences::modeGUI)
          gui->m_progressDialog->setValue(printPages.count());

      if (exportPdfElements) {
          // wrap up paint to pdfWriter
          painter.end();
      } else {
          // step 2. paint generated page pixmaps to the pdfWriter
          painter.begin(&pdfWriter);

          if (Preferences::modeGUI)
              gui->m_progressDialog->setRange(1,pages.count());

          int page;
          Q_FOREACH (page, pages.keys()) {

              message = tr("Step 2. Exporting pdf document page %1 of %2").arg(page).arg(pages.count());
              emit gui->messageSig(LOG_INFO_STATUS,message);

              if (Preferences::modeGUI) {
                  gui->m_progressDialog->setLabelText(message);
                  gui->m_progressDialog->setValue(page);
                  QApplication::processEvents();
              }

              if (! Gui::exporting()) {
                  painter.end();
                  message = tr("Export to pdf terminated before completion. %2 pages of %3 processed%4.")
                                .arg(page).arg(pages.count()).arg(gui->elapsedTime(exportTimer.elapsed()));
                  emit gui->messageSig(LOG_INFO_STATUS,message);
                  if (Preferences::modeGUI) {
                      QApplication::restoreOverrideCursor();
                      gui->m_progressDialog->setBtnToClose();
                      gui->m_progressDialog->setLabelText(message, true/*alert*/);
                  }
                  emit gui->setExportingSig(false);
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
              gui->m_progressDialog->setValue(pages.count());

          // wrap up paint to pdfWriter
          pages.clear();
          painter.end();
      } // end of step 2. paint generated page pixmaps to the pdfWriter
  }

  // hide progress bar
  if (Preferences::modeGUI) {
      QApplication::restoreOverrideCursor();
      gui->m_progressDialog->hide();
  }

  // release Visual Editor
  emit gui->setExportingSig(false);

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
          for (const QString &item : Gui::messageList)
              if (item.contains(errorRx) || item.contains(fatalRx))
                  errorSet++;
              else if (! warnSet && item.contains(warnRx))
                  warnSet++;
          message.append(tr("<br><br>There %1 %2%3 :<br>%4")
                            .arg(Gui::messageList.size() == 1 ? tr("was") : tr("were"))
                            .arg(errorSet ? QString("%1 %2").arg(QString::number(errorSet)).arg(errorSet == 1 ? tr("error") : tr("errors")) : "")
                            .arg(warnSet  ? QString("%1%2 %3").arg(errorSet ? tr(" and ") : "").arg(QString::number(warnSet )).arg(warnSet  == 1 ? tr("warning") : tr("warnings")) : "")
                            .arg(Gui::messageList.join(" ")));
      }

      box.setWindowIcon(QIcon());
      box.setIconPixmap (QPixmap(LPUB3D_MESSAGE_ICON));
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
      QProcess *Process = new QProcess(gui);
      Process->setWorkingDirectory(QFileInfo(CommandPath).absolutePath() + QDir::separator());
      QStringList arguments = QStringList() << CommandPath;
      Process->start(UNIX_SHELL, arguments);
      Process->waitForFinished();
      if (Process->exitStatus() != QProcess::NormalExit || Process->exitCode() != 0) {
          QErrorMessage *m = new QErrorMessage(gui);
          m->showMessage(tr("Failed to launch PDF document.\n%1\n%2")
                             .arg(CommandPath).arg(QString(Process->readAllStandardError())));
      }
#endif
  } else {
      LogType logType = Preferences::modeGUI ? LOG_INFO_STATUS : LOG_INFO;
      emit gui->messageSig(logType, tr("Export to pdf completed! %1").arg(exportTime));
  }
}

void Gui::exportAs(const QString &_suffix)
{
  QString suffix = QString(_suffix).replace(".","").toUpper();
  QString directoryName = Gui::m_saveDirectoryName.isEmpty() ? QDir::currentPath() : Gui::m_saveDirectoryName;

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
  Gui::prevDisplayPageNum = Gui::displayPageNum;

  // init drawPage flags
  DrawPageFlags dpFlags;

  // return to whatever page we were viewing before printing
  auto restoreCurrentPage = [&]()
  {
      Gui::displayPageNum = Gui::prevDisplayPageNum;
      if (Gui::abortProcess())
          gui->restorePreviousPage();
      else
          Gui::displayPage();
  };

  // Switch to Native Renderer for fast processing
  if (Gui::exportingObjects()) {

      Gui::setNativeRenderer();

      if (Gui::m_saveDirectoryName.isEmpty()) {
          char *exportsDir = LDVWidget::getExportsDir();
          if (exportsDir) {
              stripTrailingPathSeparators(exportsDir);
              Gui::m_saveDirectoryName = exportsDir;
          } else {
              Gui::m_saveDirectoryName = directoryName;
          }
          delete[] exportsDir;
      }
  }

  // determine location to output images
  QFileInfo fileInfo(Gui::curFile);

  // add pixel ratio info to file name
  QString dpiInfo = QString("_%1_DPI").arg(int(resolution()));
  if (exportPixelRatio > 1.0 || exportPixelRatio < 1.0) {
      dpiInfo += QString("_%1x").arg(exportPixelRatio);
  }

  gui->KexportScene = LGraphicsScene(gui);
  gui->KexportView = LGraphicsView(&gui->KexportScene);

  float pageWidthPx = 0.0f, pageHeightPx = 0.0f;
  int adjPageWidthPx = 0, adjPageHeightPx = 0;

  int _displayPageNum = 0;
  int _maxPages       = 0;
  QStringList pageRanges;
  QList<int> printPages;

  // initialize page sizes
  Gui::displayPageNum = 0;
  dpFlags.printing = true;
  drawPage(dpFlags);
  Gui::clearPage();
  Gui::displayPageNum = Gui::prevDisplayPageNum;

  if (Gui::processOption != EXPORT_PAGE_RANGE) {

      if(Gui::processOption == EXPORT_ALL_PAGES) {
          _displayPageNum = 1 + Gui::pa;
          _maxPages = Gui::maxPages;
      }
      if (Gui::processOption == EXPORT_CURRENT_PAGE) {
          _displayPageNum = Gui::displayPageNum;
          _maxPages       = Gui::displayPageNum;
      }

  } else {

      pageRanges = Gui::pageRangeText.split(",");
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

  if (Preferences::modeGUI && Gui::m_saveDirectoryName.isEmpty()) {
      directoryName = QFileDialog::getExistingDirectory(
          gui,
          tr("Save %1 %2 to folder").arg(suffix).arg(type),
          QDir::currentPath(),
          QFileDialog::ShowDirsOnly);
      if (directoryName == "") {
          // release Visual Editor
          emit gui->setExportingSig(false);
          restoreCurrentPage();
          return;
      }
      Gui::m_saveDirectoryName = directoryName;
  } else
  if (!Gui::m_saveDirectoryName.isEmpty()) {
      directoryName = Gui::m_saveDirectoryName;
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

  gui->m_progressDialog->setWindowTitle(tr("Export as %1 %2").arg(suffix).arg(type));
  gui->m_progressDialog->setLabelText(tr("Exporting %1").arg(message));
  if (Preferences::modeGUI) {
      gui->m_progressDialog->setBtnToCancel();
      gui->m_progressDialog->show();
      QApplication::setOverrideCursor(Qt::ArrowCursor);
      QCoreApplication::processEvents();
  }
  emit gui->messageSig(LOG_INFO_STATUS,tr("Starting export %1").arg(message));

  if (Gui::processOption != EXPORT_PAGE_RANGE) {

      message = tr("Export %1 %2 to %3").arg(_maxPages).arg(type).arg(suffix);
      emit gui->messageSig(LOG_INFO_STATUS,message);

      if (Preferences::modeGUI) {
          gui->m_progressDialog->setLabelText(message);
          gui->m_progressDialog->setRange(_displayPageNum,_maxPages);
          QCoreApplication::processEvents();
      }

      for (Gui::displayPageNum = _displayPageNum; Gui::displayPageNum <= _maxPages; Gui::displayPageNum++) {

          if (! Gui::exporting()) {
              message = tr("Export to pdf terminated before completion. %1 %2 of %3 processed%%4.")
                            .arg(Gui::displayPageNum - 1).arg(_maxPages).arg(type).arg(gui->elapsedTime(exportTimer.elapsed()));
              emit gui->messageSig(LOG_INFO_STATUS,message);
              if (Preferences::modeGUI) {
                  QApplication::restoreOverrideCursor();
                  gui->m_progressDialog->setBtnToClose();
                  gui->m_progressDialog->setLabelText(message, true/*alert*/);
              }
              emit gui->setExportingSig(false);
              restoreCurrentPage();
              return;
          }

          message = tr("Exporting %1 %2: %3 of %4...")
                       .arg(suffix).arg(type)
                       .arg(Gui::displayPageNum)
                       .arg(_maxPages);
          emit gui->messageSig(LOG_INFO_STATUS,message);

          if (Preferences::modeGUI) {
              gui->m_progressDialog->setLabelText(message);
              gui->m_progressDialog->setValue(Gui::displayPageNum);
              QApplication::processEvents();
          }

          if (Gui::exportingObjects()) {

              // execute drawPage to prperly process csiParts for content generation
              dpFlags.printing = false;
              drawPage(dpFlags);
              Gui::clearPage();

          } else {
              // determine size of output image, in pixels
              gui->getExportPageSize(pageWidthPx, pageHeightPx);
              adjPageWidthPx = int(double(pageWidthPx) * dpr);
              adjPageHeightPx = int(double(pageHeightPx) * dpr);

              bool  ls = getPageOrientation() == Landscape;
              message = tr("Exporting %1 %2 %3 of %4, size(in pixels) W %5 x H %6, orientation %7, DPI %8, pixel ratio %9")
                           .arg(suffix)
                           .arg(type)
                           .arg(Gui::displayPageNum)
                           .arg(_maxPages)
                           .arg(adjPageWidthPx)
                           .arg(adjPageHeightPx)
                           .arg(ls ? "Landscape" : "Portrait")
                           .arg(int(resolution()))
                           .arg(dpr);
              emit gui->messageSig(LOG_NOTICE,message);

              // paint to the image the scene we view
              QImage image(adjPageWidthPx, adjPageHeightPx, QImage::Format_ARGB32);
              image.setDevicePixelRatio(dpr);

              QPainter painter;
              painter.begin(&image);

              // set up the view
              QRectF boundingRect(0.0, 0.0, int(pageWidthPx),int(pageHeightPx));
              QRect bounding(0, 0, int(pageWidthPx),int(pageHeightPx));
              gui->KexportView.scale(1.0,1.0);
              gui->KexportView.setMinimumSize(int(pageWidthPx),int(pageHeightPx));
              gui->KexportView.setMaximumSize(int(pageWidthPx),int(pageHeightPx));
              gui->KexportView.setGeometry(bounding);
              gui->KexportView.setSceneRect(boundingRect);
              gui->KexportView.setRenderHints(
                    QPainter::Antialiasing |
                    QPainter::TextAntialiasing |
                    QPainter::SmoothPixmapTransform);
              gui->KexportView.centerOn(boundingRect.center());
              Gui::clearPage();

              // clear the pixels of the image, just in case the background is
              // transparent or uses a PNG image with transparency. This will
              // prevent rendered pixels from each page layering on top of each
              // other.
              image.fill(fillPng ? Qt::transparent : Qt::white);

              // render this page
              // scene.render instead of view.render resolves "warm up" issue
              dpFlags.printing = true;
              drawPage(dpFlags);
              gui->KexportScene.setSceneRect(0.0,0.0,image.width(),image.height());
              gui->KexportScene.render(&painter);
              Gui::clearPage();

              // save the image to the selected directory
              // internationalization of "_page_"?
              QString pn = QString::number(Gui::displayPageNum);
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
                  emit gui->messageSig(LOG_WARNING,message);
              }
              painter.end();
          }
      }

      if (Preferences::modeGUI)
          gui->m_progressDialog->setValue(_maxPages);

    } else {

      if (Preferences::modeGUI)
          gui->m_progressDialog->setRange(1,printPages.count());

      int _pageCount = 0;

      Q_FOREACH (int printPage,printPages) {

          if (! Gui::exporting()) {
              message = tr("Export to pdf terminated before completion. %1 %2 of %3 processed%%4.")
                           .arg(_pageCount).arg(printPages.count()).arg(type).arg(gui->elapsedTime(exportTimer.elapsed()));
              emit gui->messageSig(LOG_INFO_STATUS,message);
              if (Preferences::modeGUI) {
                  QApplication::restoreOverrideCursor();
                  gui->m_progressDialog->setBtnToClose();
                  gui->m_progressDialog->setLabelText(message, true/*alert*/);
              }
              emit gui->setExportingSig(false);
              restoreCurrentPage();
              return;
          }

          Gui::displayPageNum = printPage;

          message = tr("Exporting %1 %2 %3 of range %4...")
                       .arg(suffix).arg(type)
                       .arg(_pageCount++)
                       .arg(pageRanges.join(" "));
          emit gui->messageSig(LOG_INFO_STATUS,message);

          if (Preferences::modeGUI) {
              gui->m_progressDialog->setLabelText(message);
              gui->m_progressDialog->setValue(_pageCount++);
              QApplication::processEvents();
          }

          if (Gui::exportingObjects()) {

              // execute drawPage to prperly process csiParts for content generation
              dpFlags.printing = false;
              drawPage(dpFlags);
              Gui::clearPage();

          } else {
              // determine size of output image, in pixels
              gui->getExportPageSize(pageWidthPx, pageHeightPx);
              adjPageWidthPx  = int(double(pageWidthPx) * dpr);
              adjPageHeightPx = int(double(pageHeightPx) * dpr);

              bool  ls = getPageOrientation() == Landscape;
              message = tr("Exporting %1 %2 %3 of %4, size(in pixels) W %5 x H %6, orientation %7, DPI %8, pixel ratio %9")
                            .arg(suffix)
                            .arg(type)
                            .arg(Gui::displayPageNum)
                            .arg(_maxPages)
                            .arg(adjPageWidthPx)
                            .arg(adjPageHeightPx)
                            .arg(ls ? "Landscape" : "Portrait")
                            .arg(int(resolution()))
                            .arg(dpr);
              emit gui->messageSig(LOG_NOTICE,message);

              // paint to the image the scene we view
              QImage image(adjPageWidthPx, adjPageHeightPx, QImage::Format_ARGB32);
              image.setDevicePixelRatio(dpr);

              QPainter painter;
              painter.begin(&image);

              QRectF boundingRect(0.0, 0.0, int(pageWidthPx),int(pageHeightPx));
              QRect bounding(0, 0, int(pageWidthPx),int(pageHeightPx));
              gui->KexportView.scale(1.0,1.0);
              gui->KexportView.setMinimumSize(int(pageWidthPx),int(pageHeightPx));
              gui->KexportView.setMaximumSize(int(pageWidthPx),int(pageHeightPx));
              gui->KexportView.setGeometry(bounding);
              gui->KexportView.setSceneRect(boundingRect);
              gui->KexportView.setRenderHints(
                    QPainter::Antialiasing |
                    QPainter::TextAntialiasing |
                    QPainter::SmoothPixmapTransform);
              gui->KexportView.centerOn(boundingRect.center());
              Gui::clearPage();

              // clear the pixels of the image, just in case the background is
              // transparent or uses a PNG image with transparency. This will
              // prevent rendered pixels from each page layering on top of each
              // other.
              image.fill(fillPng ? Qt::transparent : Qt::white);

              // render this page
              // scene.render instead of view.render resolves "warm up" issue
              dpFlags.printing = true;
              drawPage(dpFlags);
              gui->KexportScene.setSceneRect(0.0,0.0,image.width(),image.height());
              gui->KexportScene.render(&painter);
              Gui::clearPage();

              // save the image to the selected directory
              // internationalization of "_page_"?
              QString pn = QString::number(Gui::displayPageNum);
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
                  emit gui->messageSig(LOG_WARNING,message);
              }
              painter.end();
          }
      }
      if (Preferences::modeGUI)
          gui->m_progressDialog->setValue(printPages.count());
    }

    // hide progress bar
    if (Preferences::modeGUI) {
      QApplication::restoreOverrideCursor();
      gui->m_progressDialog->hide();
    }

    // restore preferred renderer
    if (Gui::exportingObjects()) {
        gui->restorePreferredRenderer();
    }

    // release Visual Editor
    emit gui->setExportingSig(false);

    // set elapsed time
    exportTime = gui->elapsedTime(exportTimer.elapsed());

    // return to whatever page we were viewing before output
    restoreCurrentPage();

    //display completion message
    QMessageBoxResizable box;
    if (Preferences::modeGUI) {

      message = tr("Your %1 export completed. %2").arg(type).arg(exportTime);

      if (Gui::messageList.size()) {
          int errorSet = 0;
          int warnSet  = 0;
          QRegExp errorRx(">ERROR<");
          QRegExp fatalRx(">FATAL<");
          QRegExp warnRx(">WARNING<");
          for (const QString &item : Gui::messageList)
            if (item.contains(errorRx) || item.contains(fatalRx))
                errorSet++;
            else if (! warnSet && item.contains(warnRx))
                warnSet++;
          message.append(tr("<br><br>There %1 %2%3 :<br>%4")
                             .arg(Gui::messageList.size() == 1 ? tr("was") : tr("were"))
                             .arg(errorSet ? QString("%1 %2").arg(QString::number(errorSet)).arg(errorSet == 1 ? tr("error") : tr("errors")) : "")
                             .arg(warnSet  ? QString("%1%2 %3").arg(errorSet ? tr(" and ") : "").arg(QString::number(warnSet )).arg(warnSet  == 1 ? tr("warning") : tr("warnings")) : "")
                             .arg(Gui::messageList.join(" ")));
      }

      box.setWindowIcon(QIcon());
      box.setIconPixmap (QPixmap(LPUB3D_MESSAGE_ICON));
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
      emit gui->messageSig(logType, tr("Export %1 %2 completed! %3")
                                  .arg(suffix)
                                  .arg(type)
                                  .arg(exportTime));
      emit gui->messageSig(LOG_INFO, tr("Exported %1 %2 path: %3")
                                   .arg(suffix)
                                   .arg(type)
                                   .arg(directoryName));
    }

    Gui::m_saveDirectoryName.clear();
}

//-----------------PRINT FUNCTIONS------------------------//

void Gui::Print(QPrinter* Printer)
{
  QElapsedTimer exportTimer;
  exportTimer.start();
  displayPageTimer.start();

  bool preview = exportPreview;
  QString const mode = exportPdf ? "pdf" : tr("file");
  QString const action = preview ? tr("Preview") : exportPdf ? tr("Export") : tr("Print");

  emit gui->hidePreviewDialogSig();

  /*
   * Options:
   *
   */
  int DocCopies;
  int PageCopies;

  int PageCount = Gui::maxPages;
  Gui::prevDisplayPageNum = Gui::displayPageNum;

  // set drawPage flags
  DrawPageFlags dpFlags;

  // return to whatever page we were viewing before printing
  auto restoreCurrentPage = [&]()
  {
    Gui::displayPageNum = Gui::prevDisplayPageNum;
    if (Gui::abortProcess())
      gui->restorePreviousPage();
    else
      Gui::displayPage();
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
    FromPage = 1 + Gui::pa;
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

  gui->KexportScene = LGraphicsScene(gui);
  gui->KexportView = LGraphicsView(&gui->KexportScene);

  // initialize page sizes
  Gui::displayPageNum = 0;
  dpFlags.printing = true;
  drawPage(dpFlags);
  Gui::clearPage();

  // check if mixed page size and orientation
  gui->checkMixedPageSizeStatus();

  // export over the entire page
  Printer->setFullPage(true);

  // set displayPageNum so we can send the correct index to retrieve page size data
  Gui::displayPageNum = FromPage;

  // set initial page layout using first page as default
  Printer->setPageLayout(getPageLayout());

  // paint to the printer
  QPainter Painter(Printer);

  // initialize progress bar dialog
  QString message = preview ? tr("Generating preview %1...").arg(mode) : exportPdf ? tr("Exporting %1...").arg(mode) : tr("Printing %1...").arg(mode);

  gui->m_progressDialog->setWindowTitle(QString("%1 %2").arg(action).arg(mode));
  gui->m_progressDialog->setLabelText(message);
  if (Preferences::modeGUI) {
    gui->m_progressDialog->setBtnToCancel();
    gui->m_progressDialog->show();
    QApplication::setOverrideCursor(Qt::ArrowCursor);
    QCoreApplication::processEvents();
  }
  emit gui->messageSig(LOG_INFO_STATUS, message);

  for (int DocCopy = 0; DocCopy < DocCopies; DocCopy++)
  {
    int Page = FromPage;

    message = tr("%1 page %2 of %3 for document %4...").arg(action).arg(Page).arg(ToPage).arg(DocCopy);
    emit gui->messageSig(LOG_INFO_STATUS,message);

    gui->m_progressDialog->setRange(Page,ToPage);

    if (Preferences::modeGUI) {
          gui->m_progressDialog->setLabelText(message);
          gui->m_progressDialog->setRange(Page,ToPage);
          QCoreApplication::processEvents();
    }

    if (Gui::processOption != EXPORT_PAGE_RANGE) {

      if (Gui::processOption == EXPORT_CURRENT_PAGE) {
          // reset display page from saved page
          Gui::displayPageNum = Gui::prevDisplayPageNum;
          // reset page layout using display page
          Printer->setPageLayout(getPageLayout());
          // set range to display page
          Page   = Gui::displayPageNum;
          ToPage = Gui::displayPageNum;
      }

      for (Gui::displayPageNum = Page; Gui::displayPageNum <= ToPage; Gui::displayPageNum++)
      {
        if (Printer->printerState() == QPrinter::Aborted || Printer->printerState() == QPrinter::Error || ! Gui::exporting())
        {
          message = tr("%1 %2 terminated before completion.").arg(action, mode);
          emit gui->messageSig(LOG_INFO_STATUS,message);
          if (preview) {
            m_previewDialog = false;
            exportPdf = exportPreview = true;
          }
          if (Preferences::modeGUI) {
            QApplication::restoreOverrideCursor();
            gui->m_progressDialog->setBtnToClose();
            gui->m_progressDialog->setLabelText(message, true/*alert*/);
          }
          emit gui->setExportingSig(false);
          restoreCurrentPage();
          return;
        }

        message = tr("%1 %2 page %3 of %4...")
                     .arg(action)
                     .arg(mode)
                     .arg(Page)
                     .arg(ToPage);
        emit gui->messageSig(LOG_INFO_STATUS,message);
        if (Preferences::modeGUI) {
          gui->m_progressDialog->setLabelText(message);
          gui->m_progressDialog->setValue(Page);
          QApplication::processEvents();
        }

        // determine size of output image, in pixels. dimension are inches * pixels per inch
        float pageWidthPx = 0.0f, pageHeightPx = 0.0f;
        gui->getExportPageSize(pageWidthPx, pageHeightPx);

        bool  ls = getPageOrientation() == Landscape;
        message = tr("%1 %2 page %3 of %4, size(in pixels) W %5 x H %6, orientation %7")
                     .arg(action)
                     .arg(mode)
                     .arg(Gui::displayPageNum)
                     .arg(ToPage)
                     .arg(double(pageWidthPx))
                     .arg(double(pageHeightPx))
                     .arg(ls ? tr("Landscape") : tr("Portrait"));
        emit gui->messageSig(LOG_NOTICE,message);

        // set up the view
        QRectF boundingRect(0.0, 0.0, double(pageWidthPx), double(pageHeightPx));
        QRect bounding(0, 0, int(pageWidthPx), int(pageHeightPx));
        gui->KexportView.scale(1.0,1.0);
        gui->KexportView.setMinimumSize(int(pageWidthPx),int(pageHeightPx));
        gui->KexportView.setMaximumSize(int(pageWidthPx),int(pageHeightPx));
        gui->KexportView.setGeometry(bounding);
        gui->KexportView.setSceneRect(boundingRect);
        gui->KexportView.setRenderHints(
              QPainter::Antialiasing |
              QPainter::TextAntialiasing |
              QPainter::SmoothPixmapTransform);
        gui->KexportView.centerOn(boundingRect.center());
        Gui::clearPage();

        for (int PageCopy = 0; PageCopy < PageCopies; PageCopy++)
        {
          if (Printer->printerState() == QPrinter::Aborted || Printer->printerState() == QPrinter::Error || ! Gui::exporting())
          {
            message = tr("%1 %2 terminated before completion.").arg(action, mode);
            emit gui->messageSig(LOG_INFO_STATUS,message);
            if (preview) {
              m_previewDialog = false;
              exportPdf = exportPreview = false;
            }
            if (Preferences::modeGUI) {
              QApplication::restoreOverrideCursor();
              gui->m_progressDialog->setBtnToClose();
              gui->m_progressDialog->setLabelText(message, true/*alert*/);
            }
            emit gui->setExportingSig(false);
            restoreCurrentPage();
            return;
          }

          if (PageCopies > 1) {
            message = tr("%1 %2 page copy %3 of %4...").arg(action).arg(mode).arg(PageCopy).arg(PageCopies);
            emit gui->messageSig(LOG_INFO_STATUS,message);

            if (Preferences::modeGUI) {
              gui->m_progressDialog->setLabelText(message);
              // gui->m_progressDialog->setValue(Page);
              QApplication::processEvents();
            }
          }

          // render this page
          dpFlags.printing = true;
          drawPage(dpFlags);
          gui->KexportScene.setSceneRect(0.0,0.0,double(pageWidthPx),double(pageHeightPx));
          gui->KexportScene.render(&Painter);
          Gui::clearPage();

          // TODO: export header and footer

          if (PageCopy < PageCopies - 1) {
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

      if (Preferences::modeGUI)
          gui->m_progressDialog->setValue(ToPage);

    } else {

      // page range
      QStringList pageRanges = Gui::pageRangeText.split(",");
      QList<int> printPages;
      for (QString &ranges : pageRanges)
      {
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

      ToPage = printPages.last();

      if (Preferences::modeGUI)
        gui->m_progressDialog->setRange(1,printPages.count());

      int _pageCount = 0;

      for (int printPage : printPages)
      {

        if (Printer->printerState() == QPrinter::Aborted || Printer->printerState() == QPrinter::Error || ! Gui::exporting())
        {
          message = tr("%1 %2 terminated before completion.").arg(action, mode);
          emit gui->messageSig(LOG_INFO_STATUS,message);
          if (preview) {
            m_previewDialog = false;
            exportPdf = exportPreview = false;
          }
          if (Preferences::modeGUI) {
            QApplication::restoreOverrideCursor();
            gui->m_progressDialog->setBtnToClose();
            gui->m_progressDialog->setLabelText(message, true/*alert*/);
          }
          emit gui->setExportingSig(false);
          restoreCurrentPage();
          return;
        }

        Gui::displayPageNum = Page = printPage;

        message = tr("%1 %2 %3 of %4 for range %5 ")
                     .arg(action)
                     .arg(mode)
                     .arg(Page)
                     .arg(printPages.count())
                     .arg(pageRanges.join(" "));
        emit gui->messageSig(LOG_INFO_STATUS,message);

        if (Preferences::modeGUI) {
          gui->m_progressDialog->setLabelText(message);
          gui->m_progressDialog->setValue(_pageCount++);
          QApplication::processEvents();
        }

        // determine size of output image, in pixels. dimension are inches * pixels per inch
        float pageWidthPx = 0.0f, pageHeightPx = 0.0f;
        gui->getExportPageSize(pageWidthPx, pageHeightPx);

        bool  ls = getPageOrientation() == Landscape;
        message = tr("%1 %2 page %3 of %4, size(pixels) W %5 x H %6, orientation %7 for range %8")
                     .arg(action)
                     .arg(mode)
                     .arg(Page)
                     .arg(printPages.count())
                     .arg(double(pageWidthPx))
                     .arg(double(pageHeightPx))
                     .arg(ls ? tr("Landscape") : tr("Portrait"))
                     .arg(pageRanges.join(" "));
        emit gui->messageSig(LOG_NOTICE,message);

        // set up the view
        QRectF boundingRect(0.0, 0.0, double(pageWidthPx), double(pageHeightPx));
        QRect bounding(0, 0, int(pageWidthPx), int(pageHeightPx));
        gui->KexportView.scale(1.0,1.0);
        gui->KexportView.setMinimumSize(int(pageWidthPx),int(pageHeightPx));
        gui->KexportView.setMaximumSize(int(pageWidthPx),int(pageHeightPx));
        gui->KexportView.setGeometry(bounding);
        gui->KexportView.setSceneRect(boundingRect);
        gui->KexportView.setRenderHints(
              QPainter::Antialiasing |
              QPainter::TextAntialiasing |
              QPainter::SmoothPixmapTransform);
        gui->KexportView.centerOn(boundingRect.center());
        Gui::clearPage();

        for (int PageCopy = 0; PageCopy < PageCopies; PageCopy++)
          {
            if (Printer->printerState() == QPrinter::Aborted || Printer->printerState() == QPrinter::Error || ! Gui::exporting())
            {
              message = tr("%1 %2 terminated before completion.").arg(action, mode);
              emit gui->messageSig(LOG_INFO_STATUS,message);
              if (preview) {
                m_previewDialog = false;
                exportPdf = exportPreview = false;
              }
              if (Preferences::modeGUI) {
                QApplication::restoreOverrideCursor();
                gui->m_progressDialog->setBtnToClose();
                gui->m_progressDialog->setLabelText(message, true/*alert*/);
              }
              emit gui->setExportingSig(false);
              restoreCurrentPage();
              return;
            }

            if (PageCopies > 1) {
              message = tr("%1 %2 page copy %3 of %4...").arg(action).arg(mode).arg(PageCopy).arg(PageCopies);
              emit gui->messageSig(LOG_INFO_STATUS,message);

              if (Preferences::modeGUI) {
                gui->m_progressDialog->setLabelText(message);
                // gui->m_progressDialog->setValue(Page);
                QApplication::processEvents();
              }
            }

            // render this page
            dpFlags.printing = true;
            drawPage(dpFlags);
            gui->KexportScene.setSceneRect(0.0,0.0,int(pageWidthPx),int(pageHeightPx));
            gui->KexportScene.render(&Painter);
            Gui::clearPage();

            // TODO: export header and footer

            if (PageCopy < PageCopies - 1) {
              Printer->newPage();
            }
          }
        bool nextPage = true;
        QPageLayout::Orientation pageOrientation = getPageOrientation(nextPage) == Landscape ? QPageLayout::Landscape : QPageLayout::Portrait;
        Printer->setPageOrientation(pageOrientation);
        Printer->newPage();
      }
    }

    if (Preferences::modeGUI)
      gui->m_progressDialog->setValue(ToPage);

    if (DocCopy < DocCopies - 1) {
      Printer->newPage();
    }
  }

  // release Visual Editor
  setExportingSig(false);

  // return to whatever page we were viewing before output
  Gui::displayPageNum = Gui::prevDisplayPageNum;
  dpFlags.printing = false;
  drawPage(dpFlags);

  // hide progress bar
  if (Preferences::modeGUI) {
    QApplication::restoreOverrideCursor();
    gui->m_progressDialog->hide();
  }

  if (preview) {
    m_previewDialog = false;
    exportPdf = exportPreview = false;
  }

  exportTime = gui->elapsedTime(exportTimer.elapsed());
}

void Gui::showExportedFile()
{
  if (exportedFile.isEmpty())
    return;

  bool preview = exportPreview;
  QString const mode = exportPdf ? "pdf" : tr("file");
  QString const action = preview ? tr("Preview") : exportPdf ? tr("Export") : tr("Print");

  QMessageBoxResizable box;
  if (Preferences::modeGUI) {

    QString message = tr("Your %1 %2 completed. %3").arg(mode,action.toLower(),exportTime);

    if (Gui::messageList.size()) {
      int errorSet = 0;
      int warnSet  = 0;
      QRegExp errorRx(">ERROR<");
      QRegExp fatalRx(">FATAL<");
      QRegExp warnRx(">WARNING<");
      for (const QString &item : Gui::messageList)
        if (item.contains(errorRx) || item.contains(fatalRx))
            errorSet++;
        else if (! warnSet && item.contains(warnRx))
            warnSet++;
      message.append(tr("<br><br>There %1 %2%3 :<br>%4")
                         .arg(Gui::messageList.size() == 1 ? tr("was") : tr("were"))
                         .arg(errorSet ? QString("%1 %2").arg(QString::number(errorSet)).arg(errorSet == 1 ? tr("error") : tr("errors")) : "")
                         .arg(warnSet  ? QString("%1%2 %3").arg(errorSet ? tr(" and ") : "").arg(QString::number(warnSet )).arg(warnSet  == 1 ? tr("warning") : tr("warnings")) : "")
                         .arg(Gui::messageList.join(" ")));
    }

    //display completion message
    QMessageBoxResizable box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (QPixmap(LPUB3D_MESSAGE_ICON));
    box.setTextFormat (Qt::RichText);
    box.setStandardButtons (QMessageBox::Close);
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    box.setWindowTitle(tr ("%1 Status").arg(action));

    box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton   (QMessageBox::Yes);

    QString title = "<b>" + tr ("%1 %2 completed.").arg(action, mode) + "</b>";
    QString text = tr ("%1<br><br>Do you want to open this document ?<br><br>%2")
                       .arg(message).arg(QDir::toNativeSeparators(exportedFile));

    box.setText (title);
    box.setInformativeText (text);
  }

  if (Preferences::modeGUI && box.exec() == QMessageBox::Yes) {
    const QString CommandPath = exportedFile;
#ifdef Q_OS_WIN
    QDesktopServices::openUrl((QUrl("file:///"+CommandPath, QUrl::TolerantMode)));
#else
    QProcess *Process = new QProcess(gui);
    Process->setWorkingDirectory(QFileInfo(CommandPath).absolutePath() + QDir::separator());
    QStringList arguments = QStringList() << CommandPath;
    Process->start(UNIX_SHELL, arguments);
    Process->waitForFinished();
    if (Process->exitStatus() != QProcess::NormalExit || Process->exitCode() != 0) {
      QErrorMessage *m = new QErrorMessage(gui);
      m->showMessage(tr("Failed to launch exported document.\n%1\n%2")
                         .arg(CommandPath).arg(QString(Process->readAllStandardError())));
    }
#endif
  } else {
    LogType logType = Preferences::modeGUI ? LOG_INFO_STATUS : LOG_INFO;
    emit gui->messageSig(logType, tr("%1 %2 completed! %3").arg(action, mode, exportTime));
  }
}

void Gui::ShowPrintDialog()
{
    int PageCount = Gui::maxPages;

    QPrinter Printer(QPrinter::HighResolution);
    Printer.setFromTo(1 + Gui::pa, PageCount + 1);

    QPrintDialog PrintDialog(&Printer, gui);

    m_previewDialog = false;

    if (PrintDialog.exec() == QDialog::Accepted) {
        exportPdf = exportPreview = false;
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

    if (! exportAsDialog(m)) {
        m_previewDialog = false;
        return;
    }

    exportPreview = true;
    exportPdf    = m == EXPORT_PDF;

    int PageCount = Gui::maxPages;

    QPrinter Printer(QPrinter::ScreenResolution);

    //set page parameters
    Printer.setFromTo(1 + Gui::pa, PageCount + 1);

    QPrintPreviewDialog Preview(&Printer, gui);

    if (exportPdf) {
        // determine location for output file
        QFileInfo fileInfo(Gui::curFile);
        QString baseName = fileInfo.completeBaseName();
        QString fileName = QDir::currentPath() + "/" + baseName;
        fileInfo.setFile(fileName);
        QString suffix = fileInfo.suffix();
        if (suffix == "") {
            fileName += ".pdf";
        } else if (suffix != ".pdf" && suffix != ".PDF") {
            fileName = fileInfo.path() + "/" + fileInfo.completeBaseName() + ".pdf";
        }

        gui->exportedFile = fileName;
        Printer.setOutputFileName(gui->exportedFile);
    }

    connect(&Preview, SIGNAL(paintRequested(QPrinter*)),          SLOT(Print(QPrinter*)));
    connect(gui,     SIGNAL(hidePreviewDialogSig()),   &Preview, SLOT(hide()));

    int rc = Preview.exec();

    if (exportPdf) {
        emit gui->messageSig(LOG_STATUS, tr("PDF export preview result is %1")
                                       .arg(rc == 1 ? gui->exportedFile + tr(" exported") : tr("preview only")));  // 0=preview only, 1=export output
        if (rc == 1) {
            showExportedFile();
        }
    }

    m_previewDialog = false;
}
