/****************************************************************************
**
** Copyright (C) 2018 Trevor SANDY. All rights reserved.
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

#ifndef LDVWIDGET_H
#define LDVWIDGET_H

#include <QWindow>
#include <QGLContext>
#include <QGLWidget>
#include <QGLFunctions>
#include <QDateTime>
#include <QFileDialog>

#include <TCFoundation/TCAlertManager.h>
#include <LDLib/LDInputHandler.h>
#include <LDLib/LDPreferences.h>
#include <LDLib/LDSnapshotTaker.h>
#include "name.h"

#include "LDVHtmlInventory.h"

class LDrawModelViewer;
class LDVAlertHandler;
class LDVPreferences;
class LDViewExportOption;
class LDSnapshotTaker;
class LDVHtmlInventory;

class LDVWidget : public QGLWidget, protected QGLFunctions
{

  Q_OBJECT

public:
  LDVWidget(QWidget *parent = nullptr, IniFlag iniflag = LDViewIni, bool forceIni = false);
  ~LDVWidget(void);

  LDrawModelViewer *getModelViewer(void) { return modelViewer; }
  LDVPreferences   *getLDVPreferences(void) { return ldvPreferences; }
  IniFlag getIniFlag(void) { return iniFlag; }
  QString getIniTitle(void);
  QString getIniFile(void);

  static bool staticFileCaseCallback(char *filename);
  static bool staticFileCaseLevel(QDir &dir, char *filename);

  void modelViewerAlertCallback(TCAlert *alert);
  void snapshotTakerAlertCallback(TCAlert *alert);

  void showLDVExportOptions(void);
  void showLDVPreferences(void);

  bool doCommand(QStringList &arguments);

  // Parts List functions
  void doPartList(void);
  void doPartList(LDVHtmlInventory *htmlInventory, LDPartsList *partsList,
      const char *filename);
  bool saveImage(char *filename, int imageWidth, int imageHeight,
      bool fromCommandLine = false);
  LDSnapshotTaker::ImageType getSaveImageType(void);
  bool grabImage(int &imageWidth, int &imageHeight,
      bool fromCommandLine = false);
  bool chDirFromFilename(const char *filename);
  void setViewMode(LDInputHandler::ViewMode value, bool examineLatLong,
                       bool keepRightSideUp, bool saveSettings=true);
  void showWebPage(QString &htmlFilename);

  // new
  void setupMultisample(void);
  void cleanupRenderSettings(void);
  virtual void cleanupOffscreen(void);
  virtual void cleanupPBuffer(void);
  virtual bool setupPBuffer(int imageWidth, int imageHeight,
      bool antialias = false);
  virtual void setupLighting(void);
  virtual void setupMaterial(void);

signals:
  void loadBLElementsSig();

protected:
  bool setIniFile(void);
  void setupLDVFormat(void);
  void setupLDVContext(void);
  bool setupLDVApplication(void);
  void displayGLExtensions(void);

  bool getUseFBO();
  void setupSnapshotBackBuffer(int width, int height);

  IniFlag                iniFlag;
  bool                   forceIni;
  QGLFormat              ldvFormat;
  QGLContext            *ldvContext;

  LDrawModelViewer      *modelViewer;
  LDSnapshotTaker       *snapshotTaker;
  LDVPreferences        *ldvPreferences;
  LDViewExportOption    *ldvExportOption;
  LDVAlertHandler       *ldvAlertHandler;

  // Parts List vars
  LDInputHandler        *inputHandler;
  const char            *saveImageFilename;
  LDInputHandler::ViewMode viewMode;
  bool                   commandLineSnapshotSave;
  bool                   saving;

#ifdef Q_OS_WIN
  bool                   savingFromCommandLine;
  HPBUFFERARB            hPBuffer;
  HDC                    hPBufferDC;
  HGLRC                  hPBufferGLRC;
  HDC                    hCurrentDC;
  HGLRC                  hCurrentGLRC;
  HINSTANCE              hInstance;
  HWND                   hWnd;
  HDC                    hdc;
  HGLRC                  hglrc;
#endif
  bool                   saveAlpha;
  int                    saveImageType;
  bool                   saveImageZoomToFit;
  int                    saveImageWidth;
  int                    saveImageHeight;
  bool                   saveImageResult;
  int                    mwidth, mheight;
  bool                   examineLatLong;
  bool                   keepRightSide;

  int                    currentAntialiasType;

  struct IniFile
  {
      QString Title;
      QString File;
  };
  IniFile iniFiles[NumIniFiles];

};

extern LDVWidget* ldvWidget;
extern const QString iniFlagNames[];

#endif // LDVWIDGET_H
