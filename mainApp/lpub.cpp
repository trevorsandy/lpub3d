/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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
#include <QSizePolicy>
#include <QFileDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QCloseEvent>
#include <QUndoStack>
#include <QTextStream>
#include <QWidget>

#include "QPushButton"
#include "QHBoxLayout"
#include "QVBoxLayout"

#include "lpub.h"
#include "editwindow.h"
#include "paths.h"
#include "globals.h"
#include "resolution.h"
#include "lpub_preferences.h"
#include "render.h"
#include "metaitem.h"
#include "ranges_element.h"

#include "step.h"
//** 3D
#include "camera.h"
#include "piece.h"
#include "lc_mainwindow.h"
//**

Gui *gui;

void clearPliCache()
{
  gui->clearPLICache();
}

void clearCsiCache()
{
  gui->clearCSICache();
}

void clearCsi3dCache()
{
    gui->clearCSI3DCache();
}

void clearAllCache()
{
    gui->clearALLCache();
}

/****************************************************************************
 * 
 * The Gui constructor and destructor are at the bottom of the file with
 * the code that creates the basic GUI framekwork 
 *
 ***************************************************************************/

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
  ++displayPageNum;
  displayPage();  // display the page we just added
}

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
  if (macroNesting == 0) {
    clearPage(KpageView,KpageScene);
    page.coverPage = false;
    drawPage(KpageView,KpageScene,false);
    enableActions2();
    Step::isCsiDataModified = false; //reset
  }
}

void Gui::nextPage()
{
  countPages();
  if (displayPageNum < maxPages) {
    ++displayPageNum;
    displayPage();
  } else {
    statusBarMsg("You're on the last page");
  }
}

void Gui::prevPage()
{
  if (displayPageNum > 1) {
    displayPageNum--;
    displayPage();
  }
}

void Gui::firstPage()
{
  displayPageNum = 1;
  displayPage();
}

void Gui::clearAndRedrawPage()
{
  clearCSICache();
  clearPLICache();
  clearALLCache();
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
  QRegExp rx("^(\\d+)\\s+.*$");
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
      }
    }
  }
  string = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);
  setPageLineEdit->setText(string);
}

void Gui::fitWidth()
{
  fitWidth(pageview());
}

void Gui::fitWidth(
  LGraphicsView *view)
{
  view->scale(1.0,1.0);

  QRectF rect(0,0,page.meta.LPub.page.size.valuePixels(0),page.meta.LPub.page.size.valuePixels(1));

  QRectF unity = view->matrix().mapRect(QRectF(0,0,1,1));
  view->scale(1/unity.width(), 1 / unity.height());

  int margin = 2;
  QRectF viewRect = view->viewport()->rect().adjusted(margin, margin, -margin, -margin);
  QRectF sceneRect = view->matrix().mapRect(rect);
  qreal xratio = viewRect.width() / sceneRect.width();

  view->scale(xratio,xratio);
  view->centerOn(rect.center());
  fitMode = FitWidth;
}

void Gui::fitVisible()
{
  fitVisible(pageview());
}

void Gui::fitVisible(
  LGraphicsView *view)
{
  view->scale(1.0,1.0);

  QRectF rect(0,0,page.meta.LPub.page.size.valuePixels(0),page.meta.LPub.page.size.valuePixels(1));

  QRectF unity = view->matrix().mapRect(QRectF(0,0,1,1));
  view->scale(1/unity.width(), 1 / unity.height());

  int margin = 2;
  QRectF viewRect = view->viewport()->rect().adjusted(margin, margin, -margin, -margin);
  QRectF sceneRect = view->matrix().mapRect(rect);
  qreal xratio = viewRect.width() / sceneRect.width();
  qreal yratio = viewRect.height() / sceneRect.height();

  xratio = yratio = qMin(xratio,yratio);
  view->scale(xratio,yratio);
  view->centerOn(rect.center());
  fitMode = FitVisible;
}

void Gui::actualSize()
{
  actualSize(pageview());
}

void Gui::actualSize(
  LGraphicsView *view)
{
  view->resetMatrix();
  fitMode = FitNone;
}

void Gui::zoomIn()
{
  zoomIn(pageview());
}

void Gui::zoomIn(
  LGraphicsView *view)
{
  fitMode = FitNone;
  view->scale(1.1,1.1);
}

void Gui::zoomOut()
{
  zoomOut(pageview());
}

void Gui::zoomOut(
  LGraphicsView *view)
{
  fitMode = FitNone;
  view->scale(1.0/1.1,1.0/1.1);
}

void Gui::UpdateStepRotation()
{
    mModelStepRotation = lcVector3(mRotStepAngleX, mRotStepAngleY, mRotStepAngleZ);

    QString rotLabel("Step Rotation %1 %2 %3");
    rotLabel = rotLabel.arg(QString::number(mModelStepRotation[0], 'f', 2),
                            QString::number(mModelStepRotation[1], 'f', 2),
                            QString::number(mModelStepRotation[2], 'f', 2));
    statusBarMsg(rotLabel);
}


void Gui::displayFile(
  LDrawFile     *ldrawFile, 
  const QString &modelName)
{
//  if (force || modelName != curSubFile) {
    for (int i = 0; i < mpdCombo->count(); i++) {
      if (mpdCombo->itemText(i) == modelName) {
        mpdCombo->setCurrentIndex(i);
        break;
      }
    }
    curSubFile = modelName;
    displayFileSig(ldrawFile, modelName);
//  }
}

/*-----------------------------------------------------------------------------*/

void Gui::mpdComboChanged(int index)
{
  index = index;
  QString newSubFile = mpdCombo->currentText();
  if (curSubFile != newSubFile) {
    curSubFile = newSubFile;
    displayFileSig(&ldrawFile, curSubFile);
  }
}

void Gui::clearPLICache()
{
  QString dirName = QDir::currentPath() + "/" + Paths::partsDir;
  QDir dir(dirName);

  dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

  QFileInfoList list = dir.entryInfoList();
  for (int i = 0; i < list.size(); i++) {
    QFileInfo fileInfo = list.at(i);
    QFile     file(dirName + "/" + fileInfo.fileName());
    file.remove();
  }
}

void Gui::clearCSICache()
{
  QString dirName = QDir::currentPath() + "/" + Paths::assemDir;
  QDir dir(dirName);

  dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

  QFileInfoList list = dir.entryInfoList();
  for (int i = 0; i < list.size(); i++) {
    QFileInfo fileInfo = list.at(i);
    QFile     file(dirName + "/" + fileInfo.fileName());
    file.remove();
  }
}

void Gui::clearCSI3DCache()
{
    QString tmpDirName = QDir::currentPath() + "/" + Paths::tmpDir;
    QDir tmpDir(tmpDirName);

    tmpDir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    QFileInfoList tmpDirList = tmpDir.entryInfoList();
    for (int i = 0; i < tmpDirList.size(); i++) {
        QFileInfo fileInfo = tmpDirList.at(i);
        QFile     file(tmpDirName + "/" + fileInfo.fileName());
        file.remove();
    }

    QString viewDirName = QDir::currentPath() + "/" + Paths::viewerDir;
    QDir viewDir(viewDirName);

    viewDir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    QFileInfoList viewDirList = viewDir.entryInfoList();
    for (int i = 0; i < viewDirList.size(); i++) {
        QFileInfo fileInfo = viewDirList.at(i);
        QFile     file(viewDirName + "/" + fileInfo.fileName());
        file.remove();
    }

}

void Gui::clearALLCache()
{
    clearPLICache();
    clearCSICache();
    clearCSI3DCache();
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

void Gui::editFreeFormAnnitations()
{
    parmsEditor.parmsOpen(mFreeFormAnno);
    parmsEditor.setWindowTitle(tr("Freeform Annotation","Edit/add freeform part annotations"));
    parmsEditor.show();

//    QWidget *window = new QWidget;
//    window->setWindowTitle(tr("Freeform Annotation","Edit/add freeform part annotations"));

//    QPushButton *button1 = new QPushButton("Save");
//    QPushButton *button2 = new QPushButton("Exit");

//    QHBoxLayout *hlayout = new QHBoxLayout;
//    //QHBoxLayout *vlayout = new QVBoxLayout;

//    hlayout->addWidget(QWidget(parmsEditor));
//    hlayout->addWidget(button1);
//    hlayout->addWidget(button2);

//    window->setLayout(hlayout);
//    window.show();
}

void Gui::editFadeColourParts()
{
    parmsEditor.parmsOpen(mColourPartFade);
    parmsEditor.setWindowTitle(tr("Fade Colour Parts","Edit/add static coulour parts for fade"));
    parmsEditor.show();

}

void Gui::preferences()
{
  if (Preferences::getPreferences()) {
    Meta meta;
    page.meta = meta;

    Step::isCsiDataModified = true;

    QString renderer = Render::getRenderer();
    Render::setRenderer(Preferences::preferredRenderer);
    if (Render::getRenderer() != renderer) {
        gui->clearCSICache();
        gui->clearPLICache();
        gui->clearCSI3DCache();
    }

    if (Preferences::enableFadeStep)
        processFadeColorParts();

    QString topLevel = ldrawFile.topLevelFile();
    GlobalPliDialog *pliParms = new GlobalPliDialog(topLevel, page.meta, false);
    pliParms->accept();
    displayPage();
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
    Preferences::renderPreferences();
	Preferences::lgeoPreferences();
    Preferences::pliPreferences();
    Preferences::publishingPreferences();

    displayPageNum = 1;

    editWindow    = new EditWindow();
    KpageScene    = new QGraphicsScene(this);
    KpageScene->setBackgroundBrush(Qt::lightGray);
    KpageView     = new LGraphicsView(KpageScene);
    KpageView->pageBackgroundItem = NULL;
    KpageView->setRenderHints(QPainter::Antialiasing | 
                             QPainter::TextAntialiasing |
                             QPainter::SmoothPixmapTransform);
    setCentralWidget(KpageView);

    mpdCombo = new QComboBox;
    mpdCombo->setEditable(false);
    mpdCombo->setMinimumContentsLength(25);
    mpdCombo->setInsertPolicy(QComboBox::InsertAtBottom);
    mpdCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);

    connect(mpdCombo,SIGNAL(activated(int)),
            this,    SLOT(mpdComboChanged(int)));

    mExistingRotStep = lcVector3(0.0f, 0.0f, 0.0f);
    mModelStepRotation = lcVector3(0.0f, 0.0f, 0.0f);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    createDockWindows();
    toggleLCStatusBar();

    readSettings();

    undoStack = new QUndoStack();
    macroNesting = 0;

    connect(this,       SIGNAL(displayFileSig(LDrawFile *, const QString &)),
            editWindow, SLOT(  displayFile   (LDrawFile *, const QString &)));
    connect(this,       SIGNAL(showLineSig(int)),
            editWindow, SLOT(  showLine(   int)));

    connect(editWindow, SIGNAL(contentsChange(const QString &,int,int,const QString &)),
            this,       SLOT(  contentsChange(const QString &,int,int,const QString &)));

    connect(editWindow, SIGNAL(redrawSig()),
            this,       SLOT(  clearAndRedrawPage()));

    connect(undoStack,  SIGNAL(canRedoChanged(bool)),
            this,       SLOT(  canRedoChanged(bool)));
    connect(undoStack,  SIGNAL(canUndoChanged(bool)),
            this,       SLOT(  canUndoChanged(bool)));
    connect(undoStack,  SIGNAL(cleanChanged(bool)),
            this,       SLOT(  cleanChanged(bool)));

#ifdef WATCHER
    connect(&watcher,   SIGNAL(fileChanged(const QString &)),
             this,      SLOT(  fileChanged(const QString &)));
#endif
    setCurrentFile("");
    // Jaco: This sets the initial size of the main window
    resize(QSize(1000, 600));

    gui = this;

    fitMode = FitVisible;

#ifdef __APPLE__
    extern void qt_mac_set_native_menubar(bool);
    qt_mac_set_native_menubar(true);
#endif

    Preferences::getRequireds();
    Render::setRenderer(Preferences::preferredRenderer);

    mFreeFormAnno     = 1;
    mColourPartFade   = 2;
}

Gui::~Gui()
{
    delete KpageScene;
    delete KpageView;
    delete editWindow;
}


void Gui::closeEvent(QCloseEvent *event)
{
  writeSettings();

  if (maybeSave()) {
    event->accept();

    QSettings Settings;
    Settings.beginGroup(WINDOW);
    Settings.setValue("Geometry", saveGeometry());
    Settings.setValue("State", saveState());
    Settings.endGroup();

  } else {
    event->ignore();
  }
}

bool Gui::aboutDialog()
{
    AboutDialog Dialog(this, NULL);
    return Dialog.exec() == QDialog::Accepted;
}

bool Gui::updateDialog()
{
    UpdateDialog Dialog(this, NULL);
    return Dialog.exec() == QDialog::Accepted;
}

// Begin Jaco's code
// Danny: web url changed, as it pointed nowhere

#include <QDesktopServices>
#include <QUrl>

void Gui::onlineManual()
{
    QDesktopServices::openUrl(QUrl("http://sites.google.com/site/workingwithlpub/"));
}

// End Jaco's code


void Gui::meta()
{
  Meta meta;
  QStringList doc;

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
    QMessageBox::warning(NULL,
    QMessageBox::tr(VER_PRODUCTNAME_STR),
    QMessageBox::tr("Cannot write file %1:\n%2.")
    .arg(fileName)
    .arg(file.errorString()));
    return;
  }

  QTextStream out(&file);

  for (int i = 0; i < doc.size(); i++) {
    out << doc[i] << endl;
  }
  file.close();
}

void Gui::createActions()
{
    openAct = new QAction(QIcon(":/resources/open.png"), tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/resources/save.png"), tr("&Save"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the document to disk"));
    saveAct->setEnabled(false);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(QIcon(":/resources/saveas.png"),tr("Save &As..."), this);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    saveAsAct->setEnabled(false);
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    printToFileAct = new QAction(QIcon(":/resources/pdf_logo.png"), tr("Print to &File"), this);
    printToFileAct->setShortcut(tr("Ctrl+F"));
    printToFileAct->setStatusTip(tr("Print your document to a file"));
    printToFileAct->setEnabled(false);
    connect(printToFileAct, SIGNAL(triggered()), this, SLOT(printToFile()));

    exportPngAct = new QAction(tr("Export As &PNG Images"), this);
    exportPngAct->setShortcut(tr("Ctrl+Shift+P"));
    exportPngAct->setStatusTip(tr("Export your document as a sequence of PNG images"));
    exportPngAct->setEnabled(false);
    connect(exportPngAct, SIGNAL(triggered()), this, SLOT(exportAsPng()));

    exportJpgAct = new QAction(tr("Export As &JPEG Images"), this);
    exportJpgAct->setShortcut(tr("Ctrl+J"));
    exportJpgAct->setStatusTip(tr("Export your document as a sequence of JPEG images"));
    exportJpgAct->setEnabled(false);
    connect(exportJpgAct, SIGNAL(triggered()), this, SLOT(exportAsJpg()));

    exportBmpAct = new QAction(tr("Export As &Bitmap Images"), this);
    exportBmpAct->setShortcut(tr("Ctrl+B"));
    exportBmpAct->setStatusTip(tr("Export your document as a sequence of bitmap images"));
    exportBmpAct->setEnabled(false);
    connect(exportBmpAct, SIGNAL(triggered()), this, SLOT(exportAsBmp()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    for (int i = 0; i < MaxRecentFiles; i++) {
      recentFilesActs[i] = new QAction(this);
      recentFilesActs[i]->setVisible(false);
      connect(recentFilesActs[i], SIGNAL(triggered()), this, 
                                 SLOT(openRecentFile()));
    }

    // undo/redo

    undoAct = new QAction(QIcon(":/resources/editundo.png"), tr("Undo"), this);
    undoAct->setShortcut(tr("Ctrl+Z"));
    undoAct->setStatusTip(tr("Undo last change"));
    connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));
    redoAct = new QAction(QIcon(":/resources/editredo.png"), tr("Redo"), this);
#ifdef __APPLE__
    redoAct->setShortcut(tr("Ctrl+Shift+Z"));
#else
    redoAct->setShortcut(tr("Ctrl+Y"));
#endif
    redoAct->setStatusTip(tr("Redo last change"));
    connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));

    insertCoverPageAct = new QAction(tr("Insert Cover Page"),this);
    insertCoverPageAct->setStatusTip(tr("Insert an unnumbered page"));
    insertCoverPageAct->setEnabled(false);
    connect(insertCoverPageAct, SIGNAL(triggered()), this, SLOT(insertCoverPage()));


    appendCoverPageAct = new QAction(tr("Append Cover Page"),this);
    appendCoverPageAct->setStatusTip(tr("Append a numbered page"));
    appendCoverPageAct->setEnabled(false);
    connect(appendCoverPageAct, SIGNAL(triggered()), this, SLOT(appendCoverPage()));

    insertNumberedPageAct = new QAction(tr("Insert Page"),this);
    insertNumberedPageAct->setStatusTip(tr("Insert a numbered page"));
    insertNumberedPageAct->setEnabled(false);
    connect(insertNumberedPageAct, SIGNAL(triggered()), this, SLOT(insertNumberedPage()));

    appendNumberedPageAct = new QAction(tr("Append Page"),this);
    appendNumberedPageAct->setStatusTip(tr("Append a numbered page"));
    appendNumberedPageAct->setEnabled(false);
    connect(appendNumberedPageAct, SIGNAL(triggered()), this, SLOT(appendNumberedPage()));

    deletePageAct = new QAction(tr("Delete Page"),this);
    deletePageAct->setStatusTip(tr("Delete this page"));
    deletePageAct->setEnabled(false);
    connect(deletePageAct, SIGNAL(triggered()), this, SLOT(deletePage()));

    addPictureAct = new QAction(tr("Add Picture"),this);
    addPictureAct->setStatusTip(tr("Add a picture to this page"));
    addPictureAct->setEnabled(false);
    connect(addPictureAct, SIGNAL(triggered()), this, SLOT(addPicture()));

    addTextAct = new QAction(tr("Add Text"),this);
    addTextAct->setStatusTip(tr("Add text to this page"));
    addTextAct->setEnabled(false);
    connect(addTextAct, SIGNAL(triggered()), this, SLOT(addText()));

    addBomAct = new QAction(tr("Add Bill of Materials"),this);
    addBomAct->setStatusTip(tr("Add Bill of Materials to this page"));
    addBomAct->setEnabled(false);
    connect(addBomAct, SIGNAL(triggered()), this, SLOT(addBom()));

    removeLPubFormattingAct = new QAction(tr("Remove LPub Formatting"),this);
    removeLPubFormattingAct->setStatusTip(tr("Remove all LPub metacommands from all files"));
    removeLPubFormattingAct->setEnabled(false);
    connect(removeLPubFormattingAct, SIGNAL(triggered()), this, SLOT(removeLPubFormatting()));

    // fitWidth,fitVisible,actualSize

    fitWidthAct = new QAction(QIcon(":/resources/fitWidth.png"), tr("Fit Width"), this);
    fitWidthAct->setShortcut(tr("Ctrl+W"));
    fitWidthAct->setStatusTip(tr("Fit document to width"));
    connect(fitWidthAct, SIGNAL(triggered()), this, SLOT(fitWidth()));

    fitVisibleAct = new QAction(QIcon(":/resources/fitVisible.png"), tr("Fit Visible"), this);
    fitVisibleAct->setShortcut(tr("Ctrl+I"));
    fitVisibleAct->setStatusTip(tr("Fit document so whole page is visible"));
    connect(fitVisibleAct, SIGNAL(triggered()), this, SLOT(fitVisible()));

    actualSizeAct = new QAction(QIcon(":/resources/actual.png"),tr("Actual Size"), this);
    actualSizeAct->setShortcut(tr("Ctrl+A"));
    actualSizeAct->setStatusTip(tr("Show document actual size"));
    connect(actualSizeAct, SIGNAL(triggered()), this, SLOT(actualSize()));
    
    // zoomIn,zoomOut

    zoomInAct = new QAction(QIcon(":/resources/zoomin.png"), tr("&Zoom In"), this);
    zoomInAct->setShortcut(tr("Ctrl++"));
    zoomInAct->setStatusTip(tr("Zoom in"));
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(QIcon(":/resources/zoomout.png"),tr("Zoom &Out"),this);
    zoomOutAct->setShortcut(tr("Ctrl+-"));
    zoomOutAct->setStatusTip(tr("Zoom out"));
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    // firstPage,lastPage,nextPage,previousPage

    firstPageAct = new QAction(QIcon(":/resources/first.png"),tr("First Page"), this);
    firstPageAct->setShortcut(tr("Ctrl+F"));
    firstPageAct->setStatusTip(tr("Go to first page of document"));
    connect(firstPageAct, SIGNAL(triggered()), this, SLOT(firstPage()));

    lastPageAct = new QAction(QIcon(":/resources/last.png"),tr("Last Page"), this);
    lastPageAct->setShortcut(tr("Ctrl+L"));
    lastPageAct->setStatusTip(tr("Go to last page of document"));
    connect(lastPageAct, SIGNAL(triggered()), this, SLOT(lastPage()));

    nextPageAct = new QAction(QIcon(":/resources/next.png"),tr("&Next Page"),this);
    nextPageAct->setShortcut(tr("Ctrl+N"));
    nextPageAct->setStatusTip(tr("Go to next page of document"));
    connect(nextPageAct, SIGNAL(triggered()), this, SLOT(nextPage()));

    previousPageAct = new QAction(QIcon(":/resources/prev.png"),tr("&Previous Page"),this);
    previousPageAct->setShortcut(tr("Ctrl+P"));
    previousPageAct->setStatusTip(tr("Go to previous page of document"));
    connect(previousPageAct, SIGNAL(triggered()), this, SLOT(prevPage()));

    QString pageString = "";
    setPageLineEdit = new QLineEdit(pageString,this);
    QSize size = setPageLineEdit->sizeHint();
    size.setWidth(size.width()/3);
    setPageLineEdit->setMinimumSize(size);
    connect(setPageLineEdit, SIGNAL(returnPressed()), this, SLOT(setPage()));

    clearPLICacheAct = new QAction(tr("Clear Parts List Cache"), this);
    clearPLICacheAct->setStatusTip(tr("Erase the parts list image cache"));
    connect(clearPLICacheAct, SIGNAL(triggered()), this, SLOT(clearPLICache()));

    clearCSICacheAct = new QAction(tr("Clear Assembly Image Cache"), this);
    clearCSICacheAct->setStatusTip(tr("Erase the assembly image cache"));
    connect(clearCSICacheAct, SIGNAL(triggered()), this, SLOT(clearCSICache()));

    clearCSI3DCacheAct = new QAction(tr("Clear 3D Viewer Image Cache"), this);
    clearCSI3DCacheAct->setStatusTip(tr("Erase the 3D viewer image cache"));
    connect(clearCSI3DCacheAct, SIGNAL(triggered()), this, SLOT(clearCSI3DCache()));

    clearALLCacheAct = new QAction(tr("Clear All Caches"), this);
    clearALLCacheAct->setStatusTip(tr("Erase all caches"));
    connect(clearALLCacheAct, SIGNAL(triggered()), this, SLOT(clearALLCache()));

    // Config menu

    pageSetupAct = new QAction(tr("Page Setup"), this);
    pageSetupAct->setEnabled(false);
    pageSetupAct->setStatusTip(tr("Default values for your project's pages"));
    connect(pageSetupAct, SIGNAL(triggered()), this, SLOT(pageSetup()));

    assemSetupAct = new QAction(tr("Assembly Setup"), this);
    assemSetupAct->setEnabled(false);
    assemSetupAct->setStatusTip(tr("Default values for your project's assembly images"));
    connect(assemSetupAct, SIGNAL(triggered()), this, SLOT(assemSetup()));

    pliSetupAct = new QAction(tr("Parts List Setup"), this);
    pliSetupAct->setEnabled(false);
    pliSetupAct->setStatusTip(tr("Default values for your project's parts lists"));
    connect(pliSetupAct, SIGNAL(triggered()), this, SLOT(pliSetup()));

    bomSetupAct = new QAction(tr("Bill of Materials Setup"), this);
    bomSetupAct->setEnabled(false);
    bomSetupAct->setStatusTip(tr("Default values for your project's bill of materials"));
    connect(bomSetupAct, SIGNAL(triggered()), this, SLOT(bomSetup()));

    calloutSetupAct = new QAction(tr("Callout Setup"), this);
    calloutSetupAct->setEnabled(false);
    calloutSetupAct->setStatusTip(tr("Default values for your project's callouts"));
    connect(calloutSetupAct, SIGNAL(triggered()), this, SLOT(calloutSetup()));

    multiStepSetupAct = new QAction(tr("Step Group Setup"), this);
    multiStepSetupAct->setEnabled(false);
    multiStepSetupAct->setStatusTip(tr("Default values for your project's step groups"));
    connect(multiStepSetupAct, SIGNAL(triggered()), this, SLOT(multiStepSetup()));

    projectSetupAct = new QAction(tr("Project Setup"), this);
    projectSetupAct->setEnabled(false);
    projectSetupAct->setStatusTip(tr("Default values for your project"));
    connect(projectSetupAct, SIGNAL(triggered()), this, SLOT(projectSetup()));

    fadeStepSetupAct = new QAction(tr("Fade Step Setup"), this);
    fadeStepSetupAct->setEnabled(false);
    fadeStepSetupAct->setStatusTip(tr("Fade all parts not in the current step"));
    connect(fadeStepSetupAct, SIGNAL(triggered()), this, SLOT(fadeStepSetup()));

    preferencesAct = new QAction(tr("Preferences"), this);
    preferencesAct->setStatusTip(tr("Set your preferences for LPub"));
    connect(preferencesAct, SIGNAL(triggered()), this, SLOT(preferences()));

    editFreeFormAnnitationsAct = new QAction(tr("Edit Freeform Annotations"), this);
    editFreeFormAnnitationsAct->setStatusTip(tr("Add/Edit freeform part annotations"));
    connect(editFreeFormAnnitationsAct, SIGNAL(triggered()), this, SLOT(editFreeFormAnnitations()));

    editFadeColourPartsAct = new QAction(tr("Edit Fade Coloured Parts"), this);
    editFadeColourPartsAct->setStatusTip(tr("Add/Edit static coloured parts to fade-parts list"));
    connect(editFadeColourPartsAct, SIGNAL(triggered()), this, SLOT(editFadeColourParts()));

    // Help

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(aboutDialog()));

    // Begin Jaco's code

    onlineManualAct = new QAction(tr("&Online manual"), this);
    onlineManualAct->setStatusTip(tr("Visit the Online Manual Website."));
    connect(onlineManualAct, SIGNAL(triggered()), this, SLOT(onlineManual()));

    // End Jaco's code

    metaAct = new QAction(tr("&Save LPub Metacommands to File"), this);
    metaAct->setStatusTip(tr("Save a list of the known LPub meta commands to a file"));
    connect(metaAct, SIGNAL(triggered()), this, SLOT(meta()));

    updateApp = new QAction(tr("Check for &Updates..."), this);
    updateApp->setStatusTip(tr("Check if a newer LPub3D version is available for download"));
    connect(updateApp, SIGNAL(triggered()), this, SLOT(updateDialog()));
}

void Gui::enableActions()
{
    saveAct->setEnabled(true);
    saveAsAct->setEnabled(true);
    printToFileAct->setEnabled(true);
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
    addPictureAct->setEnabled(true);
    removeLPubFormattingAct->setEnabled(true);
    editFreeFormAnnitationsAct->setEnabled(true);
    editFadeColourPartsAct->setEnabled(true);

}

void Gui::enableActions2()
{
    MetaItem mi;
    insertCoverPageAct->setEnabled(mi.okToInsertCoverPage());
    appendCoverPageAct->setEnabled(mi.okToAppendCoverPage());
    bool frontCover = mi.okToInsertNumberedPage();
    insertNumberedPageAct->setEnabled(frontCover);
    bool backCover = mi.okToAppendNumberedPage();
    appendNumberedPageAct->setEnabled(backCover);
    deletePageAct->setEnabled(page.list.size() == 0);
    addBomAct->setEnabled(frontCover||backCover);
    addTextAct->setEnabled(true);
}

void Gui::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);

    QMenu *exportMenu = fileMenu->addMenu("Export As...");
    exportMenu->addAction(exportPngAct);
    exportMenu->addAction(exportJpgAct);
    exportMenu->addAction(exportBmpAct);

#ifndef __APPLE__
    exportMenu->addAction(exportBmpAct);
#endif

    fileMenu->addAction(printToFileAct);
    separatorAct = fileMenu->addSeparator();
    for (int i = 0; i < MaxRecentFiles; i++) {
      fileMenu->addAction(recentFilesActs[i]);
    }
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    menuBar()->addSeparator();

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addAction(insertCoverPageAct);
    editMenu->addAction(appendCoverPageAct);
    editMenu->addAction(insertNumberedPageAct);
    editMenu->addAction(appendNumberedPageAct);
    editMenu->addAction(deletePageAct);
    editMenu->addAction(addPictureAct);
    editMenu->addAction(addTextAct);
    editMenu->addAction(addBomAct);
    editMenu->addAction(removeLPubFormattingAct);

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(fitWidthAct);
    viewMenu->addAction(fitVisibleAct);
    viewMenu->addAction(actualSizeAct);
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addSeparator();

    toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(firstPageAct);
    toolsMenu->addAction(previousPageAct);
    toolsMenu->addAction(nextPageAct);
    toolsMenu->addAction(lastPageAct);
    toolsMenu->addSeparator();
    toolsMenu->addAction(clearPLICacheAct);
    toolsMenu->addAction(clearCSICacheAct);
    toolsMenu->addAction(clearCSI3DCacheAct);
    toolsMenu->addAction(clearALLCacheAct);

    configMenu = menuBar()->addMenu(tr("&Configuration"));
    configMenu->addAction(pageSetupAct);
    configMenu->addAction(assemSetupAct);
    configMenu->addAction(pliSetupAct);
    configMenu->addAction(bomSetupAct);
    configMenu->addAction(calloutSetupAct);
    configMenu->addAction(multiStepSetupAct);
    configMenu->addAction(projectSetupAct);
    configMenu->addAction(fadeStepSetupAct);
    configMenu->addSeparator();
    configMenu->addAction(editFreeFormAnnitationsAct);
    configMenu->addAction(editFadeColourPartsAct);
    configMenu->addSeparator();
    configMenu->addAction(preferencesAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));

    helpMenu->addAction(aboutAct);

    // Begin Jaco's code

    helpMenu->addAction(onlineManualAct);

    // End Jaco's code

    helpMenu->addAction(metaAct);

#if !DISABLE_UPDATE_CHECK
    helpMenu->addAction(updateApp);
#endif
}

void Gui::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->setObjectName("FileToolbar");
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    fileToolBar->addAction(printToFileAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->setObjectName("EditToolbar");
    editToolBar->addAction(undoAct);
    editToolBar->addAction(redoAct);

    navigationToolBar = addToolBar(tr("Navigation"));
    navigationToolBar->setObjectName("NavigationToolbar");
    navigationToolBar->addAction(firstPageAct);
    navigationToolBar->addAction(previousPageAct);
    navigationToolBar->addWidget(setPageLineEdit);
    navigationToolBar->addAction(nextPageAct);
    navigationToolBar->addAction(lastPageAct);

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

    if(!modelDockWindow->isFloating())
    statusBarMsg(gMainWindow->mLCStatusBar->currentMessage());
}

void Gui::createDockWindows()
{
    fileEditDockWindow = new QDockWidget(tr("LDraw File Editor"), this);
    fileEditDockWindow->setObjectName("LDrawFileDockWindow");
    fileEditDockWindow->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    fileEditDockWindow->setWidget(editWindow);
    addDockWidget(Qt::RightDockWidgetArea, fileEditDockWindow);
    viewMenu->addAction(fileEditDockWindow->toggleViewAction());
//**3D
    modelDockWindow = new QDockWidget(tr("3D Viewer - by LeoCAD"), this);
    modelDockWindow->setObjectName("ModelDockWindow");
    modelDockWindow->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    modelDockWindow->setWidget(gMainWindow);
    addDockWidget(Qt::RightDockWidgetArea, modelDockWindow);
    viewMenu->addAction(modelDockWindow->toggleViewAction());
//**
    tabifyDockWidget(modelDockWindow, fileEditDockWindow);
    modelDockWindow->raise();

    connect(modelDockWindow, SIGNAL (topLevelChanged(bool)), this, SLOT (toggleLCStatusBar()));
}

void Gui::toggleLCStatusBar(){

    if(modelDockWindow->isFloating())
        gMainWindow->statusBar()->show();
    else
        gMainWindow->statusBar()->hide();
}

void Gui::readSettings()
{
    QSettings Settings;
    Settings.beginGroup(WINDOW);
    restoreState(Settings.value("State").toByteArray());
    restoreGeometry(Settings.value("Geometry").toByteArray());
    QSize size = Settings.value("size", QSize(800, 600)).toSize();
    QPoint pos = Settings.value("pos", QPoint(200, 200)).toPoint();
    resize(size);
    move(pos);
    Settings.endGroup();
}

void Gui::writeSettings()
{
    QSettings Settings;
    Settings.beginGroup(WINDOW);
    Settings.setValue("pos", pos());
    Settings.setValue("size", size());
    Settings.endGroup();
}

/*
 * Insert static coloured fade parts into unofficial ldraw library
 *
 */
bool Gui::Archive(const QString &zipFile, const QDir &dir, const QString &comment = QString("")) {

    // Initialize the zip file
    QuaZip zip(zipFile);
    zip.setFileNameCodec("IBM866");

    // Check if directory exists
    if (!dir.exists()) {
        qDebug() << QString("dir.exists(%1)=FALSE").arg(dir.absolutePath());
        return false;
    }

    // Check if the zip file exist; if yes, set to add content, and if no create
    QFileInfo zipFileInfo(zipFile);
    QFileInfoList zipFiles;
    if (zipFileInfo.exists()){
        if (!zip.open(QuaZip::mdAdd)) {
            qDebug() <<  QString("Archive(): zip.open(): %1").arg(zip.getZipError());
            return false;
        }

        // We get the list of files already in the archive.
        QString zipDirPath = "parts/fade/";
        QStringList zipFileList;
        RecurseZipArchive(zipFileList, zipDirPath, zipFile, dir);

        //Create an array of archive file objects consisting of QFileInfo
        foreach (QString zipFile, zipFileList) zipFiles << QFileInfo(zipFile);

    } else {
        if (!zip.open(QuaZip::mdCreate)) {
            qDebug() <<  QString("Archive(): zip.open(): %1").arg(zip.getZipError());
            return false;
        }
    }

    // We get the list of directory files and folders recursively
    QStringList dirFileList;
    RecurseAddDir(dir, dirFileList);

    // Create an array of objects consisting of QFileInfo
    QFileInfoList files;
    foreach (QString fileName, dirFileList) files << QFileInfo(fileName);

    QFile inFile;
    QuaZipFile outFile(&zip);

    char c;
    foreach(QFileInfo fileInfo, files) {

        if (!fileInfo.isFile())
            continue;

        bool alreadyArchived = false;
        foreach (QFileInfo zipFileInfo, zipFiles) {

            if (fileInfo == zipFileInfo) {
                alreadyArchived = true;
            }
        }

        if (alreadyArchived)
            continue;

      /* If the file is in a subdirectory, then add the name of the subdirectory to filenames
         For example: fileInfo.filePath() = "D:\Work\Sources\SAGO\svn\sago\Release\tmp_DOCSWIN\Folder\123.opn"
         then after removing the absolute path portion of the line will produce the fileNameWithSubFolders "Folder\123.opn", etc.
         For example: QString fileNameWithRelativePath = fileInfo.filePath().remove(0, dir.absolutePath().length() + 1);
         But for this application, we want to capture the root (fade) and the root's parent (parts) directory to archive
         in the correct directory, so we append the string "parts/fade" to the relative file name path. */
        QString fileNameWithRelativePath = fileInfo.filePath().remove(0, dir.absolutePath().length() + 1);
        QString fileNameWithCompletePath = QString("%1/%2").arg("parts/fade").arg(fileNameWithRelativePath);

        inFile.setFileName(fileInfo.filePath());

        if (!inFile.open(QIODevice::ReadOnly)) {
            qDebug() <<  QString("Archive(): inFile.open(): %1").arg(inFile.errorString().toLocal8Bit().constData());
            return false;
        }

        if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileNameWithCompletePath, fileInfo.filePath()))) {
            qDebug() << QString("Archive(): outFile.open(): %1").arg(outFile.getZipError());
            return false;
        }

        while (inFile.getChar(&c) && outFile.putChar(c));

        if (outFile.getZipError() != UNZ_OK) {
            qDebug() << QString("Archive(): outFile.putChar(): %1").arg(outFile.getZipError());
            return false;
        }

        outFile.close();

        if (outFile.getZipError() != UNZ_OK) {
            qDebug() << QString("Archive(): outFile.close(): %1").arg(outFile.getZipError());
            return false;
        }

        inFile.close();
    }

    // + comment
    if (!comment.isEmpty())
        zip.setComment(comment);

    zip.close();

    if (zip.getZipError() != 0) {
        qDebug() << QString("Archive(): zip.close(): %1").arg(zip.getZipError());
        return false;
    }

    return true;
}

/* Recursively searches files in the archive for a given directory \ a, and adds to the list of \ b */
bool Gui::RecurseZipArchive(QStringList &zipDirFileList, QString &zipDirPath, const QString &zipFile, const QDir &dir) {

    QuaZip zip(zipFile);
    QuaZip *ptrZip = &zip;

    if (!zip.open(QuaZip::mdUnzip)) {
        qWarning("Extract(): zip.open(): %d", zip.getZipError());
        return false;
    }

    zip.setFileNameCodec("IBM866");

//    qWarning("%d entries\n", zip.getEntriesCount());
//    qWarning("Global comment: %s\n", zip.getComment().toLocal8Bit().constData());

    QuaZipDir zipDir(ptrZip,zipDirPath);

    if (zipDir.exists()) {

        zipDir.cd(zipDirPath);

//        qWarning("%d zipDir entries\n", zipDir.count());

        QStringList qsl = zipDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files, QDir::SortByMask);

        foreach (QString zipFile, qsl) {

                QFileInfo zipFileInfo(QString("%1/%2").arg(dir.absolutePath()).arg(zipFile));

                if (zipFileInfo.isSymLink())
                    return false;

                if(zipFileInfo.isDir()){

                    QString subDirPath = QString("%1%2").arg(zipDirPath).arg(zipFile);
                    QDir subDir(zipFileInfo.filePath());

                    RecurseZipArchive(zipDirFileList, subDirPath, zipFile, subDir);

                } else
                    zipDirFileList << zipFileInfo.filePath();
            }
        }

    zip.close();

    if (zip.getZipError() != UNZ_OK) {
        qWarning("Extract(): zip.close(): %d", zip.getZipError());
        return false;
    }

    return true;
}

/* Recursively searches for all files on the disk \ a, and adds to the list of \ b */
void Gui::RecurseAddDir(const QDir &dir, QStringList &list) {

    QStringList qsl = dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

    foreach (QString file, qsl) {

        QFileInfo finfo(QString("%1/%2").arg(dir.absolutePath()).arg(file));

        if (finfo.isSymLink())
            return;

        if (finfo.isDir()) {

            QDir subDir(finfo.filePath());
            RecurseAddDir(subDir, list);

        } else
            list << QDir::toNativeSeparators(finfo.filePath());
    }
}
