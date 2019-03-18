#include "lc_global.h"
#include "lc_renderdialog.h"
#include "ui_lc_renderdialog.h"
#include "project.h"
#include "lc_application.h"
#include "lc_profile.h"

/*** LPub3D Mod - includes ***/
#include "messageboxresizable.h"
#include "lpub_preferences.h"
#include "render.h"
#include "paths.h"
#include "lpub.h"
/*** LPub3D Mod end ***/

#define LC_POVRAY_PREVIEW_WIDTH 768
#define LC_POVRAY_PREVIEW_HEIGHT 432
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
#define LC_POVRAY_MEMORY_MAPPED_FILE 1
#endif

/*** LPub3D Mod - image defaults ***/
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
/*** LPub3D Mod end ***/

lcRenderDialog::lcRenderDialog(QWidget* Parent)
	: QDialog(Parent),
    ui(new Ui::lcRenderDialog)
{
#ifndef QT_NO_PROCESS
	mProcess = nullptr;
#endif
	mOutputBuffer = nullptr;

	ui->setupUi(this);

	ui->WidthEdit->setText(QString::number(lcGetProfileInt(LC_PROFILE_POVRAY_WIDTH)));
	ui->WidthEdit->setValidator(new QIntValidator(16, INT_MAX));
	ui->HeightEdit->setText(QString::number(lcGetProfileInt(LC_PROFILE_POVRAY_HEIGHT)));
	ui->HeightEdit->setValidator(new QIntValidator(16, INT_MAX));

/*** LPub3D Mod - get image file name ***/
    mViewerCsiKey = gui->getViewerCsiKey();
    mCsiKeyList = gui->getViewerStepCsiKey(mViewerCsiKey).split("_");
    ui->OutputEdit->setText(Render::getPovrayRenderFileName(mViewerCsiKey));
/*** LPub3D Mod end ***/
/*** LPub3D Mod - POV-Ray render scale and resolution ***/
    ui->ScaleEdit->setText(mCsiKeyList.at(K_MODELSCALE));
    ui->ScaleEdit->setValidator(new QDoubleValidator(0.1,1000.0,1));
    ui->ResolutionEdit->setText(mCsiKeyList.at(K_RESOLUTION));
    ui->ResolutionEdit->setValidator(new QIntValidator(50, INT_MAX));
/*** LPub3D Mod end ***/

	QImage Image(LC_POVRAY_PREVIEW_WIDTH, LC_POVRAY_PREVIEW_HEIGHT, QImage::Format_RGB32);
	Image.fill(QColor(255, 255, 255));
	ui->preview->setPixmap(QPixmap::fromImage(Image));

	connect(&mUpdateTimer, SIGNAL(timeout()), this, SLOT(Update()));
	mUpdateTimer.start(500);
}

lcRenderDialog::~lcRenderDialog()
{
	delete ui;
}

QString lcRenderDialog::GetOutputFileName() const
{
/*** LPub3D Mod - change out file name ***/
    return QDir(QDir::tempPath()).absoluteFilePath("lpub3d-render-map.out");
/*** LPub3D Mod end ***/
}

QString lcRenderDialog::GetPOVFileName() const
{
/*** LPub3D Mod - generated pov file name ***/
    return QDir::toNativeSeparators(mModelFile + ".pov");
/*** LPub3D Mod end ***/
}

void lcRenderDialog::CloseProcess()
{
#ifndef QT_NO_PROCESS
	delete mProcess;
	mProcess = nullptr;
#endif

#if LC_POVRAY_MEMORY_MAPPED_FILE
	mOutputFile.unmap((uchar*)mOutputBuffer);
	mOutputBuffer = nullptr;
	mOutputFile.close();

	QFile::remove(GetOutputFileName());
#endif

	QFile::remove(GetPOVFileName());

	ui->RenderButton->setText(tr("Render"));
}

bool lcRenderDialog::PromptCancel()
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

void lcRenderDialog::reject()
{
	if (PromptCancel())
		QDialog::reject();
}

/*** LPub3D Mod - LDV settings ***/
void lcRenderDialog::on_LdvExportSettingsButton_clicked()
{
    Render::showLdvExportSettings(NativePOVIni);
}

void lcRenderDialog::on_LdvLDrawPreferencesButton_clicked()
{
    Render::showLdvLDrawPreferences(NativePOVIni);
}
/*** LPub3D Mod end ***/

void lcRenderDialog::on_RenderButton_clicked()
{
#ifndef QT_NO_PROCESS
	if (mProcess)
	{
		PromptCancel();
		return;
	}

/*** LPub3D Mod - replace ExportPOVRay with LDV POV Generation ***/
    mRenderTime.start();
    
    mModelFile = QDir::toNativeSeparators(QDir::currentPath() + "/" + Paths::tmpDir + "/csipovray.ldr");

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
    if (!Render::doLDVCommand(Arguments,EXPORT_POVRAY))
        return ;

    message = QString("LDV POV file %1 generated. %2").arg(GetPOVFileName()).arg(gui->elapsedTime(mRenderTime.elapsed()));
    emit gui->messageSig(LOG_INFO, message);

    //if (!lcGetActiveProject()->ExportPOVRay(FileName))
    //	return;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - use LPub3D POV-Ray arguments ***/
    Arguments.clear();
    Arguments << QString("+D");
    Arguments << QString("-O-");
    Arguments << QString("+UA");
    Arguments << QString("+W%1").arg(ui->WidthEdit->text());
    Arguments << QString("+H%1").arg(ui->HeightEdit->text());
    Arguments << QString("+I\"%1\"").arg(Render::fixupDirname(GetPOVFileName()));

#if LC_POVRAY_MEMORY_MAPPED_FILE
    Arguments << QString::fromLatin1("+SM\"%1\"").arg(GetOutputFileName());
#endif

    int Quality = ui->QualityComboBox->currentIndex();

    switch (Quality)
    {
    case 0:
        Arguments << QString("+Q11");
        Arguments << QString("+R3");
        Arguments << QString("+A0.1");
        Arguments << QString("+J0.5");
        break;

    case 1:
        Arguments << QString("+Q5");
        Arguments << QString("+A0.1");
        break;

    case 2:
        break;
    }

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
/*** LPub3D Mod end ***/

/*** LPub3D Mod - set LPub3D-Trace path ***/

    message = QString("POV-Ray CSI Render Arguments: %1 %2").arg(Preferences::povrayExe).arg(Arguments.join(" "));
    emit gui->messageSig(LOG_INFO, message);

/***  Disable leoCAD POV-Ray paths
#ifdef Q_OS_WIN
	POVRayPath = QDir::cleanPath(QCoreApplication::applicationDirPath() + QLatin1String("/povconsole32-sse2.exe"));
#endif

#ifdef Q_OS_LINUX
	POVRayPath = lcGetProfileString(LC_PROFILE_POVRAY_PATH);
	Arguments.append("+FN");
	Arguments.append("-D");
#endif

#ifdef Q_OS_MACOS
	POVRayPath = QDir::cleanPath(QCoreApplication::applicationDirPath() + QLatin1String("/povray"));
#endif
***/
/*** LPub3D Mod end ***/

	mProcess = new QProcess(this);
    mProcess->setWorkingDirectory(QDir::currentPath()+ "/" + Paths::tmpDir); // pov win console app will not write to dir different from cwd or source file dir
	connect(mProcess, SIGNAL(readyReadStandardError()), this, SLOT(ReadStdErr()));
    mProcess->start(Preferences::povrayExe, Arguments);

	if (mProcess->waitForStarted())
	{
		ui->RenderButton->setText(tr("Cancel"));
		ui->RenderProgress->setValue(ui->RenderProgress->minimum());
		mStdErrList.clear();
	}
	else
	{
		QMessageBox::warning(this, tr("Error"), tr("Error starting POV-Ray."));
		CloseProcess();
	}
#endif
}

void lcRenderDialog::ReadStdErr()
{
	QString StdErr = QString(mProcess->readAllStandardError());
	mStdErrList.append(StdErr);
	QRegExp RegexPovRayProgress("Rendered (\\d+) of (\\d+) pixels.*");
	RegexPovRayProgress.setCaseSensitivity(Qt::CaseInsensitive);
	if (RegexPovRayProgress.indexIn(StdErr) == 0)
	{
		ui->RenderProgress->setMaximum(RegexPovRayProgress.cap(2).toInt());
		ui->RenderProgress->setValue(RegexPovRayProgress.cap(1).toInt());
        QApplication::processEvents();
	}
}

void lcRenderDialog::Update()
{
#ifndef QT_NO_PROCESS
	if (!mProcess)
		return;

	if (mProcess->state() == QProcess::NotRunning)
	{
#ifdef Q_OS_LINUX
		QByteArray Output = mProcess->readAllStandardOutput();
		mImage = QImage::fromData(Output);
		ShowResult();
#endif

		CloseProcess();
	}
#endif

#if LC_POVRAY_MEMORY_MAPPED_FILE
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

	int Width = Header->Width;
	int Height = Header->Height;
	int PixelsWritten = Header->PixelsWritten;

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

	Header->PixelsRead = PixelsWritten;

	if (PixelsWritten == Width * Height)
		ShowResult();
#endif
}

void lcRenderDialog::ShowResult()
{
	ReadStdErr();
	ui->RenderProgress->setValue(ui->RenderProgress->maximum());

/*** LPub3D Mod - resizable messagebox and set output log type  ***/
    bool stdOut = true;
    if (mProcess->exitStatus() != QProcess::NormalExit || mProcess->exitCode() != 0) {
        stdOut = false;
        QString text = tr ("An error occurred while rendering. Check details or try again.");
        QMessageBoxResizable box;
        box.setTextFormat (Qt::RichText);
        box.setIcon (QMessageBox::Critical);
        box.setStandardButtons (QMessageBox::Ok);
        box.setDefaultButton   (QMessageBox::Ok);
        box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        box.setWindowTitle(tr ("POV-Ray Render"));

        QString title = "<b> Render Error. </b>";
        box.setText (title);
        box.setInformativeText (text);
        box.setDetailedText(mStdErrList.join(""));
        box.exec();
        return;
    }
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Write error list to output file ***/
    QString logFile = QDir::toNativeSeparators(QDir::currentPath() + "/" + (stdOut ? "stdout-povrayrender" : "stderr-povrayrender"));
    QFile file(logFile);
    if (file.open(QFile::WriteOnly | QIODevice::Truncate | QFile::Text)) {
        QTextStream out(&file);
        foreach (QString line, mStdErrList) {
            out << line << endl;
        }
        file.close();
    } else {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not open file %1 for writing: %2")
                             .arg(logFile) .arg(file.errorString()));
    }
/*** LPub3D Mod end ***/

	ui->preview->setPixmap(QPixmap::fromImage(mImage.scaled(LC_POVRAY_PREVIEW_WIDTH, LC_POVRAY_PREVIEW_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

/*** LPub3D Mod - log message ***/
    emit gui->messageSig(LOG_INFO,QString("Writing POV-Ray rendered image '%1'").arg(ui->OutputEdit->text()));
    
    bool Result = false;
/*** LPub3D Mod end ***/

	QString FileName = ui->OutputEdit->text();

	if (!FileName.isEmpty())
	{
		QImageWriter Writer(FileName);

/*** LPub3D Mod - optional image auto crop ***/
        Result = Writer.write(mImage);

		if (!Result)
            emit gui->messageSig(LOG_ERROR,QString("Error writing to image file '%1':\n%2").arg(FileName, Writer.errorString()));
        else if (Preferences::povrayAutoCrop)
            Render::clipImage(FileName);
/*** LPub3D Mod end ***/
	}

/*** LPub3D Mod - Set completion message with elapsed time ***/
    QString message = QString("POV-Ray CSI Render %1. %2")
                              .arg(Result ? QString("generated %1").arg(FileName) : "image file failed")
                              .arg(gui->elapsedTime(mRenderTime.elapsed()));
    emit gui->messageSig(Result ? LOG_INFO : LOG_ERROR, message);
/*** LPub3D Mod end ***/
}

void lcRenderDialog::on_OutputBrowseButton_clicked()
{
	QString Result = QFileDialog::getSaveFileName(this, tr("Select Output File"), ui->OutputEdit->text(), tr("Supported Image Files (*.bmp *.png *.jpg);;BMP Files (*.bmp);;PNG Files (*.png);;JPEG Files (*.jpg);;All Files (*.*)"));

	if (!Result.isEmpty())
		ui->OutputEdit->setText(QDir::toNativeSeparators(Result));

}
