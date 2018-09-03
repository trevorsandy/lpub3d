
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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

/****************************************************************************
 *
 * This class encapsulates the external renderers.  For now, this means
 * only ldglite.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QtWidgets>
#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QtConcurrent>

#include "lpub.h"
#include "render.h"
#include "resolution.h"
#include "meta.h"
#include "math.h"
#include "lpub_preferences.h"
#include "application.h"

#include <LDVQt/LDVWidget.h>

#include "paths.h"

#include "lc_file.h"
#include "project.h"
#include "pieceinf.h"
#include "view.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

Render *renderer;

LDGLite ldglite;
LDView  ldview;
POVRay  povray;
Native  native;

//#define LduDistance 5729.57
#define CA "-ca0.01"
#define USE_ALPHA "+UA"

//Native renderer scale factor
#define SCALE_FACTOR_NATIVE 11658.9567325322

//Enable LDView single call -SaveSnapshotsList flag
//#ifndef LDVIEW_USE_SNAPSHOT_LIST
//#define LDVIEW_USE_SNAPSHOT_LIST
//#endif


static double pi = 4*atan(1.0);
// the default camera distance for real size
static float LduDistance = 10.0/tan(0.005*pi/180);

// renderer timeout in milliseconds
static int rendererTimeout(){
    if (Preferences::rendererTimeout == -1)
        return -1;
    else
        return Preferences::rendererTimeout*60*1000;
}

QString fixupDirname(const QString &dirNameIn) {
#ifdef Q_OS_WIN
    long     length = 0;
    TCHAR*   buffer = nullptr;
//  30/11/2014 Generating "invalid conversion from const ushort to const wchar" compile error:
//  LPCWSTR dirNameWin = dirNameIn.utf16();
    LPCWSTR dirNameWin = reinterpret_cast<LPCWSTR>(dirNameIn.utf16());

// First obtain the size needed by passing NULL and 0.

    length = GetShortPathName(dirNameWin, nullptr, 0);
    if (length == 0){
                QString message = QString("Couldn't get length of short path name length, lastError is %1, trying long path name").arg(GetLastError());
#ifdef QT_DEBUG_MODE
                qDebug() << message << "\n";
#else
                emit gui->messageSig(LOG_STATUS, message);
#endif
                return dirNameIn;
     }
// Dynamically allocate the correct size
// (terminating null char was included in length)

    buffer = new TCHAR[length];

// Now simply call again using same long path.

    length = GetShortPathName(dirNameWin, buffer, length);
    if (length == 0){
                QString message = QString("Couldn't get length of short path name length, lastError is %1, trying long path name").arg(GetLastError());
#ifdef QT_DEBUG_MODE
                qDebug() << message << "\n";
#else
                emit gui->messageSig(LOG_STATUS, message);
#endif
		return dirNameIn;
	}

	QString dirNameOut = QString::fromWCharArray(buffer);

    delete [] buffer;
        return dirNameOut;
#else
        return dirNameIn;
#endif
}

QString const Render::getRenderer()
{
  if (renderer == &ldglite)
  {
    return RENDERER_LDGLITE;
  }
  else
  if (renderer == &ldview)
  {
    return RENDERER_LDVIEW;
  }
  else
  if (renderer == &povray)
  {
    return RENDERER_POVRAY;
  }
  else
  {
    return RENDERER_NATIVE;
  }
}

void Render::setRenderer(QString const &name)
{
  if (name == RENDERER_LDGLITE)
  {
    renderer = &ldglite;
  }
  else
  if (name == RENDERER_LDVIEW)
  {
    renderer = &ldview;
  }
  else
  if (name == RENDERER_POVRAY)
  {
    renderer = &povray;
  }
  else
  {
    renderer = &native;
  }
}

const QString Render::getRotstepMeta(RotStepMeta &rotStep){
  QString rotstepMeta = QString("0 // ROTSTEP %1 %2 %3 %4")
                                .arg(rotStep.value().type)
                                .arg(rotStep.value().rots[0])
                                .arg(rotStep.value().rots[1])
                                .arg(rotStep.value().rots[2]);
  return rotstepMeta;
}

bool Render::useLDViewSCall(bool override){
  if (override)
    return override;
  else
    return Preferences::useLDViewSingleCall;
}

bool clipImage(QString const &pngName) {

    QImage toClip(QDir::toNativeSeparators(pngName));
    QRect clipBox;

    int minX = toClip.width(); int maxX = 0;
    int minY = toClip.height();int maxY = 0;

    for(int x=0; x < toClip.width(); x++)
        for(int y=0; y < toClip.height(); y++)
            if (qAlpha(toClip.pixel(x, y)))
            {
                minX = qMin(x, minX);
                minY = qMin(y, minY);
                maxX = qMax(x, maxX);
                maxY = qMax(y, maxY);
            }

    if (minX > maxX || minY > maxY) {
        emit gui->messageSig(LOG_STATUS, qPrintable("No opaque content in " + pngName));
        return false;
    } else {
        clipBox.setCoords(minX, minY, maxX, maxY);
    }

    //save clipBox;
    QImage clipped = toClip.copy(clipBox);
    QString clipMsg = QString("%1 (w:%2 x h:%3)")
                              .arg(pngName)
                              .arg(clipped.width())
                              .arg(clipped.height());

    if (clipped.save(QDir::toNativeSeparators(pngName))) {
        emit gui->messageSig(LOG_STATUS, qPrintable("Clipped " + clipMsg));
    } else {
        emit gui->messageSig(LOG_ERROR, qPrintable("Failed to save clipped image " + clipMsg));
        return false;
    }
    return true;
 }

// Shared calculations
float stdCameraDistance(Meta &meta, float scale) {
	float onexone;
	float factor;

	// Do the math in pixels

	onexone  = 20*meta.LPub.resolution.ldu(); // size of 1x1 in units
	onexone *= meta.LPub.resolution.value();  // size of 1x1 in pixels
	onexone *= scale;
	factor   = gui->pageSize(meta.LPub.page, 0)/onexone; // in pixels;

//	logDebug() << qPrintable(QString("LduDistance                      : %1").arg(LduDistance));
//	logDebug() << qPrintable(QString("Page Size (width in pixels)      : %1").arg(gui->pageSize(meta.LPub.page, 0)));
//	logDebug() << qPrintable(QString("Resolution Ldu                   : %1").arg(QString::number(meta.LPub.resolution.ldu(), 'f' ,10)));
//	logDebug() << qPrintable(QString("Resolution pixel                 : %1").arg(meta.LPub.resolution.value()));
//	logDebug() << qPrintable(QString("Scale                            : %1").arg(scale));
//	logDebug() << qPrintable(QString("1x1 [20*res.ldu*res.pix*scale]   : %1").arg(QString::number(onexone, 'f' ,10)));
//	logDebug() << qPrintable(QString("Factor [Page size/OnexOne]       : %1").arg(QString::number(factor, 'f' ,10)));
//	logDebug() << qPrintable(QString("Cam Distance [Factor*LduDistance]: %1").arg(QString::number(factor*LduDistance, 'f' ,10)));

	return factor*LduDistance;
}



/***************************************************************************
 *
 * The math for zoom factor.  1.0 is true size.
 *
 * 1 LDU is 1/64 of an inch
 *
 * LDGLite produces 72 DPI
 *
 * Camera angle is 0.01
 *
 * What distance do we need to put the camera, given a user chosen DPI,
 * to get zoom factor of 1.0?
 *
 **************************************************************************/


/***************************************************************************
 *
 * POVRay renderer
 *
 **************************************************************************/

float POVRay::cameraDistance(
    Meta &meta,
    float scale)
{
  return stdCameraDistance(meta, scale)*0.455;
}

int POVRay::renderCsi(
    const QString     &addLine,
    const QStringList &csiParts,
    const QStringList &csiKeys,
    const QString     &pngName,
    Meta              &meta)
{

  /* Create the CSI DAT file */
  QString ldrName = QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";
  QString povName = ldrName + ".pov";
  QStringList list;
  QString message;

  int width  = gui->pageSize(meta.LPub.page, 0);
  int height = gui->pageSize(meta.LPub.page, 1);

  int rc;
  if ((rc = rotateParts(addLine,meta.rotStep, csiParts, ldrName, QString())) < 0) {
      return rc;
   }

  /* determine camera distance */
  int cd = cameraDistance(meta,meta.LPub.assem.modelScale.value())*1700/1000;

      //QString cg = QString("-cg0.0,0.0,%1") .arg(cd);
  QString cg = QString("-cg%1,%2,%3")
      .arg(meta.LPub.assem.angle.value(0))
      .arg(meta.LPub.assem.angle.value(1))
      .arg(cd);

  QString w  = QString("-SaveWidth=%1") .arg(width);
  QString h  = QString("-SaveHeight=%1") .arg(height);
  QString f  = QString("-ExportFile=%1") .arg(povName);
  QString l  = QString("-LDrawDir=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldrawPath)));
  QString o  = QString("-HaveStdOut=1");
  QString v  = QString("-vv");

  QStringList arguments;
  arguments << CA;
  arguments << cg;
  arguments << w;
  arguments << h;
  arguments << f;
  arguments << l;

  if (!Preferences::altLDConfigPath.isEmpty()) {
     arguments << "-LDConfig=" + Preferences::altLDConfigPath;
     //logDebug() << qPrintable("-LDConfig=" + Preferences::altLDConfigPath);
  }

  // LDView block begin
  if (Preferences::povFileGenerator == RENDERER_LDVIEW) {

      arguments << o;
      arguments << v;

      if (Preferences::enableFadeSteps)
        arguments <<  QString("-SaveZMap=1");

      list = meta.LPub.assem.ldviewParms.value().split(' ');
      for (int i = 0; i < list.size(); i++) {
          if (list[i] != "" && list[i] != " ") {
              arguments << list[i];
              //logInfo() << qPrintable("-PARM META: " + list[i]);
            }
        }

      bool hasLDViewIni = Preferences::ldviewPOVIni != "";
      if(hasLDViewIni){
          QString ini  = QString("-IniFile=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldviewPOVIni)));
          arguments << ini;
        }
      if (!Preferences::altLDConfigPath.isEmpty()) {
          arguments << "-LDConfig=" + Preferences::altLDConfigPath;
          //logDebug() << qPrintable("-LDConfig=" + Preferences::altLDConfigPath);
        }

      arguments << ldrName;

      emit gui->messageSig(LOG_STATUS, "LDView POV CSI file generation...");

      QProcess    ldview;
      ldview.setEnvironment(QProcess::systemEnvironment());
      ldview.setWorkingDirectory(QDir::currentPath() + "/" + Paths::tmpDir);
      ldview.setStandardErrorFile(QDir::currentPath() + "/stderr-ldviewpov");
      ldview.setStandardOutputFile(QDir::currentPath() + "/stdout-ldviewpov");

      message = QString("LDView POV file generate CSI Arguments: %1 %2").arg(Preferences::ldviewExe).arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
      qDebug() << qPrintable(message);
#else
      emit gui->messageSig(LOG_STATUS, message);
#endif

      ldview.start(Preferences::ldviewExe,arguments);
      if ( ! ldview.waitForFinished(rendererTimeout())) {
          if (ldview.exitCode() != 0 || 1) {
              QByteArray status = ldview.readAll();
              QString str;
              str.append(status);
              emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView POV file generation failed with exit code %1\n%2") .arg(ldview.exitCode()) .arg(str));
              return -1;
          }
      }
  }
  else
  // Native POV Generator block
  if (Preferences::povFileGenerator == RENDERER_NATIVE) {

      QString workingDirectory = QDir::currentPath();

      arguments << ldrName;

      emit gui->messageSig(LOG_STATUS, "Native POV CSI file generation...");

      bool retError = false;
      ldvWidget = new LDVWidget();
      ldvWidget->setIniFlag();
      if (! ldvWidget->doCommand(arguments))  {
          emit gui->messageSig(LOG_ERROR, QString("Failed to generate CSI POV file for command: %1").arg(arguments.join(" ")));
          retError = true;
      }
      // ldvWidget changes the Working directory so we must reset
      if (! QDir::setCurrent(workingDirectory)) {
          emit gui->messageSig(LOG_ERROR, QString("Failed to restore CSI POV working directory %1").arg(workingDirectory));
          retError = true;
      }
      if (retError)
          return -1;
  }

  QStringList povArguments;
  if (Preferences::povrayDisplay){
      povArguments << QString("+d");
  } else {
      povArguments << QString("-d");
  }

  QString O = QString("+O\"%1\"").arg(QDir::toNativeSeparators(pngName));
  QString W = QString("+W%1").arg(width);
  QString H = QString("+H%1").arg(height);

  povArguments << O;
  povArguments << W;
  povArguments << H;
  povArguments << USE_ALPHA;

  list = meta.LPub.assem.povrayParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
      if (list[i] != "" && list[i] != " ") {
          povArguments << list[i];
          //logInfo() << qPrintable("-PARM META: " + list[i]);
      }
  }

  bool hasSTL       = Preferences::lgeoStlLib;
  bool hasLGEO      = Preferences::lgeoPath != "";
  bool hasPOVRayIni = Preferences::povrayIniPath != "";
  bool hasPOVRayInc = Preferences::povrayIncPath != "";

  if(hasPOVRayInc){
      QString povinc = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::povrayIncPath)));
      povArguments << povinc;
  }
  if(hasPOVRayIni){
      QString povini = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::povrayIniPath)));
      povArguments << povini;
  }
  if(hasLGEO){
      QString lgeoLg = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/lg")));
      QString lgeoAr = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/ar")));
      povArguments << lgeoLg;
      povArguments << lgeoAr;
      if (hasSTL){
          QString lgeoStl = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/stl")));
          povArguments << lgeoStl;
      }
  }

  QString I = QString("+I\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(povName)));
  povArguments.insert(2,I);

//#ifndef __APPLE__
//  povArguments << "/EXIT";
//#endif

  emit gui->messageSig(LOG_STATUS, "Executing POVRay render CSI - please wait...");

  QProcess povray;
  QStringList povEnv = QProcess::systemEnvironment();
  povray.setEnvironment(povEnv);
  povray.setWorkingDirectory(QDir::currentPath()+ "/" + Paths::assemDir); // pov win console app will not write to dir different from cwd or source file dir
  povray.setStandardErrorFile(QDir::currentPath() + "/stderr-povray");
  povray.setStandardOutputFile(QDir::currentPath() + "/stdout-povray");

  message = QString("POVRay CSI Arguments: %1 %2").arg(Preferences::povrayExe).arg(povArguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_STATUS, message);
#endif

  povray.start(Preferences::povrayExe,povArguments);
  if ( ! povray.waitForFinished(rendererTimeout())) {
      if (povray.exitCode() != 0) {
          QByteArray status = povray.readAll();
          QString str;
          str.append(status);
          emit gui->messageSig(LOG_ERROR,QMessageBox::tr("POVRay CSI render failed with code %1\n%2").arg(povray.exitCode()) .arg(str));
          return -1;
        }
    }

  if (Preferences::enableFadeSteps) {
      QString previousPngFile = imageMatt.previousStepCSIImage(csiKeys.first());
      if (!previousPngFile.isEmpty()) { // first entry returns "" so check first
          //logDebug() << qPrintable(QString("Previous CSI pngFile: %1").arg(previousPngFile));
      }
  }

  if (clipImage(pngName))
    return 0;
  else
    return -1;
}

int POVRay::renderPli(
    const QStringList &ldrNames ,
    const QString     &pngName,
    Meta    	      &meta,
    bool     	      bom)
{
  QString povName = ldrNames.first() +".pov";  
  PliMeta &metaType = bom ? meta.LPub.bom : meta.LPub.pli;
  QStringList list;
  QString message;

  int width  = gui->pageSize(meta.LPub.page, 0);
  int height = gui->pageSize(meta.LPub.page, 1);

  /* determine camera distance */
  int cd = cameraDistance(meta,metaType.modelScale.value())*1700/1000;

  //QString cg = QString("-cg0.0,0.0,%1") .arg(cd);
  QString cg = QString("-cg%1,%2,%3")
      .arg(metaType.angle.value(0))
      .arg(metaType.angle.value(1))
      .arg(cd);

  QString w  = QString("-SaveWidth=%1")  .arg(width);
  QString h  = QString("-SaveHeight=%1") .arg(height);
  QString f  = QString("-ExportFile=%1") .arg(povName);  // -ExportSuffix not required
  QString l  = QString("-LDrawDir=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldrawPath)));
  QString o  = QString("-HaveStdOut=1");
  QString v  = QString("-vv");

  QStringList arguments;
  arguments << CA;
  arguments << cg;
  arguments << w;
  arguments << h;
  arguments << f;
  arguments << l;

  if (!Preferences::altLDConfigPath.isEmpty()) {
     arguments << "-LDConfig=" + Preferences::altLDConfigPath;
     //logDebug() << qPrintable("-LDConfig=" + Preferences::altLDConfigPath);
  }

  // LDView block begin
  if (Preferences::povFileGenerator == RENDERER_LDVIEW) {

      arguments << o;
      arguments << v;

      list = meta.LPub.pli.ldviewParms.value().split(' ');
      for (int i = 0; i < list.size(); i++) {
          if (list[i] != "" && list[i] != " ") {
              arguments << list[i];
              //logInfo() << qPrintable("-PARM META: " + list[i]);
            }
        }

      bool hasLDViewIni = Preferences::ldviewPOVIni != "";
      if(hasLDViewIni){
          QString ini  = QString("-IniFile=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldviewPOVIni)));
          arguments << ini;
        }

      arguments << ldrNames.first();

      emit gui->messageSig(LOG_STATUS, "LDView POV PLI file generation...");

      QProcess    ldview;
      ldview.setEnvironment(QProcess::systemEnvironment());
      ldview.setWorkingDirectory(QDir::currentPath());
      ldview.setStandardErrorFile(QDir::currentPath() + "/stderr-ldviewpov");
      ldview.setStandardOutputFile(QDir::currentPath() + "/stdout-ldviewpov");

      message = QString("LDView POV file generate PLI Arguments: %1 %2").arg(Preferences::ldviewExe).arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
      qDebug() << qPrintable(message);
#else
      emit gui->messageSig(LOG_STATUS, message);
#endif

      ldview.start(Preferences::ldviewExe,arguments);
      if ( ! ldview.waitForFinished()) {
          if (ldview.exitCode() != 0) {
              QByteArray status = ldview.readAll();
              QString str;
              str.append(status);
              emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView POV file generation failed with exit code %1\n%2") .arg(ldview.exitCode()) .arg(str));
              return -1;
          }
      }
  }
  else
  // Native POV Generator block
  if (Preferences::povFileGenerator == RENDERER_NATIVE) {

      QString workingDirectory = QDir::currentPath();

      arguments << ldrNames.first();

      emit gui->messageSig(LOG_STATUS, "Native POV PLI file generation...");

      bool retError = false;
      ldvWidget = new LDVWidget();
      ldvWidget->setIniFlag();
      if (! ldvWidget->doCommand(arguments)) {
          emit gui->messageSig(LOG_ERROR, QString("Failed to generate PLI POV file for command: %1").arg(arguments.join(" ")));
          retError = true;
      }
      // ldvWidget changes the Working directory so we must reset
      if (! QDir::setCurrent(workingDirectory)) {
          emit gui->messageSig(LOG_ERROR, QString("Failed to restore PLI POV working directory %1").arg(workingDirectory));
          retError = true;
      }
      if (retError)
        return -1;
  }

  QStringList povArguments;
  if (Preferences::povrayDisplay){
      povArguments << QString("+d");
  } else {
      povArguments << QString("-d");
  }

  QString O = QString("+O\"%1\"").arg(QDir::toNativeSeparators(pngName));
  QString W = QString("+W%1").arg(width);
  QString H = QString("+H%1").arg(height);

  povArguments << O;
  povArguments << W;
  povArguments << H;
  povArguments << USE_ALPHA;

  list = meta.LPub.assem.povrayParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
      if (list[i] != "" && list[i] != " ") {
          povArguments << list[i];
          //logInfo() << qPrintable("-PARM META: " + list[i]);
      }
  }

  bool hasSTL       = Preferences::lgeoStlLib;
  bool hasLGEO      = Preferences::lgeoPath != "";
  bool hasPOVRayIni = Preferences::povrayIniPath != "";
  bool hasPOVRayInc = Preferences::povrayIncPath != "";

  if(hasPOVRayInc){
      QString povinc = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::povrayIncPath)));
      povArguments << povinc;
  }
  if(hasPOVRayIni){
      QString povini = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::povrayIniPath)));
      povArguments << povini;
  }
  if(hasLGEO){
      QString lgeoLg = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/lg")));
      QString lgeoAr = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/ar")));
      povArguments << lgeoLg;
      povArguments << lgeoAr;
      if (hasSTL){
          QString lgeoStl = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/stl")));
          povArguments << lgeoStl;
        }
    }

  QString I = QString("+I\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(povName)));
  povArguments.insert(2,I);

//#ifndef __APPLE__
//  povArguments << "/EXIT";
//#endif

  emit gui->messageSig(LOG_STATUS, "Executing POVRay render PLI - please wait...");

  QProcess povray;
  QStringList povEnv = QProcess::systemEnvironment();
  povray.setEnvironment(povEnv);
  povray.setWorkingDirectory(QDir::currentPath()+ "/" + Paths::partsDir); // pov win console app will not write to dir different from cwd or source file dir
  povray.setStandardErrorFile(QDir::currentPath() + "/stderr-povray");
  povray.setStandardOutputFile(QDir::currentPath() + "/stdout-povray");

  message = QString("POVRay PLI Arguments: %1 %2").arg(Preferences::povrayExe).arg(povArguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_STATUS, message);
#endif

  povray.start(Preferences::povrayExe, povArguments);
  if ( ! povray.waitForFinished(rendererTimeout())) {
      if (povray.exitCode() != 0) {
          QByteArray status = povray.readAll();
          QString str;
          str.append(status);
          emit gui->messageSig(LOG_ERROR,QMessageBox::tr("POVRay PLI render failed with code %1\n%2") .arg(povray.exitCode()) .arg(str));
          return -1;
      }
  }

  if (clipImage(pngName))
    return 0;
  else
    return -1;
}


/***************************************************************************
 *
 * LDGLite renderer
 *
 **************************************************************************/

float LDGLite::cameraDistance(
  Meta &meta,
  float scale)
{
	return stdCameraDistance(meta,scale);
}

int LDGLite::renderCsi(
  const QString     &addLine,
  const QStringList &csiParts,
  const QStringList &csiKeys,
  const QString     &pngName,
        Meta        &meta)
{
  /* Create the CSI DAT file */
  QString ldrPath, ldrName, ldrFile;
  int rc;
  ldrName = "csi.ldr";
  ldrPath = QDir::currentPath() + "/" + Paths::tmpDir;
  ldrFile = ldrPath + "/" + ldrName;
  if ((rc = rotateParts(addLine,meta.rotStep, csiParts, ldrFile,QString())) < 0) {
     return rc;
  }

  /* determine camera distance */

  int cd = cameraDistance(meta,meta.LPub.assem.modelScale.value());

  int width  = gui->pageSize(meta.LPub.page, 0);
  int height = gui->pageSize(meta.LPub.page, 1);

  QString v  = QString("-v%1,%2")   .arg(width)
                                    .arg(height);
  QString o  = QString("-o0,-%1")   .arg(height/6);
  QString mf = QString("-mF%1")     .arg(pngName);

  int lineThickness = resolution()/150+0.5;
  if (lineThickness == 0) {
    lineThickness = 1;
  }

  QString w  = QString("-W%1")      .arg(lineThickness); // ldglite always deals in 72 DPI

  //QString cg = QString("-cg0.0,0.0,%1") .arg(cd);
  QString cg = QString("-cg%1,%2,%3") .arg(meta.LPub.assem.angle.value(0))
                                      .arg(meta.LPub.assem.angle.value(1))
                                      .arg(cd);

  QStringList arguments;
  arguments << CA;                  // camera FOV angle in degrees
  arguments << cg;                  // camera globe - scale factor
  arguments << v;                   // display in X wide by Y high window
  arguments << o;                   // changes the center X across and Y down
  arguments << w;                   // line thickness

  QStringList list;
  // First, load parms from meta
  list = meta.LPub.assem.ldgliteParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
     if (list[i] != "" && list[i] != " ") {
         arguments << list[i];
         //logInfo() << qPrintable("-PARM META: " + list[i]);
      }
  }
  // Add ini parms if not already added from meta
  for (int i = 0; i < Preferences::ldgliteParms.size(); i++) {
      if (list.indexOf(QRegExp("^" + QRegExp::escape(Preferences::ldgliteParms[i]))) < 0) {
        arguments << Preferences::ldgliteParms[i];
        //logInfo() << qPrintable("-PARM INI : " + Preferences::ldgliteParms[i]);
      }
  }

  // add custom colour file if exist
  if (!Preferences::altLDConfigPath.isEmpty()) {
    arguments << "-ldcF" + Preferences::altLDConfigPath;
    //logDebug() << qPrintable("=" + Preferences::altLDConfigPath);
  }

  arguments << mf;                  // .png file name
  arguments << ldrFile;             // csi.ldr (input file)

  emit gui->messageSig(LOG_STATUS, "Executing LDGLite render CSI - please wait...");

  QProcess    ldglite;
  QStringList env = QProcess::systemEnvironment();
  env << "LDRAWDIR=" + Preferences::ldrawPath;
  //logDebug() << qPrintable("LDRAWDIR=" + Preferences::ldrawPath);

  if (!Preferences::ldgliteSearchDirs.isEmpty()) {
    env << "LDSEARCHDIRS=" + Preferences::ldgliteSearchDirs;
    //logDebug() << qPrintable("LDSEARCHDIRS: " + Preferences::ldgliteSearchDirs);
  }

  ldglite.setEnvironment(env);
  //logDebug() << qPrintable("ENV: " + env);

  ldglite.setWorkingDirectory(QDir::currentPath() + "/" + Paths::tmpDir);
  ldglite.setStandardErrorFile(QDir::currentPath() + "/stderr-ldglite");
  ldglite.setStandardOutputFile(QDir::currentPath() + "/stdout-ldglite");

  QString message = QString("LDGLite CSI Arguments: %1 %2").arg(Preferences::ldgliteExe).arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_STATUS, message);
#endif

  ldglite.start(Preferences::ldgliteExe,arguments);
  if ( ! ldglite.waitForFinished(rendererTimeout())) {
    if (ldglite.exitCode() != 0) {
      QByteArray status = ldglite.readAll();
      QString str;
      str.append(status);
      emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDGlite failed\n%1") .arg(str));
      return -1;
    }
  }

  // image matting stub
  if (Preferences::enableFadeSteps) {
      QString previousPngFile = imageMatt.previousStepCSIImage(csiKeys.first());
      if (!previousPngFile.isEmpty()) { // first entry returns "" so check first
          //logDebug() << qPrintable(QString("Previous CSI pngFile: %1").arg(previousPngFile));
      }
  }

  return 0;
}


int LDGLite::renderPli(
  const QStringList &ldrNames,
  const QString     &pngName,
  Meta              &meta,
  bool               bom)
{
  int width  = gui->pageSize(meta.LPub.page, 0);
  int height = gui->pageSize(meta.LPub.page, 1);

  int lineThickness = resolution()/72.0+0.5;

  /* determine camera distance */

  PliMeta &metaType = bom ? meta.LPub.bom : meta.LPub.pli;

  int cd = cameraDistance(meta,metaType.modelScale.value());

  QString cg = QString("-cg%1,%2,%3") .arg(metaType.angle.value(0))
                                      .arg(metaType.angle.value(1))
                                      .arg(cd);

  QString v  = QString("-v%1,%2")   .arg(width)
                                    .arg(height);
  QString o  = QString("-o0,-%1")   .arg(height/6);
  QString mf = QString("-mF%1")     .arg(pngName);

  QString w  = QString("-W%1")      .arg(lineThickness);  // ldglite always deals in 72 DPI

  QStringList arguments;
  arguments << CA;                  // camera FOV angle in degrees
  arguments << cg;                  // camera globe - scale factor
  arguments << v;                   // display in X wide by Y high window
  arguments << o;                   // changes the center X across and Y down
  arguments << w;                   // line thickness

  QStringList list;
  // First, load parms from meta
  list = meta.LPub.pli.ldgliteParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
     if (list[i] != "" && list[i] != " ") {
         arguments << list[i];
         //logInfo() << qPrintable("-PARM META: " + list[i]);
      }
  }
  // Add ini parms if not already added from meta
  for (int i = 0; i < Preferences::ldgliteParms.size(); i++) {
      if (list.indexOf(QRegExp("^" + QRegExp::escape(Preferences::ldgliteParms[i]))) < 0) {
        arguments << Preferences::ldgliteParms[i];
        //logInfo() << qPrintable("-PARM INI : " + Preferences::ldgliteParms[i]);
      }
  }

  // add custom colour file if exist
  if (!Preferences::altLDConfigPath.isEmpty()) {
    arguments << "-ldcF" + Preferences::altLDConfigPath;
    //logDebug() << qPrintable("=" + Preferences::altLDConfigPath);
  }

  arguments << mf;
  arguments << ldrNames.first();

  emit gui->messageSig(LOG_STATUS, "Executing LDGLite render PLI - please wait...");

  QProcess    ldglite;
  QStringList env = QProcess::systemEnvironment();
  env << "LDRAWDIR=" + Preferences::ldrawPath;
  //logDebug() << qPrintable("LDRAWDIR=" + Preferences::ldrawPath);

  if (!Preferences::ldgliteSearchDirs.isEmpty()){
    env << "LDSEARCHDIRS=" + Preferences::ldgliteSearchDirs;
    //logDebug() << qPrintable("LDSEARCHDIRS: " + Preferences::ldgliteSearchDirs);
  }

  ldglite.setEnvironment(env);
  ldglite.setWorkingDirectory(QDir::currentPath());
  ldglite.setStandardErrorFile(QDir::currentPath() + "/stderr-ldglite");
  ldglite.setStandardOutputFile(QDir::currentPath() + "/stdout-ldglite");

  QString message = QString("LDGLite PLI Arguments: %1 %2").arg(Preferences::ldgliteExe).arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_STATUS, message);
#endif

  ldglite.start(Preferences::ldgliteExe,arguments);
  if (! ldglite.waitForFinished()) {
    if (ldglite.exitCode()) {
      QByteArray status = ldglite.readAll();
      QString str;
      str.append(status);
      emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDGlite failed\n%1") .arg(str));
      return -1;
    }
  }
  return 0;
}


/***************************************************************************
 *
 * LDView renderer
 *                                  6x6                    5990
 *      LDView               LDView    LDGLite       LDView
 * 0.1    8x5     8x6         32x14    40x19  0.25  216x150    276x191  0.28
 * 0.2   14x10   16x10                              430x298    552x381
 * 0.3   20x14   20x15                              644x466    824x571
 * 0.4   28x18   28x19                              859x594   1100x762
 * 0.5   34x22   36x22                             1074x744   1376x949  0.28
 * 0.6   40x27   40x28                             1288x892
 * 0.7   46x31   48x32                            1502x1040
 * 0.8   54x35   56x37
 * 0.9   60x40   60x41
 * 1.0   66x44   68x46       310x135  400x175 0.29
 * 1.1   72x48
 * 1.2   80x53
 * 1.3   86x57
 * 1.4   92x61
 * 1.5   99x66
 * 1.6  106x70
 * 2.0  132x87  132x90       620x270  796x348 0.28
 * 3.0  197x131 200x134      930x404 1169x522
 * 4.0  262x174 268x178     1238x539 1592x697 0.29
 * 5.0  328x217 332x223     1548x673
 *
 *
 **************************************************************************/

float LDView::cameraDistance(
  Meta &meta,
  float scale)
{
	return stdCameraDistance(meta, scale)*0.775;
}

int LDView::renderCsi(
  const QString     &addLine,
  const QStringList &csiParts,   // ldrNames
  const QStringList &csiKeys,
  const QString     &pngName,
        Meta        &meta)
{
  /* determine camera distance */
  int cd = cameraDistance(meta,meta.LPub.assem.modelScale.value())*1700/1000;

  /* page size */
  int width  = gui->pageSize(meta.LPub.page, 0);
  int height = gui->pageSize(meta.LPub.page, 1);

  /* edge thickness */
  int edgeThickness = 1;

  bool hasLDViewIni = Preferences::ldviewIni != "";

  /* Create the CSI DAT file(s) */
  QString f;
  QStringList ldrNames;
  if (useLDViewSCall()) {
      ldrNames = csiParts;
#ifndef LDVIEW_USE_SNAPSHOT_LIST
      f  = QString("-SaveSnapShots=1");
#else
      QString snapShotList = QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";
      QFile snapShotListFile(snapShotList);
      if ( ! snapShotListFile.open(QFile::Append | QFile::Text)) {
          emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView (Single Call) CSI Snapshots list creation failed!"));
          return -1;
      }
      QTextStream out(&snapShotListFile);
      for (int i = 0; i < ldrNames.size(); i++) {
          QString smLine = ldrNames[i];
          out << smLine << endl;
          //logInfo() << qPrintable(QString("CSI Snapshots line: %1").arg(smLine));
      }
      snapShotListFile.close();
      f  = QString("-SaveSnapshotsList=%1").arg(snapShotList);
#endif
  } else {
      f  = QString("-SaveSnapShot=%1") .arg(pngName);
      int rc;
      ldrNames << QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";
      if ((rc = rotateParts(addLine, meta.rotStep,csiParts,ldrNames.first(),QString())) < 0) {
          emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView (Single Call) CSI rotate parts failed!"));
          return rc;
      }
  }

//  QString cg = QString("-cg0.0,0.0,%1") .arg(cd);
  QString cg = QString("-cg%1,%2,%3") .arg(meta.LPub.assem.angle.value(0))
                                      .arg(meta.LPub.assem.angle.value(1))
                                      .arg(cd);

  QString w  = QString("-SaveWidth=%1")  .arg(width);
  QString h  = QString("-SaveHeight=%1") .arg(height);
  QString l  = QString("-LDrawDir=%1").arg(Preferences::ldrawPath);
  QString o  = QString("-HaveStdOut=1");
  QString e  = QString("-EdgeThickness=%1").arg(edgeThickness);
  QString v  = QString("-vv");

  QStringList arguments;
  arguments << CA;
  arguments << cg;
  arguments << w;
  arguments << h;
  arguments << f;
  arguments << l;
  arguments << o;
  arguments << e;
  arguments << v;

  if (Preferences::enableFadeSteps)
    arguments <<  QString("-SaveZMap=1");

  QStringList list;
  list = meta.LPub.assem.ldviewParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
    if (list[i] != "" && list[i] != " ") {
      arguments << list[i];
      //logInfo() << qPrintable("-PARM META: " + list[i]);
    }
  }
  if(hasLDViewIni){
      QString ini  = QString("-IniFile=%1") .arg(Preferences::ldviewIni);
      arguments << ini;
  }
  if (!Preferences::altLDConfigPath.isEmpty()) {
    arguments << "-LDConfig=" + Preferences::altLDConfigPath;
    //logDebug() << qPrintable("-LDConfig=" + Preferences::altLDConfigPath);
  }

#ifndef LDVIEW_USE_SNAPSHOT_LIST
  arguments = arguments + ldrNames;
#else
  arguments << ldrNames.first();
#endif

  emit gui->messageSig(LOG_STATUS, "Executing LDView render CSI - please wait...");

  QProcess    ldview;
  ldview.setEnvironment(QProcess::systemEnvironment());
  ldview.setWorkingDirectory(QDir::currentPath() + "/" + Paths::tmpDir);
  ldview.setStandardErrorFile(QDir::currentPath() + "/stderr-ldview");
  ldview.setStandardOutputFile(QDir::currentPath() + "/stdout-ldview");

  QString message = QString("LDView (Native) CSI Arguments: %1 %2").arg(Preferences::ldviewExe).arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_STATUS, message);
#endif

  ldview.start(Preferences::ldviewExe,arguments);
  if ( ! ldview.waitForFinished(rendererTimeout())) {
    if (ldview.exitCode() != 0 || 1) {
      QByteArray status = ldview.readAll();
      QString str;
      str.append(status);
      emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView CSI render failed with code %1\n%2").arg(ldview.exitCode()) .arg(str));
      return -1;
    }
  }

  // move generated CSI images
  if (useLDViewSCall()){
    QString ldrName;
    QDir dir(QDir::currentPath() + "/" + Paths::tmpDir);
    foreach(ldrName, ldrNames){
        QFileInfo pngFileInfo(ldrName.replace(".ldr",".png"));
        QString pngFilePath = QDir::currentPath() + "/" +
            Paths::assemDir + "/" + pngFileInfo.fileName();
        if (! dir.rename(pngFileInfo.absoluteFilePath(), pngFilePath)){
            QFile pngFile(pngFilePath);
            if (! pngFile.exists()){ // file not found failure
                emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView CSI image file move failed for\n%1")
                                     .arg(pngFilePath));
                return -1;
              } else {                // file exist failure
                //file exist so delete and retry
                QFile pngFile(pngFilePath);
                if (pngFile.remove()) {
                    //retry
                    if (! dir.rename(pngFileInfo.absoluteFilePath(), pngFilePath)){
                        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView CSI image file move failed after old file removal for\n%1")
                                             .arg(pngFilePath));
                        return -1;
                      }
                  } else {
                    emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView could not remove old CSI image file \n%1")
                                         .arg(pngFilePath));
                    return -1;
                  }
              }
          }
      }

    // image mapping stub - LDView SingleCall
    if (Preferences::enableFadeSteps) {
          for (int i = 0; i < csiKeys.size(); i++) {
              QString previousPngFile = imageMatt.previousStepCSIImage(csiKeys[i]);
              QString currentPngFile = imageMatt.currentStepCSIImage(csiKeys[i]);
               if (!previousPngFile.isEmpty() && !currentPngFile.isEmpty()) {  // first entry returns "" so check first
                   imageMatt.processZMap(previousPngFile, currentPngFile);
                   logDebug() << qPrintable(QString("[DEBUG] Previous CSI pngFile: %1").arg(previousPngFile));
                   logDebug() << qPrintable(QString("[DEBUG] Current CSI pngFile: %1").arg(currentPngFile));
              }
          }
      }
  }
  else
  {
      // image matting stub
      if (Preferences::enableFadeSteps) {
          QString previousPngFile = imageMatt.previousStepCSIImage(csiKeys.first());
          QString currentPngFile = imageMatt.currentStepCSIImage(csiKeys.first());
          if (!previousPngFile.isEmpty() && !currentPngFile.isEmpty()) { // first entry returns "" so check first
              imageMatt.processZMap(previousPngFile, currentPngFile);
              logDebug() << qPrintable(QString("[DEBUG] Previous CSI pngFile: %1").arg(previousPngFile));
              logDebug() << qPrintable(QString("[DEBUG] Current CSI pngFile: %1").arg(currentPngFile));
          }
      }
  }

  return 0;
}

int LDView::renderPli(
  const QStringList &ldrNames,
  const QString     &pngName,
  Meta              &meta,
  bool              bom)
{
  QFileInfo fileInfo(ldrNames.first());
  if ( ! fileInfo.exists()) {
    emit gui->messageSig(LOG_ERROR,QMessageBox::tr("File ldrNames does not exist!"));
    return -1;
  }

  PliMeta &metaType = bom ? meta.LPub.bom : meta.LPub.pli;

  /* determine camera distance */
  int cd = cameraDistance(meta,metaType.modelScale.value())*1700/1000;

  /* page size */
  int width  = gui->pageSize(meta.LPub.page, 0);
  int height = gui->pageSize(meta.LPub.page, 1);

  /* edge thickness */
  int edgeThickness = 1;

  bool hasLDViewIni = Preferences::ldviewIni != "";

  //qDebug() << "LDView (Native) Camera Distance: " << cd;

  /* Create the CSI DAT file(s) */
  QString f;
  if (useLDViewSCall()) {
#ifndef LDVIEW_USE_SNAPSHOT_LIST
      f  = QString("-SaveSnapShots=1");
#else
      QString snapShotList = QDir::currentPath() + "/" + Paths::tmpDir + "/pli.ldr";
      QFile snapShotListFile(snapShotList);
      if ( ! snapShotListFile.open(QFile::Append | QFile::Text)) {
          emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView (Single Call) PLI Snapshots list creation failed!"));
          return -1;
      }
      QTextStream out(&snapShotListFile);
      for (int i = 0; i < ldrNames.size(); i++) {
          QString smLine = ldrNames[i];
          out << smLine << endl;
          //logInfo() << qPrintable(QString("PLI Snapshots line: %1").arg(smLine));
      }
      snapShotListFile.close();
      f  = QString("-SaveSnapshotsList=%1").arg(snapShotList);
#endif
  } else {
      f  = QString("-SaveSnapShot=%1") .arg(pngName);
  }

  QString cg = QString("-cg%1,%2,%3") .arg(metaType.angle.value(0))
                                      .arg(metaType.angle.value(1))
                                      .arg(cd);

  QString w  = QString("-SaveWidth=%1")  .arg(width);
  QString h  = QString("-SaveHeight=%1") .arg(height);
//  QString f  = QString("-SaveSnapShot=%1") .arg(pngName); // -SnapshotSuffix not required
  QString l  = QString("-LDrawDir=%1").arg(Preferences::ldrawPath);
  QString o  = QString("-HaveStdOut=1");
  QString e  = QString("-EdgeThickness=%1").arg(edgeThickness);
  QString v  = QString("-vv");

  QStringList arguments;
  arguments << CA;
  arguments << cg;
  arguments << w;
  arguments << h;
  arguments << f;
  arguments << l;
  arguments << o;
  arguments << e;
  arguments << v;

  QStringList list;
  list = meta.LPub.pli.ldviewParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
    if (list[i] != "" && list[i] != " ") {
      arguments << list[i];
      //logInfo() << qPrintable("-PARM META: " + list[i]);
    }
  }
  if(hasLDViewIni){
      QString ini  = QString("-IniFile=%1") .arg(Preferences::ldviewIni);
      arguments << ini;
  }
  if (!Preferences::altLDConfigPath.isEmpty()) {
    arguments << "-LDConfig=" + Preferences::altLDConfigPath;
    //logDebug() << qPrintable("-LDConfig=" + Preferences::altLDConfigPath);
  }

#ifndef LDVIEW_USE_SNAPSHOT_LIST
  arguments = arguments + ldrNames;
#else
  arguments << ldrNames.first();
#endif

  emit gui->messageSig(LOG_STATUS, "Executing LDView render PLI - please wait...");

  QProcess    ldview;
  ldview.setEnvironment(QProcess::systemEnvironment());
  ldview.setWorkingDirectory(QDir::currentPath());
  ldview.setStandardErrorFile(QDir::currentPath() + "/stderr-ldview");
  ldview.setStandardOutputFile(QDir::currentPath() + "/stdout-ldview");

  QString message = QString("LDView (Native) PLI Arguments: %1 %2").arg(Preferences::ldviewExe).arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_STATUS, message);
#endif

  ldview.start(Preferences::ldviewExe,arguments);
  if ( ! ldview.waitForFinished()) {
    if (ldview.exitCode() != 0) {
      QByteArray status = ldview.readAll();
      QString str;
      str.append(status);
      emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView PLI render failed with exit code %1\n%2") .arg(ldview.exitCode()) .arg(str));
      return -1;
    }
  }

  // move generated CSI images
  if (useLDViewSCall()){
    QString ldrName;
    QDir dir(QDir::currentPath() + "/" + Paths::tmpDir);
    foreach(ldrName, ldrNames){
        QFileInfo fInfo(ldrName.replace(".ldr",".png"));
        QString imageFilePath = QDir::currentPath() + "/" +
            Paths::partsDir + "/" + fInfo.fileName();
        if (! dir.rename(fInfo.absoluteFilePath(), imageFilePath)){
            //in case failure because file exist
            QFile pngFile(imageFilePath);
            if (! pngFile.exists()){
                emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView PLI image file move failed for\n%1")
                           .arg(imageFilePath));
                return -1;
              }else {
                //file exist so delete and retry
                QFile pngFile(imageFilePath);
                if (pngFile.remove()) {
                    //retry
                    if (! dir.rename(fInfo.absoluteFilePath(), imageFilePath)){
                        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView PLI image file move failed after old file removal for\n%1")
                                             .arg(imageFilePath));
                        return -1;
                      }
                  } else {
                    emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView could not remove old PLI image file \n%1")
                                         .arg(imageFilePath));
                    return -1;
                  }
              }
          }
      }
  }

  return 0;
}

/***************************************************************************
 *
 * Native renderer
 *
 **************************************************************************/

float Native::cameraDistance(
    Meta &meta,
    float scale)
{
  return -stdCameraDistance(meta,scale) / SCALE_FACTOR_NATIVE;
}

int Native::renderCsi(
  const QString     &addLine,
  const QStringList &csiParts,
  const QStringList &csiKeys,
  const QString     &pngName,
        Meta        &meta)
{
  Q_UNUSED(addLine);
  Q_UNUSED(csiParts);
  Q_UNUSED(csiKeys);

  QString ldrName = QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";
  float lineThickness = (float(resolution()/Preferences::highlightStepLineWidth));

  // Renderer options
  NativeOptions Options;
  Options.ImageType         = CSI;
  Options.OutputFileName    = pngName;
  Options.ImageWidth        = gui->pageSize(meta.LPub.page, 0);
  Options.ImageHeight       = gui->pageSize(meta.LPub.page, 1);
  Options.Latitude          = meta.LPub.assem.angle.value(0);
  Options.Longitude         = meta.LPub.assem.angle.value(1);
  Options.HighlightNewParts = gui->suppressColourMeta(); //Preferences::enableHighlightStep;
  Options.CameraDistance    = cameraDistance(meta,meta.LPub.assem.modelScale.value());
  Options.LineWidth         = lineThickness;

  // Set new project
  Project* CsiImageProject = new Project();

  // Load new project
  if (CsiImageProject->Load(ldrName))
  {
    gApplication->SetProject(CsiImageProject);
    gMainWindow->UpdateAllViews();
  }
  else
  {
    emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not create Native CSI image project."));
    delete CsiImageProject;
    return -1;
  }

  // Generate image
  emit gui->messageSig(LOG_STATUS, "Executing Native render CSI - please wait...");

  CreateNativeImage(Options);

  return 0;
}

int Native::renderPli(
  const QStringList &ldrNames,
  const QString     &pngName,
  Meta              &meta,
  bool               bom)
{
  // Select meta type
  PliMeta &metaType = bom ? meta.LPub.bom : meta.LPub.pli;

  // Renderer options
  NativeOptions Options;
  Options.ImageType         = PLI;
  Options.OutputFileName    = pngName;
  Options.ImageWidth        = gui->pageSize(meta.LPub.page, 0);
  Options.ImageHeight       = gui->pageSize(meta.LPub.page, 1);
  Options.Latitude          = metaType.angle.value(0);
  Options.Longitude         = metaType.angle.value(1);
  Options.CameraDistance    = cameraDistance(meta,metaType.modelScale.value());
  Options.LineWidth         = HIGHLIGHT_LINE_WIDTH_DEFAULT;

  // Set and load new project
  Project* PliImageProject = new Project();

  // Load project
  if (PliImageProject->Load(ldrNames.first()))
  {
    gApplication->SetProject(PliImageProject);
    gMainWindow->UpdateAllViews();
  }
  else
  {
    emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not create Native PLI image project."));
    delete PliImageProject;
    return -1;
  }

  // Generate image
  emit gui->messageSig(LOG_STATUS, "Executing Native render PLI - please wait...");

  CreateNativeImage(Options);

  return 0;
}

void Render::CreateNativeImage(const NativeOptions &Options)
{
//        if (Options.LineWidth != HIGHLIGHT_LINE_WIDTH_DEFAULT)
//            gApplication->mPreferences.mLineWidth = Options.LineWidth;

        View* ActiveView = gMainWindow->GetActiveView();
        ActiveView->MakeCurrent();

        lcModel* ActiveModel = ActiveView->GetActiveModel();

        lcStep CurrentStep = ActiveModel->GetCurrentStep();

        lcContext* Context = ActiveView->mContext;

        lcCamera* Camera = gMainWindow->GetActiveView()->mCamera;
        //Camera->SetOrtho(Options.Orthographic);
        Camera->SetAngles(Options.Latitude,Options.Longitude);
        Camera->Zoom(Options.CameraDistance,CurrentStep,true);

        const int ImageWidth = Options.ImageWidth;
        const int ImageHeight = Options.ImageHeight;

        View View(ActiveModel);
        View.SetHighlight(Options.HighlightNewParts);
        View.SetCamera(Camera, false);
        View.SetContext(Context);

        QString imageType = Options.ImageType == CSI ? "CSI" : "PLI";

        if (!View.BeginRenderToImage(ImageWidth, ImageHeight))
        {
                emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not begin RenderToImage for Native %1 image.").arg(imageType));
                return;
        }

        ActiveModel->SetTemporaryStep(CurrentStep);

        View.OnDraw();

        struct NativeImage
        {
                QImage RenderedImage;
                QRect Bounds;
        };

        NativeImage Image;
        Image.RenderedImage = View.GetRenderImage();

        auto CalculateImageBounds = [](NativeImage& Image)
        {
                QImage& RenderedImage = Image.RenderedImage;
                int Width = RenderedImage.width();
                int Height = RenderedImage.height();

                int MinX = Width;
                int MinY = Height;
                int MaxX = 0;
                int MaxY = 0;

                for (int x = 0; x < Width; x++)
                {
                        for (int y = 0; y < Height; y++)
                        {
                                if (qAlpha(RenderedImage.pixel(x, y)))
                                {
                                        MinX = qMin(x, MinX);
                                        MinY = qMin(y, MinY);
                                        MaxX = qMax(x, MaxX);
                                        MaxY = qMax(y, MaxY);
                                }
                        }
                }

                Image.Bounds = QRect(QPoint(MinX, MinY), QPoint(MaxX, MaxY));
        };

        CalculateImageBounds(Image);

        QImageWriter Writer(Options.OutputFileName);

        if (Writer.format().isEmpty())
                Writer.setFormat("PNG");

        if (!Writer.write(QImage(Image.RenderedImage.copy(Image.Bounds))))
        {
                emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not write to Native %1 image file '%2': %3.")
                                     .arg(imageType).arg(Options.OutputFileName).arg(Writer.errorString()));
                return;
        }

        View.EndRenderToImage();
        Context->ClearResources();

        ActiveModel->SetTemporaryStep(CurrentStep);

        if (!ActiveModel->mActive)
                ActiveModel->CalculateStep(LC_STEP_MAX);
}

bool Render::LoadViewer(const ViewerOptions &Options){

    QString viewerCsiName = Options.ViewerCsiName;

    Project* StepProject = new Project();
    if (LoadStepProject(StepProject, viewerCsiName)){
        gApplication->SetProject(StepProject);
        gMainWindow->UpdateAllViews();
    }
    else
    {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not load step project for %1.")
                             .arg(Options.ViewerCsiName));
        delete StepProject;
        return false;
    }

    gui->setViewerCsiName(viewerCsiName);

    View* ActiveView = gMainWindow->GetActiveView();

    ActiveView->SetProjection(Options.Orthographic);

    //ActiveView->SetCameraAngles(Options.Latitude, Options.Longitude);

    return true;
}

bool Render::LoadStepProject(Project* StepProject, const QString& viewerCsiName)
{
        QString FileName = gui->getViewerStepFilePath(viewerCsiName);

        if (FileName.isEmpty())
        {
               emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Did not receive 3DViewer CSI path for %1.").arg(FileName));
               return false;
        }

        QStringList CsiContent = gui->getViewerStepContents(viewerCsiName);
        if (CsiContent.isEmpty())
        {
                emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Did not receive 3DViewer CSI content for %1.").arg(FileName));
                return false;
        }

#ifdef QT_DEBUG_MODE
        QFileInfo outFileInfo(FileName);
        QString outfileName = QString("%1/%2_%3.ldr")
               .arg(outFileInfo.absolutePath())
               .arg(outFileInfo.baseName().replace(".ldr",""))
               .arg(QString(viewerCsiName).replace(";","_"));
        QFile file(outfileName);
        if ( ! file.open(QFile::WriteOnly | QFile::Text)) {
                emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Cannot open 3DViewer file %1 for writing: %2")
                                 .arg(outfileName) .arg(file.errorString()));
        }
        QTextStream out(&file);
        for (int i = 0; i < CsiContent.size(); i++) {
                QString line = CsiContent[i];
                out << line << endl;
        }
        file.close();
#endif

	StepProject->mModels.DeleteAll();
	StepProject->SetFileName(FileName);

	QByteArray QBA;
	foreach(QString line, CsiContent){
	       QBA.append(line);
	       QBA.append(QString("\n"));
	}

	if (StepProject->mFileName.isEmpty())
	{
		emit gui->messageSig(LOG_ERROR,QMessageBox::tr("3DViewer file name not set!"));
		return false;
	}
	QFileInfo FileInfo(StepProject->mFileName);

	QBuffer Buffer(&QBA);
	Buffer.open(QIODevice::ReadOnly);

        while (!Buffer.atEnd())
        {
                lcModel* Model = new lcModel(QString());
                Model->SplitMPD(Buffer);

                if (StepProject->mModels.IsEmpty() || !Model->GetProperties().mName.isEmpty())
                {
                        StepProject->mModels.Add(Model);
                        Model->CreatePieceInfo(StepProject);
                }
                else
                        delete Model;
        }

        Buffer.seek(0);

        for (int ModelIdx = 0; ModelIdx < StepProject->mModels.GetSize(); ModelIdx++)
        {
                lcModel* Model = StepProject->mModels[ModelIdx];
                Model->LoadLDraw(Buffer, StepProject);
                Model->SetSaved();
        }


	if (StepProject->mModels.IsEmpty())
		return false;

	if (StepProject->mModels.GetSize() == 1)
	{
		lcModel* Model = StepProject->mModels[0];

		if (Model->GetProperties().mName.isEmpty())
		{
			Model->SetName(FileInfo.fileName());
			lcGetPiecesLibrary()->RenamePiece(Model->GetPieceInfo(), FileInfo.fileName().toLatin1());
		}
	}

	lcArray<lcModel*> UpdatedModels;
	UpdatedModels.AllocGrow(StepProject->mModels.GetSize());

	for (lcModel* Model : StepProject->mModels)
	{
		Model->UpdateMesh();
		Model->UpdatePieceInfo(UpdatedModels);
	}

	StepProject->mModified = false;

	return true;
}
