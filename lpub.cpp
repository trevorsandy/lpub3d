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

#include "lpub.h"
#include "editwindow.h"
#include "name.h"
#include "paths.h"
#include "globals.h"
#include "resolution.h"
#include "lpub_preferences.h"
#include "render.h"
#include "metaitem.h"
#include "ranges_element.h"

Gui *gui;

void clearPliCache()
{
  gui->clearPLICache();
}

void clearCsiCache()
{
  gui->clearCSICache();
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

void Gui::statusBarMsg(QString msg)
{
  statusBar()->showMessage(msg);
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

void Gui::preferences()
{
  if (Preferences::getPreferences()) {
    Meta meta;
    
    page.meta = meta;
    QString renderer = Render::getRenderer();
    Render::setRenderer(Preferences::preferredRenderer);
    if (Render::getRenderer() != renderer) {
      gui->clearCSICache();
      gui->clearPLICache();
    }
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
    Preferences::lpubPreferences();
    Preferences::renderPreferences();
	Preferences::lgeoPreferences();
    Preferences::pliPreferences();
    Preferences::annotationPreferences();
    Preferences::fadestepPreferences();

    displayPageNum  = 1;

    editWindow      = new EditWindow();
//**3D
    lcqWindow       = new lcQMainWindow;
//**

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

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createDockWindows();

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
    resize(1000,600);

    gui = this;

    fitMode = FitVisible;

#ifdef __APPLE__
    extern void qt_mac_set_native_menubar(bool);
    qt_mac_set_native_menubar(true);
#endif

    Preferences::getRequireds();
    Render::setRenderer(Preferences::preferredRenderer);
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
  } else {
    event->ignore();
  }
}

void Gui::about()
{
   QMessageBox::about(this, tr("About LPub"),
            tr("<b>LPub %1</b> is a proud member of the LDraw "
               "family of tools.  LPub is a WYSIWYG tool for creating "
               "LEGO(c) style building instructions. "
               "LPub4 source code and application can be found on "
               "www.sourceforge.net/projects/lpub4/files.<br>"
               "Copyright 2000-2011 Kevin Clague "
               "kevin.clague@gmail.com<br>"
               "2014 - Daniele Benedettelli<br>"
               "2015 - Trevor SANDY "
               "<A HREF=\"%2\">trevor.sandy@gmail.com</A>"
               ).arg(QString::fromLatin1(LP_VERSION_TEXT),QString::fromLatin1(LP_VERSION_EMAIL)));
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
    QMessageBox::tr(LPUB),
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
    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the document to disk"));
    saveAct->setEnabled(false);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    saveAsAct->setEnabled(false);
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    printToFileAct = new QAction(QIcon(":/images/pdf_logo.png"), tr("Print to &File"), this);
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

    undoAct = new QAction(QIcon(":/images/editundo.png"), tr("Undo"), this);
    undoAct->setShortcut(tr("Ctrl+Z"));
    undoAct->setStatusTip(tr("Undo last change"));
    connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));
    redoAct = new QAction(QIcon(":/images/editredo.png"), tr("Redo"), this);
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

    fitWidthAct = new QAction(QIcon(":/images/fitWidth.png"), tr("Fit Width"), this);
    fitWidthAct->setShortcut(tr("Ctrl+W"));
    fitWidthAct->setStatusTip(tr("Fit document to width"));
    connect(fitWidthAct, SIGNAL(triggered()), this, SLOT(fitWidth()));

    fitVisibleAct = new QAction(QIcon(":/images/fitVisible.png"), tr("Fit Visible"), this);
    fitVisibleAct->setShortcut(tr("Ctrl+I"));
    fitVisibleAct->setStatusTip(tr("Fit document so whole page is visible"));
    connect(fitVisibleAct, SIGNAL(triggered()), this, SLOT(fitVisible()));

    actualSizeAct = new QAction(QIcon(":/images/actual.png"),tr("Actual Size"), this);
    actualSizeAct->setShortcut(tr("Ctrl+A"));
    actualSizeAct->setStatusTip(tr("Show document actual size"));
    connect(actualSizeAct, SIGNAL(triggered()), this, SLOT(actualSize()));
    
    // zoomIn,zoomOut

    zoomInAct = new QAction(QIcon(":/images/zoomin.png"), tr("&Zoom In"), this);
    zoomInAct->setShortcut(tr("Ctrl++"));
    zoomInAct->setStatusTip(tr("Zoom in"));
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(QIcon(":/images/zoomout.png"),tr("Zoom &Out"),this);
    zoomOutAct->setShortcut(tr("Ctrl+-"));
    zoomOutAct->setStatusTip(tr("Zoom out"));
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    // firstPage,lastPage,nextPage,previousPage

    firstPageAct = new QAction(QIcon(":/images/first.png"),tr("First Page"), this);
    firstPageAct->setShortcut(tr("Ctrl+F"));
    firstPageAct->setStatusTip(tr("Go to first page of document"));
    connect(firstPageAct, SIGNAL(triggered()), this, SLOT(firstPage()));

    lastPageAct = new QAction(QIcon(":/images/last.png"),tr("Last Page"), this);
    lastPageAct->setShortcut(tr("Ctrl+L"));
    lastPageAct->setStatusTip(tr("Go to last page of document"));
    connect(lastPageAct, SIGNAL(triggered()), this, SLOT(lastPage()));

    nextPageAct = new QAction(QIcon(":/images/next.png"),tr("&Next Page"),this);
    nextPageAct->setShortcut(tr("Ctrl+N"));
    nextPageAct->setStatusTip(tr("Go to next page of document"));
    connect(nextPageAct, SIGNAL(triggered()), this, SLOT(nextPage()));

    previousPageAct = new QAction(QIcon(":/images/prev.png"),tr("&Previous Page"),this);
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

    // Help

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    // Begin Jaco's code

    onlineManualAct = new QAction(tr("&Online manual"), this);
    onlineManualAct->setStatusTip(tr("Visit the Online Manual Website."));
    connect(onlineManualAct, SIGNAL(triggered()), this, SLOT(onlineManual()));

    // End Jaco's code

    metaAct = new QAction(tr("&Save LPub Metacommands to File"), this);
    metaAct->setStatusTip(tr("Save a list of the known LPub meta commands to a file"));
    connect(metaAct, SIGNAL(triggered()), this, SLOT(meta()));
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
    configMenu->addAction(preferencesAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);

    // Begin Jaco's code

    helpMenu->addAction(onlineManualAct);

    // End Jaco's code

    helpMenu->addAction(metaAct);
}

void Gui::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    fileToolBar->addAction(printToFileAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(undoAct);
    editToolBar->addAction(redoAct);

    navigationToolBar = addToolBar(tr("Navigation"));
    navigationToolBar->addAction(firstPageAct);
    navigationToolBar->addAction(previousPageAct);
    navigationToolBar->addWidget(setPageLineEdit);
    navigationToolBar->addAction(nextPageAct);
    navigationToolBar->addAction(lastPageAct);

    mpdToolBar = addToolBar(tr("MPD"));
    mpdToolBar->addWidget(mpdCombo);

    zoomToolBar = addToolBar(tr("Zoom"));
    zoomToolBar->addAction(fitVisibleAct);
    zoomToolBar->addAction(fitWidthAct);
// Jaco add actual size icon. Was missing.
    zoomToolBar->addAction(actualSizeAct);
    zoomToolBar->addAction(zoomInAct);
    zoomToolBar->addAction(zoomOutAct);
}

void Gui::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void Gui::createDockWindows()
{
//**3D
    modelDockWindow = new QDockWidget(tr("LPub 3D Viewer"), this);
    modelDockWindow->setObjectName("ModelDockWindow");
    modelDockWindow->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    modelDockWindow->setWidget(lcqWindow);
    addDockWidget(Qt::RightDockWidgetArea, modelDockWindow);
    viewMenu->addAction(modelDockWindow->toggleViewAction());
//**

    fileEditDockWindow = new QDockWidget(tr("LDraw File Editor"), this);
    fileEditDockWindow->setObjectName("LDrawFileDockWindow");
    fileEditDockWindow->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    fileEditDockWindow->setWidget(editWindow);
    addDockWidget(Qt::RightDockWidgetArea, fileEditDockWindow);
    viewMenu->addAction(fileEditDockWindow->toggleViewAction());


    tabifyDockWidget(modelDockWindow, fileEditDockWindow);
    modelDockWindow->raise();
}

void Gui::readSettings()
{
    QSettings settings(LPUB, SETTINGS);
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(600, 400)).toSize();
    resize(size);
    move(pos);
}

void Gui::writeSettings()
{
    QSettings settings(LPUB, SETTINGS);
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}



