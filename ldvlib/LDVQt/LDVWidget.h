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

#include <LDLib/LDPreferences.h>
#include <TCFoundation/TCAlertManager.h>

#include "name.h"

class LDrawModelViewer;
class LDVAlertHandler;
class LDVPreferences;
class LDViewExportOption;
class LDSnapshotTaker;
class LDHtmlInventory;

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

  void showLDVExportOptions(void);
  void closeLDVExportOptions(void);
  void showLDVPreferences(void);
  void closeLDVPreferences(void);

  bool doCommand(QStringList &arguments);

  // Extend LDV export functions
  bool fileExists(const char* filename);
  bool shouldOverwriteFile(char* filename);
  virtual bool calcSaveFilename(char* saveFilename, int len);
  bool getSaveFilename(char* saveFilename, int len);
  void fileExport(const QString &saveFilename);
  void fileExport(void);

protected:
  bool setIniFile(void);
  void setupLDVFormat(void);
  void setupLDVContext(void);
  void setupLDVUI(void);
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

  // Extend LDV export vars
  QFileDialog           *saveDialog;
  LDPreferences::SaveOp  curSaveOp;
  int                    saveDigits;
  int                    exportType;

  struct IniFile
  {
      QString Title;
      QString File;
  };
  struct IniFile iniFiles[NumIniFiles];

};
extern LDVWidget* ldvWidget;
extern const QString iniFlagNames[];

#endif // LDVWIDGET_H
