
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.
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
#include "lpub_object.h"
#include "preferencesdialog.h"
#include "renderdialog.h"
#include "metaitem.h"
#include "ranges_element.h"
#include "updatecheck.h"
#include "step.h"
#include "texteditdialog.h"
#include "messageboxresizable.h"
#include "separatorcombobox.h"
#include <commands/command.h>
#include <commands/commandcollection.h>
#include <commands/commandsdialog.h>

#include "pairdialog.h"
#include "aboutdialog.h"
#include "dialogexportpages.h"
#include "numberitem.h"
#include "progress_dialog.h"
#include "waitingspinnerwidget.h"
#include "qsimpleupdater.h"

//Visual Editor
#include "camera.h"
#include "lc_view.h"
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

QHash<SceneObject, QString> soMap;

int          Gui::pa;                     // page adjustment
int          Gui::sa;                     // step number adustment
int          Gui::maxPages;
int          Gui::boms;                   // the number of pli BOMs in the document
int          Gui::bomOccurrence;          // the actual occurrence of each pli BO
int          Gui::displayPageNum;         // what page are we displaying
int          Gui::processOption;          // export Option
int          Gui::pageDirection;          // page processing direction
int          Gui::savePrevStepPosition;   // indicate the previous step position amongst current and previous steps
int          Gui::pageProcessRunning;     // indicate page processing stage - 0=none, 1=writeToTmp,2-find/drawPage...
int          Gui::firstStepPageNum;       // the first Step page number - used to specify frontCover page
int          Gui::lastStepPageNum;        // the last Step page number - used to specify backCover page
int          Gui::saveRenderer;           // saved renderer when temporarily switching to Native renderer

bool         Gui::saveProjection;         // saved projection when temporarily switching to Native renderer
bool         Gui::buildModJumpForward;    // parse build mods in countPage call - special case for jump forward
bool         Gui::resetCache;             // reset model, fade and highlight parts
QString      Gui::curFile;                // the file name for MPD, or top level file
QString      Gui::saveFileName;           // user specified output file Name [commandline only]
QString      Gui::pageRangeText;          // page range parameters
QList<Where> Gui::topOfPages;             // topOfStep list of modelName and lineNumber for each page
QList<Where> Gui::parsedMessages;         // previously parsed messages within the current session

bool         Gui::m_exportingContent;     // indicate export/printing underway
bool         Gui::m_exportingObjects;     // indicate exporting non-image object file content
bool         Gui::m_contPageProcessing;   // indicate continuous page processing underway
bool         Gui::m_countWaitForFinished; // indicate wait for countPage to finish on exporting 'return to saved page'
bool         Gui::mloadingFile;           // when true, the endMacro() call will not call displayPage()

int          Gui::m_exportMode;           // export Mode
QString      Gui::m_saveDirectoryName;    // user specified output directory name [commandline only]

RendererData Gui::savedRendererData;      // store current renderer data when temporarily switching renderer;
QMap<int, PgSizeData>  Gui::pageSizes;    // page size and orientation object

/***********************************************************************
 * set Native renderer for fast processing
 **********************************************************************/

void Gui::setNativeRenderer() {
    if (Preferences::preferredRenderer != RENDERER_NATIVE) {
        saveRenderer   = Preferences::preferredRenderer;
        saveProjection = Preferences::perspectiveProjection;
        Preferences::preferredRenderer     = RENDERER_NATIVE;
        Preferences::perspectiveProjection = true;
        Render::setRenderer(Preferences::preferredRenderer);
    }
}

void Gui::restorePreferredRenderer() {
    if (saveRenderer != RENDERER_INVALID) {
        Preferences::preferredRenderer      = saveRenderer;
        Preferences::perspectiveProjection  = saveProjection;
        Render::setRenderer(Preferences::preferredRenderer);
        saveRenderer = RENDERER_INVALID;
    }
}

QString Gui::GetPliIconsPath(QString& key)
{
    if (mPliIconsPath.contains(key))
        return mPliIconsPath.value(key);

    return QString();
}

void Gui::setPliIconPath(QString& key, QString& value)
{
    if (!mPliIconsPath.contains(key)) {
        mPliIconsPath.insert(key,value);
        emit messageSig(LOG_NOTICE, QString("Icon Inserted: Key [%1] Value [%2]")
                                            .arg(key).arg(value));
        return;
    }
//      emit messageSig(LOG_DEBUG, QString("Icon Exist (Insert Ignored): Key [%1] Value [%2]")
//                                         .arg(key).arg(value));
}

void Gui::SetRotStepAngleX(float AngleX, bool display)
{
    mRotStepAngleX = AngleX;
    if (display)
        ShowStepRotationStatus();
}

void Gui::SetRotStepAngleY(float AngleY, bool display)
{
    mRotStepAngleY = AngleY;
    if (display)
        ShowStepRotationStatus();
}

void Gui::SetRotStepAngleZ(float AngleZ, bool display)
{
    mRotStepAngleZ = AngleZ;
    if (display)
        ShowStepRotationStatus();
}

void Gui::SetRotStepTransform(QString& Transform, bool display)
{
    mRotStepTransform = Transform;
    if (display)
        ShowStepRotationStatus();
}

void Gui::fullScreenView()
{
    if (getAct("fullScreenViewAct.1")->isChecked()) {
        showFullScreen();
    } else {
        showNormal();
    }
}

/****************************************************************************
 *
 * The Gui constructor and destructor are at the bottom of the file with
 * the code that creates the basic GUI framework
 *
 ***************************************************************************/

void Gui::insertCoverPage()
{
  lpub->mi.insertCoverPage();
}

void Gui::appendCoverPage()
{
  lpub->mi.appendCoverPage();
  countPages();
  displayPageNum = maxPages;
  displayPage();
}

void Gui::generateCoverPages()
{
    if (Preferences::generateCoverPages){
        if (!lpub->mi.frontCoverPageExist())
            lpub->mi.insertCoverPage();

        if (!lpub->mi.backCoverPageExist())
            lpub->mi.appendCoverPage();
    }
}

void Gui::insertFinalModelStep() {
  lpub->mi.insertFinalModelStep();
}

void Gui::deleteFinalModelStep() {
  lpub->mi.deleteFinalModelStep();
}

//void Gui::insertCoverPage()
//{
//  lpub->mi.insertCoverPage();
//}

//void Gui::appendCoverPage()
//{
//  lpub->mi.appendCoverPage();
//  countPages();
//  ++displayPageNum;
//  displayPage();  // display the page we just added
//}

void Gui::insertNumberedPage()
{
  lpub->mi.insertNumberedPage();
}

void Gui::appendNumberedPage()
{
  lpub->mi.appendNumberedPage();
//countPages();
//++displayPageNum;
//displayPage();    // display the page we just added
}

void Gui::deletePage()
{
  lpub->mi.deletePage();
}

void Gui::addPicture()
{
  lpub->mi.insertPicture();
}

void Gui::addText()
{
  lpub->mi.insertText();
}

void Gui::addBom()
{
  lpub->mi.insertBOM();
}

void Gui::removeBuildModFormat()
{
    bool prompt = getAct("removeBuildModFormatAct.1")->isChecked();
    Preferences::removeBuildModFormatPreference(prompt);
}

void Gui::removeChildSubmodelFormat()
{
    bool prompt = getAct("removeChildSubmodelFormatAct.1")->isChecked();
    Preferences::removeChildSubmodelFormatPreference(prompt);
}

void Gui::removeLPubFormatting()
{
    int option = RLPF_DOCUMENT;
    int saveDisplayPageNum = displayPageNum;
    displayPageNum = 1 + pa;
    if (sender() == getAct("removeLPubFormatSubmodelAct.1")) {
        option = RLPF_SUBMODEL;
    } else if (sender() == getAct("removeLPubFormatPageAct.1")) {
        option = RLPF_PAGE;
        displayPageNum = saveDisplayPageNum;
    } else if (sender() == getAct("removeLPubFormatStepAct.1")) {
        option = RLPF_STEP;
        displayPageNum = saveDisplayPageNum;
    } else if (sender() == getAct("removeLPubFormatBomAct.1")) {
        option = RLPF_BOM;
    }

    lpub->removeLPubFormatting(option);
}

#ifndef QT_NO_CLIPBOARD
void Gui::updateClipboard()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        bool isImage = false;
        QString data;
        if (action == getAct("copyFilePathToClipboardAct.1")) {
            data = QDir::toNativeSeparators(getCurFile());
        } else {
            isImage = true;
            data = action->data().toString();
        }

        if (data.isEmpty()) {
            emit messageSig(LOG_ERROR, QString("Copy to clipboard - Sender: %1, No data detected")
                                               .arg(sender()->metaObject()->className()));
            return;
        }

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(data, QClipboard::Clipboard);

        if (clipboard->supportsSelection())
            clipboard->setText(data, QClipboard::Selection);

#if defined(Q_OS_LINUX)
        QThread::msleep(1); //workaround for copied text not being available...
#endif

        QString efn =QFileInfo(data).fileName();
        // Text elided to 20 chars
        QString fileName = QString("%1 '%2' full path")
                           .arg(isImage ? "Image" : "File")
                           .arg(efn.size() > 20 ?
                                efn.left(17) + "..." +
                                efn.right(3) : efn);

        emit messageSig(LOG_INFO_STATUS, QString("'%1' copied to clipboard.").arg(fileName));
    }
}
#endif

void Gui::displayPage()
{
  if (macroNesting == 0) {
    pageProcessRunning = PROC_DISPLAY_PAGE;
    emit messageSig(LOG_STATUS, "Display page...");
    timer.start();
    bool updateViewer = lpub->currentStep ? lpub->currentStep->updateViewer : true;
    clearPage(KpageView,KpageScene); // this includes freeSteps() so harvest old step items before calling
    drawPage(KpageView,KpageScene,false/*printing*/,updateViewer,false/*buildMod*/);
    pageProcessRunning = PROC_NONE;
  }
}

void Gui::cyclePageDisplay(const int inputPageNum, bool silent/*true*/, bool fileReload/*false*/)
{
  PageDirection operation = FILE_DEFAULT;

  if (!silent || fileReload) {
    // if dialog or fileReload is true, cycleEachPage = FILE_RELOAD (1), else cycleEachPage = FILE_DEFAULT(0) do not cycle
    bool cycleEachPage = Preferences::cycleEachPage || fileReload;
    if (!cycleEachPage && displayPageNum > 1)
        cycleEachPage = LocalDialog::getLocal(VER_PRODUCTNAME_STR, tr("Cycle each page on model file reload ?"), nullptr);
    operation = PageDirection(cycleEachPage);
  }

  int move = 0;
  int goToPageNum = inputPageNum;

  auto setDirection = [this, &goToPageNum] (int &move)
  {
    move = goToPageNum - displayPageNum;
    if (move > 1)
      pageDirection = PAGE_JUMP_FORWARD;
    else if (move == 1)
      pageDirection = PAGE_NEXT;
    else if (move < -1)
      pageDirection = PAGE_JUMP_BACKWARD;
    else if (move == -1)
      pageDirection = PAGE_PREVIOUS;
  };

  auto cycleDisplay = [this, &goToPageNum] ()
  {
    setContinuousPage(displayPageNum);
    if (pageDirection < PAGE_BACKWARD)
      while (displayPageNum < goToPageNum) {
        ++displayPageNum;
        displayPage();
        secSleeper::secSleep(PAGE_CYCLE_DISPLAY_DEFAULT);
      }
    else
      while (displayPageNum > goToPageNum) {
        --displayPageNum;
        displayPage();
        secSleeper::secSleep(PAGE_CYCLE_DISPLAY_DEFAULT);
      }
    cancelContinuousPage();
  };

  if (operation == FILE_RELOAD) {
    int savePage = displayPageNum;
    if (openFile(curFile)) {
      goToPageNum = pa ? savePage + pa : savePage;
      displayPageNum = 1 + pa;
      setDirection(move);
      if (move > 1) {
        cycleDisplay();
      } else {
        displayPageNum = goToPageNum;
        displayPage();
      }
      enableActions();
    }
  } else {
    setDirection(move);
    if (move > 1 && Preferences::cycleEachPage) {
      cycleDisplay();
    } else {
      displayPageNum = goToPageNum;
      displayPage();
    }
  }
}

void Gui::cycleEachPage()
{
    bool cycleEachPageCompare  = Preferences::cycleEachPage;
    Preferences::cycleEachPage = getAct("cycleEachPageAct.1")->isChecked();
    bool cycleEachPageChanged  = Preferences::cycleEachPage  != cycleEachPageCompare;

    if (cycleEachPageChanged) {
        QSettings Settings;
        QString const cycleEachPageKey("CycleEachPage");
        Settings.setValue(QString("%1/%2").arg(SETTINGS,cycleEachPageKey), Preferences::cycleEachPage);
        emit messageSig(LOG_INFO,QString("Cycle each page step(s) when navigating forward by more than one page is %1").arg(Preferences::cycleEachPage? "ON" : "OFF"));
    }
}

void Gui::enableNavigationActions(bool enable)
{
  setPageLineEdit->setEnabled(enable);
  setGoToPageCombo->setEnabled(setGoToPageCombo->count() && enable);
  mpdCombo->setEnabled(mpdCombo->count() && enable);

  bool atStart = enable && (displayPageNum == (1 + pa));
  bool atEnd = enable && (displayPageNum == maxPages);

  getAct("firstPageAct.1")->setEnabled(!atStart);

  getAct("lastPageAct.1")->setEnabled(!atEnd);

  getAct("nextPageAct.1")->setEnabled(!atEnd);
  getAct("previousPageAct.1")->setEnabled(!atStart);

  getAct("nextPageComboAct.1")->setEnabled(!atEnd);
  getAct("previousPageComboAct.1")->setEnabled(!atStart);

  getAct("nextPageContinuousAct.1")->setEnabled(!atEnd);
  getAct("previousPageContinuousAct.1")->setEnabled(!atStart);
}

void Gui::nextPage()
{
  QString string = setPageLineEdit->displayText();
  QRegExp rx("^(\\d+).*$");
  if (string.contains(rx)) {
    bool ok;
    int inputPageNum = rx.cap(1).toInt(&ok);
    if (ok && (inputPageNum != displayPageNum)) {		// numbers are different so jump to page
      countPages();
      if (inputPageNum <= maxPages && inputPageNum != displayPageNum) {
        if (!saveBuildModification())
          return;
        cyclePageDisplay(inputPageNum);
        return;
      } else {
        statusBarMsg("Page number entered is higher than total pages");
      }
      string = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);
      setPageLineEdit->setText(string);
      return;
  } else {						// numbers are same so goto next page
      countPages();
      if (displayPageNum < maxPages) {
        if (!saveBuildModification())
          return;
        pageDirection = PAGE_NEXT;
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
      if (inputPageNum >= 1 + pa && inputPageNum != displayPageNum) {
        if (!saveBuildModification())
          return;
        const int move = inputPageNum - displayPageNum;
        pageDirection  = move < -1 ? PAGE_JUMP_BACKWARD : PAGE_PREVIOUS;
        displayPageNum = inputPageNum;
        displayPage();
        return;
      } else {
        statusBarMsg("Page number entered is invalid");
      }
      string = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);
      setPageLineEdit->setText(string);
      return;
    } else {						// numbers are same so goto previous page
      if (displayPageNum > 1 + pa) {
        if (!saveBuildModification())
          return;
        pageDirection = PAGE_PREVIOUS;
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

void Gui::setPageContinuousIsRunning(bool b, PageDirection d){

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
            getAct("nextPageContinuousAct.1")->setIcon(QIcon(":/resources/nextpagecontinuousstop.png"));
            getAct("nextPageContinuousAct.1")->setStatusTip("Stop continuous next page processing - Ctrl+Shift+E");
            getAct("nextPageContinuousAct.1")->setText("Stop Continuous Next Page");
        } else {
            getAct("previousPageContinuousAct.1")->setIcon(QIcon(":/resources/prevpagecontinuousstop.png"));
            getAct("previousPageContinuousAct.1")->setStatusTip("Stop continuous previous page processing - Ctrl+Shift+E");
            getAct("previousPageContinuousAct.1")->setText("Stop Continuous Previous Page");
        }
    } else { // SET_DEFAULT_ACTION
        if (pageDirection == PAGE_NEXT) {
            getAct("nextPageContinuousAct.1")->setIcon(QIcon(":/resources/nextpagecontinuous.png"));
            getAct("nextPageContinuousAct.1")->setStatusTip(tr("Continuously process next document page - Ctrl+Shift+N"));
            getAct("nextPageContinuousAct.1")->setText("Continuous Next Page");
            setPageContinuousIsRunning(false);
        } else {
            getAct("previousPageContinuousAct.1")->setIcon(QIcon(":/resources/prevpagecontinuous.png"));
            getAct("previousPageContinuousAct.1")->setStatusTip(tr("Continuously process previous document page - Ctrl+Shift+E"));
            getAct("previousPageContinuousAct.1")->setText("Continuous Previous Page");
            setPageContinuousIsRunning(false);
        }
    }
}

bool Gui::continuousPageDialog(PageDirection d)
{
  pageDirection = d;
  int pageCount = 0;
  int _maxPages = 0;
  bool terminateProcess = false;
  setContinuousPageSig(true);
  QElapsedTimer continuousTimer;
  QString direction = d == PAGE_NEXT ? "Next" : "Previous";

  m_exportMode = PAGE_PROCESS;

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
                  if (dialog->allPagesRange()) {
                      processOption = EXPORT_PAGE_RANGE;
                      pageRangeText = dialog->allPagesRangeText();
                  } else {
                      processOption = EXPORT_ALL_PAGES;
                  }
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

          QApplication::setOverrideCursor(Qt::ArrowCursor);
      }

      int progress = 0;

      for (d == PAGE_NEXT ? displayPageNum = 1 + pa : displayPageNum = maxPages ; d == PAGE_NEXT ? displayPageNum <= maxPages : displayPageNum >= 1 + pa ; d == PAGE_NEXT ? displayPageNum++ : displayPageNum--) {

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
              setContinuousPageAct(SET_DEFAULT_ACTION);
              emit setContinuousPageSig(false);
              if (Preferences::modeGUI) {
                QApplication::restoreOverrideCursor();
                m_progressDialog->setBtnToClose();
                m_progressDlgMessageLbl->setText(tr("%1").arg(message));
                disconnect (m_progressDialog, SIGNAL (cancelClicked()), this, SLOT (cancelContinuousPage()));
                connect (   m_progressDialog, SIGNAL (cancelClicked()), this, SLOT (cancelExporting()));
              }
              return false;
          }

          pageCount = displayPageNum;

          displayPage();

          message = QString("%1 Page Processing - Processed page %2 of %3.").arg(direction).arg(displayPageNum).arg(maxPages);
          emit messageSig(LOG_STATUS,message);

          if (Preferences::modeGUI) {
              enableNavigationActions(true);
              m_progressDlgProgressBar->setValue(d == PAGE_NEXT ? displayPageNum : ++progress);
              m_progressDlgMessageLbl->setText(tr("%1").arg(message));
              qApp->processEvents();
              secSleeper::secSleep(Preferences::pageDisplayPause);
          }
      }

      if (Preferences::modeGUI)
          QApplication::restoreOverrideCursor();

  }
  else
  // Processing page range
  if (processOption == EXPORT_PAGE_RANGE) {

      QStringList pageRanges = pageRangeText.split(",");
      QList<int> printPages;
      // Split page range values
      Q_FOREACH (QString ranges,pageRanges) {
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
          QApplication::setOverrideCursor(Qt::BusyCursor);
      }

      // process each page
      Q_FOREACH (int printPage,printPages){

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
              setContinuousPageAct(SET_DEFAULT_ACTION);
              emit setContinuousPageSig(false);
              if (Preferences::modeGUI) {
                  QApplication::restoreOverrideCursor();
                  m_progressDialog->setBtnToClose();
                  m_progressDlgMessageLbl->setText(tr("%1").arg(message));
                  disconnect (m_progressDialog, SIGNAL (cancelClicked()), this, SLOT (cancelContinuousPage()));
                  connect (   m_progressDialog, SIGNAL (cancelClicked()), this, SLOT (cancelExporting()));
              }
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
              enableNavigationActions(true);
              m_progressDlgProgressBar->setValue(pageCount);
              m_progressDlgMessageLbl->setText(tr("%1").arg(message));
              qApp->processEvents();
              secSleeper::secSleep(Preferences::pageDisplayPause);
          }
      }

      if (Preferences::modeGUI) {
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
  }

  emit setContinuousPageSig(false);

  return true;
}

bool Gui::processPageRange(const QString &range)
{
  if (!range.isEmpty()) {
      int rangeMin, rangeMax;
      bool ok[2] = {false, false};
      QRegExp rx("^(\\d+)(?:[^0-9]*|[a-zA-Z\\s]*)(\\d+)?$", Qt::CaseInsensitive);
      if (range.trimmed().contains(rx)) {
          rangeMin = rx.cap(1).toInt(&ok[0]);
          rangeMax = rx.cap(2).toInt(&ok[1]);
          if (rangeMin > rangeMax)
              pageDirection = PAGE_BACKWARD;
      }

      if (pageDirection < PAGE_BACKWARD) {
          if (ok[0] && rangeMin > displayPageNum && rangeMax == 0) {
              rangeMax = rangeMin;
              rangeMin = displayPageNum;
              ok[1]    = true;
          }
      } else {
          if (ok[0] && rangeMin < displayPageNum && rangeMax == 0) {
              rangeMax = displayPageNum;
              ok[1]    = true;
          }
      }

      const QString lineAllPages = QString("%1-%2").arg(1 + pa).arg(maxPages);
      const QString linePageRange = QString("%1-%2").arg(rangeMax).arg(rangeMin);

      if (lineAllPages == linePageRange) {
          processOption = EXPORT_ALL_PAGES;
      } else {
          processOption = EXPORT_PAGE_RANGE;
          pageRangeText = range;
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
  if (displayPageNum == 1 + pa) {
    statusBarMsg("You are on the first page");
  } else {
    if (!saveBuildModification())
        return;
    if ((1 + pa) - displayPageNum < -1)
        pageDirection = PAGE_JUMP_BACKWARD;
    else if ((1 + pa) - displayPageNum == -1)
        pageDirection = PAGE_PREVIOUS;
    displayPageNum = 1 + pa;
    displayPage();
  }
}

void Gui::lastPage()
{
  if (displayPageNum == maxPages) {
    statusBarMsg("You are on the last page");
  } else {
    countPages();
    if (!saveBuildModification())
      return;
    cyclePageDisplay(maxPages);
  }
}

void Gui::setPage()
{
  QString string = setPageLineEdit->displayText();
  QRegExp rx("^(\\d+).*$");
  if (string.contains(rx)) {
    bool ok;
    const int inputPageNum = rx.cap(1).toInt(&ok);
    if (ok) {
      countPages();
      if (inputPageNum <= maxPages && inputPageNum != displayPageNum) {
        if (!saveBuildModification())
          return;
        cyclePageDisplay(inputPageNum);
        return;
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
  const int goToPageNum = index+1;
  countPages();
  if (goToPageNum <= maxPages && goToPageNum != displayPageNum) {
    if (!saveBuildModification())
      return;
    cyclePageDisplay(goToPageNum);
  }

  QString string = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);
  setPageLineEdit->setText(string);
}

void Gui::pageLineEditReset()
{
    if (setPageLineEdit) {
        getAct("setPageLineEditResetAct.1")->setEnabled(false);
        setPageLineEdit->setText(QString("%1 of %2").arg(displayPageNum).arg(maxPages));
    }
}

void Gui::enablePageLineReset(const QString &displayText)
{
    if (setPageLineEdit)
        getAct("setPageLineEditResetAct.1")->setEnabled(displayText != QString("%1 of %2").arg(displayPageNum).arg(maxPages));
}

void Gui::fitWidth()
{
  QRectF rect(0,0,lpub->pageSize(lpub->page.meta.LPub.page, 0)
                 ,lpub->pageSize(lpub->page.meta.LPub.page, 1));
  KpageView->fitWidth(rect);
}

void Gui::fitVisible()
{
  QRectF rect(0,0,lpub->pageSize(lpub->page.meta.LPub.page, 0),
                  lpub->pageSize(lpub->page.meta.LPub.page, 1));
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
  Preferences::setSceneGuidesPreference(getAct("sceneGuidesComboAct.1")->isChecked());
  SceneGuidesLineGroup->setEnabled(Preferences::sceneGuides);
  SceneGuidesPosGroup->setEnabled(Preferences::sceneGuides);
  getAct("showGuidesCoordinatesAct.1")->setEnabled(Preferences::sceneGuides);
  KpageView->setSceneGuides();
}

void Gui::sceneGuidesLine()
{
  int line = int(Qt::DashLine);
  if (getAct("sceneGuidesSolidLineAct.1")->isChecked())
      line = int(Qt::SolidLine);

  Preferences::setSceneGuidesLinePreference(line);
  KpageView->setSceneGuidesLine();
}

void Gui::sceneGuidesPosition()
{
    int position = int(GUIDES_TOP_LEFT);
    if (getAct("sceneGuidesPosTRightAct.1")->isChecked())
        position = int(GUIDES_TOP_RIGHT);
    else
    if (getAct("sceneGuidesPosBLeftAct.1")->isChecked())
        position = int(GUIDES_BOT_LEFT);
    else
    if (getAct("sceneGuidesPosBRightAct.1")->isChecked())
        position = int(GUIDES_BOT_RIGHT);
    else
    if (getAct("sceneGuidesPosCentreAct.1")->isChecked())
        position = int(GUIDES_CENTRE);

    Preferences::setSceneGuidesPositionPreference(position);
    KpageView->setSceneGuidesPos();
}

void Gui::sceneRuler()
{
  Preferences::setSceneRulerPreference(getAct("sceneRulerComboAct.1")->isChecked());
  getAct("hideRulerPageBackgroundAct.1")->setEnabled(Preferences::sceneRuler);
  SceneRulerGroup->setEnabled(Preferences::sceneRuler);
  KpageView->setSceneRuler();
}

void Gui::sceneRulerTracking()
{
  RulerTrackingType trackingStyle = TRACKING_NONE;
  if (getAct("sceneRulerTrackingTickAct.1")->isChecked())
      trackingStyle = TRACKING_TICK;
  else
  if (getAct("sceneRulerTrackingLineAct.1")->isChecked()) {
      trackingStyle = TRACKING_LINE;
      getAct("showTrackingCoordinatesAct.1")->setEnabled(true);
  }

  Preferences::setSceneRulerTrackingPreference(int(trackingStyle));
  KpageView->setSceneRulerTracking();
}

void Gui::snapToGrid()
{
  bool checked = getAct("snapToGridComboAct.1")->isChecked();
  if (Preferences::snapToGrid == checked)
      return;

  Preferences::setSnapToGridPreference(checked);
  getAct("hideGridPageBackgroundAct.1")->setEnabled(Preferences::snapToGrid);
  GridStepSizeGroup->setEnabled(Preferences::snapToGrid);
  KpageView->setSnapToGrid();
  reloadCurrentPage();
}

void Gui::hidePageBackground()
{
  bool checked;
  if (sender() == getAct("hideRulerPageBackgroundAct.1")) {
      checked = getAct("hideRulerPageBackgroundAct.1")->isChecked();
      getAct("hideGridPageBackgroundAct.1")->setChecked(checked);
  }
  else
  {
      checked = getAct("hideGridPageBackgroundAct.1")->isChecked();
      getAct("hideRulerPageBackgroundAct.1")->setChecked(checked);
  }
  Preferences::setHidePageBackgroundPreference(checked);
  reloadCurrentPage();
}

void Gui::showCoordinates()
{
  if (sender() == getAct("showTrackingCoordinatesAct.1")) {
      Preferences::setShowTrackingCoordinatesPreference(
                  getAct("showTrackingCoordinatesAct.1")->isChecked());
  }
  else
  if (sender() == getAct("showGuidesCoordinatesAct.1"))
  {
      Preferences::setShowGuidesCoordinatesPreference(
                  getAct("showGuidesCoordinatesAct.1")->isChecked());
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
          LDrawFile *ldrawFile,
    const Where     &here,
          bool       editModelFile   /*false*/,
          bool       displayStartPage/*false*/)
{
    if (! exporting()) {
#ifdef QT_DEBUG_MODE
        QElapsedTimer t;
        t.start();
#endif        
        const QString &modelName = here.modelName;
        if (editModelFile) {

            emit displayModelFileSig(ldrawFile, modelName);

        } else {

            if (!displayStartPage) {
                const QString loadedFile = editWindow->getCurrentFile().toLower();
                if( loadedFile == modelName.toLower()) {
                    if (!ldrawFile->modified(modelName))
                        return;
                }
            }

            // limit the scope to the current page
            const Where top = lpub->page.top;
            Where bottom = lpub->page.bottom;
            if (!bottom.lineNumber) {
                bool multiStep = false;
                if (lpub->currentStep)
                    multiStep = lpub->currentStep->multiStep;
                lpub->mi.scanForward(bottom, multiStep ? StepGroupEndMask : StepMask);
            }

            const StepLines lineScope(top.lineNumber, bottom.lineNumber);

            emit displayFileSig(ldrawFile, modelName, lineScope);

#ifdef QT_DEBUG_MODE
                emit messageSig(LOG_DEBUG,tr("Editor loaded page: %1, step: %2, model: %3, line scope: %4-%5 - %6")
                                .arg(displayPageNum)
                                .arg(lpub->currentStep ? lpub->currentStep->stepNumber.number : 0)
                                .arg(modelName)
                                .arg(top.lineNumber + 1    /*adjust for 0-index*/)
                                .arg(bottom.lineNumber + 1 /*adjust for 0-index*/)
                                .arg(elapsedTime(t.elapsed())));
#endif

            if (displayStartPage) {
               int inputPageNum = ldrawFile->getModelStartPageNumber(modelName);
               if (!inputPageNum) {
                   countPages();
                   inputPageNum = ldrawFile->getModelStartPageNumber(modelName);
               }
               if (inputPageNum && displayPageNum != inputPageNum)
                   cyclePageDisplay(inputPageNum);
            }

            if (curSubFile == modelName)
                return;   // work is done here.
            else
                curSubFile = modelName;

            const int currentIndex = mpdCombo->currentIndex();
            for (int i = 0; i < mpdCombo->count(); i++) {
                if (mpdCombo->itemText(i) == modelName) { // will never equal Include File
                    if (i != currentIndex) {
                        mpdCombo->setCurrentIndex(i);
                        mpdCombo->setToolTip(tr("Current Submodel: %1").arg(mpdCombo->currentText()));
                    }
                    break;
                }
            }

            ldrawFile->setModified(modelName, false);
        }
    } // ! exporting
}

void Gui::displayParmsFile(
  const QString &fileName)
{
    emit displayParmsFileSig(fileName);
}

void Gui::refreshModelFile()
{
     editModelFile(/*saveBefore*/false);
}

void Gui::editModelFile()
{
    editModelFile(/*saveBefore*/true, sender() == editWindow);
}

void Gui::editModelFile(bool saveBefore, bool subModel)
{
    if (getCurFile().isEmpty())
        return;
    if (saveBefore)
        save();
    QString file = getCurFile();
    if (lpub->ldrawFile.isIncludeFile(curSubFile))
        file = curSubFile;
    else if (subModel) {
        file = editWindow->getCurrentFile();
        writeToTmp(file, lpub->ldrawFile.contents(file));
    }
    editModeWindow->setWindowTitle(tr("Detached LDraw Editor - Edit %1").arg(QFileInfo(file).fileName()));
    displayFile(&lpub->ldrawFile, Where(file, 0), true/*editModelFile*/);
    editModeWindow->show();
    while (!editModeWindow->contentLoading())
        QApplication::processEvents();
}


void Gui::deployBanner(bool b)
{
    if (b && Preferences::modeGUI) {
        QTemporaryDir tempDir;
        if (tempDir.isValid())
            loadBanner(Gui::m_exportMode, QDir::toNativeSeparators(QString("%1/banner.ldr").arg(tempDir.path())));
    }
}

/*-----------------------------------------------------------------------------*/

void Gui::mpdComboChanged(int index)
{

  Q_UNUSED(index)

  QString newSubFile = mpdCombo->currentText();

  bool isIncludeFile = false;
  if (newSubFile.endsWith("Include File")) {
      newSubFile = mpdCombo->currentData().toString();
      isIncludeFile = lpub->ldrawFile.isIncludeFile(newSubFile);
  }

  if (curSubFile != newSubFile) {

    bool callDisplayFile = isIncludeFile;

    if (!callDisplayFile) {
      const int modelPageNum = lpub->ldrawFile.getModelStartPageNumber(newSubFile);
      countPages();
      if (modelPageNum && displayPageNum != modelPageNum) {
        if (!saveBuildModification())
          return;
        messageSig(LOG_INFO, QString( "Select subModel: %1 @ Page: %2").arg(newSubFile).arg(modelPageNum));
        cyclePageDisplay(modelPageNum);
      } else {
        callDisplayFile = true;
      }
    }

    if (callDisplayFile) {
      messageSig(LOG_INFO, QString( "Selected %1: %2")
                 .arg(isIncludeFile ? "includeFile" : "subModel").arg(newSubFile));
      displayFile(&lpub->ldrawFile, Where(newSubFile, 0), false/*editModelFile*/, true/*displayStartPage*/);
      showLineSig(0, LINE_HIGHLIGHT);
      if (isIncludeFile) {  // Combo will not be set to include toolTip, so set here
          mpdCombo->setToolTip(tr("Include file: %1").arg(newSubFile));
      }
    }
  }
}

void  Gui::restartApplication(bool changeLibrary, bool prompt) {
    if (prompt && QMessageBox::question(this, tr(VER_PRODUCTNAME_STR),
                                        tr("%1 must restart. Do you want to continue ?")
                                        .arg(VER_PRODUCTNAME_STR),
                                        QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }
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
    messageSig(LOG_INFO, QString("Restarted %1 with Command: %2 %3")
                                 .arg(VER_PRODUCTNAME_STR).arg(QApplication::applicationFilePath()).arg(args.join(" ")));
    QCoreApplication::quit();
}

void Gui::ldrawSearchDirectories()
{
    LDrawSearchDirDialog dialog;
    dialog.getLDrawSearchDirDialog();
}

void Gui::insertConfiguredSubFile(const QString &name,
                                  QStringList &content) {
    QString subFilePath = QDir::toNativeSeparators(QDir::currentPath() + "/" + Paths::tmpDir + "/" + name);
    lpub->ldrawFile.insertConfiguredSubFile(name,content,subFilePath);
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

    // displayPage();
    cyclePageDisplay(displayPageNum, false/*silent*/);

    emit messageSig(LOG_STATUS, QString("Page %1 reloaded. %2")
                    .arg(displayPageNum)
                    .arg(elapsedTime(timer.elapsed())));

}

void Gui::reloadCurrentModelFile() { // EditModeWindow Update
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

    //reload current model
    cyclePageDisplay(displayPageNum, true/*silent*/, true/*FILE_RELOAD*/);

    emit messageSig(LOG_STATUS, QString("Model file reloaded (%1 parts). %2")
                    .arg(lpub->ldrawFile.getPartCount())
                    .arg(elapsedTime(timer.elapsed())));
}

void Gui::clearWorkingFiles(const QStringList &filePaths)
{
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,"A model must be open to clean its parts cache - no action taken.");
        return;
    }

    int count = 0;
    for (int i = 0; i < filePaths.size(); i++) {
        QFileInfo fileInfo(filePaths.at(i));
        QFile     file(filePaths.at(i));
        if (file.exists()) {
            if (!file.remove()) {
                emit messageSig(LOG_ERROR,QString("Unable to remove %1")
                                .arg(fileInfo.absoluteFilePath()));
            } else {
#ifdef QT_DEBUG_MODE
                emit messageSig(LOG_TRACE,QString("-File %1 removed").arg(fileInfo.completeBaseName()));
#endif
                count++;
            }
        }
    }
    if (count)
        emit messageSig(LOG_INFO,QString("Parts content cache cleaned. %1 %2 removed.").arg(count).arg(count == 1 ? "item" : "items" ));
}

void Gui::resetModelCache(QString file, bool commandLine)
{
    if (Gui::resetCache && !file.isEmpty()) {
        if (commandLine)
            timer.start();

        emit messageSig(LOG_TRACE, QString("Reset parts cache is destructive!"));
        curFile = file;
        QString fileDir = QFileInfo(file).absolutePath();
        emit messageSig(LOG_INFO, QString("Reset parts cache directory %1").arg(fileDir));
        QString saveCurrentDir = QDir::currentPath();
        if (! QDir::setCurrent(fileDir))
            emit messageSig(LOG_ERROR, QString("Reset cache failed to set current directory %1").arg(fileDir));

        clearCustomPartCache(true);
        if (commandLine) {
            clearPLICache();
            clearCSICache();
            clearSubmodelCache();
            clearTempCache();
        } else {
            clearAllCaches();
        }

        if (! QDir::setCurrent(saveCurrentDir))
            emit messageSig(LOG_ERROR, QString("Reset cache failed to restore current directory %1").arg(saveCurrentDir));

        Gui::resetCache = false;

        if (commandLine)
            emit messageSig(LOG_INFO, QString("All caches reset. %1").arg(elapsedTime(timer.elapsed())));
    }
}

void Gui::clearAndRedrawModelFile() { //EditModeWindow Redraw

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
        lpub->ldrawFile.clearPrevStepPositions();
    }

    clearPLICache();
    clearCSICache();
    clearSubmodelCache();
    clearTempCache();

    //reload current model
    cyclePageDisplay(displayPageNum, true/*silent*/, true/*FILE_RELOAD*/);

    emit messageSig(LOG_STATUS, QString("All caches reset and model file reloaded (%1 parts). %2")
                    .arg(lpub->ldrawFile.getPartCount())
                    .arg(elapsedTime(timer.elapsed())));

    changeAccepted = saveChange;
}

void Gui::clearAndReloadModelFile(bool global) { // EditWindow Redraw
    if (sender() == editWindow || global) {
        bool _continue;
        if (Preferences::saveOnRedraw) {
            _continue = maybeSave(false); // No prompt
        } else {
            _continue = maybeSave(true, editWindow ? SaveOnRedraw : SaveOnNone);
        }
        if (!_continue)
            return;
    }

    if (global) {
        timer.start();
        clearPLICache();
        clearCSICache();
        clearSubmodelCache();
        clearTempCache();
        emit messageSig(LOG_STATUS, QString("All caches reset (%1 parts). %2")
                        .arg(lpub->ldrawFile.getPartCount())
                        .arg(elapsedTime(timer.elapsed())));
    } else
        clearAllCaches();
}

void Gui::clearAllCaches(bool global)
{
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,"A model must be open to reset its caches - no action taken.");
        return;
    }

    bool disableSaveOnRedraw = false;
    if (sender() == getAct("clearAllCachesAct.1") || global) {
        bool _continue;
        if (Preferences::saveOnRedraw) {
            _continue = maybeSave(false); // No prompt
        } else {
            _continue = maybeSave(true);
            if (_continue) {
                // Suppress prompt in the following call
                disableSaveOnRedraw = true;
                Preferences::saveOnRedraw = true;
            }
        }
        if (!_continue)
            return;
    }

    timer.start();

    if (Preferences::enableFadeSteps || Preferences::enableHighlightStep) {
        lpub->ldrawFile.clearPrevStepPositions();
    }

    // if global, skip clearCache here and run in cycleDisPlayPage
    if (!global) {
        timer.start();
        clearPLICache();
        clearCSICache();
        clearSubmodelCache();
        clearTempCache();
        emit messageSig(LOG_STATUS, QString("All caches reset (%1 parts). %2")
                        .arg(lpub->ldrawFile.getPartCount())
                        .arg(elapsedTime(timer.elapsed())));
    }

    if (disableSaveOnRedraw) {
        Preferences::saveOnRedraw = false;
    }

    //reload current model
    cyclePageDisplay(displayPageNum, false/*silent*/, global);

    emit messageSig(LOG_STATUS, QString("All caches reset and model file reloaded (%1 parts). %2")
                                        .arg(lpub->ldrawFile.getPartCount())
                                        .arg(elapsedTime(timer.elapsed())));

}

void Gui::clearCustomPartCache(bool silent)
{
  if (Paths::customDir.isEmpty())
      return;

  if (sender() == getAct("clearCustomPartCacheAct.1")) {
      bool _continue;
      if (Preferences::saveOnRedraw) {
          _continue = maybeSave(false); // No prompt
      } else {
          _continue = maybeSave(true);
      }
      if (!_continue)
          return;
  }

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
                                 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
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
  bool overwrite = true;
  bool setup = false;
  if (Preferences::enableFadeSteps)
      processFadeColourParts(overwrite, setup);       // (re)generate and archive fade parts based on the loaded model file
  if (Preferences::enableHighlightStep)
      processHighlightColourParts(overwrite, setup);  // (re)generate and archive highlight parts based on the loaded model file
  if (!getCurFile().isEmpty() && Preferences::modeGUI) {
      bool cycleEachPage = Preferences::cycleEachPage;
      if (!cycleEachPage && displayPageNum > 1)
        cycleEachPage = LocalDialog::getLocal(VER_PRODUCTNAME_STR, "Cycle each page on model file reload?",nullptr);
      cyclePageDisplay(displayPageNum, PageDirection(cycleEachPage));
  }
}

void Gui::clearPLICache()
{
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,"A model must be open to clean its parts cache - no action taken.");
        return;
    }

    if (sender() == getAct("clearPLICacheAct.1")) {
        bool _continue;
        if (Preferences::saveOnRedraw) {
            _continue = maybeSave(false); // No prompt
        } else {
            _continue = maybeSave(true);
        }
        if (!_continue)
            return;
    }

    QDir dir(QDir::currentPath() + "/" + Paths::partsDir);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();
    int count = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(fileInfo.absoluteFilePath());
        if (file.exists()) {
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
    }
    emit messageSig(LOG_INFO_STATUS,QString("Parts content cache cleaned. %1 %2 removed.").arg(count).arg(count == 1 ? "item" : "items" ));
}

void Gui::clearCSICache()
{
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,"A model must be open to clean its assembly cache - no action taken.");
        return;
    }

    if (sender() == getAct("clearCSICacheAct.1")) {
        bool _continue;
        if (Preferences::saveOnRedraw) {
            _continue = maybeSave(false); // No prompt
        } else {
            _continue = maybeSave(true);
        }
        if (!_continue)
            return;
    }

    QDir dir(QDir::currentPath() + "/" + Paths::assemDir);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();
    int count = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(fileInfo.absoluteFilePath());
        if (file.exists()) {
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
    }

    emit messageSig(LOG_INFO_STATUS,QString("Assembly content cache cleaned. %1 %2 removed.").arg(count).arg(count == 1 ? "item" : "items" ));
}

void Gui::clearSubmodelCache(const QString &key)
{
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,"A model must be open to clean its Submodel cache - no action taken.");
        return;
    }

    if (sender() == getAct("clearSubmodelCacheAct.1")) {
        bool _continue;
        if (Preferences::saveOnRedraw) {
            _continue = maybeSave(false); // No prompt
        } else {
            _continue = maybeSave(true);
        }
        if (!_continue)
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
        if (key.isEmpty() || deleteSpecificFile) {
            if (file.exists()) {
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
    }

    emit messageSig(LOG_INFO_STATUS,QString("Submodel content cache cleaned. %1 %2 removed.").arg(count).arg(count == 1 ? "item" : "items" ));
}

void Gui::clearTempCache()
{
    if (getCurFile().isEmpty()) {
        emit messageSig(LOG_STATUS,QString("A model must be open to clean its 3D cache - no action taken."));
        return;
    }

    if (sender() == getAct("clearTempCacheAct.1")) {
        bool _continue;
        if (Preferences::saveOnRedraw) {
            _continue = maybeSave(false); // No prompt
        } else {
            _continue = maybeSave(true);
        }
        if (!_continue)
            return;
    }

    QDir tmpDir(QDir::currentPath() + "/" + Paths::tmpDir);
    tmpDir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = tmpDir.entryInfoList();
    int count = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(fileInfo.absoluteFilePath());
        if (file.exists()) {
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
    }

    lpub->ldrawFile.tempCacheCleared();

    emit messageSig(LOG_INFO_STATUS,QString("Temporary model file cache cleaned. %1 %2 removed.").arg(count).arg(count == 1 ? "item" : "items" ));
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
    QString ldrName      = tmpDirName + "/csi.ldr";
    QFileInfo fileInfo(pngName);
    QFile file(assemDirName + "/" + fileInfo.fileName());
    if (file.exists()) {
        if (!file.remove())
            emit messageSig(LOG_ERROR,QString("Unable to remove %1")
                            .arg(assemDirName + "/" + fileInfo.fileName()));
#ifdef QT_DEBUG_MODE
        else
            emit messageSig(LOG_TRACE,QString("-File %1 removed").arg(fileInfo.fileName()));
#endif
    }
    if (renderer->useLDViewSCall())
        ldrName = tmpDirName + "/" + fileInfo.completeBaseName() + ".ldr";
    file.setFileName(ldrName);
    if (file.exists()) {
        if (!file.remove())
            emit messageSig(LOG_ERROR,QString("Unable to remove %1").arg(file.fileName()));
#ifdef QT_DEBUG_MODE
        else
            emit messageSig(LOG_TRACE,QString("-File %1 removed").arg(file.fileName()));
#endif
    }
    if (Preferences::enableFadeSteps)
        clearPrevStepPositions();
    cyclePageDisplay(displayPageNum);
}

void Gui::clearPageCache(PlacementType relativeType, Page *page, int option) {
  if (page->list.size()) {
      if (relativeType == SingleStepType) {         // single step page
          Range *range = dynamic_cast<Range *>(page->list[0]);
          if (range->relativeType == RangeType) {
              Step *step = dynamic_cast<Step *>(range->list[0]);
              if (step && step->relativeType == StepType) {
                  clearPageGraphicsItems(step, option);
              } // validate step (StepType) and process...
          } // validate RangeType - to cast step
      } else if (relativeType == StepGroupType) {  // multi-step page
          for (int i = 0; i < page->list.size(); i++){
              Range *range = dynamic_cast<Range *>(page->list[i]);
              for (int j = 0; j < range->list.size(); j++) {
                  if (range->relativeType == RangeType) {
                      Step *step = dynamic_cast<Step *>(range->list[j]);
                      if (step && step->relativeType == StepType) {
                          clearPageGraphicsItems(step, option);
                      } // validate step (StepType) and process...
                  } // validate RangeType - to cast step
              } // for each step within divided group...=>list[AbstractRangeElement]->StepType
          } // for each divided group within page...=>list[AbstractStepsElement]->RangeType
      }
      if (Preferences::enableFadeSteps && !option) {
          clearPrevStepPositions();
      }
      cyclePageDisplay(displayPageNum);
   }
}

/*
 * Clear step image graphics items
 * This function recurses the step's model to clear images and associated model files.
 */
void Gui::clearPageGraphicsItems(Step *step, int option) {
    // Capture ldr and image file names
    QStringList fileNames;
    QString tmpDirName = QDir::currentPath() + "/" + Paths::tmpDir;

    if (option == Options::CSI) {
        if (renderer->useLDViewSCall())
            fileNames << QDir::toNativeSeparators(tmpDirName + "/" + QFileInfo(step->pngName).completeBaseName() + ".ldr");
         else
            fileNames << QDir::toNativeSeparators(tmpDirName + "/csi.ldr");
        fileNames << step->pngName;
    }

    if (option == Options::PLI) {
        if (!renderer->useLDViewSCall())
            fileNames << QDir::toNativeSeparators(tmpDirName + "/pli.ldr");
        QHash<QString, PliPart*> pliParts;
        step->pli.getParts(pliParts);
        if (pliParts.size()) {
            Q_FOREACH (PliPart* part, pliParts) {
                QString key = QString("%1;%2;%3").arg(QFileInfo(part->type).completeBaseName()).arg(part->color).arg(step->stepNumber.number);
                if (lpub->ldrawFile.viewerStepContentExist(key)) {
                    if (renderer->useLDViewSCall())
                        fileNames << QDir::toNativeSeparators(gui->getViewerStepFilePath(key));
                    fileNames << QDir::toNativeSeparators(gui->getViewerStepImagePath(key));
                }
            }
        }
    }

    QFile file;
    // process ldr and image files
    Q_FOREACH (QString fileName, fileNames) {
        file.setFileName(fileName);
        if (file.exists()) {
            if (!file.remove())
                emit messageSig(LOG_ERROR,QString("Unable to remove %1").arg(file.fileName()));
#ifdef QT_DEBUG_MODE
            else
                emit messageSig(LOG_TRACE,QString("-File %1 removed").arg(file.fileName()));
#endif
        }
    }

    // process callout ldr and image files
    for (int k = 0; k < step->list.size(); k++) {
        if (step->list[k]->relativeType == CalloutType) {
            Callout *callout = dynamic_cast<Callout *>(step->list[k]);
            for (int l = 0; l < callout->list.size(); l++){
                Range *range = dynamic_cast<Range *>(callout->list[l]);
                for (int m = 0; m < range->list.size(); m++){
                    if (range->relativeType == RangeType) {
                        Step *step = dynamic_cast<Step *>(range->list[m]);
                        if (step && step->relativeType == StepType) {
                            clearPageGraphicsItems(step, option);
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
  GlobalPageDialog::getPageGlobals(lpub->ldrawFile.topLevelFile(),lpub->page.meta);
}

void Gui::assemSetup()
{
  GlobalAssemDialog::getAssemGlobals(lpub->ldrawFile.topLevelFile(),lpub->page.meta);
}

void Gui::pliSetup()
{
  GlobalPliDialog::getPliGlobals(lpub->ldrawFile.topLevelFile(),lpub->page.meta);
}

void Gui::bomSetup()
{
  GlobalPliDialog::getBomGlobals(lpub->ldrawFile.topLevelFile(),lpub->page.meta);
}

void Gui::calloutSetup()
{
  GlobalCalloutDialog::getCalloutGlobals(lpub->ldrawFile.topLevelFile(),lpub->page.meta);
}

void Gui::multiStepSetup()
{
  GlobalMultiStepDialog::getMultiStepGlobals(lpub->ldrawFile.topLevelFile(),lpub->page.meta);
}

void Gui::subModelSetup()
{
  GlobalSubModelDialog::getSubModelGlobals(lpub->ldrawFile.topLevelFile(),lpub->page.meta);
}

void Gui::projectSetup()
{
  GlobalProjectDialog::getProjectGlobals(lpub->ldrawFile.topLevelFile(),lpub->page.meta);
}

void Gui::fadeStepSetup()
{
  GlobalFadeStepDialog::getFadeStepGlobals(lpub->ldrawFile.topLevelFile(),lpub->page.meta);
}

void Gui::highlightStepSetup()
{
  GlobalHighlightStepDialog::getHighlightStepGlobals(lpub->ldrawFile.topLevelFile(),lpub->page.meta);
}

void Gui::useSystemEditor()
{
   bool useSystemEditor = getAct("useSystemEditorAct.1")->isChecked();
   Preferences::useSystemEditorPreference(useSystemEditor);
   getAct("editModelFileAct.1")->setStatusTip(tr("Edit current model file with %1")
                                  .arg(useSystemEditor ? Preferences::systemEditor.isEmpty() ? "the system editor" :
                                                                                               Preferences::systemEditor : "detached LDraw Editor"));
}

/*********************************************
 *
 * Parameter edit section
 *
 *********************************************/
void Gui::editLDrawColourParts()
{
    QFileInfo fileInfo(Preferences::ldrawColourPartsFile);
    if (!fileInfo.exists()) {
        emit messageSig(LOG_ERROR, QString("Static colour part list does not exist. Generate from the Configuration menu."));
    } else {
        if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
            if (Preferences::systemEditor.isEmpty())

                QDesktopServices::openUrl(QUrl("file:///"+Preferences::ldrawColourPartsFile, QUrl::TolerantMode));
            else
#endif
                openWith(Preferences::ldrawColourPartsFile);
        } else {
            displayParmsFile(Preferences::ldrawColourPartsFile);
            parmsWindow->setWindowTitle(tr("LDraw Color Parts","Edit/add LDraw static color parts"));
            parmsWindow->show();
        }
    }
}

void Gui::editPliControlFile()
{
    QFileInfo fileInfo(Preferences::pliControlFile);
    if (!fileInfo.exists()) {
        emit messageSig(LOG_ERROR, QString("PLI control file does not exist."));
    } else {
        if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
            if (Preferences::systemEditor.isEmpty())
                QDesktopServices::openUrl(QUrl("file:///"+Preferences::pliControlFile, QUrl::TolerantMode));
            else
#endif
                openWith(Preferences::pliControlFile);
        } else {
            displayParmsFile(Preferences::pliControlFile);
            parmsWindow->setWindowTitle(tr("PLI Control","Edit/add PLI control part entries"));
            parmsWindow->show();
        }
    }
}

void Gui::editAnnotationStyle()
{
    if (Preferences::annotationStyleFile.isEmpty())
        Preferences::annotationStyleFile = QString("%1/extras/%2")
            .arg( Preferences::lpubDataPath, Preferences::validAnnotationStyles);
    QFileInfo fileInfo(Preferences::stickerPartsFile);
    if (!fileInfo.exists()) {
        if (!Annotations::exportAnnotationStyleFile()) {
            emit messageSig(LOG_ERROR, QString("Failed to export %1.").arg(Preferences::annotationStyleFile));
            return;
        }
    }

    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+fileInfo.absoluteFilePath(), QUrl::TolerantMode));
        else
#endif
            openWith(fileInfo.absoluteFilePath());
    } else {
        displayParmsFile(fileInfo.absoluteFilePath());
        parmsWindow->setWindowTitle(tr("Part Annotation Style reference",
                                       "Edit/add Part Annotation Style reference"));
        parmsWindow->show();
    }
}

void Gui::editTitleAnnotations()
{
    if (Preferences::titleAnnotationsFile.isEmpty())
        Preferences::titleAnnotationsFile = QString("%1/extras/%2")
            .arg( Preferences::lpubDataPath, Preferences::validTitleAnnotations);
    QFileInfo fileInfo(Preferences::titleAnnotationsFile);
    if (!fileInfo.exists()) {
        if (!Annotations::exportTitleAnnotationsFile()) {
            emit messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
            return;
        }
    }

    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+Preferences::titleAnnotationsFile, QUrl::TolerantMode));
        else
#endif
            openWith(Preferences::titleAnnotationsFile);
    } else {
        displayParmsFile(Preferences::titleAnnotationsFile);
        parmsWindow->setWindowTitle(tr("Part Title Annotation","Edit/add part title part annotations"));
        parmsWindow->show();
    }
}

void Gui::editFreeFormAnnitations()
{
    if (Preferences::freeformAnnotationsFile.isEmpty())
        Preferences::freeformAnnotationsFile = QString("%1/extras/%2")
            .arg( Preferences::lpubDataPath, Preferences::validFreeFormAnnotations);
    QFileInfo fileInfo(Preferences::freeformAnnotationsFile);
    if (!fileInfo.exists()) {
        if (!Annotations::exportfreeformAnnotationsHeader()) {
            emit messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
            return;
        }
    }

    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+Preferences::freeformAnnotationsFile, QUrl::TolerantMode));
        else
#endif
            openWith(Preferences::freeformAnnotationsFile);
    } else {
        displayParmsFile(Preferences::freeformAnnotationsFile);
        parmsWindow->setWindowTitle(tr("Freeform Annotation","Edit/add freeform part annotations"));
        parmsWindow->show();
    }
}

void Gui::editPliBomSubstituteParts()
{
    if (Preferences::pliSubstitutePartsFile.isEmpty())
        Preferences::pliSubstitutePartsFile = QString("%1/extras/%2")
            .arg( Preferences::lpubDataPath, Preferences::validPliSubstituteParts);
    QFileInfo fileInfo(Preferences::pliSubstitutePartsFile);
    if (!fileInfo.exists()) {
        if (!PliSubstituteParts::exportSubstitutePartsHeader()) {
            emit messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
            return;
        }
    }

    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+Preferences::pliSubstitutePartsFile, QUrl::TolerantMode));
        else
#endif
            openWith(Preferences::pliSubstitutePartsFile);
    } else {
        displayParmsFile(Preferences::pliSubstitutePartsFile);
        parmsWindow->setWindowTitle(tr("PLI/BOM Substitute Parts","Edit/add PLI/BOM substitute parts"));
        parmsWindow->show();
    }
}

void Gui::editExcludedParts()
{
    if (Preferences::excludedPartsFile.isEmpty())
        Preferences::excludedPartsFile = QString("%1/extras/%2")
            .arg( Preferences::lpubDataPath, Preferences::validExcludedPliParts);
    QFileInfo fileInfo(Preferences::excludedPartsFile);
    if (!fileInfo.exists()) {
        if (!ExcludedParts::exportExcludedParts()) {
            emit messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
            return;
        }
    }

    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+fileInfo.absoluteFilePath(), QUrl::TolerantMode));
        else
#endif
            openWith(fileInfo.absoluteFilePath());
    } else {
        displayParmsFile(fileInfo.absoluteFilePath());
        parmsWindow->setWindowTitle(tr("Parts List Excluded Parts","Edit/add excluded parts"));
        parmsWindow->show();
    }
}

void Gui::editStickerParts()
{
   if (Preferences::stickerPartsFile.isEmpty())
       Preferences::stickerPartsFile = QString("%1/extras/%2")
           .arg( Preferences::lpubDataPath, Preferences::validStickerPliParts);
   QFileInfo fileInfo(Preferences::stickerPartsFile);
   if (!fileInfo.exists()) {
       if (!StickerParts::exportStickerParts()) {
           emit messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
           return;
       }
   }

   if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
       if (Preferences::systemEditor.isEmpty())
           QDesktopServices::openUrl(QUrl("file:///"+fileInfo.absoluteFilePath(), QUrl::TolerantMode));
       else
#endif
           openWith(fileInfo.absoluteFilePath());
   } else {
       displayParmsFile(fileInfo.absoluteFilePath());
       parmsWindow->setWindowTitle(tr("Parts List Sticker Parts","Edit/add sticker parts"));
       parmsWindow->show();
   }
}

void Gui::editLdrawIniFile()
{
  if (!Preferences::ldSearchDirs.isEmpty()) {
      if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
          if (Preferences::systemEditor.isEmpty())
              QDesktopServices::openUrl(QUrl("file:///"+Preferences::ldrawiniFile, QUrl::TolerantMode));
          else
#endif
              openWith(Preferences::ldrawiniFile);
      } else {
          displayParmsFile(Preferences::ldrawiniFile);
          parmsWindow->setWindowTitle(tr("LDraw.ini Edit","Edit LDraw.ini search directory entries."));
          parmsWindow->show();
      }
    }
}

void Gui::editLPub3DIniFile()
{
    QString lpubConfigFile,fileExt;
    QString companyName = QString(VER_COMPANYNAME_STR).toLower();
#if defined Q_OS_WIN
    fileExt = "ini";
    if (Preferences::portableDistribution)
        lpubConfigFile = QString("%1/config/%2/%3.%4")
                                 .arg(Preferences::lpub3dPath)
                                 .arg(companyName)
                                 .arg(Preferences::lpub3dAppName).arg(fileExt);
#elif defined Q_OS_MACOS
    fileExt = "plist";
    lpubConfigFile = QString("%1/com.%2.%3.%4")
                             .arg(Preferences::lpubConfigPath)
                             .arg(companyName.replace(" ","-"))
                             .arg(QString(Preferences::lpub3dAppName).replace(".app","")).arg(fileExt);
#elif defined Q_OS_LINUX
    fileExt = "conf";
    lpubConfigFile = QString("%1/%2/%3.%4")
                             .arg(Preferences::lpubConfigPath)
                             .arg(companyName)
                             .arg(Preferences::lpub3dAppName).arg(fileExt);
#endif

    if (Preferences::useSystemEditor) {
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+lpubConfigFile, QUrl::TolerantMode));
        else
            openWith(lpubConfigFile);
    } else {
        displayParmsFile(lpubConfigFile);
        parmsWindow->setWindowTitle(tr("%1.%2 Edit","Edit %1 application configuration settings.")
                                       .arg(Preferences::lpub3dAppName).arg(fileExt));
        parmsWindow->show();
    }
}

void Gui::editLdgliteIni()
{
    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+Preferences::ldgliteIni, QUrl::TolerantMode));
        else
#endif
            openWith(Preferences::ldgliteIni);
    } else {
        displayParmsFile(Preferences::ldgliteIni);
        parmsWindow->setWindowTitle(tr("Edit LDGLite INI","Edit LDGLite INI "));
        parmsWindow->show();
    }
}

void Gui::editNativePovIni()
{
    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+Preferences::nativeExportIni, QUrl::TolerantMode));
        else
#endif
            openWith(Preferences::nativeExportIni);
    } else {
        displayParmsFile(Preferences::nativeExportIni);
        parmsWindow->setWindowTitle(tr("Edit Native Export INI","Edit Native Export INI"));
        parmsWindow->show();
    }
}

void Gui::editLdviewIni()
{
    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+Preferences::ldviewIni, QUrl::TolerantMode));
        else
#endif
            openWith(Preferences::ldviewIni);
    } else {
        displayParmsFile(Preferences::ldviewIni);
        parmsWindow->setWindowTitle(tr("Edit LDView INI","Edit LDView INI"));
        parmsWindow->show();
    }
}

void Gui::editBlenderParameters()
{
    QString const blenderConfigDir = QString("%1/Blender/config").arg(Preferences::lpub3d3rdPartyConfigDir);
    QFileInfo fileInfo(QString("%1/%2").arg(blenderConfigDir).arg(VER_BLENDER_PARAMS_FILE));
    if (!fileInfo.exists()) {
        if (!BlenderRenderDialogGui::exportParameterFile()) {
            emit messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
            return;
        }
    }

    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+fileInfo.absoluteFilePath(), QUrl::TolerantMode));
        else
#endif
            openWith(fileInfo.absoluteFilePath());
    } else {
        displayParmsFile(fileInfo.absoluteFilePath());
        parmsWindow->setWindowTitle(tr("Part Blender LDraw Parameters reference",
                                       "Edit/add Part Blender LDraw Parameters reference"));
        parmsWindow->show();
    }
}

void Gui::editLdviewPovIni()
{
    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+Preferences::ldviewPOVIni, QUrl::TolerantMode));
        else
#endif
            openWith(Preferences::ldviewPOVIni);
    } else {
        displayParmsFile(Preferences::ldviewPOVIni);
        parmsWindow->setWindowTitle(tr("Edit LDView POV file generation INI","Edit LDView POV file generation INI"));
        parmsWindow->show();
    }
}

void Gui::editPovrayIni()
{
    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+Preferences::povrayIni, QUrl::TolerantMode));
        else
#endif
            openWith(Preferences::povrayIni);
    } else {
        displayParmsFile(Preferences::povrayIni);
        parmsWindow->setWindowTitle(tr("Edit Raytracer INI","Edit Raytracer INI"));
        parmsWindow->show();
    }
}

void Gui::editPovrayConf()
{
    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+Preferences::povrayConf, QUrl::TolerantMode));
        else
#endif
            openWith(Preferences::povrayConf);
    } else {
        displayParmsFile(Preferences::povrayConf);
        parmsWindow->setWindowTitle(tr("Edit Raytracer file access conf","Edit Raytracer file access conf"));
        parmsWindow->show();
    }
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

    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+fileInfo.absoluteFilePath(), QUrl::TolerantMode));
        else
#endif
            openWith(fileInfo.absoluteFilePath());
    } else {
        displayParmsFile(fileInfo.absoluteFilePath());
        parmsWindow->setWindowTitle(tr("LDraw to Bricklink Design ID Cross-reference",
                                       "Edit/add LDraw to Bricklink design ID cross-reference"));
        parmsWindow->show();
    }
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

    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+fileInfo.absoluteFilePath(), QUrl::TolerantMode));
        else
#endif
            openWith(fileInfo.absoluteFilePath());
    } else {
        displayParmsFile(fileInfo.absoluteFilePath());
        parmsWindow->setWindowTitle(tr("LDraw to Bricklink Color Code cross-reference",
                                       "Edit/add LDraw to Bricklink Color Code cross-reference"));
        parmsWindow->show();
    }
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

    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+fileInfo.absoluteFilePath(), QUrl::TolerantMode));
        else
#endif
            openWith(fileInfo.absoluteFilePath());
    } else {
        displayParmsFile(fileInfo.absoluteFilePath());
        parmsWindow->setWindowTitle(tr("Bricklink Color ID reference",
                                       "Edit/add Bricklink Color ID reference"));
        parmsWindow->show();
    }
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

    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+fileInfo.absoluteFilePath(), QUrl::TolerantMode));
        else
#endif
            openWith(fileInfo.absoluteFilePath());
    } else {
        displayParmsFile(fileInfo.absoluteFilePath());
        parmsWindow->setWindowTitle(tr("LDraw to Rebrickable Design ID Cross-reference",
                                       "Edit/add LDraw to Rebrickable design ID cross-reference"));
        parmsWindow->show();
    }
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

    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+fileInfo.absoluteFilePath(), QUrl::TolerantMode));
        else
#endif
            openWith(fileInfo.absoluteFilePath());
    } else {
        displayParmsFile(fileInfo.absoluteFilePath());
        parmsWindow->setWindowTitle(tr("LDraw to Rebrickable Color Code cross-reference",
                                       "Edit/add LDraw to Rebrickable Color Code cross-reference"));
        parmsWindow->show();
    }
}

void Gui::editBLCodes()
{
    QFileInfo fileInfo(QString("%1/extras/%2").arg(Preferences::lpubDataPath,VER_LPUB3D_BLCODES_FILE));

    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+fileInfo.absoluteFilePath(), QUrl::TolerantMode));
        else
#endif
            openWith(fileInfo.absoluteFilePath());
    } else {
        displayParmsFile(fileInfo.absoluteFilePath());
        parmsWindow->setWindowTitle(tr("Bricklink Codes reference",
                                       "Edit/add Bricklink Codes reference"));
        parmsWindow->show();
    }
}

void Gui::viewLog()
{
    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+Preferences::logPath, QUrl::TolerantMode));
        else
#endif
            openWith(Preferences::logPath);
    } else {
        displayParmsFile(Preferences::logPath);
        parmsWindow->setWindowTitle(tr(VER_PRODUCTNAME_STR " log",VER_PRODUCTNAME_STR " logs"));
        parmsWindow->show();
    }
}

void Gui::preferences()
{
    bool libraryChangeRestart              = false;
    bool defaultUnitsCompare               = Preferences::preferCentimeters;
    bool enableLDViewSCallCompare          = Preferences::enableLDViewSingleCall;
    bool enableLDViewSListCompare          = Preferences::enableLDViewSnaphsotList;
    bool displayAllAttributesCompare       = Preferences::displayAllAttributes;
    bool generateCoverPagesCompare         = Preferences::generateCoverPages;
    bool enableFadeStepsCompare            = Preferences::enableFadeSteps;
    bool fadeStepsUseColourCompare         = Preferences::fadeStepsUseColour;
    int fadeStepsOpacityCompare            = Preferences::fadeStepsOpacity;
    bool enableHighlightStepCompare        = Preferences::enableHighlightStep;
    bool enableImageMattingCompare         = Preferences::enableImageMatting;
    bool applyCALocallyCompare             = Preferences::applyCALocally;
    int  highlightStepLineWidthCompare     = Preferences::highlightStepLineWidth;
    bool highlightFirstStepCompare         = Preferences::highlightFirstStep;
    bool doNotShowPageProcessDlgCompare    = Preferences::doNotShowPageProcessDlg;
    int  pageDisplayPauseCompare           = Preferences::pageDisplayPause;
    bool addLSynthSearchDirCompare         = Preferences::addLSynthSearchDir;
    bool archiveLSynthPartsCompare         = Preferences::archiveLSynthParts;
    bool perspectiveProjectionCompare      = Preferences::perspectiveProjection;
    bool inlineNativeContentCompare        = Preferences::inlineNativeContent;
    bool saveOnUpdateCompare               = Preferences::saveOnUpdate;
    bool saveOnRedrawCompare               = Preferences::saveOnRedraw;
    bool loadLastOpenedFileCompare         = Preferences::loadLastOpenedFile;
    bool extendedSubfileSearchCompare      = Preferences::extendedSubfileSearch;
    bool povrayAutoCropCompare             = Preferences::povrayAutoCrop;
    bool showDownloadRedirectsCompare      = Preferences::showDownloadRedirects;
    bool povFileGeneratorCompare           = Preferences::useNativePovGenerator;
    int preferredRendererCompare           = Preferences::preferredRenderer;
    int povrayRenderQualityCompare         = Preferences::povrayRenderQuality;
    int ldrawFilesLoadMsgsCompare          = Preferences::ldrawFilesLoadMsgs;
    bool lineParseErrorsCompare            = Preferences::lineParseErrors;
    bool showInsertErrorsCompare           = Preferences::showInsertErrors;
    bool showAnnotationErrorsCompare       = Preferences::showAnnotationErrors;
    QString altLDConfigPathCompare         = Preferences::altLDConfigPath;
    QString fadeStepsColourCompare         = Preferences::validFadeStepsColour;
    QString highlightStepColourCompare     = Preferences::highlightStepColour;
    QString ldrawPathCompare               = Preferences::ldrawLibPath;
    QString lgeoPathCompare                = Preferences::lgeoPath;
    QString displayThemeCompare            = Preferences::displayTheme;
    QString sceneBackgroundColorCompare    = Preferences::sceneBackgroundColor;
    QString sceneGridColorCompare          = Preferences::sceneGridColor;
    QString sceneRulerTickColorCompare     = Preferences::sceneRulerTickColor;
    QString sceneRulerTrackingColorCompare = Preferences::sceneRulerTrackingColor;
    QString sceneGuideColorCompare         = Preferences::sceneGuideColor;

    // 'LDView INI settings
    if (Preferences::preferredRenderer == RENDERER_POVRAY) {
        if (Preferences::useNativePovGenerator)
            TCUserDefaults::setIniFile(Preferences::nativeExportIni.toLatin1().constData());
        else
            TCUserDefaults::setIniFile(Preferences::ldviewPOVIni.toLatin1().constData());
    } else if (Preferences::preferredRenderer == RENDERER_LDVIEW) {
        TCUserDefaults::setIniFile(Preferences::ldviewIni.toLatin1().constData());
    }

    if (Preferences::getPreferences()) {

        Meta meta;
        lpub->page.meta = meta;

        lpub->setKeyboardShortcuts();
        foreach (QAction *action, editWindow->actions()) {
            lpub->setKeyboardShortcut(action);
        }
        foreach (QAction *action, editModeWindow->actions()) {
            lpub->setKeyboardShortcut(action);
        }
        foreach (QAction *action, parmsWindow->actions()) {
            lpub->setKeyboardShortcut(action);
        }

        QMessageBox box;
        box.setMinimumSize(40,20);
        box.setIcon (QMessageBox::Information);
        box.setDefaultButton   (QMessageBox::Ok);
        box.setStandardButtons (QMessageBox::Ok);

        bool defaultUnitsChanged           = Preferences::preferCentimeters                      != defaultUnitsCompare;
        bool rendererChanged               = Preferences::preferredRenderer                      != preferredRendererCompare;
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
        bool inlineNativeContentChanged    = Preferences::inlineNativeContent                     != inlineNativeContentCompare;
        bool saveOnRedrawChanged           = Preferences::saveOnRedraw                           != saveOnRedrawCompare;
        bool saveOnUpdateChanged           = Preferences::saveOnUpdate                           != saveOnUpdateCompare;
        bool applyCALocallyChanged         = Preferences::applyCALocally                         != applyCALocallyCompare;
        bool enableLDViewSCallChanged      = Preferences::enableLDViewSingleCall                 != enableLDViewSCallCompare;
        bool enableLDViewSListChanged      = Preferences::enableLDViewSnaphsotList               != enableLDViewSListCompare;
        bool displayAttributesChanged      = Preferences::displayAllAttributes                   != displayAllAttributesCompare;
        bool generateCoverPagesChanged     = Preferences::generateCoverPages                     != generateCoverPagesCompare;
        bool pageDisplayPauseChanged       = Preferences::pageDisplayPause                       != pageDisplayPauseCompare;
        bool doNotShowPageProcessDlgChanged= Preferences::doNotShowPageProcessDlg                != doNotShowPageProcessDlgCompare;
        bool povFileGeneratorChanged       = Preferences::useNativePovGenerator                  != povFileGeneratorCompare;
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
        bool sceneRulerTrackingColorChanged= Preferences::sceneRulerTrackingColor.toLower()      != sceneRulerTrackingColorCompare.toLower();
        bool sceneGuideColorChanged        = Preferences::sceneGuideColor.toLower()              != sceneGuideColorCompare.toLower();
        bool ldrawFilesLoadMsgsChanged     = Preferences::ldrawFilesLoadMsgs                     != ldrawFilesLoadMsgsCompare;

        bool lineParseErrorsChanged        = Preferences::lineParseErrors                        != lineParseErrorsCompare;
        bool showInsertErrorsChanged       = Preferences::showInsertErrors                       != showInsertErrorsCompare;
        bool showAnnotationErrorsChanged   = Preferences::showAnnotationErrors                   != showAnnotationErrorsCompare;

        bool displayThemeColorsChanged     = Preferences::displayThemeColorsChanged;
        bool textDecorationColorChanged    = Preferences::textDecorationColorChanged;

        if (displayThemeColorsChanged) {
            Preferences::displayThemeColorsChanged = false;
            emit messageSig(LOG_INFO,QString("Display theme colors have changed"));
        }

        if (textDecorationColorChanged) {
            Preferences::textDecorationColorChanged = false;
            emit messageSig(LOG_INFO,QString("Text Decoration color have changed"));
        }

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
            emit messageSig(LOG_INFO,QString("LDraw file load status dialogue set to %1").arg(
                Preferences::ldrawFilesLoadMsgs == NEVER_SHOW ? "Never Show" :
                Preferences::ldrawFilesLoadMsgs == SHOW_ERROR ? "Show Error" :
                Preferences::ldrawFilesLoadMsgs == SHOW_WARNING ? "Show Warning" :
                Preferences::ldrawFilesLoadMsgs == SHOW_MESSAGE ? "Show Message" :
                "Always Show"));

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

        if (sceneRulerTrackingColorChanged)
            emit messageSig(LOG_INFO,QString("Scene Ruler Tracking Color changed from %1 to %2")
                            .arg(sceneRulerTrackingColorCompare)
                            .arg(Preferences::sceneRulerTrackingColor));

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
            if (Preferences::enableFadeSteps && !LDrawColourParts::ldrawColorPartsIsLoaded()) {
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

        if (enableHighlightStepChanged) {
            emit messageSig(LOG_INFO,QString("Highlight Current Step is %1.").arg(Preferences::enableHighlightStep ? "ON" : "OFF"));
        }
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
            LoadDefaults();
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
            emit messageSig(LOG_INFO,QString("Renderer preference changed from %1 to %2%3")
                            .arg(preferredRendererCompare)
                            .arg(Preferences::preferredRenderer)
                            .arg(Preferences::preferredRenderer == RENDERER_POVRAY ? QString(" (POV file generator is %1)")
                                                                                             .arg(Preferences::useNativePovGenerator ? RENDERER_NATIVE : RENDERER_LDVIEW) :
                                 Preferences::preferredRenderer == RENDERER_LDVIEW ? Preferences::enableLDViewSingleCall ?
                                                                                     Preferences::enableLDViewSnaphsotList ? QString(" (Single Call using Export File List)") :
                                                                                                                             QString(" (Single Call)") :
                                                                                                                             QString() : QString()));
            Render::setRenderer(Preferences::preferredRenderer);
            if (Preferences::preferredRenderer == RENDERER_LDGLITE)
                partWorkerLDSearchDirs.populateLdgLiteSearchDirs();
        }

        if (povFileGeneratorChanged)
            emit messageSig(LOG_INFO,QString("POV file generation renderer changed from %1 to %2")
                                             .arg(povFileGeneratorCompare ? RENDERER_NATIVE : RENDERER_LDVIEW)
                                             .arg(Preferences::useNativePovGenerator ? RENDERER_NATIVE : RENDERER_LDVIEW));

        if (altLDConfigPathChanged) {
            emit messageSig(LOG_INFO,QString("Use Alternate LDConfig (Restart Required) %1.").arg(Preferences::altLDConfigPath));
            box.setText (QString("%1 will restart to properly load the alternate LDConfig file.").arg(VER_PRODUCTNAME_STR));
            box.exec();
        }

        if (lineParseErrorsChanged)
            emit messageSig(LOG_INFO,QString("Show Parse Errors is %1").arg(Preferences::lineParseErrors? "ON" : "OFF"));

        if (showAnnotationErrorsChanged)
            emit messageSig(LOG_INFO,QString("Show Parse Errors is %1").arg(Preferences::showAnnotationErrors? "ON" : "OFF"));


        if (showInsertErrorsChanged)
            emit messageSig(LOG_INFO,QString("Show Insert Errors is %1").arg(Preferences::showInsertErrors    ? "ON" : "OFF"));

        if (inlineNativeContentChanged)
            emit messageSig(LOG_INFO,QString("Inline Native Render Content is %1").arg(Preferences::inlineNativeContent? "ON" : "OFF"));

        bool sceneDisplayChanged =
            displayThemeChanged         ||
            sceneBackgroundColorChanged ||
            sceneGridColorChanged       ||
            sceneRulerTickColorChanged  ||
            sceneGuideColorChanged;
        if (displayThemeColorsChanged   ||
            sceneDisplayChanged)
        {
            if (displayThemeChanged     ||
                displayThemeColorsChanged)
                loadTheme();
            if (sceneDisplayChanged)
                KpageView->setSceneTheme();
            if (sceneGridColorChanged   ||
                textDecorationColorChanged)
                reloadCurrentPage();
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

        if (altLDConfigPathChanged || libraryChangeRestart) {
            restartApplication(libraryChangeRestart);
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

    // classes
    qRegisterMetaType<BackgroundData>("BackgroundData");
    qRegisterMetaType<BorderData>("BorderData");
    qRegisterMetaType<BuffExchgData>("BuffExchgData");
    qRegisterMetaType<BuildModFlags>("BuildModFlags");
    qRegisterMetaType<BuildModData>("BuildModData");
    qRegisterMetaType<ConstrainData>("ConstrainData");
    qRegisterMetaType<CsiAnnotationIconData>("CsiAnnotationIconData");
    qRegisterMetaType<FindPageFlags>("FindPageFlags");
    qRegisterMetaType<FreeFormData>("FreeFormData");
    qRegisterMetaType<InsertData>("InsertData");
    qRegisterMetaType<JustifyStepData>("JustifyStepData");
    qRegisterMetaType<NativeOptions>("NativeOptions");
    qRegisterMetaType<PageSizeData>("PageSizeData");
    qRegisterMetaType<PgSizeData>("PgSizeData");
    qRegisterMetaType<PlacementData>("PlacementData");
    qRegisterMetaType<PliPartGroupData>("PliPartGroupData");
    qRegisterMetaType<PointerAttribData>("PointerAttribData");
    qRegisterMetaType<PointerData>("PointerData");
    qRegisterMetaType<RotStepData>("RotStepData");
    qRegisterMetaType<SceneObjectData>("SceneObjectData");
    qRegisterMetaType<SepData>("SepData");
    qRegisterMetaType<SubData>("SubData");
    qRegisterMetaType<Where>("Where");
    qRegisterMetaType<Vector3>("Vector3");

    // structs
    qRegisterMetaType<Action>("Action");
    qRegisterMetaType<ActionAttributes>("ActionAttributes");
    qRegisterMetaType<lcLibRenderOptions>("lcLibRenderOptions");
    qRegisterMetaType<LineHighlight>("LineHighlight");
    qRegisterMetaType<StepLines>("StepLines");
    qRegisterMetaType<TypeLine>("TypeLine");

    // enumerators
    qRegisterMetaType<ActionModuleType>("ActionModuleType");
    qRegisterMetaType<AllocEnc>("AllocEnc");
    qRegisterMetaType<AnnotationCategory>("AnnotationCategory");
    qRegisterMetaType<AnnotationStyle>("AnnotationStyle");
    qRegisterMetaType<Boundary>("Boundary");
    qRegisterMetaType<BuildModEnabledEnc>("BuildModEnabledEnc");
    qRegisterMetaType<BuildModRc>("BuildModRc");
    qRegisterMetaType<CamFlag>("CamFlag");
    qRegisterMetaType<ContStepNumEnc>("ContStepNumEnc");
    qRegisterMetaType<CountInstanceEnc>("CountInstanceEnc");
    qRegisterMetaType<Dim>("Dim");
    qRegisterMetaType<Dimensions>("Dimensions");
    qRegisterMetaType<DisplayModelType>("DisplayModelType");
    qRegisterMetaType<DividerType>("DividerType");
    qRegisterMetaType<ExportMode>("ExportMode");
    qRegisterMetaType<ExportOption>("ExportOption");
    qRegisterMetaType<FillEnc>("FillEnc");
    qRegisterMetaType<FinalModelEnabledEnc>("FinalModelEnabledEnc");
    qRegisterMetaType<GridStepSize>("GridStepSize");
    qRegisterMetaType<IniFlag>("IniFlag");
    qRegisterMetaType<JustifyStepEnc>("JustifyStepEnc");
    qRegisterMetaType<LDrawFileRegExp>("LDrawFileRegExp");
    qRegisterMetaType<LDrawUnofficialFileType>("LDrawUnofficialFileType");
    qRegisterMetaType<LibType>("LibType");
    qRegisterMetaType<LightType>("LightType");
    qRegisterMetaType<LineHighlightType>("LineHighlightType");
    qRegisterMetaType<LoadMsgType>("LoadMsgType");
    qRegisterMetaType<LogType>("LogType");
    qRegisterMetaType<MissingHeader>("MissingHeader");
    qRegisterMetaType<Preferences::MsgKey>("Preferences::MsgKey");
    qRegisterMetaType<Preferences::MsgID>("Preferences::MsgID");
    qRegisterMetaType<NameKeyAttributes>("NameKeyAttributes");
    qRegisterMetaType<NameKeyAttributes2>("NameKeyAttributes2");
    qRegisterMetaType<NativeRenderType>("NativeRenderType");
    qRegisterMetaType<NativeType>("NativeType");
    qRegisterMetaType<OrientationEnc>("OrientationEnc");
    qRegisterMetaType<PAction>("PAction");
    qRegisterMetaType<PageDirection>("PageDirection");
    qRegisterMetaType<PageTypeEnc>("PageTypeEnc");
    qRegisterMetaType<PartSource>("PartSource");
    qRegisterMetaType<PartType>("PartType");
    qRegisterMetaType<PgSizeData>("PgSizeData");
    qRegisterMetaType<PlacementEnc>("PlacementEnc");
    qRegisterMetaType<PlacementTableEnc>("PlacementTableEnc");
    qRegisterMetaType<PlacementType>("PlacementType");
    qRegisterMetaType<PliType>("PliType");
    qRegisterMetaType<PrepositionEnc>("PrepositionEnc");
    qRegisterMetaType<ProcessType>("ProcessType");
    qRegisterMetaType<RectPlacement>("RectPlacement");
    qRegisterMetaType<RelativeTos>("RelativeTos");
    qRegisterMetaType<RemoveLPubFormatType>("RemoveLPubFormatType");
    qRegisterMetaType<RemoveObjectsRC>("RemoveObjectsRC");
    qRegisterMetaType<RendererType>("RendererType");
    qRegisterMetaType<RulerTrackingType>("RulerTrackingType");
    qRegisterMetaType<SaveOnSender>("SaveOnSender");
    qRegisterMetaType<SceneGuidesPosType>("SceneGuidesPosType");
    qRegisterMetaType<SceneObject>("SceneObject");
    qRegisterMetaType<SceneObjectDirection>("SceneObjectDirection");
    qRegisterMetaType<SceneObjectInfo>("SceneObjectInfo");
    qRegisterMetaType<ShowLineType>("ShowLineType");
    qRegisterMetaType<ShowLoadMsgType>("ShowLoadMsgType");
    qRegisterMetaType<SortDirection>("SortDirection");
    qRegisterMetaType<SortOption>("SortOption");
    qRegisterMetaType<SortOrder>("SortOrder");
    qRegisterMetaType<StepLines>("StepLines");
    qRegisterMetaType<StudStyleEnc>("StudStyleEnc");
    qRegisterMetaType<SubAttributes>("SubAttributes");
    qRegisterMetaType<Theme>("Theme");
    qRegisterMetaType<ThemeColorType>("ThemeColorType");
    qRegisterMetaType<TraverseRc>("TraverseRc");
    qRegisterMetaType<TypeLine>("TypeLine");

    Preferences::lgeoPreferences();
    Preferences::publishingPreferences();
    Preferences::exportPreferences();
    Preferences::keyboardShortcutPreferences();

    sa                 = 0;
    pa                 = 0;
    saveDisplayPageNum = 0;
    displayPageNum     = 1 + pa;
    numPrograms        = 0;
    previewDockWindow  = nullptr;

    pageProcessRunning              = PROC_NONE;        // display page process
    processOption                   = EXPORT_ALL_PAGES; // export process
    m_exportMode                    = EXPORT_PDF;
    pageRangeText                   = "1";
    exportPixelRatio                = 1.0;
    mloadingFile                    = false;
    resetCache                      = false;
    m_previewDialog                 = false;
    m_partListCSIFile               = false;
    m_exportingContent              = false;
    m_exportingObjects              = false;
    m_contPageProcessing            = false;
    nextPageContinuousIsRunning     = false;
    previousPageContinuousIsRunning = false;

    mBuildModRange    = { 0, 0, -1 };
    mStepRotation     = { 0.0f, 0.0f, 0.0f };
    mRotStepAngleX    = 0.0f;
    mRotStepAngleY    = 0.0f;
    mRotStepAngleZ    = 0.0f;
    mRotStepTransform = QString();
    mPliIconsPath.clear();

    selectedItemObj   = UndefinedObj;
    mViewerZoomLevel  = 50;

    if (lpub && Preferences::modeGUI)
        lpub->meta.metaKeywords(lpub->metaKeywords);

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

    if (Preferences::modeGUI) {
        QColor spinnerColor(
               Preferences::displayTheme == THEME_DARK ?
               Preferences::themeColors[THEME_DARK_PALETTE_TEXT] : LPUB3D_DEFAULT_COLOUR);
        waitingSpinner = new WaitingSpinnerWidget(KpageView);
        waitingSpinner->setColor(spinnerColor);
        waitingSpinner->setRoundness(70.0);
        waitingSpinner->setMinimumTrailOpacity(15.0);
        waitingSpinner->setTrailFadePercentage(70.0);
        waitingSpinner->setNumberOfLines(12);
        waitingSpinner->setLineLength(10);
        waitingSpinner->setLineWidth(5);
        waitingSpinner->setInnerRadius(10);
        waitingSpinner->setRevolutionsPerSecond(1);
    }

    setCentralWidget(KpageView);

    mpdCombo = new SeparatorComboBox(this);
    mpdCombo->setMinimumContentsLength(MPD_COMBO_MIN_ITEMS_DEFAULT);
    mpdCombo->setInsertPolicy(QComboBox::InsertAtBottom);
    mpdCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    mpdCombo->setToolTip(tr("Current Submodel"));
    mpdCombo->setStatusTip("Use dropdown to select submodel");
    mpdCombo->setEnabled(false);

    setGoToPageCombo = new QComboBox(this);
    setGoToPageCombo->setMinimumContentsLength(GO_TO_PAGE_MIN_ITEMS_DEFAULT);
    setGoToPageCombo->setInsertPolicy(QComboBox::InsertAtBottom);
    setGoToPageCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    setGoToPageCombo->setToolTip(tr("Current Page"));
    setGoToPageCombo->setStatusTip("Use dropdown to select page");
    setGoToPageCombo->setEnabled(false);

    undoStack = new QUndoStack();
    macroNesting = 0;
    viewerUndo = false;
    viewerRedo = false;

#if defined Q_OS_MACOS
    if (Preferences::systemEditor.isEmpty())
        Preferences::systemEditor = QString("");
#endif

    connect(&futureWatcher, &QFutureWatcher<int>::finished, this, &Gui::pagesCounted);

    connect(lpub,           SIGNAL(messageSig( LogType,QString)),
            this,           SLOT(statusMessage(LogType,QString)));

    // Gui
    connect(this,           SIGNAL(messageSig( LogType,QString)),
            this,           SLOT(statusMessage(LogType,QString)));

    connect(this,           SIGNAL(setExportingSig(bool)),
            this,           SLOT(  setExporting(   bool)));

    connect(this,           SIGNAL(setExportingObjectsSig(bool)),
            this,           SLOT(  setExportingObjects(   bool)));

    connect(this,           SIGNAL(setContinuousPageSig(bool)),
            this,           SLOT(  setContinuousPage(   bool)));

    // Gui - ParmsWindow
    connect(this,           SIGNAL(displayParmsFileSig(const QString &)),
            parmsWindow,    SLOT( displayParmsFile(    const QString &)));

    // Gui - EditWindow
    connect(this,           SIGNAL(displayFileSig(LDrawFile *, const QString &, const StepLines &)),
            editWindow,     SLOT(  displayFile   (LDrawFile *, const QString &, const StepLines &)));

    connect(this,           SIGNAL(setLineScopeSig(const StepLines &)),
            editWindow,     SLOT(  setLineScope(   const StepLines &)));

    connect(this,           SIGNAL(showLineSig(int, int)),
            editWindow,     SLOT(  showLine(   int, int)));

    connect(this,           SIGNAL(highlightSelectedLinesSig(QVector<int> &, bool)),
            editWindow,     SLOT(  highlightSelectedLines(   QVector<int> &, bool)));

    connect(this,           SIGNAL(clearEditorWindowSig()),
            editWindow,     SLOT(  clearEditorWindow()));

    connect(this,           SIGNAL(setTextEditHighlighterSig()),
            editWindow,     SLOT(  setTextEditHighlighter()));

    connect(this,           SIGNAL(setSubFilesSig(const QStringList &)),
            editWindow,     SLOT(  setSubFiles(   const QStringList &)));

    connect(this,           SIGNAL(visualEditorVisibleSig(bool)),
            editWindow,     SLOT(setVisualEditorVisible(bool)));

    // Edit Window - Gui
    connect(editWindow,     SIGNAL(SelectedPartLinesSig(QVector<TypeLine>&,PartSource)),
            this,           SLOT(SelectedPartLines(     QVector<TypeLine>&,PartSource)));

    connect(editWindow,     SIGNAL(redrawSig()),
            this,           SLOT(  clearAndReloadModelFile()));

    connect(editWindow,     SIGNAL(updateSig()),
            this,           SLOT(  reloadCurrentPage()));

    connect(editWindow,     SIGNAL(contentsChangeSig(const QString &,bool,bool,int,int,const QString &)),
            this,           SLOT(  contentsChange(   const QString &,bool,bool,int,int,const QString &)));

    connect(editWindow,     SIGNAL(setStepForLineSig()),
            this,           SLOT(  setStepForLine()));

    connect(editWindow,     SIGNAL(editModelFileSig()),
            this,           SLOT(  editModelFile()));

    connect(editWindow,     SIGNAL(getSubFileListSig()),
            this,           SLOT(  getSubFileList()));

    // Edit Window - Edit Model Window
    connect(editWindow,     SIGNAL(updateDisabledSig(bool)),
            editModeWindow, SLOT(  updateDisabled(bool)));

    // Edit Model Window
    connect(this,           SIGNAL(setTextEditHighlighterSig()),
            editModeWindow, SLOT(  setTextEditHighlighter()));

    connect(this,           SIGNAL(displayModelFileSig(LDrawFile *, const QString &)),
            editModeWindow, SLOT(  displayFile(        LDrawFile *, const QString &)));

    connect(editModeWindow, SIGNAL(refreshModelFileSig()),
            this,           SLOT(  refreshModelFile()));

    connect(editModeWindow, SIGNAL(redrawSig()),
            this,           SLOT(  clearAndRedrawModelFile()));

    connect(editModeWindow, SIGNAL(updateSig()),
            this,           SLOT(  reloadCurrentModelFile()));

    connect(this,           SIGNAL(clearEditorWindowSig()),
            editModeWindow, SLOT(  clearEditorWindow()));

    connect(editModeWindow, SIGNAL(contentsChangeSig(const QString &,bool,bool,int,int,const QString &)),
            this,           SLOT(  contentsChange(   const QString &,bool,bool,int,int,const QString &)));

    // cache management
    connect(this,           SIGNAL(clearPageCacheSig(PlacementType, Page*, int)),
            this,           SLOT(  clearPageCache(PlacementType, Page*, int)));

    connect(this,           SIGNAL(clearAndReloadModelFileSig(bool)),
            this,           SLOT(  clearAndReloadModelFile(bool)));

    connect(this,           SIGNAL(clearCustomPartCacheSig(bool)),
            this,           SLOT(  clearCustomPartCache(bool)));

    connect(this,           SIGNAL(clearAllCachesSig(bool)),          // reloadDisplayPage
            this,           SLOT(  clearAllCaches(bool)));

    connect(this,           SIGNAL(clearSubmodelCacheSig()),
            this,           SLOT(  clearSubmodelCache()));

    connect(this,           SIGNAL(clearPLICacheSig()),
            this,           SLOT(  clearPLICache()));

    connect(this,           SIGNAL(clearCSICacheSig()),
            this,           SLOT(  clearCSICache()));

    connect(this,           SIGNAL(clearTempCacheSig()),
            this,           SLOT(  clearTempCache()));

    connect(this,           SIGNAL(reloadCurrentPageSig()),
            this,           SLOT(  reloadCurrentPage()));

    connect(this,           SIGNAL(restartApplicationSig()),
            this,           SLOT(  restartApplication()));

    // Undo Stack
    connect(undoStack,      SIGNAL(cleanChanged(bool)),
            editModeWindow, SLOT(  updateDisabled(bool)));

    connect(undoStack,      SIGNAL(cleanChanged(bool)),
            editWindow,     SLOT(  updateDisabled(bool)));

    connect(undoStack,      SIGNAL(canRedoChanged(bool)),
            this,           SLOT(  canRedoChanged(bool)));

    connect(undoStack,      SIGNAL(canUndoChanged(bool)),
            this,           SLOT(  canUndoChanged(bool)));

    connect(undoStack,      SIGNAL(cleanChanged(bool)),
            this,           SLOT(  cleanChanged(bool)));

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

    connect (m_progressDialog, SIGNAL (cancelClicked()),
             this,             SLOT (  cancelExporting()));

    connect(this, SIGNAL(progressBarInitSig()),
            this, SLOT(  progressBarInit()));
    connect(this, SIGNAL(progressMessageSig(const QString &)),
            this, SLOT(  progressBarSetText(const QString &)));
    connect(this, SIGNAL(progressRangeSig(int,int)),
            this, SLOT(  progressBarSetRange(int,int)));
    connect(this, SIGNAL(progressSetValueSig(int)),
            this, SLOT(  progressBarSetValue(int)));
    connect(this, SIGNAL(progressResetSig()),
            this, SLOT(  progressBarReset()));
    connect(this, SIGNAL(progressStatusRemoveSig()),
            this, SLOT(  progressStatusRemove()));

    connect(this, SIGNAL(progressBarPermInitSig()),
            this, SLOT(  progressBarPermInit()));
    connect(this, SIGNAL(progressPermMessageSig(const QString &)),
            this, SLOT(  progressBarPermSetText(const QString &)));
    connect(this, SIGNAL(progressPermRangeSig(int,int)),
            this, SLOT(  progressBarPermSetRange(int,int)));
    connect(this, SIGNAL(progressPermSetValueSig(int)),
            this, SLOT(  progressBarPermSetValue(int)));
    connect(this, SIGNAL(progressPermResetSig()),
            this, SLOT(  progressBarPermReset()));
    connect(this, SIGNAL(progressPermStatusRemoveSig()),
            this, SLOT(  progressPermStatusRemove()));

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
  delete editModeWindow;
  delete undoStack;
  delete mpdCombo;
  delete setGoToPageCombo;

  delete progressBar;
  delete m_progressDialog;
  delete progressLabelPerm;
  delete progressBarPerm;
}

void Gui::closeEvent(QCloseEvent *event)
{
  Preferences::resetFadeSteps();
  Preferences::resetHighlightStep();
  Preferences::resetPreferredRenderer();

  writeSettings();

  emit requestEndThreadNowSig();

  if (parmsWindow->isVisible())
    parmsWindow->close();

  if (maybeSave() && saveBuildModification()) {
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

  connect(this, SIGNAL(loadFileSig(QString)),
          this, SLOT(  loadFile(QString)));
  connect(lpub, SIGNAL(loadFileSig(QString, bool)),
          this, SLOT(  loadFile(QString, bool)));
  connect(lpub, SIGNAL(consoleCommandSig(int, int*)),
          this, SLOT(  consoleCommandCurrentThread(int, int*)), Qt::DirectConnection);
  connect(this, SIGNAL(consoleCommandFromOtherThreadSig(int, int*)),
          this, SLOT(  consoleCommand(int, int*)), Qt::BlockingQueuedConnection);
  connect(this, SIGNAL(setGeneratingBomSig(bool)),
          this, SLOT(  deployBanner(bool)));
  connect(this, SIGNAL(setExportingSig(bool)),
          this, SLOT(  deployBanner(bool)));
  connect(this, SIGNAL(setExportingSig(bool)),
          this, SLOT(halt3DViewer(bool)));
  connect(this, SIGNAL(updateAllViewsSig()),
          this, SLOT(UpdateAllViews()));
  connect(this, SIGNAL(setPliIconPathSig(QString&,QString&)),
          this, SLOT(  setPliIconPath(QString&,QString&)));
  connect(this, SIGNAL(parseErrorSig(const QString &, const Where &, Preferences::MsgKey, bool, bool)),
          this, SLOT(  parseError(const QString &, const Where &, Preferences::MsgKey, bool, bool)));

/* Moved to PartWorker::ldsearchDirPreferences()  */
//  if (Preferences::preferredRenderer == RENDERER_LDGLITE)
//      partWorkerLdgLiteSearchDirs.populateLdgLiteSearchDirs();

  emit Application::instance()->splashMsgSig(QString("90% - %1 widgets loading...").arg(VER_PRODUCTNAME_STR));

  if (Preferences::modeGUI) {
      lpub->setupChangeLogUpdate();

      lpub->loadCommandCollection();

      lpub->loadSnippetCollection();

      lpub->loadDialogs();
  }

  createActions();
  createMenus();
  createToolBars();
  createStatusBar();
  createDockWindows();

  if (Preferences::modeGUI) {

      initiaizeVisualEditor();

      toggleLCStatusBar(true);
  }

  emit Application::instance()->splashMsgSig(QString("95% - LDraw colors loading..."));

  LDrawColor::LDrawColorInit();

  Preferences::setInitFadeSteps();
  Preferences::setInitHighlightStep();
  Preferences::setInitPreferredRenderer();

  setCurrentFile("");
  updateOpenWithActions(); // also adds the system editor if defined
  readSettings();

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
      soMap[ReserveBackgroundObj]     = QString("RESERVE");              // 32 ReserveType
      soMap[StepNumberObj]            = QString("STEP_NUMBER");          // 33 StepNumberType
      soMap[SubModelBackgroundObj]    = QString("SUBMODEL_DISPLAY");     // 34 SubModelType
      soMap[SubModelInstanceObj]      = QString("SUBMODEL_INSTANCE");    // 35
      soMap[SubmodelInstanceCountObj] = QString("SUBMODEL_INST_COUNT");  // 36 SubmodelInstanceCountType
      soMap[PartsListPixmapObj]       = QString("PLI_PART");             // 37
      soMap[PartsListGroupObj]        = QString("PLI_PART_GROUP");       // 38
      soMap[StepBackgroundObj]        = QString("STEP_RECTANGLE");       // 39 [StepType]
  }

  if (Preferences::modeGUI) {
      lpub->setShortcutKeywords();

      lpub->loadPreferencesDialog();
  }
}

void Gui::getSubFileList()
{
   setSubFilesSig(fileList());
}

void Gui::loadBLCodes()
{
   if (!Annotations::loadBLCodes()){
       QString URL(VER_LPUB3D_BLCODES_DOWNLOAD_URL);
       lpub->downloadFile(URL, "BrickLink Elements");
       QByteArray Buffer = lpub->getDownloadedFile();
       Annotations::loadBLCodes(Buffer);
   }
}

void Gui::ldrawColorPartsLoad()
{
    if (!LDrawColourParts::ldrawColorPartsIsLoaded()) {
        QString result;
        if (!LDrawColourParts::LDrawColorPartsLoad(result)) {
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

                QString body = QMessageBox::tr ("Would you like to generate this file now ?");
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
            messageSig(LOG_INFO_STATUS, QString("Loaded LDraw color parts file [%2]").arg(Preferences::ldrawColourPartsFile));
        }
    }
}

void Gui::reloadModelFileAfterColorFileGen() {
    if (Preferences::modeGUI) {
        QPixmap _icon = QPixmap(":/icons/lpub96.png");
        QMessageBoxResizable box;
        box.setWindowIcon(QIcon());
        box.setIconPixmap (_icon);
        box.setTextFormat (Qt::RichText);
        box.setWindowTitle(QMessageBox::tr ("%1 Color Parts File.").arg(Preferences::validLDrawLibrary));
        box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        box.setMinimumSize(10,10);
        QString entries;
        if (m_workerJobResult)
            entries = tr(" with %1 entries").arg(m_workerJobResult);
        QString message = QString("The %1 LDraw Color Parts File has finished building%2.")
                                  .arg(Preferences::validLDrawLibrary).arg(entries);
        box.setText (message);

        bool enableFadeSteps = mSetupFadeSteps || Preferences::enableFadeSteps;
        bool enableHighlightSttep = mSetupHighlightStep || Preferences::enableHighlightStep;

        if (enableFadeSteps || enableHighlightSttep) {
            QString body = QMessageBox::tr ("The color file list and current model must be reloaded.<br>Do you want to continue ?");
            box.setInformativeText (body);

            box.setStandardButtons (QMessageBox::Ok | QMessageBox::Cancel);
            if (box.exec() == QMessageBox::Ok && ! getCurFile().isEmpty()) {
                bool _continue;
                if (Preferences::saveOnRedraw) {
                    _continue = maybeSave(false); // No prompt
                } else {
                    _continue = maybeSave(true, SaveOnNone);
                }
                if (!_continue)
                    return;

                timer.start();

                clearPLICache();
                clearCSICache();
                clearSubmodelCache();
                clearTempCache();

                //reload current model file
                bool cycleEachPage = Preferences::cycleEachPage;
                if (!cycleEachPage && displayPageNum > 1)
                  cycleEachPage = LocalDialog::getLocal(VER_PRODUCTNAME_STR, "Cycle each page on model file reload?",nullptr);
                cyclePageDisplay(displayPageNum, PageDirection(cycleEachPage));

                emit messageSig(LOG_STATUS, QString("All caches reset and model file reloaded (%1 parts). %2")
                                .arg(lpub->ldrawFile.getPartCount())
                                .arg(elapsedTime(timer.elapsed())));
            }
        } else {
            box.setDefaultButton   (QMessageBox::Ok);
            box.exec();
        }
    }
}

// left side progress bar - no longer used
void Gui::progressBarInit(){
  if (okToInvokeProgressBar()) {
      progressBar->setMaximumHeight(15);
      statusBar()->addWidget(progressLabel);
      statusBar()->addWidget(progressBar);
      progressLabel->setAlignment(Qt::AlignRight);
      progressLabel->show();
      progressBar->show();
  }
}

void Gui::progressBarSetText(const QString &progressText)
{
  if (okToInvokeProgressBar()) {
      progressLabel->setText(progressText);
      emit gui->messageSig(LOG_INFO, progressText);
    }
}
void Gui::progressBarSetRange(int minimum, int maximum)
{
  if (okToInvokeProgressBar()) {
      progressBar->setRange(minimum,maximum);
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
      progressLabelPerm->setAlignment(Qt::AlignRight);
      progressLabelPerm->show();
      progressBarPerm->show();
    }
}

void Gui::progressBarPermSetText(const QString &progressText)
{
  if (okToInvokeProgressBar()) {
      progressLabelPerm->setText(progressText);
    }
}
void Gui::progressBarPermSetRange(int minimum, int maximum)
{
  if (okToInvokeProgressBar()) {
      progressBarPerm->setRange(minimum,maximum);
    }
}
void Gui::progressBarPermSetValue(int value)
{
  if (okToInvokeProgressBar()) {
      progressBarPerm->setValue(value);
    }
}
void Gui::progressBarPermReset()
{
  if (okToInvokeProgressBar()) {
      progressBarPerm->reset();
    }
}

void Gui::progressPermStatusRemove(){
  if (okToInvokeProgressBar()) {
      statusBar()->removeWidget(progressBarPerm);
      statusBar()->removeWidget(progressLabelPerm);
    }
}

void Gui::loadPages(bool frontCoverPageExist, bool backCoverPageExist){
  int pageNum = 0 + pa;
  disconnect(setGoToPageCombo,SIGNAL(activated(int)), this, SLOT(setGoToPage(int)));
  setGoToPageCombo->clear();

  for(int i=1 + pa;i <= maxPages;i++) {
      pageNum++;
      if (frontCoverPageExist && i == 1) {
          pageNum--;
          setGoToPageCombo->addItem(QString("Front Cover"));
      }
      else if (backCoverPageExist && i == maxPages) {
          setGoToPageCombo->addItem(QString("Back Cover"));
      }
      else
          setGoToPageCombo->addItem(QString("Page %1").arg(QString::number(pageNum)));
  }

  setGoToPageCombo->setCurrentIndex(displayPageNum - 1 - pa);
  connect(setGoToPageCombo,SIGNAL(activated(int)), this, SLOT(setGoToPage(int)));
}

void Gui::addEditLDrawIniFileAction()
{
    if (Preferences::ldrawiniFound) {
        getMenu("editorMenu")->insertAction(getAct("editNativePOVIniAct.1"), getAct("editLdrawIniFileAct.1"));
        getToolBar("editParamsToolBar")->insertAction(getAct("editNativePOVIniAct.1"), getAct("editLdrawIniFileAct.1"));
    }
}

void Gui::showRenderDialog()
{
    int importOnly = sender() == blenderImportAct ? BLENDER_IMPORT : 0;
    int renderType = sender() == blenderRenderAct || importOnly ? BLENDER_RENDER : POVRAY_RENDER;

    RenderDialog *dialog = new RenderDialog(nullptr/*set null for full non-modal*/, renderType, importOnly);
    dialog->setModal(false);
    dialog->show();
}

void Gui::aboutDialog()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void Gui::archivePartsOnLaunch() {
    bool archivePartsOnLaunchCompare  = Preferences::archivePartsOnLaunch;
    Preferences::archivePartsOnLaunch = getAct("archivePartsOnLaunchAct.1")->isChecked();
    bool archivePartsOnLaunchChanged  = Preferences::archivePartsOnLaunch  != archivePartsOnLaunchCompare;

    if (archivePartsOnLaunchChanged) {
        QSettings Settings;
        QString const archivePartsOnLaunchKey("ArchivePartsOnLaunch");
        Settings.setValue(QString("%1/%2").arg(SETTINGS,archivePartsOnLaunchKey), Preferences::archivePartsOnLaunch);
        emit messageSig(LOG_INFO,QString("Archive search files on launch is %1").arg(Preferences::archivePartsOnLaunch? "ON" : "OFF"));
    }
}

void Gui::archivePartsOnDemand() {
    loadLDSearchDirParts(false/*Process*/, true/*OnDemand*/, false/*Update*/);
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

        connect(listThread,           SIGNAL(started()),
                colourPartListWorker, SLOT(  generateCustomColourPartsList()));
        connect(colourPartListWorker, SIGNAL(colorPartsListResultSig(int)),
                this,                 SLOT(  workerJobResult(int)));
        connect(listThread,           SIGNAL(finished()),
                listThread,           SLOT(  deleteLater()));
        connect(colourPartListWorker, SIGNAL(colourPartListFinishedSig()),
                this,                 SLOT(  reloadModelFileAfterColorFileGen()));
        connect(colourPartListWorker, SIGNAL(colourPartListFinishedSig()),
                listThread,           SLOT(  quit()));
        connect(colourPartListWorker, SIGNAL(colourPartListFinishedSig()),
                colourPartListWorker, SLOT(  deleteLater()));
        connect(this,                 SIGNAL(requestEndThreadNowSig()),
                colourPartListWorker, SLOT(  requestEndThreadNow()));

        connect(colourPartListWorker, SIGNAL(progressBarInitSig()),
                this,                 SLOT(  progressBarPermInit()));
        connect(colourPartListWorker, SIGNAL(progressMessageSig(const QString &)),
                this,                 SLOT(  progressBarPermSetText(const QString &)));
        connect(colourPartListWorker, SIGNAL(progressRangeSig(int,int)),
                this,                 SLOT(  progressBarPermSetRange(int,int)));
        connect(colourPartListWorker, SIGNAL(progressSetValueSig(int)),
                this,                 SLOT(  progressBarPermSetValue(int)));
        connect(colourPartListWorker, SIGNAL(progressResetSig()),
                this,                 SLOT(  progressBarPermReset()));
        connect(colourPartListWorker, SIGNAL(progressStatusRemoveSig()),
                this,                 SLOT(  progressPermStatusRemove()));

        listThread->start();

        if (!Preferences::modeGUI) {
            QEventLoop  *wait = new QEventLoop();
            wait->connect(colourPartListWorker, SIGNAL(colourPartListFinishedSig()), wait, SLOT(quit()));
            wait->exec();
        }

    } else {
      return;
    }
}

void Gui::processFadeColourParts(bool overwrite, bool setup)
{
    partWorkerCustomColour = new PartWorker();

    connect(this,                   SIGNAL(operateFadeParts(bool, bool)),
            partWorkerCustomColour, SLOT(processFadeColourParts(bool, bool)));

    connect(partWorkerCustomColour, SIGNAL(progressBarInitSig()),
            this,                   SLOT( progressBarInit()));
    connect(partWorkerCustomColour, SIGNAL(progressMessageSig(const QString &)),
            this,                   SLOT( progressBarSetText(const QString &)));
    connect(partWorkerCustomColour, SIGNAL(progressRangeSig(int,int)),
            this,                   SLOT( progressBarSetRange(int,int)));
    connect(partWorkerCustomColour, SIGNAL(progressSetValueSig(int)),
            this,                   SLOT( progressBarSetValue(int)));
    connect(partWorkerCustomColour, SIGNAL(progressResetSig()),
            this,                   SLOT( progressBarReset()));
    connect(partWorkerCustomColour, SIGNAL(progressStatusRemoveSig()),
            this,                   SLOT( progressStatusRemove()));

    //qDebug() << qPrintable(QString("Sent overwrite fade parts = %1").arg(overwrite ? "True" : "False"));
    partWorkerCustomColour->setDoFadeStep(setup);
    emit operateFadeParts(overwrite, setup);
}

void Gui::processHighlightColourParts(bool overwrite, bool setup)
{
    partWorkerCustomColour = new PartWorker();

    connect(this,                   SIGNAL(operateHighlightParts(bool, bool)),
            partWorkerCustomColour, SLOT(  processHighlightColourParts(bool, bool)));

    connect(partWorkerCustomColour, SIGNAL(progressBarInitSig()),
            this,                   SLOT(  progressBarInit()));
    connect(partWorkerCustomColour, SIGNAL(progressMessageSig(const QString &)),
            this,                   SLOT(  progressBarSetText(const QString &)));
    connect(partWorkerCustomColour, SIGNAL(progressRangeSig(int,int)),
            this,                   SLOT(  progressBarSetRange(int,int)));
    connect(partWorkerCustomColour, SIGNAL(progressSetValueSig(int)),
            this,                   SLOT(  progressBarSetValue(int)));
    connect(partWorkerCustomColour, SIGNAL(progressResetSig()),
            this,                   SLOT(  progressBarReset()));
    connect(partWorkerCustomColour, SIGNAL(progressStatusRemoveSig()),
            this,                   SLOT(  progressStatusRemove()));

    //qDebug() << qPrintable(QString("Sent overwrite highlight parts = %1").arg(overwriteCustomParts ? "True" : "False"));
    partWorkerCustomColour->setDoHighlightStep(setup);
    emit operateHighlightParts(overwrite, setup);
}

bool Gui::installRenderer(int which)
{
    bool result = false;

    const QString renderer = rendererNames[which];

#if defined Q_OS_WIN
    if (!Preferences::portableDistribution) {
        QSettings Settings;
        if (Settings.contains(QString("%1/%2").arg(SETTINGS,"RendererApplicationDir"))) {
            Preferences::lpub3d3rdPartyAppDir = Settings.value(QString("%1/%2").arg(SETTINGS,"RendererApplicationDir")).toString();;
        } else {
            QString userLocalDataPath;
            QStringList dataPathList;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
            dataPathList = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
            dataPathList = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
#else
            dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
#endif
            userLocalDataPath = dataPathList.first();
#endif
            Preferences::lpub3d3rdPartyAppDir = QFileDialog::getExistingDirectory(nullptr,
                                                                                  QFileDialog::tr("Select Renderer Directory"),
                                                                                  userLocalDataPath,
                                                                                  QFileDialog::ShowDirsOnly |
                                                                                  QFileDialog::DontResolveSymlinks);

            if (Preferences::lpub3d3rdPartyAppDir.isEmpty() || !QDir(Preferences::lpub3d3rdPartyAppDir).exists())
                return result;
        }
    }
#elif defined Q_OS_LINUX
    QSettings Settings;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,"RendererExecutableDir"))) {
        Preferences::lpub3d3rdPartyAppExeDir = Settings.value(QString("%1/%2").arg(SETTINGS,"RendererExecutableDir")).toString();;
    } else {
        Preferences::lpub3d3rdPartyAppExeDir = Preferences::lpub3d3rdPartyAppDir;
    }
#endif

    // Download 3rd party renderer
    emit messageSig(LOG_STATUS, QString("Download renderer %1...").arg(renderer));
    QTemporaryDir tempDir;
    QString downloadPath = tempDir.path();
    UpdateCheck *rendererDownload;
    QString rendererArchive, destination;

    switch (which)
    {
    case RENDERER_LDVIEW:
        rendererDownload = new UpdateCheck(this, (void*)LDViewRendererDownload);
        destination = QDir::toNativeSeparators(tr("%1/%2").arg(Preferences::lpub3d3rdPartyAppDir, VER_LDVIEW_STR));
        break;
    case RENDERER_LDGLITE:
        rendererDownload = new UpdateCheck(this, (void*)LDGLiteRendererDownload);
        destination = QDir::toNativeSeparators(tr("%1/%2").arg(Preferences::lpub3d3rdPartyAppDir, VER_LDGLITE_STR));
        break;
    case RENDERER_POVRAY:
        rendererDownload = new UpdateCheck(this, (void*)POVRayRendererDownload);
        destination = QDir::toNativeSeparators(tr("%1/%2").arg(Preferences::lpub3d3rdPartyAppDir, VER_POVRAY_STR));
        break;
    default:
        return result;
    }

    QEventLoop *wait = new QEventLoop();
    wait->connect(rendererDownload, SIGNAL(rendererDownloadFinished(QString)), wait, SLOT(quit()));
    wait->connect(rendererDownload, SIGNAL(cancel()),                          wait, SLOT(quit()));
    rendererDownload->requestDownload(rendererDownload->getDEFS_URL(), downloadPath);
    wait->exec();
    if (rendererDownload->getCancelled()) {
        return result;
    }
    rendererDownload->getDownloadReturn(rendererArchive);
    if (!QFileInfo(rendererArchive).exists()) {
        return result;
    }

    // Automatically extract renderer archive
    QString message   = tr("Extracting renderer %1. Please wait...").arg(renderer);
    emit messageSig(LOG_STATUS,message);

    QStringList items = JlCompress::getFileList(rendererArchive);
    m_progressDialog = new ProgressDialog(nullptr);
    m_progressDialog->setWindowFlags(m_progressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
    m_progressDialog->setWindowTitle(QString("Installing renderer %1").arg(renderer));
    m_progressDialog->progressBarSetLabelText(QString("Extracting renderer %1 from %2...")
                                              .arg(renderer)
                                              .arg(QFileInfo(rendererArchive).fileName()));
    m_progressDialog->progressBarSetRange(0,items.count());
    m_progressDialog->setAutoHide(true);
    m_progressDialog->setModal(true);
    m_progressDialog->show();

    QThread *thread    = new QThread(this);
    ExtractWorker *job = new ExtractWorker(rendererArchive,destination);
    job->moveToThread(thread);
    wait = new QEventLoop();

    disconnect(m_progressDialog, SIGNAL (cancelClicked()),
            this, SLOT (cancelExporting()));
    connect(thread, SIGNAL(started()),
            job, SLOT(doWork()));
    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()));
    connect(job, SIGNAL(progressSetValue(int)),
            m_progressDialog, SLOT(progressBarSetValue(int)));
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
        message = tr("%1 of %2 %3 renderer files installed to %4")
                     .arg(m_workerJobResult)
                     .arg(items.count())
                     .arg(renderer)
                     .arg(destination);
        emit messageSig(LOG_INFO,message);

        result = true;

#if defined Q_OS_WIN
    if (!Preferences::portableDistribution){
        QSettings Settings;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"RendererApplicationDir"),Preferences::lpub3d3rdPartyAppDir);
    }
#elif defined Q_OS_LINUX
    Settings.setValue(QString("%1/%2").arg(SETTINGS,"RendererExecutableDir"),Preferences::lpub3d3rdPartyAppExeDir);
#endif

        Preferences::rendererPreferences();

    } else {
        message = tr("Failed to extract %1 %2 renderer files")
                     .arg(QFileInfo(rendererArchive).fileName()
                     .arg(renderer));
        emit messageSig(LOG_ERROR,message);
    }

    connect (m_progressDialog, SIGNAL(cancelClicked()),
             this,             SLOT(  cancelExporting()));

    m_progressDialog->hide();

    emit messageSig(LOG_STATUS, QString("Download renderer %1 completed.").arg(renderer));

    return result;
}

// Update parts archive from LDSearch directories

void Gui::loadLDSearchDirParts(bool Process, bool OnDemand, bool Update) {
  if (Process)
    partWorkerLDSearchDirs.ldsearchDirPreferences();

  QStringList items = Preferences::ldSearchDirs;
  if (items.count()) {
      QString message = tr("Archiving search directory parts. Please wait...");
      emit messageSig(LOG_STATUS,message);
      m_progressDialog->setWindowFlags(m_progressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
      m_progressDialog->setWindowTitle(QString("Archive Library Update"));
      m_progressDialog->progressBarSetLabelText(QString("Archiving search directory parts..."));
      m_progressDialog->progressBarSetRange(0,items.count());
      m_progressDialog->setAutoHide(true);
      m_progressDialog->setModal(true);
      m_progressDialog->show();

      QThread *thread = new QThread(this);
      PartWorker *job = &partWorkerLDSearchDirs;
      if (OnDemand)
          job = new PartWorker(true/*OnDemand*/);
      job->moveToThread(thread);
      QEventLoop *wait = new QEventLoop();

      disconnect (m_progressDialog, SIGNAL (cancelClicked()),
                  this,  SLOT (cancelExporting()));
      if (Update)
          connect(thread, SIGNAL(started()),
                  job, SLOT(updateLDSearchDirsParts()));
      else
          connect(thread, SIGNAL(started()),
                  job, SLOT(processLDSearchDirParts()));
      connect(thread, SIGNAL(finished()),
              thread, SLOT(deleteLater()));
      connect(job, SIGNAL(progressSetValueSig(int)),
              m_progressDialog, SLOT(progressBarSetValue(int)));
      connect(m_progressDialog, SIGNAL(cancelClicked()),
              job, SLOT(requestEndThreadNow()));
      connect(job, SIGNAL(progressMessageSig (QString)),
              m_progressDialog, SLOT(progressBarSetLabelText(QString)));
      connect(job, SIGNAL(partsArchiveResultSig(int)),
              this, SLOT(workerJobResult(int)));
      connect(this, SIGNAL(requestEndThreadNowSig()),
              job, SLOT(requestEndThreadNow()));
      connect(job, SIGNAL(partsArchiveFinishedSig()),
              thread, SLOT(quit()));
      if (OnDemand)
          connect(job, SIGNAL(partsArchiveFinishedSig()),
                  job, SLOT(deleteLater()));
      wait->connect(job, SIGNAL(partsArchiveFinishedSig()),
              wait, SLOT(quit()));

      workerJobResult(0);
      thread->start();
      wait->exec();

      m_progressDialog->progressBarSetValue(items.count());

      QString partsLabel = m_workerJobResult == 1 ? "part" : "parts";
      message = tr("Added %1 %2 into Unofficial library archive %3")
                   .arg(m_workerJobResult)
                   .arg(partsLabel)
                   .arg(Preferences::validLDrawCustomArchive);
      emit messageSig(LOG_INFO,message);

      connect (m_progressDialog, SIGNAL (cancelClicked()),
               this, SLOT (cancelExporting()));

      m_progressDialog->hide();
  }

  if (! getCurFile().isEmpty()) {
      bool _continue;
      if (Preferences::saveOnRedraw) {
          _continue = maybeSave(false); // No prompt
      } else {
          _continue = maybeSave(true, SaveOnNone);
      }
      if (!_continue)
          return;

      timer.start();

      clearPLICache();
      clearCSICache();
      clearSubmodelCache();
      clearTempCache();

      //reload current model file
      cyclePageDisplay(displayPageNum, FILE_RELOAD);

      emit messageSig(LOG_STATUS, QString("All caches reset and model file reloaded (%1 parts). %2")
                      .arg(lpub->ldrawFile.getPartCount())
                      .arg(elapsedTime(timer.elapsed())));
  }
}

void Gui::refreshLDrawUnoffParts() {

    // Download unofficial archive
    emit messageSig(LOG_STATUS,"Refresh LDraw Unofficial Library archive...");
    QTemporaryDir tempDir;
    QString downloadPath = tempDir.path();
    UpdateCheck *libraryDownload;
    libraryDownload      = new UpdateCheck(this, (void*)LDrawUnofficialLibraryDownload);
    QEventLoop  *wait    = new QEventLoop();
    wait->connect(libraryDownload, SIGNAL(rendererDownloadFinished(QString)), wait, SLOT(quit()));
    wait->connect(libraryDownload, SIGNAL(cancel()),                          wait, SLOT(quit()));
    libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), downloadPath);
    wait->exec();
    if (libraryDownload->getCancelled()) {
        return;
    }
    QString newarchive;
    libraryDownload->getDownloadReturn(newarchive);
    if (!QFileInfo(newarchive).exists()) {
        return;
    }

    // Automatically extract Unofficial archive
    QString destination = QDir::toNativeSeparators(tr("%1/unofficial").arg(Preferences::ldrawLibPath));
    QString message     = tr("Extracting Unofficial archive library. Please wait...");
    emit messageSig(LOG_STATUS,message);

    QStringList items = JlCompress::getFileList(newarchive);
    m_progressDialog = new ProgressDialog(nullptr);
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

    disconnect(m_progressDialog, SIGNAL (cancelClicked()),
            this, SLOT (cancelExporting()));
    connect(thread, SIGNAL(started()),
            job, SLOT(doWork()));
    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()));
    connect(job, SIGNAL(progressSetValue(int)),
            m_progressDialog, SLOT(progressBarSetValue(int)));
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
    if (items.count()) {
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

     m_progressDialog->hide();

    // Unload LDraw Unofficial archive library
    UnloadUnofficialPiecesLibrary();

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
    wait->connect(libraryDownload, SIGNAL(rendererDownloadFinished(QString)), wait, SLOT(quit()));
    wait->connect(libraryDownload, SIGNAL(cancel()),                          wait, SLOT(quit()));
    libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), downloadPath);
    wait->exec();
    if (libraryDownload->getCancelled()) {
        return;
    }
    QString newarchive;
    libraryDownload->getDownloadReturn(newarchive);
    if (!QFileInfo(newarchive).exists()) {
        return;
    }

    // Automatically extract Official archive
    QString destination = QDir::toNativeSeparators(Preferences::ldrawLibPath);
    destination         = destination.remove(destination.size() - 6,6);
    QString message = tr("Extracting Official archive library. Please wait...");
    emit messageSig(LOG_INFO_STATUS,message);

    QStringList items = JlCompress::getFileList(newarchive);
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

    disconnect(m_progressDialog, SIGNAL (cancelClicked()),
            this, SLOT (cancelExporting()));
    connect(thread, SIGNAL(started()),
            job, SLOT(doWork()));
    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()));
    connect(job, SIGNAL(progressSetValue(int)),
            m_progressDialog, SLOT(progressBarSetValue(int)));
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

    connect (m_progressDialog, SIGNAL (cancelClicked()),
             this, SLOT (cancelExporting()));

     m_progressDialog->hide();

    // Unload LDraw Official archive libraries
    UnloadOfficialPiecesLibrary();

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

void Gui::commandsDialog()
{
    CommandsDialog::showCommandsDialog();
}

QString MetaCommandsFileDialog::getCommandsSaveFileName(
        bool &exportDirections,
        QWidget *parent,
        const QString &caption,
        const QString &dir,
        const QString &filter,
        QString *selectedFilter,
        Options options)
{
    QSettings Settings;
    QString const settingsKey("ExportCommandDescriptions");

    bool exportDirectionsChecked = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,settingsKey)))
       exportDirectionsChecked =  Settings.value(QString("%1/%2").arg(SETTINGS,settingsKey)).toBool();

    QEventLoop loop;
    QUrl selectedUrl;
    const QUrl dirUrl = QUrl::fromLocalFile(dir);
    const QStringList supportedSchemes;

    QSharedPointer<MetaCommandsFileDialog> fileDialog(new MetaCommandsFileDialog(parent, caption, dirUrl.toLocalFile(), filter));
    fileDialog->setFileMode(AnyFile);
    fileDialog->setOptions(options);
    fileDialog->setSupportedSchemes(supportedSchemes);
    fileDialog->setAcceptMode(AcceptSave);
    fileDialog->setOption(DontUseNativeDialog, true);
    if (selectedFilter && !selectedFilter->isEmpty()) {
        fileDialog->selectNameFilter(*selectedFilter);
    }

    QSharedPointer<QCheckBox> exportDescriptionsBox(new QCheckBox(fileDialog.data()));
    exportDescriptionsBox->setChecked(exportDirectionsChecked);
    exportDescriptionsBox->setText(tr("Export user-defined command descriptions"));

    QSharedPointer<QGridLayout> layout(qobject_cast<QGridLayout *>(fileDialog->layout()));
    layout->addWidget(exportDescriptionsBox.data(), 4, 1);

    fileDialog->connect(fileDialog.data(), &QFileDialog::accepted, [&] {
        if (selectedFilter) {
            *selectedFilter = fileDialog->selectedNameFilter();
        }

        exportDirectionsChecked = exportDescriptionsBox->isChecked();
        Settings.setValue(QString("%1/%2").arg(SETTINGS,settingsKey), exportDirectionsChecked);

        exportDirections = exportDirectionsChecked;
        selectedUrl = fileDialog->selectedUrls().value(0);
    });

    fileDialog->connect(fileDialog.data(), &QFileDialog::finished,
                  [&](int) { loop.exit(); });

    fileDialog->open();

    // Non blocking wait
    loop.exec(QEventLoop::DialogExec);

    return selectedUrl.toLocalFile();
}

void Gui::exportMetaCommands()
{
  static const QString defaultFileName(QDir::currentPath() + QDir::separator() + VER_PRODUCTNAME_STR + "_Metacommands.txt");

  bool withDescriptions = false;
  QString fileName = MetaCommandsFileDialog::getCommandsSaveFileName(
              withDescriptions,
              this,
              tr("Meta Commands Save File Name (*.txt):"),
              defaultFileName,
              tr("txt (*.txt);; all(*.*)"));

  if (fileName.isEmpty())
    return;

  QString result;
  if (!lpub->exportMetaCommands(fileName, result, withDescriptions))
      return;

  if (!Preferences::modeGUI)
      return;

  //display completion message
  QPixmap _icon = QPixmap(":/icons/lpub96.png");
  QMessageBoxResizable box;
  box.setWindowIcon(QIcon());
  box.setIconPixmap (_icon);
  box.setTextFormat (Qt::RichText);
  box.setStandardButtons (QMessageBox::Yes| QMessageBox::No);
  box.setDefaultButton   (QMessageBox::Yes);
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  box.setWindowTitle(tr ("Export Meta Commands"));
  box.setText (tr ("<b> %1 </b>").arg(result));
  box.setInformativeText (tr ("Do you want to open this document ?\n\n%1")
                              .arg(QDir::toNativeSeparators(fileName)));

  if (box.exec() == QMessageBox::Yes) {
    QString CommandPath = fileName;
    QProcess *Process = new QProcess(this);
    Process->setWorkingDirectory(QFileInfo(fileName).absolutePath() + QDir::separator());

#ifdef Q_OS_WIN
    Process->setNativeArguments(CommandPath);
    if (Preferences::usingNPP) {
        const QStringList arguments = QStringList() << CommandPath << QLatin1String(WINDOWS_NPP_LPUB3D_UDL_ARG);
        Process->startDetached(Preferences::systemEditor, arguments);
    } else
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
  } else {
      emit messageSig(LOG_INFO_STATUS, result);
  }
}

void Gui::createOpenWithActions(int maxPrograms)
{
    // adjust for separator and system editor
    const int systemEditor = Preferences::systemEditor.isEmpty() ? 0 : 1;
    const int maxOpenWithPrograms = maxPrograms ? maxPrograms + systemEditor : Preferences::maxOpenWithPrograms + systemEditor;
    for (int i = 0; i < maxOpenWithPrograms; i++) {
        QAction *openWithAct = new QAction(tr("Open With Application %1").arg(i),this);
        openWithAct->setObjectName(tr("openWith%1Act.1").arg(i));
        openWithAct->setVisible(false);
        lpub->actions.insert(openWithAct->objectName(), Action(tr("File.Open With Application %1").arg(i), openWithAct));
        connect(openWithAct, SIGNAL(triggered()), this, SLOT(openWith()));
        if (i < openWithActList.size()) {
            openWithActList.replace(i,openWithAct);
        } else {
            openWithActList.append(openWithAct);
        }
    }
}

void SetActionShortcut(QAction *action, const QKeySequence &shortcut)
{
    action->setShortcut(shortcut);
    action->setProperty("defaultshortcut", shortcut);
}

QAction *Gui::getApplyBuildModAct()
{
    return ApplyBuildModAct;
}

QAction *Gui::getRemoveBuildModAct()
{
    return RemoveBuildModAct;
}

QAction *Gui::getDeleteBuildModAct()
{
    return DeleteBuildModAct;
}

QAction *Gui::getAct(const QString &objectName)
{
    if (lpub->actions.contains(objectName))
        return lpub->actions.value(objectName).action;
#ifdef QT_DEBUG_MODE
    emit lpub->messageSig(LOG_ERROR, QString("Mainwindow action was not found or is null [%1]").arg(objectName));
#endif
    return nullptr;
}

void Gui::createActions()
{
    // File
    QAction *openAct = new QAction(QIcon(":/resources/open.png"), tr("&Open..."), this);
    openAct->setObjectName("openAct.1");
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open an existing file"));
    lpub->actions.insert(openAct->objectName(), Action(tr("File.Open"), openAct));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    createOpenWithActions();

    QAction *saveAct = new QAction(QIcon(":/resources/save.png"), tr("&Save"), this);
    saveAct->setObjectName("saveAct.1");
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the document to disk"));
    saveAct->setEnabled(false);
    lpub->actions.insert(saveAct->objectName(), Action(tr("File.Save"), saveAct));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    QAction *saveAsAct = new QAction(QIcon(":/resources/saveas.png"),tr("Save A&s..."), this);
    saveAsAct->setObjectName("saveAsAct.1");
    saveAsAct->setShortcut(tr("Ctrl+Shift+S"));
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    saveAsAct->setEnabled(false);
    lpub->actions.insert(saveAsAct->objectName(), Action(tr("File.Save As"), saveAsAct));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    QAction *saveCopyAct = new QAction(QIcon(":/resources/savecopy.png"),tr("Save a Copy As..."), this);
    saveCopyAct->setObjectName("saveCopyAct.1");
    saveCopyAct->setShortcut(tr("Ctrl+Shift+C"));
    saveCopyAct->setStatusTip(tr("Save a copy of the document under a new name"));
    saveCopyAct->setEnabled(false);
    lpub->actions.insert(saveCopyAct->objectName(), Action(tr("File.Save A Copy As"), saveCopyAct));
    connect(saveCopyAct, SIGNAL(triggered()), this, SLOT(saveCopy()));

    QAction *closeFileAct = new QAction(QIcon(":/resources/closemodelfile.png"), tr("Close File"), this);
    closeFileAct->setObjectName("closeFileAct.1");
    closeFileAct->setShortcut(tr("Ctrl+W"));
    closeFileAct->setStatusTip(tr("Close current model file"));
    closeFileAct->setEnabled(false);
    lpub->actions.insert(closeFileAct->objectName(), Action(tr("File.Close File"), closeFileAct));
    connect(closeFileAct, SIGNAL(triggered()), this, SLOT(closeModelFile()));

    QIcon printToFilePreviewIcon;
    printToFilePreviewIcon.addFile(":/resources/file_print_preview.png");
    printToFilePreviewIcon.addFile(":/resources/file_print_preview_16.png");
    QAction *printToFilePreviewAct = new QAction(printToFilePreviewIcon, tr("File Print Pre&view..."), this);
    printToFilePreviewAct->setObjectName("printToFilePreviewAct.1");
    printToFilePreviewAct->setShortcut(tr("Alt+Shift+R"));
    printToFilePreviewAct->setStatusTip(tr("Preview the current document to be printed"));
    printToFilePreviewAct->setEnabled(false);
    lpub->actions.insert(printToFilePreviewAct->objectName(), Action(tr("File.File Print Preview"), printToFilePreviewAct));
    connect(printToFilePreviewAct, SIGNAL(triggered()), this, SLOT(TogglePrintToFilePreview()));

    QIcon printToFileIcon;
    printToFileIcon.addFile(":/resources/file_print.png");
    printToFileIcon.addFile(":/resources/file_print_16.png");
    QAction *printToFileAct = new QAction(printToFileIcon, tr("File &Print..."), this);
    printToFileAct->setObjectName("printToFileAct.1");
    printToFileAct->setShortcut(tr("Alt+Shift+P"));
    printToFileAct->setStatusTip(tr("Print the current document"));
    printToFileAct->setEnabled(false);
    lpub->actions.insert(printToFileAct->objectName(), Action(tr("File.File Print"), printToFileAct));
    connect(printToFileAct, SIGNAL(triggered()), this, SLOT(ShowPrintDialog()));

    QAction *importLDDAct = new QAction(QIcon(":/resources/importldd.png"),tr("LEGO Digital Designer File..."), this);
    importLDDAct->setObjectName("importLDDAct.1");
    importLDDAct->setShortcut(tr("Alt+Shift+L"));
    importLDDAct->setStatusTip(tr("Import LEGO Digital Designer File"));
    lpub->actions.insert(importLDDAct->objectName(), Action(tr("File.Import.LEGO Digital Designer File"), importLDDAct));
    connect(importLDDAct, SIGNAL(triggered()), this, SLOT(importLDD()));

    QAction *importSetInventoryAct = new QAction(QIcon(":/resources/importsetinventory.png"),tr("Set Inventory File..."), this);
    importSetInventoryAct->setObjectName("importSetInventoryAct.1");
    importSetInventoryAct->setShortcut(tr("Alt+Shift+S"));
    importSetInventoryAct->setStatusTip(tr("Import Rebrickable Set Inventory File"));
    lpub->actions.insert(importSetInventoryAct->objectName(), Action(tr("File.Import.Set Inventory File"), importSetInventoryAct));
    connect(importSetInventoryAct, SIGNAL(triggered()), this, SLOT(importInventory()));

    QAction *exportAsPdfPreviewAct = new QAction(QIcon(":/resources/pdf_print_preview.png"), tr("PDF Export Preview..."), this);
    exportAsPdfPreviewAct->setObjectName("exportAsPdfPreviewAct.1");
    exportAsPdfPreviewAct->setShortcut(tr("Alt+P"));
    exportAsPdfPreviewAct->setStatusTip(tr("Preview the current pdf document to be exported"));
    exportAsPdfPreviewAct->setEnabled(false);
    lpub->actions.insert(exportAsPdfPreviewAct->objectName(), Action(tr("File.PDF Export Preview"), exportAsPdfPreviewAct));
    connect(exportAsPdfPreviewAct, SIGNAL(triggered()), this, SLOT(TogglePdfExportPreview()));

    QAction *exportAsPdfAct = new QAction(QIcon(":/resources/pdf_logo.png"), tr("Export to PDF &File..."), this);
    exportAsPdfAct->setObjectName("exportAsPdfAct.1");
    exportAsPdfAct->setShortcut(tr("Alt+F"));
    exportAsPdfAct->setStatusTip(tr("Export your document to a pdf file"));
    exportAsPdfAct->setEnabled(false);
    lpub->actions.insert(exportAsPdfAct->objectName(), Action(tr("File.Export To PDF File"), exportAsPdfAct));
    connect(exportAsPdfAct, SIGNAL(triggered()), this, SLOT(exportAsPdfDialog()));

    QAction *exportPngAct = new QAction(QIcon(":/resources/exportpng.png"),tr("P&NG Images..."), this);
    exportPngAct->setObjectName("exportPngAct.1");
    exportPngAct->setShortcut(tr("Alt+N"));
    exportPngAct->setStatusTip(tr("Export your document as a sequence of PNG images"));
    exportPngAct->setEnabled(false);
    lpub->actions.insert(exportPngAct->objectName(), Action(tr("File.Export As.PNG Images"), exportPngAct));
    connect(exportPngAct, SIGNAL(triggered()), this, SLOT(exportAsPngDialog()));

    QAction *exportJpgAct = new QAction(QIcon(":/resources/exportjpeg.png"),tr("&JPEG Images..."), this);
    exportJpgAct->setObjectName("exportJpgAct.1");
    exportJpgAct->setShortcut(tr("Alt+J"));
    exportJpgAct->setStatusTip(tr("Export your document as a sequence of JPEG images"));
    exportJpgAct->setEnabled(false);
    lpub->actions.insert(exportJpgAct->objectName(), Action(tr("File.Export As.JPEG Images"), exportJpgAct));
    connect(exportJpgAct, SIGNAL(triggered()), this, SLOT(exportAsJpgDialog()));

    QAction *exportBmpAct = new QAction(QIcon(":/resources/exportbmp.png"),tr("&Bitmap Images..."), this);
    exportBmpAct->setObjectName("exportBmpAct.1");
    exportBmpAct->setShortcut(tr("Alt+B"));
    exportBmpAct->setStatusTip(tr("Export your document as a sequence of bitmap images"));
    exportBmpAct->setEnabled(false);
    lpub->actions.insert(exportBmpAct->objectName(), Action(tr("File.Export As.Bitmap Images"), exportBmpAct));
    connect(exportBmpAct, SIGNAL(triggered()), this, SLOT(exportAsBmpDialog()));

    QAction *exportHtmlAct = new QAction(QIcon(":/resources/html32.png"),tr("&HTML Part List..."), this);
    exportHtmlAct->setObjectName("exportHtmlAct.1");
    exportHtmlAct->setShortcut(tr("Alt+6"));
    exportHtmlAct->setStatusTip(tr("Export your document as an HTML part list"));
    exportHtmlAct->setEnabled(false);
    lpub->actions.insert(exportHtmlAct->objectName(), Action(tr("File.Export As.HTML Part List"), exportHtmlAct));
    connect(exportHtmlAct, SIGNAL(triggered()), this, SLOT(exportAsHtml()));

    QAction *exportHtmlStepsAct = new QAction(QIcon(":/resources/htmlsteps32.png"),tr("HT&ML Steps..."), this);
    exportHtmlStepsAct->setObjectName("exportHtmlStepsAct.1");
    exportHtmlStepsAct->setShortcut(tr("Alt+Shift+6"));
    exportHtmlStepsAct->setStatusTip(tr("Export your document as navigatable steps in HTML format"));
    exportHtmlStepsAct->setEnabled(false);
    lpub->actions.insert(exportHtmlStepsAct->objectName(), Action(tr("File.Export As.HTML Steps"), exportHtmlStepsAct));
    connect(exportHtmlStepsAct, SIGNAL(triggered()), this, SLOT(exportAsHtmlSteps()));

    QAction *exportColladaAct = new QAction(QIcon(":/resources/dae32.png"),tr("&COLLADA Objects..."), this);
    exportColladaAct->setObjectName("exportColladaAct.1");
    exportColladaAct->setShortcut(tr("Alt+4"));
    exportColladaAct->setStatusTip(tr("Export your document as a sequence of Collada objects"));
    exportColladaAct->setEnabled(false);
    lpub->actions.insert(exportColladaAct->objectName(), Action(tr("File.Export As.COLLADA Objects"), exportColladaAct));
    connect(exportColladaAct, SIGNAL(triggered()), this, SLOT(exportAsColladaDialog()));

    QAction *exportObjAct = new QAction(QIcon(":/resources/obj32.png"),tr("&Wavefront Objects..."), this);
    exportObjAct->setObjectName("exportObjAct.1");
    exportObjAct->setShortcut(tr("Alt+1"));
    exportObjAct->setStatusTip(tr("Export your document as a sequence of Wavefront objects"));
    exportObjAct->setEnabled(false);
    lpub->actions.insert(exportObjAct->objectName(), Action(tr("File.Export As.Wavefront Objects"), exportObjAct));
    connect(exportObjAct, SIGNAL(triggered()), this, SLOT(exportAsObjDialog()));

    QAction *export3dsAct = new QAction(QIcon(":/resources/3ds32.png"),tr("&3DStudio Objects..."), this);
    export3dsAct->setObjectName("export3dsAct.1");
    export3dsAct->setShortcut(tr("Alt+5"));
    export3dsAct->setStatusTip(tr("Export your document as a sequence of 3DStudio objects"));
    export3dsAct->setEnabled(false);
    lpub->actions.insert(export3dsAct->objectName(), Action(tr("File.Export As.3DStudio Objects"), export3dsAct));
    connect(export3dsAct, SIGNAL(triggered()), this, SLOT(exportAs3dsDialog()));

    QAction *exportStlAct = new QAction(QIcon(":/resources/stl32.png"),tr("&Stereo Lithography Objects..."), this);
    exportStlAct->setObjectName("exportStlAct.1");
    exportStlAct->setShortcut(tr("Alt+2"));
    exportStlAct->setStatusTip(tr("Export your document as a sequence of Stereo Lithography Objects"));
    exportStlAct->setEnabled(false);
    lpub->actions.insert(exportStlAct->objectName(), Action(tr("File.Export As.Stereo Lithography Objects"), exportStlAct));
    connect(exportStlAct, SIGNAL(triggered()), this, SLOT(exportAsStlDialog()));

    QAction *exportPovAct = new QAction(QIcon(":/resources/povray32.png"),tr("&PovRay Scene Files..."), this);
    exportPovAct->setObjectName("exportPovAct.1");
    exportPovAct->setShortcut(tr("Alt+3"));
    exportPovAct->setStatusTip(tr("Export your document as a sequence of PovRay Scene Files"));
    exportPovAct->setEnabled(false);
    lpub->actions.insert(exportPovAct->objectName(), Action(tr("File.Export As.PovRay Scene Files"), exportPovAct));
    connect(exportPovAct, SIGNAL(triggered()), this, SLOT(exportAsPovDialog()));

    QAction *exportCsvAct = new QAction(QIcon(":/resources/csv32.png"),tr("&CSV Part List..."), this);
    exportCsvAct->setObjectName("exportCsvAct.1");
    exportCsvAct->setShortcut(tr("Alt+8"));
    exportCsvAct->setStatusTip(tr("Export your document as a CSV part list"));
    exportCsvAct->setEnabled(false);
    lpub->actions.insert(exportCsvAct->objectName(), Action(tr("File.Export As.CSV Part List"), exportCsvAct));
    connect(exportCsvAct, SIGNAL(triggered()), this, SLOT(exportAsCsv()));

    QAction *exportBricklinkAct = new QAction(QIcon(":/resources/bricklink32.png"),tr("&Bricklink XML Part List..."), this);
    exportBricklinkAct->setObjectName("exportBricklinkAct.1");
    exportBricklinkAct->setShortcut(tr("Alt+7"));
    exportBricklinkAct->setStatusTip(tr("Export your document as a Bricklink XML Part List"));
    exportBricklinkAct->setEnabled(false);
    lpub->actions.insert(exportBricklinkAct->objectName(), Action(tr("File.Export As.Bricklink XML Part List"), exportBricklinkAct));
    connect(exportBricklinkAct, SIGNAL(triggered()), this, SLOT(exportAsBricklinkXML()));

    QAction *exitAct = new QAction(QIcon(":/resources/exit.png"),tr("E&xit"), this);
    exitAct->setObjectName("exitAct.1");
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    lpub->actions.insert(exitAct->objectName(), Action(tr("File.Exit"), exitAct));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    QIcon blenderIcon;
    blenderIcon.addFile(":/resources/blendericon.png");
    blenderIcon.addFile(":/resources/blendericon16.png");
    blenderRenderAct = new QAction(blenderIcon,tr("Blender Render..."), this);
    blenderRenderAct->setObjectName("blenderRenderAct.4");
    blenderRenderAct->setShortcut(tr("Alt+Shift+8"));
    blenderRenderAct->setStatusTip(tr("Render the current step using Blender Cycles"));
    blenderRenderAct->setEnabled(false);
    lpub->actions.insert(blenderRenderAct->objectName(), Action(tr("3DViewer.Blender Render"), blenderRenderAct));
    connect(blenderRenderAct, SIGNAL(triggered()), this, SLOT(showRenderDialog()));

    blenderImportAct = new QAction(blenderIcon,tr("Blender Import..."), this);
    blenderImportAct->setObjectName("blenderImportAct.4");
    blenderImportAct->setShortcut(tr("Alt+Shift+9"));
    blenderImportAct->setStatusTip(tr("Import the current step and launch Blender"));
    blenderImportAct->setEnabled(false);
    lpub->actions.insert(blenderImportAct->objectName(), Action(tr("3DViewer.Blender Import"), blenderImportAct));
    connect(blenderImportAct, SIGNAL(triggered()), this, SLOT(showRenderDialog()));
    
    povrayRenderAct = new QAction(QIcon(":/resources/povray32.png"),tr("POVRay Render..."), this);
    povrayRenderAct->setObjectName("povrayRenderAct.4");
    povrayRenderAct->setShortcut(tr("Alt+9"));
    povrayRenderAct->setStatusTip(tr("Render the current model using POV-Ray"));
    povrayRenderAct->setEnabled(false);
    lpub->actions.insert(povrayRenderAct->objectName(), Action(tr("3DViewer.POVRay Render"), povrayRenderAct));
    connect(povrayRenderAct, SIGNAL(triggered()), this, SLOT(showRenderDialog()));

    QAction *copyFilePathToClipboardAct = new QAction(QIcon(":/resources/copytoclipboard.png"),tr("Full Path to Clipboard"), this);
    copyFilePathToClipboardAct->setObjectName("copyFilePathToClipboardAct.1");
    copyFilePathToClipboardAct->setShortcut(tr("Alt+Shift+0"));
    copyFilePathToClipboardAct->setStatusTip(tr("Copy current model full file path to clipboard"));
    copyFilePathToClipboardAct->setEnabled(false);
    lpub->actions.insert(copyFilePathToClipboardAct->objectName(), Action(tr("File.Full Path To Clipboard"), copyFilePathToClipboardAct));
    connect(copyFilePathToClipboardAct, SIGNAL(triggered()), this, SLOT(updateClipboard()));

    for (int i = 0; i < MAX_RECENT_FILES; i++) {
      recentFilesActs[i] = new QAction(this);
      recentFilesActs[i]->setObjectName(tr("recentFile%1Act.1").arg(i));
      recentFilesActs[i]->setStatusTip(tr("Clear recent file %1").arg(i));
      recentFilesActs[i]->setVisible(false);
      lpub->actions.insert(recentFilesActs[i]->objectName(), Action(tr("Recent Files.Recent File %1").arg(i), recentFilesActs[i]));
      connect(recentFilesActs[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));
    }

    QAction *clearRecentAct = new QAction(tr("Clear Recent Files"),this);
    clearRecentAct->setObjectName("clearRecentAct.1");
    clearRecentAct->setStatusTip(tr("Clear recent files"));
    lpub->actions.insert(clearRecentAct->objectName(), Action(tr("Recent Files.Clear Recent Files"), clearRecentAct));
    connect(clearRecentAct, SIGNAL(triggered()), this, SLOT(clearRecentFiles()));

    // Edit
    undoAct = new QAction(QIcon(":/resources/editundo.png"), tr("&Undo"), this);
    undoAct->setObjectName("undoAct.1");
    undoAct->setShortcut(tr("Ctrl+Z"));
    undoAct->setStatusTip(tr("Undo last change"));
    undoAct->setEnabled(false);
    lpub->actions.insert(undoAct->objectName(), Action(tr("Edit.Undo"), undoAct));
    connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));

    redoAct = new QAction(QIcon(":/resources/editredo.png"), tr("&Redo"), this);
    redoAct->setObjectName("redoAct.1");
#ifdef __APPLE__
    redoAct->setShortcut(tr("Ctrl+Shift+Z"));
    redoAct->setStatusTip(tr("Redo last change"));
#else
    redoAct->setShortcut(tr("Ctrl+Y"));
    redoAct->setStatusTip(tr("Redo last change"));
#endif
    redoAct->setEnabled(false);
    lpub->actions.insert(redoAct->objectName(), Action(tr("Edit.Redo"), redoAct));
    connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));

    QAction *insertCoverPageAct = new QAction(QIcon(":/resources/insertcoverpage.png"),tr("Insert Front Cover Page"),this);
    insertCoverPageAct->setObjectName("insertCoverPageAct.1");
    insertCoverPageAct->setShortcut(tr("Ctrl+Alt+F"));
    insertCoverPageAct->setStatusTip(tr("Insert a front cover page"));
    insertCoverPageAct->setEnabled(false);
    lpub->actions.insert(insertCoverPageAct->objectName(), Action(tr("Edit.Insert Front Cover Page"), insertCoverPageAct));
    connect(insertCoverPageAct, SIGNAL(triggered()), this, SLOT(insertCoverPage()));

    QAction *appendCoverPageAct = new QAction(QIcon(":/resources/appendcoverpage.png"),tr("Append Back Cover Page"),this);
    appendCoverPageAct->setObjectName("appendCoverPageAct.1");
    appendCoverPageAct->setShortcut(tr("Ctrl+Alt+B"));
    appendCoverPageAct->setStatusTip(tr("Append back cover page"));
    appendCoverPageAct->setEnabled(false);
    lpub->actions.insert(appendCoverPageAct->objectName(), Action(tr("Edit.Append Back Cover Page"), appendCoverPageAct));
    connect(appendCoverPageAct, SIGNAL(triggered()), this, SLOT(appendCoverPage()));

    QAction *insertNumberedPageAct = new QAction(QIcon(":/resources/insertpage.png"),tr("Insert Page"),this);
    insertNumberedPageAct->setObjectName("insertNumberedPageAct.1");
    insertNumberedPageAct->setShortcut(tr("Ctrl+Alt+I"));
    insertNumberedPageAct->setStatusTip(tr("Insert a numbered page"));
    insertNumberedPageAct->setEnabled(false);
    lpub->actions.insert(insertNumberedPageAct->objectName(), Action(tr("Edit.Insert Page"), insertNumberedPageAct));
    connect(insertNumberedPageAct, SIGNAL(triggered()), this, SLOT(insertNumberedPage()));

    QAction *appendNumberedPageAct = new QAction(QIcon(":/resources/appendpage.png"),tr("Append Page"),this);
    appendNumberedPageAct->setObjectName("appendNumberedPageAct.1");
    appendNumberedPageAct->setShortcut(tr("Ctrl+Alt+A"));
    appendNumberedPageAct->setStatusTip(tr("Append a numbered page"));
    appendNumberedPageAct->setEnabled(false);
    lpub->actions.insert(appendNumberedPageAct->objectName(), Action(tr("Edit.Append Page"), appendNumberedPageAct));
    connect(appendNumberedPageAct, SIGNAL(triggered()), this, SLOT(appendNumberedPage()));

    QAction *deletePageAct = new QAction(QIcon(":/resources/deletepage.png"),tr("Delete Page"),this);
    deletePageAct->setObjectName("deletePageAct.1");
    deletePageAct->setShortcut(tr("Ctrl+Alt+D"));
    deletePageAct->setStatusTip(tr("Delete this page"));
    deletePageAct->setEnabled(false);
    lpub->actions.insert(deletePageAct->objectName(), Action(tr("Edit.Delete Page"), deletePageAct));
    connect(deletePageAct, SIGNAL(triggered()), this, SLOT(deletePage()));

    QAction *addPictureAct = new QAction(QIcon(":/resources/addpicture.png"),tr("Add Picture"),this);
    addPictureAct->setObjectName("addPictureAct.1");
    addPictureAct->setShortcut(tr("Ctrl+Alt+P"));
    addPictureAct->setStatusTip(tr("Add a picture to this page"));
    addPictureAct->setEnabled(false);
    lpub->actions.insert(addPictureAct->objectName(), Action(tr("Edit.Add Picture"), addPictureAct));
    connect(addPictureAct, SIGNAL(triggered()), this, SLOT(addPicture()));

    QAction *addTextAct = new QAction(QIcon(":/resources/addtext.png"),tr("Add Text"),this);
    addTextAct->setObjectName("addTextAct.1");
    addTextAct->setShortcut(tr("Ctrl+Alt+T"));
    addTextAct->setStatusTip(tr("Add text to this page"));
    addTextAct->setEnabled(false);
    lpub->actions.insert(addTextAct->objectName(), Action(tr("Edit.Add Text"), addTextAct));
    connect(addTextAct, SIGNAL(triggered()), this, SLOT(addText()));

    QAction *addBomAct = new QAction(QIcon(":/resources/addbom.png"),tr("Add Bill of Materials"),this);
    addBomAct->setObjectName("addBomAct.1");
    addBomAct->setShortcut(tr("Ctrl+Alt+M"));
    addBomAct->setStatusTip(tr("Add Bill of Materials to this page"));
    addBomAct->setEnabled(false);
    lpub->actions.insert(addBomAct->objectName(), Action(tr("Edit.Add Bill Of Materials"), addBomAct));
    connect(addBomAct, SIGNAL(triggered()), this, SLOT(addBom()));

    QAction *cycleEachPageAct = new QAction(QIcon(":/resources/cycleeachpage.png"),tr("Cycle Each Page"), this);
    cycleEachPageAct->setObjectName("cycleEachPageAct.1");
    cycleEachPageAct->setStatusTip(tr("Cycle each page step(s) when navigating forward by more than one page - click to enable"));
    cycleEachPageAct->setCheckable(true);
    cycleEachPageAct->setChecked(Preferences::cycleEachPage);
    lpub->actions.insert(cycleEachPageAct->objectName(), Action(tr("Edit.Cycle Each Page"), cycleEachPageAct));
    connect(cycleEachPageAct, SIGNAL(triggered()), this, SLOT(cycleEachPage()));

    QAction *removeLPubFormatBomAct = new QAction(QIcon(":/resources/removelpubformatbom.png"),tr("Unformat Bill of Materials"),this);
    removeLPubFormatBomAct->setObjectName("removeLPubFormatBomAct.1");
    removeLPubFormatBomAct->setShortcut(tr("Ctrl+Alt+J"));
    removeLPubFormatBomAct->setStatusTip(tr("Remove all LPub Bill of Materials metacommands"));
    removeLPubFormatBomAct->setEnabled(false);
    lpub->actions.insert(removeLPubFormatBomAct->objectName(), Action(tr("Edit.Remove LPub Format.Unformat Bill Of Materials"), removeLPubFormatBomAct));
    connect(removeLPubFormatBomAct, SIGNAL(triggered()), this, SLOT(removeLPubFormatting()));

    QAction *removeLPubFormatDocumentAct = new QAction(QIcon(":/resources/removelpubformatdocument.png"),tr("Unformat Document"),this);
    removeLPubFormatDocumentAct->setObjectName("removeLPubFormatDocumentAct.1");
    removeLPubFormatDocumentAct->setShortcut(tr("Ctrl+Alt+R"));
    removeLPubFormatDocumentAct->setStatusTip(tr("Remove all LPub metacommands from entire document"));
    removeLPubFormatDocumentAct->setEnabled(false);
    lpub->actions.insert(removeLPubFormatDocumentAct->objectName(), Action(tr("Edit.Remove LPub Format.Unformat Document"), removeLPubFormatDocumentAct));
    connect(removeLPubFormatDocumentAct, SIGNAL(triggered()), this, SLOT(removeLPubFormatting()));

    QAction *removeLPubFormatPageAct = new QAction(QIcon(":/resources/removelpubformatpage.png"),tr("Unformat Current Page"),this);
    removeLPubFormatPageAct->setObjectName("removeLPubFormatPageAct.1");
    removeLPubFormatPageAct->setShortcut(tr("Ctrl+Alt+E"));
    removeLPubFormatPageAct->setStatusTip(tr("Remove all LPub metacommands from the current page"));
    removeLPubFormatPageAct->setEnabled(false);
    lpub->actions.insert(removeLPubFormatPageAct->objectName(), Action(tr("Edit.Remove LPub Format.Unformat Current Page"), removeLPubFormatPageAct));
    connect(removeLPubFormatPageAct, SIGNAL(triggered()), this, SLOT(removeLPubFormatting()));

    QAction *removeLPubFormatStepAct = new QAction(QIcon(":/resources/removelpubformatstep.png"),tr("Unformat Single Step"),this);
    removeLPubFormatStepAct->setObjectName("removeLPubFormatStepAct.1");
    removeLPubFormatStepAct->setShortcut(tr("Ctrl+Alt+L"));
    removeLPubFormatStepAct->setStatusTip(tr("Remove all LPub metacommands from the current step"));
    removeLPubFormatStepAct->setEnabled(false);
    lpub->actions.insert(removeLPubFormatStepAct->objectName(), Action(tr("Edit.Remove LPub Format.Unformat Single Step"), removeLPubFormatStepAct));
    connect(removeLPubFormatStepAct, SIGNAL(triggered()), this, SLOT(removeLPubFormatting()));

    QAction *removeLPubFormatSubmodelAct = new QAction(QIcon(":/resources/removelpubformatsubmodel.png"),tr("Unformat Current Submodel"),this);
    removeLPubFormatSubmodelAct->setObjectName("removeLPubFormatSubmodelAct.1");
    removeLPubFormatSubmodelAct->setShortcut(tr("Ctrl+Alt+H"));
    removeLPubFormatSubmodelAct->setStatusTip(tr("Remove all LPub metacommands from the current submodel"));
    removeLPubFormatSubmodelAct->setEnabled(false);
    lpub->actions.insert(removeLPubFormatSubmodelAct->objectName(), Action(tr("Edit.Remove LPub Format.Unformat Current Submodel"), removeLPubFormatSubmodelAct));
    connect(removeLPubFormatSubmodelAct, SIGNAL(triggered()), this, SLOT(removeLPubFormatting()));

    QAction *removeBuildModFormatAct = new QAction(tr("Remove Build Mod Format"),this);
    removeBuildModFormatAct->setObjectName("removeBuildModFormatAct.1");
    removeBuildModFormatAct->setStatusTip(tr("Remove build modification formatting including modified part lines."));
    removeBuildModFormatAct->setCheckable(true);
    removeBuildModFormatAct->setEnabled(Preferences::buildModEnabled);
    removeBuildModFormatAct->setChecked(Preferences::removeBuildModFormat);
    lpub->actions.insert(removeBuildModFormatAct->objectName(), Action(tr("Edit.Remove LPub Format.Remove Build Mod Format"), removeBuildModFormatAct));
    connect(removeBuildModFormatAct, SIGNAL(triggered()), this, SLOT(removeBuildModFormat()));

    QAction *removeChildSubmodelFormatAct = new QAction(tr("Remove Child Submodel Format"),this);
    removeChildSubmodelFormatAct->setObjectName("removeChildSubmodelFormatAct.1");
    removeChildSubmodelFormatAct->setStatusTip(tr("Remove child submodel formatting for submodel, step, page or callout removals."));
    removeChildSubmodelFormatAct->setCheckable(true);
    removeChildSubmodelFormatAct->setChecked(Preferences::removeChildSubmodelFormat);
    lpub->actions.insert(removeChildSubmodelFormatAct->objectName(), Action(tr("Edit.Remove LPub Format.Remove Child Submodel Format"), removeChildSubmodelFormatAct));
    connect(removeChildSubmodelFormatAct, SIGNAL(triggered()), this, SLOT(removeChildSubmodelFormat()));

    // View
    QAction *fitWidthAct = new QAction(QIcon(":/resources/fitWidth.png"), tr("Fit &Width"), this);
    fitWidthAct->setObjectName("fitWidthAct.1");
    fitWidthAct->setShortcut(tr("Alt+W"));
    fitWidthAct->setStatusTip(tr("Fit document to width"));
    fitWidthAct->setEnabled(false);
    lpub->actions.insert(fitWidthAct->objectName(), Action(tr("View.Fit Width"), fitWidthAct));
    connect(fitWidthAct, SIGNAL(triggered()), this, SLOT(fitWidth()));

    QAction *fitVisibleAct = new QAction(QIcon(":/resources/fitVisible.png"), tr("Fit &Visible"), this);
    fitVisibleAct->setObjectName("fitVisibleAct.1");
    fitVisibleAct->setShortcut(tr("Alt+V"));
    fitVisibleAct->setStatusTip(tr("Fit document so whole page is visible"));
    fitVisibleAct->setEnabled(false);
    lpub->actions.insert(fitVisibleAct->objectName(), Action(tr("View.Fit Visible"), fitVisibleAct));
    connect(fitVisibleAct, SIGNAL(triggered()), this, SLOT(fitVisible()));

    QAction *fitSceneAct = new QAction(QIcon(":/resources/fitScene.png"), tr("Fit &Scene"), this);
    fitSceneAct->setObjectName("fitSceneAct.1");
    fitSceneAct->setShortcut(tr("Alt+H"));
    fitSceneAct->setStatusTip(tr("Fit document so whole scene is visible"));
    fitSceneAct->setEnabled(false);
    lpub->actions.insert(fitSceneAct->objectName(), Action(tr("View.Fit Scene"), fitSceneAct));
    connect(fitSceneAct, SIGNAL(triggered()), this, SLOT(fitScene()));

    QAction *bringToFrontAct = new QAction(QIcon(":/resources/bringtofront.png"), tr("Bring To &Front"), this);
    bringToFrontAct->setObjectName("bringToFrontAct.1");
    bringToFrontAct->setShortcut(tr("Alt+Shift+F"));
    bringToFrontAct->setStatusTip(tr("Bring item to front"));
    bringToFrontAct->setEnabled(false);
    lpub->actions.insert(bringToFrontAct->objectName(), Action(tr("View.Bring To Front"), bringToFrontAct));
    connect(bringToFrontAct, SIGNAL(triggered()), this, SLOT(bringToFront()));

    QAction *sendToBackAct = new QAction(QIcon(":/resources/sendtoback.png"), tr("Send To &Back"), this);
    sendToBackAct->setObjectName("sendToBackAct.1");
    sendToBackAct->setShortcut(tr("Alt+Shift+B"));
    sendToBackAct->setStatusTip(tr("Send item to back"));
    sendToBackAct->setEnabled(false);
    lpub->actions.insert(sendToBackAct->objectName(), Action(tr("View.Send To Back"), sendToBackAct));
    connect(sendToBackAct, SIGNAL(triggered()), this, SLOT(sendToBack()));

    QAction *actualSizeAct = new QAction(QIcon(":/resources/actual.png"),tr("&Actual Size"), this);
    actualSizeAct->setObjectName("actualSizeAct.1");
    actualSizeAct->setShortcut(tr("Alt+A"));
    actualSizeAct->setStatusTip(tr("Show document actual size"));
    actualSizeAct->setEnabled(false);
    lpub->actions.insert(actualSizeAct->objectName(), Action(tr("View.Actual Size"), actualSizeAct));
    connect(actualSizeAct, SIGNAL(triggered()), this, SLOT(actualSize()));

    // TESTING ONLY
    //connect(actualSizeAct, SIGNAL(triggered()), this, SLOT(twoPages()));

    QAction *fullScreenViewAct = new QAction(QIcon(":/resources/fullscreenview.png"),tr("Full Screen View"),this);
    fullScreenViewAct->setObjectName("fullScreenViewAct.1");
    fullScreenViewAct->setShortcut(tr("Ctrl+M"));
    fullScreenViewAct->setStatusTip(tr("Toggle full screen view"));
    fullScreenViewAct->setCheckable(true);
    lpub->actions.insert(fullScreenViewAct->objectName(), Action(tr("View.Full Screen View"), fullScreenViewAct));
    connect(fullScreenViewAct, SIGNAL(triggered()), this, SLOT(fullScreenView()));

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

    QAction *zoomInComboAct = new QAction(QIcon(":/resources/zoomin.png"), tr("&Zoom In"), this);
    zoomInComboAct->setObjectName("zoomInComboAct.1");
    zoomInComboAct->setShortcut(tr("Ctrl++"));
    zoomInComboAct->setStatusTip(tr("Zoom in"));
    zoomInComboAct->setEnabled(false);
    lpub->actions.insert(zoomInComboAct->objectName(), Action(tr("View.Zoom In"), zoomInComboAct));
    connect(zoomInComboAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    QAction *zoomOutComboAct = new QAction(QIcon(":/resources/zoomout.png"),tr("Zoom &Out"),this);
    zoomOutComboAct->setObjectName("zoomOutComboAct.1");
    zoomOutComboAct->setShortcut(tr("Ctrl+-"));
    zoomOutComboAct->setStatusTip(tr("Zoom out"));
    zoomOutComboAct->setEnabled(false);
    lpub->actions.insert(zoomOutComboAct->objectName(), Action(tr("View.Zoom Out"), zoomOutComboAct));
    connect(zoomOutComboAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    QAction *sceneRulerComboAct = new QAction(QIcon(":/resources/pageruler.png"), tr("Scene &Ruler"), this);
    sceneRulerComboAct->setObjectName("sceneRulerComboAct.1");
    sceneRulerComboAct->setShortcut(tr("Alt+U"));
    sceneRulerComboAct->setStatusTip(tr("Toggle the scene ruler"));
    sceneRulerComboAct->setEnabled(true);
    sceneRulerComboAct->setCheckable(true);
    sceneRulerComboAct->setChecked(Preferences::sceneRuler);
    lpub->actions.insert(sceneRulerComboAct->objectName(), Action(tr("View.Scene Ruler"), sceneRulerComboAct));
    connect(sceneRulerComboAct, SIGNAL(triggered()), this, SLOT(sceneRuler()));

    QAction *hideRulerPageBackgroundAct = new QAction(tr("Hide Page Background"),this);
    hideRulerPageBackgroundAct->setObjectName("hideRulerPageBackgroundAct.1");
    hideRulerPageBackgroundAct->setStatusTip(tr("Toggle hide ruler page background"));
    hideRulerPageBackgroundAct->setCheckable(true);
    hideRulerPageBackgroundAct->setChecked(Preferences::hidePageBackground);
    hideRulerPageBackgroundAct->setEnabled(Preferences::sceneRuler);
    lpub->actions.insert(hideRulerPageBackgroundAct->objectName(), Action(tr("View.Scene Ruler.Hide Page Background"), hideRulerPageBackgroundAct));
    connect(hideRulerPageBackgroundAct, SIGNAL(triggered()), this, SLOT(hidePageBackground()));

    QAction *sceneRulerTrackingNoneAct = new QAction(tr("Ruler Tracking Off"),this);
    sceneRulerTrackingNoneAct->setObjectName("sceneRulerTrackingNoneAct.1");
    sceneRulerTrackingNoneAct->setStatusTip(tr("Set scene ruler tracking Off"));
    sceneRulerTrackingNoneAct->setCheckable(true);
    lpub->actions.insert(sceneRulerTrackingNoneAct->objectName(), Action(tr("View.Scene Ruler.Ruler Tracking Off"), sceneRulerTrackingNoneAct));
    connect(sceneRulerTrackingNoneAct, SIGNAL(triggered()), this, SLOT(sceneRulerTracking()));

    QAction *sceneRulerTrackingTickAct = new QAction(tr("Ruler Tracking Tick"),this);
    sceneRulerTrackingTickAct->setObjectName("sceneRulerTrackingTickAct.1");
    sceneRulerTrackingTickAct->setStatusTip(tr("Set scene ruler tracking to use ruler tick mark"));
    sceneRulerTrackingTickAct->setCheckable(true);
    lpub->actions.insert(sceneRulerTrackingTickAct->objectName(), Action(tr("View.Scene Ruler.Ruler Tracking Tick"), sceneRulerTrackingTickAct));
    connect(sceneRulerTrackingTickAct, SIGNAL(triggered()), this, SLOT(sceneRulerTracking()));

    QAction *sceneRulerTrackingLineAct = new QAction(tr("Ruler Tracking Line"),this);
    sceneRulerTrackingLineAct->setObjectName("sceneRulerTrackingLineAct.1");
    sceneRulerTrackingLineAct->setStatusTip(tr("Set scene ruler tracking to use full scene line"));
    sceneRulerTrackingLineAct->setCheckable(true);
    lpub->actions.insert(sceneRulerTrackingLineAct->objectName(), Action(tr("View.Scene Ruler.Ruler Tracking Line"), sceneRulerTrackingLineAct));
    connect(sceneRulerTrackingLineAct, SIGNAL(triggered()), this, SLOT(sceneRulerTracking()));

    QAction *showTrackingCoordinatesAct = new QAction(tr("Show Tracking Coordinates"),this);
    showTrackingCoordinatesAct->setObjectName("showTrackingCoordinatesAct.1");
    showTrackingCoordinatesAct->setStatusTip(tr("Toggle show ruler tracking coordinates"));
    showTrackingCoordinatesAct->setCheckable(true);
    showTrackingCoordinatesAct->setChecked(Preferences::showTrackingCoordinates);
    showTrackingCoordinatesAct->setEnabled(Preferences::sceneRulerTracking == int(TRACKING_LINE));
    lpub->actions.insert(showTrackingCoordinatesAct->objectName(), Action(tr("View.Scene Ruler.Show Tracking Coordinates"), showTrackingCoordinatesAct));
    connect(showTrackingCoordinatesAct, SIGNAL(triggered()), this, SLOT(showCoordinates()));

    SceneRulerGroup = new QActionGroup(this);
    SceneRulerGroup->setEnabled(Preferences::sceneRuler);
    sceneRulerTrackingNoneAct->setChecked(Preferences::sceneRulerTracking == int(TRACKING_NONE));
    SceneRulerGroup->addAction(sceneRulerTrackingNoneAct);
    sceneRulerTrackingTickAct->setChecked(Preferences::sceneRulerTracking == int(TRACKING_TICK));
    SceneRulerGroup->addAction(sceneRulerTrackingTickAct);
    sceneRulerTrackingLineAct->setChecked(Preferences::sceneRulerTracking == int(TRACKING_LINE));
    SceneRulerGroup->addAction(sceneRulerTrackingLineAct);

    QAction *sceneGuidesComboAct = new QAction(QIcon(":/resources/pageguides.png"), tr("Scene &Guides"), this);
    sceneGuidesComboAct->setObjectName("sceneGuidesComboAct.1");
    sceneGuidesComboAct->setShortcut(tr("Alt+G"));
    sceneGuidesComboAct->setStatusTip(tr("Toggle horizontal and vertical scene guides"));
    sceneGuidesComboAct->setEnabled(true);
    sceneGuidesComboAct->setCheckable(true);
    sceneGuidesComboAct->setChecked(Preferences::sceneGuides);
    lpub->actions.insert(sceneGuidesComboAct->objectName(), Action(tr("View.Scene Guides"), sceneGuidesComboAct));
    connect(sceneGuidesComboAct, SIGNAL(triggered()), this, SLOT(sceneGuides()));

    QAction *sceneGuidesDashLineAct = new QAction(tr("Dash Line"),this);
    sceneGuidesDashLineAct->setObjectName("sceneGuidesDashLineAct.1");
    sceneGuidesDashLineAct->setStatusTip(tr("Select dash scene guide lines"));
    sceneGuidesDashLineAct->setCheckable(true);
    lpub->actions.insert(sceneGuidesDashLineAct->objectName(), Action(tr("View.Scene Guides.Dash Line"), sceneGuidesDashLineAct));
    connect(sceneGuidesDashLineAct, SIGNAL(triggered()), this, SLOT(sceneGuidesLine()));

    QAction *sceneGuidesSolidLineAct = new QAction(tr("Solid Line"),this);
    sceneGuidesSolidLineAct->setObjectName("sceneGuidesSolidLineAct.1");
    sceneGuidesSolidLineAct->setStatusTip(tr("Select solid scene guide lines"));
    sceneGuidesSolidLineAct->setCheckable(true);
    lpub->actions.insert(sceneGuidesSolidLineAct->objectName(), Action(tr("View.Scene Guides.Solid Line"), sceneGuidesSolidLineAct));
    connect(sceneGuidesSolidLineAct, SIGNAL(triggered()), this, SLOT(sceneGuidesLine()));

    SceneGuidesLineGroup = new QActionGroup(this);
    SceneGuidesLineGroup->setEnabled(Preferences::sceneGuides);
    sceneGuidesDashLineAct->setChecked(Preferences::sceneGuidesLine == int(Qt::DashLine));
    SceneGuidesLineGroup->addAction(sceneGuidesDashLineAct);
    sceneGuidesSolidLineAct->setChecked(Preferences::sceneGuidesLine == int(Qt::SolidLine));
    SceneGuidesLineGroup->addAction(sceneGuidesSolidLineAct);

    QAction *sceneGuidesPosTLeftAct = new QAction(tr("Top Left"),this);
    sceneGuidesPosTLeftAct->setObjectName("sceneGuidesPosTLeftAct.1");
    sceneGuidesPosTLeftAct->setStatusTip(tr("Set scene guides position to top left of graphic item"));
    sceneGuidesPosTLeftAct->setCheckable(true);
    lpub->actions.insert(sceneGuidesPosTLeftAct->objectName(), Action(tr("View.Scene Guides.Top Left"), sceneGuidesPosTLeftAct));
    connect(sceneGuidesPosTLeftAct, SIGNAL(triggered()), this, SLOT(sceneGuidesPosition()));

    QAction *sceneGuidesPosTRightAct = new QAction(tr("Top Right"),this);
    sceneGuidesPosTRightAct->setObjectName("sceneGuidesPosTRightAct.1");
    sceneGuidesPosTRightAct->setStatusTip(tr("Set scene guides position to top right of graphic item"));
    sceneGuidesPosTRightAct->setCheckable(true);
    lpub->actions.insert(sceneGuidesPosTRightAct->objectName(), Action(tr("View.Scene Guides.Top Right"), sceneGuidesPosTRightAct));
    connect(sceneGuidesPosTRightAct, SIGNAL(triggered()), this, SLOT(sceneGuidesPosition()));

    QAction *sceneGuidesPosBLeftAct = new QAction(tr("Bottom Left"),this);
    sceneGuidesPosBLeftAct->setObjectName("sceneGuidesPosBLeftAct.1");
    sceneGuidesPosBLeftAct->setStatusTip(tr("Set scene guides position to bottom left of graphic item"));
    sceneGuidesPosBLeftAct->setCheckable(true);
    lpub->actions.insert(sceneGuidesPosBLeftAct->objectName(), Action(tr("View.Scene Guides.Bottom Left"), sceneGuidesPosBLeftAct));
    connect(sceneGuidesPosBLeftAct, SIGNAL(triggered()), this, SLOT(sceneGuidesPosition()));

    QAction *sceneGuidesPosBRightAct = new QAction(tr("Bottom Right"),this);
    sceneGuidesPosBRightAct->setObjectName("sceneGuidesPosBRightAct.1");
    sceneGuidesPosBRightAct->setStatusTip(tr("Set scene guides position to bottom right of graphic item"));
    sceneGuidesPosBRightAct->setCheckable(true);
    lpub->actions.insert(sceneGuidesPosBRightAct->objectName(), Action(tr("View.Scene Guides.Bottom Right"), sceneGuidesPosBRightAct));
    connect(sceneGuidesPosBRightAct, SIGNAL(triggered()), this, SLOT(sceneGuidesPosition()));

    QAction *sceneGuidesPosCentreAct = new QAction(tr("Centre"),this);
    sceneGuidesPosCentreAct->setObjectName("sceneGuidesPosCentreAct.1");
    sceneGuidesPosCentreAct->setStatusTip(tr("Set scene guides position to centre of graphic item"));
    sceneGuidesPosCentreAct->setCheckable(true);
    lpub->actions.insert(sceneGuidesPosCentreAct->objectName(), Action(tr("View.Scene Guides.Centre"), sceneGuidesPosCentreAct));
    connect(sceneGuidesPosCentreAct, SIGNAL(triggered()), this, SLOT(sceneGuidesPosition()));

    QAction *showGuidesCoordinatesAct = new QAction(tr("Show Guide Coordinates"),this);
    showGuidesCoordinatesAct->setObjectName("showGuidesCoordinatesAct.1");
    showGuidesCoordinatesAct->setStatusTip(tr("Toggle show scene guide coordinates"));
    showGuidesCoordinatesAct->setCheckable(true);
    showGuidesCoordinatesAct->setChecked(Preferences::showGuidesCoordinates);
    showGuidesCoordinatesAct->setEnabled(Preferences::sceneGuides);
    lpub->actions.insert(showGuidesCoordinatesAct->objectName(), Action(tr("View.Scene Guides.Show Guide Coordinates"), showGuidesCoordinatesAct));
    connect(showGuidesCoordinatesAct, SIGNAL(triggered()), this, SLOT(showCoordinates()));

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

    QAction *snapToGridComboAct = new QAction(QIcon(":/resources/scenegrid.png"),tr("&Snap To Grid"),this);
    snapToGridComboAct->setObjectName("snapToGridComboAct.1");
    snapToGridComboAct->setShortcut(tr("Alt+K"));
    snapToGridComboAct->setStatusTip(tr("Toggle snap-to-grid"));
    snapToGridComboAct->setCheckable(true);
    snapToGridComboAct->setChecked(Preferences::snapToGrid);
    lpub->actions.insert(snapToGridComboAct->objectName(), Action(tr("View.Snap To Grid"), snapToGridComboAct));
    connect(snapToGridComboAct, SIGNAL(triggered()), this, SLOT(snapToGrid()));

    QAction *hideGridPageBackgroundAct = new QAction(tr("Hide Page Background"),this);
    hideGridPageBackgroundAct->setObjectName("hideGridPageBackgroundAct.1");
    hideGridPageBackgroundAct->setStatusTip(tr("Toggle hide snap to grid page background"));
    hideGridPageBackgroundAct->setCheckable(true);
    hideGridPageBackgroundAct->setChecked(Preferences::hidePageBackground);
    hideGridPageBackgroundAct->setEnabled(Preferences::snapToGrid);
    lpub->actions.insert(hideGridPageBackgroundAct->objectName(), Action(tr("View.Snap To Grid.Hide Page Background"), hideGridPageBackgroundAct));
    connect(hideGridPageBackgroundAct, SIGNAL(triggered()), this, SLOT(hidePageBackground()));

    GridStepSizeGroup = new QActionGroup(this);
    GridStepSizeGroup->setEnabled(Preferences::snapToGrid);
    for (int ActionIdx = GRID_SIZE_FIRST; ActionIdx < GRID_SIZE_LAST; ActionIdx++)
    {
        QAction *Action = new QAction(qApp->translate("Menu", sgCommands[ActionIdx].MenuName), this);
        Action->setStatusTip(qApp->translate("Status", sgCommands[ActionIdx].StatusText));
        connect(Action, SIGNAL(triggered()), this, SLOT(gridSizeTriggered()));
        addAction(Action);
        snapGridActions[ActionIdx] = Action;
        snapGridActions[ActionIdx]->setCheckable(true);
        GridStepSizeGroup->addAction(snapGridActions[ActionIdx]);
    }

    snapGridActions[Preferences::gridSizeIndex]->setChecked(true);

    // Tools
    QAction *firstPageAct = new QAction(QIcon(":/resources/first.png"),tr("First Page"), this);
    firstPageAct->setObjectName("firstPageAct.1");
    firstPageAct->setShortcut(tr("Ctrl+P"));
    firstPageAct->setStatusTip(tr("Go to first page of document"));
    firstPageAct->setEnabled(false);
    lpub->actions.insert(firstPageAct->objectName(), Action(tr("Navigation.First Page"), firstPageAct));
    connect(firstPageAct, SIGNAL(triggered()), this, SLOT(firstPage()));

    QAction *lastPageAct = new QAction(QIcon(":/resources/last.png"),tr("Last Page"), this);
    lastPageAct->setObjectName("lastPageAct.1");
    lastPageAct->setShortcut(tr("Ctrl+L"));
    lastPageAct->setStatusTip(tr("Go to last page of document"));
    lastPageAct->setEnabled(false);
    lpub->actions.insert(lastPageAct->objectName(), Action(tr("Navigation.Last Page"), lastPageAct));
    connect(lastPageAct, SIGNAL(triggered()), this, SLOT(lastPage()));

    QAction *nextPageAct = new QAction(QIcon(":/resources/next.png"),tr("&Next Page"),this);
    nextPageAct->setObjectName("nextPageAct.1");
    nextPageAct->setShortcut(tr("Ctrl+N"));
    nextPageAct->setStatusTip(tr("Go to next page of document"));
    nextPageAct->setEnabled(false);
    lpub->actions.insert(nextPageAct->objectName(), Action(tr("Navigation.Next Page"), nextPageAct));
    connect(nextPageAct, SIGNAL(triggered()), this, SLOT(nextPage()));

    QAction *previousPageAct = new QAction(QIcon(":/resources/prev.png"),tr("&Previous Page"),this);
    previousPageAct->setObjectName("previousPageAct.1");
    previousPageAct->setShortcut(tr("Ctrl+E"));
    previousPageAct->setStatusTip(tr("Go to previous page of document"));
    previousPageAct->setEnabled(false);
    lpub->actions.insert(previousPageAct->objectName(), Action(tr("Navigation.Previous Page"), previousPageAct));
    connect(previousPageAct, SIGNAL(triggered()), this, SLOT(previousPage()));

    QAction *nextPageComboAct = new QAction(QIcon(":/resources/next.png"),tr("&Next Page"),this);
    nextPageComboAct->setObjectName("nextPageComboAct.1");
    nextPageComboAct->setStatusTip(tr("Go to next page of document"));
    nextPageComboAct->setEnabled(false);
    lpub->actions.insert(nextPageComboAct->objectName(), Action(tr("Navigation.Next Page Combo"), nextPageComboAct));
    connect(nextPageComboAct, SIGNAL(triggered()), this, SLOT(nextPage()));

    QAction *previousPageComboAct = new QAction(QIcon(":/resources/prev.png"),tr("&Previous Page"),this);
    previousPageComboAct->setObjectName("previousPageComboAct.1");
    previousPageComboAct->setStatusTip(tr("Go to previous page of document"));
    previousPageComboAct->setEnabled(false);
    lpub->actions.insert(previousPageComboAct->objectName(), Action(tr("Navigation.Previous Page Combo"), previousPageComboAct));
    connect(previousPageComboAct, SIGNAL(triggered()), this, SLOT(previousPage()));

    QAction *nextPageContinuousAct = new QAction(QIcon(":/resources/nextpagecontinuous.png"),tr("Continuous Next Page"),this);
    nextPageContinuousAct->setObjectName("nextPageContinuousAct.1");
    nextPageContinuousAct->setShortcut(tr("Ctrl+Shift+N"));
    nextPageContinuousAct->setStatusTip(tr("Continuously process next page to end of document"));
    nextPageContinuousAct->setEnabled(false);
    lpub->actions.insert(nextPageContinuousAct->objectName(), Action(tr("Navigation.Continuous Next Page"), nextPageContinuousAct));
    connect(nextPageContinuousAct, SIGNAL(triggered()), this, SLOT(nextPageContinuous()));

    QAction *previousPageContinuousAct = new QAction(QIcon(":/resources/prevpagecontinuous.png"),tr("Continuous Previous Page"),this);
    previousPageContinuousAct->setObjectName("previousPageContinuousAct.1");
    previousPageContinuousAct->setShortcut(tr("Ctrl+Shift+E"));
    previousPageContinuousAct->setStatusTip(tr("Continuously process previous page to start of document"));
    previousPageContinuousAct->setEnabled(false);
    lpub->actions.insert(previousPageContinuousAct->objectName(), Action(tr("Navigation.Continuous Previous Page"), previousPageContinuousAct));
    connect(previousPageContinuousAct, SIGNAL(triggered()), this, SLOT(previousPageContinuous()));

    QString pageString = "";
    setPageLineEdit = new QLineEdit(pageString,this);
    QSize size = setPageLineEdit->sizeHint();
    size.setWidth(size.width()/3);
    setPageLineEdit->setMinimumSize(size);
    setPageLineEdit->setToolTip("Current Page");
    setPageLineEdit->setStatusTip("Enter the desired page number and hit enter to go to page");
    setPageLineEdit->setEnabled(false);
    QAction *setPageLineEditResetAct = setPageLineEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
    setPageLineEditResetAct->setText(tr("Current Page Edit Reset"));
    setPageLineEditResetAct->setObjectName("setPageLineEditResetAct.1");
    setPageLineEditResetAct->setEnabled(false);
    lpub->actions.insert(setPageLineEditResetAct->objectName(), Action(tr("Navigation.Current Page Edit Reset"), setPageLineEditResetAct));
    connect(setPageLineEdit, SIGNAL(returnPressed()), this, SLOT(setPage()));
    connect(setPageLineEdit, SIGNAL(textEdited(const QString &)), this, SLOT(enablePageLineReset(const QString &)));
    connect(setPageLineEditResetAct, SIGNAL(triggered()), this, SLOT(pageLineEditReset()));

    // Configuration
    QAction *preferencesAct = new QAction(QIcon(":/resources/preferences.png"),tr("&Preferences"), this);
    preferencesAct->setObjectName("preferencesAct.1");
    preferencesAct->setStatusTip(tr("Set your preferences for LPub3D"));
    lpub->actions.insert(preferencesAct->objectName(), Action(tr("Configuration.Preferences"), preferencesAct));
    connect(preferencesAct, SIGNAL(triggered()), this, SLOT(preferences()));

    QAction *pageSetupAct = new QAction(QIcon(":/resources/pagesetup.png"),tr("Page Setup"), this);
    pageSetupAct->setObjectName("pageSetupAct.1");
    pageSetupAct->setEnabled(false);
    pageSetupAct->setStatusTip(tr("Default values for your project's pages"));
    lpub->actions.insert(pageSetupAct->objectName(), Action(tr("Configuration.Build Instructions Setup.Page Setup"), pageSetupAct));
    connect(pageSetupAct, SIGNAL(triggered()), this, SLOT(pageSetup()));

    QAction *assemSetupAct = new QAction(QIcon(":/resources/assemblysetup.png"),tr("Assembly Setup"), this);
    assemSetupAct->setObjectName("assemSetupAct.1");
    assemSetupAct->setEnabled(false);
    assemSetupAct->setStatusTip(tr("Default values for your project's assembly images"));
    lpub->actions.insert(assemSetupAct->objectName(), Action(tr("Configuration.Build Instructions Setup.Assembly Setup"), assemSetupAct));
    connect(assemSetupAct, SIGNAL(triggered()), this, SLOT(assemSetup()));

    QAction *pliSetupAct = new QAction(QIcon(":/resources/partslistsetup.png"),tr("Parts List Setup"), this);
    pliSetupAct->setObjectName("pliSetupAct.1");
    pliSetupAct->setEnabled(false);
    pliSetupAct->setStatusTip(tr("Default values for your project's parts lists"));
    lpub->actions.insert(pliSetupAct->objectName(), Action(tr("Configuration.Build Instructions Setup.Parts List Setup"), pliSetupAct));
    connect(pliSetupAct, SIGNAL(triggered()), this, SLOT(pliSetup()));

    QAction *bomSetupAct = new QAction(QIcon(":/resources/bomsetup.png"),tr("Bill of Materials Setup"), this);
    bomSetupAct->setObjectName("bomSetupAct.1");
    bomSetupAct->setEnabled(false);
    bomSetupAct->setStatusTip(tr("Default values for your project's bill of materials"));
    lpub->actions.insert(bomSetupAct->objectName(), Action(tr("Configuration.Build Instructions Setup.Bill Of Materials Setup"), bomSetupAct));
    connect(bomSetupAct, SIGNAL(triggered()), this, SLOT(bomSetup()));

    QAction *calloutSetupAct = new QAction(QIcon(":/resources/calloutsetup.png"),tr("Callout Setup"), this);
    calloutSetupAct->setObjectName("calloutSetupAct.1");
    calloutSetupAct->setEnabled(false);
    calloutSetupAct->setStatusTip(tr("Default values for your project's callouts"));
    lpub->actions.insert(calloutSetupAct->objectName(), Action(tr("Configuration.Build Instructions Setup.Callout Setup"), calloutSetupAct));
    connect(calloutSetupAct, SIGNAL(triggered()), this, SLOT(calloutSetup()));

    QAction *multiStepSetupAct = new QAction(QIcon(":/resources/stepgroupsetup.png"),tr("Step Group Setup"), this);
    multiStepSetupAct->setObjectName("multiStepSetupAct.1");
    multiStepSetupAct->setEnabled(false);
    multiStepSetupAct->setStatusTip(tr("Default values for your project's step groups"));
    lpub->actions.insert(multiStepSetupAct->objectName(), Action(tr("Configuration.Build Instructions Setup.Step Group Setup"), multiStepSetupAct));
    connect(multiStepSetupAct, SIGNAL(triggered()), this, SLOT(multiStepSetup()));

    QAction *subModelSetupAct = new QAction(QIcon(":/resources/submodelsetup.png"),tr("Submodel Preview Setup"), this);
    subModelSetupAct->setObjectName("subModelSetupAct.1");
    subModelSetupAct->setEnabled(false);
    subModelSetupAct->setStatusTip(tr("Default values for your submodel preview at first step"));
    lpub->actions.insert(subModelSetupAct->objectName(), Action(tr("Configuration.Build Instructions Setup.Submodel Preview Setup"), subModelSetupAct));
    connect(subModelSetupAct, SIGNAL(triggered()), this, SLOT(subModelSetup()));

    QAction *projectSetupAct = new QAction(QIcon(":/resources/projectsetup.png"),tr("Project Setup"), this);
    projectSetupAct->setObjectName("projectSetupAct.1");
    projectSetupAct->setEnabled(false);
    projectSetupAct->setStatusTip(tr("Default values for your project"));
    lpub->actions.insert(projectSetupAct->objectName(), Action(tr("Configuration.Build Instructions Setup.Project Setup"), projectSetupAct));
    connect(projectSetupAct, SIGNAL(triggered()), this, SLOT(projectSetup()));

    QAction *fadeStepSetupAct = new QAction(QIcon(":/resources/fadestepsetup.png"),tr("Fade Step Setup"), this);
    fadeStepSetupAct->setObjectName("fadeStepSetupAct.1");
    fadeStepSetupAct->setEnabled(false);
    fadeStepSetupAct->setStatusTip(tr("Fade parts in previous step step"));
    lpub->actions.insert(fadeStepSetupAct->objectName(), Action(tr("Configuration.Build Instructions Setup.Fade Step Setup"), fadeStepSetupAct));
    connect(fadeStepSetupAct, SIGNAL(triggered()), this, SLOT(fadeStepSetup()));

    QAction *highlightStepSetupAct = new QAction(QIcon(":/resources/highlightstepsetup.png"),tr("Highlight Step Setup"), this);
    highlightStepSetupAct->setObjectName("highlightStepSetupAct.1");
    highlightStepSetupAct->setEnabled(false);
    highlightStepSetupAct->setStatusTip(tr("Highlight parts in current step"));
    lpub->actions.insert(highlightStepSetupAct->objectName(), Action(tr("Configuration.Build Instructions Setup.Highlight Step Setup"), highlightStepSetupAct));
    connect(highlightStepSetupAct, SIGNAL(triggered()), this, SLOT(highlightStepSetup()));

    QAction *useSystemEditorAct = new QAction(tr("Use System Editor"),this);
    useSystemEditorAct->setObjectName("useSystemEditorAct.1");
    useSystemEditorAct->setStatusTip(tr("Open parameter file with %1")
                                     .arg(Preferences::systemEditor.isEmpty() ? "the system editor" :
                                                                                Preferences::systemEditor));
    useSystemEditorAct->setCheckable(true);
    useSystemEditorAct->setChecked(Preferences::useSystemEditor);
    lpub->actions.insert(useSystemEditorAct->objectName(), Action(tr("Configuration.Edit Parameter Files.Use System Editor"), useSystemEditorAct));
    connect(useSystemEditorAct, SIGNAL(triggered()), this, SLOT(useSystemEditor()));

    QAction *editTitleAnnotationsAct = new QAction(QIcon(":/resources/edittitleannotations.png"),tr("Part Title PLI Annotations List"), this);
    editTitleAnnotationsAct->setObjectName("editTitleAnnotationsAct.1");
    editTitleAnnotationsAct->setStatusTip(tr("Add/Edit part title PLI part annotatons"));
    lpub->actions.insert(editTitleAnnotationsAct->objectName(), Action(tr("Configuration.Edit Parameter Files.Part Title PLI Annotations List"), editTitleAnnotationsAct));
    connect(editTitleAnnotationsAct, SIGNAL(triggered()), this, SLOT(editTitleAnnotations()));

    QAction *editFreeFormAnnitationsAct = new QAction(QIcon(":/resources/editfreeformannotations.png"),tr("Freeform PLI Annotations List"), this);
    editFreeFormAnnitationsAct->setObjectName("editFreeFormAnnitationsAct.1");
    editFreeFormAnnitationsAct->setStatusTip(tr("Add/Edit freeform PLI part annotations"));
    lpub->actions.insert(editFreeFormAnnitationsAct->objectName(), Action(tr("Configuration.Edit Parameter Files.Freeform PLI Annotations List"), editFreeFormAnnitationsAct));
    connect(editFreeFormAnnitationsAct, SIGNAL(triggered()), this, SLOT(editFreeFormAnnitations()));

    QAction *editLDrawColourPartsAct = new QAction(QIcon(":/resources/editldrawcolourparts.png"),tr("LDraw Static Color Parts List"), this);
    editLDrawColourPartsAct->setObjectName("editLDrawColourPartsAct.1");
    editLDrawColourPartsAct->setStatusTip(tr("Add/Edit the list of LDraw static color parts used to process fade and highlight steps"));
    lpub->actions.insert(editLDrawColourPartsAct->objectName(), Action(tr("Configuration.Edit Parameter Files.LDraw Static Color Parts List"), editLDrawColourPartsAct));
    connect(editLDrawColourPartsAct, SIGNAL(triggered()), this, SLOT(editLDrawColourParts()));

    QAction *editPliBomSubstitutePartsAct = new QAction(QIcon(":/resources/editplisubstituteparts.png"),tr("Substitute Parts List"), this);
    editPliBomSubstitutePartsAct->setObjectName("editPliBomSubstitutePartsAct.1");
    editPliBomSubstitutePartsAct->setStatusTip(tr("Add/Edit the list of PLI/BOM substitute parts"));
    lpub->actions.insert(editPliBomSubstitutePartsAct->objectName(), Action(tr("Configuration.Edit Parameter Files.Substitute Parts List"), editPliBomSubstitutePartsAct));
    connect(editPliBomSubstitutePartsAct, SIGNAL(triggered()), this, SLOT(editPliBomSubstituteParts()));

    QAction *editExcludedPartsAct = new QAction(QIcon(":/resources/editexcludedparts.png"),tr("Part Count Excluded Parts List"), this);
    editExcludedPartsAct->setObjectName("editExcludedPartsAct.1");
    editExcludedPartsAct->setStatusTip(tr("Add/Edit the list of part count excluded parts"));
    lpub->actions.insert(editExcludedPartsAct->objectName(), Action(tr("Configuration.Edit Parameter Files.Part Count Excluded Parts List"), editExcludedPartsAct));
    connect(editExcludedPartsAct, SIGNAL(triggered()), this, SLOT(editExcludedParts()));

    QAction *editStickerPartsAct = new QAction(QIcon(":/resources/editstickerparts.png"),tr("Part Count Sticker Parts List"), this);
    editStickerPartsAct->setObjectName("editStickerPartsAct.1");
    editStickerPartsAct->setStatusTip(tr("Add/Edit the list of part count sticker parts"));
    lpub->actions.insert(editStickerPartsAct->objectName(), Action(tr("Configuration.Edit Parameter Files.Part Count Sticker Parts List"), editStickerPartsAct));
    connect(editStickerPartsAct, SIGNAL(triggered()), this, SLOT(editStickerParts()));

    QAction *editLdrawIniFileAct = new QAction(QIcon(":/resources/editinifile.png"),tr("LDraw INI Search Directories"), this);
    editLdrawIniFileAct->setObjectName("editLdrawIniFileAct.1");
    editLdrawIniFileAct->setStatusTip(tr("Add/Edit LDraw.ini search directory entries"));
    lpub->actions.insert(editLdrawIniFileAct->objectName(), Action(tr("Configuration.Edit Parameter Files.LDraw INI Search Directories"), editLdrawIniFileAct));
    connect(editLdrawIniFileAct, SIGNAL(triggered()), this, SLOT(editLdrawIniFile()));

    QAction *editLdgliteIniAct = new QAction(QIcon(":/resources/editldgliteconf.png"),tr("LDGLite INI Configuration File"), this);
    editLdgliteIniAct->setObjectName("editLdgliteIniAct.1");
    editLdgliteIniAct->setStatusTip(tr("Edit LDGLite INI configuration file"));
    lpub->actions.insert(editLdgliteIniAct->objectName(), Action(tr("Configuration.Edit Parameter Files.LDGLite INI Configuration File"), editLdgliteIniAct));
    connect(editLdgliteIniAct, SIGNAL(triggered()), this, SLOT(editLdgliteIni()));

    QAction *editNativePOVIniAct = new QAction(QIcon(":/resources/LPub32.png"),tr("Native POV file Generation Configuration File"), this);
    editNativePOVIniAct->setObjectName("editNativePOVIniAct.1");
    editNativePOVIniAct->setStatusTip(tr("Edit Native POV file generation configuration file"));
    lpub->actions.insert(editNativePOVIniAct->objectName(), Action(tr("Configuration.Edit Parameter Files.Native POV File Generation Configuration File"), editNativePOVIniAct));
    connect(editNativePOVIniAct, SIGNAL(triggered()), this, SLOT(editNativePovIni()));

    QAction *editLdviewIniAct = new QAction(QIcon(":/resources/editldviewconf.png"),tr("LDView INI Configuration File"), this);
    editLdviewIniAct->setObjectName("editLdviewIniAct.1");
    editLdviewIniAct->setStatusTip(tr("Edit LDView INI configuration file"));
    lpub->actions.insert(editLdviewIniAct->objectName(), Action(tr("Configuration.Edit Parameter Files.LDView INI Configuration File"), editLdviewIniAct));
    connect(editLdviewIniAct, SIGNAL(triggered()), this, SLOT(editLdviewIni()));

    QAction *editLPub3DIniFileAct = new QAction(QIcon(":/resources/editsetting.png"),tr("%1 Configuration File").arg(VER_PRODUCTNAME_STR), this);
    editLPub3DIniFileAct->setObjectName("editLPub3DIniFileAct.1");
    editLPub3DIniFileAct->setStatusTip(tr("Edit %1 application configuration settings file").arg(VER_PRODUCTNAME_STR));
    lpub->actions.insert(editLPub3DIniFileAct->objectName(), Action(tr("Configuration.Edit Parameter Files.%1 Configuration File").arg(VER_PRODUCTNAME_STR), editLPub3DIniFileAct));
    connect(editLPub3DIniFileAct, SIGNAL(triggered()), this, SLOT(editLPub3DIniFile()));

    QAction *editLdviewPovIniAct = new QAction(QIcon(":/resources/editldviewconf.png"),tr("LDView POV File Generation Configuration File"), this);
    editLdviewPovIniAct->setObjectName("editLdviewPovIniAct.1");
    editLdviewPovIniAct->setStatusTip(tr("Edit LDView POV file generation configuration file"));
    lpub->actions.insert(editLdviewPovIniAct->objectName(), Action(tr("Configuration.Edit Parameter Files.LDView POV File Generation Configuration File"), editLdviewPovIniAct));
    connect(editLdviewPovIniAct, SIGNAL(triggered()), this, SLOT(editLdviewPovIni()));

    QAction *editBlenderParametersAct = new QAction(QIcon(":/resources/blendericon.png"),tr("Blender LDraw Parameters"), this);
    editBlenderParametersAct->setObjectName("editBlenderParametersAct.1");
    editBlenderParametersAct->setStatusTip(tr("Add/Edit LDraw Blender LDraw LGEO colours, sloped bricks and lighted bricks, reference"));
    lpub->actions.insert(editBlenderParametersAct->objectName(), Action(tr("Configuration.Edit Parameter Files.Blender LDraw Parameters"), editBlenderParametersAct));
    connect(editBlenderParametersAct, SIGNAL(triggered()), this, SLOT(editBlenderParameters()));

    QAction *editPovrayIniAct = new QAction(QIcon(":/resources/editpovrayconf.png"),tr("POV-Ray INI Configuration File"), this);
    editPovrayIniAct->setObjectName("editPovrayIniAct.1");
    editPovrayIniAct->setStatusTip(tr("Edit Raytracer (POV-Ray) INI configuration file"));
    lpub->actions.insert(editPovrayIniAct->objectName(), Action(tr("Configuration.Edit Parameter Files.POV-Ray INI Configuration File"), editPovrayIniAct));
    connect(editPovrayIniAct, SIGNAL(triggered()), this, SLOT(editPovrayIni()));

    QAction *editPovrayConfAct = new QAction(QIcon(":/resources/editpovrayconf.png"),tr("POV-Ray File Access Configuration File"), this);
    editPovrayConfAct->setObjectName("editPovrayConfAct.1");
    editPovrayConfAct->setStatusTip(tr("Edit Raytracer (POV-Ray) file access configuration file"));
    lpub->actions.insert(editPovrayConfAct->objectName(), Action(tr("Configuration.Edit Parameter Files.POV-Ray File Access Configuration File"), editPovrayConfAct));
    connect(editPovrayConfAct, SIGNAL(triggered()), this, SLOT(editPovrayConf()));

    QAction *editAnnotationStyleAct = new QAction(QIcon(":/resources/editstyleref.png"),tr("Part Annotation Style Reference"), this);
    editAnnotationStyleAct->setObjectName("editAnnotationStyleAct.1");
    editAnnotationStyleAct->setStatusTip(tr("Add/Edit LDraw Design ID, Part Annotation Style, Part Category reference"));
    lpub->actions.insert(editAnnotationStyleAct->objectName(), Action(tr("Configuration.Edit Parameter Files.Part Annotation Style Reference"), editAnnotationStyleAct));
    connect(editAnnotationStyleAct, SIGNAL(triggered()), this, SLOT(editAnnotationStyle()));

    QAction *editLD2BLCodesXRefAct = new QAction(QIcon(":/resources/editld2blxref.png"),tr("LDraw to Bricklink Design ID Reference"), this);
    editLD2BLCodesXRefAct->setObjectName("editLD2BLCodesXRefAct.1");
    editLD2BLCodesXRefAct->setStatusTip(tr("Add/Edit LDraw to Bricklink Design ID reference"));
    lpub->actions.insert(editLD2BLCodesXRefAct->objectName(), Action(tr("Configuration.Edit Parameter Files.LDraw To Bricklink Design ID Reference"), editLD2BLCodesXRefAct));
    connect(editLD2BLCodesXRefAct, SIGNAL(triggered()), this, SLOT(editLD2BLCodesXRef()));

    QAction *editLD2BLColorsXRefAct = new QAction(QIcon(":/resources/editld2blxref.png"),tr("LDraw to Bricklink Color Reference"), this);
    editLD2BLColorsXRefAct->setObjectName("editLD2BLColorsXRefAct.1");
    editLD2BLColorsXRefAct->setStatusTip(tr("Add/Edit LDraw LDConfig to Bricklink Color ID reference"));
    lpub->actions.insert(editLD2BLColorsXRefAct->objectName(), Action(tr("Configuration.Edit Parameter Files.LDraw To Bricklink Color Reference"), editLD2BLColorsXRefAct));
    connect(editLD2BLColorsXRefAct, SIGNAL(triggered()), this, SLOT(editLD2BLColorsXRef()));

    QAction *editLD2RBCodesXRefAct = new QAction(QIcon(":/resources/editld2rbxref.png"),tr("LDraw to Rebrickable Design ID Reference"), this);
    editLD2RBCodesXRefAct->setObjectName("editLD2RBCodesXRefAct.1");
    editLD2RBCodesXRefAct->setStatusTip(tr("Add/Edit LDraw to Rebrickable Design ID reference"));
    lpub->actions.insert(editLD2RBCodesXRefAct->objectName(), Action(tr("Configuration.Edit Parameter Files.LDraw To Rebrickable Design ID Reference"), editLD2RBCodesXRefAct));
    connect(editLD2RBCodesXRefAct, SIGNAL(triggered()), this, SLOT(editLD2RBCodesXRef()));

    QAction *editLD2RBColorsXRefAct = new QAction(QIcon(":/resources/editld2rbxref.png"),tr("LDraw to Rebrickable Color Reference"), this);
    editLD2RBColorsXRefAct->setObjectName("editLD2RBColorsXRefAct.1");
    editLD2RBColorsXRefAct->setStatusTip(tr("Add/Edit LDraw LDConfig to Rebrickable Color ID reference"));
    lpub->actions.insert(editLD2RBColorsXRefAct->objectName(), Action(tr("Configuration.Edit Parameter Files.LDraw To Rebrickable Color Reference"), editLD2RBColorsXRefAct));
    connect(editLD2RBColorsXRefAct, SIGNAL(triggered()), this, SLOT(editLD2RBColorsXRef()));

    QAction *editBLColorsAct = new QAction(QIcon(":/resources/editld2blxref.png"),tr("Edit Bricklink Color Reference"), this);
    editBLColorsAct->setObjectName("editBLColorsAct.1");
    editBLColorsAct->setStatusTip(tr("Add/Edit Bricklink Color ID reference"));
    lpub->actions.insert(editBLColorsAct->objectName(), Action(tr("Configuration.Edit Parameter Files.Edit Bricklink Color Reference"), editBLColorsAct));
    connect(editBLColorsAct, SIGNAL(triggered()), this, SLOT(editBLColors()));

    QAction *editBLCodesAct = new QAction(QIcon(":/resources/editld2blxref.png"),tr("Bricklink Codes Reference"), this);
    editBLCodesAct->setObjectName("editBLCodesAct.1");
    editBLCodesAct->setStatusTip(tr("Add/Edit Bricklink Item No, Color Name, LEGO Element reference"));
    lpub->actions.insert(editBLCodesAct->objectName(), Action(tr("Configuration.Edit Parameter Files.Bricklink Codes Reference"), editBLCodesAct));
    connect(editBLCodesAct, SIGNAL(triggered()), this, SLOT(editBLCodes()));

    QAction *editPliControlFileAct = new QAction(QIcon(":/resources/editldraw.png"),tr("PLI Parts Control File"), this);
    editPliControlFileAct->setObjectName("editPliControlFileAct.1");
    editPliControlFileAct->setStatusTip(tr("Edit PLI parts control file"));
    lpub->actions.insert(editPliControlFileAct->objectName(), Action(tr("Configuration.Edit Parameter Files.PLI Parts Control File"), editPliControlFileAct));
    connect(editPliControlFileAct, SIGNAL(triggered()), this, SLOT(editPliControlFile()));

    QAction *editModelFileAct = new QAction(QIcon(":/resources/editldraw.png"),tr("Current Model File"), this);
    editModelFileAct->setObjectName("editModelFileAct.1");
    editModelFileAct->setStatusTip(tr("Edit current model file with %1")
                                   .arg(Preferences::useSystemEditor ?
                                            Preferences::systemEditor.isEmpty() ? "the system editor" :
                                                                                  Preferences::systemEditor : "detached LDraw Editor"));
    lpub->actions.insert(editModelFileAct->objectName(), Action(tr("Configuration.Current Model File"), editModelFileAct));
    connect(editModelFileAct, SIGNAL(triggered()), this, SLOT(editModelFile()));

    QAction *generateCustomColourPartsAct = new QAction(QIcon(":/resources/generatecolourparts.png"),tr("Generate Static Color Parts List"), this);
    generateCustomColourPartsAct->setObjectName("generateCustomColourPartsAct.1");
    generateCustomColourPartsAct->setStatusTip(tr("Generate list of all static coloured parts for use with fade previous steps"));
    lpub->actions.insert(generateCustomColourPartsAct->objectName(), Action(tr("Configuration.Generate Static Color Parts List"), generateCustomColourPartsAct));
    connect(generateCustomColourPartsAct, SIGNAL(triggered()), this, SLOT(generateCustomColourPartsList()));

    QAction *openWithSetupAct = new QAction(QIcon(":/resources/openwithsetup.png"), tr("Open With S&etup..."), this);
    openWithSetupAct->setObjectName("openWithSetupAct.1");
    openWithSetupAct->setStatusTip(tr("Setup 'Open With' applications."));
    lpub->actions.insert(openWithSetupAct->objectName(), Action(tr("Configuration.Open With Setup"), openWithSetupAct));
    connect(openWithSetupAct, SIGNAL(triggered()), this, SLOT(openWithSetup()));

    QAction *ldrawSearchDirectoriesAct = new QAction(QIcon(":/resources/searchdirectories.png"),tr("LDraw Search Directories..."), this);
    ldrawSearchDirectoriesAct->setObjectName("ldrawSearchDirectoriesAct.1");
    ldrawSearchDirectoriesAct->setStatusTip(tr("Manage LDraw search directories"));
    lpub->actions.insert(ldrawSearchDirectoriesAct->objectName(), Action(tr("Configuration.LDraw Search Directories"), ldrawSearchDirectoriesAct));
    connect(ldrawSearchDirectoriesAct, SIGNAL(triggered()), this, SLOT(ldrawSearchDirectories()));

    QAction *clearPLICacheAct = new QAction(QIcon(":/resources/clearplicache.png"),tr("Parts Image Cache"), this);
    clearPLICacheAct->setObjectName("clearPLICacheAct.1");
    clearPLICacheAct->setShortcut(tr("Alt+R"));
    clearPLICacheAct->setStatusTip(tr("Reset the parts list image cache"));
    lpub->actions.insert(clearPLICacheAct->objectName(), Action(tr("Configuration.Reset Cache.Parts Image Cache"), clearPLICacheAct));
    connect(clearPLICacheAct, SIGNAL(triggered()), this, SLOT(clearPLICache()));

    QAction *clearCSICacheAct = new QAction(QIcon(":/resources/clearcsicache.png"),tr("Assembly Image Cache"), this);
    clearCSICacheAct->setObjectName("clearCSICacheAct.1");
    clearCSICacheAct->setShortcut(tr("Alt+S"));
    clearCSICacheAct->setStatusTip(tr("Reset the assembly image cache"));
    lpub->actions.insert(clearCSICacheAct->objectName(), Action(tr("Configuration.Reset Cache.Assembly Image Cache"), clearCSICacheAct));
    connect(clearCSICacheAct, SIGNAL(triggered()), this, SLOT(clearCSICache()));

    QAction *clearSubmodelCacheAct = new QAction(QIcon(":/resources/clearsubmodelcache.png"),tr("Submodel Image Cache"), this);
    clearSubmodelCacheAct->setObjectName("clearSubmodelCacheAct.1");
    clearSubmodelCacheAct->setShortcut(tr("Alt+E"));
    clearSubmodelCacheAct->setStatusTip(tr("Reset the submodel image cache"));
    lpub->actions.insert(clearSubmodelCacheAct->objectName(), Action(tr("Configuration.Reset Cache.Submodel Image Cache"), clearSubmodelCacheAct));
    connect(clearSubmodelCacheAct, SIGNAL(triggered()), this, SLOT(clearSubmodelCache()));

    QAction *clearTempCacheAct = new QAction(QIcon(":/resources/cleartempcache.png"),tr("Temp File Cache"), this);
    clearTempCacheAct->setObjectName("clearTempCacheAct.1");
    clearTempCacheAct->setShortcut(tr("Alt+T"));
    clearTempCacheAct->setStatusTip(tr("Reset the Temp file and 3D viewer image cache"));
    lpub->actions.insert(clearTempCacheAct->objectName(), Action(tr("Configuration.Reset Cache.Temp File Cache"), clearTempCacheAct));
    connect(clearTempCacheAct, SIGNAL(triggered()), this, SLOT(clearTempCache()));

    QAction *clearAllCachesAct = new QAction(QIcon(":/resources/clearimagemodelcache.png"),tr("Model Caches"), this);
    clearAllCachesAct->setObjectName("clearAllCachesAct.1");
    clearAllCachesAct->setShortcut(tr("Alt+M"));
    clearAllCachesAct->setStatusTip(tr("Reset model file Parts, Assembly and Temp file caches"));
    lpub->actions.insert(clearAllCachesAct->objectName(), Action(tr("Configuration.Reset Cache.Model Caches"), clearAllCachesAct));
    connect(clearAllCachesAct, SIGNAL(triggered()), this, SLOT(clearAllCaches()));

    QAction *clearCustomPartCacheAct = new QAction(QIcon(":/resources/clearcustompartcache.png"),tr("Custom Files Cache"), this);
    clearCustomPartCacheAct->setObjectName("clearCustomPartCacheAct.1");
    clearCustomPartCacheAct->setShortcut(tr("Alt+C"));
    clearCustomPartCacheAct->setStatusTip(tr("Reset fade and highlight part files cache"));
    lpub->actions.insert(clearCustomPartCacheAct->objectName(), Action(tr("Configuration.Reset Cache.Custom Files Cache"), clearCustomPartCacheAct));
    connect(clearCustomPartCacheAct, SIGNAL(triggered()), this, SLOT(clearCustomPartCache()));

    QAction *archivePartsOnDemandAct = new QAction(QIcon(":/resources/archivefilesondemand.png"),tr("Archive Unofficial Parts"), this);
    archivePartsOnDemandAct->setObjectName("archivePartsOnDemandAct.1");
    archivePartsOnDemandAct->setStatusTip(tr("Archive unofficial parts from LDraw search directories"));
    archivePartsOnDemandAct->setShortcut(tr("Alt+Y"));
    lpub->actions.insert(archivePartsOnDemandAct->objectName(), Action(tr("Configuration.Archive Unofficial Parts"), archivePartsOnDemandAct));
    connect(archivePartsOnDemandAct, SIGNAL(triggered()), this, SLOT(archivePartsOnDemand()));

    QAction *archivePartsOnLaunchAct = new QAction(QIcon(":/resources/archivefilesonlaunch.png"),tr("Archive Unofficial Parts On Launch"), this);
    archivePartsOnLaunchAct->setObjectName("archivePartsOnLaunchAct.1");
    archivePartsOnLaunchAct->setStatusTip(tr("Automatically archive unofficial parts from LDraw search directories on next application launch"));
    archivePartsOnLaunchAct->setCheckable(true);
    archivePartsOnLaunchAct->setChecked(Preferences::archivePartsOnLaunch);
    lpub->actions.insert(archivePartsOnLaunchAct->objectName(), Action(tr("Configuration.Archive Unofficial Parts On Launch"), archivePartsOnLaunchAct));
    connect(archivePartsOnLaunchAct, SIGNAL(triggered()), this, SLOT(archivePartsOnLaunch()));

    QAction *refreshLDrawUnoffPartsAct = new QAction(QIcon(":/resources/refreshunoffarchive.png"),tr("Refresh LDraw Unofficial Parts"), this);
    refreshLDrawUnoffPartsAct->setObjectName("refreshLDrawUnoffPartsAct.1");
    refreshLDrawUnoffPartsAct->setStatusTip(tr("Download and replace LDraw Unofficial parts archive file in User data - restart required"));
    refreshLDrawUnoffPartsAct->setEnabled(Preferences::usingDefaultLibrary);
    lpub->actions.insert(refreshLDrawUnoffPartsAct->objectName(), Action(tr("Configuration.Refresh LDraw Unofficial Parts"), refreshLDrawUnoffPartsAct));
    connect(refreshLDrawUnoffPartsAct, SIGNAL(triggered()), this, SLOT(refreshLDrawUnoffParts()));

    QAction *refreshLDrawOfficialPartsAct = new QAction(QIcon(":/resources/refreshoffarchive.png"),tr("Refresh LDraw Official Parts"), this);
    refreshLDrawOfficialPartsAct->setObjectName("refreshLDrawOfficialPartsAct.1");
    refreshLDrawOfficialPartsAct->setStatusTip(tr("Download and replace LDraw Official parts archive file in User data - restart required"));
    refreshLDrawUnoffPartsAct->setEnabled(Preferences::usingDefaultLibrary);
    lpub->actions.insert(refreshLDrawOfficialPartsAct->objectName(), Action(tr("Configuration.Refresh LDraw Official Parts"), refreshLDrawOfficialPartsAct));
    connect(refreshLDrawOfficialPartsAct, SIGNAL(triggered()), this, SLOT(refreshLDrawOfficialParts()));

    // Help
    QAction *aboutAct = new QAction(QIcon(":/resources/LPub32.png"),tr("&About %1...").arg(VER_PRODUCTNAME_STR), this);
    aboutAct->setObjectName("aboutAct.1");
    aboutAct->setStatusTip(tr("Display version, system and build information"));
    lpub->actions.insert(aboutAct->objectName(), Action(tr("Help.About"), aboutAct));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(aboutDialog()));

    QAction *visitHomepageAct = new QAction(QIcon(":/resources/homepage.png"),tr("Visit Homepage"), this);
    visitHomepageAct->setObjectName("visitHomepageAct.1");
    visitHomepageAct->setStatusTip(tr("Visit the %1 home website.").arg(VER_PRODUCTNAME_STR));
    lpub->actions.insert(visitHomepageAct->objectName(), Action(tr("Help.Visit Homepage"), visitHomepageAct));
    connect(visitHomepageAct, SIGNAL(triggered()), this, SLOT(visitHomepage()));

    QAction *reportBugAct = new QAction(QIcon(":/resources/bug.png"),tr("Report Bug"), this);
    reportBugAct->setObjectName("reportBugAct.1");
    reportBugAct->setStatusTip(tr("Create an online ticket to report a bug or issue."));
    lpub->actions.insert(reportBugAct->objectName(), Action(tr("Help.Report Bug"), reportBugAct));
    connect(reportBugAct, SIGNAL(triggered()), this, SLOT(reportBug()));

    // Begin Jaco's code

    QAction *onlineManualAct = new QAction(QIcon(":/resources/onlinemanual.png"),tr("&Online Manual"), this);
    onlineManualAct->setObjectName("onlineManualAct.1");
    onlineManualAct->setStatusTip(tr("Visit the Online Manual Website."));
    lpub->actions.insert(onlineManualAct->objectName(), Action(tr("Help.Online Manual"), onlineManualAct));
    connect(onlineManualAct, SIGNAL(triggered()), this, SLOT(onlineManual()));

    // End Jaco's code
    
    QAction *commandsDialogAct = new QAction(QIcon(":/resources/command32.png"),tr("Manage &LPub Metacommands"), this);
    commandsDialogAct->setObjectName("commandsDialogAct.1");
    commandsDialogAct->setStatusTip(tr("View LPub meta commands and customize command descriptions"));
    commandsDialogAct->setShortcut(tr("Ctrl+K"));
    lpub->actions.insert(commandsDialogAct->objectName(), Action(tr("Help.Manage LPub Metacommands"), commandsDialogAct));
    connect(commandsDialogAct, SIGNAL(triggered()), this, SLOT(commandsDialog()));
    
    QAction *exportMetaCommandsAct = new QAction(QIcon(":/resources/savemetacommands.png"),tr("&Export LPub Metacommands"), this);
    exportMetaCommandsAct->setObjectName("exportMetaCommandsAct.1");
    exportMetaCommandsAct->setStatusTip(tr("Export a list of the LPub meta commands to a text file"));
    lpub->actions.insert(exportMetaCommandsAct->objectName(), Action(tr("Help.Export LPub Metacommands"), exportMetaCommandsAct));
    connect(exportMetaCommandsAct, SIGNAL(triggered()), this, SLOT(exportMetaCommands()));

    QAction *updateAppAct = new QAction(QIcon(":/resources/softwareupdate.png"),tr("Check For &Updates..."), this);
    updateAppAct->setObjectName("updateAppAct.1");
    updateAppAct->setStatusTip(tr("Check if a newer version of  %1 is available for download").arg(VER_PRODUCTNAME_STR));
    lpub->actions.insert(updateAppAct->objectName(), Action(tr("Help.Check For Updates"), updateAppAct));
    connect(updateAppAct, SIGNAL(triggered()), this, SLOT(updateCheck()));

    QAction *viewLogAct = new QAction(QIcon(":/resources/viewlog.png"),tr("View Runtime Log"), this);
    viewLogAct->setObjectName("viewLogAct.1");
    viewLogAct->setShortcut(tr("Alt+L"));
    viewLogAct->setStatusTip(tr("View %1 log").arg(VER_PRODUCTNAME_STR));
    lpub->actions.insert(viewLogAct->objectName(), Action(tr("Help.View Runtime Log"), viewLogAct));
    connect(viewLogAct, SIGNAL(triggered()), this, SLOT(viewLog()));

    QAction *openWorkingFolderAct = new QAction(QIcon(":/resources/openworkingfolder.png"),tr("Open Working Folder"), this);
    openWorkingFolderAct->setObjectName("openWorkingFolderAct.1");
    openWorkingFolderAct->setShortcut(tr("Alt+Shift+1"));
    openWorkingFolderAct->setStatusTip(tr("Open current model file working folder"));
    lpub->actions.insert(openWorkingFolderAct->objectName(), Action(tr("Help.Open Working Folder"), openWorkingFolderAct));
    connect(openWorkingFolderAct, SIGNAL(triggered()), this, SLOT(openWorkingFolder()));

    if (Preferences::modeGUI)
        create3DActions();

    // setup default shortcuts
    foreach (Action act, lpub->actions) {
        if (!act.action->shortcut().isEmpty()) {
            QKeySequence shortcut = act.action->shortcut();
            act.action->setProperty("defaultshortcut", shortcut);
        }
    }

    lpub->setKeyboardShortcuts();

    foreach (QAction *action, editWindow->actions()) {
        lpub->setKeyboardShortcut(action);
    }
    foreach (QAction *action, editModeWindow->actions()) {
        lpub->setKeyboardShortcut(action);
    }
    foreach (QAction *action, parmsWindow->actions()) {
        lpub->setKeyboardShortcut(action);
    }
}

void Gui::enableActions()
{
  getAct("saveAsAct.1")->setEnabled(true);
  getAct("saveCopyAct.1")->setEnabled(true);
  getAct("closeFileAct.1")->setEnabled(true);
  getAct("printToFileAct.1")->setEnabled(true);
  getAct("printToFilePreviewAct.1")->setEnabled(true);
  getAct("exportAsPdfAct.1")->setEnabled(true);
  getAct("exportAsPdfPreviewAct.1")->setEnabled(true);
  getAct("exportPngAct.1")->setEnabled(true);
  getAct("exportJpgAct.1")->setEnabled(true);
  getAct("exportBmpAct.1")->setEnabled(true);
  getAct("pageSetupAct.1")->setEnabled(true);
  getAct("assemSetupAct.1")->setEnabled(true);
  getAct("pliSetupAct.1")->setEnabled(true);
  getAct("bomSetupAct.1")->setEnabled(true);
  getAct("calloutSetupAct.1")->setEnabled(true);
  getAct("multiStepSetupAct.1")->setEnabled(true);
  getAct("subModelSetupAct.1")->setEnabled(true);
  getAct("projectSetupAct.1")->setEnabled(true);
  getAct("fadeStepSetupAct.1")->setEnabled(true);
  getAct("highlightStepSetupAct.1")->setEnabled(true);
  getAct("addPictureAct.1")->setEnabled(true);
  getAct("removeLPubFormatDocumentAct.1")->setEnabled(true);
  getAct("removeLPubFormatBomAct.1")->setEnabled(true);
  getAct("removeLPubFormatPageAct.1")->setEnabled(true);
  getAct("removeLPubFormatStepAct.1")->setEnabled(true);
  getAct("removeLPubFormatSubmodelAct.1")->setEnabled(true);
  getAct("removeChildSubmodelFormatAct.1")->setEnabled(true);
  getAct("removeBuildModFormatAct.1")->setEnabled(Preferences::buildModEnabled);
  getAct("editTitleAnnotationsAct.1")->setEnabled(true);
  getAct("editFreeFormAnnitationsAct.1")->setEnabled(true);
  getAct("editPliBomSubstitutePartsAct.1")->setEnabled(true);
  getAct("editExcludedPartsAct.1")->setEnabled(true);
  getAct("editStickerPartsAct.1")->setEnabled(true);
  getAct("editLDrawColourPartsAct.1")->setEnabled(!Preferences::ldrawColourPartsFile.isEmpty());
  getAct("editLdgliteIniAct.1")->setEnabled(true);
  getAct("editNativePOVIniAct.1")->setEnabled(true);
  getAct("editLdviewIniAct.1")->setEnabled(true);
  getAct("editLdviewPovIniAct.1")->setEnabled(true);
  getAct("editBlenderParametersAct.1")->setEnabled(true);
  getAct("editPovrayIniAct.1")->setEnabled(true);
  getAct("editPovrayConfAct.1")->setEnabled(true);
  getAct("editAnnotationStyleAct.1")->setEnabled(true);
  getAct("editLD2BLCodesXRefAct.1")->setEnabled(true);
  getAct("editLD2BLColorsXRefAct.1")->setEnabled(true);
  getAct("editLD2RBCodesXRefAct.1")->setEnabled(true);
  getAct("editLD2RBColorsXRefAct.1")->setEnabled(true);
  getAct("editBLColorsAct.1")->setEnabled(true);
  getAct("editBLCodesAct.1")->setEnabled(true);
  getAct("editModelFileAct.1")->setEnabled(true);
  getAct("editPliControlFileAct.1")->setEnabled(!Preferences::pliControlFile.isEmpty());
  getAct("openWorkingFolderAct.1")->setEnabled(true);

//  setPageLineEdit)->setEnabled(true);

//  getAct("firstPageAct.1")->setEnabled(true);
//  getAct("lastPageAct.1")->setEnabled(true);

//  getAct("nextPageAct.1")->setEnabled(true);
//  getAct("previousPageAct.1")->setEnabled(true);
//  getAct("nextPageComboAct.1")->setEnabled(true);
//  getAct("previousPageComboAct.1")->setEnabled(true);
//  getAct("nextPageContinuousAct.1")->setEnabled(true);
//  getAct("previousPageContinuousAct.1")->setEnabled(true);

  getAct("fitWidthAct.1")->setEnabled(true);
  getAct("fitVisibleAct.1")->setEnabled(true);
  getAct("fitSceneAct.1")->setEnabled(true);
  getAct("bringToFrontAct.1")->setEnabled(true);
  getAct("sendToBackAct.1")->setEnabled(true);
  getAct("actualSizeAct.1")->setEnabled(true);
  getAct("zoomInComboAct.1")->setEnabled(true);
  getAct("zoomOutComboAct.1")->setEnabled(true);
  getAct("sceneGuidesComboAct.1")->setEnabled(true);
  getAct("snapToGridComboAct.1")->setEnabled(true);
  getAct("exportBricklinkAct.1")->setEnabled(true);
  getAct("exportCsvAct.1")->setEnabled(true);
  getAct("exportPovAct.1")->setEnabled(true);
  getAct("exportStlAct.1")->setEnabled(true);
  getAct("export3dsAct.1")->setEnabled(true);
  getAct("exportObjAct.1")->setEnabled(true);
  getAct("exportColladaAct.1")->setEnabled(true);
  getAct("exportHtmlAct.1")->setEnabled(true);
  getAct("exportHtmlStepsAct.1")->setEnabled(true);
  getAct("copyFilePathToClipboardAct.1")->setEnabled(true);
  blenderRenderAct->setEnabled(true);
  blenderImportAct->setEnabled(true);
  povrayRenderAct->setEnabled(true);

  getMenu("setupMenu")->setEnabled(true);
  getMenu("cacheMenu")->setEnabled(true);
  getMenu("exportMenu")->setEnabled(true);
  openWithMenu->setEnabled(numPrograms);

  //Visual Editor
  //ViewerExportMenu->setEnabled(true); // Hide Visual Editor step export functions

}

void Gui::disableActions()
{
  getAct("saveAsAct.1")->setEnabled(false);
  getAct("saveCopyAct.1")->setEnabled(false);
  getAct("closeFileAct.1")->setEnabled(false);
  getAct("printToFilePreviewAct.1")->setEnabled(false);
  getAct("printToFileAct.1")->setEnabled(false);
  getAct("exportAsPdfPreviewAct.1")->setEnabled(false);
  getAct("exportAsPdfAct.1")->setEnabled(false);
  getAct("exportPngAct.1")->setEnabled(false);
  getAct("exportJpgAct.1")->setEnabled(false);
  getAct("exportBmpAct.1")->setEnabled(false);
  getAct("pageSetupAct.1")->setEnabled(false);
  getAct("assemSetupAct.1")->setEnabled(false);
  getAct("pliSetupAct.1")->setEnabled(false);
  getAct("bomSetupAct.1")->setEnabled(false);
  getAct("calloutSetupAct.1")->setEnabled(false);
  getAct("multiStepSetupAct.1")->setEnabled(false);
  getAct("subModelSetupAct.1")->setEnabled(false);
  getAct("projectSetupAct.1")->setEnabled(false);
  getAct("fadeStepSetupAct.1")->setEnabled(false);
  getAct("highlightStepSetupAct.1")->setEnabled(false);
  getAct("addPictureAct.1")->setEnabled(false);
  getAct("removeLPubFormatBomAct.1")->setEnabled(false);
  getAct("removeLPubFormatDocumentAct.1")->setEnabled(false);
  getAct("removeLPubFormatPageAct.1")->setEnabled(false);
  getAct("removeLPubFormatStepAct.1")->setEnabled(false);
  getAct("removeLPubFormatSubmodelAct.1")->setEnabled(false);
  getAct("removeChildSubmodelFormatAct.1")->setEnabled(false);
  getAct("removeBuildModFormatAct.1")->setEnabled(false);
  getAct("editModelFileAct.1")->setEnabled(false);
  getAct("openWorkingFolderAct.1")->setEnabled(false);
  getAct("firstPageAct.1")->setEnabled(false);
  getAct("lastPageAct.1")->setEnabled(false);
  getAct("nextPageAct.1")->setEnabled(false);
  getAct("previousPageAct.1")->setEnabled(false);
  getAct("nextPageComboAct.1")->setEnabled(false);
  getAct("previousPageComboAct.1")->setEnabled(false);
  getAct("nextPageContinuousAct.1")->setEnabled(false);
  getAct("previousPageContinuousAct.1")->setEnabled(false);
  getAct("fitWidthAct.1")->setEnabled(false);
  getAct("fitVisibleAct.1")->setEnabled(false);
  getAct("fitSceneAct.1")->setEnabled(false);
  getAct("bringToFrontAct.1")->setEnabled(false);
  getAct("sendToBackAct.1")->setEnabled(false);
  getAct("actualSizeAct.1")->setEnabled(false);
  getAct("zoomInComboAct.1")->setEnabled(false);
  getAct("zoomOutComboAct.1")->setEnabled(false);
  getAct("exportBricklinkAct.1")->setEnabled(false);
  getAct("exportCsvAct.1")->setEnabled(false);
  getAct("exportPovAct.1")->setEnabled(false);
  getAct("exportStlAct.1")->setEnabled(false);
  getAct("export3dsAct.1")->setEnabled(false);
  getAct("exportObjAct.1")->setEnabled(false);
  getAct("exportColladaAct.1")->setEnabled(false);
  getAct("exportHtmlAct.1")->setEnabled(false);
  getAct("exportHtmlStepsAct.1")->setEnabled(false);
  getAct("copyFilePathToClipboardAct.1")->setEnabled(false);
  blenderRenderAct->setEnabled(false);
  blenderImportAct->setEnabled(false);
  povrayRenderAct->setEnabled(false);

  setPageLineEdit->setEnabled(false);

  getMenu("setupMenu")->setEnabled(false);
  getMenu("cacheMenu")->setEnabled(false);
  getMenu("exportMenu")->setEnabled(false);
  openWithMenu->setEnabled(false);

  // Visual Editor
  // ViewerExportMenu->setEnabled(false); // Hide Visual Editor step export functions

}

void Gui::enableActions2()
{
    bool frontCoverPageExist = lpub->mi.frontCoverPageExist();
    getAct("insertCoverPageAct.1")->setEnabled(! frontCoverPageExist &&
                                   lpub->mi.okToInsertCoverPage());
    bool backCoverPageExist = lpub->mi.backCoverPageExist();
    getAct("appendCoverPageAct.1")->setEnabled(! backCoverPageExist &&
                                   lpub->mi.okToAppendCoverPage());

    loadPages(frontCoverPageExist, backCoverPageExist);

    bool frontCover = lpub->mi.okToInsertNumberedPage();
    getAct("insertNumberedPageAct.1")->setEnabled(frontCover);
    bool backCover = lpub->mi.okToAppendNumberedPage();
    getAct("appendNumberedPageAct.1")->setEnabled(backCover);
    getAct("deletePageAct.1")->setEnabled(lpub->page.list.size() == 0);
    getAct("addBomAct.1")->setEnabled(frontCover||backCover);
    getAct("addTextAct.1")->setEnabled(true);
}

void Gui::disableActions2()
{
    getAct("insertCoverPageAct.1")->setEnabled(false);
    getAct("appendCoverPageAct.1")->setEnabled(false);
    getAct("insertNumberedPageAct.1")->setEnabled(false);
    getAct("appendNumberedPageAct.1")->setEnabled(false);
    getAct("deletePageAct.1")->setEnabled(false);
    getAct("addBomAct.1")->setEnabled(false);
    getAct("addTextAct.1")->setEnabled(false);
}

QMenu *Gui::getMenu(const QString &objectName)
{
    if (menus.contains(objectName))
        return menus.value(objectName);
    return nullptr;
}

void Gui::createMenus()
{
    // Editor Menus

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->setObjectName("fileMenu");
    menus.insert(fileMenu->objectName(), fileMenu);
    fileMenu->addAction(getAct("openAct.1"));

    openWithMenu = fileMenu->addMenu(tr("Open With..."));
    openWithMenu->setObjectName("openWithMenu");
    menus.insert(openWithMenu->objectName(), openWithMenu);
    openWithMenu->setIcon(QIcon(":/resources/openwith.png"));
    openWithMenu->setStatusTip(tr("Open model file with selected application"));
    const int systemEditor = Preferences::systemEditor.isEmpty() ? 0 : 1;
    const int maxOpenWithPrograms = Preferences::maxOpenWithPrograms + systemEditor;
    for (int i = 0; i < maxOpenWithPrograms; i++) {
      if (i == Preferences::maxOpenWithPrograms)
          openWithMenu->addSeparator();
      openWithMenu->addAction(openWithActList.at(i));
    }

    fileMenu->addAction(getAct("saveAct.1"));
    fileMenu->addAction(getAct("saveAsAct.1"));
    fileMenu->addAction(getAct("saveCopyAct.1"));
    fileMenu->addAction(getAct("closeFileAct.1"));

    QMenu *importMenu = fileMenu->addMenu("Import...");
    importMenu->setObjectName("importMenu");
    menus.insert(importMenu->objectName(), importMenu);
    importMenu->setIcon(QIcon(":/resources/import.png"));
    importMenu->setStatusTip(tr("Import LEGO Digital Designer files and set inventory lists"));
    importMenu->addAction(getAct("importLDDAct.1"));
    importMenu->addAction(getAct("importSetInventoryAct.1"));
    importMenu->addSeparator();

    QMenu *exportMenu = fileMenu->addMenu("Export As...");
    exportMenu->setObjectName("exportMenu");
    menus.insert(exportMenu->objectName(), exportMenu);
    exportMenu->setIcon(QIcon(":/resources/exportas.png"));
    exportMenu->setStatusTip(tr("Export model file images, objects or part lists"));
    exportMenu->addAction(getAct("exportPngAct.1"));
    exportMenu->addAction(getAct("exportJpgAct.1"));
#ifdef Q_OS_WIN
    exportMenu->addAction(getAct("exportBmpAct.1"));
#endif
    exportMenu->addSeparator();
    exportMenu->addAction(getAct("exportObjAct.1"));
    exportMenu->addAction(getAct("exportStlAct.1"));
    exportMenu->addAction(getAct("exportPovAct.1"));
    exportMenu->addAction(getAct("exportColladaAct.1"));
    exportMenu->addAction(getAct("export3dsAct.1"));
    exportMenu->addSeparator();
    exportMenu->addAction(getAct("exportHtmlAct.1"));
    exportMenu->addAction(getAct("exportHtmlStepsAct.1"));
    exportMenu->addAction(getAct("exportBricklinkAct.1"));
    exportMenu->addAction(getAct("exportCsvAct.1"));
    exportMenu->addSeparator();
    exportMenu->setDisabled(true);

    fileMenu->addAction(getAct("printToFilePreviewAct.1"));
    fileMenu->addAction(getAct("printToFileAct.1"));
    fileMenu->addAction(getAct("exportAsPdfPreviewAct.1"));
    fileMenu->addAction(getAct("exportAsPdfAct.1"));
#ifndef QT_NO_CLIPBOARD
    fileMenu->addSeparator();
    fileMenu->addAction(getAct("copyFilePathToClipboardAct.1"));
#endif
    fileMenu->addSeparator();

    QMenu *recentFileMenu = fileMenu->addMenu(tr("Recent Files..."));
    recentFileMenu->setObjectName("recentFileMenu");
    menus.insert(recentFileMenu->objectName(), recentFileMenu);
    recentFileMenu->setIcon(QIcon(":/resources/recentfiles.png"));
    recentFileMenu->setStatusTip(tr("Open recent model file"));

    for (int i = 0; i < MAX_RECENT_FILES; i++) {
      recentFileMenu->addAction(recentFilesActs[i]);
    }
    recentFilesSeparatorAct = recentFileMenu->addSeparator();

    recentFileMenu->addSeparator();
    recentFileMenu->addAction(getAct("clearRecentAct.1"));

    fileMenu->addSeparator();
    fileMenu->addAction(getAct("exitAct.1"));

    menuBar()->addSeparator();

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->setObjectName("editMenu");
    menus.insert(editMenu->objectName(), editMenu);

    QMenu *removeLPubFormatMenu = new QMenu(tr("Remove LPub Format..."), editMenu);
    removeLPubFormatMenu->setObjectName("removeLPubFormatMenu");
    menus.insert(removeLPubFormatMenu->objectName(), removeLPubFormatMenu);
    removeLPubFormatMenu->setIcon(QIcon(":/resources/removelpubformat.png"));
    removeLPubFormatMenu->setStatusTip(tr("Remove LPub format from document, page, step and submodel"));
    removeLPubFormatMenu->addAction(getAct("removeLPubFormatDocumentAct.1"));
    removeLPubFormatMenu->addSeparator();
    removeLPubFormatMenu->addAction(getAct("removeLPubFormatPageAct.1"));
    removeLPubFormatMenu->addAction(getAct("removeLPubFormatStepAct.1"));
    removeLPubFormatMenu->addAction(getAct("removeLPubFormatSubmodelAct.1"));
    removeLPubFormatMenu->addAction(getAct("removeLPubFormatBomAct.1"));
    removeLPubFormatMenu->addSeparator();
    removeLPubFormatMenu->addAction(getAct("removeBuildModFormatAct.1"));
    removeLPubFormatMenu->addAction(getAct("removeChildSubmodelFormatAct.1"));

    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);

    editMenu->addSeparator();

    editMenu->addAction(getAct("insertCoverPageAct.1"));
    editMenu->addAction(getAct("appendCoverPageAct.1"));
    editMenu->addAction(getAct("insertNumberedPageAct.1"));
    editMenu->addAction(getAct("appendNumberedPageAct.1"));
    editMenu->addAction(getAct("deletePageAct.1"));
    editMenu->addAction(getAct("addPictureAct.1"));
    editMenu->addAction(getAct("addTextAct.1"));
    editMenu->addAction(getAct("addBomAct.1"));
    editMenu->addAction(getAct("cycleEachPageAct.1"));
    editMenu->addSeparator();

    editMenu->insertMenu(getAct("cycleEachPageAct.1"), removeLPubFormatMenu);
    editMenu->insertSeparator(getAct("cycleEachPageAct.1"));

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->setObjectName("viewMenu");
    menus.insert(viewMenu->objectName(), viewMenu);
    viewMenu->addAction(getAct("fitWidthAct.1"));
    viewMenu->addAction(getAct("fitVisibleAct.1"));
    viewMenu->addAction(getAct("actualSizeAct.1"));
    viewMenu->addAction(getAct("fitSceneAct.1"));
    viewMenu->addAction(getAct("bringToFrontAct.1"));
    viewMenu->addAction(getAct("sendToBackAct.1"));
    viewMenu->addAction(getAct("zoomInComboAct.1"));
    viewMenu->addAction(getAct("zoomOutComboAct.1"));
    viewMenu->addAction(getAct("sceneRulerComboAct.1"));
    viewMenu->addAction(getAct("sceneGuidesComboAct.1"));
    viewMenu->addAction(getAct("snapToGridComboAct.1"));
    viewMenu->addSeparator();
    viewMenu->addAction(getAct("fullScreenViewAct.1"));

    QMenu *navigationMenu = menuBar()->addMenu(tr("&Navigation"));
    navigationMenu->setObjectName("navigationMenu");
    menus.insert(navigationMenu->objectName(), navigationMenu);
    navigationMenu->addAction(getAct("firstPageAct.1"));
    navigationMenu->addAction(getAct("nextPageAct.1"));
    navigationMenu->addAction(getAct("nextPageContinuousAct.1"));
    navigationMenu->addAction(getAct("previousPageAct.1"));
    navigationMenu->addAction(getAct("previousPageContinuousAct.1"));
    navigationMenu->addAction(getAct("lastPageAct.1"));

    QMenu *configMenu = menuBar()->addMenu(tr("&Configuration"));
    configMenu->setObjectName("configMenu");
    menus.insert(configMenu->objectName(), configMenu);
    configMenu->addAction(getAct("preferencesAct.1"));

    QMenu *setupMenu = configMenu->addMenu("Build &Instructions Setup...");
    setupMenu->setObjectName("setupMenu");
    menus.insert(setupMenu->objectName(), setupMenu);
    setupMenu->setIcon(QIcon(":/resources/instructionsetup.png"));
    setupMenu->setStatusTip(tr("Instruction document global settings"));
    setupMenu->addAction(getAct("pageSetupAct.1"));
    setupMenu->addAction(getAct("assemSetupAct.1"));
    setupMenu->addAction(getAct("pliSetupAct.1"));
    setupMenu->addAction(getAct("bomSetupAct.1"));
    setupMenu->addAction(getAct("calloutSetupAct.1"));
    setupMenu->addAction(getAct("multiStepSetupAct.1"));
    setupMenu->addAction(getAct("subModelSetupAct.1"));
    setupMenu->addAction(getAct("projectSetupAct.1"));
    setupMenu->addAction(getAct("fadeStepSetupAct.1"));
    setupMenu->addAction(getAct("highlightStepSetupAct.1"));
    setupMenu->addSeparator();
    setupMenu->setDisabled(true);

    QMenu *editorMenu = configMenu->addMenu("Edit Parameter Files...");
    editorMenu->setObjectName("editorMenu");
    menus.insert(editorMenu->objectName(), editorMenu);
    editorMenu->setIcon(QIcon(":/resources/editparameterfiles.png"));
    editorMenu->setStatusTip(tr("Edit %1 parameter files").arg(VER_PRODUCTNAME_STR));
    editorMenu->addAction(getAct("useSystemEditorAct.1"));
    editorMenu->addSeparator();
#if defined Q_OS_WIN
    if (Preferences::portableDistribution){
      editorMenu->addAction(getAct("editLPub3DIniFileAct.1"));
      editorMenu->addSeparator();
    }
#else
    editorMenu->addAction(getAct("editLPub3DIniFileAct.1"));
    editorMenu->addSeparator();
#endif
    editorMenu->addAction(getAct("editLDrawColourPartsAct.1"));
    editorMenu->addAction(getAct("editPliControlFileAct.1"));
    editorMenu->addAction(getAct("editTitleAnnotationsAct.1"));
    editorMenu->addAction(getAct("editFreeFormAnnitationsAct.1"));
    editorMenu->addAction(getAct("editPliBomSubstitutePartsAct.1"));
    editorMenu->addAction(getAct("editExcludedPartsAct.1"));
    editorMenu->addAction(getAct("editStickerPartsAct.1"));
    editorMenu->addAction(getAct("editAnnotationStyleAct.1"));
    editorMenu->addAction(getAct("editLD2BLCodesXRefAct.1"));
    editorMenu->addAction(getAct("editLD2BLColorsXRefAct.1"));
    editorMenu->addAction(getAct("editBLColorsAct.1"));
    editorMenu->addAction(getAct("editBLCodesAct.1"));
    editorMenu->addAction(getAct("editLD2RBColorsXRefAct.1"));
    editorMenu->addAction(getAct("editLD2RBCodesXRefAct.1"));
    editorMenu->addSeparator();
    editorMenu->addAction(getAct("editNativePOVIniAct.1"));
    editorMenu->addAction(getAct("editLdgliteIniAct.1"));
    editorMenu->addAction(getAct("editLdviewIniAct.1"));
    editorMenu->addAction(getAct("editLdviewPovIniAct.1"));
    editorMenu->addAction(getAct("editPovrayIniAct.1"));
    editorMenu->addAction(getAct("editPovrayConfAct.1"));
    if (!Preferences::blenderExe.isEmpty())
        editorMenu->addAction(getAct("editBlenderParametersAct.1"));
    editorMenu->addSeparator();

    configMenu->addAction(getAct("editModelFileAct.1"));
    configMenu->addAction(getAct("generateCustomColourPartsAct.1"));
    configMenu->addSeparator();
    configMenu->addAction(getAct("openWithSetupAct.1"));

    configMenu->addSeparator();

    QMenu *cacheMenu = configMenu->addMenu("Reset Cache");
    cacheMenu->setObjectName("cacheMenu");
    menus.insert(cacheMenu->objectName(), cacheMenu);
    cacheMenu->setIcon(QIcon(":/resources/resetcache.png"));
    cacheMenu->setStatusTip(tr("Reset working caches"));
    cacheMenu->addAction(getAct("clearAllCachesAct.1"));
    cacheMenu->addAction(getAct("clearPLICacheAct.1"));
    cacheMenu->addAction(getAct("clearSubmodelCacheAct.1"));
    cacheMenu->addAction(getAct("clearCSICacheAct.1"));
    cacheMenu->addAction(getAct("clearTempCacheAct.1"));
    cacheMenu->addAction(getAct("clearCustomPartCacheAct.1"));
    cacheMenu->addSeparator();
    cacheMenu->setDisabled(true);

    configMenu->addSeparator();
    configMenu->addAction(getAct("ldrawSearchDirectoriesAct.1"));
    configMenu->addAction(getAct("archivePartsOnDemandAct.1"));
    configMenu->addAction(getAct("archivePartsOnLaunchAct.1"));
    configMenu->addSeparator();
    configMenu->addAction(getAct("refreshLDrawUnoffPartsAct.1"));
    configMenu->addAction(getAct("refreshLDrawOfficialPartsAct.1"));

    // Visual Editor

    if (Preferences::modeGUI)
        create3DMenus();

    // Help Menus

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->setObjectName("helpMenu");
    menus.insert(helpMenu->objectName(), helpMenu);
    helpMenu->addAction(getAct("openWorkingFolderAct.1"));
    helpMenu->addAction(getAct("viewLogAct.1"));
#ifndef DISABLE_UPDATE_CHECK
    helpMenu->addAction(getAct("updateAppAct.1"));
#endif
    helpMenu->addAction(getAct("visitHomepageAct.1"));
    helpMenu->addAction(getAct("reportBugAct.1"));
    // Begin Jaco's code
    helpMenu->addAction(getAct("onlineManualAct.1"));
    // End Jaco's code
    helpMenu->addAction(getAct("commandsDialogAct.1"));
    helpMenu->addAction(getAct("exportMetaCommandsAct.1"));
    helpMenu->addSeparator();
    // About Editor
    helpMenu->addAction(getAct("aboutAct.1"));

    QMenu *previousPageContinuousMenu = new QMenu(tr("Continuous Page Previous"), this);
    previousPageContinuousMenu->setObjectName("previousPageContinuousMenu");
    menus.insert(previousPageContinuousMenu->objectName(), previousPageContinuousMenu);
    previousPageContinuousMenu->addAction(getAct("previousPageContinuousAct.1"));

    QMenu *nextPageContinuousMenu = new QMenu(tr("Continuous Page Next"), this);
    nextPageContinuousMenu->setObjectName("nextPageContinuousMenu");
    menus.insert(nextPageContinuousMenu->objectName(), nextPageContinuousMenu);
    nextPageContinuousMenu->addAction(getAct("nextPageContinuousAct.1"));

    QMenu *zoomSliderMenu = new QMenu(tr("Zoom Slider"),this);
    zoomSliderMenu->setObjectName("zoomSliderMenu");
    menus.insert(zoomSliderMenu->objectName(), zoomSliderMenu);
    zoomSliderMenu->addAction(zoomSliderAct);

    QMenu *sceneRulerTrackingMenu = new QMenu(tr("Ruler Tracking"),this);
    sceneRulerTrackingMenu->setObjectName("sceneRulerTrackingMenu");
    menus.insert(sceneRulerTrackingMenu->objectName(), sceneRulerTrackingMenu);
    sceneRulerTrackingMenu->addAction(getAct("hideRulerPageBackgroundAct.1"));
    sceneRulerTrackingMenu->addSeparator();
    sceneRulerTrackingMenu->addAction(getAct("sceneRulerTrackingNoneAct.1"));
    sceneRulerTrackingMenu->addAction(getAct("sceneRulerTrackingTickAct.1"));
    sceneRulerTrackingMenu->addAction(getAct("sceneRulerTrackingLineAct.1"));
    sceneRulerTrackingMenu->addSeparator();
    sceneRulerTrackingMenu->addAction(getAct("showTrackingCoordinatesAct.1"));

    QMenu *sceneGuidesMenu = new QMenu(tr("Scene Guides"),this);
    sceneGuidesMenu->setObjectName("sceneGuidesMenu");
    menus.insert(sceneGuidesMenu->objectName(), sceneGuidesMenu);
    sceneGuidesMenu->addAction(getAct("sceneGuidesDashLineAct.1"));
    sceneGuidesMenu->addAction(getAct("sceneGuidesSolidLineAct.1"));
    sceneGuidesMenu->addSeparator();
    sceneGuidesMenu->addAction(getAct("sceneGuidesPosTLeftAct.1"));
    sceneGuidesMenu->addAction(getAct("sceneGuidesPosTRightAct.1"));
    sceneGuidesMenu->addAction(getAct("sceneGuidesPosCentreAct.1"));
    sceneGuidesMenu->addAction(getAct("sceneGuidesPosBLeftAct.1"));
    sceneGuidesMenu->addAction(getAct("sceneGuidesPosBRightAct.1"));
    sceneGuidesMenu->addSeparator();
    sceneGuidesMenu->addAction(getAct("showGuidesCoordinatesAct.1"));

    QMenu *snapToGridMenu = new QMenu(tr("Snap to Grid"), this);
    snapToGridMenu->setObjectName("snapToGridMenu");
    menus.insert(snapToGridMenu->objectName(), snapToGridMenu);
    snapToGridMenu->addAction(getAct("hideGridPageBackgroundAct.1"));
    snapToGridMenu->addSeparator();
    for (int actionIdx = GRID_SIZE_FIRST; actionIdx < GRID_SIZE_LAST; actionIdx++)
        snapToGridMenu->addAction(snapGridActions[actionIdx]);
}

QToolBar *Gui::getToolBar(const QString &objectName)
{
    if (toolbars.contains(objectName))
        return toolbars.value(objectName);
    return nullptr;
}

void Gui::createToolBars()
{
    QSettings Settings;

    QToolBar *fileToolBar = addToolBar(tr("File Toolbar"));
    fileToolBar->setObjectName("fileToolBar");
    toolbars.insert(fileToolBar->objectName(), fileToolBar);
    fileToolBar->setObjectName("FileToolbar");
    fileToolBar->addAction(getAct("openAct.1"));
    fileToolBar->addAction(getAct("saveAct.1"));
    fileToolBar->addAction(getAct("saveAsAct.1"));
    //fileToolBar->addAction(getAct("saveCopyAct.1"));
    fileToolBar->addAction(getAct("closeFileAct.1"));

    fileToolBar->addAction(getAct("printToFilePreviewAct.1"));
    fileToolBar->addAction(getAct("printToFileAct.1"));
    fileToolBar->addAction(getAct("exportAsPdfPreviewAct.1"));
    fileToolBar->addAction(getAct("exportAsPdfAct.1"));

    QToolBar *importToolBar = addToolBar(tr("Import Toolbar"));
    importToolBar->setObjectName("importToolBar");
    toolbars.insert(importToolBar->objectName(), importToolBar);
    importToolBar->setObjectName("ImportToolbar");
    importToolBar->addAction(getAct("importLDDAct.1"));
    importToolBar->addAction(getAct("importSetInventoryAct.1"));
    bool visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS, VIEW_IMPORT_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS, VIEW_IMPORT_TOOLBAR_KEY)).toBool();
    getMenu("importMenu")->addAction(importToolBar->toggleViewAction());
    importToolBar->setVisible(visible);
    connect (importToolBar, SIGNAL (visibilityChanged(bool)),
                      this, SLOT (importToolBarVisibilityChanged(bool)));

    exportToolBar = addToolBar(tr("Export Toolbar"));
    exportToolBar->setObjectName("exportToolBar");
    toolbars.insert(exportToolBar->objectName(), exportToolBar);
    exportToolBar->setObjectName("ExportToolbar");
    exportToolBar->addAction(getAct("exportPngAct.1"));
    exportToolBar->addAction(getAct("exportJpgAct.1"));
#ifdef Q_OS_WIN
    exportToolBar->addAction(getAct("exportBmpAct.1"));
#endif
    exportToolBar->addSeparator();
    exportToolBar->addAction(getAct("exportObjAct.1"));
    exportToolBar->addAction(getAct("exportStlAct.1"));
    exportToolBar->addAction(getAct("exportPovAct.1"));
    exportToolBar->addAction(getAct("exportColladaAct.1"));
    exportToolBar->addSeparator();
    exportToolBar->addAction(getAct("exportHtmlAct.1"));
    exportToolBar->addAction(getAct("exportHtmlStepsAct.1"));
    exportToolBar->addAction(getAct("exportBricklinkAct.1"));
    exportToolBar->addAction(getAct("exportCsvAct.1"));
    visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS, VIEW_EXPORT_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS, VIEW_EXPORT_TOOLBAR_KEY)).toBool();
    getMenu("exportMenu")->addAction(exportToolBar->toggleViewAction());
    exportToolBar->setVisible(visible);
    connect (exportToolBar, SIGNAL (visibilityChanged(bool)),
                      this, SLOT (exportToolBarVisibilityChanged(bool)));

    QToolBar *undoredoToolBar = addToolBar(tr("UndoRedo Toolbar"));
    undoredoToolBar->setObjectName("undoredoToolBar");
    toolbars.insert(undoredoToolBar->objectName(), undoredoToolBar);
    undoredoToolBar->setObjectName("UndoRedoToolbar");
    undoredoToolBar->addAction(undoAct);
    undoredoToolBar->addAction(redoAct);

    QToolBar *cacheToolBar = addToolBar(tr("Cache Toolbar"));
    cacheToolBar->setObjectName("cacheToolBar");
    toolbars.insert(cacheToolBar->objectName(), cacheToolBar);
    cacheToolBar->setObjectName("CacheToolbar");
    cacheToolBar->addSeparator();
    cacheToolBar->addAction(getAct("clearAllCachesAct.1"));
    cacheToolBar->addAction(getAct("clearPLICacheAct.1"));
    cacheToolBar->addAction(getAct("clearSubmodelCacheAct.1"));
    cacheToolBar->addAction(getAct("clearCSICacheAct.1"));
    cacheToolBar->addAction(getAct("clearTempCacheAct.1"));
    cacheToolBar->addAction(getAct("clearCustomPartCacheAct.1"));
    visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_CACHE_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_CACHE_TOOLBAR_KEY)).toBool();
    getMenu("cacheMenu")->addAction(cacheToolBar->toggleViewAction());
    cacheToolBar->setVisible(visible);
    connect (cacheToolBar, SIGNAL (visibilityChanged(bool)),
                     this, SLOT (cacheToolBarVisibilityChanged(bool)));

    QToolBar *setupToolBar = addToolBar(tr("Global Setup Toolbar"));
    setupToolBar->setObjectName("setupToolBar");
    toolbars.insert(setupToolBar->objectName(), setupToolBar);
    setupToolBar->setObjectName("ToolsToolbar");
    setupToolBar->addAction(getAct("pageSetupAct.1"));
    setupToolBar->addAction(getAct("assemSetupAct.1"));
    setupToolBar->addAction(getAct("pliSetupAct.1"));
    setupToolBar->addAction(getAct("bomSetupAct.1"));
    setupToolBar->addAction(getAct("calloutSetupAct.1"));
    setupToolBar->addAction(getAct("multiStepSetupAct.1"));
    setupToolBar->addAction(getAct("subModelSetupAct.1"));
    setupToolBar->addAction(getAct("projectSetupAct.1"));
    setupToolBar->addAction(getAct("fadeStepSetupAct.1"));
    setupToolBar->addAction(getAct("highlightStepSetupAct.1"));
    visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_SETUP_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_SETUP_TOOLBAR_KEY)).toBool();
    getMenu("setupMenu")->addAction(setupToolBar->toggleViewAction());
    setupToolBar->setVisible(visible);
    connect (setupToolBar, SIGNAL (visibilityChanged(bool)),
                     this, SLOT (setupToolBarVisibilityChanged(bool)));

    QToolBar *editToolBar = addToolBar(tr("Edit Toolbar"));
    editToolBar->setObjectName("editToolBar");
    toolbars.insert(editToolBar->objectName(), editToolBar);
    editToolBar->setObjectName("EditToolbar");
    editToolBar->addAction(getAct("insertCoverPageAct.1"));
    editToolBar->addAction(getAct("appendCoverPageAct.1"));
    editToolBar->addAction(getAct("insertNumberedPageAct.1"));
    editToolBar->addAction(getAct("appendNumberedPageAct.1"));
    editToolBar->addAction(getAct("deletePageAct.1"));
    editToolBar->addAction(getAct("addPictureAct.1"));
    editToolBar->addAction(getAct("addTextAct.1"));
    editToolBar->addAction(getAct("addBomAct.1"));

    QToolBar *removeLPubFormatToolBar = addToolBar(tr("Remove LPub Format Toolbar"));
    removeLPubFormatToolBar->setObjectName("removeLPubFormatToolBar");
    toolbars.insert(removeLPubFormatToolBar->objectName(), removeLPubFormatToolBar);
    removeLPubFormatToolBar->setObjectName("RemoveLPubFormatToolBar");
    removeLPubFormatToolBar->addAction(getAct("removeLPubFormatDocumentAct.1"));
    removeLPubFormatToolBar->addAction(getAct("removeLPubFormatPageAct.1"));
    removeLPubFormatToolBar->addAction(getAct("removeLPubFormatStepAct.1"));
    removeLPubFormatToolBar->addAction(getAct("removeLPubFormatSubmodelAct.1"));
    removeLPubFormatToolBar->addAction(getAct("removeLPubFormatBomAct.1"));

    visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_REMOVE_LPUB_FORMAT_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_REMOVE_LPUB_FORMAT_TOOLBAR_KEY)).toBool();
    getMenu("editMenu")->addAction(removeLPubFormatToolBar->toggleViewAction());
    removeLPubFormatToolBar->setVisible(visible);
    connect (removeLPubFormatToolBar, SIGNAL (visibilityChanged(bool)),
                                this, SLOT (removeLPubFormatToolBarVisibilityChanged(bool)));

    visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_EDIT_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_EDIT_TOOLBAR_KEY)).toBool();
    getMenu("editMenu")->addAction(editToolBar->toggleViewAction());
    getMenu("editMenu")->addAction(editWindow->editToolBar->toggleViewAction());
    getMenu("editMenu")->addAction(editWindow->toolsToolBar->toggleViewAction());
    editToolBar->setVisible(visible);
    connect (editToolBar, SIGNAL (visibilityChanged(bool)),
                    this, SLOT (editToolBarVisibilityChanged(bool)));

    QToolBar *editParamsToolBar = addToolBar(tr("Edit Parameters Toolbar"));
    editParamsToolBar->setObjectName("editParamsToolBar");
    toolbars.insert(editParamsToolBar->objectName(), editParamsToolBar);
    editParamsToolBar->setObjectName("EditParamsToolbar");
    editParamsToolBar->addAction(getAct("editModelFileAct.1"));
    editParamsToolBar->addSeparator();
    editParamsToolBar->addAction(getAct("editLDrawColourPartsAct.1"));
    editParamsToolBar->addAction(getAct("editPliControlFileAct.1"));
    editParamsToolBar->addAction(getAct("editTitleAnnotationsAct.1"));
    editParamsToolBar->addAction(getAct("editFreeFormAnnitationsAct.1"));
    editParamsToolBar->addAction(getAct("editPliBomSubstitutePartsAct.1"));
    editParamsToolBar->addAction(getAct("editExcludedPartsAct.1"));
    editParamsToolBar->addAction(getAct("editStickerPartsAct.1"));
    editParamsToolBar->addAction(getAct("editAnnotationStyleAct.1"));
    editParamsToolBar->addAction(getAct("editLD2BLCodesXRefAct.1"));
    editParamsToolBar->addAction(getAct("editLD2BLColorsXRefAct.1"));
    editParamsToolBar->addAction(getAct("editBLColorsAct.1"));
    editParamsToolBar->addAction(getAct("editBLCodesAct.1"));
    editParamsToolBar->addAction(getAct("editLD2RBColorsXRefAct.1"));
    editParamsToolBar->addAction(getAct("editLD2RBCodesXRefAct.1"));
    editParamsToolBar->addSeparator();
#if defined Q_OS_WIN
    if (Preferences::portableDistribution){
        editParamsToolBar->addAction(getAct("editLPub3DIniFileAct.1"));
        editParamsToolBar->addSeparator();
    }
#else
    editParamsToolBar->addAction(getAct("editLPub3DIniFileAct.1"));
    editParamsToolBar->addSeparator();
#endif
    editParamsToolBar->addAction(getAct("editNativePOVIniAct.1"));
    editParamsToolBar->addAction(getAct("editLdgliteIniAct.1"));
    editParamsToolBar->addAction(getAct("editLdviewIniAct.1"));
    editParamsToolBar->addAction(getAct("editLdviewPovIniAct.1"));
    editParamsToolBar->addAction(getAct("editPovrayIniAct.1"));
    editParamsToolBar->addAction(getAct("editPovrayConfAct.1"));
    if (!Preferences::blenderExe.isEmpty())
        editParamsToolBar->addAction(getAct("editBlenderParametersAct.1"));
    editParamsToolBar->addSeparator();
    editParamsToolBar->addAction(getAct("generateCustomColourPartsAct.1"));
    editParamsToolBar->addSeparator();
    editParamsToolBar->addAction(getAct("viewLogAct.1"));
    visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_EDITPARAMS_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_EDITPARAMS_TOOLBAR_KEY)).toBool();
    getMenu("editorMenu")->addAction(editParamsToolBar->toggleViewAction());
    editParamsToolBar->setVisible(visible);
    connect (editParamsToolBar, SIGNAL (visibilityChanged(bool)),
                          this, SLOT (editParamsToolBarVisibilityChanged(bool)));

    getAct("previousPageComboAct.1")->setMenu(getMenu("previousPageContinuousMenu"));
    getAct("nextPageComboAct.1")->setMenu(getMenu("nextPageContinuousMenu"));

    QToolBar *navigationToolBar = addToolBar(tr("Navigation Toolbar"));
    navigationToolBar->setObjectName("navigationToolBar");
    toolbars.insert(navigationToolBar->objectName(), navigationToolBar);
    navigationToolBar->setObjectName("NavigationToolbar");
    navigationToolBar->addAction(getAct("firstPageAct.1"));
    navigationToolBar->addSeparator();
    navigationToolBar->addAction(getAct("previousPageComboAct.1"));
    navigationToolBar->addSeparator();
    navigationToolBar->addWidget(setPageLineEdit);
    navigationToolBar->addSeparator();
    navigationToolBar->addAction(getAct("nextPageComboAct.1"));
    navigationToolBar->addSeparator();
    navigationToolBar->addAction(getAct("lastPageAct.1"));
    navigationToolBar->addSeparator();
    navigationToolBar->addWidget(setGoToPageCombo);
    navigationToolBar->addSeparator();
    navigationToolBar->addWidget(mpdCombo);

    QToolBar *zoomToolBar = addToolBar(tr("Zoom Toolbar"));
    zoomToolBar->setObjectName("zoomToolBar");
    toolbars.insert(zoomToolBar->objectName(), zoomToolBar);
    zoomToolBar->setObjectName("ZoomToolbar");
    zoomToolBar->addAction(getAct("fitVisibleAct.1"));
    zoomToolBar->addAction(getAct("fitWidthAct.1"));
    zoomToolBar->addAction(getAct("fitSceneAct.1"));
    zoomToolBar->addAction(getAct("actualSizeAct.1"));

    zoomToolBar->addAction(getAct("bringToFrontAct.1"));
    zoomToolBar->addAction(getAct("sendToBackAct.1"));

    getAct("zoomInComboAct.1")->setMenu(getMenu("zoomSliderMenu"));
    zoomToolBar->addAction(getAct("zoomInComboAct.1"));
    getAct("zoomOutComboAct.1")->setMenu(getMenu("zoomSliderMenu"));
    zoomToolBar->addAction(getAct("zoomOutComboAct.1"));

    getAct("sceneRulerComboAct.1")->setMenu(getMenu("sceneRulerTrackingMenu"));
    zoomToolBar->addAction(getAct("sceneRulerComboAct.1"));

    getAct("sceneGuidesComboAct.1")->setMenu(getMenu("sceneGuidesMenu"));
    zoomToolBar->addAction(getAct("sceneGuidesComboAct.1"));

    getAct("snapToGridComboAct.1")->setMenu(getMenu("snapToGridMenu"));
    zoomToolBar->addAction(getAct("snapToGridComboAct.1"));

    zoomToolBar->addSeparator();
    zoomToolBar->addAction(getAct("fullScreenViewAct.1"));

    if (Preferences::modeGUI)
        create3DToolBars();
}

void Gui::statusBarMsg(QString msg)
{
  statusBar()->showMessage(msg);
}

void Gui::createDockWindows()
{
    commandEditDockWindow = new QDockWidget(trUtf8(wCharToUtf8("Command Editor")), this);
    commandEditDockWindow->setObjectName("CommandEditorDockWindow");
    commandEditDockWindow->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    commandEditDockWindow->setWidget(editWindow);
    addDockWidget(Qt::RightDockWidgetArea, commandEditDockWindow);
    getMenu("viewMenu")->addAction(commandEditDockWindow->toggleViewAction());

    connect(commandEditDockWindow, SIGNAL (topLevelChanged(bool)), this, SLOT (enableWindowFlags(bool)));

    if (Preferences::modeGUI)
        create3DDockWindows();
}

void Gui::importToolBarVisibilityChanged(bool visible)
{
    QSettings Settings;
    Settings.setValue(QString("%1/%2").arg(SETTINGS,VIEW_IMPORT_TOOLBAR_KEY),visible);
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

void Gui::removeLPubFormatToolBarVisibilityChanged(bool visible)
{
    QSettings Settings;
    Settings.setValue(QString("%1/%2").arg(SETTINGS,VIEW_REMOVE_LPUB_FORMAT_TOOLBAR_KEY),visible);
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
    const QByteArray state = Settings.value("State", QByteArray()).toByteArray();
    const QByteArray geometry = Settings.value("Geometry", QByteArray()).toByteArray();
    const QSize size = Settings.value("Size", QDesktopWidget().availableGeometry(this).size()*0.6).toSize();
    if (state.isEmpty()) {
        Settings.setValue("State", saveState());
    }
    restoreState(Settings.value("State").toByteArray());
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

    readNativeSettings();

    QString const MessagesNotShownKey("MessagesNotShown");
    if (Settings.contains(QString("%1/%2").arg(MESSAGES,MessagesNotShownKey))) {
        QByteArray data = Settings.value(QString("%1/%2").arg(MESSAGES,MessagesNotShownKey)).toByteArray();
        QDataStream dataStreamRead(data);
        dataStreamRead >> Preferences::messagesNotShown;
    }
}

void Gui::writeSettings()
{
    QSettings Settings;
    Settings.beginGroup(MAINWINDOW);
    Settings.setValue("Geometry", saveGeometry());
    Settings.setValue("State", saveState());
    Settings.setValue("Size", size());
    Settings.endGroup();

    writeNativeSettings();

    QByteArray data;
    QDataStream dataStreamWrite(&data, QIODevice::WriteOnly);
    dataStreamWrite << Preferences::messagesNotShown;
    QVariant uValue(data);
    QString const MessagesNotShownKey("MessagesNotShown");
    Settings.setValue(QString("%1/%2").arg(MESSAGES,MessagesNotShownKey),uValue);
}

void Gui::showLine(const Where &here, int type)
{
  if (Preferences::modeGUI && ! exporting()) {
    if (macroNesting == 0) {
      displayFile(&lpub->ldrawFile, here);
      showLineSig(here.lineNumber, type);
    }
  }
}

void Gui::parseError(const QString &message, const Where &here, Preferences::MsgKey msgKey, bool option/*false*/, bool override/*false*/)
{
    if (parsedMessages.contains(here))
        return;

    const QString keyType[][2] = {
       // Message Title,           Type Description
       {"Command Meta",            "meta parse error"},               //ParseErrors
       {"Insert Meta",             "insert parse error"},             //InsertErrors
       {"Build Modification Meta", "build modification parse error"}, //BuildModErrors
       {"Include File Meta",       "include file parse error" },      //IncludeFileErrors
       {"Annoatation Meta",        "annotation parse error"}          //AnnotationErrors
    };

    QString parseMessage = QString("%1 (file: %2, line: %3)") .arg(message) .arg(here.modelName) .arg(here.lineNumber + 1);
    if (Preferences::modeGUI && !exporting() && !ContinuousPage()) {
        if (pageProcessRunning == PROC_FIND_PAGE || pageProcessRunning == PROC_DRAW_PAGE) {
            showLine(here, LINE_ERROR);
        }
        bool okToShowMessage = Preferences::getShowMessagePreference(msgKey);
        if (okToShowMessage) {
            Where messageLine = here;
            messageLine.setModelIndex(getSubmodelIndex(messageLine.modelName));
            Preferences::MsgID msgID(msgKey,messageLine.indexToString());
            Preferences::showMessage(msgID, parseMessage, keyType[msgKey][0], keyType[msgKey][1], option, override);
        }
        if (pageProcessRunning == PROC_WRITE_TO_TMP)
            emit progressPermMessageSig(QString("Writing submodel [Parse Error%1")
                                        .arg(okToShowMessage ? "]...          " : " - see log]... " ));
    }

    logError() << qPrintable(parseMessage.replace("<br>"," "));

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
            message = QString("Failed to set log level %1.\n"
                              "Logging is off - level set to OffLevel\n")
                              .arg(Preferences::loggingLevel);
            fprintf(stderr, "%s", qPrintable(message));
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
        if (logType == LOG_COUNT_STATUS ){

            message.replace("<br>"," ");

            if (guiEnabled) {
                statusBarMsg(message);
            } else if (!Preferences::suppressStdOutToLog) {
                fprintf(stdout,"%s", qPrintable(QString(message).append("\n")));
                fflush(stdout);
            }

        } else
          if (logType == LOG_INFO_STATUS) {

              logInfo() << qPrintable(message.replace("<br>"," "));

              if (guiEnabled) {
                  statusBarMsg(message);
              } else if (!Preferences::suppressStdOutToLog) {
                  fprintf(stdout,"%s", qPrintable(QString(message).append("\n")));
                  fflush(stdout);
              }

        } else
         if (logType == LOG_STATUS ){

             logStatus() << qPrintable(message.replace("<br>"," "));

             if (guiEnabled) {
                 statusBarMsg(message);
             } else if (!Preferences::suppressStdOutToLog) {
                 fprintf(stdout,"%s", qPrintable(QString(message).append("\n")));
                 fflush(stdout);
             }

         } else
           if (logType == LOG_INFO) {

               logInfo() << qPrintable(message.replace("<br>"," "));

               if (!guiEnabled && !Preferences::suppressStdOutToLog) {
                   fprintf(stdout,"%s", qPrintable(QString(message).append("\n")));
                   fflush(stdout);
               }

         } else
           if (logType == LOG_NOTICE) {

               logNotice() << qPrintable(message.replace("<br>"," "));

               if (!guiEnabled && !Preferences::suppressStdOutToLog) {
                   fprintf(stdout,"%s", qPrintable(QString(message).append("\n")));
                   fflush(stdout);
               }

         } else
           if (logType == LOG_TRACE) {

               logTrace() << qPrintable(message.replace("<br>"," "));

               if (!guiEnabled && !Preferences::suppressStdOutToLog) {
                   fprintf(stdout,"%s",qPrintable(QString(message).append("\n")));
                   fflush(stdout);
               }

         } else
           if (logType == LOG_DEBUG) {
               logDebug() << qPrintable(message.replace("<br>"," "));

               if (!guiEnabled && !Preferences::suppressStdOutToLog) {
                   fprintf(stdout,"%s", qPrintable(QString(message).replace("<br>"," ").append("\n")));
                   fflush(stdout);
               }

         } else
           if (logType == LOG_ERROR) {

             logError() << qPrintable(QString(message).replace("<br>"," "));

             if (guiEnabled) {
                 if (ContinuousPage() || exporting()) {
                     statusBarMsg(QString(message).replace("<br>"," ").prepend("ERROR: "));
                 } else {
                     QMessageBox::warning(this,tr(VER_PRODUCTNAME_STR),message);
                 }
             } else if (!Preferences::suppressStdOutToLog) {
                 fprintf(stdout,"%s", qPrintable(QString(message).replace("<br>"," ").append("\n")));
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

void LDrawSearchDirDialog::getLDrawSearchDirDialog()
{
  QPalette readOnlyPalette = QApplication::palette();
  if (Preferences::displayTheme == THEME_DARK)
      readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DARK_PALETTE_MIDLIGHT]));
  else
      readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DEFAULT_PALETTE_LIGHT]));
  readOnlyPalette.setColor(QPalette::Text,QColor(LPUB3D_DISABLED_TEXT_COLOUR));

  dialog = new QDialog(nullptr);
  dialog->setWindowTitle(tr("Search Directories"));

  QVBoxLayout *layout = new QVBoxLayout();

  dialog->setLayout(layout);

  QGridLayout *gridLayout = new QGridLayout();

  QVBoxLayout *actionsLayout = new QVBoxLayout();

  QString ldrawSearchDirsTitle = tr("LDraw Content Search Directories for %1").arg(Preferences::validLDrawPartsLibrary);

  QGroupBox *groupBoxSearchDirs = new QGroupBox(ldrawSearchDirsTitle,dialog);

  QGroupBox *groupBoxActions = new QGroupBox(dialog);

  QSpacerItem *actionsSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);

  layout->addWidget(groupBoxSearchDirs);

  groupBoxSearchDirs->setLayout(gridLayout);

  groupBoxActions->setLayout(actionsLayout);

  lineEditIniFile = new QLineEdit(dialog);
  lineEditIniFile->setPalette(readOnlyPalette);
  lineEditIniFile->setReadOnly(true);
  gridLayout->addWidget(lineEditIniFile,0,0,1,2);

  textEditSearchDirs = new QTextEdit(dialog);
  textEditSearchDirs->setWordWrapMode(QTextOption::WordWrap);
  textEditSearchDirs->setLineWrapMode(QTextEdit::FixedColumnWidth);
  textEditSearchDirs->setLineWrapColumnOrWidth(LINE_WRAP_WIDTH);
  gridLayout->addWidget(textEditSearchDirs,1,0,2,1);

  gridLayout->addWidget(groupBoxActions, 1,1,2,1);

  pushButtonAddDirectory = new QPushButton(dialog);
  pushButtonAddDirectory->setToolTip(tr("Add LDraw search directory"));
  pushButtonAddDirectory->setIcon(QIcon(QIcon(":/resources/adddirectory.png")));
  actionsLayout->addWidget(pushButtonAddDirectory);

  pushButtonMoveUp = new QPushButton(dialog);
  pushButtonMoveUp->setToolTip(tr("Move selected directory up"));
  pushButtonMoveUp->setIcon(QIcon(QIcon(":/resources/movedirectoryup.png")));
  pushButtonMoveUp->setEnabled(Preferences::ldSearchDirs.size());
  actionsLayout->addWidget(pushButtonMoveUp);

  pushButtonMoveDown = new QPushButton(dialog);
  pushButtonMoveDown->setToolTip(tr("Move selected directory down"));
  pushButtonMoveDown->setIcon(QIcon(QIcon(":/resources/movedirectorydown.png")));
  pushButtonMoveDown->setEnabled(Preferences::ldSearchDirs.size());
  actionsLayout->addWidget(pushButtonMoveDown);

  pushButtonReset = new QPushButton(dialog);
  pushButtonReset->setToolTip(tr("Reset LPub3D LDraw search directories to default"));
  pushButtonReset->setIcon(QIcon(QIcon(":/resources/resetsetting.png")));
  actionsLayout->addWidget(pushButtonReset);

  actionsLayout->addSpacerItem(actionsSpacer);

  if (Preferences::ldrawiniFound) {
      lineEditIniFile->setText(tr("Using LDraw.ini File: %1").arg(Preferences::ldrawiniFile));
      lineEditIniFile->setToolTip(tr("LDraw.ini file"));
      pushButtonReset->hide();
      textEditSearchDirs->setReadOnly(true);
      textEditSearchDirs->setPalette(readOnlyPalette);
      textEditSearchDirs->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
      textEditSearchDirs->setToolTip("Read only list of LDraw.ini search directories.");
  } else {
      textEditSearchDirs->setToolTip("Editable list of search directories - add or edit search paths. Use a new line for each entry.");
      lineEditIniFile->setText(tr("%1").arg(Preferences::ldSearchDirs.size() ?
                                            tr("Using default %1 search.").arg(VER_PRODUCTNAME_STR) :
                                            tr("Using default search. No search directories detected.")));
      lineEditIniFile->setToolTip(tr("Default search"));
      pushButtonReset->setEnabled(Preferences::ldSearchDirs.size());
  }

  if (Preferences::ldSearchDirs.size() > 0) {
      Q_FOREACH (QString searchDir, Preferences::ldSearchDirs)
        textEditSearchDirs->append(searchDir);
  }

  const QString ldrawPath = Preferences::ldrawLibPath;
  excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(ldrawPath).arg("parts"));
  excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(ldrawPath).arg("p"));
  if (Preferences::usingDefaultLibrary) {
    excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(ldrawPath).arg("unofficial/parts"));
    excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(ldrawPath).arg("unofficial/p"));
  }

  connect(textEditSearchDirs, SIGNAL(textChanged()),this, SLOT(buttonSetState()));
  connect(pushButtonMoveUp, SIGNAL(clicked()),this, SLOT(buttonClicked()));
  connect(pushButtonMoveDown, SIGNAL(clicked()),this, SLOT(buttonClicked()));
  connect(pushButtonAddDirectory, SIGNAL(clicked()),this, SLOT(buttonClicked()));

  QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                             Qt::Horizontal, dialog);

  layout->addWidget(&buttonBox);

  connect(&buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
  connect(&buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

  dialog->setMinimumWidth(500);

  if (dialog->exec() == QDialog::Accepted) {

    QStringList searchDirs = textEditSearchDirs->toPlainText().split("\n");

    if (searchDirs.size() && searchDirs != Preferences::ldSearchDirs) {
      QStringList validDirs, invalidDirs, newDirs;

      Q_FOREACH (const QString &searchDir, searchDirs) {

        bool match = false;
        const QString dir = QDir::toNativeSeparators(searchDir);

        Q_FOREACH (const QString &ldDir, Preferences::ldSearchDirs) {
          if (dir.toLower() == ldDir.toLower()) {
            match = true;
            break;
          }
        }

        if (match) {
          validDirs << dir;
        } else {
          match = false;
          Q_FOREACH (QString excludedDir, excludedSearchDirs) {
            if (dir.toLower() == excludedDir.toLower()) {
              invalidDirs << dir;
              match = true;
              break;
            }
          }
          if (!match) {
            if (QFileInfo(dir).exists()) {
              newDirs << dir;
              validDirs << dir;
            } else {
              invalidDirs << dir;
            }
          }
        }
      }

      Preferences::ldSearchDirs = validDirs;

      if (!invalidDirs.isEmpty()) {
        const QString message("The search directory list contains excluded or invalid paths which will not be saved.");
        QMessageBox::warning(dialog, tr("Search Directories"), tr("%1<br>%2").arg(message).arg(invalidDirs.join("<br>")));
        emit gui->messageSig(LOG_INFO, message);
        Q_FOREACH (const QString &dir, invalidDirs) {
          emit gui->messageSig(LOG_INFO, QString("    - %1").arg(dir));
        }
      }

      if (!validDirs.isEmpty()) {
        emit gui->messageSig(LOG_INFO, QString("Updated LDraw Directories:"));
        Q_FOREACH (const QString &dir, validDirs) {
          emit gui->messageSig(LOG_INFO,QString("    - %1").arg(dir));
        }
      }

      if (newDirs.size()) {
        gui->partWorkerLDSearchDirs.populateUpdateSearcDirs(newDirs);
        gui->loadLDSearchDirParts(false/*Process*/, false/*OnDemand*/, true/*Update*/);
      }
    }
  }
}

void LDrawSearchDirDialog::buttonSetState()
{
  const QStringList searchDirs = textEditSearchDirs->toPlainText().split("\n");
  pushButtonReset->setEnabled(searchDirs.size());
  pushButtonMoveUp->setEnabled(searchDirs.size() > 1);
  pushButtonMoveDown->setEnabled(searchDirs.size() > 1);
}

void LDrawSearchDirDialog::buttonClicked()
{
  textEditSearchDirs->setFocus();
  QMessageBox box;
  if (sender() == pushButtonReset) {
    box.setIcon (QMessageBox::Question);
    box.setWindowTitle(tr ("Reset Search Directories?"));
    box.setDefaultButton   (QMessageBox::Yes);
    box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
    box.setText (tr("This action will reset your search directory settings to the LPub3D default.\n"
                    "Are you sure you want to continue? "));
    if (box.exec() == QMessageBox::Yes) {
      gui->partWorkerLDSearchDirs.resetSearchDirSettings();
      textEditSearchDirs->clear();

      Q_FOREACH (const QString &searchDir, Preferences::ldSearchDirs)
        textEditSearchDirs->append(searchDir);

      box.setIcon (QMessageBox::Information);
      box.setStandardButtons (QMessageBox::Ok);
      box.setText( tr("Search directories have been reset with %1 entries.").arg(Preferences::ldSearchDirs.size()));
      emit gui->messageSig(LOG_STATUS,box.text());
      box.exec();
    }
  } else if (sender() == pushButtonAddDirectory) {
    const QString ldrawPath = Preferences::ldrawLibPath;
    QString result = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(dialog, tr("Select Parts Directory"), QString("%1/%2").arg(ldrawPath).arg("unofficial")));
    Q_FOREACH (QString excludedDir, excludedSearchDirs) {
      if (result.toLower() == excludedDir.toLower()) {
          box.setIcon (QMessageBox::Warning);
          box.setWindowTitle(tr ("Invalid Search Directory"));
          box.setDefaultButton   (QMessageBox::Ok);
          box.setStandardButtons (QMessageBox::Ok);
          box.setText (tr("Invalid search directory %1").arg(result));
          box.exec();
        break;
      }
    }
    if (!result.isEmpty())
      textEditSearchDirs->append(QDir::toNativeSeparators(result));
  } else {
    int origPosition = 0, numChars = 0;
    QTextCursor cursor = textEditSearchDirs->textCursor();
    cursor.beginEditBlock();
    origPosition = cursor.position();
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    numChars = origPosition - cursor.position();
    if (sender() == pushButtonMoveUp) {
      if (cursor.atStart()) {
        cursor.endEditBlock();
        return;
      }
    } else if (sender() == pushButtonMoveDown) {
      cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
      if (cursor.atEnd()) {
        cursor.endEditBlock();
        return;
      }
      cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
      cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
    }

    cursor.select(QTextCursor::LineUnderCursor);
    QTextDocumentFragment selection = cursor.selection();
    if (selection.isEmpty()) {
      cursor.endEditBlock();
      return;
    }
    cursor.removeSelectedText();
    cursor.deleteChar(); // clean up new line
    cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor);
    cursor.insertText(selection.toPlainText()+QChar('\n'));
    cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, numChars);
    cursor.endEditBlock();
    textEditSearchDirs->setTextCursor(cursor);
  }
}

ActionAttributes sgCommands[NUM_GRID_SIZES] =
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
