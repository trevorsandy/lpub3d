/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
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
#include <QImageReader>
#include <QtConcurrent>

#include "lpub.h"
#include "render.h"
#include "resolution.h"
#include "meta.h"
#include "math.h"
#include "lpub_preferences.h"
#include "application.h"

#include <LDVQt/LDVWidget.h>
#include <LDVQt/LDVImageMatte.h>

#include "paths.h"

#include "lc_file.h"
#include "project.h"
#include "pieceinf.h"
#include "lc_qhtmldialog.h"
#include "view.h"
#include "lc_partselectionwidget.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

Render *renderer;

LDGLite ldglite;
LDView  ldview;
POVRay  povray;
Native  native;

//#define LduDistance 5729.57
//#define _CA "-ca0.01"
#define USE_ALPHA "+UA"

#define SNAPSHOTS_LIST_THRESHOLD 3

static double pi = 4*atan(1.0);

// the default camera distance for real size
static float LduDistance = 10.0/tan(0.005*pi/180);

// renderer timeout in milliseconds
int Render::rendererTimeout(){
    if (Preferences::rendererTimeout == -1)
        return -1;
    else
        return Preferences::rendererTimeout*60*1000;
}

const QString Render::fixupDirname(const QString &dirNameIn) {
#ifdef Q_OS_WIN
    long     length = 0;
    TCHAR*   buffer = nullptr;
//  30/11/2014 Generating "invalid conversion from const ushort to const wchar" compile error:
//  LPCWSTR dirNameWin = dirNameIn.utf16();
    LPCWSTR dirNameWin = reinterpret_cast<LPCWSTR>(dirNameIn.utf16());

// First obtain the size needed by passing nullptr and 0.

    length = GetShortPathName(dirNameWin, nullptr, 0);
    if (length == 0){
                QString message = QString("Couldn't get length of short path name length, lastError is %1, trying long path name").arg(GetLastError());
#ifdef QT_DEBUG_MODE
                qDebug() << message << "\n";
#else
                emit gui->messageSig(LOG_INFO, message);
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
                emit gui->messageSig(LOG_INFO, message);
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

const QString Render::getRotstepMeta(RotStepMeta &rotStep, bool isKey){
  QString rotstepString;
  if (isKey) {
      rotstepString = QString("%1_%2_%3_%4")
                              .arg(qRound(rotStep.value().rots[0]))
                              .arg(qRound(rotStep.value().rots[1]))
                              .arg(qRound(rotStep.value().rots[2]))
                              .arg(rotStep.value().type.isEmpty() ? "REL" :
                                   rotStep.value().type.trimmed());
  } else {
      rotstepString = QString("0 // ROTSTEP %1 %2 %3 %4")
                              .arg(rotStep.value().type.trimmed())
                              .arg(rotStep.value().rots[0])
                              .arg(rotStep.value().rots[1])
                              .arg(rotStep.value().rots[2]);
  }

  //gui->messageSig(LOG_DEBUG,QString("ROTSTEP String: %1").arg(rotstepString));

  return rotstepString;
}

bool Render::useLDViewSCall(){
    return (Preferences::preferredRenderer == RENDERER_LDVIEW &&
            Preferences::enableLDViewSingleCall);
}

bool Render::useLDViewSList(){
    return (Preferences::preferredRenderer == RENDERER_LDVIEW &&
            Preferences::enableLDViewSnaphsotList);
}

bool Render::clipImage(QString const &pngName) {

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
    QImage clippedImage = toClip.copy(clipBox);
    QString clipMsg = QString("%1 (w:%2 x h:%3)")
                              .arg(pngName)
                              .arg(clippedImage.width())
                              .arg(clippedImage.height());

    QImageWriter Writer(QDir::toNativeSeparators(pngName));
    if (Writer.format().isEmpty())
            Writer.setFormat("PNG");

    if (Writer.write(clippedImage)) {
        emit gui->messageSig(LOG_STATUS, QString("Clipped image saved '%1'")
                             .arg(clipMsg));
    } else {
        emit gui->messageSig(LOG_ERROR, QString("Failed to save clipped image '%1': %2")
                             .arg(clipMsg)
                             .arg(Writer.errorString()));
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

//    logDebug() << qPrintable(QString("LduDistance                      : %1").arg(double(LduDistance)));
//    logDebug() << qPrintable(QString("Page Size (width in pixels)      : %1").arg(gui->pageSize(meta.LPub.page, 0)));
//    logDebug() << qPrintable(QString("Resolution Ldu                   : %1").arg(QString::number(double(meta.LPub.resolution.ldu()), 'f' ,10)));
//    logDebug() << qPrintable(QString("Resolution pixel                 : %1").arg(double(meta.LPub.resolution.value())));
//    logDebug() << qPrintable(QString("Scale                            : %1").arg(double(scale)));
//    logDebug() << qPrintable(QString("1x1 [20*res.ldu*res.pix*scale]   : %1").arg(QString::number(double(onexone), 'f' ,10)));
//    logDebug() << qPrintable(QString("Factor [Page size/OnexOne]       : %1").arg(QString::number(double(factor), 'f' ,10)));
//    logDebug() << qPrintable(QString("Cam Distance [Factor*LduDistance]: %1").arg(QString::number(double(factor*LduDistance), 'f' ,10)));

    return factor*LduDistance;
}

float Render::getPovrayRenderCameraDistance(const QString &cdKeys){
    enum {
       K_IMAGEWIDTH = 0,
       K_IMAGEHEIGHT,
       K_MODELSCALE,
       K_RESOLUTION,
       K_RESOLUTIONTYPE
    };

    QStringList cdKey = cdKeys.split(" ");

#ifdef QT_DEBUG_MODE
    QString message = QString("DEBUG STRING IN - ResType: %1, Resolution: %2, Width: %3, Height: %4, Scale: %5")
            .arg(cdKey.at(K_RESOLUTIONTYPE) == "DPI" ? "DPI" : "DPCM")
            .arg(cdKey.at(K_RESOLUTION).toDouble())
            .arg(cdKey.at(K_IMAGEWIDTH).toDouble()).arg(cdKey.at(K_IMAGEHEIGHT).toDouble())
            .arg(cdKey.at(K_MODELSCALE).toDouble());
    emit gui->messageSig(LOG_TRACE, message);
#endif

    float scale = cdKey.at(K_MODELSCALE).toFloat();
    ResolutionType ResType = cdKey.at(K_RESOLUTIONTYPE) == "DPI" ? DPI : DPCM;
    
    Meta meta;
    meta.LPub.resolution.setValue(ResType,cdKey.at(K_RESOLUTION).toFloat());
    meta.LPub.page.size.setValuesPixels(cdKey.at(K_IMAGEWIDTH).toFloat(),cdKey.at(K_IMAGEHEIGHT).toFloat());

#ifdef QT_DEBUG_MODE
    message = QString("DEBUG META OUT - Resolution: %1, Width: %2, Height: %3")
            .arg(double(meta.LPub.resolution.value()))
            .arg(double(meta.LPub.page.size.value(0)))
            .arg(double(meta.LPub.page.size.value(1)));
    emit gui->messageSig(LOG_TRACE, message);
#endif

    // calculate LDView camera distance settings
    return stdCameraDistance(meta,scale);
}

int Render::executeLDViewProcess(QStringList &arguments, Mt module) {

  QString message = QString("LDView %1 %2 Arguments: %3 %4")
                            .arg(useLDViewSCall() ? "(SingleCall)" : "(Native)")
                            .arg(module == CSI ? "CSI" : "PLI")
                            .arg(Preferences::ldviewExe)
                            .arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_INFO, message);
#endif

  QProcess ldview;
  ldview.setEnvironment(QProcess::systemEnvironment());
  ldview.setWorkingDirectory(QDir::currentPath() + "/" + Paths::tmpDir);
  ldview.setStandardErrorFile(QDir::currentPath() + "/stderr-ldview");
  ldview.setStandardOutputFile(QDir::currentPath() + "/stdout-ldview");

  ldview.start(Preferences::ldviewExe,arguments);
  if ( ! ldview.waitForFinished(rendererTimeout())) {
      if (ldview.exitCode() != 0 || 1) {
          QByteArray status = ldview.readAll();
          QString str;
          str.append(status);
          emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView %1 %2 render failed with code %2 %3")
                               .arg(useLDViewSCall() ? "(SingleCall)" : "(Native)")
                               .arg(module == CSI ? "CSI" : "PLI")
                               .arg(ldview.exitCode())
                               .arg(str));
          return -1;
        }
    }
  return 0;
}

/***************************************************************************
 *
 * The math for zoom factor.  1.0 is true size.
 *
 * 1 LDU is 1/64 of an inch
 *
 * LDGLite produces 72 DPI
 *
 * Camera default FoV angle is 0.01
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

  Q_UNUSED(csiKeys)

  /* Create the CSI DAT file */
  QString ldrName = QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";
  QString povName = ldrName + ".pov";
  QStringList list;
  QString message;

  int rc;
  if ((rc = rotateParts(addLine, meta.rotStep, csiParts, ldrName, QString(),meta.LPub.assem.cameraAngles)) < 0) {
      return rc;
   }

  /* determine camera distance */
  int cd = cameraDistance(meta,meta.LPub.assem.modelScale.value())*1700/1000;

  /* apply camera angle */
  bool noCA  = Preferences::applyCALocally;
  bool pp    = Preferences::perspectiveProjection;

  int width  = gui->pageSize(meta.LPub.page, 0);
  int height = gui->pageSize(meta.LPub.page, 1);

  QString CA = QString("-ca%1") .arg(pp ? meta.LPub.assem.cameraFoV.value() : 0.01);
  QString cg = QString("-cg%1,%2,%3")
      .arg(noCA ? 0.0 : meta.LPub.assem.cameraAngles.value(0))
      .arg(noCA ? 0.0 : meta.LPub.assem.cameraAngles.value(1))
      .arg(cd);

  QString w  = QString("-SaveWidth=%1") .arg(width);
  QString h  = QString("-SaveHeight=%1") .arg(height);
  QString f  = QString("-ExportFile=%1") .arg(povName);
  QString l  = QString("-LDrawDir=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldrawLibPath)));
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

      arguments << QDir::toNativeSeparators(ldrName);

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
      emit gui->messageSig(LOG_INFO, message);
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

      arguments << QDir::toNativeSeparators(ldrName);

      emit gui->messageSig(LOG_STATUS, "Native POV CSI file generation...");

      bool retError = false;
      ldvWidget = new LDVWidget(nullptr,NativePOVIni,true);
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
  povArguments << getPovrayRenderQuality();

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
  povEnv.prepend("POV_IGNORE_SYSCONF_MSG=1");
  povray.setEnvironment(povEnv);
  povray.setWorkingDirectory(QDir::currentPath()+ "/" + Paths::assemDir); // pov win console app will not write to dir different from cwd or source file dir
  povray.setStandardErrorFile(QDir::currentPath() + "/stderr-povray");
  povray.setStandardOutputFile(QDir::currentPath() + "/stdout-povray");

  message = QString("POVRay CSI Arguments: %1 %2").arg(Preferences::povrayExe).arg(povArguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_INFO, message);
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

  if (clipImage(pngName))
    return 0;
  else
    return -1;
}

int POVRay::renderPli(
    const QStringList &ldrNames ,
    const QString     &pngName,
    Meta    	      &meta,
    int               pliType)
{
  // Select meta type
  PliMeta &metaType = pliType == SUBMODEL ? static_cast<PliMeta&>(meta.LPub.subModel) :
                      pliType == BOM ? meta.LPub.bom : meta.LPub.pli;

  QStringList list;
  QString message;
  QString povName = ldrNames.first() +".pov";

  int width  = gui->pageSize(meta.LPub.page, 0);
  int height = gui->pageSize(meta.LPub.page, 1);

  /* determine camera distance */
  int cd = cameraDistance(meta,metaType.modelScale.value())*1700/1000;

  bool noCA  = metaType.rotStep.value().type == "ABS";
  if (pliType == SUBMODEL)
      noCA   = Preferences::applyCALocally || noCA;

  QString CA = QString("-ca%1") .arg(metaType.cameraFoV.value());
  QString cg = QString("-cg%1,%2,%3")
      .arg(noCA ? 0.0 : metaType.cameraAngles.value(0))
      .arg(noCA ? 0.0 : metaType.cameraAngles.value(1))
      .arg(cd);

  QString w  = QString("-SaveWidth=%1")  .arg(width);
  QString h  = QString("-SaveHeight=%1") .arg(height);
  QString f  = QString("-ExportFile=%1") .arg(povName);  // -ExportSuffix not required
  QString l  = QString("-LDrawDir=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldrawLibPath)));
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

      list = metaType.ldviewParms.value().split(' ');
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

      arguments << QDir::toNativeSeparators(ldrNames.first());

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
      emit gui->messageSig(LOG_INFO, message);
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

      arguments << QDir::toNativeSeparators(ldrNames.first());

      emit gui->messageSig(LOG_STATUS, "Native POV PLI file generation...");

      bool retError = false;
      ldvWidget = new LDVWidget(nullptr,NativePOVIni,true);
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
  povArguments << getPovrayRenderQuality();

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
  povEnv.prepend("POV_IGNORE_SYSCONF_MSG=1");
  QString workingDirectory = pliType == SUBMODEL ? Paths::submodelDir : Paths::partsDir;
  povray.setEnvironment(povEnv);
  povray.setWorkingDirectory(QDir::currentPath()+ "/" + workingDirectory); // pov win console app will not write to dir different from cwd or source file dir
  povray.setStandardErrorFile(QDir::currentPath() + "/stderr-povray");
  povray.setStandardOutputFile(QDir::currentPath() + "/stdout-povray");

  message = QString("POVRay PLI Arguments: %1 %2").arg(Preferences::povrayExe).arg(povArguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_INFO, message);
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
  if ((rc = rotateParts(addLine, meta.rotStep, csiParts, ldrFile,QString(),meta.LPub.assem.cameraAngles)) < 0) {
     return rc;
  }

  /* determine camera distance */

  int cd = cameraDistance(meta,meta.LPub.assem.modelScale.value());

  /* apply camera angle */

  bool noCA  = Preferences::applyCALocally;
  bool pp    = Preferences::perspectiveProjection;

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

  QString CA = QString("-ca%1") .arg(pp ? meta.LPub.assem.cameraFoV.value() : 0.01);
  QString cg = QString("-cg%1,%2,%3") .arg(noCA ? 0.0 : meta.LPub.assem.cameraAngles.value(0))
                                      .arg(noCA ? 0.0 : meta.LPub.assem.cameraAngles.value(1))
                                      .arg(cd);

  QString J  = QString("-%1").arg(pp ? "J" : "j");

  QStringList arguments;
  arguments << CA;                  // camera FOV in degrees
  arguments << cg;                  // camera globe - scale factor
  arguments << J;                   // projection
  arguments << v;                   // display in X wide by Y high window
  arguments << o;                   // changes the centre X across and Y down
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

  // add custom color file if exist
  if (!Preferences::altLDConfigPath.isEmpty()) {
    arguments << "-ldcF" + Preferences::altLDConfigPath;
    //logDebug() << qPrintable("=" + Preferences::altLDConfigPath);
  }

  arguments << QDir::toNativeSeparators(mf);                  // .png file name
  arguments << QDir::toNativeSeparators(ldrFile);             // csi.ldr (input file)

  emit gui->messageSig(LOG_STATUS, "Executing LDGLite render CSI - please wait...");

  QProcess    ldglite;
  QStringList env = QProcess::systemEnvironment();
  env << "LDRAWDIR=" + Preferences::ldrawLibPath;
  //emit gui->messageSig(LOG_DEBUG,qPrintable("LDRAWDIR=" + Preferences::ldrawLibPath));

  if (!Preferences::ldgliteSearchDirs.isEmpty()) {
    env << "LDSEARCHDIRS=" + Preferences::ldgliteSearchDirs;
    //emit gui->messageSig(LOG_DEBUG,qPrintable("LDSEARCHDIRS: " + Preferences::ldgliteSearchDirs));
  }

  ldglite.setEnvironment(env);
  //emit gui->messageSig(LOG_DEBUG,qPrintable("ENV: " + env.join(" ")));

  ldglite.setWorkingDirectory(QDir::currentPath() + "/" + Paths::tmpDir);
  ldglite.setStandardErrorFile(QDir::currentPath() + "/stderr-ldglite");
  ldglite.setStandardOutputFile(QDir::currentPath() + "/stdout-ldglite");

  QString message = QString("LDGLite CSI Arguments: %1 %2").arg(Preferences::ldgliteExe).arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_INFO, message);
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

  return 0;
}


int LDGLite::renderPli(
  const QStringList &ldrNames,
  const QString     &pngName,
  Meta              &meta,
  int                pliType)
{
  // Select meta type
  PliMeta &metaType = pliType == SUBMODEL ? static_cast<PliMeta&>(meta.LPub.subModel) :
                      pliType == BOM ? meta.LPub.bom : meta.LPub.pli;

  int width  = gui->pageSize(meta.LPub.page, 0);
  int height = gui->pageSize(meta.LPub.page, 1);

  int lineThickness = resolution()/72.0+0.5;

  /* determine camera distance */
  int cd = cameraDistance(meta,metaType.modelScale.value());

  bool noCA  = metaType.rotStep.value().type == "ABS";
  if (pliType == SUBMODEL)
      noCA   = Preferences::applyCALocally || noCA;

  QString CA = QString("-ca%1") .arg(metaType.cameraFoV.value());
  QString cg = QString("-cg%1,%2,%3") .arg(noCA ? 0.0 : metaType.cameraAngles.value(0))
                                      .arg(noCA ? 0.0 : metaType.cameraAngles.value(1))
                                      .arg(cd);

  QString J  = QString("-J");

  QString v  = QString("-v%1,%2")   .arg(width)
                                    .arg(height);
  QString o  = QString("-o0,-%1")   .arg(height/6);
  QString mf = QString("-mF%1")     .arg(pngName);

  QString w  = QString("-W%1")      .arg(lineThickness);  // ldglite always deals in 72 DPI

  QStringList arguments;
  arguments << CA;                  // Camera FOV in degrees
  arguments << cg;                  // camera globe - scale factor
  arguments << J;                   // Perspective projection
  arguments << v;                   // display in X wide by Y high window
  arguments << o;                   // changes the centre X across and Y down
  arguments << w;                   // line thickness

  QStringList list;
  // First, load parms from meta
  list = metaType.ldgliteParms.value().split(' ');
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

  // add custom color file if exist
  if (!Preferences::altLDConfigPath.isEmpty()) {
    arguments << "-ldcF" + Preferences::altLDConfigPath;
    //logDebug() << qPrintable("=" + Preferences::altLDConfigPath);
  }

  arguments << QDir::toNativeSeparators(mf);
  arguments << QDir::toNativeSeparators(ldrNames.first());

  emit gui->messageSig(LOG_STATUS, "Executing LDGLite render PLI - please wait...");

  QProcess    ldglite;
  QStringList env = QProcess::systemEnvironment();
  env << "LDRAWDIR=" + Preferences::ldrawLibPath;
  //emit gui->messageSig(LOG_DEBUG,qPrintable("LDRAWDIR=" + Preferences::ldrawLibPath));

  if (!Preferences::ldgliteSearchDirs.isEmpty()){
    env << "LDSEARCHDIRS=" + Preferences::ldgliteSearchDirs;
    //emit gui->messageSig(LOG_DEBUG,qPrintable("LDSEARCHDIRS: " + Preferences::ldgliteSearchDirs));
  }

  ldglite.setEnvironment(env);
  ldglite.setWorkingDirectory(QDir::currentPath());
  ldglite.setStandardErrorFile(QDir::currentPath() + "/stderr-ldglite");
  ldglite.setStandardOutputFile(QDir::currentPath() + "/stdout-ldglite");

  QString message = QString("LDGLite PLI Arguments: %1 %2").arg(Preferences::ldgliteExe).arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_INFO, message);
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
        const QStringList &csiParts,   // this is ldrNames when useLDViewSCall = true
        const QStringList &csiKeys,
        const QString     &pngName,
        Meta              &meta)
{
    /* determine camera distance */
    int cd = cameraDistance(meta,meta.LPub.assem.modelScale.value())*1700/1000;

    /* apply camera angle */
    bool noCA  = Preferences::applyCALocally;
    bool pp    = Preferences::perspectiveProjection;

    /* page size */
    int width  = gui->pageSize(meta.LPub.page, 0);
    int height = gui->pageSize(meta.LPub.page, 1);

    bool hasLDViewIni = Preferences::ldviewIni != "";

    // initialize ImageMatte flag
    bool enableIM = false;

    QString tempPath = QDir::currentPath() + "/" + Paths::tmpDir;
    QString assemPath = QDir::currentPath() + "/" + Paths::assemDir;

    /* Create the CSI DAT file(s) */
    QString f;
    QStringList ldrNames = QStringList(), ldrNamesIM = QStringList();
    if (useLDViewSCall()) {

        if (Preferences::enableFadeSteps && Preferences::enableImageMatting){  // ldrName entries (IM ON)

            enableIM = true;
            foreach(QString csiEntry, csiParts){              // csiParts are ldrNames under LDViewSingleCall
                QString csiFile = QString("%1/%2").arg(assemPath).arg(QFileInfo(QString(csiEntry).replace(".ldr",".png")).fileName());
                if (LDVImageMatte::validMatteCSIImage(csiFile)) {
                    ldrNamesIM << csiEntry;                   // ldrName entries that ARE IM
                } else {
                    ldrNames << csiEntry;                     // ldrName entries that ARE NOT IM
                }
            }

        } else {                                              // ldrName entries (IM off)

            ldrNames = csiParts;
        }

        if (!useLDViewSList() || (useLDViewSList() && ldrNames.size() < SNAPSHOTS_LIST_THRESHOLD)) {

            f  = QString("-SaveSnapShots=1");

        } else {                                              // LDView SnapshotsList

            QString SnapshotsList = tempPath + "/csiSnapshotsList.lst";
            QFile SnapshotsListFile(SnapshotsList);
            if ( ! SnapshotsListFile.open(QFile::Append | QFile::Text)) {
                emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Failed to create LDView (Single Call) CSI Snapshots list file!"));
                return -1;
            }

            QTextStream out(&SnapshotsListFile);

            for (int i = 0; i < ldrNames.size(); i++) {
                QString smLine = ldrNames[i];
                out << smLine << endl;                          // ldrNames that ARE NOT IM
                emit gui->messageSig(LOG_INFO, QString("Wrote %1 to CSI Snapshots list").arg(smLine));
            }
            SnapshotsListFile.close();

            f  = QString("-SaveSnapshotsList=%1").arg(SnapshotsList);     // run in renderCsi
        }

    } else {                                                    // LDView (Native)

        int rc;
        QString csiKey = QString();
        if (Preferences::enableFadeSteps && Preferences::enableImageMatting &&
                LDVImageMatte::validMatteCSIImage(csiKeys.first())) {                    // ldrName entries (IM ON)
            enableIM = true;
            csiKey = csiKeys.first();
        }

        ldrNames << tempPath + "/csi.ldr";

        if ((rc = rotateParts(addLine, meta.rotStep, csiParts,ldrNames.first(), csiKey, meta.LPub.assem.cameraAngles)) < 0) {
            emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView (Single Call) CSI rotate parts failed!"));
            return rc;
        } else
          // recheck csiKey - may have been deleted by rotateParts if IM files not created.
          if (enableIM) {
            enableIM = LDVImageMatte::validMatteCSIImage(csiKeys.first());
        }

        f  = QString("-SaveSnapShot=%1") .arg(pngName);     // applied for ldrName entry that IS NOT IM
    }

  bool haveLdrNames   = !ldrNames.isEmpty();
  bool haveLdrNamesIM = !ldrNamesIM.isEmpty();

  // Build (Native) arguments
  QString CA = QString("-ca%1") .arg(pp ? meta.LPub.assem.cameraFoV.value() : 0.01);
  QString cg = QString("-cg%1,%2,%3") .arg(noCA ? 0.0f : meta.LPub.assem.cameraAngles.value(0))
                                      .arg(noCA ? 0.0f : meta.LPub.assem.cameraAngles.value(1))
                                      .arg(cd);

  QString w  = QString("-SaveWidth=%1")  .arg(width);
  QString h  = QString("-SaveHeight=%1") .arg(height);
  QString l  = QString("-LDrawDir=%1")   .arg(Preferences::ldrawLibPath);
  QString o  = QString("-HaveStdOut=1");
  QString v  = QString("-vv");

  QStringList arguments;
  arguments << CA;                        // 00. Camera FOV in degrees
  arguments << cg;                        // 01. Camera globe
  arguments << w;                         // 03. SaveWidth
  arguments << h;                         // 04. SaveHeight
  arguments << f;                         // 05. SaveSnapshot/SaveSnapshots/SaveSnapshotsList
  arguments << l;                         // 06. LDrawDir
  arguments << o;                         // 07. HaveStdOut
  arguments << v;                         // 09. Verbose

//  QString a  = QString("-AutoCrop=1");
//  if (!enableIM)
//    arguments.insert(2,a);                // 02. AutoCrop On if IM Off

  QStringList ldviewParmslist;
  ldviewParmslist = meta.LPub.assem.ldviewParms.value().split(' ');
  for (int i = 0; i < ldviewParmslist.size(); i++) {
    if (ldviewParmslist[i] != "" && ldviewParmslist[i] != " ") {
      arguments << ldviewParmslist[i];    // 10. ldviewParms [usually empty]
    }
  }

  QString ini;
  if(hasLDViewIni){
      ini  = QString("-IniFile=%1") .arg(Preferences::ldviewIni);
      arguments << ini;                  // 11. LDView.ini
  }

  QString altldc;
  if (!Preferences::altLDConfigPath.isEmpty()) {
      altldc = QString("-LDConfig=%1").arg(Preferences::altLDConfigPath);
      arguments << altldc;               // 12.Alternate LDConfig
  }

  if (haveLdrNames) {      
      if (useLDViewSCall()) {
          //-SaveSnapShots=1
          if ((!useLDViewSList()) || (useLDViewSList() && ldrNames.size() < SNAPSHOTS_LIST_THRESHOLD))
              arguments = arguments + ldrNames;  // 13. LDR input file(s)
      } else {
          // SaveSnapShot=1
          arguments << ldrNames.first();
      }

      emit gui->messageSig(LOG_STATUS, "Executing LDView render CSI - please wait...");

      // execute LDView process
      if (executeLDViewProcess(arguments, CSI) != 0) // ldrName entries that ARE NOT IM exist - e.g. first step
          return -1;
  }

  // Build IM arguments and process IM [Not implemented]
  QStringList im_arguments;
  if (enableIM && haveLdrNamesIM) {
      QString a  = QString("-AutoCrop=0");
      im_arguments << CA;                         // 00. Camera FOV in degrees
      im_arguments << cg;                         // 01. Camera globe
      im_arguments << a;                          // 02. AutoCrop off - to create same size IM pair files
      im_arguments << w;                          // 03. SaveWidth
      im_arguments << h;                          // 04. SaveHeight
      im_arguments << f;                          // 05. SaveSnapshot/SaveSnapshots/SaveSnapshotsList
      im_arguments << l;                          // 06. LDrawDir
      im_arguments << o;                          // 07. HaveStdOut
      im_arguments << v;                          // 09. Verbose
      for (int i = 0; i < ldviewParmslist.size(); i++) {
          if (ldviewParmslist[i] != "" &&
                  ldviewParmslist[i] != " ") {
              im_arguments << ldviewParmslist[i]; // 10. ldviewParms [usually empty]
          }
      }
      im_arguments << ini;                        // 11. LDView.ini
      if (!altldc.isEmpty())
          im_arguments << altldc;                 // 12.Alternate LDConfig

      if (useLDViewSCall()){

          if (enableIM) {
              if (haveLdrNamesIM) {
                  // IM each ldrNameIM file
                  emit gui->messageSig(LOG_STATUS, "Executing LDView render Image Matte CSI - please wait...");

                  foreach(QString ldrNameIM, ldrNamesIM){
                      QFileInfo pngFileInfo(QString("%1/%2").arg(assemPath).arg(QFileInfo(QString(ldrNameIM).replace(".ldr",".png")).fileName()));
                      QString csiKey = LDVImageMatte::getMatteCSIImage(pngFileInfo.absoluteFilePath());
                      if (!csiKey.isEmpty()) {
                          if (!LDVImageMatte::matteCSIImage(im_arguments, csiKey))
                              return -1;
                      }
                  }
              }
          }

      } else {

          // image matte - LDView Native csiKeys.first()
          if (enableIM) {
              QString csiFile = LDVImageMatte::getMatteCSIImage(csiKeys.first());
              if (!csiFile.isEmpty())
                  if (!LDVImageMatte::matteCSIImage(im_arguments, csiFile))
                      return -1;
          }
      }
  }

  // move generated CSI images to assem subfolder
  if (useLDViewSCall()){
      foreach(QString ldrName, ldrNames){
          QString pngFileTmpPath = ldrName.replace(".ldr",".png");
          QString pngFilePath = QString("%1/%2").arg(assemPath).arg(QFileInfo(pngFileTmpPath).fileName());
          QFile destinationFile(pngFilePath);
          QFile sourceFile(pngFileTmpPath);
          if (! destinationFile.exists() || destinationFile.remove()) {
              if (! sourceFile.rename(destinationFile.fileName()))
                  emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView CSI image move failed for %1").arg(pngFilePath));
          } else {
              emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView could not remove old CSI image file %1").arg(pngFilePath));
          }
      }
  }

  return 0;
}

int LDView::renderPli(
  const QStringList &ldrNames,
  const QString     &pngName,
  Meta              &meta,
  int                pliType)
{
  // Select meta type
  PliMeta &metaType = pliType == SUBMODEL ? static_cast<PliMeta&>(meta.LPub.subModel) :
                      pliType == BOM ? meta.LPub.bom : meta.LPub.pli;

  QFileInfo fileInfo(ldrNames.first());
  if ( ! fileInfo.exists()) {
    emit gui->messageSig(LOG_ERROR,QMessageBox::tr("File ldrNames does not exist!"));
    return -1;
  }

  /* determine camera distance */
  int cd = cameraDistance(meta,metaType.modelScale.value())*1700/1000;

  /* page size */
  int width  = gui->pageSize(meta.LPub.page, 0);
  int height = gui->pageSize(meta.LPub.page, 1);

  bool hasLDViewIni = Preferences::ldviewIni != "";

  QString tempPath = QDir::currentPath() + "/" + Paths::tmpDir;
  QString partsPath = QDir::currentPath() + "/" + (pliType == SUBMODEL ? Paths::submodelDir : Paths::partsDir);

  //qDebug() << "LDView (Native) Camera Distance: " << cd;

  /* Create the CSI DAT file(s) */
  QString f;
  if (useLDViewSCall() && pliType != SUBMODEL) {

      if (!useLDViewSList() || (useLDViewSList() && ldrNames.size() < SNAPSHOTS_LIST_THRESHOLD)) {
          f  = QString("-SaveSnapShots=1");
      } else {
          QString SnapshotsList = tempPath + "/pliSnapshotsList.lst";
          QFile SnapshotsListFile(SnapshotsList);
          if ( ! SnapshotsListFile.open(QFile::Append | QFile::Text)) {
              emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Failed to create LDView (Single Call) PLI Snapshots list file!"));
              return -1;
          }

          QTextStream out(&SnapshotsListFile);

          for (int i = 0; i < ldrNames.size(); i++) {
              QString smLine = ldrNames[i];
              if (QFileInfo(smLine).exists()) {
                  out << smLine << endl;
                  emit gui->messageSig(LOG_INFO, QString("Wrote %1 to PLI Snapshots list").arg(smLine));
              } else {
                  emit gui->messageSig(LOG_ERROR, QString("Error %1 not written to Snapshots list - file does not exist").arg(smLine));
              }
          }
          SnapshotsListFile.close();

          f  = QString("-SaveSnapshotsList=%1").arg(SnapshotsList);    // run in renderCsi
      }

  } else {

      f  = QString("-SaveSnapShot=%1") .arg(pngName);
  }

  bool noCA  = metaType.rotStep.value().type == "ABS";
  if (pliType == SUBMODEL)
      noCA   = Preferences::applyCALocally || noCA;

  QString CA = QString("-ca%1") .arg(metaType.cameraFoV.value());
  QString cg = QString("-cg%1,%2,%3") .arg(noCA ? 0.0 : metaType.cameraAngles.value(0))
                                      .arg(noCA ? 0.0 : metaType.cameraAngles.value(1))
                                      .arg(cd);

  QString w  = QString("-SaveWidth=%1")  .arg(width);
  QString h  = QString("-SaveHeight=%1") .arg(height);
  QString l  = QString("-LDrawDir=%1")   .arg(Preferences::ldrawLibPath);
  QString o  = QString("-HaveStdOut=1");
  QString v  = QString("-vv");

  QStringList arguments;
  arguments << CA;
  arguments << cg;
  arguments << w;
  arguments << h;
  arguments << f;
  arguments << l;
  arguments << o;
  arguments << v;

  QStringList ldviewParmslist;
  ldviewParmslist = metaType.ldviewParms.value().split(' ');
  for (int i = 0; i < ldviewParmslist.size(); i++) {
    if (ldviewParmslist[i] != "" && ldviewParmslist[i] != " ") {
      arguments << ldviewParmslist[i];    // 10. ldviewParms [usually empty]
    }
  }

  QString ini;
  if(hasLDViewIni){
      ini  = QString("-IniFile=%1") .arg(Preferences::ldviewIni);
      arguments << ini;
  }

  QString altldc;
  if (!Preferences::altLDConfigPath.isEmpty()) {
      altldc = QString("-LDConfig=%1").arg(Preferences::altLDConfigPath);
      arguments << altldc;
  }

  if (useLDViewSCall() && pliType != SUBMODEL) {
      //-SaveSnapShots=1
      if ((!useLDViewSList()) || (useLDViewSList() && ldrNames.size() < SNAPSHOTS_LIST_THRESHOLD))
          arguments = arguments + ldrNames;  // 13. LDR input file(s)
  } else {
      // SaveSnapShot=1
      arguments << ldrNames.first();
  }

  emit gui->messageSig(LOG_STATUS, "Executing LDView render PLI - please wait...");

  // execute LDView process
  if (executeLDViewProcess(arguments, PLI) != 0)
      return -1;

  // move generated PLI images to parts subfolder
  if (useLDViewSCall() && pliType != SUBMODEL){
      foreach(QString ldrName, ldrNames){
          QString pngFileTmpPath = ldrName.replace(".ldr",".png");
          QString pngFilePath = partsPath + "/" + QFileInfo(pngFileTmpPath).fileName();
          QFile destinationFile(pngFilePath);
          QFile sourceFile(pngFileTmpPath);
          if (! destinationFile.exists() || destinationFile.remove()) {
              if (! sourceFile.rename(destinationFile.fileName()))
                  emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView PLI image move failed for %1").arg(pngFilePath));
          } else {
              emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView could not remove old PLI image file %1").arg(pngFilePath));
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
  Q_UNUSED(scale);

#ifdef IS_SUBMODEL
  return meta.LPub.subModel.cameraDistNative.factor.value();
#else
  return meta.LPub.assem.cameraDistNative.factor.value();
#endif
}

int Native::renderCsi(
  const QString     &addLine,
  const QStringList &csiParts,
  const QStringList &csiKeys,
  const QString     &pngName,
        Meta        &meta)
{
  QString ldrName     = QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";
  float lineThickness = (float(resolution()/Preferences::highlightStepLineWidth));

  // Camera Angles always applied by Native renderer except if ABS rotstep
  bool noCA = meta.rotStep.value().type == "ABS";

  // Renderer options
  NativeOptions Options;
  Options.ImageType         = CSI;
  Options.InputFileName     = ldrName;
  Options.OutputFileName    = pngName;
  Options.ImageWidth        = gui->pageSize(meta.LPub.page, 0);
  Options.ImageHeight       = gui->pageSize(meta.LPub.page, 1);
  Options.FoV               = meta.LPub.assem.cameraFoV.value();
  Options.ZNear             = meta.LPub.assem.znear.value();
  Options.ZFar              = meta.LPub.assem.zfar.value();
  Options.Latitude          = noCA ? 0.0 : meta.LPub.assem.cameraAngles.value(0);
  Options.Longitude         = noCA ? 0.0 : meta.LPub.assem.cameraAngles.value(1);
  Options.HighlightNewParts = gui->suppressColourMeta(); //Preferences::enableHighlightStep;
  Options.CameraDistance    = cameraDistance(meta,meta.LPub.assem.modelScale.value());
  Options.LineWidth         = lineThickness;
  Options.UsingViewpoint    = gApplication->mPreferences.mNativeViewpoint <= 6;

  // Set CSI project
  Project* CsiImageProject = new Project();
  gApplication->SetProject(CsiImageProject);

  // Render image
  emit gui->messageSig(LOG_STATUS, "Rendering Native CSI image - please wait...");

  if (gui->exportingObjects()) {
      if (csiKeys.size()) {
          emit gui->messageSig(LOG_STATUS, "Rendering CSI Objects - please wait...");
          QString baseName = csiKeys.first();
          QString outPath  = gui->saveDirectoryName;
          bool ldvExport   = true;

          switch (gui->exportMode){
          case EXPORT_3DS_MAX:
              Options.ExportMode = int(EXPORT_3DS_MAX);
              Options.ExportFileName = QDir::toNativeSeparators(outPath+"/"+baseName+".3ds");
              break;
          case EXPORT_STL:
              Options.ExportMode = int(EXPORT_STL);
              Options.ExportFileName = QDir::toNativeSeparators(outPath+"/"+baseName+".stl");
              break;
          case EXPORT_POVRAY:
              Options.ExportMode = int(EXPORT_POVRAY);
              Options.ExportFileName = QDir::toNativeSeparators(outPath+"/"+baseName+".pov");
              break;
          case EXPORT_COLLADA:
              Options.ExportMode = int(EXPORT_COLLADA);
              Options.ExportFileName = QDir::toNativeSeparators(outPath+"/"+baseName+".dae");
              ldvExport = false;
              break;
          case EXPORT_WAVEFRONT:
              Options.ExportMode = int(EXPORT_WAVEFRONT);
              Options.ExportFileName = QDir::toNativeSeparators(outPath+"/"+baseName+".obj");
              ldvExport = false;
              break;
          default:
              emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Invalid CSI Object export option."));
              delete CsiImageProject;
              return -1;
          }
          // These exports are performed by the Native LDV module (LDView).
          if (ldvExport) {
              if (gui->exportMode == EXPORT_POVRAY ||
                      gui->exportMode == EXPORT_STL ||
                      gui->exportMode == EXPORT_HTML ||
                      gui->exportMode == EXPORT_3DS_MAX) {
                  Options.IniFlag = gui->exportMode == EXPORT_POVRAY ? NativePOVIni :
                                    gui->exportMode == EXPORT_STL ? NativeSTLIni : Native3DSIni;
                  /*  Options.IniFlag = gui->exportMode == EXPORT_POVRAY ? NativePOVIni :
                                    gui->exportMode == EXPORT_STL ? NativeSTLIni : EXPORT_HTML; */
              }

              ldrName = QDir::currentPath() + "/" + Paths::tmpDir + "/exportcsi.ldr";

              // rotate parts for ldvExport - apply camera angles
              int rc;
              if ((rc = rotateParts(addLine, meta.rotStep, csiParts, ldrName, QString(),meta.LPub.assem.cameraAngles,ldvExport)) < 0) {
                  return rc;
              }

              /* determine camera distance */
              int cd = int((stdCameraDistance(meta,meta.LPub.assem.modelScale.value())*0.775)*1700/1000);

              /* apply camera angles */
              noCA  = Preferences::applyCALocally || noCA;
              //bool pp      = Preferences::perspectiveProjection;

              QString CA = QString("-ca%1") .arg(/*pp ? Options.FoV : */ 0.01);  // Effectively defaults to orthographic projection.
              QString cg = QString("-cg%1,%2,%3")
                      .arg(noCA ? 0.0 : meta.LPub.assem.cameraAngles.value(0))
                      .arg(noCA ? 0.0 : meta.LPub.assem.cameraAngles.value(1))
                      .arg(cd);

              QString w  = QString("-SaveWidth=%1") .arg(Options.ImageWidth);
              QString h  = QString("-SaveHeight=%1") .arg(Options.ImageHeight);
              QString f  = QString("-ExportFile=%1") .arg(Options.ExportFileName);
              QString l  = QString("-LDrawDir=%1") .arg(QDir::toNativeSeparators(Preferences::ldrawLibPath));
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
              }

              arguments << QDir::toNativeSeparators(ldrName);

              Options.ExportArgs = arguments;
          }
      } else {
          Options.ExportMode = EXPORT_NONE;
      }
  }

  if (!RenderNativeImage(Options)) {
      emit gui->messageSig(LOG_ERROR,QString("Native %1 render failed.")
                          .arg(Options.ExportMode == EXPORT_NONE ? QString("CSI image") :
                               QString("CSI %1 object").arg(nativeExportNames[gui->exportMode])));
      // delete CsiImageProject;
      return -1;
  }

  return 0;
}

int Native::renderPli(
  const QStringList &ldrNames,
  const QString     &pngName,
  Meta              &meta,
  int               pliType)
{
  // Select meta type
  PliMeta &metaType = pliType == SUBMODEL ? static_cast<PliMeta&>(meta.LPub.subModel) :
                      pliType == BOM ? meta.LPub.bom : meta.LPub.pli;

  // Camera Angles always applied by Native renderer except if ABS rotstep
  bool noCA  = metaType.rotStep.value().type == "ABS";

  // Renderer options
  NativeOptions Options;
  Options.ImageType         = PLI;
  Options.InputFileName     = ldrNames.first();
  Options.OutputFileName    = pngName;
  Options.ImageWidth        = gui->pageSize(meta.LPub.page, 0);
  Options.ImageHeight       = gui->pageSize(meta.LPub.page, 1);
  Options.FoV               = metaType.cameraFoV.value();
  Options.ZNear             = metaType.znear.value();
  Options.ZFar              = metaType.zfar.value();
  Options.Latitude          = noCA ? 0.0 : metaType.cameraAngles.value(0);
  Options.Longitude         = noCA ? 0.0 : metaType.cameraAngles.value(1);
  Options.CameraDistance    = cameraDistance(meta,metaType.modelScale.value());
  Options.LineWidth         = HIGHLIGHT_LINE_WIDTH_DEFAULT;
  Options.UsingViewpoint    = gApplication->mPreferences.mNativeViewpoint <= 6;

  // Set PLI project
  Project* PliImageProject = new Project();
  gApplication->SetProject(PliImageProject);

  // Render image
  emit gui->messageSig(LOG_STATUS, "Rendering Native PLI image - please wait...");

  if (!RenderNativeImage(Options)) {
      emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Native PLI image render failed."));
      delete PliImageProject;
      return -1;
  }

  return 0;
}

bool Render::RenderNativeImage(const NativeOptions &Options)
{
    // Load model
    if (! gMainWindow->OpenProject(Options.InputFileName))
        return false;

    QString ImageType = Options.ImageType == CSI ? "CSI" : "PLI";

    View* ActiveView = gMainWindow->GetActiveView();

    lcModel* ActiveModel = ActiveView->GetActiveModel();

    lcCamera* Camera =  ActiveView->mCamera;

    lcStep CurrentStep = ActiveModel->GetCurrentStep();

    if (Options.UsingViewpoint) { // ViewPoints (Front, Back, Top, Bottom, Left, Right, Home)
        ActiveView->SetViewpoint(lcViewpoint(gApplication->mPreferences.mNativeViewpoint));
    } else {                      // Default View (Angles + Perspective|Orthographic)
        Camera->m_fovy  = Options.FoV;
        Camera->m_zNear = Options.ZNear;
        Camera->m_zFar  = Options.ZFar;
        Camera->UpdatePosition(CurrentStep);
        ActiveView->SetProjection(gApplication->mPreferences.mNativeProjection);
        ActiveView->SetCameraAngles(Options.Latitude, Options.Longitude);
    }

    // Set zoom
    ActiveView->MakeCurrent();
    lcContext* Context = ActiveView->mContext;
    Camera->Zoom(Options.CameraDistance,CurrentStep,true);

    View View(ActiveModel);
    View.SetHighlight(Options.HighlightNewParts);
    View.SetCamera(Camera, false);
    View.SetContext(Context);

    // generate image
    const int ImageWidth  = Options.ImageWidth;
    const int ImageHeight = Options.ImageHeight;

    if (!View.BeginRenderToImage(ImageWidth, ImageHeight))
    {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not begin RenderToImage for Native %1 image.").arg(ImageType));
        return false;
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
                             .arg(ImageType)
                             .arg(Options.OutputFileName).arg(Writer.errorString()));
        return false;
    }

    View.EndRenderToImage();
    Context->ClearResources();

    ActiveModel->SetTemporaryStep(CurrentStep);

    if (!ActiveModel->mActive)
        ActiveModel->CalculateStep(LC_STEP_MAX);

     emit gui->messageSig(LOG_INFO,QMessageBox::tr("Native %1 image file rendered '%2'")
                          .arg(ImageType).arg(Options.OutputFileName));

    if (Options.ExportMode != EXPORT_NONE) {
        if (!NativeExport(Options)) {
            emit gui->messageSig(LOG_ERROR,QMessageBox::tr("CSI Objects render failed."));
            return false;
        }
    }

    return true;
}

bool Render::LoadViewer(const ViewerOptions &Options){

    QString viewerCsiKey = Options.ViewerCsiKey;

    // Load model
    Project* StepProject = new Project();
    if (LoadStepProject(StepProject, viewerCsiKey)){
        gApplication->SetProject(StepProject);
        gMainWindow->UpdateAllViews();
    }
    else
    {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not load 3DViewer model file %1.")
                             .arg(viewerCsiKey));
        delete StepProject;
        return false;
    }

    gui->setViewerCsiKey(viewerCsiKey);

    View* ActiveView = gMainWindow->GetActiveView();

    gMainWindow->GetPartSelectionWidget()->SetDefaultPart();

    lcModel* ActiveModel = ActiveView->GetActiveModel();

    lcCamera* Camera =  ActiveView->mCamera;

    lcStep CurrentStep = ActiveModel->GetCurrentStep();

    if (Options.UsingViewpoint) { // ViewPoints (Front, Back, Top, Bottom, Left, Right, Home)
        ActiveView->SetViewpoint(lcViewpoint(gApplication->mPreferences.mNativeViewpoint));
    } else {                      // Default View (Angles + Perspective|Orthographic)
        Camera->m_fovy  = Options.FoV;
        Camera->m_zNear = Options.ZNear;
        Camera->m_zFar  = Options.ZFar;
        Camera->UpdatePosition(CurrentStep);
        ActiveView->SetProjection(gApplication->mPreferences.mNativeProjection);
        ActiveView->SetCameraAngles(Options.Latitude, Options.Longitude);
        if (ActiveView->mCamera->IsOrtho())
            ActiveView->ZoomExtents();
        else
            Camera->Zoom(Options.CameraDistance,CurrentStep,true);
    }
    ActiveView->MakeCurrent();
    lcContext* Context = ActiveView->mContext;
    View View(ActiveModel);
    View.SetHighlight(false);
    View.SetCamera(Camera, false);
    View.SetContext(Context);

    return true;
}

bool Render::LoadStepProject(Project* StepProject, const QString& viewerCsiKey)
{
    QString FileName = gui->getViewerStepFilePath(viewerCsiKey);

    if (FileName.isEmpty())
    {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Did not receive 3DViewer CSI path for %1.").arg(FileName));
        return false;
    }

    QStringList CsiContent = gui->getViewerStepRotatedContents(viewerCsiKey);
    if (CsiContent.isEmpty())
    {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Did not receive 3DViewer CSI content for %1.").arg(FileName));
        return false;
    }

#ifdef QT_DEBUG_MODE
    QString valueAt0 = viewerCsiKey.at(0);
    bool inside = (valueAt0 == "\"");                                                 // true if the first character is "
    QStringList tmpList = viewerCsiKey.split(QRegExp("\""), QString::SkipEmptyParts); // Split by "
    QStringList argv01;
    foreach (QString s, tmpList) {
        if (inside) {                                                                 // If 's' is inside quotes ...
            argv01.append(s);                                                         // ... get the whole string
        } else {                                                                      // If 's' is outside quotes ...
            argv01.append(s.split(" ", QString::SkipEmptyParts));                     // ... get the split string
        }
        inside = !inside;
    }
    QString modelName  = argv01[0];                                                   //0=modelName

    QFileInfo outFileInfo(FileName);
    QString outfileName = QString("%1/viewer_%2_%3.ldr")
            .arg(outFileInfo.absolutePath())
            .arg(outFileInfo.baseName().replace(".ldr",""))
            .arg(QFileInfo(modelName).baseName());

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

bool Render::NativeExport(const NativeOptions &Options) {

    QString exportModeName = nativeExportNames[Options.ExportMode];

    if (Options.ExportMode == EXPORT_WAVEFRONT ||
        Options.ExportMode == EXPORT_COLLADA   ||
        Options.ExportMode == EXPORT_CSV       ||
        Options.ExportMode == EXPORT_BRICKLINK /*||
        Options.ExportMode == EXPORT_3DS_MAX*/) {
        emit gui->messageSig(LOG_STATUS, QString("Native CSI %1 Export...").arg(exportModeName));
        Project* NativeExportProject = new Project();
        gApplication->SetProject(NativeExportProject);

        if (! gMainWindow->OpenProject(Options.InputFileName)) {
            emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Failed to open CSI %1 Export project")
                                                           .arg(exportModeName));
            delete NativeExportProject;
            return false;
        }
    }
    else
    {
        return doLDVCommand(Options.ExportArgs, Options.ExportMode);
    }

    if (Options.ExportMode == EXPORT_CSV)
    {
        lcGetActiveProject()->ExportCSV();
    }
    else
    if (Options.ExportMode == EXPORT_BRICKLINK)
    {
        lcGetActiveProject()->ExportBrickLink();
    }
    else
    if (Options.ExportMode == EXPORT_WAVEFRONT)
    {
        lcGetActiveProject()->ExportWavefront(Options.ExportFileName);
    }
    else
    if (Options.ExportMode == EXPORT_COLLADA)
    {
        lcGetActiveProject()->ExportCOLLADA(Options.ExportFileName);
    }

/*
    // These are executed through the LDV Native renderer

    if (Options.ExportMode == EXPORT_HTML)
    {
        lcHTMLExportOptions HTMLOptions(lcGetActiveProject());

        lcQHTMLDialog Dialog(gMainWindow, &HTMLOptions);
        if (Dialog.exec() != QDialog::Accepted)
            return false;

        HTMLOptions.SaveDefaults();

        //HTMLOptions.PathName = Options.ExportFileName;

        lcGetActiveProject()->ExportHTML(HTMLOptions);
    }
    else
    if (Options.ExportMode == EXPORT_POVRAY)
    {
        lcGetActiveProject()->ExportPOVRay(Options.ExportFileName);
    }
    else
    if (Options.ExportMode == EXPORT_3DS_MAX)
    {
        lcGetActiveProject()->Export3DStudio(Options.ExportFileName);
    }
*/

    return true;
}

void Render::showLdvExportSettings(int iniFlag){
    ldvWidget = new LDVWidget(nullptr,IniFlag(iniFlag),true);
    ldvWidget->showLDVExportOptions();
}

void Render::showLdvLDrawPreferences(int iniFlag){
    ldvWidget = new LDVWidget(nullptr,IniFlag(iniFlag),true);
    ldvWidget->showLDVPreferences();
}

bool Render::doLDVCommand(const QStringList &args, int exportMode, int iniFlag){
    QString exportModeName = nativeExportNames[exportMode];
    bool exportHTML = exportMode == EXPORT_HTML;
    QStringList arguments = args;

    if (exportMode == EXPORT_NONE && iniFlag == NumIniFiles) {
        emit gui->messageSig(LOG_ERROR, QString("Invalid export mode and ini flag codes specified."));
        return false;
    }

    switch (exportMode){
    case EXPORT_HTML:
        iniFlag = NativePartList;
        break;
    case EXPORT_POVRAY:
        iniFlag = NativePOVIni;
        break;
    case POVRAY_RENDER:
        iniFlag = POVRayRender;
        break;
    case EXPORT_STL:
        iniFlag = NativeSTLIni;
        break;
    case EXPORT_3DS_MAX:
        iniFlag = Native3DSIni;
        break;
    default:
        if (iniFlag == NumIniFiles)
            return false;
        break;
    }

    QString workingDirectory = QDir::currentPath();
    emit gui->messageSig(LOG_TRACE, QString("Native CSI %1 Export for command: %2")
                                             .arg(exportModeName)
                                             .arg(arguments.join(" ")));
    ldvWidget = new LDVWidget(nullptr,IniFlag(iniFlag),true);
    if (exportHTML)
        gui->connect(ldvWidget, SIGNAL(loadBLCodesSig()), gui, SLOT(loadBLCodes()));
    if (! ldvWidget->doCommand(arguments))  {
        emit gui->messageSig(LOG_ERROR, QString("Failed to generate CSI %1 Export for command: %2")
                                                .arg(exportModeName)
                                                .arg(arguments.join(" ")));
        return false;
    }
    if (! QDir::setCurrent(workingDirectory)) {
        emit gui->messageSig(LOG_ERROR, QString("Failed to restore CSI %1 export working directory: %2")
                                                .arg(exportModeName)
                                                .arg(workingDirectory));
        return false;
    }
    if (exportHTML)
        gui->disconnect(ldvWidget, SIGNAL(loadBLCodesSig()), gui, SLOT(loadBLCodes()));

    return true;
}

const QString Render::getPovrayRenderQuality(int quality)
{
    if (quality == -1)
        quality = Preferences::povrayRenderQuality;

    QStringList Arguments;

    switch (quality)
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

    return Arguments.join(" ");
}

const QString Render::getPovrayRenderFileName(const QString &viewerCsiKey)
{
    QString valueAt0 = viewerCsiKey.at(0);
    bool inside = (valueAt0 == "\"");                                                 // true if the first character is "
    QStringList tmpList = viewerCsiKey.split(QRegExp("\""), QString::SkipEmptyParts); // Split by "
    QStringList argv01;
    foreach (QString s, tmpList) {
        if (inside) {                                                                 // If 's' is inside quotes ...
            argv01.append(s);                                                         // ... get the whole string
        } else {                                                                      // If 's' is outside quotes ...
            argv01.append(s.split(" ", QString::SkipEmptyParts));                     // ... get the split string
        }
        inside = !inside;
    }
    QString csiModel  = argv01[0];                                                   //0=modelName

    QString fileName = gui->getViewerStepFilePath(viewerCsiKey);

    if (fileName.isEmpty()){
        emit gui->messageSig(LOG_ERROR, QString("Failed to receive ldrFileName for viewerCsiKey : %1").arg(viewerCsiKey));
        return QString();
    }

    QDir povrayDir(QString("%1/%2").arg(QDir::currentPath()).arg(Paths::povrayRenderDir));
    if (!povrayDir.exists())
        Paths::mkPovrayDir();

    QFileInfo csiFile(fileName);
    QString imageFile = QDir::toNativeSeparators(QString("%1/%2_%3.png")
                       .arg(povrayDir.absolutePath())
                       .arg(csiFile.baseName().replace(".ldr",""))
                       .arg(QFileInfo(csiModel).baseName()));

    return imageFile;

}

// create Native version of the csi file
int Render::createNativeCSI(
    QStringList &csiRotatedParts,
    bool         doFadeStep,
    bool         doHighlightStep)
{
  QStringList csiSubModels;
  QStringList csiSubModelParts;
  QStringList csiParts = csiRotatedParts;

  QStringList argv;
  int         rc;

  if (csiRotatedParts.size() > 0) {

      for (int index = 0; index < csiRotatedParts.size(); index++) {

          QString csiLine = csiRotatedParts[index];
          split(csiLine, argv);
          if (argv.size() == 15 && argv[0] == "1") {

              /* process subfiles in csiRotatedParts */
              QString type = argv[argv.size()-1];

              bool isCustomSubModel = false;
              bool isCustomPart = false;
              QString customType;

              // Custom part types
              if (doFadeStep) {
                  bool isFadedItem = type.contains("-fade.");
                  // Fade file
                  if (isFadedItem) {
                      customType = type;
                      customType = customType.replace("-fade.",".");
                      isCustomSubModel = gui->isSubmodel(customType);
                      isCustomPart = gui->isUnofficialPart(customType);
                    }
                }

              if (doHighlightStep) {
                  bool isHighlightItem = type.contains("-highlight");
                  // Highlight file
                  if (isHighlightItem) {
                      customType = type;
                      customType = customType.replace("-highlight.",".");
                      isCustomSubModel = gui->isSubmodel(customType);
                      isCustomPart = gui->isUnofficialPart(customType);
                    }
                }

              if (gui->isSubmodel(type) || gui->isUnofficialPart(type) || isCustomSubModel || isCustomPart) {
                  /* capture subfiles (full string) to be processed when finished */
                  if (!csiSubModels.contains(type.toLower()))
                       csiSubModels << type.toLower();
                }
            }
        } //end for

      /* process extracted submodels and unofficial files */
      if (csiSubModels.size() > 0){
          if (csiSubModels.size() > 2)
              csiSubModels.removeDuplicates();
          if ((rc = mergeNativeCSISubModels(csiSubModels, csiSubModelParts, doFadeStep, doHighlightStep)) != 0){
              emit gui->messageSig(LOG_ERROR,QString("Failed to process viewer CSI submodels"));
              return rc;
            }
        }

      /* add sub model content to csiRotatedParts file */
      if (! csiSubModelParts.empty())
        {
          for (int i = 0; i < csiSubModelParts.size(); i++) {
              QString smLine = csiSubModelParts[i];
              csiParts << smLine;
            }
        }
      csiRotatedParts = csiParts;
    }
  return 0;
}

int Render::mergeNativeCSISubModels(QStringList &subModels,
                                  QStringList &subModelParts,
                                  bool doFadeStep,
                                  bool doHighlightStep)
{
  QStringList csiSubModels        = subModels;
  QStringList csiSubModelParts    = subModelParts;
  QStringList newSubModels;

  QStringList argv;
  int         rc;

  if (csiSubModels.size() > 0) {

      /* read in all detected sub model file content */
      for (int index = 0; index < csiSubModels.size(); index++) {
          QString ldrName(QDir::currentPath() + "/" +
                          Paths::tmpDir + "/" +
                          csiSubModels[index]);

          /* initialize the working submodel file - define header. */
          QString modelName = QFileInfo(csiSubModels[index]).baseName().toLower();
          modelName = modelName.replace(
                      modelName.indexOf(modelName.at(0)),1,modelName.at(0).toUpper());
          csiSubModelParts << QString("0 FILE %1").arg(csiSubModels[index]);
          csiSubModelParts << QString("0 %1").arg(modelName);
          csiSubModelParts << QString("0 Name: %1").arg(csiSubModels[index]);
          csiSubModelParts << QString("0 !LEOCAD MODEL NAME %1").arg(modelName);

          /* read the actual submodel file */
          QFile ldrfile(ldrName);
          if ( ! ldrfile.open(QFile::ReadOnly | QFile::Text)) {
              emit gui->messageSig(LOG_ERROR,QString("Could not read CSI submodel file %1: %2")
                                   .arg(ldrName)
                                   .arg(ldrfile.errorString()));
              return -1;
            }
          /* populate file contents into working submodel csi parts */
          QTextStream in(&ldrfile);
          while ( ! in.atEnd()) {
              QString csiLine = in.readLine(0);
              split(csiLine, argv);

              if (argv.size() == 15 && argv[0] == "1") {
                  /* check and process any subfiles in csiRotatedParts */
                  QString type = argv[argv.size()-1];

                  bool isCustomSubModel = false;
                  bool isCustomPart = false;
                  QString customType;

                  // Custom part types
                  if (doFadeStep) {
                      bool isFadedItem = type.contains("-fade.");
                      // Fade file
                      if (isFadedItem) {
                          customType = type;
                          customType = customType.replace("-fade.",".");
                          isCustomSubModel = gui->isSubmodel(customType);
                          isCustomPart = gui->isUnofficialPart(customType);
                        }
                    }

                  if (doHighlightStep) {
                      bool isHighlightItem = type.contains("-highlight");
                      // Highlight file
                      if (isHighlightItem) {
                          customType = type;
                          customType = customType.replace("-highlight.",".");
                          isCustomSubModel = gui->isSubmodel(customType);
                          isCustomPart = gui->isUnofficialPart(customType);
                        }
                    }

                  if (gui->isSubmodel(type) || gui->isUnofficialPart(type) || isCustomSubModel || isCustomPart) {
                      /* capture all subfiles (full string) to be processed when finished */
                      if (!newSubModels.contains(type.toLower()))
                              newSubModels << type.toLower();
                    }
                }
              if (isGhost(csiLine))
                  argv.prepend(GHOST_META);
              csiLine = argv.join(" ");
              csiSubModelParts << csiLine;
            }
          csiSubModelParts << "0 NOFILE";
        }

      /* recurse and process any identified submodel files */
      if (newSubModels.size() > 0){
          if (newSubModels.size() > 2)
              newSubModels.removeDuplicates();
          if ((rc = mergeNativeCSISubModels(newSubModels, csiSubModelParts, doFadeStep, doHighlightStep)) != 0){
              emit gui->messageSig(LOG_ERROR,QString("Failed to recurse viewer CSI submodels"));
              return rc;
            }
        }
      subModelParts = csiSubModelParts;
    }
  return 0;
}
