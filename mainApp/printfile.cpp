/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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

#include "lpub.h"
#include "messageboxresizable.h"

// Set to enable print/export trace logging
#ifndef PRINT_DEBUG
//#define PRINT_DEBUG
#endif

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


  QMap<int,PgSizeData>::iterator i = pageSizes.find(pageNum);
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

#ifdef PRINT_DEBUG
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
  QMap<int,PgSizeData>::iterator i = pageSizes.find(displayPageNum);   // this page
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
#ifdef PRINT_DEBUG
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

  QMap<int,PgSizeData>::iterator i = pageSizes.find(pageNum);   // this page
  if (i != pageSizes.end()){

#ifdef PRINT_DEBUG
      bool  ls = i.value().orientation == Landscape;
      logDebug() << QString("PAGE %2 ORIENTATION - %1 CurPage: %3")
                    .arg(ls ? "Landscape":"Portrait")
                    .arg(pageNum).arg(displayPageNum);
#endif
      return i.value().orientation;
    }
  emit messageSig(LOG_ERROR,tr("Could not load page %1 orientaiton parameter.").arg(pageNum));
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
              "of the underlying LPub3D development platform. You can export\n"
              "a mixed size and orientation document using the Export pdf\n"
              "or image (PNG, JPG, BMP) functions.\n");

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

      if (printPages.size() > 1){

          bool defaultSizesUpdated = false;

          foreach(key,printPages) {

              QMap<int,PgSizeData>::iterator i = pageSizes.find(key);   // this page
              if (i != pageSizes.end()){
#ifdef PRINT_DEBUG
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
                      title = "<b> Mixed page orientation detected. </b>";
                      box.setIcon (QMessageBox::Information);
                    }

                  if ((pageSizes[key].sizeID != sizeID      ||
                       pageSizes[key].sizeW  != pageWidthIn ||
                       pageSizes[key].sizeH  != pageHeightIn) && size_warning != true) {
                      size_warning = true;
                      text = text2;
                      title = "<b> Mixed page size detected. </b>";
                      box.setIcon (QMessageBox::Warning);
                    }

                  if (orientation_warning && size_warning) {
                      double_warning = true;
                      text = text1 + text2;
                      title = "<b> Mixed page size and orientation detected. </b>";
                      box.setIcon (QMessageBox::Warning);
                      break;
                    }
                }
            }
        }

    } else if (processOption == EXPORT_ALL_PAGES) {
#ifdef PRINT_DEBUG
      logDebug() << QString("Default  page         size(Inches) W %1 x H %2, ID %3, orientation %4")
                               .arg(pageSizes[DEF_SIZE].sizeW)
                               .arg(pageSizes[DEF_SIZE].sizeH)
                               .arg(pageSizes[DEF_SIZE].sizeID)
                               .arg(pageSizes[DEF_SIZE].orientation == Landscape ? "Landscape" : "Portrait");
#endif
      foreach(key,pageSizes.keys()){

          if (pageSizes[key].orientation != orientation && orientation_warning != true) {
              orientation_warning = true;
              text = text1;
              title = "<b> Mixed page orientation detected. </b>";
              box.setIcon (QMessageBox::Information);
            }

          if ((pageSizes[key].sizeID != sizeID      ||
               pageSizes[key].sizeW  != pageWidthIn ||
               pageSizes[key].sizeH  != pageHeightIn) && size_warning != true) {
              size_warning = true;
              text = text2;
              title = "<b> Mixed page size detected. </b>";
              box.setIcon (QMessageBox::Warning);
            }

          if (orientation_warning && size_warning) {
              double_warning = true;
              text = text1 + text2;
              title = "<b> Mixed page size and orientation detected. </b>";
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

bool Gui::validatePageRange(){

  QMessageBox box;
  box.setTextFormat (Qt::RichText);
  box.setIcon (QMessageBox::Information);
  box.setStandardButtons (QMessageBox::Close);
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  box.setWindowTitle(tr ("Page Range"));

  QString title;
  QString text;

  if(pageRangeText.isEmpty()){

      title = "<b> Empty page range. </b>";
      text = tr ("You must enter a page range");

      box.setText (title);
      box.setInformativeText (text);

      if (Preferences::modeGUI)
        box.exec();
      else
        emit messageSig(LOG_STATUS,QString("Empty page range. %1").arg(text));

      return false;
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

      title = "<b> Invalid page number(s). </b>";
      text = tr("Invalid page number(s) %1 detected. You must enter valid page number(s).").arg(message);

      box.setText (title);
      box.setInformativeText (text);

      if (Preferences::modeGUI)
        box.exec();
      else
        emit messageSig(LOG_STATUS,QString("Invalid page number(s). %1").arg(text));

      return false;
    }

  return true;
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

bool Gui::exportAsDialog(Mode t)
{
  exportType = t;

  if (Preferences::modeGUI) {
      DialogExportPages *dialog = new DialogExportPages();
      if (dialog->exec() == QDialog::Accepted) {
          if(dialog->allPages()){
              processOption = EXPORT_ALL_PAGES;
            }
          else
          if(dialog->currentPage()){
              processOption = EXPORT_CURRENT_PAGE;
            }
          else
          if(dialog->pageRange()){
              processOption  = EXPORT_PAGE_RANGE;
              pageRangeText = dialog->pageRangeText();
              if (! validatePageRange()){
                  return false;
                }
            }
        } else {
          return false;
        }
      resetCache = dialog->resetCache();

      QSettings Settings;
      if (Preferences::ignoreMixedPageSizesMsg != dialog->ignoreMixedPageSizesMsg())
        {
          Preferences::ignoreMixedPageSizesMsg = dialog->ignoreMixedPageSizesMsg();
          Settings.setValue(QString("%1/%2").arg(DEFAULTS,"IgnoreMixedPageSizesMsg "),Preferences::ignoreMixedPageSizesMsg);
        }
    }

  if(resetCache){
      clearCustomPartCache(true);
      clearAndRedrawPage();
    }

  if (! m_previewDialog){
      if (t == EXPORT_PDF)
        exportAsPdf();
      else
      if (t == EXPORT_PNG)
        exportAsPng();
      else
      if (t == EXPORT_JPG)
        exportAsJpg();
      else
        exportAsBmp();
    }

  return true;
}

void Gui::exportAsPdf()
{

  // store current display page number
  int savePageNumber = displayPageNum;

  // send signal to halt 3DViewer
  setExportingSig(true);

  // determine location for output file
  QFileInfo fileInfo(curFile);
  QString baseName = fileInfo.baseName();
  QString fileName = QDir::currentPath() + "/" + baseName;

  if (Preferences::modeGUI) {
      fileName = QFileDialog::getSaveFileName(
            this,
            tr("Export File Name"),
            QDir::currentPath() + "/" + baseName,
            tr("PDF (*.pdf)"));

      if (fileName == "") {
          // release 3D Viewer
          setExportingSig(false);
          return;
        }
    } else
    if (!saveFileName.isEmpty()) {
        fileName = saveFileName;
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

  // QMessageBox box; // old
  QMessageBoxResizable box;
  box.setTextFormat (Qt::RichText);
  box.setIcon (QMessageBox::Critical);
  box.setStandardButtons (QMessageBox::Retry | QMessageBox::Abort);
  box.setDefaultButton   (QMessageBox::Retry);
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  box.setWindowTitle(tr ("Export pdf"));

  // check if file open by another program
  QFile printFile(fileName);
  if ( ! printFile.open(QFile::ReadWrite)) {

      QString title = "<b> Cannot open file. </b>";
      QString text = tr ("Cannot open %1 for writing:\n%2")
          .arg(fileName)
          .arg(printFile.errorString());

      box.setText (title);
      box.setInformativeText (text);

      if (Preferences::modeGUI && (box.exec() == QMessageBox::Retry)){
          if ( ! printFile.open(QFile::ReadWrite))
            return;
        } else {
          // release 3D Viewer
          setExportingSig(false);
          emit messageSig(LOG_STATUS, QString("Cannot open file. %1").arg(text));
          return;
        }
    }

  // determine size of output pages, in pixels
  float pageWidthPx, pageHeightPx;

  // create a PDF pdfWriter
  QPdfWriter pdfWriter(fileName);

  // instantiate the scene and view
  QGraphicsScene scene;
  LGraphicsView view(&scene);

  // initialize page sizes
  logStatus() << "INITIALIZE PAGE SIZES START ---->>>>";
  displayPageNum = 0;
  drawPage(&view,&scene,true);
  clearPage(&view,&scene);
  displayPageNum = savePageNumber;
  logStatus() << "INITIALIZE PAGE SIZES END  ----<<<<";

  int _displayPageNum = 0;
  int _maxPages       = 0;

  // initialize progress bar dialog
  m_progressDialog->setWindowTitle("Export pdf");
   if (Preferences::modeGUI)
     m_progressDialog->show();

  // reset page indicators
  _displayPageNum = 0;
  _maxPages       = 0;

  m_progressDlgMessageLbl->setText("Exporting instructions to pdf...");

  if (processOption != EXPORT_PAGE_RANGE){

      if(processOption == EXPORT_ALL_PAGES){
          _displayPageNum = 1;
          _maxPages = maxPages;
        }

      if (processOption == EXPORT_CURRENT_PAGE){
          _displayPageNum = displayPageNum;
          _maxPages       = displayPageNum;
        }

      m_progressDlgProgressBar->setRange(1,_maxPages);
      // set displayPageNum so we can send the correct index to retrieve page size data
      displayPageNum = _displayPageNum;
      // set initial page layout
      pdfWriter.setPageLayout(getPageLayout());

      // paint to the pdfWriter the scene we view
      QPainter painter;
      painter.begin(&pdfWriter);

      for (displayPageNum = _displayPageNum; displayPageNum <= _maxPages; displayPageNum++) {

          if (! exporting()) {
              painter.end();
              if (Preferences::modeGUI)
                  m_progressDialog->hide();
              displayPageNum = savePageNumber;
              drawPage(KpageView,KpageScene,false);
              emit messageSig(LOG_STATUS,QString("Export to pdf terminated before completion."));
              return;
            }

          m_progressDlgMessageLbl->setText(QString("Exporting page %1 of %2").arg(displayPageNum).arg(_maxPages));
          m_progressDlgProgressBar->setValue(displayPageNum);
          QApplication::processEvents();

          getExportPageSize(pageWidthPx, pageHeightPx);

          bool  ls = getPageOrientation() == Landscape;
          logNotice() << QString("                  Exporting page %3 of %4, size(pixels) W %1 x H %2, orientation %5")
                        .arg(pageWidthPx)
                        .arg(pageHeightPx)
                        .arg(displayPageNum)
                        .arg(_maxPages)
                        .arg(ls ? "Landscape" : "Portrait");

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

          // render this page
          drawPage(&view,&scene,true);
          scene.setSceneRect(0.0,0.0,pageWidthPx,pageHeightPx);
          scene.render(&painter);
          clearPage(&view,&scene);

          // prepare to render next page
          if(displayPageNum < _maxPages) {
              bool nextPage = true;
              pdfWriter.setPageLayout(getPageLayout(nextPage));
              pdfWriter.newPage();
            }
        }
      painter.end();
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

       m_progressDlgProgressBar->setRange(1,printPages.count());

      int _pageCount = 0;

      // set displayPageNum so we can send the correct index to retrieve page size data
      displayPageNum = printPages.first();
      // set initial page layout
      pdfWriter.setPageLayout(getPageLayout());

      // paint to the pdfWriter the scene we view
      QPainter painter;
      painter.begin(&pdfWriter);

      foreach(int printPage,printPages){

          if (! exporting()) {
              painter.end();
              if (Preferences::modeGUI)
                  m_progressDialog->hide();
              displayPageNum = savePageNumber;
              drawPage(KpageView,KpageScene,false);
              emit messageSig(LOG_STATUS,QString("Export to pdf terminated before completion."));
              return;
            }

          displayPageNum = printPage;

          m_progressDlgMessageLbl->setText(QString("Exporting page %1 of %2 for range %3").arg(displayPageNum).arg(printPages.count()).arg(pageRanges.join(" ")));
          m_progressDlgProgressBar->setValue(_pageCount++);
          QApplication::processEvents();

          // determine size of output image, in pixels
          getExportPageSize(pageWidthPx, pageHeightPx);

          bool  ls = getPageOrientation() == Landscape;
          logNotice() << QString("Exporting page %3 of %6 for range %4, size(in pixels) W %1 x H %2, orientation %5")
                        .arg(pageWidthPx)
                        .arg(pageHeightPx)
                        .arg(displayPageNum)
                        .arg(pageRanges.join(" "))
                        .arg(ls ? "Landscape" : "Portrait")
                        .arg(printPages.count());

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

          // render this page
          drawPage(&view,&scene,true);
          scene.setSceneRect(0.0,0.0,pageWidthPx,pageHeightPx);
          scene.render(&painter);
          clearPage(&view,&scene);

          // prepare to export another page
          if(_pageCount < printPages.count()) {
              displayPageNum = printPages.at(_pageCount);
              pdfWriter.setPageLayout(getPageLayout());
              pdfWriter.newPage();
            }
        }
      painter.end();
      m_progressDlgProgressBar->setValue(printPages.count());
    }

  // release 3D Viewer
  setExportingSig(false);

  // return to whatever page we were viewing before output
  displayPageNum = savePageNumber;
  drawPage(KpageView,KpageScene,false);

  // hide progress bar
  if (Preferences::modeGUI)
      m_progressDialog->hide();

  //display completion message
  QPixmap _icon = QPixmap(":/icons/lpub96.png");
  box.setWindowIcon(QIcon());
  box.setIconPixmap (_icon);
  box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
  box.setDefaultButton   (QMessageBox::Yes);

  QString title = "<b> Export to pdf completed. </b>";
  QString text = tr ("Your instruction document has finished exporting.\n\n"
                     "Do you want to open this document ?\n\n%1").arg(fileName);

  box.setText (title);
  box.setInformativeText (text);

  if (Preferences::modeGUI && (box.exec() == QMessageBox::Yes)) {
      QString CommandPath = fileName;
      QProcess *Process = new QProcess(this);
      Process->setWorkingDirectory(QDir::currentPath() + "/");

#ifdef Q_OS_WIN
      Process->setNativeArguments(CommandPath);
      QDesktopServices::openUrl((QUrl("file:///"+CommandPath, QUrl::TolerantMode)));
#else
      Process->execute(CommandPath);
      Process->waitForFinished();

      QProcess::ExitStatus Status = Process->exitStatus();

      if (Status != 0) {  // look for error
          QErrorMessage *m = new QErrorMessage(this);
          m->showMessage(QString("%1\n%2").arg("Failed to launch PDF document!").arg(CommandPath));
        }
#endif
      return;
    } else {
      emit messageSig(LOG_STATUS, "Export to pdf completed!");
      return;
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
  // store current display page number
  int savePageNumber = displayPageNum;

  // send signal to halt 3DViewer
  setExportingSig(true);

  // determine location to output images
  QFileInfo fileInfo(curFile);
  QString baseName = fileInfo.baseName();
  QString directoryName = QDir::currentPath();

  if (Preferences::modeGUI) {
      directoryName = QFileDialog::getExistingDirectory(
            this,
            tr("Save images to folder"), // needs translation! also, include suffix in here
            QDir::currentPath(),
            QFileDialog::ShowDirsOnly);
      if (directoryName == "") {
          // release 3D Viewer
          setExportingSig(false);
          return;
        }
     } else
    if (!saveDirectoryName.isEmpty()) {
        directoryName = saveDirectoryName;
    }

  QGraphicsScene scene;
  LGraphicsView view(&scene);
  float pageWidthPx, pageHeightPx;
  int _displayPageNum = 0;
  int _maxPages       = 0;

  // initialize page sizes
  displayPageNum = 0;
  drawPage(&view,&scene,true);
  clearPage(&view,&scene);
  displayPageNum = savePageNumber;

  // Support transparency for formats that can handle it, but use white for those that can't.
  //QColor fillClear = (suffix.compare(".png", Qt::CaseInsensitive) == 0) ? Qt::transparent :  Qt::white;
  QColor::Spec fillClear = QColor((suffix.compare(".png", Qt::CaseInsensitive) == 0) ? Qt::transparent :  Qt::white).Rgb;

  // initialize progress bar
  m_progressDialog->setWindowTitle(QString("Export as %1").arg(suffix));
  if (Preferences::modeGUI)
      m_progressDialog->show();
  m_progressDlgMessageLbl->setText(QString("Exporting instructions to %1 format.").arg(suffix));

  if (processOption != EXPORT_PAGE_RANGE){

      if(processOption == EXPORT_ALL_PAGES){
          _displayPageNum = 1;
          _maxPages = maxPages;
        }

      if (processOption == EXPORT_CURRENT_PAGE){
          _displayPageNum = displayPageNum;
          _maxPages       = displayPageNum;
        }

      m_progressDlgProgressBar->setRange(1,_maxPages);

      for (displayPageNum = _displayPageNum; displayPageNum <= _maxPages; displayPageNum++) {

          if (! exporting()) {
              if (Preferences::modeGUI)
                  m_progressDialog->hide();
              displayPageNum = savePageNumber;
              drawPage(KpageView,KpageScene,false);
              emit messageSig(LOG_STATUS,QString("Export terminated before completion."));
              return;
            }

          m_progressDlgMessageLbl->setText(QString("Exporting image: %1 of %2").arg(displayPageNum).arg(_maxPages));
          m_progressDlgProgressBar->setValue(displayPageNum);
          QApplication::processEvents();

          // determine size of output image, in pixels
          getExportPageSize(pageWidthPx, pageHeightPx);

          bool  ls = getPageOrientation() == Landscape;
          logNotice() << QString("Exporting image %3 of %4, size(in pixels) W %1 x H %2, orientation %5")
                         .arg(pageWidthPx)
                         .arg(pageHeightPx)
                         .arg(displayPageNum)
                         .arg(_maxPages)
                         .arg(ls ? "Landscape" : "Portrait");

          // paint to the image the scene we view
          QImage image(pageWidthPx, pageHeightPx, QImage::Format_ARGB32);
          QPainter painter;
          painter.begin(&image);

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
          clearPage(&view,&scene);

          // clear the pixels of the image, just in case the background is
          // transparent or uses a PNG image with transparency. This will
          // prevent rendered pixels from each page layering on top of each
          // other.
          //image.fill(fillClear.Rgb);
          image.fill(fillClear);
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

          painter.end();
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

      m_progressDlgProgressBar->setRange(1,printPages.count());

      int _pageCount = 0;

      foreach(int printPage,printPages){

          if (! exporting()) {
              if (Preferences::modeGUI)
                  m_progressDialog->hide();
              displayPageNum = savePageNumber;
              drawPage(KpageView,KpageScene,false);
              emit messageSig(LOG_STATUS,QString("Export terminated before completion."));
              return;
            }

          displayPageNum = printPage;

          m_progressDlgMessageLbl->setText(QString("Exporting image %1 of range %2").arg(displayPageNum).arg(pageRanges.join(" ")));
          m_progressDlgProgressBar->setValue(_pageCount++);
          QApplication::processEvents();

          // determine size of output image, in pixels
          getExportPageSize(pageWidthPx, pageHeightPx);

          bool  ls = getPageOrientation() == Landscape;
          logNotice() << QString("Exporting image %3 of range %4, size(in pixels) W %1 x H %2, orientation %5")
                         .arg(pageWidthPx)
                         .arg(pageHeightPx)
                         .arg(displayPageNum)
                         .arg(pageRanges.join(" "))
                         .arg(ls ? "Landscape" : "Portrait");

          // paint to the image the scene we view
          QImage image(pageWidthPx, pageHeightPx, QImage::Format_ARGB32);
          QPainter painter;
          painter.begin(&image);

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
          clearPage(&view,&scene);
          // clear the pixels of the image, just in case the background is
          // transparent or uses a PNG image with transparency. This will
          // prevent rendered pixels from each page layering on top of each
          // other.
          //image.fill(fillClear.Rgb);
          image.fill(fillClear);
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

          painter.end();
        }
      m_progressDlgProgressBar->setValue(printPages.count());
    }

  // release 3D Viewer
  setExportingSig(false);

  // return to whatever page we were viewing before output
  displayPageNum = savePageNumber;
  drawPage(KpageView,KpageScene,false);

  // hide progress bar
  if (Preferences::modeGUI)
      m_progressDialog->hide();

  //display completion message
  QPixmap _icon = QPixmap(":/icons/lpub96.png");
  QMessageBoxResizable box;
  box.setWindowIcon(QIcon());
  box.setIconPixmap (_icon);
  box.setTextFormat (Qt::RichText);
  box.setStandardButtons (QMessageBox::Yes| QMessageBox::No);
  box.setDefaultButton   (QMessageBox::Yes);
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  box.setWindowTitle(tr ("Export %1").arg(suffix.remove(".")));

  QString title = "<b> Export " + suffix.remove(".") + " completed. </b>";
  QString text = tr ("Your instruction document images have finished exportng.\n\n"
                     "Do you want to open the image folder ?\n\n%1")
                     .arg(directoryName);

  box.setText (title);
  box.setInformativeText (text);

  if (Preferences::modeGUI && (box.exec() == QMessageBox::Yes)){
      QString CommandPath = directoryName;
      QProcess *Process = new QProcess(this);
      Process->setWorkingDirectory(QDir::currentPath() + "/");

#ifdef Q_OS_WIN
      Process->setNativeArguments(CommandPath);
      QDesktopServices::openUrl((QUrl("file:///"+CommandPath, QUrl::TolerantMode)));
#else
      Process->execute(CommandPath);
      Process->waitForFinished();

      QProcess::ExitStatus Status = Process->exitStatus();

      if (Status != 0) {  // look for error
          QErrorMessage *m = new QErrorMessage(this);
          m->showMessage(QString("%1\n%2").arg("Failed to open image folder!").arg(CommandPath));
        }
#endif
      return;
    } else {
      emit messageSig(LOG_STATUS, "Export " + suffix.remove(".") + " completed!");
      emit messageSig(LOG_STATUS, QString("Export " + suffix.remove(".") + " images path: %1").arg(directoryName));
      return;
    }
}

//-----------------PRINT FUNCTIONS------------------------//

void Gui::PrintPdf(QPrinter* Printer)
{
  bool preview = false /* m_previewDialog */;  //Hack
  emit hidePreviewDialogSig();

  /*
     * Options:
     *
     */
  int DocCopies;
  int PageCopies;

  int PageCount = maxPages;
  int savePageNumber = displayPageNum;

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
      FromPage = 1;
      ToPage = PageCount;
    }

  FromPage = qMax(1, FromPage);
  ToPage = qMin(PageCount, ToPage);

  if (ToPage < FromPage)
    return;

  if (Printer->pageOrder() == QPrinter::LastPageFirst)
    {
      int Tmp = FromPage;
      FromPage = ToPage;
      ToPage = Tmp;
      Ascending = false;
    }

  // send signal to halt 3DViewer
  setExportingSig(true);

  QGraphicsScene scene;
  LGraphicsView view(&scene);

  // initialize page sizes
  displayPageNum = 0;
  drawPage(&view,&scene,true);
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
  m_progressDialog->setWindowTitle(preview ? "Preview pdf" : "Preview pdf" /* Export pdf */);  //Hack
  if (Preferences::modeGUI)
      m_progressDialog->show();
  m_progressDlgMessageLbl->setText(preview ? "Generating preview..." : "Exporting...");

  for (int DocCopy = 0; DocCopy < DocCopies; DocCopy++)
    {
      int Page = FromPage;

      m_progressDlgProgressBar->setRange(Page,ToPage);

      if (processOption != EXPORT_PAGE_RANGE){

          if (processOption == EXPORT_CURRENT_PAGE){
              // reset display page from saved page
              displayPageNum = savePageNumber;
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
                  displayPageNum = savePageNumber;
                  drawPage(KpageView,KpageScene,false);
                  emit messageSig(LOG_STATUS,QString("Exporting terminated before completion."));
                  if (preview){
                      m_previewDialog = false;
                    }
                  return;
                }

              m_progressDlgMessageLbl->setText(QString("%3 page %1 of %2").arg(Page).arg(ToPage)
                                               .arg(preview ? "Preview" : "Exporting"));
              m_progressDlgProgressBar->setValue(Page);
              QApplication::processEvents();

              // determine size of output image, in pixels. dimension are inches * pixels per inch
              float pageWidthPx, pageHeightPx;
              getExportPageSize(pageWidthPx, pageHeightPx);

              bool  ls = getPageOrientation() == Landscape;
              logNotice() << QString("%6 page %3 of %4, size(in pixels) W %1 x H %2, orientation %5")
                                       .arg(pageWidthPx)
                                       .arg(pageHeightPx)
                                       .arg(displayPageNum)
                                       .arg(ToPage)
                                       .arg(ls ? "Landscape" : "Portrait")
                                       .arg(preview ? "Preview" : "Exporting");

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

              for (int PageCopy = 0; PageCopy < PageCopies; PageCopy++)
                {
                  if (Printer->printerState() == QPrinter::Aborted || Printer->printerState() == QPrinter::Error || ! exporting())
                    {
                      if (Preferences::modeGUI)
                          m_progressDialog->hide();
                      displayPageNum = savePageNumber;
                      drawPage(KpageView,KpageScene,false);
                      emit messageSig(LOG_STATUS,QString("%1 terminated before completion.")
                                      .arg(preview ? "Preview" : "Exporting"));
                      if (preview){
                          m_previewDialog = false;
                        }
                      return;
                    }

                  // render this page
                  drawPage(&view,&scene,true);
                  scene.setSceneRect(0.0,0.0,pageWidthPx,pageHeightPx);
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

          ToPage = printPages.last();

          m_progressDlgProgressBar->setRange(1,printPages.count());

          int _pageCount = 0;

          foreach(int printPage,printPages){

              if (Printer->printerState() == QPrinter::Aborted || Printer->printerState() == QPrinter::Error || ! exporting())
                {
                  if (Preferences::modeGUI)
                      m_progressDialog->hide();
                  displayPageNum = savePageNumber;
                  drawPage(KpageView,KpageScene,false);
                  emit messageSig(LOG_STATUS,QString("Exporting terminated before completion."));
                  if (preview){
                      m_previewDialog = false;
                    }
                  return;
                }

              displayPageNum = Page = printPage;

              m_progressDlgMessageLbl->setText(QString("%1 page %2 of %3 for range %4 ")
                                               .arg(preview ? "Preview" : "Exporting")
                                               .arg(Page)
                                               .arg(printPages.count())
                                               .arg(pageRanges.join(" ")));
              m_progressDlgProgressBar->setValue(_pageCount++);
              QApplication::processEvents();

              // determine size of output image, in pixels. dimension are inches * pixels per inch
              float pageWidthPx, pageHeightPx;
              getExportPageSize(pageWidthPx, pageHeightPx);

              bool  ls = getPageOrientation() == Landscape;
              logNotice() << QString("%6 page %3 of %4, size(pixels) W %1 x H %2, orientation %5 for range %7")
                                       .arg(pageWidthPx)
                                       .arg(pageHeightPx)
                                       .arg(Page)
                                       .arg(printPages.count())
                                       .arg(ls ? "Landscape" : "Portrait")
                                       .arg(preview ? "Preview" : "Exporting")
                                       .arg(pageRanges.join(" "));

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

              for (int PageCopy = 0; PageCopy < PageCopies; PageCopy++)
                {
                  if (Printer->printerState() == QPrinter::Aborted || Printer->printerState() == QPrinter::Error || ! exporting())
                    {
                      if (Preferences::modeGUI)
                          m_progressDialog->hide();
                      displayPageNum = savePageNumber;
                      drawPage(KpageView,KpageScene,false);
                      emit messageSig(LOG_STATUS,QString("%1 terminated before completion.")
                                      .arg(preview ? "Preview" : "Exporting"));
                      if (preview){
                          m_previewDialog = false;
                        }
                      return;
                    }

                  // render this page
                  drawPage(&view,&scene,true);
                  scene.setSceneRect(0.0,0.0,pageWidthPx,pageHeightPx);
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
      m_progressDlgProgressBar->setValue(ToPage);

      if (DocCopy < DocCopies - 1) {
          Printer->newPage();
        }
    }

  // release 3D Viewer
  setExportingSig(false);

  // return to whatever page we were viewing before output
  displayPageNum = savePageNumber;
  drawPage(KpageView,KpageScene,false);

  // hide progress bar
  if (Preferences::modeGUI)
      m_progressDialog->hide();

  emit messageSig(LOG_STATUS,QString("%1 completed.").arg(preview ? "Preview" : "Export to pdf"));

  if (preview){
      m_previewDialog = false;
    }
}

void Gui::showPrintedFile(){

  if (! pdfPrintedFile.isEmpty()){

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

      QString title = "<b> Export to pdf completed. </b>";
      QString text = tr ("Your instruction document has finished exporting.\n\n"
                         "Do you want to open this document ?\n\n%1").arg(pdfPrintedFile);

      box.setText (title);
      box.setInformativeText (text);

      if (Preferences::modeGUI && (box.exec() == QMessageBox::Yes)) {
          QString CommandPath = pdfPrintedFile;
          QProcess *Process = new QProcess(this);
          Process->setWorkingDirectory(QDir::currentPath() + "/");
#ifdef Q_OS_WIN
          Process->setNativeArguments(CommandPath);
          QDesktopServices::openUrl((QUrl("file:///"+CommandPath, QUrl::TolerantMode)));
#else
          Process->execute(CommandPath);
          Process->waitForFinished();

          QProcess::ExitStatus Status = Process->exitStatus();

          if (Status != 0) {  // look for error
              QErrorMessage *m = new QErrorMessage(this);
              m->showMessage(QString("%1\n%2").arg("Failed to launch PDF document!").arg(CommandPath));
            }
#endif
          return;
        } else {
          emit messageSig(LOG_STATUS, "Export to pdf completed!");
          return;

        }
    }
}

void Gui::ShowPrintDialog()
{
    int PageCount = maxPages;

    QPrinter Printer(QPrinter::HighResolution);
    Printer.setFromTo(1, PageCount + 1);

    QPrintDialog PrintDialog(&Printer, this);

    if (PrintDialog.exec() == QDialog::Accepted)
        PrintPdf(&Printer);
}

void Gui::TogglePrintPreview()
{
  m_previewDialog = true;

  if (! exportAsDialog(EXPORT_PDF)){
      m_previewDialog = false;
      return;
    }

  int PageCount = maxPages;

  QPrinter Printer(QPrinter::ScreenResolution);

  //set page parameters
  Printer.setFromTo(1, PageCount + 1);

  // determine location for output file
  QFileInfo fileInfo(curFile);
  QString baseName = fileInfo.baseName();
  QString fileName = QDir::currentPath() + "/" + baseName;
  fileInfo.setFile(fileName);
  QString suffix = fileInfo.suffix();
  if (suffix == "") {
      fileName += ".pdf";
    } else if (suffix != ".pdf" && suffix != ".PDF") {
      fileName = fileInfo.path() + "/" + fileInfo.completeBaseName() + ".pdf";
    }

  pdfPrintedFile = fileName;
  Printer.setOutputFileName(pdfPrintedFile);

  QPrintPreviewDialog Preview(&Printer, this);

  connect(&Preview, SIGNAL(paintRequested(QPrinter*)),          SLOT(PrintPdf(QPrinter*)));
  connect(this,     SIGNAL(hidePreviewDialogSig()),   &Preview, SLOT(hide()));

  int rc = Preview.exec();

  logStatus() << "Pdf export preview result is" << (rc == 1 ? pdfPrintedFile + " exported" : "preview only");  // 0=preview only, 1=export output

  if (rc == 1) {
      showPrintedFile();
    }

  m_previewDialog = false;
}






