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

#include <QPrintDialog>
#include <QPrintPreviewDialog>

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

// Compare two variants.
bool lessThan(const int &v1, const int &v2)
{
    return v1 < v2;
}

QPageLayout Gui::getPageLayout(){
    // determine page orientation
    bool  ls = page.meta.LPub.page.orientation.value() == Landscape;
    QPageLayout::Orientation layoutOrientation = ls ? QPageLayout::Landscape : QPageLayout::Portrait;
    // determine the page size in inches. only concerned with inches because resolution() reports DPI
    qreal pageWidthIn, pageHeightIn;
    if (page.meta.LPub.resolution.type() == DPI) {
        pageWidthIn = page.meta.LPub.page.size.value(0);
        pageHeightIn = page.meta.LPub.page.size.value(1);
    } else {
        pageWidthIn = centimeters2inches(page.meta.LPub.page.size.value(0));
        pageHeightIn = centimeters2inches(page.meta.LPub.page.size.value(1));
    }
    // switch because we always want to send portrait width x height to the printer
    if(ls){qreal temp = pageHeightIn; pageHeightIn = pageWidthIn; pageWidthIn = temp;}
    QPageSize pageSize(QSizeF(pageWidthIn,pageHeightIn),QPageSize::Inch,"",QPageSize::FuzzyMatch);

    return QPageLayout(pageSize, layoutOrientation, QMarginsF(0,0,0,0));
}

QPageLayout Gui::retrievePageLayout(){
    QMap<int,QPageLayout>::iterator i = pageLayouts.find(displayPageNum +1); // next page
    if (i != pageLayouts.end()){
//        bool ls = i.value().orientation() == QPageLayout::Landscape ;
//        logDebug() << QString("Returning itemNo %3 Page layout Orientation [%1] for page %2")
//                      .arg(ls ? "Landscape" : "Portrait")
//                      .arg(displayPageNum + 1)
//                      .arg(displayPageNum + 1);
        return i.value();
    } else {
        emit messageSig(false,tr("Could not find page %1 layout parameters.").arg(displayPageNum + 1));
    }
    return  QPageLayout();
}

FloatPairMeta Gui::retrievePageSize(){
    QMap<int,FloatPairMeta>::iterator i = pageSizes.find(displayPageNum);   // same page
    if (i != pageSizes.end()){
//        logDebug() << QString("Returning itemNo %4: Page size WidthPx %1 x HeigthPx %2 for page %3")
//                      .arg(QString::number(i.value().value(0),'f',4),
//                           QString::number(i.value().value(1),'f',4))
//                      .arg(displayPageNum)
//                      .arg(displayPageNum);
        return i.value();
    } else {
        emit messageSig(false,tr("Could not find page %1 size parameters.").arg(displayPageNum));
    }
    return FloatPairMeta();
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

//  logDebug() << QString("PAGE SIZE:  WidthIn %3 x HeightIn %4 << pageWidthPx %1px X pageHeightPx %2px %5")
//                .arg(QString::number(pixelWidth,'f',4),
//                     QString::number(pixelHeight,'f',4),
//                     QString::number(pageWidthIn,'f',4),
//                     QString::number(pageHeightIn,'f',4))
//                .arg(page.meta.LPub.resolution.type() == DPI ? "Default dots = DPI":"Default dots = DPCM");

}

bool Gui::validatePageRange(){

  QMessageBox box;
  box.setTextFormat (Qt::RichText);
  box.setIcon (QMessageBox::Information);
  box.setStandardButtons (QMessageBox::Close);
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  box.setWindowTitle(tr ("Print pdf"));

  QString title;
  QString text;

  if(pageRangeText.isEmpty()){

      title = "<b> Empty page range. </b>";
      text = tr ("Custom page range is empty. You must enter a page range");

      box.setText (title);
      box.setInformativeText (text);

      box.exec();

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

      box.exec();

      return false;
    }

  return true;
}

bool Gui::printToPdfFileDialog()
{
  exportType = EXPORT_PDF;
  DialogExportPages *dialog = new DialogExportPages();
  if (dialog->exec() == QDialog::Accepted) {
      if(dialog->allPages()){
          exportOption = EXPORT_ALL_PAGES;
        } else
      if(dialog->currentPage()){
          exportOption = EXPORT_CURRENT_PAGE;
        } else
      if(dialog->pageRange()){
          exportOption  = EXPORT_PAGE_RANGE;
          pageRangeText = dialog->pageRangeText();
          if (! validatePageRange()){
              return false;
            }
        }
      if(dialog->mixedPageSize()){
          mixedPageSize = dialog->mixedPageSize();
      }
      if(dialog->resetCache()){
          clearPLICache();
          clearCSICache();
          //TODO add remove ldraw viewer content when move to 2.1
        }
      if (! m_previewDialog){
          printToPdfFile();
        }
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
        } else
      if(dialog->currentPage()){
          exportOption = EXPORT_CURRENT_PAGE;
        } else
      if(dialog->pageRange()){
          exportOption  = EXPORT_PAGE_RANGE;
          pageRangeText = dialog->pageRangeText();
          if (! validatePageRange()){
              return false;
            }
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
        } else
      if(dialog->currentPage()){
          exportOption = EXPORT_CURRENT_PAGE;
        } else
      if(dialog->pageRange()){
          exportOption  = EXPORT_PAGE_RANGE;
          pageRangeText = dialog->pageRangeText();
          if (! validatePageRange()){
              return false;
            }
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
        } else
      if(dialog->currentPage()){
          exportOption = EXPORT_CURRENT_PAGE;
        } else
      if(dialog->pageRange()){
          exportOption  = EXPORT_PAGE_RANGE;
          pageRangeText = dialog->pageRangeText();
          if (! validatePageRange()){
              return false;
            }
        }
      exportAsBmp();
      return true;
    } else {
      return false;
    }
}

void Gui::printToPdfFile()
{
  // store current display page number
  int savePageNumber = displayPageNum;

  // send signal to halt 3DViewer
  setExportingSig(true);

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
      setExportingSig(false);
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
  float pageWidthPx, pageHeightPx;

  // create a PDF pdfWriter
  QPdfWriter pdfWriter(fileName);
  pdfWriter.setPageLayout(getPageLayout());

  // instantiate the scene and view
  QGraphicsScene scene;
  LGraphicsView view(&scene);

  int _displayPageNum = 0;
  int _maxPages       = 0;

  // initialize progress bar dialog
  m_progressDialog->setWindowTitle("Print pdf");
  m_progressDialog->show();

  if (mixedPageSize) {
      logStatus() << "Processing mixed page size or orientation parameters...";

      m_progressDlgMessageLbl->setText("Capturing page layouts...");

      // capture all pageLayout
      _displayPageNum = 1;
      _maxPages       = maxPages;
      m_progressDlgProgressBar->setRange(1,_maxPages);

      for (displayPageNum = _displayPageNum; displayPageNum <= _maxPages; displayPageNum++) {
          if (! exporting()) {
              m_progressDialog->hide();
              displayPageNum = savePageNumber;
              drawPage(KpageView,KpageScene,false);
              emit messageSig(true,QString("Print to pdf terminated before completion."));
              return;
            }
          // render this page - to capture display page layout and size
          drawPage(&view,&scene,true);
          pageLayouts.insert(displayPageNum, getPageLayout());
          bool ls = getPageLayout().orientation() == QPageLayout::Landscape;
          logStatus() << QString("Inserting itemNo %3 Page layout Orientation [%1] for page %2")
                        .arg(ls ? "Landscape" : "Portrait")
                        .arg(displayPageNum)
                        .arg(displayPageNum);

          // determine size of output image, in pixels. dimension are inches * pixels per inch
          GetPixelDimensions(pageWidthPx, pageHeightPx);
          FloatPairMeta pgSizeInPx;
          pgSizeInPx.setValues(pageWidthPx, pageHeightPx);
          pageSizes.insert(displayPageNum, pgSizeInPx);
          logStatus() << QString("Inserting itemNo %4: Page size WidthPx %1 x HeigthPx %2 for page %3")
                        .arg(QString::number(pgSizeInPx.value(0),'f',4),
                             QString::number(pgSizeInPx.value(1),'f',4))
                        .arg(displayPageNum)
                        .arg(displayPageNum);

          m_progressDlgMessageLbl->setText(QString("Capturing page layout %1 of %2").arg(displayPageNum).arg(_maxPages));
          m_progressDlgProgressBar->setValue(displayPageNum);
      }
      clearPage(&view,&scene);
      m_progressDlgProgressBar->setValue(_maxPages);

      m_progressDlgProgressBar->reset();

  } else {

      // determine size of output image, in pixels (global setting)
      GetPixelDimensions(pageWidthPx, pageHeightPx);

      // set page layout (global setting)
      pdfWriter.setPageLayout(getPageLayout());
  }

  // paint to the pdfWriter the scene we view
  QPainter painter;
  painter.begin(&pdfWriter);

  // reset page indicators
  _displayPageNum = 0;
  _maxPages       = 0;

  m_progressDlgMessageLbl->setText("Printing instructions to pdf...");

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

          if (! exporting()) {
              painter.end();
              m_progressDialog->hide();
              displayPageNum = savePageNumber;
              drawPage(KpageView,KpageScene,false);
              emit messageSig(true,QString("Print to pdf terminated before completion."));
              return;
            }

          m_progressDlgMessageLbl->setText(QString("Printing page %1 of %2").arg(displayPageNum).arg(_maxPages));
          m_progressDlgProgressBar->setValue(displayPageNum);

          if (mixedPageSize) {
              // determine size of output image, in pixels
              pageWidthPx  = retrievePageSize().value(0);
              pageHeightPx = retrievePageSize().value(1);
          }

          bool  ls = page.meta.LPub.page.orientation.value() == Landscape;
          logNotice() << QString("Exporting page %3 of %4, size(in pixels) W %1 x H %2, orientation %5")
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

          // render this page
          drawPage(&view,&scene,true);
          scene.setSceneRect(0.0,0.0,pageWidthPx,pageHeightPx);
          scene.render(&painter);
          clearPage(&view,&scene);

          // prepare to render next page
          if(displayPageNum < _maxPages) {
              if (mixedPageSize)
                  pdfWriter.setPageLayout(retrievePageLayout());
              pdfWriter.newPage();
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

       m_progressDlgProgressBar->setRange(1,printPages.count());

      int _pageCount = 0;
      foreach(int printPage,printPages){

          if (! exporting()) {
              painter.end();
              m_progressDialog->hide();
              displayPageNum = savePageNumber;
              drawPage(KpageView,KpageScene,false);
              emit messageSig(true,QString("Print to pdf terminated before completion."));
              return;
            }

          displayPageNum = printPage;

          m_progressDlgMessageLbl->setText(QString("Printing page %1 of range %2").arg(displayPageNum).arg(pageRanges.join(" ")));
          m_progressDlgProgressBar->setValue(_pageCount++);

          if (mixedPageSize) {
              // determine size of output image, in pixels
              pageWidthPx  = retrievePageSize().value(0);
              pageHeightPx = retrievePageSize().value(1);
          }

          bool  ls = page.meta.LPub.page.orientation.value() == Landscape;
          logNotice() << QString("Printing page %3 of range %4, size(in pixels) W %1 x H %2, orientation %5")
                        .arg(pageWidthPx)
                        .arg(pageHeightPx)
                        .arg(displayPageNum)
                        .arg(pageRanges.join(" "))
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

          // prepare to print another page
          if(displayPageNum < _maxPages) {
              if (mixedPageSize)
                  pdfWriter.setPageLayout(retrievePageLayout());
              pdfWriter.newPage();
            }
        }
      m_progressDlgProgressBar->setValue(printPages.count());
    }

  painter.end();

  // release 3D Viewer
  setExportingSig(false);

  // return to whatever page we were viewing before output
  displayPageNum = savePageNumber;
  drawPage(KpageView,KpageScene,false);

  // hide progress bar
  m_progressDialog->hide();

  if (!m_cancelPrinting) {

      emit messageSig(true,QString("Print to pdf completed."));

      QMessageBox box;
      box.setTextFormat (Qt::RichText);
      box.setIcon (QMessageBox::Information);
      box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
      box.setDefaultButton   (QMessageBox::Yes);
      box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      box.setWindowTitle(tr ("Print pdf"));

      //display completion message
      QString title = "<b> Print to pdf completed. </b>";
      QString text = tr ("Your instruction document has finished printing.\n"
                        "Do you want to open this document ?\n %1").arg(fileName);

      box.setText (title);
      box.setInformativeText (text);

      if (box.exec() == QMessageBox::Yes) {
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
      } else {
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
  // store current display page number
    int savePageNumber = displayPageNum;

//    if (exportOption == EXPORT_PAGE_RANGE) {
//        if(pageRangeText.isEmpty()){
//            QMessageBox::warning(NULL,
//                                 tr("LPub3D"),
//                                 tr("Custom page range is empty. You must enter a page range"));
//            return;
//        }
//        bool validEntry = true;
//        QString message;
//        QStringList pageRanges = pageRangeText.split(",");
//        foreach(QString ranges, pageRanges){
//            if (ranges.contains("-")){
//                bool ok[2];
//                QStringList range = ranges.split("-");
//                int minPage = range[0].toInt(&ok[0]);
//                int maxPage = range[1].toInt(&ok[1]);
//                if (!ok[0] || !ok[1] || (minPage > maxPage)){
//                    message = QString("%1-%2").arg(minPage).arg(maxPage);
//                    validEntry = false;
//                    break;
//                }
//            } else {
//                bool ok;
//                int pageNum = ranges.toInt(&ok);
//                if (!ok){
//                    message = QString("%1").arg(pageNum);
//                    validEntry = false;
//                    break;
//                }
//            }
//        }
//        if (! validEntry) {
//            QMessageBox::warning(NULL,
//                                 tr("LPub3D"),
//                                 tr("Invalid page number(s) %1 detected. You must enter valid page number(s).").arg(message));
//            return;
//        }
//    }

    // send signal to halt 3DViewer
    setExportingSig(true);

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
        setExportingSig(false);
        return;
    }

    QGraphicsScene scene;
    LGraphicsView view(&scene);
    float pageWidthPx, pageHeightPx;
    int _displayPageNum = 0;
    int _maxPages       = 0;

    // Support transparency for formats that can handle it, but use white for those that can't.
    QColor fillClear = (suffix.compare(".png", Qt::CaseInsensitive) == 0) ? Qt::transparent :  Qt::white;

    // initialize progress bar
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

            if (! exporting()) {
                m_progressDialog->hide();
                displayPageNum = savePageNumber;
                drawPage(KpageView,KpageScene,false);
                emit messageSig(true,QString("Export terminated before completion."));
                return;
              }

            m_progressDlgMessageLbl->setText(QString("Exporting page: %1 of %2").arg(displayPageNum).arg(_maxPages));
            m_progressDlgProgressBar->setValue(displayPageNum);

            // render scene to capture page size
            drawPage(&view,&scene,false);

            // determine size of output image, in pixels
            GetPixelDimensions(pageWidthPx, pageHeightPx);

            bool  ls = page.meta.LPub.page.orientation.value() == Landscape;
            logNotice() << QString("Exporting page %3 of %4, size(in pixels) W %1 x H %2, orientation %5")
                           .arg(pageWidthPx)
                           .arg(pageHeightPx)
                           .arg(displayPageNum)
                           .arg(_maxPages)
                           .arg(ls ? "Landscape" : "Portrait");

            // paint to the image the scene we view
            QImage image(pageWidthPx, pageHeightPx, QImage::Format_ARGB32);
            QPainter painter;
            painter.begin(&image);

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

            if (! exporting())
              m_progressDialog->hide();
              displayPageNum = savePageNumber;
              drawPage(KpageView,KpageScene,false);
              emit messageSig(true,QString("Export terminated before completion."));
              return;

            displayPageNum = printPage;

            m_progressDlgMessageLbl->setText(QString("Exporting page %1 of range %2").arg(displayPageNum).arg(pageRanges.join(" ")));
            m_progressDlgProgressBar->setValue(_pageCount++);

            // render scene to capture page size
            drawPage(&view,&scene,false);

            // determine size of output image, in pixels
            GetPixelDimensions(pageWidthPx, pageHeightPx);

            bool  ls = page.meta.LPub.page.orientation.value() == Landscape;
            logNotice() << QString("Exporting page %3 of range %4, size(in pixels) W %1 x H %2, orientation %5")
                           .arg(pageWidthPx)
                           .arg(pageHeightPx)
                           .arg(displayPageNum)
                           .arg(pageRanges.join(" "))
                           .arg(ls ? "Landscape" : "Portrait");

            // paint to the image the scene we view
            QImage image(pageWidthPx, pageHeightPx, QImage::Format_ARGB32);
            QPainter painter;
            painter.begin(&image);

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
    m_progressDialog->hide();

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
}

//-----------------PRINT FUNCTIONS------------------------//

void Gui::Print(QPrinter* Printer)
{
  QObject *previewDialog  = qobject_cast<QPrintPreviewDialog *>(sender());
  bool preview = previewDialog ? false /* true */ : false;  //Hack
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

  // determine get orientation from print preview if requested
  QPageLayout::Orientation previewOrientation = Printer->pageLayout().orientation();

  // determine the page size in inches. only concerned with inches because resolution() reports DPI
  qreal pageWidthIn, pageHeightIn;
  if (page.meta.LPub.resolution.type() == DPI) {
      pageWidthIn = page.meta.LPub.page.size.value(0);
      pageHeightIn = page.meta.LPub.page.size.value(1);
    } else {
      pageWidthIn = centimeters2inches(page.meta.LPub.page.size.value(0));
      pageHeightIn = centimeters2inches(page.meta.LPub.page.size.value(1));
    }
  // switch sizes if landscape because we always want to send portrait width x height to the printer
  bool  ls = page.meta.LPub.page.orientation.value() == Landscape;
  if(ls){qreal temp = pageHeightIn; pageHeightIn = pageWidthIn; pageWidthIn = temp;}

  QPageSize pageSize(QSizeF(pageWidthIn,pageHeightIn),QPageSize::Inch,"",QPageSize::FuzzyOrientationMatch);
  QPageLayout pageLayout(pageSize, previewOrientation, QMarginsF(0,0,0,0),QPageLayout::Inch);
  Printer->setPageLayout(pageLayout);

  Printer->setFullPage(true);

  // paint to the printer the scene we view
  QPainter Painter(Printer);
  QGraphicsScene scene;
  LGraphicsView view(&scene);

  // initialize progress bar dialog
  m_progressDialog->setWindowTitle(preview ? "Preview pdf" : "Preview pdf" /* Print pdf */);  //Hack
  m_progressDialog->show();
  m_progressDlgMessageLbl->setText(preview ? "Generating preview..." : "Printing...");

  for (int DocCopy = 0; DocCopy < DocCopies; DocCopy++)
    {
      int Page = FromPage;

      m_progressDlgProgressBar->setRange(Page,ToPage);

      if (exportOption != EXPORT_PAGE_RANGE){

          if (exportOption == EXPORT_CURRENT_PAGE){
              Page   = displayPageNum;
              ToPage = displayPageNum;
            }

          for (displayPageNum = Page; displayPageNum <= ToPage; displayPageNum++)
            {
              if (Printer->printerState() == QPrinter::Aborted || Printer->printerState() == QPrinter::Error || ! exporting())
                {
                  emit messageSig(true,QString("Printing terminated before completion."));
                  return;
                }

              m_progressDlgMessageLbl->setText(QString("%3 page %1 of %2").arg(Page).arg(ToPage)
                                               .arg(preview ? "Previewing" : "Printing"));
              m_progressDlgProgressBar->setValue(Page);

              logNotice() << QString("%3 page %1 of %2").arg(Page).arg(ToPage)
                             .arg(preview ? "Previewing" : "Printing");

              // render this page to get specific page parameters
              drawPage(&view,&scene,true);

              // determine page orientation
              //bool  ls = page.meta.LPub.page.orientation.value() == Landscape;
              //logDebug() << QString("PAGE ORIENTATION %1").arg(ls? "Landscape":"Portrait");

              // determine size of output image, in pixels. dimension are inches * pixels per inch
              float pageWidthPx, pageHeightPx;
              GetPixelDimensions(pageWidthPx, pageHeightPx);

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
                      emit messageSig(true,QString("%1 terminated before completion.")
                                      .arg(preview ? "Preview" : "Printing"));
                      return;
                    }

                  // render this page
                  drawPage(&view,&scene,true);
                  scene.setSceneRect(0.0,0.0,pageWidthPx,pageHeightPx);
                  scene.render(&Painter);
                  clearPage(&view,&scene);

                  // TODO: print header and footer

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
                  emit messageSig(true,QString("Printing terminated before completion."));
                  return;
                }

              displayPageNum = Page = printPage;

              m_progressDlgMessageLbl->setText(QString("%3 page %1 of range %2 ").arg(Page).arg(pageRanges.join(" "))
                                               .arg(preview ? "Previewing" : "Printing"));
              m_progressDlgProgressBar->setValue(_pageCount++);

              logNotice() << QString("%3 page %1 of %2").arg(Page).arg(ToPage)
                             .arg(preview ? "Previewing" : "Printing");

              // render this page to get specific page parameters
              drawPage(&view,&scene,true);

              // determine page orientation
              //bool  ls = page.meta.LPub.page.orientation.value() == Landscape;
              //logDebug() << QString("PAGE ORIENTATION %1").arg(ls? "Landscape":"Portrait");

              // determine size of output image, in pixels. dimension are inches * pixels per inch
              float pageWidthPx, pageHeightPx;
              GetPixelDimensions(pageWidthPx, pageHeightPx);

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
                      emit messageSig(true,QString("%1 terminated before completion.")
                                      .arg(preview ? "Preview" : "Printing"));
                      return;
                    }

                  // render this page
                  drawPage(&view,&scene,true);
                  scene.setSceneRect(0.0,0.0,pageWidthPx,pageHeightPx);
                  scene.render(&Painter);
                  clearPage(&view,&scene);

                  // TODO: print header and footer

                  if (PageCopy < PageCopies - 1){
                      Printer->newPage();
                    }
                }
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
  m_progressDialog->hide();

  emit messageSig(true,QString("%1 completed.").arg(preview ? "Preview" : "Print to pdf"));

  if (preview){
      // reset
      m_previewDialog = false;
    }
}

void Gui::showPrintedFile(){

  if (! pdfPrintedFile.isEmpty()){

      QMessageBox box;
      box.setTextFormat (Qt::RichText);
      box.setIcon (QMessageBox::Information);
      box.setStandardButtons (QMessageBox::Close);
      box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      box.setWindowTitle(tr ("Print Status"));

      //display completion message
      box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
      box.setDefaultButton   (QMessageBox::Yes);

      QString title = "<b> Print to pdf completed. </b>";
      QString text = tr ("Your instruction document has finished printing.\n"
                         "Do you want to open this document ?\n %1").arg(pdfPrintedFile);

      box.setText (title);
      box.setInformativeText (text);

      if (box.exec() == QMessageBox::Yes) {
          QString CommandPath = pdfPrintedFile;
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
        } else {
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
        Print(&Printer);
}

void Gui::TogglePrintPreview()
{
  m_previewDialog = true;
  if (! printToPdfFileDialog()) {
      logInfo() << "Print to pdf dialog returned false";
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

  connect(&Preview, SIGNAL(paintRequested(QPrinter*)),          SLOT(Print(QPrinter*)));
  connect(this,     SIGNAL(hidePreviewDialogSig()),   &Preview, SLOT(hide()));

  int rc = Preview.exec();

  logStatus() << "Pdf print preview result is" << (rc == 1 ? pdfPrintedFile + " printed" : "preview only");  // 0=preview only, 1=print output

  if (rc == 1) {
      showPrintedFile();
    }

  m_previewDialog = false;
}
