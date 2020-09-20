
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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

#include <QWidget>
#include <QDesktopWidget>
#include <QSizePolicy>
#include <QFileDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QCloseEvent>
#include <QUndoStack>
#include <QTextStream>
#include <QStringList>
#include <JlCompress.h>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QProgressDialog>

#include <ui_progress_dialog.h>

#include "lpub.h"

#include "name.h"
#include "editwindow.h"
#include "parmswindow.h"
#include "paths.h"
#include "globals.h"
#include "resolution.h"
#include "lpubalert.h"
#include "preferencesdialog.h"
#include "povrayrenderdialog.h"
#include "metaitem.h"
#include "ranges_element.h"
#include "updatecheck.h"
#include "step.h"
#include "messageboxresizable.h"

#include "aboutdialog.h"
#include "dialogexportpages.h"
#include "numberitem.h"
#include "progress_dialog.h"

//3D Viewer
#include "camera.h"
#include "project.h"
#include "view.h"
#include "piece.h"
#include "lc_profile.h"
#include "application.h"
#include "lc_partselectionwidget.h"
#include "lc_http.h"

//** LDVLib
#include <TCFoundation/TCUserDefaults.h>
#include <LDLib/LDUserDefaultsKeys.h>
#include <LDVQt/LDVWidget.h>

#if _MSC_VER > 1310
// Visual C++ 2005 and later require the source files in UTF-8, and all strings
// to be encoded as wchar_t otherwise the strings will be converted into the
// local multibyte encoding and cause errors. To use a wchar_t as UTF-8, these
// strings then need to be convert back to UTF-8.
#define wCharToUtf8(str) ConvertWideCharToUTF8(L##str)
const char * ConvertWideCharToUTF8(const wchar_t * wstr) {
    int requiredSize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, 0, 0, 0, 0);
    if (requiredSize > 0) {
        static char * buffer = new char[requiredSize + 1];
        buffer[requiredSize] = 0;
        WideCharToMultiByte(CP_UTF8, 0, wstr, -1, buffer, requiredSize, 0, 0);
        return buffer;
    }
    return nullptr;
}
#else
// Visual C++ 2003 and gcc will use the string literals as is, so the files
// should be saved as UTF-8. gcc requires the files to not have a UTF-8 BOM.
#define wCharToUtf8(str) str
#endif

Gui *gui;

void clearPliCache()
{
  gui->clearPLICache();
}

void clearCsiCache()
{
  gui->clearCSICache();
}

void clearSubmodelCache()
{
  gui->clearSubmodelCache();
}

void clearTempCache()
{
  gui->clearTempCache();
}

QHash<SceneObject, QString> soMap;

/****************************************************************************
 *
 * Download with progress monotor
 *
 ***************************************************************************/
void Gui::downloadFile(QString URL, QString title)
{
    mTitle = title;
    QProgressDialog ProgressDialog(nullptr);
    ProgressDialog.setWindowTitle(tr("Downloading"));
    ProgressDialog.setLabelText(tr("Downloading %1").arg(mTitle));
    ProgressDialog.setMaximum(0);
    ProgressDialog.setMinimum(0);
    ProgressDialog.setValue(0);
    ProgressDialog.show();

    QString DownloadUrl = QString(URL);
    mHttpReply = mHttpManager->DownloadFile(DownloadUrl);
    while (mHttpReply) {
        QApplication::processEvents();
        if (ProgressDialog.wasCanceled()){
            mHttpReply->abort();
            mHttpReply->deleteLater();
            mHttpReply = nullptr;
            return;
        }
    }
}

void Gui::DownloadFinished(lcHttpReply* Reply){
    if (Reply == mHttpReply)
    {
        if (!Reply->error()) {
            mByteArray = Reply->readAll();
        } else {
            QString message = QString("Error downloading %1").arg(mTitle);
            if (Preferences::modeGUI){
                QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),  message);
            } else {
                logError() << message;
            }
        }
        mHttpReply = nullptr;
    }
    Reply->deleteLater();
}

/****************************************************************************
 *
 * The Gui constructor and destructor are at the bottom of the file with
 * the code that creates the basic GUI framework
 *
 ***************************************************************************/

// flip orientation for landscape
int Gui::pageSize(PageMeta &meta, int which){
  int _which;
  if (meta.orientation.value() == Landscape){
      which == 0 ? _which = 1 : _which = 0;
    } else {
      _which = which;
    }
  return meta.size.valuePixels(_which);
}

// Compare two variants - less than.
bool lt(const int &v1, const int &v2){ return v1 < v2; }
// Compare two variants - greater than.
bool gt(const int &v1, const int &v2){ return v1 > v2; }

// Sleeper
class secSleeper : public QThread
{
public:
    static void secSleep(unsigned long Secs){ QThread::sleep(Secs); }
};

void Gui::insertCoverPage()
{
  MetaItem mi;
  mi.insertCoverPage();
}

void Gui::appendCoverPage()
{
  MetaItem mi;
  mi.appendCoverPage();
  countPages();
  displayPageNum = maxPages;
  displayPage();
}

void Gui::generateCoverPages()
{
    if (Preferences::generateCoverPages){
        MetaItem mi;
        if (!mi.frontCoverPageExist())
            mi.insertCoverPage();

        if (!mi.backCoverPageExist())
            mi.appendCoverPage();
    }
}

void Gui::insertFinalModel(){
  MetaItem mi;
  static int modelExist = -1;
  if (Preferences::enableFadeSteps ||
      Preferences::enableHighlightStep){
      int modelStatus = mi.okToInsertFinalModel();
      if (modelStatus != modelExist)
        mi.insertFinalModel(modelStatus);
  } else
  if (! Preferences::enableFadeSteps &&
      ! Preferences::enableHighlightStep){
      if (mi.okToInsertFinalModel() == modelExist)
        mi.deleteFinalModel();
  }
}

//void Gui::insertCoverPage()
//{
//  MetaItem mi;
//  mi.insertCoverPage();
//}

//void Gui::appendCoverPage()
//{
//  MetaItem mi;
//  mi.appendCoverPage();
//  countPages();
//  ++displayPageNum;
//  displayPage();  // display the page we just added
//}

void Gui::insertNumberedPage()
{
  MetaItem mi;
  mi.insertNumberedPage();
}

void Gui::appendNumberedPage()
{
  MetaItem mi;
  mi.appendNumberedPage();
  //countPages();
  //++displayPageNum;
  //displayPage();  // display the page we just added
}

void Gui::deletePage()
{
  MetaItem mi;
  mi.deletePage();
}

void Gui::addPicture()
{
  MetaItem mi;
  mi.insertPicture();
}

void Gui::addText()
{
  MetaItem mi;
  mi.insertText();
}

void Gui::addBom()
{
  MetaItem mi;
  mi.insertBOM();
}

void Gui::removeLPubFormatting()
{
  MetaItem mi;
  mi.removeLPubFormatting();
  displayPageNum = 1;
  displayPage();
}

void Gui::displayPage()
{
  emit messageSig(LOG_STATUS, "Processing page display...");

  timer.start();
  if (macroNesting == 0) {
      clearPage(KpageView,KpageScene);
      page.coverPage = false;
      drawPage(KpageView,KpageScene,false);

      enableActions2();
      emit enable3DActionsSig();
    }
  if (! ContinuousPage())
      emit messageSig(LOG_STATUS,QString("Page loaded%1.")
                  .arg(Preferences::modeGUI ? QString(". %1").arg(gui->elapsedTime(timer.elapsed())) : ""));
}

void Gui::nextPage()
{
  QString string = setPageLineEdit->displayText();
  QRegExp rx("^(\\d+).*$");
  if (string.contains(rx)) {
      bool ok;
      int inputPageNum;
      inputPageNum = rx.cap(1).toInt(&ok);
      if (ok && (inputPageNum != displayPageNum)) {		// numbers are different so jump to page
          countPages();
          if (inputPageNum <= maxPages) {
              if (inputPageNum != displayPageNum) {
                  displayPageNum = inputPageNum;
                  displayPage();
                  return;
                }
            } else {
              statusBarMsg("Page number entered is higher than total pages");
            }
          string = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);
          setPageLineEdit->setText(string);
          return;
        } else {						// numbers are same so goto next page
          countPages();
          if (displayPageNum < maxPages) {
              ++displayPageNum;
              displayPage();
            } else {
              statusBarMsg("You are on the last page");
            }
        }
    }
}

void Gui::nextPageContinuous()
{
  // This effectively acts like a toggle. If true, it sets false and vice versa.
  nextPageContinuousIsRunning = !nextPageContinuousIsRunning;

  QMessageBox box;
  box.setTextFormat (Qt::RichText);
  box.setIcon (QMessageBox::Information);
  box.setWindowTitle(tr ("Next page continuous processing"));
  box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  QString title = "<b>       Next page continuous processing&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; </b>";
  QString message = QString("Next page continuous processing is running.\n");

  // Request to terminate Next page process while it is still running
  if (!nextPageContinuousIsRunning) {
      box.setIcon (QMessageBox::Warning);
      title = "<b> Stop next page continuous processing ?&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; </b>";
      message = QString("Next page continuous processing has not completed.\n"
                        "Do you want to terminate this process ?");
      box.setText (title);
      box.setInformativeText (message);

      if (box.exec() == QMessageBox::Yes)  // Yes, let's terminate
          return;
      else                                 // Oops, do not want to terminate
          setPageContinuousIsRunning(true, PAGE_NEXT);
    }
  // If user clicks Next page process while Previous page process is still running
  else if (previousPageContinuousIsRunning && nextPageContinuousIsRunning) {
      box.setIcon (QMessageBox::Warning);
      title = "<b>       Stop Previous page continuous processing ?&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; </b>";
      message = QString("Previous page continuous processing is running.\n"
                        "Do you want to stop this process and start Next page processing ?");

      box.setText (title);
      box.setInformativeText (message);

    // User wants to stop Previous page process
    if (box.exec() == QMessageBox::Yes){
        setPageContinuousIsRunning(false, PAGE_PREVIOUS);
        continuousPageDialog(PAGE_NEXT);
      }
    // User wants to continue running Previous page process so stop Next
    else {
        setPageContinuousIsRunning(false, PAGE_NEXT);
        return;
      }
  // No conflicts, everything looks good, let's go
  } else {
      continuousPageDialog(PAGE_NEXT);
  }
}

void Gui::previousPage()
{
  QString string = setPageLineEdit->displayText();
  QRegExp rx("^(\\d+).*$");
  if (string.contains(rx)) {
      bool ok;
      int inputPageNum;
      inputPageNum = rx.cap(1).toInt(&ok);
      if (ok && (inputPageNum != displayPageNum)) {		// numbers are different so jump to page
          countPages();
          if (inputPageNum >= 1) {
              if (inputPageNum != displayPageNum) {
                  displayPageNum = inputPageNum;
                  displayPage();
                  return;
                }
            } else {
              statusBarMsg("Page number entered is invalid");
            }
          string = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);
          setPageLineEdit->setText(string);
          return;
        } else {						// numbers are same so goto previous page
          if (displayPageNum > 1) {
              displayPageNum--;
              displayPage();
            } else {
              statusBarMsg("You are on the first page");
            }
        }
    }
}

void Gui::previousPageContinuous()
{
  // This effectively acts like a toggle. If true, it sets false and vice versa.
  previousPageContinuousIsRunning = !previousPageContinuousIsRunning;

  QMessageBox box;
  box.setTextFormat (Qt::RichText);
  box.setIcon (QMessageBox::Information);
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  box.setWindowTitle(tr ("Previous page continuous processing"));
  box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
  QString title = "<b> Previous page continuous processing&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; </b>";
  QString message = QString("Previous page continuous processing is running.\n");

  // Request to terminate Previous page process while it is still running
  if (!previousPageContinuousIsRunning) {
      box.setIcon (QMessageBox::Warning);
      title = "<b> Stop previous page continuous processing ?;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; </b>";
      message = QString("Previous page continuous processing has not completed.\n"
                        "Do you want to terminate this process ?");

      box.setText (title);
      box.setInformativeText (message);

      if (box.exec() == QMessageBox::Yes) // Yes, let's terminate
          return;
      else                                // Oops, do not want to terminate
          setPageContinuousIsRunning(true, PAGE_PREVIOUS);
    }
  // If user clicks Previous page process while Next page process is still running
  else if (nextPageContinuousIsRunning  && previousPageContinuousIsRunning) {
      box.setIcon (QMessageBox::Warning);
      title = "<b> Stop Next page continuous processing ?;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</b>";
      message = QString("Next page continuous processing is running.         \n"
                        "Do you want to terminate this process and start over ?");

      box.setText (title);
      box.setInformativeText (message);

    // User wants to stop Next page process
    if (box.exec() == QMessageBox::Yes){
        setPageContinuousIsRunning(false, PAGE_NEXT);
        continuousPageDialog(PAGE_PREVIOUS);
      }
    // User wants to continue running Next page process so stop Previous
    else {
        setPageContinuousIsRunning(false, PAGE_PREVIOUS);
        return;
      }
    // No conflicts, everything looks good, let's go
    } else {
      continuousPageDialog(PAGE_PREVIOUS);
    }
}

void Gui::setPageContinuousIsRunning(bool b, Direction d){

    if (d != DIRECTION_NOT_SET) pageDirection = d;

    if (pageDirection == PAGE_NEXT) {
        nextPageContinuousIsRunning = b;
    } else {
        previousPageContinuousIsRunning = b;
    }
}

void Gui::setContinuousPageAct(PAction p) {
    if (p == SET_STOP_ACTION) {
        if (pageDirection == PAGE_NEXT) {
            nextPageContinuousAct->setIcon(QIcon(":/resources/nextpagecontinuousstop.png"));
            nextPageContinuousAct->setStatusTip("Stop continuous next page processing - Ctrl+Shift+E");
            nextPageContinuousAct->setText("Stop Continuous Next Page");
        } else {
            previousPageContinuousAct->setIcon(QIcon(":/resources/prevpagecontinuousstop.png"));
            previousPageContinuousAct->setStatusTip("Stop continuous previous page processing - Ctrl+Shift+E");
            previousPageContinuousAct->setText("Stop Continuous Previous Page");
        }
    } else { // SET_DEFAULT_ACTION
        if (pageDirection == PAGE_NEXT) {
            nextPageContinuousAct->setIcon(QIcon(":/resources/nextpagecontinuous.png"));
            nextPageContinuousAct->setStatusTip(tr("Continuously process next document page - Ctrl+Shift+N"));
            nextPageContinuousAct->setText("Continuous Next Page");
            setPageContinuousIsRunning(false);
        } else {
            previousPageContinuousAct->setIcon(QIcon(":/resources/prevpagecontinuous.png"));
            previousPageContinuousAct->setStatusTip(tr("Continuously process previous document page - Ctrl+Shift+E"));
            previousPageContinuousAct->setText("Continuous Previous Page");
            setPageContinuousIsRunning(false);
        }
    }
}

bool Gui::continuousPageDialog(Direction d)
{
  pageDirection = d;
  int pageCount = 0;
  int _maxPages = 0;
  bool terminateProcess = false;
  setContinuousPageSig(true);
  QElapsedTimer continuousTimer;
  QString direction = d == PAGE_NEXT ? "Next" : "Previous";

  exportMode = PAGE_PROCESS;

  if (Preferences::modeGUI) {
      if (Preferences::doNotShowPageProcessDlg) {
          if (!processPageRange(setPageLineEdit->displayText())) {
              emit messageSig(LOG_STATUS,QString("%1 page processing terminated.").arg(direction));
              setPageContinuousIsRunning(false);
              emit setContinuousPageSig(false);
              return false;
          }
      }
      else
      {
          DialogExportPages *dialog = new DialogExportPages();

          if (dialog->exec() == QDialog::Accepted) {
              continuousTimer.start();

              if(dialog->allPages()){
                  processOption = EXPORT_ALL_PAGES;
              }
              else
              if(dialog->pageRange()){
                  processOption  = EXPORT_PAGE_RANGE;
                  pageRangeText = dialog->pageRangeText();
              }

              resetCache = dialog->resetCache();

              QSettings Settings;
              if (Preferences::doNotShowPageProcessDlg != dialog->doNotShowPageProcessDlg()) {
                  Preferences::doNotShowPageProcessDlg = dialog->doNotShowPageProcessDlg();
                  QVariant uValue(Preferences::doNotShowPageProcessDlg);
                  Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DoNotShowPageProcessDlg"),uValue);
              }

              if (Preferences::pageDisplayPause != dialog->pageDisplayPause()){
                  Preferences::pageDisplayPause = dialog->pageDisplayPause();
                  Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PageDisplayPause"),Preferences::pageDisplayPause);
              }

              // initialize progress dialogue
              m_progressDialog->setAutoHide(false);
              disconnect (m_progressDialog, SIGNAL (cancelClicked()), this, SLOT (cancelExporting()));
              connect (   m_progressDialog, SIGNAL (cancelClicked()), this, SLOT (cancelContinuousPage()));

          } else {
              emit messageSig(LOG_STATUS,QString("%1 page processing terminated.").arg(direction));
              setPageContinuousIsRunning(false);
              emit setContinuousPageSig(false);
              return false;
          }
      }
  }
  else
  { // command line mode
      continuousTimer.start();
      // Set processing direction - This effectively acts like a toggle.
      // If true, it sets false and vice versa.
      nextPageContinuousIsRunning = !nextPageContinuousIsRunning;
  }

  // Validate the page range
  if (processOption == EXPORT_PAGE_RANGE){
      if (! validatePageRange()){
          emit messageSig(LOG_STATUS,QString("%1 page processing terminated.").arg(direction));
          setPageContinuousIsRunning(false);
          emit setContinuousPageSig(false);
          return false;
      }
  }

  // Process is running - configure to stop
  setContinuousPageAct(SET_STOP_ACTION);

  if(resetCache)
      resetModelCache();

  QString message = QString("Continuous %1 Page Processing").arg(direction);

  // store current display page number
  logStatus() << QString("%1 start...").arg(message);

  if (processOption == EXPORT_ALL_PAGES){

      _maxPages = maxPages;

      if (Preferences::modeGUI) {
          m_progressDialog->setWindowTitle(tr("%1 Page Processing").arg(direction));
          m_progressDlgMessageLbl->setText(tr("%1").arg(message));
          m_progressDlgProgressBar->setRange(0,_maxPages);
          m_progressDialog->show();
      }
      int progress = 0;

      QApplication::setOverrideCursor(Qt::ArrowCursor);

      for (d == PAGE_NEXT ? displayPageNum = 1 : displayPageNum = maxPages ; d == PAGE_NEXT ? displayPageNum <= maxPages : displayPageNum >= 1 ; d == PAGE_NEXT ? displayPageNum++ : displayPageNum--) {

          if (! ContinuousPage()) {
              setPageContinuousIsRunning(false,d);
          }

          if (d == PAGE_NEXT) {
              terminateProcess =  !nextPageContinuousIsRunning;
          } else {
              terminateProcess =  !previousPageContinuousIsRunning;
          }

          if (terminateProcess) {
              message = QString("%1 page processing terminated before completion. %2 pages of %3 processed%4.")
                                .arg(direction)
                                .arg(d == PAGE_NEXT ? (displayPageNum - 1) : (maxPages - (displayPageNum - 1)))
                                .arg(maxPages)
                                .arg(QString(". %1").arg(gui->elapsedTime(continuousTimer.elapsed())));
              emit messageSig(LOG_STATUS,message);
              QApplication::restoreOverrideCursor();
              setContinuousPageAct(SET_DEFAULT_ACTION);
              emit setContinuousPageSig(false);
              m_progressDialog->setBtnToClose();
              m_progressDlgMessageLbl->setText(tr("%1").arg(message));
              disconnect (m_progressDialog, SIGNAL (cancelClicked()), this, SLOT (cancelContinuousPage()));
              connect (   m_progressDialog, SIGNAL (cancelClicked()), this, SLOT (cancelExporting()));

              return false;
          }

          pageCount = displayPageNum;

          displayPage();

          message = QString("%1 Page Processing - Processed page %2 of %3.").arg(direction).arg(displayPageNum).arg(maxPages);
          emit messageSig(LOG_STATUS,message);

          if (Preferences::modeGUI) {
              m_progressDlgProgressBar->setValue(d == PAGE_NEXT ? displayPageNum : ++progress);
              m_progressDlgMessageLbl->setText(tr("%1").arg(message));
          }

          qApp->processEvents();

          secSleeper::secSleep(Preferences::pageDisplayPause);
      }

      QApplication::restoreOverrideCursor();

  }
  else
  // Processing page range
  if (processOption == EXPORT_PAGE_RANGE) {

      QStringList pageRanges = pageRangeText.split(",");
      QList<int> printPages;
      // Split page range values
      foreach(QString ranges,pageRanges){
          // If n-n range, split into start through end pages
          if (ranges.contains("-")){
              QStringList range = ranges.split("-");
              int startPage = range[0].toInt();
              int endPage = range[1].toInt();
              if (d == PAGE_NEXT) {
                  for(int i = startPage; i <= endPage; i++){
                      printPages.append(i);
                  }
              } else {
                  for(int i = startPage; i >= endPage; i--){
                      printPages.append(i);
                  }
              }
          }
          // Or just add the range value
          else
          {
              printPages.append(ranges.toInt());
          }
      }

      // Sort the page numbers
      if (d == PAGE_NEXT)
          std::sort(printPages.begin(),printPages.end(),lt);    // Next - small to large
      else
          std::sort(printPages.begin(),printPages.end(),gt);    // Previous - large to small

      displayPageNum = printPages.first();

      _maxPages = printPages.count();

      if (Preferences::modeGUI) {
          m_progressDialog->setWindowTitle(tr("%1 Page Processing").arg(direction));
          m_progressDlgMessageLbl->setText(tr("%1").arg(message));
          m_progressDlgProgressBar->setRange(0,_maxPages);
          m_progressDialog->show();
      }

      QApplication::setOverrideCursor(Qt::BusyCursor);

      // process each page
      foreach(int printPage,printPages){

          displayPageNum = printPage;

          if (! ContinuousPage()) {
              setPageContinuousIsRunning(false,d);
          }

          if (d == PAGE_NEXT) {
              terminateProcess = !nextPageContinuousIsRunning;
          } else {
              terminateProcess = !previousPageContinuousIsRunning;
          }

          if (terminateProcess) {
              message = QString("%1 page processing terminated before completion. %2 pages of %3 processed%4.")
                                .arg(direction)
                                .arg(d == PAGE_NEXT ? pageCount : (_maxPages - pageCount))
                                .arg(_maxPages)
                                .arg(QString(". %1").arg(gui->elapsedTime(continuousTimer.elapsed())));
              emit messageSig(LOG_STATUS,message);
              QApplication::restoreOverrideCursor();
              setContinuousPageAct(SET_DEFAULT_ACTION);
              emit setContinuousPageSig(false);
              m_progressDialog->setBtnToClose();
              m_progressDlgMessageLbl->setText(tr("%1").arg(message));
              disconnect (m_progressDialog, SIGNAL (cancelClicked()), this, SLOT (cancelContinuousPage()));
              connect (   m_progressDialog, SIGNAL (cancelClicked()), this, SLOT (cancelExporting()));

              return false;
          }

          pageCount++;

          displayPage();

          message = QString("%1 Page Processing - Processed page %2 (%3 of %4) from the range of %5")
                            .arg(direction)
                            .arg(displayPageNum)
                            .arg(pageCount)
                            .arg(_maxPages)
                            .arg(pageRanges.join(" "));
          emit messageSig(LOG_STATUS,message);

          if (Preferences::modeGUI) {
              m_progressDlgProgressBar->setValue(pageCount);
              m_progressDlgMessageLbl->setText(tr("%1").arg(message));
          }

          qApp->processEvents();

          secSleeper::secSleep(Preferences::pageDisplayPause);
      }

      QApplication::restoreOverrideCursor();
  }

  setContinuousPageAct(SET_DEFAULT_ACTION);

  message = QString("%1 page processing completed. %2 of %3 %4 processed%5.")
                    .arg(direction)
                    .arg(d == PAGE_NEXT ? pageCount : _maxPages)
                    .arg(_maxPages)
                    .arg(_maxPages > 1 ? "pages" : "page")
                    .arg(QString(". %1").arg(gui->elapsedTime(continuousTimer.elapsed())));
  emit messageSig(LOG_INFO_STATUS,message);

  if (Preferences::modeGUI) {
      m_progressDialog->setBtnToClose();
      m_progressDlgProgressBar->setValue(_maxPages);
      m_progressDlgMessageLbl->setText(tr("%1").arg(message));
      disconnect (m_progressDialog, SIGNAL (cancelClicked()), this, SLOT (cancelContinuousPage()));
      connect (   m_progressDialog, SIGNAL (cancelClicked()), this, SLOT (cancelExporting()));
  }
  emit setContinuousPageSig(false);

  return true;
}

bool Gui::processPageRange(const QString &range)
{
  if (!range.isEmpty()){
      QString lineAllPages = QString("1-%1").arg(gui->maxPages);
      QString linePageRange = QString(range)
          .replace("of","-")
          .replace("to","-")
          .replace(" ","");
      if (lineAllPages == linePageRange) {
          processOption = EXPORT_ALL_PAGES;
        } else {
          processOption = EXPORT_PAGE_RANGE;
          pageRangeText = linePageRange;
        }
      return true;
    } else {
      processOption = EXPORT_ALL_PAGES;
      return true;
    }
  return false;
}

void Gui::firstPage()
{
  if (displayPageNum == 1) {
    statusBarMsg("You are on the first page");
  } else {
    displayPageNum = 1;
    displayPage();
  }
}

void Gui::lastPage()
{
  if (displayPageNum == maxPages) {
    statusBarMsg("You are on the last page");
  } else {
    countPages();
    displayPageNum = maxPages;
    displayPage();
  }
}

void Gui::setPage()
{
  QString string = setPageLineEdit->displayText();
  QRegExp rx("^(\\d+).*$");
  if (string.contains(rx)) {
    bool ok;
    int inputPage;
    inputPage = rx.cap(1).toInt(&ok);
    if (ok) {
      countPages();
      if (inputPage <= maxPages) {
        if (inputPage != displayPageNum) {
          displayPageNum = inputPage;
          displayPage();
          return;
        }
      } else {
        statusBarMsg("Page number entered is higher than total pages");
      }
    }
  }
  string = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);
  setPageLineEdit->setText(string);
}

void Gui::setGoToPage(int index)
{
  int goToPageNum = index+1;
  countPages();
  if (goToPageNum <= maxPages) {
      if (goToPageNum != displayPageNum) {
          displayPageNum = goToPageNum;
          displayPage();
        }
    }

  QString string = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);
  setPageLineEdit->setText(string);
}

void Gui::fitWidth()
{
  QRectF rect(0,0,pageSize(page.meta.LPub.page, 0)
                 ,pageSize(page.meta.LPub.page, 1));
  KpageView->fitWidth(rect);
}

void Gui::fitVisible()
{
  QRectF rect(0,0,pageSize(page.meta.LPub.page, 0),
                  pageSize(page.meta.LPub.page, 1));
  KpageView->fitVisible(rect);
  zoomSliderWidget->setValue(50);
}

void Gui::fitScene()
{
  QRectF rect(KpageView->scene()->itemsBoundingRect());
  KpageView->fitScene(rect);
  zoomSliderWidget->setValue(50);
}

void Gui::actualSize()
{
  KpageView->actualSize();
  zoomSliderWidget->setValue(100);
}

void Gui::zoomIn()
{
  KpageView->zoomIn();
}

void Gui::zoomOut()
{
  KpageView->zoomOut();
}

void Gui::viewResolution()
{
    KpageView->setResolution(resolution());
}

void Gui::zoomSlider(int value)
{
  const qreal z = value == 0 ? 0.1 : value*0.01;
  KpageView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
  KpageView->setTransform(QTransform::fromScale(z, z));
  KpageView->fitMode = FitNone;
}

void Gui::sceneGuides()
{
  Preferences::setSceneGuidesPreference(sceneGuidesComboAct->isChecked());
  SceneGuidesLineGroup->setEnabled(Preferences::sceneGuides);
  SceneGuidesPosGroup->setEnabled(Preferences::sceneGuides);
  KpageView->setSceneGuides();
}

void Gui::sceneGuidesLine()
{
  int line = int(Qt::DashLine);
  if (sceneGuidesSolidLineAct->isChecked())
      line = int(Qt::SolidLine);

  Preferences::setSceneGuidesLinePreference(line);
  KpageView->setSceneGuidesLine();
}

void Gui::sceneGuidesPosition()
{
    int position = int(GUIDES_TOP_LEFT);
    if (sceneGuidesPosTRightAct->isChecked())
        position = int(GUIDES_TOP_RIGHT);
    else
    if (sceneGuidesPosBLeftAct->isChecked())
        position = int(GUIDES_BOT_LEFT);
    else
    if (sceneGuidesPosBRightAct->isChecked())
        position = int(GUIDES_BOT_RIGHT);
    else
    if (sceneGuidesPosCentreAct->isChecked())
        position = int(GUIDES_CENTRE);

    Preferences::setSceneGuidesPositionPreference(position);
    KpageView->setSceneGuidesPos();
}

void Gui::sceneRuler()
{
  Preferences::setSceneRulerPreference(sceneRulerComboAct->isChecked());
  hideRulerPageBackgroundAct->setEnabled(Preferences::sceneRuler);
  SceneRulerGroup->setEnabled(Preferences::sceneRuler);
  KpageView->setSceneRuler();
}

void Gui::sceneRulerTracking()
{
  RulerTrackingType trackingStyle = TRACKING_NONE;
  if (sceneRulerTrackingTickAct->isChecked())
      trackingStyle = TRACKING_TICK;
  else
  if (sceneRulerTrackingLineAct->isChecked()) {
      trackingStyle = TRACKING_LINE;
      showTrackingCoordinatesAct->setEnabled(true);
  }

  Preferences::setSceneRulerTrackingPreference(int(trackingStyle));
  KpageView->setSceneRulerTracking();
}

void Gui::snapToGrid()
{
  bool checked = snapToGridComboAct->isChecked();
  if (Preferences::snapToGrid == checked)
      return;

  Preferences::setSnapToGridPreference(checked);
  hideGridPageBackgroundAct->setEnabled(Preferences::snapToGrid);
  GridStepSizeGroup->setEnabled(Preferences::snapToGrid);
  KpageView->setSnapToGrid();
  reloadCurrentPage();
}

void Gui::hidePageBackground()
{
  bool checked;
  if (sender() == hideRulerPageBackgroundAct) {
      checked = hideRulerPageBackgroundAct->isChecked();
      hideGridPageBackgroundAct->setChecked(checked);
  }
  else
  {
      checked = hideGridPageBackgroundAct->isChecked();
      hideRulerPageBackgroundAct->setChecked(checked);
  }
  Preferences::setHidePageBackgroundPreference(checked);
  reloadCurrentPage();
}

void Gui::showCoordinates()
{
  if (sender() == showTrackingCoordinatesAct) {
      Preferences::setShowTrackingCoordinatesPreference(
                  showTrackingCoordinatesAct->isChecked());
  }
  else
  if (sender() == showGuidesCoordinatesAct)
  {
      Preferences::setShowGuidesCoordinatesPreference(
                  showGuidesCoordinatesAct->isChecked());
  }
  KpageView->setShowCoordinates();
}

void Gui::gridSize(int index)
{
  if (Preferences::gridSizeIndex == index)
    return;

  Preferences::setGridSizeIndexPreference(index);
  snapGridActions[GRID_SIZE_FIRST + index]->setChecked(true);
  KpageView->setGridSize();
  reloadCurrentPage();
}

void Gui::gridSizeTriggered()
{
    QObject* Action = sender();

    for (int CommandIdx = 0; CommandIdx < NUM_GRID_SIZES; CommandIdx++)
    {
        if (Action == snapGridActions[CommandIdx])
        {
            switch (CommandIdx)
            {
            case SCENE_GRID_SIZE_S1:
            case SCENE_GRID_SIZE_S2:
            case SCENE_GRID_SIZE_S3:
            case SCENE_GRID_SIZE_S4:
            case SCENE_GRID_SIZE_S5:
            case SCENE_GRID_SIZE_S6:
            case SCENE_GRID_SIZE_S7:
            case SCENE_GRID_SIZE_S8:
            case SCENE_GRID_SIZE_S9:
                gridSize(CommandIdx - GRID_SIZE_FIRST);
                break;
            }
        }
    }
}

bool Gui::isUserSceneObject(const int so)
{
    for ( const auto iso : IncludedSceneObjects)
        if (iso == SceneObject(so))
            return true;
    return false;
}

void Gui::bringToFront()
{
    setSceneItemZValue(BringToFront);
}

void Gui::sendToBack()
{
    setSceneItemZValue(SendToBack);
}

void Gui::SetRotStepMeta()
{
    mStepRotation[0] = mRotStepAngleX;
    mStepRotation[1] = mRotStepAngleY;
    mStepRotation[2] = mRotStepAngleZ;

    if (getCurFile() != "") {
        ShowStepRotationStatus();
        Step *currentStep = gui->getCurrentStep();

        if (currentStep){
            bool newCommand = currentStep->rotStepMeta.here() == Where();
            int it = lcGetActiveProject()->GetImageType();
            Where top = currentStep->topOfStep();

            RotStepData rotStepData = currentStep->rotStepMeta.value();
            rotStepData.type    = mRotStepTransform;
            rotStepData.rots[0] = double(mStepRotation[0]);
            rotStepData.rots[1] = double(mStepRotation[1]);
            rotStepData.rots[2] = double(mStepRotation[2]);
            currentStep->rotStepMeta.setValue(rotStepData);
            QString metaString = currentStep->rotStepMeta.format(false/*no LOCAL tag*/,false);

            if (newCommand){
                if (top.modelName == gui->topLevelFile())
                    currentStep->mi(it)->scanPastLPubMeta(top);

                QString line = gui->readLine(top);
                Rc rc = page.meta.parse(line,top);
                if (rc == RotStepRc || rc == StepRc){
                   currentStep->mi(it)->replaceMeta(top, metaString);
                } else {
                   currentStep->mi(it)->insertMeta(top, metaString);
                }
            } else {
                currentStep->mi(it)->replaceMeta(top, metaString);
            }
        }
    }
}

void Gui::ShowStepRotationStatus()
{
    QString rotLabel = QString("ROTSTEP X: %1 Y: %2 Z: %3 Transform: %4")
                               .arg(QString::number(double(mRotStepAngleX), 'f', 2))
                               .arg(QString::number(double(mRotStepAngleY), 'f', 2))
                               .arg(QString::number(double(mRotStepAngleZ), 'f', 2))
                               .arg(mRotStepTransform == "REL" ? "RELATIVE" :
                                    mRotStepTransform == "ABS" ? "ABSOLUTE" : "ADD");
    statusBarMsg(rotLabel);
}

/*
 Compares the two version strings (first and second).
 If first is greater than second, this function returns true.
 If second is greater than first, this function returns false.
 If both versions are the same, this function returns false.
 */
bool  Gui::compareVersionStr (const QString& first, const QString& second)
{
    QStringList versionsFirst = first.split (".");
    QStringList versionsSecond = second.split (".");

    int count = qMin (versionsFirst.count(), versionsSecond.count());

    for (int i = 0; i < count; ++i) {
        int a = QString (versionsFirst.at (i)).toInt();
        int b = QString (versionsSecond.at (i)).toInt();

        if (a > b)
            return true;

        else if (b > a)
            return false;
    }

    return versionsSecond.count() < versionsFirst.count();
}

void Gui::displayFile(
    LDrawFile     *ldrawFile,
    const QString &modelName,
    bool editModelFile,
    bool displayStartPage)
{
    if (! exporting()) {
        if (editModelFile) {
            displayModelFileSig(ldrawFile, modelName);
        } else {
            displayFileSig(ldrawFile, modelName);

            if (curSubFile == modelName)
                return;
            else
                curSubFile = modelName;

            if (displayStartPage)
                countPages();

            int modelPageNum = ldrawFile->getModelStartPageNumber(modelName);

            if (displayStartPage) {
               if (displayPageNum != modelPageNum && modelPageNum != 0) {
                   displayPageNum  = modelPageNum;
                   displayPage();
               }
            } else if (displayPageNum < modelPageNum) {
                Where topOfSteps(modelName,0);
                showLineSig(topOfSteps.lineNumber);
            }

            int currentIndex = 0;
            for (int i = 0; i < mpdCombo->count(); i++) {
                if (mpdCombo->itemText(i) == modelName) {
                    currentIndex = i;
                    break;
                }
            }
            mpdCombo->setCurrentIndex(currentIndex);
            mpdCombo->setToolTip(tr("Current Submodel: %1").arg(mpdCombo->currentText()));
        }
    }
}

void Gui::displayParmsFile(
  const QString &fileName)
{
    displayParmsFileSig(fileName);
}

void Gui::refreshModelFile()
{
     editModelFile(/*saveBefore*/false);
}

void Gui::editModelFile()
{
    editModelFile(/*saveBefore*/true);
}

void Gui::editModelFile(bool saveBefore)
{
    if (getCurFile().isEmpty())
        return;
    if (saveBefore)
        save();
    displayFile(&ldrawFile, getCurFile(), true/*modelFile*/);
    editModeWindow->setWindowTitle(tr("Edit %1").arg(QFileInfo(getCurFile()).fileName()));
    editModeWindow->show();
}


void Gui::deployExportBanner(bool b)
{

  if (b) {
      QString exportBanner, imageFile;

#ifdef __APPLE__

      exportBanner = QString("%1/%2").arg(Preferences::lpubDataPath,"extras/printbanner.ldr");
      imageFile = QString("%1/%2").arg(Preferences::lpubDataPath,"extras/PDFPrint.jpg");

#else

      exportBanner = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath,"extras/printbanner.ldr"));
      imageFile = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath,"extras/PDFPrint.jpg"));

#endif

      installExportBanner(exportMode, exportBanner,imageFile);
    }
}

/*-----------------------------------------------------------------------------*/

bool Gui::installExportBanner(const int &type, const QString &printFile, const QString &imageFile){

    QList<QString> bannerData;
    bannerData << "0 Print Banner";
    bannerData << "0 Name: printbanner.ldr";
    bannerData << "0 Author: Trevor SANDY";
    bannerData << "0 Unofficial Model";
    bannerData << "0 !LEOCAD MODEL NAME Printbanner";
    bannerData << "0 !LEOCAD MODEL AUTHOR LPub3D";
    bannerData << "0 !LEOCAD MODEL DESCRIPTION Graphic displayed during pdf printing";
    bannerData << "0 !LEOCAD MODEL BACKGROUND IMAGE NAME " + imageFile;
    bannerData << "1 71 0 0 0 1 0 0 0 1 0 0 0 1 3020.dat";
    bannerData << "1 71 30 -8 10 1 0 0 0 1 0 0 0 1 3024.dat";
    bannerData << "1 71 30 -16 10 1 0 0 0 1 0 0 0 1 3024.dat";
    bannerData << "1 71 -30 -8 10 1 0 0 0 1 0 0 0 1 3024.dat";
    bannerData << "1 71 -30 -16 10 1 0 0 0 1 0 0 0 1 3024.dat";
    bannerData << "1 71 -30 -32 10 1 0 0 0 1 0 0 0 1 6091.dat";
    bannerData << "1 71 30 -32 10 1 0 0 0 1 0 0 0 1 6091.dat";
    bannerData << "1 71 30 -32 10 1 0 0 0 1 0 0 0 1 30039.dat";
    bannerData << "1 2 -30 -32 10 1 0 0 0 1 0 0 0 1 30039.dat";
    bannerData << "1 71 0 -24 10 1 0 0 0 1 0 0 0 1 3937.dat";
    bannerData << "1 72 0 -8 -10 1 0 0 0 1 0 0 0 1 3023.dat";
    bannerData << "1 72 0 -8 -10 -1 0 0 0 1 0 0 0 -1 85984.dat";
    bannerData << "1 71 0 -23.272 6.254 -1 0 0 0 0.927 0.375 0 0.375 -0.927 3938.dat";
    bannerData << "1 72 0 -45.524 -2.737 -1 0 0 0 0.927 0.375 0 0.375 -0.927 4865a.dat";
    switch (type) {
    case EXPORT_PNG:
        bannerData << "1 25 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
        bannerData << "1 25 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptn.dat";
        bannerData << "1 25 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptg.dat";
        break;
    case EXPORT_JPG:
        bannerData << "1 92 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptj.dat";
        bannerData << "1 92 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
        bannerData << "1 92 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptg.dat";
        break;
    case EXPORT_BMP:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptb.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptm.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
        break;
    case EXPORT_WAVEFRONT:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bpto.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptb.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptj.dat";
        break;
    case EXPORT_COLLADA:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptd.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bpta.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bpte.dat";
        break;
    case EXPORT_3DS_MAX:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptx.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptd.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bpts.dat";
        break;
    case EXPORT_STL:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bpts.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptt.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptl.dat";
        break;
    case EXPORT_POVRAY:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bpto.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptv.dat";
        break;
    case EXPORT_HTML_PARTS:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bpth.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptt.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptm.dat";
        break;
    case EXPORT_CSV:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptc.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bpts.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptv.dat";
        break;
    case EXPORT_BRICKLINK:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptb.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptl.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptk.dat";
        break;
    default:
        bannerData << "1 216 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
        bannerData << "1 216 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptd.dat";
        bannerData << "1 216 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptf.dat";
    }
    bannerData << "0";
    bannerData << "0 NOFILE";

    QFile bannerFile(printFile);
    if ( ! bannerFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
        emit gui->messageSig(LOG_ERROR,tr("Cannot open Export Banner file %1 for writing:\n%2")
                             .arg(printFile)
                             .arg(bannerFile.errorString()));
        return false;
    }
    QTextStream out(&bannerFile);
    for (int i = 0; i < bannerData.size(); i++) {
        QString fileLine = bannerData[i];
        out << fileLine << endl;
    }
    bannerFile.close();

    Project* BannerProject = new Project();
    if (!gMainWindow->OpenProject(bannerFile.fileName()))
    {
      emit gui->messageSig(LOG_ERROR, tr("Could not load banner'%1'.").arg(bannerFile.fileName()));
      delete BannerProject;
      return false;
    }

    return true;
}

void Gui::mpdComboChanged(int index)
{
  Q_UNUSED(index);
  QString newSubFile = mpdCombo->currentText();
  if (curSubFile != newSubFile) {
      int modelPageNum = ldrawFile.getModelStartPageNumber(newSubFile);
      logInfo() << "SELECT Model: " << newSubFile << " @ Page: " << modelPageNum;
      countPages();
      if (displayPageNum != modelPageNum && modelPageNum != 0) {
          displayPageNum  = modelPageNum;
          displayPage();
        } else {
          // TODO add status bar message
          Where topOfSteps(newSubFile,0);
          curSubFile = newSubFile;
          displayFileSig(&ldrawFile, curSubFile);
          showLineSig(topOfSteps.lineNumber);
        }
    }
  mpdCombo->setCurrentIndex(index);
  mpdCombo->setToolTip(tr("Current Submodel: %1").arg(mpdCombo->currentText()));
}

void Gui::reloadViewer(){
  if (!getCurFile().isEmpty()) {
      Project* NewProject = new Project();
      gApplication->SetProject(NewProject);
  }
}

void Gui::loadTheme(bool restart){
  Application::instance()->setTheme();
  if (!restart) {
      reloadViewer();
      reloadCurrentModelFile();
    }
}

void  Gui::restartApplication(bool changeLibrary){
    QStringList args;
    if (! changeLibrary && ! getCurFile().isEmpty()){
        args = QApplication::arguments();
        args.removeFirst();
        if (!args.contains(getCurFile(),Qt::CaseInsensitive))
            args << QString("%1").arg(getCurFile());
        QSettings Settings;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,SAVE_DISPLAY_PAGE_NUM_KEY),displayPageNum);
    } else {
        args << (Preferences::validLDrawLibraryChange == LEGO_LIBRARY  ? "++liblego" :
                 Preferences::validLDrawLibraryChange == TENTE_LIBRARY ? "++libtente" : "++libvexiq");
    }
    QProcess::startDetached(QApplication::applicationFilePath(), args);
    messageSig(LOG_INFO, QString("Restarted LPub3D with Command: %1 %2")
               .arg(QApplication::applicationFilePath()).arg(args.join(" ")));
    QCoreApplication::quit();
}

void Gui::loadUpdatedImages()
{
   m_updateViewer = false;
   reloadCurrentPage();
   m_updateViewer = true;
}

void Gui::reloadCurrentPage(){
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,"No model file page to redisplay.");
        return;
    }

    if (sender() == editWindow) {
        bool _continue;
        if (Preferences::saveOnUpdate) {
            _continue = maybeSave(false); // No prompt
        } else {
            _continue = maybeSave(true,SaveOnUpdate);
        }
        if (!_continue)
            return;
    }

    timer.start();
    displayPage();
    emit messageSig(LOG_STATUS, QString("Page %1 reloaded. %2")
                    .arg(displayPageNum)
                    .arg(elapsedTime(timer.elapsed())));

}

void Gui::reloadCurrentModelFile(){
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,"No model file to reopen.");
        return;
    }

    if (sender() == editModeWindow) {
        bool _continue;
        if (Preferences::saveOnUpdate) {
            _continue = maybeSave(false); // No prompt
        } else {
            _continue = maybeSave(true, SaveOnUpdate);
        }
        if (!_continue)
            return;
    }

    timer.start();

    int savePage = displayPageNum;
    openFile(curFile);
    displayPageNum = savePage;
    displayPage();
    enableActions();

    emit messageSig(LOG_STATUS, QString("Model file reloaded (%1 parts). %2")
                    .arg(ldrawFile.getPartCount())
                    .arg(elapsedTime(timer.elapsed())));
}

void Gui::resetModelCache(QString file)
{
    if (resetCache && !file.isEmpty()) {
        emit messageSig(LOG_TRACE, QString("Reset parts cache is destructive!"));
        curFile = file;
        QString fileDir = QFileInfo(file).absolutePath();
        emit messageSig(LOG_INFO, QString("Reset parts cache directory %1").arg(fileDir));
        QString saveCurrentDir = QDir::currentPath();
        if (! QDir::setCurrent(fileDir))
            emit messageSig(LOG_ERROR, QString("Reset cache failed to set current directory %1").arg(fileDir));

        clearCustomPartCache(true);
        clearAllCaches();

        if (! QDir::setCurrent(saveCurrentDir))
            emit messageSig(LOG_ERROR, QString("Reset cache failed to restore current directory %1").arg(saveCurrentDir));
        resetCache = false;
    }
}

void Gui::clearAndRedrawModelFile() {

    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,"A model must be open to reset its caches - no action taken.");
        return;
    }

    bool saveChange = changeAccepted;
    changeAccepted = true;

    if (sender() == editModeWindow) {
        bool _continue;
        if (Preferences::saveOnRedraw) {
            _continue = maybeSave(false); // No prompt
        } else {
            _continue = maybeSave(true,SaveOnRedraw);
        }
        if (!_continue)
            return;
    }

    timer.start();

    if (Preferences::enableFadeSteps || Preferences::enableHighlightStep) {
        ldrawFile.clearPrevStepPositions();
    }

    clearPLICache();
    clearCSICache();
    clearSubmodelCache();
    clearTempCache();

    //reload current model file
    int savePage = displayPageNum;
    openFile(curFile);
    displayPageNum = savePage;
    displayPage();
    enableActions();

    emit messageSig(LOG_STATUS, QString("All caches reset and model file reloaded (%1 parts). %2")
                    .arg(ldrawFile.getPartCount())
                    .arg(elapsedTime(timer.elapsed())));

    changeAccepted = saveChange;
}

void Gui::clearAndReloadModelFile() {
    if (sender() == editWindow) {
        bool _continue;
        if (Preferences::saveOnRedraw) {
            _continue = maybeSave(false); // No prompt
        } else {
            _continue = maybeSave(true, SaveOnRedraw);
        }
        if (!_continue)
            return;
    }
    clearAllCaches();
}

void Gui::clearAllCaches()
{
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,"A model must be open to reset its caches - no action taken.");
        return;
    }

    timer.start();

    if (Preferences::enableFadeSteps || Preferences::enableHighlightStep) {
        ldrawFile.clearPrevStepPositions();
    }

    clearPLICache();
    clearCSICache();
    clearSubmodelCache();
    clearTempCache();

    //reload current model file
    int savePage = displayPageNum;
    openFile(curFile);
    displayPageNum = savePage;
    displayPage();
    enableActions();

    emit messageSig(LOG_STATUS, QString("All caches reset and model file reloaded (%1 parts). %2")
                                        .arg(ldrawFile.getPartCount())
                                        .arg(elapsedTime(timer.elapsed())));

}

void Gui::clearCustomPartCache(bool silent)
{
  if (Paths::customDir.isEmpty())
      return;

  QMessageBox::StandardButton ret = QMessageBox::Ok;
  QString message = QString("Fade and highlight parts in %1 will be deleted. The current model parts will be regenerated. "
                            "Regenerated files will be updated in %2.")
                            .arg(Paths::customDir)
                            .arg(Preferences::validLDrawCustomArchive);
  if (silent || !Preferences::modeGUI) {
      emit messageSig(LOG_INFO,message);
  } else {
      ret = QMessageBox::warning(this, tr(VER_PRODUCTNAME_STR),
                                 tr("%1 Do you want to delete the custom file cache?").arg(message),
                                 QMessageBox::Yes | QMessageBox::Discard | QMessageBox::Cancel);
  }

  if (ret == QMessageBox::Cancel)
      return;

  QString dirName = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customDir));

  int count = 0;
  emit messageSig(LOG_INFO,QString("-Removing folder %1").arg(dirName));
  if (removeDir(count, dirName)){
      emit messageSig(LOG_INFO_STATUS,QString("Custom parts cache cleaned.  %1 %2 removed.")
                                              .arg(count)
                                              .arg(count == 1 ? "item": "items"));
  } else {
      emit messageSig(LOG_ERROR,QString("Unable to remove custom parts cache directory: %1").arg(dirName));
      return;
  }

  // regenerate custom parts
  bool overwriteCustomParts = true;
  processFadeColourParts(overwriteCustomParts);       // (re)generate and archive fade parts based on the loaded model file
  processHighlightColourParts(overwriteCustomParts);  // (re)generate and archive highlight parts based on the loaded model file

  if (!getCurFile().isEmpty() && Preferences::modeGUI){
      displayPage();
  }
}

void Gui::clearPLICache()
{
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,"A model must be open to clean its parts cache - no action taken.");
        return;
    }

    QDir dir(QDir::currentPath() + "/" + Paths::partsDir);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();
    int count = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(fileInfo.absoluteFilePath());
        if (!file.remove()) {
            emit messageSig(LOG_ERROR,QString("Unable to remove %1")
                            .arg(fileInfo.absoluteFilePath()));
        } else {
#ifdef QT_DEBUG_MODE
            emit messageSig(LOG_TRACE,QString("-File %1 removed").arg(fileInfo.absoluteFilePath()));
#endif
            count++;
        }
    }
    emit messageSig(LOG_INFO_STATUS,QString("Parts content cache cleaned. %1 items removed.").arg(count));
}

void Gui::clearCSICache()
{
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,"A model must be open to clean its assembly cache - no action taken.");
        return;
    }

    QDir dir(QDir::currentPath() + "/" + Paths::assemDir);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();
    int count = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(fileInfo.absoluteFilePath());
        if (!file.remove()) {
            emit messageSig(LOG_ERROR,QString("Unable to remove %1")
                            .arg(fileInfo.absoluteFilePath()));
        } else {
#ifdef QT_DEBUG_MODE
            emit messageSig(LOG_TRACE,QString("-File %1 removed").arg(fileInfo.absoluteFilePath()));
#endif
            count++;
        }
    }

    emit messageSig(LOG_INFO_STATUS,QString("Assembly content cache cleaned. %1 items removed.").arg(count));
}

void Gui::clearSubmodelCache(const QString &key)
{
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,"A model must be open to clean its Submodel cache - no action taken.");
        return;
    }

    QDir dir(QDir::currentPath() + "/" + Paths::submodelDir);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();
    int count = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(fileInfo.absoluteFilePath());
        bool deleteSpecificFile = fileInfo.absoluteFilePath().contains(key);
        if (key.isEmpty() || deleteSpecificFile){
            if (!file.remove()) {
                emit messageSig(LOG_ERROR,QString("Unable to remove %1")
                                .arg(fileInfo.absoluteFilePath()));
            } else {
#ifdef QT_DEBUG_MODE
                emit messageSig(LOG_TRACE,QString("-File %1 removed").arg(fileInfo.absoluteFilePath()));
#endif
                count++;
                if (deleteSpecificFile)
                    break;
            }
        }
    }

    emit messageSig(LOG_INFO_STATUS,QString("Submodel content cache cleaned. %1 items removed.").arg(count));
}

void Gui::clearTempCache()
{
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,QString("A model must be open to clean its 3D cache - no action taken."));
        return;
    }

    QDir tmpDir(QDir::currentPath() + "/" + Paths::tmpDir);
    tmpDir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = tmpDir.entryInfoList();
    int count1 = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(fileInfo.absoluteFilePath());
        if (!file.remove()) {
            emit messageSig(LOG_ERROR,QString("Unable to remove %1")
                                              .arg(fileInfo.absoluteFilePath()));
          } else {
#ifdef QT_DEBUG_MODE
            emit messageSig(LOG_TRACE,QString("-File %1 removed").arg(fileInfo.absoluteFilePath()));
#endif
            count1++;
        }
      }

    ldrawFile.tempCacheCleared();

    emit messageSig(LOG_INFO_STATUS,QString("Temporary model file cache cleaned. %1 items removed.").arg(count1));
}

bool Gui::removeDir(int &count, const QString & dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        QList<QFileInfo> entryInfoList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files, QDir::DirsFirst);
        Q_FOREACH(QFileInfo fileInfo, entryInfoList) {
            if (fileInfo.isDir()) {
                QString subDir = fileInfo.absoluteFilePath();
                result = removeDir(count,subDir);
            }
            else
            if (fileInfo.isFile()){
                if ((result = QFile::remove(fileInfo.absoluteFilePath()))) {
#ifdef QT_DEBUG_MODE
                    emit messageSig(LOG_TRACE,QString("-File %1 removed").arg(fileInfo.absoluteFilePath()));
#endif
                    count++;
                }
            }
            if (!result) {
                return result;
            }
        }
        if ((result = dir.rmdir(dir.absolutePath())))
            emit messageSig(LOG_TRACE,QString("-Folder %1 cleaned").arg(dir.absolutePath()));
    }
    return result;
}

void Gui::clearStepCSICache(QString &pngName) {
  QString tmpDirName   = QDir::currentPath() + "/" + Paths::tmpDir;
  QString assemDirName = QDir::currentPath() + "/" + Paths::assemDir;
  QFileInfo fileInfo(pngName);
  QFile file(assemDirName + "/" + fileInfo.fileName());
  if (!file.remove()) {
      emit messageSig(LOG_ERROR,QString("Unable to remove %1")
                                        .arg(assemDirName + "/" + fileInfo.fileName()));
    }
  QString ldrName;
  if (renderer->useLDViewSCall()){
      ldrName = tmpDirName + "/" + fileInfo.completeBaseName() + ".ldr";
    } else {
      ldrName = tmpDirName + "/csi.ldr";
    }
  file.setFileName(ldrName);
  if (!file.remove()) {
      emit messageSig(LOG_ERROR,QString("Unable to remove %1")
                                        .arg(ldrName));
    }
  if (Preferences::enableFadeSteps) {
      clearPrevStepPositions();
    }
  displayPage();
}

void Gui::clearPageCSICache(PlacementType relativeType, Page *page) {
  if (page->list.size()) {
      if (relativeType == SingleStepType) {         // single step page
          Range *range = dynamic_cast<Range *>(page->list[0]);
          if (range->relativeType == RangeType) {
              Step *step = dynamic_cast<Step *>(range->list[0]);
              if (step && step->relativeType == StepType) {
                  clearPageCSIGraphicsItems(step);
              } // validate step (StepType) and process...
          } // validate RangeType - to cast step
      } else if (relativeType == StepGroupType) {  // multi-step page
          for (int i = 0; i < page->list.size(); i++){
              Range *range = dynamic_cast<Range *>(page->list[i]);
              for (int j = 0; j < range->list.size(); j++){
                  if (range->relativeType == RangeType) {
                      Step *step = dynamic_cast<Step *>(range->list[j]);
                      if (step && step->relativeType == StepType){
                          clearPageCSIGraphicsItems(step);
                      } // validate step (StepType) and process...
                  } // validate RangeType - to cast step
              } // for each step within divided group...=>list[AbstractRangeElement]->StepType
          } // for each divided group within page...=>list[AbstractStepsElement]->RangeType
      }
      if (Preferences::enableFadeSteps) {
          clearPrevStepPositions();
      }
      displayPage();
   }
}

/*
 * Clear step image graphics items
 * This function recurses the step's model to clear images and associated model files.
 * Call only if using LDView Single Call (useLDViewSCall=true)
 */
void Gui::clearPageCSIGraphicsItems(Step *step) {
  // Capture ldr and image names
  QString tmpDirName   = QDir::currentPath() + "/" + Paths::tmpDir;
  QString assemDirName = QDir::currentPath() + "/" + Paths::assemDir;
  QString ldrName;
  bool itemsCleared = false;
  // process step's image(s)
  QFileInfo fileInfo(step->pngName);
  QFile file(assemDirName + "/" + fileInfo.fileName());
  if (!file.remove()) {
      emit messageSig(LOG_ERROR,QString("Unable to remove %1")
                            .arg(assemDirName + "/" + fileInfo.fileName()));
  }
  if (renderer->useLDViewSCall()){
      ldrName = tmpDirName + "/" + fileInfo.completeBaseName() + ".ldr";
      file.setFileName(ldrName);
      if (!file.remove()) {
         emit messageSig(LOG_ERROR,QString("Unable to remove %1")
                                           .arg(ldrName));
      }
  } else if (! itemsCleared){
      ldrName = tmpDirName + "/csi.ldr";
      file.setFileName(ldrName);
      if (!file.remove()) {
          emit messageSig(LOG_ERROR,QString("Unable to remove %1")
                                            .arg(ldrName));
      }
      itemsCleared = true;
  }
  // process callout step(s) image(s)
  for (int k = 0; k < step->list.size(); k++) {
      if (step->list[k]->relativeType == CalloutType) {
          Callout *callout = dynamic_cast<Callout *>(step->list[k]);
          for (int l = 0; l < callout->list.size(); l++){
              Range *range = dynamic_cast<Range *>(callout->list[l]);
              for (int m = 0; m < range->list.size(); m++){
                  if (range->relativeType == RangeType) {
                      Step *step = dynamic_cast<Step *>(range->list[m]);
                      if (step && step->relativeType == StepType) {
                          clearPageCSIGraphicsItems(step);
                      } // 1.6 validate if Step relativeType is StepType - to clear image, check for Callout
                  } // 1.5 validate if Range relativeType is RangeType - to cast as Step
              } // 1.4 for each Step list-item within a Range...=>list[AbstractRangeElement]->StepType
          } // 1.3 for each Range list-item within a Callout...=>list[AbstractStepsElement]->RangeType
      } // 1.2 validate if relativeType is CalloutType - to cast as Callout...
  } // 1.1 for each Callout list-item within a Step...=>list[Steps]->CalloutType
}

/***************************************************************************
 * These are infrequently used functions for basic environment
 * configuration stuff
 **************************************************************************/

void Gui::pageSetup()
{
  GlobalPageDialog::getPageGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::assemSetup()
{
  GlobalAssemDialog::getAssemGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::pliSetup()
{
  GlobalPliDialog::getPliGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::bomSetup()
{
  GlobalPliDialog::getBomGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::calloutSetup()
{
  GlobalCalloutDialog::getCalloutGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::multiStepSetup()
{
  GlobalMultiStepDialog::getMultiStepGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::subModelSetup()
{
  GlobalSubModelDialog::getSubModelGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::projectSetup()
{
  GlobalProjectDialog::getProjectGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::fadeStepSetup()
{
  GlobalFadeStepDialog::getFadeStepGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::highlightStepSetup()
{
  GlobalHighlightStepDialog::getHighlightStepGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::editTitleAnnotations()
{
    QFileInfo fileInfo(Preferences::titleAnnotationsFile);
    if (!fileInfo.exists()) {
        if (!Annotations::exportTitleAnnotationsFile()) {
            emit messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
            return;
        }
    }
    displayParmsFile(Preferences::titleAnnotationsFile);
    parmsWindow->setWindowTitle(tr("Part Title Annotation","Edit/add part title part annotations"));
    parmsWindow->show();
}

void Gui::editFreeFormAnnitations()
{
    QFileInfo fileInfo(Preferences::freeformAnnotationsFile);
    if (!fileInfo.exists()) {
        if (!Annotations::exportfreeformAnnotationsHeader()) {
            emit messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
            return;
        }
    }
    displayParmsFile(Preferences::freeformAnnotationsFile);
    parmsWindow->setWindowTitle(tr("Freeform Annotation","Edit/add freeform part annotations"));
    parmsWindow->show();
}

void Gui::editLDrawColourParts()
{
    displayParmsFile(Preferences::ldrawColourPartsFile);
    parmsWindow->setWindowTitle(tr("LDraw Color Parts","Edit/add LDraw static color parts"));
    parmsWindow->show();
}

void Gui::editPliBomSubstituteParts()
{
    QFileInfo fileInfo(Preferences::pliSubstitutePartsFile);
    if (!fileInfo.exists()) {
        if (!PliSubstituteParts::exportSubstitutePartsHeader()) {
            emit messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
            return;
        }
    }
    displayParmsFile(Preferences::pliSubstitutePartsFile);
    parmsWindow->setWindowTitle(tr("PLI/BOM Substitute Parts","Edit/add PLI/BOM substitute parts"));
    parmsWindow->show();
}

void Gui::editExcludedParts()
{
    QFileInfo fileInfo(Preferences::excludedPartsFile);
    if (!fileInfo.exists()) {
        if (!ExcludedParts::exportExcludedParts()) {
            emit messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
            return;
        }
    }
    displayParmsFile(fileInfo.absoluteFilePath());
    parmsWindow->setWindowTitle(tr("Parts List Excluded Parts","Edit/add excluded parts"));
    parmsWindow->show();
}

void Gui::editLdrawIniFile()
{
  if (!Preferences::ldSearchDirs.isEmpty()) {
      displayParmsFile(Preferences::ldrawiniFile);
      parmsWindow->setWindowTitle(tr("LDraw.ini Edit","Edit LDraw.ini search directory entries."));
      parmsWindow->show();
    }
}

void Gui::editLPub3DIniFile()
{
    QString lpubConfigFile,fileExt;
#if defined Q_OS_WIN
    fileExt = "ini";
    if (Preferences::portableDistribution)
        lpubConfigFile = QString("%1/config/%2/%3.%4")
                                 .arg(Preferences::lpub3dPath)
                                 .arg(COMPANYNAME_STR).arg(Preferences::lpub3dAppName).arg(fileExt);
#else
#if defined Q_OS_MACOS
    fileExt = "plist";
    lpubConfigFile = QString("%1/com.lpub3d-software.%2.%3")
                             .arg(Preferences::lpubConfigPath)
                             .arg(QString(Preferences::lpub3dAppName).replace(".app","")).arg(fileExt);
#elif defined Q_OS_LINUX
    fileExt = "conf";
    lpubConfigFile = QString("%1/%2/%3.%4")
                             .arg(Preferences::lpubConfigPath)
                             .arg(COMPANYNAME_STR)
                             .arg(Preferences::lpub3dAppName).arg(fileExt);
#endif
#endif
    displayParmsFile(lpubConfigFile);
    parmsWindow->setWindowTitle(tr("%1.%2 Edit","Edit %1 application configuration settings.")
                                   .arg(Preferences::lpub3dAppName).arg(fileExt));
    parmsWindow->show();
}

void Gui::editLdgliteIni()
{
    displayParmsFile(Preferences::ldgliteIni);
    parmsWindow->setWindowTitle(tr("Edit LDGLite INI","Edit LDGLite INI "));
    parmsWindow->show();
}

void Gui::editNativePovIni()
{
    displayParmsFile(Preferences::nativeExportIni);
    parmsWindow->setWindowTitle(tr("Edit Native Export INI","Edit Native Export INI"));
    parmsWindow->show();
}

void Gui::editLdviewIni()
{
    displayParmsFile(Preferences::ldviewIni);
    parmsWindow->setWindowTitle(tr("Edit LDView INI","Edit LDView INI"));
    parmsWindow->show();
}

void Gui::editLdviewPovIni()
{
    displayParmsFile(Preferences::ldviewPOVIni);
    parmsWindow->setWindowTitle(tr("Edit LDView POV file generation INI","Edit LDView POV file generation INI"));
    parmsWindow->show();
}

void Gui::editPovrayIni()
{
    displayParmsFile(Preferences::povrayIni);
    parmsWindow->setWindowTitle(tr("Edit Raytracer INI","Edit Raytracer INI"));
    parmsWindow->show();
}

void Gui::editPovrayConf()
{
    displayParmsFile(Preferences::povrayConf);
    parmsWindow->setWindowTitle(tr("Edit Raytracer file access conf","Edit Raytracer file access conf"));
    parmsWindow->show();
}

void Gui::editAnnotationStyle()
{
    QFileInfo fileInfo(QString("%1/extras/%2").arg(Preferences::lpubDataPath,Preferences::validAnnotationStyleFile));
    if (!fileInfo.exists()) {
        if (!Annotations::exportAnnotationStyleFile()) {
            emit messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
            return;
        }
    }
    displayParmsFile(fileInfo.absoluteFilePath());
    parmsWindow->setWindowTitle(tr("Part Annotation Style reference",
                                   "Edit/add Part Annotation Style reference"));
    parmsWindow->show();
}

void Gui::editLD2BLCodesXRef()
{
    QFileInfo fileInfo(QString("%1/extras/%2").arg(Preferences::lpubDataPath,VER_LPUB3D_LD2BLCODESXREF_FILE));
    if (!fileInfo.exists()) {
        if (!Annotations::exportLD2BLCodesXRefFile()) {
            emit messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
            return;
        }
    }
    displayParmsFile(fileInfo.absoluteFilePath());
    parmsWindow->setWindowTitle(tr("LDraw to Bricklink Design ID Cross-reference",
                                   "Edit/add LDraw to Bricklink design ID cross-reference"));
    parmsWindow->show();
}

void Gui::editLD2BLColorsXRef()
{
    QFileInfo fileInfo(QString("%1/extras/%2").arg(Preferences::lpubDataPath,VER_LPUB3D_LD2BLCOLORSXREF_FILE));
    if (!fileInfo.exists()) {
        if (!Annotations::exportLD2BLColorsXRefFile()) {
            emit messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
            return;
        }
    }
    displayParmsFile(fileInfo.absoluteFilePath());
    parmsWindow->setWindowTitle(tr("LDraw to Bricklink Color Code cross-reference",
                                   "Edit/add LDraw to Bricklink Color Code cross-reference"));
    parmsWindow->show();
}

void Gui::editBLColors()
{
    QFileInfo fileInfo(QString("%1/extras/%2").arg(Preferences::lpubDataPath,VER_LPUB3D_BLCOLORS_FILE));
    if (!fileInfo.exists()) {
        if (!Annotations::exportBLColorsFile()) {
            emit messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
            return;
        }
    }
    displayParmsFile(fileInfo.absoluteFilePath());
    parmsWindow->setWindowTitle(tr("Bricklink Color ID reference",
                                   "Edit/add Bricklink Color ID reference"));
    parmsWindow->show();
}

void Gui::editLD2RBCodesXRef()
{
    QFileInfo fileInfo(QString("%1/extras/%2").arg(Preferences::lpubDataPath,VER_LPUB3D_LD2RBCODESXREF_FILE));
    if (!fileInfo.exists()) {
        if (!Annotations::exportLD2RBCodesXRefFile()) {
            emit messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
            return;
        }
    }
    displayParmsFile(fileInfo.absoluteFilePath());
    parmsWindow->setWindowTitle(tr("LDraw to Rebrickable Design ID Cross-reference",
                                   "Edit/add LDraw to Rebrickable design ID cross-reference"));
    parmsWindow->show();
}

void Gui::editLD2RBColorsXRef()
{
    QFileInfo fileInfo(QString("%1/extras/%2").arg(Preferences::lpubDataPath,VER_LPUB3D_LD2RBCOLORSXREF_FILE));
    if (!fileInfo.exists()) {
        if (!Annotations::exportLD2RBColorsXRefFile()) {
            emit messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
            return;
        }
    }
    displayParmsFile(fileInfo.absoluteFilePath());
    parmsWindow->setWindowTitle(tr("LDraw to Rebrickable Color Code cross-reference",
                                   "Edit/add LDraw to Rebrickable Color Code cross-reference"));
    parmsWindow->show();
}

void Gui::editBLCodes()
{
    QFileInfo fileInfo(QString("%1/extras/%2").arg(Preferences::lpubDataPath,VER_LPUB3D_BLCODES_FILE));
    displayParmsFile(fileInfo.absoluteFilePath());
    parmsWindow->setWindowTitle(tr("Bricklink Codes reference",
                                   "Edit/add Bricklink Codes reference"));
    parmsWindow->show();
}

void Gui::viewLog()
{
    displayParmsFile(Preferences::logPath);
    parmsWindow->setWindowTitle(tr(VER_PRODUCTNAME_STR " log",VER_PRODUCTNAME_STR " logs"));
    parmsWindow->show();
}

void Gui::preferences()
{
    bool displayThemeRestart            = false;
    bool libraryChangeRestart           = false;
    bool defaultUnitsCompare            = Preferences::preferCentimeters;
    bool enableLDViewSCallCompare       = Preferences::enableLDViewSingleCall;
    bool enableLDViewSListCompare       = Preferences::enableLDViewSnaphsotList;
    bool displayAllAttributesCompare    = Preferences::displayAllAttributes;
    bool generateCoverPagesCompare      = Preferences::generateCoverPages;
    bool enableFadeStepsCompare         = Preferences::enableFadeSteps;
    bool fadeStepsUseColourCompare      = Preferences::fadeStepsUseColour;
    int fadeStepsOpacityCompare         = Preferences::fadeStepsOpacity;
    bool enableHighlightStepCompare     = Preferences::enableHighlightStep;
    bool enableImageMattingCompare      = Preferences::enableImageMatting;
    bool applyCALocallyCompare          = Preferences::applyCALocally;
    int  highlightStepLineWidthCompare  = Preferences::highlightStepLineWidth;
    bool highlightFirstStepCompare      = Preferences::highlightFirstStep;
    bool doNotShowPageProcessDlgCompare = Preferences::doNotShowPageProcessDlg;
    int  pageDisplayPauseCompare        = Preferences::pageDisplayPause;
    bool addLSynthSearchDirCompare      = Preferences::addLSynthSearchDir;
    bool archiveLSynthPartsCompare      = Preferences::archiveLSynthParts;
    bool perspectiveProjectionCompare   = Preferences::perspectiveProjection;
    bool saveOnUpdateCompare            = Preferences::saveOnUpdate;
    bool saveOnRedrawCompare            = Preferences::saveOnRedraw;
    bool loadLastOpenedFileCompare      = Preferences::loadLastOpenedFile;
    bool extendedSubfileSearchCompare   = Preferences::extendedSubfileSearch;
    bool povrayAutoCropCompare          = Preferences::povrayAutoCrop;
    bool showDownloadRedirectsCompare   = Preferences::showDownloadRedirects;
    int povrayRenderQualityCompare      = Preferences::povrayRenderQuality;
    int ldrawFilesLoadMsgsCompare       = Preferences::ldrawFilesLoadMsgs;
    bool lineParseErrorsCompare         = Preferences::lineParseErrors;
    bool showAnnotationMessagesCompare  = Preferences::showAnnotationMessages;
    QString altLDConfigPathCompare      = Preferences::altLDConfigPath;
    QString povFileGeneratorCompare     = Preferences::povFileGenerator;
    QString fadeStepsColourCompare      = Preferences::validFadeStepsColour;
    QString highlightStepColourCompare  = Preferences::highlightStepColour;
    QString ldrawPathCompare            = Preferences::ldrawLibPath;
    QString lgeoPathCompare             = Preferences::lgeoPath;
    QString preferredRendererCompare    = Preferences::preferredRenderer;
    QString displayThemeCompare         = Preferences::displayTheme;
    QString sceneBackgroundColorCompare = Preferences::sceneBackgroundColor;
    QString sceneGridColorCompare       = Preferences::sceneGridColor;
    QString sceneRulerTickColorCompare  = Preferences::sceneRulerTickColor;
    QString sceneGuideColorCompare      = Preferences::sceneGuideColor;
    QStringList ldSearchDirsCompare     = Preferences::ldSearchDirs;

    // Native POV file generation settings
    if (Preferences::preferredRenderer == RENDERER_POVRAY) {
        if (Preferences::povFileGenerator == RENDERER_NATIVE )
            TCUserDefaults::setIniFile(Preferences::nativeExportIni.toLatin1().constData());
        else
            TCUserDefaults::setIniFile(Preferences::ldviewPOVIni.toLatin1().constData());
    } else if (Preferences::preferredRenderer == RENDERER_LDVIEW) {
        TCUserDefaults::setIniFile(Preferences::ldviewIni.toLatin1().constData());
    }

    if (Preferences::getPreferences()) {

        Meta meta;
        page.meta = meta;

        QMessageBox box;
        box.setMinimumSize(40,20);
        box.setIcon (QMessageBox::Information);
        box.setDefaultButton   (QMessageBox::Ok);
        box.setStandardButtons (QMessageBox::Ok);

        bool defaultUnitsChanged           = Preferences::preferCentimeters                      != defaultUnitsCompare;
        bool rendererChanged               = QString(Preferences::preferredRenderer).toLower()   != preferredRendererCompare.toLower();
        bool enableFadeStepsChanged        = Preferences::enableFadeSteps                        != enableFadeStepsCompare;
        bool fadeStepsUseColourChanged     = Preferences::fadeStepsUseColour                     != fadeStepsUseColourCompare;
        bool fadeStepsColourChanged        = QString(Preferences::validFadeStepsColour).toLower()!= fadeStepsColourCompare.toLower();
        bool fadeStepsOpacityChanged       = Preferences::fadeStepsOpacity                       != fadeStepsOpacityCompare;
        bool enableHighlightStepChanged    = Preferences::enableHighlightStep                    != enableHighlightStepCompare;
        bool highlightStepColorChanged     = QString(Preferences::highlightStepColour).toLower() != highlightStepColourCompare.toLower();
        bool highlightStepLineWidthChanged = Preferences::highlightStepLineWidth                 != highlightStepLineWidthCompare;
        bool highlightFirstStepChanged     = Preferences::highlightFirstStep                     != highlightFirstStepCompare;
        bool enableImageMattingChanged     = Preferences::enableImageMatting                     != enableImageMattingCompare;
        bool perspectiveProjectionChanged  = Preferences::perspectiveProjection                  != perspectiveProjectionCompare;
        bool saveOnRedrawChanged           = Preferences::saveOnRedraw                           != saveOnRedrawCompare;
        bool saveOnUpdateChanged           = Preferences::saveOnUpdate                           != saveOnUpdateCompare;
        bool applyCALocallyChanged         = Preferences::applyCALocally                         != applyCALocallyCompare;
        bool enableLDViewSCallChanged      = Preferences::enableLDViewSingleCall                 != enableLDViewSCallCompare;
        bool enableLDViewSListChanged      = Preferences::enableLDViewSnaphsotList               != enableLDViewSListCompare;
        bool displayAttributesChanged      = Preferences::displayAllAttributes                   != displayAllAttributesCompare;
        bool generateCoverPagesChanged     = Preferences::generateCoverPages                     != generateCoverPagesCompare;
        bool pageDisplayPauseChanged       = Preferences::pageDisplayPause                       != pageDisplayPauseCompare;
        bool doNotShowPageProcessDlgChanged= Preferences::doNotShowPageProcessDlg                != doNotShowPageProcessDlgCompare;
        bool povFileGeneratorChanged       = Preferences::povFileGenerator                       != povFileGeneratorCompare;
        bool altLDConfigPathChanged        = Preferences::altLDConfigPath                        != altLDConfigPathCompare;
        bool addLSynthSearchDirChanged     = Preferences::addLSynthSearchDir                     != addLSynthSearchDirCompare;
        bool archiveLSynthPartsChanged     = Preferences::archiveLSynthParts                     != archiveLSynthPartsCompare;
        bool ldrawPathChanged              = QString(Preferences::ldrawLibPath).toLower()        != ldrawPathCompare.toLower();
        bool lgeoPathChanged               = QString(Preferences::lgeoPath).toLower()            != lgeoPathCompare.toLower();
        bool displayThemeChanged           = Preferences::displayTheme.toLower()                 != displayThemeCompare.toLower();
        bool loadLastOpenedFileChanged     = Preferences::loadLastOpenedFile                     != loadLastOpenedFileCompare;
        bool extendedSubfileSearchChanged  = Preferences::extendedSubfileSearch                  != extendedSubfileSearchCompare;
        bool povrayAutoCropChanged         = Preferences::povrayAutoCrop                         != povrayAutoCropCompare;
        bool povrayRenderQualityChanged    = Preferences::povrayRenderQuality                    != povrayRenderQualityCompare;
        bool showDownloadRedirectsChanged  = Preferences::showDownloadRedirects                  != showDownloadRedirectsCompare;

        bool sceneBackgroundColorChanged   = Preferences::sceneBackgroundColor.toLower()         != sceneBackgroundColorCompare.toLower();
        bool sceneGridColorChanged         = Preferences::sceneGridColor.toLower()               != sceneGridColorCompare.toLower();
        bool sceneRulerTickColorChanged    = Preferences::sceneRulerTickColor.toLower()          != sceneRulerTickColorCompare.toLower();
        bool sceneGuideColorChanged        = Preferences::sceneGuideColor.toLower()              != sceneGuideColorCompare.toLower();
        bool ldrawFilesLoadMsgsChanged     = Preferences::ldrawFilesLoadMsgs                     != ldrawFilesLoadMsgsCompare;
        bool ldSearchDirsChanged           = Preferences::ldSearchDirs                           != ldSearchDirsCompare;

        bool lineParseErrorsChanged        = Preferences::lineParseErrors                        != lineParseErrorsCompare;
        bool showAnnotationMessagesChanged = Preferences::showAnnotationMessages                 != showAnnotationMessagesCompare;

        if (defaultUnitsChanged     )
                    emit messageSig(LOG_INFO,QString("Default units changed to %1").arg(Preferences::preferCentimeters? "Centimetres" : "Inches"));

        if (ldrawPathChanged) {
            emit messageSig(LOG_INFO,QString("LDraw Library path changed from %1 to %2")
                            .arg(ldrawPathCompare)
                            .arg(Preferences::ldrawLibPath));
            if (Preferences::validLDrawLibrary != Preferences::validLDrawLibraryChange) {
                libraryChangeRestart = true;
                emit messageSig(LOG_INFO,QString("LDraw parts library changed from %1 to %2")
                                .arg(Preferences::validLDrawLibrary)
                                .arg(Preferences::validLDrawLibraryChange));
                box.setText (QString("%1 will restart to properly load the %2 parts library.")
                                     .arg(VER_PRODUCTNAME_STR).arg(Preferences::validLDrawLibraryChange));
                box.exec();
            }
        }

        if (ldrawFilesLoadMsgsChanged     )
                    emit messageSig(LOG_INFO,QString("LdrawFiles Load message dialogue set to %1").arg(
                        Preferences::ldrawFilesLoadMsgs == NEVER_SHOW ? "Never Show" :
                        Preferences::ldrawFilesLoadMsgs == SHOW_ERROR ? "Show Error" :
                        Preferences::ldrawFilesLoadMsgs == SHOW_WARNING ? "Show Warning" :
                        Preferences::ldrawFilesLoadMsgs == SHOW_MESSAGE ? "Show Message" :
                        "Always Show"));

        if (ldSearchDirsChanged) {
            emit messageSig(LOG_INFO,QString("LDraw search directories has changed"));
            emit messageSig(LOG_INFO,QString("Previous Directories:"));
            for(int i =0; i < ldSearchDirsCompare.size(); i++) {
                emit messageSig(LOG_INFO,QString("    - %1. %2").arg(i).arg(QDir::toNativeSeparators(ldSearchDirsCompare.at(i))));
            }
            emit messageSig(LOG_INFO,QString("Updated Directories:"));
            for(int i =0; i < Preferences::ldSearchDirs.size(); i++) {
                emit messageSig(LOG_INFO,QString("    - %1. %2").arg(i).arg(QDir::toNativeSeparators(Preferences::ldSearchDirs.at(i))));
            }
            gui->partWorkerLDSearchDirs.updateLDSearchDirs(true);
        }

        if (lgeoPathChanged && !ldrawPathChanged)
            emit messageSig(LOG_INFO,QString("LGEO path preference changed from %1 to %2")
                            .arg(lgeoPathCompare)
                            .arg(Preferences::lgeoPath));

        if (sceneBackgroundColorChanged)
            emit messageSig(LOG_INFO,QString("Scene Background Color changed from %1 to %2")
                            .arg(sceneBackgroundColorCompare)
                            .arg(Preferences::sceneBackgroundColor));

        if (sceneRulerTickColorChanged)
            emit messageSig(LOG_INFO,QString("Scene Ruler Tick Color changed from %1 to %2")
                            .arg(sceneRulerTickColorCompare)
                            .arg(Preferences::sceneRulerTickColor));

        if (sceneGridColorChanged)
            emit messageSig(LOG_INFO,QString("Scene Grid Color changed from %1 to %2")
                            .arg(sceneGridColorCompare)
                            .arg(Preferences::sceneGridColor));

        if (showDownloadRedirectsChanged)
                    emit messageSig(LOG_INFO,QString("Show download redirects is %1").arg(Preferences::showDownloadRedirects? "ON" : "OFF"));

        if (sceneGuideColorChanged && !ldrawPathChanged)
            emit messageSig(LOG_INFO,QString("Scene Guide Color changed from %1 to %2")
                            .arg(sceneGuideColorCompare)
                            .arg(Preferences::sceneGuideColor));

        if (enableFadeStepsChanged) {
            emit messageSig(LOG_INFO,QString("Fade Previous Steps is %1.").arg(Preferences::enableFadeSteps ? "ON" : "OFF"));
            if (Preferences::enableFadeSteps && !ldrawColourParts.ldrawColorPartsIsLoaded()) {
                QString result;
                if (!LDrawColourParts::LDrawColorPartsLoad(result)){
                    QString message = QString("Could not open %1 LDraw color parts file [%2], Error: %3")
                                      .arg(Preferences::validLDrawLibrary).arg(Preferences::ldrawColourPartsFile).arg(result);
                    emit messageSig(LOG_ERROR, message);
                }
            }
        }

        if (fadeStepsUseColourChanged && Preferences::enableFadeSteps)
            emit messageSig(LOG_INFO,QString("Use Global Fade Color is %1").arg(Preferences::fadeStepsUseColour ? "ON" : "OFF"));

        if (fadeStepsOpacityChanged && Preferences::enableFadeSteps)
            emit messageSig(LOG_INFO,QString("Fade Step Transparency changed from %1 to %2 percent")
                            .arg(fadeStepsOpacityCompare)
                            .arg(Preferences::fadeStepsOpacity));

        if (fadeStepsColourChanged && Preferences::enableFadeSteps && Preferences::fadeStepsUseColour)
            emit messageSig(LOG_INFO,QString("Fade Step Color preference changed from %1 to %2")
                            .arg(fadeStepsColourCompare.replace("_"," "))
                            .arg(QString(Preferences::validFadeStepsColour).replace("_"," ")));

        if (enableHighlightStepChanged)
            emit messageSig(LOG_INFO,QString("Highlight Current Step is %1.").arg(Preferences::enableHighlightStep ? "ON" : "OFF"));

        if (highlightFirstStepChanged     )
                    emit messageSig(LOG_INFO,QString("Highlight First Step is %1").arg(Preferences::highlightFirstStep ? "ON" : "OFF"));

        if (loadLastOpenedFileChanged)
                    emit messageSig(LOG_INFO,QString("Load Last Opened File is %1").arg(Preferences::loadLastOpenedFile ? "ON" : "OFF"));

        if (extendedSubfileSearchChanged     )
                    emit messageSig(LOG_INFO,QString("Extended Subfile Search is %1").arg(Preferences::extendedSubfileSearch ? "ON" : "OFF"));

        if (povrayRenderQualityChanged)
                    emit messageSig(LOG_INFO,QString("Povray Render Quality changed from %1 to %2")
                                    .arg(povrayRenderQualityCompare == 0 ? "High" :
                                         povrayRenderQualityCompare == 1 ? "Medium" : "Low")
                                    .arg(Preferences::povrayRenderQuality == 0 ? "High" :
                                         Preferences::povrayRenderQuality == 1 ? "Medium" : "Low"));
        povrayRenderQualityChanged = (povrayRenderQualityChanged && Preferences::preferredRenderer == RENDERER_POVRAY);

        if (povrayAutoCropChanged)
                    emit messageSig(LOG_INFO,QString("Povray AutoCrop is %1").arg(Preferences::povrayAutoCrop ? "ON" : "OFF"));

        if (highlightStepLineWidthChanged && Preferences::enableHighlightStep)
            emit messageSig(LOG_INFO,QString("Highlight Step line width changed from %1 to %2")
                            .arg(highlightStepLineWidthCompare)
                            .arg(Preferences::highlightStepLineWidth));

        if (highlightStepColorChanged && Preferences::enableHighlightStep)
            emit messageSig(LOG_INFO,QString("Highlight Step Color preference changed from %1 to %2")
                            .arg(highlightStepColourCompare)
                            .arg(Preferences::highlightStepColour));

        if (generateCoverPagesChanged)
            emit messageSig(LOG_INFO,QString("Generate Cover Pages preference is %1").arg(Preferences::generateCoverPages ? "ON" : "OFF"));

        if (perspectiveProjectionChanged) {
           lcSetProfileInt(LC_PROFILE_NATIVE_PROJECTION, Preferences::perspectiveProjection ? 0 : 1);
           gApplication->mPreferences.LoadDefaults();
           emit messageSig(LOG_INFO,QString("Projection set to %1").arg(Preferences::perspectiveProjection ? "Perspective" : "Orthographic"));
        }

        if (saveOnRedrawChanged     )
                    emit messageSig(LOG_INFO,QString("Save On Redraw is %1").arg(Preferences::saveOnRedraw? "ON" : "OFF"));

        if (saveOnUpdateChanged     )
                    emit messageSig(LOG_INFO,QString("Save On Update is %1").arg(Preferences::saveOnUpdate? "ON" : "OFF"));

        if (pageDisplayPauseChanged)
            emit messageSig(LOG_INFO,QString("Continuous process page display pause changed from %1 to %2")
                            .arg(highlightStepLineWidthCompare)
                            .arg(Preferences::pageDisplayPause));

        if (addLSynthSearchDirChanged)
                    emit messageSig(LOG_INFO,QString("Add LSynth Search Directory is %1").arg(Preferences::addLSynthSearchDir? "ON" : "OFF"));

        if (archiveLSynthPartsChanged)
                    emit messageSig(LOG_INFO,QString("Archive LSynth Parts is %1").arg(Preferences::archiveLSynthParts? "ON" : "OFF"));

        if ((addLSynthSearchDirChanged || archiveLSynthPartsChanged) && Preferences::archiveLSynthParts)
                loadLDSearchDirParts();

        if (doNotShowPageProcessDlgChanged)
            emit messageSig(LOG_INFO,QString("Show continuous page process options dialog is %1.").arg(Preferences::doNotShowPageProcessDlg ? "ON" : "OFF"));

        if ((((fadeStepsColourChanged && Preferences::fadeStepsUseColour) ||
              fadeStepsUseColourChanged || fadeStepsOpacityChanged) &&
             Preferences::enableFadeSteps && !enableFadeStepsChanged) ||
                ((highlightStepColorChanged || highlightStepLineWidthChanged) &&
                 Preferences::enableHighlightStep && !enableHighlightStepChanged))
            clearCustomPartCache(true);    // true = silent

        if (enableImageMattingChanged && Preferences::enableImageMatting)
            emit messageSig(LOG_INFO,QString("Enable image matting is %1").arg(Preferences::enableImageMatting ? "ON" : "OFF"));

        if (applyCALocallyChanged)
            emit messageSig(LOG_INFO,QString("Apply camera angles locally is %1").arg(Preferences::applyCALocally ? "ON" : "OFF"));

        if (enableLDViewSCallChanged)
            emit messageSig(LOG_INFO,QString("Enable LDView Single Call is %1").arg(Preferences::enableLDViewSingleCall ? "ON" : "OFF"));

        if (enableLDViewSListChanged)
            emit messageSig(LOG_INFO,QString("Enable LDView Snapshots List is %1").arg(Preferences::enableLDViewSnaphsotList ? "ON" : "OFF"));

        if (rendererChanged) {
            emit messageSig(LOG_INFO,QString("Renderer preference changed from %1 to %2")
                            .arg(preferredRendererCompare)
                            .arg(QString("%1%2").arg(Preferences::preferredRenderer)
                                                .arg(Preferences::preferredRenderer == RENDERER_POVRAY ? QString("(POV file generator is %1)").arg(Preferences::povFileGenerator) :
                                                     Preferences::preferredRenderer == RENDERER_LDVIEW ? Preferences::enableLDViewSingleCall ? "(Single Call)" : "" : "")));

            Render::setRenderer(Preferences::preferredRenderer);
            if (Preferences::preferredRenderer == RENDERER_LDGLITE)
                partWorkerLdgLiteSearchDirs.populateLdgLiteSearchDirs();
        }

        if (povFileGeneratorChanged)
            emit messageSig(LOG_INFO,QString("POV file generation renderer changed from %1 to %2")
                            .arg(povFileGeneratorCompare)
                            .arg(Preferences::povFileGenerator));

        if (altLDConfigPathChanged) {
            emit messageSig(LOG_INFO,QString("Use Alternate LDConfig (Restart Required) %1.").arg(Preferences::altLDConfigPath));
            box.setText (QString("%1 will restart to properly load the alternate LDConfig file.").arg(VER_PRODUCTNAME_STR));
            box.exec();
        }

        if (lineParseErrorsChanged     )
                    emit messageSig(LOG_INFO,QString("Show Parse Errors is %1").arg(Preferences::lineParseErrors? "ON" : "OFF"));

        if (showAnnotationMessagesChanged     )
                    emit messageSig(LOG_INFO,QString("Show Parse Errors is %1").arg(Preferences::showAnnotationMessages? "ON" : "OFF"));


        if (displayThemeChanged) {
            if( Preferences::themeAutoRestart) {
                displayThemeRestart = true;
            }
            else
            if (displayThemeChanged) {
                box.setStandardButtons (QMessageBox::Ok | QMessageBox::Close);
                box.setText (QString("You must close and restart %1 to fully configure the Theme.\n"
                                     "Editor syntax highlighting will update the next time you start %1")
                                     .arg(QString::fromLatin1(VER_PRODUCTNAME_STR)));
                box.setInformativeText (QString("Click \"OK\" to close and restart %1 or \"Close\" set the Theme without restart.\n\n"
                                                "You can suppress this message in Preferences, Themes")
                                                .arg(QString::fromLatin1(VER_PRODUCTNAME_STR)));
                if (box.exec() == QMessageBox::Ok) {
                    displayThemeRestart = true;
                }
            }
            loadTheme(displayThemeRestart);
        }

        if (!getCurFile().isEmpty()) {
            if (defaultUnitsChanged           ||
                enableFadeStepsChanged        ||
                altLDConfigPathChanged        ||
                fadeStepsColourChanged        ||
                fadeStepsUseColourChanged     ||
                fadeStepsOpacityChanged       ||
                enableHighlightStepChanged    ||
                highlightStepColorChanged     ||
                highlightStepLineWidthChanged ||
                highlightFirstStepChanged     ||
                rendererChanged               ||
                enableLDViewSCallChanged      ||
                enableLDViewSListChanged      ||
                displayAttributesChanged      ||
                povFileGeneratorChanged       ||
                enableImageMattingChanged     ||
                perspectiveProjectionChanged  ||
                povrayRenderQualityChanged    ||
                generateCoverPagesChanged){
                clearAndReloadModelFile();
            }
        }

        // set logging options
        using namespace QsLogging;
        Logger& logger = Logger::instance();
        if (Preferences::logging) {
            if (Preferences::logLevels){

                logger.setLoggingLevels();
                logger.setDebugLevel(Preferences::debugLevel);
                logger.setTraceLevel(Preferences::traceLevel);
                logger.setNoticeLevel(Preferences::noticeLevel);
                logger.setInfoLevel(Preferences::infoLevel);
                logger.setStatusLevel(Preferences::statusLevel);
                logger.setErrorLevel(Preferences::errorLevel);
                logger.setFatalLevel(Preferences::fatalLevel);

            } else if (Preferences::logLevel){

                bool ok;
                Level logLevel = logger.fromLevelString(Preferences::loggingLevel,&ok);
                if (!ok)
                {
                    QString Message = QString("Failed to set log level %1.\n"
                                                      "Logging is off - level set to OffLevel")
                            .arg(Preferences::loggingLevel);
                    if (Preferences::modeGUI)
                        QMessageBox::critical(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR), Message);
                    else
                        fprintf(stderr, "%s", Message.toLatin1().constData());
                }
                logger.setLoggingLevel(logLevel);
            }

            logger.setIncludeLogLevel(Preferences::includeLogLevel);
            logger.setIncludeTimestamp(Preferences::includeTimestamp);
            logger.setIncludeLineNumber(Preferences::includeLineNumber);
            logger.setIncludeFileName(Preferences::includeFileName);
            logger.setIncludeFunctionInfo(Preferences::includeFunction);

        } else {
            logger.setLoggingLevel(OffLevel);
        }

        if (displayThemeRestart || altLDConfigPathChanged || libraryChangeRestart) {
            restartApplication(libraryChangeRestart);
        }

        if (sceneBackgroundColorChanged ||
            sceneGridColorChanged       ||
            sceneRulerTickColorChanged  ||
            sceneGuideColorChanged)
        {
            KpageView->setSceneTheme();
            if (sceneGridColorChanged)
                reloadCurrentPage();
        }
    }
}


/*******************************************************************************
 *
 * This is all the initialization stuff.  It is used once when the program
 * starts up
 *
 ******************************************************************************/

Gui::Gui()
{
    emit Application::instance()->splashMsgSig(QString("25% - %1 window defaults loading...").arg(VER_PRODUCTNAME_STR));

    Preferences::lgeoPreferences();
    Preferences::rendererPreferences(SkipExisting);
    Preferences::viewerPreferences();
    Preferences::publishingPreferences();
    Preferences::exportPreferences();

    displayPageNum = 1;

    processOption                   = EXPORT_ALL_PAGES;
    exportMode                      = EXPORT_PDF;
    pageRangeText                   = "1";
    exportPixelRatio                = 1.0;
    resetCache                      = false;
    m_updateViewer                  = true;
    m_previewDialog                 = false;
    m_partListCSIFile               = false;
    m_exportingContent              = false;
    m_exportingObjects              = false;
    m_contPageProcessing            = false;
    nextPageContinuousIsRunning     = false;
    previousPageContinuousIsRunning = false;

    buildModRange     = { 0, -1, 0 };
    mStepRotation     = lcVector3(0.0f, 0.0f, 0.0f);
    mRotStepAngleX    = 0.0f;
    mRotStepAngleY    = 0.0f;
    mRotStepAngleZ    = 0.0f;
    mRotStepTransform = QString();
    mPliIconsPath.clear();

    selectedItemObj   = UndefinedObj;
    mViewerZoomLevel  = 50;

    mHttpManager = new lcHttpManager(this);

    editWindow    = new EditWindow(this);         // remove inheritance 'this' to independently manage window
    editModeWindow= new EditWindow(nullptr,true); // true = this is a model file edit window
    parmsWindow   = new ParmsWindow();

    KpageScene    = new LGraphicsScene(this);
    KpageView     = new LGraphicsView(KpageScene);
    KpageView->pageBackgroundItem = nullptr;
    KpageView->setSceneTheme();
    KpageView->setRenderHints(QPainter::Antialiasing |
                              QPainter::TextAntialiasing |
                              QPainter::SmoothPixmapTransform);
    KpageView->setResolution(resolution());

    setCentralWidget(KpageView);

    mpdCombo = new QComboBox(this);
    mpdCombo->setMinimumContentsLength(25);
    mpdCombo->setInsertPolicy(QComboBox::InsertAtBottom);
    mpdCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    mpdCombo->setToolTip(tr("Current Submodel"));
    mpdCombo->setStatusTip("Use dropdown to select submodel");
    connect(mpdCombo,SIGNAL(activated(int)),
            this,    SLOT(mpdComboChanged(int)));

    setGoToPageCombo = new QComboBox(this);
    setGoToPageCombo->setToolTip(tr("Current Page"));
    setGoToPageCombo->setMinimumContentsLength(10);
    setGoToPageCombo->setInsertPolicy(QComboBox::InsertAtBottom);
    setGoToPageCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    setGoToPageCombo->setStatusTip("Use dropdown to select page");
    connect(setGoToPageCombo,SIGNAL(activated(int)),
            this,            SLOT(setGoToPage(int)));

    undoStack = new QUndoStack();
    macroNesting = 0;

    lpubAlert = new LPubAlert();

    connect(lpubAlert,      SIGNAL(messageSig(LogType,QString)),
            this,           SLOT(statusMessage(LogType,QString)));

    connect(this,           SIGNAL(messageSig(LogType,QString)),
            this,           SLOT(statusMessage(LogType,QString)));

    connect(mHttpManager,   SIGNAL(DownloadFinished(lcHttpReply*)),
            this,           SLOT(DownloadFinished(lcHttpReply*)));

    connect(this,           SIGNAL(setExportingSig(bool)),
            this,           SLOT(  setExporting(   bool)));

    connect(this,           SIGNAL(setExportingObjectsSig(bool)),
            this,           SLOT(  setExportingObjects(   bool)));

    connect(this,           SIGNAL(setContinuousPageSig(bool)),
            this,           SLOT(  setContinuousPage(   bool)));

    connect(this,           SIGNAL(displayFileSig(LDrawFile *, const QString &)),
            editWindow,     SLOT(  displayFile   (LDrawFile *, const QString &)));

    connect(this,           SIGNAL(displayParmsFileSig(const QString &)),
            parmsWindow,    SLOT( displayParmsFile   (const QString &)));

    connect(this,           SIGNAL(showLineSig(int)),
            editWindow,     SLOT(  showLine(   int)));

    connect(this,           SIGNAL(highlightSelectedLinesSig(QVector<int> &)),
            editWindow,     SLOT(  highlightSelectedLines(   QVector<int> &)));

    connect(editWindow,     SIGNAL(SelectedPartLinesSig(QVector<TypeLine>&,PartSource)),
            this,           SLOT(SelectedPartLines(QVector<TypeLine>&,PartSource)));

    connect(editWindow,     SIGNAL(redrawSig()),
            this,           SLOT(  clearAndReloadModelFile()));

    connect(editWindow,     SIGNAL(updateSig()),
            this,           SLOT(  reloadCurrentPage()));

    connect(this,           SIGNAL(disableEditorActionsSig()),
            editWindow,     SLOT(  disableActions()));

    connect(editWindow,     SIGNAL(contentsChange(const QString &,int,int,const QString &)),
            this,           SLOT(  contentsChange(const QString &,int,int,const QString &)));

    connect(undoStack,      SIGNAL(canRedoChanged(bool)),
            this,           SLOT(  canRedoChanged(bool)));
    connect(undoStack,      SIGNAL(canUndoChanged(bool)),
            this,           SLOT(  canUndoChanged(bool)));
    connect(undoStack,      SIGNAL(cleanChanged(bool)),
            this,           SLOT(  cleanChanged(bool)));
    connect(undoStack,      SIGNAL(cleanChanged(bool)),
            editWindow,     SLOT(  updateDisabled(bool)));

    connect(editWindow,     SIGNAL(updateDisabledSig(bool)),
            editModeWindow, SLOT(  updateDisabled(bool)));

    // edit model file
    connect(this,           SIGNAL(displayModelFileSig(LDrawFile *, const QString &)),
            editModeWindow, SLOT(  displayFile   (LDrawFile *, const QString &)));
    connect(editModeWindow, SIGNAL(refreshModelFileSig()),
            this,           SLOT(  refreshModelFile()));
    connect(editModeWindow, SIGNAL(redrawSig()),
            this,           SLOT(  clearAndRedrawModelFile()));
    connect(editModeWindow, SIGNAL(updateSig()),
            this,           SLOT(  reloadCurrentModelFile()));
    connect(this,           SIGNAL(disableEditorActionsSig()),
            editModeWindow, SLOT(  disableActions()));

    connect(editModeWindow, SIGNAL(contentsChange(const QString &,int,int,const QString &)),
            this,           SLOT(  contentsChange(const QString &,int,int,const QString &)));
    connect(undoStack,      SIGNAL(cleanChanged(bool)),
            editModeWindow, SLOT(  updateDisabled(bool)));

    progressLabel = new QLabel(this);
    progressLabel->setMinimumWidth(200);
    progressBar = new QProgressBar();
    progressBar->setMaximumWidth(300);

    progressLabelPerm = new QLabel();
    progressLabelPerm->setMinimumWidth(200);
    progressBarPerm = new QProgressBar();
    progressBarPerm->setMaximumWidth(300);

    m_progressDialog         = new ProgressDialog();
    m_progressDlgProgressBar = m_progressDialog->findChild<QProgressBar*>("progressDlgProgressBar");
    m_progressDlgMessageLbl  = m_progressDialog->findChild<QLabel*>("progressDlgMessageLbl");

    connect (m_progressDialog, SIGNAL (cancelClicked()),   this, SLOT (cancelExporting()));

    connect(this, SIGNAL(progressBarInitSig()),            this, SLOT(progressBarInit()));
    connect(this, SIGNAL(progressMessageSig(QString)),     this, SLOT(progressBarSetText(QString)));
    connect(this, SIGNAL(progressRangeSig(int,int)),       this, SLOT(progressBarSetRange(int,int)));
    connect(this, SIGNAL(progressSetValueSig(int)),        this, SLOT(progressBarSetValue(int)));
    connect(this, SIGNAL(progressResetSig()),              this, SLOT(progressBarReset()));
    connect(this, SIGNAL(progressStatusRemoveSig()),       this, SLOT(progressStatusRemove()));

    connect(this, SIGNAL(progressBarPermInitSig()),        this, SLOT(progressBarPermInit()));
    connect(this, SIGNAL(progressPermMessageSig(QString)), this, SLOT(progressBarPermSetText(QString)));
    connect(this, SIGNAL(progressPermRangeSig(int,int)),   this, SLOT(progressBarPermSetRange(int,int)));
    connect(this, SIGNAL(progressPermSetValueSig(int)),    this, SLOT(progressBarPermSetValue(int)));
    connect(this, SIGNAL(progressPermResetSig()),          this, SLOT(progressBarPermReset()));
    connect(this, SIGNAL(progressPermStatusRemoveSig()),   this, SLOT(progressPermStatusRemove()));

#ifdef WATCHER
    connect(&watcher,       SIGNAL(fileChanged(const QString &)),
             this,          SLOT(  fileChanged(const QString &)));
    changeAccepted = true;
#endif

    gui = this;

    KpageView->fitMode = FitVisible;
}

Gui::~Gui()
{
  delete KpageScene;
  delete KpageView;
  delete editWindow;
  delete parmsWindow;
  delete undoStack;

  delete progressBar;
  delete m_progressDialog;
  delete progressLabelPerm;
  delete progressBarPerm;
}

void Gui::closeEvent(QCloseEvent *event)
{

  writeSettings();

  emit requestEndThreadNowSig();

  if (parmsWindow->isVisible())
    parmsWindow->close();

  if (maybeSave()) {
      event->accept();
    } else {
      event->ignore();
    }
}

void Gui::workerJobResult(int value){
    m_workerJobResult = value;
}

void Gui::getRequireds(){
    // Check preferred renderer value is set before setting Renderer class
    Preferences::getRequireds();
    Render::setRenderer(Preferences::preferredRenderer);
}

void Gui::initialize()
{

  emit Application::instance()->splashMsgSig(QString("85% - %1 initialization...").arg(VER_PRODUCTNAME_STR));

  connect(this,        SIGNAL(loadFileSig(QString)),                     this,        SLOT(loadFile(QString)));
  connect(this,        SIGNAL(processCommandLineSig()),                  this,        SLOT(processCommandLine()));
  connect(this,        SIGNAL(setExportingSig(bool)),                    this,        SLOT(deployExportBanner(bool)));
  connect(this,        SIGNAL(setPliIconPathSig(QString&,QString&)),     this,        SLOT(setPliIconPath(QString&,QString&)));
  connect(this,        SIGNAL(enable3DActionsSig()),                     this,        SLOT(Enable3DActions()));
  connect(this,        SIGNAL(disable3DActionsSig()),                    this,        SLOT(Disable3DActions()));

  connect(this,        SIGNAL(setExportingSig(bool)),                    gMainWindow, SLOT(Halt3DViewer(bool)));
  connect(this,        SIGNAL(enable3DActionsSig()),                     gMainWindow, SLOT(Enable3DActions()));
  connect(this,        SIGNAL(disable3DActionsSig()),                    gMainWindow, SLOT(Disable3DActions()));
  connect(this,        SIGNAL(updateAllViewsSig()),                      gMainWindow, SLOT(UpdateAllViews()));
  connect(this,        SIGNAL(clearViewerWindowSig()),                   gMainWindow, SLOT(NewProject()));
  connect(this,        SIGNAL(setSelectedPiecesSig(QVector<int>&)),      gMainWindow, SLOT(SetSelectedPieces(QVector<int>&)));

  connect(gMainWindow, SIGNAL(SetRotStepMeta()),                         this,        SLOT(SetRotStepMeta()));
  connect(gMainWindow, SIGNAL(SetRotStepAngleX(float,bool)),             this,        SLOT(SetRotStepAngleX(float,bool)));
  connect(gMainWindow, SIGNAL(SetRotStepAngleY(float,bool)),             this,        SLOT(SetRotStepAngleY(float,bool)));
  connect(gMainWindow, SIGNAL(SetRotStepAngleZ(float,bool)),             this,        SLOT(SetRotStepAngleZ(float,bool)));
  connect(gMainWindow, SIGNAL(SetRotStepTransform(QString&,bool)),       this,        SLOT(SetRotStepTransform(QString&,bool)));
  connect(gMainWindow, SIGNAL(GetRotStepMeta()),                         this,        SLOT(GetRotStepMeta()));
  connect(gMainWindow, SIGNAL(updateSig()),                              this,        SLOT(loadUpdatedImages()));
  connect(gMainWindow, SIGNAL(SetActiveModelSig(const QString&,bool)),   this,        SLOT(SetActiveModel(const QString&,bool)));
  connect(gMainWindow, SIGNAL(SelectedPartLinesSig(QVector<TypeLine>&,PartSource)),this,SLOT(SelectedPartLines(QVector<TypeLine>&,PartSource)));

/* Moved to PartWorker::ldsearchDirPreferences()  */
//  if (Preferences::preferredRenderer == RENDERER_LDGLITE)
//      partWorkerLdgLiteSearchDirs.populateLdgLiteSearchDirs();

  emit Application::instance()->splashMsgSig(QString("90% - %1 widgets loading...").arg(VER_PRODUCTNAME_STR));

  createActions();
  createMenus();
  createToolBars();
  createStatusBar();
  createDockWindows();
  toggleLCStatusBar(true);

  emit Application::instance()->splashMsgSig(QString("95% - LDraw colors loading..."));

  LDrawColor::LDrawColorInit();

  emit disable3DActionsSig();
  setCurrentFile("");
  readSettings();
  updateOpenWithActions();

  // scene item z direction
  if (soMap.size() == 0) {
      soMap[AssemAnnotationObj]       = QString("CSI_ANNOTATION");       //  0 CsiAnnotationType
      soMap[AssemAnnotationPartObj]   = QString("CSI_ANNOTATION_PART");  //  1 CsiPartType
      soMap[AssemObj]                 = QString("ASSEM");                //  2 CsiType
      soMap[CalloutBackgroundObj]     = QString("CALLOUT");              //  4 CalloutType
      soMap[CalloutInstanceObj]       = QString("CALLOUT_INSTANCE");     //  5
      soMap[CalloutPointerObj]        = QString("CALLOUT_POINTER");      //  6
      soMap[CalloutUnderpinningObj]   = QString("CALLOUT_UNDERPINNING"); //  7
      soMap[DividerBackgroundObj]     = QString("DIVIDER");              //  8
      soMap[DividerObj]               = QString("DIVIDER_ITEM");         //  9
      soMap[DividerLineObj]           = QString("DIVIDER_LINE");         // 10
      soMap[DividerPointerObj]        = QString("DIVIDER_POINTER");      // 11 DividerPointerType
      soMap[PointerGrabberObj]        = QString("POINTER_GRABBER");      // 12
      soMap[PliGrabberObj]            = QString("PLI_GRABBER");          // 13
      soMap[SubmodelGrabberObj]       = QString("SUBMODEL_GRABBER");     // 14
      soMap[InsertPixmapObj]          = QString("PICTURE");              // 15
      soMap[InsertTextObj]            = QString("TEXT");                 // 16 TextType
      soMap[MultiStepBackgroundObj]   = QString("MULTI_STEP");           // 17 StepGroupType
      soMap[MultiStepsBackgroundObj]  = QString("MULTI_STEPS");          // 18
      soMap[PageAttributePixmapObj]   = QString("ATTRIBUTE_PIXMAP");     // 19
      soMap[PageAttributeTextObj]     = QString("ATTRIBUTE_TEXT");       // 20
      soMap[PageBackgroundObj]        = QString("PAGE");                 // 21 PageType
      soMap[PageNumberObj]            = QString("PAGE_NUMBER");          // 22 PageNumberType
      soMap[PagePointerObj]           = QString("PAGE_POINTER");         // 23 PagePointerType
      soMap[PartsListAnnotationObj]   = QString("PLI_ANNOTATION");       // 24
      soMap[PartsListBackgroundObj]   = QString("PLI");                  // 25 PartsListType
      soMap[PartsListInstanceObj]     = QString("PLI_INSTANCE");         // 26
      soMap[PointerFirstSegObj]       = QString("POINTER_SEG_FIRST");    // 27
      soMap[PointerHeadObj]           = QString("POINTER_HEAD");         // 28
      soMap[PointerSecondSegObj]      = QString("POINTER_SEG_SECOND");   // 29
      soMap[PointerThirdSegObj]       = QString("POINTER_SEG_THIRD");    // 30
      soMap[RotateIconBackgroundObj]  = QString("ROTATE_ICON");          // 31 RotateIconType
      soMap[StepNumberObj]            = QString("STEP_NUMBER");          // 32 StepNumberType
      soMap[SubModelBackgroundObj]    = QString("SUBMODEL_DISPLAY");     // 33 SubModelType
      soMap[SubModelInstanceObj]      = QString("SUBMODEL_INSTANCE");    // 34
      soMap[SubmodelInstanceCountObj] = QString("SUBMODEL_INST_COUNT");  // 35 SubmodelInstanceCountType
      soMap[PartsListPixmapObj]       = QString("PLI_PART");             // 36
      soMap[PartsListGroupObj]        = QString("PLI_PART_GROUP");       // 37
      soMap[StepBackgroundObj]        = QString("STEP_RECTANGLE");       // 38 [StepType]
  }
}

void Gui::loadBLCodes()
{
   if (!Annotations::loadBLCodes()){
       QString URL(VER_LPUB3D_BLCODES_DOWNLOAD_URL);
       downloadFile(URL, "BrickLink Elements");
       QByteArray Buffer = getDownloadedFile();
       Annotations::loadBLCodes(Buffer);
   }
}

void Gui::ldrawColorPartsLoad()
{
    if (!ldrawColourParts.ldrawColorPartsIsLoaded()) {
        QString result;
        if (!LDrawColourParts::LDrawColorPartsLoad(result)){
            QString message = QString("Could not open the %1 LDraw color parts file [%2], Error: %3")
                    .arg(Preferences::validLDrawLibrary).arg(Preferences::ldrawColourPartsFile).arg(result);
            emit messageSig(LOG_NOTICE, message);
            bool prompt = false;
            if (Preferences::modeGUI) {
                QPixmap _icon = QPixmap(":/icons/lpub96.png");
                QMessageBoxResizable box;
                box.setWindowIcon(QIcon());
                box.setIconPixmap (_icon);
                box.setTextFormat (Qt::RichText);

                box.setWindowTitle(QMessageBox::tr ("%1 Color Parts File.").arg(Preferences::validLDrawLibrary));
                box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
                box.setMinimumSize(40,20);

                QString body = QMessageBox::tr ("Would you like to generate it now ?");
                box.setText (message);
                box.setInformativeText (body);
                box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
                box.setDefaultButton   (QMessageBox::Yes);
                if (box.exec() == QMessageBox::Yes) {
                    generateCustomColourPartsList(prompt); /* false */
                }
            } else {
                generateCustomColourPartsList(prompt); /* false */
            }
        } else {
            messageSig(LOG_INFO, QString("Loaded LDraw color parts file [%2]").arg(Preferences::ldrawColourPartsFile));
        }
    }
}

void Gui::reloadModelFileAfterColorFileGen(){
    if (Preferences::modeGUI) {
        QPixmap _icon = QPixmap(":/icons/lpub96.png");
        QMessageBoxResizable box;
        box.setWindowIcon(QIcon());
        box.setIconPixmap (_icon);
        box.setTextFormat (Qt::RichText);
        box.setWindowTitle(QMessageBox::tr ("%1 Color Parts File.").arg(Preferences::validLDrawLibrary));
        box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        box.setMinimumSize(10,10);
        QString message = QString("The %1 LDraw Color Parts File has finished building.")
                .arg(Preferences::validLDrawLibrary);
        QString body = QMessageBox::tr ("The opened model must be reloaded.");
        box.setText (message);
        box.setInformativeText (body);
        box.setStandardButtons (QMessageBox::Ok);
        box.setDefaultButton   (QMessageBox::Ok);
        box.exec();
    }
    if (!getCurFile().isEmpty())
        clearAndReloadModelFile();
}

void Gui::generateCustomColourPartsList(bool prompt)
{
    QMessageBox::StandardButton ret = QMessageBox::Cancel;
    QString message = QString("Generate the %1 color parts list. This may take some time.").arg(Preferences::validLDrawLibrary);

    if (Preferences::modeGUI && prompt && Preferences::lpub3dLoaded) {
            ret = QMessageBox::warning(this, tr(VER_PRODUCTNAME_STR),
                                   tr("%1 Do you want to continue ?").arg(message),
                                   QMessageBox::Yes | QMessageBox::Cancel);
    }

    if (ret == QMessageBox::Yes || ! prompt || !Preferences::lpub3dLoaded) {
        emit messageSig(LOG_INFO,message);

        QThread *listThread   = new QThread();
        colourPartListWorker  = new ColourPartListWorker();
        colourPartListWorker->moveToThread(listThread);

        connect(listThread,           SIGNAL(started()),                     colourPartListWorker, SLOT(generateCustomColourPartsList()));
        connect(listThread,           SIGNAL(finished()),                              listThread, SLOT(deleteLater()));
        connect(colourPartListWorker, SIGNAL(colourPartListFinishedSig()),                   this, SLOT(reloadModelFileAfterColorFileGen()));
        connect(colourPartListWorker, SIGNAL(colourPartListFinishedSig()),             listThread, SLOT(quit()));
        connect(colourPartListWorker, SIGNAL(colourPartListFinishedSig()),   colourPartListWorker, SLOT(deleteLater()));
        connect(this,                 SIGNAL(requestEndThreadNowSig()),      colourPartListWorker, SLOT(requestEndThreadNow()));
//      uses gui signal
//        connect(colourPartListWorker, SIGNAL(messageSig(LogType,QString)),                   this, SLOT(statusMessage(LogType,QString)));

        connect(colourPartListWorker, SIGNAL(progressBarInitSig()),                          this, SLOT(progressBarInit()));
        connect(colourPartListWorker, SIGNAL(progressMessageSig(QString)),                   this, SLOT(progressBarSetText(QString)));
        connect(colourPartListWorker, SIGNAL(progressRangeSig(int,int)),                     this, SLOT(progressBarSetRange(int,int)));
        connect(colourPartListWorker, SIGNAL(progressSetValueSig(int)),                      this, SLOT(progressBarSetValue(int)));
        connect(colourPartListWorker, SIGNAL(progressResetSig()),                            this, SLOT(progressBarReset()));
        connect(colourPartListWorker, SIGNAL(progressStatusRemoveSig()),                     this, SLOT(progressStatusRemove()));

        listThread->start();

        if (!Preferences::modeGUI) {
            QEventLoop  *wait = new QEventLoop();
            wait->connect(colourPartListWorker, SIGNAL(colourPartListFinishedSig()),         wait, SLOT(quit()));
            wait->exec();
        }

    } else {
      return;
    }
}

void Gui::processFadeColourParts(bool overwriteCustomParts)
{
  if (gui->page.meta.LPub.fadeStep.fadeStep.value() || Preferences::enableFadeSteps) {

      partWorkerCustomColour = new PartWorker();

      connect(this,                   SIGNAL(operateFadeParts(bool)),    partWorkerCustomColour, SLOT(processFadeColourParts(bool)));

      connect(partWorkerCustomColour, SIGNAL(progressBarInitSig()),                       this, SLOT(progressBarInit()));
      connect(partWorkerCustomColour, SIGNAL(progressMessageSig(QString)),                this, SLOT(progressBarSetText(QString)));
      connect(partWorkerCustomColour, SIGNAL(progressRangeSig(int,int)),                  this, SLOT(progressBarSetRange(int,int)));
      connect(partWorkerCustomColour, SIGNAL(progressSetValueSig(int)),                   this, SLOT(progressBarSetValue(int)));
      connect(partWorkerCustomColour, SIGNAL(progressResetSig()),                         this, SLOT(progressBarReset()));
      connect(partWorkerCustomColour, SIGNAL(progressStatusRemoveSig()),                  this, SLOT(progressStatusRemove()));

      //qDebug() << qPrintable(QString("Sent overwrite fade parts = %1").arg(overwriteCustomParts ? "True" : "False"));
      emit operateFadeParts(overwriteCustomParts);
    }
}

void Gui::processHighlightColourParts(bool overwriteCustomParts)
{
  if (gui->page.meta.LPub.highlightStep.highlightStep.value() || Preferences::enableHighlightStep) {

      partWorkerCustomColour = new PartWorker();

      connect(this,                   SIGNAL(operateHighlightParts(bool)), partWorkerCustomColour, SLOT(processHighlightColourParts(bool)));

      connect(partWorkerCustomColour, SIGNAL(progressBarInitSig()),                       this, SLOT(progressBarInit()));
      connect(partWorkerCustomColour, SIGNAL(progressMessageSig(QString)),                this, SLOT(progressBarSetText(QString)));
      connect(partWorkerCustomColour, SIGNAL(progressRangeSig(int,int)),                  this, SLOT(progressBarSetRange(int,int)));
      connect(partWorkerCustomColour, SIGNAL(progressSetValueSig(int)),                   this, SLOT(progressBarSetValue(int)));
      connect(partWorkerCustomColour, SIGNAL(progressResetSig()),                         this, SLOT(progressBarReset()));
      connect(partWorkerCustomColour, SIGNAL(progressStatusRemoveSig()),                  this, SLOT(progressStatusRemove()));

      //qDebug() << qPrintable(QString("Sent overwrite highlight parts = %1").arg(overwriteCustomParts ? "True" : "False"));
      emit operateHighlightParts(overwriteCustomParts);
    }
}

// Update parts archive from LDSearch directories
void Gui::loadLDSearchDirParts() {
  partWorkerLDSearchDirs.ldsearchDirPreferences();
  partWorkerLDSearchDirs.processLDSearchDirParts();
}

// left side progress bar
void Gui::progressBarInit(){
  if (okToInvokeProgressBar()) {
      progressBar->setMaximumHeight(15);
      statusBar()->addWidget(progressLabel);
      statusBar()->addWidget(progressBar);
      progressLabel->show();
      progressBar->show();
  }
}

void Gui::progressBarSetText(const QString &progressText)
{
  if (okToInvokeProgressBar()) {
      progressLabel->setText(progressText);
      QApplication::processEvents();
    }
}
void Gui::progressBarSetRange(int minimum, int maximum)
{
  if (okToInvokeProgressBar()) {
      progressBar->setRange(minimum,maximum);
      QApplication::processEvents();
    }
}
void Gui::progressBarSetValue(int value)
{
  if (okToInvokeProgressBar()) {
      progressBar->setValue(value);
      QApplication::processEvents();
    }
}
void Gui::progressBarReset()
{
  if (okToInvokeProgressBar()) {
      progressBar->reset();
      QApplication::processEvents();
    }
}

void Gui::progressStatusRemove(){
  if (okToInvokeProgressBar()) {
      statusBar()->removeWidget(progressBar);
      statusBar()->removeWidget(progressLabel);
    }
}

// right side progress bar
void Gui::progressBarPermInit(){
  if (okToInvokeProgressBar()) {
      progressBarPerm->setMaximumHeight(15);
      statusBar()->addPermanentWidget(progressLabelPerm);
      statusBar()->addPermanentWidget(progressBarPerm);
      progressLabelPerm->show();
      progressBarPerm->show();
      QApplication::processEvents();
    }
}

void Gui::progressBarPermSetText(const QString &progressText)
{
  if (okToInvokeProgressBar()) {
      progressLabelPerm->setText(progressText);
      QApplication::processEvents();
    }
}
void Gui::progressBarPermSetRange(int minimum, int maximum)
{
  if (okToInvokeProgressBar()) {
      progressBarPerm->setRange(minimum,maximum);
      QApplication::processEvents();
    }
}
void Gui::progressBarPermSetValue(int value)
{
  if (okToInvokeProgressBar()) {
      progressBarPerm->setValue(value);
      QApplication::processEvents();
    }
}
void Gui::progressBarPermReset()
{
  if (okToInvokeProgressBar()) {
      progressBarPerm->reset();
      QApplication::processEvents();
    }
}

void Gui::progressPermStatusRemove(){
  if (okToInvokeProgressBar()) {
      statusBar()->removeWidget(progressBarPerm);
      statusBar()->removeWidget(progressLabelPerm);
    }
}

void Gui::showPovrayRenderDialog()
{
    PovrayRenderDialog dialog(this);
    dialog.exec();
}

void Gui::aboutDialog()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void Gui::refreshLDrawUnoffParts() {

    // Download unofficial archive
    emit messageSig(LOG_STATUS,"Refresh LDraw Unofficial Library archive...");
    QTemporaryDir tempDir;
    QString downloadPath = tempDir.path();
    UpdateCheck *libraryDownload;
    libraryDownload      = new UpdateCheck(this, (void*)LDrawUnofficialLibraryDownload);
    QEventLoop  *wait    = new QEventLoop();
    wait->connect(libraryDownload, SIGNAL(downloadFinished(QString,QString)), wait, SLOT(quit()));
    wait->connect(libraryDownload, SIGNAL(cancel()),                          wait, SLOT(quit()));
    libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), downloadPath);
    wait->exec();
    if (libraryDownload->getCancelled()) {
        return;
    }

    // Automatically extract Unofficial archive
    QString newarchive  = QDir::toNativeSeparators(tr("%1/%2").arg(downloadPath).arg(VER_LPUB3D_UNOFFICIAL_ARCHIVE));
    QString destination = QDir::toNativeSeparators(tr("%1/unofficial").arg(Preferences::ldrawLibPath));
    QString message     = tr("Extracting Unofficial archive library. Please wait...");
    emit messageSig(LOG_STATUS,message);

    QStringList items = JlCompress::getFileList(newarchive);
    m_progressDialog  = new ProgressDialog(nullptr);
    m_progressDialog->setWindowFlags(m_progressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
    m_progressDialog->setWindowTitle(QString("LDraw Library Update"));
    m_progressDialog->progressBarSetLabelText(QString("Extracting LDraw Unofficial parts from %1...")
                                              .arg(QFileInfo(newarchive).fileName()));
    m_progressDialog->progressBarSetRange(0,items.count());
    m_progressDialog->setAutoHide(true);
    m_progressDialog->setModal(true);
    m_progressDialog->show();

    QThread *thread    = new QThread(this);
    ExtractWorker *job = new ExtractWorker(newarchive,destination);
    job->moveToThread(thread);
    wait = new QEventLoop();

    connect(thread, SIGNAL(started()),
            job, SLOT(doWork()));
    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()));
    connect(job, SIGNAL(progressSetValue(int)),
            m_progressDialog, SLOT(progressBarSetValue(int)));
    disconnect(m_progressDialog, SIGNAL (cancelClicked()),
            this, SLOT (cancelExporting()));
    connect(m_progressDialog, SIGNAL(cancelClicked()),
            job, SLOT(requestEndWorkNow()));
    connect(job, SIGNAL(result(int)),
            this, SLOT(workerJobResult(int)));
    connect(this, SIGNAL(requestEndThreadNowSig()),
            job, SLOT(requestEndWorkNow()));
    connect(job, SIGNAL(finished()),
            thread, SLOT(quit()));
    connect(job, SIGNAL(finished()),
            job, SLOT(deleteLater()));
    wait->connect(job, SIGNAL(finished()),
            wait, SLOT(quit()));

    workerJobResult(0);
    thread->start();
    wait->exec();

    m_progressDialog->progressBarSetValue(items.count());

    if (m_workerJobResult) {
        message = tr("%1 of %2 Unofficial library files extracted to %3")
                     .arg(m_workerJobResult)
                     .arg(items.count())
                     .arg(destination);
        emit messageSig(LOG_INFO,message);
    } else {
        message = tr("Failed to extract %1 library files")
                     .arg(QFileInfo(newarchive).fileName());
        emit messageSig(LOG_ERROR,message);
    }

   // Process custom and color parts if any
    items = Preferences::ldSearchDirs;
    if (items.count()){
        QString message = tr("Archiving custom parts. Please wait...");
        emit messageSig(LOG_STATUS,message);
        m_progressDialog->progressBarSetLabelText(QString("Archiving custom parts..."));
        m_progressDialog->progressBarSetRange(0,items.count());
        m_progressDialog->show();

        QThread *thread = new QThread(this);
        PartWorker *job = new PartWorker(newarchive);
        job->moveToThread(thread);
        wait = new QEventLoop();

        connect(thread, SIGNAL(started()),
                job, SLOT(processPartsArchive()));
        connect(thread, SIGNAL(finished()),
                thread, SLOT(deleteLater()));
        connect(job, SIGNAL(progressSetValueSig(int)),
                m_progressDialog, SLOT(progressBarSetValue(int)));
        connect(m_progressDialog, SIGNAL(cancelClicked()),
                job, SLOT(requestEndThreadNow()));
        connect(job, SIGNAL(progressMessageSig (QString)),
                m_progressDialog, SLOT(progressBarSetLabelText(QString)));
        connect(job, SIGNAL(progressSetValueSig(int)),
                this, SLOT(workerJobResult(int)));
        connect(this, SIGNAL(requestEndThreadNowSig()),
                job, SLOT(requestEndThreadNow()));
        connect(job, SIGNAL(partsArchiveFinishedSig()),
                thread, SLOT(quit()));
        connect(job, SIGNAL(partsArchiveFinishedSig()),
                job, SLOT(deleteLater()));
        wait->connect(job, SIGNAL(partsArchiveFinishedSig()),
                wait, SLOT(quit()));

        workerJobResult(0);
        thread->start();
        wait->exec();

        m_progressDialog->progressBarSetValue(items.count());

        QString partsLabel = m_workerJobResult == 1 ? "part" : "parts";
        message = tr("Added %1 custom %2 into Unofficial library archive %3")
                     .arg(m_workerJobResult)
                     .arg(partsLabel)
                     .arg(QFileInfo(newarchive).fileName());
        emit messageSig(LOG_INFO,message);

        if (m_workerJobResult) {
            QSettings Settings;
            QVariant uValue(true);
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,SAVE_SKIP_PARTS_ARCHIVE_KEY),uValue);
        }
    }

    connect (m_progressDialog, SIGNAL (cancelClicked()),
             this, SLOT (cancelExporting()));

    // Unload LDraw Unofficial archive library
    gApplication->mLibrary->UnloadUnofficialLib();

    // Copy new archive library to user data
    QString archivePath = QDir::toNativeSeparators(tr("%1/libraries").arg(Preferences::lpubDataPath));
    QString archive     = QDir::toNativeSeparators(tr("%1/%2").arg(archivePath).arg(VER_LPUB3D_UNOFFICIAL_ARCHIVE));
    QFile oldFile(archive);
    QFile newFile(newarchive);
    if (! newFile.exists()) {
        message = tr("Could not find file %1").arg(newFile.fileName());
        emit messageSig(LOG_ERROR,message);
    } else if (! oldFile.exists() || oldFile.remove()) {
        if (! newFile.rename(archive)) {
            message = tr("Could not rename file %1").arg(newFile.fileName());
            emit messageSig(LOG_ERROR,message);
        }
    } else {
        message = tr("Could not remove old file %1").arg(oldFile.fileName());
        emit  messageSig(LOG_ERROR,message);
    }

    // Restart LDraw archive libraries
    restartApplication(false);
}

void Gui::refreshLDrawOfficialParts() {

    // Download official archive
    emit messageSig(LOG_STATUS,"Refresh LDraw Official Library archive...");
    QTemporaryDir tempDir;
    QString downloadPath = tempDir.path();
    UpdateCheck *libraryDownload;
    libraryDownload      = new UpdateCheck(this, (void*)LDrawOfficialLibraryDownload);
    QEventLoop  *wait    = new QEventLoop();
    wait->connect(libraryDownload, SIGNAL(downloadFinished(QString,QString)), wait, SLOT(quit()));
    wait->connect(libraryDownload, SIGNAL(cancel()),                          wait, SLOT(quit()));
    libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), downloadPath);
    wait->exec();
    if (libraryDownload->getCancelled()) {
        return;
    }

    // Automatically extract Official archive
    QString newarchive  = QDir::toNativeSeparators(tr("%1/%2").arg(downloadPath).arg(VER_LDRAW_OFFICIAL_ARCHIVE));
    QString destination = QDir::toNativeSeparators(Preferences::ldrawLibPath);
    destination         = destination.remove(destination.size() - 6,6);
    QString message = tr("Extracting Official archive library. Please wait...");
    emit messageSig(LOG_INFO_STATUS,message);

    QStringList items = JlCompress::getFileList(newarchive);
    m_progressDialog  = new ProgressDialog(nullptr);
    m_progressDialog->setWindowFlags(m_progressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
    m_progressDialog->setWindowTitle(QString("LDraw Library Update"));
    m_progressDialog->progressBarSetLabelText(QString("Extracting LDraw Official parts from %1...")
                                              .arg(QFileInfo(newarchive).fileName()));
    m_progressDialog->progressBarSetRange(0,items.count());
    m_progressDialog->setAutoHide(true);
    m_progressDialog->setModal(true);
    m_progressDialog->show();

    QThread *thread    = new QThread(this);
    ExtractWorker *job = new ExtractWorker(newarchive,destination);
    job->moveToThread(thread);
    wait = new QEventLoop();

    connect(thread, SIGNAL(started()),
            job, SLOT(doWork()));
    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()));
    connect(job, SIGNAL(progressSetValue(int)),
            m_progressDialog, SLOT(progressBarSetValue(int)));
    disconnect(m_progressDialog, SIGNAL (cancelClicked()),
            this, SLOT (cancelExporting()));
    connect(m_progressDialog, SIGNAL(cancelClicked()),
            job, SLOT(requestEndWorkNow()));
    connect(job, SIGNAL(result(int)),
            this, SLOT(workerJobResult(int)));
    connect(this, SIGNAL(requestEndThreadNowSig()),
            job, SLOT(requestEndWorkNow()));
    connect(job, SIGNAL(finished()),
            thread, SLOT(quit()));
    connect(job, SIGNAL(finished()),
            job, SLOT(deleteLater()));
    wait->connect(job, SIGNAL(finished()),
            wait, SLOT(quit()));

    workerJobResult(0);
    thread->start();
    wait->exec();

    m_progressDialog->progressBarSetValue(items.count());
    connect (m_progressDialog, SIGNAL (cancelClicked()),
             this, SLOT (cancelExporting()));

    if (m_workerJobResult) {
        message = tr("%1 of %2 Library files extracted to %3")
                     .arg(m_workerJobResult)
                     .arg(items.count())
                     .arg(destination);
        emit messageSig(LOG_INFO,message);
    } else {
        message = tr("Failed to extract %1 library files")
                     .arg(QFileInfo(newarchive).fileName());
        emit messageSig(LOG_ERROR,message);
    }

    // Unload LDraw Official archive libraries
    gApplication->mLibrary->UnloadOfficialLib();

    // Copy archive library to user data
    QString archivePath = QDir::toNativeSeparators(tr("%1/libraries").arg(Preferences::lpubDataPath));
    QString archive     = QDir::toNativeSeparators(tr("%1/%2").arg(archivePath).arg(VER_LDRAW_OFFICIAL_ARCHIVE));
    QFile oldFile(archive);
    QFile newFile(newarchive);
    if (! newFile.exists()) {
        message = tr("Could not find file %1").arg(newFile.fileName());
        emit messageSig(LOG_ERROR,message);
    } else if (! oldFile.exists() || oldFile.remove()) {
        if (! newFile.rename(archive)) {
            message = tr("Could not rename file %1").arg(newFile.fileName());
            emit messageSig(LOG_ERROR,message);
        }
    } else {
        message = tr("Could not remove old file %1").arg(oldFile.fileName());
        emit  messageSig(LOG_ERROR,message);
    }

    // Restart LDraw archive libraries
    restartApplication(false);
}

void Gui::updateCheck()
{
    // Create an instance of update check
    new UpdateCheck(this, (void*)SoftwareUpdate);
}

#include <QDesktopServices>
#include <QUrl>

void Gui::onlineManual()
{
    QDesktopServices::openUrl(QUrl(VER_LPUB3D_ONLINE_MANUAL_URL));
}

void Gui::meta()
{
  Meta meta;
  QStringList doc;
  static const QString fmtDateTime("MM-dd-yyyy hh:mm:ss");

  QString fileName = QFileDialog::getSaveFileName(
    this,
    tr("Metacommands Save File Name"),
    QDir::currentPath() + "/metacommands.txt",
    tr("txt (*.txt)"));

  if (fileName == "") {
    return;
  }
  meta.doc(doc);

  QFile file(fileName);
  if (!file.open(QFile::WriteOnly | QFile::Text)) {
    emit messageSig(LOG_ERROR, QString("Cannot write file %1:\n%2.")
                    .arg(fileName)
                    .arg(file.errorString()));
    return;
  }

  QTextStream out(&file);

  doc.prepend(QString());
  doc.prepend(QString("%1 %2 - Generated on %3")
                       .arg(VER_PRODUCTNAME_STR)
                       .arg(QFileInfo(fileName).completeBaseName())
                       .arg(QDateTime::currentDateTime().toString(fmtDateTime)));
  doc.append(QString());
  doc.append(QString("End of file."));
  int n = 0;
  for (int i = 0; i < doc.size(); i++) {
    QString number;
    if (QString(doc[i]).startsWith('0'))
        number = QString("%1. ").arg(++n,3,10,QChar('0'));
    else
        number = QString();
    out << number << doc[i] << endl;
  }

  file.close();
}

void Gui::createOpenWithActions(int maxPrograms)
{
    int maxOpenWithPrograms = maxPrograms ? maxPrograms : Preferences::maxOpenWithPrograms;

    for (int i = 0; i < maxOpenWithPrograms; i++) {
        QAction *openWithAct = new QAction(this);
        openWithAct->setVisible(false);
        connect(openWithAct, SIGNAL(triggered()), this, SLOT(openWith()));
        if (i < openWithActList.size()) {
            openWithActList.replace(i,openWithAct);
        } else {
            openWithActList.append(openWithAct);
        }
    }
}

void Gui::createActions()
{
    openAct = new QAction(QIcon(":/resources/open.png"), tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open an existing file - Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    createOpenWithActions();

    openWithSetupAct = new QAction(QIcon(":/resources/openwithsetup.png"), tr("Open With S&etup..."), this);
    openWithSetupAct->setStatusTip(tr("Setup 'Open With' applications."));
    connect(openWithSetupAct, SIGNAL(triggered()), this, SLOT(openWithSetup()));

    saveAct = new QAction(QIcon(":/resources/save.png"), tr("&Save"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the document to disk - Ctrl+S"));
    saveAct->setEnabled(false);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(QIcon(":/resources/saveas.png"),tr("Save A&s..."), this);
    saveAsAct->setShortcut(tr("Ctrl+Shift+S"));
    saveAsAct->setStatusTip(tr("Save the document under a new name - Ctrl+Shift+S"));
    saveAsAct->setEnabled(false);
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    saveCopyAct = new QAction(QIcon(":/resources/savecopy.png"),tr("Save a Copy As..."), this);
    saveCopyAct->setShortcut(tr("Ctrl+Shift+C"));
    saveCopyAct->setStatusTip(tr("Save a copy of the document under a new name - Ctrl+Shift+C"));
    saveCopyAct->setEnabled(false);
    connect(saveCopyAct, SIGNAL(triggered()), this, SLOT(saveCopy()));

    closeFileAct = new QAction(QIcon(":/resources/closemodelfile.png"), tr("Close"), this);
    closeFileAct->setShortcut(tr("Ctrl+W"));
    closeFileAct->setStatusTip(tr("Close current model file - Ctrl+W"));
    closeFileAct->setEnabled(false);
    connect(closeFileAct, SIGNAL(triggered()), this, SLOT(closeModelFile()));

    QIcon printToFilePreviewIcon;
    printToFilePreviewIcon.addFile(":/resources/file_print_preview.png");
    printToFilePreviewIcon.addFile(":/resources/file_print_preview_16.png");
    printToFilePreviewAct = new QAction(printToFilePreviewIcon, tr("File Print Pre&view..."), this);
    printToFilePreviewAct->setShortcut(tr("Alt+Shift+P"));
    printToFilePreviewAct->setStatusTip(tr("Preview the current document to be printed - Alt+Shift+P"));
    printToFilePreviewAct->setEnabled(false);
    connect(printToFilePreviewAct, SIGNAL(triggered()), this, SLOT(TogglePrintToFilePreview()));

    QIcon printToFileIcon;
    printToFileIcon.addFile(":/resources/file_print.png");
    printToFileIcon.addFile(":/resources/file_print_16.png");
    printToFileAct = new QAction(printToFileIcon, tr("File &Print..."), this);
    printToFileAct->setShortcut(tr("Alt+Shift+F"));
    printToFileAct->setStatusTip(tr("Print the current document - Alt+Shift+F"));
    printToFileAct->setEnabled(false);
    connect(printToFileAct, SIGNAL(triggered()), this, SLOT(ShowPrintDialog()));

    exportAsPdfPreviewAct = new QAction(QIcon(":/resources/pdf_print_preview.png"), tr("PDF Export Preview..."), this);
    exportAsPdfPreviewAct->setShortcut(tr("Alt+P"));
    exportAsPdfPreviewAct->setStatusTip(tr("Preview the current pdf document to be exported - Alt+P"));
    exportAsPdfPreviewAct->setEnabled(false);
    connect(exportAsPdfPreviewAct, SIGNAL(triggered()), this, SLOT(TogglePdfExportPreview()));

    exportAsPdfAct = new QAction(QIcon(":/resources/pdf_logo.png"), tr("Export to PDF &File..."), this);
    exportAsPdfAct->setShortcut(tr("Alt+F"));
    exportAsPdfAct->setStatusTip(tr("Export your document to a pdf file - Alt+F"));
    exportAsPdfAct->setEnabled(false);
    connect(exportAsPdfAct, SIGNAL(triggered()), this, SLOT(exportAsPdfDialog()));

    exportPngAct = new QAction(QIcon(":/resources/exportpng.png"),tr("Export As P&NG Images..."), this);
    exportPngAct->setShortcut(tr("Alt+N"));
    exportPngAct->setStatusTip(tr("Export your document as a sequence of PNG images - Alt+N"));
    exportPngAct->setEnabled(false);
    connect(exportPngAct, SIGNAL(triggered()), this, SLOT(exportAsPngDialog()));

    exportJpgAct = new QAction(QIcon(":/resources/exportjpeg.png"),tr("Export As &JPEG Images..."), this);
    exportJpgAct->setShortcut(tr("Alt+J"));
    exportJpgAct->setStatusTip(tr("Export your document as a sequence of JPEG images - Alt+J"));
    exportJpgAct->setEnabled(false);
    connect(exportJpgAct, SIGNAL(triggered()), this, SLOT(exportAsJpgDialog()));

    exportBmpAct = new QAction(QIcon(":/resources/exportbmp.png"),tr("Export As &Bitmap Images..."), this);
    exportBmpAct->setShortcut(tr("Alt+B"));
    exportBmpAct->setStatusTip(tr("Export your document as a sequence of bitmap images - Alt+B"));
    exportBmpAct->setEnabled(false);
    connect(exportBmpAct, SIGNAL(triggered()), this, SLOT(exportAsBmpDialog()));

    exportHtmlAct = new QAction(QIcon(":/resources/html32.png"),tr("Export As &HTML Part List..."), this);
    exportHtmlAct->setShortcut(tr("Alt+6"));
    exportHtmlAct->setStatusTip(tr("Export your document as an HTML part list - Alt+6"));
    exportHtmlAct->setEnabled(false);
    connect(exportHtmlAct, SIGNAL(triggered()), this, SLOT(exportAsHtml()));

    exportHtmlStepsAct = new QAction(QIcon(":/resources/htmlsteps32.png"),tr("Export As HT&ML Steps..."), this);
    exportHtmlStepsAct->setShortcut(tr("Alt+Shift+6"));
    exportHtmlStepsAct->setStatusTip(tr("Export your document as navigatable steps in HTML format - Alt+Shift+6"));
    exportHtmlStepsAct->setEnabled(false);
    connect(exportHtmlStepsAct, SIGNAL(triggered()), this, SLOT(exportAsHtmlSteps()));

    exportColladaAct = new QAction(QIcon(":/resources/dae32.png"),tr("Export As &COLLADA Objects..."), this);
    exportColladaAct->setShortcut(tr("Alt+4"));
    exportColladaAct->setStatusTip(tr("Export your document as a sequence of Collada objects - Alt+4"));
    exportColladaAct->setEnabled(false);
    connect(exportColladaAct, SIGNAL(triggered()), this, SLOT(exportAsColladaDialog()));

    exportObjAct = new QAction(QIcon(":/resources/obj32.png"),tr("Export As &Wavefront Objects..."), this);
    exportObjAct->setShortcut(tr("Alt+1"));
    exportObjAct->setStatusTip(tr("Export your document as a sequence of Wavefront objects - Alt+1"));
    exportObjAct->setEnabled(false);
    connect(exportObjAct, SIGNAL(triggered()), this, SLOT(exportAsObjDialog()));

    export3dsAct = new QAction(QIcon(":/resources/3ds32.png"),tr("Export As &3DStudio Objects..."), this);
    export3dsAct->setShortcut(tr("Alt+5"));
    export3dsAct->setStatusTip(tr("Export your document as a sequence of 3DStudio objects - Alt+5"));
    export3dsAct->setEnabled(false);
    connect(export3dsAct, SIGNAL(triggered()), this, SLOT(exportAs3dsDialog()));

    exportStlAct = new QAction(QIcon(":/resources/stl32.png"),tr("Export As &Stereo Lithography Objects..."), this);
    exportStlAct->setShortcut(tr("Alt+2"));
    exportStlAct->setStatusTip(tr("Export your document as a sequence of Stereo Lithography Objects - Alt+2"));
    exportStlAct->setEnabled(false);
    connect(exportStlAct, SIGNAL(triggered()), this, SLOT(exportAsStlDialog()));

    exportPovAct = new QAction(QIcon(":/resources/povray32.png"),tr("Export As &PovRay Scene Files..."), this);
    exportPovAct->setShortcut(tr("Alt+3"));
    exportPovAct->setStatusTip(tr("Export your document as a sequence of PovRay Scene Files - Alt+3"));
    exportPovAct->setEnabled(false);
    connect(exportPovAct, SIGNAL(triggered()), this, SLOT(exportAsPovDialog()));

    exportCsvAct = new QAction(QIcon(":/resources/csv32.png"),tr("Export As &CSV Part List..."), this);
    exportCsvAct->setShortcut(tr("Alt+8"));
    exportCsvAct->setStatusTip(tr("Export your document as a CSV part list - Alt+8"));
    exportCsvAct->setEnabled(false);
    connect(exportCsvAct, SIGNAL(triggered()), this, SLOT(exportAsCsv()));

    exportBricklinkAct = new QAction(QIcon(":/resources/bricklink32.png"),tr("Export As &Bricklink XML Part List..."), this);
    exportBricklinkAct->setShortcut(tr("Alt+7"));
    exportBricklinkAct->setStatusTip(tr("Export your document as a Bricklink XML Part List - Alt+7"));
    exportBricklinkAct->setEnabled(false);
    connect(exportBricklinkAct, SIGNAL(triggered()), this, SLOT(exportAsBricklinkXML()));

    povrayRenderAct = new QAction(QIcon(":/resources/povray32.png"),tr("Render..."), this);
    povrayRenderAct->setShortcut(tr("Alt+9"));
    povrayRenderAct->setStatusTip(tr("Render the current model using POV-Ray - Alt+9"));
    povrayRenderAct->setEnabled(false);
    connect(povrayRenderAct, SIGNAL(triggered()), this, SLOT(showPovrayRenderDialog()));

    exitAct = new QAction(QIcon(":/resources/exit.png"),tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application - Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    for (int i = 0; i < MaxRecentFiles; i++) {
      recentFilesActs[i] = new QAction(this);
      recentFilesActs[i]->setVisible(false);
      connect(recentFilesActs[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));
    }

    clearRecentAct = new QAction(tr("Clear Files"),this);
    clearRecentAct->setStatusTip(tr("Clear recent files"));
    connect(clearRecentAct, SIGNAL(triggered()), this, SLOT(clearRecentFiles()));
    // undo/redo

    undoAct = new QAction(QIcon(":/resources/editundo.png"), tr("Undo"), this);
    undoAct->setShortcut(tr("Ctrl+Z"));
    undoAct->setStatusTip(tr("Undo last change - Ctrl+Z"));
    undoAct->setEnabled(false);
    connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));
    redoAct = new QAction(QIcon(":/resources/editredo.png"), tr("Redo"), this);
#ifdef __APPLE__
    redoAct->setShortcut(tr("Ctrl+Shift+Z"));
    redoAct->setStatusTip(tr("Redo last change - Ctrl+Shift+Z"));
#else
    redoAct->setShortcut(tr("Ctrl+Y"));
    redoAct->setStatusTip(tr("Redo last change - Ctrl+Y"));
#endif
    redoAct->setEnabled(false);
    connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));

    insertCoverPageAct = new QAction(QIcon(":/resources/insertcoverpage.png"),tr("Insert Front Cover Page"),this);
    insertCoverPageAct->setShortcut(tr("Ctrl+Alt+F"));
    insertCoverPageAct->setStatusTip(tr("Insert a front cover page - Ctrl+Alt+F"));
    insertCoverPageAct->setEnabled(false);
    connect(insertCoverPageAct, SIGNAL(triggered()), this, SLOT(insertCoverPage()));

    appendCoverPageAct = new QAction(QIcon(":/resources/appendcoverpage.png"),tr("Append Back Cover Page"),this);
    appendCoverPageAct->setShortcut(tr("Ctrl+Alt+B"));
    appendCoverPageAct->setStatusTip(tr("Append back cover page - Ctrl+Alt+B"));
    appendCoverPageAct->setEnabled(false);
    connect(appendCoverPageAct, SIGNAL(triggered()), this, SLOT(appendCoverPage()));

    insertNumberedPageAct = new QAction(QIcon(":/resources/insertpage.png"),tr("Insert Page"),this);
    insertNumberedPageAct->setShortcut(tr("Ctrl+Alt+I"));
    insertNumberedPageAct->setStatusTip(tr("Insert a numbered page - Ctrl+Alt+I"));
    insertNumberedPageAct->setEnabled(false);
    connect(insertNumberedPageAct, SIGNAL(triggered()), this, SLOT(insertNumberedPage()));

    appendNumberedPageAct = new QAction(QIcon(":/resources/appendpage.png"),tr("Append Page"),this);
    appendNumberedPageAct->setShortcut(tr("Ctrl+Alt+A"));
    appendNumberedPageAct->setStatusTip(tr("Append a numbered page - Ctrl+Alt+A"));
    appendNumberedPageAct->setEnabled(false);
    connect(appendNumberedPageAct, SIGNAL(triggered()), this, SLOT(appendNumberedPage()));

    deletePageAct = new QAction(QIcon(":/resources/deletepage.png"),tr("Delete Page"),this);
    deletePageAct->setShortcut(tr("Ctrl+Alt+D"));
    deletePageAct->setStatusTip(tr("Delete this page - Ctrl+Alt+D"));
    deletePageAct->setEnabled(false);
    connect(deletePageAct, SIGNAL(triggered()), this, SLOT(deletePage()));

    addPictureAct = new QAction(QIcon(":/resources/addpicture.png"),tr("Add Picture"),this);
    addPictureAct->setShortcut(tr("Ctrl+Alt+P"));
    addPictureAct->setStatusTip(tr("Add a picture to this page - Ctrl+Alt+P"));
    addPictureAct->setEnabled(false);
    connect(addPictureAct, SIGNAL(triggered()), this, SLOT(addPicture()));

    addTextAct = new QAction(QIcon(":/resources/addtext.png"),tr("Add Text"),this);
    addTextAct->setShortcut(tr("Ctrl+Alt+T"));
    addTextAct->setStatusTip(tr("Add text to this page - Ctrl+Alt+T"));
    addTextAct->setEnabled(false);
    connect(addTextAct, SIGNAL(triggered()), this, SLOT(addText()));

    addBomAct = new QAction(QIcon(":/resources/addbom.png"),tr("Add Bill of Materials"),this);
    addBomAct->setShortcut(tr("Ctrl+Alt+M"));
    addBomAct->setStatusTip(tr("Add Bill of Materials to this page - Ctrl+Alt+M"));
    addBomAct->setEnabled(false);
    connect(addBomAct, SIGNAL(triggered()), this, SLOT(addBom()));

    removeLPubFormattingAct = new QAction(QIcon(":/resources/removelpubformat.png"),tr("Remove LPub Formatting"),this);
    removeLPubFormattingAct->setShortcut(tr("Ctrl+Alt+R"));
    removeLPubFormattingAct->setStatusTip(tr("Remove all LPub metacommands from all files - Ctrl+Alt+R"));
    removeLPubFormattingAct->setEnabled(false);
    connect(removeLPubFormattingAct, SIGNAL(triggered()), this, SLOT(removeLPubFormatting()));

    // fitWidth,fitVisible,actualSize,fitScene

    fitWidthAct = new QAction(QIcon(":/resources/fitWidth.png"), tr("Fit &Width"), this);
    fitWidthAct->setShortcut(tr("Alt+W"));
    fitWidthAct->setStatusTip(tr("Fit document to width - Alt+W"));
    fitWidthAct->setEnabled(false);
    connect(fitWidthAct, SIGNAL(triggered()), this, SLOT(fitWidth()));

    fitVisibleAct = new QAction(QIcon(":/resources/fitVisible.png"), tr("Fit &Visible"), this);
    fitVisibleAct->setShortcut(tr("Alt+V"));
    fitVisibleAct->setStatusTip(tr("Fit document so whole page is visible - Alt+V"));
    fitVisibleAct->setEnabled(false);
    connect(fitVisibleAct, SIGNAL(triggered()), this, SLOT(fitVisible()));

    fitSceneAct = new QAction(QIcon(":/resources/fitScene.png"), tr("Fit &Scene"), this);
    fitSceneAct->setShortcut(tr("Alt+H"));
    fitSceneAct->setStatusTip(tr("Fit document so whole scene is visible - Alt+H"));
    fitSceneAct->setEnabled(false);
    connect(fitSceneAct, SIGNAL(triggered()), this, SLOT(fitScene()));

    bringToFrontAct = new QAction(QIcon(":/resources/bringtofront.png"), tr("Bring to &Front"), this);
    bringToFrontAct->setShortcut(tr("Alt+Shift+F"));
    bringToFrontAct->setStatusTip(tr("Bring item to front - Alt+Shift+F"));
    bringToFrontAct->setEnabled(false);
    connect(bringToFrontAct, SIGNAL(triggered()), this, SLOT(bringToFront()));

    sendToBackAct = new QAction(QIcon(":/resources/sendtoback.png"), tr("Send to &Back"), this);
    sendToBackAct->setShortcut(tr("Alt+Shift+B"));
    sendToBackAct->setStatusTip(tr("Send item to back - Alt+Shift+B"));
    sendToBackAct->setEnabled(false);
    connect(sendToBackAct, SIGNAL(triggered()), this, SLOT(sendToBack()));

    sceneRulerTrackingNoneAct = new QAction(tr("Ruler Tracking Off"),this);
    sceneRulerTrackingNoneAct->setStatusTip(tr("Set scene ruler tracking Off"));
    sceneRulerTrackingNoneAct->setCheckable(true);
    connect(sceneRulerTrackingNoneAct, SIGNAL(triggered()), this, SLOT(sceneRulerTracking()));

    sceneRulerTrackingTickAct = new QAction(tr("Ruler Tracking Tick"),this);
    sceneRulerTrackingTickAct->setStatusTip(tr("Set scene ruler tracking to use ruler tick mark"));
    sceneRulerTrackingTickAct->setCheckable(true);
    connect(sceneRulerTrackingTickAct, SIGNAL(triggered()), this, SLOT(sceneRulerTracking()));

    sceneRulerTrackingLineAct = new QAction(tr("Ruler Tracking Line"),this);
    sceneRulerTrackingLineAct->setStatusTip(tr("Set scene ruler tracking to use full scene line"));
    sceneRulerTrackingLineAct->setCheckable(true);
    connect(sceneRulerTrackingLineAct, SIGNAL(triggered()), this, SLOT(sceneRulerTracking()));

    hideRulerPageBackgroundAct = new QAction(tr("Hide Page Background"),this);
    hideRulerPageBackgroundAct->setStatusTip(tr("Toggle hide ruler page background"));
    hideRulerPageBackgroundAct->setCheckable(true);
    hideRulerPageBackgroundAct->setChecked(Preferences::hidePageBackground);
    hideRulerPageBackgroundAct->setEnabled(Preferences::sceneRuler);
    connect(hideRulerPageBackgroundAct, SIGNAL(triggered()), this, SLOT(hidePageBackground()));

    SceneRulerGroup = new QActionGroup(this);
    SceneRulerGroup->setEnabled(Preferences::sceneRuler);
    sceneRulerTrackingNoneAct->setChecked(Preferences::sceneRulerTracking == int(TRACKING_NONE));
    SceneRulerGroup->addAction(sceneRulerTrackingNoneAct);
    sceneRulerTrackingTickAct->setChecked(Preferences::sceneRulerTracking == int(TRACKING_TICK));
    SceneRulerGroup->addAction(sceneRulerTrackingTickAct);
    sceneRulerTrackingLineAct->setChecked(Preferences::sceneRulerTracking == int(TRACKING_LINE));
    SceneRulerGroup->addAction(sceneRulerTrackingLineAct);

    sceneRulerComboAct = new QAction(QIcon(":/resources/pageruler.png"), tr("Scene &Ruler"), this);
    sceneRulerComboAct->setShortcut(tr("Alt+U"));
    sceneRulerComboAct->setStatusTip(tr("Toggle the scene ruler - Alt+U"));
    sceneRulerComboAct->setEnabled(true);
    sceneRulerComboAct->setCheckable(true);
    sceneRulerComboAct->setChecked(Preferences::sceneRuler);
    connect(sceneRulerComboAct, SIGNAL(triggered()), this, SLOT(sceneRuler()));

    sceneGuidesDashLineAct = new QAction(tr("Dash Line"),this);
    sceneGuidesDashLineAct->setStatusTip(tr("Select dash scene guide lines"));
    sceneGuidesDashLineAct->setCheckable(true);
    connect(sceneGuidesDashLineAct, SIGNAL(triggered()), this, SLOT(sceneGuidesLine()));

    sceneGuidesSolidLineAct = new QAction(tr("Solid Line"),this);
    sceneGuidesSolidLineAct->setStatusTip(tr("Select solid scene guide lines"));
    sceneGuidesSolidLineAct->setCheckable(true);
    connect(sceneGuidesSolidLineAct, SIGNAL(triggered()), this, SLOT(sceneGuidesLine()));

    SceneGuidesLineGroup = new QActionGroup(this);
    SceneGuidesLineGroup->setEnabled(Preferences::sceneGuides);
    sceneGuidesDashLineAct->setChecked(Preferences::sceneGuidesLine == int(Qt::DashLine));
    SceneGuidesLineGroup->addAction(sceneGuidesDashLineAct);
    sceneGuidesSolidLineAct->setChecked(Preferences::sceneGuidesLine == int(Qt::SolidLine));
    SceneGuidesLineGroup->addAction(sceneGuidesSolidLineAct);

    sceneGuidesPosTLeftAct = new QAction(tr("Top Left"),this);
    sceneGuidesPosTLeftAct->setStatusTip(tr("Set scene guides position to top left of graphic item"));
    sceneGuidesPosTLeftAct->setCheckable(true);
    connect(sceneGuidesPosTLeftAct, SIGNAL(triggered()), this, SLOT(sceneGuidesPosition()));

    sceneGuidesPosTRightAct = new QAction(tr("Top Right"),this);
    sceneGuidesPosTRightAct->setStatusTip(tr("Set scene guides position to top right of graphic item"));
    sceneGuidesPosTRightAct->setCheckable(true);
    connect(sceneGuidesPosTRightAct, SIGNAL(triggered()), this, SLOT(sceneGuidesPosition()));

    sceneGuidesPosBLeftAct = new QAction(tr("Bottom Left"),this);
    sceneGuidesPosBLeftAct->setStatusTip(tr("Set scene guides position to bottom left of graphic item"));
    sceneGuidesPosBLeftAct->setCheckable(true);
    connect(sceneGuidesPosBLeftAct, SIGNAL(triggered()), this, SLOT(sceneGuidesPosition()));

    sceneGuidesPosBRightAct = new QAction(tr("Bottom Right"),this);
    sceneGuidesPosBRightAct->setStatusTip(tr("Set scene guides position to bottom right of graphic item"));
    sceneGuidesPosBRightAct->setCheckable(true);
    connect(sceneGuidesPosBRightAct, SIGNAL(triggered()), this, SLOT(sceneGuidesPosition()));

    sceneGuidesPosCentreAct = new QAction(tr("Centre"),this);
    sceneGuidesPosCentreAct->setStatusTip(tr("Set scene guides position to centre of graphic item"));
    sceneGuidesPosCentreAct->setCheckable(true);
    connect(sceneGuidesPosCentreAct, SIGNAL(triggered()), this, SLOT(sceneGuidesPosition()));

    SceneGuidesPosGroup = new QActionGroup(this);
    SceneGuidesPosGroup->setEnabled(Preferences::sceneGuides);
    sceneGuidesPosTLeftAct->setChecked(Preferences::sceneGuidesPosition == int(GUIDES_TOP_LEFT));
    SceneGuidesPosGroup->addAction(sceneGuidesPosTLeftAct);
    sceneGuidesPosTRightAct->setChecked(Preferences::sceneGuidesPosition == int(GUIDES_TOP_RIGHT));
    SceneGuidesPosGroup->addAction(sceneGuidesPosTRightAct);
    sceneGuidesPosBLeftAct->setChecked(Preferences::sceneGuidesPosition == int(GUIDES_BOT_LEFT));
    SceneGuidesPosGroup->addAction(sceneGuidesPosBLeftAct);
    sceneGuidesPosBRightAct->setChecked(Preferences::sceneGuidesPosition == int(GUIDES_BOT_RIGHT));
    SceneGuidesPosGroup->addAction(sceneGuidesPosBRightAct);
    sceneGuidesPosCentreAct->setChecked(Preferences::sceneGuidesPosition == int(GUIDES_CENTRE));
    SceneGuidesPosGroup->addAction(sceneGuidesPosCentreAct);

    sceneGuidesComboAct = new QAction(QIcon(":/resources/pageguides.png"), tr("Scene &Guides"), this);
    sceneGuidesComboAct->setShortcut(tr("Alt+G"));
    sceneGuidesComboAct->setStatusTip(tr("Toggle horizontal and vertical scene guides - Alt+G"));
    sceneGuidesComboAct->setEnabled(true);
    sceneGuidesComboAct->setCheckable(true);
    sceneGuidesComboAct->setChecked(Preferences::sceneGuides);
    connect(sceneGuidesComboAct, SIGNAL(triggered()), this, SLOT(sceneGuides()));

    actualSizeAct = new QAction(QIcon(":/resources/actual.png"),tr("&Actual Size"), this);
    actualSizeAct->setShortcut(tr("Alt+A"));
    actualSizeAct->setStatusTip(tr("Show document actual size - Alt+A"));
    actualSizeAct->setEnabled(false);
    connect(actualSizeAct, SIGNAL(triggered()), this, SLOT(actualSize()));

    hideGridPageBackgroundAct = new QAction(tr("Hide Page Background"),this);
    hideGridPageBackgroundAct->setStatusTip(tr("Toggle hide snap to grid page background"));
    hideGridPageBackgroundAct->setCheckable(true);
    hideGridPageBackgroundAct->setChecked(Preferences::hidePageBackground);
    hideGridPageBackgroundAct->setEnabled(Preferences::snapToGrid);
    connect(hideGridPageBackgroundAct, SIGNAL(triggered()), this, SLOT(hidePageBackground()));

    for (int CommandIdx = 0; CommandIdx < NUM_GRID_SIZES; CommandIdx++)
    {
        QAction* Action = new QAction(qApp->translate("Menu", sgCommands[CommandIdx].MenuName), this);
        Action->setStatusTip(qApp->translate("Status", sgCommands[CommandIdx].StatusText));
        connect(Action, SIGNAL(triggered()), this, SLOT(gridSizeTriggered()));
        addAction(Action);
        snapGridActions[CommandIdx] = Action;
    }

    GridStepSizeGroup = new QActionGroup(this);
    GridStepSizeGroup->setEnabled(Preferences::snapToGrid);
    for (int ActionIdx = GRID_SIZE_FIRST; ActionIdx <= GRID_SIZE_LAST; ActionIdx++)
    {
        snapGridActions[ActionIdx]->setCheckable(true);
        GridStepSizeGroup->addAction(snapGridActions[ActionIdx]);
    }

    snapGridActions[Preferences::gridSizeIndex]->setChecked(true);

    snapToGridComboAct = new QAction(QIcon(":/resources/scenegrid.png"),tr("&Snap To Grid"),this);
    snapToGridComboAct->setShortcut(tr("Alt+K"));
    snapToGridComboAct->setStatusTip(tr("Toggle snap-to-grid - Alt+K"));
    snapToGridComboAct->setCheckable(true);
    snapToGridComboAct->setChecked(Preferences::snapToGrid);
    connect(snapToGridComboAct, SIGNAL(triggered()), this, SLOT(snapToGrid()));

    showTrackingCoordinatesAct = new QAction(tr("Show Tracking Coordinates"),this);
    showTrackingCoordinatesAct->setStatusTip(tr("Toggle show ruler tracking coordinates"));
    showTrackingCoordinatesAct->setCheckable(true);
    showTrackingCoordinatesAct->setChecked(Preferences::showTrackingCoordinates);
    showTrackingCoordinatesAct->setEnabled(Preferences::sceneRulerTracking == int(TRACKING_LINE));
    connect(showTrackingCoordinatesAct, SIGNAL(triggered()), this, SLOT(showCoordinates()));


    showGuidesCoordinatesAct = new QAction(tr("Show Guide Coordinates"),this);
    showGuidesCoordinatesAct->setStatusTip(tr("Toggle show scene guide coordinates"));
    showGuidesCoordinatesAct->setCheckable(true);
    showGuidesCoordinatesAct->setChecked(Preferences::showGuidesCoordinates);
    showGuidesCoordinatesAct->setEnabled(Preferences::sceneGuides);
    connect(showGuidesCoordinatesAct, SIGNAL(triggered()), this, SLOT(showCoordinates()));
    // TESTING ONLY
    //connect(actualSizeAct, SIGNAL(triggered()), this, SLOT(twoPages()));

    // zoomIn,zoomOut

    zoomSliderAct = new QWidgetAction(nullptr);
    zoomSliderWidget = new QSlider();
    zoomSliderWidget->setSingleStep(1);
    zoomSliderWidget->setTickInterval(10);
    zoomSliderWidget->setTickPosition(QSlider::TicksBelow);
    zoomSliderWidget->setMaximum(150);
    zoomSliderWidget->setMinimum(1);
    zoomSliderWidget->setValue(50);
    zoomSliderAct->setDefaultWidget(zoomSliderWidget);
    connect(zoomSliderWidget, SIGNAL(valueChanged(int)), this, SLOT(zoomSlider(int)));

    zoomInComboAct = new QAction(QIcon(":/resources/zoomin.png"), tr("&Zoom In"), this);
    zoomInComboAct->setShortcut(tr("Ctrl++"));
    zoomInComboAct->setStatusTip(tr("Zoom in - Ctrl++"));
    zoomInComboAct->setEnabled(false);
    connect(zoomInComboAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutComboAct = new QAction(QIcon(":/resources/zoomout.png"),tr("Zoom &Out"),this);
    zoomOutComboAct->setShortcut(tr("Ctrl+-"));
    zoomOutComboAct->setStatusTip(tr("Zoom out - Ctrl+-"));
    zoomOutComboAct->setEnabled(false);
    connect(zoomOutComboAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    // firstPage,lastPage,nextPage,previousPage
    firstPageAct = new QAction(QIcon(":/resources/first.png"),tr("First Page"), this);
    firstPageAct->setShortcut(tr("Ctrl+P"));
    firstPageAct->setStatusTip(tr("Go to first page of document - Ctrl+P"));
    firstPageAct->setEnabled(false);
    connect(firstPageAct, SIGNAL(triggered()), this, SLOT(firstPage()));

    lastPageAct = new QAction(QIcon(":/resources/last.png"),tr("Last Page"), this);
    lastPageAct->setShortcut(tr("Ctrl+L"));
    lastPageAct->setStatusTip(tr("Go to last page of document - Ctrl+L"));
    lastPageAct->setEnabled(false);
    connect(lastPageAct, SIGNAL(triggered()), this, SLOT(lastPage()));

    nextPageAct = new QAction(QIcon(":/resources/next.png"),tr("&Next Page"),this);
    nextPageAct->setShortcut(tr("Ctrl+N"));
    nextPageAct->setStatusTip(tr("Go to next page of document - Ctrl+N"));
    nextPageAct->setEnabled(false);
    connect(nextPageAct, SIGNAL(triggered()), this, SLOT(nextPage()));

    previousPageAct = new QAction(QIcon(":/resources/prev.png"),tr("&Previous Page"),this);
    previousPageAct->setShortcut(tr("Ctrl+E"));
    previousPageAct->setStatusTip(tr("Go to previous page of document - Ctrl+E"));
    previousPageAct->setEnabled(false);
    connect(previousPageAct, SIGNAL(triggered()), this, SLOT(previousPage()));

    // nextContinuousPage,previousContinuousPage
    nextPageComboAct = new QAction(QIcon(":/resources/next.png"),tr("&Next Page"),this);
    nextPageComboAct->setStatusTip(tr("Go to next page of document - Ctrl+N"));
    nextPageComboAct->setEnabled(false);
    connect(nextPageComboAct, SIGNAL(triggered()), this, SLOT(nextPage()));

    previousPageComboAct = new QAction(QIcon(":/resources/prev.png"),tr("&Previous Page"),this);
    previousPageComboAct->setStatusTip(tr("Go to previous page of document - Ctrl+E"));
    previousPageComboAct->setEnabled(false);
    connect(previousPageComboAct, SIGNAL(triggered()), this, SLOT(previousPage()));

    nextPageContinuousAct = new QAction(QIcon(":/resources/nextpagecontinuous.png"),tr("Continuous Next Page"),this);
    nextPageContinuousAct->setShortcut(tr("Ctrl+Shift+N"));
    nextPageContinuousAct->setStatusTip(tr("Continuously process next page to end of document - Ctrl+Shift+N"));
    nextPageContinuousAct->setEnabled(false);
    connect(nextPageContinuousAct, SIGNAL(triggered()), this, SLOT(nextPageContinuous()));

    previousPageContinuousAct = new QAction(QIcon(":/resources/prevpagecontinuous.png"),tr("Continuous Previous Page"),this);
    previousPageContinuousAct->setShortcut(tr("Ctrl+Shift+E"));
    previousPageContinuousAct->setStatusTip(tr("Continuously process previous page to start of document - Ctrl+Shift+E"));
    previousPageContinuousAct->setEnabled(false);
    connect(previousPageContinuousAct, SIGNAL(triggered()), this, SLOT(previousPageContinuous()));

    QString pageString = "";
    setPageLineEdit = new QLineEdit(pageString,this);
    QSize size = setPageLineEdit->sizeHint();
    size.setWidth(size.width()/3);
    setPageLineEdit->setMinimumSize(size);
    setPageLineEdit->setToolTip("Current Page Index");
    setPageLineEdit->setStatusTip("Enter index and hit enter to go to page");
    setPageLineEdit->setEnabled(false);
    connect(setPageLineEdit, SIGNAL(returnPressed()), this, SLOT(setPage()));

    clearPLICacheAct = new QAction(QIcon(":/resources/clearplicache.png"),tr("Reset Parts Image Cache"), this);
    clearPLICacheAct->setShortcut(tr("Alt+R"));
    clearPLICacheAct->setStatusTip(tr("Reset the parts list image cache - Alt+R"));
    connect(clearPLICacheAct, SIGNAL(triggered()), this, SLOT(clearPLICache()));

    clearCSICacheAct = new QAction(QIcon(":/resources/clearcsicache.png"),tr("Reset Assembly Image Cache"), this);
    clearCSICacheAct->setShortcut(tr("Alt+S"));
    clearCSICacheAct->setStatusTip(tr("Reset the assembly image cache - Alt+S"));
    connect(clearCSICacheAct, SIGNAL(triggered()), this, SLOT(clearCSICache()));

    clearSubmodelCacheAct = new QAction(QIcon(":/resources/clearsubmodelcache.png"),tr("Reset Submodel Image Cache"), this);
    clearSubmodelCacheAct->setShortcut(tr("Alt+E"));
    clearSubmodelCacheAct->setStatusTip(tr("Reset the submodel image cache - Alt+E"));
    connect(clearSubmodelCacheAct, SIGNAL(triggered()), this, SLOT(clearSubmodelCache()));

    clearTempCacheAct = new QAction(QIcon(":/resources/cleartempcache.png"),tr("Reset Temp File Cache"), this);
    clearTempCacheAct->setShortcut(tr("Alt+T"));
    clearTempCacheAct->setStatusTip(tr("Reset the Temp file and 3D viewer image cache - Alt+T"));
    connect(clearTempCacheAct, SIGNAL(triggered()), this, SLOT(clearTempCache()));

    clearAllCachesAct = new QAction(QIcon(":/resources/clearimagemodelcache.png"),tr("Reset Model Caches"), this);
    clearAllCachesAct->setShortcut(tr("Alt+M"));
    clearAllCachesAct->setStatusTip(tr("Reset model file Parts, Assembly and Temp file caches - Alt+M"));
    connect(clearAllCachesAct, SIGNAL(triggered()), this, SLOT(clearAllCaches()));

    clearCustomPartCacheAct = new QAction(QIcon(":/resources/clearcustompartcache.png"),tr("Reset Custom Files Cache"), this);
    clearCustomPartCacheAct->setShortcut(tr("Alt+C"));
    clearCustomPartCacheAct->setStatusTip(tr("Reset fade and highlight part files cache - Alt+C"));
    connect(clearCustomPartCacheAct, SIGNAL(triggered()), this, SLOT(clearCustomPartCache()));

    refreshLDrawUnoffPartsAct = new QAction(QIcon(":/resources/refreshunoffarchive.png"),tr("Refresh LDraw Unofficial Parts"), this);
    refreshLDrawUnoffPartsAct->setStatusTip(tr("Download and replace LDraw Unofficial parts archive file in User data"));
    refreshLDrawUnoffPartsAct->setEnabled(Preferences::usingDefaultLibrary);
    connect(refreshLDrawUnoffPartsAct, SIGNAL(triggered()), this, SLOT(refreshLDrawUnoffParts()));

    refreshLDrawOfficialPartsAct = new QAction(QIcon(":/resources/refreshoffarchive.png"),tr("Refresh LDraw Official Parts"), this);
    refreshLDrawOfficialPartsAct->setStatusTip(tr("Download and replace LDraw Official parts archive file in User data"));
    refreshLDrawUnoffPartsAct->setEnabled(Preferences::usingDefaultLibrary);
    connect(refreshLDrawOfficialPartsAct, SIGNAL(triggered()), this, SLOT(refreshLDrawOfficialParts()));

    // Config menu

    pageSetupAct = new QAction(QIcon(":/resources/pagesetup.png"),tr("Page Setup"), this);
    pageSetupAct->setEnabled(false);
    pageSetupAct->setStatusTip(tr("Default values for your project's pages"));
    connect(pageSetupAct, SIGNAL(triggered()), this, SLOT(pageSetup()));

    assemSetupAct = new QAction(QIcon(":/resources/assemblysetup.png"),tr("Assembly Setup"), this);
    assemSetupAct->setEnabled(false);
    assemSetupAct->setStatusTip(tr("Default values for your project's assembly images"));
    connect(assemSetupAct, SIGNAL(triggered()), this, SLOT(assemSetup()));

    pliSetupAct = new QAction(QIcon(":/resources/partslistsetup.png"),tr("Parts List Setup"), this);
    pliSetupAct->setEnabled(false);
    pliSetupAct->setStatusTip(tr("Default values for your project's parts lists"));
    connect(pliSetupAct, SIGNAL(triggered()), this, SLOT(pliSetup()));

    bomSetupAct = new QAction(QIcon(":/resources/bomsetup.png"),tr("Bill of Materials Setup"), this);
    bomSetupAct->setEnabled(false);
    bomSetupAct->setStatusTip(tr("Default values for your project's bill of materials"));
    connect(bomSetupAct, SIGNAL(triggered()), this, SLOT(bomSetup()));

    calloutSetupAct = new QAction(QIcon(":/resources/calloutsetup.png"),tr("Callout Setup"), this);
    calloutSetupAct->setEnabled(false);
    calloutSetupAct->setStatusTip(tr("Default values for your project's callouts"));
    connect(calloutSetupAct, SIGNAL(triggered()), this, SLOT(calloutSetup()));

    multiStepSetupAct = new QAction(QIcon(":/resources/stepgroupsetup.png"),tr("Step Group Setup"), this);
    multiStepSetupAct->setEnabled(false);
    multiStepSetupAct->setStatusTip(tr("Default values for your project's step groups"));
    connect(multiStepSetupAct, SIGNAL(triggered()), this, SLOT(multiStepSetup()));

    subModelSetupAct = new QAction(QIcon(":/resources/submodelsetup.png"),tr("Submodel Preview Setup"), this);
    subModelSetupAct->setEnabled(false);
    subModelSetupAct->setStatusTip(tr("Default values for your submodel preview at first step"));
    connect(subModelSetupAct, SIGNAL(triggered()), this, SLOT(subModelSetup()));

    projectSetupAct = new QAction(QIcon(":/resources/projectsetup.png"),tr("Project Setup"), this);
    projectSetupAct->setEnabled(false);
    projectSetupAct->setStatusTip(tr("Default values for your project"));
    connect(projectSetupAct, SIGNAL(triggered()), this, SLOT(projectSetup()));

    fadeStepSetupAct = new QAction(QIcon(":/resources/fadestepsetup.png"),tr("Fade Step Setup"), this);
    fadeStepSetupAct->setEnabled(false);
    fadeStepSetupAct->setStatusTip(tr("Fade parts in previous step step"));
    connect(fadeStepSetupAct, SIGNAL(triggered()), this, SLOT(fadeStepSetup()));

    highlightStepSetupAct = new QAction(QIcon(":/resources/highlightstepsetup.png"),tr("Highlight Step Setup"), this);
    highlightStepSetupAct->setEnabled(false);
    highlightStepSetupAct->setStatusTip(tr("Highlight parts in current step"));
    connect(highlightStepSetupAct, SIGNAL(triggered()), this, SLOT(highlightStepSetup()));

    preferencesAct = new QAction(QIcon(":/resources/preferences.png"),tr("Preferences"), this);
    preferencesAct->setStatusTip(tr("Set your preferences for LPub3D"));
    connect(preferencesAct, SIGNAL(triggered()), this, SLOT(preferences()));

    editTitleAnnotationsAct = new QAction(QIcon(":/resources/edittitleannotations.png"),tr("Edit Part Title PLI Annotations list"), this);
    editTitleAnnotationsAct->setStatusTip(tr("Add/Edit part title PLI part annotatons"));
    connect(editTitleAnnotationsAct, SIGNAL(triggered()), this, SLOT(editTitleAnnotations()));

    editFreeFormAnnitationsAct = new QAction(QIcon(":/resources/editfreeformannotations.png"),tr("Edit Freeform PLI Annotations list"), this);
    editFreeFormAnnitationsAct->setStatusTip(tr("Add/Edit freeform PLI part annotations"));
    connect(editFreeFormAnnitationsAct, SIGNAL(triggered()), this, SLOT(editFreeFormAnnitations()));

    editLDrawColourPartsAct = new QAction(QIcon(":/resources/editldrawcolourparts.png"),tr("Edit LDraw Static Color Parts list"), this);
    editLDrawColourPartsAct->setStatusTip(tr("Add/Edit the list of LDraw static color parts used to process fade and highlight steps"));
    connect(editLDrawColourPartsAct, SIGNAL(triggered()), this, SLOT(editLDrawColourParts()));

    editPliBomSubstitutePartsAct = new QAction(QIcon(":/resources/editplisubstituteparts.png"),tr("Edit PLI/BOM Substitute Parts list"), this);
    editPliBomSubstitutePartsAct->setStatusTip(tr("Add/Edit the list of PLI/BOM substitute parts"));
    connect(editPliBomSubstitutePartsAct, SIGNAL(triggered()), this, SLOT(editPliBomSubstituteParts()));

    editExcludedPartsAct = new QAction(QIcon(":/resources/editexcludedparts.png"),tr("Edit Part Count Excluded Parts list"), this);
    editExcludedPartsAct->setStatusTip(tr("Add/Edit the list of part count excluded parts"));
    connect(editExcludedPartsAct, SIGNAL(triggered()), this, SLOT(editExcludedParts()));

    editLdrawIniFileAct = new QAction(QIcon(":/resources/editinifile.png"),tr("Edit LDraw INI search directories"), this);
    editLdrawIniFileAct->setStatusTip(tr("Add/Edit LDraw.ini search directory entries"));
    connect(editLdrawIniFileAct, SIGNAL(triggered()), this, SLOT(editLdrawIniFile()));

    editLdgliteIniAct = new QAction(QIcon(":/resources/editldgliteconf.png"),tr("Edit LDGLite INI configuration file"), this);
    editLdgliteIniAct->setStatusTip(tr("Edit LDGLite INI configuration file"));
    connect(editLdgliteIniAct, SIGNAL(triggered()), this, SLOT(editLdgliteIni()));

    editNativePOVIniAct = new QAction(QIcon(":/resources/LPub32.png"),tr("Edit Native POV file generation configuration file"), this);
    editNativePOVIniAct->setStatusTip(tr("Edit Native POV file generation configuration file"));
    connect(editNativePOVIniAct, SIGNAL(triggered()), this, SLOT(editNativePovIni()));

    editLdviewIniAct = new QAction(QIcon(":/resources/editldviewconf.png"),tr("Edit LDView INI configuration file"), this);
    editLdviewIniAct->setStatusTip(tr("Edit LDView INI configuration file"));
    connect(editLdviewIniAct, SIGNAL(triggered()), this, SLOT(editLdviewIni()));

    editLPub3DIniFileAct = new QAction(QIcon(":/resources/editsetting.png"),tr("Edit %1 configuration file").arg(VER_PRODUCTNAME_STR), this);
    editLPub3DIniFileAct->setStatusTip(tr("Edit %1 application configuration settings file").arg(VER_PRODUCTNAME_STR));
    connect(editLPub3DIniFileAct, SIGNAL(triggered()), this, SLOT(editLPub3DIniFile()));

    editLdviewPovIniAct = new QAction(QIcon(":/resources/editldviewconf.png"),tr("Edit LDView POV file generation configuration file"), this);
    editLdviewPovIniAct->setStatusTip(tr("Edit LDView POV file generation configuration file"));
    connect(editLdviewPovIniAct, SIGNAL(triggered()), this, SLOT(editLdviewPovIni()));

    editPovrayIniAct = new QAction(QIcon(":/resources/editpovrayconf.png"),tr("Edit Raytracer (POV-Ray) INI configuration file"), this);
    editPovrayIniAct->setStatusTip(tr("Edit Raytracer (POV-Ray) INI configuration file"));
    connect(editPovrayIniAct, SIGNAL(triggered()), this, SLOT(editPovrayIni()));

    editPovrayConfAct = new QAction(QIcon(":/resources/editpovrayconf.png"),tr("Edit Raytracer (POV-Ray) file access configuration file"), this);
    editPovrayConfAct->setStatusTip(tr("Edit Raytracer (POV-Ray) file access configuration file"));
    connect(editPovrayConfAct, SIGNAL(triggered()), this, SLOT(editPovrayConf()));

    editAnnotationStyleAct = new QAction(QIcon(":/resources/editstyleref.png"),tr("Edit Part Annotation Style reference"), this);
    editAnnotationStyleAct->setStatusTip(tr("Add/edit LDraw Design ID, Part Annotation Style, Part Category reference"));
    connect(editAnnotationStyleAct, SIGNAL(triggered()), this, SLOT(editAnnotationStyle()));

    editLD2BLCodesXRefAct = new QAction(QIcon(":/resources/editld2blxref.png"),tr("Edit LDraw to Bricklink Design ID reference"), this);
    editLD2BLCodesXRefAct->setStatusTip(tr("Add/Edit LDraw to Bricklink Design ID reference"));
    connect(editLD2BLCodesXRefAct, SIGNAL(triggered()), this, SLOT(editLD2BLCodesXRef()));

    editLD2BLColorsXRefAct = new QAction(QIcon(":/resources/editld2blxref.png"),tr("Edit LDraw to Bricklink Color reference"), this);
    editLD2BLColorsXRefAct->setStatusTip(tr("Add/edit LDraw LDConfig to Bricklink Color ID reference"));
    connect(editLD2BLColorsXRefAct, SIGNAL(triggered()), this, SLOT(editLD2BLColorsXRef()));

    editLD2RBCodesXRefAct = new QAction(QIcon(":/resources/editld2rbxref.png"),tr("Edit LDraw to Rebrickable Design ID reference"), this);
    editLD2RBCodesXRefAct->setStatusTip(tr("Add/Edit LDraw to Rebrickable Design ID reference"));
    connect(editLD2RBCodesXRefAct, SIGNAL(triggered()), this, SLOT(editLD2RBCodesXRef()));

    editLD2RBColorsXRefAct = new QAction(QIcon(":/resources/editld2rbxref.png"),tr("Edit LDraw to Rebrickable Color reference"), this);
    editLD2RBColorsXRefAct->setStatusTip(tr("Add/edit LDraw LDConfig to Rebrickable Color ID reference"));
    connect(editLD2RBColorsXRefAct, SIGNAL(triggered()), this, SLOT(editLD2RBColorsXRef()));

    editBLColorsAct = new QAction(QIcon(":/resources/editld2blxref.png"),tr("Edit Bricklink Color reference"), this);
    editBLColorsAct->setStatusTip(tr("Add/edit Bricklink Color ID reference"));
    connect(editBLColorsAct, SIGNAL(triggered()), this, SLOT(editBLColors()));

    editBLCodesAct = new QAction(QIcon(":/resources/editld2blxref.png"),tr("Edit Bricklink Codes reference"), this);
    editBLCodesAct->setStatusTip(tr("Add/edit Bricklink Item No, Color Name, LEGO Element reference"));
    connect(editBLCodesAct, SIGNAL(triggered()), this, SLOT(editBLCodes()));

    editModelFileAct = new QAction(QIcon(":/resources/editldraw.png"),tr("Edit current model file"), this);
    editModelFileAct->setStatusTip(tr("Edit loaded LDraw model file"));
    connect(editModelFileAct, SIGNAL(triggered()), this, SLOT(editModelFile()));

    generateCustomColourPartsAct = new QAction(QIcon(":/resources/generatecolourparts.png"),tr("Generate Static Color Parts list"), this);
    generateCustomColourPartsAct->setStatusTip(tr("Generate list of all static coloured parts"));
    connect(generateCustomColourPartsAct, SIGNAL(triggered()), this, SLOT(generateCustomColourPartsList()));

    // Help

    aboutAct = new QAction(QIcon(":/resources/LPub32.png"),tr("&About %1...").arg(VER_PRODUCTNAME_STR), this);
    aboutAct->setStatusTip(tr("Display version, system and build information"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(aboutDialog()));

    // Begin Jaco's code

    onlineManualAct = new QAction(QIcon(":/resources/onlinemanual.png"),tr("&Online manual"), this);
    onlineManualAct->setStatusTip(tr("Visit the Online Manual Website."));
    connect(onlineManualAct, SIGNAL(triggered()), this, SLOT(onlineManual()));

    // End Jaco's code

    metaAct = new QAction(QIcon(":/resources/savemetacommands.png"),tr("&Save LPub Metacommands to File"), this);
    metaAct->setStatusTip(tr("Save a list of the known LPub meta commands to a file"));
    connect(metaAct, SIGNAL(triggered()), this, SLOT(meta()));

    updateAppAct = new QAction(QIcon(":/resources/softwareupdate.png"),tr("Check for &Updates..."), this);
    updateAppAct->setStatusTip(tr("Check if a newer version of  %1 is available for download").arg(VER_PRODUCTNAME_STR));
    connect(updateAppAct, SIGNAL(triggered()), this, SLOT(updateCheck()));

    viewLogAct = new QAction(QIcon(":/resources/viewlog.png"),tr("View %1 log").arg(VER_PRODUCTNAME_STR), this);
    viewLogAct->setShortcut(tr("Alt+L"));
    viewLogAct->setStatusTip(tr("View %1 log - Alt+L").arg(VER_PRODUCTNAME_STR));
    connect(viewLogAct, SIGNAL(triggered()), this, SLOT(viewLog()));

    openWorkingFolderAct = new QAction(QIcon(":/resources/openworkingfolder.png"),tr("Open working folder"), this);
    openWorkingFolderAct->setShortcut(tr("Alt+Shift+D"));
    openWorkingFolderAct->setStatusTip(tr("Open current model file working folder - Alt+Shift+D"));
    connect(openWorkingFolderAct, SIGNAL(triggered()), this, SLOT(openWorkingFolder()));

    create3DActions();
}

void Gui::loadPages(){

  MetaItem mi;
  int pageNum     = 0;
  setGoToPageCombo->setMaxCount(0);
  setGoToPageCombo->setMaxCount(1000);
  bool frontCoverPage = mi.frontCoverPageExist();
  bool backCoverPage  = mi.backCoverPageExist();

  for(int i=1;i <= maxPages;i++){
      QApplication::processEvents();
      pageNum++;
      if (frontCoverPage && i == 1){
          pageNum--;
          setGoToPageCombo->addItem(QString("Front Cover"));
        }
      else if (backCoverPage && i == maxPages){
          setGoToPageCombo->addItem(QString("Back Cover"));
        }
      else
        setGoToPageCombo->addItem(QString("Page %1").arg(QString::number(pageNum)));
    }
  setGoToPageCombo->setCurrentIndex(displayPageNum-1);
}

void Gui::enableActions()
{
  saveAsAct->setEnabled(true);
  saveCopyAct->setEnabled(true);
  closeFileAct->setEnabled(true);

  printToFileAct->setEnabled(true);
  printToFilePreviewAct->setEnabled(true);
  exportAsPdfAct->setEnabled(true);
  exportAsPdfPreviewAct->setEnabled(true);

  exportPngAct->setEnabled(true);
  exportJpgAct->setEnabled(true);
  exportBmpAct->setEnabled(true);

  pageSetupAct->setEnabled(true);
  assemSetupAct->setEnabled(true);
  pliSetupAct->setEnabled(true);
  bomSetupAct->setEnabled(true);
  calloutSetupAct->setEnabled(true);
  multiStepSetupAct->setEnabled(true);
  subModelSetupAct->setEnabled(true);
  projectSetupAct->setEnabled(true);
  fadeStepSetupAct->setEnabled(true);
  highlightStepSetupAct->setEnabled(true);

  addPictureAct->setEnabled(true);
  removeLPubFormattingAct->setEnabled(true);

  editTitleAnnotationsAct->setEnabled(true);
  editFreeFormAnnitationsAct->setEnabled(true);
  editLDrawColourPartsAct->setEnabled(true);
  editPliBomSubstitutePartsAct->setEnabled(true);
  editExcludedPartsAct->setEnabled(true);
  editLdgliteIniAct->setEnabled(true);
  editNativePOVIniAct->setEnabled(true);
  editLdviewIniAct->setEnabled(true);
  editLdviewPovIniAct->setEnabled(true);
  editPovrayIniAct->setEnabled(true);
  editPovrayConfAct->setEnabled(true);
  editAnnotationStyleAct->setEnabled(true);
  editLD2BLCodesXRefAct->setEnabled(true);
  editLD2BLColorsXRefAct->setEnabled(true);
  editLD2RBCodesXRefAct->setEnabled(true);
  editLD2RBColorsXRefAct->setEnabled(true);
  editBLColorsAct->setEnabled(true);
  editBLCodesAct->setEnabled(true);
  editModelFileAct->setEnabled(true);

  openWorkingFolderAct->setEnabled(true);
  setPageLineEdit->setEnabled(true);

  firstPageAct->setEnabled(true);
  lastPageAct->setEnabled(true);
  nextPageAct->setEnabled(true);
  previousPageAct->setEnabled(true);
  nextPageComboAct->setEnabled(true);
  previousPageComboAct->setEnabled(true);
  nextPageContinuousAct->setEnabled(true);
  previousPageContinuousAct->setEnabled(true);

  fitWidthAct->setEnabled(true);
  fitVisibleAct->setEnabled(true);
  fitSceneAct->setEnabled(true);
  bringToFrontAct->setEnabled(true);
  sendToBackAct->setEnabled(true);
  actualSizeAct->setEnabled(true);
  zoomInComboAct->setEnabled(true);
  zoomOutComboAct->setEnabled(true);
  sceneGuidesComboAct->setEnabled(true);
  snapToGridComboAct->setEnabled(true);

  setupMenu->setEnabled(true);
  cacheMenu->setEnabled(true);
  exportMenu->setEnabled(true);
  openWithMenu->setEnabled(numPrograms > 0);

  exportBricklinkAct->setEnabled(true);
  exportCsvAct->setEnabled(true);
  exportPovAct->setEnabled(true);
  exportStlAct->setEnabled(true);
  export3dsAct->setEnabled(true);
  exportObjAct->setEnabled(true);
  exportColladaAct->setEnabled(true);
  exportHtmlAct->setEnabled(true);
  exportHtmlStepsAct->setEnabled(true);

  povrayRenderAct->setEnabled(true);

  //3DViewer
  //ViewerExportMenu->setEnabled(true); // Hide 3DViewer step export functions

}

void Gui::disableActions()
{
  saveAsAct->setEnabled(false);
  saveCopyAct->setEnabled(false);
  closeFileAct->setEnabled(false);

  printToFilePreviewAct->setEnabled(false);
  printToFileAct->setEnabled(false);
  exportAsPdfPreviewAct->setEnabled(false);
  exportAsPdfAct->setEnabled(false);

  exportPngAct->setEnabled(false);
  exportJpgAct->setEnabled(false);
  exportBmpAct->setEnabled(false);

  pageSetupAct->setEnabled(false);
  assemSetupAct->setEnabled(false);
  pliSetupAct->setEnabled(false);
  bomSetupAct->setEnabled(false);
  calloutSetupAct->setEnabled(false);
  multiStepSetupAct->setEnabled(false);
  subModelSetupAct->setEnabled(false);
  projectSetupAct->setEnabled(false);
  fadeStepSetupAct->setEnabled(false);
  highlightStepSetupAct->setEnabled(false);

  addPictureAct->setEnabled(false);
  removeLPubFormattingAct->setEnabled(false);

  editModelFileAct->setEnabled(false);

  openWorkingFolderAct->setEnabled(false);
  setPageLineEdit->setEnabled(false);

  firstPageAct->setEnabled(false);
  lastPageAct->setEnabled(false);
  nextPageAct->setEnabled(false);
  previousPageAct->setEnabled(false);
  nextPageComboAct->setEnabled(false);
  previousPageComboAct->setEnabled(false);
  nextPageContinuousAct->setEnabled(false);
  previousPageContinuousAct->setEnabled(false);

  fitWidthAct->setEnabled(false);
  fitVisibleAct->setEnabled(false);
  fitSceneAct->setEnabled(false);
  bringToFrontAct->setEnabled(false);
  sendToBackAct->setEnabled(false);
  actualSizeAct->setEnabled(false);
  zoomInComboAct->setEnabled(false);
  zoomOutComboAct->setEnabled(false);

  setupMenu->setEnabled(false);
  cacheMenu->setEnabled(false);
  exportMenu->setEnabled(false);
  openWithMenu->setEnabled(false);

  exportBricklinkAct->setEnabled(false);
  exportCsvAct->setEnabled(false);
  exportPovAct->setEnabled(false);
  exportStlAct->setEnabled(false);
  export3dsAct->setEnabled(false);
  exportObjAct->setEnabled(false);
  exportColladaAct->setEnabled(false);
  exportHtmlAct->setEnabled(false);
  exportHtmlStepsAct->setEnabled(false);

  povrayRenderAct->setEnabled(false);

  // 3DViewer
  // ViewerExportMenu->setEnabled(false); // Hide 3DViewer step export functions

}

void Gui::enableActions2()
{
    MetaItem mi;
    insertCoverPageAct->setEnabled(mi.okToInsertCoverPage() &&
                                   ! mi.frontCoverPageExist());
    appendCoverPageAct->setEnabled(mi.okToAppendCoverPage() &&
                                   ! mi.backCoverPageExist());
    bool frontCover = mi.okToInsertNumberedPage();
    insertNumberedPageAct->setEnabled(frontCover);
    bool backCover = mi.okToAppendNumberedPage();
    appendNumberedPageAct->setEnabled(backCover);
    deletePageAct->setEnabled(page.list.size() == 0);
    addBomAct->setEnabled(frontCover||backCover);
    addTextAct->setEnabled(true);

    loadPages();
}

void Gui::disableActions2()
{
    insertCoverPageAct->setEnabled(false);
    appendCoverPageAct->setEnabled(false);
    insertNumberedPageAct->setEnabled(false);
    appendNumberedPageAct->setEnabled(false);
    deletePageAct->setEnabled(false);
    addBomAct->setEnabled(false);
    addTextAct->setEnabled(false);
}

void Gui::createMenus()
{
    // Editor Menus

    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    openWithMenu = fileMenu->addMenu(tr("Open With..."));
    openWithMenu->setIcon(QIcon(":/resources/openwith.png"));
    openWithMenu->setStatusTip(tr("Open model file with selected application"));
    for (int i = 0; i < Preferences::maxOpenWithPrograms; i++) {
      openWithMenu->addAction(openWithActList.at(i));
    }
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(saveCopyAct);
    fileMenu->addAction(closeFileAct);

    exportMenu = fileMenu->addMenu("Export As...");
    exportMenu->setIcon(QIcon(":/resources/exportas.png"));
    exportMenu->setStatusTip(tr("Export model file images, objects or part lists"));
    exportMenu->addAction(exportPngAct);
    exportMenu->addAction(exportJpgAct);
#ifdef Q_OS_WIN
    exportMenu->addAction(exportBmpAct);
#endif
    exportMenu->addSeparator();
    exportMenu->addAction(exportObjAct);
    exportMenu->addAction(exportStlAct);
    exportMenu->addAction(exportPovAct);
    exportMenu->addAction(exportColladaAct);
    exportMenu->addAction(export3dsAct);
    exportMenu->addSeparator();
    exportMenu->addAction(exportHtmlAct);
    exportMenu->addAction(exportHtmlStepsAct);
    exportMenu->addAction(exportBricklinkAct);
    exportMenu->addAction(exportCsvAct);
    exportMenu->addSeparator();
    exportMenu->setDisabled(true);

    fileMenu->addAction(printToFilePreviewAct);
    fileMenu->addAction(printToFileAct);
    fileMenu->addAction(exportAsPdfPreviewAct);
    fileMenu->addAction(exportAsPdfAct);
    fileMenu->addSeparator();

    recentFileMenu = fileMenu->addMenu(tr("Recent Files..."));
    recentFileMenu->setIcon(QIcon(":/resources/recentfiles.png"));
    recentFileMenu->setStatusTip(tr("Open recent model file"));
    for (int i = 0; i < MaxRecentFiles; i++) {
      recentFileMenu->addAction(recentFilesActs[i]);
    }
    separatorAct = recentFileMenu->addSeparator();
    recentFileMenu->addAction(clearRecentAct);

    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    menuBar()->addSeparator();

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);

    editMenu->addSeparator();

    editMenu->addAction(insertCoverPageAct);
    editMenu->addAction(appendCoverPageAct);
    editMenu->addAction(insertNumberedPageAct);
    editMenu->addAction(appendNumberedPageAct);
    editMenu->addAction(deletePageAct);
    editMenu->addAction(addPictureAct);
    editMenu->addAction(addTextAct);
    editMenu->addAction(addBomAct);
    editMenu->addAction(removeLPubFormattingAct);
    editMenu->addSeparator();

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(fitWidthAct);
    viewMenu->addAction(fitVisibleAct);
    viewMenu->addAction(actualSizeAct);
    viewMenu->addAction(fitSceneAct);
    viewMenu->addAction(bringToFrontAct);
    viewMenu->addAction(sendToBackAct);
    viewMenu->addAction(zoomInComboAct);
    viewMenu->addAction(zoomOutComboAct);
    viewMenu->addAction(sceneRulerComboAct);
    viewMenu->addAction(sceneGuidesComboAct);
    viewMenu->addAction(snapToGridComboAct);
    viewMenu->addSeparator();

    toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(firstPageAct);
    toolsMenu->addAction(nextPageAct);
    toolsMenu->addAction(nextPageContinuousAct);
    toolsMenu->addAction(previousPageAct);
    toolsMenu->addAction(previousPageContinuousAct);
    toolsMenu->addAction(lastPageAct);
    toolsMenu->addSeparator();

    cacheMenu = toolsMenu->addMenu("Reset Cache");
    cacheMenu->setIcon(QIcon(":/resources/resetcache.png"));
    cacheMenu->setStatusTip(tr("Reset working caches"));
    cacheMenu->addAction(clearAllCachesAct);
    cacheMenu->addAction(clearPLICacheAct);
    cacheMenu->addAction(clearSubmodelCacheAct);
    cacheMenu->addAction(clearCSICacheAct);
    cacheMenu->addAction(clearTempCacheAct);
    cacheMenu->addAction(clearCustomPartCacheAct);
    cacheMenu->addSeparator();
    cacheMenu->setDisabled(true);

    toolsMenu->addAction(refreshLDrawUnoffPartsAct);
    toolsMenu->addAction(refreshLDrawOfficialPartsAct);
    toolsMenu->addSeparator();

    configMenu = menuBar()->addMenu(tr("&Configuration"));
    setupMenu = configMenu->addMenu("Build &Instructions Setup...");
    setupMenu->setIcon(QIcon(":/resources/instructionsetup.png"));
    setupMenu->setStatusTip(tr("Instruction document global settings"));
    setupMenu->addAction(pageSetupAct);
    setupMenu->addAction(assemSetupAct);
    setupMenu->addAction(pliSetupAct);
    setupMenu->addAction(bomSetupAct);
    setupMenu->addAction(calloutSetupAct);
    setupMenu->addAction(multiStepSetupAct);
    setupMenu->addAction(subModelSetupAct);
    setupMenu->addAction(projectSetupAct);
    setupMenu->addAction(fadeStepSetupAct);
    setupMenu->addAction(highlightStepSetupAct);
    setupMenu->addSeparator();
    setupMenu->setDisabled(true);

    configMenu->addSeparator();
    editorMenu = configMenu->addMenu("Edit Parameter Files...");
    editorMenu->setIcon(QIcon(":/resources/editparameterfiles.png"));
    editorMenu->setStatusTip(tr("Edit %1 parameter files").arg(VER_PRODUCTNAME_STR));
    editorMenu->addAction(editLDrawColourPartsAct);
    editorMenu->addAction(editTitleAnnotationsAct);
    editorMenu->addAction(editFreeFormAnnitationsAct);
    editorMenu->addAction(editPliBomSubstitutePartsAct);
    editorMenu->addAction(editExcludedPartsAct);
    editorMenu->addAction(editAnnotationStyleAct);
    editorMenu->addAction(editLD2BLCodesXRefAct);
    editorMenu->addAction(editLD2BLColorsXRefAct);
    editorMenu->addAction(editBLColorsAct);
    editorMenu->addAction(editBLCodesAct);
    editorMenu->addAction(editLD2RBColorsXRefAct);
    editorMenu->addAction(editLD2RBCodesXRefAct);
    if (Preferences::ldrawiniFound){
      editorMenu->addAction(editLdrawIniFileAct);
    }
    editorMenu->addSeparator();
#if defined Q_OS_WIN
    if (Preferences::portableDistribution){
      editorMenu->addAction(editLPub3DIniFileAct);
      editorMenu->addSeparator();
    }
#else
    editorMenu->addAction(editLPub3DIniFileAct);
    editorMenu->addSeparator();
#endif
    editorMenu->addAction(editNativePOVIniAct);
    editorMenu->addAction(editLdgliteIniAct);
    editorMenu->addAction(editLdviewIniAct);
    editorMenu->addAction(editLdviewPovIniAct);
    editorMenu->addAction(editPovrayIniAct);
    editorMenu->addAction(editPovrayConfAct);
    editorMenu->addSeparator();

    configMenu->addAction(editModelFileAct);
    configMenu->addAction(generateCustomColourPartsAct);
    configMenu->addSeparator();
    configMenu->addAction(openWithSetupAct);
    configMenu->addAction(preferencesAct);

    // 3DViewer

    create3DMenus();

    // Help Menus

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(openWorkingFolderAct);
    helpMenu->addAction(viewLogAct);
#ifndef DISABLE_UPDATE_CHECK
    helpMenu->addAction(updateAppAct);
#endif
    // Begin Jaco's code
    helpMenu->addAction(onlineManualAct);
    // End Jaco's code
    helpMenu->addAction(metaAct);
    helpMenu->addSeparator();
    // About Editor
    helpMenu->addAction(aboutAct);

    previousPageContinuousMenu = new QMenu(tr("Continuous Page Previous"), this);
    previousPageContinuousMenu->addAction(previousPageContinuousAct);
    nextPageContinuousMenu = new QMenu(tr("Continuous Page Next"), this);
    nextPageContinuousMenu->addAction(nextPageContinuousAct);

    zoomSliderMenu = new QMenu(tr("Zoom Slider"),this);
    zoomSliderMenu->addAction(zoomSliderAct);

    sceneRulerTrackingMenu = new QMenu(tr("Ruler Tracking"),this);
    sceneRulerTrackingMenu->addAction(hideRulerPageBackgroundAct);
    sceneRulerTrackingMenu->addSeparator();
    sceneRulerTrackingMenu->addAction(sceneRulerTrackingNoneAct);
    sceneRulerTrackingMenu->addAction(sceneRulerTrackingTickAct);
    sceneRulerTrackingMenu->addAction(sceneRulerTrackingLineAct);
    sceneRulerTrackingMenu->addSeparator();
    sceneRulerTrackingMenu->addAction(showTrackingCoordinatesAct);

    sceneGuidesMenu = new QMenu(tr("Scene Guides"),this);
    sceneGuidesMenu->addAction(sceneGuidesDashLineAct);
    sceneGuidesMenu->addAction(sceneGuidesSolidLineAct);
    sceneGuidesMenu->addSeparator();
    sceneGuidesMenu->addAction(sceneGuidesPosTLeftAct);
    sceneGuidesMenu->addAction(sceneGuidesPosTRightAct);
    sceneGuidesMenu->addAction(sceneGuidesPosCentreAct);
    sceneGuidesMenu->addAction(sceneGuidesPosBLeftAct);
    sceneGuidesMenu->addAction(sceneGuidesPosBRightAct);
    sceneGuidesMenu->addSeparator();
    sceneGuidesMenu->addAction(showGuidesCoordinatesAct);

    snapToGridMenu = new QMenu(tr("Snap to Grid"), this);
    snapToGridMenu->addAction(hideGridPageBackgroundAct);
    snapToGridMenu->addSeparator();
    for (int actionIdx = GRID_SIZE_FIRST; actionIdx <= GRID_SIZE_LAST; actionIdx++)
        snapToGridMenu->addAction(snapGridActions[actionIdx]);
}

void Gui::createToolBars()
{
    QSettings Settings;

    fileToolBar = addToolBar(tr("File Toolbar"));
    fileToolBar->setObjectName("FileToolbar");
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    fileToolBar->addAction(saveAsAct);
    //fileToolBar->addAction(saveCopyAct);
    fileToolBar->addAction(closeFileAct);

    fileToolBar->addAction(printToFilePreviewAct);
    fileToolBar->addAction(printToFileAct);
    fileToolBar->addAction(exportAsPdfPreviewAct);
    fileToolBar->addAction(exportAsPdfAct);

    exportToolBar = addToolBar(tr("Export Toolbar"));
    exportToolBar->setObjectName("ExportToolbar");
    exportToolBar->addAction(exportPngAct);
    exportToolBar->addAction(exportJpgAct);
#ifdef Q_OS_WIN
    exportToolBar->addAction(exportBmpAct);
#endif
    exportToolBar->addSeparator();
    exportToolBar->addAction(exportObjAct);
    exportToolBar->addAction(exportStlAct);
    exportToolBar->addAction(exportPovAct);
    exportToolBar->addAction(exportColladaAct);
    exportToolBar->addSeparator();
    exportToolBar->addAction(exportHtmlAct);
    exportToolBar->addAction(exportHtmlStepsAct);
    exportToolBar->addAction(exportBricklinkAct);
    exportToolBar->addAction(exportCsvAct);
    bool visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS, VIEW_EXPORT_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS, VIEW_EXPORT_TOOLBAR_KEY)).toBool();
    exportMenu->addAction(exportToolBar->toggleViewAction());
    exportToolBar->setVisible(visible);
    connect (exportToolBar, SIGNAL (visibilityChanged(bool)),
                      this, SLOT (exportToolBarVisibilityChanged(bool)));

    undoredoToolBar = addToolBar(tr("UndoRedo Toolbar"));
    undoredoToolBar->setObjectName("UndoRedoToolbar");
    undoredoToolBar->addAction(undoAct);
    undoredoToolBar->addAction(redoAct);

    cacheToolBar = addToolBar(tr("Cache Toolbar"));
    cacheToolBar->setObjectName("CacheToolbar");
    cacheToolBar->addSeparator();
    cacheToolBar->addAction(clearAllCachesAct);
    cacheToolBar->addAction(clearPLICacheAct);
    cacheToolBar->addAction(clearSubmodelCacheAct);
    cacheToolBar->addAction(clearCSICacheAct);
    cacheToolBar->addAction(clearTempCacheAct);
    cacheToolBar->addAction(clearCustomPartCacheAct);
    visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_CACHE_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_CACHE_TOOLBAR_KEY)).toBool();
    cacheMenu->addAction(cacheToolBar->toggleViewAction());
    cacheToolBar->setVisible(visible);
    connect (cacheToolBar, SIGNAL (visibilityChanged(bool)),
                     this, SLOT (cacheToolBarVisibilityChanged(bool)));

    setupToolBar = addToolBar(tr("Global Setup Toolbar"));
    setupToolBar->setObjectName("ToolsToolbar");
    setupToolBar->addAction(pageSetupAct);
    setupToolBar->addAction(assemSetupAct);
    setupToolBar->addAction(pliSetupAct);
    setupToolBar->addAction(bomSetupAct);
    setupToolBar->addAction(calloutSetupAct);
    setupToolBar->addAction(multiStepSetupAct);
    setupToolBar->addAction(subModelSetupAct);
    setupToolBar->addAction(projectSetupAct);
    setupToolBar->addAction(fadeStepSetupAct);
    setupToolBar->addAction(highlightStepSetupAct);
    visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_SETUP_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_SETUP_TOOLBAR_KEY)).toBool();
    setupMenu->addAction(setupToolBar->toggleViewAction());
    setupToolBar->setVisible(visible);
    connect (setupToolBar, SIGNAL (visibilityChanged(bool)),
                     this, SLOT (setupToolBarVisibilityChanged(bool)));

    editToolBar = addToolBar(tr("Edit Toolbar"));
    editToolBar->setObjectName("EditToolbar");
    editToolBar->addAction(insertCoverPageAct);
    editToolBar->addAction(appendCoverPageAct);
    editToolBar->addAction(insertNumberedPageAct);
    editToolBar->addAction(appendNumberedPageAct);
    editToolBar->addAction(deletePageAct);
    editToolBar->addAction(addPictureAct);
    editToolBar->addAction(addTextAct);
    editToolBar->addAction(addBomAct);
    editToolBar->addAction(removeLPubFormattingAct);
    visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_EDIT_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_EDIT_TOOLBAR_KEY)).toBool();
    editMenu->addAction(editToolBar->toggleViewAction());
    editMenu->addAction(editWindow->editToolBar->toggleViewAction());
    editToolBar->setVisible(visible);
    connect (editToolBar, SIGNAL (visibilityChanged(bool)),
                     this, SLOT (editToolBarVisibilityChanged(bool)));

    editParamsToolBar = addToolBar(tr("Edit Parameters Toolbar"));
    editParamsToolBar->setObjectName("EditParamsToolbar");
    editParamsToolBar->addAction(editModelFileAct);
    editParamsToolBar->addSeparator();
    editParamsToolBar->addAction(editLDrawColourPartsAct);
    editParamsToolBar->addAction(editTitleAnnotationsAct);
    editParamsToolBar->addAction(editFreeFormAnnitationsAct);
    editParamsToolBar->addAction(editPliBomSubstitutePartsAct);
    editParamsToolBar->addAction(editExcludedPartsAct);
    editParamsToolBar->addAction(editAnnotationStyleAct);
    editParamsToolBar->addAction(editLD2BLCodesXRefAct);
    editParamsToolBar->addAction(editLD2BLColorsXRefAct);
    editParamsToolBar->addAction(editBLColorsAct);
    editParamsToolBar->addAction(editBLCodesAct);
    editParamsToolBar->addAction(editLD2RBColorsXRefAct);
    editParamsToolBar->addAction(editLD2RBCodesXRefAct);
    if (Preferences::ldrawiniFound){
        editParamsToolBar->addAction(editLdrawIniFileAct);
    }
    editParamsToolBar->addSeparator();
#if defined Q_OS_WIN
    if (Preferences::portableDistribution){
        editParamsToolBar->addAction(editLPub3DIniFileAct);
        editParamsToolBar->addSeparator();
    }
#else
    editParamsToolBar->addAction(editLPub3DIniFileAct);
    editParamsToolBar->addSeparator();
#endif
    editParamsToolBar->addAction(editNativePOVIniAct);
    editParamsToolBar->addAction(editLdgliteIniAct);
    editParamsToolBar->addAction(editLdviewIniAct);
    editParamsToolBar->addAction(editLdviewPovIniAct);
    editParamsToolBar->addAction(editPovrayIniAct);
    editParamsToolBar->addAction(editPovrayConfAct);
    editParamsToolBar->addSeparator();
    editParamsToolBar->addAction(generateCustomColourPartsAct);
    editParamsToolBar->addSeparator();
    editParamsToolBar->addAction(viewLogAct);
    visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_EDITPARAMS_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_EDITPARAMS_TOOLBAR_KEY)).toBool();
    editorMenu->addAction(editParamsToolBar->toggleViewAction());
    editParamsToolBar->setVisible(visible);
    connect (editParamsToolBar, SIGNAL (visibilityChanged(bool)),
                          this, SLOT (editParamsToolBarVisibilityChanged(bool)));

    navigationToolBar = addToolBar(tr("Navigation Toolbar"));
    navigationToolBar->setObjectName("NavigationToolbar");
    navigationToolBar->addAction(firstPageAct);

    previousPageComboAct->setMenu(previousPageContinuousMenu);
    navigationToolBar->addAction(previousPageComboAct);
    navigationToolBar->addWidget(setPageLineEdit);

    nextPageComboAct->setMenu(nextPageContinuousMenu);
    navigationToolBar->addAction(nextPageComboAct);
    navigationToolBar->addAction(lastPageAct);
    navigationToolBar->addWidget(setGoToPageCombo);

    mpdToolBar = addToolBar(tr("MPD Toolbar"));
    mpdToolBar->setObjectName("MPDToolbar");
    mpdToolBar->addWidget(mpdCombo);

    zoomToolBar = addToolBar(tr("Zoom Toolbar"));
    zoomToolBar->setObjectName("ZoomToolbar");
    zoomToolBar->addAction(fitVisibleAct);
    zoomToolBar->addAction(fitWidthAct);
    zoomToolBar->addAction(fitSceneAct);
    zoomToolBar->addAction(actualSizeAct);

    zoomToolBar->addAction(bringToFrontAct);
    zoomToolBar->addAction(sendToBackAct);

    zoomInComboAct->setMenu(zoomSliderMenu);
    zoomToolBar->addAction(zoomInComboAct);
    zoomOutComboAct->setMenu(zoomSliderMenu);
    zoomToolBar->addAction(zoomOutComboAct);

    sceneRulerComboAct->setMenu(sceneRulerTrackingMenu);
    zoomToolBar->addAction(sceneRulerComboAct);

    sceneGuidesComboAct->setMenu(sceneGuidesMenu);
    zoomToolBar->addAction(sceneGuidesComboAct);

    snapToGridComboAct->setMenu(snapToGridMenu);
    zoomToolBar->addAction(snapToGridComboAct);

    create3DToolBars();
}

void Gui::statusBarMsg(QString msg)
{
  statusBar()->showMessage(msg);
}

void Gui::createDockWindows()
{
    fileEditDockWindow = new QDockWidget(trUtf8(wCharToUtf8("LDraw\u2122 Editor")), this);
    fileEditDockWindow->setObjectName("LDrawFileDockWindow");
    fileEditDockWindow->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    fileEditDockWindow->setWidget(editWindow);
    addDockWidget(Qt::RightDockWidgetArea, fileEditDockWindow);
    viewMenu->addAction(fileEditDockWindow->toggleViewAction());

    create3DDockWindows();

    // launching with viewerDockWindow raised is not stable so start with fileEdit until I figure out what's wrong.
    fileEditDockWindow->raise();
//#ifdef Q_OS_MACOS
//    fileEditDockWindow->raise();
//#else
//    viewerDockWindow->raise();
//#endif
}

void Gui::exportToolBarVisibilityChanged(bool visible)
{
    QSettings Settings;
    Settings.setValue(QString("%1/%2").arg(SETTINGS,VIEW_EXPORT_TOOLBAR_KEY),visible);
}

void Gui::cacheToolBarVisibilityChanged(bool visible)
{
    QSettings Settings;
    Settings.setValue(QString("%1/%2").arg(SETTINGS,VIEW_CACHE_TOOLBAR_KEY),visible);
}

void Gui::setupToolBarVisibilityChanged(bool visible)
{
    QSettings Settings;
    Settings.setValue(QString("%1/%2").arg(SETTINGS,VIEW_SETUP_TOOLBAR_KEY),visible);
}

void Gui::editToolBarVisibilityChanged(bool visible)
{
    QSettings Settings;
    Settings.setValue(QString("%1/%2").arg(SETTINGS,VIEW_EDIT_TOOLBAR_KEY),visible);
}

void Gui::editParamsToolBarVisibilityChanged(bool visible)
{
    QSettings Settings;
    Settings.setValue(QString("%1/%2").arg(SETTINGS,VIEW_EDITPARAMS_TOOLBAR_KEY),visible);
}

void Gui::readSettings()
{
    QSettings Settings;
    Settings.beginGroup(MAINWINDOW);
    restoreState(Settings.value("State").toByteArray());
    QSize size = Settings.value("Size", QDesktopWidget().availableGeometry(this).size()*0.6).toSize();
    const QByteArray geometry = Settings.value("Geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
    resize(size);
    gMainWindow->mPartSelectionWidget->LoadState(Settings);
    Settings.endGroup();
}

void Gui::writeSettings()
{
    QSettings Settings;
    Settings.beginGroup(MAINWINDOW);
    Settings.setValue("Geometry", saveGeometry());
    Settings.setValue("State", saveState());
    Settings.setValue("Size", size());
    gMainWindow->mPartSelectionWidget->SaveState(Settings);
    Settings.endGroup();

    gApplication->SaveTabLayout();
}

void Gui::showLineMessage(QString errorMsg, Where &here, Preferences::MsgKey msgKey)
{
    if (parsedMessages.contains(here))
        return;

    QString parseMessage = QString("%1 (file: %2, line: %3)") .arg(errorMsg) .arg(here.modelName) .arg(here.lineNumber + 1);
    if (Preferences::modeGUI) {
        showLine(here);
        if (Preferences::lineParseErrors && Preferences::getShowMessagePreference(msgKey)) {
            QCheckBox *cb = new QCheckBox("Do not show this line message again.");
            QMessageBoxResizable box;
            box.setWindowTitle(tr(VER_PRODUCTNAME_STR " Line Message"));
            box.setText(parseMessage);
            box.setIcon(QMessageBox::Icon::Warning);
            box.addButton(QMessageBox::Ok);
            box.setDefaultButton(QMessageBox::Ok);
            box.setCheckBox(cb);

            QObject::connect(cb, &QCheckBox::stateChanged, [&msgKey](int state){
                if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked)
                    Preferences::setShowMessagePreference(false,msgKey);
                else
                    Preferences::setShowMessagePreference(true,msgKey);
            });
            box.adjustSize();
            box.exec();
        }
    }
    logError() << qPrintable(parseMessage);

    parsedMessages.append(here);
}

void Gui::statusMessage(LogType logType, QString message) {
    /* logTypes
     * LOG_STATUS:   - same as INFO but writes to log file also
     * LOG_INFO:
     * LOG_TRACE:
     * LOG_DEBUG:
     * LOG_NOTICE:
     * LOG_ERROR:
     * LOG_FATAL:
     * LOG_QWARNING: - visible in Qt debug mode
     * LOG_QDEBUG:   - visible in Qt debug mode
     */

    // Set StatusMessage Logging options
    using namespace QsLogging;
    Logger& logger = Logger::instance();
    if (Preferences::logging) {

        // Set logging Level
        bool ok;
        Level logLevel = logger.fromLevelString(Preferences::loggingLevel,&ok);
        if (!ok)
        {
            QString Message = QString("Failed to set log level %1.\n"
                                              "Logging is off - level set to OffLevel")
                    .arg(Preferences::loggingLevel);
            fprintf(stderr, "%s", Message.append("\n").toLatin1().constData());
        }
        logger.setLoggingLevel(logLevel);

        logger.setIncludeLogLevel(Preferences::includeLogLevel);
        logger.setIncludeTimestamp(Preferences::includeTimestamp);
        logger.setIncludeLineNumber(false);
        logger.setIncludeTimestamp(true);
        logger.setIncludeFileName(false);
        logger.setColorizeFunctionInfo(false);
        logger.setIncludeFunctionInfo(false);

        bool guiEnabled = (Preferences::modeGUI && Preferences::lpub3dLoaded);
        if (logType == LOG_STATUS ){

            logStatus() << message.replace("<br>"," ");

            if (guiEnabled) {
                statusBarMsg(message);
            } else {
                fprintf(stdout,"%s",QString(message).append("\n").toLatin1().constData());
                fflush(stdout);
            }
        } else
            if (logType == LOG_INFO) {

                logInfo() << message.replace("<br>"," ");

                if (!guiEnabled && !Preferences::suppressStdOutToLog) {
                    fprintf(stdout,"%s",QString(message).append("\n").toLatin1().constData());
                    fflush(stdout);
                }

            } else
              if (logType == LOG_NOTICE) {

                  logNotice() << message.replace("<br>"," ");

                  if (!guiEnabled && !Preferences::suppressStdOutToLog) {
                      fprintf(stdout,"%s",QString(message).append("\n").toLatin1().constData());
                      fflush(stdout);
                  }

            } else
              if (logType == LOG_TRACE) {

                  logTrace() << message.replace("<br>"," ");

                  if (!guiEnabled && !Preferences::suppressStdOutToLog) {
                      fprintf(stdout,"%s",QString(message).append("\n").toLatin1().constData());
                      fflush(stdout);
                  }

            } else
              if (logType == LOG_DEBUG) {
#ifdef QT_DEBUG_MODE
                  logDebug() << message.replace("<br>"," ");

                  if (!guiEnabled && !Preferences::suppressStdOutToLog) {
                      fprintf(stdout,"%s",QString(message).replace("<br>"," ").append("\n").toLatin1().constData());
                      fflush(stdout);
                  }
#endif
                } else
              if (logType == LOG_INFO_STATUS) {

                  statusBarMsg(message.replace("<br>"," "));

                  logInfo() << message;

                  if (!guiEnabled && !Preferences::suppressStdOutToLog) {
                      fprintf(stdout,"%s",QString(message).append("\n").toLatin1().constData());
                      fflush(stdout);
                  }
            } else
              if (logType == LOG_ERROR) {

                  logError() << QString(message).replace("<br>"," ");

                  if (guiEnabled) {
                      if (ContinuousPage()) {
                          statusBarMsg(QString(message).replace("<br>"," ").prepend("ERROR: "));
                      } else {
                          QMessageBox::warning(this,tr(VER_PRODUCTNAME_STR),tr(message.toLatin1()));
                      }
                  } else if (!Preferences::suppressStdOutToLog) {
                      fprintf(stdout,"%s",QString(message).replace("<br>"," ").append("\n").toLatin1().constData());
                      fflush(stdout);
                  }
            }

        // Reset Logging to default settings
        if (Preferences::logLevels){

            logger.setLoggingLevels();
            logger.setDebugLevel(Preferences::debugLevel);
            logger.setTraceLevel(Preferences::traceLevel);
            logger.setNoticeLevel(Preferences::noticeLevel);
            logger.setInfoLevel(Preferences::infoLevel);
            logger.setStatusLevel(Preferences::statusLevel);
            logger.setErrorLevel(Preferences::errorLevel);
            logger.setFatalLevel(Preferences::fatalLevel);
        }

        logger.setIncludeLineNumber(Preferences::includeLineNumber);
        logger.setIncludeFileName(Preferences::includeFileName);
        logger.setIncludeFunctionInfo(Preferences::includeFunction);
        logger.setColorizeFunctionInfo(true);

    } else {
        logger.setLoggingLevel(OffLevel);
    }
}

SnapGridCommands sgCommands[NUM_GRID_SIZES] =
{
    // SCENE_GRID_SIZE_S1
    {
        QT_TRANSLATE_NOOP("Action", "Edit.GridSize.S1"),
        QT_TRANSLATE_NOOP("Menu", "10 pixels"),
        QT_TRANSLATE_NOOP("Status", "Set grid step to 10 pixels"),
    },
    // SCENE_GRID_SIZE_S2
    {
        QT_TRANSLATE_NOOP("Action", "Edit.GridSize.S2"),
        QT_TRANSLATE_NOOP("Menu", "20 Pixels"),
        QT_TRANSLATE_NOOP("Status", "Set grid step to 20 pixels"),
    },
    // SCENE_GRID_SIZE_S3
    {
        QT_TRANSLATE_NOOP("Action", "Edit.GridSize.S3"),
        QT_TRANSLATE_NOOP("Menu", "30 Pixels"),
        QT_TRANSLATE_NOOP("Status", "Set grid step to 30 pixels"),
    },
    // SCENE_GRID_SIZE_S4
    {
        QT_TRANSLATE_NOOP("Action", "Edit.GridSize.S4"),
        QT_TRANSLATE_NOOP("Menu", "40 Pixels"),
        QT_TRANSLATE_NOOP("Status", "Set grid step to 40 pixels"),
    },
    // SCENE_GRID_SIZE_S5
    {
        QT_TRANSLATE_NOOP("Action", "Edit.GridSize.S5"),
        QT_TRANSLATE_NOOP("Menu", "50 Pixels"),
        QT_TRANSLATE_NOOP("Status", "Set grid step to 50 pixels"),
    },
    // SCENE_GRID_SIZE_S6
    {
        QT_TRANSLATE_NOOP("Action", "Edit.GridSize.S6"),
        QT_TRANSLATE_NOOP("Menu", "60 Pixels"),
        QT_TRANSLATE_NOOP("Status", "Set grid step to 60 pixels"),
    },
    // SCENE_GRID_SIZE_S7
    {
        QT_TRANSLATE_NOOP("Action", "Edit.GridSize.S7"),
        QT_TRANSLATE_NOOP("Menu", "70 pixels"),
        QT_TRANSLATE_NOOP("Status", "Set grid step to 70 pixels"),
    },
    // SCENE_GRID_SIZE_S8
    {
        QT_TRANSLATE_NOOP("Action", "Edit.GridSize.S8"),
        QT_TRANSLATE_NOOP("Menu", "80 pixels"),
        QT_TRANSLATE_NOOP("Status", "Set grid step to 80 pixels"),
    },
    // SCENE_GRID_SIZE_S9
    {
        QT_TRANSLATE_NOOP("Action", "Edit.GridSize.S9"),
        QT_TRANSLATE_NOOP("Menu", "90 pixels"),
        QT_TRANSLATE_NOOP("Status", "Set grid step to 90 pixels"),
    }
};

static_assert(sizeof(sgCommands)/sizeof(sgCommands[0]) == NUM_GRID_SIZES, "Array size mismatch.");
