/****************************************************************************
**
** Copyright (C) 2019 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "povrayrenderdialog.h"
#include "ui_povrayrenderdialog.h"
#include "messageboxresizable.h"
#include "lpub_preferences.h"
#include "render.h"
#include "paths.h"
#include "lpub.h"

#define POVRAY_RENDER_DEFAULT_WIDTH 1280
#define POVRAY_RENDER_DEFAULT_HEIGHT 720

#define POVRAY_RENDER_PREVIEW_WIDTH 768
#define POVRAY_RENDER_PREVIEW_HEIGHT 432

enum CsiKeyList{
    K_STEPNUMBER = 0,  // 0  not used
    K_IMAGEWIDTH,      // 1  not used
    K_RESOLUTION,      // 2
    K_RESOLUTIONTYPE,  // 3
    K_SCALEFACTOR,     // 4
    K_FOV,             // 5  not used
    K_LATITUDE,        // 6
    K_LONGITUDE,       // 7
    K_ROTSX,           // 8
    K_ROTSY,           // 9
    K_ROTSZ,           // 10
    K_ROTSTYPE,        // 11
    K_MODELSCALE       // 12
};

PovrayRenderDialog::PovrayRenderDialog(QWidget* Parent)
    : QDialog(Parent),
    ui(new Ui::PovrayRenderDialog)
{
#ifndef QT_NO_PROCESS
    mProcess = nullptr;
#endif
    mOutputBuffer = nullptr;

    mViewerCsiKey = gui->getViewerCsiKey();
    mCsiKeyList = gui->getViewerStepCsiKey(mViewerCsiKey).split("_");
    
    ui->setupUi(this);

    ui->OutputAlphaBox->setChecked(true);
    ui->WidthEdit->setText(QString::number(POVRAY_RENDER_DEFAULT_WIDTH));
    ui->WidthEdit->setValidator(new QIntValidator(16, INT_MAX));
    ui->HeightEdit->setText(QString::number(POVRAY_RENDER_DEFAULT_HEIGHT));
    ui->HeightEdit->setValidator(new QIntValidator(16, INT_MAX));
    ui->OutputEdit->setText(Render::getPovrayRenderFileName(mViewerCsiKey));
    ui->OutputEdit->setValidator(new QRegExpValidator(QRegExp("^.*\\.png$",Qt::CaseInsensitive)));
    ui->ScaleEdit->setText(mCsiKeyList.at(K_MODELSCALE));
    ui->ScaleEdit->setValidator(new QDoubleValidator(0.1,1000.0,1));
    ui->ResolutionEdit->setText(mCsiKeyList.at(K_RESOLUTION));
    ui->ResolutionEdit->setValidator(new QIntValidator(50, INT_MAX));
    ui->QualityComboBox->setCurrentIndex(Preferences::povrayRenderQuality);

    QImage Image(POVRAY_RENDER_PREVIEW_WIDTH, POVRAY_RENDER_PREVIEW_HEIGHT, QImage::Format_RGB32);
    Image.fill(QColor(255, 255, 255));
    ui->preview->setPixmap(QPixmap::fromImage(Image));

    connect(&mUpdateTimer, SIGNAL(timeout()), this, SLOT(Update()));
    connect(&mUpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateElapsedTime()));

    mUpdateTimer.start(500);

    setMinimumSize(100, 100);
    setSizeGripEnabled(true);
}

PovrayRenderDialog::~PovrayRenderDialog()
{
    delete ui;
}

QString PovrayRenderDialog::GetOutputFileName() const
{
    return QDir(QDir::tempPath()).absoluteFilePath("lpub3d-render-map.out");
}

QString PovrayRenderDialog::GetPOVFileName() const
{
    return QDir::toNativeSeparators(mModelFile + ".pov");
}

QString PovrayRenderDialog::GetLogFileName() const
{
    return QDir::toNativeSeparators(QDir::currentPath() + QDir::separator() + "stderr-povrayrender");
}

void PovrayRenderDialog::CloseProcess()
{
#ifndef QT_NO_PROCESS
    delete mProcess;
    mProcess = nullptr;
#endif

    mOutputFile.unmap((uchar*)mOutputBuffer);
    mOutputBuffer = nullptr;
    mOutputFile.close();

    QFile::remove(GetOutputFileName());

    QFile::remove(GetPOVFileName());

    ui->RenderButton->setText(tr("Render"));
}

bool PovrayRenderDialog::PromptCancel()
{
#ifndef QT_NO_PROCESS
    if (mProcess)
    {
        if (QMessageBox::question(this, tr("Cancel Render"), tr("Are you sure you want to cancel the current render?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            if (mProcess)
            {
                mProcess->kill();
                CloseProcess();
            }
        }
        else
            return false;
    }
#endif
    
    return true;
}

void PovrayRenderDialog::reject()
{
    if (PromptCancel())
        QDialog::reject();
}

void PovrayRenderDialog::on_LdvExportSettingsButton_clicked()
{
    Render::showLdvExportSettings(POVRayRender);
}

void PovrayRenderDialog::on_LdvLDrawPreferencesButton_clicked()
{
    Render::showLdvLDrawPreferences(POVRayRender);
}

void PovrayRenderDialog::on_RenderButton_clicked()
{
#ifndef QT_NO_PROCESS
    if (mProcess)
    {
        PromptCancel();
        return;
    }

    mRenderTime.start();

    ui->TimeLabel->setText("Preparing POV file...");

    QApplication::processEvents();
    
    mModelFile = QDir::toNativeSeparators(QDir::currentPath() + QDir::separator() + Paths::tmpDir + QDir::separator() + "csipovray.ldr");

    QStringList csiParts = gui->getViewerStepUnrotatedContents(mViewerCsiKey);
    if (csiParts.isEmpty())
    {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Did not receive CSI parts for %1.").arg(mViewerCsiKey));
        return;
    }

    // Camera angle keys
    QString caKey = mCsiKeyList.at(K_LATITUDE)+" "+  // latitude
                    mCsiKeyList.at(K_LONGITUDE);     // longitude
    // Rotstep keys
    QString rsKey = mCsiKeyList.at(K_ROTSX)+" "+     // rots[X]
                    mCsiKeyList.at(K_ROTSY)+" "+     // rots[Y]
                    mCsiKeyList.at(K_ROTSZ)+" "+     // rots[Z]
                    mCsiKeyList.at(K_ROTSTYPE);      // type [REL|ABS]

    // Rotate parts for ldvExport - apply rotstep and camera angles
    if ((Render::rotateParts(csiParts, mModelFile, rsKey, caKey)) < 0) {
        return ;
    }

    // Camera distance keys
    QString cdKey = ui->WidthEdit->text()+" "+         // imageWidth
                    ui->HeightEdit->text()+" "+        // imageHeight
                    ui->ScaleEdit->text()+" "+         // modelScale
                    mCsiKeyList.at(K_RESOLUTION)+" "+  // resolution
                    mCsiKeyList.at(K_RESOLUTIONTYPE);  // resolutionType (DPI,DPCM)

    /* determine camera distance - use LDView POV file generation values */
    int cd = int((double(Render::getPovrayRenderCameraDistance(cdKey))*0.455)*1700/1000);

    /* set camera angles */
    bool noCA  = Preferences::applyCALocally;

    /* set LDV arguments */
    QString CA = QString("-ca%1") .arg(0.01);  // Effectively defaults to orthographic projection.
    QString cg = QString("-cg%1,%2,%3")
            .arg(noCA ? 0.0 : mCsiKeyList.at(K_LATITUDE).toDouble())
            .arg(noCA ? 0.0 : mCsiKeyList.at(K_LONGITUDE).toDouble())
            .arg(cd);

    QString w  = QString("-SaveWidth=%1") .arg(ui->WidthEdit->text());
    QString h  = QString("-SaveHeight=%1") .arg(ui->HeightEdit->text());
    QString f  = QString("-ExportFile=%1") .arg(GetPOVFileName());
    QString l  = QString("-LDrawDir=%1") .arg(QDir::toNativeSeparators(Preferences::ldrawLibPath));

    QStringList Arguments;
    Arguments << CA;
    Arguments << cg;
    Arguments << w;
    Arguments << h;
    Arguments << f;
    Arguments << l;

    if (!Preferences::altLDConfigPath.isEmpty()) {
        Arguments << "-LDConfig=" + Preferences::altLDConfigPath;
    }

    Arguments << QDir::toNativeSeparators(mModelFile);

    QString message = QString("LDV CSI POV File Generation Arguments: %1 %2")
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

    if (ui->OutputAlphaBox->isChecked()) {
        Arguments << QString("+UA");
    }

    Arguments << QString("+D");
    Arguments << QString("-O-");
    Arguments << Render::getPovrayRenderQuality(ui->QualityComboBox->currentIndex());
    Arguments << QString("+W%1").arg(ui->WidthEdit->text());
    Arguments << QString("+H%1").arg(ui->HeightEdit->text());
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

    mProcess = new QProcess(this);
    mProcess->setEnvironment(povenv);
    mProcess->setWorkingDirectory(QDir::currentPath() + "/" + Paths::tmpDir); // pov win console app will not write to dir different from cwd or source file dir
    mProcess->setStandardErrorFile(GetLogFileName());
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
#endif
}

QString PovrayRenderDialog::ReadStdErrLog(bool &hasError) const
{
    hasError = true;
    QFile file(GetLogFileName());
    if ( ! file.open(QFile::ReadOnly | QFile::Text))
    {
        QString message = QString("Failed to open log file: %1:\n%2")
                  .arg(file.fileName())
                  .arg(file.errorString());
        return message;
    }
    QTextStream in(&file);
    QStringList lines;
    while ( ! in.atEnd())
    {
        QString line = in.readLine(0);
        lines << line.trimmed() + "<br>";
        if (line.contains(QRegExp("^POV-Ray finished$",Qt::CaseSensitive)))
            hasError = false;
    }
    return lines.join(" ");
}

void PovrayRenderDialog::Update()
{
#ifndef QT_NO_PROCESS
    if (!mProcess)
        return;

    if (mProcess->state() == QProcess::NotRunning)
    {
        ShowResult();
        CloseProcess();
    }
#endif

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

    ui->preview->setPixmap(QPixmap::fromImage(mImage.scaled(POVRAY_RENDER_PREVIEW_WIDTH, POVRAY_RENDER_PREVIEW_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

    ui->RenderProgress->setMaximum(mImage.width() * mImage.height());
    ui->RenderProgress->setValue(int(Header->PixelsRead));

    if (PixelsWritten == Width * Height)
        ui->RenderProgress->setValue(ui->RenderProgress->maximum());
}

void PovrayRenderDialog::UpdateElapsedTime()
{
    if (mProcess)
    {
        ui->TimeLabel->setText(gui->elapsedTime(mRenderTime.elapsed()));
    }
}

void PovrayRenderDialog::ShowResult()
{
#ifndef QT_NO_PROCESS
    bool hasError;
    const QString StdErrLog = ReadStdErrLog(hasError);
    if (mProcess->exitStatus() != QProcess::NormalExit || mProcess->exitCode() != 0 || hasError)
    {
        ui->RenderProgress->setRange(0,1);
        ui->RenderProgress->setValue(0);
        QMessageBoxResizable box;
        box.setTextFormat (Qt::RichText);
        box.setIcon (QMessageBox::Critical);
        box.setStandardButtons (QMessageBox::Ok);
        box.setDefaultButton   (QMessageBox::Ok);
        box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        box.setWindowTitle(tr ("POV-Ray Render"));

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
    }
#endif

    ui->preview->setPixmap(QPixmap::fromImage(mImage.scaled(POVRAY_RENDER_PREVIEW_WIDTH, POVRAY_RENDER_PREVIEW_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

    emit gui->messageSig(LOG_INFO,QString("Writing POV-Ray rendered image '%1'...").arg(ui->OutputEdit->text()));
    
    bool Result = false;

    QString FileName = ui->OutputEdit->text();

    if (!FileName.isEmpty())
    {
        QImageWriter Writer(FileName);

        Result = Writer.write(mImage);

        if (!Result)
            emit gui->messageSig(LOG_ERROR,QString("Error writing to image file '%1':\n%2").arg(FileName, Writer.errorString()));
        else if (Preferences::povrayAutoCrop)
            Render::clipImage(FileName);
    }

    QString message = QString("POV-Ray CSI Render %1. %2")
                              .arg(Result ? QString("generated %1").arg(FileName) : "image file failed")
                              .arg(gui->elapsedTime(mRenderTime.elapsed()));
    emit gui->messageSig(Result ? LOG_INFO : LOG_ERROR, message);
}

void PovrayRenderDialog::on_OutputBrowseButton_clicked()
{
    QString Result = QFileDialog::getSaveFileName(this, tr("Select Output File"), ui->OutputEdit->text(), tr("Supported Image Files (*.bmp *.png *.jpg);;BMP Files (*.bmp);;PNG Files (*.png);;JPEG Files (*.jpg);;All Files (*.*)"));

    if (!Result.isEmpty())
        ui->OutputEdit->setText(QDir::toNativeSeparators(Result));
}
