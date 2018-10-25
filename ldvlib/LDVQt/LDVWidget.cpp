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

#include "LDVWidget.h"

#include <QPainter>
#include <QDateTime>
#include <QFileInfo>
#include <QApplication>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QDesktopServices>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <ApplicationServices/ApplicationServices.h>
// On the Mac, when using Qt, glextmacosqt.h has to be included prior to anything
// else that might include GL/gl.h, to override and force-load the needed extensions,
// otherwise things don't compile. This is annoying, but it doesn't appear to hurt anything.
#include <GL/glextmacosqt.h>
#endif // __APPLE__

#include <TCFoundation/mystring.h>
#include <TCFoundation/TCAutoreleasePool.h>
#include <TCFoundation/TCUserDefaults.h>
#include <TCFoundation/TCLocalStrings.h>
#include <TCFoundation/TCStringArray.h>
#include <TCFoundation/TCAlertManager.h>
#include <LDLib/LDUserDefaultsKeys.h>

#include <LDLoader/LDLModel.h>
#include <LDLib/LDrawModelViewer.h>
#include <LDLib/LDSnapshotTaker.h>

#include <TRE/TREMainModel.h>
#include <TRE/TREGLExtensions.h>

#include <LDVPreferences.h>
#include <LDViewExportOption.h>
#include <LDVAlertHandler.h>

#include <vector>
#include <string>
#include <assert.h>

//#include "lpub_messages.h" // not used
#include "lpub_preferences.h"
#include "lpubalert.h"
#include "version.h"
#include "paths.h"

#define WIN_WIDTH 640
#define WIN_HEIGHT 480

LDVWidget* ldvWidget;

LDVWidget::LDVWidget(QWidget *parent, IniFlag iniflag, bool forceIni)
        : QGLWidget(parent),
        iniFlag(iniflag),
        forceIni(forceIni),
        ldvFormat(nullptr),
        ldvContext(nullptr),
        modelViewer(nullptr),
        snapshotTaker(nullptr),
        ldvPreferences(nullptr),
        ldvExportOption(nullptr),
        ldvAlertHandler(new LDVAlertHandler(this))
{
  setupLDVFormat();

  setupLDVContext();

  QString messagesPath = QDir::toNativeSeparators(QString("%1%2")
                                                  .arg(Preferences::dataLocation)
                                                  .arg(VER_LDVMESSAGESINI_FILE));
  if (!TCLocalStrings::loadStringTable(messagesPath.toLatin1().constData()))
  {
      emit lpubAlert->messageSig(LOG_ERROR, QString("Could not load  %1 file %2")
                                 .arg(VER_LDVMESSAGESINI_FILE).arg(messagesPath));
  }

  LDLModel::setFileCaseCallback(staticFileCaseCallback);

  iniFiles[NativePOVIni] = { "Native POV", Preferences::nativePOVIni };
  iniFiles[LDViewPOVIni] = { "LDView POV", Preferences::ldviewPOVIni };
  iniFiles[LDViewIni] = { "LDView", Preferences::ldviewIni } ;

  QString programPath = QCoreApplication::applicationFilePath();
  TCUserDefaults::setCommandLine(programPath.toLatin1().constData());

  TCUserDefaults::setAppName(Preferences::lpub3dAppName.toLatin1().constData());

  ldvWidget = this;

}

LDVWidget::~LDVWidget(void)
{
    makeCurrent();
    TCAutoreleasePool::processReleases();

    doneCurrent();
    ldvWidget = nullptr;
}

void LDVWidget::setupLDVUI(){

    setIniFile();

    modelViewer = new LDrawModelViewer(100, 100);

    QFile fontFile(":/resources/SansSerif.fnt");
    if (fontFile.exists())
    {
        int len = fontFile.size();
        if (len > 0)
        {
            char *buffer = (char*)malloc(len);
            if ( fontFile.open( QIODevice::ReadOnly ))
            {
                QDataStream stream( &fontFile );
                stream.readRawData(buffer,len);
                modelViewer->setFontData((TCByte*)buffer,len);
            }
            delete buffer;
        }
    }

    QImage fontImage2x(":/resources/SansSerif@2x.png");
    long len = fontImage2x.byteCount();
    modelViewer->setRawFont2xData(fontImage2x.bits(),len);

    // Saved session
    char *sessionName = TCUserDefaults::getSavedSessionNameFromKey(PREFERENCE_SET_KEY);
    if (sessionName && sessionName[0])
    {
          TCUserDefaults::setSessionName(sessionName, nullptr, false);
    }
    delete sessionName;
}

bool LDVWidget::setIniFile()
{    
    if (!TCUserDefaults::isIniFileSet() || forceIni)
    {
        if (getIniTitle().isEmpty()){
            emit lpubAlert->messageSig(LOG_ERROR, QString("INI file was not specified").arg(getIniTitle()).arg(getIniFile()));
            return false;
        }
        if (!TCUserDefaults::setIniFile(getIniFile().toLatin1().constData()))
        {
            emit lpubAlert->messageSig(LOG_ERROR, QString("Could not set %1 INI %2").arg(getIniTitle()).arg(getIniFile()));
            return false;
        }
//        if (TCUserDefaults::isIniFileSet())
//            emit lpubAlert->messageSig(LOG_INFO, QString("%1 INI set: %2").arg(getIniTitle()).arg(getIniFile()));
    }
    return true;
}

bool LDVWidget::doCommand(QStringList &arguments)
{	
    TCUserDefaults::setCommandLine(arguments.join(" ").toLatin1().constData());

    setIniFile();

    QImage studImage(":/resources/StudLogo.png");
    TREMainModel::setRawStudTextureData(studImage.bits(),studImage.byteCount());

    bool retValue = LDSnapshotTaker::doCommandLine(false, true);
    if (!retValue)
    {
         fprintf(stdout, "ERROR - Failed to process Native command arguments: %s\n",
                 arguments.join(" ").toLatin1().constData());
         fflush(stdout);
    }
    return retValue;
}

void LDVWidget::showLDVExportOptions()
{
    setupLDVUI();

    ldvExportOption = new LDViewExportOption(this);

    if (ldvExportOption->exec() == QDialog::Rejected)
        ldvExportOption->doCancel();
    else
        ldvExportOption->doOk();
}

void LDVWidget::closeLDVExportOptions()
{
    if (ldvExportOption)
        ldvExportOption->doOk();
}

void LDVWidget::showLDVPreferences()
{
    setupLDVUI();

    ldvPreferences = new LDVPreferences(this);

    if (ldvPreferences->exec() == QDialog::Rejected)
        ldvPreferences->doCancel();
    else
        ldvPreferences->doOk();
}

void LDVWidget::closeLDVPreferences()
{
    if (ldvPreferences)
        ldvPreferences->doOk();
}

void LDVWidget::setupLDVFormat(void)
{
    // Specify the format and create platform-specific surface
    ldvFormat.setDepthBufferSize( 24 );
    ldvFormat.setStencilBufferSize(8);
    ldvFormat.setRedBufferSize(8);
    ldvFormat.setGreenBufferSize(8);
    ldvFormat.setBlueBufferSize(8);
    ldvFormat.setAlphaBufferSize(8);

    ldvFormat.setSamples(16);
    ldvFormat.setProfile(QGLFormat::CoreProfile);
    QGLFormat::setDefaultFormat(ldvFormat);
}

void LDVWidget::setupLDVContext()
{
    bool needsInitialize = false;

    ldvContext = context();

    if (ldvContext->isValid()) {
        setFormat(ldvFormat);
        needsInitialize = true;
    } else {
        fprintf(stdout, "ERROR - The OpenGL context is not valid!");
    }

    if (needsInitialize) {
        initializeGLFunctions();
        //displayGLExtensions();
    }
}

void LDVWidget::displayGLExtensions()
{
    // Query version
    const GLubyte *Version = glGetString(GL_VERSION);
    int VersionMajor = 0, VersionMinor = 0;
    if (Version) {
        sscanf((const char*)Version, "%d.%d", &VersionMajor, &VersionMinor);
        fprintf(stdout, "OpenGL version (%d.%d).\n", VersionMajor, VersionMinor);
    }

    // Query extensions
//    QList<QByteArray> extensions = ldvContext->extensions().toList();
//    std::sort( extensions.begin(), extensions.end() );
//    fprintf(stdout, "OpenGL supported extensions (%d).\n", extensions.count());
//    foreach ( const QByteArray &extension, extensions )
//        fprintf(stdout, "     %s\n", extension.constData());
//    fflush(stdout);
}

void LDVWidget::modelViewerAlertCallback(TCAlert *alert)
{
    if (alert)
    {
        fprintf(stdout, "%s\n", alert->getMessage());
        fflush(stdout);
    }
}

bool LDVWidget::getUseFBO()
{
    return snapshotTaker != nullptr && snapshotTaker->getUseFBO();
}

void LDVWidget::snapshotTakerAlertCallback(TCAlert *alert)
{
    if (strcmp(alert->getAlertClass(), "LDSnapshotTaker") == 0)
    {
        if (strcmp(alert->getMessage(), "MakeCurrent") == 0)
        {
              makeCurrent();
        }

        if (strcmp(alert->getMessage(), "PreFbo") == 0)
        {
            if (getUseFBO())
            {
                return;
            }
            else
            {
                makeCurrent();
                TREGLExtensions::setup();
                snapshotTaker = (LDSnapshotTaker*)alert->getSender()->retain();
                if (TREGLExtensions::haveFramebufferObjectExtension())
                {
                    snapshotTaker->setUseFBO(true);
                }
                if (!snapshotTaker->getUseFBO())
                {
                    setupSnapshotBackBuffer(ldvPreferences->getWindowWidth(), ldvPreferences->getWindowHeight());
                }
            }
        }
    }
}

bool LDVWidget::staticFileCaseLevel(QDir &dir, char *filename)
{
	int i;
	int len = strlen(filename);
	QString wildcard;
	QStringList files;

	if (!dir.isReadable())
	{
		return false;
	}
	for (i = 0; i < len; i++)
	{
		QChar letter = filename[i];

		if (letter.isLetter())
		{
			wildcard.append('[');
			wildcard.append(letter.toLower());
			wildcard.append(letter.toUpper());
			wildcard.append(']');
		}
		else
		{
			wildcard.append(letter);
		}
	}
	dir.setNameFilters(QStringList(wildcard));
	files = dir.entryList();
	if (files.count())
	{
		QString file = files[0];

		if (file.length() == (int)strlen(filename))
		{
			// This should never be false, but just want to be sure.
			strcpy(filename, file.toLatin1().constData());
			return true;
		}
	}
	return false;
}

bool LDVWidget::staticFileCaseCallback(char *filename)
{
	char *shortName;
	QDir dir;
	char *firstSlashSpot;

	dir.setFilter(QDir::AllEntries | QDir::Readable | QDir::Hidden | QDir::System);
	replaceStringCharacter(filename, '\\', '/');
	firstSlashSpot = strchr(filename, '/');
	if (firstSlashSpot)
	{
		char *lastSlashSpot = strrchr(filename, '/');
		int dirLen;
		char *dirName;

		while (firstSlashSpot != lastSlashSpot)
		{
			char *nextSlashSpot = strchr(firstSlashSpot + 1, '/');

			dirLen = firstSlashSpot - filename + 1;
			dirName = new char[dirLen + 1];
			*nextSlashSpot = 0;
			strncpy(dirName, filename, dirLen);
			dirName[dirLen] = 0;
			if (dirLen)
			{
				dir.setPath(dirName);
				delete dirName;
				if (!staticFileCaseLevel(dir, firstSlashSpot + 1))
				{
					return false;
				}
			}
			firstSlashSpot = nextSlashSpot;
			*firstSlashSpot = '/';
		}
		dirLen = lastSlashSpot - filename;
		dirName = new char[dirLen + 1];
		strncpy(dirName, filename, dirLen);
		dirName[dirLen] = 0;
		dir.setPath(dirName);
		shortName = lastSlashSpot + 1;
		delete dirName;
	}
	else
	{
		shortName = filename;
	}
	return staticFileCaseLevel(dir, shortName);
}

void LDVWidget::initializeGL(void)
{
    makeCurrent();
    TREGLExtensions::setup();
    ldvPreferences->doCancel();
}

void LDVWidget::resizeGL(int width, int height)
{
    width =  TCUserDefaults::longForKey(SAVE_ACTUAL_SIZE_KEY, 1, false) ?
             TCUserDefaults::longForKey(WINDOW_WIDTH_KEY, WIN_WIDTH, false) :
             TCUserDefaults::longForKey(SAVE_WIDTH_KEY, 1024, false);
    height = TCUserDefaults::longForKey(SAVE_ACTUAL_SIZE_KEY, 1, false) ?
             TCUserDefaults::longForKey(WINDOW_HEIGHT_KEY, WIN_HEIGHT, false) :
             TCUserDefaults::longForKey(SAVE_HEIGHT_KEY, 768, false);

    resize(width, height);

    modelViewer->setWidth(width);
    modelViewer->setHeight(height);

    glViewport(0, 0, width, height);

    QSize surfaceWindowSize = size();
    ldvPreferences->setWindowSize(surfaceWindowSize.width(),
                                  surfaceWindowSize.height());
}

void LDVWidget::paintGL(void)
{
    glEnable(GL_DEPTH_TEST);
    makeCurrent();
    if (!TREGLExtensions::haveFramebufferObjectExtension())
    {
         glDrawBuffer(GL_BACK);
         glReadBuffer(GL_BACK);
    }
    modelViewer->update();
}

void LDVWidget::setupSnapshotBackBuffer(int width, int height)
{
    modelViewer->setSlowClear(true);
    modelViewer->setWidth(width);
    modelViewer->setHeight(height);
    modelViewer->setup();
    glReadBuffer(GL_BACK);
}
