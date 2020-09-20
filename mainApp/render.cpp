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
#define LP3D_CA 0.01
#define LP3D_CDF 1.0
#define USE_ALPHA "+UA"

#define SNAPSHOTS_LIST_THRESHOLD 3

static double pi = 4*atan(1.0);

// the default camera distance for real size
static float LduDistance = float(10.0/tan(0.005*pi/180));

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

const QString Render::getRotstepMeta(RotStepMeta &rotStep, bool isKey /*false*/){
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
                              .arg(rotStep.value().type.isEmpty() ? "REL" :
                                   rotStep.value().type.trimmed())
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

void Render::addArgument(
        QStringList   &arguments,
        const QString &newArg,
        const QString &argChk,
        const int      povGenerator,
        const int      additionalArgs) {

    if (!additionalArgs) {
        arguments.append(newArg);
        return;
    }

    auto isMatch = [&newArg] (QString &oldArg) {
        QStringList flags = QStringList()
            << "-ca" << "-cg"
                ;
        if (getRenderer() == RENDERER_LDGLITE) {
            flags
            << "-J" << "-j" << "-v" << "-o"
            << "-w"  << "-mF" << "-ldcF" << "-i2"
            << "-fh" << "-q"    << "-2g,2x"
            << "-N"  << "-2g" << "-l3"   << "-fe"
            << "-fr" << "-ff" << "-x"    << "-ms"
            << "-mS" << "-e"  << "-p"    << "-ld"
            << "-z"  << "-Z"  << "-cc"   << "-co"
            << "-cla"<< "-cu" << "-lc"   << "-lC"
               ;
        } else {
            flags
            << "-vv" << "-q" << "-qq"
               ;
        }
        for (int i = 0; i < flags.size(); i++){
            if (newArg.startsWith(flags[i]) && oldArg.startsWith(flags[i]))
               return true;
        }
        return false;
    };

    int insertIndex = -1;
    for (int i = 0; i < arguments.size(); i++) {
        if (arguments[i] != "" && arguments[i] != " ") {
            if (argChk.isEmpty()) {
                if (!(getRenderer() == RENDERER_POVRAY && !povGenerator)){
                    if (isMatch(arguments[i]) ||
                        arguments[i].startsWith(newArg.left(newArg.indexOf("=")))) {
                        insertIndex = arguments.indexOf(arguments[i]);
                        break;
                    }
                }
            } else
            if (arguments[i].contains(argChk)) {
                insertIndex = arguments.indexOf(arguments[i]);
                break;
            }
        }
    }

    if (insertIndex < 0) {
        arguments.append(newArg);
    } else {
        arguments.replace(insertIndex,newArg);
    }
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
//#ifdef QT_DEBUG_MODE
//    logTrace() << "\n" << QString("DEBUG - STANDARD CAMERA DISTANCE")
//               << "\n" << QString("PI [4*atan(1.0)]                    : %1").arg(double(pi))
//               << "\n" << QString("LduDistance [10.0/tan(0.005*pi/180)]: %1").arg(double(LduDistance))
//               << "\n" << QString("Page Width [pixels]                 : %1").arg(gui->pageSize(meta.LPub.page, 0))
//               << "\n" << QString("Resolution [LDU]                    : %1").arg(QString::number(double(meta.LPub.resolution.ldu()), 'f' ,10))
//               << "\n" << QString("Resolution [pixels]                 : %1").arg(double(meta.LPub.resolution.value()))
//               << "\n" << QString("Scale                               : %1").arg(double(scale))
//               << "\n" << QString("1x1 [20*res.ldu*res.pix*scale]      : %1").arg(QString::number(double(onexone), 'f' ,10))
//               << "\n" << QString("Factor [Page size/OnexOne]          : %1").arg(QString::number(double(factor), 'f' ,10))
//               << "\n" << QString("Cam Distance [Factor*LduDistance]   : %1").arg(QString::number(double(factor*LduDistance), 'f' ,10))
//                  ;
//#endif
    return factor*LduDistance;
}

float Render::getPovrayRenderCameraDistance(const QString &cdKeys){
    enum Cda {
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

const QStringList Render::getSubAttributes(const QString &nameKey)
{
    QFileInfo fileInfo(nameKey);
    QString cleanString = fileInfo.completeBaseName();
    if (Preferences::enableFadeSteps && nameKey.endsWith(FADE_SFX))
        cleanString.chop(QString(FADE_SFX).size());
    else
    if (Preferences::enableHighlightStep && nameKey.endsWith(HIGHLIGHT_SFX))
        cleanString.chop(QString(HIGHLIGHT_SFX).size());

    return cleanString.split("_");
}

bool Render::createSnapshotsList(
    const QStringList &ldrNames,
    const QString &SnapshotsList)
{
    QFile SnapshotsListFile(SnapshotsList);
    if ( ! SnapshotsListFile.open(QFile::WriteOnly | QFile::Text)) {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Failed to create LDView (Single Call) PLI Snapshots list file!"));
        return false;
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
    return true;
}

int Render::executeLDViewProcess(QStringList &arguments, Options::Mt module) {

  QString message = QString("LDView %1 %2 Arguments: %3 %4")
                            .arg(useLDViewSCall() ? "(SingleCall)" : "(Default)")
                            .arg(module == Options::CSI ? "CSI" : "PLI")
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
                               .arg(useLDViewSCall() ? "(SingleCall)" : "(Default)")
                               .arg(module == Options::CSI ? "CSI" : "PLI")
                               .arg(ldview.exitCode())
                               .arg(str));
          return -1;
        }
    }

  QFile outputImageFile(arguments.last());
  if (! outputImageFile.exists()) {
      emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView %1 image generation failed for %2 with message %3")
                           .arg(module == Options::CSI ? "CSI" : "PLI")
                           .arg(outputImageFile.fileName())
                           .arg(outputImageFile.errorString()));
      return -1;
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
  return stdCameraDistance(meta, scale)*0.455f;
}

int POVRay::renderCsi(
    const QString     &addLine,
    const QStringList &csiParts,
    const QStringList &csiKeys,
    const QString     &pngName,
    Meta              &meta,
    int                nType)
{
  Q_UNUSED(csiKeys)
  Q_UNUSED(nType)

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
  int cd = int(meta.LPub.assem.cameraDistance.value());
  if (cd){
      cd = int((cd*0.455)*1700/1000);
  } else {
      cd = int(cameraDistance(meta,meta.LPub.assem.modelScale.value())*1700/1000);
  }

  /* apply camera angle */
  bool pp    = Preferences::perspectiveProjection;
  bool noCA  = Preferences::applyCALocally;

  bool useImageSize = meta.LPub.assem.imageSize.value(0) > 0;
  int width  = useImageSize ? int(meta.LPub.assem.imageSize.value(0)) : gui->pageSize(meta.LPub.page, 0);
  int height = useImageSize ? int(meta.LPub.assem.imageSize.value(1)) : gui->pageSize(meta.LPub.page, 1);

  QString CA = QString("-ca%1").arg(LP3D_CA);
  QString cg = QString("-cg%1,%2,%3")
                       .arg(noCA ? 0.0 : double(meta.LPub.assem.cameraAngles.value(0)))
                       .arg(noCA ? 0.0 : double(meta.LPub.assem.cameraAngles.value(1)))
                       .arg(QString::number(pp ? cd * LP3D_CDF : cd,'f',0));
  QString sl = meta.LPub.assem.studLogo.value() ?
                         QString("-StudLogo=%1")
                                 .arg(meta.LPub.assem.studLogo.value()) : QString();
  QString m  = meta.LPub.assem.target.isPopulated() ?
                         QString("-ModelCenter=%1,%2,%3")
                                 .arg(double(meta.LPub.assem.target.x()))
                                 .arg(double(meta.LPub.assem.target.y()))
                                 .arg(double(meta.LPub.assem.target.z())) : QString();

  QString w  = QString("-SaveWidth=%1") .arg(width);
  QString h  = QString("-SaveHeight=%1") .arg(height);
  QString f  = QString("-ExportFile=%1") .arg(povName);
  QString l  = QString("-LDrawDir=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldrawLibPath)));
  QString o  = QString("-HaveStdOut=1");
  QString v  = QString("-vv");

  // projection settings
  QString df = QString("-FOV=%1").arg(double(meta.LPub.assem.cameraFoV.value()));
  QString dz = QString("-DefaultZoom=%1").arg(double(meta.LPub.assem.modelScale.value()));
  bool pl    = false, pf = false, pz = false, pd = false;
  qreal cdf  = LP3D_CDF;

  QStringList arguments;
  arguments << CA;
  arguments << cg;
  arguments << sl;

  // additional LDView parameters;
  list = meta.LPub.assem.ldviewParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
    if (list[i] != "" && list[i] != " ") {
        if (pp) {
          if ((pl = list[i].contains("-DefaultLatLong=")))
            arguments.replace(arguments.indexOf(cg),list[i]); // replace Camera globe
          if ((pf = list[i].contains("-FOV=")))
            arguments.replace(arguments.indexOf(CA),list[i]); // replace Camera CA
          if ((pd = list[i].contains("-LDViewPerspectiveDistanceFactor="))) {
              bool ok;
              qreal _cdf = QStringList(list[i].split("=")).last().toDouble(&ok);
              if (ok && _cdf < LP3D_CDF)
                  cdf = _cdf;
          }
          pz = list[i].contains("-DefaultZoom=");
        }
      if (!pl && !pf && !pd) {
        addArgument(arguments,list[i]);    // 10. ldviewParms
      }
    }
  }
  if (pp) {
    if (!pf)
      arguments.replace(arguments.indexOf(CA),df); // replace CA with FOV
    if (!pz && pl)
      arguments.insert(2,dz);                      // add DefaultZoom if DefaultLatLon specified
    if (!pl && cdf < LP3D_CDF) {                   // update camera distance factor if custom value specified
      QStringList cgl;
      cgl = cg.split(",");
      cgl.replace(cgl.indexOf(cgl.last()),QString::number(cd * cdf,'f',0));
      arguments.replace(arguments.indexOf(cg),cgl.join(","));
    }
  }
  if (list.size())
      emit gui->messageSig(LOG_INFO,QMessageBox::tr("LDView additional POV-Ray CSI renderer parameters: %1")
                           .arg(list.join(" ")));

  // Set alternate target position or use specified image size
  if ((!m.isEmpty() && !pl) || (useImageSize && m.isEmpty())){
    arguments.removeAt(arguments.indexOf(cg));      // remove camera globe
    QString dl;
    dl = QString("-DefaultLatLong=%1,%2")
                  .arg(noCA ? 0.0 : double(meta.LPub.assem.cameraAngles.value(0)))
                  .arg(noCA ? 0.0 : double(meta.LPub.assem.cameraAngles.value(1)));
    dz = QString("-DefaultZoom=%1").arg(double(meta.LPub.assem.modelScale.value()));
    addArgument(arguments, dl, "-DefaultLatLong");
    addArgument(arguments, dz, "-DefaultZoom");

    // Set zoom to fit when image size specified
    if (useImageSize && m.isEmpty()){
      QString sz = QString("-SaveZoomToFit=1");
      addArgument(arguments, sz, "SaveZoomToFit");
    }
  }

  arguments << m;
  arguments << w;
  arguments << h;
  arguments << f;
  arguments << l;

  if (!Preferences::altLDConfigPath.isEmpty()) {
     QString altldc = "-LDConfig=" + Preferences::altLDConfigPath;
     addArgument(arguments, altldc, "-LDConfig", 0, list.size());
  }

  // LDView block begin
  if (Preferences::povFileGenerator == RENDERER_LDVIEW) {

      addArgument(arguments, o, "-HaveStdOut", 0, list.size());
      addArgument(arguments, v, "-vv", 0, list.size());

//      if (Preferences::enableFadeSteps)
//        arguments <<  QString("-SaveZMap=1");

      bool hasLDViewIni = Preferences::ldviewPOVIni != "";
      if(hasLDViewIni){
          QString ini  = QString("-IniFile=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldviewPOVIni)));
          addArgument(arguments, ini, "-IniFile", 0, list.size());
        }

      arguments << QDir::toNativeSeparators(ldrName);

      removeEmptyStrings(arguments);

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

      removeEmptyStrings(arguments);

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

  list = meta.LPub.assem.povrayParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
      if (list[i] != "" && list[i] != " ") {
          addArgument(povArguments, list[i], QString(), true);
        }
    }
  if (list.size())
      emit gui->messageSig(LOG_INFO,QMessageBox::tr("POV-Ray additional CSI renderer parameters: %1")
                           .arg(list.join(" ")));

//#ifndef __APPLE__
//  povArguments << "/EXIT";
//#endif

  removeEmptyStrings(povArguments);

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
    int                pliType,
    int                keySub)
{
  // Select meta type
  PliMeta &metaType = pliType == SUBMODEL ? static_cast<PliMeta&>(meta.LPub.subModel) :
                      pliType == BOM ? meta.LPub.bom : meta.LPub.pli;

  QStringList list;
  QString message;
  QString povName = ldrNames.first() +".pov";

  // Populate render attributes
  QString transform  = metaType.rotStep.value().type;
  bool  noCA         = transform  == "ABS";
  float modelScale   = metaType.modelScale.value();
  float cameraFov    = metaType.cameraFoV.value();
  float cameraAngleX = metaType.cameraAngles.value(0);
  float cameraAngleY = metaType.cameraAngles.value(1);
  xyzVector target   = xyzVector(metaType.target.x(),metaType.target.y(),metaType.target.z());

  // process substitute attributes
  // Process substitute part attributes
  if (keySub) {
    QStringList attributes = getSubAttributes(pngName);
    bool hr;
    if ((hr = attributes.size() == nHasRotstep) || attributes.size() == nHasTargetAndRotstep)
      noCA = attributes.at(hr ? nRotTrans : nRot_Trans) == "ABS";
    if (attributes.size() >= nHasTarget)
      target = xyzVector(attributes.at(nTargetX).toFloat(),attributes.at(nTargetY).toFloat(),attributes.at(nTargetZ).toFloat());
    if (keySub > PliBeginSub2Rc) {
      modelScale   = attributes.at(nModelScale).toFloat();
      cameraFov    = attributes.at(nCameraFoV).toFloat();
      cameraAngleX = attributes.at(nCameraAngleXX).toFloat();
      cameraAngleY = attributes.at(nCameraAngleYY).toFloat();
    }
  }

  /* determine camera distance */
  int cd = int(metaType.cameraDistance.value());
  if (cd){
      cd = int((cd*0.455)*1700/1000);
  } else {
      cd = int((cameraDistance(meta,modelScale))*1700/1000);
  }


  bool useImageSize = metaType.imageSize.value(0) > 0;
  int width  = useImageSize ? int(metaType.imageSize.value(0)) : gui->pageSize(meta.LPub.page, 0);
  int height = useImageSize ? int(metaType.imageSize.value(1)) : gui->pageSize(meta.LPub.page, 1);

  bool pp    = Preferences::perspectiveProjection;
  if (pliType == SUBMODEL)
      noCA   = Preferences::applyCALocally || noCA;

  QString CA = QString("-ca%1")       .arg(LP3D_CA);
  QString cg = QString("-cg%1,%2,%3") .arg(noCA ? 0.0 : double(cameraAngleX))
                                      .arg(noCA ? 0.0 : double(cameraAngleY))
                                      .arg(QString::number(pp ? cd * LP3D_CDF : cd,'f',0));

  QString m  = target.isPopulated() ?
               QString("-co%1,%2,%3")
                       .arg(double(target.x))
                       .arg(double(target.y))
                       .arg(double(target.z)) : QString();

  QString sl = metaType.studLogo.value() ?
                         QString("-StudLogo=%1")
                                 .arg(metaType.studLogo.value()) : QString();
  QString w  = QString("-SaveWidth=%1")  .arg(width);
  QString h  = QString("-SaveHeight=%1") .arg(height);
  QString f  = QString("-ExportFile=%1") .arg(povName);  // -ExportSuffix not required
  QString l  = QString("-LDrawDir=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldrawLibPath)));
  QString o  = QString("-HaveStdOut=1");
  QString v  = QString("-vv");

  // projection settings
  QString df = QString("-FOV=%1").arg(double(cameraFov));
  QString dz = QString("-DefaultZoom=%1").arg(double(metaType.modelScale.value()));
  bool pl    = false, pf = false, pz = false, pd = false;
  qreal cdf  = LP3D_CDF;

  QStringList arguments;
  arguments << CA;
  arguments << cg;
  arguments << sl;

  // additional LDView parameters;
  list = metaType.ldviewParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
    if (list[i] != "" && list[i] != " ") {
        if (pp) {
          if ((pl = list[i].contains("-DefaultLatLong=")))
            arguments.replace(arguments.indexOf(cg),list[i]); // remove Camera globe
          if ((pf = list[i].contains("-FOV=")))
            arguments.replace(arguments.indexOf(CA),list[i]); // remove Camera CA
          if ((pd = list[i].contains("-LDViewPerspectiveDistanceFactor="))) {
              bool ok;
              qreal _cdf = QStringList(list[i].split("=")).last().toDouble(&ok);
              if (ok && _cdf < LP3D_CDF)
                  cdf = _cdf;
          }
          pz = list[i].contains("-DefaultZoom=");
        }
      if (!pl && !pf && !pd) {
        addArgument(arguments, list[i]); // 10. ldviewParms
      }
    }
  }
  if (pp) {
    if (!pf)
      arguments.replace(arguments.indexOf(CA),df); // replace CA with FOV
    if (!pz && pl)
      arguments.insert(2,dz);                      // add DefaultZoom if DefaultLatLon specified
    if (!pl && cdf < LP3D_CDF) {                   // update camera distance factor if custom value specified
      QStringList cgl;
      cgl = cg.split(",");
      cgl.replace(cgl.indexOf(cgl.last()),QString::number(cd * cdf,'f',0));
      arguments.replace(arguments.indexOf(cg),cgl.join(","));
    }
  }
  if (list.size())
      emit gui->messageSig(LOG_INFO,QMessageBox::tr("LDView additional POV-Ray PLI renderer parameters: %1")
                           .arg(list.join(" ")));

  // Set alternate target position or use specified image size
  if ((!m.isEmpty() && !pl) || (useImageSize && m.isEmpty())){
    arguments.removeAt(arguments.indexOf(cg));      // remove camera globe
    QString dl;
    dl = QString("-DefaultLatLong=%1,%2")
                  .arg(noCA ? 0.0 : double(metaType.cameraAngles.value(0)))
                  .arg(noCA ? 0.0 : double(metaType.cameraAngles.value(1)));
    dz = QString("-DefaultZoom=%1").arg(double(metaType.modelScale.value()));
    addArgument(arguments, dl, "-DefaultLatLong");
    addArgument(arguments, dz, "-DefaultZoom");

    // Set zoom to fit when image size specified
    if (useImageSize && m.isEmpty()){
      QString sz = QString("-SaveZoomToFit=1");
      addArgument(arguments, sz, "SaveZoomToFit");
    }
  }

  arguments << m;
  arguments << w;
  arguments << h;
  arguments << f;
  arguments << l;

  if (!Preferences::altLDConfigPath.isEmpty()) {
     QString altldc = "-LDConfig=" + Preferences::altLDConfigPath;
     addArgument(arguments, altldc, "-LDConfig", 0, list.size());
  }

  // LDView block begin
  if (Preferences::povFileGenerator == RENDERER_LDVIEW) {

      addArgument(arguments, o, "-HaveStdOut", 0, list.size());
      addArgument(arguments, v, "-vv", 0, list.size());

      bool hasLDViewIni = Preferences::ldviewPOVIni != "";
      if(hasLDViewIni){
          QString ini  = QString("-IniFile=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldviewPOVIni)));
          addArgument(arguments, ini, "-IniFile", 0, list.size());
        }

      arguments << QDir::toNativeSeparators(ldrNames.first());

      removeEmptyStrings(arguments);

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

      removeEmptyStrings(arguments);

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

  list = meta.LPub.assem.povrayParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
      if (list[i] != "" && list[i] != " ") {
          addArgument(povArguments, list[i], QString(), true);
        }
    }
  if (list.size())
      emit gui->messageSig(LOG_INFO,QMessageBox::tr("POV-Ray additional PLI renderer parameters: %1")
                           .arg(list.join(" ")));

//#ifndef __APPLE__
//  povArguments << "/EXIT";
//#endif

  removeEmptyStrings(povArguments);

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
        Meta        &meta,
  int                nType)
{
  Q_UNUSED(csiKeys)
  Q_UNUSED(nType)

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
  int cd = int(meta.LPub.assem.cameraDistance.value());
  if (cd){
      cd = int(meta.LPub.assem.cameraDistance.value());
  } else {
      cd = int(cameraDistance(meta,meta.LPub.assem.modelScale.value()));
  }

  /* apply camera angle */

  bool noCA  = Preferences::applyCALocally;
  bool pp    = Preferences::perspectiveProjection;

  bool useImageSize = meta.LPub.assem.imageSize.value(0) > 0;
  int width  = useImageSize ? int(meta.LPub.assem.imageSize.value(0)) : gui->pageSize(meta.LPub.page, 0);
  int height = useImageSize ? int(meta.LPub.assem.imageSize.value(1)) : gui->pageSize(meta.LPub.page, 1);

  QString v  = QString("-v%1,%2")   .arg(width)
                                    .arg(height);
  QString o  = QString("-o0,-%1")   .arg(height/6);
  QString mf = QString("-mF%1")     .arg(pngName);

  int lineThickness = int(resolution()/150+0.5f);
  if (lineThickness == 0) {
    lineThickness = 1;
  }
  QString w  = QString("-W%1")      .arg(lineThickness); // ldglite always deals in 72 DPI

  QString CA = QString("-ca%1") .arg(pp ? double(meta.LPub.assem.cameraFoV.value()) : LP3D_CA);
  QString cg = QString("-cg%1,%2,%3") .arg(noCA ? 0.0 : double(meta.LPub.assem.cameraAngles.value(0)))
                                      .arg(noCA ? 0.0 : double(meta.LPub.assem.cameraAngles.value(1)))
                                      .arg(cd);

  QString J  = QString("-%1").arg(pp ? "J" : "j");

  QString m  = meta.LPub.assem.target.isPopulated() ?
                         QString("-co%1,%2,%3")
                                 .arg(double(meta.LPub.assem.target.x()))
                                 .arg(double(meta.LPub.assem.target.y()))
                                 .arg(double(meta.LPub.assem.target.z())) : QString();
  QString sl = meta.LPub.assem.studLogo.value() ?
                         QString("-sl%1")
                                 .arg(meta.LPub.assem.studLogo.value()) : QString();

  QStringList arguments;
  arguments << CA;                  // camera FOV in degrees
  arguments << cg;                  // camera globe - scale factor
  arguments << J;                   // projection
  arguments << m;                   // model origin for the camera to look at
  arguments << v;                   // display in X wide by Y high window
  arguments << o;                   // changes the centre X across and Y down
  arguments << w;                   // line thickness
  arguments << sl;                  // stud logo

  QStringList list;
  // First, load parms from meta if any
  list = meta.LPub.assem.ldgliteParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
     if (list[i] != "" && list[i] != " ") {
         addArgument(arguments, list[i]);
      }
  }
  if (list.size())
      emit gui->messageSig(LOG_INFO,QMessageBox::tr("LDGlite additional CSI renderer parameters: %1") .arg(list.join(" ")));

  // Add ini arguments if not already in additional parameters
  for (int i = 0; i < Preferences::ldgliteParms.size(); i++) {
      if (list.indexOf(QRegExp("^" + QRegExp::escape(Preferences::ldgliteParms[i]))) < 0) {
        addArgument(arguments, Preferences::ldgliteParms[i], "", 0, list.size());
      }
  }

  // Add custom color file if exist
  if (!Preferences::altLDConfigPath.isEmpty()) {
    addArgument(arguments, QString("-ldcF%1").arg(Preferences::altLDConfigPath), "-ldcF", 0, list.size());
  }

  arguments << QDir::toNativeSeparators(mf);                  // .png file name
  arguments << QDir::toNativeSeparators(ldrFile);             // csi.ldr (input file)

  removeEmptyStrings(arguments);

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

  QFile outputImageFile(pngName);
  if (! outputImageFile.exists()) {
      emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDGLite CSI image generation failed for %1 with message %2")
                           .arg(outputImageFile.fileName()).arg(outputImageFile.errorString()));
      return -1;
  }

  return 0;
}


int LDGLite::renderPli(
  const QStringList &ldrNames,
  const QString     &pngName,
  Meta              &meta,
  int                pliType,
  int                keySub)
{
  // Select meta type
  PliMeta &metaType = pliType == SUBMODEL ? static_cast<PliMeta&>(meta.LPub.subModel) :
                      pliType == BOM ? meta.LPub.bom : meta.LPub.pli;

  // Populate render attributes
  QString transform  = metaType.rotStep.value().type;
  bool  noCA         = transform  == "ABS";
  float modelScale   = metaType.modelScale.value();
  float cameraFov    = metaType.cameraFoV.value();
  float cameraAngleX = metaType.cameraAngles.value(0);
  float cameraAngleY = metaType.cameraAngles.value(1);
  xyzVector target   = xyzVector(metaType.target.x(),metaType.target.y(),metaType.target.z());

  // Process substitute part attributes
  if (keySub) {
    QStringList attributes = getSubAttributes(pngName);
    bool hr;
    if ((hr = attributes.size() == nHasRotstep) || attributes.size() == nHasTargetAndRotstep)
      noCA = attributes.at(hr ? nRotTrans : nRot_Trans) == "ABS";
    if (attributes.size() >= nHasTarget)
      target = xyzVector(attributes.at(nTargetX).toFloat(),attributes.at(nTargetY).toFloat(),attributes.at(nTargetZ).toFloat());
    if (keySub > PliBeginSub2Rc) {
      modelScale   = attributes.at(nModelScale).toFloat();
      cameraFov    = attributes.at(nCameraFoV).toFloat();
      cameraAngleX = attributes.at(nCameraAngleXX).toFloat();
      cameraAngleY = attributes.at(nCameraAngleYY).toFloat();
    }
  }

  /* determine camera distance */
  int cd = int(metaType.cameraDistance.value());
  if (cd){
      cd = int(metaType.cameraDistance.value());
  } else {
      cd = int(cameraDistance(meta,modelScale));
  }

  bool useImageSize = metaType.imageSize.value(0) > 0;
  int width  = useImageSize ? int(metaType.imageSize.value(0)) : gui->pageSize(meta.LPub.page, 0);
  int height = useImageSize ? int(metaType.imageSize.value(1)) : gui->pageSize(meta.LPub.page, 1);

  int lineThickness = int(double(resolution())/72.0+0.5);

  if (pliType == SUBMODEL)
      noCA   = Preferences::applyCALocally || noCA;

  QString CA = QString("-ca%1") .arg(double(cameraFov));
  QString cg = QString("-cg%1,%2,%3") .arg(noCA ? 0.0 : double(cameraAngleX))
                                      .arg(noCA ? 0.0 : double(cameraAngleY))
                                      .arg(cd);

  QString J  = QString("-J");
  QString v  = QString("-v%1,%2")   .arg(width)
                                    .arg(height);
  QString o  = QString("-o0,-%1")   .arg(height/6);
  QString mf = QString("-mF%1")     .arg(pngName);
  QString w  = QString("-W%1")      .arg(lineThickness);  // ldglite always deals in 72 DPI

  QString m  = target.isPopulated() ?
               QString("-co%1,%2,%3")
                       .arg(double(target.x))
                       .arg(double(target.y))
                       .arg(double(target.z)) : QString();

  QString sl = metaType.studLogo.value() ?
                         QString("-sl%1")
                                 .arg(metaType.studLogo.value()) : QString();

  QStringList arguments;
  arguments << CA;                  // Camera FOV in degrees
  arguments << cg;                  // camera globe - scale factor
  arguments << m;                   // model origin for the camera to look at
  arguments << J;                   // Perspective projection
  arguments << v;                   // display in X wide by Y high window
  arguments << o;                   // changes the centre X across and Y down
  arguments << w;                   // line thickness
  arguments << sl;                  // stud logo

  QStringList list;
  // First, load additional parms from meta if any
  list = metaType.ldgliteParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
     if (list[i] != "" && list[i] != " ") {
         addArgument(arguments, list[i]);
      }
  }
  if (list.size())
      emit gui->messageSig(LOG_INFO,QMessageBox::tr("LDGlite additional PLI renderer parameters %1") .arg(list.join(" ")));

  // Add ini parms if not already added from meta
  for (int i = 0; i < Preferences::ldgliteParms.size(); i++) {
      if (list.indexOf(QRegExp("^" + QRegExp::escape(Preferences::ldgliteParms[i]))) < 0) {
        addArgument(arguments, Preferences::ldgliteParms[i], "", 0, list.size());
      }
  }

  // add custom color file if exist
  if (!Preferences::altLDConfigPath.isEmpty()) {
    addArgument(arguments, QString("-ldcF%1").arg(Preferences::altLDConfigPath), "-ldcF", 0, list.size());
  }

  arguments << QDir::toNativeSeparators(mf);
  arguments << QDir::toNativeSeparators(ldrNames.first());

  removeEmptyStrings(arguments);

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
  if (! ldglite.waitForFinished(rendererTimeout())) {
    if (ldglite.exitCode()) {
      QByteArray status = ldglite.readAll();
      QString str;
      str.append(status);
      emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDGlite failed\n%1") .arg(str));
      return -1;
    }
  }

  QFile outputImageFile(pngName);
  if (! outputImageFile.exists()) {
      emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDGLite PLI image generation failed for %1 with message %2")
                           .arg(outputImageFile.fileName()).arg(outputImageFile.errorString()));
      return -1;
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
    return stdCameraDistance(meta, scale)*0.775f;
}

int LDView::renderCsi(
        const QString     &addLine,
        const QStringList &csiParts,   // this is ldrNames when useLDViewSCall = true
        const QStringList &csiKeys,
        const QString     &pngName,
        Meta              &meta,
        int                nType)
{
    Q_UNUSED(nType)

    /* determine camera distance */
    int cd = int(meta.LPub.assem.cameraDistance.value());
    if (cd){
        cd = int(cd*0.775*1700/1000);
    } else {
        cd = int(cameraDistance(meta,meta.LPub.assem.modelScale.value())*1700/1000);
    }

    /* apply camera angle */
    bool pp    = Preferences::perspectiveProjection;
    bool noCA  = Preferences::applyCALocally;

    /* page size */
    bool useImageSize = meta.LPub.assem.imageSize.value(0) > 0;
    int width  = useImageSize ? int(meta.LPub.assem.imageSize.value(0)) : gui->pageSize(meta.LPub.page, 0);
    int height = useImageSize ? int(meta.LPub.assem.imageSize.value(1)) : gui->pageSize(meta.LPub.page, 1);

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

    } else {                                                    // LDView (Default)

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
  QString CA = QString("-ca%1")       .arg(LP3D_CA);
  QString cg = QString("-cg%1,%2,%3") .arg(noCA ? 0.0 : double(meta.LPub.assem.cameraAngles.value(0)))
                                      .arg(noCA ? 0.0 : double(meta.LPub.assem.cameraAngles.value(1)))
                                      .arg(cd);

  QString m  = meta.LPub.assem.target.isPopulated() ?
                         QString("-ModelCenter=%1,%2,%3")
                                 .arg(double(meta.LPub.assem.target.x()))
                                 .arg(double(meta.LPub.assem.target.y()))
                                 .arg(double(meta.LPub.assem.target.z())) : QString();
  QString sl = meta.LPub.assem.studLogo.value() ?
                         QString("-StudLogo=%1")
                                 .arg(meta.LPub.assem.studLogo.value()) : QString();
  QString w  = QString("-SaveWidth=%1")  .arg(width);
  QString h  = QString("-SaveHeight=%1") .arg(height);
  QString l  = QString("-LDrawDir=%1")   .arg(Preferences::ldrawLibPath);
  QString o  = QString("-HaveStdOut=1");
  QString v  = QString("-vv");

  // projection settings
  QString df = QString("-FOV=%1").arg(double(meta.LPub.assem.cameraFoV.value()));
  QString dz = QString("-DefaultZoom=%1").arg(double(meta.LPub.assem.modelScale.value()));
  bool pl = false, pf = false, pz = false, pd = false;
  qreal cdf = LP3D_CDF;

  QStringList arguments;
  arguments << CA;                        // 00. Camera FOV in degrees
  arguments << cg;                        // 01. Camera globe
  arguments << sl;                        // stud logo

//  QString a  = QString("-AutoCrop=1");
//  if (!enableIM)
//    arguments.insert(2,a);                // 02. AutoCrop On if IM Off

  // additional LDView parameters;
  QStringList cgl,ldviewParmslist;
  ldviewParmslist = meta.LPub.assem.ldviewParms.value().split(' ');
  for (int i = 0; i < ldviewParmslist.size(); i++) {
    if (ldviewParmslist[i] != "" && ldviewParmslist[i] != " ") {
        if (pp) {
          if ((pl = ldviewParmslist[i].contains("-DefaultLatLong=")))
            arguments.replace(arguments.indexOf(cg),ldviewParmslist[i]); // replace Camera globe
          if ((pf = ldviewParmslist[i].contains("-FOV=")))
            arguments.replace(arguments.indexOf(CA),ldviewParmslist[i]); // replace Camera CA
          if ((pd = ldviewParmslist[i].contains("-LDViewPerspectiveDistanceFactor="))) {
              bool ok;
              qreal _cdf = QStringList(ldviewParmslist[i].split("=")).last().toDouble(&ok);
              if (ok && _cdf < LP3D_CDF)
                  cdf = _cdf;
          }
          pz = ldviewParmslist[i].contains("-DefaultZoom=");
        }
      if (!pd && !pl && !pf)
        addArgument(arguments, ldviewParmslist[i]);    // 10. ldviewParms
    }
  }
  if (pp) {
    if (!pf)
      arguments.replace(arguments.indexOf(CA),df);    // replace CA with FOV
    if (!pz && pl)
      addArgument(arguments ,dz, "-DefaultZoom");     // add DefaultZoom if DefaultLatLon specified
    if (!pl && cdf < LP3D_CDF) {                      // update camera distance factor if custom value specified
      QStringList cgl;
      cgl = cg.split(",");
      cgl.replace(cgl.indexOf(cgl.last()),QString::number(cd * cdf,'f',0));
      arguments.replace(arguments.indexOf(cg),cgl.join(","));
    }
  }

  // Set alternate target position or use specified image size
  if ((!m.isEmpty() && !pl) || (useImageSize && m.isEmpty())){
    arguments.removeAt(arguments.indexOf(cg));      // remove camera globe
    QString dl;
    dl = QString("-DefaultLatLong=%1,%2")
                  .arg(noCA ? 0.0 : double(meta.LPub.assem.cameraAngles.value(0)))
                  .arg(noCA ? 0.0 : double(meta.LPub.assem.cameraAngles.value(1)));
    dz = QString("-DefaultZoom=%1").arg(double(meta.LPub.assem.modelScale.value()));
    addArgument(arguments, dl, "-DefaultLatLong");
    addArgument(arguments, dz, "-DefaultZoom");

    // Set zoom to fit when image size specified
    if (useImageSize && m.isEmpty()){
      QString sz = QString("-SaveZoomToFit=1");
      addArgument(arguments, sz, "SaveZoomToFit");
    }
  }

  arguments << m;                         // 03. model origin for the camera to look at
  arguments << w;                         // 04. SaveWidth
  arguments << h;                         // 05. SaveHeight
  arguments << f;                         // 06. SaveSnapshot/SaveSnapshots/SaveSnapshotsList
  arguments << l;                         // 07. LDrawDir
  arguments << o;                         // 08. HaveStdOut
  arguments << v;                         // 09. Verbose

  if (ldviewParmslist.size())
      emit gui->messageSig(LOG_INFO,QMessageBox::tr("LDView additional CSI renderer parameters: %1")
                           .arg(ldviewParmslist.join(" ")));

  bool hasLDViewIni = Preferences::ldviewIni != "";
  QString ini;
  if(hasLDViewIni){
      ini  = QString("-IniFile=%1") .arg(Preferences::ldviewIni);
      addArgument(arguments, ini, "-IniFile", 0, ldviewParmslist.size());        // 11. LDView.ini
  }

  QString altldc;
  if (!Preferences::altLDConfigPath.isEmpty()) {
      altldc = QString("-LDConfig=%1").arg(Preferences::altLDConfigPath); 
      addArgument(arguments, altldc, "-LDConfig", 0, ldviewParmslist.size());    // 12.Alternate LDConfig
  }

  if (haveLdrNames) {
      if (useLDViewSCall()) {
          //-SaveSnapShots=1
          if ((!useLDViewSList()) || (useLDViewSList() && ldrNames.size() < SNAPSHOTS_LIST_THRESHOLD))
              arguments = arguments + ldrNames;  // 13. LDR input file(s)
      } else {
          // SaveSnapShot=1
          arguments << QDir::toNativeSeparators(ldrNames.first());

      }

      removeEmptyStrings(arguments);

      emit gui->messageSig(LOG_STATUS, "Executing LDView render CSI - please wait...");

      // execute LDView process
      if (executeLDViewProcess(arguments, Options::CSI) != 0) // ldrName entries that ARE NOT IM exist - e.g. first step
          return -1;
  }

  // Build IM arguments and process IM [Not implemented - not updated with perspective 'pp' routines]
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

      removeEmptyStrings(arguments);

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
  int                pliType,
  int                keySub)
{
  // Select meta type
  PliMeta &metaType = pliType == SUBMODEL ? static_cast<PliMeta&>(meta.LPub.subModel) :
                      pliType == BOM ? meta.LPub.bom : meta.LPub.pli;

  QFileInfo fileInfo(ldrNames.first());
  if ( ! fileInfo.exists()) {
    emit gui->messageSig(LOG_ERROR,QMessageBox::tr("File ldrNames does not exist!"));
    return -1;
  }

  QString tempPath = QDir::currentPath() + QDir::separator() + Paths::tmpDir;
  QString partsPath = QDir::currentPath() + QDir::separator() + (pliType == SUBMODEL ? Paths::submodelDir : Paths::partsDir);

  // Populate render attributes
  QString transform  = metaType.rotStep.value().type;
  bool  noCA         = transform  == "ABS";
  float modelScale   = metaType.modelScale.value();
  float cameraFov    = metaType.cameraFoV.value();
  float cameraAngleX = metaType.cameraAngles.value(0);
  float cameraAngleY = metaType.cameraAngles.value(1);
  xyzVector target   = xyzVector(metaType.target.x(),metaType.target.y(),metaType.target.z());

  /* determine camera distance */
  int cd = int(metaType.cameraDistance.value());
  if (cd){
      cd = int(cd*0.775*1700/1000);
  } else {
      cd = int(cameraDistance(meta,modelScale)*1700/1000);
  }

  //qDebug() << "LDView (Default) Camera Distance: " << cd;

  bool pp    = Preferences::perspectiveProjection;
  if (pliType == SUBMODEL)
      noCA   = Preferences::applyCALocally || noCA;

  // projection settings
  qreal cdf = LP3D_CDF;
  bool pl = false, pf = false, pz = false, pd = false;
  QString dl;
  QString df = QString("-FOV=%1").arg(double(cameraFov));
  QString dz = QString("-DefaultZoom=%1").arg(double(metaType.modelScale.value()));

  // additional LDView parameters;
  QStringList ldviewParmsArgs,ldviewParmslist;
  ldviewParmslist = metaType.ldviewParms.value().split(' ');
  for (int i = 0; i < ldviewParmslist.size(); i++) {
    if (ldviewParmslist[i] != "" && ldviewParmslist[i] != " ") {
      if (pp) {
        if ((pl = ldviewParmslist[i].contains("-DefaultLatLong=")))
             dl = ldviewParmslist[i];
        if ((pz = ldviewParmslist[i].contains("-DefaultZoom=")))
             dz = ldviewParmslist[i];
        if ((pf = ldviewParmslist[i].contains("-FOV=")))
             df = ldviewParmslist[i];
        if ((pd = ldviewParmslist[i].contains("-LDViewPerspectiveDistanceFactor="))) {
          bool ok;
          qreal _cdf = QStringList(ldviewParmslist[i].split("=")).last().toDouble(&ok);
          if (ok && _cdf < LP3D_CDF)
              cdf = _cdf;
        }
      }
      if (!pd && !pl && !pf && !pz)
        addArgument(ldviewParmsArgs, ldviewParmslist[i]);    // 10. ldviewParms [usually empty]
    }
  }

  // Set camera angle and camera globe and update arguments with perspective projection settings
  QString CA = pp ? QString("-FOV=%1") .arg(double(cameraFov)) :
                    QString("-ca%1") .arg(LP3D_CA);                   // replace CA with FOV
  QString cg = pp ? pl ? QString("-DefaultLatLong=%1,%2%3")
                                 .arg(noCA ? 0.0 : double(cameraAngleX))
                                 .arg(noCA ? 0.0 : double(cameraAngleY))
                                 .arg(dz)                             // replace Camera Globe with DefaultLatLon and add DefaultZoom
                       : QString("-cg%1,%2,%3")
                                 .arg(noCA ? 0.0 : double(cameraAngleX))
                                 .arg(noCA ? 0.0 : double(cameraAngleY))
                                 .arg(QString::number(cd * cdf,'f',0) )
                  : QString("-cg%1,%2,%3")
                            .arg(noCA ? 0.0 : double(cameraAngleX))
                            .arg(noCA ? 0.0 : double(cameraAngleY))
                            .arg(cd);

  /* Create the PLI DAT file(s) */

  QStringList attributes;
  auto processAttributes = [this, &meta, &keySub, &pp, &pl, &dz, &cdf] (
          QStringList &attributes,
          xyzVector   &target,
          bool        &noCA,
          int         &cd,
          QString     &CA,
          QString     &cg,
          float       &modelScale,
          float       &cameraFov,
          float       &cameraAngleX,
          float       &cameraAngleY) {
      bool hr;
      if ((hr = attributes.size() == nHasRotstep) || attributes.size() == nHasTargetAndRotstep)
        noCA = attributes.at(hr ? nRotTrans : nRot_Trans) == "ABS";
      if (attributes.size() >= nHasTarget)
        target       = xyzVector(attributes.at(nTargetX).toFloat(),attributes.at(nTargetY).toFloat(),attributes.at(nTargetZ).toFloat());
      if (keySub > PliBeginSub2Rc) {
        modelScale   = attributes.at(nModelScale).toFloat();
        cameraFov    = attributes.at(nCameraFoV).toFloat();
        cameraAngleX = attributes.at(nCameraAngleXX).toFloat();
        cameraAngleY = attributes.at(nCameraAngleYY).toFloat();
      }
      cd = int(cameraDistance(meta,modelScale)*1700/1000);

      CA = pp ? QString("-FOV=%1").arg(double(cameraFov)) : CA;
      cg = pp ? pl ? QString("-DefaultLatLong=%1,%2%3")
                                       .arg(noCA ? 0.0 : double(cameraAngleX))
                                       .arg(noCA ? 0.0 : double(cameraAngleY))
                                       .arg(dz)                             // replace Camera Globe with DefaultLatLon and add DefaultZoom
                             : QString("-cg%1,%2,%3")
                                       .arg(noCA ? 0.0 : double(cameraAngleX))
                                       .arg(noCA ? 0.0 : double(cameraAngleY))
                                       .arg(QString::number(cd * cdf,'f',0) )
                        : QString("-cg%1,%2,%3")
                                  .arg(noCA ? 0.0 : double(cameraAngleX))
                                  .arg(noCA ? 0.0 : double(cameraAngleY))
                                  .arg(cd);
  };

  QString f;
  bool useSnapshotArgs = false;
  if (useLDViewSCall() && pliType != SUBMODEL) {  // SingleCall
      if (keySub) {
          // process substitute attributes
          QStringList snapShotArgs,snapShotsLdrs;
          foreach (QString ldrName,ldrNames) {
              if (!QFileInfo(ldrName).exists()) {
                  emit gui->messageSig(LOG_ERROR, QString("LDR file %1 not found.").arg(ldrName));
                  continue;
              }

              attributes = getSubAttributes(ldrName);
              if (attributes.endsWith("SUB")) { // based on keySub
                  processAttributes(attributes, target, noCA, cd, CA, cg, modelScale, cameraFov, cameraAngleX, cameraAngleY);
                  QString pngName = QString(ldrName).replace("_SUB.ldr",".png");
                  snapShotArgs.append(QString("%1 %2 -SaveSnapShot=%3 %4").arg(CA).arg(cg).arg(pngName).arg(ldrName));
              } else {
                  snapShotsLdrs.append(ldrName);
              }
          }

          QString snapShotsCmdArgs,snapShotsArgs;
          if (snapShotsLdrs.size()) {
              if (snapShotsLdrs.size() < SNAPSHOTS_LIST_THRESHOLD) {

                  snapShotsArgs = QString("-SaveSnapShots=1 %1").arg(snapShotsLdrs.join(" "));
                  snapShotsCmdArgs = QString("%1 %2 %3").arg(CA).arg(cg).arg(snapShotsArgs);

              } else {

                  QString SnapshotsList = tempPath + QDir::separator() + "pliSnapshotsList.lst";
                  if (!createSnapshotsList(snapShotsLdrs,SnapshotsList))
                      return -1;
                  snapShotsArgs = QString("-SaveSnapshotsList=1 %1").arg(SnapshotsList);
                  snapShotsCmdArgs = QString("%1 %2 %3").arg(CA).arg(cg).arg(snapShotsArgs);
              }
          }

          // we have commandline snapShotArgs so create a list file
          if ((useSnapshotArgs = snapShotArgs.size())) {

              QString CommandLinesList = tempPath + QDir::separator() + "pliCommandLinesList.lst";
              QFile CommandLinesListFile(CommandLinesList);
              if ( ! CommandLinesListFile.open(QFile::WriteOnly | QFile::Text)) {
                  emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Failed to create LDView (Single Call) PLI CommandLines list file!"));
                  return -1;
              }

              QTextStream out(&CommandLinesListFile);
              if (snapShotsLdrs.size())
                  out << snapShotsCmdArgs << endl;
              foreach (QString argsLine,snapShotArgs)
                  out << argsLine << endl;
              CommandLinesListFile.close();

              f  = QString("-CommandLinesList=%1").arg(CommandLinesList);    // run in renderCsi
          }
          // otherwise just use the saveSnapShots argument
          else {

              f  = snapShotsArgs;
          }

      } else if (!useLDViewSList() || (useLDViewSList() && ldrNames.size() < SNAPSHOTS_LIST_THRESHOLD)) {

          f  = QString("-SaveSnapShots=1");
      } else {

          QString SnapshotsList = tempPath + QDir::separator() + "pliSnapshotsList.lst";
          if (!createSnapshotsList(ldrNames,SnapshotsList))
              return -1;
          f  = QString("-SaveSnapshotsList=%1").arg(SnapshotsList);    // run in renderCsi
      }

  } else {
      if (keySub) {
          // process substitute attributes
          attributes = getSubAttributes(pngName);
          processAttributes(attributes, target, noCA, cd, CA, cg, modelScale, cameraFov, cameraAngleX, cameraAngleY);
      }

      f  = QString("-SaveSnapShot=%1") .arg(pngName);
  }

  /* page size */
  bool useImageSize = metaType.imageSize.value(0) > 0;
  int width  = useImageSize ? int(metaType.imageSize.value(0)) : gui->pageSize(meta.LPub.page, 0);
  int height = useImageSize ? int(metaType.imageSize.value(1)) : gui->pageSize(meta.LPub.page, 1);

  QString m  = target.isPopulated() ?
               QString("-co%1,%2,%3")
                       .arg(double(target.x))
                       .arg(double(target.y))
                       .arg(double(target.z)) : QString();

  QString sl = metaType.studLogo.value() ?
                         QString("-StudLogo=%1")
                                 .arg(metaType.studLogo.value()) : QString();

  QString w  = QString("-SaveWidth=%1")  .arg(width);
  QString h  = QString("-SaveHeight=%1") .arg(height);
  QString l  = QString("-LDrawDir=%1")   .arg(Preferences::ldrawLibPath);
  QString o  = QString("-HaveStdOut=1");
  QString v  = QString("-vv");

  QStringList arguments;
  if (!useSnapshotArgs){
    arguments << CA;
    arguments << cg;
  }

  // append additional LDView parameters
  if (ldviewParmsArgs.size()) {
    for (int i = 0; i < ldviewParmsArgs.size(); i++) {
      addArgument(arguments, ldviewParmsArgs[i]);
    }
    emit gui->messageSig(LOG_INFO,QMessageBox::tr("LDView additional PLI renderer parameters: %1")
                         .arg(ldviewParmsArgs.join(" ")));
  }

  arguments << sl;
  arguments << m;

  // Set alternate target position or use specified image size
  if ((!m.isEmpty() && !pl) || (useImageSize && m.isEmpty())){
    arguments.removeAt(arguments.indexOf(cg));      // remove camera globe
    QString dl;
    dl = QString("-DefaultLatLong=%1,%2")
                  .arg(noCA ? 0.0 : double(metaType.cameraAngles.value(0)))
                  .arg(noCA ? 0.0 : double(metaType.cameraAngles.value(1)));
    dz = QString("-DefaultZoom=%1").arg(double(metaType.modelScale.value()));
    addArgument(arguments, dl, "-DefaultLatLong");
    addArgument(arguments, dz, "-DefaultZoom");

    // Set zoom to fit when image size specified
    if (useImageSize && m.isEmpty()){
      QString sz = QString("-SaveZoomToFit=1");
      addArgument(arguments, sz, "SaveZoomToFit");
    }
  }

  arguments << w;
  arguments << h;
  arguments << f;
  arguments << l;
  arguments << o;
  arguments << v;

  if(Preferences::ldviewIni != ""){
      QString ini;
      ini = QString("-IniFile=%1") .arg(Preferences::ldviewIni);
      addArgument(arguments, ini, "-IniFile", 0, ldviewParmsArgs.size());
  }

  QString altldc;
  if (!Preferences::altLDConfigPath.isEmpty()) {
      altldc = QString("-LDConfig=%1").arg(Preferences::altLDConfigPath);
      addArgument(arguments, altldc, "-LDConfig", 0, ldviewParmsArgs.size());
  }

  if (useLDViewSCall() && pliType != SUBMODEL) {
      //-SaveSnapShots=1
      if (!keySub && (!useLDViewSList() || (useLDViewSList() && ldrNames.size() < SNAPSHOTS_LIST_THRESHOLD)))
          arguments = arguments + ldrNames;  // 13. LDR input file(s)
  } else {
      //-SaveSnapShot=%1
      arguments << ldrNames.first();
  }

  removeEmptyStrings(arguments);

  emit gui->messageSig(LOG_STATUS, "Executing LDView render PLI - please wait...");

  // execute LDView process
  if (executeLDViewProcess(arguments, Options::PLI) != 0)
      return -1;

  // move generated PLI images to parts subfolder
  if (useLDViewSCall() && pliType != SUBMODEL){
      foreach(QString ldrName, ldrNames){
          QString pngFileTmpPath = ldrName.endsWith("_SUB.ldr") ?
                                   ldrName.replace("_SUB.ldr",".png") :
                                   ldrName.replace(".ldr",".png");
          QString pngFilePath = partsPath + QDir::separator() + QFileInfo(pngFileTmpPath).fileName();
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
  return stdCameraDistance(meta,scale);
}

int Native::renderCsi(
  const QString     &addLine,
  const QStringList &csiParts,
  const QStringList &csiKeys,
  const QString     &pngName,
        Meta        &meta,
  int                nType)
{
  QString ldrName     = QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";
  float lineThickness = (float(resolution()/Preferences::highlightStepLineWidth));

  // process native settings
  int distanceFactor   = meta.LPub.nativeCD.factor.value();
  int studLogo         = meta.LPub.assem.studLogo.value();
  float camDistance    = meta.LPub.assem.cameraDistance.value();
  float cameraAngleX   = meta.LPub.assem.cameraAngles.value(0);
  float cameraAngleY   = meta.LPub.assem.cameraAngles.value(1);
  float modelScale     = meta.LPub.assem.modelScale.value();
  float cameraFoV      = meta.LPub.assem.cameraFoV.value();
  float zNear          = meta.LPub.assem.znear.value();
  float zFar           = meta.LPub.assem.zfar.value();
  bool  isOrtho        = meta.LPub.assem.isOrtho.value();
  QString cameraName   = meta.LPub.assem.cameraName.value();
  xyzVector target     = xyzVector(meta.LPub.assem.target.x(),meta.LPub.assem.target.y(),meta.LPub.assem.target.z());
  if (nType == NTypeCalledOut) {
    studLogo           = meta.LPub.callout.csi.studLogo.value();
    camDistance        = meta.LPub.callout.csi.cameraDistance.value();
    cameraAngleX       = meta.LPub.callout.csi.cameraAngles.value(0);
    cameraAngleY       = meta.LPub.callout.csi.cameraAngles.value(1);
    modelScale         = meta.LPub.callout.csi.modelScale.value();
    cameraFoV          = meta.LPub.callout.csi.cameraFoV.value();
    zNear              = meta.LPub.callout.csi.znear.value();
    zFar               = meta.LPub.callout.csi.zfar.value();
    isOrtho            = meta.LPub.callout.csi.isOrtho.value();
    cameraName         = meta.LPub.callout.csi.cameraName.value();
    target             = xyzVector(meta.LPub.callout.csi.target.x(),meta.LPub.callout.csi.target.y(),meta.LPub.callout.csi.target.z());
  } else if (nType == NTypeMultiStep) {
    studLogo           = meta.LPub.multiStep.csi.studLogo.value();
    camDistance        = meta.LPub.multiStep.csi.cameraDistance.value();
    cameraAngleX       = meta.LPub.multiStep.csi.cameraAngles.value(0);
    cameraAngleY       = meta.LPub.multiStep.csi.cameraAngles.value(1);
    modelScale         = meta.LPub.multiStep.csi.modelScale.value();
    cameraFoV          = meta.LPub.multiStep.csi.cameraFoV.value();
    zNear              = meta.LPub.multiStep.csi.znear.value();
    zFar               = meta.LPub.multiStep.csi.zfar.value();
    isOrtho            = meta.LPub.multiStep.csi.isOrtho.value();
    cameraName         = meta.LPub.multiStep.csi.cameraName.value();
    target             = xyzVector(meta.LPub.multiStep.csi.target.x(),meta.LPub.multiStep.csi.target.y(),meta.LPub.multiStep.csi.target.z());
  }

  // Camera Angles always passed to Native renderer except if ABS rotstep
  QString rotstepType      = meta.rotStep.value().type;
  bool noCA = rotstepType == "ABS";
  bool useImageSize = meta.LPub.assem.imageSize.value(0) > 0;

  // Renderer options
  NativeOptions *Options    = new NativeOptions();
  Options->ImageType         = Options::CSI;
  Options->InputFileName     = ldrName;
  Options->OutputFileName    = pngName;
  Options->StudLogo          = studLogo;
  Options->Resolution        = resolution();
  Options->ImageWidth        = useImageSize ? int(meta.LPub.assem.imageSize.value(0)) : gui->pageSize(meta.LPub.page, 0);
  Options->ImageHeight       = useImageSize ? int(meta.LPub.assem.imageSize.value(1)) : gui->pageSize(meta.LPub.page, 1);
  Options->IsOrtho           = isOrtho;
  Options->CameraName        = cameraName;
  Options->FoV               = cameraFoV;
  Options->ZNear             = zNear;
  Options->ZFar              = zFar;
  Options->Latitude          = noCA ? 0.0 : cameraAngleX;
  Options->Longitude         = noCA ? 0.0 : cameraAngleY;
  Options->Target            = target;
  Options->ModelScale        = modelScale;
  Options->NativeCDF         = distanceFactor;
  Options->CameraDistance    = camDistance > 0 ? camDistance : cameraDistance(meta,modelScale);
  Options->LineWidth         = lineThickness;
  Options->UsingViewpoint    = gApplication->mPreferences.mNativeViewpoint <= 6;
  Options->HighlightNewParts = gui->suppressColourMeta(); //Preferences::enableHighlightStep;

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
              Options->ExportMode = int(EXPORT_3DS_MAX);
              Options->ExportFileName = QDir::toNativeSeparators(outPath+"/"+baseName+".3ds");
              break;
          case EXPORT_STL:
              Options->ExportMode = int(EXPORT_STL);
              Options->ExportFileName = QDir::toNativeSeparators(outPath+"/"+baseName+".stl");
              break;
          case EXPORT_POVRAY:
              Options->ExportMode = int(EXPORT_POVRAY);
              Options->ExportFileName = QDir::toNativeSeparators(outPath+"/"+baseName+".pov");
              break;
          case EXPORT_COLLADA:
              Options->ExportMode = int(EXPORT_COLLADA);
              Options->ExportFileName = QDir::toNativeSeparators(outPath+"/"+baseName+".dae");
              ldvExport = false;
              break;
          case EXPORT_WAVEFRONT:
              Options->ExportMode = int(EXPORT_WAVEFRONT);
              Options->ExportFileName = QDir::toNativeSeparators(outPath+"/"+baseName+".obj");
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
                  Options->IniFlag = gui->exportMode == EXPORT_POVRAY ? NativePOVIni :
                                    gui->exportMode == EXPORT_STL ? NativeSTLIni : Native3DSIni;
                  /*  Options->IniFlag = gui->exportMode == EXPORT_POVRAY ? NativePOVIni :
                                    gui->exportMode == EXPORT_STL ? NativeSTLIni : EXPORT_HTML; */
              }

              ldrName = QDir::currentPath() + "/" + Paths::tmpDir + "/exportcsi.ldr";

              // rotate parts for ldvExport - apply camera angles
              int rc;
              if ((rc = rotateParts(addLine, meta.rotStep, csiParts, ldrName, QString(),meta.LPub.assem.cameraAngles,ldvExport)) < 0) {
                  return rc;
              }

              /* determine camera distance */
              int cd = int(meta.LPub.assem.cameraDistance.value());
              if (cd){
                  cd = int(cd*0.775*1700/1000);
              } else {
                  cd = int(cameraDistance(meta,meta.LPub.assem.modelScale.value())*1700/1000);
              }

              /* apply camera angles */
              noCA  = Preferences::applyCALocally || noCA;
              //bool pp      = Preferences::perspectiveProjection;

              QString CA = QString("-ca%1") .arg(/*pp ? Options->FoV : */ 0.01);  // Effectively defaults to orthographic projection.
              QString cg = QString("-cg%1,%2,%3")
                      .arg(noCA ? 0.0 : double(meta.LPub.assem.cameraAngles.value(0)))
                      .arg(noCA ? 0.0 : double(meta.LPub.assem.cameraAngles.value(1)))
                      .arg(cd);

              QString w  = QString("-SaveWidth=%1") .arg(double(Options->ImageWidth));
              QString h  = QString("-SaveHeight=%1") .arg(double(Options->ImageHeight));
              QString f  = QString("-ExportFile=%1") .arg(Options->ExportFileName);
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

              removeEmptyStrings(arguments);

              Options->ExportArgs = arguments;
          }
      } else {
          Options->ExportMode = EXPORT_NONE;
      }
  }

  if (!RenderNativeImage(Options)) {
      return -1;
  }

  return 0;
}

int Native::renderPli(
  const QStringList &ldrNames,
  const QString     &pngName,
  Meta              &meta,
  int               pliType,
  int               keySub)
{
  // Select meta type
  PliMeta &metaType = pliType == SUBMODEL ? static_cast<PliMeta&>(meta.LPub.subModel) :
                      pliType == BOM ? meta.LPub.bom : meta.LPub.pli;

  // Populate render attributes
  int distanceFactor   = meta.LPub.nativeCD.factor.value();
  int studLogo         = metaType.studLogo.value();
  float camDistance    = metaType.cameraDistance.value();
  float modelScale     = metaType.modelScale.value();
  float cameraFov      = metaType.cameraFoV.value();
  float cameraAngleX   = metaType.cameraAngles.value(0);
  float cameraAngleY   = metaType.cameraAngles.value(1);
  float zNear          = metaType.znear.value();
  float zFar           = metaType.zfar.value();
  bool  isOrtho        = metaType.isOrtho.value();
  QString cameraName   = metaType.cameraName.value();
  xyzVector target     = xyzVector(metaType.target.x(),metaType.target.y(),metaType.target.z());
  bool useImageSize    = metaType.imageSize.value(0) > 0;

  // Camera Angles always passed to Native renderer except if ABS rotstep
  bool  noCA          = metaType.rotStep.value().type  == "ABS";

  // Process substitute part attributes
  if (keySub) {
    QStringList attributes = getSubAttributes(pngName);
    bool hr;
    if ((hr = attributes.size() == nHasRotstep) || attributes.size() == nHasTargetAndRotstep)
      noCA = attributes.at(hr ? nRotTrans : nRot_Trans) == "ABS";
    if (attributes.size() >= nHasTarget)
      target = xyzVector(attributes.at(nTargetX).toFloat(),attributes.at(nTargetY).toFloat(),attributes.at(nTargetZ).toFloat());
    if (keySub > PliBeginSub2Rc) {
      modelScale   = attributes.at(nModelScale).toFloat();
      cameraFov    = attributes.at(nCameraFoV).toFloat();
      cameraAngleX = attributes.at(nCameraAngleXX).toFloat();
      cameraAngleY = attributes.at(nCameraAngleYY).toFloat();
    }
  }

  // Renderer options
  NativeOptions *Options = new NativeOptions();
  Options->ImageType      = Options::PLI;
  Options->InputFileName  = ldrNames.first();
  Options->OutputFileName = pngName;
  Options->StudLogo       = studLogo;
  Options->Resolution     = resolution();
  Options->ImageWidth     = useImageSize ? int(metaType.imageSize.value(0)) : gui->pageSize(meta.LPub.page, 0);
  Options->ImageHeight    = useImageSize ? int(metaType.imageSize.value(1)) : gui->pageSize(meta.LPub.page, 1);
  Options->IsOrtho        = isOrtho;
  Options->CameraName     = cameraName;
  Options->FoV            = cameraFov;
  Options->ZNear          = zNear;
  Options->ZFar           = zFar;
  Options->Latitude       = noCA ? 0.0 : cameraAngleX;
  Options->Longitude      = noCA ? 0.0 : cameraAngleY;
  Options->Target         = target;
  Options->ModelScale     = modelScale;
  Options->NativeCDF      = distanceFactor;
  Options->CameraDistance = camDistance > 0 ? camDistance : cameraDistance(meta,modelScale);
  Options->LineWidth      = HIGHLIGHT_LINE_WIDTH_DEFAULT;
  Options->UsingViewpoint = gApplication->mPreferences.mNativeViewpoint <= 6;

  // Set PLI project
  Project* PliImageProject = new Project();
  gApplication->SetProject(PliImageProject);

  // Render image
  emit gui->messageSig(LOG_STATUS, "Rendering Native PLI image - please wait...");

  if (!RenderNativeImage(Options)) {
      return -1;
  }

  return 0;
}

float Render::ViewerCameraDistance(
  Meta &meta,
  float scale)
{
    return stdCameraDistance(meta,scale);
}

bool Render::ExecuteViewer(const NativeOptions *O, bool Export/*false*/){

    lcGetActiveProject()->SetRenderAttributes(
                O->ImageType,
                O->ImageWidth,
                O->ImageHeight,
                Export ? O->ImageWidth : O->PageWidth,
                Export ? O->ImageHeight : O->PageHeight,
                O->ImageFileName,
                O->Resolution,
                O->ModelScale,
                O->NativeCDF);

    lcGetPiecesLibrary()->SetStudLogo(O->StudLogo,true);

    if (Preferences::debugLogging){
        QStringList arguments;
        if (Export) {
            arguments << (O->InputFileName.isEmpty() ? QString() : QString("InputFileName: %1").arg(O->InputFileName));
            arguments << (O->OutputFileName.isEmpty() ? QString() : QString("OutputFileName: %1").arg(O->OutputFileName));
            arguments << (O->ExportFileName.isEmpty() ? QString() : QString("ExportFileName: %1").arg(O->ExportFileName));
            arguments << (O->IniFlag == -1 ? QString() : QString("IniFlag: %1").arg(iniFlagNames[O->IniFlag]));
            arguments << QString("ExportMode: %1").arg(nativeExportNames[O->ExportMode]);
            arguments << QString("ExportArgs: %1").arg(O->ExportArgs.size() ? O->ExportArgs.join(" ") : QString());
            arguments << QString("LineWidth: %1").arg(double(O->LineWidth));
            arguments << QString("TransBackground: %1").arg(O->TransBackground ? "True" : "False");
            arguments << QString("HighlightNewParts: %1").arg(O->HighlightNewParts ? "True" : "False");
        } else {
            arguments << QString("ViewerStepKey: %1").arg(O->ViewerStepKey);
            arguments << (O->ImageFileName.isEmpty() ? QString() : QString("ImageFileName: %1").arg(O->ImageFileName));
            arguments << QString("PageWidth: %1").arg(O->PageWidth);
            arguments << QString("PageHeight: %1").arg(O->PageHeight);
            arguments << QString("RotStep: X(%1) Y(%2) Z(%3) %4").arg(double(O->RotStep.x)).arg(double(O->RotStep.y)).arg(double(O->RotStep.z)).arg(O->RotStepType);
        }
        arguments << QString("StudLogo: %1").arg(O->StudLogo);
        arguments << QString("Resolution: %1").arg(double(O->Resolution));
        arguments << QString("ImageWidth: %1").arg(O->ImageWidth);
        arguments << QString("ImageHeight: %1").arg(O->ImageHeight);
        arguments << QString("UsingViewpoint: %1").arg(O->UsingViewpoint ? "True" : "False");
        arguments << QString("ModelScale: %1").arg(double(O->ModelScale));
        arguments << QString("CameraFoV: %1").arg(double(O->FoV));
        arguments << QString("CameraZNear: %1").arg(double(O->ZNear));
        arguments << QString("CameraZNear: %1").arg(double(O->ZFar));
        arguments << QString("CameraDistance: %1").arg(double(O->CameraDistance),0,'f',0);
        arguments << QString("CameraNativeCDF: %1").arg(O->NativeCDF);
        arguments << QString("CameraProjection: %1").arg(O->IsOrtho ? "Orthographic" : "Perspective");
        arguments << QString("CameraName: %1").arg(O->CameraName.isEmpty() ? "Default" : O->CameraName);
        arguments << QString("CameraLatitude: %1").arg(double(O->Latitude));
        arguments << QString("CameraLongitude: %1").arg(double(O->Longitude));
        arguments << QString("CameraTarget: X(%1) Y(%2) Z(%3)").arg(double(O->Target.x)).arg(double(O->Target.y)).arg(double(O->Target.z));

        removeEmptyStrings(arguments);

        QString message = QString("%1 %2 Arguments: %3")
                                .arg(Export ? "Native Renderer" : "3DViewer")
                                .arg(O->ImageType == Options::CSI ? "CSI" : O->ImageType == Options::PLI ? "PLI" : "SMP")
                                .arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
      qDebug() << qPrintable(message) << "\n";
#else
      emit gui->messageSig(LOG_INFO, message);
      emit gui->messageSig(LOG_INFO, QString());
#endif
    }

    if (!Export)
        gMainWindow->GetPartSelectionWidget()->SetDefaultPart();

    View* ActiveView = gMainWindow->GetActiveView();

    lcModel* ActiveModel = ActiveView->GetActiveModel();

    lcCamera* Camera =  ActiveView->mCamera;

    lcStep CurrentStep = ActiveModel->GetCurrentStep();

    // LeoCAD flips Y an Z axis so that Z is up and Y represents depth
    lcVector3 Target = lcVector3(O->Target.x,O->Target.z,O->Target.y);

    if (O->UsingViewpoint) {   // ViewPoints (Front, Back, Top, Bottom, Left, Right, Home)
        ActiveView->SetViewpoint(lcViewpoint(gApplication->mPreferences.mNativeViewpoint));
    } else {                  // Default View (Angles + Distance + Perspective|Orthographic)
        auto validCameraValue = [&O, &Camera] (const CamFlag flag)
        {
            if (Preferences::usingNativeRenderer)
                return flag == DefFoV ?
                            Camera->m_fovy :
                       flag == DefZNear ?
                            Camera->m_zNear : Camera->m_zFar;

            float result;
            switch (flag)
            {
            case DefFoV:
                // e.g.  0.01  + 30.0           - 0.01
                result = O->FoV + Camera->m_fovy - CAMERA_FOV_DEFAULT;
                break;
            case DefZNear:
                // e.g.     10.0 +            25.0 - 10.0
                result = O->ZNear + Camera->m_zNear - CAMERA_ZNEAR_DEFAULT;
                break;
            case DefZFar:
                // e.g.  4000.0 + 50000.0         - 4000.0
                result = O->ZFar + Camera->m_zFar  - CAMERA_ZFAR_DEFAULT;
                break;
            }

            return result;
        };

        if (Export) {
            Camera->m_fovy  = validCameraValue(DefFoV);
            Camera->m_zNear = validCameraValue(DefZNear);
            Camera->m_zFar  = validCameraValue(DefZFar);
        }

        bool NoCamera    = O->CameraName.isEmpty();
        bool IsOrtho     = NoCamera ? gApplication->mPreferences.mNativeProjection : O->IsOrtho;
        bool ZoomExtents = !Export && IsOrtho;

        ActiveView->SetProjection(IsOrtho);
        ActiveView->SetCameraGlobe(O->Latitude, O->Longitude, O->CameraDistance, Target, ZoomExtents);
    }

    ActiveView->MakeCurrent();
    lcContext* Context = ActiveView->mContext;
    View View(ActiveModel);
    View.SetCamera(Camera, false);
    View.SetHighlight(false);
    View.SetContext(Context);

    if (!Export) {
        if (lcGetPreferences().mDefaultCameraProperties)
            gMainWindow->UpdateDefaultCamera(Camera);
        return true;
    }

    // generate image
    const int ImageWidth  = int(O->ImageWidth);
    const int ImageHeight = int(O->ImageHeight);
    QString ImageType     = O->ImageType == Options::CSI ? "CSI" : O->ImageType == Options::CSI ? "PLI" : "SMP";

    bool rc = true;
    if (!(rc = View.BeginRenderToImage(ImageWidth, ImageHeight)))
    {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not begin RenderToImage for Native %1 image.<br>"
                                                       "Render framebuffer is not valid").arg(ImageType));
    }

    if (rc) {

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

        QImageWriter Writer(O->OutputFileName);

        if (Writer.format().isEmpty())
            Writer.setFormat("PNG");

        if (!Writer.write(QImage(Image.RenderedImage.copy(Image.Bounds))))
        {
            emit gui->messageSig(LOG_ERROR,QString("Could not write to Native %1 %2 file:<br>[%3].<br>Reason: %4.<br>"
                                                   "Ensure Field of View (default is 30) and Camera Distance Factor <br>"
                                                   "are configured for the Native Renderer")
                                                   .arg(ImageType)
                                                   .arg(O->ExportMode == EXPORT_NONE ?
                                                        QString("image") :
                                                        QString("%1 object")
                                                                .arg(nativeExportNames[gui->exportMode]))
                                                   .arg(O->OutputFileName)
                                                   .arg(Writer.errorString()));
            rc = false;
        }

        View.EndRenderToImage();
    }

    Context->ClearResources();

    ActiveModel->SetTemporaryStep(CurrentStep);

    if (!ActiveModel->mActive)
        ActiveModel->CalculateStep(LC_STEP_MAX);

    if (rc)
        emit gui->messageSig(LOG_INFO,QMessageBox::tr("Native %1 image file rendered '%2'")
                          .arg(ImageType).arg(O->OutputFileName));

    if (O->ExportMode != EXPORT_NONE) {
        if (!NativeExport(O)) {
            emit gui->messageSig(LOG_ERROR,QMessageBox::tr("%1 Objects render failed.").arg(ImageType));
            rc = false;
        }
    }

    return rc;
}

bool Render::RenderNativeImage(const NativeOptions *Options)
{
    if (! gMainWindow->OpenProject(Options->InputFileName))
        return false;

    return ExecuteViewer(Options,true/*exportImage*/);
}

bool Render::LoadViewer(const ViewerOptions *Options){

    QString viewerStepKey = Options->ViewerStepKey;

    Project* StepProject = new Project();
    if (LoadStepProject(StepProject, viewerStepKey)){
        gApplication->SetProject(StepProject);
        gMainWindow->UpdateAllViews();
    }
    else
    {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not load 3DViewer model file %1.")
                             .arg(viewerStepKey));
        delete StepProject;
        return false;
    }

    gui->setViewerStepKey(viewerStepKey);

    NativeOptions *derived = new NativeOptions(*Options);
    if (derived)
        return ExecuteViewer(derived);
    else
        return false;
}

bool Render::LoadStepProject(Project* StepProject, const QString& viewerStepKey)
{
    QString FileName = gui->getViewerStepFilePath(viewerStepKey);

    if (FileName.isEmpty())
    {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Did not receive 3DViewer CSI path for %1.").arg(FileName));
        return false;
    }

    QStringList CsiContent = gui->getViewerStepRotatedContents(viewerStepKey);
    if (CsiContent.isEmpty())
    {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Did not receive 3DViewer CSI content for %1.").arg(FileName));
        return false;
    }

#ifdef QT_DEBUG_MODE
    QString valueAt0 = viewerStepKey.at(0);
    bool inside = (valueAt0 == "\"");                                                 // true if the first character is "
    QStringList tmpList = viewerStepKey.split(QRegExp("\""), QString::SkipEmptyParts); // Split by "
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
    QString imageType   = outFileInfo.completeBaseName().replace(".ldr","");
    QString baseName    = QFileInfo(modelName).completeBaseName();
    QStringList argv02  = imageType == "pli" ? argv01[1].split(";") : QStringList();
    QString outfileName = QString("%1/viewer_%2_%3.ldr")
            .arg(outFileInfo.absolutePath())
            .arg(imageType)
            .arg(argv02.size() ? QString("%1_%2_%3").arg(baseName).arg(argv02.first()).arg(argv02.last()) : baseName);

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

    std::vector<lcModel*> UpdatedModels;

    for (lcModel* Model : StepProject->mModels)
    {
        Model->UpdateMesh();
        Model->UpdatePieceInfo(UpdatedModels);
    }

    StepProject->mModified = false;

    return true;
}

bool Render::NativeExport(const NativeOptions *Options) {

    QString exportModeName = nativeExportNames[Options->ExportMode];

    if (Options->ExportMode == EXPORT_WAVEFRONT ||
        Options->ExportMode == EXPORT_COLLADA   ||
        Options->ExportMode == EXPORT_CSV       ||
        Options->ExportMode == EXPORT_BRICKLINK /*||
        Options->ExportMode == EXPORT_3DS_MAX*/) {
        emit gui->messageSig(LOG_STATUS, QString("Native CSI %1 Export...").arg(exportModeName));
        Project* NativeExportProject = new Project();
        gApplication->SetProject(NativeExportProject);

        if (! gMainWindow->OpenProject(Options->InputFileName)) {
            emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Failed to open CSI %1 Export project")
                                                           .arg(exportModeName));
            delete NativeExportProject;
            return false;
        }
    }
    else
    {
        return doLDVCommand(Options->ExportArgs, Options->ExportMode);
    }

    if (Options->ExportMode == EXPORT_CSV)
    {
        lcGetActiveProject()->ExportCSV();
    }
    else
    if (Options->ExportMode == EXPORT_BRICKLINK)
    {
        lcGetActiveProject()->ExportBrickLink();
    }
    else
    if (Options->ExportMode == EXPORT_WAVEFRONT)
    {
        lcGetActiveProject()->ExportWavefront(Options->ExportFileName);
    }
    else
    if (Options->ExportMode == EXPORT_COLLADA)
    {
        lcGetActiveProject()->ExportCOLLADA(Options->ExportFileName);
    }

/*
    // These are executed through the LDV Native renderer

    if (Options->ExportMode == EXPORT_HTML)
    {
        lcHTMLExportOptions HTMLOptions(lcGetActiveProject());

        lcQHTMLDialog Dialog(gMainWindow, &HTMLOptions);
        if (Dialog.exec() != QDialog::Accepted)
            return false;

        HTMLOptions.SaveDefaults();

        //HTMLOptions.PathName = Options->ExportFileName;

        lcGetActiveProject()->ExportHTML(HTMLOptions);
    }
    else
    if (Options->ExportMode == EXPORT_POVRAY)
    {
        lcGetActiveProject()->ExportPOVRay(Options->ExportFileName);
    }
    else
    if (Options->ExportMode == EXPORT_3DS_MAX)
    {
        lcGetActiveProject()->Export3DStudio(Options->ExportFileName);
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

const QString Render::getPovrayRenderFileName(const QString &viewerStepKey)
{
    QDir povrayDir(QString("%1/%2").arg(QDir::currentPath()).arg(Paths::povrayRenderDir));
    if (!povrayDir.exists())
        Paths::mkPovrayDir();

    QString fileName = gui->getViewerConfigKey(viewerStepKey).replace(";","_");

    if (fileName.isEmpty()){
        emit gui->messageSig(LOG_ERROR, QString("Failed to receive ldrFileName for viewerStepKey : %1").arg(viewerStepKey));
       fileName = "imagerender";
    }

    QString imageFile = QDir::toNativeSeparators(QString("%1/%2.png")
                       .arg(povrayDir.absolutePath())
                       .arg(fileName));

    return imageFile;

}

// create Native version of the CSI/PLI file - consolidate subfiles and parts into single file
int Render::createNativeModelFile(
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
                  QString fadeSfx = QString("%1.").arg(FADE_SFX);
                  bool isFadedItem = type.contains(fadeSfx);
                  // Fade file
                  if (isFadedItem) {
                      customType = type;
                      customType = customType.replace(fadeSfx,".");
                      isCustomSubModel = gui->isSubmodel(customType);
                      isCustomPart = gui->isUnofficialPart(customType);
                    }
                }

              if (doHighlightStep) {
                  QString highlightSfx = QString("%1.").arg(HIGHLIGHT_SFX);
                  bool isHighlightItem = type.contains(highlightSfx);
                  // Highlight file
                  if (isHighlightItem) {
                      customType = type;
                      customType = customType.replace(highlightSfx,".");
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
          QString modelName = QFileInfo(csiSubModels[index]).completeBaseName().toLower();
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
                      QString fadeSfx = QString("%1.").arg(FADE_SFX);
                      bool isFadedItem = type.contains(fadeSfx);
                      // Fade file
                      if (isFadedItem) {
                          customType = type;
                          customType = customType.replace(fadeSfx,".");
                          isCustomSubModel = gui->isSubmodel(customType);
                          isCustomPart = gui->isUnofficialPart(customType);
                        }
                    }

                  if (doHighlightStep) {
                      QString highlightSfx = QString("%1.").arg(HIGHLIGHT_SFX);
                      bool isHighlightItem = type.contains(highlightSfx);
                      // Highlight file
                      if (isHighlightItem) {
                          customType = type;
                          customType = customType.replace(highlightSfx,".");
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
