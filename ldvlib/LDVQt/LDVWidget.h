/****************************************************************************
**
** Copyright (C) 2018 - 2023 Trevor SANDY. All rights reserved.
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
#include <QtGui>

#include <TCFoundation/TCAlertManager.h>
#include <LDLib/LDLibraryUpdater.h>
#include <LDLib/LDInputHandler.h>
#include <LDLib/LDPreferences.h>
#include <LDLib/LDSnapshotTaker.h>

#include <QProgressDialog>
#include <QTimer>
#include "declarations.h"

#include "LDVHtmlInventory.h"

#define LIBRARY_UPDATE_FINISHED 1
#define LIBRARY_UPDATE_CANCELED 2
#define LIBRARY_UPDATE_NONE 3
#define LIBRARY_UPDATE_ERROR 4

class LDrawModelViewer;
class LDVAlertHandler;
class LDSnapshotTaker;
class LDVHtmlInventory;
class TCProgressAlert;

class lcHttpReply;
class lcHttpManager;

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
	bool setDirFromFilename(const char *filename);

	void showLDVExportOptions(void);
	void showLDVPreferences(void);

	bool doCommand(QStringList &arguments);

	void doSetRebrickableParts(const QString &list);
	void doSetDefaultRebrickableKeys();
	void doSetRebrickableKey(QString &key) { m_Keys.append(key); }

	int doGetRebrickableColor(const int ldrawColorID) const;
	std::string doGetRebrickablePartURL(const std::string &ldrawPartID, bool = false) const;

	void doPartList(void);
	void doPartList(LDVHtmlInventory *htmlInventory, LDPartsList *partsList,
					const char *filename);
	bool saveImage(char *filename, int imageWidth, int imageHeight);
	bool grabImage(int &imageWidth, int &imageHeight);
	void setViewMode(LDInputHandler::ViewMode value, bool examineLatLong,
					 bool keepRightSideUp, bool saveSettings=true);
	void showDocument(QString &htmlFilename);
	bool getDarkTheme() { return darkTheme; }

	bool installLDraw();
	void createLibraryUpdateWindow(void);
	void showLibraryUpdateWindow(bool initialInstall);
	void libraryUpdateFinished(int finishType);
	void checkForLibraryUpdates(void);

signals:
	void loadBLCodesSig();

public slots:
	void DownloadFinished(lcHttpReply* Reply);

protected slots:
	virtual void doLibraryUpdateCanceled(void);

protected:
	bool setIniFile(void);
	bool setupPartList(void);
	void setupLDVFormat(void);
	void setupLDVContext(void);
	bool setupLDVApplication(void);
	void displayGLExtensions(void);

	bool getUseFBO(void);
	bool loadModel(const char *filename);
	void setupSnapshotBackBuffer(int width, int height);

	char *getLDrawDir(void);
	bool verifyLDrawDir(bool forceChoose = false);
	bool verifyLDrawDir(char *value);
	bool promptForLDrawDir(QString prompt);
	void libraryUpdateProgress(TCProgressAlert *alert);
	void setLibraryUpdateProgress(float progress);

#if !defined(_NO_BOOST) || defined(USE_CPP11)
	LDLibraryUpdater *libraryUpdater;
#endif
	bool ldrawLibraryUpdateFinished;
	bool ldrawLibraryUpdateCanceled;
	bool ldrawLibraryUpdateFinishNotified;
	QString libraryUpdateProgressMessage;
	float libraryUpdateProgressValue;
	bool libraryUpdateProgressReady;
	int libraryUpdateFinishCode;
	int libraryUpdateTimer;
	QProgressDialog *libraryUpdateWindow;

	IniFlag                iniFlag;
	bool                   forceIni;
	int                    darkTheme;
	QGLFormat              ldvFormat;
	QGLContext            *ldvContext;

	LDPreferences         *ldPrefs;
	LDrawModelViewer      *modelViewer;
	LDSnapshotTaker       *snapshotTaker;
	LDVAlertHandler       *ldvAlertHandler;

	LDInputHandler        *inputHandler;
	char                  *modelFilename;
	const char            *saveImageFilename;
	const char            *imageInputFilename;
	const char            *partListKey;
	const char            *modelExt;
	bool                   saveImageZoomToFit;
	bool                   commandLineSnapshotSave;
	bool                   showLDrawZipMsg;
	LDInputHandler::ViewMode viewMode;

	struct IniFile
	{
		QString Title;
		QString File;
	};
	IniFile iniFiles[NumIniFiles];

	lcHttpManager* m_HttpManager;
	lcHttpReply* m_KeyListReply;
	lcHttpReply* m_PartsReply;
	QStringList m_Keys;
	QByteArray m_RebrickableParts;

};

extern LDVWidget* ldvWidget;
extern const QString iniFlagNames[];

#endif // LDVWIDGET_H
