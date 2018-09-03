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

#include <QGLWidget>
#include <QDateTime>
#include <QFileDialog>

//#include <TCFoundation/TCObject.h>
#include <TCFoundation/TCAlertManager.h>

//#include <LDLib/LDrawModelViewer.h>
//#include <LDLib/LDSnapshotTaker.h>
//#include <LDVQt/LDVPreferences.h>

#include "name.h"

class LDrawModelViewer;
class LDSnapshotTaker;
class AlertHandler;
class LDVPreferences;

class LDVWidget : public QGLWidget
{

  Q_OBJECT

public:
  LDVWidget(QWidget *parent=NULL);
  ~LDVWidget(void);

  bool setIniFlag(IniFlag iniflag = NativePOVIni, IniStat iniStat = AfterInit);

  LDrawModelViewer *getModelViewer(void) { return modelViewer; }

  static void setupLDVFormat(void);

  void modelViewerAlertCallback(TCAlert *alert);
  void snapshotTakerAlertCallback(TCAlert *alert);

  void showLDVExportOptions(void);
  void showLDVPreferences(void);

  bool doCommand(QStringList &arguments);

  static IniFlag iniFlag;

protected:
  // GL Widget overrides
  void initializeGL(void);
  void resizeGL(int width, int height);
  void paintGL(void);

  bool getUseFBO();
  void setupSnapshotBackBuffer(int width, int height);

  LDrawModelViewer *modelViewer;
  LDSnapshotTaker *snapshotTaker;
  AlertHandler *alertHandler;
  QString programPath;

  LDVPreferences *ldvPreferences;
};
extern LDVWidget* ldvWidget;

#endif // LDVWIDGET_H
