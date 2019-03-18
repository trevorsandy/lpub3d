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

#include <QProgressDialog>
#include <QTimer>
#include "name.h"

#include "LDVHtmlInventory.h"

class LDrawModelViewer;
class LDVAlertHandler;
class LDSnapshotTaker;
class LDVHtmlInventory;

class LDVWidget : public QGLWidget, protected QGLFunctions
{

  Q_OBJECT

public:
  LDVWidget(QWidget *parent = nullptr, IniFlag iniflag = LDViewIni, bool forceIni = false);
  ~LDVWidget(void);

  LDrawModelViewer *getModelViewer(void) { return modelViewer; }
  IniFlag getIniFlag(void) { return iniFlag; }
  QString getIniTitle(void);
  QString getIniFile(void);

  static bool staticFileCaseCallback(char *filename);
  static bool staticFileCaseLevel(QDir &dir, char *filename);

  void modelViewerAlertCallback(TCAlert *alert);
  void snapshotTakerAlertCallback(TCAlert *alert);
  bool chDirFromFilename(const char *filename);

  void showLDVExportOptions(void);
  void showLDVPreferences(void);

  bool doCommand(QStringList &arguments);

  void doPartList(void);
  void doPartList(LDVHtmlInventory *htmlInventory, LDPartsList *partsList,
      const char *filename);
  bool saveImage(char *filename, int imageWidth, int imageHeight);
  LDSnapshotTaker::ImageType getSaveImageType(void);
  bool grabImage(int &imageWidth, int &imageHeight);
  void setViewMode(LDInputHandler::ViewMode value, bool examineLatLong,
                       bool keepRightSideUp, bool saveSettings=true);
  void cleanupRenderSettings(void);
  void showDocument(QString &htmlFilename);

#ifdef Q_OS_WIN
  virtual void setupWinExtensions(void);
  virtual void setupMultisample(void);
  virtual int getFSAAFactor(void);
  virtual void cleanupOffscreen(void);
  virtual void cleanupPBuffer(void);
  virtual bool setupPBuffer(int imageWidth, int imageHeight,
      bool antialias = false);
  virtual void setupLighting(void);
  virtual void setupMaterial(void);
#endif

signals:
  void loadBLElementsSig();

private slots:
  void updateProgressDialog();

protected:
  bool setIniFile(void);
  bool setupPartList(void);
  void setupLDVFormat(void);
  void setupLDVContext(void);
  bool setupLDVApplication(void);
  void displayGLExtensions(void);

  bool getUseFBO(void);
  void endProgressBar();
  void startProgressBar(const char *msg);
  bool loadModel(const char *filename);
  void setupSnapshotBackBuffer(int width, int height);

  IniFlag                iniFlag;
  bool                   forceIni;
  QGLFormat              ldvFormat;
  QGLContext            *ldvContext;

  LDPreferences         *ldPrefs;
  LDrawModelViewer      *modelViewer;
  LDSnapshotTaker       *snapshotTaker;
  LDVAlertHandler       *ldvAlertHandler;

  LDInputHandler        *inputHandler;
  const char            *saveImageFilename;
  const char            *imageInputFilename;
  LDInputHandler::ViewMode viewMode;
  bool                   saving;

  QProgressDialog        *progressDialog;
  QTimer                 *timer;
  int                     interval;

#ifdef Q_OS_WIN
  int                    currentAntialiasType;
  HPBUFFERARB            hPBuffer;
  HDC                    hPBufferDC;
  HGLRC                  hPBufferGLRC;
  HDC                    hCurrentDC;
  HGLRC                  hCurrentGLRC;
  HINSTANCE              hInstance;
  HWND                   hWnd;
  HDC                    hdc;
  HGLRC                  hglrc;
  int                    fsaaMode;
#endif
  bool                   saveAlpha;
  int                    saveImageType;
  bool                   saveImageZoomToFit;
  int                    saveImageWidth;
  int                    saveImageHeight;
  bool                   saveImageResult;
  int                    mwidth, mheight;

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
