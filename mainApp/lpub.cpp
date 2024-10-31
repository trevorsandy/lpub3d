
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

#include <LDVQt/LDVWidget.h>

#include "lpub.h"
#include "declarations.h"
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
#include "stickerparts.h"
#include "blenderpreferences.h"
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
#include "commonmenus.h"

//Visual Editor
#include "camera.h"
#include "lc_view.h"
#include "piece.h"
#include "lc_profile.h"
#include "application.h"
#include "lc_partselectionwidget.h"
#include "lc_http.h"

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

const bool showMsgBox = true;

QHash<SceneObject, QString> soMap;

int          Gui::pa;                     // page adjustment
int          Gui::sa;                     // step number adustment
int          Gui::maxPages;
int          Gui::boms;                   // the number of pli BOMs in the document
int          Gui::bomOccurrence;          // the actual occurrence of each pli BO
int          Gui::displayPageNum;         // what page are we displaying
int          Gui::prevDisplayPageNum;     // previously displayed page - used to roll back after exporting or an error.
int          Gui::prevMaxPages;           // previous page count - used by continuousPageDialog to roll back after encountering an error.
int          Gui::processOption;          // export Option
int          Gui::pageDirection;          // page processing direction
int          Gui::savePrevStepPosition;   // indicate the previous step position between current and previous steps  - used for fade/highlight steps and to roll back after exporting or an error.
int          Gui::pageProcessRunning;     // indicate page processing stage - 0=none, 1=writeToTmp,2-find/drawPage...
int          Gui::pageProcessParent;      // the page process that triggers another process - e.g. drawPage -> writeToTmp
int          Gui::firstStepPageNum;       // the first Step page number - used to specify frontCover page
int          Gui::lastStepPageNum;        // the last Step page number - used to specify backCover page
int          Gui::saveRenderer;           // saved renderer when temporarily switching to Native renderer

int          Gui::stepPageNum;            // the number displayed on the page
int          Gui::saveStepPageNum;        // saved instance of the number displayed on the page
int          Gui::saveContStepNum;        // saved continuous step number for steps before displayPage, subModel exit and stepGroup end
int          Gui::saveGroupStepNum;       // saved step group step number when pli per step is false
int          Gui::saveDisplayPageNum;     // saved display page number when counting pages
int          Gui::saveMaxPages;           // saved page count when count (actually parse) build mods requested
qreal        Gui::exportPixelRatio;       // export resolution pixel density

bool         Gui::saveProjection;         // saved projection when temporarily switching to Native renderer
bool         Gui::buildModJumpForward;    // parse build mods in countPage call - special case for jump forward
bool         Gui::resetCache;             // reset model, fade and highlight parts
bool         Gui::submodelIconsLoaded;    // load submodel images

QString      Gui::curFile;                // the file name for MPD, or top level file
QString      Gui::curSubFile;             // whats being displayed in the edit window
QString      Gui::saveFileName;           // user specified output file Name [commandline only]
QString      Gui::pageRangeText;          // page range parameters
QList<Where> Gui::topOfPages;             // topOfStep list of modelName and lineNumber for each page
QList<Where> Gui::parsedMessages;         // previously parsed messages within the current session
QStringList  Gui::messageList;            // message list used when exporting or continuous processing
QStringList  Gui::bomParts;               // list of part strings configured for BOM setup
QList<PliPartGroupMeta> Gui::bomPartGroups;// list of BOM part groups used for multi-page BOMs

bool         Gui::m_exportingContent;     // indicate export/printing underway
bool         Gui::m_exportingObjects;     // indicate exporting non-image object file content
bool         Gui::m_contPageProcessing;   // indicate continuous page processing underway
bool         Gui::m_countWaitForFinished; // indicate wait for countPage to finish on exporting 'return to saved page'
bool         Gui::suspendFileDisplay;     // when true, the endMacro() call will not call displayPage()
bool         Gui::m_partListCSIFile;      // processing part list CSI file
bool         Gui::m_abort;                // set to true when response to critcal error is abort

bool         Gui::doFadeStep;
bool         Gui::doHighlightStep;

bool         Gui::m_fadeStepsSetup;       // enable fade previous steps locally
bool         Gui::m_highlightStepSetup;   // enable highlight current step locally

int          Gui::m_exportMode;           // export Mode
int          Gui::m_saveExportMode;       // saved export mode used when exporting BOM
QString      Gui::m_saveDirectoryName;    // user specified output directory name [commandline only]

RendererData Gui::savedRendererData;      // store current renderer data when temporarily switching renderer;
QMap<int, PageSizeData> Gui::pageSizes;  // page size and orientation object

/***********************************************************************
 * set Native renderer for fast processing
 **********************************************************************/

void Gui::setNativeRenderer() {
    if (Preferences::preferredRenderer != RENDERER_NATIVE) {
        Gui::saveRenderer   = Preferences::preferredRenderer;
        Gui::saveProjection = Preferences::perspectiveProjection;
        Preferences::preferredRenderer     = RENDERER_NATIVE;
        Preferences::perspectiveProjection = true;
        Render::setRenderer(Preferences::preferredRenderer);
    }
}

void Gui::restorePreferredRenderer() {
    if (Gui::saveRenderer != RENDERER_INVALID) {
        Preferences::preferredRenderer      = Gui::saveRenderer;
        Preferences::perspectiveProjection  = Gui::saveProjection;
        Render::setRenderer(Preferences::preferredRenderer);
        Gui::saveRenderer = RENDERER_INVALID;
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
        emit gui->messageSig(LOG_NOTICE, QString("Icon Inserted: Key [%1] Value [%2]")
                                                 .arg(key).arg(value));
        return;
    }
//      emit gui->messageSig(LOG_DEBUG, QString("Icon Exist (Insert Ignored): Key [%1] Value [%2]")
//                                              .arg(key).arg(value));
}

void Gui::fullScreenView()
{
    if (gui->getAct("fullScreenViewAct.1")->isChecked()) {
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
  gui->countPages();
  Gui::displayPageNum = Gui::maxPages;
  Gui::displayPage();
}

void Gui::generateCoverPages()
{
    if (Preferences::generateCoverPages) {
        if (!lpub->mi.frontCoverPageExist())
            lpub->mi.insertCoverPage();

        if (!lpub->mi.backCoverPageExist())
            lpub->mi.appendCoverPage();
    }
}

void Gui::insertFinalModelStep() {
  if (lpub->mi.insertFinalModelStep())
    undoStack->setClean();
}

void Gui::deleteFinalModelStep(bool force) {
  lpub->mi.deleteFinalModelStep(force);
}

//void Gui::insertCoverPage()
//{
//  lpub->mi.insertCoverPage();
//}

//void Gui::appendCoverPage()
//{
//  lpub->mi.appendCoverPage();
//  countPages();
//  ++Gui::displayPageNum;
//  Gui::displayPage();  // display the page we just added
//}

void Gui::insertNumberedPage()
{
  lpub->mi.insertNumberedPage();
}

void Gui::appendNumberedPage()
{
  lpub->mi.appendNumberedPage();
//countPages();
//++Gui::displayPageNum;
//Gui::displayPage();    // display the page we just added
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
    bool prompt = gui->getAct("removeBuildModFormatAct.1")->isChecked();
    Preferences::removeBuildModFormatPreference(prompt);
}

void Gui::removeChildSubmodelFormat()
{
    bool prompt = gui->getAct("removeChildSubmodelFormatAct.1")->isChecked();
    Preferences::removeChildSubmodelFormatPreference(prompt);
}

void Gui::removeLPubFormatting()
{
    int option = RLPF_DOCUMENT;
    int savePageNum = Gui::displayPageNum;
    Gui::displayPageNum = 1 + Gui::pa;
    if (sender() == gui->getAct("removeLPubFormatSubmodelAct.1")) {
        option = RLPF_SUBMODEL;
    } else if (sender() == gui->getAct("removeLPubFormatPageAct.1")) {
        option = RLPF_PAGE;
        Gui::displayPageNum = savePageNum;
    } else if (sender() == gui->getAct("removeLPubFormatStepAct.1")) {
        option = RLPF_STEP;
        Gui::displayPageNum = savePageNum;
    } else if (sender() == gui->getAct("removeLPubFormatBomAct.1")) {
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
        bool fullPath = false;
        QString data;
        if (action == gui->getAct("copyFileNameToClipboardAct.1")) {
            data = QFileInfo(Gui::getCurFile()).fileName();
        } else if (action == gui->getAct("copyFilePathToClipboardAct.1")) {
            fullPath = true;
            data = QDir::toNativeSeparators(Gui::getCurFile());
        } else {
            isImage = true;
            data = action->data().toString();
        }

        if (data.isEmpty()) {
            emit gui->messageSig(LOG_ERROR, QString("Copy to clipboard - Sender: %1, No data detected")
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
        QString fileName = QString("%1 '%2' %3")
                               .arg(isImage ? tr("Image") : tr("File"))
                               .arg(efn.size() > 20 ?
                                        efn.left(17) + "..." +
                                        efn.right(3) : efn)
                               .arg(fullPath ? tr("full path") : tr("name"));

        emit gui->messageSig(LOG_INFO_STATUS, QString("'%1' copied to clipboard.").arg(fileName));
    }
}
#endif

void Gui::displayPage()
{
  if (gui->macroNesting == 0) {
    Gui::setPageProcessRunning(PROC_DISPLAY_PAGE);
    gui->displayPageTimer.start();
    Gui::setAbortProcess(false);
    gui->clearPage(); // this includes freeSteps() so harvest old step items before calling
    DrawPageFlags dpFlags;
    dpFlags.updateViewer = lpub->currentStep ? lpub->currentStep->updateViewer : true;
    Gui::drawPage(dpFlags);
    Gui::pageProcessRunning = PROC_NONE;
    if (Gui::abortProcess()) {
      QApplication::restoreOverrideCursor();
      if (Preferences::modeGUI) {
        if (Gui::displayPageNum > (1 + Gui::pa)) {
          gui->restorePreviousPage();
        } else {
          Gui::setAbortProcess(false);
          gui->closeModelFile();
          if (gui->waitingSpinner->isSpinning())
            gui->waitingSpinner->stop();
        }
      }
    } else if (!Gui::exporting()) {
      Gui::prevDisplayPageNum = Gui::displayPageNum;
    }
  }
}

void Gui::cyclePageDisplay(const int inputPageNum, bool silent/*true*/, bool fileReload/*false*/)
{
  int move = 0;
  int goToPageNum = inputPageNum;

  auto setDirection = [&] (int &move)
  {
    move = goToPageNum - Gui::displayPageNum;
    if (move == 1)
      Gui::pageDirection = PAGE_NEXT;
    else if (move > 1)
      Gui::pageDirection = PAGE_JUMP_FORWARD;
    else if (move == -1)
      Gui::pageDirection = PAGE_PREVIOUS;
    else if (move < -1)
      Gui::pageDirection = PAGE_JUMP_BACKWARD;
  };

  auto cycleDisplay = [&] ()
  {
    if (Preferences::modeGUI && !Gui::exporting())
        gui->enableNavigationActions(false);
    bool savedDlgOpt = Preferences::doNotShowPageProcessDlg;
    int savedProc = Gui::pageProcessRunning;
    Gui::pageProcessRunning = PROC_NONE;
    Preferences::doNotShowPageProcessDlg = true;
    if (Gui::pageDirection < PAGE_BACKWARD)
        gui->nextPageContinuousIsRunning = !gui->nextPageContinuousIsRunning;
    else
        gui->previousPageContinuousIsRunning = !gui->previousPageContinuousIsRunning;
    Gui::continuousPageDialog(Gui::pageDirection < PAGE_BACKWARD ? PAGE_NEXT : PAGE_PREVIOUS);
    Preferences::doNotShowPageProcessDlg = savedDlgOpt;
    Gui::pageProcessRunning = savedProc;
  };

  bool saveCycleEachPage = Preferences::cycleEachPage;
  PageDirection operation = FILE_DEFAULT;

  if (!silent || fileReload) {
    // if dialog or fileReload is true, cycleEachPage = FILE_RELOAD (1), else cycleEachPage = FILE_DEFAULT(0) do not cycle
    int cycleEachPage = Preferences::cycleEachPage || fileReload;
    if (!cycleEachPage && (Gui::displayPageNum > 1 + Gui::pa || inputPageNum > 1 + Gui::pa)) {
      const QString directionName[] = {
        tr("Next"),
        tr("Jump Forward"),
        tr("Previous"),
        tr("Jump Backward")
      };
      setDirection(move);
      // On page update, (move = 0), subtract first page from displayPageNum.
      int pages = move ? qAbs(move) : Gui::displayPageNum - (1 + Gui::pa);
      const QString message = tr("Cycle each of the %1 pages for the model file %2 %3 ?")
                                 .arg(pages)
                                 .arg(directionName[Gui::pageDirection-1].toLower())
                                 .arg(fileReload ? tr("reload") : tr("load"));
      int result = CycleDialog::getCycle(tr("%1 Page %2")
                                            .arg(VER_PRODUCTNAME_STR)
                                            .arg(directionName[Gui::pageDirection-1]), message, nullptr);
      if (result == CycleYes)
        cycleEachPage = 1; // FILE_RELOAD
      else if (result == CycleNo)
        cycleEachPage = 0; // FILE_DEFAULT
      else if (result == CycleCancel)
        return;
    }

    if (cycleEachPage) {
      Preferences::setCyclePageDisplay(cycleEachPage);
      if (Preferences::buildModEnabled)
        cycleEachPage = PAGE_JUMP_FORWARD;
    }

    operation = PageDirection(cycleEachPage);
  }

  QElapsedTimer t;
  t.start();
  if (operation == FILE_RELOAD) {
    int savePage = Gui::displayPageNum;
    if (gui->openFile(Gui::curFile)) {
      goToPageNum = Gui::pa ? savePage + Gui::pa : savePage;
      Gui::displayPageNum = 1 + Gui::pa;
      if (!move)
        setDirection(move);
      if (move > 1) {
        cycleDisplay();
        gui->enableActions();
      } else {
        Gui::displayPageNum = goToPageNum;
        Gui::displayPage();
      }
    }
  } else {
    if (!move)
      setDirection(move);
    if (move > 1 && Preferences::cycleEachPage) {
      cycleDisplay();
    } else {
      Gui::displayPageNum = goToPageNum;
      Gui::displayPage();
    }
  }

  Preferences::setCyclePageDisplay(saveCycleEachPage);

  if (Preferences::modeGUI && ! Gui::exporting() && ! Gui::abortProcess()) {
    gui->enableEditActions();
    if (!Gui::ContinuousPage())
      gui->enableNavigationActions(true);
    QApplication::restoreOverrideCursor();
  }

  Gui::pageProcessRunning = PROC_NONE;
}

void Gui::cycleEachPage()
{
    bool cycleEachPageCompare  = Preferences::cycleEachPage;
    Preferences::cycleEachPage = gui->getAct("cycleEachPageAct.1")->isChecked();
    bool cycleEachPageChanged  = Preferences::cycleEachPage  != cycleEachPageCompare;

    if (cycleEachPageChanged) {
        QSettings Settings;
        QString const cycleEachPageKey("CycleEachPage");
        Settings.setValue(QString("%1/%2").arg(SETTINGS,cycleEachPageKey), Preferences::cycleEachPage);
        emit gui->messageSig(LOG_INFO,tr("Cycle each page step(s) when navigating forward by more than one page is %1").arg(Preferences::cycleEachPage? "ON" : "OFF"));
    }
}

void Gui::enableNavigationActions(bool enable)
{
  bool enabled = !Preferences::doNotShowPageProcessDlg && enable;
  bool nextEnabled = Gui::pageDirection < PAGE_BACKWARD ? enable : enabled;
  bool previousEnabled = Gui::pageDirection >= PAGE_BACKWARD ? enable : enabled;
  bool atStart = Gui::displayPageNum == (1 + Gui::pa);
  bool atEnd = Gui::displayPageNum == Gui::maxPages;

  gui->setPageLineEdit->setEnabled(enabled);
  gui->setGoToPageCombo->setEnabled(gui->setGoToPageCombo->count() && enabled);
  gui->mpdCombo->setEnabled(gui->mpdCombo->count() && enabled);

  gui->getAct("firstPageAct.1")->setEnabled(!atStart && enabled);

  gui->getAct("lastPageAct.1")->setEnabled(!atEnd && enabled);

  gui->getAct("nextPageAct.1")->setEnabled(!atEnd && enabled);
  gui->getAct("previousPageAct.1")->setEnabled(!atStart && enabled);

  gui->getAct("nextPageComboAct.1")->setEnabled(!atEnd && nextEnabled);
  gui->getAct("previousPageComboAct.1")->setEnabled(!atStart && previousEnabled);

  gui->getAct("nextPageContinuousAct.1")->setEnabled(!atEnd && nextEnabled);
  gui->getAct("previousPageContinuousAct.1")->setEnabled(!atStart && previousEnabled);
}

void Gui::pageProcessUpdate()
{
    if (!Gui::exporting() && !Gui::ContinuousPage())
        return;
    QCoreApplication::processEvents();
}

void Gui::nextPage()
{
  QString string = setPageLineEdit->displayText();
  QRegExp rx("^(\\d+).*$");
  if (string.contains(rx)) {
    bool ok;
    int inputPageNum = rx.cap(1).toInt(&ok);
    if (ok && (inputPageNum != Gui::displayPageNum)) { // numbers are different so jump to page
      gui->countPages();
      if (inputPageNum <= Gui::maxPages) {
        if (!gui->saveBuildModification())
          return;
        gui->cyclePageDisplay(inputPageNum, !Preferences::buildModEnabled);
        return;
      } else {
        gui->statusBarMsg("Page number entered is higher than total pages");
      }
      string = tr("%1 of %2") .arg(Gui::displayPageNum) .arg(Gui::maxPages);
      setPageLineEdit->setText(string);
      return;
  } else {   // numbers are same so goto next page
      gui->countPages();
      if (Gui::displayPageNum < Gui::maxPages) {
        if (!gui->saveBuildModification())
          return;
        Gui::pageDirection = PAGE_NEXT;
        ++Gui::displayPageNum;
        Gui::displayPage();
      } else {
        gui->statusBarMsg(tr("You are on the last page"));
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
  box.setWindowTitle(tr ("Next Page Continuous Processing"));
  box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  QString title = tr ("<b> Next page continuous processing </b>");
  QString message = tr ("Continuous processing is running.\n");

  Gui::setPageProcessRunning(PROC_NONE);

  // Request to terminate Next page process while it is still running
  if (!gui->nextPageContinuousIsRunning) {
    box.setIcon (QMessageBox::Warning);
    title = tr ("<b> Stop next page continuous processing ? </b>");
    message = tr ("Continuous processing has not completed.\n\n"
                  "Terminate this process ?");
    box.setText (title);
    box.setInformativeText (message);

    if (box.exec() == QMessageBox::Yes) { // Yes, let's terminate
      Gui::cancelContinuousPage();
      return;
    } else {                              // Oops, do not want to terminate
      setPageContinuousIsRunning(true, PAGE_NEXT);
    }
  }
  // If user clicks Next page process while Previous page process is still running
  else if (previousPageContinuousIsRunning && nextPageContinuousIsRunning) {
    box.setIcon (QMessageBox::Warning);
    title = tr ("<b>Stop previous page continuous processing ? </b>");
    message = tr ("Continuous processing is running.\n\n"
                  "Restart continuous processing ?");

    box.setText (title);
    box.setInformativeText (message);

    // User wants to stop Previous page process
    if (box.exec() == QMessageBox::Yes) {
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
    if (ok && (inputPageNum != Gui::displayPageNum)) { // numbers are different so jump to page
      countPages();
      if (inputPageNum >= 1 + Gui::pa && inputPageNum != Gui::displayPageNum) {
        if (!saveBuildModification())
          return;
        const int move = inputPageNum - Gui::displayPageNum;
        Gui::pageDirection  = move < -1 ? PAGE_JUMP_BACKWARD : PAGE_PREVIOUS;
        Gui::displayPageNum = inputPageNum;
        Gui::displayPage();
        return;
      } else {
        gui->statusBarMsg(tr("Page number entered is invalid"));
      }
      string = tr("%1 of %2") .arg(Gui::displayPageNum) .arg(Gui::maxPages);
      setPageLineEdit->setText(string);
      return;
    } else {               // numbers are same so goto previous page
      if (Gui::displayPageNum > 1 + Gui::pa) {
        if (!gui->saveBuildModification())
          return;
        Gui::pageDirection = PAGE_PREVIOUS;
        Gui::displayPageNum--;
        Gui::displayPage();
      } else {
        gui->statusBarMsg("You are on the first page");
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
  box.setWindowTitle(tr ("Previous Page Continuous Processing"));
  box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
  QString title = tr ("<b> Previous page continuous processing </b>");
  QString message = tr ("Continuous processing is running.\n");

  Gui::setPageProcessRunning(PROC_NONE);

  // Request to terminate Previous page process while it is still running
  if (!previousPageContinuousIsRunning) {
    box.setIcon (QMessageBox::Warning);
    title = tr ("<b> Stop previous page continuous processing ? </b>");
    message = tr ("Continuous processing has not completed.\n\n"
                  "Terminate this process ?");

    box.setText (title);
    box.setInformativeText (message);

    if (box.exec() == QMessageBox::Yes) {   // Yes, let's terminate
      Gui::cancelContinuousPage();
      return;
    } else {                                // Oops, do not want to terminate
      setPageContinuousIsRunning(true, PAGE_PREVIOUS);
    }
  }
  // If user clicks Previous page process while Next page process is still running
  else if (nextPageContinuousIsRunning  && previousPageContinuousIsRunning) {
    box.setIcon (QMessageBox::Warning);
    title = tr ("<b> Stop next page continuous processing ? </b>");
    message = tr ("Continuous processing is running.\n\n"
                  "Restart continuous processing ?");

    box.setText (title);
    box.setInformativeText (message);

    // User wants to stop Next page process
    if (box.exec() == QMessageBox::Yes) {
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

void Gui::setPageContinuousIsRunning(bool b, PageDirection d) {

    if (d != DIRECTION_NOT_SET) Gui::pageDirection = d;

    if (Gui::pageDirection == PAGE_NEXT) {
        gui->nextPageContinuousIsRunning = b;
    } else {
        gui->previousPageContinuousIsRunning = b;
    }
}

void Gui::setContinuousPageAct(PageActType p) {
    if (p == SET_STOP_ACTION) {
        if (Gui::pageDirection == PAGE_NEXT) {
            gui->getAct("nextPageContinuousAct.1")->setIcon(QIcon(":/resources/nextpagecontinuousstop.png"));
            gui->getAct("nextPageContinuousAct.1")->setStatusTip(tr("Stop continuous next page processing"));
            gui->getAct("nextPageContinuousAct.1")->setText(tr("Stop Continuous Next Page"));
        } else {
            gui->getAct("previousPageContinuousAct.1")->setIcon(QIcon(":/resources/prevpagecontinuousstop.png"));
            gui->getAct("previousPageContinuousAct.1")->setStatusTip(tr("Stop continuous previous page processing"));
            gui->getAct("previousPageContinuousAct.1")->setText(tr("Stop Continuous Previous Page"));
        }
    } else { // SET_DEFAULT_ACTION
        if (Gui::pageDirection == PAGE_NEXT) {
            gui->getAct("nextPageContinuousAct.1")->setIcon(QIcon(":/resources/nextpagecontinuous.png"));
            gui->getAct("nextPageContinuousAct.1")->setStatusTip(tr("Continuously process next document page"));
            gui->getAct("nextPageContinuousAct.1")->setText(tr("Continuous Next Page"));
            Gui::setPageContinuousIsRunning(false);
        } else {
            gui->getAct("previousPageContinuousAct.1")->setIcon(QIcon(":/resources/prevpagecontinuous.png"));
            gui->getAct("previousPageContinuousAct.1")->setStatusTip(tr("Continuously process previous document page"));
            gui->getAct("previousPageContinuousAct.1")->setText(tr("Continuous Previous Page"));
            Gui::setPageContinuousIsRunning(false);
        }
    }
}

bool Gui::continuousPageDialog(PageDirection d)
{
  Gui::messageList.clear();
  Gui::pageDirection = d;
  int pageCount = 0;
  int _maxPages = 0;
  Gui::prevMaxPages  = Gui::maxPages;
  bool terminateProcess = false;
  emit gui->setContinuousPageSig(true);
  QElapsedTimer continuousTimer;
  const QString direction = d == PAGE_NEXT ? tr("Next") : tr("Previous");
  QString const pageRangeDisplayText = QString("%1 of %2") .arg(Gui::displayPageNum) .arg(Gui::maxPages);

  Gui::m_exportMode = PAGE_PROCESS;

  Gui::setPageProcessRunning(PROC_DISPLAY_PAGE);

  QString message = tr("%1 Page Processing...").arg(direction);

  if (Preferences::modeGUI) {
      if (Preferences::doNotShowPageProcessDlg) {
          if (!gui->processPageRange(gui->setPageLineEdit->displayText())) {
              emit gui->messageSig(LOG_STATUS,tr("Continuous %1 page processing terminated.").arg(direction));
              Gui::setPageContinuousIsRunning(false);
              emit gui->setContinuousPageSig(false);
              gui->setPageLineEdit->setText(pageRangeDisplayText);
              Gui::revertPageProcess();
              return false;
          }
          continuousTimer.start();
      }
      else
      {
          DialogExportPages *dialog = new DialogExportPages();

          if (dialog->exec() == QDialog::Accepted) {
              continuousTimer.start();

              // initialize progress dialogue
              gui->m_progressDialog->setWindowTitle(tr("Continuous %1 Page Processing").arg(direction));
              gui->m_progressDialog->setLabelText(message);
              gui->m_progressDialog->setBtnToCancel();
              gui->m_progressDialog->setPageDirection(d);
              gui->m_progressDialog->show();
              QApplication::setOverrideCursor(Qt::ArrowCursor);
              QCoreApplication::processEvents();
              disconnect (gui->m_progressDialog, SIGNAL (cancelClicked()), gui, SLOT (cancelExporting()));
              connect    (gui->m_progressDialog, SIGNAL (cancelNextPageContinuous()),gui, SLOT (nextPageContinuous()));
              connect    (gui->m_progressDialog, SIGNAL (cancelPreviousPageContinuous()),gui, SLOT (previousPageContinuous()));

              if(dialog->allPages()) {
                  if (dialog->allPagesRange()) {
                      Gui::processOption = EXPORT_PAGE_RANGE;
                      Gui::pageRangeText = dialog->allPagesRangeText();
                  } else {
                      Gui::processOption = EXPORT_ALL_PAGES;
                  }
              }
              else
              if(dialog->pageRange()) {
                  Gui::processOption  = EXPORT_PAGE_RANGE;
                  Gui::pageRangeText = dialog->pageRangeText();
              }

              Gui::resetCache = dialog->resetCache();

              QSettings Settings;
              if (Preferences::doNotShowPageProcessDlg != dialog->doNotShowPageProcessDlg()) {
                  Preferences::doNotShowPageProcessDlg = dialog->doNotShowPageProcessDlg();
                  QVariant uValue(Preferences::doNotShowPageProcessDlg);
                  Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DoNotShowPageProcessDlg"),uValue);
              }

              if (Preferences::pageDisplayPause != dialog->pageDisplayPause()) {
                  Preferences::pageDisplayPause = dialog->pageDisplayPause();
                  Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PageDisplayPause"),Preferences::pageDisplayPause);
              }
          } else {
              emit gui->messageSig(LOG_STATUS,tr("Continuous %1 page processing terminated.").arg(direction));
              Gui::setPageContinuousIsRunning(false);
              emit gui->setContinuousPageSig(false);
              Gui::revertPageProcess();
              return false;
          }
      }
  }
  else
  { // command line mode
      continuousTimer.start();
      // Set processing direction - This effectively acts like a toggle.
      // If true, it sets false and vice versa.
      gui->nextPageContinuousIsRunning = !gui->nextPageContinuousIsRunning;
  }

  // Validate the page range
  if (Gui::processOption == EXPORT_PAGE_RANGE) {
      if (! gui->validatePageRange()) {
          message = tr("Continuous %1 page processing terminated.").arg(direction);
          gui->m_progressDialog->setBtnToClose();
          gui->m_progressDialog->setLabelText(message, true/*alert*/);
          emit gui->messageSig(LOG_STATUS,message);
          Gui::setPageContinuousIsRunning(false);
          emit gui->setContinuousPageSig(false);
          gui->setPageLineEdit->setText(pageRangeDisplayText);
          Gui::revertPageProcess();
          return false;
      }
  }

  // Process is running - configure to stop
  setContinuousPageAct(SET_STOP_ACTION);

  if(Gui::resetCache)
      gui->resetModelCache();

  emit gui->messageSig(LOG_STATUS,tr("Starting %1").arg(message));

  if (Gui::processOption == EXPORT_ALL_PAGES) {

      _maxPages = Gui::maxPages;

      if (Preferences::modeGUI) {
          gui->m_progressDialog->setWindowTitle(tr("Continuous %1 Page Processing").arg(direction));
          gui->m_progressDialog->setLabelText(message);
          gui->m_progressDialog->setRange(0,_maxPages);
          QCoreApplication::processEvents();
      }

      int progress = 0;

      for (d == PAGE_NEXT ? Gui::displayPageNum = 1 + Gui::pa : Gui::displayPageNum = Gui::maxPages ; d == PAGE_NEXT ? Gui::displayPageNum <= Gui::maxPages : Gui::displayPageNum >= 1 + Gui::pa ; d == PAGE_NEXT ? Gui::displayPageNum++ : Gui::displayPageNum--) {

          if (! Gui::ContinuousPage())
              Gui::setPageContinuousIsRunning(false,d);

          if (d == PAGE_NEXT) {
              terminateProcess =  !gui->nextPageContinuousIsRunning;
          } else {
              terminateProcess =  !gui->previousPageContinuousIsRunning;
          }

          if (terminateProcess) {
              message = tr("%1 page processing terminated before completion. %2 pages of %3 processed%4.")
                           .arg(direction)
                           .arg(d == PAGE_NEXT ? (Gui::displayPageNum - 1) : (Gui::maxPages - (Gui::displayPageNum - 1)))
                           .arg(Gui::maxPages)
                           .arg(QString(". %1").arg(Gui::elapsedTime(continuousTimer.elapsed())));
              emit gui->messageSig(LOG_STATUS,message);
              Gui::setContinuousPageAct(SET_DEFAULT_ACTION);
              emit gui->setContinuousPageSig(false);
              if (Preferences::modeGUI) {
                QApplication::restoreOverrideCursor();
                gui->m_progressDialog->setBtnToClose();
                gui->m_progressDialog->setLabelText(message, true/*alert*/);
                disconnect (gui->m_progressDialog, SIGNAL (cancelNextPageContinuous()),gui, SLOT (nextPageContinuous()));
                disconnect (gui->m_progressDialog, SIGNAL (cancelPreviousPageContinuous()),gui, SLOT (previousPageContinuous()));
                connect    (gui->m_progressDialog, SIGNAL (cancelClicked()), gui, SLOT (cancelExporting()));
              }
              Gui::revertPageProcess();
              return false;
          }

          gui->setPageLineEdit->setText(QString("%1 of %2") .arg(Gui::displayPageNum) .arg(Gui::maxPages));

          pageCount = Gui::displayPageNum;

          Gui::displayPage();

          message = tr("%1 Page Processing - Processed page %2 of %3.")
                       .arg(direction)
                       .arg(pageCount)
                       .arg(Gui::maxPages);
          emit gui->messageSig(LOG_STATUS,message);

          if (Preferences::modeGUI) {
              gui->enableNavigationActions(true);
              gui->m_progressDialog->setLabelText(message);
              gui->m_progressDialog->setValue(d == PAGE_NEXT ? Gui::displayPageNum : ++progress);
              if (d == PAGE_NEXT ? Gui::displayPageNum < Gui::maxPages : Gui::displayPageNum > 1 + Gui::pa) {
                  QTime waiting = QTime::currentTime().addSecs(Preferences::pageDisplayPause);
                  while (QTime::currentTime() < waiting)
                      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
              }
          }
      }

      if (Preferences::modeGUI)
          QApplication::restoreOverrideCursor();

  }
  else
  // Processing page range
  if (processOption == EXPORT_PAGE_RANGE) {

      QStringList pageRanges = Gui::pageRangeText.split(",");
      QList<int> printPages;
      // Split page range values
      for (QString &ranges : pageRanges) {
          // If n-n range, split into start through end pages
          if (ranges.contains("-")) {
              QStringList range = ranges.split("-");
              int startPage = range[0].toInt();
              int endPage = range[1].toInt();
              if (d == PAGE_NEXT) {
                  for(int i = startPage; i <= endPage; i++) {
                      printPages.append(i);
                  }
              } else {
                  for(int i = startPage; i >= endPage; i--) {
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

      Gui::displayPageNum = printPages.first();

      _maxPages = printPages.count();

      if (Preferences::modeGUI) {
          gui->m_progressDialog->setWindowTitle(tr("Continuous %1 Page Processing").arg(direction));
          gui->m_progressDialog->setLabelText(message);
          gui->m_progressDialog->setRange(0,_maxPages);
          QCoreApplication::processEvents();
      }

      // process each page
      for (int printPage : printPages) {

          Gui::displayPageNum = printPage;

          if (! Gui::ContinuousPage()) {
              Gui::setPageContinuousIsRunning(false,d);
          }

          if (d == PAGE_NEXT) {
              terminateProcess = !gui->nextPageContinuousIsRunning;
          } else {
              terminateProcess = !gui->previousPageContinuousIsRunning;
          }

          if (terminateProcess) {
              message = tr("%1 page processing terminated before completion. %2 pages of %3 processed%4.")
                           .arg(direction)
                           .arg(d == PAGE_NEXT ? pageCount : (_maxPages - pageCount))
                           .arg(_maxPages)
                           .arg(QString(". %1").arg(Gui::elapsedTime(continuousTimer.elapsed())));
              emit gui->messageSig(LOG_STATUS,message);
              Gui::setContinuousPageAct(SET_DEFAULT_ACTION);
              emit gui->setContinuousPageSig(false);
              if (Preferences::modeGUI) {
                  QApplication::restoreOverrideCursor();
                  gui->m_progressDialog->setBtnToClose();
                  gui->m_progressDialog->setLabelText(message, true/*alert*/);
                  disconnect (gui->m_progressDialog, SIGNAL (cancelNextPageContinuous()),gui, SLOT (nextPageContinuous()));
                  disconnect (gui->m_progressDialog, SIGNAL (cancelPreviousPageContinuous()),gui, SLOT (previousPageContinuous()));
                  connect    (gui->m_progressDialog, SIGNAL (cancelClicked()), gui, SLOT (cancelExporting()));
              }
              Gui::revertPageProcess();
              return false;
          }

          gui->setPageLineEdit->setText(QString("%1 of %2") .arg(Gui::displayPageNum) .arg(Gui::maxPages));

          pageCount++;

          Gui::displayPage();

          message = tr("%1 Page Processing - Processed page %2 (%3 of %4) from the range of %5")
                       .arg(direction)
                       .arg(Gui::displayPageNum)
                       .arg(pageCount)
                       .arg(_maxPages)
                       .arg(pageRanges.join(" "));
          emit gui->messageSig(LOG_STATUS,message);

          if (Preferences::modeGUI) {
              gui->enableNavigationActions(true);
              gui->m_progressDialog->setLabelText(message);
              gui->m_progressDialog->setValue(pageCount);
              if (Gui::displayPageNum < _maxPages) {
                  QTime waiting = QTime::currentTime().addSecs(Preferences::pageDisplayPause);
                  while (QTime::currentTime() < waiting)
                      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
              }
          }
      }

      if (Preferences::modeGUI) {
          QApplication::restoreOverrideCursor();
      }

      setContinuousPageAct(SET_DEFAULT_ACTION);

      message = tr("%1 page processing completed. %2 of %3 %4 processed%5.")
                   .arg(direction)
                   .arg(d == PAGE_NEXT ? pageCount : _maxPages)
                   .arg(_maxPages)
                   .arg(_maxPages > 1 ? tr("pages") : tr("page"))
                   .arg(QString(". %1").arg(Gui::elapsedTime(continuousTimer.elapsed())));
      emit gui->messageSig(LOG_INFO_STATUS,message);

      if (Preferences::modeGUI) {
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
          gui->m_progressDialog->setBtnToClose();
          gui->m_progressDialog->setLabelText(message);
          gui->m_progressDialog->setValue(_maxPages);
          disconnect (gui->m_progressDialog, SIGNAL (cancelNextPageContinuous()),gui, SLOT (nextPageContinuous()));
          disconnect (gui->m_progressDialog, SIGNAL (cancelPreviousPageContinuous()),gui, SLOT (previousPageContinuous()));
          connect    (gui->m_progressDialog, SIGNAL (cancelClicked()), gui, SLOT (cancelExporting()));
      }
  }

  emit gui->setContinuousPageSig(false);

  Gui::setPageProcessRunning(PROC_NONE);

  return true;
}

bool Gui::processPageRange(const QString &range)
{
  if (!range.isEmpty()) {
      int rangeMin = 0, rangeMax = 0;
      bool ok[2] = {false, false};
      bool multiRange = range.contains(',');
      bool ofPages = range.contains("of",Qt::CaseInsensitive);
      QRegExp startRx("^(\\d+)(?:[\\w\\-\\,\\s]*)$", Qt::CaseInsensitive);
      QRegExp endRx("([^\\s|^,|^\\-|^a-zA-Z]\\d+)$");
      QString cleanRange = range.trimmed().replace(QRegExp("of|to",Qt::CaseInsensitive),"-").replace(" ", "");
      if (cleanRange.contains(startRx)) {
          rangeMin = startRx.cap(1).toInt(&ok[0]);
          if (cleanRange.contains(endRx))
              rangeMax = endRx.cap(1).toInt(&ok[1]);
          if (ok[0] && ok[1] && rangeMin > rangeMax)
              Gui::pageDirection = PAGE_BACKWARD;
      }

      bool stdRange = !multiRange && (!rangeMax || ofPages);
      if (Gui::pageDirection < PAGE_BACKWARD) {
          if (ok[0] && rangeMin > Gui::displayPageNum && stdRange) {
              rangeMax = rangeMin;
              rangeMin = Gui::displayPageNum;
              ok[1]    = true;
          }
      } else {
          if (ok[0] && rangeMin < Gui::displayPageNum && stdRange) {
              rangeMax = Gui::displayPageNum;
              ok[1]    = true;
          }
      }

      const QString allPages = QString("%1-%2").arg(1 + Gui::pa).arg(Gui::maxPages);
      const QString pageRange = QString("%1-%2").arg(rangeMin).arg(rangeMax);

      if ((allPages == pageRange) && !multiRange) {
          Gui::processOption = EXPORT_ALL_PAGES;
      } else {
          Gui::processOption = EXPORT_PAGE_RANGE;
          Gui::pageRangeText = stdRange ? pageRange : cleanRange;
      }
      return true;
    } else {
      Gui::processOption = EXPORT_ALL_PAGES;
      return true;
    }
  return false;
}

void Gui::restorePreviousPage()
{
    if (!Preferences::modeGUI)
        return;

    Gui::displayPageNum = 1 + Gui::pa;
    Gui::maxPages = Gui::prevMaxPages != Gui::displayPageNum ? Gui::prevMaxPages : Gui::displayPageNum;
    gui->countPages();
    gui->cyclePageDisplay(Gui::prevDisplayPageNum != Gui::displayPageNum ? Gui::prevDisplayPageNum : Gui::displayPageNum);
}

void Gui::firstPage()
{
  if (Gui::displayPageNum == 1 + Gui::pa) {
    gui->statusBarMsg("You are on the first page");
  } else {
    if (!gui->saveBuildModification())
        return;
    if ((1 + Gui::pa) - Gui::displayPageNum < -1)
        Gui::pageDirection = PAGE_JUMP_BACKWARD;
    else if ((1 + Gui::pa) - Gui::displayPageNum == -1)
        Gui::pageDirection = PAGE_PREVIOUS;
    Gui::displayPageNum = 1 + Gui::pa;
    Gui::displayPage();
  }
}

void Gui::lastPage()
{
  if (Gui::displayPageNum == Gui::maxPages) {
    gui->statusBarMsg("You are on the last page");
  } else {
    gui->countPages();
    if (!saveBuildModification())
      return;
    gui->cyclePageDisplay(Gui::maxPages, !Preferences::buildModEnabled);
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
      gui->countPages();
      if (inputPageNum <= Gui::maxPages && inputPageNum != Gui::displayPageNum) {
        if (!gui->saveBuildModification())
          return;
        gui->cyclePageDisplay(inputPageNum, !Preferences::buildModEnabled);
        return;
      } else {
        gui->statusBarMsg("Page number entered is higher than total pages");
      }
    }
  }
  string = QString("%1 of %2") .arg(Gui::displayPageNum) .arg(Gui::maxPages);
  setPageLineEdit->setText(string);
}

void Gui::setGoToPage(int index)
{
  const int goToPageNum = index+1;
  gui->countPages();
  if (goToPageNum <= Gui::maxPages && goToPageNum != displayPageNum) {
    if (!gui->saveBuildModification())
      return;
    gui->cyclePageDisplay(goToPageNum, !Preferences::buildModEnabled);
  }

  QString string = QString("%1 of %2") .arg(Gui::displayPageNum) .arg(Gui::maxPages);
  setPageLineEdit->setText(string);
}

void Gui::pageLineEditReset()
{
    if (setPageLineEdit) {
        gui->getAct("setPageLineEditResetAct.1")->setEnabled(false);
        setPageLineEdit->setText(QString("%1 of %2").arg(Gui::displayPageNum).arg(Gui::maxPages));
    }
}

void Gui::enablePageLineReset(const QString &displayText)
{
    if (setPageLineEdit)
        gui->getAct("setPageLineEditResetAct.1")->setEnabled(displayText != QString("%1 of %2").arg(Gui::displayPageNum).arg(Gui::maxPages));
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
  bool isChecked = gui->getAct("sceneGuidesComboAct.1")->isChecked();
  QIcon pageGuidesIcon;
  if (isChecked) pageGuidesIcon.addFile(":/resources/pageguidescheck.png");
  else pageGuidesIcon.addFile(":/resources/pageguides.png");
  gui->getAct("sceneGuidesComboAct.1")->setIcon(pageGuidesIcon);
  Preferences::setSceneGuidesPreference(isChecked);
  SceneGuidesLineGroup->setEnabled(isChecked);
  SceneGuidesPosGroup->setEnabled(isChecked);
  gui->getAct("showGuidesCoordinatesAct.1")->setEnabled(isChecked);
  KpageView->setSceneGuides();
}

void Gui::sceneGuidesLine()
{
  int line = int(Qt::DashLine);
  if (gui->getAct("sceneGuidesSolidLineAct.1")->isChecked())
      line = int(Qt::SolidLine);

  Preferences::setSceneGuidesLinePreference(line);
  KpageView->setSceneGuidesLine();
}

void Gui::sceneGuidesPosition()
{
    int position = int(GUIDES_TOP_LEFT);
    if (gui->getAct("sceneGuidesPosTRightAct.1")->isChecked())
        position = int(GUIDES_TOP_RIGHT);
    else
    if (gui->getAct("sceneGuidesPosBLeftAct.1")->isChecked())
        position = int(GUIDES_BOT_LEFT);
    else
    if (gui->getAct("sceneGuidesPosBRightAct.1")->isChecked())
        position = int(GUIDES_BOT_RIGHT);
    else
    if (gui->getAct("sceneGuidesPosCentreAct.1")->isChecked())
        position = int(GUIDES_CENTRE);

    Preferences::setSceneGuidesPositionPreference(position);
    KpageView->setSceneGuidesPos();
}

void Gui::sceneRuler()
{
  bool isChecked = gui->getAct("sceneRulerComboAct.1")->isChecked();
  QIcon sceneRulerIcon;
  if (isChecked) sceneRulerIcon.addFile(":/resources/pagerulercheck.png");
  else sceneRulerIcon.addFile(":/resources/pageruler.png");
  gui->getAct("sceneRulerComboAct.1")->setIcon(sceneRulerIcon);
  Preferences::setSceneRulerPreference(isChecked);
  gui->getAct("hideRulerPageBackgroundAct.1")->setEnabled(isChecked);
  SceneRulerGroup->setEnabled(isChecked);
  KpageView->setSceneRuler();
}

void Gui::sceneRulerTracking()
{
  RulerTrackingType trackingStyle = TRACKING_NONE;
  if (gui->getAct("sceneRulerTrackingTickAct.1")->isChecked())
      trackingStyle = TRACKING_TICK;
  else
  if (gui->getAct("sceneRulerTrackingLineAct.1")->isChecked()) {
      trackingStyle = TRACKING_LINE;
      gui->getAct("showTrackingCoordinatesAct.1")->setEnabled(true);
  }

  Preferences::setSceneRulerTrackingPreference(int(trackingStyle));
  KpageView->setSceneRulerTracking();
}

void Gui::snapToGrid()
{
  bool isChecked = gui->getAct("snapToGridComboAct.1")->isChecked();
  if (Preferences::snapToGrid == isChecked)
      return;
  QIcon snapToGridIcon;
  if (isChecked) snapToGridIcon.addFile(":/resources/scenegridcheck.png");
  else snapToGridIcon.addFile(":/resources/scenegrid.png");
  gui->getAct("snapToGridComboAct.1")->setIcon(snapToGridIcon);
  Preferences::setSnapToGridPreference(isChecked);
  gui->getAct("hideGridPageBackgroundAct.1")->setEnabled(isChecked);
  GridStepSizeGroup->setEnabled(isChecked);
  KpageView->setSnapToGrid();
  reloadCurrentPage();
}

void Gui::hidePageBackground()
{
  bool checked;
  if (sender() == gui->getAct("hideRulerPageBackgroundAct.1")) {
      checked = gui->getAct("hideRulerPageBackgroundAct.1")->isChecked();
      gui->getAct("hideGridPageBackgroundAct.1")->setChecked(checked);
  }
  else
  {
      checked = gui->getAct("hideGridPageBackgroundAct.1")->isChecked();
      gui->getAct("hideRulerPageBackgroundAct.1")->setChecked(checked);
  }
  Preferences::setHidePageBackgroundPreference(checked);
  gui->reloadCurrentPage();
}

void Gui::showCoordinates()
{
  if (sender() == gui->getAct("showTrackingCoordinatesAct.1")) {
      Preferences::setShowTrackingCoordinatesPreference(
                  gui->getAct("showTrackingCoordinatesAct.1")->isChecked());
  }
  else
  if (sender() == gui->getAct("showGuidesCoordinatesAct.1"))
  {
      Preferences::setShowGuidesCoordinatesPreference(
                  gui->getAct("showGuidesCoordinatesAct.1")->isChecked());
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
  gui->reloadCurrentPage();
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
                gui->gridSize(CommandIdx - GRID_SIZE_FIRST);
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
    gui->setSceneItemZValue(BringToFront);
}

void Gui::sendToBack()
{
    gui->setSceneItemZValue(SendToBack);
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
    LDrawFile   *ldrawFile,
    const Where &here,
    bool         editModelFile   /*false*/,
    bool         displayStartPage/*false*/,
    bool         cycleSilent     /*false*/)
{
    if (! Gui::exporting()) {
#ifdef QT_DEBUG_MODE
        QElapsedTimer t;
        t.start();
#endif
        const QString &modelName = here.modelName;
        if (editModelFile) {

            emit gui->displayModelFileSig(ldrawFile, modelName);

        } else {
            Where top = lpub->page.top;
            Where bottom = lpub->page.bottom;

            if (!displayStartPage) {
                const QString loadedFile = editWindow->getCurrentFile().toLower();
                if( loadedFile == modelName.toLower() && top.lineNumber == editWindow->linesTop()) {
                    if (!ldrawFile->modified(modelName))
                        return;
                }
            }

            // limit the scope to the current page
            bool current;
            if ((current = top.lineNumber)) {
                if (!bottom.lineNumber || bottom.lineNumber == top.lineNumber) {
                    if ((current = lpub->currentStep)) {
                        bottom = lpub->currentStep->multiStep ?
                                    lpub->currentStep->bottomOfSteps() :
                                    lpub->currentStep->bottomOfStep();
                    }
                }
            }
            // error message load for showLine - no current page/step
            if (!current) {
                if (!top.lineNumber)
                    top = here;
                bottom = top;
                bool multiStep = lpub->mi.scanForward(bottom, StepMask|StepGroupEndMask) == StepGroupEndRc;
                lpub->mi.scanBackward(top, multiStep ? StepGroupEndMask : StepMask);
            }

            const StepLines lineScope(top.lineNumber, bottom.lineNumber);

            emit gui->displayFileSig(ldrawFile, modelName, lineScope);

#ifdef QT_DEBUG_MODE
                emit gui->messageSig(LOG_DEBUG,tr("Editor loaded page: %1, step: %2, model: %3, line scope: %4-%5 - %6")
                                .arg(Gui::displayPageNum)
                                .arg(lpub->currentStep ? lpub->currentStep->stepNumber.number : 0)
                                .arg(modelName)
                                .arg(top.lineNumber + 1    /*adjust for 0-index*/)
                                .arg(bottom.lineNumber + 1 /*adjust for 0-index*/)
                                .arg(Gui::elapsedTime(t.elapsed())));
#endif

            if (displayStartPage) {
               int inputPageNum = ldrawFile->getModelStartPageNumber(modelName);
               if (!inputPageNum) {
                   gui->countPages();
                   inputPageNum = ldrawFile->getModelStartPageNumber(modelName);
               }
               if (!cycleSilent && inputPageNum && Gui::displayPageNum != inputPageNum)
                   gui->cyclePageDisplay(inputPageNum);
            }

            if (Gui::curSubFile == modelName)
                return;   // work is done here.
            else
                Gui::curSubFile = modelName;

            const int currentIndex = gui->mpdCombo->currentIndex();
            for (int i = 0; i < gui->mpdCombo->count(); i++) {
                if (gui->mpdCombo->itemText(i) == modelName) { // will never equal Include File
                    if (i != currentIndex) {
                        gui->mpdCombo->setCurrentIndex(i);
                        gui->mpdCombo->setToolTip(tr("Current Submodel: %1").arg(gui->mpdCombo->currentText()));
                    }
                    break;
                }
            }

            ldrawFile->setModified(modelName.toLower(), false);
        }
    } // ! exporting
}

void Gui::displayParmsFile(
  const QString &fileName)
{
    emit gui->displayParmsFileSig(fileName);
}

void Gui::refreshModelFile()
{
     gui->editModelFile(/*saveBefore*/false);
}

void Gui::editModelFile()
{
    gui->editModelFile(/*saveBefore*/true, sender() == editWindow);
}

void Gui::editModelFile(bool saveBefore, bool subModel)
{
    if (Gui::getCurFile().isEmpty())
        return;
    if (saveBefore)
        gui->save();
    QString file = Gui::getCurFile();
    if (lpub->ldrawFile.isIncludeFile(Gui::curSubFile))
        file = Gui::curSubFile;
    else if (subModel) {
        file = editWindow->getCurrentFile();
        gui->writeToTmp(file, lpub->ldrawFile.contents(file));
    }
    editModeWindow->setWindowTitle(tr("Detached LDraw Editor - Edit %1").arg(QFileInfo(file).fileName()));
    gui->displayFile(&lpub->ldrawFile, Where(file, 0), true/*editModelFile*/);
    editModeWindow->show();
    while (!editModeWindow->contentLoading())
        QApplication::processEvents();
}


void Gui::deployBanner(bool b)
{
    if (b)
        LPub::loadBanner(Gui::m_exportMode);
}

/*-----------------------------------------------------------------------------*/

void Gui::mpdComboChanged(int index)
{

  Q_UNUSED(index)

  QString newSubFile = gui->mpdCombo->currentText();

  bool isIncludeFile = false;
  if (newSubFile.endsWith("Include File")) {
      newSubFile = gui->mpdCombo->currentData().toString();
      isIncludeFile = lpub->ldrawFile.isIncludeFile(newSubFile);
  }

  if (Gui::curSubFile != newSubFile) {

    bool callDisplayFile = isIncludeFile;

    bool cycleSilent = !Preferences::buildModEnabled || !lpub->ldrawFile.buildModDetected();

    if (!callDisplayFile) {
      const int modelPageNum = lpub->ldrawFile.getModelStartPageNumber(newSubFile);
      if (cycleSilent)
        gui->countPages();
      if (modelPageNum && Gui::displayPageNum != modelPageNum) {
        if (!gui->saveBuildModification())
          return;
        emit gui->messageSig(LOG_INFO, tr( "Select subModel: %1 @ Page: %2").arg(newSubFile).arg(modelPageNum));
        gui->cyclePageDisplay(modelPageNum, cycleSilent);
      } else {
        callDisplayFile = true;
      }
    }

    if (callDisplayFile) {
      emit gui->messageSig(LOG_INFO, tr( "Selected %1: %2")
                              .arg(isIncludeFile ? QLatin1String("includeFile") : QLatin1String("subModel")).arg(newSubFile));
      gui->displayFile(
          &lpub->ldrawFile,
          Where(newSubFile, 0),
          false/*editModelFile*/,
          true/*displayStartPage*/,
          cycleSilent);
      emit gui->showLineSig(0, LINE_HIGHLIGHT);
      if (isIncludeFile) {  // Combo will not be set to include toolTip, so set here
          gui->mpdCombo->setToolTip(tr("Include file: %1").arg(newSubFile));
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
    if (! changeLibrary && ! Gui::getCurFile().isEmpty()) {
        args = QApplication::arguments();
        args.removeFirst();
        if (!args.contains(Gui::getCurFile(),Qt::CaseInsensitive))
            args << Gui::getCurFile();
        QSettings Settings;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,SAVE_DISPLAY_PAGE_NUM_KEY),Gui::displayPageNum);
    } else {
        args << (Preferences::validLDrawLibraryChange == LEGO_LIBRARY  ? "++liblego" :
                 Preferences::validLDrawLibraryChange == TENTE_LIBRARY ? "++libtente" : "++libvexiq");
    }
    QProcess::startDetached(QApplication::applicationFilePath(), args);
    messageSig(LOG_INFO, tr("Restarted %1 with Command: %2 %3")
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

void Gui::reloadCurrentPage(bool prompt) {
    if (Gui::getCurFile().isEmpty()) {
        emit gui->messageSig(LOG_STATUS,tr("No model file page to redisplay."));
        return;
    }

    if (sender() == gui->editWindow || prompt) {
        bool _continue;
        if (Preferences::saveOnUpdate) {
            _continue = gui->maybeSave(false); // No prompt
        } else {
            _continue = gui->maybeSave(true,SaveOnUpdate);
        }
        if (!_continue)
            return;
    }

    QElapsedTimer timer;
    timer.start();

    // Gui::displayPage();
    gui->cyclePageDisplay(Gui::displayPageNum, false/*silent*/);

    emit gui->messageSig(LOG_STATUS, tr("Page %1 reloaded. %2").arg(Gui::displayPageNum).arg(Gui::elapsedTime(timer.elapsed())));

}

void Gui::reloadCurrentModelFile() { // EditModeWindow Update
    if (Gui::getCurFile().isEmpty()) {
        emit gui->messageSig(LOG_STATUS,tr("No model file to reopen."));
        return;
    }

    if (sender() == gui->editModeWindow) {
        bool _continue;
        if (Preferences::saveOnUpdate) {
            _continue = gui->maybeSave(false); // No prompt
        } else {
            _continue = gui->maybeSave(true, SaveOnUpdate);
        }
        if (!_continue)
            return;
    }

    QElapsedTimer timer;
    timer.start();

    //reload current model
    gui->cyclePageDisplay(Gui::displayPageNum, true/*silent*/, true/*FILE_RELOAD*/);

    emit gui->messageSig(LOG_STATUS, tr("Model file reloaded (%1 parts). %2")
                                   .arg(lpub->ldrawFile.getPartCount())
                                   .arg(Gui::elapsedTime(timer.elapsed())));
}

void Gui::clearWorkingFiles(const QStringList &filePaths)
{
    if (Gui::getCurFile().isEmpty()) {
        emit gui->messageSig(LOG_STATUS,tr("A model must be open to clean its parts cache - no action taken."));
        return;
    }

    int count = 0;
    for (int i = 0; i < filePaths.size(); i++) {
        QFileInfo fileInfo(filePaths.at(i));
        QFile     file(filePaths.at(i));
        if (file.exists()) {
            if (!file.remove()) {
                emit gui->messageSig(LOG_ERROR,tr("Unable to remove %1")
                                .arg(fileInfo.absoluteFilePath()));
            } else {
#ifdef QT_DEBUG_MODE
                emit gui->messageSig(LOG_TRACE,tr("-File %1 removed").arg(fileInfo.completeBaseName()));
#endif
                count++;
            }
        }
    }
    if (count)
        emit gui->messageSig(LOG_INFO_STATUS,tr("Parts content cache cleaned. %1 %2 removed.")
                                           .arg(count).arg(count == 1 ? QLatin1String("item") : QLatin1String("items")));
}

void Gui::resetModelCache(QString file, bool commandLine)
{
    if (Gui::resetCache && !file.isEmpty()) {
        QElapsedTimer timer;
        if (commandLine)
            timer.start();

        emit gui->messageSig(LOG_TRACE, tr("Reset parts cache is destructive!"));
        Gui::curFile = file;
        QString fileDir = QFileInfo(file).absolutePath();
        emit gui->messageSig(LOG_INFO, tr("Reset parts cache directory %1").arg(fileDir));
        QString saveCurrentDir = QDir::currentPath();
        if (! QDir::setCurrent(fileDir))
            emit gui->messageSig(LOG_ERROR, tr("Reset cache failed to set current directory %1").arg(fileDir));

        if (Preferences::enableFadeSteps || Preferences::enableHighlightStep)
            gui->clearCustomPartCache(true);
        gui->clearAllCaches();

        if (! QDir::setCurrent(saveCurrentDir))
            emit gui->messageSig(LOG_ERROR, tr("Reset cache failed to restore current directory %1").arg(saveCurrentDir));

        Gui::resetCache = false;

        if (commandLine)
            emit gui->messageSig(LOG_INFO, tr("All caches reset (%1 models, %2 parts). %3")
                                         .arg(lpub->ldrawFile.getSubModels().size())
                                         .arg(lpub->ldrawFile.getPartCount())
                                         .arg(Gui::elapsedTime(timer.elapsed())));
    }
}

void Gui::clearAndRedrawModelFile() { //EditModeWindow Redraw

    if (Gui::getCurFile().isEmpty()) {
        emit gui->messageSig(LOG_STATUS, tr("A model must be open to reset its caches - no action taken."));
        return;
    }

    bool saveChange = changeAccepted;
    changeAccepted = true;

    if (sender() == editModeWindow) {
        bool _continue;
        if (Preferences::saveOnRedraw) {
            _continue = gui->maybeSave(false); // No prompt
        } else {
            _continue = gui->maybeSave(true,SaveOnRedraw);
        }
        if (!_continue)
            return;
    }

    QElapsedTimer timer;
    timer.start();

    if (Preferences::enableFadeSteps || Preferences::enableHighlightStep) {
        lpub->ldrawFile.clearPrevStepPositions();
    }

    gui->clearAllCaches();

    //reload current model
    gui->cyclePageDisplay(Gui::displayPageNum, true/*silent*/, true/*FILE_RELOAD*/);

    emit gui->messageSig(LOG_INFO_STATUS, tr("All caches reset and model file reloaded (%1 models, %2 parts). %3")
                                        .arg(lpub->ldrawFile.getSubModels().size())
                                        .arg(lpub->ldrawFile.getPartCount())
                                        .arg(Gui::elapsedTime(timer.elapsed())));

    changeAccepted = saveChange;
}

void Gui::clearAndReloadModelFile(bool fileReload, bool savePrompt, bool keepWork) { // EditWindow Redraw
    if (sender() == editWindow || savePrompt) {
        bool _continue;
        if (Preferences::saveOnRedraw) {
            _continue = gui->maybeSave(false); // No prompt
        } else {
            _continue = gui->maybeSave(true, editWindow ? SaveOnRedraw : SaveOnNone);
        }
        if (!_continue)
            return;
    }

    if (!keepWork)
        gui->clearAllCaches();

    gui->cyclePageDisplay(Gui::displayPageNum, !savePrompt/*silent*/, fileReload);
}

void Gui::clearAllCaches()
{
    if (Gui::getCurFile().isEmpty()) {
        emit gui->messageSig(LOG_STATUS,tr("A model must be open to reset its caches - no action taken."));
        return;
    }

    bool showMsg = sender() == gui->getAct("clearAllCachesAct.1");

    QElapsedTimer timer;
    timer.start();

    if (Preferences::enableFadeSteps || Preferences::enableHighlightStep) {
        lpub->ldrawFile.clearPrevStepPositions();
    }

    gui->clearPLICache();
    gui->clearBOMCache();
    gui->clearCSICache();
    gui->clearSMICache();
    gui->clearTempCache();

    emit gui->messageSig(showMsg ? LOG_INFO : LOG_INFO_STATUS,
                    tr("Parts, bill of material, assembly, submodel and temporary LDraw file caches reset (%1 models, %2 parts).%3")
                                 .arg(lpub->ldrawFile.getSubModels().size())
                                 .arg(lpub->ldrawFile.getPartCount())
                                 .arg(showMsg ? "" : QString(" %1").arg(Gui::elapsedTime(timer.elapsed()))),showMsg);
}

void Gui::clearCustomPartCache(bool silent)
{
  if (Paths::customDir.isEmpty())
      return;

  bool showMsg = false;
  if (sender() == gui->getAct("clearCustomPartCacheAct.1")) {
      showMsg = true;
      bool _continue;
      if (Preferences::saveOnRedraw) {
          _continue = gui->maybeSave(false); // No prompt
      } else {
          _continue = gui->maybeSave(true);
      }
      if (!_continue)
          return;
  }

  QMessageBox::StandardButton ret = QMessageBox::Ok;
  QString message = tr("Fade and highlight parts in %1 will be deleted. The current model parts will be regenerated. "
                       "Regenerated files will be updated in %2.")
                       .arg(Paths::customDir)
                       .arg(Preferences::validLDrawCustomArchive);
  if (silent || !Preferences::modeGUI) {
      emit gui->messageSig(LOG_INFO,message);
  } else {
      ret = QMessageBox::warning(this, tr(VER_PRODUCTNAME_STR),
                                 tr("%1 Do you want to delete the custom file cache?").arg(message),
                                 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
  }

  if (ret == QMessageBox::Cancel)
      return;

  QString dirName = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customDir));

  int count = 0;
  emit gui->messageSig(LOG_INFO,tr("-Removing folder %1").arg(dirName));
  if (gui->removeDir(count, dirName)) {
      emit gui->messageSig(showMsg ? LOG_INFO : LOG_INFO_STATUS,
                      tr("Custom parts cache cleaned.  %1 %2 removed.")
                                         .arg(count)
                                         .arg(count == 1 ? QLatin1String("item") : QLatin1String("items")),showMsg);
  } else {
      emit gui->messageSig(LOG_ERROR,tr("Unable to remove custom parts cache directory: %1").arg(dirName));
      return;
  }

  // regenerate custom parts
  bool overwrite = true;
  bool setup = false;
  if (Preferences::enableFadeSteps)
      gui->processFadeColourParts(overwrite, setup);       // (re)generate and archive fade parts based on the loaded model file
  if (Preferences::enableHighlightStep)
      gui->processHighlightColourParts(overwrite, setup);  // (re)generate and archive highlight parts based on the loaded model file
  if (!Gui::getCurFile().isEmpty() && Preferences::modeGUI) {
      bool cycleEachPage = Preferences::cycleEachPage;
      if (!cycleEachPage && Gui::displayPageNum > 1)
        cycleEachPage = LocalDialog::getLocal(VER_PRODUCTNAME_STR, tr("Cycle each page on model file reload ?"),nullptr);
      gui->cyclePageDisplay(Gui::displayPageNum, PageDirection(cycleEachPage));
  }
}

void Gui::clearPLICache()
{
    if (Gui::getCurFile().isEmpty()) {
        emit gui->messageSig(LOG_STATUS,tr("A model must be open to clean its parts cache - no action taken."));
        return;
    }

    bool showMsg = false;
    if (sender() == gui->getAct("clearPLICacheAct.1")) {
        showMsg = true;
        bool _continue;
        if (Preferences::saveOnRedraw) {
            _continue = gui->maybeSave(false); // No prompt
        } else {
            _continue = gui->maybeSave(true);
        }
        if (!_continue)
            return;
    }

    QDir dir(QDir::currentPath() + QDir::separator() + Paths::partsDir);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();
    int count = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(fileInfo.absoluteFilePath());
        if (file.exists()) {
            if (!file.remove()) {
                emit gui->messageSig(LOG_ERROR,tr("Unable to remove %1").arg(fileInfo.absoluteFilePath()));
            } else {
#ifdef QT_DEBUG_MODE
                emit gui->messageSig(LOG_TRACE,tr("-File %1 removed").arg(fileInfo.absoluteFilePath()));
#endif
                count++;
            }
        }
    }

    emit gui->messageSig(showMsg ? LOG_INFO : LOG_INFO_STATUS,
                    tr("Parts content cache cleaned. %1 %2 removed.")
                                       .arg(count).arg(count == 1 ? QLatin1String("item") : QLatin1String("items")),showMsg);
}

void Gui::clearCSICache()
{
    if (Gui::getCurFile().isEmpty()) {
        emit gui->messageSig(LOG_STATUS,tr("A model must be open to clean its assembly cache - no action taken."));
        return;
    }

    bool showMsg = false;
    if (sender() == gui->getAct("clearCSICacheAct.1")) {
        showMsg = true;
        bool _continue;
        if (Preferences::saveOnRedraw) {
            _continue = gui->maybeSave(false); // No prompt
        } else {
            _continue = gui->maybeSave(true);
        }
        if (!_continue)
            return;
    }

    QDir dir(QDir::currentPath() + QDir::separator() + Paths::assemDir);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();
    int count = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(fileInfo.absoluteFilePath());
        if (file.exists()) {
            if (!file.remove()) {
                emit gui->messageSig(LOG_ERROR,tr("Unable to remove %1")
                                .arg(fileInfo.absoluteFilePath()));
            } else {
#ifdef QT_DEBUG_MODE
                emit gui->messageSig(LOG_TRACE,tr("-File %1 removed").arg(fileInfo.absoluteFilePath()));
#endif
                count++;
            }
        }
    }

    emit gui->messageSig(showMsg ? LOG_INFO : LOG_INFO_STATUS,
                    tr("Assembly content cache cleaned. %1 %2 removed.")
                                       .arg(count).arg(count == 1 ? QLatin1String("item") : QLatin1String("items")), showMsg);
}

void Gui::clearBOMCache()
{
    if (Gui::getCurFile().isEmpty()) {
        emit gui->messageSig(LOG_STATUS,tr("A model must be open to clean its bill of material cache - no action taken."));
        return;
    }

    bool showMsg = false;
    if (sender() == gui->getAct("clearBOMCacheAct.1")) {
        showMsg = true;
        bool _continue;
        if (Preferences::saveOnRedraw) {
            _continue = gui->maybeSave(false); // No prompt
        } else {
            _continue = gui->maybeSave(true);
        }
        if (!_continue)
            return;
    }

    QDir dir(QDir::currentPath() + QDir::separator() + Paths::bomDir);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();
    int count = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(fileInfo.absoluteFilePath());
        if (file.exists()) {
            if (!file.remove()) {
                emit gui->messageSig(LOG_ERROR,tr("Unable to remove %1").arg(fileInfo.absoluteFilePath()));
            } else {
#ifdef QT_DEBUG_MODE
                emit gui->messageSig(LOG_TRACE,tr("-File %1 removed").arg(fileInfo.absoluteFilePath()));
#endif
                count++;
            }
        }
    }

    emit gui->messageSig(showMsg ? LOG_INFO : LOG_INFO_STATUS,
                    tr("Bill of material content cache cleaned. %1 %2 removed.")
                                       .arg(count).arg(count == 1 ? QLatin1String("item") : QLatin1String("items")),showMsg);
}

void Gui::clearSMICache(const QString &key)
{
    if (Gui::getCurFile().isEmpty()) {
        emit gui->messageSig(LOG_STATUS,tr("A model must be open to clean its Submodel cache - no action taken."));
        return;
    }

    bool showMsg = false;
    if (sender() == gui->getAct("clearSMICacheAct.1")) {
        showMsg = true;
        bool _continue;
        if (Preferences::saveOnRedraw) {
            _continue = gui->maybeSave(false); // No prompt
        } else {
            _continue = gui->maybeSave(true);
        }
        if (!_continue)
            return;
    }

    QDir dir(QDir::currentPath() + QDir::separator() + Paths::submodelDir);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();
    int count = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(fileInfo.absoluteFilePath());
        bool deleteSpecificFile = false;
        if (!key.isEmpty()) {
            const QString filePath = QDir::toNativeSeparators(fileInfo.absoluteFilePath()).toLower();
            deleteSpecificFile = filePath.contains(QDir::toNativeSeparators(key).toLower());
        }
        if (key.isEmpty() || deleteSpecificFile) {
            if (file.exists()) {
                if (!file.remove()) {
                    emit gui->messageSig(LOG_ERROR,tr("Unable to remove %1")
                                                 .arg(fileInfo.absoluteFilePath()));
                } else {
#ifdef QT_DEBUG_MODE
                    emit gui->messageSig(LOG_TRACE,tr("-File %1 removed").arg(fileInfo.absoluteFilePath()));
#endif
                    count++;
                    if (deleteSpecificFile)
                        break;
                }
            }
        }
    }

    emit gui->messageSig(showMsg ? LOG_INFO : LOG_INFO_STATUS,
                    tr("Submodel content cache cleaned. %1 %2 removed.")
                                       .arg(count).arg(count == 1 ? QLatin1String("item") : QLatin1String("items")),showMsg);
}

void Gui::clearTempCache()
{
    if (Gui::getCurFile().isEmpty()) {
        emit gui->messageSig(LOG_STATUS,tr("A model must be open to clean its 3D cache - no action taken."));
        return;
    }

    bool showMsg = false;
    if (sender() == gui->getAct("clearTempCacheAct.1")) {
        showMsg = true;
        bool _continue;
        if (Preferences::saveOnRedraw) {
            _continue = gui->maybeSave(false); // No prompt
        } else {
            _continue = gui->maybeSave(true);
        }
        if (!_continue)
            return;
    }

    QDir tmpDir(QDir::currentPath() + QDir::separator() + Paths::tmpDir);
    tmpDir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = tmpDir.entryInfoList();
    int count = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(fileInfo.absoluteFilePath());
        if (file.exists()) {
            if (!file.remove()) {
                emit gui->messageSig(LOG_ERROR,tr("Unable to remove %1")
                                .arg(fileInfo.absoluteFilePath()));
            } else {
#ifdef QT_DEBUG_MODE
                emit gui->messageSig(LOG_TRACE,tr("-File %1 removed").arg(fileInfo.absoluteFilePath()));
#endif
                count++;
            }
        }
    }

    lpub->ldrawFile.tempCacheCleared();

    emit gui->messageSig(showMsg ? LOG_INFO : LOG_INFO_STATUS,
                    tr("Temporary model file cache cleaned. %1 %2 removed.")
                                       .arg(count).arg(count == 1 ? QLatin1String("item") : QLatin1String("items")),showMsg);
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
            if (fileInfo.isFile()) {
                if ((result = QFile::remove(fileInfo.absoluteFilePath()))) {
#ifdef QT_DEBUG_MODE
                    emit gui->messageSig(LOG_TRACE,tr("-File %1 removed").arg(fileInfo.absoluteFilePath()));
#endif
                    count++;
                }
            }
            if (!result) {
                return result;
            }
        }
        if ((result = dir.rmdir(dir.absolutePath())))
            emit gui->messageSig(LOG_TRACE,tr("-Folder %1 cleaned").arg(dir.absolutePath()));
    }
    return result;
}

void Gui::clearStepCSICache(QString &pngName) {
    QString tmpDirName   = QDir::currentPath() + QDir::separator() + Paths::tmpDir;
    QString assemDirName = QDir::currentPath() + QDir::separator() + Paths::assemDir;
    QString ldrName      = tmpDirName + QDir::separator() + QLatin1String("csi.ldr");
    QFileInfo fileInfo(pngName);
    QFile file(assemDirName + QDir::separator() + fileInfo.fileName());
    if (file.exists()) {
        if (!file.remove())
            emit gui->messageSig(LOG_ERROR,tr("Unable to remove %1")
                                         .arg(assemDirName + QDir::separator() + fileInfo.fileName()));
#ifdef QT_DEBUG_MODE
        else
            emit gui->messageSig(LOG_TRACE,tr("-File %1 removed").arg(fileInfo.fileName()));
#endif
    }
    if (Render::useLDViewSCall())
        ldrName = tmpDirName + QDir::separator() + fileInfo.completeBaseName() + QLatin1String(".ldr");
    file.setFileName(ldrName);
    if (file.exists()) {
        if (!file.remove())
            emit gui->messageSig(LOG_ERROR,tr("Unable to remove %1").arg(file.fileName()));
#ifdef QT_DEBUG_MODE
        else
            emit gui->messageSig(LOG_TRACE,tr("-File %1 removed").arg(file.fileName()));
#endif
    }
    if (Preferences::enableFadeSteps)
        clearPrevStepPositions();
    cyclePageDisplay(Gui::displayPageNum);
}

void Gui::clearPageCache(PlacementType relativeType, Page *page, int option) {
  if (page->list.size()) {
      if (relativeType == SingleStepType) {         // single step page
          Range *range = dynamic_cast<Range *>(page->list[0]);
          if (range->relativeType == RangeType) {
              Step *step = dynamic_cast<Step *>(range->list[0]);
              if (step && step->relativeType == StepType) {
                  clearStepGraphicsItems(step, option);
              } // validate step (StepType) and process...
          } // validate RangeType - to cast step
      } else if (relativeType == StepGroupType) {  // multi-step page
          for (int i = 0; i < page->list.size(); i++) {
              Range *range = dynamic_cast<Range *>(page->list[i]);
              for (int j = 0; j < range->list.size(); j++) {
                  if (range->relativeType == RangeType) {
                      Step *step = dynamic_cast<Step *>(range->list[j]);
                      if (step && step->relativeType == StepType) {
                          clearStepGraphicsItems(step, option);
                      } // validate step (StepType) and process...
                  } // validate RangeType - to cast step
              } // for each step within divided group...=>list[AbstractRangeElement]->StepType
          } // for each divided group within page...=>list[AbstractStepsElement]->RangeType
      }
      if (Preferences::enableFadeSteps && option != Options::PLI) {
          clearPrevStepPositions();
      }
      cyclePageDisplay(Gui::displayPageNum);
   }
}

void Gui::clearStepCache(Step *step, int option) {
    clearStepGraphicsItems(step, option);
    if (Preferences::enableFadeSteps && option != Options::PLI) {
        clearPrevStepPositions();
    }
    cyclePageDisplay(Gui::displayPageNum);
}

/*
 * Clear step image graphics items
 * This function recurses the step's model to clear images and associated model files.
 */
void Gui::clearStepGraphicsItems(Step *step, int option) {
    // Capture ldr and image file names
    QStringList fileNames;
    QString tmpDirName = QDir::currentPath() + QDir::separator() + Paths::tmpDir;

    using namespace Options;
    Mt stepOption = static_cast<Mt>(option);
    const bool clearAll = stepOption == MON;

    if (stepOption == CSI || clearAll) {
        if (Render::useLDViewSCall())
            fileNames << QDir::toNativeSeparators(tmpDirName + QDir::separator() + QFileInfo(step->pngName).completeBaseName() + QLatin1String(".ldr"));
         else
            fileNames << QDir::toNativeSeparators(tmpDirName + QDir::separator() + QLatin1String("csi.ldr"));
        fileNames << step->pngName;
    }

    if (stepOption == PLI || clearAll) {
        if (!Render::useLDViewSCall())
            fileNames << QDir::toNativeSeparators(tmpDirName + QDir::separator() + QLatin1String("pli.ldr"));
        QHash<QString, PliPart*> pliParts;
        step->pli.getParts(pliParts);
        if (pliParts.size()) {
            Q_FOREACH (PliPart* part, pliParts) {
                QString key = QString("%1;%2;%3").arg(QFileInfo(part->type).completeBaseName()).arg(part->color).arg(step->stepNumber.number);
                if (lpub->ldrawFile.viewerStepContentExist(key)) {
                    if (Render::useLDViewSCall())
                        fileNames << QDir::toNativeSeparators(gui->getViewerStepFilePath(key));
                    fileNames << QDir::toNativeSeparators(gui->getViewerStepImagePath(key));
                }
            }
        }
    }

    if (stepOption == SMI || clearAll) {
        if (!Render::useLDViewSCall()) {
            fileNames << QDir::toNativeSeparators(tmpDirName + QDir::separator() + SUBMODEL_IMAGE_BASENAME + QLatin1String(".ldr"));
            fileNames << QDir::toNativeSeparators(tmpDirName + QDir::separator() + SUBMODEL_COVER_PAGE_PREVIEW_BASENAME + QLatin1String(".ldr"));
        }
        SubModelPart* submodel = step->subModel.getSubmodel();
        if (submodel) {
            const QString key = QString("%1;%2;%3_%4")
                                        .arg(lpub->ldrawFile.getSubmodelIndex(step->subModel.bottom.modelName))
                                        .arg(step->subModel.bottom.lineNumber)
                                        .arg(step->stepNumber.number)
                                        .arg(SUBMODEL_IMAGE_BASENAME);
            if (lpub->ldrawFile.viewerStepContentExist(key)) {
                if (Render::useLDViewSCall())
                    fileNames << QDir::toNativeSeparators(gui->getViewerStepFilePath(key));
                fileNames << QDir::toNativeSeparators(gui->getViewerStepImagePath(key));
            }
        }
    }

    QFile file;
    // process ldr and image files
    Q_FOREACH (QString fileName, fileNames) {
        file.setFileName(fileName);
        if (file.exists()) {
            if (!file.remove())
                emit gui->messageSig(LOG_ERROR,tr("Unable to remove %1").arg(file.fileName()));
#ifdef QT_DEBUG_MODE
            else
                emit gui->messageSig(LOG_TRACE,tr("-File %1 removed").arg(file.fileName()));
#endif
        }
    }

    // process callout ldr and image files
    for (int k = 0; k < step->list.size(); k++) {
        if (step->list[k]->relativeType == CalloutType) {
            Callout *callout = dynamic_cast<Callout *>(step->list[k]);
            for (int l = 0; l < callout->list.size(); l++) {
                Range *range = dynamic_cast<Range *>(callout->list[l]);
                for (int m = 0; m < range->list.size(); m++) {
                    if (range->relativeType == RangeType) {
                        Step *step = dynamic_cast<Step *>(range->list[m]);
                        if (step && step->relativeType == StepType) {
                            clearStepGraphicsItems(step, option);
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

void Gui::fadeStepsSetup()
{
  GlobalFadeStepDialog::getFadeStepGlobals(lpub->ldrawFile.topLevelFile(),lpub->page.meta);
}

void Gui::highlightStepSetup()
{
  GlobalHighlightStepDialog::getHighlightStepGlobals(lpub->ldrawFile.topLevelFile(),lpub->page.meta);
}

void Gui::useSystemEditor()
{
   bool useSystemEditor = gui->getAct("useSystemEditorAct.1")->isChecked();
   Preferences::useSystemEditorPreference(useSystemEditor);
   gui->getAct("editModelFileAct.1")->setStatusTip(tr("Edit current model file with %1")
                                                      .arg(useSystemEditor ? Preferences::systemEditor.isEmpty() ? "the system editor" :
                                                                                               Preferences::systemEditor : "detached LDraw Editor"));
}

void Gui::recountParts()
{
   bool recountParts = gui->getAct("recountPartsAct.1")->isChecked();
   Preferences::recountPartsPreference(recountParts);
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
        emit gui->messageSig(LOG_ERROR, QString("Static colour part list does not exist. Generate from the Configuration menu."));
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
        emit gui->messageSig(LOG_ERROR, QString("PLI control parts file does not exist."));
    } else {
        if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
            if (Preferences::systemEditor.isEmpty())
                QDesktopServices::openUrl(QUrl("file:///"+Preferences::pliControlFile, QUrl::TolerantMode));
            else
#endif
                openWith(Preferences::pliControlFile);
        } else {
            emit displayModelFileSig(nullptr, Preferences::pliControlFile);
            editModeWindow->setWindowTitle(tr("PLI Control Parts","Edit/add PLI control part entries"));
            editModeWindow->show();
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
            emit gui->messageSig(LOG_ERROR, QString("Failed to export %1.").arg(Preferences::annotationStyleFile));
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
            emit gui->messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
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
            emit gui->messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
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
            emit gui->messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
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
            emit gui->messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
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
           emit gui->messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
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
        lpubConfigFile = QString("%1/%2/%3.%4")
                                 .arg(Preferences::lpub3dConfigPath)
                                 .arg(companyName)
                                 .arg(Preferences::lpub3dAppName)
                                 .arg(fileExt);
#elif defined Q_OS_MACOS
    fileExt = "plist";
    lpubConfigFile = QString("%1/com.%2.%3.%4")
                             .arg(Preferences::lpub3dConfigPath)
                             .arg(companyName.replace(" ","-"))
                             .arg(QString(Preferences::lpub3dAppName).replace(".app",""))
                             .arg(fileExt);
#elif defined Q_OS_LINUX
    fileExt = "conf";
    lpubConfigFile = QString("%1/%2/%3.%4")
                             .arg(Preferences::lpub3dConfigPath)
                             .arg(companyName)
                             .arg(Preferences::lpub3dAppName)
                             .arg(fileExt);
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
    QString blenderConfigFile, titleLabel;
    bool blenderParameters = sender() == gui->getAct("editBlenderParametersAct.1");
    bool blenderPreferences = sender() == gui->getAct("editBlenderPreferencesAct.1");
    if (blenderParameters) {
        titleLabel = tr("LDraw Parameters");
        blenderConfigFile = QString("%1/%2").arg(Preferences::blenderConfigDir).arg(VER_BLENDER_LDRAW_PARAMS_FILE);
    } else if (blenderPreferences) {
        titleLabel = tr("Import and Render Preferences");
        blenderConfigFile = QString("%1/%2").arg(Preferences::blenderConfigDir).arg(VER_BLENDER_ADDON_CONFIG_FILE);
    }

    QFileInfo fileInfo(blenderConfigFile);
    if (blenderParameters && !fileInfo.exists()) {
        if (!BlenderPreferences::exportParameterFile()) {
            emit gui->messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
            return;
        }
    } else if (blenderPreferences && !fileInfo.exists()) {
        emit gui->messageSig(LOG_ERROR, QString("File %1 was not found.").arg(fileInfo.absoluteFilePath()));
        return;
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
        parmsWindow->setWindowTitle(tr("Blender %1",
                                       "Edit/add Blender %1 settings")
                                    .arg(titleLabel));
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
            emit gui->messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
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
            emit gui->messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
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
            emit gui->messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
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
            emit gui->messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
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
            emit gui->messageSig(LOG_ERROR, QString("Failed to export %1.").arg(fileInfo.absoluteFilePath()));
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

void Gui::loadStatus()
{
    lpub->ldrawFile.loadStatus(true/*menuAction*/);
}

void Gui::viewLog()
{
    if (Preferences::useSystemEditor) {
#ifndef Q_OS_MACOS
        if (Preferences::systemEditor.isEmpty())
            QDesktopServices::openUrl(QUrl("file:///"+Preferences::logFilePath, QUrl::TolerantMode));
        else
#endif
            openWith(Preferences::logFilePath);
    } else {
        displayParmsFile(Preferences::logFilePath);
        parmsWindow->setWindowTitle(tr(VER_PRODUCTNAME_STR " log",VER_PRODUCTNAME_STR " logs"));
        parmsWindow->show();
    }
}

void Gui::preferences()
{
    if (Preferences::getPreferences()) {

        if (Preferences::restartApplication) {
            return restartApplication(Preferences::libraryChangeRestart);
        }

        Meta meta;
        lpub->page.meta = meta;

        commonMenus.setWhatsThis();

        foreach (QAction *action, editWindow->actions()) {
            lpub->setKeyboardShortcut(action);
        }
        foreach (QAction *action, editModeWindow->actions()) {
            lpub->setKeyboardShortcut(action);
        }
        foreach (QAction *action, parmsWindow->actions()) {
            lpub->setKeyboardShortcut(action);
        }

        if (Preferences::loadTheme)
            loadTheme();

        if (Preferences::setSceneTheme)
            setSceneTheme();

        if (!Gui::getCurFile().isEmpty()) {
            if (Preferences::reloadPage) {
                lpub->ldrawFile.clearViewerSteps();
                reloadCurrentPage(true);
            }
            else
            if (Preferences::reloadFile) {
                lpub->ldrawFile.clearViewerSteps();
                if (Preferences::resetCustomCache)
                    clearCustomPartCache(true);
                clearAndReloadModelFile(false, true);
            }
        }

        Preferences::resetPreferenceFlags();
    }
}

/*******************************************************************************
 *
 * This is all the initialization stuff.  It is used once when the program
 * starts up
 *
 ******************************************************************************/

Gui::Gui() : pageMutex(QMutex::Recursive)
{
    emit Application::instance()->splashMsgSig(tr("25% - %1 window defaults loading...").arg(VER_PRODUCTNAME_STR));

    // classes
    qRegisterMetaType<BackgroundData>("BackgroundData");
    qRegisterMetaType<BorderData>("BorderData");
    qRegisterMetaType<BuffExchgData>("BuffExchgData");
    qRegisterMetaType<BuildModFlags>("BuildModFlags");
    qRegisterMetaType<BuildModData>("BuildModData");
    qRegisterMetaType<CameraAnglesData>("CameraAnglesData");
    qRegisterMetaType<ConstrainData>("ConstrainData");
    qRegisterMetaType<CsiAnnotationIconData>("CsiAnnotationIconData");
    qRegisterMetaType<FindPageFlags>("FindPageFlags");
    qRegisterMetaType<FreeFormData>("FreeFormData");
    qRegisterMetaType<InsertData>("InsertData");
    qRegisterMetaType<JustifyStepData>("JustifyStepData");
    qRegisterMetaType<NativeOptions>("NativeOptions");
    qRegisterMetaType<PageSizeData>("PageSizeData");
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
    qRegisterMetaType<DisplayType>("DisplayType");
    qRegisterMetaType<DividerType>("DividerType");
    qRegisterMetaType<ExportMode>("ExportMode");
    qRegisterMetaType<ExportOptionType>("ExportOptionType");
    qRegisterMetaType<FillEnc>("FillEnc");
    qRegisterMetaType<FinalModelEnabledEnc>("FinalModelEnabledEnc");
    qRegisterMetaType<GridStepSize>("GridStepSize");
    qRegisterMetaType<IniFlag>("IniFlag");
    qRegisterMetaType<JustifyStepEnc>("JustifyStepEnc");
    qRegisterMetaType<LDrawFileRegExpType>("LDrawFileRegExpType");
    qRegisterMetaType<LDrawUnofficialFileType>("LDrawUnofficialFileType");
    qRegisterMetaType<LibType>("LibType");
    qRegisterMetaType<LightType>("LightType");
    qRegisterMetaType<LineHighlightType>("LineHighlightType");
    qRegisterMetaType<LoadMsgType>("LoadMsgType");
    qRegisterMetaType<LogType>("LogType");
    qRegisterMetaType<MissingHeaderType>("MissingHeaderType");
    qRegisterMetaType<Preferences::MsgKey>("Preferences::MsgKey");
    qRegisterMetaType<Preferences::MsgID>("Preferences::MsgID");
    qRegisterMetaType<NameKeyAttributes>("NameKeyAttributes");
    qRegisterMetaType<NameKeyAttributes2>("NameKeyAttributes2");
    qRegisterMetaType<NativeRenderType>("NativeRenderType");
    qRegisterMetaType<NativeType>("NativeType");
    qRegisterMetaType<OrientationEnc>("OrientationEnc");
    qRegisterMetaType<PageActType>("PageActType");
    qRegisterMetaType<PageDirection>("PageDirection");
    qRegisterMetaType<PageTypeEnc>("PageTypeEnc");
    qRegisterMetaType<PartSource>("PartSource");
    qRegisterMetaType<PartType>("PartType");
    qRegisterMetaType<PageSizeData>("PageSizeData");
    qRegisterMetaType<PlacementEnc>("PlacementEnc");
    qRegisterMetaType<PlacementTableEnc>("PlacementTableEnc");
    qRegisterMetaType<PlacementType>("PlacementType");
    qRegisterMetaType<PliType>("PliType");
    qRegisterMetaType<PrepositionEnc>("PrepositionEnc");
    qRegisterMetaType<ProcessType>("ProcessType");
    qRegisterMetaType<RectPlacement>("RectPlacement");
    qRegisterMetaType<RelativeTos>("RelativeTos");
    qRegisterMetaType<RemoveLPubFormatType>("RemoveLPubFormatType");
    qRegisterMetaType<RemoveObjectsRc>("RemoveObjectsRc");
    qRegisterMetaType<RendererType>("RendererType");
    qRegisterMetaType<RulerTrackingType>("RulerTrackingType");
    qRegisterMetaType<SaveOnSenderType>("SaveOnSenderType");
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
    qRegisterMetaType<ThemeType>("ThemeType");
    qRegisterMetaType<ThemeColorType>("ThemeColorType");
    qRegisterMetaType<TraverseRc>("TraverseRc");
    qRegisterMetaType<TypeLine>("TypeLine");
    qRegisterMetaType<WT_Type>("WT_Type");

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
    pageProcessParent               = PROC_NONE;        // display page porcess
    processOption                   = EXPORT_ALL_PAGES; // export process
    m_exportMode                    = EXPORT_PDF;
    pageRangeText                   = "1";
    exportPixelRatio                = 1.0;
    suspendFileDisplay              = false;
    resetCache                      = false;
    m_previewDialog                 = false;
    m_partListCSIFile               = false;
    m_exportingContent              = false;
    m_exportingObjects              = false;
    m_contPageProcessing            = false;
    nextPageContinuousIsRunning     = false;
    previousPageContinuousIsRunning = false;

    doFadeStep                      = false;
    doHighlightStep                 = false;

    m_fadeStepsSetup                = Preferences::enableFadeSteps;
    m_highlightStepSetup            = Preferences::enableHighlightStep;

    mBuildModRange    = { 0, 0, -1 };
    mStepRotation     = { 0.0f, 0.0f, 0.0f };
    mRotStepAngleX    = 0.0f;
    mRotStepAngleY    = 0.0f;
    mRotStepAngleZ    = 0.0f;
    mRotStepType = QString();
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

    KexportView = LGraphicsView(&KexportScene);

    if (Preferences::modeGUI) {
        QColor spinnerColor(
               Preferences::darkTheme ?
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
    visualEditUndo = false;
    visualEditRedo = false;

#if defined Q_OS_MACOS
    if (Preferences::systemEditor.isEmpty())
        Preferences::systemEditor = QString("");
#endif

    connect(&futureWatcher, &QFutureWatcher<int>::finished, this, &Gui::finishedCountingPages);

    // LPub - Object connection is Qt::AutoConnection so it will trigger without an active event loop
    connect(lpub,           SIGNAL(messageSig( LogType,const QString &,int)),
            this,           SLOT(statusMessage(LogType,const QString &,int)));

    // Gui - MainWindow
    connect(this,           SIGNAL(messageSig( LogType,const QString &,int)),
            this,           SLOT(statusMessage(LogType,const QString &,int)),
            Qt::QueuedConnection); // this connection will only trigger when the Main thread event loop, m_application.exec(), is active

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
    connect(this,           SIGNAL(clearStepCacheSig(Step*, int)),
            this,           SLOT(  clearStepCache(Step*, int)));

    connect(this,           SIGNAL(clearPageCacheSig(PlacementType, Page*, int)),
            this,           SLOT(  clearPageCache(PlacementType, Page*, int)));

    connect(this,           SIGNAL(clearAndReloadModelFileSig(bool, bool, bool)),   // reloadModelFile
            this,           SLOT(  clearAndReloadModelFile(bool, bool, bool)));

    connect(this,           SIGNAL(clearCustomPartCacheSig(bool)),
            this,           SLOT(  clearCustomPartCache(bool)));

    connect(this,           SIGNAL(clearAllCachesSig()),
            this,           SLOT(  clearAllCaches()));

    connect(this,           SIGNAL(clearSMICacheSig()),
            this,           SLOT(  clearSMICache()));

    connect(this,           SIGNAL(clearPLICacheSig()),
            this,           SLOT(  clearPLICache()));

    connect(this,           SIGNAL(clearBOMCacheSig()),
            this,           SLOT(  clearBOMCache()));

    connect(this,           SIGNAL(clearCSICacheSig()),
            this,           SLOT(  clearCSICache()));

    connect(this,           SIGNAL(clearTempCacheSig()),
            this,           SLOT(  clearTempCache()));

    connect(this,           SIGNAL(reloadCurrentPageSig(bool)),          // reloadDisplayPage
            this,           SLOT(  reloadCurrentPage(bool)));

    connect(this,           SIGNAL(restartApplicationSig(bool, bool)),
            this,           SLOT(  restartApplication(bool, bool)));

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

    // Fade and Highlight
    connect(this,           SIGNAL(enableLPubFadeOrHighlightSig(bool,bool,bool)),
            this,           SLOT(  enableLPubFadeOrHighlight(bool,bool,bool)));

    progressLabel = new QLabel(this);
    progressLabel->setMinimumWidth(200);
    progressBar = new QProgressBar();
    progressBar->setMaximumWidth(300);

    progressLabelPerm = new QLabel();
    progressLabelPerm->setMinimumWidth(200);
    progressBarPerm = new QProgressBar();
    progressBarPerm->setMaximumWidth(300);

    m_progressDialog         = new ProgressDialog();

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
  if (maybeSave())
  {
    emit requestEndThreadNowSig();

    if (parmsWindow->isVisible()) {
        parmsWindow->close();
    }

    Preferences::resetFadeSteps();
    Preferences::resetHighlightStep();
    Preferences::resetPreferredRenderer();

    event->accept();

    writeSettings();
  }
  else
  {
    event->ignore();
  }
}

void Gui::workerJobResult(int value) {
    gui->m_workerJobResult = value;
}

void Gui::getRequireds() {
    // Check preferred renderer value is set before setting Renderer class
    Preferences::getRequireds();
    Render::setRenderer(Preferences::preferredRenderer);
}

void Gui::initialize()
{

  emit Application::instance()->splashMsgSig(tr("85% - %1 initialization...").arg(VER_PRODUCTNAME_STR));

  connect(this, SIGNAL(loadFileSig(QString)),
          this, SLOT(  loadFile(QString)));
  connect(lpub, SIGNAL(loadFileSig(QString, bool)),
          this, SLOT(  loadFile(QString, bool)));
  connect(lpub, SIGNAL(consoleCommandSig(int, int*)),
          this, SLOT(  consoleCommandCurrentThread(int, int*)), Qt::DirectConnection);
  connect(this, SIGNAL(consoleCommandFromOtherThreadSig(int, int*)),
          this, SLOT(  consoleCommand(int, int*)), Qt::BlockingQueuedConnection);
  connect(this, SIGNAL(countPagesSig()),
          this, SLOT(countPages()));
  connect(this, SIGNAL(setGeneratingBomSig(bool)),
          this, SLOT(  deployBanner(bool)));
  connect(this, SIGNAL(setExportingSig(bool)),
          this, SLOT(  deployBanner(bool)));
  connect(this, SIGNAL(setExportingSig(bool)),
          this, SLOT(halt3DViewer(bool)));
  connect(this, SIGNAL(updateAllViewsSig()),
          this, SLOT(UpdateAllViews()));
  connect(this, SIGNAL(previewModelSig(QString const &)),
          this, SLOT(previewModel(QString const &)));
  connect(this, SIGNAL(setPliIconPathSig(QString&,QString&)),
          this, SLOT(  setPliIconPath(QString&,QString&)));
  connect(this, SIGNAL(parseErrorSig(const QString &, const Where &, Preferences::MsgKey, bool, bool, int, const QString &, const QString &)),
          this, SLOT(  parseError(const QString &, const Where &, Preferences::MsgKey, bool, bool, int, const QString &, const QString &)));

/* Moved to PartWorker::ldsearchDirPreferences()  */
//  if (Preferences::preferredRenderer == RENDERER_LDGLITE)
//      partWorkerLdgLiteSearchDirs.populateLdgLiteSearchDirs();

  emit Application::instance()->splashMsgSig(tr("90% - %1 widgets loading...").arg(VER_PRODUCTNAME_STR));

  if (Preferences::modeGUI) {
      commonMenus.setWhatsThis();

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

  emit Application::instance()->splashMsgSig(tr("95% - LDraw colors loading..."));

  LDrawColor::LDrawColorInit();

  Preferences::setInitFadeSteps();
  Preferences::setInitHighlightStep();
  Preferences::setInitPreferredRenderer();

  setCurrentFile("");
  updateOpenWithActions(); // also adds the system editor if defined

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

  readSettings();
}

void Gui::getSubFileList()
{
   emit gui->setSubFilesSig(fileList());
}

void Gui::loadBLCodes()
{
   if (!Annotations::loadBLCodes()) {
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
            QString message = tr("Could not open the %1 LDraw color parts file [%2], Error: %3")
                    .arg(Preferences::validLDrawLibrary).arg(Preferences::ldrawColourPartsFile).arg(result);
            emit gui->messageSig(LOG_NOTICE, message);
            bool prompt = false;
            if (Preferences::modeGUI) {
                QMessageBoxResizable box;
                box.setWindowIcon(QIcon());
                box.setIconPixmap (QPixmap(LPUB3D_MESSAGE_ICON));
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
                    gui->generateCustomColourPartsList(prompt); /* false */
                }
            } else {
                gui->generateCustomColourPartsList(prompt); /* false */
            }
        } else {
            emit gui->messageSig(LOG_INFO_STATUS, tr ("Loaded LDraw color parts file [%2]").arg(Preferences::ldrawColourPartsFile));
        }
    }
}

void Gui::reloadModelFileAfterColorFileGen() {
    if (Preferences::modeGUI) {
        QMessageBoxResizable box;
        box.setWindowIcon(QIcon());
        box.setIconPixmap (QPixmap(LPUB3D_MESSAGE_ICON));
        box.setTextFormat (Qt::RichText);
        box.setWindowTitle(QMessageBox::tr ("%1 Color Parts File.").arg(Preferences::validLDrawLibrary));
        box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        box.setMinimumSize(10,10);
        QString entries;
        if (gui->m_workerJobResult)
            entries = tr(" with %1 entries").arg(gui->m_workerJobResult);
        QString message = tr("The %1 LDraw Color Parts File has finished building%2.")
                             .arg(Preferences::validLDrawLibrary).arg(entries);
        box.setText (message);

        bool enableFadeSteps = Gui::m_fadeStepsSetup || Preferences::enableFadeSteps;
        bool enableHighlightSttep = Gui::m_highlightStepSetup || Preferences::enableHighlightStep;

        if (enableFadeSteps || enableHighlightSttep) {
            QString body = tr ("The color file list and current model must be reloaded.<br>Do you want to continue ?");
            box.setInformativeText (body);

            box.setStandardButtons (QMessageBox::Ok | QMessageBox::Cancel);
            if (box.exec() == QMessageBox::Ok && ! Gui::getCurFile().isEmpty()) {
                bool _continue;
                if (Preferences::saveOnRedraw) {
                    _continue = gui->maybeSave(false); // No prompt
                } else {
                    _continue = gui->maybeSave(true, SaveOnNone);
                }
                if (!_continue)
                    return;

                QElapsedTimer timer;
                timer.start();

                gui->clearPLICache();
                gui->clearBOMCache();
                gui->clearCSICache();
                gui->clearSMICache();
                gui->clearTempCache();

                //reload current model file
                gui->cyclePageDisplay(Gui::displayPageNum, true/*silent*/, true/*FILE_RELOAD*/);

                emit gui->messageSig(LOG_STATUS, QString("All caches reset and model file reloaded (%1 models, %2 parts). %3")
                                                    .arg(lpub->ldrawFile.getSubModels().size())
                                                    .arg(lpub->ldrawFile.getPartCount())
                                                    .arg(Gui::elapsedTime(timer.elapsed())));
            }
        } else {
            box.setDefaultButton   (QMessageBox::Ok);
            box.exec();
        }
    }
}

// left side progress bar - no longer used
void Gui::progressBarInit() {
  if (Gui::okToInvokeProgressBar()) {
      gui->progressBar->setMaximumHeight(15);
      gui->statusBar()->addWidget(gui->progressLabel);
      gui->statusBar()->addWidget(gui->progressBar);
      gui->progressLabel->setAlignment(Qt::AlignRight);
      gui->progressLabel->show();
      gui->progressBar->show();
  }
}

void Gui::progressBarSetText(const QString &progressText)
{
  if (Gui::okToInvokeProgressBar()) {
      gui->progressLabel->setText(progressText);
      emit gui->messageSig(LOG_INFO, progressText);
    }
}
void Gui::progressBarSetRange(int minimum, int maximum)
{
  if (Gui::okToInvokeProgressBar()) {
      gui->progressBar->setRange(minimum,maximum);
    }
}
void Gui::progressBarSetValue(int value)
{
  if (Gui::okToInvokeProgressBar()) {
      gui->progressBar->setValue(value);
      QApplication::processEvents();
    }
}
void Gui::progressBarReset()
{
  if (Gui::okToInvokeProgressBar()) {
      gui->progressBar->reset();
    }
}

void Gui::progressStatusRemove() {
  if (Gui::okToInvokeProgressBar()) {
      gui->statusBar()->removeWidget(gui->progressBar);
      gui->statusBar()->removeWidget(gui->progressLabel);
    }
}

// right side progress bar
void Gui::progressBarPermInit() {
  if (Gui::okToInvokeProgressBar()) {
      gui->progressBarPerm->setMaximumHeight(15);
      gui->statusBar()->addPermanentWidget(gui->progressLabelPerm);
      gui->statusBar()->addPermanentWidget(gui->progressBarPerm);
      gui->progressLabelPerm->setAlignment(Qt::AlignRight);
      gui->progressLabelPerm->show();
      gui->progressBarPerm->show();
    }
}

void Gui::progressBarPermSetText(const QString &progressText)
{
  if (Gui::okToInvokeProgressBar()) {
      gui->progressLabelPerm->setText(progressText);
    }
}
void Gui::progressBarPermSetRange(int minimum, int maximum)
{
  if (Gui::okToInvokeProgressBar()) {
      gui->progressBarPerm->setRange(minimum,maximum);
    }
}
void Gui::progressBarPermSetValue(int value)
{
  if (Gui::okToInvokeProgressBar()) {
      gui->progressBarPerm->setValue(value);
    }
}
void Gui::progressBarPermReset()
{
  if (Gui::okToInvokeProgressBar()) {
      gui->progressBarPerm->reset();
    }
}

void Gui::progressPermStatusRemove() {
  if (Gui::okToInvokeProgressBar()) {
      gui->statusBar()->removeWidget(gui->progressBarPerm);
      gui->statusBar()->removeWidget(gui->progressLabelPerm);
    }
}

void Gui::updateGoToPage(bool frontCoverPageExist, bool backCoverPageExist) {
  int pageNum = 0 + Gui::pa;
  gui->disconnect(gui->setGoToPageCombo,SIGNAL(activated(int)), gui, SLOT(setGoToPage(int)));
  gui->setGoToPageCombo->clear();

  for(int i=1 + Gui::pa;i <= Gui::maxPages;i++) {
      pageNum++;
      if (frontCoverPageExist && i == 1) {
          pageNum--;
          gui->setGoToPageCombo->addItem(QString("Front Cover"));
      }
      else if (backCoverPageExist && i == Gui::maxPages) {
          gui->setGoToPageCombo->addItem(QString("Back Cover"));
      }
      else
          gui->setGoToPageCombo->addItem(QString("Page %1").arg(QString::number(pageNum)));
  }

  gui->setGoToPageCombo->setCurrentIndex(Gui::displayPageNum - 1 - Gui::pa);
  gui->connect(gui->setGoToPageCombo,SIGNAL(activated(int)), gui, SLOT(setGoToPage(int)));
}

void Gui::addEditLDrawIniFileAction()
{
    if (Preferences::ldrawiniFound) {
        gui->getMenu("editorMenu")->insertAction(gui->getAct("editNativePOVIniAct.1"), gui->getAct("editLdrawIniFileAct.1"));
        gui->getToolBar("editParamsToolBar")->insertAction(gui->getAct("editNativePOVIniAct.1"), gui->getAct("editLdrawIniFileAct.1"));
    }
}

void Gui::showRenderDialog()
{
    int importOnly = sender() == gui->blenderImportAct ? BLENDER_IMPORT : 0;
    int renderType = sender() == gui->blenderRenderAct || importOnly ? BLENDER_RENDER : POVRAY_RENDER;

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
    Preferences::archivePartsOnLaunch = gui->getAct("archivePartsOnLaunchAct.1")->isChecked();
    bool archivePartsOnLaunchChanged  = Preferences::archivePartsOnLaunch  != archivePartsOnLaunchCompare;

    if (archivePartsOnLaunchChanged) {
        QSettings Settings;
        QString const archivePartsOnLaunchKey("ArchivePartsOnLaunch");
        Settings.setValue(QString("%1/%2").arg(SETTINGS,archivePartsOnLaunchKey), Preferences::archivePartsOnLaunch);
        emit gui->messageSig(LOG_INFO,QString("Archive search files on launch is %1").arg(Preferences::archivePartsOnLaunch? "ON" : "OFF"));
    }
}

void Gui::archivePartsOnDemand() {
    gui->loadLDSearchDirParts(false/*Process*/, true/*OnDemand*/, false/*Update*/);
}

void Gui::generateCustomColourPartsList(bool prompt)
{
    QMessageBox::StandardButton ret = QMessageBox::Cancel;
    QString message = tr("Generate the %1 color parts list. This may take some time.").arg(Preferences::validLDrawLibrary);

    if (Preferences::modeGUI && prompt && Preferences::lpub3dLoaded) {
            ret = QMessageBox::warning(this, tr(VER_PRODUCTNAME_STR),
                                   tr("%1 Do you want to continue ?").arg(message),
                                   QMessageBox::Yes | QMessageBox::Cancel);
    }

    if (ret == QMessageBox::Yes || ! prompt || !Preferences::lpub3dLoaded) {
        emit gui->messageSig(LOG_INFO,message);

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
    gui->partWorkerCustomColour = new PartWorker();

    connect(gui,                   SIGNAL(operateFadeParts(bool, bool)),
            partWorkerCustomColour, SLOT(processFadeColourParts(bool, bool)));

    connect(partWorkerCustomColour, SIGNAL(progressBarInitSig()),
            gui,                   SLOT( progressBarInit()));
    connect(partWorkerCustomColour, SIGNAL(progressMessageSig(const QString &)),
            gui,                   SLOT( progressBarSetText(const QString &)));
    connect(partWorkerCustomColour, SIGNAL(progressRangeSig(int,int)),
            gui,                   SLOT( progressBarSetRange(int,int)));
    connect(partWorkerCustomColour, SIGNAL(progressSetValueSig(int)),
            gui,                   SLOT( progressBarSetValue(int)));
    connect(partWorkerCustomColour, SIGNAL(progressResetSig()),
            gui,                   SLOT( progressBarReset()));
    connect(partWorkerCustomColour, SIGNAL(progressStatusRemoveSig()),
            gui,                   SLOT( progressStatusRemove()));

    //qDebug() << qPrintable(QString("Sent overwrite fade parts = %1").arg(overwrite ? "True" : "False"));
    gui->partWorkerCustomColour->setDoFadeStep(setup);
    emit gui->operateFadeParts(overwrite, setup);
}

void Gui::processHighlightColourParts(bool overwrite, bool setup)
{
    gui->partWorkerCustomColour = new PartWorker();

    connect(gui,                   SIGNAL(operateHighlightParts(bool, bool)),
            partWorkerCustomColour, SLOT(  processHighlightColourParts(bool, bool)));

    connect(partWorkerCustomColour, SIGNAL(progressBarInitSig()),
            gui,                   SLOT(  progressBarInit()));
    connect(partWorkerCustomColour, SIGNAL(progressMessageSig(const QString &)),
            gui,                   SLOT(  progressBarSetText(const QString &)));
    connect(partWorkerCustomColour, SIGNAL(progressRangeSig(int,int)),
            gui,                   SLOT(  progressBarSetRange(int,int)));
    connect(partWorkerCustomColour, SIGNAL(progressSetValueSig(int)),
            gui,                   SLOT(  progressBarSetValue(int)));
    connect(partWorkerCustomColour, SIGNAL(progressResetSig()),
            gui,                   SLOT(  progressBarReset()));
    connect(partWorkerCustomColour, SIGNAL(progressStatusRemoveSig()),
            gui,                   SLOT(  progressStatusRemove()));

    //qDebug() << qPrintable(QString("Sent overwrite highlight parts = %1").arg(overwriteCustomParts ? "True" : "False"));
    gui->partWorkerCustomColour->setDoHighlightStep(setup);
    emit gui->operateHighlightParts(overwrite, setup);
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
    emit gui->messageSig(LOG_STATUS, tr("Download renderer %1...").arg(renderer));
    QTemporaryDir tempDir;
    QString downloadPath = tempDir.path();
    UpdateCheck *rendererDownload;
    QString rendererArchive, destination;

    switch (which)
    {
    case RENDERER_LDVIEW:
        rendererDownload = new UpdateCheck(gui, (void*)LDViewRendererDownload);
        destination = QDir::toNativeSeparators(tr("%1/%2").arg(Preferences::lpub3d3rdPartyAppDir, VER_LDVIEW_STR));
        break;
    case RENDERER_LDGLITE:
        rendererDownload = new UpdateCheck(gui, (void*)LDGLiteRendererDownload);
        destination = QDir::toNativeSeparators(tr("%1/%2").arg(Preferences::lpub3d3rdPartyAppDir, VER_LDGLITE_STR));
        break;
    case RENDERER_POVRAY:
        rendererDownload = new UpdateCheck(gui, (void*)POVRayRendererDownload);
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
    emit gui->messageSig(LOG_STATUS, message);

    QStringList items = JlCompress::getFileList(rendererArchive);
    gui->m_progressDialog = new ProgressDialog(nullptr);
    gui->m_progressDialog->setWindowFlags(gui->m_progressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
    gui->m_progressDialog->setWindowTitle(QString("Installing Renderer %1").arg(renderer));
    gui->m_progressDialog->setLabelText(QString("Extracting renderer %1 from %2...")
                                              .arg(renderer)
                                              .arg(QFileInfo(rendererArchive).fileName()));
    gui->m_progressDialog->setRange(0,items.count());
    gui->m_progressDialog->setAutoHide(true);
    gui->m_progressDialog->setModal(true);
    gui->m_progressDialog->show();

    QThread *thread    = new QThread(this);
    ExtractWorker *job = new ExtractWorker(rendererArchive,destination);
    job->moveToThread(thread);
    wait = new QEventLoop();

    disconnect(gui->m_progressDialog, SIGNAL (cancelClicked()),
            gui, SLOT (cancelExporting()));
    connect(thread, SIGNAL(started()),
            job, SLOT(doWork()));
    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()));
    connect(job, SIGNAL(setValueSig(int)),
            gui->m_progressDialog, SLOT(setValue(int)));
    connect(gui->m_progressDialog, SIGNAL(cancelClicked()),
            job, SLOT(requestEndWorkNow()));
    connect(job, SIGNAL(resultSig(int)),
            gui, SLOT(workerJobResult(int)));
    connect(gui, SIGNAL(requestEndThreadNowSig()),
            job, SLOT(requestEndWorkNow()));
    connect(job, SIGNAL(finishedSig()),
            thread, SLOT(quit()));
    connect(job, SIGNAL(finishedSig()),
            job, SLOT(deleteLater()));
    wait->connect(job, SIGNAL(finishedSig()),
            wait, SLOT(quit()));

    workerJobResult(0);
    thread->start();
    wait->exec();

    gui->m_progressDialog->setValue(items.count());

    if (gui->m_workerJobResult) {
        message = tr("%1 of %2 %3 renderer files installed to %4")
                     .arg(gui->m_workerJobResult)
                     .arg(items.count())
                     .arg(renderer)
                     .arg(destination);
        emit gui->messageSig(LOG_INFO,message);

        result = true;

#if defined Q_OS_WIN
    if (!Preferences::portableDistribution) {
        QSettings Settings;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"RendererApplicationDir"),Preferences::lpub3d3rdPartyAppDir);
    }
#elif defined Q_OS_LINUX
    Settings.setValue(QString("%1/%2").arg(SETTINGS,"RendererExecutableDir"), Preferences::lpub3d3rdPartyAppExeDir);
#endif

        Preferences::rendererPreferences();

    } else {
        message = tr("Failed to extract %1 %2 renderer files")
                     .arg(QFileInfo(rendererArchive).fileName()
                     .arg(renderer));
        emit gui->messageSig(LOG_ERROR,message);
    }

    connect (gui->m_progressDialog, SIGNAL(cancelClicked()),
             this,             SLOT(  cancelExporting()));

    gui->m_progressDialog->hide();

    emit gui->messageSig(LOG_STATUS, tr("Download renderer %1 completed.").arg(renderer));

    return result;
}

// Update parts archive from LDSearch directories

void Gui::loadLDSearchDirParts(bool Process, bool OnDemand, bool Update) {
  if (Process)
    gui->partWorkerLDSearchDirs.ldsearchDirPreferences();

  QStringList items = Preferences::ldSearchDirs;
  QString message;
  if (items.count()) {
      message = tr("Archiving search directory parts. Please wait...");
      emit gui->messageSig(LOG_INFO_STATUS,message);
      gui->m_progressDialog->setWindowFlags(gui->m_progressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
      gui->m_progressDialog->setWindowTitle(QString("LDraw Archive Library Update"));
      gui->m_progressDialog->setLabelText(QString("Archiving search directory parts..."));
      if (OnDemand) {
        gui->m_progressDialog->setRange(0,0);
        gui->m_progressDialog->setValue(1);
      } else {
        gui->m_progressDialog->setRange(0,items.count());
      }
      gui->m_progressDialog->setAutoHide(true);
      gui->m_progressDialog->setModal(true);
      gui->m_progressDialog->show();

      QThread *thread = new QThread(gui);
      PartWorker *job = &gui->partWorkerLDSearchDirs;
      if (OnDemand)
          job = new PartWorker(true/*OnDemand*/);
      job->moveToThread(thread);
      QEventLoop *wait = new QEventLoop();

      disconnect (gui->m_progressDialog, SIGNAL (cancelClicked()),
                  this,             SLOT (cancelExporting()));
      connect(gui->m_progressDialog,     SIGNAL(cancelClicked()),
              job,                  SLOT(requestEndThreadNow()));
      connect(job,                  SIGNAL(progressMessageSig (QString)),
              gui->m_progressDialog,     SLOT(setLabelText(QString)));
      if (!OnDemand)
          connect(job,                  SIGNAL(progressSetValueSig(int)),
                  m_progressDialog,     SLOT(setValue(int)));

      if (Update)
          connect(thread,           SIGNAL(started()),
                  job,              SLOT(updateLDSearchDirsParts()));
      else
          connect(thread,           SIGNAL(started()),
                  job,              SLOT(processLDSearchDirParts()));

      connect(thread,               SIGNAL(finished()),
              thread,               SLOT(deleteLater()));
      connect(job,                  SIGNAL(partsArchiveResultSig(int)),
              gui,                  SLOT(workerJobResult(int)));
      connect(gui,                  SIGNAL(requestEndThreadNowSig()),
              job,                  SLOT(requestEndThreadNow()));
      connect(job,                  SIGNAL(partsArchiveFinishedSig()),
              thread,               SLOT(quit()));

      if (OnDemand)
          connect(job,              SIGNAL(partsArchiveFinishedSig()),
                  job,              SLOT(deleteLater()));

      wait->connect(job,            SIGNAL(partsArchiveFinishedSig()),
                    wait,           SLOT(quit()));

      gui->workerJobResult(0);
      thread->start();
      wait->exec();

      if (!OnDemand)
          gui->m_progressDialog->setValue(items.count());

      connect (gui->m_progressDialog, SIGNAL (cancelClicked()),
               gui, SLOT (cancelExporting()));

      gui->m_progressDialog->hide();
      QString partsLabel = gui->m_workerJobResult == 1 ? "part" : "parts";
      message = tr("Added %1 %2 to %3 unofficial library archive.")
                   .arg(gui->m_workerJobResult)
                   .arg(partsLabel)
                   .arg(Preferences::validLDrawCustomArchive);
      emit gui->messageSig(LOG_INFO_STATUS,message);
  }

  if (! Gui::getCurFile().isEmpty()) {
      bool _continue;
      if (Preferences::saveOnRedraw) {
          _continue = gui->maybeSave(false); // No prompt
      } else {
          _continue = gui->maybeSave(true, SaveOnNone);
      }
      if (!_continue)
          return;

      QElapsedTimer timer;
      timer.start();

      gui->clearPLICache();
      gui->clearBOMCache();
      gui->clearCSICache();
      gui->clearSMICache();
      gui->clearTempCache();

      //reload current model file
      gui->cyclePageDisplay(Gui::displayPageNum, true/*silent*/, true/*FILE_RELOAD*/);

      emit gui->messageSig(LOG_INFO_STATUS, QString("%1 File %2 reloaded. %3")
                      .arg(message)
                      .arg(QFileInfo(Gui::getCurFile()).fileName())
                      .arg(Gui::elapsedTime(timer.elapsed())));
  }
}

void Gui::refreshLDrawUnoffParts() {

    // Download unofficial archive
    emit gui->messageSig(LOG_STATUS,"Refresh LDraw Unofficial Library archive...");
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
    emit gui->messageSig(LOG_STATUS,message);

    QStringList items = JlCompress::getFileList(newarchive);
    gui->m_progressDialog = new ProgressDialog(nullptr);
    gui->m_progressDialog->setWindowFlags(gui->m_progressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
    gui->m_progressDialog->setWindowTitle(QString("LDraw Library Update"));
    gui->m_progressDialog->setLabelText(QString("Extracting LDraw Unofficial parts from %1...")
                                              .arg(QFileInfo(newarchive).fileName()));
    gui->m_progressDialog->setRange(0,items.count());
    gui->m_progressDialog->setAutoHide(true);
    gui->m_progressDialog->setModal(true);
    gui->m_progressDialog->show();

    QThread *thread    = new QThread(this);
    ExtractWorker *job = new ExtractWorker(newarchive,destination);
    job->moveToThread(thread);
    wait = new QEventLoop();

    disconnect(m_progressDialog, SIGNAL (cancelClicked()),
            gui,                 SLOT (cancelExporting()));
    connect(thread,              SIGNAL(started()),
            job,                 SLOT(doWork()));
    connect(thread,              SIGNAL(finished()),
            thread,              SLOT(deleteLater()));
    connect(job,                 SIGNAL(setValueSig(int)),
            m_progressDialog,    SLOT(setValue(int)));
    connect(m_progressDialog,    SIGNAL(cancelClicked()),
            job,                 SLOT(requestEndWorkNow()));
    connect(job,                 SIGNAL(resultSig(int)),
            gui,                 SLOT(workerJobResult(int)));
    connect(gui,                 SIGNAL(requestEndThreadNowSig()),
            job,                 SLOT(requestEndWorkNow()));
    connect(job,                 SIGNAL(finishedSig()),
            thread,              SLOT(quit()));
    connect(job,                 SIGNAL(finishedSig()),
            job,                 SLOT(deleteLater()));
    wait->connect(job,           SIGNAL(finishedSig()),
                  wait,          SLOT(quit()));

    gui->workerJobResult(0);
    thread->start();
    wait->exec();

    gui->m_progressDialog->setValue(items.count());

    if (m_workerJobResult) {
        message = tr("%1 of %2 Unofficial library files extracted to %3")
                     .arg(m_workerJobResult)
                     .arg(items.count())
                     .arg(destination);
        emit gui->messageSig(LOG_INFO,message);
    } else {
        message = tr("Failed to extract %1 library files")
                     .arg(QFileInfo(newarchive).fileName());
        emit gui->messageSig(LOG_ERROR,message);
    }

   // Process custom and color parts if any
    items = Preferences::ldSearchDirs;
    if (items.count()) {
        QString message = tr("Archiving custom parts. Please wait...");
        emit gui->messageSig(LOG_STATUS,message);
        gui->m_progressDialog->setLabelText(QString("Archiving custom parts..."));
        gui->m_progressDialog->setRange(0,items.count());
        gui->m_progressDialog->show();

        QThread *thread = new QThread(this);
        PartWorker *job = new PartWorker(newarchive);
        job->moveToThread(thread);
        wait = new QEventLoop();

        connect(thread, SIGNAL(started()),
                job, SLOT(processPartsArchive()));
        connect(thread, SIGNAL(finished()),
                thread, SLOT(deleteLater()));
        connect(job, SIGNAL(progressSetValueSig(int)),
                m_progressDialog, SLOT(setValue(int)));
        connect(m_progressDialog, SIGNAL(cancelClicked()),
                job, SLOT(requestEndThreadNow()));
        connect(job, SIGNAL(progressMessageSig (QString)),
                m_progressDialog, SLOT(setLabelText(QString)));
        connect(job, SIGNAL(progressSetValueSig(int)),
                gui, SLOT(workerJobResult(int)));
        connect(gui, SIGNAL(requestEndThreadNowSig()),
                job, SLOT(requestEndThreadNow()));
        connect(job, SIGNAL(partsArchiveFinishedSig()),
                thread, SLOT(quit()));
        connect(job, SIGNAL(partsArchiveFinishedSig()),
                job, SLOT(deleteLater()));
        wait->connect(job, SIGNAL(partsArchiveFinishedSig()),
                wait, SLOT(quit()));

        gui->workerJobResult(0);
        thread->start();
        wait->exec();

        m_progressDialog->setValue(items.count());

        QString partsLabel = m_workerJobResult == 1 ? "part" : "parts";
        message = tr("Added %1 custom %2 into Unofficial library archive %3")
                     .arg(m_workerJobResult)
                     .arg(partsLabel)
                     .arg(QFileInfo(newarchive).fileName());
        emit gui->messageSig(LOG_INFO,message);

        if (m_workerJobResult) {
            QSettings Settings;
            QVariant uValue(true);
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,SAVE_SKIP_PARTS_ARCHIVE_KEY),uValue);
        }
    }

    connect (m_progressDialog, SIGNAL (cancelClicked()),
             gui, SLOT (cancelExporting()));

     m_progressDialog->hide();

    // Unload LDraw Unofficial archive library
    gui->UnloadUnofficialPiecesLibrary();

    // Copy new archive library to user data
    QString archivePath = QDir::toNativeSeparators(tr("%1/libraries").arg(Preferences::lpubDataPath));
    QString archive     = QDir::toNativeSeparators(tr("%1/%2").arg(archivePath).arg(VER_LPUB3D_UNOFFICIAL_ARCHIVE));
    QFile oldFile(archive);
    QFile newFile(newarchive);
    if (! newFile.exists()) {
        message = tr("Could not find file %1").arg(newFile.fileName());
        emit gui->messageSig(LOG_ERROR,message);
    } else if (! oldFile.exists() || oldFile.remove()) {
        if (! newFile.rename(archive)) {
            message = tr("Could not rename file %1").arg(newFile.fileName());
            emit gui->messageSig(LOG_ERROR,message);
        }
    } else {
        message = tr("Could not remove old file %1").arg(oldFile.fileName());
        emit  gui->messageSig(LOG_ERROR,message);
    }

    // Restart LDraw archive libraries
    gui->restartApplication(false);
}

void Gui::refreshLDrawOfficialParts() {

    // Download official archive
    emit gui->messageSig(LOG_STATUS,"Refresh LDraw Official Library archive...");
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
    emit gui->messageSig(LOG_INFO_STATUS,message);

    QStringList items = JlCompress::getFileList(newarchive);
    gui->m_progressDialog->setWindowFlags(gui->m_progressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
    gui->m_progressDialog->setWindowTitle(QString("LDraw Library Update"));
    gui->m_progressDialog->setLabelText(QString("Extracting LDraw Official parts from %1...")
                                              .arg(QFileInfo(newarchive).fileName()));
    gui->m_progressDialog->setRange(0,items.count());
    gui->m_progressDialog->setAutoHide(true);
    gui->m_progressDialog->setModal(true);
    gui->m_progressDialog->show();

    QThread *thread    = new QThread(gui);
    ExtractWorker *job = new ExtractWorker(newarchive,destination);
    job->moveToThread(thread);
    wait = new QEventLoop();

    disconnect(m_progressDialog, SIGNAL (cancelClicked()),
            gui,                 SLOT (cancelExporting()));
    connect(thread,              SIGNAL(started()),
            job,                 SLOT(doWork()));
    connect(thread,              SIGNAL(finished()),
            thread,              SLOT(deleteLater()));
    connect(job,                 SIGNAL(setValueSig(int)),
            m_progressDialog,    SLOT(setValue(int)));
    connect(m_progressDialog,    SIGNAL(cancelClicked()),
            job,                 SLOT(requestEndWorkNow()));
    connect(job,                 SIGNAL(resultSig(int)),
            gui,                 SLOT(workerJobResult(int)));
    connect(gui,                 SIGNAL(requestEndThreadNowSig()),
            job,                 SLOT(requestEndWorkNow()));
    connect(job,                 SIGNAL(finishedSig()),
            thread,              SLOT(quit()));
    connect(job,                 SIGNAL(finishedSig()),
            job,                 SLOT(deleteLater()));
    wait->connect(job,           SIGNAL(finishedSig()),
                  wait,          SLOT(quit()));

    gui->workerJobResult(0);
    thread->start();
    wait->exec();

    m_progressDialog->setValue(items.count());

    if (m_workerJobResult) {
        message = tr("%1 of %2 Library files extracted to %3")
                     .arg(m_workerJobResult)
                     .arg(items.count())
                     .arg(destination);
        emit gui->messageSig(LOG_INFO,message);
    } else {
        message = tr("Failed to extract %1 library files")
                     .arg(QFileInfo(newarchive).fileName());
        emit gui->messageSig(LOG_ERROR,message);
    }

    connect (m_progressDialog, SIGNAL (cancelClicked()),
             gui, SLOT (cancelExporting()));

     m_progressDialog->hide();

    // Unload LDraw Official archive libraries
    gui->UnloadOfficialPiecesLibrary();

    // Copy archive library to user data
    QString archivePath = QDir::toNativeSeparators(tr("%1/libraries").arg(Preferences::lpubDataPath));
    QString archive     = QDir::toNativeSeparators(tr("%1/%2").arg(archivePath).arg(VER_LDRAW_OFFICIAL_ARCHIVE));
    QFile oldFile(archive);
    QFile newFile(newarchive);
    if (! newFile.exists()) {
        message = tr("Could not find file %1").arg(newFile.fileName());
        emit gui->messageSig(LOG_ERROR,message);
    } else if (! oldFile.exists() || oldFile.remove()) {
        if (! newFile.rename(archive)) {
            message = tr("Could not rename file %1").arg(newFile.fileName());
            emit gui->messageSig(LOG_ERROR,message);
        }
    } else {
        message = tr("Could not remove old file %1").arg(oldFile.fileName());
        emit gui->messageSig(LOG_ERROR,message);
    }

    // Restart LDraw archive libraries
    gui->restartApplication(false);
}

void Gui::updateCheck()
{
    // Create an instance of update check
    new UpdateCheck(gui, (void*)SoftwareUpdate);
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
  QMessageBoxResizable box;
  box.setWindowIcon(QIcon());
  box.setIconPixmap (QPixmap(LPUB3D_MESSAGE_ICON));
  box.setTextFormat (Qt::RichText);
  box.setStandardButtons (QMessageBox::Yes| QMessageBox::No);
  box.setDefaultButton   (QMessageBox::Yes);
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  box.setWindowTitle(tr ("Export Meta Commands"));
  box.setText (tr ("<b> %1 </b>").arg(result));
  box.setInformativeText (tr ("Do you want to open this document ?\n\n%1")
                              .arg(QDir::toNativeSeparators(fileName)));

  if (box.exec() == QMessageBox::Yes) {
    QStringList arguments;
    QString CommandPath = fileName;
    QProcess *Process = new QProcess(this);
    Process->setWorkingDirectory(QFileInfo(CommandPath).absolutePath() + QDir::separator());
#ifdef Q_OS_WIN
    if (Preferences::usingNPP) {
        Process->setNativeArguments(CommandPath);
        arguments << CommandPath << QLatin1String(WINDOWS_NPP_LPUB3D_UDL_ARG);
        Process->startDetached(Preferences::systemEditor, arguments);
    } else
        QDesktopServices::openUrl((QUrl("file:///"+CommandPath, QUrl::TolerantMode)));
#else
    arguments << CommandPath;
    Process->start(UNIX_SHELL, arguments);
    Process->waitForFinished();
    if (Process->exitStatus() != QProcess::NormalExit || Process->exitCode() != 0) {
        QErrorMessage *m = new QErrorMessage(this);
        m->showMessage(tr("Failed to launch PDF document.\n%1\n%2")
                         .arg(CommandPath).arg(QString(Process->readAllStandardError())));
    }
#endif
  } else {
      emit gui->messageSig(LOG_INFO_STATUS, result);
  }
}

void Gui::createOpenWithActions(int maxPrograms)
{
    // adjust for separator and system editor
    const int systemEditor = Preferences::systemEditor.isEmpty() ? 0 : 1;
    const int maxOpenWithPrograms = maxPrograms ? maxPrograms + systemEditor : Preferences::maxOpenWithPrograms + systemEditor;

    if (!maxPrograms) {
        // set open with default program entries
        QSettings Settings;
        QString const openWithProgramListKey("OpenWithProgramList");
        if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,openWithProgramListKey))) {
            QStringList programEntries;
            for (int i = 0; i < maxPrograms; i++)
                programEntries.append(QString("Program %1|").arg(i + 1));
            Settings.setValue(QString("%1/%2").arg(SETTINGS,openWithProgramListKey), programEntries);
        }
    }

    for (int i = 0; i < maxOpenWithPrograms; i++) {
        QAction *openWithAct = new QAction(tr("Open With Application %1").arg(i), gui);
        openWithAct->setObjectName(tr("openWith%1Act.1").arg(i));
        openWithAct->setVisible(false);
        lpub->actions.insert(openWithAct->objectName(), Action(QStringLiteral("File.OpenWith.Application %1").arg(i), openWithAct));
        connect(openWithAct, SIGNAL(triggered()), gui, SLOT(openWith()));
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

QAction *Gui::getAct(const QString &objectName)
{
    if (lpub->actions.contains(objectName))
        return lpub->actions.value(objectName).action;
#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_ERROR, QString("Mainwindow action was not found or is null [%1]").arg(objectName));
#endif
    return nullptr;
}

void Gui::createActions()
{
    // File
    QAction *openAct = new QAction(QIcon(":/resources/open.png"), tr("&Open..."), gui);
    openAct->setObjectName("openAct.1");
    openAct->setShortcut(QStringLiteral("Ctrl+O"));
    openAct->setStatusTip(tr("Open an existing file"));
    lpub->actions.insert(openAct->objectName(), Action(QStringLiteral("File.Open"), openAct));
    connect(openAct, SIGNAL(triggered()), gui, SLOT(open()));

    createOpenWithActions();

    QAction *openWorkingFolderAct = new QAction(QIcon(":/resources/openworkingfolder.png"),tr("Open Working Folder..."), gui);
    openWorkingFolderAct->setObjectName("openWorkingFolderAct.1");
    openWorkingFolderAct->setShortcut(QStringLiteral("Alt+Shift+1"));
    openWorkingFolderAct->setStatusTip(tr("Open current model file working folder"));
    lpub->actions.insert(openWorkingFolderAct->objectName(), Action(QStringLiteral("File.Open Working Folder"), openWorkingFolderAct));
    connect(openWorkingFolderAct, SIGNAL(triggered()), gui, SLOT(openWorkingFolder()));

    QAction *reloadFromDiskAct = new QAction(QIcon(":/resources/loadfromdisk.png"), tr("&Reload from Disk"), gui);
    reloadFromDiskAct->setObjectName("reloadFromDiskAct.1");
    reloadFromDiskAct->setShortcut(QStringLiteral("Ctrl+Shift+R"));
    reloadFromDiskAct->setStatusTip(tr("Reload current file from disk"));
    reloadFromDiskAct->setEnabled(false);
    lpub->actions.insert(reloadFromDiskAct->objectName(), Action(QStringLiteral("File.Reload From Disk"), reloadFromDiskAct));
    connect(reloadFromDiskAct, SIGNAL(triggered()), gui, SLOT(reloadFromDisk()));

    QAction *saveAct = new QAction(QIcon(":/resources/save.png"), tr("&Save"), gui);
    saveAct->setObjectName("saveAct.1");
    saveAct->setShortcut(QStringLiteral("Ctrl+S"));
    saveAct->setStatusTip(tr("Save current file to disk"));
    saveAct->setEnabled(false);
    lpub->actions.insert(saveAct->objectName(), Action(QStringLiteral("File.Save"), saveAct));
    connect(saveAct, SIGNAL(triggered()), gui, SLOT(save()));

    QAction *saveAsAct = new QAction(QIcon(":/resources/saveas.png"),tr("Save A&s..."), gui);
    saveAsAct->setObjectName("saveAsAct.1");
    saveAsAct->setShortcut(QStringLiteral("Ctrl+Shift+S"));
    saveAsAct->setStatusTip(tr("Save current file under a new name"));
    saveAsAct->setEnabled(false);
    lpub->actions.insert(saveAsAct->objectName(), Action(QStringLiteral("File.Save As"), saveAsAct));
    connect(saveAsAct, SIGNAL(triggered()), gui, SLOT(saveAs()));

    QAction *saveCopyAct = new QAction(QIcon(":/resources/savecopy.png"),tr("Save a Copy As..."), gui);
    saveCopyAct->setObjectName("saveCopyAct.1");
    saveCopyAct->setShortcut(QStringLiteral("Ctrl+Shift+C"));
    saveCopyAct->setStatusTip(tr("Save a copy of the current file under a new name"));
    saveCopyAct->setEnabled(false);
    lpub->actions.insert(saveCopyAct->objectName(), Action(QStringLiteral("File.Save A Copy As"), saveCopyAct));
    connect(saveCopyAct, SIGNAL(triggered()), gui, SLOT(saveCopy()));

    QAction *closeFileAct = new QAction(QIcon(":/resources/closemodelfile.png"), tr("Close File"), gui);
    closeFileAct->setObjectName("closeFileAct.1");
    closeFileAct->setShortcut(QStringLiteral("Ctrl+W"));
    closeFileAct->setStatusTip(tr("Close current file"));
    closeFileAct->setEnabled(false);
    lpub->actions.insert(closeFileAct->objectName(), Action(QStringLiteral("File.Close File"), closeFileAct));
    connect(closeFileAct, SIGNAL(triggered()), gui, SLOT(closeModelFile()));

    QIcon printToFilePreviewIcon;
    printToFilePreviewIcon.addFile(":/resources/file_print_preview.png");
    printToFilePreviewIcon.addFile(":/resources/file_print_preview_16.png");
    QAction *printToFilePreviewAct = new QAction(printToFilePreviewIcon, tr("File Print Pre&view..."), gui);
    printToFilePreviewAct->setObjectName("printToFilePreviewAct.1");
    printToFilePreviewAct->setShortcut(QStringLiteral("Alt+Shift+R"));
    printToFilePreviewAct->setStatusTip(tr("Preview the current document to be printed"));
    printToFilePreviewAct->setEnabled(false);
    lpub->actions.insert(printToFilePreviewAct->objectName(), Action(QStringLiteral("File.File Print Preview"), printToFilePreviewAct));
    connect(printToFilePreviewAct, SIGNAL(triggered()), gui, SLOT(TogglePrintToFilePreview()));

    QIcon printToFileIcon;
    printToFileIcon.addFile(":/resources/file_print.png");
    printToFileIcon.addFile(":/resources/file_print_16.png");
    QAction *printToFileAct = new QAction(printToFileIcon, tr("File &Print..."), gui);
    printToFileAct->setObjectName("printToFileAct.1");
    printToFileAct->setShortcut(QStringLiteral("Alt+Shift+P"));
    printToFileAct->setStatusTip(tr("Print the current document"));
    printToFileAct->setEnabled(false);
    lpub->actions.insert(printToFileAct->objectName(), Action(QStringLiteral("File.File Print"), printToFileAct));
    connect(printToFileAct, SIGNAL(triggered()), gui, SLOT(ShowPrintDialog()));

    QAction *importLDDAct = new QAction(QIcon(":/resources/importldd.png"),tr("LEGO Digital Designer File..."), gui);
    importLDDAct->setObjectName("importLDDAct.1");
    importLDDAct->setShortcut(QStringLiteral("Alt+Shift+L"));
    importLDDAct->setStatusTip(tr("Import LEGO Digital Designer File"));
    lpub->actions.insert(importLDDAct->objectName(), Action(QStringLiteral("File.Import.LEGO Digital Designer File"), importLDDAct));
    connect(importLDDAct, SIGNAL(triggered()), gui, SLOT(importLDD()));

    QAction *importSetInventoryAct = new QAction(QIcon(":/resources/importsetinventory.png"),tr("Set Inventory File..."), gui);
    importSetInventoryAct->setObjectName("importSetInventoryAct.1");
    importSetInventoryAct->setShortcut(QStringLiteral("Alt+Shift+S"));
    importSetInventoryAct->setStatusTip(tr("Import Rebrickable Set Inventory File"));
    lpub->actions.insert(importSetInventoryAct->objectName(), Action(QStringLiteral("File.Import.Set Inventory File"), importSetInventoryAct));
    connect(importSetInventoryAct, SIGNAL(triggered()), gui, SLOT(importInventory()));

    QAction *exportAsPdfPreviewAct = new QAction(QIcon(":/resources/pdf_print_preview.png"), tr("PDF Export Preview..."), gui);
    exportAsPdfPreviewAct->setObjectName("exportAsPdfPreviewAct.1");
    exportAsPdfPreviewAct->setShortcut(QStringLiteral("Alt+P"));
    exportAsPdfPreviewAct->setStatusTip(tr("Preview the current pdf document to be exported"));
    exportAsPdfPreviewAct->setEnabled(false);
    lpub->actions.insert(exportAsPdfPreviewAct->objectName(), Action(QStringLiteral("File.PDF Export Preview"), exportAsPdfPreviewAct));
    connect(exportAsPdfPreviewAct, SIGNAL(triggered()), gui, SLOT(TogglePdfExportPreview()));

    QAction *exportAsPdfAct = new QAction(QIcon(":/resources/pdf_logo.png"), tr("Export to PDF &File..."), gui);
    exportAsPdfAct->setObjectName("exportAsPdfAct.1");
    exportAsPdfAct->setShortcut(QStringLiteral("Alt+F"));
    exportAsPdfAct->setStatusTip(tr("Export your document to a pdf file"));
    exportAsPdfAct->setEnabled(false);
    lpub->actions.insert(exportAsPdfAct->objectName(), Action(QStringLiteral("File.Export To PDF File"), exportAsPdfAct));
    connect(exportAsPdfAct, SIGNAL(triggered()), gui, SLOT(exportAsPdfDialog()));

    QAction *exportPngAct = new QAction(QIcon(":/resources/exportpng.png"),tr("P&NG Images..."), gui);
    exportPngAct->setObjectName("exportPngAct.1");
    exportPngAct->setShortcut(QStringLiteral("Alt+N"));
    exportPngAct->setStatusTip(tr("Export your document as a sequence of PNG images"));
    exportPngAct->setEnabled(false);
    lpub->actions.insert(exportPngAct->objectName(), Action(QStringLiteral("File.Export As.PNG Images"), exportPngAct));
    connect(exportPngAct, SIGNAL(triggered()), gui, SLOT(exportAsPngDialog()));

    QAction *exportJpgAct = new QAction(QIcon(":/resources/exportjpeg.png"),tr("&JPEG Images..."), gui);
    exportJpgAct->setObjectName("exportJpgAct.1");
    exportJpgAct->setShortcut(QStringLiteral("Alt+J"));
    exportJpgAct->setStatusTip(tr("Export your document as a sequence of JPEG images"));
    exportJpgAct->setEnabled(false);
    lpub->actions.insert(exportJpgAct->objectName(), Action(QStringLiteral("File.Export As.JPEG Images"), exportJpgAct));
    connect(exportJpgAct, SIGNAL(triggered()), gui, SLOT(exportAsJpgDialog()));

    QAction *exportBmpAct = new QAction(QIcon(":/resources/exportbmp.png"),tr("&Bitmap Images..."), gui);
    exportBmpAct->setObjectName("exportBmpAct.1");
    exportBmpAct->setShortcut(QStringLiteral("Alt+B"));
    exportBmpAct->setStatusTip(tr("Export your document as a sequence of bitmap images"));
    exportBmpAct->setEnabled(false);
    lpub->actions.insert(exportBmpAct->objectName(), Action(QStringLiteral("File.Export As.Bitmap Images"), exportBmpAct));
    connect(exportBmpAct, SIGNAL(triggered()), gui, SLOT(exportAsBmpDialog()));

    QAction *exportHtmlAct = new QAction(QIcon(":/resources/html32.png"),tr("&HTML Part List..."), gui);
    exportHtmlAct->setObjectName("exportHtmlAct.1");
    exportHtmlAct->setShortcut(QStringLiteral("Alt+6"));
    exportHtmlAct->setStatusTip(tr("Export your document as an HTML part list"));
    exportHtmlAct->setEnabled(false);
    lpub->actions.insert(exportHtmlAct->objectName(), Action(QStringLiteral("File.Export As.HTML Part List"), exportHtmlAct));
    connect(exportHtmlAct, SIGNAL(triggered()), gui, SLOT(exportAsHtml()));

    QAction *exportHtmlStepsAct = new QAction(QIcon(":/resources/htmlsteps32.png"),tr("HT&ML Steps..."), gui);
    exportHtmlStepsAct->setObjectName("exportHtmlStepsAct.1");
    exportHtmlStepsAct->setShortcut(QStringLiteral("Alt+Shift+6"));
    exportHtmlStepsAct->setStatusTip(tr("Export your document as navigatable steps in HTML format"));
    exportHtmlStepsAct->setEnabled(false);
    lpub->actions.insert(exportHtmlStepsAct->objectName(), Action(QStringLiteral("File.Export As.HTML Steps"), exportHtmlStepsAct));
    connect(exportHtmlStepsAct, SIGNAL(triggered()), gui, SLOT(exportAsHtmlSteps()));

    QAction *exportColladaAct = new QAction(QIcon(":/resources/dae32.png"),tr("&COLLADA Objects..."), gui);
    exportColladaAct->setObjectName("exportColladaAct.1");
    exportColladaAct->setShortcut(QStringLiteral("Alt+4"));
    exportColladaAct->setStatusTip(tr("Export your document as a sequence of Collada objects"));
    exportColladaAct->setEnabled(false);
    lpub->actions.insert(exportColladaAct->objectName(), Action(QStringLiteral("File.Export As.COLLADA Objects"), exportColladaAct));
    connect(exportColladaAct, SIGNAL(triggered()), gui, SLOT(exportAsColladaDialog()));

    QAction *exportObjAct = new QAction(QIcon(":/resources/obj32.png"),tr("&Wavefront Objects..."), gui);
    exportObjAct->setObjectName("exportObjAct.1");
    exportObjAct->setShortcut(QStringLiteral("Alt+1"));
    exportObjAct->setStatusTip(tr("Export your document as a sequence of Wavefront objects"));
    exportObjAct->setEnabled(false);
    lpub->actions.insert(exportObjAct->objectName(), Action(QStringLiteral("File.Export As.Wavefront Objects"), exportObjAct));
    connect(exportObjAct, SIGNAL(triggered()), gui, SLOT(exportAsObjDialog()));

    QAction *export3dsAct = new QAction(QIcon(":/resources/3ds32.png"),tr("&3DStudio Objects..."), gui);
    export3dsAct->setObjectName("export3dsAct.1");
    export3dsAct->setShortcut(QStringLiteral("Alt+5"));
    export3dsAct->setStatusTip(tr("Export your document as a sequence of 3DStudio objects"));
    export3dsAct->setEnabled(false);
    lpub->actions.insert(export3dsAct->objectName(), Action(QStringLiteral("File.Export As.3DStudio Objects"), export3dsAct));
    connect(export3dsAct, SIGNAL(triggered()), gui, SLOT(exportAs3dsDialog()));

    QAction *exportStlAct = new QAction(QIcon(":/resources/stl32.png"),tr("&Stereo Lithography Objects..."), gui);
    exportStlAct->setObjectName("exportStlAct.1");
    exportStlAct->setShortcut(QStringLiteral("Alt+2"));
    exportStlAct->setStatusTip(tr("Export your document as a sequence of Stereo Lithography Objects"));
    exportStlAct->setEnabled(false);
    lpub->actions.insert(exportStlAct->objectName(), Action(QStringLiteral("File.Export As.Stereo Lithography Objects"), exportStlAct));
    connect(exportStlAct, SIGNAL(triggered()), gui, SLOT(exportAsStlDialog()));

    QAction *exportPovAct = new QAction(QIcon(":/resources/povray32.png"),tr("&PovRay Scene Files..."), gui);
    exportPovAct->setObjectName("exportPovAct.1");
    exportPovAct->setShortcut(QStringLiteral("Alt+3"));
    exportPovAct->setStatusTip(tr("Export your document as a sequence of PovRay Scene Files"));
    exportPovAct->setEnabled(false);
    lpub->actions.insert(exportPovAct->objectName(), Action(QStringLiteral("File.Export As.PovRay Scene Files"), exportPovAct));
    connect(exportPovAct, SIGNAL(triggered()), gui, SLOT(exportAsPovDialog()));

    QAction *exportCsvAct = new QAction(QIcon(":/resources/csv32.png"),tr("&CSV Part List..."), gui);
    exportCsvAct->setObjectName("exportCsvAct.1");
    exportCsvAct->setShortcut(QStringLiteral("Alt+8"));
    exportCsvAct->setStatusTip(tr("Export your document as a CSV part list"));
    exportCsvAct->setEnabled(false);
    lpub->actions.insert(exportCsvAct->objectName(), Action(QStringLiteral("File.Export As.CSV Part List"), exportCsvAct));
    connect(exportCsvAct, SIGNAL(triggered()), gui, SLOT(exportAsCsv()));

    QAction *exportBricklinkAct = new QAction(QIcon(":/resources/bricklink32.png"),tr("&Bricklink XML Part List..."), gui);
    exportBricklinkAct->setObjectName("exportBricklinkAct.1");
    exportBricklinkAct->setShortcut(QStringLiteral("Alt+7"));
    exportBricklinkAct->setStatusTip(tr("Export your document as a Bricklink XML Part List"));
    exportBricklinkAct->setEnabled(false);
    lpub->actions.insert(exportBricklinkAct->objectName(), Action(QStringLiteral("File.Export As.Bricklink XML Part List"), exportBricklinkAct));
    connect(exportBricklinkAct, SIGNAL(triggered()), gui, SLOT(exportAsBricklinkXML()));

    QAction *loadStatusAct = new QAction(QIcon(":/resources/loadstatus.png"),tr("View Load Status..."), gui);
    loadStatusAct->setObjectName("loadStatusAct.1");
    loadStatusAct->setShortcut(QStringLiteral("Ctrl+Shift+V"));
    loadStatusAct->setStatusTip(tr("Display the current model file load status"));
    loadStatusAct->setEnabled(false);
    lpub->actions.insert(loadStatusAct->objectName(), Action(QStringLiteral("File.View Load Status"), loadStatusAct));
    connect(loadStatusAct, SIGNAL(triggered()), gui, SLOT(loadStatus()));

    QAction *recountPartsAct = new QAction(tr("Recount LDraw Parts"),gui);
    recountPartsAct->setObjectName("recountPartsAct.1");
    recountPartsAct->setStatusTip(tr("Enable recount LDraw parts on view load status"));
    recountPartsAct->setCheckable(true);
    recountPartsAct->setChecked(Preferences::recountParts);
    lpub->actions.insert(recountPartsAct->objectName(), Action(QStringLiteral("File.Recount LDraw Parts"), recountPartsAct));
    connect(recountPartsAct, SIGNAL(triggered()), gui, SLOT(recountParts()));

    QAction *exitAct = new QAction(QIcon(":/resources/exit.png"),tr("E&xit"), gui);
    exitAct->setObjectName("exitAct.1");
    exitAct->setShortcut(QStringLiteral("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    lpub->actions.insert(exitAct->objectName(), Action(QStringLiteral("File.Exit"), exitAct));
    connect(exitAct, SIGNAL(triggered()), gui, SLOT(close()));

    QIcon blenderIcon;
    blenderIcon.addFile(":/resources/blendericon.png");
    blenderIcon.addFile(":/resources/blendericon16.png");
    blenderRenderAct = new QAction(blenderIcon,tr("Render With Blender..."), gui);
    blenderRenderAct->setObjectName("blenderRenderAct.4");
    blenderRenderAct->setShortcut(QStringLiteral("Alt+Shift+8"));
    blenderRenderAct->setStatusTip(tr("Render the current step model using Blender Cycles"));
    blenderRenderAct->setEnabled(false);
    lpub->actions.insert(blenderRenderAct->objectName(), Action(QStringLiteral("3DViewer.Blender Render"), blenderRenderAct));
    connect(blenderRenderAct, SIGNAL(triggered()), gui, SLOT(showRenderDialog()));

    blenderImportAct = new QAction(blenderIcon,tr("Open In Blender..."), gui);
    blenderImportAct->setObjectName("blenderImportAct.4");
    blenderImportAct->setShortcut(QStringLiteral("Alt+Shift+9"));
    blenderImportAct->setStatusTip(tr("Import and open the current step model in Blender"));
    blenderImportAct->setEnabled(false);
    lpub->actions.insert(blenderImportAct->objectName(), Action(QStringLiteral("3DViewer.Blender Import"), blenderImportAct));
    connect(blenderImportAct, SIGNAL(triggered()), gui, SLOT(showRenderDialog()));

    povrayRenderAct = new QAction(QIcon(":/resources/povray32.png"),tr("Render With POVRay..."), gui);
    povrayRenderAct->setObjectName("povrayRenderAct.4");
    povrayRenderAct->setShortcut(QStringLiteral("Alt+9"));
    povrayRenderAct->setStatusTip(tr("Render the current step model using POV-Ray"));
    povrayRenderAct->setEnabled(false);
    lpub->actions.insert(povrayRenderAct->objectName(), Action(QStringLiteral("3DViewer.POVRay Render"), povrayRenderAct));
    connect(povrayRenderAct, SIGNAL(triggered()), gui, SLOT(showRenderDialog()));

    QAction *copyFileNameToClipboardAct = new QAction(QIcon(":/resources/copytoclipboard.png"),tr("File Name to Clipboard"), gui);
    copyFileNameToClipboardAct->setObjectName("copyFileNameToClipboardAct.1");
    copyFileNameToClipboardAct->setShortcut(QStringLiteral("Alt+Shift+4"));
    copyFileNameToClipboardAct->setStatusTip(tr("Copy file name to clipboard"));
    lpub->actions.insert(copyFileNameToClipboardAct->objectName(), Action(QStringLiteral("File.File Name To Clipboard"), copyFileNameToClipboardAct));
    connect(copyFileNameToClipboardAct, SIGNAL(triggered()), gui, SLOT(updateClipboard()));

    QAction *copyFilePathToClipboardAct = new QAction(QIcon(":/resources/copytoclipboard.png"),tr("Full Path to Clipboard"), gui);
    copyFilePathToClipboardAct->setObjectName("copyFilePathToClipboardAct.1");
    copyFilePathToClipboardAct->setShortcut(QStringLiteral("Alt+Shift+0"));
    copyFilePathToClipboardAct->setStatusTip(tr("Copy current model full file path to clipboard"));
    copyFilePathToClipboardAct->setEnabled(false);
    lpub->actions.insert(copyFilePathToClipboardAct->objectName(), Action(QStringLiteral("File.Full Path To Clipboard"), copyFilePathToClipboardAct));
    connect(copyFilePathToClipboardAct, SIGNAL(triggered()), gui, SLOT(updateClipboard()));

    for (int i = 0; i < MAX_RECENT_FILES; i++) {
      recentFilesActs[i] = new QAction(gui);
      recentFilesActs[i]->setObjectName(tr("recentFile%1Act.1").arg(i));
      recentFilesActs[i]->setStatusTip(tr("Recently opened file %1").arg(i));
      recentFilesActs[i]->setVisible(false);
      lpub->actions.insert(recentFilesActs[i]->objectName(), Action(QStringLiteral("Recent Files.Recent File %1").arg(i), recentFilesActs[i]));
      connect(recentFilesActs[i], SIGNAL(triggered()), gui, SLOT(openRecentFile()));
    }

    QAction *clearRecentAct = new QAction(tr("Clear Recent Files"),gui);
    clearRecentAct->setObjectName("clearRecentAct.1");
    clearRecentAct->setStatusTip(tr("Clear recent files"));
    lpub->actions.insert(clearRecentAct->objectName(), Action(QStringLiteral("Recent Files.Clear Recent Files"), clearRecentAct));
    connect(clearRecentAct, SIGNAL(triggered()), gui, SLOT(clearRecentFiles()));

    // Edit
    undoAct = new QAction(QIcon(":/resources/editundo.png"), tr("&Undo"), gui);
    undoAct->setObjectName("undoAct.1");
    undoAct->setShortcut(QStringLiteral("Ctrl+Z"));
    undoAct->setStatusTip(tr("Undo last change"));
    undoAct->setEnabled(false);
    lpub->actions.insert(undoAct->objectName(), Action(QStringLiteral("Edit.Undo"), undoAct));
    connect(undoAct, SIGNAL(triggered()), gui, SLOT(undo()));

    redoAct = new QAction(QIcon(":/resources/editredo.png"), tr("&Redo"), gui);
    redoAct->setObjectName("redoAct.1");
#ifdef __APPLE__
    redoAct->setShortcut(QStringLiteral("Ctrl+Shift+Z"));
    redoAct->setStatusTip(tr("Redo last change"));
#else
    redoAct->setShortcut(QStringLiteral("Ctrl+Y"));
    redoAct->setStatusTip(tr("Redo last change"));
#endif
    redoAct->setEnabled(false);
    lpub->actions.insert(redoAct->objectName(), Action(QStringLiteral("Edit.Redo"), redoAct));
    connect(redoAct, SIGNAL(triggered()), gui, SLOT(redo()));

    QAction *insertCoverPageAct = new QAction(QIcon(":/resources/insertcoverpage.png"),tr("Insert Front Cover Page"),gui);
    insertCoverPageAct->setObjectName("insertCoverPageAct.1");
    insertCoverPageAct->setShortcut(QStringLiteral("Ctrl+Alt+F"));
    insertCoverPageAct->setStatusTip(tr("Insert a front cover page"));
    insertCoverPageAct->setEnabled(false);
    lpub->actions.insert(insertCoverPageAct->objectName(), Action(QStringLiteral("Edit.Insert Front Cover Page"), insertCoverPageAct));
    connect(insertCoverPageAct, SIGNAL(triggered()), gui, SLOT(insertCoverPage()));

    QAction *appendCoverPageAct = new QAction(QIcon(":/resources/appendcoverpage.png"),tr("Append Back Cover Page"),gui);
    appendCoverPageAct->setObjectName("appendCoverPageAct.1");
    appendCoverPageAct->setShortcut(QStringLiteral("Ctrl+Alt+B"));
    appendCoverPageAct->setStatusTip(tr("Append back cover page"));
    appendCoverPageAct->setEnabled(false);
    lpub->actions.insert(appendCoverPageAct->objectName(), Action(QStringLiteral("Edit.Append Back Cover Page"), appendCoverPageAct));
    connect(appendCoverPageAct, SIGNAL(triggered()), gui, SLOT(appendCoverPage()));

    QAction *insertNumberedPageAct = new QAction(QIcon(":/resources/insertpage.png"),tr("Insert Page"),gui);
    insertNumberedPageAct->setObjectName("insertNumberedPageAct.1");
    insertNumberedPageAct->setShortcut(QStringLiteral("Ctrl+Alt+I"));
    insertNumberedPageAct->setStatusTip(tr("Insert a numbered page"));
    insertNumberedPageAct->setEnabled(false);
    lpub->actions.insert(insertNumberedPageAct->objectName(), Action(QStringLiteral("Edit.Insert Page"), insertNumberedPageAct));
    connect(insertNumberedPageAct, SIGNAL(triggered()), gui, SLOT(insertNumberedPage()));

    QAction *appendNumberedPageAct = new QAction(QIcon(":/resources/appendpage.png"),tr("Append Page"),gui);
    appendNumberedPageAct->setObjectName("appendNumberedPageAct.1");
    appendNumberedPageAct->setShortcut(QStringLiteral("Ctrl+Alt+A"));
    appendNumberedPageAct->setStatusTip(tr("Append a numbered page"));
    appendNumberedPageAct->setEnabled(false);
    lpub->actions.insert(appendNumberedPageAct->objectName(), Action(QStringLiteral("Edit.Append Page"), appendNumberedPageAct));
    connect(appendNumberedPageAct, SIGNAL(triggered()), gui, SLOT(appendNumberedPage()));

    QAction *deletePageAct = new QAction(QIcon(":/resources/deletepage.png"),tr("Delete Page"),gui);
    deletePageAct->setObjectName("deletePageAct.1");
    deletePageAct->setShortcut(QStringLiteral("Ctrl+Alt+D"));
    deletePageAct->setStatusTip(tr("Delete this page"));
    deletePageAct->setEnabled(false);
    lpub->actions.insert(deletePageAct->objectName(), Action(QStringLiteral("Edit.Delete Page"), deletePageAct));
    connect(deletePageAct, SIGNAL(triggered()), gui, SLOT(deletePage()));

    QAction *addPictureAct = new QAction(QIcon(":/resources/addpicture.png"),tr("Add Picture"),gui);
    addPictureAct->setObjectName("addPictureAct.1");
    addPictureAct->setShortcut(QStringLiteral("Ctrl+Alt+P"));
    addPictureAct->setStatusTip(tr("Add a picture to this page"));
    addPictureAct->setEnabled(false);
    lpub->actions.insert(addPictureAct->objectName(), Action(QStringLiteral("Edit.Add Picture"), addPictureAct));
    connect(addPictureAct, SIGNAL(triggered()), gui, SLOT(addPicture()));

    QAction *addTextAct = new QAction(QIcon(":/resources/addtext.png"),tr("Add Text"),gui);
    addTextAct->setObjectName("addTextAct.1");
    addTextAct->setShortcut(QStringLiteral("Ctrl+Alt+T"));
    addTextAct->setStatusTip(tr("Add text to this page"));
    addTextAct->setEnabled(false);
    lpub->actions.insert(addTextAct->objectName(), Action(QStringLiteral("Edit.Add Text"), addTextAct));
    connect(addTextAct, SIGNAL(triggered()), gui, SLOT(addText()));

    QAction *addBomAct = new QAction(QIcon(":/resources/addbom.png"),tr("Add Bill of Materials"),gui);
    addBomAct->setObjectName("addBomAct.1");
    addBomAct->setShortcut(QStringLiteral("Ctrl+Alt+M"));
    addBomAct->setStatusTip(tr("Add Bill of Materials to this page"));
    addBomAct->setEnabled(false);
    lpub->actions.insert(addBomAct->objectName(), Action(QStringLiteral("Edit.Add Bill Of Materials"), addBomAct));
    connect(addBomAct, SIGNAL(triggered()), gui, SLOT(addBom()));

    QAction *cycleEachPageAct = new QAction(QIcon(":/resources/cycleeachpage.png"),tr("Cycle Each Page"), gui);
    cycleEachPageAct->setObjectName("cycleEachPageAct.1");
    cycleEachPageAct->setStatusTip(tr("Cycle each page step(s) when navigating forward by more than one page - click to enable"));
    cycleEachPageAct->setCheckable(true);
    cycleEachPageAct->setChecked(Preferences::cycleEachPage);
    lpub->actions.insert(cycleEachPageAct->objectName(), Action(QStringLiteral("Edit.Cycle Each Page"), cycleEachPageAct));
    connect(cycleEachPageAct, SIGNAL(triggered()), gui, SLOT(cycleEachPage()));

    QAction *removeLPubFormatBomAct = new QAction(QIcon(":/resources/removelpubformatbom.png"),tr("Unformat Bill of Materials"),gui);
    removeLPubFormatBomAct->setObjectName("removeLPubFormatBomAct.1");
    removeLPubFormatBomAct->setShortcut(QStringLiteral("Ctrl+Alt+J"));
    removeLPubFormatBomAct->setStatusTip(tr("Remove all LPub Bill of Materials metacommands"));
    removeLPubFormatBomAct->setEnabled(false);
    lpub->actions.insert(removeLPubFormatBomAct->objectName(), Action(QStringLiteral("Edit.Remove LPub Format.Unformat Bill Of Materials"), removeLPubFormatBomAct));
    connect(removeLPubFormatBomAct, SIGNAL(triggered()), gui, SLOT(removeLPubFormatting()));

    QAction *removeLPubFormatDocumentAct = new QAction(QIcon(":/resources/removelpubformatdocument.png"),tr("Unformat Document"),gui);
    removeLPubFormatDocumentAct->setObjectName("removeLPubFormatDocumentAct.1");
    removeLPubFormatDocumentAct->setShortcut(QStringLiteral("Ctrl+Alt+R"));
    removeLPubFormatDocumentAct->setStatusTip(tr("Remove all LPub metacommands from entire document"));
    removeLPubFormatDocumentAct->setEnabled(false);
    lpub->actions.insert(removeLPubFormatDocumentAct->objectName(), Action(QStringLiteral("Edit.Remove LPub Format.Unformat Document"), removeLPubFormatDocumentAct));
    connect(removeLPubFormatDocumentAct, SIGNAL(triggered()), gui, SLOT(removeLPubFormatting()));

    QAction *removeLPubFormatPageAct = new QAction(QIcon(":/resources/removelpubformatpage.png"),tr("Unformat Current Page"),gui);
    removeLPubFormatPageAct->setObjectName("removeLPubFormatPageAct.1");
    removeLPubFormatPageAct->setShortcut(QStringLiteral("Ctrl+Alt+E"));
    removeLPubFormatPageAct->setStatusTip(tr("Remove all LPub metacommands from the current page"));
    removeLPubFormatPageAct->setEnabled(false);
    lpub->actions.insert(removeLPubFormatPageAct->objectName(), Action(QStringLiteral("Edit.Remove LPub Format.Unformat Current Page"), removeLPubFormatPageAct));
    connect(removeLPubFormatPageAct, SIGNAL(triggered()), gui, SLOT(removeLPubFormatting()));

    QAction *removeLPubFormatStepAct = new QAction(QIcon(":/resources/removelpubformatstep.png"),tr("Unformat Single Step"),gui);
    removeLPubFormatStepAct->setObjectName("removeLPubFormatStepAct.1");
    removeLPubFormatStepAct->setShortcut(QStringLiteral("Ctrl+Alt+L"));
    removeLPubFormatStepAct->setStatusTip(tr("Remove all LPub metacommands from the current step"));
    removeLPubFormatStepAct->setEnabled(false);
    lpub->actions.insert(removeLPubFormatStepAct->objectName(), Action(QStringLiteral("Edit.Remove LPub Format.Unformat Single Step"), removeLPubFormatStepAct));
    connect(removeLPubFormatStepAct, SIGNAL(triggered()), gui, SLOT(removeLPubFormatting()));

    QAction *removeLPubFormatSubmodelAct = new QAction(QIcon(":/resources/removelpubformatsubmodel.png"),tr("Unformat Current Submodel"),gui);
    removeLPubFormatSubmodelAct->setObjectName("removeLPubFormatSubmodelAct.1");
    removeLPubFormatSubmodelAct->setShortcut(QStringLiteral("Ctrl+Alt+H"));
    removeLPubFormatSubmodelAct->setStatusTip(tr("Remove all LPub metacommands from the current submodel"));
    removeLPubFormatSubmodelAct->setEnabled(false);
    lpub->actions.insert(removeLPubFormatSubmodelAct->objectName(), Action(QStringLiteral("Edit.Remove LPub Format.Unformat Current Submodel"), removeLPubFormatSubmodelAct));
    connect(removeLPubFormatSubmodelAct, SIGNAL(triggered()), gui, SLOT(removeLPubFormatting()));

    QAction *removeBuildModFormatAct = new QAction(tr("Remove Build Mod Format"),gui);
    removeBuildModFormatAct->setObjectName("removeBuildModFormatAct.1");
    removeBuildModFormatAct->setStatusTip(tr("Remove build modification formatting, including modified part lines, for submodel, step, or page unformat."));
    removeBuildModFormatAct->setCheckable(true);
    removeBuildModFormatAct->setEnabled(Preferences::buildModEnabled);
    removeBuildModFormatAct->setChecked(Preferences::removeBuildModFormat);
    lpub->actions.insert(removeBuildModFormatAct->objectName(), Action(QStringLiteral("Edit.Remove LPub Format.Remove Build Mod Format"), removeBuildModFormatAct));
    connect(removeBuildModFormatAct, SIGNAL(triggered()), gui, SLOT(removeBuildModFormat()));

    QAction *removeChildSubmodelFormatAct = new QAction(tr("Remove Child Submodel Format"),gui);
    removeChildSubmodelFormatAct->setObjectName("removeChildSubmodelFormatAct.1");
    removeChildSubmodelFormatAct->setStatusTip(tr("Remove child submodel formatting for submodel, step, or page unformat."));
    removeChildSubmodelFormatAct->setCheckable(true);
    removeChildSubmodelFormatAct->setChecked(Preferences::removeChildSubmodelFormat);
    lpub->actions.insert(removeChildSubmodelFormatAct->objectName(), Action(QStringLiteral("Edit.Remove LPub Format.Remove Child Submodel Format"), removeChildSubmodelFormatAct));
    connect(removeChildSubmodelFormatAct, SIGNAL(triggered()), gui, SLOT(removeChildSubmodelFormat()));

    // View
    QAction *fitWidthAct = new QAction(QIcon(":/resources/fitWidth.png"), tr("Fit &Width"), gui);
    fitWidthAct->setObjectName("fitWidthAct.1");
    fitWidthAct->setShortcut(QStringLiteral("Alt+W"));
    fitWidthAct->setStatusTip(tr("Fit document to width"));
    fitWidthAct->setEnabled(false);
    lpub->actions.insert(fitWidthAct->objectName(), Action(QStringLiteral("View.Fit Width"), fitWidthAct));
    connect(fitWidthAct, SIGNAL(triggered()), gui, SLOT(fitWidth()));

    QAction *fitVisibleAct = new QAction(QIcon(":/resources/fitVisible.png"), tr("Fit &Visible"), gui);
    fitVisibleAct->setObjectName("fitVisibleAct.1");
    fitVisibleAct->setShortcut(QStringLiteral("Alt+V"));
    fitVisibleAct->setStatusTip(tr("Fit document so whole page is visible"));
    fitVisibleAct->setEnabled(false);
    lpub->actions.insert(fitVisibleAct->objectName(), Action(QStringLiteral("View.Fit Visible"), fitVisibleAct));
    connect(fitVisibleAct, SIGNAL(triggered()), gui, SLOT(fitVisible()));

    QAction *fitSceneAct = new QAction(QIcon(":/resources/fitScene.png"), tr("Fit &Scene"), gui);
    fitSceneAct->setObjectName("fitSceneAct.1");
    fitSceneAct->setShortcut(QStringLiteral("Alt+H"));
    fitSceneAct->setStatusTip(tr("Fit document so whole scene is visible"));
    fitSceneAct->setEnabled(false);
    lpub->actions.insert(fitSceneAct->objectName(), Action(QStringLiteral("View.Fit Scene"), fitSceneAct));
    connect(fitSceneAct, SIGNAL(triggered()), gui, SLOT(fitScene()));

    QAction *bringToFrontAct = new QAction(QIcon(":/resources/bringtofront.png"), tr("Bring To &Front"), gui);
    bringToFrontAct->setObjectName("bringToFrontAct.1");
    bringToFrontAct->setShortcut(QStringLiteral("Alt+Shift+F"));
    bringToFrontAct->setStatusTip(tr("Bring item to front"));
    bringToFrontAct->setEnabled(false);
    lpub->actions.insert(bringToFrontAct->objectName(), Action(QStringLiteral("View.Bring To Front"), bringToFrontAct));
    connect(bringToFrontAct, SIGNAL(triggered()), gui, SLOT(bringToFront()));

    QAction *sendToBackAct = new QAction(QIcon(":/resources/sendtoback.png"), tr("Send To &Back"), gui);
    sendToBackAct->setObjectName("sendToBackAct.1");
    sendToBackAct->setShortcut(QStringLiteral("Alt+Shift+B"));
    sendToBackAct->setStatusTip(tr("Send item to back"));
    sendToBackAct->setEnabled(false);
    lpub->actions.insert(sendToBackAct->objectName(), Action(QStringLiteral("View.Send To Back"), sendToBackAct));
    connect(sendToBackAct, SIGNAL(triggered()), gui, SLOT(sendToBack()));

    QAction *actualSizeAct = new QAction(QIcon(":/resources/actual.png"),tr("&Actual Size"), gui);
    actualSizeAct->setObjectName("actualSizeAct.1");
    actualSizeAct->setShortcut(QStringLiteral("Alt+A"));
    actualSizeAct->setStatusTip(tr("Show document actual size"));
    actualSizeAct->setEnabled(false);
    lpub->actions.insert(actualSizeAct->objectName(), Action(QStringLiteral("View.Actual Size"), actualSizeAct));
    connect(actualSizeAct, SIGNAL(triggered()), gui, SLOT(actualSize()));

    // TESTING ONLY
    //connect(actualSizeAct, SIGNAL(triggered()), gui, SLOT(twoPages()));

    QAction *fullScreenViewAct = new QAction(QIcon(":/resources/fullscreenview.png"),tr("Full Screen View"),gui);
    fullScreenViewAct->setObjectName("fullScreenViewAct.1");
    fullScreenViewAct->setShortcut(QStringLiteral("Ctrl+M"));
    fullScreenViewAct->setStatusTip(tr("Toggle full screen view"));
    fullScreenViewAct->setCheckable(true);
    lpub->actions.insert(fullScreenViewAct->objectName(), Action(QStringLiteral("View.Full Screen View"), fullScreenViewAct));
    connect(fullScreenViewAct, SIGNAL(triggered()), gui, SLOT(fullScreenView()));

    zoomSliderAct = new QWidgetAction(nullptr);
    zoomSliderWidget = new QSlider();
    zoomSliderWidget->setSingleStep(1);
    zoomSliderWidget->setTickInterval(10);
    zoomSliderWidget->setTickPosition(QSlider::TicksBelow);
    zoomSliderWidget->setMaximum(150);
    zoomSliderWidget->setMinimum(1);
    zoomSliderWidget->setValue(50);
    zoomSliderAct->setDefaultWidget(zoomSliderWidget);
    connect(zoomSliderWidget, SIGNAL(valueChanged(int)), gui, SLOT(zoomSlider(int)));

    QAction *zoomInComboAct = new QAction(QIcon(":/resources/zoomin.png"), tr("&Zoom In"), gui);
    zoomInComboAct->setObjectName("zoomInComboAct.1");
    zoomInComboAct->setShortcut(QStringLiteral("Ctrl++"));
    zoomInComboAct->setStatusTip(tr("Zoom in"));
    zoomInComboAct->setEnabled(false);
    lpub->actions.insert(zoomInComboAct->objectName(), Action(QStringLiteral("View.Zoom In"), zoomInComboAct));
    connect(zoomInComboAct, SIGNAL(triggered()), gui, SLOT(zoomIn()));

    QAction *zoomOutComboAct = new QAction(QIcon(":/resources/zoomout.png"),tr("Zoom &Out"),gui);
    zoomOutComboAct->setObjectName("zoomOutComboAct.1");
    zoomOutComboAct->setShortcut(QStringLiteral("Ctrl+-"));
    zoomOutComboAct->setStatusTip(tr("Zoom out"));
    zoomOutComboAct->setEnabled(false);
    lpub->actions.insert(zoomOutComboAct->objectName(), Action(QStringLiteral("View.Zoom Out"), zoomOutComboAct));
    connect(zoomOutComboAct, SIGNAL(triggered()), gui, SLOT(zoomOut()));

    QIcon sceneRulerIcon;
    if (Preferences::sceneRuler) sceneRulerIcon.addFile(":/resources/pagerulercheck.png");
    else sceneRulerIcon.addFile(":/resources/pageruler.png");
    QAction *sceneRulerComboAct = new QAction(sceneRulerIcon, tr("Scene &Ruler"), gui);
    sceneRulerComboAct->setObjectName("sceneRulerComboAct.1");
    sceneRulerComboAct->setShortcut(QStringLiteral("Alt+U"));
    sceneRulerComboAct->setStatusTip(tr("Toggle the scene ruler"));
    sceneRulerComboAct->setEnabled(true);
    sceneRulerComboAct->setCheckable(true);
    sceneRulerComboAct->setChecked(Preferences::sceneRuler);
    lpub->actions.insert(sceneRulerComboAct->objectName(), Action(QStringLiteral("View.Scene Ruler"), sceneRulerComboAct));
    connect(sceneRulerComboAct, SIGNAL(triggered()), gui, SLOT(sceneRuler()));

    QAction *hideRulerPageBackgroundAct = new QAction(tr("Hide Page Background"),gui);
    hideRulerPageBackgroundAct->setObjectName("hideRulerPageBackgroundAct.1");
    hideRulerPageBackgroundAct->setStatusTip(tr("Toggle hide ruler page background"));
    hideRulerPageBackgroundAct->setCheckable(true);
    hideRulerPageBackgroundAct->setChecked(Preferences::hidePageBackground);
    hideRulerPageBackgroundAct->setEnabled(Preferences::sceneRuler);
    lpub->actions.insert(hideRulerPageBackgroundAct->objectName(), Action(QStringLiteral("View.Scene Ruler.Hide Page Background"), hideRulerPageBackgroundAct));
    connect(hideRulerPageBackgroundAct, SIGNAL(triggered()), gui, SLOT(hidePageBackground()));

    QAction *sceneRulerTrackingNoneAct = new QAction(tr("Ruler Tracking Off"),gui);
    sceneRulerTrackingNoneAct->setObjectName("sceneRulerTrackingNoneAct.1");
    sceneRulerTrackingNoneAct->setStatusTip(tr("Set scene ruler tracking Off"));
    sceneRulerTrackingNoneAct->setCheckable(true);
    lpub->actions.insert(sceneRulerTrackingNoneAct->objectName(), Action(QStringLiteral("View.Scene Ruler.Ruler Tracking Off"), sceneRulerTrackingNoneAct));
    connect(sceneRulerTrackingNoneAct, SIGNAL(triggered()), gui, SLOT(sceneRulerTracking()));

    QAction *sceneRulerTrackingTickAct = new QAction(tr("Ruler Tracking Tick"),gui);
    sceneRulerTrackingTickAct->setObjectName("sceneRulerTrackingTickAct.1");
    sceneRulerTrackingTickAct->setStatusTip(tr("Set scene ruler tracking to use ruler tick mark"));
    sceneRulerTrackingTickAct->setCheckable(true);
    lpub->actions.insert(sceneRulerTrackingTickAct->objectName(), Action(QStringLiteral("View.Scene Ruler.Ruler Tracking Tick"), sceneRulerTrackingTickAct));
    connect(sceneRulerTrackingTickAct, SIGNAL(triggered()), gui, SLOT(sceneRulerTracking()));

    QAction *sceneRulerTrackingLineAct = new QAction(tr("Ruler Tracking Line"),gui);
    sceneRulerTrackingLineAct->setObjectName("sceneRulerTrackingLineAct.1");
    sceneRulerTrackingLineAct->setStatusTip(tr("Set scene ruler tracking to use full scene line"));
    sceneRulerTrackingLineAct->setCheckable(true);
    lpub->actions.insert(sceneRulerTrackingLineAct->objectName(), Action(QStringLiteral("View.Scene Ruler.Ruler Tracking Line"), sceneRulerTrackingLineAct));
    connect(sceneRulerTrackingLineAct, SIGNAL(triggered()), gui, SLOT(sceneRulerTracking()));

    QAction *showTrackingCoordinatesAct = new QAction(tr("Show Tracking Coordinates"),gui);
    showTrackingCoordinatesAct->setObjectName("showTrackingCoordinatesAct.1");
    showTrackingCoordinatesAct->setStatusTip(tr("Toggle show ruler tracking coordinates"));
    showTrackingCoordinatesAct->setCheckable(true);
    showTrackingCoordinatesAct->setChecked(Preferences::showTrackingCoordinates);
    showTrackingCoordinatesAct->setEnabled(Preferences::sceneRulerTracking == int(TRACKING_LINE));
    lpub->actions.insert(showTrackingCoordinatesAct->objectName(), Action(QStringLiteral("View.Scene Ruler.Show Tracking Coordinates"), showTrackingCoordinatesAct));
    connect(showTrackingCoordinatesAct, SIGNAL(triggered()), gui, SLOT(showCoordinates()));

    SceneRulerGroup = new QActionGroup(gui);
    SceneRulerGroup->setEnabled(Preferences::sceneRuler);
    sceneRulerTrackingNoneAct->setChecked(Preferences::sceneRulerTracking == int(TRACKING_NONE));
    SceneRulerGroup->addAction(sceneRulerTrackingNoneAct);
    sceneRulerTrackingTickAct->setChecked(Preferences::sceneRulerTracking == int(TRACKING_TICK));
    SceneRulerGroup->addAction(sceneRulerTrackingTickAct);
    sceneRulerTrackingLineAct->setChecked(Preferences::sceneRulerTracking == int(TRACKING_LINE));
    SceneRulerGroup->addAction(sceneRulerTrackingLineAct);

    QIcon pageGuidesIcon;
    if (Preferences::sceneGuides) pageGuidesIcon.addFile(":/resources/pageguidescheck.png");
    else pageGuidesIcon.addFile(":/resources/pageguides.png");
    QAction *sceneGuidesComboAct = new QAction(pageGuidesIcon, tr("Scene &Guides"), gui);
    sceneGuidesComboAct->setObjectName("sceneGuidesComboAct.1");
    sceneGuidesComboAct->setShortcut(QStringLiteral("Alt+G"));
    sceneGuidesComboAct->setStatusTip(tr("Toggle horizontal and vertical scene guides"));
    sceneGuidesComboAct->setEnabled(true);
    sceneGuidesComboAct->setCheckable(true);
    sceneGuidesComboAct->setChecked(Preferences::sceneGuides);
    lpub->actions.insert(sceneGuidesComboAct->objectName(), Action(QStringLiteral("View.Scene Guides.Toggle"), sceneGuidesComboAct));
    connect(sceneGuidesComboAct, SIGNAL(triggered()), gui, SLOT(sceneGuides()));

    QAction *sceneGuidesDashLineAct = new QAction(tr("Dash Line"),gui);
    sceneGuidesDashLineAct->setObjectName("sceneGuidesDashLineAct.1");
    sceneGuidesDashLineAct->setStatusTip(tr("Select dash scene guide lines"));
    sceneGuidesDashLineAct->setCheckable(true);
    lpub->actions.insert(sceneGuidesDashLineAct->objectName(), Action(QStringLiteral("View.Scene Guides.Dash Line"), sceneGuidesDashLineAct));
    connect(sceneGuidesDashLineAct, SIGNAL(triggered()), gui, SLOT(sceneGuidesLine()));

    QAction *sceneGuidesSolidLineAct = new QAction(tr("Solid Line"),gui);
    sceneGuidesSolidLineAct->setObjectName("sceneGuidesSolidLineAct.1");
    sceneGuidesSolidLineAct->setStatusTip(tr("Select solid scene guide lines"));
    sceneGuidesSolidLineAct->setCheckable(true);
    lpub->actions.insert(sceneGuidesSolidLineAct->objectName(), Action(QStringLiteral("View.Scene Guides.Solid Line"), sceneGuidesSolidLineAct));
    connect(sceneGuidesSolidLineAct, SIGNAL(triggered()), gui, SLOT(sceneGuidesLine()));

    SceneGuidesLineGroup = new QActionGroup(gui);
    SceneGuidesLineGroup->setEnabled(Preferences::sceneGuides);
    sceneGuidesDashLineAct->setChecked(Preferences::sceneGuidesLine == int(Qt::DashLine));
    SceneGuidesLineGroup->addAction(sceneGuidesDashLineAct);
    sceneGuidesSolidLineAct->setChecked(Preferences::sceneGuidesLine == int(Qt::SolidLine));
    SceneGuidesLineGroup->addAction(sceneGuidesSolidLineAct);

    QAction *sceneGuidesPosTLeftAct = new QAction(tr("Top Left"),gui);
    sceneGuidesPosTLeftAct->setObjectName("sceneGuidesPosTLeftAct.1");
    sceneGuidesPosTLeftAct->setStatusTip(tr("Set scene guides position to top left of graphic item"));
    sceneGuidesPosTLeftAct->setCheckable(true);
    lpub->actions.insert(sceneGuidesPosTLeftAct->objectName(), Action(QStringLiteral("View.Scene Guides.Top Left"), sceneGuidesPosTLeftAct));
    connect(sceneGuidesPosTLeftAct, SIGNAL(triggered()), gui, SLOT(sceneGuidesPosition()));

    QAction *sceneGuidesPosTRightAct = new QAction(tr("Top Right"),gui);
    sceneGuidesPosTRightAct->setObjectName("sceneGuidesPosTRightAct.1");
    sceneGuidesPosTRightAct->setStatusTip(tr("Set scene guides position to top right of graphic item"));
    sceneGuidesPosTRightAct->setCheckable(true);
    lpub->actions.insert(sceneGuidesPosTRightAct->objectName(), Action(QStringLiteral("View.Scene Guides.Top Right"), sceneGuidesPosTRightAct));
    connect(sceneGuidesPosTRightAct, SIGNAL(triggered()), gui, SLOT(sceneGuidesPosition()));

    QAction *sceneGuidesPosBLeftAct = new QAction(tr("Bottom Left"),gui);
    sceneGuidesPosBLeftAct->setObjectName("sceneGuidesPosBLeftAct.1");
    sceneGuidesPosBLeftAct->setStatusTip(tr("Set scene guides position to bottom left of graphic item"));
    sceneGuidesPosBLeftAct->setCheckable(true);
    lpub->actions.insert(sceneGuidesPosBLeftAct->objectName(), Action(QStringLiteral("View.Scene Guides.Bottom Left"), sceneGuidesPosBLeftAct));
    connect(sceneGuidesPosBLeftAct, SIGNAL(triggered()), gui, SLOT(sceneGuidesPosition()));

    QAction *sceneGuidesPosBRightAct = new QAction(tr("Bottom Right"),gui);
    sceneGuidesPosBRightAct->setObjectName("sceneGuidesPosBRightAct.1");
    sceneGuidesPosBRightAct->setStatusTip(tr("Set scene guides position to bottom right of graphic item"));
    sceneGuidesPosBRightAct->setCheckable(true);
    lpub->actions.insert(sceneGuidesPosBRightAct->objectName(), Action(QStringLiteral("View.Scene Guides.Bottom Right"), sceneGuidesPosBRightAct));
    connect(sceneGuidesPosBRightAct, SIGNAL(triggered()), gui, SLOT(sceneGuidesPosition()));

    QAction *sceneGuidesPosCentreAct = new QAction(tr("Centre"),gui);
    sceneGuidesPosCentreAct->setObjectName("sceneGuidesPosCentreAct.1");
    sceneGuidesPosCentreAct->setStatusTip(tr("Set scene guides position to centre of graphic item"));
    sceneGuidesPosCentreAct->setCheckable(true);
    lpub->actions.insert(sceneGuidesPosCentreAct->objectName(), Action(QStringLiteral("View.Scene Guides.Centre"), sceneGuidesPosCentreAct));
    connect(sceneGuidesPosCentreAct, SIGNAL(triggered()), gui, SLOT(sceneGuidesPosition()));

    QAction *showGuidesCoordinatesAct = new QAction(tr("Show Guide Coordinates"),gui);
    showGuidesCoordinatesAct->setObjectName("showGuidesCoordinatesAct.1");
    showGuidesCoordinatesAct->setStatusTip(tr("Toggle show scene guide coordinates"));
    showGuidesCoordinatesAct->setCheckable(true);
    showGuidesCoordinatesAct->setChecked(Preferences::showGuidesCoordinates);
    showGuidesCoordinatesAct->setEnabled(Preferences::sceneGuides);
    lpub->actions.insert(showGuidesCoordinatesAct->objectName(), Action(QStringLiteral("View.Scene Guides.Show Guide Coordinates"), showGuidesCoordinatesAct));
    connect(showGuidesCoordinatesAct, SIGNAL(triggered()), gui, SLOT(showCoordinates()));

    SceneGuidesPosGroup = new QActionGroup(gui);
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

    QIcon snapToGridIcon;
    if (Preferences::snapToGrid) snapToGridIcon.addFile(":/resources/scenegridcheck.png");
    else snapToGridIcon.addFile(":/resources/scenegrid.png");
    QAction *snapToGridComboAct = new QAction(snapToGridIcon,tr("&Snap To Grid"),gui);
    snapToGridComboAct->setObjectName("snapToGridComboAct.1");
    snapToGridComboAct->setShortcut(QStringLiteral("Alt+K"));
    snapToGridComboAct->setStatusTip(tr("Toggle snap-to-grid"));
    snapToGridComboAct->setCheckable(true);
    snapToGridComboAct->setChecked(Preferences::snapToGrid);
    lpub->actions.insert(snapToGridComboAct->objectName(), Action(QStringLiteral("View.Snap To Grid.Toggle"), snapToGridComboAct));
    connect(snapToGridComboAct, SIGNAL(triggered()), gui, SLOT(snapToGrid()));

    QAction *hideGridPageBackgroundAct = new QAction(tr("Hide Page Background"),gui);
    hideGridPageBackgroundAct->setObjectName("hideGridPageBackgroundAct.1");
    hideGridPageBackgroundAct->setStatusTip(tr("Toggle hide snap to grid page background"));
    hideGridPageBackgroundAct->setCheckable(true);
    hideGridPageBackgroundAct->setChecked(Preferences::hidePageBackground);
    hideGridPageBackgroundAct->setEnabled(Preferences::snapToGrid);
    lpub->actions.insert(hideGridPageBackgroundAct->objectName(), Action(QStringLiteral("View.Snap To Grid.Hide Page Background"), hideGridPageBackgroundAct));
    connect(hideGridPageBackgroundAct, SIGNAL(triggered()), gui, SLOT(hidePageBackground()));

    GridStepSizeGroup = new QActionGroup(gui);
    GridStepSizeGroup->setEnabled(Preferences::snapToGrid);
    for (int ActionIdx = GRID_SIZE_FIRST; ActionIdx < GRID_SIZE_LAST; ActionIdx++)
    {
        QAction *Action = new QAction(qApp->translate("Menu", sgCommands[ActionIdx].MenuName), gui);
        Action->setStatusTip(qApp->translate("Status", sgCommands[ActionIdx].StatusText));
        connect(Action, SIGNAL(triggered()), gui, SLOT(gridSizeTriggered()));
        addAction(Action);
        snapGridActions[ActionIdx] = Action;
        snapGridActions[ActionIdx]->setCheckable(true);
        GridStepSizeGroup->addAction(snapGridActions[ActionIdx]);
    }

    snapGridActions[Preferences::gridSizeIndex]->setChecked(true);

    // Tools
    QAction *firstPageAct = new QAction(QIcon(":/resources/first.png"),tr("First Page"), gui);
    firstPageAct->setObjectName("firstPageAct.1");
    firstPageAct->setShortcut(QStringLiteral("Ctrl+P"));
    firstPageAct->setStatusTip(tr("Go to first page of document"));
    firstPageAct->setEnabled(false);
    lpub->actions.insert(firstPageAct->objectName(), Action(QStringLiteral("Navigation.First Page"), firstPageAct));
    connect(firstPageAct, SIGNAL(triggered()), gui, SLOT(firstPage()));

    QAction *lastPageAct = new QAction(QIcon(":/resources/last.png"),tr("Last Page"), gui);
    lastPageAct->setObjectName("lastPageAct.1");
    lastPageAct->setShortcut(QStringLiteral("Ctrl+L"));
    lastPageAct->setStatusTip(tr("Go to last page of document"));
    lastPageAct->setEnabled(false);
    lpub->actions.insert(lastPageAct->objectName(), Action(QStringLiteral("Navigation.Last Page"), lastPageAct));
    connect(lastPageAct, SIGNAL(triggered()), gui, SLOT(lastPage()));

    QAction *nextPageAct = new QAction(QIcon(":/resources/next.png"),tr("&Next Page"),gui);
    nextPageAct->setObjectName("nextPageAct.1");
    nextPageAct->setShortcut(QStringLiteral("Ctrl+N"));
    nextPageAct->setStatusTip(tr("Go to next page of document"));
    nextPageAct->setEnabled(false);
    lpub->actions.insert(nextPageAct->objectName(), Action(QStringLiteral("Navigation.Next Page"), nextPageAct));
    connect(nextPageAct, SIGNAL(triggered()), gui, SLOT(nextPage()));

    QAction *previousPageAct = new QAction(QIcon(":/resources/prev.png"),tr("&Previous Page"),gui);
    previousPageAct->setObjectName("previousPageAct.1");
    previousPageAct->setShortcut(QStringLiteral("Ctrl+E"));
    previousPageAct->setStatusTip(tr("Go to previous page of document"));
    previousPageAct->setEnabled(false);
    lpub->actions.insert(previousPageAct->objectName(), Action(QStringLiteral("Navigation.Previous Page"), previousPageAct));
    connect(previousPageAct, SIGNAL(triggered()), gui, SLOT(previousPage()));

    QAction *nextPageComboAct = new QAction(QIcon(":/resources/next.png"),tr("&Next Page"),gui);
    nextPageComboAct->setObjectName("nextPageComboAct.1");
    nextPageComboAct->setStatusTip(tr("Go to next page of document"));
    nextPageComboAct->setEnabled(false);
    lpub->actions.insert(nextPageComboAct->objectName(), Action(QStringLiteral("Navigation.Next Page Combo"), nextPageComboAct));
    connect(nextPageComboAct, SIGNAL(triggered()), gui, SLOT(nextPage()));

    QAction *previousPageComboAct = new QAction(QIcon(":/resources/prev.png"),tr("&Previous Page"),gui);
    previousPageComboAct->setObjectName("previousPageComboAct.1");
    previousPageComboAct->setStatusTip(tr("Go to previous page of document"));
    previousPageComboAct->setEnabled(false);
    lpub->actions.insert(previousPageComboAct->objectName(), Action(QStringLiteral("Navigation.Previous Page Combo"), previousPageComboAct));
    connect(previousPageComboAct, SIGNAL(triggered()), gui, SLOT(previousPage()));

    QAction *nextPageContinuousAct = new QAction(QIcon(":/resources/nextpagecontinuous.png"),tr("Continuous Next Page"),gui);
    nextPageContinuousAct->setObjectName("nextPageContinuousAct.1");
    nextPageContinuousAct->setShortcut(QStringLiteral("Ctrl+Shift+N"));
    nextPageContinuousAct->setStatusTip(tr("Continuously process next page to end of document"));
    nextPageContinuousAct->setEnabled(false);
    lpub->actions.insert(nextPageContinuousAct->objectName(), Action(QStringLiteral("Navigation.Continuous Next Page"), nextPageContinuousAct));
    connect(nextPageContinuousAct, SIGNAL(triggered()), gui, SLOT(nextPageContinuous()));

    QAction *previousPageContinuousAct = new QAction(QIcon(":/resources/prevpagecontinuous.png"),tr("Continuous Previous Page"),gui);
    previousPageContinuousAct->setObjectName("previousPageContinuousAct.1");
    previousPageContinuousAct->setShortcut(QStringLiteral("Ctrl+Shift+E"));
    previousPageContinuousAct->setStatusTip(tr("Continuously process previous page to start of document"));
    previousPageContinuousAct->setEnabled(false);
    lpub->actions.insert(previousPageContinuousAct->objectName(), Action(QStringLiteral("Navigation.Continuous Previous Page"), previousPageContinuousAct));
    connect(previousPageContinuousAct, SIGNAL(triggered()), gui, SLOT(previousPageContinuous()));

    QString pageString = "";
    setPageLineEdit = new QLineEdit(pageString,gui);
    QSize size = setPageLineEdit->sizeHint();
    size.setWidth(size.width()/3);
    setPageLineEdit->setMinimumSize(size);
    setPageLineEdit->setToolTip("Current Page");
    setPageLineEdit->setStatusTip("Enter desired page(s) in the format 'page', 'page of pages', or 'page to pages'.");
    setPageLineEdit->setEnabled(false);
    QAction *setPageLineEditResetAct = setPageLineEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
    setPageLineEditResetAct->setText(tr("Current Page Edit Reset"));
    setPageLineEditResetAct->setObjectName("setPageLineEditResetAct.1");
    setPageLineEditResetAct->setEnabled(false);
    lpub->actions.insert(setPageLineEditResetAct->objectName(), Action(QStringLiteral("Navigation.Current Page Edit Reset"), setPageLineEditResetAct));
    connect(setPageLineEdit, SIGNAL(returnPressed()), gui, SLOT(setPage()));
    connect(setPageLineEdit, SIGNAL(textEdited(const QString &)), gui, SLOT(enablePageLineReset(const QString &)));
    connect(setPageLineEditResetAct, SIGNAL(triggered()), gui, SLOT(pageLineEditReset()));

    // Configuration
    QAction *preferencesAct = new QAction(QIcon(":/resources/preferences.png"),tr("&Preferences"), gui);
    preferencesAct->setObjectName("preferencesAct.1");
    preferencesAct->setStatusTip(tr("Set your preferences for %1").arg(VER_PRODUCTNAME_STR));
    lpub->actions.insert(preferencesAct->objectName(), Action(QStringLiteral("Configuration.Preferences"), preferencesAct));
    connect(preferencesAct, SIGNAL(triggered()), gui, SLOT(preferences()));

    QAction *pageSetupAct = new QAction(QIcon(":/resources/pagesetup.png"),tr("Page Setup..."), gui);
    pageSetupAct->setObjectName("pageSetupAct.1");
    pageSetupAct->setEnabled(false);
    pageSetupAct->setStatusTip(tr("Default values for your project's pages"));
    lpub->actions.insert(pageSetupAct->objectName(), Action(QStringLiteral("Configuration.Build Instructions Setup.Page Setup"), pageSetupAct));
    connect(pageSetupAct, SIGNAL(triggered()), gui, SLOT(pageSetup()));

    QAction *assemSetupAct = new QAction(QIcon(":/resources/assemblysetup.png"),tr("Assembly Setup..."), gui);
    assemSetupAct->setObjectName("assemSetupAct.1");
    assemSetupAct->setEnabled(false);
    assemSetupAct->setStatusTip(tr("Default values for your project's assembly images"));
    lpub->actions.insert(assemSetupAct->objectName(), Action(QStringLiteral("Configuration.Build Instructions Setup.Assembly Setup"), assemSetupAct));
    connect(assemSetupAct, SIGNAL(triggered()), gui, SLOT(assemSetup()));

    QAction *pliSetupAct = new QAction(QIcon(":/resources/partslistsetup.png"),tr("Parts List Setup..."), gui);
    pliSetupAct->setObjectName("pliSetupAct.1");
    pliSetupAct->setEnabled(false);
    pliSetupAct->setStatusTip(tr("Default values for your project's parts lists"));
    lpub->actions.insert(pliSetupAct->objectName(), Action(QStringLiteral("Configuration.Build Instructions Setup.Parts List Setup"), pliSetupAct));
    connect(pliSetupAct, SIGNAL(triggered()), gui, SLOT(pliSetup()));

    QAction *bomSetupAct = new QAction(QIcon(":/resources/bomsetup.png"),tr("Bill of Materials Setup..."), gui);
    bomSetupAct->setObjectName("bomSetupAct.1");
    bomSetupAct->setEnabled(false);
    bomSetupAct->setStatusTip(tr("Default values for your project's bill of materials"));
    lpub->actions.insert(bomSetupAct->objectName(), Action(QStringLiteral("Configuration.Build Instructions Setup.Bill Of Materials Setup"), bomSetupAct));
    connect(bomSetupAct, SIGNAL(triggered()), gui, SLOT(bomSetup()));

    QAction *calloutSetupAct = new QAction(QIcon(":/resources/calloutsetup.png"),tr("Callout Setup..."), gui);
    calloutSetupAct->setObjectName("calloutSetupAct.1");
    calloutSetupAct->setEnabled(false);
    calloutSetupAct->setStatusTip(tr("Default values for your project's callouts"));
    lpub->actions.insert(calloutSetupAct->objectName(), Action(QStringLiteral("Configuration.Build Instructions Setup.Callout Setup"), calloutSetupAct));
    connect(calloutSetupAct, SIGNAL(triggered()), gui, SLOT(calloutSetup()));

    QAction *multiStepSetupAct = new QAction(QIcon(":/resources/stepgroupsetup.png"),tr("Step Group Setup..."), gui);
    multiStepSetupAct->setObjectName("multiStepSetupAct.1");
    multiStepSetupAct->setEnabled(false);
    multiStepSetupAct->setStatusTip(tr("Default values for your project's step groups"));
    lpub->actions.insert(multiStepSetupAct->objectName(), Action(QStringLiteral("Configuration.Build Instructions Setup.Step Group Setup"), multiStepSetupAct));
    connect(multiStepSetupAct, SIGNAL(triggered()), gui, SLOT(multiStepSetup()));

    QAction *subModelSetupAct = new QAction(QIcon(":/resources/submodelsetup.png"),tr("Submodel Preview Setup..."), gui);
    subModelSetupAct->setObjectName("subModelSetupAct.1");
    subModelSetupAct->setEnabled(false);
    subModelSetupAct->setStatusTip(tr("Default values for your submodel preview at first step"));
    lpub->actions.insert(subModelSetupAct->objectName(), Action(QStringLiteral("Configuration.Build Instructions Setup.Submodel Preview Setup"), subModelSetupAct));
    connect(subModelSetupAct, SIGNAL(triggered()), gui, SLOT(subModelSetup()));

    QAction *projectSetupAct = new QAction(QIcon(":/resources/projectsetup.png"),tr("Project Setup..."), gui);
    projectSetupAct->setObjectName("projectSetupAct.1");
    projectSetupAct->setEnabled(false);
    projectSetupAct->setStatusTip(tr("Default values for your project"));
    lpub->actions.insert(projectSetupAct->objectName(), Action(QStringLiteral("Configuration.Build Instructions Setup.Project Setup"), projectSetupAct));
    connect(projectSetupAct, SIGNAL(triggered()), gui, SLOT(projectSetup()));

    QAction *fadeStepsSetupAct = new QAction(QIcon(":/resources/fadestepsetup.png"),tr("Fade Steps Setup..."), gui);
    fadeStepsSetupAct->setObjectName("fadeStepsSetupAct.1");
    fadeStepsSetupAct->setEnabled(false);
    fadeStepsSetupAct->setStatusTip(tr("Fade parts in previous steps"));
    lpub->actions.insert(fadeStepsSetupAct->objectName(), Action(QStringLiteral("Configuration.Build Instructions Setup.Fade Steps Setup"), fadeStepsSetupAct));
    connect(fadeStepsSetupAct, SIGNAL(triggered()), gui, SLOT(fadeStepsSetup()));

    QAction *highlightStepSetupAct = new QAction(QIcon(":/resources/highlightstepsetup.png"),tr("Highlight Step Setup..."), gui);
    highlightStepSetupAct->setObjectName("highlightStepSetupAct.1");
    highlightStepSetupAct->setEnabled(false);
    highlightStepSetupAct->setStatusTip(tr("Highlight parts in current step"));
    lpub->actions.insert(highlightStepSetupAct->objectName(), Action(QStringLiteral("Configuration.Build Instructions Setup.Highlight Step Setup"), highlightStepSetupAct));
    connect(highlightStepSetupAct, SIGNAL(triggered()), gui, SLOT(highlightStepSetup()));

    QAction *useSystemEditorAct = new QAction(tr("Use System Editor"),gui);
    useSystemEditorAct->setObjectName("useSystemEditorAct.1");
    useSystemEditorAct->setStatusTip(tr("Enable open parameter files with %1")
                                     .arg(Preferences::systemEditor.isEmpty() ? "the system editor" :
                                                                                Preferences::systemEditor));
    useSystemEditorAct->setCheckable(true);
    useSystemEditorAct->setChecked(Preferences::useSystemEditor);
    lpub->actions.insert(useSystemEditorAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.Use System Editor"), useSystemEditorAct));
    connect(useSystemEditorAct, SIGNAL(triggered()), gui, SLOT(useSystemEditor()));

    QAction *openParameterFileFolderAct = new QAction(QIcon(":/resources/openworkingfolder.png"),tr("Open Parameter File Folder..."), gui);
    openParameterFileFolderAct->setObjectName("openParameterFileFolderAct.1");
    openParameterFileFolderAct->setStatusTip(tr("Open the folder containing common %1 parameter files").arg(VER_PRODUCTNAME_STR));
    lpub->actions.insert(openParameterFileFolderAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.Open Parameter File Folder"), openParameterFileFolderAct));
    connect(openParameterFileFolderAct, SIGNAL(triggered()), gui, SLOT(openWorkingFolder()));

    QAction *editTitleAnnotationsAct = new QAction(QIcon(":/resources/edittitleannotations.png"),tr("Part Title PLI Annotations List"), gui);
    editTitleAnnotationsAct->setObjectName("editTitleAnnotationsAct.1");
    editTitleAnnotationsAct->setStatusTip(tr("Add/Edit part title PLI part annotatons"));
    lpub->actions.insert(editTitleAnnotationsAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.Part Title PLI Annotations List"), editTitleAnnotationsAct));
    connect(editTitleAnnotationsAct, SIGNAL(triggered()), gui, SLOT(editTitleAnnotations()));

    QAction *editFreeFormAnnitationsAct = new QAction(QIcon(":/resources/editfreeformannotations.png"),tr("Freeform PLI Annotations List"), gui);
    editFreeFormAnnitationsAct->setObjectName("editFreeFormAnnitationsAct.1");
    editFreeFormAnnitationsAct->setStatusTip(tr("Add/Edit freeform PLI part annotations"));
    lpub->actions.insert(editFreeFormAnnitationsAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.Freeform PLI Annotations List"), editFreeFormAnnitationsAct));
    connect(editFreeFormAnnitationsAct, SIGNAL(triggered()), gui, SLOT(editFreeFormAnnitations()));

    QAction *editLDrawColourPartsAct = new QAction(QIcon(":/resources/editldrawcolourparts.png"),tr("LDraw Static Color Parts List"), gui);
    editLDrawColourPartsAct->setObjectName("editLDrawColourPartsAct.1");
    editLDrawColourPartsAct->setStatusTip(tr("Add/Edit the list of LDraw static color parts used to process fade and highlight steps"));
    lpub->actions.insert(editLDrawColourPartsAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.LDraw Static Color Parts List"), editLDrawColourPartsAct));
    connect(editLDrawColourPartsAct, SIGNAL(triggered()), gui, SLOT(editLDrawColourParts()));

    QAction *editPliBomSubstitutePartsAct = new QAction(QIcon(":/resources/editplisubstituteparts.png"),tr("Substitute Parts List"), gui);
    editPliBomSubstitutePartsAct->setObjectName("editPliBomSubstitutePartsAct.1");
    editPliBomSubstitutePartsAct->setStatusTip(tr("Add/Edit the list of PLI/BOM substitute parts"));
    lpub->actions.insert(editPliBomSubstitutePartsAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.Substitute Parts List"), editPliBomSubstitutePartsAct));
    connect(editPliBomSubstitutePartsAct, SIGNAL(triggered()), gui, SLOT(editPliBomSubstituteParts()));

    QAction *editExcludedPartsAct = new QAction(QIcon(":/resources/editexcludedparts.png"),tr("Part Count Excluded Parts List"), gui);
    editExcludedPartsAct->setObjectName("editExcludedPartsAct.1");
    editExcludedPartsAct->setStatusTip(tr("Add/Edit the list of part count excluded parts"));
    lpub->actions.insert(editExcludedPartsAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.Part Count Excluded Parts List"), editExcludedPartsAct));
    connect(editExcludedPartsAct, SIGNAL(triggered()), gui, SLOT(editExcludedParts()));

    QAction *editStickerPartsAct = new QAction(QIcon(":/resources/editstickerparts.png"),tr("Part Count Sticker Parts List"), gui);
    editStickerPartsAct->setObjectName("editStickerPartsAct.1");
    editStickerPartsAct->setStatusTip(tr("Add/Edit the list of part count sticker parts"));
    lpub->actions.insert(editStickerPartsAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.Part Count Sticker Parts List"), editStickerPartsAct));
    connect(editStickerPartsAct, SIGNAL(triggered()), gui, SLOT(editStickerParts()));

    QAction *editLdrawIniFileAct = new QAction(QIcon(":/resources/editinifile.png"),tr("LDraw INI Search Directories"), gui);
    editLdrawIniFileAct->setObjectName("editLdrawIniFileAct.1");
    editLdrawIniFileAct->setStatusTip(tr("Add/Edit LDraw.ini search directory entries"));
    lpub->actions.insert(editLdrawIniFileAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.LDraw INI Search Directories"), editLdrawIniFileAct));
    connect(editLdrawIniFileAct, SIGNAL(triggered()), gui, SLOT(editLdrawIniFile()));

    QAction *editLdgliteIniAct = new QAction(QIcon(":/resources/editldgliteconf.png"),tr("LDGLite INI Configuration File"), gui);
    editLdgliteIniAct->setObjectName("editLdgliteIniAct.1");
    editLdgliteIniAct->setStatusTip(tr("Edit LDGLite INI configuration file"));
    lpub->actions.insert(editLdgliteIniAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.LDGLite INI Configuration File"), editLdgliteIniAct));
    connect(editLdgliteIniAct, SIGNAL(triggered()), gui, SLOT(editLdgliteIni()));

    QAction *editNativePOVIniAct = new QAction(QIcon(":/resources/LPub32.png"),tr("Native POV file Generation Configuration File"), gui);
    editNativePOVIniAct->setObjectName("editNativePOVIniAct.1");
    editNativePOVIniAct->setStatusTip(tr("Edit Native POV file generation configuration file"));
    lpub->actions.insert(editNativePOVIniAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.Native POV File Generation Configuration File"), editNativePOVIniAct));
    connect(editNativePOVIniAct, SIGNAL(triggered()), gui, SLOT(editNativePovIni()));

    QAction *editLdviewIniAct = new QAction(QIcon(":/resources/editldviewconf.png"),tr("LDView INI Configuration File"), gui);
    editLdviewIniAct->setObjectName("editLdviewIniAct.1");
    editLdviewIniAct->setStatusTip(tr("Edit LDView INI configuration file"));
    lpub->actions.insert(editLdviewIniAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.LDView INI Configuration File"), editLdviewIniAct));
    connect(editLdviewIniAct, SIGNAL(triggered()), gui, SLOT(editLdviewIni()));

    QAction *editLPub3DIniFileAct = new QAction(QIcon(":/resources/editsetting.png"),tr("%1 Configuration File").arg(VER_PRODUCTNAME_STR), gui);
    editLPub3DIniFileAct->setObjectName("editLPub3DIniFileAct.1");
    editLPub3DIniFileAct->setStatusTip(tr("Edit %1 application configuration settings file").arg(VER_PRODUCTNAME_STR));
    lpub->actions.insert(editLPub3DIniFileAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.%1 Configuration File").arg(VER_PRODUCTNAME_STR), editLPub3DIniFileAct));
    connect(editLPub3DIniFileAct, SIGNAL(triggered()), gui, SLOT(editLPub3DIniFile()));

    QAction *editLdviewPovIniAct = new QAction(QIcon(":/resources/editldviewconf.png"),tr("LDView POV File Generation Configuration File"), gui);
    editLdviewPovIniAct->setObjectName("editLdviewPovIniAct.1");
    editLdviewPovIniAct->setStatusTip(tr("Edit LDView POV file generation configuration file"));
    lpub->actions.insert(editLdviewPovIniAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.LDView POV File Generation Configuration File"), editLdviewPovIniAct));
    connect(editLdviewPovIniAct, SIGNAL(triggered()), gui, SLOT(editLdviewPovIni()));

    QAction *editBlenderPreferencesAct = new QAction(QIcon(":/resources/blendericon.png"),tr("Blender Render Preferences"), gui);
    editBlenderPreferencesAct->setObjectName("editBlenderPreferencesAct.1");
    editBlenderPreferencesAct->setStatusTip(tr("Add/Edit Blender LDraw import and image render preferences"));
    lpub->actions.insert(editBlenderPreferencesAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.Blender Render Preferences"), editBlenderPreferencesAct));
    connect(editBlenderPreferencesAct, SIGNAL(triggered()), gui, SLOT(editBlenderParameters()));

    QAction *editBlenderParametersAct = new QAction(QIcon(":/resources/blendericon.png"),tr("Blender LDraw Parameters"), gui);
    editBlenderParametersAct->setObjectName("editBlenderParametersAct.1");
    editBlenderParametersAct->setStatusTip(tr("Add/Edit LDraw Blender LDraw LGEO colours, sloped bricks and lighted bricks, reference"));
    lpub->actions.insert(editBlenderParametersAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.Blender LDraw Parameters"), editBlenderParametersAct));
    connect(editBlenderParametersAct, SIGNAL(triggered()), gui, SLOT(editBlenderParameters()));

    QAction *editPovrayIniAct = new QAction(QIcon(":/resources/editpovrayconf.png"),tr("POV-Ray INI Configuration File"), gui);
    editPovrayIniAct->setObjectName("editPovrayIniAct.1");
    editPovrayIniAct->setStatusTip(tr("Edit Raytracer (POV-Ray) INI configuration file"));
    lpub->actions.insert(editPovrayIniAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.POV-Ray INI Configuration File"), editPovrayIniAct));
    connect(editPovrayIniAct, SIGNAL(triggered()), gui, SLOT(editPovrayIni()));

    QAction *editPovrayConfAct = new QAction(QIcon(":/resources/editpovrayconf.png"),tr("POV-Ray File Access Configuration File"), gui);
    editPovrayConfAct->setObjectName("editPovrayConfAct.1");
    editPovrayConfAct->setStatusTip(tr("Edit Raytracer (POV-Ray) file access configuration file"));
    lpub->actions.insert(editPovrayConfAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.POV-Ray File Access Configuration File"), editPovrayConfAct));
    connect(editPovrayConfAct, SIGNAL(triggered()), gui, SLOT(editPovrayConf()));

    QAction *editAnnotationStyleAct = new QAction(QIcon(":/resources/editstyleref.png"),tr("Part Annotation Style Reference"), gui);
    editAnnotationStyleAct->setObjectName("editAnnotationStyleAct.1");
    editAnnotationStyleAct->setStatusTip(tr("Add/Edit LDraw Design ID, Part Annotation Style, Part Category reference"));
    lpub->actions.insert(editAnnotationStyleAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.Part Annotation Style Reference"), editAnnotationStyleAct));
    connect(editAnnotationStyleAct, SIGNAL(triggered()), gui, SLOT(editAnnotationStyle()));

    QAction *editLD2BLCodesXRefAct = new QAction(QIcon(":/resources/editld2blxref.png"),tr("LDraw to Bricklink Design ID Reference"), gui);
    editLD2BLCodesXRefAct->setObjectName("editLD2BLCodesXRefAct.1");
    editLD2BLCodesXRefAct->setStatusTip(tr("Add/Edit LDraw to Bricklink Design ID reference"));
    lpub->actions.insert(editLD2BLCodesXRefAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.LDraw To Bricklink Design ID Reference"), editLD2BLCodesXRefAct));
    connect(editLD2BLCodesXRefAct, SIGNAL(triggered()), gui, SLOT(editLD2BLCodesXRef()));

    QAction *editLD2BLColorsXRefAct = new QAction(QIcon(":/resources/editld2blxref.png"),tr("LDraw to Bricklink Color Reference"), gui);
    editLD2BLColorsXRefAct->setObjectName("editLD2BLColorsXRefAct.1");
    editLD2BLColorsXRefAct->setStatusTip(tr("Add/Edit LDraw LDConfig to Bricklink Color ID reference"));
    lpub->actions.insert(editLD2BLColorsXRefAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.LDraw To Bricklink Color Reference"), editLD2BLColorsXRefAct));
    connect(editLD2BLColorsXRefAct, SIGNAL(triggered()), gui, SLOT(editLD2BLColorsXRef()));

    QAction *editLD2RBCodesXRefAct = new QAction(QIcon(":/resources/editld2rbxref.png"),tr("LDraw to Rebrickable Design ID Reference"), gui);
    editLD2RBCodesXRefAct->setObjectName("editLD2RBCodesXRefAct.1");
    editLD2RBCodesXRefAct->setStatusTip(tr("Add/Edit LDraw to Rebrickable Design ID reference"));
    lpub->actions.insert(editLD2RBCodesXRefAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.LDraw To Rebrickable Design ID Reference"), editLD2RBCodesXRefAct));
    connect(editLD2RBCodesXRefAct, SIGNAL(triggered()), gui, SLOT(editLD2RBCodesXRef()));

    QAction *editLD2RBColorsXRefAct = new QAction(QIcon(":/resources/editld2rbxref.png"),tr("LDraw to Rebrickable Color Reference"), gui);
    editLD2RBColorsXRefAct->setObjectName("editLD2RBColorsXRefAct.1");
    editLD2RBColorsXRefAct->setStatusTip(tr("Add/Edit LDraw LDConfig to Rebrickable Color ID reference"));
    lpub->actions.insert(editLD2RBColorsXRefAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.LDraw To Rebrickable Color Reference"), editLD2RBColorsXRefAct));
    connect(editLD2RBColorsXRefAct, SIGNAL(triggered()), gui, SLOT(editLD2RBColorsXRef()));

    QAction *editBLColorsAct = new QAction(QIcon(":/resources/editld2blxref.png"),tr("Edit Bricklink Color Reference"), gui);
    editBLColorsAct->setObjectName("editBLColorsAct.1");
    editBLColorsAct->setStatusTip(tr("Add/Edit Bricklink Color ID reference"));
    lpub->actions.insert(editBLColorsAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.Edit Bricklink Color Reference"), editBLColorsAct));
    connect(editBLColorsAct, SIGNAL(triggered()), gui, SLOT(editBLColors()));

    QAction *editBLCodesAct = new QAction(QIcon(":/resources/editld2blxref.png"),tr("Bricklink Codes Reference"), gui);
    editBLCodesAct->setObjectName("editBLCodesAct.1");
    editBLCodesAct->setStatusTip(tr("Add/Edit Bricklink Item No, Color Name, LEGO Element reference"));
    lpub->actions.insert(editBLCodesAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.Bricklink Codes Reference"), editBLCodesAct));
    connect(editBLCodesAct, SIGNAL(triggered()), gui, SLOT(editBLCodes()));

    QAction *editPliControlFileAct = new QAction(QIcon(":/resources/editldraw.png"),tr("PLI Control Parts File"), gui);
    editPliControlFileAct->setObjectName("editPliControlFileAct.1");
    editPliControlFileAct->setStatusTip(tr("Edit the PLI control parts file in the detached command editor"));
    lpub->actions.insert(editPliControlFileAct->objectName(), Action(QStringLiteral("Configuration.Edit Parameter Files.PLI Parts Control File"), editPliControlFileAct));
    connect(editPliControlFileAct, SIGNAL(triggered()), gui, SLOT(editPliControlFile()));

    QAction *editModelFileAct = new QAction(QIcon(":/resources/editldraw.png"),tr("Current Model File"), gui);
    editModelFileAct->setObjectName("editModelFileAct.1");
    editModelFileAct->setStatusTip(tr("Edit current model file with %1")
                                   .arg(Preferences::useSystemEditor ?
                                            Preferences::systemEditor.isEmpty() ? "the system editor" :
                                                                                  Preferences::systemEditor : "detached LDraw Editor"));
    lpub->actions.insert(editModelFileAct->objectName(), Action(QStringLiteral("Configuration.Current Model File"), editModelFileAct));
    connect(editModelFileAct, SIGNAL(triggered()), gui, SLOT(editModelFile()));

    QAction *generateCustomColourPartsAct = new QAction(QIcon(":/resources/generatecolourparts.png"),tr("Generate Static Color Parts List"), gui);
    generateCustomColourPartsAct->setObjectName("generateCustomColourPartsAct.1");
    generateCustomColourPartsAct->setStatusTip(tr("Generate list of all static coloured parts for use with fade previous steps"));
    lpub->actions.insert(generateCustomColourPartsAct->objectName(), Action(QStringLiteral("Configuration.Generate Static Color Parts List"), generateCustomColourPartsAct));
    connect(generateCustomColourPartsAct, SIGNAL(triggered()), gui, SLOT(generateCustomColourPartsList()));

    QAction *openWithSetupAct = new QAction(QIcon(":/resources/openwithsetup.png"), tr("Open With S&etup..."), gui);
    openWithSetupAct->setObjectName("openWithSetupAct.1");
    openWithSetupAct->setStatusTip(tr("Setup 'Open With' applications."));
    lpub->actions.insert(openWithSetupAct->objectName(), Action(QStringLiteral("Configuration.Open With Setup"), openWithSetupAct));
    connect(openWithSetupAct, SIGNAL(triggered()), gui, SLOT(openWithSetup()));

    QAction *ldrawSearchDirectoriesAct = new QAction(QIcon(":/resources/searchdirectories.png"),tr("LDraw Search Directories..."), gui);
    ldrawSearchDirectoriesAct->setObjectName("ldrawSearchDirectoriesAct.1");
    ldrawSearchDirectoriesAct->setStatusTip(tr("Manage LDraw search directories"));
    lpub->actions.insert(ldrawSearchDirectoriesAct->objectName(), Action(QStringLiteral("Configuration.LDraw Search Directories"), ldrawSearchDirectoriesAct));
    connect(ldrawSearchDirectoriesAct, SIGNAL(triggered()), gui, SLOT(ldrawSearchDirectories()));

    QAction *clearPLICacheAct = new QAction(QIcon(":/resources/clearplicache.png"),tr("Parts Image Cache"), gui);
    clearPLICacheAct->setObjectName("clearPLICacheAct.1");
    clearPLICacheAct->setShortcut(QStringLiteral("Alt+R"));
    clearPLICacheAct->setStatusTip(tr("Reset the parts list image cache"));
    lpub->actions.insert(clearPLICacheAct->objectName(), Action(QStringLiteral("Configuration.Reset Cache.Parts Image Cache"), clearPLICacheAct));
    connect(clearPLICacheAct, SIGNAL(triggered()), gui, SLOT(clearPLICache()));

    QAction *clearBOMCacheAct = new QAction(QIcon(":/resources/clearbomcache.png"),tr("Bill Of Material Image Cache"), gui);
    clearBOMCacheAct->setObjectName("clearBOMCacheAct.1");
    clearBOMCacheAct->setShortcut(QStringLiteral("Alt+O"));
    clearBOMCacheAct->setStatusTip(tr("Reset the bill of material list image cache"));
    lpub->actions.insert(clearBOMCacheAct->objectName(), Action(QStringLiteral("Configuration.Reset Cache.Bill Of Material Image Cache"), clearBOMCacheAct));
    connect(clearBOMCacheAct, SIGNAL(triggered()), gui, SLOT(clearBOMCache()));

    QAction *clearCSICacheAct = new QAction(QIcon(":/resources/clearcsicache.png"),tr("Assembly Image Cache"), gui);
    clearCSICacheAct->setObjectName("clearCSICacheAct.1");
    clearCSICacheAct->setShortcut(QStringLiteral("Alt+S"));
    clearCSICacheAct->setStatusTip(tr("Reset the assembly image cache"));
    lpub->actions.insert(clearCSICacheAct->objectName(), Action(QStringLiteral("Configuration.Reset Cache.Assembly Image Cache"), clearCSICacheAct));
    connect(clearCSICacheAct, SIGNAL(triggered()), gui, SLOT(clearCSICache()));

    QAction *clearSMICacheAct = new QAction(QIcon(":/resources/clearsmicache.png"),tr("Submodel Image Cache"), gui);
    clearSMICacheAct->setObjectName("clearSMICacheAct.1");
    clearSMICacheAct->setShortcut(QStringLiteral("Alt+E"));
    clearSMICacheAct->setStatusTip(tr("Reset the submodel image cache"));
    lpub->actions.insert(clearSMICacheAct->objectName(), Action(QStringLiteral("Configuration.Reset Cache.Submodel Image Cache"), clearSMICacheAct));
    connect(clearSMICacheAct, SIGNAL(triggered()), gui, SLOT(clearSMICache()));

    QAction *clearTempCacheAct = new QAction(QIcon(":/resources/cleartempcache.png"),tr("Temporary LDraw File Cache"), gui);
    clearTempCacheAct->setObjectName("clearTempCacheAct.1");
    clearTempCacheAct->setShortcut(QStringLiteral("Alt+T"));
    clearTempCacheAct->setStatusTip(tr("Reset the temporary LDraw working files cache"));
    lpub->actions.insert(clearTempCacheAct->objectName(), Action(QStringLiteral("Configuration.Reset Cache.Temporary LDraw File Cache"), clearTempCacheAct));
    connect(clearTempCacheAct, SIGNAL(triggered()), gui, SLOT(clearTempCache()));

    QAction *clearAllCachesAct = new QAction(QIcon(":/resources/clearallfilecaches.png"),tr("All Image And LDraw File Caches"), gui);
    clearAllCachesAct->setObjectName("clearAllCachesAct.1");
    clearAllCachesAct->setShortcut(QStringLiteral("Alt+M"));
    clearAllCachesAct->setStatusTip(tr("Reset the parts, bill of material, assembly, submodel image and the temporary LDraw file caches"));
    lpub->actions.insert(clearAllCachesAct->objectName(), Action(QStringLiteral("Configuration.Reset Cache.All Image And LDraw File Caches"), clearAllCachesAct));
    connect(clearAllCachesAct, SIGNAL(triggered()), gui, SLOT(clearAllCaches()));

    QAction *clearCustomPartCacheAct = new QAction(QIcon(":/resources/clearcustompartcache.png"),tr("Custom LDraw Files Cache"), gui);
    clearCustomPartCacheAct->setObjectName("clearCustomPartCacheAct.1");
    clearCustomPartCacheAct->setShortcut(QStringLiteral("Alt+C"));
    clearCustomPartCacheAct->setStatusTip(tr("Reset fade and highlight LDraw colour part file cache"));
    lpub->actions.insert(clearCustomPartCacheAct->objectName(), Action(QStringLiteral("Configuration.Reset Cache.Custom LDraw Files Cache"), clearCustomPartCacheAct));
    connect(clearCustomPartCacheAct, SIGNAL(triggered()), gui, SLOT(clearCustomPartCache()));

    QAction *archivePartsOnDemandAct = new QAction(QIcon(":/resources/archivefilesondemand.png"),tr("Archive Unofficial Parts"), gui);
    archivePartsOnDemandAct->setObjectName("archivePartsOnDemandAct.1");
    archivePartsOnDemandAct->setStatusTip(tr("Archive unofficial parts from LDraw search directories"));
    archivePartsOnDemandAct->setShortcut(QStringLiteral("Alt+Y"));
    lpub->actions.insert(archivePartsOnDemandAct->objectName(), Action(QStringLiteral("Configuration.Archive Unofficial Parts"), archivePartsOnDemandAct));
    connect(archivePartsOnDemandAct, SIGNAL(triggered()), gui, SLOT(archivePartsOnDemand()));

    QAction *archivePartsOnLaunchAct = new QAction(QIcon(":/resources/archivefilesonlaunch.png"),tr("Archive Unofficial Parts On Launch"), gui);
    archivePartsOnLaunchAct->setObjectName("archivePartsOnLaunchAct.1");
    archivePartsOnLaunchAct->setStatusTip(tr("Automatically archive unofficial parts from LDraw search directories on next application launch"));
    archivePartsOnLaunchAct->setCheckable(true);
    archivePartsOnLaunchAct->setChecked(Preferences::archivePartsOnLaunch);
    lpub->actions.insert(archivePartsOnLaunchAct->objectName(), Action(QStringLiteral("Configuration.Archive Unofficial Parts On Launch"), archivePartsOnLaunchAct));
    connect(archivePartsOnLaunchAct, SIGNAL(triggered()), gui, SLOT(archivePartsOnLaunch()));

    QAction *refreshLDrawUnoffPartsAct = new QAction(QIcon(":/resources/refreshunoffarchive.png"),tr("Refresh LDraw Unofficial Parts"), gui);
    refreshLDrawUnoffPartsAct->setObjectName("refreshLDrawUnoffPartsAct.1");
    refreshLDrawUnoffPartsAct->setStatusTip(tr("Download and replace LDraw Unofficial parts archive file in User data - restart required"));
    refreshLDrawUnoffPartsAct->setEnabled(Preferences::usingDefaultLibrary);
    lpub->actions.insert(refreshLDrawUnoffPartsAct->objectName(), Action(QStringLiteral("Configuration.Refresh LDraw Unofficial Parts"), refreshLDrawUnoffPartsAct));
    connect(refreshLDrawUnoffPartsAct, SIGNAL(triggered()), gui, SLOT(refreshLDrawUnoffParts()));

    QAction *refreshLDrawOfficialPartsAct = new QAction(QIcon(":/resources/refreshoffarchive.png"),tr("Refresh LDraw Official Parts"), gui);
    refreshLDrawOfficialPartsAct->setObjectName("refreshLDrawOfficialPartsAct.1");
    refreshLDrawOfficialPartsAct->setStatusTip(tr("Download and replace LDraw Official parts archive file in User data - restart required"));
    refreshLDrawUnoffPartsAct->setEnabled(Preferences::usingDefaultLibrary);
    lpub->actions.insert(refreshLDrawOfficialPartsAct->objectName(), Action(QStringLiteral("Configuration.Refresh LDraw Official Parts"), refreshLDrawOfficialPartsAct));
    connect(refreshLDrawOfficialPartsAct, SIGNAL(triggered()), gui, SLOT(refreshLDrawOfficialParts()));

    // Help
    QAction *aboutAct = new QAction(QIcon(":/resources/LPub32.png"),tr("&About %1...").arg(VER_PRODUCTNAME_STR), gui);
    aboutAct->setObjectName("aboutAct.1");
	aboutAct->setShortcut(QStringLiteral("Ctrl+Alt+V"));
    aboutAct->setStatusTip(tr("Display version, system and build information"));
    lpub->actions.insert(aboutAct->objectName(), Action(QStringLiteral("Help.About"), aboutAct));
    connect(aboutAct, SIGNAL(triggered()), gui, SLOT(aboutDialog()));

    QAction *visitHomepageAct = new QAction(QIcon(":/resources/homepage.png"),tr("Visit Homepage..."), gui);
    visitHomepageAct->setObjectName("visitHomepageAct.1");
    visitHomepageAct->setStatusTip(tr("Visit the %1 home website.").arg(VER_PRODUCTNAME_STR));
    lpub->actions.insert(visitHomepageAct->objectName(), Action(QStringLiteral("Help.Visit Homepage"), visitHomepageAct));
    connect(visitHomepageAct, SIGNAL(triggered()), gui, SLOT(visitHomepage()));

    QAction *openTicketAct = new QAction(QIcon(":/resources/bug.png"),tr("Open Issue or Request..."), gui);
    openTicketAct->setObjectName("openTicketAct.1");
    openTicketAct->setStatusTip(tr("Create a GitHub ticket to report an issue or requst an enhancement."));
    lpub->actions.insert(openTicketAct->objectName(), Action(QStringLiteral("Help.Open A Ticket"), openTicketAct));
    connect(openTicketAct, SIGNAL(triggered()), gui, SLOT(openTicket()));

    // Begin Jaco's code

    QAction *onlineManualAct = new QAction(QIcon(":/resources/onlinemanual.png"),tr("&Online Manual..."), gui);
    onlineManualAct->setObjectName("onlineManualAct.1");
    onlineManualAct->setStatusTip(tr("Visit the Online Manual Website."));
    lpub->actions.insert(onlineManualAct->objectName(), Action(QStringLiteral("Help.Online Manual"), onlineManualAct));
    connect(onlineManualAct, SIGNAL(triggered()), gui, SLOT(onlineManual()));

    // End Jaco's code
    QIcon commandsDialogIcon;
    if (Preferences::darkTheme) {
        commandsDialogIcon.addFile(":/resources/command_dark32.png");
        commandsDialogIcon.addFile(":/resources/command_dark16.png");
    } else {
        commandsDialogIcon.addFile(":/resources/command32.png");
        commandsDialogIcon.addFile(":/resources/command16.png");
    }
    QAction *commandsDialogAct = new QAction(commandsDialogIcon,tr("Manage &LPub Metacommands..."), gui);
    commandsDialogAct->setObjectName("commandsDialogAct.1");
    commandsDialogAct->setStatusTip(tr("View LPub meta commands and customize command descriptions"));
    commandsDialogAct->setShortcut(QStringLiteral("Ctrl+K"));
    lpub->actions.insert(commandsDialogAct->objectName(), Action(QStringLiteral("Help.Manage LPub Metacommands"), commandsDialogAct));
    connect(commandsDialogAct, SIGNAL(triggered()), gui, SLOT(commandsDialog()));

    QAction *exportMetaCommandsAct = new QAction(QIcon(":/resources/savemetacommands.png"),tr("&Export LPub Metacommands..."), gui);
    exportMetaCommandsAct->setObjectName("exportMetaCommandsAct.1");
    exportMetaCommandsAct->setStatusTip(tr("Export a list of the LPub meta commands to a text file"));
    lpub->actions.insert(exportMetaCommandsAct->objectName(), Action(QStringLiteral("Help.Export LPub Metacommands"), exportMetaCommandsAct));
    connect(exportMetaCommandsAct, SIGNAL(triggered()), gui, SLOT(exportMetaCommands()));

    QAction *updateAppAct = new QAction(QIcon(":/resources/softwareupdate.png"),tr("Check For &Updates..."), gui);
    updateAppAct->setObjectName("updateAppAct.1");
    updateAppAct->setStatusTip(tr("Check if a newer version of  %1 is available for download").arg(VER_PRODUCTNAME_STR));
    lpub->actions.insert(updateAppAct->objectName(), Action(QStringLiteral("Help.Check For Updates"), updateAppAct));
    connect(updateAppAct, SIGNAL(triggered()), gui, SLOT(updateCheck()));

    QAction *viewLogAct = new QAction(QIcon(":/resources/viewlog.png"),tr("View Runtime Log..."), gui);
    viewLogAct->setObjectName("viewLogAct.1");
    viewLogAct->setShortcut(QStringLiteral("Alt+L"));
    viewLogAct->setStatusTip(tr("View %1 log").arg(VER_PRODUCTNAME_STR));
    lpub->actions.insert(viewLogAct->objectName(), Action(QStringLiteral("Help.View Runtime Log"), viewLogAct));
    connect(viewLogAct, SIGNAL(triggered()), gui, SLOT(viewLog()));

    if (Preferences::modeGUI) {

        // context menu actions

        QMenu *contextMenu = new QMenu(gui);
        QString name = tr(DEF_ITEM);

        // page context menu actions

        QAction *calloutAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/convertcallout.png"),tr("Convert To Callout"));
        calloutAction->setObjectName("calloutAction.1");
        calloutAction->setStatusTip(tr("Convert a submodel build steps to a model image placed next to where it is added in the build instructions"));
        calloutAction->setWhatsThis(tr("Convert to Callout:\n"
                                       "  A callout shows how to build these steps in a picture next\n"
                                       "  to where it is added in the build instructions.\n"));
        lpub->actions.insert(calloutAction->objectName(), Action(QStringLiteral("PageContext.Convert To Callout"), calloutAction));

        QAction *calloutNoPointAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/convertcalloutwithoutpointer.png"),tr("Convert To Pointerless Callout"));
        calloutNoPointAction->setObjectName("calloutNoPointAction.1");
        calloutNoPointAction->setStatusTip(tr("A callout without pointer that shows build steps in a model image next to where it is added in the build instructions"));
        calloutNoPointAction->setWhatsThis(tr("Convert to Pointerless Callout:\n"
                                              "  A callout without pointer shows how to build these steps in a\n"
                                              "  picture next to where it is added in the build instructions.\n"));
        lpub->actions.insert(calloutNoPointAction->objectName(), Action(QStringLiteral("PageContext.Convert To Pointerless Callout"), calloutNoPointAction));

        QAction *assembledAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/addassembledimage.png"),tr("Add Assembled Image To Parent Page"));
        assembledAction->setObjectName("assembledAction.1");
        assembledAction->setStatusTip(QMessageBox::tr("A callout like image is added to the page where this submodel is added to the build instructions"));
        assembledAction->setWhatsThis(tr("Add Assembled Image to Parent Page\n"
                                         "  A callout like image is added to the page where this submodel\n"
                                         "  is added in the build instructions.\n"));
        lpub->actions.insert(assembledAction->objectName(), Action(QStringLiteral("PageContext.Add Assembled Image To Parent Page"), assembledAction));

        QAction *ignoreAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/ignoresubmodel.png"),tr("Ignore This Submodel"));
        ignoreAction->setObjectName("ignoreAction.1");
        ignoreAction->setStatusTip(tr("The submodel steps are not displayed in the build instructions"));
        ignoreAction->setWhatsThis(tr("Ignore This Submodel:\n"
                                      "  Stops these steps from showing up in your build instructions.\n"));
        lpub->actions.insert(ignoreAction->objectName(), Action(QStringLiteral("PageContext.Ignore This Submodel"), ignoreAction));

        QAction *partAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/treataspart.png"),tr("Treat As Part"));
        partAction->setObjectName("partAction.1");
        partAction->setStatusTip(tr("The submodel steps are removed and it is displayed as a part in the parent step's part list image"));
        partAction->setWhatsThis(tr("Treat As Part:\n"
                                    "  Treating this submodel as a part means these steps go away,\n"
                                    "  and the submodel is displayed as a part in the parent step's\n"
                                    "  part list image.\n"));
        lpub->actions.insert(partAction->objectName(), Action(QStringLiteral("PageContext.Treat As Part"), partAction));

        QAction *sizeAndOrientationAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/pagesizeandorientation.png"),tr("Change Page Size Or Orientation..."));
        sizeAndOrientationAction->setObjectName("sizeAndOrientationAction.1");
        sizeAndOrientationAction->setStatusTip(tr("Change the page size and orientation"));
        sizeAndOrientationAction->setWhatsThis(tr("Change the page size and orientation:\n"
                                                  "  You can change the page size using common size designations\n"
                                                  "  like A4, Legal etc...\n"
                                                  "  You can also change the page orientation from portrait to\n"
                                                  "  landscape.\n"));
        lpub->actions.insert(sizeAndOrientationAction->objectName(), Action(QStringLiteral("PageContext.Change Size Or Orientation"), sizeAndOrientationAction));

        // assembly context menu actions

        QAction *movePrevAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/addprevious.png"),tr("Add to Previous Row"));
        movePrevAction->setObjectName("movePrevAction.1");
        movePrevAction->setStatusTip(tr("Move this step to the previous row"));
        movePrevAction->setWhatsThis(tr("Add to Previous Row:\n"
                                        "  You can move this step to the previous row.\n"));
        lpub->actions.insert(movePrevAction->objectName(), Action(QStringLiteral("AssemblyContext.Add to Previous Row"), movePrevAction));

        QAction *moveNextAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/addnext.png"),tr("Add to Next Row"));
        moveNextAction->setObjectName("moveNextAction.1");
        moveNextAction->setStatusTip(tr("Remove this step from its current column, and put it in the row above"));
        moveNextAction->setWhatsThis(tr("Add to Next Row:\n"
                                        "  You can remove this step from its current column,\n"
                                        "  and put it in the row above.\n"));
        lpub->actions.insert(moveNextAction->objectName(), Action(QStringLiteral("AssemblyContext.Add to Next Row"), moveNextAction));

        QAction *addDividerAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/divider.png"),tr("Add Divider"));
        addDividerAction->setObjectName("addDividerAction.1");
        addDividerAction->setStatusTip(tr("Place a divider before or after the current step"));
        addDividerAction->setWhatsThis(tr("Add Divider:\n"
                                          "  Before step - You can place a divider to the left of this step\n"
                                          "  After Step - You can put the step(s) after this into a new row.\n"));
        lpub->actions.insert(addDividerAction->objectName(), Action(QStringLiteral("AssemblyContext.Add Divider"), addDividerAction));

        QAction *addCsiAnnoAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/addpartannotation.png"),tr("Add Part Annotations"));
        addCsiAnnoAction->setObjectName("addCsiAnnoAction.1");
        addCsiAnnoAction->setStatusTip(tr("Add part annotations to the model assembly"));
        addCsiAnnoAction->setWhatsThis(tr("Add Part Annotations:\n"
                                          "  You can add part annotations to the model assembly.\n"));
        lpub->actions.insert(addCsiAnnoAction->objectName(), Action(QStringLiteral("AssemblyContext.Add Part Annotations"), addCsiAnnoAction));

        QAction *refreshCsiAnnoAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/reloadpartannotation.png"),tr("Reload Part Annotations"));
        refreshCsiAnnoAction->setObjectName("refreshCsiAnnoAction.1");
        refreshCsiAnnoAction->setStatusTip(tr("Reload part annotations"));
        refreshCsiAnnoAction->setWhatsThis(tr("Reload Part Annotations:\n"
                                              "  You can reload part annotations.\n"));
        lpub->actions.insert(refreshCsiAnnoAction->objectName(), Action(QStringLiteral("AssemblyContext.Reload Part Annotations"), refreshCsiAnnoAction));

        QAction *showCsiAnnoAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/hidepartannotation.png"),tr("Show Hidden Part Annotations"));
        showCsiAnnoAction->setObjectName("showCsiAnnoAction.1");
        showCsiAnnoAction->setStatusTip(tr("Show hidden part annotations"));
        showCsiAnnoAction->setWhatsThis(tr("Show Hidden Part Annotations:\n"
                                           "  You can show hidden part annotations.\n"));
        lpub->actions.insert(showCsiAnnoAction->objectName(), Action(QStringLiteral("AssemblyContext.Show Hidden Part Annotations"), showCsiAnnoAction));

        QAction *insertRotateIconAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/rotateicon.png"),tr("Insert Rotate Icon"));
        insertRotateIconAction->setObjectName("insertRotateIconAction.1");
        insertRotateIconAction->setStatusTip(tr("Insert a rotate image to the model assembly..."));
        insertRotateIconAction->setWhatsThis(tr("Insert Rotate Icon:\n"
                                                "  You can insert a rotate image to the model assembly.\n"));
        lpub->actions.insert(insertRotateIconAction->objectName(), Action(QStringLiteral("AssemblyContext.Insert Rotate Icon"), insertRotateIconAction));

        QAction *addPagePointerAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/addpointer.png"),tr("Place Page Pointer"));
        addPagePointerAction->setObjectName("addPagePointerAction.1");
        addPagePointerAction->setStatusTip(tr("Add pointer from the page to this assembly image"));
        addPagePointerAction->setWhatsThis(tr("Place Page Pointer:\n"
                                              "  You can add pointer from the page to this assembly image.\n"));
        lpub->actions.insert(addPagePointerAction->objectName(), Action(QStringLiteral("AssemblyContext.Place Page Pointer"), addPagePointerAction));

        QAction *addDividerPointerAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/adddividerpointer.png"),tr("Place Divider Pointer"));
        addDividerPointerAction->setObjectName("addDividerPointerAction.1");
        addDividerPointerAction->setStatusTip(tr("Add pointer from the step divider to this assembly image"));
        addDividerPointerAction->setWhatsThis(tr("Place Divider Pointer:\n"
                                                 "  You can add a pointer from the step divider to this\n"
                                                 "  assembly image.\n"));
        lpub->actions.insert(addDividerPointerAction->objectName(), Action(QStringLiteral("AssemblyContext.Place Divider Pointer"), addDividerPointerAction));

        QAction *noStepAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/display.png"),tr("Hide This Step"));
        noStepAction->setObjectName("noStepAction.1");
        noStepAction->setStatusTip(tr("Do not show this step"));
        noStepAction->setWhatsThis(tr("Do Not Show This Step:\n"
                                      "  You can choose to not display current step.\n"));
        lpub->actions.insert(noStepAction->objectName(), Action(QStringLiteral("AssemblyContext.Do Not Show This Step"), noStepAction));

        QAction *viewCSIFileAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/editldraw.png"),tr("View Step Assembly File"));
        viewCSIFileAction->setObjectName("viewCSIFileAction.1");
        viewCSIFileAction->setStatusTip(tr("View the current LDraw model assembly file in read-only mode"));
        viewCSIFileAction->setWhatsThis(tr("View Step Assembly File:\n"
                                           "  You can view the current model assembly file in\n"
                                           "  read-only mode.\n"));
        lpub->actions.insert(viewCSIFileAction->objectName(), Action(QStringLiteral("AssemblyContext.Add Part Annotations"), viewCSIFileAction));

#ifdef QT_DEBUG_MODE
        QAction *view3DViewerFileAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/editldraw.png"),tr("View Step Assembly Visual Editor File"));
        view3DViewerFileAction->setObjectName("view3DViewerFileAction.1");
        view3DViewerFileAction->setStatusTip(tr("View the current LDraw Visual Editor file"));
        view3DViewerFileAction->setWhatsThis(tr("View Step Assembly Visual Editor File:\n"
                                                "  You can view the current visual editor model assembly file\n"
                                                "  in read-only mode.\n"));
        lpub->actions.insert(view3DViewerFileAction->objectName(), Action(QStringLiteral("AssemblyContext.View Step Assembly Visual Editor File"), view3DViewerFileAction));
#endif

        // bom context menu items

        QAction *splitBomAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/splitbom.png"),tr("Split Bill Of Materials"));
        splitBomAction->setObjectName("splitBomAction.1");
        splitBomAction->setStatusTip(tr("Split the bill of materials across this page and the next page"));
        splitBomAction->setWhatsThis(tr("Split Bill Of Materials:\n"
                                        "  You can split the bill of materials on this page across\n"
                                        "  this page and the next page.\n"));
        lpub->actions.insert(splitBomAction->objectName(), Action(QStringLiteral("BOMContext.Split Bill Of Materials"), splitBomAction));

        //callout context menu items

        QAction *unpackCalloutAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/unpackcallout.png"),tr("Unpack Callout"));
        unpackCalloutAction->setObjectName("unpackCalloutAction.1");
        unpackCalloutAction->setStatusTip(tr("Unpack the unassembled callout into individual steps"));
        unpackCalloutAction->setWhatsThis(tr("Unpack Callout:\n"
                                             "  You can unpack the unassembled callout into individual steps\n"));
        lpub->actions.insert(unpackCalloutAction->objectName(), Action(QStringLiteral("CalloutContext.Unpack Callout"), unpackCalloutAction));

        QAction *removeCalloutAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/remove.png"),tr("Remove Callout"));
        removeCalloutAction->setObjectName("removeCalloutAction.1");
        removeCalloutAction->setStatusTip(tr("Remove the assembled callout from the build instructions"));
        removeCalloutAction->setWhatsThis(tr("Remove Callout:\n"
                                             "  You can remove the assembled callout from the build instructions.\n"));
        lpub->actions.insert(removeCalloutAction->objectName(), Action(QStringLiteral("CalloutContext.Remove Callout"), removeCalloutAction));

        QAction *rotateCalloutAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/rotate.png"),tr("Rotate Callout"));
        rotateCalloutAction->setObjectName("rotateCalloutAction.1");
        rotateCalloutAction->setStatusTip(tr("Rotate the assembled callout"));
        rotateCalloutAction->setWhatsThis(tr("Rotate Callout:\n"
                                             "  You can rotate the assembled callout.\n"));
        lpub->actions.insert(rotateCalloutAction->objectName(), Action(QStringLiteral("CalloutContext.Rotate Callout"), rotateCalloutAction));

        QAction *unrotateCalloutAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/unrotate.png"),tr("Unrotate Callout"));
        unrotateCalloutAction->setObjectName("unrotateCalloutAction.1");
        unrotateCalloutAction->setStatusTip(tr("Unrotate the assembled callout"));
        unrotateCalloutAction->setWhatsThis(tr("Unrotate Callout:\n"
                                               "  You can unrotate the unassembled callout.\n"));
        lpub->actions.insert(unrotateCalloutAction->objectName(), Action(QStringLiteral("CalloutContext.Unrotate Callout"), unrotateCalloutAction));

        QAction *addPointerAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/addpointer.png"),tr("Add Callout Pointer"));
        addPointerAction->setObjectName("addPointerAction.1");
        addPointerAction->setStatusTip(tr("Add pointer from this callout to the step model where it is used"));
        addPointerAction->setWhatsThis(tr("Add Callout Pointer:\n"
                                          "  You can add a pointer from this callout to the step\n"
                                          "  model where it is used.\n"));
        lpub->actions.insert(addPointerAction->objectName(), Action(QStringLiteral("CalloutContext.Add Callout Pointer"), addPointerAction));

        // divider context menu actions

        QAction *editDividerAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/editdivider.png"),tr("Edit Divider"));
        editDividerAction->setObjectName("editDividerAction.1");
        editDividerAction->setStatusTip(tr("Edit this divider margin, thickness, length, and color"));
        editDividerAction->setWhatsThis(tr("Edit Divider:\n"
                                           "  You can edit this divider margin, thickness,\n"
                                           "  length, and color.\n"));
        lpub->actions.insert(editDividerAction->objectName(), Action(QStringLiteral("DividerContext.Edit Divider"), editDividerAction));

        //rotate icon context menu actions

        QAction *editRotateIconArrowAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/editrotateicon.png"),tr("Edit Rotate Icon Arrows"));
        editRotateIconArrowAction->setObjectName("editRotateIconArrowAction.1");
        editRotateIconArrowAction->setStatusTip(tr("Edit the rotate icon arrows"));
        editRotateIconArrowAction->setWhatsThis(tr("Edit Rotate Icon Arrows:\n"
                                                   "  You can change the rotate icon arrow attributes such\n"
                                                   "  as color and line.\n"));
        lpub->actions.insert(editRotateIconArrowAction->objectName(), Action(QStringLiteral("RotateIconContext.Edit Rotate Icon Arrows"), editRotateIconArrowAction));

        QAction *rotateIconSizeAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/rotateiconsize.png"),tr("Change Rotate Icon Size"));
        rotateIconSizeAction->setObjectName("rotateIconSizeAction.1");
        rotateIconSizeAction->setStatusTip(tr("Change the rotateIcon size"));
        rotateIconSizeAction->setWhatsThis(tr("Change Rotate Icon Size:\n"
                                              "  You can change the rotateIcon size.\n"));
        lpub->actions.insert(rotateIconSizeAction->objectName(), Action(QStringLiteral("RotateIconContext.Change Rotate Icon Size"), rotateIconSizeAction));

        // pointer context menu actions

        const QString units = QString(" in %1").arg(Preferences::preferCentimeters ? tr("centimetres") : tr("inches"));

        QAction *setLineAttributesAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/lineattributes.png"),tr("Edit Line Attributes"));
        setLineAttributesAction->setObjectName("setLineAttributesAction.1");
        setLineAttributesAction->setStatusTip(tr("Edit pointer line attributes"));
        setLineAttributesAction->setWhatsThis(tr("Edit Line Attributes:\n"
                                                 "  You can edit the pointer line color, thickness,\n"
                                                 "  and type %1.\n").arg(units));
        lpub->actions.insert(setLineAttributesAction->objectName(), Action(QStringLiteral("PointerContext.Edit Line Attributes"), setLineAttributesAction));

        QAction *setBorderAttributesAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/borderattributes.png"),tr("Edit Border Attributes"));
        setBorderAttributesAction->setObjectName("setBorderAttributesAction.1");
        setBorderAttributesAction->setStatusTip(tr("Edit pointer border attributes"));
        setBorderAttributesAction->setWhatsThis(tr("Edit Border Attributes:\n"
                                                   "  You can edit the pointer border color, thickness,\n"
                                                   "  and line type %1.\n").arg(units));
        lpub->actions.insert(setBorderAttributesAction->objectName(), Action(QStringLiteral("PointerContext.Edit Border Attributes"), setBorderAttributesAction));

        QAction *setTipAttributesAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/tipattributes.png"),tr("Edit Tip Attributes"));
        setTipAttributesAction->setObjectName("setTipAttributesAction.1");
        setTipAttributesAction->setStatusTip(tr("Edit pointer border attributes"));
        setTipAttributesAction->setWhatsThis(tr("Edit Tip Attributes:\n"
                                                "  You can edit the pointer pointer tip width,\n"
                                                "  and height %1.\n").arg(units));
        lpub->actions.insert(setTipAttributesAction->objectName(), Action(QStringLiteral("PointerContext.Edit Tip Attributes"), setTipAttributesAction));

        QAction *resetLineAttributesAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/resetlineattributes.png"),tr("Reset Line Attributes"));
        resetLineAttributesAction->setObjectName("resetLineAttributesAction.1");
        resetLineAttributesAction->setStatusTip(tr("Reset pointer line attributes"));
        resetLineAttributesAction->setWhatsThis(tr("Reset Line Attributes:\n"
                                                   "  You can reset the pointer border color, thickness,\n"
                                                   "  and line type %1.\n").arg(units));
        lpub->actions.insert(resetLineAttributesAction->objectName(), Action(QStringLiteral("PointerContext.Reset Line Attributes"), resetLineAttributesAction));

        QAction *resetBorderAttributesAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/resetborderattributes.png"),tr("Reset Border Attributes"));
        resetBorderAttributesAction->setObjectName("resetBorderAttributesAction.1");
        resetBorderAttributesAction->setStatusTip(tr("Reset pointer border attributes"));
        resetBorderAttributesAction->setWhatsThis(tr("Reset Border Attributes:\n"
                                                     "  You can reset the pointer border color, thickness, and\n"
                                                     "  line type %1.\n").arg(units));
        lpub->actions.insert(resetBorderAttributesAction->objectName(), Action(QStringLiteral("PointerContext.Edit Border Attributes"), resetBorderAttributesAction));

        QAction *resetTipAttributesAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/resettipattributes.png"),tr("Reset Tip Attributes"));
        resetTipAttributesAction->setObjectName("resetTipAttributesAction.1");
        resetTipAttributesAction->setStatusTip(tr("Reset pointer border attributes"));
        resetTipAttributesAction->setWhatsThis(tr("Reset Tip Attributes:\n"
                                                  "  You can reset the pointer pointer tip width,\n"
                                                  "  and height %1.\n").arg(units));
        lpub->actions.insert(resetTipAttributesAction->objectName(), Action(QStringLiteral("PointerContext.Reset Tip Attributes"), resetTipAttributesAction));

        QAction *addSegmentAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/addpointersegment.png"),tr("Add Pointer Segment"));
        addSegmentAction->setObjectName("addSegmentAction.1");
        addSegmentAction->setStatusTip(tr("Add a new pointer shaft segment"));
        addSegmentAction->setWhatsThis(tr("Add Pointer Segment:\n"
                                          "  You can add a new pointer shaft segment\n"
                                          "  the build instruction.\n"
                                          "  A total of three shaft segments per pointer\n"
                                          "  are allowed.\n"));
        lpub->actions.insert(addSegmentAction->objectName(), Action(QStringLiteral("PointerContext.Add Pointer Segment"), addSegmentAction));

        QAction *removeSegmentAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/removepointersegment.png"),tr("Remove Pointer Segment"));
        removeSegmentAction->setObjectName("removeSegmentAction.1");
        removeSegmentAction->setStatusTip(tr("Remove pointer shaft segment"));
        removeSegmentAction->setWhatsThis(tr("Remove Pointer Segment:\n"
                                             "  You can remove a pointer shaft segments\n"
                                             "  from the pointer.\n"));
        lpub->actions.insert(removeSegmentAction->objectName(), Action(QStringLiteral("PointerContext.Remove Pointer Segment"), removeSegmentAction));

        QAction *toggleCsiPartRectAction = commonMenus.addAction(*contextMenu,QIcon(":/resources/togglepartoutline.png"),tr("Toggle CSI Part Outline"));
        toggleCsiPartRectAction->setObjectName("toggleCsiPartRectAction.1");
        toggleCsiPartRectAction->setStatusTip(tr("Toggle on and off an outline of the annotated CSI part"));
        toggleCsiPartRectAction->setWhatsThis(tr("Toggle CSI Part Outline:\n"
                                                 "  You turn on and off the outline of the\n"
                                                 "  annotated CSI part to aid in interactively\n"
                                                 "  moving and placing the annotation item.\n"));
        lpub->actions.insert(toggleCsiPartRectAction->objectName(), Action(QStringLiteral("AssemblyContext.Toggle Part Outline"), toggleCsiPartRectAction));

        // shared delete context menu items

        QAction *deleteImageAction = commonMenus.deleteMenu(*contextMenu,name,"deleteImageAction.1","deleteimage.png");
        lpub->actions.insert(deleteImageAction->objectName(), Action(QStringLiteral("PageContext.Delete Image"), deleteImageAction));

        QAction *refreshPageCacheAction = commonMenus.refreshImageCacheMenu(*contextMenu,name,"refreshPageCacheAction.1");
        lpub->actions.insert(refreshPageCacheAction->objectName(), Action(QStringLiteral("PageContext.Reset Page Image Cache"), refreshPageCacheAction));

        QAction *deletePointerAction = commonMenus.deleteMenu(*contextMenu,name,"deletePointerAction.1","deletepointer.png");
        lpub->actions.insert(deletePointerAction->objectName(), Action(QStringLiteral("PointerContext.Delete Pointer"), deletePointerAction));

        QAction *deleteTextAction = commonMenus.deleteMenu(*contextMenu,name,"deleteTextAction.1","deletetext.png");
        lpub->actions.insert(deleteTextAction->objectName(), Action(QStringLiteral("TextEdit.Context.Delete Text"), deleteTextAction));

        QAction *deleteDividerAction = commonMenus.deleteMenu(*contextMenu,name,"deleteDividerAction.1","deletedivider.png",tr("step group"));
        lpub->actions.insert(deleteDividerAction->objectName(), Action(QStringLiteral("DividerContext.Delete Divider"), deleteDividerAction));

        QAction *deleteRotateIconAction = commonMenus.deleteMenu(*contextMenu,name,"deleteRotateIconAction.1");
        lpub->actions.insert(deleteRotateIconAction->objectName(), Action(QStringLiteral("RotateIconContext.Delete Roate Icon"), deleteRotateIconAction));

        QAction *deleteBomAction = commonMenus.deleteMenu(*contextMenu,name,"deleteBomAction.1");
        lpub->actions.insert(deleteBomAction->objectName(), Action(QStringLiteral("BOMContext.Delete Bill Of Materials"), deleteBomAction));

        // shared step context menu items

        QAction *removeStepAction = commonMenus.removeStepMenu(*contextMenu,name);
        lpub->actions.insert(removeStepAction->objectName(), Action(QStringLiteral("StepContext.Remove This Step"), removeStepAction));

        QAction *addNextStepAction = commonMenus.addNextStepMenu(*contextMenu,name);
        lpub->actions.insert(addNextStepAction->objectName(), Action(QStringLiteral("StepContext.Add Next Step"), addNextStepAction));

        QAction *addNextStepsAction = commonMenus.addNextStepsMenu(*contextMenu,name);
        lpub->actions.insert(addNextStepsAction->objectName(), Action(QStringLiteral("StepContext.Add Next Steps"), addNextStepsAction));

        QAction *addPrevStepAction = commonMenus.addPrevStepMenu(*contextMenu,name);
        lpub->actions.insert(addPrevStepAction->objectName(), Action(QStringLiteral("StepContext.Add Previous Step"), addPrevStepAction));

        QAction *noPartsListAction = commonMenus.noPartsListMenu(*contextMenu,name);
        lpub->actions.insert(noPartsListAction->objectName(), Action(QStringLiteral("StepContext.No Parts List Per Step"), noPartsListAction));

        QAction *partsListAction = commonMenus.partsListMenu(*contextMenu,name);
        lpub->actions.insert(partsListAction->objectName(), Action(QStringLiteral("StepContext.Parts List Per Step"), partsListAction));

        QAction *refreshStepCacheAction = commonMenus.refreshImageCacheMenu(*contextMenu,name,"refreshStepCacheAction.1","refreshstepcache.png", CsiType);
        lpub->actions.insert(refreshStepCacheAction->objectName(), Action(QStringLiteral("StepContext.Reset Step Assembly Image Cache"), refreshStepCacheAction));

        // shared context menu actions

        QAction *refreshPartsCacheAction = commonMenus.refreshImageCacheMenu(*contextMenu,name,"refreshPartsCacheAction.1","refreshpartscache.png", PartsListType);
        lpub->actions.insert(refreshPartsCacheAction->objectName(), Action(QStringLiteral("SharedContext.Reset Step Assembly Image Cache"), refreshPartsCacheAction));

        QAction *refreshSubmodelCacheAction = commonMenus.refreshImageCacheMenu(*contextMenu,name,"refreshSubmodelCacheAction.1","refreshsubmodelcache.png", SubModelType);
        lpub->actions.insert(refreshSubmodelCacheAction->objectName(), Action(QStringLiteral("SharedContext.Reset Step Assembly Image Cache"), refreshSubmodelCacheAction));

        QAction *rendererArgumentsAction = commonMenus.rendererArgumentsMenu(*contextMenu,name);
        lpub->actions.insert(rendererArgumentsAction->objectName(), Action(QStringLiteral("SharedContext.Add Renderer Arguments"), rendererArgumentsAction));

        QAction *povrayRendererArgumentsAction = commonMenus.rendererArgumentsMenu(*contextMenu,name,"povrayRendererArgumentsAction.1");
        lpub->actions.insert(povrayRendererArgumentsAction->objectName(), Action(QStringLiteral("SharedContext.Add Povray Renderer Arguments"), povrayRendererArgumentsAction));

        QAction *highlightStepAction = commonMenus.highlightStepMenu(*contextMenu,name);
        lpub->actions.insert(highlightStepAction->objectName(), Action(QStringLiteral("SharedContext.Highlight Step"), highlightStepAction));

        QAction *fadeStepsAction = commonMenus.fadeStepsMenu(*contextMenu,name);
        lpub->actions.insert(fadeStepsAction->objectName(), Action(QStringLiteral("SharedContext.Fade Steps"), fadeStepsAction));

        QAction *preferredRendererAction = commonMenus.preferredRendererMenu(*contextMenu,name);
        lpub->actions.insert(preferredRendererAction->objectName(), Action(QStringLiteral("SharedContext.Change Renderer"), preferredRendererAction));

        QAction *previewPartAction = commonMenus.previewPartMenu(*contextMenu,name);
        lpub->actions.insert(previewPartAction->objectName(), Action(QStringLiteral("SharedContext.Preview Item"), previewPartAction));

        QAction *resetViewerImageAction = commonMenus.resetViewerImageMenu(*contextMenu,name);
        lpub->actions.insert(resetViewerImageAction->objectName(), Action(QStringLiteral("SharedContext.Reset Viewer Image"), resetViewerImageAction));

#ifndef QT_NO_CLIPBOARD
        QAction *copyToClipboardAction = commonMenus.copyToClipboardMenu(*contextMenu,name);
        lpub->actions.insert(copyToClipboardAction->objectName(), Action(QStringLiteral("SharedContext.Copy Image Path To Clipboard"), copyToClipboardAction));
#endif

        QAction *displayRowsAction = commonMenus.displayRowsMenu(*contextMenu,name);
        lpub->actions.insert(displayRowsAction->objectName(), Action(QStringLiteral("SharedContext.Display As Rows"), displayRowsAction));

        QAction *displayColumnsAction = commonMenus.displayColumnsMenu(*contextMenu,name);
        lpub->actions.insert(displayColumnsAction->objectName(), Action(QStringLiteral("SharedContext.Display As Columns"), displayColumnsAction));

        QAction *backgroundAction = commonMenus.backgroundMenu(*contextMenu,name);
        lpub->actions.insert(backgroundAction->objectName(), Action(QStringLiteral("SharedContext.Change Background"), backgroundAction));

        QAction *subModelColorAction = commonMenus.subModelColorMenu(*contextMenu,name);
        lpub->actions.insert(subModelColorAction->objectName(), Action(QStringLiteral("SharedContext.Change Submodel Color"), subModelColorAction));

        QAction *placementAction = commonMenus.placementMenu(*contextMenu,name);
        lpub->actions.insert(placementAction->objectName(), Action(QStringLiteral("SharedContext.Change Placement"), placementAction));

        QAction *cameraAnglesAction = commonMenus.cameraAnglesMenu(*contextMenu,name);
        lpub->actions.insert(cameraAnglesAction->objectName(), Action(QStringLiteral("SharedContext.Change Camera Angles"), cameraAnglesAction));

        QAction *cameraFoVAction = commonMenus.cameraFoVMenu(*contextMenu,name);
        lpub->actions.insert(cameraFoVAction->objectName(), Action(QStringLiteral("SharedContext.Change Camera FOV Angle"), cameraFoVAction));

        QAction *scaleAction = commonMenus.scaleMenu(*contextMenu,name);
        lpub->actions.insert(scaleAction->objectName(), Action(QStringLiteral("SharedContext.Change Scale"), scaleAction));

        QAction *stretchImageAction = commonMenus.stretchImageMenu(*contextMenu,name);
        lpub->actions.insert(stretchImageAction->objectName(), Action(QStringLiteral("SharedContext.Stretch Image"), stretchImageAction));

        QAction *tileImageAction = commonMenus.tileImageMenu(*contextMenu,name);
        lpub->actions.insert(tileImageAction->objectName(), Action(QStringLiteral("SharedContext.Tile Image"), tileImageAction));

        QAction *marginAction = commonMenus.marginMenu(*contextMenu,name);
        lpub->actions.insert(marginAction->objectName(), Action(QStringLiteral("SharedContext.Change Margins"), marginAction));

        QAction *borderAction = commonMenus.borderMenu(*contextMenu,name);
        lpub->actions.insert(borderAction->objectName(), Action(QStringLiteral("SharedContext.Change Border"), borderAction));

        QAction *fontAction = commonMenus.fontMenu(*contextMenu,name);
        lpub->actions.insert(fontAction->objectName(), Action(QStringLiteral("SharedContext.Change Font"), fontAction));

        QAction *colorAction = commonMenus.colorMenu(*contextMenu,name);
        lpub->actions.insert(colorAction->objectName(), Action(QStringLiteral("SharedContext.Change Color"), colorAction));

        QAction *changeImageAction = commonMenus.changeImageMenu(*contextMenu,name);
        lpub->actions.insert(changeImageAction->objectName(), Action(QStringLiteral("SharedContext.Change Image"), changeImageAction));

        QAction *displayTextAction = commonMenus.displayMenu(*contextMenu,name,"displayTextAction.1");
        lpub->actions.insert(displayTextAction->objectName(), Action(QStringLiteral("SharedContext.Toggle Text Display"), displayTextAction));

        QAction *displayImageAction = commonMenus.displayMenu(*contextMenu,name,"displayImageAction.1");
        lpub->actions.insert(displayImageAction->objectName(), Action(QStringLiteral("SharedContext.Toggle Image Display"), displayImageAction));

        QAction *displayRotateIconAction = commonMenus.displayMenu(*contextMenu,name,"displayRotateIconAction.1");
        lpub->actions.insert(displayRotateIconAction->objectName(), Action(QStringLiteral("SharedContext.Toggle RotateIcon Display"), displayRotateIconAction));

        QAction *constrainAction = commonMenus.constrainMenu(*contextMenu,name);
        lpub->actions.insert(constrainAction->objectName(), Action(QStringLiteral("SharedContext.Change Shape"), constrainAction));

        QAction *partGroupsOffAction = commonMenus.partGroupsOffMenu(*contextMenu,name);
        lpub->actions.insert(partGroupsOffAction->objectName(), Action(QStringLiteral("SharedContext.Turn Off Movable Group"), partGroupsOffAction));

        QAction *partGroupsOnAction = commonMenus.partGroupsOnMenu(*contextMenu,name);
        lpub->actions.insert(partGroupsOnAction->objectName(), Action(QStringLiteral("SharedContext.Turn On Movable Group"), partGroupsOnAction));

        QAction *sortAction = commonMenus.sortMenu(*contextMenu,name);
        lpub->actions.insert(sortAction->objectName(), Action(QStringLiteral("SharedContext.Sort Parts"), sortAction));

        QAction *annotationAction = commonMenus.annotationMenu(*contextMenu,name);
        lpub->actions.insert(annotationAction->objectName(), Action(QStringLiteral("SharedContext.Annotation Options"), annotationAction));

        QAction *sizeAction = commonMenus.sizeMenu(*contextMenu,name);
        lpub->actions.insert(sizeAction->objectName(), Action(QStringLiteral("SharedContext.Change Size"), sizeAction));

        QAction *hideAction = commonMenus.hideMenu(*contextMenu,name);
        lpub->actions.insert(hideAction->objectName(), Action(QStringLiteral("SharedContext.Hide Item"), hideAction));

        QAction *hideCsiAnnotationAction = commonMenus.hideMenu(*contextMenu,name,"hideCsiAnnotationAction.1","hidepartannotation.png");
        lpub->actions.insert(hideCsiAnnotationAction->objectName(), Action(QStringLiteral("AssemblyContext.Hide Csi Annotation"), hideCsiAnnotationAction));

        QAction *resetPartGroupAction = commonMenus.resetPartGroupMenu(*contextMenu,name);
        lpub->actions.insert(resetPartGroupAction->objectName(), Action(QStringLiteral("SharedContext.Reset Group"), resetPartGroupAction));

        QAction *substitutePartAction = commonMenus.substitutePartMenu(*contextMenu,name);
        lpub->actions.insert(substitutePartAction->objectName(), Action(QStringLiteral("SharedContext.Substitute Part"), substitutePartAction));

        QAction *changeSubstitutePartAction = commonMenus.changeSubstitutePartMenu(*contextMenu,name);
        lpub->actions.insert(changeSubstitutePartAction->objectName(), Action(QStringLiteral("SharedContext.Change Substitute"), changeSubstitutePartAction));

        QAction *removeSubstitutePartAction = commonMenus.removeSubstitutePartMenu(*contextMenu,name);
        lpub->actions.insert(removeSubstitutePartAction->objectName(), Action(QStringLiteral("SharedContext.Remove Substitute"), removeSubstitutePartAction));

        QAction *textAction = commonMenus.textMenu(*contextMenu,name);
        lpub->actions.insert(textAction->objectName(), Action(QStringLiteral("SharedContext.Edit Text"), textAction));

        QAction *rotStepAction = commonMenus.rotStepMenu(*contextMenu,name);
        lpub->actions.insert(rotStepAction->objectName(), Action(QStringLiteral("SharedContext.Change Rotation"), rotStepAction));

        QAction *overrideCountAction = commonMenus.overrideCountMenu(*contextMenu,name);
        lpub->actions.insert(overrideCountAction->objectName(), Action(QStringLiteral("SharedContext.Override Count"), overrideCountAction));

        QAction *restoreCountAction = commonMenus.restoreCountMenu(*contextMenu,name);
        lpub->actions.insert(restoreCountAction->objectName(), Action(QStringLiteral("SharedContext.Restore Count"), restoreCountAction));

        // 3DViewer actions
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
}

void Gui::enableActions()
{
  if (suspendFileDisplay)
    return;

  gui->getAct("openWorkingFolderAct.1")->setEnabled(true);
  gui->getAct("reloadFromDiskAct.1")->setEnabled(true);
  gui->getAct("saveAsAct.1")->setEnabled(true);
  gui->getAct("saveCopyAct.1")->setEnabled(true);
  gui->getAct("closeFileAct.1")->setEnabled(true);
  gui->getAct("printToFileAct.1")->setEnabled(true);
  gui->getAct("printToFilePreviewAct.1")->setEnabled(true);
  gui->getAct("exportAsPdfAct.1")->setEnabled(true);
  gui->getAct("exportAsPdfPreviewAct.1")->setEnabled(true);
  gui->getAct("exportPngAct.1")->setEnabled(true);
  gui->getAct("exportJpgAct.1")->setEnabled(true);
  gui->getAct("exportBmpAct.1")->setEnabled(true);
  gui->getAct("loadStatusAct.1")->setEnabled(true);
  gui->getAct("recountPartsAct.1")->setEnabled(true);
  gui->getAct("pageSetupAct.1")->setEnabled(true);
  gui->getAct("assemSetupAct.1")->setEnabled(true);
  gui->getAct("pliSetupAct.1")->setEnabled(true);
  gui->getAct("bomSetupAct.1")->setEnabled(true);
  gui->getAct("calloutSetupAct.1")->setEnabled(true);
  gui->getAct("multiStepSetupAct.1")->setEnabled(true);
  gui->getAct("subModelSetupAct.1")->setEnabled(true);
  gui->getAct("projectSetupAct.1")->setEnabled(true);
  gui->getAct("fadeStepsSetupAct.1")->setEnabled(true);
  gui->getAct("highlightStepSetupAct.1")->setEnabled(true);
  gui->getAct("addPictureAct.1")->setEnabled(true);
  gui->getAct("removeLPubFormatDocumentAct.1")->setEnabled(true);
  gui->getAct("removeLPubFormatBomAct.1")->setEnabled(true);
  gui->getAct("removeLPubFormatPageAct.1")->setEnabled(true);
  gui->getAct("removeLPubFormatStepAct.1")->setEnabled(true);
  gui->getAct("removeLPubFormatSubmodelAct.1")->setEnabled(true);
  gui->getAct("removeChildSubmodelFormatAct.1")->setEnabled(true);
  gui->getAct("removeBuildModFormatAct.1")->setEnabled(Preferences::buildModEnabled);
  gui->getAct("editTitleAnnotationsAct.1")->setEnabled(true);
  gui->getAct("editFreeFormAnnitationsAct.1")->setEnabled(true);
  gui->getAct("editPliBomSubstitutePartsAct.1")->setEnabled(true);
  gui->getAct("editExcludedPartsAct.1")->setEnabled(true);
  gui->getAct("editStickerPartsAct.1")->setEnabled(true);
  gui->getAct("editLDrawColourPartsAct.1")->setEnabled(!Preferences::ldrawColourPartsFile.isEmpty());
  gui->getAct("editLdgliteIniAct.1")->setEnabled(true);
  gui->getAct("editNativePOVIniAct.1")->setEnabled(true);
  gui->getAct("editLdviewIniAct.1")->setEnabled(true);
  gui->getAct("editLdviewPovIniAct.1")->setEnabled(true);
  gui->getAct("editBlenderParametersAct.1")->setEnabled(true);
  gui->getAct("editBlenderPreferencesAct.1")->setEnabled(true);
  gui->getAct("editPovrayIniAct.1")->setEnabled(true);
  gui->getAct("editPovrayConfAct.1")->setEnabled(true);
  gui->getAct("editAnnotationStyleAct.1")->setEnabled(true);
  gui->getAct("editLD2BLCodesXRefAct.1")->setEnabled(true);
  gui->getAct("editLD2BLColorsXRefAct.1")->setEnabled(true);
  gui->getAct("editLD2RBCodesXRefAct.1")->setEnabled(true);
  gui->getAct("editLD2RBColorsXRefAct.1")->setEnabled(true);
  gui->getAct("editBLColorsAct.1")->setEnabled(true);
  gui->getAct("editBLCodesAct.1")->setEnabled(true);
  gui->getAct("editModelFileAct.1")->setEnabled(true);
  gui->getAct("editPliControlFileAct.1")->setEnabled(!Preferences::pliControlFile.isEmpty());
  gui->getAct("openParameterFileFolderAct.1")->setEnabled(true);

//  setPageLineEdit)->setEnabled(true);

//  gui->getAct("firstPageAct.1")->setEnabled(true);
//  gui->getAct("lastPageAct.1")->setEnabled(true);

//  gui->getAct("nextPageAct.1")->setEnabled(true);
//  gui->getAct("previousPageAct.1")->setEnabled(true);
//  gui->getAct("nextPageComboAct.1")->setEnabled(true);
//  gui->getAct("previousPageComboAct.1")->setEnabled(true);
//  gui->getAct("nextPageContinuousAct.1")->setEnabled(true);
//  gui->getAct("previousPageContinuousAct.1")->setEnabled(true);

  gui->getAct("fitWidthAct.1")->setEnabled(true);
  gui->getAct("fitVisibleAct.1")->setEnabled(true);
  gui->getAct("fitSceneAct.1")->setEnabled(true);
  gui->getAct("bringToFrontAct.1")->setEnabled(true);
  gui->getAct("sendToBackAct.1")->setEnabled(true);
  gui->getAct("actualSizeAct.1")->setEnabled(true);
  gui->getAct("zoomInComboAct.1")->setEnabled(true);
  gui->getAct("zoomOutComboAct.1")->setEnabled(true);
  gui->getAct("sceneGuidesComboAct.1")->setEnabled(true);
  gui->getAct("snapToGridComboAct.1")->setEnabled(true);
  gui->getAct("exportBricklinkAct.1")->setEnabled(true);
  gui->getAct("exportCsvAct.1")->setEnabled(true);
  gui->getAct("exportPovAct.1")->setEnabled(true);
  gui->getAct("exportStlAct.1")->setEnabled(true);
  gui->getAct("export3dsAct.1")->setEnabled(true);
  gui->getAct("exportObjAct.1")->setEnabled(true);
  gui->getAct("exportColladaAct.1")->setEnabled(true);
  gui->getAct("exportHtmlAct.1")->setEnabled(true);
  gui->getAct("exportHtmlStepsAct.1")->setEnabled(true);
  gui->getAct("copyFileNameToClipboardAct.1")->setEnabled(true);
  gui->getAct("copyFilePathToClipboardAct.1")->setEnabled(true);
  gui->blenderRenderAct->setEnabled(true);
  gui->blenderImportAct->setEnabled(true);
  gui->povrayRenderAct->setEnabled(true);

  gui->getMenu("setupMenu")->setEnabled(true);
  gui->getMenu("cacheMenu")->setEnabled(true);
  gui->getMenu("exportMenu")->setEnabled(true);
  gui->getMenu("loadStatusMenu")->setEnabled(true);

  //Visual Editor
  //ViewerExportMenu->setEnabled(true); // Hide Visual Editor step export functions

}

void Gui::disableActions()
{
  gui->getAct("openWorkingFolderAct.1")->setEnabled(false);
  gui->getAct("reloadFromDiskAct.1")->setEnabled(false);
  gui->getAct("saveAsAct.1")->setEnabled(false);
  gui->getAct("saveCopyAct.1")->setEnabled(false);
  gui->getAct("closeFileAct.1")->setEnabled(false);
  gui->getAct("printToFilePreviewAct.1")->setEnabled(false);
  gui->getAct("printToFileAct.1")->setEnabled(false);
  gui->getAct("exportAsPdfPreviewAct.1")->setEnabled(false);
  gui->getAct("exportAsPdfAct.1")->setEnabled(false);
  gui->getAct("exportPngAct.1")->setEnabled(false);
  gui->getAct("exportJpgAct.1")->setEnabled(false);
  gui->getAct("exportBmpAct.1")->setEnabled(false);
  gui->getAct("loadStatusAct.1")->setEnabled(false);
  gui->getAct("recountPartsAct.1")->setEnabled(false);
  gui->getAct("pageSetupAct.1")->setEnabled(false);
  gui->getAct("assemSetupAct.1")->setEnabled(false);
  gui->getAct("pliSetupAct.1")->setEnabled(false);
  gui->getAct("bomSetupAct.1")->setEnabled(false);
  gui->getAct("calloutSetupAct.1")->setEnabled(false);
  gui->getAct("multiStepSetupAct.1")->setEnabled(false);
  gui->getAct("subModelSetupAct.1")->setEnabled(false);
  gui->getAct("projectSetupAct.1")->setEnabled(false);
  gui->getAct("fadeStepsSetupAct.1")->setEnabled(false);
  gui->getAct("highlightStepSetupAct.1")->setEnabled(false);
  gui->getAct("addPictureAct.1")->setEnabled(false);
  gui->getAct("removeLPubFormatBomAct.1")->setEnabled(false);
  gui->getAct("removeLPubFormatDocumentAct.1")->setEnabled(false);
  gui->getAct("removeLPubFormatPageAct.1")->setEnabled(false);
  gui->getAct("removeLPubFormatStepAct.1")->setEnabled(false);
  gui->getAct("removeLPubFormatSubmodelAct.1")->setEnabled(false);
  gui->getAct("removeChildSubmodelFormatAct.1")->setEnabled(false);
  gui->getAct("removeBuildModFormatAct.1")->setEnabled(false);
  gui->getAct("editModelFileAct.1")->setEnabled(false);
  gui->getAct("firstPageAct.1")->setEnabled(false);
  gui->getAct("lastPageAct.1")->setEnabled(false);
  gui->getAct("nextPageAct.1")->setEnabled(false);
  gui->getAct("previousPageAct.1")->setEnabled(false);
  gui->getAct("nextPageComboAct.1")->setEnabled(false);
  gui->getAct("previousPageComboAct.1")->setEnabled(false);
  gui->getAct("nextPageContinuousAct.1")->setEnabled(false);
  gui->getAct("previousPageContinuousAct.1")->setEnabled(false);
  gui->getAct("fitWidthAct.1")->setEnabled(false);
  gui->getAct("fitVisibleAct.1")->setEnabled(false);
  gui->getAct("fitSceneAct.1")->setEnabled(false);
  gui->getAct("bringToFrontAct.1")->setEnabled(false);
  gui->getAct("sendToBackAct.1")->setEnabled(false);
  gui->getAct("actualSizeAct.1")->setEnabled(false);
  gui->getAct("zoomInComboAct.1")->setEnabled(false);
  gui->getAct("zoomOutComboAct.1")->setEnabled(false);
  gui->getAct("exportBricklinkAct.1")->setEnabled(false);
  gui->getAct("exportCsvAct.1")->setEnabled(false);
  gui->getAct("exportPovAct.1")->setEnabled(false);
  gui->getAct("exportStlAct.1")->setEnabled(false);
  gui->getAct("export3dsAct.1")->setEnabled(false);
  gui->getAct("exportObjAct.1")->setEnabled(false);
  gui->getAct("exportColladaAct.1")->setEnabled(false);
  gui->getAct("exportHtmlAct.1")->setEnabled(false);
  gui->getAct("exportHtmlStepsAct.1")->setEnabled(false);
  gui->getAct("copyFileNameToClipboardAct.1")->setEnabled(false);
  gui->getAct("copyFilePathToClipboardAct.1")->setEnabled(false);
  gui->blenderRenderAct->setEnabled(false);
  gui->blenderImportAct->setEnabled(false);
  gui->povrayRenderAct->setEnabled(false);

  gui->setPageLineEdit->setEnabled(false);

  gui->getMenu("setupMenu")->setEnabled(false);
  gui->getMenu("cacheMenu")->setEnabled(false);
  gui->getMenu("exportMenu")->setEnabled(false);
  gui->getMenu("loadStatusMenu")->setEnabled(false);
  gui->openWithMenu->setEnabled(false);

  // Visual Editor
  // ViewerExportMenu->setEnabled(false); // Hide Visual Editor step export functions

}

void Gui::enableEditActions()
{
    bool frontCoverPageExist = lpub->mi.frontCoverPageExist();
    gui->getAct("insertCoverPageAct.1")->setEnabled(!frontCoverPageExist &&
                                               lpub->mi.okToInsertCoverPage());
    bool backCoverPageExist = lpub->mi.backCoverPageExist();
    gui->getAct("appendCoverPageAct.1")->setEnabled(!backCoverPageExist &&
                                               lpub->mi.okToAppendCoverPage());

    gui->updateGoToPage(frontCoverPageExist, backCoverPageExist);

    bool insertNumberedPage = lpub->mi.okToInsertNumberedPage();
    gui->getAct("insertNumberedPageAct.1")->setEnabled(insertNumberedPage);
    bool appendNumberedPage = lpub->mi.okToAppendNumberedPage();
    gui->getAct("appendNumberedPageAct.1")->setEnabled(appendNumberedPage);
    gui->getAct("deletePageAct.1")->setEnabled(lpub->page.list.size() == 0);
    gui->getAct("addBomAct.1")->setEnabled(insertNumberedPage||appendNumberedPage);
    gui->getAct("addTextAct.1")->setEnabled(true);
}

void Gui::disableEditActions()
{
    gui->getAct("insertCoverPageAct.1")->setEnabled(false);
    gui->getAct("appendCoverPageAct.1")->setEnabled(false);
    gui->getAct("insertNumberedPageAct.1")->setEnabled(false);
    gui->getAct("appendNumberedPageAct.1")->setEnabled(false);
    gui->getAct("deletePageAct.1")->setEnabled(false);
    gui->getAct("addBomAct.1")->setEnabled(false);
    gui->getAct("addTextAct.1")->setEnabled(false);
}

QMenu *Gui::getMenu(const QString &objectName)
{
    if (gui->menus.contains(objectName))
        return gui->menus.value(objectName);
    return nullptr;
}

void Gui::createMenus()
{
    // Editor Menus

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->setObjectName("fileMenu");
    gui->menus.insert(fileMenu->objectName(), fileMenu);
    fileMenu->addAction(gui->getAct("openAct.1"));

    gui->openWithMenu = fileMenu->addMenu(tr("Open With..."));
    gui->openWithMenu->setObjectName("openWithMenu");
    gui->openWithMenu->setEnabled(false);
    gui->menus.insert(gui->openWithMenu->objectName(), gui->openWithMenu);
    gui->openWithMenu->setIcon(QIcon(":/resources/openwith.png"));
    gui->openWithMenu->setStatusTip(tr("Open model file with selected application"));
    const int systemEditor = Preferences::systemEditor.isEmpty() ? 0 : 1;
    const int maxOpenWithPrograms = Preferences::maxOpenWithPrograms + systemEditor;
    for (int i = 0; i < maxOpenWithPrograms; i++) {
      if (i == Preferences::maxOpenWithPrograms)
          gui->openWithMenu->addSeparator();
      gui->openWithMenu->addAction(gui->openWithActList.at(i));
    }

    fileMenu->addAction(gui->getAct("openWorkingFolderAct.1"));
    fileMenu->addAction(gui->getAct("reloadFromDiskAct.1"));

    fileMenu->addAction(gui->getAct("saveAct.1"));
    fileMenu->addAction(gui->getAct("saveAsAct.1"));
    fileMenu->addAction(gui->getAct("saveCopyAct.1"));
    fileMenu->addAction(gui->getAct("closeFileAct.1"));

    QMenu *importMenu = fileMenu->addMenu("Import...");
    importMenu->setObjectName("importMenu");
    gui->menus.insert(importMenu->objectName(), importMenu);
    importMenu->setIcon(QIcon(":/resources/import.png"));
    importMenu->setStatusTip(tr("Import LEGO Digital Designer files and set inventory lists"));
    importMenu->addAction(gui->getAct("importLDDAct.1"));
    importMenu->addAction(gui->getAct("importSetInventoryAct.1"));
    importMenu->addSeparator();

    QMenu *exportMenu = fileMenu->addMenu("Export As...");
    exportMenu->setObjectName("exportMenu");
    gui->menus.insert(exportMenu->objectName(), exportMenu);
    exportMenu->setIcon(QIcon(":/resources/exportas.png"));
    exportMenu->setStatusTip(tr("Export model file images, objects or part lists"));
    exportMenu->addAction(gui->getAct("exportPngAct.1"));
    exportMenu->addAction(gui->getAct("exportJpgAct.1"));
#ifdef Q_OS_WIN
    exportMenu->addAction(gui->getAct("exportBmpAct.1"));
#endif
    exportMenu->addSeparator();
    exportMenu->addAction(gui->getAct("exportObjAct.1"));
    exportMenu->addAction(gui->getAct("exportStlAct.1"));
    exportMenu->addAction(gui->getAct("exportPovAct.1"));
    exportMenu->addAction(gui->getAct("exportColladaAct.1"));
    exportMenu->addAction(gui->getAct("export3dsAct.1"));
    exportMenu->addSeparator();
    exportMenu->addAction(gui->getAct("exportHtmlAct.1"));
    exportMenu->addAction(gui->getAct("exportHtmlStepsAct.1"));
    exportMenu->addAction(gui->getAct("exportBricklinkAct.1"));
    exportMenu->addAction(gui->getAct("exportCsvAct.1"));
    exportMenu->addSeparator();
    exportMenu->setDisabled(true);

    fileMenu->addAction(gui->getAct("printToFilePreviewAct.1"));
    fileMenu->addAction(gui->getAct("printToFileAct.1"));
    fileMenu->addAction(gui->getAct("exportAsPdfPreviewAct.1"));
    fileMenu->addAction(gui->getAct("exportAsPdfAct.1"));
    fileMenu->addSeparator();

    QMenu *loadStatusMenu = fileMenu->addMenu(tr("Load Status..."));
    loadStatusMenu->setObjectName("loadStatusMenu");
    gui->menus.insert(loadStatusMenu->objectName(), loadStatusMenu);
    loadStatusMenu->setIcon(QIcon(":/resources/loadstatus.png"));
    loadStatusMenu->setStatusTip(tr("Display the current model file load status and optionally recount parts"));
    loadStatusMenu->addAction(gui->getAct("loadStatusAct.1"));
    loadStatusMenu->addAction(gui->getAct("recountPartsAct.1"));
    loadStatusMenu->addSeparator();
    loadStatusMenu->setDisabled(true);

#ifndef QT_NO_CLIPBOARD
    fileMenu->addSeparator();
    fileMenu->addAction(gui->getAct("copyFileNameToClipboardAct.1"));
    fileMenu->addAction(gui->getAct("copyFilePathToClipboardAct.1"));
#endif
    fileMenu->addSeparator();

    QMenu *recentFileMenu = fileMenu->addMenu(tr("Recent Files..."));
    recentFileMenu->setObjectName("recentFileMenu");
    gui->menus.insert(recentFileMenu->objectName(), recentFileMenu);
    recentFileMenu->setIcon(QIcon(":/resources/recentfiles.png"));
    recentFileMenu->setStatusTip(tr("Open recent model file"));

    for (int i = 0; i < MAX_RECENT_FILES; i++) {
      recentFileMenu->addAction(gui->recentFilesActs[i]);
    }
    gui->recentFilesSeparatorAct = recentFileMenu->addSeparator();

    recentFileMenu->addSeparator();
    recentFileMenu->addAction(gui->getAct("clearRecentAct.1"));

    fileMenu->addSeparator();
    fileMenu->addAction(gui->getAct("exitAct.1"));

    menuBar()->addSeparator();

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->setObjectName("editMenu");
    gui->menus.insert(editMenu->objectName(), editMenu);

    QMenu *removeLPubFormatMenu = new QMenu(tr("Remove LPub Format..."), editMenu);
    removeLPubFormatMenu->setObjectName("removeLPubFormatMenu");
    gui->menus.insert(removeLPubFormatMenu->objectName(), removeLPubFormatMenu);
    removeLPubFormatMenu->setIcon(QIcon(":/resources/removelpubformat.png"));
    removeLPubFormatMenu->setStatusTip(tr("Remove LPub format from document, page, step and submodel"));
    removeLPubFormatMenu->addAction(gui->getAct("removeLPubFormatDocumentAct.1"));
    removeLPubFormatMenu->addSeparator();
    removeLPubFormatMenu->addAction(gui->getAct("removeLPubFormatPageAct.1"));
    removeLPubFormatMenu->addAction(gui->getAct("removeLPubFormatStepAct.1"));
    removeLPubFormatMenu->addAction(gui->getAct("removeLPubFormatSubmodelAct.1"));
    removeLPubFormatMenu->addAction(gui->getAct("removeLPubFormatBomAct.1"));
    removeLPubFormatMenu->addSeparator();
    removeLPubFormatMenu->addAction(gui->getAct("removeBuildModFormatAct.1"));
    removeLPubFormatMenu->addAction(gui->getAct("removeChildSubmodelFormatAct.1"));

    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);

    editMenu->addSeparator();

    editMenu->addAction(gui->getAct("insertCoverPageAct.1"));
    editMenu->addAction(gui->getAct("appendCoverPageAct.1"));
    editMenu->addAction(gui->getAct("insertNumberedPageAct.1"));
    editMenu->addAction(gui->getAct("appendNumberedPageAct.1"));
    editMenu->addAction(gui->getAct("deletePageAct.1"));
    editMenu->addAction(gui->getAct("addPictureAct.1"));
    editMenu->addAction(gui->getAct("addTextAct.1"));
    editMenu->addAction(gui->getAct("addBomAct.1"));
    editMenu->addAction(gui->getAct("cycleEachPageAct.1"));
    editMenu->addSeparator();

    editMenu->insertMenu(gui->getAct("cycleEachPageAct.1"), removeLPubFormatMenu);
    editMenu->insertSeparator(gui->getAct("cycleEachPageAct.1"));

    gui->viewMenu = menuBar()->addMenu(tr("&View"));
    gui->viewMenu->setObjectName("viewMenu");
    gui->menus.insert(gui->viewMenu->objectName(), gui->viewMenu);
    gui->viewMenu->addAction(gui->getAct("fitWidthAct.1"));
    gui->viewMenu->addAction(gui->getAct("fitVisibleAct.1"));
    gui->viewMenu->addAction(gui->getAct("actualSizeAct.1"));
    gui->viewMenu->addAction(gui->getAct("fitSceneAct.1"));
    gui->viewMenu->addAction(gui->getAct("bringToFrontAct.1"));
    gui->viewMenu->addAction(gui->getAct("sendToBackAct.1"));
    gui->viewMenu->addAction(gui->getAct("zoomInComboAct.1"));
    gui->viewMenu->addAction(gui->getAct("zoomOutComboAct.1"));
    gui->viewMenu->addAction(gui->getAct("sceneRulerComboAct.1"));
    gui->viewMenu->addAction(gui->getAct("sceneGuidesComboAct.1"));
    gui->viewMenu->addAction(gui->getAct("snapToGridComboAct.1"));
    gui->viewMenu->addSeparator();
    gui->viewMenu->addAction(gui->getAct("fullScreenViewAct.1"));

    QMenu *navigationMenu = menuBar()->addMenu(tr("&Navigation"));
    navigationMenu->setObjectName("navigationMenu");
    gui->menus.insert(navigationMenu->objectName(), navigationMenu);
    navigationMenu->addAction(gui->getAct("firstPageAct.1"));
    navigationMenu->addAction(gui->getAct("nextPageAct.1"));
    navigationMenu->addAction(gui->getAct("nextPageContinuousAct.1"));
    navigationMenu->addAction(gui->getAct("previousPageAct.1"));
    navigationMenu->addAction(gui->getAct("previousPageContinuousAct.1"));
    navigationMenu->addAction(gui->getAct("lastPageAct.1"));

    QMenu *configMenu = menuBar()->addMenu(tr("&Configuration"));
    configMenu->setObjectName("configMenu");
    gui->menus.insert(configMenu->objectName(), configMenu);
    configMenu->addAction(gui->getAct("preferencesAct.1"));
    configMenu->addSeparator();

    QMenu *setupMenu = configMenu->addMenu("Build &Instructions Setup...");
    setupMenu->setObjectName("setupMenu");
    gui->menus.insert(setupMenu->objectName(), setupMenu);
    setupMenu->setIcon(QIcon(":/resources/instructionsetup.png"));
    setupMenu->setStatusTip(tr("Instruction document global settings"));
    setupMenu->addAction(gui->getAct("pageSetupAct.1"));
    setupMenu->addAction(gui->getAct("assemSetupAct.1"));
    setupMenu->addAction(gui->getAct("pliSetupAct.1"));
    setupMenu->addAction(gui->getAct("bomSetupAct.1"));
    setupMenu->addAction(gui->getAct("calloutSetupAct.1"));
    setupMenu->addAction(gui->getAct("multiStepSetupAct.1"));
    setupMenu->addAction(gui->getAct("subModelSetupAct.1"));
    setupMenu->addAction(gui->getAct("projectSetupAct.1"));
    setupMenu->addAction(gui->getAct("fadeStepsSetupAct.1"));
    setupMenu->addAction(gui->getAct("highlightStepSetupAct.1"));
    setupMenu->addSeparator();
    setupMenu->setDisabled(true);

    QMenu *editorMenu = configMenu->addMenu("Edit Parameter Files...");
    editorMenu->setObjectName("editorMenu");
    gui->menus.insert(editorMenu->objectName(), editorMenu);
    editorMenu->setIcon(QIcon(":/resources/editparameterfiles.png"));
    editorMenu->setStatusTip(tr("Edit %1 parameter files").arg(VER_PRODUCTNAME_STR));
    editorMenu->addAction(gui->getAct("useSystemEditorAct.1"));
    editorMenu->addSeparator();
#if defined Q_OS_WIN
    if (Preferences::portableDistribution) {
      editorMenu->addAction(gui->getAct("editLPub3DIniFileAct.1"));
      editorMenu->addSeparator();
    }
#else
    editorMenu->addAction(gui->getAct("editLPub3DIniFileAct.1"));
    editorMenu->addSeparator();
#endif
    editorMenu->addAction(gui->getAct("editLDrawColourPartsAct.1"));
    editorMenu->addAction(gui->getAct("editPliControlFileAct.1"));
    editorMenu->addAction(gui->getAct("editTitleAnnotationsAct.1"));
    editorMenu->addAction(gui->getAct("editFreeFormAnnitationsAct.1"));
    editorMenu->addAction(gui->getAct("editPliBomSubstitutePartsAct.1"));
    editorMenu->addAction(gui->getAct("editExcludedPartsAct.1"));
    editorMenu->addAction(gui->getAct("editStickerPartsAct.1"));
    editorMenu->addAction(gui->getAct("editAnnotationStyleAct.1"));
    editorMenu->addAction(gui->getAct("editLD2BLCodesXRefAct.1"));
    editorMenu->addAction(gui->getAct("editLD2BLColorsXRefAct.1"));
    editorMenu->addAction(gui->getAct("editBLColorsAct.1"));
    editorMenu->addAction(gui->getAct("editBLCodesAct.1"));
    editorMenu->addAction(gui->getAct("editLD2RBColorsXRefAct.1"));
    editorMenu->addAction(gui->getAct("editLD2RBCodesXRefAct.1"));
    editorMenu->addSeparator();
    if (Preferences::blenderInstalled)
        editorMenu->addAction(gui->getAct("editBlenderParametersAct.1"));
    if (Preferences::blenderInstalled && !Preferences::blenderLDrawConfigFile.isEmpty())
        editorMenu->addAction(gui->getAct("editBlenderPreferencesAct.1"));
    editorMenu->addAction(gui->getAct("editNativePOVIniAct.1"));
    editorMenu->addAction(gui->getAct("editLdgliteIniAct.1"));
    editorMenu->addAction(gui->getAct("editLdviewIniAct.1"));
    editorMenu->addAction(gui->getAct("editLdviewPovIniAct.1"));
    editorMenu->addAction(gui->getAct("editPovrayIniAct.1"));
    editorMenu->addAction(gui->getAct("editPovrayConfAct.1"));
    editorMenu->addSeparator();
    editorMenu->addAction(gui->getAct("openParameterFileFolderAct.1"));
    editorMenu->addSeparator();

    configMenu->addAction(gui->getAct("editModelFileAct.1"));
    configMenu->addAction(gui->getAct("generateCustomColourPartsAct.1"));
    configMenu->addSeparator();
    configMenu->addAction(gui->getAct("openWithSetupAct.1"));

    configMenu->addSeparator();

    QMenu *cacheMenu = configMenu->addMenu("Reset Cache...");
    cacheMenu->setObjectName("cacheMenu");
    gui->menus.insert(cacheMenu->objectName(), cacheMenu);
    cacheMenu->setIcon(QIcon(":/resources/resetcache.png"));
    cacheMenu->setStatusTip(tr("Reset working caches"));
    cacheMenu->addAction(gui->getAct("clearAllCachesAct.1"));
    cacheMenu->addAction(gui->getAct("clearPLICacheAct.1"));
    cacheMenu->addAction(gui->getAct("clearBOMCacheAct.1"));
    cacheMenu->addAction(gui->getAct("clearSMICacheAct.1"));
    cacheMenu->addAction(gui->getAct("clearCSICacheAct.1"));
    cacheMenu->addAction(gui->getAct("clearTempCacheAct.1"));
    cacheMenu->addAction(gui->getAct("clearCustomPartCacheAct.1"));
    cacheMenu->addSeparator();
    cacheMenu->setDisabled(true);

    configMenu->addSeparator();
    configMenu->addAction(gui->getAct("ldrawSearchDirectoriesAct.1"));
    configMenu->addAction(gui->getAct("archivePartsOnDemandAct.1"));
    configMenu->addAction(gui->getAct("archivePartsOnLaunchAct.1"));
    configMenu->addSeparator();
    configMenu->addAction(gui->getAct("refreshLDrawUnoffPartsAct.1"));
    configMenu->addAction(gui->getAct("refreshLDrawOfficialPartsAct.1"));

    // Visual Editor

    if (Preferences::modeGUI)
        create3DMenus();

    // Help Menus

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->setObjectName("helpMenu");
    gui->menus.insert(helpMenu->objectName(), helpMenu);
    helpMenu->addAction(gui->getAct("viewLogAct.1"));
#ifndef DISABLE_UPDATE_CHECK
    helpMenu->addAction(gui->getAct("updateAppAct.1"));
#endif
    helpMenu->addAction(gui->getAct("visitHomepageAct.1"));
    helpMenu->addAction(gui->getAct("openTicketAct.1"));
    // Begin Jaco's code
    helpMenu->addAction(gui->getAct("onlineManualAct.1"));
    // End Jaco's code
    helpMenu->addSeparator();
    helpMenu->addAction(gui->getAct("commandsDialogAct.1"));
    helpMenu->addAction(gui->getAct("exportMetaCommandsAct.1"));
    helpMenu->addSeparator();
    // About Editor
    helpMenu->addAction(gui->getAct("aboutAct.1"));

    QMenu *previousPageContinuousMenu = new QMenu(tr("Continuous Page Previous"), gui);
    previousPageContinuousMenu->setObjectName("previousPageContinuousMenu");
    gui->menus.insert(previousPageContinuousMenu->objectName(), previousPageContinuousMenu);
    previousPageContinuousMenu->addAction(gui->getAct("previousPageContinuousAct.1"));

    QMenu *nextPageContinuousMenu = new QMenu(tr("Continuous Page Next"), gui);
    nextPageContinuousMenu->setObjectName("nextPageContinuousMenu");
    gui->menus.insert(nextPageContinuousMenu->objectName(), nextPageContinuousMenu);
    nextPageContinuousMenu->addAction(gui->getAct("nextPageContinuousAct.1"));

    QMenu *zoomSliderMenu = new QMenu(tr("Zoom Slider"),gui);
    zoomSliderMenu->setObjectName("zoomSliderMenu");
    gui->menus.insert(zoomSliderMenu->objectName(), zoomSliderMenu);
    zoomSliderMenu->addAction(gui->zoomSliderAct);

    QMenu *sceneRulerTrackingMenu = new QMenu(tr("Ruler Tracking"),gui);
    sceneRulerTrackingMenu->setObjectName("sceneRulerTrackingMenu");
    gui->menus.insert(sceneRulerTrackingMenu->objectName(), sceneRulerTrackingMenu);
    sceneRulerTrackingMenu->addAction(gui->getAct("hideRulerPageBackgroundAct.1"));
    sceneRulerTrackingMenu->addSeparator();
    sceneRulerTrackingMenu->addAction(gui->getAct("sceneRulerTrackingNoneAct.1"));
    sceneRulerTrackingMenu->addAction(gui->getAct("sceneRulerTrackingTickAct.1"));
    sceneRulerTrackingMenu->addAction(gui->getAct("sceneRulerTrackingLineAct.1"));
    sceneRulerTrackingMenu->addSeparator();
    sceneRulerTrackingMenu->addAction(gui->getAct("showTrackingCoordinatesAct.1"));

    QMenu *sceneGuidesMenu = new QMenu(tr("Scene Guides"),gui);
    sceneGuidesMenu->setObjectName("sceneGuidesMenu");
    gui->menus.insert(sceneGuidesMenu->objectName(), sceneGuidesMenu);
    sceneGuidesMenu->addAction(gui->getAct("sceneGuidesDashLineAct.1"));
    sceneGuidesMenu->addAction(gui->getAct("sceneGuidesSolidLineAct.1"));
    sceneGuidesMenu->addSeparator();
    sceneGuidesMenu->addAction(gui->getAct("sceneGuidesPosTLeftAct.1"));
    sceneGuidesMenu->addAction(gui->getAct("sceneGuidesPosTRightAct.1"));
    sceneGuidesMenu->addAction(gui->getAct("sceneGuidesPosCentreAct.1"));
    sceneGuidesMenu->addAction(gui->getAct("sceneGuidesPosBLeftAct.1"));
    sceneGuidesMenu->addAction(gui->getAct("sceneGuidesPosBRightAct.1"));
    sceneGuidesMenu->addSeparator();
    sceneGuidesMenu->addAction(gui->getAct("showGuidesCoordinatesAct.1"));

    QMenu *snapToGridMenu = new QMenu(tr("Snap to Grid"), gui);
    snapToGridMenu->setObjectName("snapToGridMenu");
    gui->menus.insert(snapToGridMenu->objectName(), snapToGridMenu);
    snapToGridMenu->addAction(gui->getAct("hideGridPageBackgroundAct.1"));
    snapToGridMenu->addSeparator();
    for (int actionIdx = GRID_SIZE_FIRST; actionIdx < GRID_SIZE_LAST; actionIdx++)
        snapToGridMenu->addAction(gui->snapGridActions[actionIdx]);
}

QToolBar *Gui::getToolBar(const QString &objectName)
{
    if (gui->toolbars.contains(objectName))
        return gui->toolbars.value(objectName);
    return nullptr;
}

void Gui::createToolBars()
{
    QSettings Settings;

    QToolBar *fileToolBar = addToolBar(tr("File Toolbar"));
    fileToolBar->setObjectName("fileToolBar");
    gui->toolbars.insert(fileToolBar->objectName(), fileToolBar);
    fileToolBar->addAction(gui->getAct("openAct.1"));
    fileToolBar->addAction(gui->getAct("reloadFromDiskAct.1"));
    fileToolBar->addAction(gui->getAct("saveAct.1"));
    fileToolBar->addAction(gui->getAct("saveAsAct.1"));
    //fileToolBar->addAction(gui->getAct("saveCopyAct.1"));
    fileToolBar->addAction(gui->getAct("closeFileAct.1"));

    fileToolBar->addAction(gui->getAct("printToFilePreviewAct.1"));
    fileToolBar->addAction(gui->getAct("printToFileAct.1"));
    fileToolBar->addAction(gui->getAct("exportAsPdfPreviewAct.1"));
    fileToolBar->addAction(gui->getAct("exportAsPdfAct.1"));

    QToolBar *importToolBar = addToolBar(tr("Import Toolbar"));
    importToolBar->setObjectName("importToolBar");
    gui->toolbars.insert(importToolBar->objectName(), importToolBar);
    importToolBar->addAction(gui->getAct("importLDDAct.1"));
    importToolBar->addAction(gui->getAct("importSetInventoryAct.1"));
    bool visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS, VIEW_IMPORT_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS, VIEW_IMPORT_TOOLBAR_KEY)).toBool();
    gui->getMenu("importMenu")->addAction(importToolBar->toggleViewAction());
    importToolBar->setVisible(visible);
    gui->connect (importToolBar, SIGNAL (visibilityChanged(bool)),
                      gui, SLOT (importToolBarVisibilityChanged(bool)));

    gui->exportToolBar = addToolBar(tr("Export Toolbar"));
    gui->exportToolBar->setObjectName("exportToolBar");
    gui->toolbars.insert(gui->exportToolBar->objectName(), gui->exportToolBar);
    gui->exportToolBar->addAction(gui->getAct("exportPngAct.1"));
    gui->exportToolBar->addAction(gui->getAct("exportJpgAct.1"));
#ifdef Q_OS_WIN
    gui->exportToolBar->addAction(gui->getAct("exportBmpAct.1"));
#endif
    gui->exportToolBar->addSeparator();
    gui->exportToolBar->addAction(gui->getAct("exportObjAct.1"));
    gui->exportToolBar->addAction(gui->getAct("exportStlAct.1"));
    gui->exportToolBar->addAction(gui->getAct("exportPovAct.1"));
    gui->exportToolBar->addAction(gui->getAct("exportColladaAct.1"));
    gui->exportToolBar->addSeparator();
    gui->exportToolBar->addAction(gui->getAct("exportHtmlAct.1"));
    gui->exportToolBar->addAction(gui->getAct("exportHtmlStepsAct.1"));
    gui->exportToolBar->addAction(gui->getAct("exportBricklinkAct.1"));
    gui->exportToolBar->addAction(gui->getAct("exportCsvAct.1"));
    visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS, VIEW_EXPORT_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS, VIEW_EXPORT_TOOLBAR_KEY)).toBool();
    gui->getMenu("exportMenu")->addAction(gui->exportToolBar->toggleViewAction());
    gui->exportToolBar->setVisible(visible);
    gui->connect (gui->exportToolBar, SIGNAL (visibilityChanged(bool)),
                      gui, SLOT (exportToolBarVisibilityChanged(bool)));

    QToolBar *loadStatusToolBar = addToolBar(tr("Load Status Toolbar"));
    loadStatusToolBar->setObjectName("loadStatusToolBar");
    gui->toolbars.insert(loadStatusToolBar->objectName(), loadStatusToolBar);
    loadStatusToolBar->addAction(gui->getAct("loadStatusAct.1"));
    loadStatusToolBar->addAction(gui->getAct("recountPartsAct.1"));
    visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS, VIEW_LOAD_STATUS_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS, VIEW_LOAD_STATUS_TOOLBAR_KEY)).toBool();
    gui->getMenu("loadStatusMenu")->addAction(loadStatusToolBar->toggleViewAction());
    loadStatusToolBar->setVisible(visible);
    gui->connect (loadStatusToolBar, SIGNAL (visibilityChanged(bool)),
                      gui, SLOT (loadStatusToolBarVisibilityChanged(bool)));

    QToolBar *undoredoToolBar = addToolBar(tr("UndoRedo Toolbar"));
    undoredoToolBar->setObjectName("undoredoToolBar");
    gui->toolbars.insert(undoredoToolBar->objectName(), undoredoToolBar);
    undoredoToolBar->addAction(gui->undoAct);
    undoredoToolBar->addAction(gui->redoAct);

    QToolBar *cacheToolBar = addToolBar(tr("Cache Toolbar"));
    cacheToolBar->setObjectName("cacheToolBar");
    gui->toolbars.insert(cacheToolBar->objectName(), cacheToolBar);
    cacheToolBar->addSeparator();
    cacheToolBar->addAction(gui->getAct("clearAllCachesAct.1"));
    cacheToolBar->addAction(gui->getAct("clearPLICacheAct.1"));
    cacheToolBar->addAction(gui->getAct("clearBOMCacheAct.1"));
    cacheToolBar->addAction(gui->getAct("clearSMICacheAct.1"));
    cacheToolBar->addAction(gui->getAct("clearCSICacheAct.1"));
    cacheToolBar->addAction(gui->getAct("clearTempCacheAct.1"));
    cacheToolBar->addAction(gui->getAct("clearCustomPartCacheAct.1"));
    visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_CACHE_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_CACHE_TOOLBAR_KEY)).toBool();
    gui->getMenu("cacheMenu")->addAction(cacheToolBar->toggleViewAction());
    cacheToolBar->setVisible(visible);
    gui->connect (cacheToolBar, SIGNAL (visibilityChanged(bool)),
                     gui, SLOT (cacheToolBarVisibilityChanged(bool)));

    QToolBar *setupToolBar = addToolBar(tr("Global Setup Toolbar"));
    setupToolBar->setObjectName("setupToolBar");
    gui->toolbars.insert(setupToolBar->objectName(), setupToolBar);
    setupToolBar->addAction(gui->getAct("pageSetupAct.1"));
    setupToolBar->addAction(gui->getAct("assemSetupAct.1"));
    setupToolBar->addAction(gui->getAct("pliSetupAct.1"));
    setupToolBar->addAction(gui->getAct("bomSetupAct.1"));
    setupToolBar->addAction(gui->getAct("calloutSetupAct.1"));
    setupToolBar->addAction(gui->getAct("multiStepSetupAct.1"));
    setupToolBar->addAction(gui->getAct("subModelSetupAct.1"));
    setupToolBar->addAction(gui->getAct("projectSetupAct.1"));
    setupToolBar->addAction(gui->getAct("fadeStepsSetupAct.1"));
    setupToolBar->addAction(gui->getAct("highlightStepSetupAct.1"));
    visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_SETUP_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_SETUP_TOOLBAR_KEY)).toBool();
    gui->getMenu("setupMenu")->addAction(setupToolBar->toggleViewAction());
    setupToolBar->setVisible(visible);
    gui->connect (setupToolBar, SIGNAL (visibilityChanged(bool)),
                     gui, SLOT (setupToolBarVisibilityChanged(bool)));

    QToolBar *editToolBar = addToolBar(tr("Edit Toolbar"));
    editToolBar->setObjectName("editToolBar");
    gui->toolbars.insert(editToolBar->objectName(), editToolBar);
    editToolBar->addAction(gui->getAct("insertCoverPageAct.1"));
    editToolBar->addAction(gui->getAct("appendCoverPageAct.1"));
    editToolBar->addAction(gui->getAct("insertNumberedPageAct.1"));
    editToolBar->addAction(gui->getAct("appendNumberedPageAct.1"));
    editToolBar->addAction(gui->getAct("deletePageAct.1"));
    editToolBar->addAction(gui->getAct("addPictureAct.1"));
    editToolBar->addAction(gui->getAct("addTextAct.1"));
    editToolBar->addAction(gui->getAct("addBomAct.1"));

    QToolBar *removeLPubFormatToolBar = addToolBar(tr("Remove LPub Format Toolbar"));
    removeLPubFormatToolBar->setObjectName("removeLPubFormatToolBar");
    gui->toolbars.insert(removeLPubFormatToolBar->objectName(), removeLPubFormatToolBar);
    removeLPubFormatToolBar->addAction(gui->getAct("removeLPubFormatDocumentAct.1"));
    removeLPubFormatToolBar->addAction(gui->getAct("removeLPubFormatPageAct.1"));
    removeLPubFormatToolBar->addAction(gui->getAct("removeLPubFormatStepAct.1"));
    removeLPubFormatToolBar->addAction(gui->getAct("removeLPubFormatSubmodelAct.1"));
    removeLPubFormatToolBar->addAction(gui->getAct("removeLPubFormatBomAct.1"));

    visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_REMOVE_LPUB_FORMAT_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_REMOVE_LPUB_FORMAT_TOOLBAR_KEY)).toBool();
    gui->getMenu("editMenu")->addAction(removeLPubFormatToolBar->toggleViewAction());
    removeLPubFormatToolBar->setVisible(visible);
    gui->connect (removeLPubFormatToolBar, SIGNAL (visibilityChanged(bool)),
                                gui, SLOT (removeLPubFormatToolBarVisibilityChanged(bool)));

    visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_EDIT_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_EDIT_TOOLBAR_KEY)).toBool();
    gui->getMenu("editMenu")->addAction(editToolBar->toggleViewAction());
    gui->getMenu("editMenu")->addAction(gui->editWindow->editToolBar->toggleViewAction());
    gui->getMenu("editMenu")->addAction(gui->editWindow->toolsToolBar->toggleViewAction());
    editToolBar->setVisible(visible);
    gui->connect (editToolBar, SIGNAL (visibilityChanged(bool)),
                    gui, SLOT (editToolBarVisibilityChanged(bool)));

    QToolBar *editParamsToolBar = addToolBar(tr("Edit Parameters Toolbar"));
    editParamsToolBar->setObjectName("editParamsToolBar");
    gui->toolbars.insert(editParamsToolBar->objectName(), editParamsToolBar);
    editParamsToolBar->addAction(gui->getAct("editModelFileAct.1"));
    editParamsToolBar->addSeparator();
    editParamsToolBar->addAction(gui->getAct("editLDrawColourPartsAct.1"));
    editParamsToolBar->addAction(gui->getAct("editPliControlFileAct.1"));
    editParamsToolBar->addAction(gui->getAct("editTitleAnnotationsAct.1"));
    editParamsToolBar->addAction(gui->getAct("editFreeFormAnnitationsAct.1"));
    editParamsToolBar->addAction(gui->getAct("editPliBomSubstitutePartsAct.1"));
    editParamsToolBar->addAction(gui->getAct("editExcludedPartsAct.1"));
    editParamsToolBar->addAction(gui->getAct("editStickerPartsAct.1"));
    editParamsToolBar->addAction(gui->getAct("editAnnotationStyleAct.1"));
    editParamsToolBar->addAction(gui->getAct("editLD2BLCodesXRefAct.1"));
    editParamsToolBar->addAction(gui->getAct("editLD2BLColorsXRefAct.1"));
    editParamsToolBar->addAction(gui->getAct("editBLColorsAct.1"));
    editParamsToolBar->addAction(gui->getAct("editBLCodesAct.1"));
    editParamsToolBar->addAction(gui->getAct("editLD2RBColorsXRefAct.1"));
    editParamsToolBar->addAction(gui->getAct("editLD2RBCodesXRefAct.1"));
    editParamsToolBar->addSeparator();
#if defined Q_OS_WIN
    if (Preferences::portableDistribution) {
        editParamsToolBar->addAction(gui->getAct("editLPub3DIniFileAct.1"));
        editParamsToolBar->addSeparator();
    }
#else
    editParamsToolBar->addAction(gui->getAct("editLPub3DIniFileAct.1"));
    editParamsToolBar->addSeparator();
#endif
    if (!Preferences::blenderExe.isEmpty())
        editParamsToolBar->addAction(gui->getAct("editBlenderParametersAct.1"));
    if (Preferences::blenderInstalled && !Preferences::blenderLDrawConfigFile.isEmpty())
        editParamsToolBar->addAction(gui->getAct("editBlenderPreferencesAct.1"));
    editParamsToolBar->addAction(gui->getAct("editNativePOVIniAct.1"));
    editParamsToolBar->addAction(gui->getAct("editLdgliteIniAct.1"));
    editParamsToolBar->addAction(gui->getAct("editLdviewIniAct.1"));
    editParamsToolBar->addAction(gui->getAct("editLdviewPovIniAct.1"));
    editParamsToolBar->addAction(gui->getAct("editPovrayIniAct.1"));
    editParamsToolBar->addAction(gui->getAct("editPovrayConfAct.1"));
    editParamsToolBar->addSeparator();
    editParamsToolBar->addAction(gui->getAct("generateCustomColourPartsAct.1"));
    editParamsToolBar->addSeparator();
    editParamsToolBar->addAction(gui->getAct("viewLogAct.1"));
    editParamsToolBar->addSeparator();
    editParamsToolBar->addAction(gui->getAct("openParameterFileFolderAct.1"));
    visible = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_EDITPARAMS_TOOLBAR_KEY)))
        visible = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_EDITPARAMS_TOOLBAR_KEY)).toBool();
    getMenu("editorMenu")->addAction(editParamsToolBar->toggleViewAction());
    editParamsToolBar->setVisible(visible);
    connect (editParamsToolBar, SIGNAL (visibilityChanged(bool)),
                          gui, SLOT (editParamsToolBarVisibilityChanged(bool)));

    gui->getAct("previousPageComboAct.1")->setMenu(getMenu("previousPageContinuousMenu"));
    gui->getAct("nextPageComboAct.1")->setMenu(getMenu("nextPageContinuousMenu"));

    QToolBar *navigationToolBar = addToolBar(tr("Navigation Toolbar"));
    navigationToolBar->setObjectName("navigationToolBar");
    gui->toolbars.insert(navigationToolBar->objectName(), navigationToolBar);
    navigationToolBar->addAction(gui->getAct("firstPageAct.1"));
    navigationToolBar->addSeparator();
    navigationToolBar->addAction(gui->getAct("previousPageComboAct.1"));
    navigationToolBar->addSeparator();
    navigationToolBar->addWidget(gui->setPageLineEdit);
    navigationToolBar->addSeparator();
    navigationToolBar->addAction(gui->getAct("nextPageComboAct.1"));
    navigationToolBar->addSeparator();
    navigationToolBar->addAction(gui->getAct("lastPageAct.1"));
    navigationToolBar->addSeparator();
    navigationToolBar->addWidget(gui->setGoToPageCombo);
    navigationToolBar->addSeparator();
    navigationToolBar->addWidget(gui->mpdCombo);

    QToolBar *zoomToolBar = addToolBar(tr("Zoom Toolbar"));
    zoomToolBar->setObjectName("zoomToolBar");
    gui->toolbars.insert(zoomToolBar->objectName(), zoomToolBar);
    zoomToolBar->addAction(gui->getAct("fitVisibleAct.1"));
    zoomToolBar->addAction(gui->getAct("fitWidthAct.1"));
    zoomToolBar->addAction(gui->getAct("fitSceneAct.1"));
    zoomToolBar->addAction(gui->getAct("actualSizeAct.1"));

    zoomToolBar->addAction(gui->getAct("bringToFrontAct.1"));
    zoomToolBar->addAction(gui->getAct("sendToBackAct.1"));

    gui->getAct("zoomInComboAct.1")->setMenu(getMenu("zoomSliderMenu"));
    zoomToolBar->addAction(gui->getAct("zoomInComboAct.1"));
    gui->getAct("zoomOutComboAct.1")->setMenu(getMenu("zoomSliderMenu"));
    zoomToolBar->addAction(gui->getAct("zoomOutComboAct.1"));

    gui->getAct("sceneRulerComboAct.1")->setMenu(getMenu("sceneRulerTrackingMenu"));
    zoomToolBar->addAction(gui->getAct("sceneRulerComboAct.1"));

    gui->getAct("sceneGuidesComboAct.1")->setMenu(getMenu("sceneGuidesMenu"));
    zoomToolBar->addAction(gui->getAct("sceneGuidesComboAct.1"));

    gui->getAct("snapToGridComboAct.1")->setMenu(getMenu("snapToGridMenu"));
    zoomToolBar->addAction(gui->getAct("snapToGridComboAct.1"));

    zoomToolBar->addSeparator();
    zoomToolBar->addAction(gui->getAct("fullScreenViewAct.1"));

    if (Preferences::modeGUI)
        gui->create3DToolBars();
}

void Gui::statusBarMsg(QString msg)
{
  gui->statusBar()->showMessage(msg);
}

void Gui::createDockWindows()
{
    gui->commandEditDockWindow = new QDockWidget(tr("Command Editor"), gui);
    gui->commandEditDockWindow->setObjectName("CommandEditorDockWindow");
    gui->commandEditDockWindow->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    gui->commandEditDockWindow->setWidget(gui->editWindow);
    gui->addDockWidget(Qt::RightDockWidgetArea, gui->commandEditDockWindow);
    gui->getMenu("viewMenu")->addAction(gui->commandEditDockWindow->toggleViewAction());

    gui->connect(gui->commandEditDockWindow, SIGNAL (topLevelChanged(bool)), gui, SLOT (enableWindowFlags(bool)));

    if (Preferences::modeGUI)
        gui->create3DDockWindows();
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

void Gui::loadStatusToolBarVisibilityChanged(bool visible)
{
    QSettings Settings;
    Settings.setValue(QString("%1/%2").arg(SETTINGS,VIEW_LOAD_STATUS_TOOLBAR_KEY),visible);
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
    readVisualEditorSettings(Settings);
    resize(size);
    Settings.endGroup();

    Settings.beginGroup(MESSAGES);
    QByteArray data = Settings.value("MessagesNotShown", QByteArray()).toByteArray();
    if (!data.isEmpty()) {
        QDataStream dataStreamRead(data);
        dataStreamRead >> Preferences::messagesNotShown;
    }
    Settings.endGroup();
}

void Gui::writeSettings()
{
    QSettings Settings;
    Settings.beginGroup(MAINWINDOW);
    Settings.setValue("Geometry", saveGeometry());
    Settings.setValue("State", saveState());
    writeVisualEditorSettings(Settings);
    Settings.setValue("Size", size());
    Settings.endGroup();

    gApplication->SaveTabLayout();

    Settings.beginGroup(MESSAGES);
    QByteArray data;
    QDataStream dataStreamWrite(&data, QIODevice::WriteOnly);
    dataStreamWrite << Preferences::messagesNotShown;
    QVariant uValue(data);
    Settings.setValue("MessagesNotShown",uValue);
    Settings.endGroup();

    // remove tab layouts greater than 10 entries
    Settings.beginGroup(TABLAYOUTS);
    const QStringList tabLayoutKeys = Settings.allKeys();
    if (tabLayoutKeys.size() > MAX_TABLAYOUTS) {
        const int retainedTabLayouts = tabLayoutKeys.size() - MAX_TABLAYOUTS;
        for (int i = 0; i < tabLayoutKeys.size(); i++) {
            if (i < retainedTabLayouts) {
                Settings.remove(tabLayoutKeys.at(i));
            }
        }
    }
    Settings.endGroup();
}

void Gui::showLine(const Where &here, int type)
{
  if (Preferences::modeGUI && ! Gui::exporting()) {
    if (gui->macroNesting == 0) {
      gui->displayFile(&lpub->ldrawFile, here);
      emit gui->showLineSig(here.lineNumber, type);
    }
  }
}

void Gui::parseError(const QString &message,
                     const Where &here,
                     Preferences::MsgKey msgKey,
                     bool option/*false*/,
                     bool override/*false*/,
                     int icon/*NoIcon*/,
                     const QString &title,
                     const QString &type)
{
    if (Gui::parsedMessages.contains(here))
        return;

    const QString keyType[][2] = {
       // Message Title,              Type Description
       {"LPub Meta Command",       tr("LPub command parse error")},         //ParseErrors
       {"Insert Meta",             tr("insert parse error")},               //InsertErrors
       {"Include File Meta",       tr("include file parse error")},         //IncludeFileErrors
       {"Build Modification Meta", tr("build modification parse error")},   //BuildModErrors
       {"Build Modification Edit", tr("build modification editing error")}, //BuildModEditErrors
       {"Annoatation Meta",        tr("annotation configuration error")}    //AnnotationErrors
    };

    QString parseMessage = tr("%1 (file: %2, line: %3)") .arg(message) .arg(here.modelName) .arg(here.lineNumber + 1);

    int abortProcess = 0;

    bool abortInProgress = Gui::abortProcess();

    bool guiEnabled = Preferences::modeGUI && Preferences::lpub3dLoaded;

    QMessageBox::Icon messageIcon = static_cast<QMessageBox::Icon>(icon);

    bool abort = (messageIcon == QMessageBox::Icon::Critical || messageIcon == QMessageBox::Icon::NoIcon) && msgKey < Preferences::BuildModEditErrors;

    if (guiEnabled && !abortInProgress) {
        if ((!Gui::exporting() && !Gui::ContinuousPage()) || ((Gui::exporting() || Gui::ContinuousPage()) && Preferences::displayPageProcessingErrors)) {
            if (Gui::pageProcessRunning == PROC_FIND_PAGE || Gui::pageProcessRunning == PROC_DRAW_PAGE)
                showLine(here, LINE_ERROR);
            bool okToShowMessage = Preferences::getShowMessagePreference(msgKey);
            if (okToShowMessage) {
                Where messageLine = here;
                const QString msgTitle = title.isEmpty() ? keyType[msgKey][0] : title;
                const QString msgType = type.isEmpty() ? keyType[msgKey][1] : type;
                messageLine.setModelIndex(getSubmodelIndex(messageLine.modelName));
                Preferences::MsgID msgID(msgKey,messageLine.indexToString());
                abortProcess = Preferences::showMessage(msgID, parseMessage, msgTitle, msgType, option, override, icon) == QMessageBox::Abort;
                if (abortProcess)
                    Gui::setAbortProcess(abortProcess);
            } else
                Gui::setAbortProcess(abort);
            if (Gui::pageProcessRunning == PROC_WRITE_TO_TMP)
                emit gui->progressPermMessageSig(tr("Writing submodel [Parse Error%1")
                                               .arg(okToShowMessage ? "]...          " : " - see log]... " ));
        } else {
            const QString status(messageIcon == QMessageBox::Icon::Warning
                                     ? "<FONT COLOR='#FFBF00'>WARNING</FONT>: "
                                     : messageIcon == QMessageBox::Icon::Critical
                                           ? "<FONT COLOR='#FF0000'>ERROR</FONT>: "
                                           : "");
            Gui::messageList << QString("%1%2<br>").arg(status).arg(parseMessage);
        }
    } else if (!guiEnabled) {
        Gui::setAbortProcess(abort);
    }

    if (!abortProcess && !abortInProgress) {
        Gui::parsedMessages.append(here);
        abortProcess = abort;
        if (messageIcon == QMessageBox::Icon::NoIcon)
            messageIcon = QMessageBox::Icon::Critical;
    }

    // Set Logging settings
    Preferences::setMessageLogging(DEFAULT_LOG_LEVEL);

    Preferences::fprintMessage(parseMessage, messageIcon == QMessageBox::Icon::Critical ? true : false/*stdError*/);

    switch (messageIcon) {
    case QMessageBox::Icon::Information:
        if (Preferences::loggingEnabled)
            logInfo() << qPrintable(parseMessage.replace("<br>"," "));
        break;
    case QMessageBox::Icon::Warning:
        if (Preferences::loggingEnabled)
            logWarning() << qPrintable(parseMessage.replace("<br>"," "));
        break;
    case QMessageBox::Icon::Question:
        if (Preferences::loggingEnabled)
            logNotice() << qPrintable(parseMessage.replace("<br>"," "));
        break;
    case QMessageBox::Icon::Critical:
        if (Preferences::loggingEnabled)
            logError() << qPrintable(parseMessage.replace("<br>"," "));
        if (abortProcess) {
            Gui::displayPageNum = Gui::prevDisplayPageNum;
            if (Gui::exporting()) {             // exporting
                emit gui->setExportingSig(false);
            } else if (Gui::ContinuousPage()) { // continuous page processing
                emit gui->setContinuousPageSig(false);
                while (Gui::pageProcessRunning != PROC_NONE) {
                    QTime waiting = QTime::currentTime().addMSecs(500);
                    while (QTime::currentTime() < waiting)
                        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                }
            }
        }
        break;
    default:
        break;
    }

    // Reset Logging to default settings
    Preferences::setMessageLogging();
}

void Gui::statusMessage(LogType logType, const QString &statusMessage, int msgBox/*0=false*/) {
    /* logTypes
     * LOG_STATUS:   - same as INFO but writes to log file also
     * LOG_INFO:
     * LOG_TRACE:
     * LOG_DEBUG:
     * LOG_NOTICE:
     * LOG_WARNING:
     * LOG_ERROR:
     * LOG_FATAL:
     */
    QString message = statusMessage;

    bool abortProcess = false;

    bool abortInProgress = Gui::abortProcess();

    bool guiEnabled = Preferences::modeGUI && Preferences::lpub3dLoaded;

    Preferences::setMessageLogging(DEFAULT_LOG_LEVEL);

    Preferences::fprintMessage(message, logType > LOG_WARNING ? true : false/*stdError*/);

    if (logType == LOG_INFO_STATUS) {

        if (Preferences::loggingEnabled)
            logInfo() << qPrintable(message.replace("<br>"," "));

        if (guiEnabled) {
            gui->statusBarMsg(message);
            if (msgBox && !abortInProgress && !Gui::ContinuousPage() && !Gui::exporting())
                QMessageBox::information(gui,tr("%1 Info Status").arg(VER_PRODUCTNAME_STR),message);
        }
    } else
    if (logType == LOG_STATUS) {

         if (Preferences::loggingEnabled)
             logStatus() << qPrintable(message.replace("<br>"," "));

         if (guiEnabled) {
             gui->statusBarMsg(message);
             if (msgBox && !abortInProgress && !Gui::ContinuousPage() && !Gui::exporting())
                 QMessageBox::information(gui,tr("%1 Status").arg(VER_PRODUCTNAME_STR),message);
         }
    } else
    if (logType == LOG_INFO) {

        if (Preferences::loggingEnabled)
            logInfo() << qPrintable(message.replace("<br>"," "));

        if (guiEnabled && msgBox) {
            if (Gui::ContinuousPage() || Gui::exporting()) {
                gui->statusBarMsg(QString(message).replace("<br>"," ").prepend("INFO: "));
            } else if (!abortInProgress) {
                QMessageBox::information(gui,tr("%1 Information").arg(VER_PRODUCTNAME_STR),message);
            }
        }
    } else
    if (logType == LOG_NOTICE) {

        if (Preferences::loggingEnabled)
            logNotice() << qPrintable(message.replace("<br>"," "));

        if (guiEnabled && msgBox) {
            if (Gui::ContinuousPage() || Gui::exporting()) {
                gui->statusBarMsg(QString(message).replace("<br>"," ").prepend("NOTICE: "));
            } else if (!abortInProgress) {
                QMessageBox::information(gui,tr("%1 Notice").arg(VER_PRODUCTNAME_STR),message);
            }
        }
    } else
    if (logType == LOG_TRACE) {

        if (Preferences::loggingEnabled)
            logTrace() << qPrintable(message.replace("<br>"," "));

        if (guiEnabled && msgBox) {
            if (Gui::ContinuousPage() || Gui::exporting()) {
                gui->statusBarMsg(QString(message).replace("<br>"," ").prepend("TRACE: "));
            } else if (!abortInProgress) {
                QMessageBox::information(gui,tr("%1 Trace").arg(VER_PRODUCTNAME_STR),message);
            }
        }
    } else
    if (logType == LOG_DEBUG) {

        if (Preferences::loggingEnabled)
            logDebug() << qPrintable(message.replace("<br>"," "));

        if (guiEnabled && msgBox) {
            if (Gui::ContinuousPage() || Gui::exporting()) {
                gui->statusBarMsg(QString(message).replace("<br>"," ").prepend("DEBUG: "));
            } else if (!abortInProgress) {
                QMessageBox::information(gui,tr("%1 Debug").arg(VER_PRODUCTNAME_STR),message);
            }
        }
    } else
    if (logType == LOG_WARNING) {

        if (Preferences::loggingEnabled)
            logWarning() << qPrintable(QString(message).replace("<br>"," "));

        if (guiEnabled) {
            if (Gui::ContinuousPage() || Gui::exporting() || msgBox == 0/*false*/) {
                Gui::messageList << QString("<FONT COLOR='#FFBF00'>WARNING</FONT>: %1<br>").arg(message);
                gui->statusBarMsg(QString(message).replace("<br>"," ").prepend("WARNING: "));
            } else if (((!Gui::exporting() && !Gui::ContinuousPage()) || Preferences::displayPageProcessingErrors) && msgBox != 2 && !abortInProgress) {
                QMessageBox::warning(gui,tr("%1 Warning").arg(VER_PRODUCTNAME_STR),message);
            }
        }
    } else
    if (logType == LOG_ERROR) {

        if (Preferences::loggingEnabled)
            logError() << qPrintable(QString(message).replace("<br>"," "));

        if (guiEnabled) {
            if ((Gui::ContinuousPage() || Gui::exporting()) && ! Preferences::displayPageProcessingErrors) {
                Gui::messageList << QString("<FONT COLOR='#FF0000'>ERROR</FONT>: %1<br>").arg(message);
                gui->statusBarMsg(QString(message).replace("<br>"," ").prepend("ERROR: "));
            } else if (!abortInProgress) {
                Gui::setAbortProcess(true);
                if (Gui::pageProcessRunning == PROC_NONE) {
                    QMessageBox::critical(gui,tr("%1 Error").arg(VER_PRODUCTNAME_STR),message,QMessageBox::Ok,QMessageBox::Ok);
                } else {
                    abortProcess = QMessageBox::critical(gui,tr("%1 Error").arg(VER_PRODUCTNAME_STR),message,
                                                         QMessageBox::Abort | QMessageBox::Ignore,QMessageBox::Ignore) == QMessageBox::Abort;
                    Gui::setAbortProcess(abortProcess);
                }
            }
        }
    } else
    if (logType == LOG_FATAL) {

        if (Preferences::loggingEnabled)
            logFatal() << qPrintable(QString(message).replace("<br>"," "));

        Preferences::setMessageLogging();
        Gui::setAbortProcess(true);
        emit gui->setExportingSig(false);
        emit gui->setContinuousPageSig(false);

        if (guiEnabled) {
            QApplication::restoreOverrideCursor();
            if (QMessageBox::critical(gui,tr("%1 Fatal Error").arg(VER_PRODUCTNAME_STR),message.append(tr("<br><br>Restart %1 ? ").arg(VER_PRODUCTNAME_STR)),
                                      QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
                Gui::displayPageNum = 1;
                gui->restartApplication(false, false);
            }
        }
    }

    // reset logging to default settings
    Preferences::setMessageLogging();

    if (logType == LOG_ERROR && abortProcess) {
        Gui::displayPageNum = Gui::prevDisplayPageNum;
        if (Gui::exporting()) {             // exporting
            emit gui->setExportingSig(false);
        } else if (Gui::ContinuousPage()) { // continuous page processing
            emit gui->setContinuousPageSig(false);
            while (Gui::pageProcessRunning != PROC_NONE) {
                QTime waiting = QTime::currentTime().addMSecs(500);
                while (QTime::currentTime() < waiting)
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
            }
            if (Gui::pageProcessRunning == PROC_NONE) {
                Gui::setAbortProcess(false);
                current = Gui::topOfPages.last();
                Gui::buildModJumpForward = false;
                Gui::maxPages = Gui::prevMaxPages;
                gui->pagesCounted();
            }
        } else {                     // processing a page
            if (Gui::pageProcessRunning == PROC_NONE) {
                if (Gui::displayPageNum > (1 + Gui::pa)) {
                    gui->restorePreviousPage();
                } else {
                    Gui::setAbortProcess(false);
                    current  = Where(lpub->ldrawFile.topLevelFile(),0,0);
                    Gui::buildModJumpForward = false;
                    Gui::maxPages = 1 + Gui::pa;
                    gui->pagesCounted();
                }
            }
        }
    }
}

/******************************
 * LDraw Search Directory Dialog
 */

void LDrawSearchDirDialog::getLDrawSearchDirDialog()
{
  QPalette readOnlyPalette = QApplication::palette();
  if (Preferences::darkTheme)
      readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DARK_PALETTE_MIDLIGHT]));
  else
      readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DEFAULT_PALETTE_LIGHT]));
  readOnlyPalette.setColor(QPalette::Text,QColor(LPUB3D_DISABLED_TEXT_COLOUR));

  dialog = new QDialog(nullptr);

  dialog->setWindowTitle(tr("LDraw Search Directories"));

  dialog->setWhatsThis(lpubWT(WT_DIALOG_LDRAW_SEARCH_DIRECTORIES,dialog->windowTitle()));

  QVBoxLayout *layout = new QVBoxLayout();

  dialog->setLayout(layout);

  QGridLayout *gridLayout = new QGridLayout();

  QVBoxLayout *actionsLayout = new QVBoxLayout();

  QString ldrawSearchDirsTitle = tr("LDraw Content Search Directories for %1").arg(Preferences::validLDrawPartsLibrary);

  QGroupBox *searchDirsGrpBox = new QGroupBox(ldrawSearchDirsTitle,dialog);

  QGroupBox *groupBoxActions = new QGroupBox(dialog);

  QSpacerItem *actionsSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);

  layout->addWidget(searchDirsGrpBox);

  searchDirsGrpBox->setLayout(gridLayout);

  groupBoxActions->setLayout(actionsLayout);

  lineEditIniFile = new QLineEdit(dialog);
  lineEditIniFile->setPalette(readOnlyPalette);
  lineEditIniFile->setReadOnly(true);
  gridLayout->addWidget(lineEditIniFile,0,0,1,2);

  textEditSearchDirs = new TextEditSearchDirs(dialog);
  textEditSearchDirs->setLineWrapMode(QPlainTextEdit::NoWrap);
  gridLayout->addWidget(textEditSearchDirs,1,0,2,1);

  gridLayout->addWidget(groupBoxActions, 1,1,2,1);

  pushButtonAddDirectory = new QPushButton(dialog);
  pushButtonAddDirectory->setToolTip(tr("Add LDraw search directory"));
  pushButtonAddDirectory->setIcon(QIcon(QIcon(":/resources/adddirectory.png")));
  actionsLayout->addWidget(pushButtonAddDirectory);

  pushButtonOpenFolder = new QPushButton(dialog);
  pushButtonOpenFolder->setToolTip(tr("Open the selected LDraw search directory"));
  pushButtonOpenFolder->setIcon(QIcon(QIcon(":/resources/openworkingfolder.png")));
  actionsLayout->addWidget(pushButtonOpenFolder);

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
  pushButtonReset->setToolTip(tr("Reset %1 LDraw search directories to default").arg(VER_PRODUCTNAME_STR));
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
      for (const QString &searchDir : Preferences::ldSearchDirs)
        textEditSearchDirs->appendPlainText(searchDir);
  }

  const QString ldrawPath = Preferences::ldrawLibPath;
  excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(ldrawPath).arg("parts"));
  excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(ldrawPath).arg("p"));
  if (Preferences::usingDefaultLibrary) {
    excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(ldrawPath).arg("unofficial/parts"));
    excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(ldrawPath).arg("unofficial/p"));
  }

  connect(textEditSearchDirs, SIGNAL(textChanged()),gui, SLOT(buttonSetState()));
  connect(pushButtonMoveUp, SIGNAL(clicked()),gui, SLOT(buttonClicked()));
  connect(pushButtonMoveDown, SIGNAL(clicked()),gui, SLOT(buttonClicked()));
  connect(pushButtonAddDirectory, SIGNAL(clicked()),gui, SLOT(buttonClicked()));
  connect(pushButtonOpenFolder, SIGNAL(clicked()),gui, SLOT(buttonClicked()));

  QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                             Qt::Horizontal, dialog);

  layout->addWidget(&buttonBox);

  connect(&buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
  connect(&buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

  textEditSearchDirs->moveCursor(QTextCursor::Start,QTextCursor::MoveAnchor);

  dialog->setMinimumWidth(500);

  dialog->setSizeGripEnabled(true);

  if (dialog->exec() == QDialog::Accepted) {

    QStringList searchDirs = textEditSearchDirs->toPlainText().split("\n");

    if (searchDirs.size() && searchDirs != Preferences::ldSearchDirs) {
      QStringList validDirs, invalidDirs, newDirs;

      for (const QString &searchDir : searchDirs) {

        bool match = false;
        const QString dir = QDir::toNativeSeparators(searchDir);

        for (const QString &ldDir : Preferences::ldSearchDirs) {
          if (dir.toLower() == ldDir.toLower()) {
            match = true;
            break;
          }
        }

        if (match) {
          validDirs << dir;
        } else {
          match = false;
          for (const QString &excludedDir : excludedSearchDirs) {
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
        const QString message = tr("The search directory list contains excluded or invalid paths which will not be saved.");
        QMessageBox::warning(dialog, tr("Search Directories"), QString("%1<br>%2").arg(message).arg(invalidDirs.join("<br>")));
        emit gui->messageSig(LOG_INFO, message);
        for (const QString &dir : invalidDirs) {
          emit gui->messageSig(LOG_INFO, QString("    - %1").arg(dir));
        }
      }

      if (!validDirs.isEmpty()) {
        emit gui->messageSig(LOG_INFO, tr("Updated LDraw Directories:"));
        for (const QString &dir : validDirs) {
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
    box.setText (tr("This action will reset your search directory settings to the %1 default.\n"
                    "Are you sure you want to continue ? ").arg(VER_PRODUCTNAME_STR));
    if (box.exec() == QMessageBox::Yes) {
      gui->partWorkerLDSearchDirs.resetSearchDirSettings();
      textEditSearchDirs->clear();

      for (const QString &searchDir : Preferences::ldSearchDirs)
        textEditSearchDirs->appendPlainText(searchDir);

      box.setIcon (QMessageBox::Information);
      box.setStandardButtons (QMessageBox::Ok);
      box.setText( tr("Search directories have been reset with %1 entries.").arg(Preferences::ldSearchDirs.size()));
      emit gui->messageSig(LOG_STATUS,box.text());
      box.exec();
    }
  } else if (sender() == pushButtonAddDirectory) {
    const QString ldrawPath = Preferences::ldrawLibPath;
    const QString result = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(dialog, tr("Select Parts Directory"), QString("%1/%2").arg(ldrawPath).arg("unofficial")));
    for (const QString &excludedDir : excludedSearchDirs) {
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
      textEditSearchDirs->appendPlainText(QDir::toNativeSeparators(result));
  } else if (sender() == pushButtonOpenFolder) {
    QTextCursor cursor = textEditSearchDirs->textCursor();
    cursor.select(QTextCursor::LineUnderCursor);
    QTextDocumentFragment selection = cursor.selection();
    if (selection.isEmpty())
      return;
    QDir folder(selection.toPlainText());
    if (folder.isReadable())
      gui->openFolderSelect(QDir::toNativeSeparators(QString("%1/a").arg(folder.absolutePath())));
    else
      emit gui->messageSig(LOG_WARNING, tr("Selected folder is not readable<br>%1").arg(folder.absolutePath()), true/*msgBox*/);
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

TextEditSearchDirs::TextEditSearchDirs(QWidget *parent) :
    QPlainTextEdit(parent),
    lineNumberArea(new EditLineNumberArea(this))
{
  QPalette lineNumberPalette = lineNumberArea->palette();
  lineNumberPalette.setCurrentColorGroup(QPalette::Active);
  lineNumberPalette.setColor(QPalette::Highlight,QColor(Qt::magenta));
  if (Preferences::darkTheme) {
    lineNumberPalette.setColor(QPalette::Text,QColor(Qt::darkGray).darker(150));
    lineNumberPalette.setColor(QPalette::Background,QColor(Preferences::themeColors[THEME_DARK_EDIT_MARGIN]));
  } else {
    lineNumberPalette.setColor(QPalette::Text,QColor(Qt::darkGray));
    lineNumberPalette.setColor(QPalette::Background,QColor(Preferences::themeColors[THEME_DEFAULT_PALETTE_LIGHT]).lighter(130));
  }
  lineNumberArea->setPalette(lineNumberPalette);

  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

  updateLineNumberAreaWidth(0);
  highlightCurrentLine();
}

int TextEditSearchDirs::lineNumberAreaWidth()
{
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
  }

  QFont font = lineNumberArea->font();
  const QFontMetrics linefmt(font);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
  int space = 10 + linefmt.horizontalAdvance(QLatin1Char('9')) * digits;
#else
  int space = 10 + linefmt.width(QLatin1Char('9')) * digits;
#endif

  return space;
}

void TextEditSearchDirs::updateLineNumberAreaWidth(int /* newBlockCount */)
{
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void TextEditSearchDirs::updateLineNumberArea(const QRect &rect, int dy)
{
  if (dy)
    lineNumberArea->scroll(0, dy);
  else
    lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

  if (rect.contains(viewport()->rect()))
    updateLineNumberAreaWidth(0);
}

void TextEditSearchDirs::resizeEvent(QResizeEvent *e)
{
  QPlainTextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TextEditSearchDirs::highlightCurrentLine()
{
  QList<QTextEdit::ExtraSelection> extraSelections;

  if (!isReadOnly()) {
    QTextEdit::ExtraSelection selection;

    QColor lineColor;
    if (Preferences::darkTheme)
        lineColor = QColor(Preferences::themeColors[THEME_DARK_LINE_HIGHLIGHT]);
    else
        lineColor = QColor(Preferences::themeColors[THEME_DEFAULT_LINE_HIGHLIGHT]);

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
  }

  setExtraSelections(extraSelections);
}

void TextEditSearchDirs::lineNumberAreaPaintEvent(QPaintEvent *event)
{
  QPainter painter(lineNumberArea);

  int selStart = textCursor().selectionStart();
  int selEnd = textCursor().selectionEnd();

  QPalette palette = lineNumberArea->palette();

  painter.fillRect(event->rect(), palette.color(QPalette::Background));

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  qreal top = blockBoundingGeometry(block).translated(contentOffset()).top();
  qreal bottom = top;

  while (block.isValid() && top <= event->rect().bottom()) {
    top = bottom;

    const qreal height = blockBoundingRect(block).height();
    bottom = top + height;

    if (block.isVisible() && bottom >= event->rect().top()) {
      painter.setPen(palette.color(QPalette::Text));

      bool selected = (
          (selStart < block.position() + block.length() && selEnd > block.position())
          || (selStart == selEnd && selStart == block.position())
          );

      if (selected) {
        painter.save();
        painter.setPen(palette.color(QPalette::Highlight));
      }

      const QString number = QString::number(blockNumber + 1);
      painter.drawText(0, top, lineNumberArea->width() - 4, height, Qt::AlignRight, number);

      if (selected)
        painter.restore();
    }

    block = block.next();
    ++blockNumber;
  }
}
/*******************************/

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
