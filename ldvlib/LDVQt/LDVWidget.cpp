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

#include "LDVWidget.h"

#include <QPainter>
#include <QDateTime>
#include <QFileInfo>
#include <QFileDialog>
#include <QProcess>
#include <QByteArray>
#include <QApplication>
#include <QErrorMessage>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QFile>
#include <QTextStream>
#include <QImageWriter>
#include <QTextDocument>
#include <QDialog>
#include <QMessageBox>
#include <QTimer>
#include <QWindow>
#include <QGLContext>

#include <string>
#include <assert.h>

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
#include <TCFoundation/TCProgressAlert.h>
#ifdef Q_OS_WIN
#include <TCFoundation/TCTypedValueArray.h>
#endif // WIN32

#include <LDLoader/LDLModel.h>
#include <LDLoader/LDLPalette.h>
#include <LDLoader/LDLMainModel.h>

#include <LDLib/LDUserDefaultsKeys.h>
#include <LDLib/LDrawModelViewer.h>
#include <LDLib/LDSnapshotTaker.h>
#include <LDLib/LDLibraryUpdater.h>
#include <LDLib/LDPartsList.h>
#include <LDLib/LDPreferences.h>

#include <TRE/TREMainModel.h>
#include <TRE/TREGLExtensions.h>

#include <LDVPreferences.h>
#include <LDViewExportOption.h>
#include "LDViewPartList.h"
#include <LDVAlertHandler.h>
#include <LDVWidgetDefaultKeys.h>
#include <LDVMisc.h>
#ifdef Q_OS_WIN
#include <LDVExtensionsSetup.h>
#endif // WIN32

#include "messageboxresizable.h"
#include "lpub_preferences.h"
#include "annotations.h"
#include "lpub_object.h"
#include "version.h"
#include "lc_http.h"
#include "lpub_qtcompat.h"

#define PNG_IMAGE_TYPE_INDEX 1
#define BMP_IMAGE_TYPE_INDEX 2
#define JPG_IMAGE_TYPE_INDEX 3
#define IMAGE_WIDTH_DEFAULT 800
#define IMAGE_HEIGHT_DEFAULT 600
#define WINDOW_WIDTH_DEFAULT 640
#define WINDOW_HEIGHT_DEFAULT 480
#define LDRAW_ZIP_SHOW_WARNING_KEY "LDrawZipShowWarning"

LDVWidget* ldvWidget;

const QString iniFlagNames [] =
{
	"Native POV",
	"Native STL",
	"Native 3DS",
	"Native Part List",
	"POV-Ray Render",
	"LDView POV",
	"LDView"
};

LDVWidget::LDVWidget(QWidget *parent, IniFlag iniflag, bool forceIni)
		: QGLWidget(parent),
		iniFlag(iniflag),
		forceIni(forceIni),
		darkTheme(Preferences::displayTheme == THEME_DARK),
		ldvContext(nullptr),
		ldvParent(parent),
		ldPrefs(nullptr),
		modelViewer(nullptr),
		snapshotTaker(nullptr),
		ldvAlertHandler(new LDVAlertHandler(this)),
		inputHandler(nullptr),
		modelFilename(nullptr),
		saveImageFilename(nullptr),
		imageInputFilename(nullptr),
		partListKey(nullptr),
		modelExt(nullptr),
		saveImageZoomToFit(false),
		commandLineSnapshotSave(false),
		viewMode(LDInputHandler::VMExamine),
		libraryUpdateWindow(nullptr),
#if !defined(_NO_BOOST) || defined(USE_CPP11)
		libraryUpdater(nullptr),
#endif
		libraryUpdateProgressReady(false),
		libraryUpdateTimer(0)
{
	iniFiles[NativePOVIni]	 = { iniFlagNames[NativePOVIni],   Preferences::nativeExportIni };
	iniFiles[NativeSTLIni]	 = { iniFlagNames[NativeSTLIni],   Preferences::nativeExportIni };
	iniFiles[Native3DSIni]	 = { iniFlagNames[Native3DSIni],   Preferences::nativeExportIni };
	iniFiles[NativePartList] = { iniFlagNames[NativePartList], Preferences::nativeExportIni };
	iniFiles[POVRayRender]	 = { iniFlagNames[POVRayRender],   Preferences::nativeExportIni };
	iniFiles[LDViewPOVIni]	 = { iniFlagNames[LDViewPOVIni],   Preferences::ldviewPOVIni };
	iniFiles[LDViewIni]		 = { iniFlagNames[LDViewIni],	   Preferences::ldviewIni };

	setupLDVFormat();

	setupLDVContext();

	if (!setIniFile())
		return;

	const QString ldvMessages = QDir::toNativeSeparators(QString("%1%2")
												  .arg(Preferences::dataLocation)
												  .arg(VER_LDVMESSAGESINI_FILE));
	if (!TCLocalStrings::loadStringTable(copyString(ldvMessages.toUtf8().constData())))
		emit lpub->messageSig(LOG_ERROR,
							  QString::fromWCharArray(TCLocalStrings::get(L"LoadLDVMessagesError")).arg(ldvMessages));

	LDLModel::setFileCaseCallback(staticFileCaseCallback);

	const QString programPath = QCoreApplication::applicationFilePath();
	TCUserDefaults::setCommandLine(copyString(programPath.toUtf8().constData()));

	TCUserDefaults::setAppName(copyString(Preferences::lpub3dAppName.toUtf8().constData()));

	if (LDVPreferences::getCheckPartTracker())
		LDVPreferences::setCheckPartTracker(false);

	setSession();

	if (!Preferences::ldrawLibPath.isEmpty())
	{
		const char *path = copyString(Preferences::ldrawLibPath.toUtf8().constData());
		TCUserDefaults::setStringForKey(path, LDRAWDIR_KEY, false);
	}
	if (!Preferences::lpub3dLibFile.isEmpty())
	{
		const char *path = copyString(Preferences::lpub3dLibFile.toUtf8().constData());
		TCUserDefaults::setStringForKey(path, LDRAWZIP_KEY, false);
	}

	ldvWidget = this;
}

LDVWidget::~LDVWidget(void)
{
	makeCurrent();
	TCAutoreleasePool::processReleases();
	doneCurrent();

	if (m_KeyListReply)
	{
		m_KeyListReply->abort();
		m_KeyListReply->deleteLater();
	}

	ldvWidget = nullptr;
}

void LDVWidget::showLDVPreferences()
{
	setupLDVApplication();

	LDVPreferences *ldvPreferences = new LDVPreferences(this, ldvParent);

	if (ldvPreferences->exec() == QDialog::Accepted)
		ldvPreferences->doOk();
	else
		ldvPreferences->doCancel();

	delete ldvPreferences;

	setHidden(true);
}

void LDVWidget::showLDVExportOptions()
{
	setupLDVApplication();

	LDViewExportOption *ldvExportOption = new LDViewExportOption(this, ldvParent);

	if (ldvExportOption->exec() == QDialog::Accepted)
		ldvExportOption->doOk();
	else
		ldvExportOption->doCancel();

	delete ldvExportOption;

	setHidden(true);
}

bool LDVWidget::setupLDVApplication()
{

	if (!TCUserDefaults::isIniFileSet())
		setIniFile();

	modelViewer = new LDrawModelViewer(WINDOW_WIDTH_DEFAULT, WINDOW_HEIGHT_DEFAULT);

	if (! modelViewer)
	{
		emit lpub->messageSig(LOG_ERROR,
							  QString::fromWCharArray(TCLocalStrings::get(L"InitModelViewerError")));
		return false;
	}

	ldPrefs = new LDPreferences(modelViewer);

	setSession(true/*savedSession*/);

	if (!setFontSettings())
		return false;

	return true;
}

void LDVWidget::setSession(bool savedSession)
{
	if (savedSession)
	{
		char *sessionName = TCUserDefaults::getSavedSessionNameFromKey(PREFERENCE_SET_KEY);
		if (sessionName && sessionName[0])
		{
			TCUserDefaults::setSessionName(sessionName, nullptr, false);
		}

		delete sessionName;

		return;
	}

	TCStringArray *sessionNames = TCUserDefaults::getAllSessionNames();

	const char *title = copyString(iniFiles[iniFlag].Title.toUtf8().constData());

	if (sessionNames->indexOfString(title) != -1)
	{
		TCUserDefaults::setSessionName(title, PREFERENCE_SET_KEY);
	}
	else
	{
		TCUserDefaults::setSessionName(nullptr, PREFERENCE_SET_KEY);
	}
	sessionNames->release();

	const char *prefSet = TCUserDefaults::getSessionName();
	emit lpub->messageSig(LOG_INFO,
						  QString::fromWCharArray(TCLocalStrings::get(L"PrefSetLoaded"))
							  .arg(prefSet ? QString(prefSet) : QString::fromWCharArray(TCLocalStrings::get(L"DefaultPrefSet"))));
}

bool LDVWidget::setFontSettings(void)
{
	QFile fontFile(":/resources/SansSerif.fnt");
	if (!fontFile.exists())
	{
		emit lpub->messageSig(LOG_ERROR, QString::fromWCharArray(TCLocalStrings::get(L"FontFileError")));
		return false;
	}

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
		free(buffer);
	}

	QImage fontImage2x(":/resources/SansSerif@2x.png");
	if (fontImage2x.isNull())
	{
		emit lpub->messageSig(LOG_ERROR, QString::fromWCharArray(TCLocalStrings::get(L"FontImageError")));
		return false;
	}

	long fontLength =
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
		fontImage2x.sizeInBytes();
#else
		fontImage2x.byteCount();
#endif

	modelViewer->setRawFont2xData(fontImage2x.bits(),fontLength);

	return true;
}

bool LDVWidget::setStudLogo(void)
{
	QImage studImage(":/resources/StudLogo.png");
	if (studImage.isNull())
	{
		emit lpub->messageSig(LOG_ERROR,
							  QString::fromWCharArray(TCLocalStrings::get(L"StudLogoFileError")));
		return false;
	}

	long studLength =
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
		studImage.sizeInBytes();
#else
		studImage.byteCount();
#endif

	TREMainModel::setRawStudTextureData(studImage.bits(),studLength);

	return true;
}

bool LDVWidget::setIni(IniFlag iniflag)
{
	forceIni = true;
	iniFlag = iniflag;
	if (!setIniFile())
		return false;
	setSession();
	return true;
}

bool LDVWidget::setIniFile(const char* value)
{
	return TCUserDefaults::setIniFile(value);
}

char *LDVWidget::getExportsDir(void)
{
	return TCUserDefaults::stringForKey(EXPORTS_DIR_KEY, nullptr,false);
}

bool LDVWidget::setIniFile()
{
	if (!TCUserDefaults::isIniFileSet() || forceIni)
	{
		if (getIniTitle().isEmpty())
		{
			emit lpub->messageSig(LOG_ERROR,
								  QString::fromWCharArray(TCLocalStrings::get(L"IniNotSpecifiedError")));
			return false;
		}

		const char *title = copyString(getIniFile().toUtf8().constData());

		if (!setIniFile(title))
		{
			emit lpub->messageSig(LOG_ERROR,
								  QString::fromWCharArray(TCLocalStrings::get(L"IniNotSetError")).arg(getIniTitle()).arg(getIniFile()));
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
	TCUserDefaults::setCommandLine(copyString(arguments.join(" ").toUtf8().constData()));

	setStudLogo();

	bool retValue = true;

	if (!LDSnapshotTaker::doCommandLine(false, true))
	{
		if ((arguments.indexOf(QRegExp("^.*-ExportFile=.*$", Qt::CaseInsensitive), 0) != -1))
		{
			emit lpub->messageSig(LOG_ERROR,
								  QString::fromWCharArray(TCLocalStrings::get(L"ExportCommandError")).arg(arguments.join(" ")));
			retValue = false;
		}
		else
		if (iniFlag == NativePartList)
		{
			if (setupLDVApplication())
			{
				if (setupPartList())
					doPartList();
			}
		}
	}

	char *snapshotFilename =
		TCUserDefaults::stringForKey(SAVE_SNAPSHOT_KEY);

	commandLineSnapshotSave = (snapshotFilename ? true : false);

	return retValue;
}

bool LDVWidget::getUseFBO()
{
	if (snapshotTaker)
		return snapshotTaker->getUseFBO();
	else
		return false;
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

	if (ldvContext->isValid())
	{
		setFormat(ldvFormat);
		needsInitialize = true;
	}
	else
	{
		emit lpub->messageSig(LOG_ERROR,
							  QString::fromWCharArray(TCLocalStrings::get(L"OpenGLContextError")));
	}

	if (needsInitialize)
	{
		initializeGLFunctions();
		//displayGLExtensions();
	}
}

bool LDVWidget::grabImage(int &imageWidth,
						  int &imageHeight,
						  bool fromCommandLine)
{
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
	if (fbo == NULL && (TCUserDefaults::longForKey(IGNORE_FRAMEBUFFER_OBJECT_KEY, 0, false)==0))
	{
		QOpenGLFramebufferObjectFormat fboFormat;
		GLsizei fboSize = 1024;
		fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
		fbo = new QOpenGLFramebufferObject(fboSize, fboSize, fboFormat);
		if (fbo->isValid() && fbo->bind())
		{
			snapshotTaker->setUseFBO(false);
			glViewport(0, 0, fboSize, fboSize);
			if (modelViewer->getMainTREModel() == NULL && !modelViewer->getNeedsReload())
			{
				modelViewer->loadModel(true);
			}
			inputHandler->stopRotation();
			bool retValue = snapshotTaker->saveImage(saveImageFilename, imageWidth, imageHeight, saveImageZoomToFit);
			fbo->release();
			delete fbo;
			fbo = NULL;
			return retValue;
		}
		else
		{
			delete fbo;
		}
	}
#endif

	if (!fromCommandLine)
	{
		makeCurrent();
		if (modelViewer->getMainTREModel() == NULL && !modelViewer->getNeedsReload())
		{
			modelViewer->loadModel(true);
		}
		bool retValue = snapshotTaker->saveImage(saveImageFilename, imageWidth, imageHeight, saveImageZoomToFit);

		return retValue;
	}

	int newWidth = IMAGE_WIDTH_DEFAULT;
	int newHeight = IMAGE_HEIGHT_DEFAULT;
	int origWidth = imageWidth;
	int origHeight = imageHeight;
	int numXTiles, numYTiles;
	int saveImageWidth, saveImageHeight;
	bool saveImageResult = false;
	bool origSlowClear = modelViewer->getSlowClear();
	int origMemoryUsage = modelViewer->getMemoryUsage();
	modelViewer->setMemoryUsage(0);

	if (snapshotTaker->getUseFBO())
	{
		newWidth = snapshotTaker->getFBOSize();
		newHeight = snapshotTaker->getFBOSize();
	}

	snapshotTaker->calcTiling(imageWidth, imageHeight, newWidth, newHeight, numXTiles, numYTiles);

	if (!snapshotTaker->getUseFBO())
	{
		setupSnapshotBackBuffer(newWidth, newHeight);
	}

	imageWidth = newWidth * numXTiles;
	imageHeight = newHeight * numYTiles;
	saveImageWidth = imageWidth;
	saveImageHeight = imageHeight;

	if (snapshotTaker->getUseFBO())
	{
		makeCurrent();
		saveImageResult = snapshotTaker->saveImage(saveImageFilename,
												   saveImageWidth, saveImageHeight, saveImageZoomToFit);
	}
	else
	{
#if (QT_VERSION >= 0x50400) && defined(QOPENGLWIDGET)
//      Need code for renderPixmap else saved snapshot image is garbage
#else
		renderPixmap(newWidth, newHeight);
#endif
	}

	makeCurrent();
	TREGLExtensions::setup();

	if (!snapshotTaker->getUseFBO())
	{
		modelViewer->openGlWillEnd();
	}

	modelViewer->setWidth(origWidth);
	modelViewer->setHeight(origHeight);
	modelViewer->setMemoryUsage(origMemoryUsage);
	modelViewer->setSlowClear(origSlowClear);
	modelViewer->setup();

	return saveImageResult;
}

void LDVWidget::setupSnapshotBackBuffer(int width, int height)
{
	modelViewer->setSlowClear(true);
	modelViewer->setWidth(width);
	modelViewer->setHeight(height);
	modelViewer->setup();
	glReadBuffer(GL_BACK);
}

bool LDVWidget::saveImage(
	char *filename,
	int imageWidth,
	int imageHeight)
{
	makeCurrent();
	TREGLExtensions::setup();
	if (!snapshotTaker)
	{
		snapshotTaker =	 new LDSnapshotTaker(modelViewer);
	}
	if (TREGLExtensions::haveFramebufferObjectExtension())
	{
		snapshotTaker->setUseFBO(true);
	}
	snapshotTaker->setImageType(getSaveImageType());
	snapshotTaker->setTrySaveAlpha(
		TCUserDefaults::longForKey(SAVE_ALPHA_KEY, 0, false));
	snapshotTaker->setAutoCrop(
		TCUserDefaults::boolForKey(AUTO_CROP_KEY, false, false));
	saveImageFilename = filename;
	saveImageZoomToFit = TCUserDefaults::longForKey(SAVE_ZOOM_TO_FIT_KEY, 1, false);

	return grabImage(imageWidth, imageHeight, true);
}

bool LDVWidget::loadModel(const char *filename)
{
	if (!filename)
		return false;

	filename = copyString(QFileInfo(filename).absoluteFilePath().toUtf8().constData());
	if (setDirFromFilename(filename))
	{
		emit lpub->messageSig(LOG_INFO_STATUS,
							  QString::fromWCharArray(TCLocalStrings::get(L"LoadingModelStatus"))
								  .arg(QFileInfo(filename).completeBaseName()));
		modelViewer->setFilename(filename);
		if (!modelViewer->loadModel())
		{
			emit lpub->messageSig(LOG_ERROR,
								  QString::fromWCharArray(TCLocalStrings::get(L"LoadingModelError"))
									  .arg(filename));
			return false;
		}
	}
	else
	{
		emit lpub->messageSig(LOG_ERROR,
							  QString::fromWCharArray(TCLocalStrings::get(L"FileDirectoryError"))
								  .arg(filename));
		return false;
	}

	return true;
}

void LDVWidget::snapshotTakerAlertCallback(TCAlert *alert)
{
	if (strcmp(alert->getAlertClass(), "LDSnapshotTaker") == 0)
	{
		if (strcmp(alert->getMessage(), "MakeCurrent") == 0)
		{
			if (isFboActive())
			{
				if (!fbo->isBound())
				{
					fbo->bind();
				}
			}
			else
			{
				makeCurrent();
			}
		}
		else
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
					setupSnapshotBackBuffer(WINDOW_WIDTH_DEFAULT, WINDOW_HEIGHT_DEFAULT);
				}
			}
		}
	}
}

LDSnapshotTaker::ImageType LDVWidget::getSaveImageType(void)
{
	switch (saveImageType)
	{
	case PNG_IMAGE_TYPE_INDEX:
		return LDSnapshotTaker::ITPng;
	case BMP_IMAGE_TYPE_INDEX:
		return LDSnapshotTaker::ITBmp;
	case JPG_IMAGE_TYPE_INDEX:
		return LDSnapshotTaker::ITJpg;
	default:
		return LDSnapshotTaker::ITPng;
	}
}

bool LDVWidget::setupPartList(void)
{

	QString programPath = QCoreApplication::applicationFilePath();

	modelViewer->setProgramPath(programPath.toUtf8().constData());

	modelViewer->setNoUI(true);

	inputHandler = modelViewer->getInputHandler();

	setViewMode(LDVPreferences::getViewMode(),
				LDVPreferences::getLatLongMode(),
				LDVPreferences::getKeepRightSideUp());

	TCStringArray *commandLine = (TCStringArray *)TCUserDefaults::getProcessedCommandLine();

	if (commandLine)
	{
		int i;
		int count = commandLine->getCount();
		for (i = 0; i < count && !modelFilename; i++)
		{
			char *arg = commandLine->stringAtIndex(i);

			if (arg[0] != '-')
				modelFilename = arg;
			if (stringHasCaseInsensitivePrefix(arg, "-ca"))
			{
				float value;

				if (sscanf(arg + 3, "%f", &value) == 1)
				{
					TCUserDefaults::removeValue(HFOV_KEY, false);
					TCUserDefaults::setFloatForKey(value, HFOV_KEY, false);
				}
			}
			else
			if (stringHasCaseInsensitivePrefix(arg, "-cg"))
			{
				bool ok;
				int lat,lon;
				float distance = -1.0f;
				QStringList cg(QString(arg + 3).split(","));
				lat = cg.at(0).toInt(&ok);
				if (ok) {
					TCUserDefaults::removeValue(LAST_LAT_KEY, false);
					TCUserDefaults::setFloatForKey(lat, LAST_LAT_KEY, false);
				}
				lon = cg.at(1).toInt(&ok);
				if (ok) {
					TCUserDefaults::removeValue(LAST_LON_KEY, false);
					TCUserDefaults::setFloatForKey(lon, LAST_LON_KEY, false);
				}
				modelViewer->setLatLon(lat,lon, distance);

				TCUserDefaults::removeValue(CAMERA_GLOBE_KEY, false);
				TCUserDefaults::setStringForKey(arg + 3, CAMERA_GLOBE_KEY,false);
			}
			else
			if (stringHasCaseInsensitivePrefix(arg, "-Snapshot="))
			{
				imageInputFilename = (arg + 10);
			}
			else
			if (stringHasCaseInsensitivePrefix(arg, "-PartlistKey="))
			{
				partListKey = (arg + 13);
				std::string tempStr = partListKey;
				size_t nDotSpot = tempStr.find_last_of('.');
				if (nDotSpot < tempStr.size())
				{
					size_t nInputSize = tempStr.size();
					modelExt = (nInputSize > 4) ? copyString(tempStr.substr(nInputSize - 4).c_str()) : ".ldr";
					partListKey = copyString(tempStr.substr(0, nDotSpot).c_str());
				}
			}
		}

		// apply and commit LDV settings
		LDVPreferences *ldvPreferences = new LDVPreferences(this);
		ldvPreferences->doApply();
		delete ldvPreferences;
	}

	return true;
}

void LDVWidget::doPartList(
	LDVHtmlInventory *htmlInventory,
	LDPartsList *partsList,
	const char *filename)
{
	std::string modelFile = filename;
	size_t nDotSpot = modelFile.find_last_of('.');
	if (nDotSpot < modelFile.size())
	{
		modelFile = modelFile.substr(0, nDotSpot);
		modelFile = modelFile.append(modelExt);
	}
	if (htmlInventory->generateHtml(filename, partsList, modelFile.c_str()))
	{
		if (htmlInventory->isSnapshotNeeded())
		{
			int imageWidth = htmlInventory->getSnapshotWidthFlag();
			int imageHeight = htmlInventory->getSnapshotHeightFlag();
			char *snapshotPath = copyString(htmlInventory->getSnapshotPath());

			QString userDefinedSnapshot = htmlInventory->getUserDefinedSnapshot();

			if (!userDefinedSnapshot.isEmpty())
			{
				QString snapshot = QDir::toNativeSeparators(snapshotPath);
				delete snapshotPath;
				emit lpub->messageSig(LOG_INFO_STATUS,
									  QString::fromWCharArray(TCLocalStrings::get(L"SnapshotImageStatus")).arg(userDefinedSnapshot));

				if (htmlInventory->getOverwriteSnapshotFlag())
				{
					QFile snapshotFile(snapshot);
					if (snapshotFile.exists() && !snapshotFile.remove())
					{
						emit lpub->messageSig(LOG_ERROR,
											  QString::fromWCharArray(TCLocalStrings::get(L"SnapshotFileRemoveError")).arg(snapshot));
						return;
					}
				}

				QImage image(userDefinedSnapshot);
				if (image.width() != imageWidth || image.height() != imageHeight)
					image.scaled(imageWidth, imageHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

				QImageWriter Writer(snapshot);
				if (!Writer.write(image))
				{
					emit lpub->messageSig(LOG_ERROR,
										  QString::fromWCharArray(TCLocalStrings::get(L"SnapshotWriteError"))
											  .arg(snapshot).arg(Writer.errorString()));
					return;
				}
			}
			else
			{
				emit lpub->messageSig(LOG_INFO_STATUS,
									  QString::fromWCharArray(TCLocalStrings::get(L"SnapshotGenerateStatus")));

				char *snapshotPath = copyString(htmlInventory->getSnapshotPath());
				bool saveZoomToFit = modelViewer->getForceZoomToFit();
				bool saveActualSize = TCUserDefaults::longForKey(SAVE_ACTUAL_SIZE_KEY, 1, false);
				int saveWidth = TCUserDefaults::longForKey(SAVE_WIDTH_KEY, 1024, false);
				int saveHeight = TCUserDefaults::longForKey(SAVE_HEIGHT_KEY, 768, false);
				bool origSteps = TCUserDefaults::boolForKey(SAVE_STEPS_KEY, false, false);
				bool seams = TCUserDefaults::boolForKey(SEAMS_KEY, false, false);
				int origStep = modelViewer->getStep();

				TCUserDefaults::setBoolForKey(seams, SEAMS_KEY, false);
				TCUserDefaults::setBoolForKey(false, SAVE_STEPS_KEY, false);
				modelViewer->setStep(modelViewer->getNumSteps());
				htmlInventory->prepForSnapshot(modelViewer);
				modelViewer->setForceZoomToFit(true);
				TCUserDefaults::setLongForKey(false, SAVE_ACTUAL_SIZE_KEY, false);
				TCUserDefaults::setLongForKey(imageWidth, SAVE_WIDTH_KEY, false);
				TCUserDefaults::setLongForKey(imageHeight, SAVE_HEIGHT_KEY, false);

				std::string trimmedFilename = imageInputFilename;
				if (trimmedFilename.front() == '"' && trimmedFilename.back() == '"')
				{
					trimmedFilename.erase(0, 1);
					trimmedFilename.pop_back();
				}

				if (!loadModel(trimmedFilename.c_str()))
				{
					emit lpub->messageSig(LOG_ERROR, QString::fromWCharArray(TCLocalStrings::get(L"SnapshotFileLoadError")));
				}

				saveImageType = PNG_IMAGE_TYPE_INDEX;
				saveImage(snapshotPath, imageWidth, imageHeight);
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
	}
	else
	{
		emit lpub->messageSig(LOG_STATUS, QString("%1")
								  .arg(QString::fromWCharArray(TCLocalStrings::get(L"PLGenerateError"))));
	}
}

void LDVWidget::doPartList(void)
{
	if (modelViewer)
	{
		if (!loadModel(modelFilename))
		{
			emit lpub->messageSig(LOG_ERROR,
								  QString::fromWCharArray(TCLocalStrings::get(L"PartListFileLoadError")).arg(modelFilename));
			return;
		}
		LDPartsList *partsList = modelViewer->getPartsList();
		if (partsList)
		{
			QString htmlFilename;
			LDVHtmlInventory *htmlInventory = new LDVHtmlInventory();
			LDVPartList *ldvPartList = new LDVPartList(this, htmlInventory);
			if (ldvPartList->exec() == QDialog::Accepted)
			{
				QString filename = modelViewer->getFilename();
				if (filename.isEmpty())
				{
					emit lpub->messageSig(LOG_STATUS, QString::fromWCharArray(TCLocalStrings::get(L"PartListFileNameError")));
				}
				filename = QFileInfo(filename).completeBaseName();
				filename = filename.left(int(filename.length()) - 6); // remove _parts
				filename += ".html";

				if (partListKey)
					htmlInventory->setPartListKey(partListKey);

				if (htmlInventory->getLookupSite() == LDVHtmlInventory::LookUp::Rebrickable)
				{
					// wire up downloads
					m_HttpManager = new lcHttpManager(this);

					connect(m_HttpManager, SIGNAL(DownloadFinished(lcHttpReply*)), this, SLOT(DownloadFinished(lcHttpReply*)));

					// set API key
					QString userKey = QString::fromStdString(htmlInventory->getRebrickableKey());
					if (userKey.isEmpty())
						doSetDefaultRebrickableKeys();
					else
						doSetRebrickableKey(userKey);

					// set model widget
					htmlInventory->setModelWidget(this);
				}

				QString initialDir = QString::fromStdString(ldPrefs->getInvLastSavePath());
				if (initialDir.isEmpty())
				{
					initialDir = QString::fromStdString(ldPrefs->getDefaultSaveDir(LDPreferences::SOPartsList,
																				   modelViewer->getFilename()).c_str());
					QDir cwd(initialDir);			 // <model>/LPub3D/tmp
					cwd.cdUp();						 // <model>/LPub3D
					cwd.cdUp();						 // <model>
					initialDir = cwd.absolutePath();
				}
				QDir::setCurrent(initialDir);

				QString filter = QString(TCLocalStrings::get("HtmlFileType")) +
					" (*.html)";

				bool done = false;
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
							box.setWindowTitle(QString::fromWCharArray(TCLocalStrings::get(L"PartListTitle")));

							box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
							box.setDefaultButton   (QMessageBox::Yes);

							QString prompt =
								QString::fromWCharArray(TCLocalStrings::get(L"OverwritePrompt"));
							prompt.replace("%s", htmlFilename);

							QString title = QString("<b> %1 </b>").arg(QString::fromWCharArray(TCLocalStrings::get(L"PartListGenTitle")));
							QString text = prompt;

							box.setText (title);
							box.setInformativeText (text);

							if (Preferences::modeGUI && (box.exec() == QMessageBox::No))
							{
								 continue;
							}
						}
						doPartList(htmlInventory, partsList, htmlFilename.toUtf8().constData());
						done = true;
					}
				}
			}
			if (htmlInventory->getShowFileFlag() && QFileInfo(htmlFilename).exists())
			{
				showDocument(htmlFilename);
			}
			htmlInventory->release();
			partsList->release();
		}
	}
}

void LDVWidget::showDocument(QString &htmlFilename)
{

	if (QFileInfo(htmlFilename).exists())
	{

		//display completion message
		QPixmap _icon = QPixmap(":/icons/lpub96.png");
		QMessageBoxResizable box;
		box.setWindowIcon(QIcon());
		box.setIconPixmap (_icon);
		box.setTextFormat (Qt::RichText);
		box.setStandardButtons (QMessageBox::Close);
		box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
		box.setWindowTitle(QString::fromWCharArray(TCLocalStrings::get(L"PartListTitle")));

		box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
		box.setDefaultButton	 (QMessageBox::Yes);

		QString title = QString("<b> %1 </b>").arg(QString::fromWCharArray(TCLocalStrings::get(L"PartListGenedTitle")));
		QString text = QString::fromWCharArray(TCLocalStrings::get(L"PartListGenedText")).arg(htmlFilename);

		box.setText (title);
		box.setInformativeText (text);

		if (Preferences::modeGUI && (box.exec() == QMessageBox::Yes))
		{
			const QString CommandPath = htmlFilename;
#ifdef Q_OS_WIN
			QDesktopServices::openUrl((QUrl("file:///"+CommandPath, QUrl::TolerantMode)));
#else
			QProcess *Process = new QProcess(this);
			Process->setWorkingDirectory(QFileInfo(CommandPath).absolutePath() + QDir::separator());
			QStringList arguments = QStringList() << CommandPath;
			Process->start(UNIX_SHELL, arguments);
			Process->waitForFinished();
			if (Process->exitStatus() != QProcess::NormalExit || Process->exitCode() != 0)
			{
				QErrorMessage *m = new QErrorMessage(this);
				m->showMessage(QString::fromWCharArray(TCLocalStrings::get(L"PartListLaunchError"))
								   .arg(CommandPath).arg(QString(Process->readAllStandardError())));
			}
#endif
			return;
		}
		else
		{
			emit lpub->messageSig(LOG_INFO_STATUS, QString::fromWCharArray(TCLocalStrings::get(L"PartListGenerateStatus"))
									  .arg(QFileInfo(htmlFilename).completeBaseName()));
			return;

		}
	}
	else
	{
		emit lpub->messageSig(LOG_ERROR, QString::fromWCharArray(TCLocalStrings::get(L"PartListGenerateError"))
								  .arg(QFileInfo(htmlFilename).completeBaseName()));
	}
}

void LDVWidget::modelViewerAlertCallback(TCAlert *alert)
{
	if (alert)
	{
		emit lpub->messageSig(LOG_STATUS, alert->getMessage());
	}
}

bool LDVWidget::setDirFromFilename(const char *filename)
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
			LDVPreferences::setLastOpenPath(path);
		}
		delete[] path;
	}
	return retValue;
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
			strcpy(filename, file.toUtf8().constData());
			return true;
		}
	}
	return false;
}

void LDVWidget::messageSig(LogType type, const QString &message, int i)
{
	emit lpub->messageSig(type, message, i);
}

void LDVWidget::libraryUpdateProgress(TCProgressAlert *alert)
{
	// NOTE: this gets called from inside one of the library update threads.  It
	// does NOT happen in the app's main thread.


	// Are we allowed to update widgets from outside the main thread? NOPE!
	//lock();
	//debugPrintf("Updater progress (%s): %f\n", alert->getMessage(),
	//	alert->getProgress());
	libraryUpdateProgressMessage = QString::fromWCharArray(alert->getWMessage());
	libraryUpdateProgressValue = alert->getProgress();
	libraryUpdateProgressReady = true;
	//unlock();
	if (alert->getProgress() == 1.0f)
	{
		// Progress of 1.0 means the library updater is done.
		if (alert->getExtraInfo())
		{
			// We can't call doLibraryUpdateFinished directly, because we're
			// executing from the library update thread.  The
			// doLibraryUpdateFinished function waits for the library update
			// thread to complete.	That will never happen if it's executing
			// inside the library update thread.  So we record the finish code,
			// tell ourselves we have done so, and the doLibraryUpdateTimer()
			// slot will take care of the rest when running in non-modal mode,
			// and the modal loop will take care of things when running in
			// modal mode (during initial library install).
			if (strcmp((*(alert->getExtraInfo()))[0], "None") == 0)
			{
				libraryUpdateFinishCode = LIBRARY_UPDATE_NONE;
			}
			else
			{
				libraryUpdateFinishCode = LIBRARY_UPDATE_FINISHED;
			}
		}
		else
		{
			libraryUpdateFinishCode = LIBRARY_UPDATE_CANCELED;
		}
		// Just as a note, while I believe that assignment of an int is an
		// atomic operation (and therefore doesn't require thread checking),
		// I'm not 100% sure of this.  So set the code first, and then once
		// it's definitely set, set the notification.  I really couldn't care
		// less if the notification setting is atomic, since I'm only doing a
		// boolean compare on it.
		ldrawLibraryUpdateFinishNotified = true;
	}
	else if (alert->getProgress() == 2.0f)
	{
		// Progress of 2.0 means the library updater encountered an
		// error.
		libraryUpdateFinishCode = LIBRARY_UPDATE_ERROR;
		ldrawLibraryUpdateFinishNotified = true;
	}
	if (ldrawLibraryUpdateCanceled)
	{
		alert->abort();
	}
}

void LDVWidget::progressAlertCallback(TCProgressAlert *alert)
{
	if (alert)
	{
		if (strcmp(alert->getSource(), "LDLibraryUpdater") == 0)
		{
			libraryUpdateProgress(alert);
		}
	}
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
				delete[] dirName;
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
		delete[] dirName;
	}
	else
	{
		shortName = filename;
	}
	return staticFileCaseLevel(dir, shortName);
}

bool LDVWidget::verifyLDrawDir(char *value)
{
	QString currentDir = QDir::currentPath();
	bool found = false;
	char buf[128];

	if (QDir::setCurrent(value))
	{
		strcpy(buf, "parts");
		if (staticFileCaseCallback(buf) && QDir::current().cd(buf))
		{
			QDir::setCurrent(value);
			strcpy(buf, "p");
			if (staticFileCaseCallback(buf) && QDir::current().cd(buf))
			{
				LDLModel::setLDrawDir(value);
				found = true;
			}
		}
		QDir::setCurrent(currentDir);
		if (LDVPreferences::getLDrawZipPath())
		{
			LDLModel::setLDrawDir(value);
			return true;
		}
	}
	return found;
}

bool LDVWidget::verifyLDrawDir(bool forceChoose)
{
	char *lDrawDir = getLDrawDir();
	bool found = false;

	if (!forceChoose &&
		(!TCUserDefaults::longForKey(VERIFY_LDRAW_DIR_KEY, 1, false) ||
		verifyLDrawDir(lDrawDir)))
	{
		delete lDrawDir;
		found = true;
	}
	else
	{
		if (commandLineSnapshotSave) return true;
		bool ans = true;
		if (!verifyLDrawDir(lDrawDir))
		{
			ans = (QMessageBox::question(this, "LDView",
				QString::fromWCharArray(TCLocalStrings::get(L"LDrawDirExistsPrompt")),
#if QT_VERSION >= QT_VERSION_CHECK(6,2,0)
					QMessageBox::Yes | QMessageBox::No , QMessageBox::No
#else
					QMessageBox::Yes, QMessageBox::No | QMessageBox::Default
#endif
				) == QMessageBox::Yes);
		}
		delete lDrawDir;
		if (ans)
		{
			while (!found)
			{
				if (promptForLDrawDir(QString()))
				{
					lDrawDir = getLDrawDir();
					if (verifyLDrawDir(lDrawDir))
					{
						found = true;
					}
					else
					{
						QMessageBox::warning(this,
							QString::fromWCharArray(TCLocalStrings::get(L"InvalidDir")),
							QString::fromWCharArray(TCLocalStrings::get(L"LDrawNotInDir")),
							QMessageBox::Ok, QMessageBox::NoButton);
					}
					delete lDrawDir;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			if (QMessageBox::warning(this,
				"LDView", QString::fromWCharArray(TCLocalStrings::get(L"WillDownloadLDraw")),
				QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok)
			{
				LDLModel::setLDrawDir("/");
				if (promptForLDrawDir(
					QString::fromWCharArray(TCLocalStrings::get(L"LDrawInstallDirPrompt"))))
				{
					if (installLDraw())
					{
						found = true;
					}
				}
			}
		}
	}
	return found;
}

char *LDVWidget::getLDrawZipPath(void)
{
	char *ldrawZip = LDVPreferences::getLDrawZipPath();

	if (!QFileInfo(ldrawZip).exists())
	{
		QString zipPath = QDir::toNativeSeparators("%1/complete.zip").arg(getLDrawDir());
		if (QFileInfo(zipPath).exists())
			ldrawZip = copyString(zipPath.toUtf8().constData());
		else
			ldrawZip = copyString("");
	}
	return ldrawZip;
}

char *LDVWidget::getLDrawDir(void)
{
	char *lDrawDir = LDVPreferences::getLDrawDir();

	if (!QFileInfo(lDrawDir).exists())
	{
		lDrawDir = copyString(getenv("LDRAWDIR"));
		if (!QFileInfo(lDrawDir).exists())
		{
#ifdef WIN32
			auto getWinDir = []() {
				QString path = QDir::toNativeSeparators("%1/LDraw").arg(getenv("USERPROFILE"));
				if (QFileInfo(path).exists())
					return path.toUtf8().constData();
				path = QDir::toNativeSeparators("%1/LDraw").arg(getenv("ALLUSERSPROFILE"));
				if (QFileInfo(path).exists())
					return path.toUtf8().constData();
				path = QString("C:\\LDraw");
				return path.toUtf8().constData();
			};

			lDrawDir = copyString(getWinDir());
#else
			lDrawDir = copyString("/usr/share/ldraw");
#endif
		}
	}
	stripTrailingPathSeparators(lDrawDir);
	return lDrawDir;
}

bool LDVWidget::promptForLDrawDir(QString prompt)
{
	char *initialDir = getLDrawDir();
	bool retValue = false;

	if (prompt.isEmpty())
	{
		prompt = QString::fromWCharArray(TCLocalStrings::get(L"LDrawDirPrompt"));
	}
	QDir::setCurrent(initialDir);
	QString selectedfile=QFileDialog::getExistingDirectory(this,prompt,".");
	if (!selectedfile.isEmpty())
	{
		LDVPreferences::setLDrawDir(selectedfile.toUtf8().constData());
		retValue = true;
	}
	return retValue;
}

void LDVWidget::checkForLibraryUpdates(void)
{
#if !defined(_NO_BOOST) || defined(USE_CPP11)
	bool showLDrawZipMsg = TCUserDefaults::boolForKey(LDRAW_ZIP_SHOW_WARNING_KEY, true, false);
	char *ldrawZip = getLDrawZipPath();
	if (ldrawZip && showLDrawZipMsg)
	{
		QMessageBox mb;
		QString title,message,zipPath=QString(ldrawZip);
		message=QString::fromWCharArray(TCLocalStrings::get(L"ReplaceLDrawZipMessage"));
		message.replace(QString("%s"),zipPath);
		mb.setText(message);
		mb.setWindowTitle(QString::fromWCharArray(TCLocalStrings::get(L"ReplaceLDrawZipTitle")));
		mb.addButton(QMessageBox::Yes);
		mb.addButton(QMessageBox::No);
		mb.setIcon(QMessageBox::Icon::Question);
		mb.setDefaultButton(QMessageBox::No);
#if QT_VERSION >= QT_VERSION_CHECK(5,2,0)
		QCheckBox *cb = new QCheckBox(QString::fromWCharArray(TCLocalStrings::get(L"DoNotShowMessage")));
		mb.setCheckBox(cb);
		QObject::connect(cb, &QCheckBox::stateChanged, [&](int state) {
		showLDrawZipMsg = (static_cast<Qt::CheckState>(state) != Qt::CheckState::Checked); });
#endif
		mb.exec();
		TCUserDefaults::setBoolForKey(showLDrawZipMsg, LDRAW_ZIP_SHOW_WARNING_KEY, false);
		if (mb.result()==QMessageBox::No) { return; }
	}
	if (libraryUpdater)
	{
		showLibraryUpdateWindow(false);
	}
	else
	{
		libraryUpdater = new LDLibraryUpdater;
		char *ldrawDir = getLDrawDir();
		char *ldrawZip = getLDrawZipPath();
		wchar_t *updateCheckError = NULL;

		ldrawLibraryUpdateCanceled = false;
		ldrawLibraryUpdateFinishNotified = false;
		ldrawLibraryUpdateFinished = false;
		libraryUpdater->setLibraryUpdateKey(LAST_LIBRARY_UPDATE_KEY);
		libraryUpdater->setLdrawDir(ldrawDir);
		libraryUpdater->setLdrawZipPath(ldrawZip);
		delete ldrawDir;
		delete(ldrawZip);

		if (libraryUpdater->canCheckForUpdates(updateCheckError))
		{
			showLibraryUpdateWindow(false);
			if (!libraryUpdateTimer)
			{
				libraryUpdateTimer = startTimer(50);
			}
			libraryUpdater->checkForUpdates();
		}
		else
		{
			QString qs;
			wcstoqstring(qs, updateCheckError);
			QMessageBox::warning(this,"LDView", qs,
				QMessageBox::Ok, QMessageBox::NoButton);
			delete updateCheckError;
		}
	}
#endif // _NO_BOOST
}

bool LDVWidget::installLDraw(void)
{
#if !defined(_NO_BOOST) || defined(USE_CPP11)
	if (libraryUpdater)
	{
		return false;
	}
	else
	{
		char *ldrawParentDir = getLDrawDir();
		char *ldrawDir = copyString(ldrawParentDir, 255);
		QDir originalDir = QDir::current();
		bool progressDialogClosed = false;

		ldrawLibraryUpdateFinished = false;
		strcat(ldrawDir, "/ldraw");

		QDir dir(ldrawDir);
		if (!dir.exists())
		{
			dir.mkdir(ldrawDir);
		}
		libraryUpdater = new LDLibraryUpdater;
		ldrawLibraryUpdateCanceled = false;
		ldrawLibraryUpdateFinishNotified = false;
		ldrawLibraryUpdateFinished = false;
		libraryUpdater->setLibraryUpdateKey(LAST_LIBRARY_UPDATE_KEY);
		libraryUpdater->setLdrawDir(ldrawDir);
		libraryUpdater->installLDraw();
		showLibraryUpdateWindow(true);
		if (!libraryUpdateTimer)
		{
			libraryUpdateTimer = startTimer(50);
		}
		while (libraryUpdater || !progressDialogClosed)
		{
			// We want the update window to be modal, so process events in a
			// tight modal loop.  (See modal section in QProgressDialog
			// documentation.)
			qApp->processEvents();
			if (!progressDialogClosed && libraryUpdateWindow->wasCanceled())
			{
				progressDialogClosed = true;
				// When the install finishes for real, we change the button
				// title from "Cancel" to "OK".	 However, it still acts like
				// a cancel.  So check to se if the update really finished, and
				// if it didn't, then note that the user canceled.
				if (!ldrawLibraryUpdateFinished)
				{
					ldrawLibraryUpdateCanceled = true;
					libraryUpdateFinished(LIBRARY_UPDATE_CANCELED);
					//libraryUpdateCanceled = false;
				}
				break;
			}
			if (ldrawLibraryUpdateFinishNotified)
			{
				libraryUpdateFinished(libraryUpdateFinishCode);
			}
			// Sleep for 50ms.	Unlike the QProgressDialog example, we aren't
			// doing anything inside this loop other than processing the
			// events.	All the work is happening in other threads.	 So sleep
			// for a short time in order to avoid monopolizing the CPU.	 Keep in
			// mind that while 50ms is essentially unnoticable to a user, it's
			// quite a long time to the computer.
#ifdef WIN32
			Sleep(50);
#else // WIN32
			usleep(50000);
#endif // WIN32
		}
		if (ldrawLibraryUpdateFinished)
		{
			LDLModel::setLDrawDir(ldrawDir);
			LDVPreferences::setLDrawDir(ldrawDir);
		}
		delete ldrawDir;
		return ldrawLibraryUpdateFinished;
	}
#endif // _NO_BOOST
}

void LDVWidget::libraryUpdateFinished(int finishType)
{
#if !defined(_NO_BOOST) || defined(USE_CPP11)
	if (libraryUpdater)
	{
		QString statusText;

		libraryUpdateWindow->setCancelButtonText(QString::fromWCharArray(TCLocalStrings::get(L"OK")));
		setLibraryUpdateProgress(1.0f);
		if (libraryUpdater->getError() && ucstrlen(libraryUpdater->getError()))
		{
			QString qError;

			statusText = QString::fromWCharArray(TCLocalStrings::get(L"LibraryUpdateError"));
			statusText += ":\n";
			ucstringtoqstring(qError, libraryUpdater->getError());
			statusText += qError;
		}
		switch (finishType)
		{
		case LIBRARY_UPDATE_FINISHED:
			ldrawLibraryUpdateFinished = true;
			statusText = QString::fromWCharArray(TCLocalStrings::get(L"LibraryUpdateComplete"));
			break;
		case LIBRARY_UPDATE_CANCELED:
			statusText = QString::fromWCharArray(TCLocalStrings::get(L"LibraryUpdateCanceled"));
			break;
		case LIBRARY_UPDATE_NONE:
			statusText = QString::fromWCharArray(TCLocalStrings::get(L"LibraryUpdateUnnecessary"));
			break;
		}
		debugPrintf("About to release library updater.\n");
		libraryUpdater->release();
		debugPrintf("Released library updater.\n");
		libraryUpdater = nullptr;
		if (statusText.length())
		{
			libraryUpdateWindow->setLabelText(statusText);
		}
	}
#endif // _NO_BOOST
}

void LDVWidget::showLibraryUpdateWindow(bool initialInstall)
{
#if !defined(_NO_BOOST) || defined(USE_CPP11)
	if (!libraryUpdateWindow)
	{
		createLibraryUpdateWindow();
	}
	libraryUpdateWindow->setCancelButtonText(QString::fromWCharArray(TCLocalStrings::get(L"Cancel")));
	libraryUpdateWindow->reset();
	libraryUpdateWindow->show();
	if (initialInstall)
	{
		libraryUpdateWindow->setModal(true);
	}
	else
	{
		libraryUpdateWindow->setModal(false);
		connect(libraryUpdateWindow, SIGNAL(canceled()), this,
			SLOT(doLibraryUpdateCanceled()));
	}
#endif // _NO_BOOST
}

void LDVWidget::createLibraryUpdateWindow(void)
{
	if (!libraryUpdateWindow)
	{
		libraryUpdateWindow = new QProgressDialog(
						QString::fromWCharArray(TCLocalStrings::get(L"CheckingForUpdates")),
						QString::fromWCharArray(TCLocalStrings::get(L"Cancel")),
						0,100,this);
		libraryUpdateWindow->setMinimumDuration(0);
		libraryUpdateWindow->setAutoReset(false);
	}
}

void LDVWidget::setLibraryUpdateProgress(float progress)
{
	libraryUpdateWindow->setValue((int)(progress * 100));
}

void LDVWidget::doLibraryUpdateCanceled(void)
{
	ldrawLibraryUpdateCanceled = true;
}

void LDVWidget::timerEvent(QTimerEvent* event)
{
	if (event->timerId() == libraryUpdateTimer)
	{
		if (ldrawLibraryUpdateFinishNotified)
		{
			killTimer(libraryUpdateTimer);
			libraryUpdateTimer = 0;
			libraryUpdateFinished(libraryUpdateFinishCode);
		}
		else if (!ldrawLibraryUpdateCanceled)
		{
			if (libraryUpdateProgressReady)
			{
				libraryUpdateProgressReady = false;
				libraryUpdateWindow->setLabelText(libraryUpdateProgressMessage);
				libraryUpdateProgressMessage = "";
				setLibraryUpdateProgress(libraryUpdateProgressValue);
			}
		}
	}
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
}

void LDVWidget::displayGLExtensions()
{
#ifdef QT_DEBUG_MODE
	QString openGLString, countString;
	int extensionCount;
	UCSTR temp = LDrawModelViewer::getOpenGLDriverInfo(extensionCount);
	ucstringtoqstring(openGLString, temp);
	QStringList list = openGLString.split(QRegExp("\n|\r\n|\r"), SkipEmptyParts);
	countString = QString::number(extensionCount);
	countString += QString::fromWCharArray((TCLocalStrings::get(L"OpenGlnExtensionsSuffix")));

	emit lpub->messageSig(LOG_INFO, QString::fromWCharArray(TCLocalStrings::get(L"OpenGLValidExtensions")).arg(countString));
	Q_FOREACH (const QString &item, list)
	{
		emit lpub->messageSig(LOG_INFO, item);
	}
#endif
}

int LDVWidget::doGetRebrickableColor(const int LDrawColorID) const
{
	int colorMatch = Annotations::getRBColorID(QString::number(LDrawColorID));
	if (colorMatch != -1)
		return colorMatch;

	return	LDrawColorID;
}

std::string LDVWidget::doGetRebrickablePartURL(const std::string &LDrawPartID, bool alt) const
{
	QJsonDocument Document = QJsonDocument::fromJson(m_RebrickableParts);
	QJsonObject Root = Document.object();
	QJsonArray Parts = Root["results"].toArray();
	// Converting QByteArray to QString Utf8 constData
	// because QByteArray.toStdString() is only available
	//if Qt is configured with STL compatibility enabled.
	std::string utf8String;
	for (const QJsonValue& Part : Parts)
	{
		// primary check
		QJsonObject PartObject = Part.toObject();
		QByteArray RBPartUrl = PartObject["part_url"].toString().toLatin1();
		QByteArray RBPartCode = PartObject["part_num"].toString().toLatin1();
		utf8String = QString(RBPartCode).toUtf8().constData();
		if (LDrawPartID == utf8String)
		{
			emit lpub->messageSig(LOG_INFO, QString::fromWCharArray(TCLocalStrings::get(L"RebrickablePartID"))
									   .arg(QString::fromStdString(LDrawPartID))
									   .arg(QString(RBPartCode))
									   .arg(QString(RBPartUrl)));
			utf8String = QString(RBPartUrl).toUtf8().constData();
			return utf8String;
		}
		// secondary check
		QJsonArray PartIDArray = PartObject["external_ids"].toObject()["LDraw"].toArray();
		if (!PartIDArray.isEmpty())
		{
			for (int i = 0; i < PartIDArray.size(); i++)
			{
				QByteArray LDPartCode = PartIDArray[i].toString().toLatin1();
				utf8String = QString(LDPartCode).toUtf8().constData();
				if (LDrawPartID == utf8String)
				{
					emit lpub->messageSig(LOG_INFO, QString::fromWCharArray(TCLocalStrings::get(L"RebrickableLDrawPartCode"))
											   .arg(QString(LDPartCode))
											   .arg(QString(RBPartCode))
											   .arg(QString(RBPartUrl)));
					std::string utf8String = QString(RBPartUrl).toUtf8().constData();
					return utf8String;
				}
			}
		}
	}
	// final check - if we get here, check cross-reference
	std::string altPart = Annotations::getRBPartID(QString::fromStdString(LDrawPartID)).toStdString();
	if (!alt && !altPart.empty())
		doGetRebrickablePartURL(altPart, true);

	return std::string("");
}

void LDVWidget::DownloadFinished(lcHttpReply* Reply)
{
	if (Reply == m_KeyListReply)
	{
		if (!Reply->error())
		{
			QJsonDocument Document = QJsonDocument::fromJson(Reply->readAll());
			QJsonObject Root = Document.object();

			int Version = Root["Version"].toInt();

			if (Version == 1)
			{
				QJsonArray Keys = Root["Keys"].toArray();

				for (const QJsonValue& Key : Keys)
					m_Keys.append(Key.toString());
			}
		}

		if (m_Keys.isEmpty())
		{
			emit lpub->messageSig(LOG_ERROR,
								  QString::fromWCharArray(TCLocalStrings::get(L"RebrickableServerError")));
			close();
		}

		m_KeyListReply = nullptr;
	}
	else if (Reply == m_PartsReply)
	{
		if (!Reply->error())
			m_RebrickableParts = Reply->readAll();
		else
			emit lpub->messageSig(LOG_ERROR,
								  QString::fromWCharArray(TCLocalStrings::get(L"RebrickableDownloadError")));

		m_PartsReply = nullptr;
	}

	Reply->deleteLater();
}

void LDVWidget::doSetRebrickableParts(const QString &parts)
{
	QProgressDialog ProgressDialog(this);
	ProgressDialog.setWindowTitle(QString::fromWCharArray(TCLocalStrings::get(L"RebrickableTitle")));
	ProgressDialog.setLabelText(QString::fromWCharArray(TCLocalStrings::get(L"RebrickableText")));
	ProgressDialog.setMaximum(0);
	ProgressDialog.setMinimum(0);
	ProgressDialog.setValue(0);
//	  ProgressDialog.show();

	if (m_Keys.isEmpty())
		return;

	QString partNumbers = QString(parts).replace(",","% 2C");

	int KeyIndex = m_Keys.size() > 1 ? QTime::currentTime().msec() % m_Keys.size() : 0 ;

	QString SearchUrl = QString("%1/parts/?part_nums=%2&key=%3")
							.arg(VER_REBRICKABLE_API_URL)
							.arg(partNumbers)
							.arg(m_Keys[KeyIndex]).replace("% 2C","%2C");

	m_PartsReply = m_HttpManager->DownloadFile(SearchUrl);

	while (m_PartsReply)
	{
		QApplication::processEvents();
		if (ProgressDialog.wasCanceled())
		{
			m_PartsReply->abort();
			m_PartsReply->deleteLater();
			m_PartsReply = nullptr;
			return;
		}
	}
}

void LDVWidget::doSetDefaultRebrickableKeys()
{
	const char* defaultKeyUrl(VER_REBRICKABLE_KEYS_JSON_URL);
	m_KeyListReply = m_HttpManager->DownloadFile(QLatin1String(defaultKeyUrl));
}
