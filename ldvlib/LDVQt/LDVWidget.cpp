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
#include <QProcess>
#include <QApplication>
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
#include "LDViewPartList.h"
#include <LDVAlertHandler.h>

#include "LDViewPartList.h"
#include <LDLib/LDPartsList.h>
#include <LDVWidgetDefaultKeys.h>
#include <LDVMisc.h>

#include <vector>
#include <string>
#include <assert.h>

#include <LDLoader/LDLPalette.h>
#include <LDLoader/LDLMainModel.h>

#ifdef Q_OS_WIN
#include <TCFoundation/TCTypedValueArray.h>
#include <LDVExtensionsSetup.h>
#endif // WIN32

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <functional>

#include "messageboxresizable.h"
#include "lpub_preferences.h"
#include "lpubalert.h"
#include "version.h"
#include "paths.h"

#define PNG_IMAGE_TYPE_INDEX 1
#define WINDOW_WIDTH_DEFAULT 640
#define WINDOW_HEIGHT_DEFAULT 480

LDVWidget* ldvWidget;
const QString iniFlagNames [] =
{
    "Native POV",
    "Native STL",
    "Native 3DS",
    "Native Part List",
    "LDView POV",
    "LDView"
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
        ldvAlertHandler(new LDVAlertHandler(this)),
        viewMode(LDInputHandler::VMExamine),
        commandLineSnapshotSave(false),
        saving(false)
#ifdef Q_OS_WIN
        ,savingFromCommandLine(false),
        hPBuffer(nullptr),
        hPBufferDC(nullptr),
        hPBufferGLRC(nullptr),
        hCurrentDC(nullptr),
        hCurrentGLRC(nullptr)
#endif

{
  iniFiles[NativePOVIni]   = { "Native POV",       Preferences::nativeExportIni };
  iniFiles[NativeSTLIni]   = { "Native STL",       Preferences::nativeExportIni };
  iniFiles[Native3DSIni]   = { "Native 3DS",       Preferences::nativeExportIni };
  iniFiles[NativePartList] = { "Native Part List", Preferences::nativeExportIni };
  iniFiles[LDViewPOVIni]   = { "LDView POV",       Preferences::ldviewPOVIni };
  iniFiles[LDViewIni]      = { "LDView",           Preferences::ldviewIni };

  setupLDVFormat();

  setupLDVContext();

  if (!setIniFile())
      return;

  QString messagesPath = QDir::toNativeSeparators(QString("%1%2")
                                                  .arg(Preferences::dataLocation)
                                                  .arg(VER_LDVMESSAGESINI_FILE));
  if (!TCLocalStrings::loadStringTable(messagesPath.toLatin1().constData()))
  {
      emit lpubAlert->messageSig(LOG_ERROR, QString("Could not load file %1")
                                 .arg(messagesPath));
  }

  LDLModel::setFileCaseCallback(staticFileCaseCallback);

  QString programPath = QCoreApplication::applicationFilePath();
  TCUserDefaults::setCommandLine(programPath.toLatin1().constData());

  TCUserDefaults::setAppName(Preferences::lpub3dAppName.toLatin1().constData());

  ldvWidget = this;

}

LDVWidget::~LDVWidget(void)
{
    TCAutoreleasePool::processReleases();
    ldvWidget = nullptr;
}

bool LDVWidget::doCommand(QStringList &arguments)
{
    TCUserDefaults::setCommandLine(arguments.join(" ").toLatin1().constData());

    QImage studImage(":/resources/StudLogo.png");
    TREMainModel::setRawStudTextureData(studImage.bits(),studImage.byteCount());

    bool retValue = true;
    if (!LDSnapshotTaker::doCommandLine(false, true))
    {
        if ((arguments.indexOf(QRegExp("^.*-ExportFile=.*$", Qt::CaseInsensitive), 0) != -1)){
            emit lpubAlert->messageSig(LOG_ERROR,QString("Failed to process Native export command arguments: %1").arg(arguments.join(" ")));
            retValue = false;
        } else if (iniFlag == NativePartList) {
            if (setupLDVApplication()) {
                doPartList();
            }
        }
    }
    return retValue;
}

bool LDVWidget::getUseFBO()
{
    if (snapshotTaker)
        return snapshotTaker->getUseFBO();
    else
        return false;
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
#ifdef Q_OS_WIN
                ldvPreferences = new LDVPreferences(this);
                ldvPreferences->doApply();
                hdc = ldvPreferences->hdc;
                TREGLExtensions::setup();
                setupMultisample();
                snapshotTaker = (LDSnapshotTaker*)alert->getSender()->retain();
                if (LDVExtensionsSetup::havePixelBufferExtension())
                {
                    snapshotTaker->setUseFBO(true);
                }
                delete ldvPreferences;
#else
                TREGLExtensions::setup();
                snapshotTaker = (LDSnapshotTaker*)alert->getSender()->retain();
                if (TREGLExtensions::haveFramebufferObjectExtension())
                {
                    snapshotTaker->setUseFBO(true);
                }
#endif
                if (!snapshotTaker->getUseFBO())
                {
                    setupSnapshotBackBuffer(WINDOW_WIDTH_DEFAULT, WINDOW_HEIGHT_DEFAULT);
                }
            }
        }
    }
}

bool LDVWidget::chDirFromFilename(const char *filename)
{
    const char *fileSpot = strrchr(filename, '/');
    bool retValue = false;

    if (fileSpot)
    {
        int len = fileSpot - filename;
        char *path = new char[len + 1];

        strncpy(path, filename, len);
        path[len] = 0;
        retValue = QDir::setCurrent(path);
        if (retValue)
        {
            ldvPreferences->setLastOpenPath(path);
        }
        delete[] path;
    }
    return retValue;
}

bool LDVWidget::setupLDVApplication(){

    // Saved session
    char *sessionName = TCUserDefaults::getSavedSessionNameFromKey(PREFERENCE_SET_KEY);
    if (sessionName && sessionName[0])
    {
          TCUserDefaults::setSessionName(sessionName, nullptr, false);
    }
    delete sessionName;

    modelViewer = new LDrawModelViewer(WINDOW_WIDTH_DEFAULT, WINDOW_HEIGHT_DEFAULT);

    if (! modelViewer)
        return false;

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

    bool retValue = true;

    if (iniFlag == NativePartList) {
        QString programPath = QCoreApplication::applicationFilePath();

        modelViewer->setProgramPath(programPath.toLatin1().constData());

        modelViewer->setNoUI(true);

        ldvPreferences = new LDVPreferences(this);
        ldvPreferences->doApply();

        inputHandler = modelViewer->getInputHandler();

        setViewMode(LDVPreferences::getViewMode(),
                    examineLatLong = LDVPreferences::getLatLongMode(),
                    keepRightSide = LDVPreferences::getKeepRightSideUp());

        TCStringArray *commandLine = (TCStringArray *)TCUserDefaults::getProcessedCommandLine();
        char *commandLineFilename = nullptr;

        TCUserDefaults::removeValue(HFOV_KEY, false);
        TCUserDefaults::removeValue(CAMERA_GLOBE_KEY, false);
        if (commandLine)
        {
            int i;
            int count = commandLine->getCount();
            for (i = 0; i < count && !commandLineFilename; i++)
            {
                char *arg = commandLine->stringAtIndex(i);

                if (arg[0] != '-')
                    commandLineFilename = arg;
                if (stringHasCaseInsensitivePrefix(arg, "-ca"))
                {
                    float value;

                    if (sscanf(arg + 3, "%f", &value) == 1)
                    {
                        TCUserDefaults::setFloatForKey(value, HFOV_KEY, false);
                    }
                }
                else if (stringHasCaseInsensitivePrefix(arg, "-cg"))
                {
                    TCUserDefaults::setStringForKey(arg + 3, CAMERA_GLOBE_KEY,false);
                }
            }
        }

        char *snapshotFilename =
            TCUserDefaults::stringForKey(SAVE_SNAPSHOT_KEY);
        commandLineSnapshotSave = (snapshotFilename ? true : false);
        QString current = QDir::currentPath();
        if (commandLineFilename)
        {
            QUrl qurl(commandLineFilename);
            if (qurl.scheme()=="file")
            {
                commandLineFilename=copyString(QUrl::fromPercentEncoding(qurl.toLocalFile().toLatin1().constData()).toLatin1().constData());
            }
            QFileInfo fi(commandLineFilename);
            commandLineFilename = copyString(fi.absoluteFilePath().toLatin1().constData());
            if (chDirFromFilename(commandLineFilename))
            {
                modelViewer->setFilename(commandLineFilename);
                if (! modelViewer->loadModel())
                {
                    emit lpubAlert->messageSig(LOG_ERROR, QString("Could not load command line file %1")
                                               .arg(commandLineFilename));
                    retValue = false;
                }
            }
            else
            {
                emit lpubAlert->messageSig(LOG_ERROR, QString("The directory containing the file %1 could not be found.")
                                           .arg(commandLineFilename));
                retValue = false;
            }
        }
        else
        {
            emit lpubAlert->messageSig(LOG_ERROR, QString("Command line file name was not specified."));
            retValue = false;
        }
    }
    return retValue;
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

void LDVWidget::showLDVExportOptions()
{
    setupLDVApplication();

    ldvExportOption = new LDViewExportOption(this);

    if (ldvExportOption->exec() == QDialog::Accepted)
        ldvExportOption->doOk();
    else
        ldvExportOption->doCancel();
}

void LDVWidget::showLDVPreferences()
{
    setupLDVApplication();

    ldvPreferences = new LDVPreferences(this);

    if (ldvPreferences->exec() == QDialog::Accepted)
        ldvPreferences->doOk();
    else
        ldvPreferences->doCancel();
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
        emit lpubAlert->messageSig(LOG_ERROR, QString("ERROR - The OpenGL context is not valid!"));
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
        emit lpubAlert->messageSig(LOG_STATUS, QString("%1")
                                   .arg(alert->getMessage()));
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

/*************************************************/
/*************************************************/

// Parts List

LDSnapshotTaker::ImageType LDVWidget::getSaveImageType(void)
{
    return LDSnapshotTaker::ITPng;
}

void LDVWidget::setViewMode(LDInputHandler::ViewMode value,
     bool examine, bool keep, bool /*saveSettings*/)
{
    viewMode = value;
    if (viewMode == LDInputHandler::VMExamine)
    {
        LDrawModelViewer::ExamineMode examineMode = ( examine ?
                LDrawModelViewer::EMLatLong : LDrawModelViewer::EMFree );
        inputHandler->setViewMode(LDInputHandler::VMExamine);
        modelViewer->setConstrainZoom(true);
        modelViewer->setExamineMode(examineMode);
    }
    else if (viewMode == LDInputHandler::VMFlyThrough)
    {
        inputHandler->setViewMode(LDInputHandler::VMFlyThrough);
        modelViewer->setConstrainZoom(false);
        modelViewer->setKeepRightSideUp(keep);
    }
    else if (viewMode == LDInputHandler::VMWalk)
    {
        inputHandler->setViewMode(LDInputHandler::VMWalk);
        modelViewer->setKeepRightSideUp(true);
    }
    LDVPreferences::setViewMode(viewMode);
}

bool LDVWidget::grabImage(
    int &imageWidth,
    int &imageHeight,
    bool fromCommandLine /*= false*/)
{
    int newWidth = 800;
    int newHeight = 600;
    int origWidth = mwidth;
    int origHeight = mheight;
    int numXTiles, numYTiles;
    bool origSlowClear = modelViewer->getSlowClear();
    int origMemoryUsage = modelViewer->getMemoryUsage();

    saving = true;
    modelViewer->setMemoryUsage(0);

    snapshotTaker->calcTiling(imageWidth, imageHeight, newWidth, newHeight,
        numXTiles, numYTiles);

    if (snapshotTaker->getUseFBO())
    {
#ifdef Q_OS_WIN
        ldvPreferences = new LDVPreferences(this);
        ldvPreferences->doApply();
        hdc = ldvPreferences->hdc;
        currentAntialiasType = TCUserDefaults::longForKey(FSAA_MODE_KEY);
        if (!setupPBuffer(newWidth, newHeight, currentAntialiasType))
        {
           newWidth = snapshotTaker->getFBOSize();
           newHeight = snapshotTaker->getFBOSize();
           setupSnapshotBackBuffer(newWidth, newHeight);
        }
#else
        newWidth = snapshotTaker->getFBOSize();
        newHeight = snapshotTaker->getFBOSize();
    }
    else
    {
       setupSnapshotBackBuffer(newWidth, newHeight);
#endif
    }

    imageWidth = newWidth * numXTiles;
    imageHeight = newHeight * numYTiles;
    saveImageWidth = imageWidth;
    saveImageHeight = imageHeight;
    if (snapshotTaker->getUseFBO())
    {
        makeCurrent();
        if (fromCommandLine)
        {
            saveImageResult = snapshotTaker->saveImage();
        }
        else
        {
            saveImageResult = snapshotTaker->saveImage(saveImageFilename,
                saveImageWidth, saveImageHeight, saveImageZoomToFit);
        }
    }
    else
    {
        renderPixmap(newWidth, newHeight);
    }
    makeCurrent();
    TREGLExtensions::setup();
    if (!snapshotTaker->getUseFBO())
    {
        modelViewer->openGlWillEnd();
    }
    saving = false;
    mwidth = origWidth;
    mheight = origHeight;
    modelViewer->setWidth(mwidth);
    modelViewer->setHeight(mheight);
    modelViewer->setMemoryUsage(origMemoryUsage);
    modelViewer->setSlowClear(origSlowClear);
    modelViewer->setup();
    return saveImageResult;
}

bool LDVWidget::saveImage(
    char *filename,
    int imageWidth,
    int imageHeight,
    bool fromCommandLine /*= false*/)
{
    bool retValue = false;

    if (!snapshotTaker)
    {
        if (fromCommandLine)
        {
           // The command line snapshot will be blank if LDSnapshotTaker has no argument
            snapshotTaker =  new LDSnapshotTaker(modelViewer);
        }
        else
        {
            snapshotTaker =  new LDSnapshotTaker(modelViewer);
        }
    }
#ifdef Q_OS_WIN
    if (LDVExtensionsSetup::havePixelBufferExtension())
    {
        snapshotTaker->setUseFBO(true);
    }
#else
    if (TREGLExtensions::haveFramebufferObjectExtension())
    {
        snapshotTaker->setUseFBO(true);
    }
#endif
    snapshotTaker->setImageType(getSaveImageType());
    snapshotTaker->setTrySaveAlpha(saveAlpha =
        TCUserDefaults::longForKey(SAVE_ALPHA_KEY, 0, false) != 0);
    snapshotTaker->setAutoCrop(
        TCUserDefaults::boolForKey(AUTO_CROP_KEY, false, false));
    saveImageFilename = filename;
    saveImageZoomToFit = TCUserDefaults::longForKey(SAVE_ZOOM_TO_FIT_KEY, 1,
        false);
    retValue = grabImage(imageWidth, imageHeight, fromCommandLine);
    return retValue;
}

void LDVWidget::doPartList(
    LDVHtmlInventory *htmlInventory,
    LDPartsList *partsList,
    const char *filename)
{
    if (htmlInventory->generateHtml(filename, partsList,
        modelViewer->getFilename()))
    {
        if (htmlInventory->isSnapshotNeeded())
        {
            char *snapshotPath = copyString(htmlInventory->getSnapshotPath());
            bool saveZoomToFit = modelViewer->getForceZoomToFit();
            bool saveActualSize = TCUserDefaults::longForKey(SAVE_ACTUAL_SIZE_KEY, 1, false);
            int saveWidth = TCUserDefaults::longForKey(SAVE_WIDTH_KEY, 1024, false);
            int saveHeight = TCUserDefaults::longForKey(SAVE_HEIGHT_KEY, 768, false);
            bool origSteps = TCUserDefaults::boolForKey(SAVE_STEPS_KEY, false, false);
            bool seams     = TCUserDefaults::boolForKey(SEAMS_KEY, false, false);
            int origStep = modelViewer->getStep();

            TCUserDefaults::setBoolForKey(seams, SEAMS_KEY, false);
            TCUserDefaults::setBoolForKey(false, SAVE_STEPS_KEY, false);
            modelViewer->setStep(modelViewer->getNumSteps());
            htmlInventory->prepForSnapshot(modelViewer);
            modelViewer->setForceZoomToFit(true);
            TCUserDefaults::setLongForKey(false, SAVE_ACTUAL_SIZE_KEY, false);
            TCUserDefaults::setLongForKey(400, SAVE_WIDTH_KEY, false);
            TCUserDefaults::setLongForKey(300, SAVE_HEIGHT_KEY, false);
            saveImageType = PNG_IMAGE_TYPE_INDEX;

            // By saying it's from the command line, none of the above settings
            // will be written to TCUserDefaults.
            savingFromCommandLine = false;
            saveImage(snapshotPath, 400, 300, savingFromCommandLine);
            delete snapshotPath;
            htmlInventory->restoreAfterSnapshot(modelViewer);
            modelViewer->setForceZoomToFit(saveZoomToFit);
            TCUserDefaults::setLongForKey(saveActualSize, SAVE_ACTUAL_SIZE_KEY, false);
            TCUserDefaults::setLongForKey(saveWidth, SAVE_WIDTH_KEY, false);
            TCUserDefaults::setLongForKey(saveHeight, SAVE_HEIGHT_KEY, false);
            modelViewer->setStep(origStep);
            TCUserDefaults::setBoolForKey(origSteps, SAVE_STEPS_KEY, false);
        }
    }
    else
    {
        emit lpubAlert->messageSig(LOG_STATUS, QString("%1")
                                   .arg(QString::fromWCharArray(
                                            TCLocalStrings::get(L"PLGenerateError"))
                                        ));
    }
}

void LDVWidget::doPartList(void)
{
    if (modelViewer)
    {
        LDPartsList *partsList = modelViewer->getPartsList();
        if (partsList)
        {
            QString htmlFilename;
            LDVHtmlInventory *htmlInventory = new LDVHtmlInventory;
            LDVPartList *ldvPartList = new LDVPartList(this, htmlInventory);
            if (ldvPartList->exec() == QDialog::Accepted)
            {
                QString initialDir = ldvPreferences->getSaveDir(LDPreferences::SOPartsList,
                                                                modelViewer->getFilename());
                QDir::setCurrent(initialDir);

                bool done = false;
                QString filename = modelViewer->getFilename();
                if (filename.isEmpty())
                {
                    emit lpubAlert->messageSig(LOG_STATUS, QString("No filename from modelViewer."));
                }
                int findSpot = filename.lastIndexOf((QRegExp("/\\")));
                if (findSpot >= 0 && findSpot < (int)filename.length())
                    filename=filename.mid(findSpot+1);
                findSpot = filename.lastIndexOf(('.'));
                if (findSpot >= 0 && findSpot < (int)filename.length())
                    filename=filename.left(findSpot);
                filename += ".html";
                findSpot = filename.lastIndexOf(('/'));
                if (findSpot >= 0 && findSpot < (int)filename.length())
                    filename = filename.mid(findSpot + 1);
                QString startWith = QString(htmlInventory->getLastSavePath()) +
                    QString("/") + filename;
                QString filter = QString(TCLocalStrings::get("HtmlFileType")) +
                    " (*.html)";
                while (!done)
                {
                    htmlFilename = QFileDialog::getSaveFileName(this,
                        QString::fromWCharArray(TCLocalStrings::get(L"GeneratePartsList")),
                        initialDir + "/" + filename,
                        filter,
                        nullptr,
                        QFileDialog::DontConfirmOverwrite);
                    if (htmlFilename.isEmpty())
                    {
                        done = true;
                    }
                    else
                    {
                        if (QFileInfo(htmlFilename).exists())
                        {
                            QPixmap _icon = QPixmap(":/icons/lpub96.png");
                            QMessageBoxResizable box;
                            box.setWindowIcon(QIcon());
                            box.setIconPixmap (_icon);
                            box.setTextFormat (Qt::RichText);
                            box.setStandardButtons (QMessageBox::Close);
                            box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
                            box.setWindowTitle(tr ("HTML Part List"));

                            box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
                            box.setDefaultButton   (QMessageBox::Yes);

                            QString prompt =
                                QString::fromWCharArray(TCLocalStrings::get(L"OverwritePrompt"));
                            prompt.replace("%s", htmlFilename);

                            QString title = "<b> HTML part list generation. </b>";
                            QString text = prompt;

                            box.setText (title);
                            box.setInformativeText (text);

                            if (Preferences::modeGUI && (box.exec() == QMessageBox::No)) {
                                 continue;
                            }
                        }
                        doPartList(htmlInventory, partsList,
                            htmlFilename.toLatin1().constData());
                        done = true;
                    }
                }
            }
            if (htmlInventory->getShowFileFlag() &&
                QFileInfo(htmlFilename).exists())
            {
                showWebPage(htmlFilename);
            }
            htmlInventory->release();
            partsList->release();
        }
    }
}

void LDVWidget::showWebPage(QString &htmlFilename){

  if (QFileInfo(htmlFilename).exists()){

      //display completion message
      QPixmap _icon = QPixmap(":/icons/lpub96.png");
      QMessageBoxResizable box;
      box.setWindowIcon(QIcon());
      box.setIconPixmap (_icon);
      box.setTextFormat (Qt::RichText);
      box.setStandardButtons (QMessageBox::Close);
      box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      box.setWindowTitle(tr ("HTML Part List"));

      box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
      box.setDefaultButton   (QMessageBox::Yes);

      QString title = "<b> HTML part list generated. </b>";
      QString text = tr ("Your HTML part list web page was generated successfully.\n\n"
                         "Do you want to open this document ?\n\n%1").arg(htmlFilename);

      box.setText (title);
      box.setInformativeText (text);

      if (Preferences::modeGUI && (box.exec() == QMessageBox::Yes)) {
          QString CommandPath = htmlFilename;
          QProcess *Process = new QProcess(this);
          Process->setWorkingDirectory(QDir::currentPath() + "/");
#ifdef Q_OS_WIN
          Process->setNativeArguments(CommandPath);
          QDesktopServices::openUrl((QUrl("file:///"+CommandPath, QUrl::TolerantMode)));
#else
          Process->execute(CommandPath);
          Process->waitForFinished();

          QProcess::ExitStatus Status = Process->exitStatus();

          if (Status != 0) {  // look for error
              QErrorMessage *m = new QErrorMessage(this);
              m->showMessage(QString("%1<br>%2").arg("Failed to launch HTML part list web page!").arg(CommandPath));
          }
#endif
          return;
        } else {
          emit lpubAlert->messageSig(LOG_STATUS, QString("HTML part list generation completed!")
                                                         .arg(QFileInfo(htmlFilename).baseName()));
          return;

        }
  } else {
      emit lpubAlert->messageSig(LOG_ERROR, QString("Generation failed for %1 HTML Part List.")
                                                    .arg(QFileInfo(htmlFilename).baseName()));
  }
}

void LDVWidget::setupSnapshotBackBuffer(int width, int height)
{
    modelViewer->setSlowClear(true);
    modelViewer->setWidth(width);
    modelViewer->setHeight(height);
    modelViewer->setup();
    glReadBuffer(GL_BACK);
}

void LDVWidget::setupMultisample(void)
{
#ifdef Q_OS_WIN
    if (LDVExtensionsSetup::haveMultisampleExtension())
    {
        if (currentAntialiasType)
        {
            if (TREGLExtensions::haveNvMultisampleFilterHintExtension())
            {
                if (ldvPreferences->getUseNvMultisampleFilter())
                {
                    glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
                }
                else
                {
                    glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_FASTEST);
                }
            }
            glEnable(GL_MULTISAMPLE_ARB);
        }
        else
        {
            if (TREGLExtensions::haveNvMultisampleFilterHintExtension())
            {
                glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_FASTEST);
            }
            glDisable(GL_MULTISAMPLE_ARB);
        }
    }
#endif
}

void LDVWidget::setupMaterial(void)
{
    // Don't call super.
}

void LDVWidget::setupLighting(void)
{
    modelViewer->setup();
}

void LDVWidget::cleanupOffscreen(void)
{
#ifdef Q_OS_WIN
    hCurrentDC = hdc;
    hCurrentGLRC = hglrc;
    if (hPBuffer)
    {
        cleanupPBuffer();
    }
    savingFromCommandLine = true;
    cleanupRenderSettings();
#endif
}

void LDVWidget::cleanupPBuffer(void)
{
#ifdef Q_OS_WIN
    if (hPBuffer)
    {
        using namespace TREGLExtensionsNS;

        if (hPBufferDC)
        {
            if (hPBufferGLRC)
            {
                wglDeleteContext(hPBufferGLRC);
                hPBufferGLRC = nullptr;
            }
            wglReleasePbufferDCARB(hPBuffer, hPBufferDC);
            hPBufferDC = nullptr;
        }
        wglDestroyPbufferARB(hPBuffer);
        hPBuffer = nullptr;
    }
#endif
}

void LDVWidget::cleanupRenderSettings(void)
{
    if (!savingFromCommandLine)
    {
        // If we're saving from the command line, there's no need to
        // put things back for regular rendering (particularly
        // recompiling the model, which takes quite a bit of extra
        // time.
        makeCurrent();
        modelViewer->setup();
    }
}

bool LDVWidget::setupPBuffer(int imageWidth, int imageHeight,
    bool antialias)
{
#ifndef Q_OS_WIN
    return true;
#else
    if (LDVExtensionsSetup::havePixelBufferExtension())
    {
        GLint intValues[] = {
            WGL_DRAW_TO_PBUFFER_ARB, GL_TRUE,
            WGL_RED_BITS_ARB, 8,
            WGL_GREEN_BITS_ARB, 8,
            WGL_BLUE_BITS_ARB, 8,
            WGL_ALPHA_BITS_ARB, 8,
            WGL_STENCIL_BITS_ARB, 2,
            0, 0,
            0, 0,
            0, 0
        };
        int index;

        if (antialias)
        {
            int offset = sizeof(intValues) / sizeof(GLint) - 6;

            intValues[offset++] = WGL_SAMPLES_EXT;
            intValues[offset++] = ldvPreferences->getFSAAFactor();
            intValues[offset++] = WGL_SAMPLE_BUFFERS_EXT;
            intValues[offset++] = GL_TRUE;
        }
        index = LDVExtensionsSetup::choosePixelFormat(hdc, intValues);
        if (index >= 0)
        {
            using namespace TREGLExtensionsNS;

            if (wglCreatePbufferARB && wglGetPbufferDCARB &&
                wglGetPixelFormatAttribivARB)
            {
                int pfSizeAttribs[3] = {
                    WGL_MAX_PBUFFER_WIDTH_ARB,
                    WGL_MAX_PBUFFER_HEIGHT_ARB,
                    WGL_MAX_PBUFFER_PIXELS_ARB
                };
                int attribValues[3];

                wglGetPixelFormatAttribivARB(hdc, index, 0, 3, pfSizeAttribs,
                    attribValues);
                // This shouldn't be necessary, but ATI returns a PBuffer even
                // if we ask for one that is too big, so we can't rely on their
                // failure to trigger failure.  The one it returns CLAIMS to be
                // the size we asked for; it just doesn't work right.
                if (attribValues[0] >= imageWidth &&
                    attribValues[1] >= imageHeight &&
                    attribValues[2] >= imageWidth * imageHeight)
                {
                    // Given the above check, the following shouldn't really
                    // matter, but I'll leave it in anyway.
                    GLint cbpIntValues[] = {
                        WGL_PBUFFER_LARGEST_ARB, 0,
                        0, 0
                    };
                    hPBuffer = wglCreatePbufferARB(hdc, index, imageWidth,
                        imageHeight, cbpIntValues);

                    if (hPBuffer)
                    {
                        hPBufferDC = wglGetPbufferDCARB(hPBuffer);
                        if (hPBufferDC)
                        {
                            hPBufferGLRC = wglCreateContext(hPBufferDC);

                            if (hPBufferGLRC)
                            {
                                wglShareLists(hglrc, hPBufferGLRC);
                                hCurrentDC = hPBufferDC;
                                hCurrentGLRC = hPBufferGLRC;
                                makeCurrent();
                                if (antialias)
                                {
                                    setupMultisample();
                                }
                                setupMaterial();
                                setupLighting();
                                glDepthFunc(GL_LEQUAL);
                                glEnable(GL_DEPTH_TEST);
                                glDrawBuffer(GL_FRONT);
                                glReadBuffer(GL_FRONT);
                                modelViewer->setWidth(imageWidth);
                                modelViewer->setHeight(imageHeight);
                                modelViewer->setup();
                                modelViewer->pause();
                                if (modelViewer->getNeedsReload())
                                {
                                    return true;
                                }
                                else
                                {
                                    // No need to recompile as before, because
                                    // we're sharing display lists.
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
        cleanupOffscreen();
        if (antialias)
        {
            return setupPBuffer(imageWidth, imageHeight, false);
        }
    }
    return false;
#endif
}
