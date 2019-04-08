/****************************************************************************
**
** Copyright (C) 2018 - 2019 Trevor SANDY. All rights reserved.
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
#include <QErrorMessage>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QFile>
#include <QTextStream>
#include <QImageWriter>
#include <functional>

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
    "POV-Ray Render",
    "LDView POV",
    "LDView"
};

LDVWidget::LDVWidget(QWidget *parent, IniFlag iniflag, bool forceIni)
        : QGLWidget(parent),
        iniFlag(iniflag),
        forceIni(forceIni),
        ldvFormat(nullptr),
        ldvContext(nullptr),
        ldPrefs(nullptr),
        modelViewer(nullptr),
        snapshotTaker(nullptr),
        ldvAlertHandler(new LDVAlertHandler(this)),
        inputHandler(nullptr),
        modelFilename(nullptr),
        saveImageFilename(nullptr),
        imageInputFilename(nullptr),
        viewMode(LDInputHandler::VMExamine)
{
  iniFiles[NativePOVIni]   = { iniFlagNames[NativePOVIni],   Preferences::nativeExportIni };
  iniFiles[NativeSTLIni]   = { iniFlagNames[NativeSTLIni],   Preferences::nativeExportIni };
  iniFiles[Native3DSIni]   = { iniFlagNames[Native3DSIni],   Preferences::nativeExportIni };
  iniFiles[NativePartList] = { iniFlagNames[NativePartList], Preferences::nativeExportIni };
  iniFiles[POVRayRender]   = { iniFlagNames[POVRayRender],   Preferences::nativeExportIni };
  iniFiles[LDViewPOVIni]   = { iniFlagNames[LDViewPOVIni],   Preferences::ldviewPOVIni };
  iniFiles[LDViewIni]      = { iniFlagNames[LDViewIni],      Preferences::ldviewIni };

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

  if (LDVPreferences::getCheckPartTracker())
      LDVPreferences::setCheckPartTracker(false);

  TCStringArray *sessionNames =
      TCUserDefaults::getAllSessionNames();

  if (sessionNames->indexOfString(iniFiles[iniFlag].Title.toLatin1().constData()) != -1)
  {
      TCUserDefaults::setSessionName(iniFiles[iniFlag].Title.toLatin1().constData(),
          PREFERENCE_SET_KEY);
  }
  else
  {
      TCUserDefaults::setSessionName(nullptr, PREFERENCE_SET_KEY);
  }
  sessionNames->release();

  QString prefSet = TCUserDefaults::getSessionName();
  emit lpubAlert->messageSig(LOG_INFO, QString("LDV loaded '%1' preference set")
                             .arg(prefSet.isEmpty() ? "Default" : prefSet));

  ldvWidget = this;
}

LDVWidget::~LDVWidget(void)
{
    makeCurrent();
    TCAutoreleasePool::processReleases();

    doneCurrent();
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
        } else
        if (iniFlag == NativePartList) {
            if (setupLDVApplication()) {
                if (setupPartList())
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

bool LDVWidget::setupLDVApplication(){

    if (!TCUserDefaults::isIniFileSet())
        setIniFile();

    modelViewer = new LDrawModelViewer(WINDOW_WIDTH_DEFAULT, WINDOW_HEIGHT_DEFAULT);

    if (! modelViewer) {
        emit lpubAlert->messageSig(LOG_ERROR, QString("Unable to initialize LDVApplication modelViewer"));
        return false;
    }

    ldPrefs = new LDPreferences(modelViewer);

    char *sessionName = TCUserDefaults::getSavedSessionNameFromKey(PREFERENCE_SET_KEY);
    if (sessionName && sessionName[0])
    {
          TCUserDefaults::setSessionName(sessionName, nullptr, false);
    }
    delete sessionName;

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

    return true;
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

    LDViewExportOption *ldvExportOption = new LDViewExportOption(this);

    if (ldvExportOption->exec() == QDialog::Accepted)
        ldvExportOption->doOk();
    else
        ldvExportOption->doCancel();
}

void LDVWidget::showLDVPreferences()
{
    setupLDVApplication();

    LDVPreferences *ldvPreferences = new LDVPreferences(this);

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

bool LDVWidget::setupPartList(void){

    QString programPath = QCoreApplication::applicationFilePath();

    modelViewer->setProgramPath(programPath.toLatin1().constData());

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
            else if (stringHasCaseInsensitivePrefix(arg, "-cg"))
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
            else if (stringHasCaseInsensitivePrefix(arg, "-Snapshot="))
            {
                imageInputFilename = (arg + 10);
            }
        }

        // apply and commit LDV settings
        LDVPreferences *ldvPreferences = new LDVPreferences(this);
        ldvPreferences->doApply();
        delete ldvPreferences;
    }

    return true;
}

bool LDVWidget::loadModel(const char *filename)
{
    if (!filename)
        return false;

    bool retValue = true;

    QFileInfo fi(filename);
    filename = copyString(fi.absoluteFilePath().toLatin1().constData());
    if (setDirFromFilename(filename))
    {
        emit lpubAlert->messageSig(LOG_STATUS, QString("Loading %1. Please wait...")
                                   .arg(QFileInfo(filename).baseName()));
        modelViewer->setFilename(filename);
        if (! modelViewer->loadModel())
        {
            emit lpubAlert->messageSig(LOG_ERROR, QString("Could not load command line file %1")
                                       .arg(filename));
            retValue = false;
        }
    }
    else
    {
        emit lpubAlert->messageSig(LOG_ERROR, QString("The directory containing the file %1 could not be found.")
                                   .arg(filename));
        retValue = false;
    }
    return retValue;
}

bool LDVWidget::grabImage(
    int &imageWidth,
    int &imageHeight)
{
    bool ok = false;
    modelViewer->setMemoryUsage(0);

    if (snapshotTaker->getUseFBO())
    {
        makeCurrent();
        ok = snapshotTaker->saveImage(saveImageFilename,
            imageWidth, imageHeight, saveImageZoomToFit);
    }
    else
    {
        setupSnapshotBackBuffer(imageWidth, imageHeight);
        renderPixmap(imageWidth, imageHeight);
    }
    return ok;
}

bool LDVWidget::saveImage(
    char *filename,
    int imageWidth,
    int imageHeight)
{
    bool retValue = false;
    makeCurrent();
    TREGLExtensions::setup();
    if (!snapshotTaker)
    {
        snapshotTaker =  new LDSnapshotTaker(modelViewer);
    }
    if (TREGLExtensions::haveFramebufferObjectExtension())
    {
        snapshotTaker->setUseFBO(true);
    }
    snapshotTaker->setImageType(LDSnapshotTaker::ITPng);
    snapshotTaker->setTrySaveAlpha(
        TCUserDefaults::longForKey(SAVE_ALPHA_KEY, 0, false));
    snapshotTaker->setAutoCrop(
        TCUserDefaults::boolForKey(AUTO_CROP_KEY, false, false));
    saveImageFilename = filename;
    saveImageZoomToFit = TCUserDefaults::longForKey(SAVE_ZOOM_TO_FIT_KEY, 1,
        false);
    retValue = grabImage(imageWidth, imageHeight);
    return retValue;
}

void LDVWidget::doPartList(
    LDVHtmlInventory *htmlInventory,
    LDPartsList *partsList,
    const char *filename)
{
    if (htmlInventory->generateHtml(filename, partsList, filename))
    {
        if (htmlInventory->isSnapshotNeeded())
        {
            int imageWidth = htmlInventory->getSnapshotWidthFlag();
            int imageHeight = htmlInventory->getSnapshotHeightFlag();
            char *snapshotPath = copyString(htmlInventory->getSnapshotPath());

            QString userDefinedSnapshot = htmlInventory->getUserDefinedSnapshot();

            if (!userDefinedSnapshot.isEmpty()) {
                QString snapshot = QDir::toNativeSeparators(snapshotPath);
                delete snapshotPath;
                emit lpubAlert->messageSig(LOG_INFO, QString("Using user defined Snapshot image %1.").arg(userDefinedSnapshot));

                if (htmlInventory->getOverwriteSnapshotFlag()){
                    QFile snapshotFile(snapshot);
                    if (snapshotFile.exists() && !snapshotFile.remove()){
                        emit lpubAlert->messageSig(LOG_ERROR, QString("Could not remove existing snapshot file '%1'").arg(snapshot));
                        return;
                    }
                }

                QImage image(userDefinedSnapshot);
                if (image.width() != imageWidth || image.height() != imageHeight)
                    image.scaled(imageWidth, imageHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                QImageWriter Writer(snapshot);
                if (!Writer.write(image)) {
                    emit lpubAlert->messageSig(LOG_ERROR, QString("Error writing to file '%1':\n%2").arg(snapshot).arg(Writer.errorString()));
                    return;
                }
            } else {
                emit lpubAlert->messageSig(LOG_INFO, QString("Generating Snapshot image..."));

                bool seams     = TCUserDefaults::boolForKey(SEAMS_KEY, false, false);

                TCUserDefaults::setBoolForKey(seams, SEAMS_KEY, false);
                TCUserDefaults::setBoolForKey(false, SAVE_STEPS_KEY, false);
                modelViewer->setStep(modelViewer->getNumSteps());
                htmlInventory->prepForSnapshot(modelViewer);
                modelViewer->setForceZoomToFit(true);
                TCUserDefaults::setLongForKey(false, SAVE_ACTUAL_SIZE_KEY, false);
                TCUserDefaults::setLongForKey(imageWidth, SAVE_WIDTH_KEY, false);
                TCUserDefaults::setLongForKey(imageHeight, SAVE_HEIGHT_KEY, false);
                if (!loadModel(imageInputFilename)){
                    emit lpubAlert->messageSig(LOG_ERROR, QString("Snapshot image input file was not loaded."));
                }
                saveImage(snapshotPath, imageWidth, imageHeight);
                delete snapshotPath;
            }
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
        if (!loadModel(modelFilename)){
            emit lpubAlert->messageSig(LOG_ERROR, QString("Model file %1 was not loaded.").arg(modelFilename));
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
                    emit lpubAlert->messageSig(LOG_STATUS, QString("No filename received from modelViewer."));
                }
                filename = QFileInfo(filename).baseName();
                filename = filename.left(int(filename.length()) - 6); // remove _parts
                filename += ".html";

                QString initialDir = QString(ldPrefs->getInvLastSavePath());
                if (initialDir.isEmpty()) {
                    initialDir = QString(ldPrefs->getDefaultSaveDir(LDPreferences::SOPartsList,
                                                 modelViewer->getFilename()).c_str());
                    QDir cwd(initialDir);            // <model>/LPub3D/tmp
                    cwd.cdUp();                      // <model>/LPub3D
                    cwd.cdUp();                      // <model>
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
                showDocument(htmlFilename);
            }
            htmlInventory->release();
            partsList->release();
        }
    }
}

void LDVWidget::showDocument(QString &htmlFilename){

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
      QString text = tr ("Your HTML part list was generated successfully.\n\n"
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

          if (Status != 0) {         // look for error
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

void LDVWidget::modelViewerAlertCallback(TCAlert *alert)
{
    if (alert)
    {
        emit lpubAlert->messageSig(LOG_STATUS, QString("%1")
                                   .arg(alert->getMessage()));
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

void LDVWidget::displayGLExtensions()
{
#ifdef QT_DEBUG_MODE
    QString openGLString, countString;
    int extensionCount;
    UCSTR temp = LDrawModelViewer::getOpenGLDriverInfo(extensionCount);
    ucstringtoqstring(openGLString, temp);
    QStringList list = openGLString.split(QRegExp("\n|\r\n|\r"),QString::SkipEmptyParts);
    countString = QString::number(extensionCount);
    countString += QString::fromWCharArray((TCLocalStrings::get(L"OpenGlnExtensionsSuffix")));

    emit lpubAlert->messageSig(LOG_INFO, QString("OpenGL supported extensions (%1)").arg(countString));
    foreach(openGLString, list){
        emit lpubAlert->messageSig(LOG_INFO, QString("%1").arg(openGLString));
    }
#endif
}
