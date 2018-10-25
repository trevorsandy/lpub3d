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

#include "name.h"

class LDrawModelViewer;
class LDVAlertHandler;
class LDVPreferences;
class LDViewExportOption;
class LDSnapshotTaker;

class LDVWidget : public QGLWidget, protected QGLFunctions
{

  Q_OBJECT

public:
  LDVWidget(QWidget *parent = nullptr, IniFlag iniflag = LDViewIni, bool forceIni = false);
  ~LDVWidget(void);

  LDrawModelViewer *getModelViewer(void) { return modelViewer; }
  IniFlag getIniFlag(void) { return iniFlag; }
  QString getIniTitle(void) {return iniFiles[iniFlag].Title; }
  QString getIniFile(void) {return iniFiles[iniFlag].File; }

  static bool staticFileCaseCallback(char *filename);
  static bool staticFileCaseLevel(QDir &dir, char *filename);

  void modelViewerAlertCallback(TCAlert *alert);
  void snapshotTakerAlertCallback(TCAlert *alert);

  void showLDVExportOptions(void);
  void closeLDVExportOptions(void);
  void showLDVPreferences(void);
  void closeLDVPreferences(void);

  bool doCommand(QStringList &arguments);

protected:
  bool setIniFile(void);
  void setupLDVFormat(void);
  void setupLDVContext(void);
  void setupLDVUI(void);
  void displayGLExtensions(void);

  bool getUseFBO();
  void setupSnapshotBackBuffer(int width, int height);

  IniFlag iniFlag;
  bool    forceIni;
  QGLFormat ldvFormat;
  QGLContext *ldvContext;

  LDrawModelViewer *modelViewer;
  LDSnapshotTaker *snapshotTaker;
  LDVPreferences *ldvPreferences;
  LDViewExportOption *ldvExportOption;
  LDVAlertHandler *ldvAlertHandler;

  struct IniFile
  {
      QString Title;
      QString File;
  };
  struct IniFile iniFiles[NumIniFiles];

};
extern LDVWidget* ldvWidget;

#endif // LDVWIDGET_H
