/****************************************************************************
**
** Copyright (C) 2019 - 2020 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public License (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "renderdialog.h"
#include "ui_renderdialog.h"
#include "messageboxresizable.h"
#include "lpub_preferences.h"
#include "render.h"
#include "paths.h"
#include "lpub.h"
#include "metagui.h"

#define RENDER_DEFAULT_WIDTH 1280
#define RENDER_DEFAULT_HEIGHT 720
#define LP3D_CA 0.01
#define LP3D_CDF 1.0

#ifdef Q_OS_WIN
#include <TlHelp32.h>
#endif

RenderDialog::RenderDialog(QWidget* Parent, int renderType, int importOnly)
    : QDialog(Parent),
      mRenderType(renderType),
      mImportOnly(importOnly),
      ui(new Ui::RenderDialog)
{
    ui->setupUi(this);

#ifndef QT_NO_PROCESS
    mProcess = nullptr;
#endif

    mWidth      = RENDER_DEFAULT_WIDTH;
    mHeight     = RENDER_DEFAULT_HEIGHT;

    mViewerStepKey = gui->getViewerStepKey();

    ui->OutputEdit->setText(Render::getRenderImageFile(renderType));
    ui->OutputEdit->setValidator(new QRegExpValidator(QRegExp("^.*\\.png$",Qt::CaseInsensitive)));

    mCsiKeyList      = gui->getViewerConfigKey(mViewerStepKey).split(";").last().split("_");

    QAction *OutputEditAction = ui->OutputEdit->addAction(QIcon(":/resources/resetlineedit.png"), QLineEdit::TrailingPosition);
    OutputEditAction->setToolTip("Reset");
    connect(OutputEditAction, SIGNAL(triggered(bool)), this, SLOT(resetOutputEdit(bool)));

    if (mRenderType == POVRAY_RENDER) {

        setWindowTitle(tr ("POV-Ray Render"));

        ui->TimeLabel->setText("Preparing POV file...");

        ui->RenderSettingsButton->setToolTip("POV-Ray render settings");

        mOutputBuffer    = nullptr;

        mQuality         = Preferences::povrayRenderQuality;

        mTransBackground = true;

    } else if (mRenderType == BLENDER_RENDER) {

        QString title = mImportOnly ? "Import" : "Render";
        if (mImportOnly) {
            title = "Import";
            ui->RenderButton->setText(tr("Import"));

            ui->outputLabel->hide();
            ui->OutputEdit->hide();
            ui->RenderProgress->hide();
            ui->OutputBrowseButton->hide();
            ui->OutputLine->hide();
            ui->ProgressLine->hide();
        }
        setWindowTitle(tr ("Blender %1").arg(title));

        if (mImportOnly)
            ui->TimeLabel->setText("Open model in Blender");

        bool blenderInstalled = ! Preferences::blenderVersion.isEmpty();

        ui->RenderSettingsButton->setToolTip("Blender render settings");
        ui->RenderButton->setEnabled(blenderInstalled);
        if (!blenderInstalled)
            ui->RenderButton->setToolTip(tr("Blender not configured. Click 'Settings' to configure."));

        if (QFileInfo(Preferences::blenderRenderConfigFile).exists())
        {
            QSettings Settings(Preferences::blenderRenderConfigFile, QSettings::IniFormat);
            if (Settings.value(QString("%1/cropImage").arg(IMPORTLDRAW), QString()).toBool())
            {
                mWidth  = gui->GetImageWidth();
                mHeight = gui->GetImageHeight();
            }
        }
    }

    connect(&mUpdateTimer, SIGNAL(timeout()), this, SLOT(Update()));
    connect(&mUpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateElapsedTime()));

    mUpdateTimer.start(500);

    if (mRenderType == BLENDER_RENDER) {
        adjustSize();
        if (!mImportOnly)
            setMinimumWidth(int(ui->preview->geometry().width()));
        ui->preview->hide();
    } else {
        setMinimumSize(100, 100);
    }
    setSizeGripEnabled(true);
}

RenderDialog::~RenderDialog()
{
    delete ui;
}

void RenderDialog::on_RenderSettingsButton_clicked()
{
    if (mRenderType == POVRAY_RENDER) {

        POVRayRenderDialogGui *povrayRenderDialogGui =
                new POVRayRenderDialogGui();
        povrayRenderDialogGui->getRenderSettings(
                    mCsiKeyList,
                    mWidth,
                    mHeight,
                    mQuality,
                    mTransBackground);

    } else if (mRenderType == BLENDER_RENDER) {

        double scale = mCsiKeyList.at(K_MODELSCALE).toDouble();
        BlenderRenderDialogGui *blenderRenderDialogGui =
                new BlenderRenderDialogGui();
        blenderRenderDialogGui->getRenderSettings(
                    mWidth,
                    mHeight,
                    scale,
                    false /*document model*/);

        mCsiKeyList[K_MODELSCALE] = QString::number(scale);
        bool blenderInstalled = !Preferences::blenderVersion.isEmpty();
        ui->RenderButton->setEnabled(blenderInstalled);
        if (!blenderInstalled)
            ui->RenderButton->setToolTip(tr("Blender not configured. Click 'Settings' to configure."));
        else
            ui->RenderButton->setToolTip(QString());

    }
}

void RenderDialog::on_RenderButton_clicked()
{

#ifndef QT_NO_PROCESS
    if (mProcess)
    {
        PromptCancel();
        return;
    }

    mPreviewWidth  = ui->preview->width();
    mPreviewHeight = ui->preview->height();

    QImage Image(mPreviewWidth, mPreviewHeight, QImage::Format_RGB32);
    Image.fill(QColor(255, 255, 255));
    ui->preview->setPixmap(QPixmap::fromImage(Image));

    mRenderTime.start();

    QApplication::processEvents();

    mModelFile = Render::getRenderModelFile(mRenderType);

    QString message;

    if (mRenderType == POVRAY_RENDER) {

        ui->TimeLabel->setText("Generating POV-Ray scene file...");
        QApplication::processEvents();

        QStringList csiParts = gui->getViewerStepUnrotatedContents(mViewerStepKey);
        if (csiParts.isEmpty())
        {
            emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Did not receive CSI parts for %1.").arg(mViewerStepKey));
            return;
        }

        // Camera angle keys
        QString caKey = mCsiKeyList.at(K_LATITUDE)+" "+  // latitude
                mCsiKeyList.at(K_LONGITUDE);             // longitude

        // Target keys
        QString mKey =  mCsiKeyList.at(K_TARGETX)+" "+   // target[X]
                mCsiKeyList.at(K_TARGETY)+" "+           // target[Y]
                mCsiKeyList.at(K_TARGETZ)+" ";           // target[Z]

        // Rotstep keys
        QString rsKey = mCsiKeyList.at(K_ROTSX)+" "+     // rots[X]
                mCsiKeyList.at(K_ROTSY)+" "+             // rots[Y]
                mCsiKeyList.at(K_ROTSZ)+" "+             // rots[Z]
                mCsiKeyList.at(K_ROTSTYPE);              // type [REL|ABS]

        // Rotate parts for ldvExport - apply rotstep and camera angles
        if ((Render::rotateParts(csiParts, mModelFile, rsKey, caKey)) < 0) {
            return ;
        }

        // Camera distance keys
        QString cdKey = QString::number(mWidth)+" "+     // imageWidth
                QString::number(mHeight)+" "+            // imageHeight
                mCsiKeyList.at(K_MODELSCALE)+" "+        // modelScale
                mCsiKeyList.at(K_RESOLUTION)+" "+        // resolution
                mCsiKeyList.at(K_RESOLUTIONTYPE);        // resolutionType (DPI,DPCM)

        /* perspective projection settings */
        bool pp    = Preferences::perspectiveProjection;

        /* set camera angles */
        bool noCA  = Preferences::applyCALocally;

        /* determine camera distance - use LDView POV file generation values */
        int cd = int((double(Render::getPovrayRenderCameraDistance(cdKey))*0.455)*1700/1000);

        /* set LDV arguments */
        QString CA = QString("-ca%1") .arg(0.01);        // Effectively defaults to orthographic projection.
        QString cg = QString("-cg%1,%2,%3")
                .arg(noCA ? 0.0 : mCsiKeyList.at(K_LATITUDE).toDouble())
                .arg(noCA ? 0.0 : mCsiKeyList.at(K_LONGITUDE).toDouble())
                .arg(QString::number(pp ? cd * LP3D_CDF : cd,'f',0));

        QString m  = mKey == "0 0 0" ? QString() : mKey;
        QString w  = QString("-SaveWidth=%1")  .arg(mWidth);
        QString h  = QString("-SaveHeight=%1") .arg(mHeight);
        QString f  = QString("-ExportFile=%1") .arg(GetPOVFileName());
        QString l  = QString("-LDrawDir=%1") .arg(QDir::toNativeSeparators(Preferences::ldrawLibPath));

        QStringList Arguments;
        Arguments << CA;
        Arguments << cg;

        /*
                K_STEPNUMBER = 0,  // 0  not used
                K_IMAGEWIDTH,      // 1  not used
                K_RESOLUTION,      // 2
                K_RESOLUTIONTYPE,  // 3
                K_MODELSCALE,      // 4
                K_FOV,             // 5  not used
                K_LATITUDE,        // 6
                K_LONGITUDE,       // 7
                K_TARGETX,         // 8
                K_TARGETY,         // 9
                K_TARGETZ,         // 10
                K_ROTSX,           // 11
                K_ROTSY,           // 12
                K_ROTSZ,           // 13
                K_ROTSTYPE         // 14
            */

        // replace CA with FOV
        if (pp) {
            QString df = QString("-FOV=%1").arg(mCsiKeyList.at(K_FOV).toDouble());
            Arguments.replace(Arguments.indexOf(CA),df);
        }

        // Set alternate target position or use specified image size
        if (!m.isEmpty()){
            Arguments.removeAt(Arguments.indexOf(cg)); // remove camera globe
            QString dz = QString("-DefaultZoom=%1")
                    .arg(mCsiKeyList.at(K_MODELSCALE).toDouble());
            QString dl = QString("-DefaultLatLong=%1,%2")
                    .arg(noCA ? 0.0 : mCsiKeyList.at(K_LATITUDE).toDouble())
                    .arg(noCA ? 0.0 : mCsiKeyList.at(K_LONGITUDE).toDouble());
            Render::addArgument(Arguments, dz, "-DefaultZoom");
            Render::addArgument(Arguments, dl, "-DefaultLatLong");
        }

        Arguments << m;
        Arguments << w;
        Arguments << h;
        Arguments << f;
        Arguments << l;

        if (!Preferences::altLDConfigPath.isEmpty()) {
            Arguments << "-LDConfig=" + Preferences::altLDConfigPath;
        }

        Arguments << QDir::toNativeSeparators(mModelFile);

        message = QString("LDV CSI POV File Generation Arguments: %1 %2")
                .arg(Preferences::ldviewExe)
                .arg(Arguments.join(" "));
        emit gui->messageSig(LOG_INFO, message);

        // generate POV file
        if (!Render::doLDVCommand(Arguments,POVRAY_RENDER))
            return ;

        message = QString("LDV POV file %1 generated. %2").arg(GetPOVFileName()).arg(gui->elapsedTime(mRenderTime.elapsed()));
        emit gui->messageSig(LOG_INFO, message);

        /* set POV-Ray arguments */
        Arguments.clear();

        if (mTransBackground) {
            Arguments << QString("+UA");
        }

        Arguments << QString("+D");
        Arguments << QString("-O-");
        Arguments << Render::getPovrayRenderQuality(mQuality);
        Arguments << QString("+W%1").arg(mWidth);
        Arguments << QString("+H%1").arg(mHeight);
        Arguments << QString("+I\"%1\"").arg(Render::fixupDirname(GetPOVFileName()));
        Arguments << QString("+SM\"%1\"").arg(Render::fixupDirname(GetOutputFileName()));

        bool hasSTL       = Preferences::lgeoStlLib;
        bool hasLGEO      = Preferences::lgeoPath != "";
        bool hasPOVRayIni = Preferences::povrayIniPath != "";
        bool hasPOVRayInc = Preferences::povrayIncPath != "";

        if(hasPOVRayInc){
            QString povinc = QString("+L\"%1\"").arg(Render::fixupDirname(QDir::toNativeSeparators(Preferences::povrayIncPath)));
            Arguments << povinc;
        }
        if(hasPOVRayIni){
            QString povini = QString("+L\"%1\"").arg(Render::fixupDirname(QDir::toNativeSeparators(Preferences::povrayIniPath)));
            Arguments << povini;
        }
        if(hasLGEO){
            QString lgeoLg = QString("+L\"%1\"").arg(Render::fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/lg")));
            QString lgeoAr = QString("+L\"%1\"").arg(Render::fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/ar")));
            Arguments << lgeoLg;
            Arguments << lgeoAr;
            if (hasSTL){
                QString lgeoStl = QString("+L\"%1\"").arg(Render::fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/stl")));
                Arguments << lgeoStl;
            }
        }

        message = QString("POV-Ray CSI Render Arguments: %1 %2").arg(Preferences::povrayExe).arg(Arguments.join(" "));
        emit gui->messageSig(LOG_INFO, message);

        QStringList povenv = QProcess::systemEnvironment();
        povenv.prepend("POV_IGNORE_SYSCONF_MSG=1");

        mProcess = new RenderProcess(this);
        mProcess->setEnvironment(povenv);
        mProcess->setWorkingDirectory(QDir::currentPath() + QDir::separator() + Paths::tmpDir); // pov win console app will not write to dir different from cwd or source file dir
        mProcess->setStandardErrorFile(GetLogFileName(false/*stdOut*/));
        mProcess->start(Preferences::povrayExe, Arguments);
        if (mProcess->waitForStarted())
        {
            ui->RenderButton->setText(tr("Cancel"));
            ui->RenderProgress->setValue(ui->RenderProgress->minimum());
        }
        else
        {
            message = QString("Error starting POV-Ray.");
            emit gui->messageSig(LOG_ERROR, message);
            CloseProcess();
        }

    } else if (mRenderType == BLENDER_RENDER) {

        ui->TimeLabel->setText("Saving model...");
        QApplication::processEvents();

        mBlendProgValue = 0;
        mBlendProgMax   = 0;

        if (! QFileInfo(Preferences::blenderRenderConfigFile).exists())
            BlenderRenderDialogGui::saveSettings();

        QString defaultBlendFile = QString("%1/Blender/config/%2")
                .arg(Preferences::lpub3d3rdPartyConfigDir)
                .arg(VER_BLENDER_DEFAULT_BLEND_FILE);
        bool searchCustomDir = Preferences::enableFadeSteps || Preferences::enableHighlightStep;

        QString Program = QDir::toNativeSeparators(Preferences::blenderExe);

        QStringList Arguments;
        QString pythonExpression;
        pythonExpression.append(QString("\"import bpy; bpy.ops.render_scene.lpub3drenderldraw("
                                        "'EXEC_DEFAULT', "
                                        "resolution_width=%1, resolution_height=%2, "
                                        "render_percentage=%3, model_file=r'%4', "
                                        "image_file=r'%5', preferences_file=r'%6'")
                                .arg(mWidth).arg(mHeight)
                                .arg(mCsiKeyList.at(K_MODELSCALE).toDouble() * 100)
                                .arg(QDir::toNativeSeparators(mModelFile).replace("\\","\\\\"))
                                .arg(QDir::toNativeSeparators(ui->OutputEdit->text()).replace("\\","\\\\"))
                                .arg(QDir::toNativeSeparators(Preferences::blenderRenderConfigFile).replace("\\","\\\\")));
        if (searchCustomDir)
            pythonExpression.append(", search_additional_paths=True");
        if (mImportOnly) {
            pythonExpression.append(", import_only=True");

            Arguments << QString("--window-geometry");
            Arguments << QString("200 100 1440 900");
        } else {
            Arguments << QString("--background");
        }

        pythonExpression.append(", cli_render=True)\"");

        if (Preferences::defaultBlendFile)
            Arguments << QDir::toNativeSeparators(defaultBlendFile);
        Arguments << QString("--python-expr");
        Arguments << pythonExpression;

        QString scriptDir, scriptName, scriptCommand, shellProgram;

        QStringList configPathList = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
        scriptDir = configPathList.first();

         QFile script;
        if (QFileInfo(scriptDir).exists()) {

#ifdef Q_OS_WIN
            scriptName =  "render_ldraw_model.bat";
#else
            scriptName =  "render_ldraw_model.sh";
#endif
            scriptCommand =QString("%1 %2").arg(Preferences::blenderExe).arg(Arguments.join(" "));

            QString message = QString("Blender image render command: %1").arg(scriptCommand);
            emit gui->messageSig(LOG_DEBUG, message);

            script.setFileName(QString("%1/%2").arg(scriptDir).arg(scriptName));
            if(script.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream stream(&script);
#ifdef Q_OS_WIN
                stream << "@ECHO OFF &SETLOCAL" << endl;
#else
                stream << "#!/bin/bash" << endl;
#endif
                stream << scriptCommand << endl;
                script.close();
                emit gui->messageSig(LOG_DEBUG, QString("Script: %1").arg(script.fileName()));
            } else {
                emit gui->messageSig(LOG_ERROR, QString("Cannot write Blender render script file [%1] %2.")
                                     .arg(script.fileName())
                                     .arg(script.errorString()));
                return;
            }
        } else {
            emit gui->messageSig(LOG_ERROR, "Cannot create Blender render script temp path.");
            return;
        }

        QThread::sleep(2);

#ifdef Q_OS_WIN
        shellProgram = "cmd.exe";
#else
        shellProgram = "/bin/sh";
#endif

        mProcess = new RenderProcess(this);

        connect(mProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(ReadStdOut()));

        QStringList systemEnvironment = QProcess::systemEnvironment();
        systemEnvironment.prepend("LDRAW_DIRECTORY=" + Preferences::ldrawLibPath);

        mProcess->setEnvironment(systemEnvironment);

        mProcess->setWorkingDirectory(QDir::toNativeSeparators(QString("%1/%2").arg(QDir::currentPath()).arg(Paths::blenderRenderDir)));

        mProcess->setStandardErrorFile(GetLogFileName(false/*stdOut*/));

        if (mImportOnly) {
#ifdef Q_OS_WIN
            mProcess->startDetached(shellProgram, QStringList() << "/C" << script.fileName());
#else
            mProcess->startDetached(shellProgram, QStringList() << script.fileName());
#endif
            if (mProcess)
            {
                mProcess->kill();
                CloseProcess();
                if (mStdOutList.size())
                    WriteStdOut();
                close();
                return;
            }
        } else {
#ifdef Q_OS_WIN
            mProcess->start(shellProgram, QStringList() << "/C" << script.fileName());
#else
            mProcess->start(shellProgram, QStringList() << script.fileName());
#endif
        }

        if (mProcess->waitForStarted())
        {
            ui->RenderButton->setText(tr("Cancel"));
            ui->RenderProgress->setValue(ui->RenderProgress->minimum());
            emit gui->messageSig(LOG_INFO, QString("Blender render process [%1] running...").arg(mProcess->processId()));
        }
        else
        {
            message = QString("Error starting Blender render process");
            emit gui->messageSig(LOG_ERROR, message);
            CloseProcess();
        }
    }
#endif
}

#ifdef Q_OS_WIN
int RenderDialog::TerminateChildProcess(const qint64 pid, const qint64 ppid)
{
//    emit gui->messageSig(LOG_DEBUG, QString("PID %1, Parent PID: %2").arg(pid).arg(ppid));
    DWORD pID        = DWORD(pid);
    DWORD ppID       = DWORD(ppid);
    BOOL bResult     = FALSE;
    BOOL bChild      = FALSE;
    HANDLE hSnapshot = INVALID_HANDLE_VALUE, hProcess = INVALID_HANDLE_VALUE;
    PROCESSENTRY32 pe32;

    if ((hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, ppID)) == INVALID_HANDLE_VALUE) {
        emit gui->messageSig(LOG_ERROR, QString("CreateToolhelp32Snapshot failed: %1").arg(GetLastError()));
        return -1;
    }
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if ((bResult = Process32First(hSnapshot, &pe32)) == FALSE) {
        emit gui->messageSig(LOG_ERROR, QString("Process32First failed: %1").arg(GetLastError()));
        CloseHandle(hSnapshot);
        return -2;
    }
    do {
        if (QString::fromWCharArray(pe32.szExeFile).contains(QRegExp("^(?:cmd\\.exe|conhost\\.exe|blender\\.exe)$", Qt::CaseInsensitive))) {
            if ((pe32.th32ProcessID == pID && pe32.th32ParentProcessID == ppID) || // parent:   cmd.exe
                (bChild = pe32.th32ParentProcessID == pID)) {                      // children: conhost.exe, blender.exe
                if ((hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID)) == INVALID_HANDLE_VALUE) {
                    emit gui->messageSig(LOG_ERROR, QString("OpenProcess failed: %1").arg(GetLastError()));
                    return -3;
                } else {
                    TerminateProcess(hProcess, 9);
                    CloseHandle(hProcess);
                    emit gui->messageSig(LOG_INFO, QString("%1 Process Terminated: PID: %2  Parent PID: %3  Name: %4")
                                         .arg(bChild ? "Child" : "Parent")
                                         .arg(pe32.th32ProcessID)
                                         .arg(pe32.th32ParentProcessID)
                                         .arg(QString::fromWCharArray(pe32.szExeFile)));
                }
            }
        }
    } while ((bResult = Process32Next(hSnapshot, &pe32)));
    CloseHandle(hSnapshot);

    return 0;
}
#endif

void RenderDialog::ReadStdOut()
{
    QString StdOut = QString(mProcess->readAllStandardOutput());
    mStdOutList.append(StdOut);
    QRegExp rxBlenderProgress("\\/(\\d+) Tiles, Denoised (\\d+) tiles",Qt::CaseInsensitive);
    if (StdOut.contains(rxBlenderProgress))
    {
        mBlendProgValue = rxBlenderProgress.cap(2).toInt();
        mBlendProgMax   = rxBlenderProgress.cap(1).toInt();
        ui->RenderProgress->setMaximum(mBlendProgMax);
        ui->RenderProgress->setValue(mBlendProgValue);
        emit gui->messageSig(LOG_INFO, QString("Rendered Tile %1/%2")
                                               .arg(mBlendProgValue)
                                               .arg(mBlendProgMax));
    }
}

QString RenderDialog::ReadStdErr(bool &hasError) const
{
    hasError = mRenderType == BLENDER_RENDER ? false : true;
    QFile file;
    QStringList returnLines;

    file.setFileName(GetLogFileName(false/*stdOut*/));

    if ( ! file.open(QFile::ReadOnly | QFile::Text))
    {
        QString message = QString("Failed to open log file: %1:\n%2")
                .arg(file.fileName())
                .arg(file.errorString());
        return message;
    }

    QTextStream in(&file);
    while ( ! in.atEnd())
    {
        QString line = in.readLine(0);
        returnLines << line.trimmed().replace("<","&lt;")
                                     .replace(">","&gt;")
                                     .replace("&","&amp;") + "<br>";
        if (mRenderType == POVRAY_RENDER) {
            if (line.contains(QRegExp("^POV-Ray finished$", Qt::CaseSensitive)))
                hasError = false;
        } else if (mRenderType == BLENDER_RENDER) {
            if (!hasError && !line.isEmpty())
                hasError = true;
        }
    }

    return returnLines.join(" ");
}

void RenderDialog::WriteStdOut()
{
    QFile file(GetLogFileName(true/*stdOut*/));

    if (file.open(QFile::WriteOnly | QIODevice::Truncate | QFile::Text))
    {
        QTextStream Out(&file);
        for (const QString& Line : mStdOutList)
            Out << Line;
        file.close();
    }
    else
    {
       emit gui->messageSig(LOG_INFO, QString("Error writing to %1 file '%2':\n%3")
                                     .arg("stdout").arg(file.fileName(), file.errorString()));
    }
}

void RenderDialog::Update()
{
#ifndef QT_NO_PROCESS
    if (!mProcess)
        return;

    if (mProcess->state() == QProcess::NotRunning)
    {
        if (mRenderType == BLENDER_RENDER)
            emit gui->messageSig(LOG_INFO, QString("Blender process finished"));
        ShowResult();
        CloseProcess();
    }
#endif

    if (mRenderType == POVRAY_RENDER) {

        if (!mOutputBuffer)
        {
            mOutputFile.setFileName(GetOutputFileName());

            if (!mOutputFile.open(QFile::ReadWrite))
                return;

            mOutputBuffer = mOutputFile.map(0, mOutputFile.size());

            if (!mOutputBuffer)
            {
                mOutputFile.close();
                return;
            }
        }

        struct lcSharedMemoryHeader
        {
            quint32 Version;
            quint32 Width;
            quint32 Height;
            quint32 PixelsWritten;
            quint32 PixelsRead;
        };

        lcSharedMemoryHeader* Header = (lcSharedMemoryHeader*)mOutputBuffer;

        if (Header->PixelsWritten == Header->PixelsRead)
            return;

        int Width = int(Header->Width);
        int Height = int(Header->Height);
        int PixelsWritten = int(Header->PixelsWritten);

        if (!Header->PixelsRead)
            mImage = QImage(Width, Height, QImage::Format_ARGB32);

        quint8* Pixels = (quint8*)(Header + 1);

        for (int y = 0; y < Height; y++)
        {
            for (int x = 0; x < Width; x++)
            {
                mImage.setPixel(x, y, qRgba(Pixels[0], Pixels[1], Pixels[2], Pixels[3]));
                Pixels += 4;
            }
        }

        Header->PixelsRead = quint32(PixelsWritten);

        ui->preview->setPixmap(QPixmap::fromImage(mImage.scaled(mPreviewWidth, mPreviewHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

        ui->RenderProgress->setMaximum(mImage.width() * mImage.height());
        ui->RenderProgress->setValue(int(Header->PixelsRead));

        if (PixelsWritten == Width * Height)
            ui->RenderProgress->setValue(ui->RenderProgress->maximum());

    }
}

void RenderDialog::ShowResult()
{
#ifndef QT_NO_PROCESS
    bool hasError;

    const QString StdErrLog = ReadStdErr(hasError);

    if (mProcess->exitStatus() != QProcess::NormalExit || mProcess->exitCode() != 0 || hasError)
    {
        emit gui->messageSig(LOG_NOTICE, QString("Render failed. See %2 for details.")
                             .arg(GetLogFileName(false/*stdOut*/)));
        ui->TimeLabel->setText(QString("Image generation failed."));
        ui->RenderProgress->setRange(0,1);
        ui->RenderProgress->setValue(0);
        QMessageBoxResizable box;
        box.setTextFormat (Qt::RichText);
        box.setIcon (QMessageBox::Critical);
        box.setStandardButtons (QMessageBox::Ok);
        box.setDefaultButton   (QMessageBox::Ok);
        box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        box.setWindowTitle(QString(mRenderType == BLENDER_RENDER ? QString("Blender Render") : "POV-Ray Render"));

        QString header = "<b>" + tr ("Render Error.") +
            "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" +
            "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" +
            "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" +
            "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" +
            "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" +
            "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</b>";
        QString body = tr ("An error occurred while rendering. See Show Details...");
        box.setText (header);
        box.setInformativeText (body);
        box.setDetailedText(StdErrLog);
        box.exec();
        return;

    } else {
        ui->RenderProgress->setValue(ui->RenderProgress->maximum());
    }

    if (mRenderType == BLENDER_RENDER)
        ReadStdOut();
#endif

    QString message;

    bool Success = false;

    QString FileName = ui->OutputEdit->text();

    if (mRenderType == POVRAY_RENDER) {

        ui->preview->setPixmap(QPixmap::fromImage(mImage.scaled(mPreviewWidth, mPreviewHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

        emit gui->messageSig(LOG_INFO,QString("Writing POV-Ray rendered image '%1'...").arg(ui->OutputEdit->text()));

        if (!FileName.isEmpty())
        {
            QImageWriter Writer(FileName);

            Success = Writer.write(mImage);

            if (!Success)
                emit gui->messageSig(LOG_ERROR,QString("Error writing to image file '%1':\n%2").arg(FileName, Writer.errorString()));
            else if (Preferences::povrayAutoCrop)
                Render::clipImage(FileName);
        }

    } else if (mRenderType == BLENDER_RENDER) {

        Success = QFileInfo(FileName).exists();
        if (Success){

            mPreviewWidth  = mWidth;
            mPreviewHeight = mHeight;

            QImageReader reader(FileName);
            mImage = reader.read();
            mImage = mImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);;

            ui->preview->setPixmap(QPixmap::fromImage(mImage.scaled(mPreviewWidth, mPreviewHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
            ui->preview->show();
        }
    }

    QString imageType = mRenderType == BLENDER_RENDER ? "Blender" : "POV-Ray";

    if (!Success) {
        ui->TimeLabel->setStyleSheet("QLabel { color : red; }");
        ui->TimeLabel->setText(QString("Image render failed."));
    }

    WriteStdOut();

    message = QString("%1 CSI %2. %3")
                      .arg(imageType)
                      .arg(Success ? mImportOnly ? "completed" :
                                                   QString("generated %1").arg(FileName) :
                                                   "failed (unknown reason)")
                      .arg(gui->elapsedTime(mRenderTime.elapsed()));
    emit gui->messageSig(Success ? LOG_INFO : LOG_ERROR, message);
}

QString RenderDialog::GetOutputFileName() const
{
    return QDir(QDir::tempPath()).absoluteFilePath("lpub3d-render-map.out");
}

QString RenderDialog::GetPOVFileName() const
{
    return QDir::toNativeSeparators(mModelFile + ".pov");
}

QString RenderDialog::GetLogFileName(bool stdOut) const
{
    QString logFile = mRenderType == POVRAY_RENDER ? stdOut ? "stdout-povray-render" : "stderr-povray-render" :
                                                     stdOut ? "stdout-blender-render"  : "stderr-blender-render";
    return QDir::toNativeSeparators(QDir::currentPath() + QDir::separator() + logFile);
}

void RenderDialog::UpdateElapsedTime()
{
    if (mProcess && !mImportOnly)
    {
        ui->TimeLabel->setText(QString("Tiles: %1/%2, %3")
                               .arg(mBlendProgValue)
                               .arg(mBlendProgMax)
                               .arg(gui->elapsedTime(mRenderTime.elapsed())));
    }
}

void RenderDialog::CloseProcess()
{
#ifndef QT_NO_PROCESS
    delete mProcess;
    mProcess = nullptr;
#endif

    if (mRenderType == POVRAY_RENDER) {

        mOutputFile.unmap((uchar*)mOutputBuffer);
        mOutputBuffer = nullptr;
        mOutputFile.close();

        QFile::remove(GetOutputFileName());

        QFile::remove(GetPOVFileName());

    } else if (mRenderType == BLENDER_RENDER) {
        emit gui->messageSig(LOG_INFO, QString("Blender process closed"));
    }

    ui->RenderButton->setText(tr(mImportOnly ? "Import" : "Render"));
}

bool RenderDialog::PromptCancel()
{
#ifndef QT_NO_PROCESS
    if (mProcess)
    {
        if (QMessageBox::question(this,
                                  QString("Cancel Render"),
                                  QString("Are you sure you want to cancel the current Render?"),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
#ifdef Q_OS_WIN
            TerminateChildProcess(mProcess->processId(),
                                  QCoreApplication::applicationPid());
#endif
            mProcess->kill();
            CloseProcess();
            ui->TimeLabel->setText(QString("Tiles: %1/%2, Render Cancelled.")
                                           .arg(mBlendProgValue)
                                           .arg(mBlendProgMax));
        }
        else
            return false;
    }
#endif

    return true;
}

void RenderDialog::reject()
{
    if (PromptCancel())
        QDialog::reject();
}

void RenderDialog::on_OutputBrowseButton_clicked()
{
    QString Result = QFileDialog::getSaveFileName(this, tr("Select Output File"), ui->OutputEdit->text(), tr("Supported Image Files (*.bmp *.png *.jpg);;BMP Files (*.bmp);;PNG Files (*.png);;JPEG Files (*.jpg);;All Files (*.*)"));

    if (!Result.isEmpty())
        ui->OutputEdit->setText(QDir::toNativeSeparators(Result));
}

void RenderDialog::resetOutputEdit(bool)
{
    ui->OutputEdit->setText(Render::getRenderImageFile(mRenderType));
    ui->RenderProgress->setRange(0,1);
    ui->RenderProgress->setValue(0);
    ui->TimeLabel->setText(QString());
    if (mRenderType == BLENDER_RENDER ) {
        ui->preview->hide();
        adjustSize();
        setMinimumWidth(int(ui->preview->geometry().width()/*mWidth*/));
    }
}

void RenderDialog::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    mPreviewWidth  = ui->preview->geometry().width();
    mPreviewHeight = ui->preview->geometry().height();
}

RenderProcess::~RenderProcess(){
    if(state() == QProcess::Running ||
       state() == QProcess::Starting)
    {
        terminate();
        waitForFinished();
    }
}
