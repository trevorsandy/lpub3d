
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
#include "lpub.h"
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

#include "QPushButton"
#include "QHBoxLayout"
#include "QVBoxLayout"

#include "name.h"
#include "editwindow.h"
#include "parmswindow.h"
#include "paths.h"
#include "globals.h"
#include "resolution.h"
#include "lpub_preferences.h"
#include "lpubalert.h"
#include "preferencesdialog.h"
#include "render.h"
#include "metaitem.h"
#include "ranges_element.h"
#include "updatecheck.h"
#include "step.h"

//** 3D
#include "camera.h"
#include "project.h"
#include "view.h"
#include "piece.h"
#include "lc_profile.h"
#include "application.h"

#include <ui_progress_dialog.h>

#include <TCFoundation/TCUserDefaults.h>
#include <LDLib/LDUserDefaultsKeys.h>
#include <LDVQt/LDVPovUserDefaultsKeys.h>
//**

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

void clearTempCache()
{
  gui->clearTempCache();
}

/****************************************************************************
 * 
 * The Gui constructor and destructor are at the bottom of the file with
 * the code that creates the basic GUI framekwork 
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
  // get line to insert final model
  int modelStatus = mi.okToInsertFinalModel();

  if ((Preferences::enableFadeSteps || Preferences::enableHighlightStep) && modelStatus != modelExist){
      mi.insertFinalModel(modelStatus);
    } else if ((! Preferences::enableFadeSteps && ! Preferences::enableHighlightStep) && modelStatus == modelExist){
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
    // User wants to continue running Next page process so stop Pevious
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
  QElapsedTimer continuousTimer;
  QString direction = d == PAGE_NEXT ? "Next" : "Previous";

  exportType = PAGE_PROCESS;

  if (Preferences::modeGUI) {
      continuousTimer.start();
      setContinuousPageSig(true);
      if (Preferences::doNotShowPageProcessDlg) {
          if (!processPageRange(setPageLineEdit->displayText())) {
              setPageContinuousIsRunning(false);
              emit messageSig(LOG_STATUS,QString("%1 page processing terminated.").arg(direction));
              emit setContinuousPageSig(false);
              return false;
            }
        }
      else
      {
          DialogExportPages *dialog = new DialogExportPages();

          if (dialog->exec() == QDialog::Accepted) {

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

          } else {
              setPageContinuousIsRunning(false);
              emit messageSig(LOG_STATUS,QString("%1 page processing terminated.").arg(direction));
              emit setContinuousPageSig(false);
              return false;
          }
        }
    }
  else { // command line mode
      // This effectively acts like a toggle. If true, it sets false and vice versa.
      nextPageContinuousIsRunning = !nextPageContinuousIsRunning;
    }

  if (processOption == EXPORT_PAGE_RANGE){
      if (! validatePageRange()){
          setPageContinuousIsRunning(false);
          emit messageSig(LOG_STATUS,QString("%1 page processing terminated.").arg(direction));
          emit setContinuousPageSig(false);
          return false;
        }
    }

  // Process is running - configure to stop
  setContinuousPageAct(SET_STOP_ACTION);

  if(resetCache){
      clearCustomPartCache(true);
      clearAndRedrawPage();
    }

  // store current display page number
  logStatus() << QString("Continuous %1 page processing start...").arg(direction);

  if (processOption == EXPORT_ALL_PAGES){

      _maxPages = maxPages;

      QApplication::setOverrideCursor(Qt::ArrowCursor);

      for (d == PAGE_NEXT ? displayPageNum = 1 : displayPageNum = maxPages ; d == PAGE_NEXT ? displayPageNum <= maxPages : displayPageNum >= 1 ; d == PAGE_NEXT ? displayPageNum++ : displayPageNum--) {

          if (d == PAGE_NEXT) {
              terminateProcess =  !nextPageContinuousIsRunning;
            } else {
              terminateProcess =  !previousPageContinuousIsRunning;
            }

          if (terminateProcess) {
              emit messageSig(LOG_STATUS,QString("%1 page processing terminated before completion. %2 pages of %3 processed%4.")
                              .arg(direction)
                              .arg(d == PAGE_NEXT ? (displayPageNum - 1) : (maxPages - (displayPageNum - 1)))
                              .arg(maxPages)
                              .arg(QString(". %1").arg(gui->elapsedTime(continuousTimer.elapsed()))));
              QApplication::restoreOverrideCursor();
              setContinuousPageAct(SET_DEFAULT_ACTION);
              emit setContinuousPageSig(false);
              return false;
            }

          pageCount = displayPageNum;

          displayPage();

          emit messageSig(LOG_STATUS,QString("Processed page %1 of %2").arg(displayPageNum).arg(maxPages));

          qApp->processEvents();

          secSleeper::secSleep(Preferences::pageDisplayPause);
        }

      QApplication::restoreOverrideCursor();

    } else if (processOption == EXPORT_PAGE_RANGE) {

      QStringList pageRanges = pageRangeText.split(",");
      QList<int> printPages;
      foreach(QString ranges,pageRanges){
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
            } else {
              printPages.append(ranges.toInt());
            }
        }

      if (d == PAGE_NEXT)
        std::sort(printPages.begin(),printPages.end(),lt);    // Next - small to large
      else
        std::sort(printPages.begin(),printPages.end(),gt);    // Previous - large to small

      displayPageNum = printPages.first();

      _maxPages = printPages.count();

      QApplication::setOverrideCursor(Qt::BusyCursor);

      foreach(int printPage,printPages){

          displayPageNum = printPage;

          if (d == PAGE_NEXT) {
              terminateProcess = !nextPageContinuousIsRunning;
            } else {
              terminateProcess = !previousPageContinuousIsRunning;
            }

          if (terminateProcess) {
              emit messageSig(LOG_STATUS,QString("%1 page processing terminated before completion. %2 pages of %3 processed%4.")
                              .arg(direction)
                              .arg(d == PAGE_NEXT ? pageCount : (_maxPages - pageCount))
                              .arg(_maxPages)
                              .arg(QString(". %1").arg(gui->elapsedTime(continuousTimer.elapsed()))));
              QApplication::restoreOverrideCursor();
              setContinuousPageAct(SET_DEFAULT_ACTION);
              emit setContinuousPageSig(false);
              return false;
            }

          pageCount++;

          displayPage();

          emit messageSig(LOG_STATUS,QString("Processed page %1, %2 of %3 for page range %4")
                          .arg(displayPageNum)
                          .arg(pageCount)
                          .arg(_maxPages)
                          .arg(pageRanges.join(" ")));

          qApp->processEvents();

          secSleeper::secSleep(Preferences::pageDisplayPause);
        }

      QApplication::restoreOverrideCursor();
    }

    setContinuousPageAct(SET_DEFAULT_ACTION);

  emit messageSig(LOG_INFO_STATUS,
                  QString("%1 page processing completed. %2 of %3 %4 processed%5.")
                          .arg(direction)
                          .arg(d == PAGE_NEXT ? pageCount : _maxPages)
                          .arg(_maxPages)
                          .arg(_maxPages > 1 ? "pages" : "page")
                          .arg(QString(". %1").arg(gui->elapsedTime(continuousTimer.elapsed()))));
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
  displayPageNum = 1;
  displayPage();
}

void Gui::lastPage()
{
  countPages();
  displayPageNum = maxPages;
  displayPage();
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
}

void Gui::pageGuides()
{
  Preferences::setPageGuidesPreference(pageGuidesAct->isChecked());
  KpageView->setPageGuides(getTheme());
}

void Gui::pageRuler()
{
  Preferences::setPageRulerPreference(pageRulerAct->isChecked());
  KpageView->setPageRuler(getTheme());
}

void Gui::actualSize()
{
  KpageView->actualSize();
}

void Gui::zoomIn()
{
  KpageView->zoomIn();
}

void Gui::zoomOut()
{
  KpageView->zoomOut();
}

void Gui::SetRotStepMeta()
{
    mStepRotation[0] = mRotStepAngleX;
    mStepRotation[1] = mRotStepAngleY;
    mStepRotation[2] = mRotStepAngleZ;

    if (getCurFile() != "") {
        ShowStepRotationStatus();
        QString rotationValue = QString("%1 %2 %3 %4 %5")
                                        .arg(getViewerCsiName())
                                        .arg(QString::number(mStepRotation[0], 'f', 2))
                                        .arg(QString::number(mStepRotation[1], 'f', 2))
                                        .arg(QString::number(mStepRotation[2], 'f', 2))
                                        .arg(mRotStepTransform);
        MetaItem mi;
        mi.writeRotateStep(rotationValue);
    }
}

void Gui::ShowStepRotationStatus()
{
    QString rotLabel = QString("Adjusted ROTSTEP Rotation %1 %2 %3 %4")
                               .arg(QString::number(mRotStepAngleX, 'f', 2))
                               .arg(QString::number(mRotStepAngleY, 'f', 2))
                               .arg(QString::number(mRotStepAngleZ, 'f', 2))
                               .arg(mRotStepTransform);
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
    const QString &modelName)
{
  if (! exporting()) {
      displayFileSig(ldrawFile, modelName);
      curSubFile = modelName;
      int currentIndex = 0;
      for (int i = 0; i < mpdCombo->count(); i++) {
          if (mpdCombo->itemText(i) == modelName) {
              currentIndex = i;
              break;
            }
        }
      mpdCombo->setCurrentIndex(currentIndex);
    }
}

void Gui::displayParmsFile(
  const QString &fileName)
{
    displayParmsFileSig(fileName);
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

      installExportBanner(exportType, exportBanner,imageFile);
    }
}

/*-----------------------------------------------------------------------------*/

bool Gui::installExportBanner(const int &type, const QString &printFile, const QString &imageFile){

    QList<QString> bannerData;
    bannerData << "0 FILE printbanner.ldr";
    bannerData << "0 Name: printbanner.ldr";
    bannerData << "0 Author: Trevor SANDY";
    bannerData << "0 Unofficial Model";
    bannerData << "0 !LEOCAD MODEL NAME Printbanner";
    bannerData << "0 !LEOCAD MODEL AUTHOR Trevor SANDY";
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

void  Gui::restartApplication(){
  QStringList args = QApplication::arguments();
  if (! getCurFile().isEmpty()){
      args << QString("%1").arg(getCurFile());
      QSettings Settings;
      Settings.setValue(QString("%1/%2").arg(DEFAULTS,SAVE_DISPLAY_PAGE_NUM),displayPageNum);
    } else {
      args << QString();
    }
  args.removeFirst();
  QProcess::startDetached(QApplication::applicationFilePath(), args);
  messageSig(LOG_INFO, QString("Restarted LPub3D: %1, args: %2").arg(QApplication::applicationFilePath()).arg(args.join(" ")));
  QCoreApplication::quit();
}

void Gui::reloadCurrentPage(){
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,"No model file page to redisplay.");
        return;
    }
    if (maybeSave()) {
        timer.start();

        int savePage = displayPageNum;
        displayPageNum = savePage;
        displayPage();
        emit messageSig(LOG_STATUS, QString("Page %1 reloaded. %2")
                        .arg(displayPageNum)
                        .arg(elapsedTime(timer.elapsed())));
    }
}

void Gui::reloadCurrentModelFile(){
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,"No model file to reopen.");
        return;
    }
    if (maybeSave()) {
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
}

void Gui::clearAllCaches()
{
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,"A model must be open to reset its caches - no action taken.");
        return;
    }

    if (maybeSave(false)) {  // No prompt
        timer.start();

        if (Preferences::enableFadeSteps || Preferences::enableHighlightStep) {
            ldrawFile.clearPrevStepPositions();
        }

        clearPLICache();
        clearCSICache();
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
}

void Gui::clearCustomPartCache(bool silent)
{
  QMessageBox::StandardButton ret = QMessageBox::Ok;
  QString message = QString("All existing custom part files will be deleted and regenerated.\n"
                            "Warning: Only custom part files for the currently loaded model file will be updated in %1.")
                            .arg(FILE_LPUB3D_UNOFFICIAL_ARCHIVE);
  if (silent || !Preferences::modeGUI) {
      emit messageSig(LOG_INFO,message);
  } else {
      ret = QMessageBox::warning(this, tr(VER_PRODUCTNAME_STR),
                                 tr("%1\nDo you want to delete the custom file cache?").arg(message),
                                 QMessageBox::Yes | QMessageBox::Discard | QMessageBox::Cancel);
  }

  if (ret == QMessageBox::Cancel)
      return;

  QString dirName = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customDir));

  int count = 0;
  if (removeDir(count, dirName)){
      emit messageSig(LOG_STATUS,QMessageBox::tr("Custom parts cache cleaned.  %1 %2 removed.")
                           .arg(count)
                           .arg(count == 1 ? "item": "items"));
  } else {
      emit messageSig(LOG_ERROR,QMessageBox::tr("Unable to remeove custom parts cache directory: %1").arg(dirName));
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

    QString dirName = QDir::currentPath() + "/" + Paths::partsDir;
    QDir dir(dirName);

    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();
    int count = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(dirName + "/" + fileInfo.fileName());
        if (!file.remove()) {
            QMessageBox::critical(nullptr,
                                  tr("LPub3D"),
                                  tr("Unable to remeove %1")
                                  .arg(dirName + "/" + fileInfo.fileName()));
            count--;
          } else
          count++;
      }

    emit messageSig(LOG_STATUS,QString("Parts content cache cleaned. %1 items removed.").arg(count));
}

void Gui::clearCSICache()
{
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,"A model must be open to clean its assembly cache - no action taken.");
        return;
    }

    QString dirName = QDir::currentPath() + "/" + Paths::assemDir;
    QDir dir(dirName);

    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();
    int count = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(dirName + "/" + fileInfo.fileName());
        if (!file.remove()) {
            QMessageBox::critical(nullptr,
                                  tr("LPub3D"),
                                  tr("Unable to remeove %1")
                                  .arg(dirName + "/" + fileInfo.fileName()));
            count--;
          } else
          count++;
      }

    emit messageSig(LOG_STATUS,QString("Assembly content cache cleaned. %1 items removed.").arg(count));
}

void Gui::clearTempCache()
{
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,"A model must be open to clean its 3D cache - no action taken.");
        return;
    }

    QString tmpDirName = QDir::currentPath() + "/" + Paths::tmpDir;
    QDir tmpDir(tmpDirName);

    tmpDir.setFilter(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = tmpDir.entryInfoList();
    int count1 = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(tmpDirName + "/" + fileInfo.fileName());
        if (!file.remove()) {
            QMessageBox::critical(nullptr,
                                  tr("LPub3D"),
                                  tr("Unable to remeove %1")
                                  .arg(tmpDirName + "/" + fileInfo.fileName()));
            count1--;
          } else
          count1++;
      }

    ldrawFile.tempCacheCleared();

    emit messageSig(LOG_STATUS,QString("Temporary model file cache cleaned. %1 items removed.").arg(count1));
}

bool Gui::removeDir(int &count, const QString & dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(count,info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
                count++;
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
    return result;
}

void Gui::clearStepCSICache(QString &pngName) {
  QString tmpDirName   = QDir::currentPath() + "/" + Paths::tmpDir;
  QString assemDirName = QDir::currentPath() + "/" + Paths::assemDir;
  QFileInfo fileInfo(pngName);
  QFile file(assemDirName + "/" + fileInfo.fileName());
  if (!file.remove()) {
      emit messageSig(LOG_ERROR, QMessageBox::tr("Unable to remove %1")
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
      emit messageSig(LOG_ERROR,QMessageBox::tr("Unable to remeove %1")
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
 * Call only if using LDView Single Call (useLDViewsCall=true)
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
      emit messageSig(LOG_ERROR,QMessageBox::tr("Unable to remove %1")
                            .arg(assemDirName + "/" + fileInfo.fileName()));
  }
  if (renderer->useLDViewSCall()){
      ldrName = tmpDirName + "/" + fileInfo.completeBaseName() + ".ldr";
      file.setFileName(ldrName);
      if (!file.remove()) {
         emit messageSig(LOG_ERROR,QMessageBox::tr("Unable to remeove %1")
                                .arg(ldrName));
      }
  } else if (! itemsCleared){
      ldrName = tmpDirName + "/csi.ldr";
      file.setFileName(ldrName);
      if (!file.remove()) {
          emit messageSig(LOG_ERROR,QMessageBox::tr("Unable to remeove %1")
                                .arg(ldrName));
      }
      itemsCleared = true;
  }
  // process callout's step(s) image(s)
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
    displayParmsFile(Preferences::titleAnnotationsFile);
    parmsWindow->setWindowTitle(tr("Part Title Annotation","Edit/add part title part annotations"));
    parmsWindow->show();
}

void Gui::editFreeFormAnnitations()
{
    displayParmsFile(Preferences::freeformAnnotationsFile);
    parmsWindow->setWindowTitle(tr("Freeform Annotation","Edit/add freeform part annotations"));
    parmsWindow->show();
}

void Gui::editLDrawColourParts()
{
    displayParmsFile(Preferences::ldrawColourPartsFile);
    parmsWindow->setWindowTitle(tr("LDraw Colour Parts","Edit/add LDraw static coulour parts"));
    parmsWindow->show();
}

void Gui::editPliBomSubstituteParts()
{
    displayParmsFile(Preferences::pliSubstitutePartsFile);
    parmsWindow->setWindowTitle(tr("PLI/BOM Substitute Parts","Edit/add PLI/BOM substitute parts"));
    parmsWindow->show();
}

void Gui::editExcludedParts()
{
    displayParmsFile(Preferences::excludedPartsFile);
    parmsWindow->setWindowTitle(tr("Part Count Excluded Parts","Edit/add excluded parts"));
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
    parmsWindow->setWindowTitle(tr("Edit LDGLite ini","Edit LDGLite ini "));
    parmsWindow->show();
}

void Gui::editNativePovIni()
{
    displayParmsFile(Preferences::nativePOVIni);
    parmsWindow->setWindowTitle(tr("Edit Native POV file generation ini","Edit Native POV file generation ini "));
    parmsWindow->show();
}

void Gui::editLdviewIni()
{
    displayParmsFile(Preferences::ldviewIni);
    parmsWindow->setWindowTitle(tr("Edit LDView ini","Edit LDView ini "));
    parmsWindow->show();
}

void Gui::editLdviewPovIni()
{
    displayParmsFile(Preferences::ldviewPOVIni);
    parmsWindow->setWindowTitle(tr("Edit LDView POV file generation ini","Edit LDView POV file generation ini"));
    parmsWindow->show();
}

void Gui::editPovrayIni()
{
    displayParmsFile(Preferences::povrayIni);
    parmsWindow->setWindowTitle(tr("Edit Raytracer ini","Edit Raytracer ini"));
    parmsWindow->show();
}

void Gui::editPovrayConf()
{
    displayParmsFile(Preferences::povrayConf);
    parmsWindow->setWindowTitle(tr("Edit Raytracer file access conf","Edit Raytracer file access conf"));
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
    bool altLDConfigPathRestart         = false;
    bool enableLDViewSCallCompare       = Preferences::enableLDViewSingleCall;
    bool enableLDViewSListCompare       = Preferences::enableLDViewSnaphsotList;
    bool displayAllAttributesCompare    = Preferences::displayAllAttributes;
    bool generateCoverPagesCompare      = Preferences::generateCoverPages;
    bool enableFadeStepsCompare         = Preferences::enableFadeSteps;
    bool fadeStepsUseColourCompare      = Preferences::fadeStepsUseColour;
    int fadeStepsOpacityCompare         = Preferences::fadeStepsOpacity;
    bool enableHighlightStepCompare     = Preferences::enableHighlightStep;
    bool enableImageMattingCompare      = Preferences::enableImageMatting;
    int  highlightStepLineWidthCompare  = Preferences::highlightStepLineWidth;
    bool doNotShowPageProcessDlgCompare = Preferences::doNotShowPageProcessDlg;
    int  pageDisplayPauseCompare        = Preferences::pageDisplayPause;
    QString altLDConfigPathCompare      = Preferences::altLDConfigPath;
    QString povFileGeneratorCompare     = Preferences::povFileGenerator;
    QString fadeStepsColourCompare      = Preferences::fadeStepsColour;
    QString highlightStepColourCompare  = Preferences::highlightStepColour;
    QString ldrawPathCompare            = Preferences::ldrawPath;
    QString lgeoPathCompare             = Preferences::lgeoPath;
    QString preferredRendererCompare    = Preferences::preferredRenderer;
    QString displayThemeCompare         = Preferences::displayTheme;

    // Native Pov file generation settings
    QString selectedAspectRatioCompare;
    switch (int(TCUserDefaults::longForKey(SELECTED_ASPECT_RATIO_KEY, SELECTED_ASPECT_RATIO_DEFAULT)))
    {
    case -1:
        selectedAspectRatioCompare = ASPECT_RATIO_0;
        break;
    case 0:
        selectedAspectRatioCompare = ASPECT_RATIO_1;
        break;
    case 2:
        selectedAspectRatioCompare = ASPECT_RATIO_2;
        break;
    case 3:
        selectedAspectRatioCompare = ASPECT_RATIO_3;
        break;
    case 4:
        selectedAspectRatioCompare = ASPECT_RATIO_4;
        break;
    case 5:
        selectedAspectRatioCompare = ASPECT_RATIO_5;
        break;
    case 6:
        selectedAspectRatioCompare = ASPECT_RATIO_6;
        break;
    case 7:
        selectedAspectRatioCompare = ASPECT_RATIO_7;
        break;
    default:
        selectedAspectRatioCompare = ASPECT_RATIO_8;
    }

    long qualityCompare               = TCUserDefaults::longForKey(QUALITY_EXPORT_KEY, QUALITY_EXPORT_DEFAULT);
    float customAspectRatioCompare    = TCUserDefaults::floatForKey(CUSTOM_ASPECT_RATIO_KEY, CUSTOM_ASPECT_RATIO_DEFAULT);
    float edgeRadiusCompare           = TCUserDefaults::floatForKey(EDGE_RADIUS_KEY, EDGE_RADIUS_DEFAULT);
    float seamWidthCompare            = TCUserDefaults::floatForKey(SEAM_WIDTH_KEY, EDGE_RADIUS_DEFAULT);
    float ambientCompare              = TCUserDefaults::floatForKey(AMBIENT_KEY, AMBIENT_DEFAULT);
    float diffuseCompare              = TCUserDefaults::floatForKey(DIFFUSE_KEY, DIFFUSE_DEFAULT);
    float reflCompare                 = TCUserDefaults::floatForKey(REFLECTION_KEY, REFLECTION_DEFAULT);
    float phongCompare                = TCUserDefaults::floatForKey(PHONG_KEY, PHONG_DEFAULT);
    float phongSizeCompare            = TCUserDefaults::floatForKey(PHONG_SIZE_KEY, PHONG_SIZE_DEFAULT);
    float transReflCompare            = TCUserDefaults::floatForKey(TRANS_REFLECTION_KEY, TRANS_REFLECTION_DEFAULT);
    float transFilterCompare          = TCUserDefaults::floatForKey(TRANS_FILTER_KEY, TRANS_FILTER_DEFAULT);
    float transIoRCompare             = TCUserDefaults::floatForKey(TRANS_IOR_KEY, TRANS_IOR_DEFAULT);
    float rubberReflCompare           = TCUserDefaults::floatForKey(RUBBER_REFLECTION_KEY, RUBBER_REFLECTION_DEFAULT);
    float rubberPhongCompare          = TCUserDefaults::floatForKey(RUBBER_PHONG_KEY, RUBBER_PHONG_DEFAULT);
    float rubberPhongSizeCompare      = TCUserDefaults::floatForKey(RUBBER_PHONG_SIZE_KEY, RUBBER_PHONG_SIZE_DEFAULT);
    float chromeReflCompare           = TCUserDefaults::floatForKey(CHROME_REFLECTION_KEY, CHROME_REFLECTION_DEFAULT);
    float chromeBrilCompare           = TCUserDefaults::floatForKey(CHROME_BRILLIANCE_KEY, CHROME_BRILLIANCE_DEFAULT);
    float chromeSpecularCompare       = TCUserDefaults::floatForKey(CHROME_SPECULAR_KEY, CHROME_SPECULAR_DEFAULT);
    float chromeRoughnessCompare      = TCUserDefaults::floatForKey(CHROME_ROUGHNESS_KEY, CHROME_ROUGHNESS_DEFAULT);
    float fileVersionCompare          = TCUserDefaults::floatForKey(FILE_VERSION_KEY, FILE_VERSION_DEFAULT);

    bool seamsCompare                 = TCUserDefaults::boolForKey(SEAMS_KEY, true);
    bool reflectionsCompare           = TCUserDefaults::boolForKey(REFLECTIONS_KEY, true);
    bool shadowsCompare               = TCUserDefaults::boolForKey(SHADOWS_KEY, true);
    bool xmlMapCompare                = TCUserDefaults::boolForKey(XML_MAP_KEY, true);
    bool inlinePovCompare             = TCUserDefaults::boolForKey(INLINE_POV_KEY, true);
    bool smoothCurvesCompare          = TCUserDefaults::boolForKey(SMOOTH_CURVES_KEY, true);
    bool hideStudsCompare             = TCUserDefaults::boolForKey(HIDE_STUDS_KEY, false);
    bool unmirrorStudsCompare         = TCUserDefaults::boolForKey(UNMIRROR_STUDS_KEY, true);
    bool findReplacementsCompare      = TCUserDefaults::boolForKey(FIND_REPLACEMENTS_KEY, false);
    bool conditionalEdgeLinesCompare  = TCUserDefaults::boolForKey(CONDITIONAL_EDGE_LINES_KEY, false);
    bool primitiveSubstitutionCompare = TCUserDefaults::boolForKey(PRIMITIVE_SUBSTITUTION_KEY, true);
    //                                  TCUserDefaults::boolForKey(DRAW_EDGES_KEY, false);

    QString xmlMapPathCompare         = QString(TCUserDefaults::pathForKey(XML_MAP_PATH_KEY));
    QString topIncludeCompare         = QString(TCUserDefaults::stringForKey(TOP_INCLUDE_KEY));
    QString bottomIncludeCompare      = QString(TCUserDefaults::stringForKey(BOTTOM_INCLUDE_KEY));

    QString lightsCompare             = Preferences::ldvLights;

    if (Preferences::getPreferences()) {

        Meta meta;
        page.meta = meta;

        QMessageBox box;
        box.setMinimumSize(40,20);
        box.setIcon (QMessageBox::Question);
        box.setDefaultButton   (QMessageBox::Ok);
        box.setStandardButtons (QMessageBox::Ok | QMessageBox::Close | QMessageBox::Cancel);

        bool rendererChanged               = QString(Preferences::preferredRenderer).toLower()   != preferredRendererCompare.toLower();
        bool enableFadeStepsChanged        = Preferences::enableFadeSteps                        != enableFadeStepsCompare;
        bool fadeStepsUseColourChanged     = Preferences::fadeStepsUseColour                     != fadeStepsUseColourCompare;
        bool fadeStepsColourChanged        = QString(Preferences::fadeStepsColour).toLower()     != fadeStepsColourCompare.toLower();
        bool fadeStepsOpacityChanged       = Preferences::fadeStepsOpacity                       != fadeStepsOpacityCompare;
        bool enableHighlightStepChanged    = Preferences::enableHighlightStep                    != enableHighlightStepCompare;
        bool highlightStepColorChanged     = QString(Preferences::highlightStepColour).toLower() != highlightStepColourCompare.toLower();
        bool highlightStepLineWidthChanged = Preferences::highlightStepLineWidth                 != highlightStepLineWidthCompare;
        bool enableImageMattingChanged     = Preferences::enableImageMatting                     != enableImageMattingCompare;
        bool enableLDViewSCallChanged      = Preferences::enableLDViewSingleCall                 != enableLDViewSCallCompare;
        bool enableLDViewSListChanged      = Preferences::enableLDViewSnaphsotList               != enableLDViewSListCompare;
        bool displayAttributesChanged      = Preferences::displayAllAttributes                   != displayAllAttributesCompare;
        bool generateCoverPagesChanged     = Preferences::generateCoverPages                     != generateCoverPagesCompare;
        bool pageDisplayPauseChanged       = Preferences::pageDisplayPause                       != pageDisplayPauseCompare;
        bool doNotShowPageProcessDlgChanged= Preferences::doNotShowPageProcessDlg                != doNotShowPageProcessDlgCompare;
        bool povFileGeneratorChanged       = Preferences::povFileGenerator                       != povFileGeneratorCompare;
        bool altLDConfigPathChanged        = Preferences::altLDConfigPath                        != altLDConfigPathCompare;

        bool ldrawPathChanged              = QString(Preferences::ldrawPath).toLower()           != ldrawPathCompare.toLower();
        bool lgeoPathChanged               = QString(Preferences::lgeoPath).toLower()            != lgeoPathCompare.toLower();
        bool displayThemeChanged           = Preferences::displayTheme.toLower()                 != displayThemeCompare.toLower();

        // Native Pov file generation settings
        QString selectedAspectRatio;
        switch (int(TCUserDefaults::longForKey(SELECTED_ASPECT_RATIO_KEY, SELECTED_ASPECT_RATIO_DEFAULT)))
        {
        case -1:
            selectedAspectRatio = ASPECT_RATIO_0;
            break;
        case 0:
            selectedAspectRatio = ASPECT_RATIO_1;
            break;
        case 2:
            selectedAspectRatio = ASPECT_RATIO_2;
            break;
        case 3:
            selectedAspectRatio = ASPECT_RATIO_3;
            break;
        case 4:
            selectedAspectRatio = ASPECT_RATIO_4;
            break;
        case 5:
            selectedAspectRatio = ASPECT_RATIO_5;
            break;
        case 6:
            selectedAspectRatio = ASPECT_RATIO_6;
            break;
        case 7:
            selectedAspectRatio = ASPECT_RATIO_7;
            break;
        default:
            selectedAspectRatio = ASPECT_RATIO_8;
        }
        bool qualityChanged               = TCUserDefaults::longForKey(QUALITY_EXPORT_KEY, QUALITY_EXPORT_DEFAULT)            != qualityCompare;
        bool selectedAspectRatioChanged   = selectedAspectRatio                                                               != selectedAspectRatioCompare;
        bool customAspectRatioChanged     = TCUserDefaults::floatForKey(CUSTOM_ASPECT_RATIO_KEY, CUSTOM_ASPECT_RATIO_DEFAULT) != customAspectRatioCompare;
        bool edgeRadiusChanged            = TCUserDefaults::floatForKey(EDGE_RADIUS_KEY, EDGE_RADIUS_DEFAULT)                 != edgeRadiusCompare;
        bool seamWidthChanged             = TCUserDefaults::floatForKey(SEAM_WIDTH_KEY, EDGE_RADIUS_DEFAULT)                  != seamWidthCompare;
        bool ambientChanged               = TCUserDefaults::floatForKey(AMBIENT_KEY, AMBIENT_DEFAULT)                         != ambientCompare;
        bool diffuseChanged               = TCUserDefaults::floatForKey(DIFFUSE_KEY, DIFFUSE_DEFAULT)                         != diffuseCompare;
        bool reflChanged                  = TCUserDefaults::floatForKey(REFLECTION_KEY, REFLECTION_DEFAULT)                   != reflCompare;
        bool phongChanged                 = TCUserDefaults::floatForKey(PHONG_KEY, PHONG_DEFAULT)                             != phongCompare;
        bool phongSizeChanged             = TCUserDefaults::floatForKey(PHONG_SIZE_KEY, PHONG_SIZE_DEFAULT)                   != phongSizeCompare;
        bool transReflChanged             = TCUserDefaults::floatForKey(TRANS_REFLECTION_KEY, TRANS_REFLECTION_DEFAULT)       != transReflCompare;
        bool transFilterChanged           = TCUserDefaults::floatForKey(TRANS_FILTER_KEY, TRANS_FILTER_DEFAULT)               != transFilterCompare;
        bool transIoRChanged              = TCUserDefaults::floatForKey(TRANS_IOR_KEY, TRANS_IOR_DEFAULT)                     != transIoRCompare;
        bool rubberReflChanged            = TCUserDefaults::floatForKey(RUBBER_REFLECTION_KEY, RUBBER_REFLECTION_DEFAULT)     != rubberReflCompare;
        bool rubberPhongChanged           = TCUserDefaults::floatForKey(RUBBER_PHONG_KEY, RUBBER_PHONG_DEFAULT)               != rubberPhongCompare;
        bool rubberPhongSizeChanged       = TCUserDefaults::floatForKey(RUBBER_PHONG_SIZE_KEY, RUBBER_PHONG_SIZE_DEFAULT)     != rubberPhongSizeCompare;
        bool chromeReflChanged            = TCUserDefaults::floatForKey(CHROME_REFLECTION_KEY, CHROME_REFLECTION_DEFAULT)     != chromeReflCompare;
        bool chromeBrilChanged            = TCUserDefaults::floatForKey(CHROME_BRILLIANCE_KEY, CHROME_BRILLIANCE_DEFAULT)     != chromeBrilCompare;
        bool chromeSpecularChanged        = TCUserDefaults::floatForKey(CHROME_SPECULAR_KEY, CHROME_SPECULAR_DEFAULT)         != chromeSpecularCompare;
        bool chromeRoughnessChanged       = TCUserDefaults::floatForKey(CHROME_ROUGHNESS_KEY, CHROME_ROUGHNESS_DEFAULT)       != chromeRoughnessCompare;
        bool fileVersionChanged           = TCUserDefaults::floatForKey(FILE_VERSION_KEY, FILE_VERSION_DEFAULT)               != fileVersionCompare;

        bool seamsChanged                 = TCUserDefaults::boolForKey(SEAMS_KEY, true)                                       != seamsCompare;
        bool reflectionsChanged           = TCUserDefaults::boolForKey(REFLECTIONS_KEY, true)                                 != reflectionsCompare;
        bool shadowsChanged               = TCUserDefaults::boolForKey(SHADOWS_KEY, true)                                     != shadowsCompare;
        bool xmlMapChanged                = TCUserDefaults::boolForKey(XML_MAP_KEY, true)                                     != xmlMapCompare;
        bool inlinePovChanged             = TCUserDefaults::boolForKey(INLINE_POV_KEY, true)                                  != inlinePovCompare;
        bool smoothCurvesChanged          = TCUserDefaults::boolForKey(SMOOTH_CURVES_KEY, true)                               != smoothCurvesCompare;
        bool hideStudsChanged             = TCUserDefaults::boolForKey(HIDE_STUDS_KEY, false)                                 != hideStudsCompare;
        bool unmirrorStudsChanged         = TCUserDefaults::boolForKey(UNMIRROR_STUDS_KEY, true)                              != unmirrorStudsCompare;
        bool findReplacementsChanged      = TCUserDefaults::boolForKey(FIND_REPLACEMENTS_KEY, false)                          != findReplacementsCompare;
        bool conditionalEdgeLinesChanged  = TCUserDefaults::boolForKey(CONDITIONAL_EDGE_LINES_KEY, false)                     != conditionalEdgeLinesCompare;
        bool primitiveSubstitutionChanged = TCUserDefaults::boolForKey(PRIMITIVE_SUBSTITUTION_KEY, true)                      != primitiveSubstitutionCompare;
        //                                  TCUserDefaults::boolForKey(DRAW_EDGES_KEY, false)

        bool xmlMapPathChanged            = QString(TCUserDefaults::pathForKey(XML_MAP_PATH_KEY))                             != xmlMapPathCompare;
        bool topIncludeChanged            = QString(TCUserDefaults::stringForKey(TOP_INCLUDE_KEY))                            != topIncludeCompare;
        bool bottomIncludeChanged         = QString(TCUserDefaults::stringForKey(BOTTOM_INCLUDE_KEY))                         != bottomIncludeCompare;

        bool lightsChanged                = QString(Preferences::ldvLights).toLower()                                               != lightsCompare.toLower();

        bool nativePovRendererConfig      = Preferences::preferredRenderer == RENDERER_POVRAY && Preferences::povFileGenerator == RENDERER_NATIVE;

        if (enableFadeStepsChanged)
            emit messageSig(LOG_INFO,QString("Fade Previous Steps is %1.").arg(Preferences::enableFadeSteps ? "ON" : "OFF"));

        if (fadeStepsUseColourChanged && Preferences::enableFadeSteps)
            emit messageSig(LOG_INFO,QString("Use Global Fade Colour is %1").arg(Preferences::fadeStepsUseColour ? "ON" : "OFF"));

        if (fadeStepsOpacityChanged && Preferences::enableFadeSteps)
            emit messageSig(LOG_INFO,QString("Fade Step Transparency changed from %1 to %2 percent")
                            .arg(fadeStepsOpacityCompare)
                            .arg(Preferences::fadeStepsOpacity));

        if (fadeStepsColourChanged && Preferences::enableFadeSteps && Preferences::fadeStepsUseColour)
            emit messageSig(LOG_INFO,QString("Fade Step Colour preference changed from %1 to %2")
                            .arg(fadeStepsColourCompare.replace("_"," "))
                            .arg(QString(Preferences::fadeStepsColour).replace("_"," ")));

        if (enableHighlightStepChanged)
            emit messageSig(LOG_INFO,QString("Highlight Current Step is %1.").arg(Preferences::enableHighlightStep ? "ON" : "OFF"));

        if (highlightStepLineWidthChanged && Preferences::enableHighlightStep)
            emit messageSig(LOG_INFO,QString("Highlight Step line width changed from %1 to %2")
                            .arg(highlightStepLineWidthCompare)
                            .arg(Preferences::highlightStepLineWidth));

        if (highlightStepColorChanged && Preferences::enableHighlightStep)
            emit messageSig(LOG_INFO,QString("Highlight Step Colour preference changed from %1 to %2")
                            .arg(highlightStepColourCompare)
                            .arg(Preferences::highlightStepColour));

        if (ldrawPathChanged)
            emit messageSig(LOG_INFO,QString("LDraw path preference changed from %1 to %1")
                            .arg(ldrawPathCompare)
                            .arg(Preferences::ldrawPath));

        if (lgeoPathChanged && !ldrawPathChanged)
            emit messageSig(LOG_INFO,QString("LGEO path preference changed from %1 to %1")
                            .arg(lgeoPathCompare)
                            .arg(Preferences::lgeoPath));

        if (generateCoverPagesChanged)
            emit messageSig(LOG_INFO,QString("Generate Cover Pages preference is %1").arg(Preferences::generateCoverPages ? "ON" : "OFF"));


        if (pageDisplayPauseChanged)
            emit messageSig(LOG_INFO,QString("Continuous process page display pause changed from %1 to %2")
                            .arg(highlightStepLineWidthCompare)
                            .arg(Preferences::pageDisplayPause));

        if (doNotShowPageProcessDlgChanged)
            emit messageSig(LOG_INFO,QString("Show continuous page process options dialog is %1.").arg(Preferences::doNotShowPageProcessDlg ? "ON" : "OFF"));

        if ((((fadeStepsColourChanged && Preferences::fadeStepsUseColour) ||
              fadeStepsUseColourChanged || fadeStepsOpacityChanged) &&
             Preferences::enableFadeSteps && !enableFadeStepsChanged) ||
                ((highlightStepColorChanged || highlightStepLineWidthChanged) &&
                 Preferences::enableHighlightStep && !enableHighlightStepChanged))
            clearCustomPartCache(true);    // true = silent

        if (enableImageMattingChanged && Preferences::enableImageMatting)
            emit messageSig(LOG_INFO,QString("Enable image matting preference changed from %1 to %2")
                            .arg(enableImageMattingCompare)
                            .arg(Preferences::enableImageMatting));

        if (enableLDViewSCallChanged)
            emit messageSig(LOG_INFO,QString("Enable LDView Single Call is %1").arg(Preferences::enableLDViewSingleCall ? "ON" : "OFF"));

        if (enableLDViewSListChanged)
            emit messageSig(LOG_INFO,QString("Enable LDView Snapshots List is %1").arg(Preferences::enableLDViewSnaphsotList ? "ON" : "OFF"));

        if (rendererChanged) {
            emit messageSig(LOG_INFO,QString("Renderer preference changed from %1 to %2 %3")
                            .arg(Preferences::preferredRenderer)
                            .arg(preferredRendererCompare)
                            .arg(preferredRendererCompare == RENDERER_POVRAY ? QString("(PoV file generator is %1)").arg(Preferences::povFileGenerator) :
                                                                               preferredRendererCompare == RENDERER_LDVIEW ? Preferences::enableLDViewSingleCall ? "(Single Call)" : "" : ""));
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
            box.setText (QString("You must close and restart %1 to properly load your alternate LDConfig file.").arg(VER_PRODUCTNAME_STR));
            box.setInformativeText (QString("Click \"OK\" to close and restart %1.\n\n").arg(VER_PRODUCTNAME_STR));
            int result = box.exec();
            if (result == QMessageBox::Ok) {
                altLDConfigPathRestart = true;
            }
        }

        if (Preferences::themeAutoRestart) {
            displayThemeRestart = true;
        }else {
            QMessageBox box;
            box.setMinimumSize(40,20);
            box.setIcon (QMessageBox::Question);
            box.setDefaultButton   (QMessageBox::Ok);
            box.setStandardButtons (QMessageBox::Ok | QMessageBox::Close | QMessageBox::Cancel);
            box.setText (QString("You must close and restart %1 to fully configure the Theme.\n"
                                 "Editor syntax highlighting will update the next time you start %1")
                         .arg(QString::fromLatin1(VER_PRODUCTNAME_STR)));
            box.setInformativeText (QString("Click \"OK\" to close and restart %1 or \"Close\" set the Theme without restart.\n\n"
                                            "You can suppress this message in Preferences, Themes")
                                    .arg(QString::fromLatin1(VER_PRODUCTNAME_STR)));
            int result = box.exec();
            if (result == QMessageBox::Ok)
                displayThemeRestart = true;
        }
        loadTheme(displayThemeRestart);

        if (!getCurFile().isEmpty()) {
            if (enableFadeStepsChanged            ||
                    altLDConfigPathChanged        ||
                    fadeStepsColourChanged        ||
                    fadeStepsUseColourChanged     ||
                    fadeStepsOpacityChanged       ||
                    enableHighlightStepChanged    ||
                    highlightStepColorChanged     ||
                    highlightStepLineWidthChanged ||
                    rendererChanged               ||
                    enableLDViewSCallChanged      ||
                    enableLDViewSListChanged      ||
                    displayAttributesChanged      ||
                    povFileGeneratorChanged       ||
                    enableImageMattingChanged     ||
                    generateCoverPagesChanged){
                clearAndRedrawPage();
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
                    QString Message = QMessageBox::tr("Failed to set log level %1.\n"
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

        // Native Pov file generation settings
        if (qualityChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Quality changed from %1 to %2.")
                            .arg(qualityCompare)
                            .arg(TCUserDefaults::longForKey(QUALITY_EXPORT_KEY, QUALITY_EXPORT_DEFAULT)));

        if (selectedAspectRatioChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Selected aspect ratio changed from %1 to %2.")
                            .arg(selectedAspectRatioCompare)
                            .arg(selectedAspectRatio));

        if (customAspectRatioChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Custom aspect ratio changed from %1 to %2.")
                            .arg(customAspectRatioCompare)
                            .arg(TCUserDefaults::floatForKey(CUSTOM_ASPECT_RATIO_KEY, CUSTOM_ASPECT_RATIO_DEFAULT)));

        if (edgeRadiusChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Edge radius changed from %1 to %2.")
                            .arg(edgeRadiusCompare)
                            .arg(TCUserDefaults::floatForKey(EDGE_RADIUS_KEY, EDGE_RADIUS_DEFAULT)));

        if (seamWidthChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Seam width changed from %1 to %2.")
                            .arg(seamWidthCompare)
                            .arg(TCUserDefaults::floatForKey(SEAM_WIDTH_KEY, EDGE_RADIUS_DEFAULT)));

        if (ambientChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Ambient changed from %1 to %2.")
                            .arg(ambientCompare)
                            .arg(TCUserDefaults::floatForKey(AMBIENT_KEY, AMBIENT_DEFAULT)));

        if (diffuseChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Diffuse changed from %1 to %2.")
                            .arg(diffuseCompare)
                            .arg(TCUserDefaults::floatForKey(DIFFUSE_KEY, DIFFUSE_DEFAULT)));

        if (reflChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Reflection changed from %1 to %2.")
                            .arg(reflCompare)
                            .arg(TCUserDefaults::floatForKey(REFLECTION_KEY, REFLECTION_DEFAULT) ));

        if (phongChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Phong changed from %1 to %2.")
                            .arg(phongCompare)
                            .arg(TCUserDefaults::floatForKey(PHONG_KEY, PHONG_DEFAULT)));

        if (phongSizeChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Phong size changed from %1 to %2.")
                            .arg(phongSizeCompare)
                            .arg(TCUserDefaults::floatForKey(PHONG_SIZE_KEY, PHONG_SIZE_DEFAULT)));

        if (transReflChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Trans reflection changed from %1 to %2.")
                            .arg(transReflCompare)
                            .arg(TCUserDefaults::floatForKey(TRANS_REFLECTION_KEY, TRANS_REFLECTION_DEFAULT)));

        if (transFilterChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Trans filter changed from %1 to %2.")
                            .arg(transFilterCompare)
                            .arg(TCUserDefaults::floatForKey(TRANS_FILTER_KEY, TRANS_FILTER_DEFAULT)));

        if (transIoRChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Trans IoR changed from %1 to %2.")
                            .arg(transIoRCompare)
                            .arg(TCUserDefaults::floatForKey(TRANS_IOR_KEY, TRANS_IOR_DEFAULT)));

        if (rubberReflChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Rubber reflection changed from %1 to %2.")
                            .arg(rubberReflCompare)
                            .arg(TCUserDefaults::floatForKey(RUBBER_REFLECTION_KEY, RUBBER_REFLECTION_DEFAULT)));

        if (rubberPhongChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Rubber phong changed from %1 to %2.")
                            .arg(rubberPhongCompare)
                            .arg(TCUserDefaults::floatForKey(RUBBER_PHONG_KEY, RUBBER_PHONG_DEFAULT)));

        if (rubberPhongSizeChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Rubber phong size changed from %1 to %2.")
                            .arg(rubberPhongSizeCompare)
                            .arg(TCUserDefaults::floatForKey(RUBBER_PHONG_SIZE_KEY, RUBBER_PHONG_SIZE_DEFAULT)));

        if (chromeReflChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Chrome reflection changed from %1 to %2.")
                            .arg(chromeReflCompare)
                            .arg(TCUserDefaults::floatForKey(CHROME_REFLECTION_KEY, CHROME_REFLECTION_DEFAULT)));

        if (chromeBrilChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Chrome brilliance changed from %1 to %2.")
                            .arg(chromeBrilCompare)
                            .arg(TCUserDefaults::floatForKey(CHROME_BRILLIANCE_KEY, CHROME_BRILLIANCE_DEFAULT)));

        if (chromeSpecularChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Chrome specular changed from %1 to %2.")
                            .arg(chromeSpecularCompare)
                            .arg(TCUserDefaults::floatForKey(CHROME_SPECULAR_KEY, CHROME_SPECULAR_DEFAULT)));

        if (chromeRoughnessChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Chrome roughness changed from %1 to %2.")
                            .arg(chromeRoughnessCompare)
                            .arg(TCUserDefaults::floatForKey(CHROME_ROUGHNESS_KEY, CHROME_ROUGHNESS_DEFAULT)));

        if (fileVersionChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("File version changed from %1 to %2.")
                            .arg(fileVersionCompare)
                            .arg(TCUserDefaults::floatForKey(FILE_VERSION_KEY, FILE_VERSION_DEFAULT)));

        if (seamsChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Seams is %1.").arg(TCUserDefaults::boolForKey(SEAMS_KEY, true) ? "ON" : "OFF"));

        if (reflectionsChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Reflections is %1.").arg(TCUserDefaults::boolForKey(REFLECTIONS_KEY, true) ? "ON" : "OFF"));

        if (shadowsChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Shadows is %1.").arg(TCUserDefaults::boolForKey(SHADOWS_KEY, true) ? "ON" : "OFF"));

        if (xmlMapChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Xml map is %1.").arg(TCUserDefaults::boolForKey(XML_MAP_KEY, true) ? "ON" : "OFF"));

        if (inlinePovChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Inline Pov is %1.").arg(TCUserDefaults::boolForKey(INLINE_POV_KEY, true) ? "ON" : "OFF"));

        if (smoothCurvesChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Smooth curves is %1.").arg(TCUserDefaults::boolForKey(SMOOTH_CURVES_KEY, true) ? "ON" : "OFF"));

        if (hideStudsChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Hide studs is %1.").arg(TCUserDefaults::boolForKey(HIDE_STUDS_KEY, false) ? "ON" : "OFF"));

        if (unmirrorStudsChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Unmirror Studs is %1.").arg(TCUserDefaults::boolForKey(UNMIRROR_STUDS_KEY, true) ? "ON" : "OFF"));

        if (findReplacementsChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Find POV replacements is %1.").arg(TCUserDefaults::boolForKey(FIND_REPLACEMENTS_KEY, false) ? "ON" : "OFF"));

        if (conditionalEdgeLinesChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Conditional edge lines is %1.").arg(TCUserDefaults::boolForKey(CONDITIONAL_EDGE_LINES_KEY, false) ? "ON" : "OFF"));

        if (primitiveSubstitutionChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Primitive Substitution is %1.").arg(TCUserDefaults::boolForKey(PRIMITIVE_SUBSTITUTION_KEY, true) ? "ON" : "OFF"));

        //                                   TCUserDefaults::boolForKey(DRAW_EDGES_KEY, false)

        if (xmlMapPathChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("XmlMapPath changed from %1 to %2.")
                            .arg(xmlMapPathCompare)
                            .arg(TCUserDefaults::pathForKey(XML_MAP_PATH_KEY)));

        if (topIncludeChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Top Include changed from %1 to %2.")
                            .arg(topIncludeCompare)
                            .arg(TCUserDefaults::stringForKey(TOP_INCLUDE_KEY)));

        if (bottomIncludeChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Bottom Include changed from %1 to %2.")
                            .arg(bottomIncludeCompare)
                            .arg(TCUserDefaults::stringForKey(BOTTOM_INCLUDE_KEY)));

        if (lightsChanged && nativePovRendererConfig)
            emit messageSig(LOG_INFO,QString("Lights changed from %1 to %2.")
                            .arg(lightsCompare)
                            .arg(Preferences::ldvLights));

        if (displayThemeRestart || altLDConfigPathRestart) {
            restartApplication();
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
    Preferences::nativePovGenPreferences();
    Preferences::viewerPreferences();
    Preferences::publishingPreferences();
    Preferences::exportPreferences();

    displayPageNum = 1;

    processOption                   = EXPORT_ALL_PAGES;
    exportType                      = EXPORT_PDF;
    pageRangeText                   = "1";
    resetCache                      = false;
    m_previewDialog                 = false;
    m_exportingContent              = false;
    nextPageContinuousIsRunning     = false;
    previousPageContinuousIsRunning = false;

    mStepRotation     = lcVector3(0.0f, 0.0f, 0.0f);
    mRotStepAngleX    = 0.0f;
    mRotStepAngleY    = 0.0f;
    mRotStepAngleZ    = 0.0f;
    mRotStepTransform = QString();
    mPliIconsPath.clear();

    editWindow    = new EditWindow(this);  // remove inheritance 'this' to independently manage window
    parmsWindow   = new ParmsWindow();

    KpageScene    = new LGraphicsScene(this);
    KpageScene->setBackgroundBrush(Qt::lightGray);
    KpageView     = new LGraphicsView(KpageScene);
    KpageView->pageBackgroundItem = nullptr;
    KpageView->setRenderHints(QPainter::Antialiasing | 
                              QPainter::TextAntialiasing |
                              QPainter::SmoothPixmapTransform);
    setCentralWidget(KpageView);

    mpdCombo = new QComboBox(this);
    mpdCombo->setToolTip(tr("Current Submodel: %1").arg(mpdCombo->currentText()));
    mpdCombo->setMinimumContentsLength(25);
    mpdCombo->setInsertPolicy(QComboBox::InsertAtBottom);
    mpdCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
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

    connect(this,           SIGNAL(setExportingSig(bool)),
            this,           SLOT(  setExporting(   bool)));

    connect(this,           SIGNAL(setContinuousPageSig(bool)),
            this,           SLOT(  setContinuousPage(   bool)));

    connect(this,           SIGNAL(displayFileSig(LDrawFile *, const QString &)),
            editWindow,     SLOT(  displayFile   (LDrawFile *, const QString &)));

    connect(this,           SIGNAL(displayParmsFileSig(const QString &)),
            parmsWindow,    SLOT( displayParmsFile   (const QString &)));

    connect(this,           SIGNAL(showLineSig(int)),
            editWindow,     SLOT(  showLine(   int)));

    connect(editWindow,     SIGNAL(redrawSig()),
            this,           SLOT(  clearAndRedrawPage()));

    connect(editWindow,     SIGNAL(contentsChange(const QString &,int,int,const QString &)),
            this,           SLOT(  contentsChange(const QString &,int,int,const QString &)));

    connect(undoStack,      SIGNAL(canRedoChanged(bool)),
            this,           SLOT(  canRedoChanged(bool)));
    connect(undoStack,      SIGNAL(canUndoChanged(bool)),
            this,           SLOT(  canUndoChanged(bool)));
    connect(undoStack,      SIGNAL(cleanChanged(bool)),
            this,           SLOT(  cleanChanged(bool)));

    lpubAlert = new LPubAlert();
    connect(lpubAlert, SIGNAL(messageSig(LogType,QString)),   this, SLOT(statusMessage(LogType,QString)));
    connect(this,      SIGNAL(messageSig(LogType,QString)),   this, SLOT(statusMessage(LogType,QString)));

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

    connect (m_progressDialog, SIGNAL (cancelClicked()), this, SLOT (cancelExporting()));

    connect(this, SIGNAL(progressBarInitSig()),               this, SLOT(progressBarInit()));
    connect(this, SIGNAL(progressMessageSig(QString)),        this, SLOT(progressBarSetText(QString)));
    connect(this, SIGNAL(progressRangeSig(int,int)),          this, SLOT(progressBarSetRange(int,int)));
    connect(this, SIGNAL(progressSetValueSig(int)),           this, SLOT(progressBarSetValue(int)));
    connect(this, SIGNAL(progressResetSig()),                 this, SLOT(progressBarReset()));
    connect(this, SIGNAL(progressStatusRemoveSig()),          this, SLOT(progressStatusRemove()));

    connect(this, SIGNAL(progressBarPermInitSig()),           this, SLOT(progressBarPermInit()));
    connect(this, SIGNAL(progressPermMessageSig(QString)),    this, SLOT(progressBarPermSetText(QString)));
    connect(this, SIGNAL(progressPermRangeSig(int,int)),      this, SLOT(progressBarPermSetRange(int,int)));
    connect(this, SIGNAL(progressPermSetValueSig(int)),       this, SLOT(progressBarPermSetValue(int)));
    connect(this, SIGNAL(progressPermResetSig()),             this, SLOT(progressBarPermReset()));
    connect(this, SIGNAL(progressPermStatusRemoveSig()),      this, SLOT(progressPermStatusRemove()));

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

void Gui::getRequireds(){
    // Check preferred renderer value is set before setting Renderer class
    Preferences::getRequireds();
    Render::setRenderer(Preferences::preferredRenderer);
}

void Gui::initialize()
{

  emit Application::instance()->splashMsgSig(QString("85% - %1 initialization...").arg(VER_PRODUCTNAME_STR));

  connect(this,        SIGNAL(loadFileSig(QString)),               this,        SLOT(loadFile(QString)));
  connect(this,        SIGNAL(processCommandLineSig()),            this,        SLOT(processCommandLine()));
  connect(this,        SIGNAL(setExportingSig(bool)),              this,        SLOT(deployExportBanner(bool)));
  connect(this,        SIGNAL(setPliIconPathSig(QString&,QString&)), this,      SLOT(setPliIconPath(QString&,QString&)));

  connect(this,        SIGNAL(setExportingSig(bool)),              gMainWindow, SLOT(Halt3DViewer(bool)));
  connect(this,        SIGNAL(enable3DActionsSig()),               gMainWindow, SLOT(Enable3DActions()));
  connect(this,        SIGNAL(disable3DActionsSig()),              gMainWindow, SLOT(Disable3DActions()));
  connect(this,        SIGNAL(updateAllViewsSig()),                gMainWindow, SLOT(UpdateAllViews()));
  connect(this,        SIGNAL(clearViewerWindowSig()),             gMainWindow, SLOT(NewProject()));

  connect(gMainWindow, SIGNAL(SetRotStepMeta()),                   this,        SLOT(SetRotStepMeta()));
  connect(gMainWindow, SIGNAL(SetRotStepAngleX(float,bool)),       this,        SLOT(SetRotStepAngleX(float,bool)));
  connect(gMainWindow, SIGNAL(SetRotStepAngleY(float,bool)),       this,        SLOT(SetRotStepAngleY(float,bool)));
  connect(gMainWindow, SIGNAL(SetRotStepAngleZ(float,bool)),       this,        SLOT(SetRotStepAngleZ(float,bool)));
  connect(gMainWindow, SIGNAL(SetRotStepTransform(QString&,bool)), this,        SLOT(SetRotStepTransform(QString&,bool)));
  connect(gMainWindow, SIGNAL(GetRotStepMeta()),                   this,        SLOT(GetRotStepMeta()));

/* Moved to PartWorker::ldsearchDirPreferences()  */
//  if (Preferences::preferredRenderer == RENDERER_LDGLITE)
//      partWorkerLdgLiteSearchDirs.populateLdgLiteSearchDirs();

  emit Application::instance()->splashMsgSig(QString("90% - %1 widgets loading...").arg(VER_PRODUCTNAME_STR));

  createActions();
  createMenus();
  createToolBars();
  createStatusBar();
  createDockWindows();
  toggleLCStatusBar();

  emit disable3DActionsSig();
  setCurrentFile("");

  readSettings();

}

void Gui::generateCustomColourPartsList()
{
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr(VER_PRODUCTNAME_STR),
            tr("Generating the colour parts list may take a long time.\n"
                "Are you sure you want to generate this list?"),
            QMessageBox::Yes | QMessageBox::Cancel);
    if (ret == QMessageBox::Yes) {

        QThread *listThread   = new QThread();
        colourPartListWorker  = new ColourPartListWorker();
        colourPartListWorker->moveToThread(listThread);

        connect(listThread,           SIGNAL(started()),                     colourPartListWorker, SLOT(generateCustomColourPartsList()));
        connect(listThread,           SIGNAL(finished()),                              listThread, SLOT(deleteLater()));
        connect(colourPartListWorker, SIGNAL(colourPartListFinishedSig()),             listThread, SLOT(quit()));
        connect(colourPartListWorker, SIGNAL(colourPartListFinishedSig()),   colourPartListWorker, SLOT(deleteLater()));
        connect(this,                 SIGNAL(requestEndThreadNowSig()),      colourPartListWorker, SLOT(requestEndThreadNow()));

        connect(colourPartListWorker, SIGNAL(messageSig(LogType,QString)),                   this, SLOT(statusMessage(LogType,QString)));

        connect(colourPartListWorker, SIGNAL(progressBarInitSig()),                          this, SLOT(progressBarInit()));
        connect(colourPartListWorker, SIGNAL(progressMessageSig(QString)),                   this, SLOT(progressBarSetText(QString)));
        connect(colourPartListWorker, SIGNAL(progressRangeSig(int,int)),                     this, SLOT(progressBarSetRange(int,int)));
        connect(colourPartListWorker, SIGNAL(progressSetValueSig(int)),                      this, SLOT(progressBarSetValue(int)));
        connect(colourPartListWorker, SIGNAL(progressResetSig()),                            this, SLOT(progressBarReset()));
        connect(colourPartListWorker, SIGNAL(progressStatusRemoveSig()),                     this, SLOT(progressStatusRemove()));

        listThread->start();

    } else {
      return;
    }
}

void Gui::processFadeColourParts(bool overwriteCustomParts)
{
  if (gui->page.meta.LPub.fadeStep.fadeStep.value()) {

      partWorkerCustomColour = new PartWorker();

      connect(this,                   SIGNAL(operateFadeParts(bool)),    partWorkerCustomColour, SLOT(processFadeColourParts(bool)));

      connect(partWorkerCustomColour, SIGNAL(messageSig(LogType,QString)),                 this, SLOT(statusMessage(LogType,QString)));

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
  if (gui->page.meta.LPub.highlightStep.highlightStep.value()) {

      partWorkerCustomColour = new PartWorker();

      connect(this,                   SIGNAL(operateHighlightParts(bool)), partWorkerCustomColour, SLOT(processHighlightColourParts(bool)));

      connect(partWorkerCustomColour, SIGNAL(messageSig(LogType,QString)),                this, SLOT(statusMessage(LogType,QString)));

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
void Gui::processLDSearchDirParts(){
  PartWorker partWorkerLDSearchDirs;
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

bool Gui::aboutDialog()
{
    AboutDialog Dialog(this);
    return Dialog.exec() == QDialog::Accepted;
}

void Gui::refreshLDrawUnoffParts(){

    // Download unofficial archive
    emit messageSig(LOG_STATUS,"Refresh LDraw Unofficial Library archive...");
    UpdateCheck *libraryDownload;
    QEventLoop *wait = new QEventLoop();
    QString archivePath = tr("%1/%2").arg(Preferences::lpubDataPath, "libraries");
    libraryDownload = new UpdateCheck(this, (void*)LDrawUnofficialLibraryDownload);
    wait->connect(libraryDownload, SIGNAL(downloadFinished(QString,QString)), wait, SLOT(quit()));
    libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), archivePath);
    wait->exec();

    // Automatically extract unofficial archive
    QString archive = tr("%1/%2").arg(archivePath).arg(FILE_LPUB3D_UNOFFICIAL_ARCHIVE);
    QString destination = tr("%1/unofficial").arg(Preferences::ldrawPath);
    QStringList result = JlCompress::extractDir(archive,destination);
    if (result.isEmpty()){
        emit messageSig(LOG_ERROR,tr("Failed to extract %1 to %2").arg(archive).arg(destination));
    } else {
        QString message = tr("%1 Unofficial Library files extracted to %2").arg(result.size()).arg(destination);
        emit messageSig(LOG_STATUS,message);
    }
}

void Gui::refreshLDrawOfficialParts(){

    // Download official archive
    emit messageSig(LOG_STATUS,"Refresh LDraw Official Library archive...");
    UpdateCheck *libraryDownload;
    QEventLoop *wait = new QEventLoop();
    QString archivePath = tr("%1/%2").arg(Preferences::lpubDataPath, "libraries");
    libraryDownload = new UpdateCheck(this, (void*)LDrawOfficialLibraryDownload);
    wait->connect(libraryDownload, SIGNAL(downloadFinished(QString,QString)), wait, SLOT(quit()));
    libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), archivePath);
    wait->exec();

    // Automatically extract official archive
    QString archive = tr("%1/%2").arg(archivePath).arg(VER_LDRAW_OFFICIAL_ARCHIVE);
    QString destination = Preferences::ldrawPath;
    destination = destination.remove(destination.size() - 6,6);
    QStringList result = JlCompress::extractDir(archive,destination);
    if (result.isEmpty()){
        emit messageSig(LOG_ERROR,tr("Failed to extract %1 to %2/ldraw").arg(archive).arg(destination));
    } else {
        QString message = tr("%1 Official Library files extracted to %2/ldraw").arg(result.size()).arg(destination);
        emit messageSig(LOG_STATUS,message);
    }
}

void Gui::updateCheck()
{
    // Create an instance of update check
    new UpdateCheck(this, (void*)SoftwareUpdate);
}

// Begin Jaco's code
// Danny: web url changed, as it pointed nowhere

#include <QDesktopServices>
#include <QUrl>

void Gui::onlineManual()
{
    QDesktopServices::openUrl(QUrl("http://sites.google.com/site/workingwithlpub/lpub3d"));
}

// End Jaco's code


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
    emit messageSig(LOG_ERROR, QMessageBox::tr("Cannot write file %1:\n%2.")
                    .arg(fileName)
                    .arg(file.errorString()));
    return;
  }

  QTextStream out(&file);

  doc.prepend(QString());
  doc.prepend(QString("%1 %2 - Generated on %3")
                       .arg(VER_PRODUCTNAME_STR)
                       .arg(QFileInfo(fileName).baseName())
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

void Gui::createActions()
{
    openAct = new QAction(QIcon(":/resources/open.png"), tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open an existing file - Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

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

    closeFileAct = new QAction(QIcon(":/resources/closemodelfile.png"), tr("Cl&ose..."), this);
    closeFileAct->setShortcut(tr("Ctrl+O"));
    closeFileAct->setStatusTip(tr("Close an existing file - Ctrl+O"));
    closeFileAct->setEnabled(false);
    connect(closeFileAct, SIGNAL(triggered()), this, SLOT(closeModelFile()));

    printToFileAct = new QAction(QIcon(":/resources/file_print.png"), tr("&Print..."), this);
    printToFileAct->setShortcut(tr(""));
    printToFileAct->setStatusTip(tr("Print the current document"));
    printToFileAct->setEnabled(false);
    connect(printToFileAct, SIGNAL(triggered()), this, SLOT(ShowPrintDialog()));

    exportAsPdfPreviewAct = new QAction(QIcon(":/resources/pdf_print_preview.png"), tr("PDF Export Preview..."), this);
    exportAsPdfPreviewAct->setShortcut(tr("Alt+P"));
    exportAsPdfPreviewAct->setStatusTip(tr("Preview the current document to be printed - Alt+P"));
    exportAsPdfPreviewAct->setEnabled(false);
    connect(exportAsPdfPreviewAct, SIGNAL(triggered()), this, SLOT(TogglePrintPreview()));

    exportAsPdfAct = new QAction(QIcon(":/resources/pdf_logo.png"), tr("Export to PDF &File"), this);
    exportAsPdfAct->setShortcut(tr("Alt+F"));
    exportAsPdfAct->setStatusTip(tr("Export your document to a pdf file - Alt+F"));
    exportAsPdfAct->setEnabled(false);
    connect(exportAsPdfAct, SIGNAL(triggered()), this, SLOT(exportAsPdfDialog()));

    exportPngAct = new QAction(QIcon(":/resources/exportpng.png"),tr("Export As P&NG Images"), this);
    exportPngAct->setShortcut(tr("Alt+N"));
    exportPngAct->setStatusTip(tr("Export your document as a sequence of PNG images - Alt+N"));
    exportPngAct->setEnabled(false);
    connect(exportPngAct, SIGNAL(triggered()), this, SLOT(exportAsPngDialog()));

    exportJpgAct = new QAction(QIcon(":/resources/exportjpeg.png"),tr("Export As &JPEG Images"), this);
    exportJpgAct->setShortcut(tr("Alt+J"));
    exportJpgAct->setStatusTip(tr("Export your document as a sequence of JPEG images - Alt+J"));
    exportJpgAct->setEnabled(false);
    connect(exportJpgAct, SIGNAL(triggered()), this, SLOT(exportAsJpgDialog()));

    exportBmpAct = new QAction(QIcon(":/resources/exportbmp.png"),tr("Export As &Bitmap Images"), this);
    exportBmpAct->setShortcut(tr("Alt+B"));
    exportBmpAct->setStatusTip(tr("Export your document as a sequence of bitmap images - Alt+B"));
    exportBmpAct->setEnabled(false);
    connect(exportBmpAct, SIGNAL(triggered()), this, SLOT(exportAsBmpDialog()));

    exitAct = new QAction(QIcon(":/resources/exit.png"),tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application - Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    for (int i = 0; i < MaxRecentFiles; i++) {
      recentFilesActs[i] = new QAction(this);
      recentFilesActs[i]->setVisible(false);
      connect(recentFilesActs[i], SIGNAL(triggered()), this, 
                                 SLOT(openRecentFile()));
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

    // fitWidth,fitVisible,actualSize

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

    pageRulerAct = new QAction(QIcon(":/resources/pageruler.png"), tr("Page &Ruler"), this);
    pageRulerAct->setShortcut(tr("Alt+U"));
    pageRulerAct->setStatusTip(tr("Display the page ruler - Alt+U"));
    pageRulerAct->setEnabled(true);
    pageRulerAct->setCheckable(true);
    pageRulerAct->setChecked(Preferences::pageRuler);
    connect(pageRulerAct, SIGNAL(triggered()), this, SLOT(pageRuler()));

    pageGuidesAct = new QAction(QIcon(":/resources/pageguides.png"), tr("Page &Guides"), this);
    pageGuidesAct->setShortcut(tr("Alt+G"));
    pageGuidesAct->setStatusTip(tr("Display horizontal and vertical guides - Alt+G"));
    pageGuidesAct->setEnabled(true);
    pageGuidesAct->setCheckable(true);
    pageGuidesAct->setChecked(Preferences::pageGuides);
    connect(pageGuidesAct, SIGNAL(triggered()), this, SLOT(pageGuides()));

    actualSizeAct = new QAction(QIcon(":/resources/actual.png"),tr("&Actual Size"), this);
    actualSizeAct->setShortcut(tr("Alt+A"));
    actualSizeAct->setStatusTip(tr("Show document actual size - Alt+A"));
    actualSizeAct->setEnabled(false);
    connect(actualSizeAct, SIGNAL(triggered()), this, SLOT(actualSize()));

    // TESTING ONLY
    //connect(actualSizeAct, SIGNAL(triggered()), this, SLOT(twoPages()));
    
    // zoomIn,zoomOut

    zoomInAct = new QAction(QIcon(":/resources/zoomin.png"), tr("&Zoom In"), this);
    zoomInAct->setShortcut(tr("Ctrl++"));
    zoomInAct->setStatusTip(tr("Zoom in - Ctrl++"));
    zoomInAct->setEnabled(false);
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(QIcon(":/resources/zoomout.png"),tr("Zoom &Out"),this);
    zoomOutAct->setShortcut(tr("Ctrl+-"));
    zoomOutAct->setStatusTip(tr("Zoom out - Ctrl+-"));
    zoomOutAct->setEnabled(false);
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

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
    refreshLDrawUnoffPartsAct->setStatusTip(tr("Download and replace LDraw Unofficial parts archive file"));
    connect(refreshLDrawUnoffPartsAct, SIGNAL(triggered()), this, SLOT(refreshLDrawUnoffParts()));

    refreshLDrawOfficialPartsAct = new QAction(QIcon(":/resources/refreshoffarchive.png"),tr("Refresh LDraw Official Parts"), this);
    refreshLDrawOfficialPartsAct->setStatusTip(tr("Download and replace LDraw Official parts archive file"));
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

    editTitleAnnotationsAct = new QAction(QIcon(":/resources/edittitleannotations.png"),tr("Edit Part Title PLI Annotations"), this);
    editTitleAnnotationsAct->setStatusTip(tr("Add/Edit part title PLI part annotatons"));
    connect(editTitleAnnotationsAct, SIGNAL(triggered()), this, SLOT(editTitleAnnotations()));

    editFreeFormAnnitationsAct = new QAction(QIcon(":/resources/editfreeformannotations.png"),tr("Edit Freeform PLI Annotations"), this);
    editFreeFormAnnitationsAct->setStatusTip(tr("Add/Edit freeform PLI part annotations"));
    connect(editFreeFormAnnitationsAct, SIGNAL(triggered()), this, SLOT(editFreeFormAnnitations()));

    editLDrawColourPartsAct = new QAction(QIcon(":/resources/editldrawcolourparts.png"),tr("Edit LDraw Static Colour Parts List"), this);
    editLDrawColourPartsAct->setStatusTip(tr("Add/Edit the list of LDraw static colour parts used to process fade and highlight steps"));
    connect(editLDrawColourPartsAct, SIGNAL(triggered()), this, SLOT(editLDrawColourParts()));

    editPliBomSubstitutePartsAct = new QAction(QIcon(":/resources/editplisubstituteparts.png"),tr("Edit PLI/BOM Substitute Parts List"), this);
    editPliBomSubstitutePartsAct->setStatusTip(tr("Add/Edit the list of PLI/BOM substitute parts"));
    connect(editPliBomSubstitutePartsAct, SIGNAL(triggered()), this, SLOT(editPliBomSubstituteParts()));

    editExcludedPartsAct = new QAction(QIcon(":/resources/editexcludedparts.png"),tr("Edit Part Count Excluded Parts List"), this);
    editExcludedPartsAct->setStatusTip(tr("Add/Edit the list of part count excluded parts"));
    connect(editExcludedPartsAct, SIGNAL(triggered()), this, SLOT(editExcludedParts()));

    editLdrawIniFileAct = new QAction(QIcon(":/resources/editinifile.png"),tr("Edit LDraw.ini"), this);
    editLdrawIniFileAct->setStatusTip(tr("Add/Edit LDraw.ini search directory entries"));
    connect(editLdrawIniFileAct, SIGNAL(triggered()), this, SLOT(editLdrawIniFile()));

    editLdgliteIniAct = new QAction(QIcon(":/resources/editldgliteconf.png"),tr("Edit LDGLite ini configuration file"), this);
    editLdgliteIniAct->setStatusTip(tr("Edit LDGLite ini configuration file"));
    connect(editLdgliteIniAct, SIGNAL(triggered()), this, SLOT(editLdgliteIni()));

    editNativePOVIniAct = new QAction(QIcon(":/resources/LPub32.png"),tr("Edit Native POV file generation configuration file"), this);
    editNativePOVIniAct->setStatusTip(tr("Edit Native POV file generation configuration file"));
    connect(editNativePOVIniAct, SIGNAL(triggered()), this, SLOT(editNativePovIni()));

    editLdviewIniAct = new QAction(QIcon(":/resources/editldviewconf.png"),tr("Edit LDView ini configuration file"), this);
    editLdviewIniAct->setStatusTip(tr("Edit LDView ini configuration file"));
    connect(editLdviewIniAct, SIGNAL(triggered()), this, SLOT(editLdviewIni()));

    editLPub3DIniFileAct = new QAction(QIcon(":/resources/editsetting.png"),tr("Edit %1 configuration file").arg(VER_PRODUCTNAME_STR), this);
    editLPub3DIniFileAct->setStatusTip(tr("Edit %1 application configuration settings file").arg(VER_PRODUCTNAME_STR));
    connect(editLPub3DIniFileAct, SIGNAL(triggered()), this, SLOT(editLPub3DIniFile()));

    editLdviewPovIniAct = new QAction(QIcon(":/resources/editldviewconf.png"),tr("Edit LDView POV file generation configuration file"), this);
    editLdviewPovIniAct->setStatusTip(tr("Edit LDView POV file generation configuration file"));
    connect(editLdviewPovIniAct, SIGNAL(triggered()), this, SLOT(editLdviewPovIni()));

    editPovrayIniAct = new QAction(QIcon(":/resources/editpovrayconf.png"),tr("Edit Raytracer (POV-Ray) ini configuration file"), this);
    editPovrayIniAct->setStatusTip(tr("Edit Raytracer (POV-Ray) ini configuration file"));
    connect(editPovrayIniAct, SIGNAL(triggered()), this, SLOT(editPovrayIni()));

    editPovrayConfAct = new QAction(QIcon(":/resources/editpovrayconf.png"),tr("Edit Raytracer (POV-Ray) file access configuration file"), this);
    editPovrayConfAct->setStatusTip(tr("Edit Raytracer (POV-Ray) file access configuration file"));
    connect(editPovrayConfAct, SIGNAL(triggered()), this, SLOT(editPovrayConf()));

    generateCustomColourPartsAct = new QAction(QIcon(":/resources/generatecolourparts.png"),tr("Generate Static Colour Parts List"), this);
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
  closeFileAct->setEnabled(true);

  exportAsPdfAct->setEnabled(true);
  printToFileAct->setEnabled(true);
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
  actualSizeAct->setEnabled(true);
  zoomInAct->setEnabled(true);
  zoomOutAct->setEnabled(true);

  setupMenu->setEnabled(true);
  cacheMenu->setEnabled(true);
  exportMenu->setEnabled(true);

  //ExportMenuViewer->setEnabled(true); // Hide 3DViewer step export functions

}

void Gui::disableActions()
{
  saveAsAct->setEnabled(false);
  closeFileAct->setEnabled(false);

  exportAsPdfAct->setEnabled(false);
  printToFileAct->setEnabled(false);
  exportAsPdfPreviewAct->setEnabled(false);

  exportPngAct->setEnabled(false);
  exportJpgAct->setEnabled(false);
  exportBmpAct->setEnabled(false);

  pageSetupAct->setEnabled(false);
  assemSetupAct->setEnabled(false);
  pliSetupAct->setEnabled(false);
  bomSetupAct->setEnabled(false);
  calloutSetupAct->setEnabled(false);
  multiStepSetupAct->setEnabled(false);
  projectSetupAct->setEnabled(false);
  fadeStepSetupAct->setEnabled(false);
  highlightStepSetupAct->setEnabled(false);

  addPictureAct->setEnabled(false);
  removeLPubFormattingAct->setEnabled(false);

  editTitleAnnotationsAct->setEnabled(false);
  editFreeFormAnnitationsAct->setEnabled(false);
  editLDrawColourPartsAct->setEnabled(false);
  editPliBomSubstitutePartsAct->setEnabled(false);
  editExcludedPartsAct->setEnabled(false);
  editLdgliteIniAct->setEnabled(false);
  editNativePOVIniAct->setEnabled(false);
  editLdviewIniAct->setEnabled(false);
  editLdviewPovIniAct->setEnabled(false);
  editPovrayIniAct->setEnabled(false);
  editPovrayConfAct->setEnabled(false);

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
  actualSizeAct->setEnabled(false);
  zoomInAct->setEnabled(false);
  zoomOutAct->setEnabled(false);

  setupMenu->setEnabled(false);
  cacheMenu->setEnabled(false);
  exportMenu->setEnabled(false);

  // ExportMenuViewer->setEnabled(false); // Hide 3DViewer step export functions

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
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(closeFileAct);

    exportMenu = fileMenu->addMenu("Export As...");
    exportMenu->setIcon(QIcon(":/resources/exportas.png"));
    exportMenu->addAction(exportPngAct);
    exportMenu->addAction(exportJpgAct);
    exportMenu->addAction(exportBmpAct);
#ifdef Q_OS_WIN
    exportMenu->addAction(exportBmpAct);
#endif
    exportMenu->setDisabled(true);

    //fileMenu->addAction(printToFileAct);
    fileMenu->addAction(exportAsPdfPreviewAct);
    fileMenu->addAction(exportAsPdfAct);
    fileMenu->addSeparator();

    recentFileMenu = fileMenu->addMenu(tr("Recent Files..."));
    recentFileMenu->setIcon(QIcon(":/resources/recentfiles.png"));
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

    QMenu* ToolBarEditorMenu = editMenu->addMenu(tr("LDraw File Editor T&oolbar"));
    ToolBarEditorMenu->addAction(editWindow->editToolBar->toggleViewAction());

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(fitWidthAct);
    viewMenu->addAction(fitVisibleAct);
    viewMenu->addAction(actualSizeAct);
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addAction(pageRulerAct);
    viewMenu->addAction(pageGuidesAct);

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
    toolsMenu->addAction(refreshLDrawUnoffPartsAct);
    toolsMenu->addAction(refreshLDrawOfficialPartsAct);
    cacheMenu->addAction(clearAllCachesAct);
    cacheMenu->addAction(clearPLICacheAct);
    cacheMenu->addAction(clearCSICacheAct);
    cacheMenu->addAction(clearTempCacheAct);
    cacheMenu->addAction(clearCustomPartCacheAct);
    cacheMenu->setDisabled(true);

    configMenu = menuBar()->addMenu(tr("&Configuration"));
    // TODO - insert Render Preferences Action Here...

    setupMenu = configMenu->addMenu("Build &Instructions Setup...");
    setupMenu->setIcon(QIcon(":/resources/instructionsetup.png"));
    setupMenu->addAction(pageSetupAct);
    setupMenu->addAction(assemSetupAct);
    setupMenu->addAction(pliSetupAct);
    setupMenu->addAction(bomSetupAct);
    setupMenu->addAction(calloutSetupAct);
    setupMenu->addAction(multiStepSetupAct);
    setupMenu->addAction(projectSetupAct);
    setupMenu->addAction(fadeStepSetupAct);
    setupMenu->addAction(highlightStepSetupAct);
    setupMenu->setDisabled(true);

    configMenu->addSeparator();
    editorMenu = configMenu->addMenu("Edit Parameter Files");
    editorMenu->setIcon(QIcon(":/resources/editparameterfiles.png"));
    editorMenu->addAction(editLDrawColourPartsAct);
    editorMenu->addAction(editTitleAnnotationsAct);
    editorMenu->addAction(editFreeFormAnnitationsAct);
    editorMenu->addAction(editPliBomSubstitutePartsAct);
    editorMenu->addAction(editExcludedPartsAct);
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
    configMenu->addAction(generateCustomColourPartsAct);
    configMenu->addSeparator();
    configMenu->addAction(preferencesAct);

   // 3DViewer Menus
    ViewerMenu = menuBar()->addMenu(tr("&3DViewer"));
    ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_PREFERENCES]);
    ViewerMenu->addSeparator();
    ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_ZOOM_EXTENTS]);
    ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_LOOK_AT]);

    ViewerMenu->addMenu(gMainWindow->GetViewpointMenu());
    ViewerMenu->addMenu(gMainWindow->GetCameraMenu());
    ViewerMenu->addMenu(gMainWindow->GetProjectionMenu());
    ViewerMenu->addMenu(gMainWindow->GetShadingMenu());

    ViewerMenu->addSeparator();
    ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_SPLIT_HORIZONTAL]);
    ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_SPLIT_VERTICAL]);
    ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_REMOVE_VIEW]);
    ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_RESET_VIEWS]);
    ViewerMenu->addSeparator();

    QMenu* ToolBarViewerMenu = ViewerMenu->addMenu(tr("3DViewer Too&lbar"));
    ToolBarViewerMenu->addAction(gMainWindow->mToolsToolBar->toggleViewAction());

    /* // Hide 3DViewer step export functions
    FileMenuViewer = menuBar()->addMenu(tr("&Step"));
    FileMenuViewer->addAction(gMainWindow->mActions[LC_FILE_SAVEAS]);
    FileMenuViewer->addAction(gMainWindow->mActions[LC_FILE_SAVE_IMAGE]);

    ExportMenuViewer = FileMenuViewer->addMenu(tr("&Export Step As"));
    ExportMenuViewer->addAction(gMainWindow->mActions[LC_FILE_EXPORT_3DS]);
    ExportMenuViewer->addAction(gMainWindow->mActions[LC_FILE_EXPORT_BRICKLINK]);
    ExportMenuViewer->addAction(gMainWindow->mActions[LC_FILE_EXPORT_COLLADA]);
    ExportMenuViewer->addAction(gMainWindow->mActions[LC_FILE_EXPORT_CSV]);
    ExportMenuViewer->addAction(gMainWindow->mActions[LC_FILE_EXPORT_HTML]);
    ExportMenuViewer->addAction(gMainWindow->mActions[LC_FILE_EXPORT_POVRAY]);
    ExportMenuViewer->addAction(gMainWindow->mActions[LC_FILE_EXPORT_WAVEFRONT]);
    ExportMenuViewer->setDisabled(true);
    // 3D Viewer Menus End
    */

    // Help Menus

    helpMenu = menuBar()->addMenu(tr("&Help"));
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
    // About 3D Viewer
    helpMenu->addAction(gMainWindow->mActions[LC_HELP_ABOUT]);
}

void Gui::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->setObjectName("FileToolbar");
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);

    //fileToolBar->addAction(printToFileAct);
    fileToolBar->addAction(exportAsPdfPreviewAct);
    fileToolBar->addAction(exportAsPdfAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->setObjectName("EditToolbar");
    editToolBar->addAction(undoAct);
    editToolBar->addAction(redoAct);

    navigationToolBar = addToolBar(tr("Navigation"));
    navigationToolBar->setObjectName("NavigationToolbar");
    navigationToolBar->addAction(firstPageAct);
    previousPageContinuousMenu = new QMenu(tr("Continuous Page Previous"), this);
    previousPageContinuousMenu->addAction(previousPageContinuousAct);
    previousPageComboAct->setMenu(previousPageContinuousMenu);
    navigationToolBar->addAction(previousPageComboAct);
    navigationToolBar->addWidget(setPageLineEdit);
    nextPageContinuousMenu = new QMenu(tr("Continuous Page Next"), this);
    nextPageContinuousMenu->addAction(nextPageContinuousAct);
    nextPageComboAct->setMenu(nextPageContinuousMenu);
    navigationToolBar->addAction(nextPageComboAct);
    navigationToolBar->addAction(lastPageAct);
    navigationToolBar->addWidget(setGoToPageCombo);

    mpdToolBar = addToolBar(tr("MPD"));
    mpdToolBar->setObjectName("MPDToolbar");
    mpdToolBar->addWidget(mpdCombo);

    zoomToolBar = addToolBar(tr("Zoom"));
    zoomToolBar->setObjectName("ZoomToolbar");
    zoomToolBar->addAction(fitVisibleAct);
    zoomToolBar->addAction(fitWidthAct);
// Jaco add actual size icon. Was missing.
    zoomToolBar->addAction(actualSizeAct);
    zoomToolBar->addAction(zoomInAct);
    zoomToolBar->addAction(zoomOutAct);
    zoomToolBar->addAction(pageRulerAct);
    zoomToolBar->addAction(pageGuidesAct);
}

void Gui::statusBarMsg(QString msg)
{
  statusBar()->showMessage(msg);
}

void Gui::createStatusBar()
{

  statusBar()->showMessage(tr("Ready"));
  connect(gMainWindow->mLCStatusBar, SIGNAL(messageChanged(QString)), this, SLOT(showLCStatusMessage()));

}

void Gui::showLCStatusMessage(){

    if(!viewerDockWindow->isFloating())
    statusBarMsg(gMainWindow->mLCStatusBar->currentMessage());
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
            QString Message = QMessageBox::tr("Failed to set log level %1.\n"
                                              "Logging is off - level set to OffLevel")
                    .arg(Preferences::loggingLevel);
            fprintf(stderr, "%s", Message.toLatin1().constData());
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

            logStatus() << message;

            if (guiEnabled) {
                statusBarMsg(message);
            } else {
                fprintf(stdout,"%s",QString(message).append("\n").toLatin1().constData());
                fflush(stdout);
            }
        } else
            if (logType == LOG_INFO) {

                logInfo() << message;

                if (!guiEnabled) {
                    fprintf(stdout,"%s",QString(message).append("\n").toLatin1().constData());
                    fflush(stdout);
                }

            } else
              if (logType == LOG_NOTICE) {

                  logNotice() << message;

                  if (!guiEnabled) {
                      fprintf(stdout,"%s",QString(message).append("\n").toLatin1().constData());
                      fflush(stdout);
                  }

            } else
              if (logType == LOG_TRACE) {

                  logTrace() << message;

                  if (!guiEnabled) {
                      fprintf(stdout,"%s",QString(message).append("\n").toLatin1().constData());
                      fflush(stdout);
                  }

            } else
              if (logType == LOG_DEBUG) {

                  logDebug() << message;

                  if (!guiEnabled) {
                      fprintf(stdout,"%s",QString(message).append("\n").toLatin1().constData());
                      fflush(stdout);
                  }

                } else
              if (logType == LOG_INFO_STATUS) {

                  statusBarMsg(message);

                  logInfo() << message;

                  if (!guiEnabled) {
                      fprintf(stdout,"%s",QString(message).append("\n").toLatin1().constData());
                      fflush(stdout);
                  }
            } else
              if (logType == LOG_ERROR) {

                  logError() << message;

                  if (guiEnabled) {
                      if (ContinuousPage()) {
                          statusBarMsg(message.prepend("ERROR: "));
                      } else {
                          QMessageBox::warning(this,tr(VER_PRODUCTNAME_STR),tr(message.toLatin1()));
                      }
                  } else {
                      fprintf(stdout,"%s",QString(message).append("\n").toLatin1().constData());
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


void Gui::parseError(QString errorMsg,Where &here)
{
    QString parseMessage = QString("%1 (%2:%3)") .arg(errorMsg) .arg(here.modelName) .arg(here.lineNumber);
    if (Preferences::modeGUI) {
        showLine(here);
        if (Preferences::showParseErrors) {
            QCheckBox *cb = new QCheckBox("Do not show this message again.");
            QMessageBox box;
            box.setText(parseMessage);
            box.setIcon(QMessageBox::Icon::Warning);
            box.addButton(QMessageBox::Ok);
            box.addButton(QMessageBox::Cancel);
            box.setDefaultButton(QMessageBox::Cancel);
            box.setCheckBox(cb);

            QObject::connect(cb, &QCheckBox::stateChanged, [this](int state){
                if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) {
                    Preferences::setShowParseErrorsPreference(false);
                } else {
                    Preferences::setShowParseErrorsPreference(true);
                }
            });
            box.exec();
        }
    }
    logError() << qPrintable(parseMessage);
}

void Gui::createDockWindows()
{
    fileEditDockWindow = new QDockWidget(trUtf8(wCharToUtf8("LDraw\u2122 File Editor")), this);
    fileEditDockWindow->setObjectName("LDrawFileDockWindow");
    fileEditDockWindow->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    fileEditDockWindow->setWidget(editWindow);
    addDockWidget(Qt::RightDockWidgetArea, fileEditDockWindow);
    viewMenu->addAction(fileEditDockWindow->toggleViewAction());

    //3D Viewer

    viewerDockWindow = new QDockWidget(trUtf8(wCharToUtf8("3DViewer - by LeoCAD\u00A9")), this);
    viewerDockWindow->setObjectName("ModelDockWindow");
    viewerDockWindow->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    viewerDockWindow->setWidget(gMainWindow);
    addDockWidget(Qt::RightDockWidgetArea, viewerDockWindow);
    viewMenu->addAction(viewerDockWindow->toggleViewAction());

    tabifyDockWidget(viewerDockWindow, fileEditDockWindow);

    //timeline
    gMainWindow->mTimelineToolBar->setWindowTitle(trUtf8(wCharToUtf8("Timeline - by LeoCAD\u00A9")));
    gMainWindow->mTimelineToolBar->setObjectName("TimelineToolbar");
    gMainWindow->mTimelineToolBar->setAllowedAreas(
         Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    gMainWindow->mTimelineToolBar->setAcceptDrops(true);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->mTimelineToolBar);
    viewMenu->addAction(gMainWindow->mTimelineToolBar->toggleViewAction());

    tabifyDockWidget(viewerDockWindow, gMainWindow->mTimelineToolBar);

    //Properties
    gMainWindow->mPropertiesToolBar->setWindowTitle(trUtf8(wCharToUtf8("Properties - by LeoCAD\u00A9")));
    gMainWindow->mPropertiesToolBar->setObjectName("PropertiesToolbar");
    gMainWindow->mPropertiesToolBar->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->mPropertiesToolBar);
    viewMenu->addAction(gMainWindow->mPropertiesToolBar->toggleViewAction());

    tabifyDockWidget(gMainWindow->mTimelineToolBar, gMainWindow->mPropertiesToolBar);

#ifdef Q_OS_MAC
    // modelDock window Hack is not stable on macOS so start with fileEdit Window until I figure out what's wrong.
    fileEditDockWindow->raise();
#else
    viewerDockWindow->raise();
#endif

    connect(viewerDockWindow, SIGNAL (topLevelChanged(bool)), this, SLOT (toggleLCStatusBar()));
    //**
}

void Gui::toggleLCStatusBar(){

    if(viewerDockWindow->isFloating())
        gMainWindow->statusBar()->show();
    else
        gMainWindow->statusBar()->hide();
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
    Settings.endGroup();
}

void Gui::writeSettings()
{
    QSettings Settings;
    Settings.beginGroup(MAINWINDOW);
    Settings.setValue("Geometry", saveGeometry());
    Settings.setValue("State", saveState());
    Settings.setValue("Size", size());
    Settings.endGroup();
}
