/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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

#include "lpub.h"

struct paperSizes {
  QPrinter::PaperSize paperSize;
  float              width;
  float              height;
} paperSizes[] = {
{ QPrinter::A0,         841,   1189 },
{ QPrinter::A1,         594,    841 },
{ QPrinter::A2,         420,    594 },
{ QPrinter::A3,         297,    420 },
{ QPrinter::A4,         210,    297 },
{ QPrinter::A5,         148,    210 },
{ QPrinter::A6,         105,    148 },
{ QPrinter::A7,          74,    105 },
{ QPrinter::A8,          52,     74 },
{ QPrinter::A9,          37,     52 },
{ QPrinter::B0,        1030,   1456 },
{ QPrinter::B1,         728,   1030 },
{ QPrinter::B10,         32,     45 },
{ QPrinter::B2,         515,    728 },
{ QPrinter::B3,         364,    515 },
{ QPrinter::B4,         257,    364 },
{ QPrinter::B5,         182,    257 },
{ QPrinter::B6,         128,    182 },
{ QPrinter::B7,          91,    128 },
{ QPrinter::B8,          64,     91 },
{ QPrinter::B9,          45,     64 },
{ QPrinter::B10,         31,     44 },
{ QPrinter::C5E,        163,    229 },
{ QPrinter::Comm10E,    105,    241 },
{ QPrinter::DLE,        110,    220 },
{ QPrinter::Executive,  190.5,  254 },
{ QPrinter::Folio,      210,    330 },
{ QPrinter::Ledger,     431.8,  279.4 },
{ QPrinter::Legal,      215.9,  355.6 },
{ QPrinter::Letter,     215.9,  279.4 },
{ QPrinter::Tabloid,    279.4,  431.8 },
};

// Compare two variants.
bool lessThan(const int &v1, const int &v2)
{
    return v1 < v2;
}

int Gui::bestPaperSizeOrientation(
    float widthMm,
    float heightMm,
    QPrinter::PaperSize &paperSize,
    QPrinter::Orientation &orient)
{
  int   numPaperSizes = sizeof(paperSizes)/sizeof(paperSizes[0]);
  float diffWidth = 1000;
  float diffHeight = 1000;
  int     bestSize = 0;
  
  for (int i = 0; i < numPaperSizes; i++) {

      float widthDiff = paperSizes[i].width - widthMm;
      float heightDiff = paperSizes[i].height - heightMm;

      if (widthDiff >= 0  && heightDiff >= 0) {
          if (widthDiff <= diffWidth && heightDiff <= diffHeight) {
              bestSize = i;
              paperSize = paperSizes[i].paperSize;
              orient = QPrinter::Portrait;
              diffWidth = widthDiff;
              diffHeight = heightDiff;
            }
        }

      widthDiff = paperSizes[i].height - widthMm;
      heightDiff  = paperSizes[i].width - heightMm;

      if (widthDiff >= 0  && heightDiff >= 0) {
          if (widthDiff <= diffWidth && heightDiff <= diffHeight) {
              bestSize = i;
              paperSize = paperSizes[i].paperSize;
              orient = QPrinter::Landscape;
              diffWidth = widthDiff;
              diffHeight = heightDiff;
            }
        }
    }
  return bestSize;
}

void Gui::GetPixelDimensions(float &pixelWidth, float &pixelHeight)
{
  float pageWidthIn, pageHeightIn;

  // only concerned with inches because resolution() reports DPI
  if (page.meta.LPub.resolution.type() == DPI) {
      pageWidthIn = page.meta.LPub.page.size.value(0);
      pageHeightIn = page.meta.LPub.page.size.value(1);
    } else {
      pageWidthIn = centimeters2inches(page.meta.LPub.page.size.value(0));
      pageHeightIn = centimeters2inches(page.meta.LPub.page.size.value(1));
    }

  // pixel dimension are inches * pixels per inch
  pixelWidth = int((pageWidthIn * resolution()) + 0.5);
  pixelHeight = int((pageHeightIn * resolution()) + 0.5);
}


void Gui::GetPagePixelDimensions(float &pagePixelWidth, float &pagePixelHeight, QPrinter::PaperSize &paperSize, QPrinter::Orientation &orientation)
{
  float pageWidthMm, pageHeightMm;
  float pageWidthCm, pageHeightCm;
  float pageWidthIn, pageHeightIn;

  if (page.meta.LPub.resolution.type() == DPI) {
      pageWidthIn = page.meta.LPub.page.size.value(0);
      pageHeightIn = page.meta.LPub.page.size.value(1);
      pageWidthCm = inches2centimeters(pageWidthIn);
      pageHeightCm = inches2centimeters(pageHeightIn);
    } else {
      pageWidthCm = page.meta.LPub.page.size.value(0);
      pageHeightCm = page.meta.LPub.page.size.value(1);
      pageWidthIn = centimeters2inches(pageWidthCm);
      pageHeightIn = centimeters2inches(pageHeightCm);
    }

  pageWidthMm = pageWidthCm * 10.0;
  pageHeightMm = pageHeightCm * 10.0;

  int sizeIndex = bestPaperSizeOrientation(pageWidthMm, pageHeightMm, paperSize, orientation);

  if (orientation == QPrinter::Portrait) {
      pageWidthCm = paperSizes[sizeIndex].width / 10.0;
      pageHeightCm = paperSizes[sizeIndex].height / 10.0;
    } else {
      pageWidthCm = paperSizes[sizeIndex].height / 10.0;
      pageHeightCm = paperSizes[sizeIndex].width / 10.0;
    }

  pageWidthIn = centimeters2inches(pageWidthCm);
  pageHeightIn = centimeters2inches(pageHeightCm);

  // important: note that pixel dimensions converted back from mm paper sizes may
  // not match GetPixelDimensions. For example, Letter mm dimensions do not *exactly* equal 8.5 x 11

  pagePixelWidth = int((pageWidthIn * resolution()) + 0.5);
  pagePixelHeight = int((pageHeightIn * resolution()) + 0.5);
}

bool Gui::printToPdfDialog()
{
  exportType = EXPORT_PDF;
  DialogExportPages *dialog = new DialogExportPages();
  if (dialog->exec() == QDialog::Accepted) {
      if(dialog->allPages()){
          exportOption = EXPORT_ALL_PAGES;
        }
      if(dialog->currentPage()){
          exportOption = EXPORT_CURRENT_PAGE;
        }
      if(dialog->pageRange()){
          exportOption  = EXPORT_PAGE_RANGE;
          pageRangeText = dialog->pageRangeText();
        }
      if(dialog->resetCache()){
          clearPLICache();
          clearCSICache();
        }
      printToPdfFile();
      return true;
    } else {
      return false;
    }
}

bool Gui::exportAsPngDialog()
{
  exportType = EXPORT_PNG;
  DialogExportPages *dialog = new DialogExportPages();
  if (dialog->exec() == QDialog::Accepted) {
      if(dialog->allPages()){
          exportOption = EXPORT_ALL_PAGES;
        }
      if(dialog->currentPage()){
          exportOption = EXPORT_CURRENT_PAGE;
        }
      if(dialog->pageRange()){
          exportOption  = EXPORT_PAGE_RANGE;
          pageRangeText = dialog->pageRangeText();
        }
      exportAsPng();
      return true;
    } else {
      return false;
    }
}

bool Gui::exportAsJpgDialog()
{
  exportType = EXPORT_JPG;
  DialogExportPages *dialog = new DialogExportPages();
  if (dialog->exec() == QDialog::Accepted) {
      if(dialog->allPages()){
          exportOption = EXPORT_ALL_PAGES;
        }
      if(dialog->currentPage()){
          exportOption = EXPORT_CURRENT_PAGE;
        }
      if(dialog->pageRange()){
          exportOption  = EXPORT_PAGE_RANGE;
          pageRangeText = dialog->pageRangeText();
        }
      exportAsJpg();
      return true;
    } else {
      return false;
    }
}

bool Gui::exportAsBmpDialog()
{
  exportType = EXPORT_BMP;
  DialogExportPages *dialog = new DialogExportPages();
  if (dialog->exec() == QDialog::Accepted) {
      if(dialog->allPages()){
          exportOption = EXPORT_ALL_PAGES;
        }
      if(dialog->currentPage()){
          exportOption = EXPORT_CURRENT_PAGE;
        }
      if(dialog->pageRange()){
          exportOption  = EXPORT_PAGE_RANGE;
          pageRangeText = dialog->pageRangeText();
        }
      exportAsBmp();
      return true;
    } else {
      return false;
    }
}

void Gui::printToPdfFile()
{  
  int savePageNumber = displayPageNum;

  if (exportOption == EXPORT_PAGE_RANGE) {
      if(pageRangeText.isEmpty()){
          QMessageBox::warning(NULL,
                               tr("LPub3D"),
                               tr("Custom page range is empty. You must enter a page range"));
          return;
        }
      bool validEntry = true;
      QString message;
      QStringList pageRanges = pageRangeText.split(",");
      foreach(QString ranges, pageRanges){
          if (ranges.contains("-")){
              bool ok[2];
              QStringList range = ranges.split("-");
              int minPage = range[0].toInt(&ok[0]);
              int maxPage = range[1].toInt(&ok[1]);
              if (!ok[0] || !ok[1] || (minPage > maxPage)){
                  message = QString("%1-%2").arg(minPage).arg(maxPage);
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
          QMessageBox::warning(NULL,
                               tr("LPub3D"),
                               tr("Invalid page number(s) %1 detected. You must enter valid page number(s).").arg(message));
          return;
        }
    }

  // send signal to halt 3DViewer
  halt3DViewer(true);

  // determine location for output file
  QFileInfo fileInfo(curFile);
  QString baseName = fileInfo.baseName();
  QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Print File Name"),
        QDir::currentPath() + "/" + baseName,
        tr("PDF (*.pdf)"));

  if (fileName == "") {
      // release 3D Viewer
      halt3DViewer(false);
      return;
    }
  
  // want info about output file now, not model file
  fileInfo.setFile(fileName);
  
  // append proper PDF file extension if needed
  QString suffix = fileInfo.suffix();
  if (suffix == "") {
      fileName += ".pdf";
    } else if (suffix != ".pdf" && suffix != ".PDF") {
      fileName = fileInfo.path() + "/" + fileInfo.completeBaseName() + ".pdf";
    }

  // determine size of output pages, in pixels
  QPrinter::PaperSize paperSize = QPrinter::PaperSize();
  QPrinter::Orientation orientation = QPrinter::Orientation();
  float pageWidthPx, pageHeightPx;
  GetPagePixelDimensions(pageWidthPx, pageHeightPx, paperSize, orientation);

  // create a PDF printer
  QPrinter printer(QPrinter::ScreenResolution);
  //printer.setResolution(resolution());
  printer.setOutputFileName(fileName);
  printer.setOrientation(orientation);
  printer.setPaperSize(paperSize);
  printer.setPageMargins(0,0,0,0,QPrinter::Inch);
  printer.setFullPage(true);
  
  // paint to the printer the scene we view
  QPainter painter;
  painter.begin(&printer);
  QGraphicsScene scene;
  LGraphicsView view(&scene);
  
  // set up the view
  QRectF boundingRect(0.0, 0.0, pageWidthPx, pageHeightPx);
  QRect bounding(0, 0, pageWidthPx, pageHeightPx);
  view.scale(1.0,1.0);
  view.setMinimumSize(pageWidthPx,pageHeightPx);
  view.setMaximumSize(pageWidthPx,pageHeightPx);
  view.setGeometry(bounding);
  view.setSceneRect(boundingRect);
  view.setRenderHints(
        QPainter::Antialiasing |
        QPainter::TextAntialiasing |
        QPainter::SmoothPixmapTransform);
  view.centerOn(boundingRect.center());
  clearPage(&view,&scene);

  int _displayPageNum = 0;
  int _maxPages       = 0;

  // initialize progress bar dialog
  m_cancelPrinting = false;
  m_progressDialog->setWindowTitle("Print pdf");
  m_progressDialog->show();
  m_progressDlgMessageLbl->setText("Printing instructions to pdf.");

  if (exportOption != EXPORT_PAGE_RANGE){

      if(exportOption == EXPORT_ALL_PAGES){
          _displayPageNum = 1;
          _maxPages = maxPages;
        }

      if (exportOption == EXPORT_CURRENT_PAGE){
          _displayPageNum = displayPageNum;
          _maxPages       = displayPageNum;
        }

      m_progressDlgProgressBar->setRange(1,_maxPages);

      for (displayPageNum = _displayPageNum; displayPageNum <= _maxPages; displayPageNum++) {

          if (m_cancelPrinting)
            break;

          logNotice() << QString("Printing: page %1 of %2").arg(displayPageNum).arg(_maxPages);

          m_progressDlgMessageLbl->setText(QString("Printing: page %1 of %2").arg(displayPageNum).arg(_maxPages));
          m_progressDlgProgressBar->setValue(displayPageNum);

          // render this page
          drawPage(&view,&scene,true);
          scene.setSceneRect(0.0,0.0,pageWidthPx,pageHeightPx);
          scene.render(&painter);
          clearPage(&view,&scene);

          // prepare to print another page
          if(displayPageNum < _maxPages) {
              printer.newPage();
            }
        }
      m_progressDlgProgressBar->setValue(_maxPages);

    } else {

      QStringList pageRanges = pageRangeText.split(",");
      QList<int> printPages;
      foreach(QString ranges,pageRanges){
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

      _maxPages = printPages.last();

       m_progressDlgProgressBar->setRange(1,printPages.count());

      int _pageCount = 0;
      foreach(int printPage,printPages){

          if (m_cancelPrinting)
            break;

          displayPageNum = printPage;

          logNotice() << QString("Printing: page %1 of %2").arg(displayPageNum).arg(_maxPages);

          m_progressDlgMessageLbl->setText(QString("Printing: page %1 of %2").arg(displayPageNum).arg(_maxPages));
          m_progressDlgProgressBar->setValue(_pageCount++);

          // render this page
          drawPage(&view,&scene,true);
          scene.setSceneRect(0.0,0.0,pageWidthPx,pageHeightPx);
          scene.render(&painter);
          clearPage(&view,&scene);

          // prepare to print another page
          if(displayPageNum < _maxPages) {
              printer.newPage();
            }
        }
      m_progressDlgProgressBar->setValue(printPages.count());
    }

  painter.end();

  // release 3D Viewer
  halt3DViewer(false);

  // return to whatever page we were viewing before output
  displayPageNum = savePageNumber;
  drawPage(KpageView,KpageScene,false);

  // hide progress bar
  m_progressDialog->hide();

  if (!m_cancelPrinting) {

      emit messageSig(true,QString("Print to pdf completed."));

  //display completion message
  QMessageBox::StandardButton ret;
  ret = QMessageBox::information(this, tr(VER_PRODUCTNAME_STR),
                                 tr("Your instruction document has finished printing.\n"
                                    "Do you want to open this document ?\n %1").arg(fileName),
                                 QMessageBox::Yes| /*QMessageBox::Discard | */ QMessageBox::Cancel);

  if (ret == QMessageBox::Yes) {
      QString CommandPath = fileName;
      QProcess *Process = new QProcess(this);
      Process->setWorkingDirectory(QDir::currentPath() + "/");
      Process->setNativeArguments(CommandPath);

#ifdef __APPLE__

      Process->execute(CommandPath);
      Process->waitForFinished();

      QProcess::ExitStatus Status = Process->exitStatus();

      if (Status != 0) {  // look for error
          QErrorMessage *m = new QErrorMessage(this);
          m->showMessage(QString("%1\n%2").arg("Failed to launch PDF document!").arg(CommandPath));
        }
#else
      QDesktopServices::openUrl((QUrl("file:///"+CommandPath, QUrl::TolerantMode)));
#endif
      return;
    } else if (ret == QMessageBox::Cancel) {
      return;
    }
    } else {
      emit messageSig(true,QString("Print to pdf cancelled."));
    }
}

void Gui::exportAsPng()
{
  QString suffix(".png");
  exportAs(suffix);
}

void Gui::exportAsJpg()
{
  QString suffix(".jpg");
  exportAs(suffix);
}

void Gui::exportAsBmp()
{
  QString suffix(".bmp");
  exportAs(suffix);
}

void Gui::exportAs(QString &suffix)
{
  int savePageNumber = displayPageNum;

  if (exportOption == EXPORT_PAGE_RANGE) {
      if(pageRangeText.isEmpty()){
          QMessageBox::warning(NULL,
                               tr("LPub3D"),
                               tr("Custom page range is empty. You must enter a page range"));
          return;
        }
      bool validEntry = true;
      QString message;
      QStringList pageRanges = pageRangeText.split(",");
      foreach(QString ranges, pageRanges){
          if (ranges.contains("-")){
              bool ok[2];
              QStringList range = ranges.split("-");
              int minPage = range[0].toInt(&ok[0]);
              int maxPage = range[1].toInt(&ok[1]);
              if (!ok[0] || !ok[1] || (minPage > maxPage)){
                  message = QString("%1-%2").arg(minPage).arg(maxPage);
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
          QMessageBox::warning(NULL,
                               tr("LPub3D"),
                               tr("Invalid page number(s) %1 detected. You must enter valid page number(s).").arg(message));
          return;
        }
    }

  // send signal to halt 3DViewer
  halt3DViewer(true);

  // determine location to output images
  QFileInfo fileInfo(curFile);
  //QDir initialDirectory = fileInfo.dir();
  QString baseName = fileInfo.baseName();
  QString directoryName = QFileDialog::getExistingDirectory(
        this,
        tr("Save images to folder"), // needs translation! also, include suffix in here
        QDir::currentPath(),
        QFileDialog::ShowDirsOnly);
  if (directoryName == "") {
      // release 3D Viewer
      halt3DViewer(false);
      // remove progress bar
//      emit removeProgressPermStatusSig();
      return;
    }
  
  // determine size of output image, in pixels
  float pageWidthPx, pageHeightPx;
  GetPixelDimensions(pageWidthPx, pageHeightPx);
  
  // paint to the image the scene we view
  QImage image(pageWidthPx, pageHeightPx, QImage::Format_ARGB32);
  QPainter painter;
  painter.begin(&image);
  QGraphicsScene scene;
  LGraphicsView view(&scene);

  // set up the view
  QRectF boundingRect(0.0,0.0,pageWidthPx,pageHeightPx);
  QRect  bounding(0,0,pageWidthPx,pageHeightPx);
  view.scale(1.0,1.0);
  view.setMinimumSize(pageWidthPx, pageHeightPx);
  view.setMaximumSize(pageWidthPx, pageHeightPx);
  view.setGeometry(bounding);
  view.setSceneRect(boundingRect);
  view.setRenderHints(
        QPainter::Antialiasing |
        QPainter::TextAntialiasing |
        QPainter::SmoothPixmapTransform);
  view.centerOn(boundingRect.center());
  // view.fitInView(boundingRect);
  clearPage(&view, &scene);

  // Support transparency for formats that can handle it, but use white for those that can't.
  QColor fillClear = (suffix.compare(".png", Qt::CaseInsensitive) == 0) ? Qt::transparent :  Qt::white;

  int _displayPageNum = 0;
  int _maxPages       = 0;

  // initialize progress bar
  m_cancelPrinting = false;
  m_progressDialog->setWindowTitle(QString("Export as %1").arg(suffix));
  m_progressDialog->show();
  m_progressDlgMessageLbl->setText(QString("Exporting instructions to %1 format.").arg(suffix));

  if (exportOption != EXPORT_PAGE_RANGE){

      if(exportOption == EXPORT_ALL_PAGES){
          _displayPageNum = 1;
          _maxPages = maxPages;
        }

      if (exportOption == EXPORT_CURRENT_PAGE){
          _displayPageNum = displayPageNum;
          _maxPages       = displayPageNum;
        }

      m_progressDlgProgressBar->setRange(1,_maxPages);

      for (displayPageNum = _displayPageNum; displayPageNum <= _maxPages; displayPageNum++) {

          if (m_cancelPrinting)
            break;

          logNotice() << QString("Exporting page: %1 of %2").arg(displayPageNum).arg(_maxPages);

          m_progressDlgMessageLbl->setText(QString("Exporting page: %1 of %2").arg(displayPageNum).arg(_maxPages));
          m_progressDlgProgressBar->setValue(displayPageNum);

          // clear the pixels of the image, just in case the background is
          // transparent or uses a PNG image with transparency. This will
          // prevent rendered pixels from each page layering on top of each
          // other.
          image.fill(fillClear.Rgb);
          // render this page
          // scene.render instead of view.render resolves "warm up" issue
          drawPage(&view,&scene,false);
          scene.setSceneRect(0.0,0.0,pageWidthPx,pageHeightPx);
          scene.render(&painter);
          clearPage(&view,&scene);
          // save the image to the selected directory
          // internationalization of "_page_"?
          QString pn = QString("%1") .arg(displayPageNum);
          image.save( QDir::toNativeSeparators(directoryName + "/" + baseName + "_page_" + pn + suffix));
        }
      m_progressDlgProgressBar->setValue(_maxPages);

    } else {

      QStringList pageRanges = pageRangeText.split(",");
      QList<int> printPages;
      foreach(QString ranges,pageRanges){
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

      _maxPages = printPages.last();

      m_progressDlgProgressBar->setRange(1,printPages.count());

      int _pageCount = 0;
      foreach(int printPage,printPages){

          if (m_cancelPrinting)
            break;

          displayPageNum = printPage;

          logNotice() << QString("Exporting: page range %1 of %2").arg(displayPageNum).arg(_maxPages);

          m_progressDlgMessageLbl->setText(QString("Exporting: page range %1 of %2").arg(displayPageNum).arg(_maxPages));
          m_progressDlgProgressBar->setValue(_pageCount++);

          // clear the pixels of the image, just in case the background is
          // transparent or uses a PNG image with transparency. This will
          // prevent rendered pixels from each page layering on top of each
          // other.
          image.fill(fillClear.Rgb);
          // render this page
          // scene.render instead of view.render resolves "warm up" issue
          drawPage(&view,&scene,false);
          scene.setSceneRect(0.0,0.0,pageWidthPx,pageHeightPx);
          scene.render(&painter);
          clearPage(&view,&scene);
          // save the image to the selected directory
          // internationalization of "_page_"?
          QString pn = QString("%1") .arg(displayPageNum);
          image.save( QDir::toNativeSeparators(directoryName + "/" + baseName + "_page_" + pn + suffix));
        }
       m_progressDlgProgressBar->setValue(printPages.count());
    }

  painter.end();

  // release 3D Viewer
  halt3DViewer(false);

  // return to whatever page we were viewing before output
  displayPageNum = savePageNumber;
  drawPage(KpageView,KpageScene,false);

  // hide progress bar
  m_progressDialog->hide();

  if (!m_cancelPrinting) {

      emit messageSig(true,QString("Export as %1 completed.").arg(suffix.remove(".")));

  //display completion message
  QMessageBox::StandardButton ret;
  ret = QMessageBox::information(this, tr(VER_PRODUCTNAME_STR),
                                 tr("Your instruction document has finished printing.\n"
                                    "Do you want to open the image folder ?\n %1").arg(directoryName),
                                 QMessageBox::Yes| QMessageBox::Discard | QMessageBox::Cancel);

  if (ret == QMessageBox::Yes) {
      QString CommandPath = directoryName;
      QProcess *Process = new QProcess(this);
      Process->setWorkingDirectory(QDir::currentPath() + "/");
      Process->setNativeArguments(CommandPath);

#ifdef __APPLE__

      Process->execute(CommandPath);
      Process->waitForFinished();

      QProcess::ExitStatus Status = Process->exitStatus();

      if (Status != 0) {  // look for error
          QErrorMessage *m = new QErrorMessage(this);
          m->showMessage(QString("%1\n%2").arg("Failed to open image folder!").arg(CommandPath));
        }
#else
      QDesktopServices::openUrl((QUrl("file:///"+CommandPath, QUrl::TolerantMode)));
#endif

      return;
    } else if (ret == QMessageBox::Cancel) {
      return;
    }
} else {
  emit messageSig(true,QString("Export as %1 cancelled.").arg(suffix.remove(".")));
}
}

