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

#include <LDVMisc.h>

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
const QString iniFlagNames [] =
{
  "Native POV", "Native STL", "Native 3DS", "LDView POV", "LDView"
};

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

  iniFiles[NativePOVIni] = { "Native POV", Preferences::nativeExportIni };
  iniFiles[NativeSTLIni] = { "Native STL", Preferences::nativeExportIni };
  iniFiles[Native3DSIni] = { "Native 3DS", Preferences::nativeExportIni };
  iniFiles[LDViewPOVIni] = { "LDView POV", Preferences::ldviewPOVIni };
  iniFiles[LDViewIni]    = { "LDView",     Preferences::ldviewIni };

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
    }
    return true;
}

QString LDVWidget::getIniTitle()
{

    if (iniFlag < 0 || iniFlag > NumIniFiles -1)
        return QString();
    else
        return iniFiles[iniFlag].Title;
}

QString LDVWidget::getIniFile()
{
    if (iniFlag < 0 || iniFlag > NumIniFiles -1)
        return QString();
    else
        return iniFiles[iniFlag].File;
}

bool LDVWidget::doCommand(QStringList &arguments)
{	
    TCUserDefaults::setCommandLine(arguments.join(" ").toLatin1().constData());

    setIniFile();

    QImage studImage(":/resources/StudLogo.png");
    TREMainModel::setRawStudTextureData(studImage.bits(),studImage.byteCount());

    if (!LDSnapshotTaker::doCommandLine(false, true))
    {
         emit lpubAlert->messageSig(LOG_ERROR,QString("Failed to process Native command arguments: %1").arg(arguments.join(" ")));
         return false;
    }

    return true;
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
    int Samples = (QGLFormat::defaultFormat().sampleBuffers()) ? QGLFormat::defaultFormat().samples() : 1;
    ldvFormat.setSamples(Samples);
    ldvFormat.setAlpha(true);
    ldvFormat.setStencil(true);
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

void LDVWidget::setupSnapshotBackBuffer(int width, int height)
{
    modelViewer->setSlowClear(true);
    modelViewer->setWidth(width);
    modelViewer->setHeight(height);
    modelViewer->setup();
    glReadBuffer(GL_BACK);
}

  // Extend LDV export
bool LDVWidget::fileExists(const char* filename)
{
    FILE* file = fopen(filename, "r");

    if (file)
    {
        fclose(file);
        return true;
    }
    else
    {
        return false;
    }
}

bool LDVWidget::shouldOverwriteFile(char* filename)
{
    char buf[256];

    sprintf(buf, TCLocalStrings::get("OverwritePrompt"),
        filename);
    switch( QMessageBox::warning( this, tr(VER_PRODUCTNAME_STR),
        buf,
        QMessageBox::Yes | QMessageBox::Default,
        QMessageBox::No | QMessageBox::Escape )) {
    case QMessageBox::Yes:
        return true;
    }
        return false;
}

bool LDVWidget::calcSaveFilename(char* saveFilename, int /*len*/)
{
    char* filename = modelViewer->getFilename();
    saveDigits = TCUserDefaults::longForKey(SAVE_DIGITS_KEY, 1, false);
    if (filename)
    {
        char baseFilename[1024];

        if (strrchr(filename, '/'))
        {
            filename = strrchr(filename, '/') + 1;
        }
        if (strrchr(filename, '\\'))
        {
            filename = strrchr(filename, '\\') + 1;
        }
        strcpy(baseFilename, filename);
        if (strchr(baseFilename, '.'))
        {
            *strchr(baseFilename, '.') = 0;
        }
        if (curSaveOp == LDPreferences::SOExport)
        {
            sprintf(saveFilename, "%s.%s", baseFilename,
                modelViewer->getExporter(
                LDrawModelViewer::ExportType(exportType))->getExtension().c_str());
            return true;
        }
    }
    return false;
}

bool LDVWidget::getSaveFilename(char* saveFilename, int len)
{
    QString initialDir = ldvPreferences->getSaveDir(curSaveOp,
                         modelViewer->getFilename());
    LDrawModelViewer::ExportType origExportType = modelViewer->getExportType();
    QStringList exportFilters;

    QDir::setCurrent(initialDir);
    exportType = TCUserDefaults::longForKey(SAVE_EXPORT_TYPE_KEY,
                 LDrawModelViewer::ETPov, false);
    if (!calcSaveFilename(saveFilename, len))
    {
        saveFilename[0] = 0;
    }
    switch (curSaveOp)
    {
    case LDPreferences::SOExport:
        origExportType = modelViewer->getExportType();
        for (int i = LDrawModelViewer::ETFirst; i <= LDrawModelViewer::ETLast;i++)
        {
            const LDExporter *exporter = modelViewer->getExporter(
                  LDrawModelViewer::ExportType(i));

            if (exporter != nullptr)
            {
                ucstring ucFileType = exporter->getTypeDescription();
                QString qFileType;

                ucstringtoqstring(qFileType, ucFileType);
                qFileType += " (*.";
                qFileType += exporter->getExtension().c_str();
                qFileType += ")";
                exportFilters << qFileType;
            }
        }
        modelViewer->getExporter(origExportType);
        saveDialog = new QFileDialog(this,QString::fromWCharArray(TCLocalStrings::get(L"ExportModel")),".");
        saveDialog->setWindowIcon(QPixmap( ":/resources/LPub32.png"));

        saveDialog->setNameFilters(exportFilters);
        saveDialog->selectNameFilter(exportFilters.at(exportType - LDrawModelViewer::ETFirst));

        saveDialog->setFileMode(QFileDialog::AnyFile);
        saveDialog->setAcceptMode(QFileDialog::AcceptSave);
        saveDialog->setLabelText(QFileDialog::Accept,"Export");
        break;
    default:
        break;
    }
    saveDialog->selectFile(saveFilename);
    if (saveDialog->exec() == QDialog::Accepted)
    {
        QString selectedfile="";
        if (!saveDialog->selectedFiles().isEmpty())
        {
            selectedfile = saveDialog->selectedFiles()[0];
        }
        QString filename = selectedfile, dir = saveDialog->directory().path();
        switch (curSaveOp)
        {
        case LDPreferences::SOExport:
            TCUserDefaults::setPathForKey(dir.toLatin1().constData(), LAST_EXPORT_DIR_KEY, false);
            break;
        default:
            break;
        }
        QDir::setCurrent(dir);
        strncpy(saveFilename,filename.toLatin1().constData(),len);

        QString filter = saveDialog->selectedNameFilter();

        if (filter.indexOf(".pov") != -1)
        {
            exportType = LDrawModelViewer::ETPov;
        }
        if (filter.indexOf(".stl") != -1)
        {
            exportType = LDrawModelViewer::ETStl;
        }
#ifdef EXPORT_3DS
        if (filter.indexOf(".3ds") != -1)
        {
            exportType = LDrawModelViewer::ET3ds;
        }
#endif
        TCUserDefaults::setLongForKey(exportType, SAVE_EXPORT_TYPE_KEY, false);
        delete saveDialog;
        return true;
    }
    delete saveDialog;
    return false;
}

void LDVWidget::fileExport(const QString &saveFilename)
{
    curSaveOp = LDPreferences::SOExport;

    if (saveFilename.indexOf(".pov") != -1)
    {
        exportType = LDrawModelViewer::ETPov;
    }
    if (saveFilename.indexOf(".stl") != -1)
    {
        exportType = LDrawModelViewer::ETStl;
    }
#ifdef EXPORT_3DS
    if (saveFilename.indexOf(".3ds") != -1)
    {
        exportType = LDrawModelViewer::ET3ds;
    }
#endif
    TCUserDefaults::setLongForKey(exportType, SAVE_EXPORT_TYPE_KEY, false);
    if (!saveFilename.isEmpty())
    {
        modelViewer->setExportType(LDrawModelViewer::ExportType(exportType));
        modelViewer->exportCurModel(saveFilename.toLatin1().constData());
    }
}

void LDVWidget::fileExport()
{
    curSaveOp = LDPreferences::SOExport;
    char saveFilename[1024] = "";
    if (getSaveFilename(saveFilename, 1024) &&
       (!fileExists(saveFilename)||shouldOverwriteFile(saveFilename)))
    {
        modelViewer->setExportType(LDrawModelViewer::ExportType(exportType));
        modelViewer->exportCurModel(saveFilename);
    }
}
