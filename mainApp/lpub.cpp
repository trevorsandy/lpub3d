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
#include "lpub.h"
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets/QWidget>
#else
#include <QWidget>
#endif
#include <QDesktopWidget>
#include <QSizePolicy>
#include <QFileDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QCloseEvent>
#include <QUndoStack>
#include <QTextStream>
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
#include "preferencesdialog.h"
#include "render.h"
#include "metaitem.h"
#include "ranges_element.h"
#include "updatecheck.h"

#include "step.h"
//** 3D
#include "camera.h"
#include "piece.h"
#include "lc_profile.h"

#include "application.h"

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
	return NULL;
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

void clearCsi3dCache()
{
    gui->clearCSI3DCache();
}

void clearAndRedrawPage()
{
    gui->clearAndRedrawPage();
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
  int modelStatus = mi.okToInsertFinalModel();

  if (Preferences::enableFadeStep && modelStatus != modelExist){
      mi.insertFinalModel(modelStatus);
    } else if (! Preferences::enableFadeStep && modelStatus == modelExist){
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
  emit messageSig(true, "Processing page display...");

  if (macroNesting == 0) {
      clearPage(KpageView,KpageScene);
      page.coverPage = false;
      drawPage(KpageView,KpageScene,false);
      enableActions2();

      emit enable3DActionsSig();
      Step::refreshCsi = false; //reset
    }
  emit messageSig(true,"Page display ready.");
}

/*
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
  } else {
     statusBarMsg("You're on the first page");
  }
} */

void Gui::nextPage()
{
  QString string = setPageLineEdit->displayText();
  QRegExp rx("^(\\d+)\\s+.*$");
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

void Gui::prevPage()
{
  QString string = setPageLineEdit->displayText();
  QRegExp rx("^(\\d+)\\s+.*$");
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
          return;
        }
    }

  QString string = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);
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
//~~~~~~~~~~~~~~
void Gui::twoPages()
{
  twoPages(pageview());
}

void Gui::twoPages(
  LGraphicsView *view)
{
   view->scale(1.0,1.0);
  fitMode = FitTwoPages;
}

void Gui::continuousScroll()
{
  continuousScroll(pageview());
}
void Gui::continuousScroll(
  LGraphicsView *view)
{
  view->scale(1.0,1.0);
  fitMode = FitContinuousScroll;
}
//~~~~~~~~~~~~~~


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
    int currentIndex = 0;
    for (int i = 0; i < mpdCombo->count(); i++) {
      if (mpdCombo->itemText(i) == modelName) {
        currentIndex = i;
        break;
      }
    }
    mpdCombo->setCurrentIndex(currentIndex);
    curSubFile = modelName;
    displayFileSig(ldrawFile, modelName);
//  }
}

void Gui::displayParmsFile(
  const QString &fileName)
{
    displayParmsFileSig(fileName);
}

void Gui::halt3DViewer(bool b)
{

    QString printBanner, imageFile;

#ifdef __APPLE__

    printBanner = QString("%1/%2").arg(Preferences::lpubDataPath,"extras/printbanner.ldr");
    imageFile = QString("%1/%2").arg(Preferences::lpubDataPath,"extras/PDFPrint.jpg");

#else

    printBanner = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath,"extras/printbanner.ldr"));
    imageFile = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath,"extras/PDFPrint.jpg"));

#endif

    if (b){
        installExportBanner(exportType, printBanner,imageFile);
    }

    bool rc = b;
    emit halt3DViewerSig(rc);
}
/*-----------------------------------------------------------------------------*/

bool Gui::installExportBanner(const int &type, const QString &printFile, const QString &imageFile){

    QList<QString> ldrData;
    ldrData << "0 FILE printbanner.ldr";
    ldrData << "0 Name: printbanner.ldr";
    ldrData << "0 Author: Trevor SANDY";
    ldrData << "0 Unofficial Model";
    ldrData << "0 !LEOCAD MODEL NAME Printbanner";
    ldrData << "0 !LEOCAD MODEL AUTHOR Trevor SANDY";
    ldrData << "0 !LEOCAD MODEL DESCRIPTION Graphic displayed during pdf printing";
    ldrData << "0 !LEOCAD MODEL BACKGROUND IMAGE NAME " + imageFile;
    ldrData << "1 71 27.754391 -8.000004 15.334212 -0.500038 0 -0.866003 0 1 0 0.866003 0 -0.500038 3020.DAT";
    ldrData << "1 71 21.413288 -16 46.314728 -0.866003 0 0.500038 0 1 0 -0.500038 0 -0.866003 3024.DAT";
    ldrData << "1 71 21.413288 -23.999998 46.314728 -0.866003 0 0.500038 0 1 0 -0.500038 0 -0.866003 3024.DAT";
    ldrData << "1 71 21.413288 -39.999996 46.314728 0.500038 0 0.866003 0 1 0 -0.866003 0 0.500038 6091.DAT";
    ldrData << "1 2 21.413288 -39.999996 46.314728 -0.866003 0 0.500038 0 1 0 -0.500038 0 -0.866003 30039.DAT";
    ldrData << "1 71 51.415627 -15.999998 -5.645548 -0.866003 0 0.500038 0 1 0 -0.500038 0 -0.866003 3024.DAT";
    ldrData << "1 71 51.415627 -23.999998 -5.645548 -0.866003 0 0.500038 0 1 0 -0.500038 0 -0.866003 3024.DAT";
    ldrData << "1 71 51.415627 -39.999996 -5.645548 0.500038 0 0.866003 0 1 0 -0.866003 0 0.500038 6091.DAT";
    ldrData << "1 71 51.415627 -39.999996 -5.645548 -0.866003 0 0.500038 0 1 0 -0.500038 0 -0.866003 30039.DAT";
    ldrData << "1 71 36.414444 -32 20.334612 0.500038 0 0.866003 0 1 0 -0.866003 0 0.500038 3937.DAT";
    ldrData << "1 72 19.094339 -16.000002 10.333817 -0.500038 0 -0.866003 0 1 0 0.866003 0 -0.500038 3023.DAT";
    ldrData << "1 72 19.094339 -16 10.333817 -0.500038 0 -0.866003 0 1 0 0.866003 0 -0.500038 85984.DAT";
    ldrData << "1 71 33.315815 -31.337952 18.545441 -0.500038 0.309864 -0.80867 0 0.933795 0.357809 0.866003 0.178918 -0.466933 3938.DAT";
    ldrData << "1 72 25.879089 -53.749039 14.251392 -0.500038 0.309864 -0.80867 0 0.933795 0.357809 0.866003 0.178918 -0.466933 4865A.DAT";
    switch (type) {
      case EXPORT_PNG:
        ldrData << "1 25 -8.957584 -12.000671 14.921569 0.500038 0.757423 0.419848 0 0.484811 -0.874619 -0.866003 0.437343 0.242424 3070BPTP.DAT";
        ldrData << "1 25 1.043207 -12.000679 -2.398535 0.500038 0.757423 0.419848 0 0.484811 -0.874619 -0.866003 0.437343 0.242424 3070BPTN.DAT";
        ldrData << "1 25 11.043989 -12.000673 -19.718601 0.500038 0.757423 0.419848 0 0.484811 -0.874619 -0.866003 0.437343 0.242424 3070BPTG.DAT";
        break;
      case EXPORT_JPG:
        ldrData << "1 92 -8.957584 -12.000671 14.921569 0.500038 0.757423 0.419848 0 0.484811 -0.874619 -0.866003 0.437343 0.242424 3070BPTJ.DAT";
        ldrData << "1 92 1.043207 -12.000679 -2.398535 0.500038 0.757423 0.419848 0 0.484811 -0.874619 -0.866003 0.437343 0.242424 3070BPTP.DAT";
        ldrData << "1 92 11.043989 -12.000673 -19.718601 0.500038 0.757423 0.419848 0 0.484811 -0.874619 -0.866003 0.437343 0.242424 3070BPTG.DAT";
        break;
      case EXPORT_BMP:
        ldrData << "1 73 -8.957584 -12.000671 14.921569 0.500038 0.757423 0.419848 0 0.484811 -0.874619 -0.866003 0.437343 0.242424 3070BPTB.DAT";
        ldrData << "1 73 1.043207 -12.000679 -2.398535 0.500038 0.757423 0.419848 0 0.484811 -0.874619 -0.866003 0.437343 0.242424 3070BPTM.DAT";
        ldrData << "1 73 11.043989 -12.000673 -19.718601 0.500038 0.757423 0.419848 0 0.484811 -0.874619 -0.866003 0.437343 0.242424 3070BPTP.DAT";
        break;
      default:
        ldrData << "1 216 -8.957584 -12.000671 14.921569 0.500038 0.757423 0.419848 0 0.484811 -0.874619 -0.866003 0.437343 0.242424 3070BPTP.DAT";
        ldrData << "1 216 1.043207 -12.000679 -2.398535 0.500038 0.757423 0.419848 0 0.484811 -0.874619 -0.866003 0.437343 0.242424 3070BPTD.DAT";
        ldrData << "1 216 11.043989 -12.000673 -19.718601 0.500038 0.757423 0.419848 0 0.484811 -0.874619 -0.866003 0.437343 0.242424 3070BPTF.DAT";
      }
    ldrData << "0";
    ldrData << "0 NOFILE";

    QFile ldrFile(printFile);
    if ( ! ldrFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
        QMessageBox::warning(NULL,
                             tr("LPub3D"),
                             tr("Cannot open Print Banner file %1 for writing:\n%2")
                             .arg(printFile)
                             .arg(ldrFile.errorString()));
        return false;
    }
    QTextStream out(&ldrFile);
    for (int i = 0; i < ldrData.size(); i++) {
        QString fileLine = ldrData[i];
        out << fileLine << endl;
    }
    ldrFile.close();

    //load CSI 3D file into viewer
    if (gMainWindow->OpenProject(ldrFile.fileName())){
        return true;
    } else {return false;}

    return true;
}

void Gui::mpdComboChanged(int index)
{
  index = index;
  QString newSubFile = mpdCombo->currentText();

  if (curSubFile != newSubFile) {
      int modelPageNum = ldrawFile.getModelStartPageNumber(newSubFile);
      logInfo() << "SELECT Model: " << newSubFile << " @ Page: " << modelPageNum;
      countPages();
      if (displayPageNum != modelPageNum && modelPageNum != 0) {
          displayPageNum  = modelPageNum;
          displayPage();
          return;
        } else {

          // TODO add status bar message
          Where topOfSteps(newSubFile,0);
          curSubFile = newSubFile;
          displayFileSig(&ldrawFile, curSubFile);
          showLineSig(topOfSteps.lineNumber);
          return;
        }
    }
}


void Gui::clearImageModelCaches()
{
    if (getCurFile().isEmpty()) {
        statusBarMsg("A model must be open to reset its caches - no action taken.");
        return;
    }

       clearPLICache();
       clearCSICache();
       clearCSI3DCache();

       //reload current model file
       openFile(curFile);
       displayPage();
       enableActions();

       statusBarMsg("All content reset and file reloaded");
}

void Gui::clearAndRedrawPage()
{
    if (getCurFile().isEmpty()) {
        statusBarMsg("A model must be open to reset its caches - no action taken.");
        return;
    }

       clearPLICache();
       clearCSICache();
       clearCSI3DCache();
       displayPage();

       QObject *obj = sender();
       if (obj == editWindow)
         statusBarMsg("Page regenerated.");
       else if (obj == clearImageModelCacheAct)
         statusBarMsg("Assembly, Parts and 3D content caches reset.");
       else
         statusBarMsg("All content reset.");
}

void Gui::clearFadeCache()
{
  QMessageBox::StandardButton ret;
  ret = QMessageBox::warning(this, tr(VER_PRODUCTNAME_STR),
                             tr("All fade files will be deleted. \nFade files are automatically generated.\n"
                                "Do you want to delete the current fade files?"),
                             QMessageBox::Ok | QMessageBox::Discard | QMessageBox::Cancel);
  if (ret == QMessageBox::Ok) {

      QString dirName = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("Unofficial/fade"));

      int count = 0;
      if (removeDir(count, dirName)){
          statusBarMsg(QString("Fade parts content cache cleaned.  %1 items removed.").arg(count));
        } else {
          QMessageBox::critical(NULL,
                                tr("LPub3D"),
                                tr("Unable to remeove fade cache directory \n%1")
                                .arg(dirName));
          return;
        }

    } else if (ret == QMessageBox::Cancel) {
      return;
    }

}

void Gui::clearPLICache()
{
    if (getCurFile().isEmpty()) {
        statusBarMsg("A model must be open to clean its parts cache - no action taken.");
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
            QMessageBox::critical(NULL,
                                  tr("LPub3D"),
                                  tr("Unable to remeove %1")
                                  .arg(dirName + "/" + fileInfo.fileName()));
            count--;
          } else
          count++;
      }

    statusBarMsg(QString("Parts content cache cleaned. %1 items removed.").arg(count));
}

void Gui::clearCSICache()
{
    if (getCurFile().isEmpty()) {
        statusBarMsg("A model must be open to clean its assembly cache - no action taken.");
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
            QMessageBox::critical(NULL,
                                  tr("LPub3D"),
                                  tr("Unable to remeove %1")
                                  .arg(dirName + "/" + fileInfo.fileName()));
            count--;
          } else
          count++;
      }

    statusBarMsg(QString("Assembly content cache cleaned. %1 items removed.").arg(count));
}

void Gui::clearCSI3DCache()
{
    if (getCurFile().isEmpty()) {
        statusBarMsg("A model must be open to clean its 3D cache - no action taken.");
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
            QMessageBox::critical(NULL,
                                  tr("LPub3D"),
                                  tr("Unable to remeove %1")
                                  .arg(tmpDirName + "/" + fileInfo.fileName()));
            count1--;
          } else
          count1++;
      }

    ldrawFile.tempCacheCleared();

    QString viewDirName = QDir::currentPath() + "/" + Paths::viewerDir;
    QDir viewDir(viewDirName);

    viewDir.setFilter(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList viewDirList = viewDir.entryInfoList();
    int count2 = 0;
    for (int i = 0; i < viewDirList.size(); i++) {
        QFileInfo fileInfo = viewDirList.at(i);
        QFile     file(viewDirName + "/" + fileInfo.fileName());
        if (!file.remove()) {
            QMessageBox::critical(NULL,
                                  tr("LPub3D"),
                                  tr("Unable to remeove %1")
                                  .arg(viewDirName + "/" + fileInfo.fileName()));
            count2--;
          } else
          count2++;
    }

    statusBarMsg(QString("3D Viewer content cache cleaned. %1 temp and %2 viewer items removed.").arg(count1).arg(count2));
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

void Gui::editFadeColourParts()
{
    displayParmsFile(Preferences::fadeStepColorPartsFile);
    parmsWindow->setWindowTitle(tr("Fade Colour Parts","Edit/add static coulour parts for fade"));
    parmsWindow->show();
}

void Gui::editPliBomSubstituteParts()
{
    displayParmsFile(Preferences::pliSubstitutePartsFile);
    parmsWindow->setWindowTitle(tr("PLI/BOM Substitute Parts","Edit/add PLI/BOM substitute parts"));
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

void Gui::preferences()
{
  bool useLDViewSCall= renderer->useLDViewSCall();

  if (Preferences::getPreferences()) {

      Meta meta;
      page.meta = meta;

      Step::refreshCsi = true;

      if (!getCurFile().isEmpty()){

          QString currentRenderer = Render::getRenderer();
          Render::setRenderer(Preferences::preferredRenderer);
          bool rendererChanged = Render::getRenderer() != currentRenderer;
          bool fadeStepColorChanged = Preferences::fadeStepColorChanged && !Preferences::fadeStepSettingChanged;
          bool useLDViewSCallChanged = useLDViewSCall != renderer->useLDViewSCall();

          if (rendererChanged && Preferences::preferredRenderer == "LDGLite") {
              partWorkerLdgLiteSearchDirs.populateLdgLiteSearchDirs();
            }

          if (Preferences::fadeStepSettingChanged){
              clearImageModelCaches();
            } else if (rendererChanged ||
                       fadeStepColorChanged ||
                       useLDViewSCallChanged){
              clearAndRedrawPage();
            }
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
    emit Application::instance()->splashMsgSig("40% - Mainwindow defaults loading...");

    Preferences::renderPreferences();
    Preferences::lgeoPreferences();
    Preferences::publishingPreferences();

    displayPageNum = 1;

    exportOption  = EXPORT_ALL_PAGES;
    exportType    = EXPORT_PDF;
    pageRangeText = displayPageNum;

    editWindow    = new EditWindow(this);
    parmsWindow   = new ParmsWindow(this);

    KpageScene    = new QGraphicsScene(this);
    KpageScene->setBackgroundBrush(Qt::lightGray);
    KpageView     = new LGraphicsView(KpageScene);
    KpageView->pageBackgroundItem = NULL;
    KpageView->setRenderHints(QPainter::Antialiasing | 
                             QPainter::TextAntialiasing |
                             QPainter::SmoothPixmapTransform);
    setCentralWidget(KpageView);

    mpdCombo = new QComboBox(this);
    mpdCombo->setToolTip(tr("Current Submodel"));
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

    progressLabel = new QLabel(this);
    progressLabel->setMinimumWidth(200);
    progressBar = new QProgressBar();
    progressBar->setMaximumWidth(300);

    progressLabelPerm = new QLabel();
    progressLabelPerm->setMinimumWidth(200);
    progressBarPerm = new QProgressBar();
    progressBarPerm->setMaximumWidth(300);

    mExistingRotStep = lcVector3(0.0f, 0.0f, 0.0f);
    mModelStepRotation = lcVector3(0.0f, 0.0f, 0.0f);

    emit Application::instance()->splashMsgSig("50% - Mainwindow initializing...");

    createActions();
    createMenus();
    createToolBars();
//    createStatusBar();

//    createDockWindows();
//    toggleLCStatusBar();

//    readSettings();

    undoStack = new QUndoStack();
    macroNesting = 0;

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

    connect(this, SIGNAL(messageSig(bool,QString)),              this, SLOT(statusMessage(bool,QString)));

    connect(this, SIGNAL(progressBarInitSig()),               this, SLOT(progressBarInit()));
    connect(this, SIGNAL(progressMessageSig(QString)),        this, SLOT(progressBarSetText(QString)));
    connect(this, SIGNAL(progressRangeSig(int,int)),          this, SLOT(progressBarSetRange(int,int)));
    connect(this, SIGNAL(progressSetValueSig(int)),           this, SLOT(progressBarSetValue(int)));
    connect(this, SIGNAL(progressResetSig()),                 this, SLOT(progressBarReset()));
    connect(this, SIGNAL(removeProgressStatusSig()),          this, SLOT(removeProgressStatus()));

    connect(this, SIGNAL(progressBarPermInitSig()),           this, SLOT(progressBarPermInit()));
    connect(this, SIGNAL(progressPermMessageSig(QString)),    this, SLOT(progressBarPermSetText(QString)));
    connect(this, SIGNAL(progressPermRangeSig(int,int)),      this, SLOT(progressBarPermSetRange(int,int)));
    connect(this, SIGNAL(progressPermSetValueSig(int)),       this, SLOT(progressBarPermSetValue(int)));
    connect(this, SIGNAL(progressPermResetSig()),             this, SLOT(progressBarPermReset()));
    connect(this, SIGNAL(removeProgressPermStatusSig()),      this, SLOT(removeProgressPermStatus()));

    m_progressDialog = new ProgressDialog();
    m_progressDialog->setWindowFlags(Qt::WindowStaysOnTopHint);
    m_progressDlgProgressBar = m_progressDialog->findChild<QProgressBar*>("progressBar");
    m_progressDlgMessageLbl  = m_progressDialog->findChild<QLabel*>("ui_progress_bar");

    connect (m_progressDialog, SIGNAL (cancelClicked()), this, SLOT (cancelPrinting()));

#ifdef WATCHER
    connect(&watcher,       SIGNAL(fileChanged(const QString &)),
             this,          SLOT(  fileChanged(const QString &)));
#endif
    setCurrentFile("");

// Jaco: This sets the initial size of the main window
//    resize(QSize(1000, 600));  // moved to readSettings() r602

    gui = this;

    fitMode = FitVisible;

#ifdef __APPLE__
    extern void qt_mac_set_native_menubar(bool);
    qt_mac_set_native_menubar(true);
#endif

    emit Application::instance()->splashMsgSig("60% - Mainwindow renderer loading...");

    Preferences::getRequireds();
    Render::setRenderer(Preferences::preferredRenderer);
}

Gui::~Gui()
{ 

    delete KpageScene;
    delete KpageView;
    delete editWindow;
    delete parmsWindow;

}

void Gui::closeEvent(QCloseEvent *event)
{

  writeSettings();

  if (maybeSave() /*&& gMainWindow->SaveProjectIfModified()*/) {

    parmsWindow->close();

    emit requestEndThreadNowSig();

    event->accept();
  } else {
    event->ignore();
  }

}

bool Gui::InitializeApp(int argc, char *argv[], const char* LibraryInstallPath, const char* LDrawPath){

  /* load sequence
   * lc_application::LoadDefaults
   * lc_application::ldsearchDirPreferences()
   * lc_application::Initialize()
   * lc_application::LoadPiecesLibrary()
   *
   */
  g_App = new lcApplication();

  if (Preferences::preferredRenderer == "LDGLite")
    partWorkerLdgLiteSearchDirs.populateLdgLiteSearchDirs();

  bool initialized = g_App->Initialize(argc, argv, LibraryInstallPath, LDrawPath, this);
  if (initialized){

      gMainWindow->SetColorIndex(lcGetColorIndex(4));
      gMainWindow->UpdateRecentFiles();

      createStatusBar();
      createDockWindows();
      toggleLCStatusBar();

      readSettings();

      connect(this,           SIGNAL(loadFileSig(QString)),
              this,           SLOT(  loadFile(QString)));

      connect(this,           SIGNAL(halt3DViewerSig(bool)),
              gMainWindow,    SLOT(  halt3DViewer   (bool)));

      connect(this,           SIGNAL(enable3DActionsSig()),
              gMainWindow,    SLOT(  enable3DActions()));

    }

  return initialized;
}

void Gui::generateFadeColourPartsList()
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

        connect(listThread,           SIGNAL(started()),                     colourPartListWorker, SLOT(generateFadeColourPartsList()));
        connect(listThread,           SIGNAL(finished()),                              listThread, SLOT(deleteLater()));
        connect(colourPartListWorker, SIGNAL(colourPartListFinishedSig()),             listThread, SLOT(quit()));
        connect(colourPartListWorker, SIGNAL(colourPartListFinishedSig()),   colourPartListWorker, SLOT(deleteLater()));
        connect(this,                 SIGNAL(requestEndThreadNowSig()),      colourPartListWorker, SLOT(requestEndThreadNow()));

        connect(colourPartListWorker, SIGNAL(messageSig(bool,QString)),                      this, SLOT(statusMessage(bool,QString)));

        connect(colourPartListWorker, SIGNAL(progressBarInitSig()),                          this, SLOT(progressBarInit()));
        connect(colourPartListWorker, SIGNAL(progressMessageSig(QString)),                   this, SLOT(progressBarSetText(QString)));
        connect(colourPartListWorker, SIGNAL(progressRangeSig(int,int)),                     this, SLOT(progressBarSetRange(int,int)));
        connect(colourPartListWorker, SIGNAL(progressSetValueSig(int)),                      this, SLOT(progressBarSetValue(int)));
        connect(colourPartListWorker, SIGNAL(progressResetSig()),                            this, SLOT(progressBarReset()));
        connect(colourPartListWorker, SIGNAL(removeProgressStatusSig()),                     this, SLOT(removeProgressStatus()));

        listThread->start();

    } else {
      return;
    }
}

void Gui::processFadeColourParts()
{
  bool doFadeStep = (page.meta.LPub.fadeStep.fadeStep.value() || Preferences::enableFadeStep);

  if (doFadeStep) {

      QThread *partThread  = new QThread();
      partWorkerFadeColour = new PartWorker();
      partWorkerFadeColour->moveToThread(partThread);

      connect(partThread,           SIGNAL(started()),                partWorkerFadeColour, SLOT(processFadeColourParts()));
      connect(partThread,           SIGNAL(finished()),                         partThread, SLOT(deleteLater()));
      connect(partWorkerFadeColour, SIGNAL(fadeColourFinishedSig()),            partThread, SLOT(quit()));
      connect(partWorkerFadeColour, SIGNAL(fadeColourFinishedSig()),  partWorkerFadeColour, SLOT(deleteLater()));
      connect(partWorkerFadeColour, SIGNAL(requestFinishSig()),                 partThread, SLOT(quit()));
      connect(partWorkerFadeColour, SIGNAL(requestFinishSig()),       partWorkerFadeColour, SLOT(deleteLater()));
      connect(this,                 SIGNAL(requestEndThreadNowSig()), partWorkerFadeColour, SLOT(requestEndThreadNow()));

      connect(partWorkerFadeColour, SIGNAL(messageSig(bool,QString)),                 this, SLOT(statusMessage(bool,QString)));

      connect(partWorkerFadeColour, SIGNAL(progressBarInitSig()),                     this, SLOT(progressBarInit()));
      connect(partWorkerFadeColour, SIGNAL(progressMessageSig(QString)),              this, SLOT(progressBarSetText(QString)));
      connect(partWorkerFadeColour, SIGNAL(progressRangeSig(int,int)),                this, SLOT(progressBarSetRange(int,int)));
      connect(partWorkerFadeColour, SIGNAL(progressSetValueSig(int)),                 this, SLOT(progressBarSetValue(int)));
      connect(partWorkerFadeColour, SIGNAL(progressResetSig()),                       this, SLOT(progressBarReset()));
      connect(partWorkerFadeColour, SIGNAL(removeProgressStatusSig()),                this, SLOT(removeProgressStatus()));

      partThread->start();
    }
}

// left side progress bar
void Gui::progressBarInit(){
    progressBar->setMaximumHeight(15);
    statusBar()->addWidget(progressLabel);
    statusBar()->addWidget(progressBar);
    progressLabel->show();
    progressBar->show();
}

void Gui::progressBarSetText(const QString &progressText)
{
  progressLabel->setText(progressText);
  QApplication::processEvents();
}
void Gui::progressBarSetRange(int minimum, int maximum)
{
  progressBar->setRange(minimum,maximum);
  QApplication::processEvents();
}
void Gui::progressBarSetValue(int value)
{
  progressBar->setValue(value);
  QApplication::processEvents();
}
void Gui::progressBarReset()
{
  progressBar->reset();
  QApplication::processEvents();
}

// right side progress bar
void Gui::progressBarPermInit(){
  progressBarPerm->setMaximumHeight(15);
  statusBar()->addPermanentWidget(progressLabelPerm);
  statusBar()->addPermanentWidget(progressBarPerm);
  progressLabelPerm->show();
  progressBarPerm->show();
  QApplication::processEvents();
}

void Gui::progressBarPermSetText(const QString &progressText)
{
progressLabelPerm->setText(progressText);
QApplication::processEvents();
}
void Gui::progressBarPermSetRange(int minimum, int maximum)
{
progressBarPerm->setRange(minimum,maximum);
QApplication::processEvents();
}
void Gui::progressBarPermSetValue(int value)
{
progressBarPerm->setValue(value);
QApplication::processEvents();
}
void Gui::progressBarPermReset()
{
progressBarPerm->reset();
QApplication::processEvents();
}

bool Gui::aboutDialog()
{
    AboutDialog Dialog(this, NULL);
    return Dialog.exec() == QDialog::Accepted;
}

void Gui::refreshLDrawUnoffParts(){

    // Create an instance of update ldraw archive
    libraryDownload = new UpdateCheck(this, (void*)LDrawUnofficialLibraryDownload);
    libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), tr("%1/%2").arg(Preferences::lpubDataPath, "libraries"));

}

void Gui::refreshLDrawOfficialParts(){

    // Create an instance of update ldraw archive
    libraryDownload = new UpdateCheck(this, (void*)LDrawOfficialLibraryDownload);
    libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), tr("%1/%2").arg(Preferences::lpubDataPath, "libraries"));

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

    printToPdfFileAct = new QAction(QIcon(":/resources/pdf_logo.png"), tr("Print to PDF &File"), this);
    printToPdfFileAct->setShortcut(tr("Ctrl+F"));
    printToPdfFileAct->setStatusTip(tr("Print your document to a pdf file"));
    printToPdfFileAct->setEnabled(false);
    connect(printToPdfFileAct, SIGNAL(triggered()), this, SLOT(printToPdfDialog()));

    exportPngAct = new QAction(QIcon(":/resources/exportpng.png"),tr("Export As &PNG Images"), this);
    exportPngAct->setShortcut(tr("Ctrl+Shift+P"));
    exportPngAct->setStatusTip(tr("Export your document as a sequence of PNG images"));
    exportPngAct->setEnabled(false);
    connect(exportPngAct, SIGNAL(triggered()), this, SLOT(exportAsPngDialog()));

    exportJpgAct = new QAction(QIcon(":/resources/exportjpeg.png"),tr("Export As &JPEG Images"), this);
    exportJpgAct->setShortcut(tr("Ctrl+J"));
    exportJpgAct->setStatusTip(tr("Export your document as a sequence of JPEG images"));
    exportJpgAct->setEnabled(false);
    connect(exportJpgAct, SIGNAL(triggered()), this, SLOT(exportAsJpgDialog()));

    exportBmpAct = new QAction(QIcon(":/resources/exportbmp.png"),tr("Export As &Bitmap Images"), this);
    exportBmpAct->setShortcut(tr("Ctrl+B"));
    exportBmpAct->setStatusTip(tr("Export your document as a sequence of bitmap images"));
    exportBmpAct->setEnabled(false);
    connect(exportBmpAct, SIGNAL(triggered()), this, SLOT(exportAsBmpDialog()));

    exitAct = new QAction(QIcon(":/resources/exit.png"),tr("E&xit"), this);
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
    undoAct->setEnabled(false);
    connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));
    redoAct = new QAction(QIcon(":/resources/editredo.png"), tr("Redo"), this);
#ifdef __APPLE__
    redoAct->setShortcut(tr("Ctrl+Shift+Z"));
#else
    redoAct->setShortcut(tr("Ctrl+Y"));
#endif
    redoAct->setStatusTip(tr("Redo last change"));
    redoAct->setEnabled(false);
    connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));

    insertCoverPageAct = new QAction(QIcon(":/resources/insertcoverpage.png"),tr("Insert Front Cover Page"),this);
    insertCoverPageAct->setStatusTip(tr("Insert a front cover page"));
    insertCoverPageAct->setEnabled(false);
    connect(insertCoverPageAct, SIGNAL(triggered()), this, SLOT(insertCoverPage()));

    appendCoverPageAct = new QAction(QIcon(":/resources/appendcoverpage.png"),tr("Append Back Cover Page"),this);
    appendCoverPageAct->setStatusTip(tr("Append back cover page"));
    appendCoverPageAct->setEnabled(false);
    connect(appendCoverPageAct, SIGNAL(triggered()), this, SLOT(appendCoverPage()));

    insertNumberedPageAct = new QAction(QIcon(":/resources/insertpage.png"),tr("Insert Page"),this);
    insertNumberedPageAct->setStatusTip(tr("Insert a numbered page"));
    insertNumberedPageAct->setEnabled(false);
    connect(insertNumberedPageAct, SIGNAL(triggered()), this, SLOT(insertNumberedPage()));

    appendNumberedPageAct = new QAction(QIcon(":/resources/appendpage.png"),tr("Append Page"),this);
    appendNumberedPageAct->setStatusTip(tr("Append a numbered page"));
    appendNumberedPageAct->setEnabled(false);
    connect(appendNumberedPageAct, SIGNAL(triggered()), this, SLOT(appendNumberedPage()));

    deletePageAct = new QAction(QIcon(":/resources/deletepage.png"),tr("Delete Page"),this);
    deletePageAct->setStatusTip(tr("Delete this page"));
    deletePageAct->setEnabled(false);
    connect(deletePageAct, SIGNAL(triggered()), this, SLOT(deletePage()));

    addPictureAct = new QAction(QIcon(":/resources/addpicture.png"),tr("Add Picture"),this);
    addPictureAct->setStatusTip(tr("Add a picture to this page"));
    addPictureAct->setEnabled(false);
    connect(addPictureAct, SIGNAL(triggered()), this, SLOT(addPicture()));

    addTextAct = new QAction(QIcon(":/resources/addtext.png"),tr("Add Text"),this);
    addTextAct->setStatusTip(tr("Add text to this page"));
    addTextAct->setEnabled(false);
    connect(addTextAct, SIGNAL(triggered()), this, SLOT(addText()));

    addBomAct = new QAction(QIcon(":/resources/addbom.png"),tr("Add Bill of Materials"),this);
    addBomAct->setStatusTip(tr("Add Bill of Materials to this page"));
    addBomAct->setEnabled(false);
    connect(addBomAct, SIGNAL(triggered()), this, SLOT(addBom()));

    removeLPubFormattingAct = new QAction(QIcon(":/resources/removelpubformat.png"),tr("Remove LPub Formatting"),this);
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

    // TESTING ONLY
    //connect(actualSizeAct, SIGNAL(triggered()), this, SLOT(twoPages()));
    
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
    setPageLineEdit->setToolTip("Current Page Index");
    setPageLineEdit->setStatusTip("Enter index and hit enter to go to page");
    connect(setPageLineEdit, SIGNAL(returnPressed()), this, SLOT(setPage()));

    clearPLICacheAct = new QAction(QIcon(":/resources/clearplicache.png"),tr("Reset Parts Image Cache"), this);
    clearPLICacheAct->setStatusTip(tr("Reset the parts list image cache"));
    connect(clearPLICacheAct, SIGNAL(triggered()), this, SLOT(clearPLICache()));

    clearCSICacheAct = new QAction(QIcon(":/resources/clearcsicache.png"),tr("Reset Assembly Image Cache"), this);
    clearCSICacheAct->setStatusTip(tr("Reset the assembly image cache"));
    connect(clearCSICacheAct, SIGNAL(triggered()), this, SLOT(clearCSICache()));

    clearCSI3DCacheAct = new QAction(QIcon(":/resources/clearcsi3dcache.png"),tr("Reset 3D Viewer Model Cache"), this);
    clearCSI3DCacheAct->setStatusTip(tr("Reset the 3D viewer image cache"));
    connect(clearCSI3DCacheAct, SIGNAL(triggered()), this, SLOT(clearCSI3DCache()));

    clearImageModelCacheAct = new QAction(QIcon(":/resources/clearimagemodelcache.png"),tr("Reset Image and 3D Model Caches"), this);
    clearImageModelCacheAct->setStatusTip(tr("Reset all image and model caches"));
    connect(clearImageModelCacheAct, SIGNAL(triggered()), this, SLOT(clearImageModelCaches()));

    clearFadeCacheAct = new QAction(QIcon(":/resources/clearfadecache.png"),tr("Reset Fade Files Cache"), this);
    clearFadeCacheAct->setStatusTip(tr("Reset the fade part files cache"));
    connect(clearFadeCacheAct, SIGNAL(triggered()), this, SLOT(clearFadeCache()));

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
    fadeStepSetupAct->setStatusTip(tr("Fade all parts not in the current step"));
    connect(fadeStepSetupAct, SIGNAL(triggered()), this, SLOT(fadeStepSetup()));

    preferencesAct = new QAction(QIcon(":/resources/preferences.png"),tr("Preferences"), this);
    preferencesAct->setStatusTip(tr("Set your preferences for LPub3D"));
    connect(preferencesAct, SIGNAL(triggered()), this, SLOT(preferences()));

    editTitleAnnotationsAct = new QAction(QIcon(":/resources/edittitleannotations.png"),tr("Edit Part Title PLI Annotations"), this);
    editTitleAnnotationsAct->setStatusTip(tr("Add/Edit part title PLI part annotatons"));
    connect(editTitleAnnotationsAct, SIGNAL(triggered()), this, SLOT(editTitleAnnotations()));

    editFreeFormAnnitationsAct = new QAction(QIcon(":/resources/editfreeformannotations.png"),tr("Edit Freeform PLI Annotations"), this);
    editFreeFormAnnitationsAct->setStatusTip(tr("Add/Edit freeform PLI part annotations"));
    connect(editFreeFormAnnitationsAct, SIGNAL(triggered()), this, SLOT(editFreeFormAnnitations()));

    editFadeColourPartsAct = new QAction(QIcon(":/resources/editfadeparts.png"),tr("Edit Fade Colour Parts List"), this);
    editFadeColourPartsAct->setStatusTip(tr("Add/Edit the list of static colour parts used to fade parts"));
    connect(editFadeColourPartsAct, SIGNAL(triggered()), this, SLOT(editFadeColourParts()));

    editPliBomSubstitutePartsAct = new QAction(QIcon(":/resources/editplisubstituteparts.png"),tr("Edit PLI/BOM Substitute Parts List"), this);
    editPliBomSubstitutePartsAct->setStatusTip(tr("Add/Edit the list of PLI/BOM substitute parts"));
    connect(editPliBomSubstitutePartsAct, SIGNAL(triggered()), this, SLOT(editPliBomSubstituteParts()));

    editLdrawIniFileAct = new QAction(QIcon(":/resources/editinifile.png"),tr("Edit LDraw.ini"), this);
    editLdrawIniFileAct->setStatusTip(tr("Add/Edit LDraw.ini search directory entries"));
    connect(editLdrawIniFileAct, SIGNAL(triggered()), this, SLOT(editLdrawIniFile()));

    generateFadeColourPartsAct = new QAction(QIcon(":/resources/generatefadeparts.png"),tr("Generate Fade Colour Parts List"), this);
    generateFadeColourPartsAct->setStatusTip(tr("Generate list of all static coloured parts"));
    connect(generateFadeColourPartsAct, SIGNAL(triggered()), this, SLOT(generateFadeColourPartsList()));

    // Help

    aboutAct = new QAction(QIcon(":/resources/LPub32.png"),tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(aboutDialog()));

    // Begin Jaco's code

    onlineManualAct = new QAction(QIcon(":/resources/onlinemanual.png"),tr("&Online manual"), this);
    onlineManualAct->setStatusTip(tr("Visit the Online Manual Website."));
    connect(onlineManualAct, SIGNAL(triggered()), this, SLOT(onlineManual()));

    // End Jaco's code

    metaAct = new QAction(QIcon(":/resources/savemetacommands.png"),tr("&Save LPub Metacommands to File"), this);
    metaAct->setStatusTip(tr("Save a list of the known LPub meta commands to a file"));
    connect(metaAct, SIGNAL(triggered()), this, SLOT(meta()));

    updateApp = new QAction(QIcon(":/resources/softwareupdate.png"),tr("Check for &Updates..."), this);
    updateApp->setStatusTip(tr("Check if a newer LPub3D version is available for download"));
    connect(updateApp, SIGNAL(triggered()), this, SLOT(updateCheck()));
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
  printToPdfFileAct->setEnabled(true);
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
  editTitleAnnotationsAct->setEnabled(true);
  editFreeFormAnnitationsAct->setEnabled(true);
  editFadeColourPartsAct->setEnabled(true);
  editPliBomSubstitutePartsAct->setEnabled(true);
  cacheMenu->setEnabled(true);
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

void Gui::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);

    exportMenu = fileMenu->addMenu("Export As...");
    exportMenu->setIcon(QIcon(":/resources/exportas.png"));
    exportMenu->addAction(exportPngAct);
    exportMenu->addAction(exportJpgAct);
    exportMenu->addAction(exportBmpAct);

#ifndef __APPLE__
    exportMenu->addAction(exportBmpAct);
#endif

    fileMenu->addAction(printToPdfFileAct);
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

    cacheMenu = toolsMenu->addMenu("Reset Cache...");
    toolsMenu->addAction(refreshLDrawUnoffPartsAct);
    toolsMenu->addAction(refreshLDrawOfficialPartsAct);
    cacheMenu->setIcon(QIcon(":/resources/resetcache.png"));
    cacheMenu->addAction(clearImageModelCacheAct);
    cacheMenu->addAction(clearPLICacheAct);
    cacheMenu->addAction(clearCSICacheAct);
    cacheMenu->addAction(clearCSI3DCacheAct);
    cacheMenu->addAction(clearFadeCacheAct);
    cacheMenu->setDisabled(true);

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

    configMenu->addAction(editFadeColourPartsAct);
    configMenu->addAction(editTitleAnnotationsAct);
    configMenu->addAction(editFreeFormAnnitationsAct);
    configMenu->addAction(editPliBomSubstitutePartsAct);
    if (Preferences::ldrawiniFound) {
      configMenu->addAction(editLdrawIniFileAct);}
    configMenu->addAction(generateFadeColourPartsAct);

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
    fileToolBar->addAction(printToPdfFileAct);

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
    fileEditDockWindow = new QDockWidget(trUtf8(wCharToUtf8("LDraw\u2122 File Editor")), this);
    modelDockWindow = new QDockWidget(trUtf8(wCharToUtf8("3D Viewer - by LeoCAD\u00A9")), this);
    fileEditDockWindow->setObjectName("LDrawFileDockWindow");
    fileEditDockWindow->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    fileEditDockWindow->setWidget(editWindow);
    addDockWidget(Qt::RightDockWidgetArea, fileEditDockWindow);
    viewMenu->addAction(fileEditDockWindow->toggleViewAction());
//**3D
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
    Settings.beginGroup(MAINWINDOW);
    restoreGeometry(Settings.value("Geometry").toByteArray());
    restoreState(Settings.value("State").toByteArray());
    QSize size = Settings.value("Size", QDesktopWidget().availableGeometry(this).size()*0.6).toSize();
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
