/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2023 Trevor SANDY. All rights reserved.
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
#include "step.h"
#include "lpub_preferences.h"
#include "application.h"

#include <LDVQt/LDVWidget.h>
#include <LDVQt/LDVImageMatte.h>

#include "paths.h"

#include "lc_file.h"
#include "project.h"
#include "pieceinf.h"
#include "lc_profile.h"
#include "lc_model.h"
#include "lc_view.h"
#include "camera.h"
#include "lc_qhtmldialog.h"
#include "lc_partselectionwidget.h"

#include "lc_library.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

Render *renderer;

LDGLite ldglite;
LDView  ldview;
POVRay  povray;
Native  native;

const QString nativeExportNames[] =
{
  "",                // 0  PAGE_PROCESS
  "PDF",             // 1  EXPORT_PDF
  "PNG",             // 2  EXPORT_PNG
  "JPG",             // 3  EXPORT_JPG
  "BMP",             // 4  EXPORT_BMP
  "3DS MAX",         // 5  EXPORT_3DS_MAX
  "COLLADA",         // 6  EXPORT_COLLADA
  "WAVEFRONT",       // 7  EXPORT_WAVEFRONT
  "STL",             // 8  EXPORT_STL
  "POVRAY",          // 9  EXPORT_POVRAY
  "BRICKLINK",       // 10 EXPORT_BRICKLINK
  "CSV",             // 11 EXPORT_CSV
  "ELEMENT",         // 12 EXPORT_ELEMENT
  "HTML Parts",      // 13 EXPORT_HTML_PARTS
  "HTML Steps",      // 14 EXPORT_HTML_STEPS
  "POV-RAY Render",  // 15 RENDER_POVRAY
  "Blender Render",  // 16 BLENDER_RENDER
  "Blender Import",  // 17 BLENDER_IMPORT
  "Bill Of Material" // 18 GENERATE_BOM
};

//#define LduDistance 5729.57
//#define _CA "-ca0.01"
#define LP3D_CA 0.01
#define LP3D_CDF 1.0
#define USE_ALPHA "+UA"

#define SNAPSHOTS_LIST_THRESHOLD 3

static double pi = 4*atan(1.0);

// the default camera distance for real size
static float LduDistance = float(10.0/tan(0.005*pi/180));

bool notEqual(const double v1, const double v2, int p = 4)
{
    const QString _v1 = QString::number(v1,'f',p);
    const QString _v2 = QString::number(v2,'f',p);
    const bool     r  = _v1 != _v2;
    return r;
}

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
        QString message = QObject::tr("Couldn't get length of short path name length, lastError is %1, trying long path name").arg(GetLastError());
#ifdef QT_DEBUG_MODE
        qDebug() << qPrintable(message);
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
        QString message = QObject::tr("Couldn't get length of short path name length, lastError is %1, trying long path name").arg(GetLastError());
#ifdef QT_DEBUG_MODE
        qDebug() << qPrintable(message);
#else
        emit gui->messageSig(LOG_INFO, message);
#endif
        delete [] buffer;
        return dirNameIn;
    }

    QString dirNameOut = QString::fromWCharArray(buffer);

    delete [] buffer;
    return dirNameOut;
#else
        return dirNameIn;
#endif
}

int Render::getRenderer()
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

int Render::getDistanceRendererIndex()
{
    int renderer = 0; // RENDERER_NATIVE, RENDERER_LDGLITE
    if (getRenderer() == RENDERER_POVRAY)      // 1
        renderer = 1;
    else if (getRenderer() == RENDERER_LDVIEW) // 2
        renderer = 2;
    return renderer;
}

void Render::setRenderer(int id)
{
  if (id == RENDERER_LDGLITE)
  {
    renderer = &ldglite;
  }
  else
  if (id == RENDERER_LDVIEW)
  {
    renderer = &ldview;
  }
  else
  if (id == RENDERER_POVRAY)
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

int Render::setLDrawHeaderAndFooterMeta(QStringList &lines, const QString &_modelName, int type, int displayType)
{
    QStringList tokens;
    Options::Mt imageType = static_cast<Options::Mt>(type);
    DisplayType modelType = static_cast<DisplayType>(displayType);
    QString baseName = imageType == Options::SMI ? lpub->ldrawFile.description(_modelName) : QFileInfo(_modelName).completeBaseName();
    bool isMPD       = imageType == Options::SMI || imageType == Options::MON;  // always MPD if imageType is SMI or MON[o] image
    baseName         = QString("%1").arg(baseName.replace(baseName.indexOf(baseName.at(0)),1,baseName.at(0).toUpper()));

    // Test for MPD - if single subfile line
    if (!isMPD) {
        for (int i = 0; i < lines.size(); i++) {
            QString line = lines.at(i);
            split(line, tokens);
            if (tokens[0] == "1" && tokens.size() == 15) {
                QString type = tokens[tokens.size()-1];
                if (Preferences::enableFadeSteps) {
                    QString fadeSfx = QString("%1.").arg(FADE_SFX);
                    if (type.contains(fadeSfx)) {
                        type = type.replace(fadeSfx,".");
                        if ((isMPD = lpub->ldrawFile.isSubmodel(type) || lpub->ldrawFile.isUnofficialPart(type)))
                            break;
                    }
                }
                if (Preferences::enableHighlightStep) {
                    QString highlightSfx = QString("%1.").arg(HIGHLIGHT_SFX);
                    if (type.contains(highlightSfx)) {
                        type = type.replace(highlightSfx,".");
                        if ((isMPD = lpub->ldrawFile.isSubmodel(type) || lpub->ldrawFile.isUnofficialPart(type)))
                            break;
                    }
                }
                if ((isMPD = lpub->ldrawFile.isSubmodel(type) || lpub->ldrawFile.isUnofficialPart(type)))
                    break;
            }
        }
    }

    // special case where the modelName will match the line type name so we append '-Smi' to the modelName
    if (imageType == Options::SMI) {
         QString smi(SUBMODEL_IMAGE_BASENAME);
         baseName = baseName.append(QString("-%1").arg(smi.toUpper()));
    }

    // case where PLI is an MPD - i.e. LDCad generated part, append name to to workaround Visual Editor abend
    QString modelName = _modelName;
    if (imageType == Options::PLI && isMPD) {
        modelName.prepend("Pli_");
        baseName.prepend("Pli_");
    }

    // special case where model file is a display model or final step in fade step document
    if (modelType >= DT_MODEL_DEFAULT) {
        baseName = baseName.append("_Display_Model");
    }

    // description and name are already added to mono image
    if (imageType != Options::MON) {
        lines.prepend(QString("0 Name: %1").arg(modelName));
        lines.prepend(QString("0 %1").arg(baseName));
    }

    if (isMPD) {
        lines.prepend(QString("0 FILE %1").arg(modelName));
        lines.append("0 NOFILE");
    }

    return isMPD;
}

bool Render::useLDViewSCall() {
    return (Preferences::preferredRenderer == RENDERER_LDVIEW &&
            Preferences::enableLDViewSingleCall);
}

bool Render::useLDViewSList() {
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
        emit gui->messageSig(LOG_STATUS, QObject::tr("No opaque content in ").arg(pngName));
        return false;
    } else {
        clipBox.setCoords(minX, minY, maxX, maxY);
    }

    //save clipBox;
    QImage clippedImage = toClip.copy(clipBox);
    QString clipMsg = QObject::tr("%1 (w:%2 x h:%3)")
                                  .arg(pngName)
                                  .arg(clippedImage.width())
                                  .arg(clippedImage.height());

    QImageWriter Writer(QDir::toNativeSeparators(pngName));
    if (Writer.format().isEmpty())
            Writer.setFormat("PNG");

    if (Writer.write(clippedImage)) {
        emit gui->messageSig(LOG_STATUS, QObject::tr("Clipped image saved '%1'")
                                                     .arg(clipMsg));
    } else {
        emit gui->messageSig(LOG_ERROR, QObject::tr("Failed to save clipped image '%1': %2")
                                                    .arg(clipMsg)
                                                    .arg(Writer.errorString()));
        return false;
    }
    return true;
 }

void Render::addArgument(
        QStringList   &_arguments,
        const QString &_newArg,
        const QString &_argChk,
        const int      povGenerator,
        const int      additionalArgs,
        const bool     quoteNewArg) {

    QString newArg = _newArg;

    int insertIndex = -1;
    if (quoteNewArg) {
        insertIndex = newArg.indexOf("=");
        if (insertIndex < 0)
            newArg = QString("\"%1\"").arg(_newArg);
        else
            newArg = QString(_newArg).insert(insertIndex+1,"\"").append("\"");
    }

    if (!additionalArgs) {
        _arguments.append(newArg);
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

    insertIndex = -1;
    for (int i = 0; i < _arguments.size(); i++) {
        if (_arguments[i] != "" && _arguments[i] != " ") {
            if (_argChk.isEmpty()) {
                if (!(getRenderer() == RENDERER_POVRAY && !povGenerator)){
                    if (isMatch(_arguments[i]) ||
                        _arguments[i].startsWith(newArg.left(newArg.indexOf("=")))) {
                        insertIndex = _arguments.indexOf(_arguments[i]);
                        break;
                    }
                }
            } else if (_arguments[i].contains(_argChk)) {
                insertIndex = _arguments.indexOf(_arguments[i]);
                break;
            }
        }
    }

    if (insertIndex < 0) {
        _arguments.append(newArg);
    } else {
        _arguments.replace(insertIndex,newArg);
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
    factor   = LPub::pageSize(meta.LPub.page, XX)/onexone; // in pixels;
//#ifdef QT_DEBUG_MODE
//    logTrace() << "\n" << QString("DEBUG - STANDARD CAMERA DISTANCE")
//               << "\n" << QString("PI [4*atan(1.0)]                    : %1").arg(double(pi))
//               << "\n" << QString("LduDistance [10.0/tan(0.005*pi/180)]: %1").arg(double(LduDistance))
//               << "\n" << QString("Page Width [pixels]                 : %1").arg(LPub::pageSize(meta.LPub.page, XX))
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

const QStringList Render::getImageAttributes(const QString &pngName)
{
    QFileInfo fileInfo(pngName);
    QString cleanString = fileInfo.completeBaseName();
    if (Preferences::enableFadeSteps && pngName.endsWith(FADE_SFX))
        cleanString.chop(QString(FADE_SFX).size());
    else
    if (Preferences::enableHighlightStep && pngName.endsWith(HIGHLIGHT_SFX))
        cleanString.chop(QString(HIGHLIGHT_SFX).size());

    QStringList cleanStringList = cleanString.split("_");

    // treat parts with '_' in the name - decode
    if (cleanStringList.at(nType).count(";"))
        cleanStringList[nType].replace(";", "_");

    // treat parts with renderer index prefix
    QRegExp renderIdRx("-([0-3])$");
    int rendererIndex = cleanStringList.at(nType).lastIndexOf(renderIdRx);
    if (rendererIndex > -1) {
        int rendererId = renderIdRx.cap(1).toInt();
        bool rendererSuffix = rendererId == getRenderer();
        // get type name without renderer index
        if (rendererSuffix) {
            const QString type = cleanStringList[nType].left(rendererIndex);
            cleanStringList[nType] = type;
        }
    }

    return cleanStringList;
}

bool Render::compareImageAttributes(
    const QStringList &attributes,
    const QString &compareKey,
    bool pare)
{
    bool result;
    QString message;
    QStringList attributesList = attributes;
    if (attributesList.size() >= nBaseAttributes) {
        if (!compareKey.endsWith("SUB") &&
             attributes.endsWith("SUB"))
           attributesList.removeLast();
        if (pare) {
            attributesList.removeAt(nResType);
            attributesList.removeAt(nResolution);
            attributesList.removeAt(nPageWidth);
            attributesList.removeAt(nColorCode);
            attributesList.removeAt(nType);
        }
        const QString attributesKey = attributesList.join("_");
        result = compareKey != attributesKey;
        if (Preferences::debugLogging) {
            message = QObject::tr("Attributes compare: [%1], attributesKey [%2], compareKey [%3]")
                                  .arg(result ? "No Match - usingSnapshotArgs (attributes)" : "Match" )
                                  .arg(attributesKey).arg(compareKey);
            gui->messageSig(LOG_DEBUG, message);
        }
    } else {
        result = false;
        message = QObject::tr("Malformed image file attributes list [%1]")
                              .arg(attributesList.join("_"));
        gui->messageSig(LOG_NOTICE, message);
    }
    return result;
}

bool Render::createSnapshotsList(
    const QStringList &ldrNames,
    const QString &SnapshotsList)
{
    QFile SnapshotsListFile(SnapshotsList);
    if ( ! SnapshotsListFile.open(QFile::WriteOnly | QFile::Text)) {
        emit gui->messageSig(LOG_ERROR,QObject::tr("Failed to create LDView (Single Call) PLI Snapshots list file!"));
        return false;
    }

    QTextStream out(&SnapshotsListFile);

    for (int i = 0; i < ldrNames.size(); i++) {
        QString smLine = ldrNames[i];
        if (QFileInfo(smLine).exists()) {
            out << smLine << lpub_endl;
            if (Preferences::debugLogging)
                emit gui->messageSig(LOG_DEBUG, QObject::tr("Wrote %1 to PLI Snapshots list").arg(smLine));
        } else {
            emit gui->messageSig(LOG_ERROR, QObject::tr("Error %1 not written to Snapshots list - file does not exist").arg(smLine));
        }
    }
    SnapshotsListFile.close();
    return true;
}

int Render::executeLDViewProcess(QStringList &arguments, Options::Mt module) {

  QString message = QObject::tr("LDView %1 %2 Arguments: %3 %4")
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
          emit gui->messageSig(LOG_ERROR,QObject::tr("LDView %1 %2 render failed with code %2 %3")
                               .arg(useLDViewSCall() ? "(SingleCall)" : "(Default)")
                               .arg(module == Options::CSI ? "CSI" : "PLI")
                               .arg(ldview.exitCode())
                               .arg(str));
          return -1;
        }
    }

  bool usingInputFileList = false;
  Q_FOREACH (QString argument, arguments){
      if (argument.startsWith("-CommandLinesList=") ||
          argument.startsWith("-SaveSnapshotsList=")) {
          usingInputFileList = true;
          break;
      }
  }

  if (!usingInputFileList) {
      QFile outputImageFile(arguments.last());
      if (! outputImageFile.exists()) {
          emit gui->messageSig(LOG_ERROR,QObject::tr("LDView %1 image generation failed for %2 with message %3")
                               .arg(module == Options::CSI ? "CSI" : "PLI")
                               .arg(outputImageFile.fileName())
                               .arg(outputImageFile.errorString()));
          return -1;
      }
  }

  return 0;
}

void Render::getStudStyleAndAutoEdgeSettings(
  StudStyleMeta *ssm, HighContrastColorMeta *hccm, AutoEdgeColorMeta *acm,
  QString &ss, QString &ae, QString &ac, QString &ai,
  QString &hs, QString &hp, QString &hb, QString &hd)
{
    bool ldv = renderer == &ldview;
    if (acm->enable.value())
    {
        ae = QString(ldv ? "-AutomateEdgeColor=1" : "-laA");
        if (acm->contrast.value() != EDGE_COLOR_CONTRAST_DEFAULT)
            ac = QString(ldv ? "-PartEdgeContrast=%1" : "-laC%1").arg(acm->contrast.value());
        if (acm->saturation.value() != LIGHT_DARK_INDEX_DEFAULT)
            ai = QString(ldv ? "-PartColorValueLDIndex=%1" : "-laI%1").arg(acm->saturation.value());
    }
    if (ssm->value())
    {
        ss = QString(ldv ? "-StudStyle=%1" : "-fss%1").arg(ssm->value());
        if (ssm->value() < 5)
            return;
        if (hccm->lightDarkIndex.value() != LIGHT_DARK_INDEX_DEFAULT)
            ac = QString(ldv ? "-PartColorValueLDIndex=%1" : "-laI%1").arg(hccm->lightDarkIndex.value());
        if (hccm->studCylinderColor.value() != hccm->studCylinderColor.getRGBAFromString(HIGH_CONTRAST_STUD_CYLINDER_DEFAULT))
            hs = QString(ldv ? "--StudCylinderColor=%1" : "-lhS%1")
                    .arg(hccm->studCylinderColor.validStringValue(HIGH_CONTRAST_STUD_CYLINDER_DEFAULT));
        if (hccm->partEdgeColor.value() != hccm->partEdgeColor.getRGBAFromString(HIGH_CONTRAST_PART_EDGE_DEFAULT))
            hp = QString(ldv ? "-PartEdgeColor=%1" : "-lhP%1")
                        .arg(hccm->partEdgeColor.validStringValue(HIGH_CONTRAST_PART_EDGE_DEFAULT));
        if (hccm->blackEdgeColor.value() != hccm->blackEdgeColor.getRGBAFromString(HIGH_CONTRAST_BLACK_EDGE_DEFAULT))
            hb = QString(ldv ? "-BlackEdgeColor=%1" : "-lhB%1")
                        .arg(hccm->blackEdgeColor.validStringValue(HIGH_CONTRAST_BLACK_EDGE_DEFAULT));
        if (hccm->darkEdgeColor.value() != hccm->darkEdgeColor.getRGBAFromString(HIGH_CONTRAST_DARK_EDGE_DEFAULT))
            hd = QString(ldv ? "-DarkEdgeColor=%1" : "-lhD%1")
                        .arg(hccm->darkEdgeColor.validStringValue(HIGH_CONTRAST_DARK_EDGE_DEFAULT));
    }
}

QStringList Render::splitParms(const QString &parms)
{
    QStringList values;
    if (parms.isEmpty())
        return values;
    QRegExp quoteRx("\"|'");
    bool quoted = QString(parms.at(0)).contains(quoteRx);      // true if the first character is " or '
    if (!quoted)
        return parms.split(' ');                               // if not quoted split on space
    bool inside = true;
    QStringList list = parms.split(quoteRx, SkipEmptyParts);   // Split by " or '
    if (list.size() == 1) {
        values << list.first();
    } else {
        for (const QString &item : list) {
            if (inside) {                                       // If 's' is inside quotes ...
                values.append(item);                            // ... get the whole string
            } else {                                            // If 's' is outside quotes ...
                values.append(item.split(' ', SkipEmptyParts)); // ... get the split string
            }
            inside = !inside;
        }
    }
    return values;
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
  QString message, newArg;
  QString ldrName = QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";
  QString povName = ldrName + ".pov";
  FloatPairMeta cameraAngles;
  cameraAngles.setValues(meta.LPub.assem.cameraAngles.value(0),
                         meta.LPub.assem.cameraAngles.value(1));

  // RotateParts #2 - 8 parms
  int rc;
  if ((rc = rotateParts(addLine, meta.rotStep, csiParts, ldrName, QString(),cameraAngles,DT_DEFAULT,Options::CSI)) < 0) {
      return rc;
   }

  // Populate render attributes
  QStringList ldviewParmslist = splitParms(meta.LPub.assem.ldviewParms.value());
  QString transform  = meta.rotStep.value().type.toUpper();
  bool customViewpoint = meta.LPub.assem.cameraAngles.customViewpoint();
  bool noCA          = !customViewpoint && (Preferences::applyCALocally || transform == QLatin1String("ABS"));
  bool pp            = Preferences::perspectiveProjection;
  float modelScale   = meta.LPub.assem.modelScale.value();
  float cameraFoV    = meta.LPub.assem.cameraFoV.value();
  float cameraAngleX = noCA ? 0.0f : meta.LPub.assem.cameraAngles.value(XX);
  float cameraAngleY = noCA ? 0.0f : meta.LPub.assem.cameraAngles.value(YY);
  Vector3 target     = Vector3(meta.LPub.assem.target.x(),meta.LPub.assem.target.y(),meta.LPub.assem.target.z());
  StudStyleMeta* ssm = meta.LPub.studStyle.value() ? &meta.LPub.studStyle : &meta.LPub.assem.studStyle;
  AutoEdgeColorMeta* aecm = meta.LPub.autoEdgeColor.enable.value() ? &meta.LPub.autoEdgeColor : &meta.LPub.assem.autoEdgeColor;
  HighContrastColorMeta* hccm = meta.LPub.studStyle.value() ? &meta.LPub.highContrast : &meta.LPub.assem.highContrast;

  /* determine camera distance */
  int cd = int(meta.LPub.assem.cameraDistance.value());
  if (!cd)
      cd = int(cameraDistance(meta,modelScale)*1700/1000);

  // set page size
  bool useImageSize = meta.LPub.assem.imageSize.value(XX) > 0;
  int width  = useImageSize ? int(meta.LPub.assem.imageSize.value(XX)) : LPub::pageSize(meta.LPub.page, XX);
  int height = useImageSize ? int(meta.LPub.assem.imageSize.value(YY)) : LPub::pageSize(meta.LPub.page, YY);

  // projection settings
  QString CA, cg;

  // parameter arguments;
  QStringList parmsArgs;

  auto getRendererSettings = [
          &pp,
          &cd,
          &target,
          &modelScale,
          &cameraFoV,
          &cameraAngleX,
          &cameraAngleY,
          &ldviewParmslist,
          &useImageSize] (
      QString     &CA,
      QString     &cg,
      QStringList &parmsArgs)
  {
      // additional LDView parameters;
      qreal cdf = LP3D_CDF;
      QString dz, dl, df = QString("-FOV=%1").arg(double(cameraFoV));
      bool pd = false, pl = false, pf = false, pz = false;
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
              if (!pd && !pl && !pf && !pz) {
                addArgument(parmsArgs, ldviewParmslist[i]);    // 10. ldviewParms [usually empty but can include PovLights]
              }
          }
      }

      // Set camera angle and camera globe and update arguments with perspective projection settings
      if (pp && pl && !pz)
          dz = QString("-DefaultZoom=%1").arg(double(modelScale));

      CA = pp ? df : QString("-ca%1") .arg(LP3D_CA);              // replace CA with FOV

      // Set alternate target position or use specified image size
      QString _mc;
      if (target.isPopulated())
          _mc = QString("-ModelCenter=%1,%2,%3 ").arg(double(target.x)).arg(double(target.y)).arg(double(target.z));
      if ((!_mc.isEmpty() && !pl) || (useImageSize && _mc.isEmpty())){
          // Set model center
          QString _dl = QString("-DefaultLatLong=%1,%2")
                                .arg(double(cameraAngleX))
                                .arg(double(cameraAngleY));
          QString _dz = QString("-DefaultZoom=%1").arg(double(modelScale));
          // Set zoom to fit when use image size specified
          QString _sz;
          if (useImageSize && _mc.isEmpty())
              _sz = QString(" -SaveZoomToFit=1");
          cg = QString("%1%2 %3%4").arg(_mc.isEmpty() ? "" : _mc).arg(_dl).arg(_dz).arg(_sz.isEmpty() ? "" : _sz);
      } else {
          cg = pp ? pl ? QString("-DefaultLatLong=%1 %2")
                                 .arg(dl)
                                 .arg(dz)                         // replace Camera Globe with DefaultLatLon and add DefaultZoom
                       : QString("-cg%1,%2,%3")
                                 .arg(double(cameraAngleX))
                                 .arg(double(cameraAngleY))
                                 .arg(QString::number(cd * cdf,'f',0) )
                  : QString("-cg%1,%2,%3")
                            .arg(double(cameraAngleX))
                            .arg(double(cameraAngleY))
                            .arg(cd);
      }

      // additional LDView parameters;
      if (parmsArgs.size()){
          emit gui->messageSig(LOG_INFO,QObject::tr("LDView additional POV-Ray PLI renderer parameters: %1")
                                                    .arg(parmsArgs.join(" ")));
          cg.append(QString(" %1").arg(parmsArgs.join(" ")));
      }
  };

  getRendererSettings(CA, cg, parmsArgs);

  QString ss,ae,ac,ai,hs,hp,pb,hd;
  getStudStyleAndAutoEdgeSettings(ssm, hccm, aecm, ss, ae, ac, ai, hs, hp, pb, hd);
  QString w  = QString("-SaveWidth=%1") .arg(width);
  QString h  = QString("-SaveHeight=%1") .arg(height);

  QStringList arguments;
  arguments << CA;
  arguments << cg;
  arguments << ss;  // stud style
  arguments << ae;  // automate edge color
  arguments << ac;  // part edge contrast
  arguments << ai;  // part color value light/dark index (saturation)
  arguments << hs;  // stud cylinder color
  arguments << hp;  // part edge color
  arguments << pb;  // black edge color
  arguments << hd;  // dark edge color
  arguments << w;
  arguments << h;

  // Native (LDV) POV Generator block
  if (Preferences::useNativePovGenerator) {

      if (!Preferences::altLDConfigPath.isEmpty())
          arguments << QString("-LDConfig=\"%1\"") .arg(QDir::toNativeSeparators(Preferences::altLDConfigPath));
      arguments << QString("-ExportFile=\"%1\"") .arg(QDir::toNativeSeparators(povName));
      arguments << QString("-LDrawDir=\"%1\"") .arg(QDir::toNativeSeparators(Preferences::ldrawLibPath));

      arguments << QString("\"%1\"").arg(QDir::toNativeSeparators(ldrName));

      removeEmptyStrings(arguments);

      QString workingDirectory = QDir::currentPath();

      emit gui->messageSig(LOG_STATUS, QObject::tr("Native CSI POV file generation..."));

      message = QObject::tr("Native CSI POV File Generation Arguments: %1").arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
      qDebug() << qPrintable(message);
#else
      emit gui->messageSig(LOG_INFO, message);
#endif

      bool retError = false;
      ldvWidget = new LDVWidget(nullptr,NativePOVIni,true);
      if (! ldvWidget->doCommand(arguments))  {
          emit gui->messageSig(LOG_ERROR, QObject::tr("Failed to generate CSI POV file for command: %1").arg(arguments.join(" ")));
          retError = true;
      }

      // ldvWidget changes the Working directory so we must reset
      if (! QDir::setCurrent(workingDirectory)) {
          emit gui->messageSig(LOG_ERROR, QObject::tr("Failed to restore CSI POV working directory %1").arg(workingDirectory));
          retError = true;
      }
      if (retError)
          return -1;
  }
  else // LDView  POV generator block
  {
      if (!Preferences::altLDConfigPath.isEmpty())
          arguments << QString("-LDConfig=%1") .arg(QDir::toNativeSeparators(Preferences::altLDConfigPath));
      arguments << QString("-ExportFile=%1") .arg(QDir::toNativeSeparators(povName));
      arguments << QString("-LDrawDir=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldrawLibPath)));

      QString o("-HaveStdOut=1");
      QString v("-vv");

      addArgument(arguments, o, "-HaveStdOut", 0/*POVGen*/, parmsArgs.size());
      addArgument(arguments, v, "-vv", 0/*POVGen*/, parmsArgs.size());

//      if (Preferences::enableFadeSteps)
//        arguments <<  QString("-SaveZMap=1");

      bool hasLDViewIni = Preferences::ldviewPOVIni != "";
      if(hasLDViewIni){
          newArg  = QString("-IniFile=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldviewPOVIni)));
          addArgument(arguments, newArg, "-IniFile", 0/*POVGen*/, parmsArgs.size());
        }

      arguments << QString("%1").arg(QDir::toNativeSeparators(ldrName));;

      removeEmptyStrings(arguments);

      emit gui->messageSig(LOG_STATUS, QObject::tr("LDView CSI POV file generation..."));

      message = QObject::tr("LDView CSI POV File Generation CSI Arguments: %1 %2").arg(Preferences::ldviewExe).arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
      qDebug() << qPrintable(message);
#else
      emit gui->messageSig(LOG_INFO, message);
#endif

      QProcess    ldview;
      ldview.setEnvironment(QProcess::systemEnvironment());
      ldview.setWorkingDirectory(QDir::currentPath() + "/" + Paths::tmpDir);
      ldview.setStandardErrorFile(QDir::currentPath() + "/stderr-ldviewpov");
      ldview.setStandardOutputFile(QDir::currentPath() + "/stdout-ldviewpov");
      ldview.start(Preferences::ldviewExe,arguments);
      if ( ! ldview.waitForFinished(rendererTimeout())) {
          if (ldview.exitCode() != 0 || 1) {
              QByteArray status = ldview.readAll();
              QString str;
              str.append(status);
              emit gui->messageSig(LOG_ERROR,QObject::tr("LDView POV file generation failed with exit code %1\n%2") .arg(ldview.exitCode()) .arg(str));
              return -1;
          }
      }
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

  parmsArgs = splitParms(meta.LPub.assem.povrayParms.value());
  for (int i = 0; i < parmsArgs.size(); i++) {
      if (parmsArgs[i] != "" && parmsArgs[i] != " ") {
          addArgument(povArguments, parmsArgs[i], QString(), 1/*POVGen*/);
        }
    }
  if (parmsArgs.size())
      emit gui->messageSig(LOG_INFO,QObject::tr("POV-Ray additional CSI renderer parameters: %1")
                           .arg(parmsArgs.join(" ")));

//#ifndef __APPLE__
//  povArguments << "/EXIT";
//#endif

  removeEmptyStrings(povArguments);

  emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("Executing POVRay %1 CSI render - please wait...")
                                                    .arg(pp ? "Perspective" : "Orthographic"));

  message = QString("POVRay CSI Arguments: %1 %2").arg(Preferences::povrayExe).arg(povArguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_INFO, message);
#endif

  QProcess povray;
  QStringList povEnv = QProcess::systemEnvironment();
  povEnv.prepend("POV_IGNORE_SYSCONF_MSG=1");
  povray.setEnvironment(povEnv);
  povray.setWorkingDirectory(QDir::currentPath()+ "/" + Paths::assemDir); // pov win console app will not write to dir different from cwd or source file dir
  povray.setStandardErrorFile(QDir::currentPath() + "/stderr-povray");
  povray.setStandardOutputFile(QDir::currentPath() + "/stdout-povray");
  povray.start(Preferences::povrayExe,povArguments);
  if ( ! povray.waitForFinished(rendererTimeout())) {
      if (povray.exitCode() != 0) {
          QByteArray status = povray.readAll();
          QString str;
          str.append(status);
          emit gui->messageSig(LOG_ERROR,QObject::tr("POVRay CSI render failed with code %1\n%2").arg(povray.exitCode()) .arg(str));
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

  // test ldrNames
  QFileInfo fileInfo(ldrNames.first());
  if ( ! fileInfo.exists()) {
      emit gui->messageSig(LOG_ERROR,QObject::tr("POV PLI render input file was not found at the specified path [%1]")
                                                 .arg(ldrNames.first()));
    return -1;
  }

  //  QStringList list;
  QString message, newArg;
  QString povName = ldrNames.first() +".pov";

  // Populate render attributes
  QStringList ldviewParmslist = splitParms(metaType.ldviewParms.value());
  QString transform  = metaType.rotStep.value().type.toUpper();
  bool customViewpoint = metaType.cameraAngles.customViewpoint();
  bool noCA          = !customViewpoint && pliType == SUBMODEL ? Preferences::applyCALocally || transform == QLatin1String("ABS") : transform == QLatin1String("ABS");
  bool pp            = Preferences::perspectiveProjection;
  float modelScale   = metaType.modelScale.value();
  float cameraFoV    = metaType.cameraFoV.value();
  float cameraAngleX = noCA ? 0.0f : metaType.cameraAngles.value(XX);
  float cameraAngleY = noCA ? 0.0f : metaType.cameraAngles.value(YY);
  Vector3 target     = Vector3(metaType.target.x(),metaType.target.y(),metaType.target.z());
  StudStyleMeta* ssm = meta.LPub.studStyle.value() ? &meta.LPub.studStyle : &metaType.studStyle;
  AutoEdgeColorMeta* aecm = meta.LPub.autoEdgeColor.enable.value() ? &meta.LPub.autoEdgeColor : &metaType.autoEdgeColor;
  HighContrastColorMeta* hccm = meta.LPub.studStyle.value() ? &meta.LPub.highContrast : &metaType.highContrast;

  /* determine camera distance */
  int cd = int(metaType.cameraDistance.value());
  if (!cd)
      cd = int(cameraDistance(meta,modelScale)*1700/1000);

  //qDebug() << qPrintable(QString("LDView (Default) Camera Distance: %1").arg(cd));

  // set page size
  bool useImageSize = metaType.imageSize.value(XX) > 0;
  int width  = useImageSize ? int(metaType.imageSize.value(XX)) : LPub::pageSize(meta.LPub.page, XX);
  int height = useImageSize ? int(metaType.imageSize.value(YY)) : LPub::pageSize(meta.LPub.page, YY);

  // projection settings
  QString CA, cg;

  // parameter arguments;
  QStringList parmsArgs;

  auto getRendererSettings = [
          &keySub,
          &pp,
          &cd,
          &pngName,
          &target,
          &pliType,
          &modelScale,
          &cameraFoV,
          &cameraAngleX,
          &cameraAngleY,
          &ldviewParmslist,
          &useImageSize] (
      QString     &CA,
      QString     &cg,
      bool        &noCA,
      QStringList &parmsArgs,
      QString     &cleanPngName)
  {
      // additional LDView parameters;
      qreal cdf = LP3D_CDF;
      QString dz, dl, df = QString("-FOV=%1").arg(double(cameraFoV));
      bool pd = false, pl = false, pf = false, pz = false;
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
              if (!pd && !pl && !pf && !pz) {
                addArgument(parmsArgs, ldviewParmslist[i]);    // 10. ldviewParms [usually empty]
              }
          }
      }

      // Set camera angle and camera globe and update arguments with perspective projection settings
      if (pp && pl && !pz)
          dz = QString("-DefaultZoom=%1").arg(double(modelScale));

      cleanPngName = pngName;

      // Process substitute part attributes
      if (keySub) {
        QStringList attributes = getImageAttributes(pngName);
        bool hr;
        if ((hr = attributes.size() == nHasRotstep) || attributes.size() == nHasTargetAndRotstep)
          noCA = attributes.at(hr ? nRotTrans : nRot_Trans).toUpper() == QLatin1String("ABS");
        if (attributes.size() >= nHasTarget)
          target = Vector3(attributes.at(nTargetX).toFloat(),attributes.at(nTargetY).toFloat(),attributes.at(nTargetZ).toFloat());
        if (keySub > PliBeginSub2Rc) {
          modelScale   = attributes.at(nModelScale).toFloat();
          cameraFoV    = attributes.at(nCameraFoV).toFloat();
          cameraAngleX = attributes.at(nCameraAngleXX).toFloat();
          cameraAngleY = attributes.at(nCameraAngleYY).toFloat();
        }
      }

      if (keySub || pliType == BOM)
          cleanPngName.replace(";", "_");

      CA = pp ? df : QString("-ca%1") .arg(LP3D_CA);              // replace CA with FOV

      // Set alternate target position or use specified image size
      QString _mc;
      if (target.isPopulated())
          _mc = QString("-ModelCenter=%1,%2,%3 ").arg(double(target.x)).arg(double(target.y)).arg(double(target.z));
      if ((!_mc.isEmpty() && !pl) || (useImageSize && _mc.isEmpty())){
          // Set model center
          QString _dl = QString("-DefaultLatLong=%1,%2")
                                .arg(double(cameraAngleX))
                                .arg(double(cameraAngleY));
          QString _dz = QString("-DefaultZoom=%1").arg(double(modelScale));
          // Set zoom to fit when use image size specified
          QString _sz;
          if (useImageSize && _mc.isEmpty())
              _sz = QString(" -SaveZoomToFit=1");
          cg = QString("%1%2 %3%4").arg(_mc.isEmpty() ? "" : _mc).arg(_dl).arg(_dz).arg(_sz.isEmpty() ? "" : _sz);
      } else {
          cg = pp ? pl ? QString("-DefaultLatLong=%1 %2")
                                 .arg(dl)
                                 .arg(dz)                         // replace Camera Globe with DefaultLatLon and add DefaultZoom
                       : QString("-cg%1,%2,%3")
                                 .arg(double(cameraAngleX))
                                 .arg(double(cameraAngleY))
                                 .arg(QString::number(cd * cdf,'f',0) )
                  : QString("-cg%1,%2,%3")
                            .arg(double(cameraAngleX))
                            .arg(double(cameraAngleY))
                            .arg(cd);
      }

      // additional LDView parameters;
      if (parmsArgs.size()){
          emit gui->messageSig(LOG_INFO,QObject::tr("LDView additional POV-Ray PLI renderer parameters: %1")
                                                        .arg(parmsArgs.join(" ")));
          cg.append(QString(" %1").arg(parmsArgs.join(" ")));
      }
  };

  QString cleanPngName;

  getRendererSettings(CA, cg, noCA, parmsArgs, cleanPngName);

  QString ss,ae,ac,ai,hs,hp,pb,hd;
  getStudStyleAndAutoEdgeSettings(ssm, hccm, aecm, ss, ae, ac, ai, hs, hp, pb, hd);
  QString w  = QString("-SaveWidth=%1")  .arg(width);
  QString h  = QString("-SaveHeight=%1") .arg(height);

  QStringList arguments;
  arguments << CA;             // Camera Angle (i.e. Field of Veiw)
  arguments << cg.split(" ");  // Camera Globe, Target and Additional Parameters when specified
  arguments << ss;             // stud style
  arguments << ae;             // automate edge color
  arguments << ac;             // part edge contrast
  arguments << ai;             // part color value light/dark index (saturation)
  arguments << hs;             // stud cylinder color
  arguments << hp;             // part edge color
  arguments << pb;             // black edge color
  arguments << hd;             // dark edge color
  arguments << w;
  arguments << h;

  // Native (LDV) POV generator block begin
  if (Preferences::useNativePovGenerator) {

      if (!Preferences::altLDConfigPath.isEmpty())
          arguments << QString("-LDConfig=\"%1\"") .arg(QDir::toNativeSeparators(Preferences::altLDConfigPath));
      arguments << QString("-ExportFile=\"%1\"") .arg(QDir::toNativeSeparators(povName));
      arguments << QString("-LDrawDir=\"%1\"") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldrawLibPath)));

      arguments << QString("\"%1\"").arg(QDir::toNativeSeparators(ldrNames.first()));

      removeEmptyStrings(arguments);

      QString workingDirectory = QDir::currentPath();

      emit gui->messageSig(LOG_STATUS, QObject::tr("Native POV PLI file generation..."));

      message = QObject::tr("Native PLI POV File Generation Arguments: %1").arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
      qDebug() << qPrintable(message);
#else
      emit gui->messageSig(LOG_INFO, message);
#endif

      bool retError = false;
      ldvWidget = new LDVWidget(nullptr,NativePOVIni,true);
      if (! ldvWidget->doCommand(arguments)) {
          emit gui->messageSig(LOG_ERROR, QObject::tr("Failed to generate PLI POV file for command: %1").arg(arguments.join(" ")));
          retError = true;
      }

      // ldvWidget changes the Working directory so we must reset
      if (! QDir::setCurrent(workingDirectory)) {
          emit gui->messageSig(LOG_ERROR, QObject::tr("Failed to restore PLI POV working directory %1").arg(workingDirectory));
          retError = true;
      }
      if (retError)
        return -1;
  }
  else // LDView POV generator block
  {
      if (!Preferences::altLDConfigPath.isEmpty())
          arguments << QString("-LDConfig=%1") .arg(QDir::toNativeSeparators(Preferences::altLDConfigPath));
      arguments << QString("-ExportFile=%1") .arg(QDir::toNativeSeparators(povName));
      arguments << QString("-LDrawDir=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldrawLibPath)));

      QString o("-HaveStdOut=1");
      QString v("-vv");

      addArgument(arguments, o, "-HaveStdOut", 0/*POVGen*/, parmsArgs.size());
      addArgument(arguments, v, "-vv", 0/*POVGen*/, parmsArgs.size());

      bool hasLDViewIni = Preferences::ldviewPOVIni != "";
      if(hasLDViewIni){
          newArg  = QString("-IniFile=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldviewPOVIni)));
          addArgument(arguments, newArg, "-IniFile", 0/*POVGen*/, parmsArgs.size());
        }

      arguments << QString("%1").arg(QDir::toNativeSeparators(ldrNames.first()));

      removeEmptyStrings(arguments);

      emit gui->messageSig(LOG_STATUS, QObject::tr("LDView PLI POV file generation..."));

      message = QObject::tr("LDView PLI POV File Generation PLI Arguments: %1 %2").arg(Preferences::ldviewExe).arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
      qDebug() << qPrintable(message);
#else
      emit gui->messageSig(LOG_INFO, message);
#endif

      QProcess    ldview;
      ldview.setEnvironment(QProcess::systemEnvironment());
      ldview.setWorkingDirectory(QDir::currentPath());
      ldview.setStandardErrorFile(QDir::currentPath() + "/stderr-ldviewpov");
      ldview.setStandardOutputFile(QDir::currentPath() + "/stdout-ldviewpov");

      ldview.start(Preferences::ldviewExe,arguments);
      if ( ! ldview.waitForFinished()) {
          if (ldview.exitCode() != 0) {
              QByteArray status = ldview.readAll();
              QString str;
              str.append(status);
              emit gui->messageSig(LOG_ERROR,QObject::tr("LDView POV file generation failed with exit code %1\n%2") .arg(ldview.exitCode()) .arg(str));
              return -1;
          }
      }
  }

  QStringList povArguments;
  if (Preferences::povrayDisplay){
      povArguments << QString("+d");
  } else {
      povArguments << QString("-d");
  }

  QString O = QString("+O\"%1\"").arg(QDir::toNativeSeparators(cleanPngName));
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

  parmsArgs = splitParms(meta.LPub.assem.povrayParms.value());
  for (int i = 0; i < parmsArgs.size(); i++) {
      if (parmsArgs[i] != "" && parmsArgs[i] != " ") {
          addArgument(povArguments, parmsArgs[i], QString(), 1/*POVGen*/);
        }
    }
  if (parmsArgs.size())
      emit gui->messageSig(LOG_INFO,QObject::tr("POV-Ray additional PLI renderer parameters: %1")
                                                .arg(parmsArgs.join(" ")));

//#ifndef __APPLE__
//  povArguments << "/EXIT";
//#endif

  removeEmptyStrings(povArguments);

  emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("Executing POVRay %1 PLI render - please wait...")
                                                    .arg(pp ? "Perspective" : "Orthographic"));

  message = QObject::tr("POVRay PLI Arguments: %1 %2").arg(Preferences::povrayExe).arg(povArguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_INFO, message);
#endif

  QProcess povray;
  QStringList povEnv = QProcess::systemEnvironment();
  povEnv.prepend("POV_IGNORE_SYSCONF_MSG=1");
  QString partsDir = pliType == BOM ? Paths::bomDir : Paths::partsDir;
  QString workingDirectory = pliType == SUBMODEL ? Paths::submodelDir : partsDir;
  povray.setEnvironment(povEnv);
  povray.setWorkingDirectory(QDir::currentPath()+ "/" + workingDirectory); // pov win console app will not write to dir different from cwd or source file dir
  povray.setStandardErrorFile(QDir::currentPath() + "/stderr-povray");
  povray.setStandardOutputFile(QDir::currentPath() + "/stdout-povray");

  povray.start(Preferences::povrayExe, povArguments);
  if ( ! povray.waitForFinished(rendererTimeout())) {
      if (povray.exitCode() != 0) {
          QByteArray status = povray.readAll();
          QString str;
          str.append(status);
          emit gui->messageSig(LOG_ERROR,QObject::tr("POVRay PLI render failed with code %1\n%2") .arg(povray.exitCode()) .arg(str));
          return -1;
      }
  }

  if (clipImage(cleanPngName))
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

int LDGLite::   renderCsi(
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
  FloatPairMeta cameraAngles;
  cameraAngles.setValues(meta.LPub.assem.cameraAngles.value(0),
                         meta.LPub.assem.cameraAngles.value(1));
  // RotateParts #2 - 8 parms
  if ((rc = rotateParts(addLine, meta.rotStep, csiParts, ldrFile,QString(),cameraAngles,DT_DEFAULT,Options::CSI)) < 0) {
     return rc;
  }

  /* determine camera distance */
  int cd = int(meta.LPub.assem.cameraDistance.value());
  if (!cd)
      cd = int(cameraDistance(meta,meta.LPub.assem.modelScale.value()));

  /* apply camera angle */

  bool customViewpoint = meta.LPub.assem.cameraAngles.customViewpoint();
  bool noCA  = !customViewpoint && Preferences::applyCALocally;
  bool pp    = Preferences::perspectiveProjection;

  bool useImageSize = meta.LPub.assem.imageSize.value(XX) > 0;
  int width  = useImageSize ? int(meta.LPub.assem.imageSize.value(XX)) : LPub::pageSize(meta.LPub.page, XX);
  int height = useImageSize ? int(meta.LPub.assem.imageSize.value(YY)) : LPub::pageSize(meta.LPub.page, YY);
  StudStyleMeta* ssm = meta.LPub.studStyle.value() ? &meta.LPub.studStyle : &meta.LPub.assem.studStyle;
  AutoEdgeColorMeta* aecm = meta.LPub.autoEdgeColor.enable.value() ? &meta.LPub.autoEdgeColor : &meta.LPub.assem.autoEdgeColor;
  HighContrastColorMeta* hccm = meta.LPub.studStyle.value() ? &meta.LPub.highContrast : &meta.LPub.assem.highContrast;

  QString v  = QString("-v%1,%2")   .arg(width)
                                    .arg(height);
  QString o  = QString("-o0,-%1")   .arg(height/6);
  QString mf = QString("-mF%1")     .arg(pngName);

  QString ss,ae,ac,ai,hs,hp,pb,hd;
  getStudStyleAndAutoEdgeSettings(ssm, hccm, aecm, ss, ae, ac, ai, hs, hp, pb, hd);

  int lineThickness = int(resolution()/150+0.5f);
  if (lineThickness == 0) {
    lineThickness = 1;
  }
  QString w  = QString("-W%1")      .arg(lineThickness); // ldglite always deals in 72 DPI

  QString CA = QString("-ca%1") .arg(pp ? double(meta.LPub.assem.cameraFoV.value()) : LP3D_CA);

  QString cg;
  if (meta.LPub.assem.target.isPopulated()){
      cg = QString("-co%1,%2,%3")
               .arg(double(meta.LPub.assem.target.x()))
               .arg(double(meta.LPub.assem.target.y()))
               .arg(double(meta.LPub.assem.target.z()));
  } else {
      cg = QString("-cg%1,%2,%3") .arg(noCA ? double(0.0f) : double(meta.LPub.assem.cameraAngles.value(XX)))
                                  .arg(noCA ? double(0.0f) : double(meta.LPub.assem.cameraAngles.value(YY)))
                                  .arg(cd);
  }

  QString J  = QString("-%1").arg(pp ? "J" : "j");

  QStringList arguments;
  arguments << CA;                  // camera FOV in degrees
  arguments << cg;                  // camera globe - scale factor or model origin for the camera to look at
  arguments << J;                   // projection
  arguments << v;                   // display in X wide by Y high window
  arguments << o;                   // changes the centre X across and Y down
  arguments << w;                   // line thickness
  arguments << ss;                  // stud style
  arguments << ae;                  // automate edge color
  arguments << ac;                  // part edge contrast
  arguments << ai;                  // part color value light/dark index (saturation)
  arguments << hs;                  // stud cylinder color
  arguments << hp;                  // part edge color
  arguments << pb;                  // black edge color
  arguments << hd;                  // dark edge color

  QStringList list;
  // First, load parms from meta if any
  list = splitParms(meta.LPub.assem.ldgliteParms.value());
  for (int i = 0; i < list.size(); i++) {
     if (list[i] != "" && list[i] != " ") {
         addArgument(arguments, list[i]);
      }
  }
  if (list.size())
      emit gui->messageSig(LOG_INFO,QObject::tr("LDGlite additional CSI renderer parameters: %1") .arg(list.join(" ")));

  // Add ini arguments if not already in additional parameters
  for (int i = 0; i < Preferences::ldgliteParms.size(); i++) {
      if (list.indexOf(QRegExp("^" + QRegExp::escape(Preferences::ldgliteParms[i]))) < 0) {
        addArgument(arguments, Preferences::ldgliteParms[i], QString(), 0/*POVGen*/, list.size());
      }
  }

  // Add custom color file if exist
  if (!Preferences::altLDConfigPath.isEmpty()) {
    addArgument(arguments, QString("-ldcF%1").arg(Preferences::altLDConfigPath), "-ldcF", 0, list.size());
  }

  arguments << QDir::toNativeSeparators(mf);                  // .png file name
  arguments << QDir::toNativeSeparators(ldrFile);             // csi.ldr (input file)

  removeEmptyStrings(arguments);

  emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("Executing LDGLite %1 CSI render - please wait...")
                                                    .arg(pp ? "Perspective" : "Orthographic"));

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

  QString message = QObject::tr("LDGLite CSI Arguments: %1 %2").arg(Preferences::ldgliteExe).arg(arguments.join(" "));
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
      emit gui->messageSig(LOG_ERROR,QObject::tr("LDGlite failed\n%1") .arg(str));
      return -1;
    }
  }

  QFile outputImageFile(pngName);
  if (! outputImageFile.exists()) {
      emit gui->messageSig(LOG_ERROR,QObject::tr("LDGLite CSI image generation failed for %1 with message %2")
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
  QString transform  = metaType.rotStep.value().type.toUpper();
  bool customViewpoint = metaType.cameraAngles.customViewpoint();
  bool  noCA         = !customViewpoint && transform  == QLatin1String("ABS");
  bool pp            = Preferences::perspectiveProjection;
  float modelScale   = metaType.modelScale.value();
  float cameraFoV    = metaType.cameraFoV.value();
  float cameraAngleX = metaType.cameraAngles.value(XX);
  float cameraAngleY = metaType.cameraAngles.value(YY);
  Vector3 target     = Vector3(metaType.target.x(),metaType.target.y(),metaType.target.z());

  // Process substitute part attributes
  QString cleanPngName = pngName;
  if (keySub) {
    QStringList attributes = getImageAttributes(pngName);
    bool hr;
    if ((hr = attributes.size() == nHasRotstep) || attributes.size() == nHasTargetAndRotstep)
      noCA = attributes.at(hr ? nRotTrans : nRot_Trans).toUpper() == QLatin1String("ABS");
    if (attributes.size() >= nHasTarget)
      target = Vector3(attributes.at(nTargetX).toFloat(),attributes.at(nTargetY).toFloat(),attributes.at(nTargetZ).toFloat());
    if (keySub > PliBeginSub2Rc) {
      modelScale   = attributes.at(nModelScale).toFloat();
      cameraFoV    = attributes.at(nCameraFoV).toFloat();
      cameraAngleX = attributes.at(nCameraAngleXX).toFloat();
      cameraAngleY = attributes.at(nCameraAngleYY).toFloat();
    }
  }

  if (keySub || pliType == BOM)
    cleanPngName.replace(";", "_");

  /* determine camera distance */
  int cd = int(metaType.cameraDistance.value());
  if (!cd)
      cd = int(cameraDistance(meta,modelScale));

  bool useImageSize = metaType.imageSize.value(XX) > 0;
  int lineThickness = int(double(resolution())/72.0+0.5);
  int width  = useImageSize ? int(metaType.imageSize.value(XX)) : LPub::pageSize(meta.LPub.page, XX);
  int height = useImageSize ? int(metaType.imageSize.value(YY)) : LPub::pageSize(meta.LPub.page, YY);
  StudStyleMeta* ssm = meta.LPub.studStyle.value() ? &meta.LPub.studStyle : &metaType.studStyle;
  AutoEdgeColorMeta* aecm = meta.LPub.autoEdgeColor.enable.value() ? &meta.LPub.autoEdgeColor : &metaType.autoEdgeColor;
  HighContrastColorMeta* hccm = meta.LPub.studStyle.value() ? &meta.LPub.highContrast : &metaType.highContrast;

  if (pliType == SUBMODEL)
      noCA   = Preferences::applyCALocally || noCA;

  QString CA = QString("-ca%1") .arg(double(cameraFoV));

  QString cg;
  if (target.isPopulated()) {
      cg = QString("-co%1,%2,%3")
                   .arg(double(target.x))
                   .arg(double(target.y))
                   .arg(double(target.z));
  } else {
      cg = QString("-cg%1,%2,%3") .arg(noCA ? double(0.0f) : double(cameraAngleX))
                                  .arg(noCA ? double(0.0f) : double(cameraAngleY))
                                  .arg(cd);
  }

  QString J  = QString("-J");
  QString v  = QString("-v%1,%2")   .arg(width)
                                    .arg(height);
  QString o  = QString("-o0,-%1")   .arg(height/6);
  QString mf = QString("-mF%1")     .arg(cleanPngName);
  QString w  = QString("-W%1")      .arg(lineThickness);  // ldglite always deals in 72 DPI

  QString ss,ae,ac,ai,hs,hp,pb,hd;
  getStudStyleAndAutoEdgeSettings(ssm, hccm, aecm, ss, ae, ac, ai, hs, hp, pb, hd);

  QStringList arguments;
  arguments << CA;                  // Camera FOV in degrees
  arguments << cg;                  // camera globe - scale factor or model origin for the camera to look at
  arguments << J;                   // Perspective projection
  arguments << v;                   // display in X wide by Y high window
  arguments << o;                   // changes the centre X across and Y down
  arguments << w;                   // line thickness
  arguments << ss;                  // stud style
  arguments << ae;                  // automate edge color
  arguments << ac;                  // part edge contrast
  arguments << ai;                  // part color value light/dark index (saturation)
  arguments << hs;                  // stud cylinder color
  arguments << hp;                  // part edge color
  arguments << pb;                  // black edge color
  arguments << hd;                  // dark edge color

  QStringList list;
  // First, load additional parms from meta if any
  list = splitParms(metaType.ldgliteParms.value());
  for (int i = 0; i < list.size(); i++) {
     if (list[i] != "" && list[i] != " ") {
         addArgument(arguments, list[i]);
      }
  }
  if (list.size())
      emit gui->messageSig(LOG_INFO,QObject::tr("LDGlite additional PLI renderer parameters %1") .arg(list.join(" ")));

  // Add ini parms if not already added from meta
  for (int i = 0; i < Preferences::ldgliteParms.size(); i++) {
      if (list.indexOf(QRegExp("^" + QRegExp::escape(Preferences::ldgliteParms[i]))) < 0) {
        addArgument(arguments, Preferences::ldgliteParms[i], QString(), 0/*POVGen*/, list.size());
      }
  }

  // add custom color file if exist
  if (!Preferences::altLDConfigPath.isEmpty()) {
    addArgument(arguments, QString("-ldcF%1").arg(Preferences::altLDConfigPath), "-ldcF", 0/*POVGen*/, list.size());
  }

  arguments << QDir::toNativeSeparators(mf);
  arguments << QDir::toNativeSeparators(ldrNames.first());

  removeEmptyStrings(arguments);

  emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("Executing LDGLite %1 PLI render - please wait...")
                                                    .arg(pp ? "Perspective" : "Orthographic"));

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

  QString message = QObject::tr("LDGLite PLI Arguments: %1 %2").arg(Preferences::ldgliteExe).arg(arguments.join(" "));
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
      emit gui->messageSig(LOG_ERROR,QObject::tr("LDGlite failed\n%1") .arg(str));
      return -1;
    }
  }

  QFile outputImageFile(cleanPngName);
  if (! outputImageFile.exists()) {
      emit gui->messageSig(LOG_ERROR,QObject::tr("LDGLite PLI image generation failed for %1 with message %2")
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
        int                type)
{
    Q_UNUSED(type)

    // paths
    QString tempPath  = QDir::toNativeSeparators(QDir::currentPath() + "/" + Paths::tmpDir);
    QString assemPath = QDir::toNativeSeparators(QDir::currentPath() + "/" + Paths::assemDir);

    // Populate render attributes
    QStringList ldviewParmslist;
    if (useLDViewSCall())
        ;
    else
        ldviewParmslist = splitParms(meta.LPub.assem.ldviewParms.value());
    QString transform  = meta.rotStep.value().type.toUpper();
    bool customViewpoint = meta.LPub.assem.cameraAngles.customViewpoint();
    bool noCA          = !customViewpoint && (Preferences::applyCALocally || transform == QLatin1String("ABS"));
    bool pp            = Preferences::perspectiveProjection;
    float modelScale   = meta.LPub.assem.modelScale.value();
    float cameraFoV    = meta.LPub.assem.cameraFoV.value();
    float cameraAngleX = noCA ? 0.0f : meta.LPub.assem.cameraAngles.value(XX);
    float cameraAngleY = noCA ? 0.0f : meta.LPub.assem.cameraAngles.value(YY);
    Vector3 target     = Vector3(meta.LPub.assem.target.x(),meta.LPub.assem.target.y(),meta.LPub.assem.target.z());
    StudStyleMeta* ssm = meta.LPub.studStyle.value() ? &meta.LPub.studStyle : &meta.LPub.assem.studStyle;
    AutoEdgeColorMeta* aecm = meta.LPub.autoEdgeColor.enable.value() ? &meta.LPub.autoEdgeColor : &meta.LPub.assem.autoEdgeColor;
    HighContrastColorMeta* hccm = meta.LPub.studStyle.value() ? &meta.LPub.highContrast : &meta.LPub.assem.highContrast;

    // Assemble compareKey and test csiParts if Single Call
    QString compareKey;
    if (useLDViewSCall()){

        // test first csiParts
        QFileInfo fileInfo(csiParts.first());
        if ( ! fileInfo.exists()) {
          emit gui->messageSig(LOG_ERROR,QObject::tr("CSI render input file was not found at the specified path [%1]")
                                                     .arg(csiParts.first()));
          return -1;
        }

        compareKey = QString("%1_%2_%3_%4")
                             .arg(double(modelScale))                    // 1
                             .arg(double(cameraFoV))                     // 2
                             .arg(double(cameraAngleX))                  // 3
                             .arg(double(cameraAngleY));                 // 4
        // append target vector if specified
        if (meta.LPub.assem.target.isPopulated())
            compareKey.append(QString("_%1_%2_%3")
                              .arg(double(meta.LPub.assem.target.x()))   // 5
                              .arg(double(meta.LPub.assem.target.y()))   // 6
                              .arg(double(meta.LPub.assem.target.z()))); // 7
        // append rotate type if specified
        if (meta.rotStep.isPopulated())
            compareKey.append(QString("_%1")                             // 8
                              .arg(transform.isEmpty() ? "REL" : transform));
    }

    /* determine camera distance */
    int cd = int(meta.LPub.assem.cameraDistance.value());
    if (!cd)
        cd = int(cameraDistance(meta,modelScale)*1700/1000);

    // set page size
    bool useImageSize = meta.LPub.assem.imageSize.value(XX) > 0;
    int width  = useImageSize ? int(meta.LPub.assem.imageSize.value(XX)) : LPub::pageSize(meta.LPub.page, XX);
    int height = useImageSize ? int(meta.LPub.assem.imageSize.value(YY)) : LPub::pageSize(meta.LPub.page, YY);

    // arguments settings
    bool usingSnapshotArgs = false;
    QStringList attributes;;

    // projection settings
    QString CA, cg;

    // parameter arguments;
    QStringList ldviewParmsArgs;

    auto getRendererSettings = [
            &pp,
            &cd,
            &target,
            &modelScale,
            &cameraFoV,
            &cameraAngleX,
            &cameraAngleY,
            &ldviewParmslist,
            &useImageSize] (
        QString     &CA,
        QString     &cg,
        QStringList &ldviewParmsArgs)
    {
        // additional LDView parameters;
        qreal cdf = LP3D_CDF;
        QString dz, dl, df = QString("-FOV=%1").arg(double(cameraFoV));
        bool pd = false, pl = false, pf = false, pz = false;
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
                if (!pd && !pl && !pf && !pz) {
                  addArgument(ldviewParmsArgs, ldviewParmslist[i]);    // 10. ldviewParms [usually empty]
                }
            }
        }

        // Set camera angle and camera globe and update arguments with perspective projection settings
        if (pp && pl && !pz)
            dz = QString("-DefaultZoom=%1").arg(double(modelScale));

        CA = pp ? df : QString("-ca%1") .arg(LP3D_CA);              // replace CA with FOV

        // Set alternate target position or use specified image size
        QString _mc;
        if (target.isPopulated())
            _mc = QString("-ModelCenter=%1,%2,%3 ").arg(double(target.x)).arg(double(target.y)).arg(double(target.z));
        if ((!_mc.isEmpty() && !pl) || (useImageSize && _mc.isEmpty())){
            // Set model center
            QString _dl = QString("-DefaultLatLong=%1,%2")
                                  .arg(double(cameraAngleX))
                                  .arg(double(cameraAngleY));
            QString _dz = QString("-DefaultZoom=%1").arg(double(modelScale));
            // Set zoom to fit when use image size specified
            QString _sz;
            if (useImageSize && _mc.isEmpty())
                _sz = QString(" -SaveZoomToFit=1");
            cg = QString("%1%2 %3%4").arg(_mc.isEmpty() ? "" : _mc).arg(_dl).arg(_dz).arg(_sz.isEmpty() ? "" : _sz);
        } else {
            cg = pp ? pl ? QString("-DefaultLatLong=%1 %2")
                                   .arg(dl)
                                   .arg(dz)                         // replace Camera Globe with DefaultLatLon and add DefaultZoom
                         : QString("-cg%1,%2,%3")
                                   .arg(double(cameraAngleX))
                                   .arg(double(cameraAngleY))
                                   .arg(QString::number(cd * cdf,'f',0) )
                    : QString("-cg%1,%2,%3")
                              .arg(double(cameraAngleX))
                              .arg(double(cameraAngleY))
                              .arg(cd);
        }

        // additional LDView parameters;
        if (ldviewParmsArgs.size()) {
            emit gui->messageSig(LOG_INFO,QObject::tr("LDView additional CSI renderer parameters: %1")
                                                      .arg(ldviewParmsArgs.join(" ")));
            cg.append(QString(" %1").arg(ldviewParmsArgs.join(" ")));
        }
    };

    auto processAttributes = [this, &meta, &usingSnapshotArgs, &getRendererSettings] (
        QStringList &attributes,
        Vector3     &target,
        bool        &noCA,
        int         &cd,
        QString     &CA,
        QString     &cg,
        QStringList &ldviewParmsArgs,
        float       &modelScale,
        float       &cameraFoV,
        float       &cameraAngleX,
        float       &cameraAngleY)
    {
        if (usingSnapshotArgs) {
            // set scale FOV and camera angles
            bool hr;
            if ((hr = attributes.size() == nHasRotstep) || attributes.size() == nHasTargetAndRotstep)
                noCA = attributes.at(hr ? nRotTrans : nRot_Trans).toUpper() == QLatin1String("ABS");
            // set target attribute
            if (attributes.size() >= nHasTarget)
                target = Vector3(attributes.at(nTargetX).toFloat(),attributes.at(nTargetY).toFloat(),attributes.at(nTargetZ).toFloat());
            // set scale FOV and camera angles
            modelScale   = attributes.at(nModelScale).toFloat();
            cameraFoV    = attributes.at(nCameraFoV).toFloat();
            cameraAngleX = noCA ? 0.0f : attributes.at(nCameraAngleXX).toFloat();
            cameraAngleY = noCA ? 0.0f : attributes.at(nCameraAngleYY).toFloat();
            cd = int(cameraDistance(meta,modelScale)*1700/1000);
            getRendererSettings(CA, cg, ldviewParmsArgs);
        }
    };

    auto getSnapshotArgsAttributes = [] (const QString& snapshotArgsKey)
    {
        // treat parts with '_' in the name
        const int index = snapshotArgsKey.indexOf("-");
        // 1. get type name without renderer index
        const QString type = snapshotArgsKey.left(index);
        // 2. remove renderer index and delimiter from string - e.g. '-0'
        QString cleanString = snapshotArgsKey.right(snapshotArgsKey.length() - (index + 2));
        // 3. prepend type
        cleanString.prepend(QString("%1").arg(type));

        return cleanString.split("_");
    };

    /* Create the CSI DAT file(s) */

    QString f, snapshotArgsKey, imageMatteArgsKey;
    bool usingListCmdArg     = false;
    bool usingDefaultArgs    = true;
    bool usingSingleSetArgs  = false;
    bool snapshotArgsChanged = false;
    bool enableIM            = false;
    QStringList ldrNames, ldrNamesIM, snapshotLdrs;
    if (useLDViewSCall()) {  // Use LDView SingleCall
        // populate ldrNames
        if (Preferences::enableFadeSteps && Preferences::enableImageMatting){  // ldrName entries (IM ON)
            enableIM = true;
            Q_FOREACH (QString csiEntry, csiParts){           // csiParts are ldrNames under LDViewSingleCall
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

        // process part attributes
        QString snapshotsCmdLineArgs,snapshotArgs;
        QStringList snapShotsListArgs,keys;
        for (int i = 0; i < ldrNames.size(); i++) {
            QString ldrName = ldrNames.at(i);

            if (!QFileInfo(ldrName).exists()) {
                emit gui->messageSig(LOG_ERROR, QObject::tr("LDR file %1 not found.").arg(ldrName));
                continue;
            }

            // split snapshot, imageMatte and additional renderer keys,
            keys              = csiKeys.at(i).split("|");
            snapshotArgsKey   = keys.at(0); // using compareKey - keyPart2 and rotStep
            imageMatteArgsKey = keys.at(1); // using nameAndStepKey - csi_name, nameExtension, renderer index
            if (keys.size() == 3)
                ldviewParmslist = keys.at(2).split(" ");

            attributes = getSnapshotArgsAttributes(snapshotArgsKey);

            if (attributes.size() > 2)
                attributes.replace(1,"0"); // replace step number with 0

            // attributes are different from default
            usingSnapshotArgs = compareImageAttributes(attributes, compareKey, usingDefaultArgs);
            if (usingSnapshotArgs){
                processAttributes(attributes, target, noCA, cd, CA, cg, ldviewParmsArgs,
                                  modelScale, cameraFoV, cameraAngleX, cameraAngleY);
                snapshotArgsChanged = !usingDefaultArgs;
                usingDefaultArgs    = usingDefaultArgs ? false: usingDefaultArgs;
                usingSnapshotArgs   = false;                // reset
                compareKey          = attributes.join("_");

                if (!snapshotArgsChanged) {
                    snapshotArgs = QString("%1 %2").arg(CA).arg(cg);
                }
            } else {
                getRendererSettings(CA, cg, ldviewParmsArgs);
            }

            snapshotLdrs.append(ldrName);

            if (!usingDefaultArgs) {

                QString saveArgs = QString("-SaveSnapShots=1 %1").arg(ldrName);
                saveArgs.prepend(QString("%1 %2 ").arg(CA).arg(cg));
                snapShotsListArgs.append(QString(" %1").arg(saveArgs));
            }
        }

        // using same snapshot args for all parts
        usingSnapshotArgs = !usingDefaultArgs && !snapshotArgsChanged;

        if (snapshotLdrs.size() ) {
            // using default args or same snapshot args for all parts
            if ((usingSingleSetArgs = usingDefaultArgs || usingSnapshotArgs)) {

                // using same snapshot args for all parts
                if (usingSnapshotArgs) {
                    keys = snapshotArgs.split(" ");
                    CA   = keys.at(0);
                    cg   = keys.mid(1).join(" ");
                }

                // use single line snapshots command
                if (snapshotLdrs.size() < SNAPSHOTS_LIST_THRESHOLD || !useLDViewSList()) {

                    // use single line snapshots command
                    snapshotsCmdLineArgs = QString("-SaveSnapShots=1");

                }
                // create snapshot list
                else {

                    usingListCmdArg = true;
                    QString SnapshotsList = tempPath + QDir::separator() + "pliSnapshotsList.lst";
                    if (!createSnapshotsList(snapshotLdrs,SnapshotsList))
                        return -1;
                    snapshotsCmdLineArgs = QString("-SaveSnapshotsList=%1").arg(SnapshotsList);
                }

                f  = snapshotsCmdLineArgs;
            }
            // create a command lines list - we have subSnapShotsListArgs or not usingDefaultArgs
            else
            {
                usingListCmdArg = true;
                QString CommandLinesList = tempPath + QDir::separator() + "pliCommandLinesList.lst";
                QFile CommandLinesListFile(CommandLinesList);
                if ( ! CommandLinesListFile.open(QFile::WriteOnly | QFile::Text)) {
                    emit gui->messageSig(LOG_ERROR,QObject::tr("Failed to create LDView (Single Call) PLI CommandLines list file!"));
                    return -1;
                }

                QTextStream out(&CommandLinesListFile);
                // add normal snapshot lines
                if (snapshotLdrs.size()) {
                    Q_FOREACH (QString argsLine,snapShotsListArgs) {
                        out << argsLine << lpub_endl;
                        if (Preferences::debugLogging)
                            emit gui->messageSig(LOG_DEBUG, QObject::tr("Wrote %1 to CSI Command line list").arg(argsLine));
                    }
                }
                CommandLinesListFile.close();

                f  = QString("-CommandLinesList=%1").arg(CommandLinesList);    // run in renderCsi
            }
        }

    } else { // End Use SingleCall

        usingSingleSetArgs = usingDefaultArgs;

        int rc;
        QString csiKey = QString();
        if (Preferences::enableFadeSteps && Preferences::enableImageMatting &&
                LDVImageMatte::validMatteCSIImage(csiKeys.first())) {                    // ldrName entries (IM ON)
            enableIM = true;
            csiKey = csiKeys.first();
        }

        ldrNames << QDir::fromNativeSeparators(tempPath + "/csi.ldr");

        getRendererSettings(CA, cg, ldviewParmsArgs);

        FloatPairMeta cameraAngles;
        cameraAngles.setValues(meta.LPub.assem.cameraAngles.value(0),
                               meta.LPub.assem.cameraAngles.value(1));

        // RotateParts #2 - 8 parms
        if ((rc = rotateParts(addLine, meta.rotStep, csiParts, ldrNames.first(), csiKey, cameraAngles,DT_DEFAULT,Options::CSI)) < 0) {
            emit gui->messageSig(LOG_ERROR,QObject::tr("LDView CSI rotate parts failed!"));
            return rc;
        } else
          // recheck csiKey - may have been deleted by rotateParts if IM files not created.
          if (enableIM) {
            enableIM = LDVImageMatte::validMatteCSIImage(csiKeys.first());
        }

        f  = QString("-SaveSnapShot=%1") .arg(pngName);
    }

    bool haveLdrNames   = !ldrNames.isEmpty();
    bool haveLdrNamesIM = !ldrNamesIM.isEmpty();

    QString ss,ae,ac,ai,hs,hp,pb,hd;
    getStudStyleAndAutoEdgeSettings(ssm, hccm, aecm, ss, ae, ac, ai, hs, hp, pb, hd);
    QString w  = QString("-SaveWidth=%1")  .arg(width);
    QString h  = QString("-SaveHeight=%1") .arg(height);
    QString l  = QString("-LDrawDir=%1")   .arg(Preferences::ldrawLibPath);
    QString o  = QString("-HaveStdOut=1");
    QString v  = QString("-vv");

    QStringList arguments;
    if (usingSingleSetArgs){
        arguments << CA;             // Camera Angle (i.e. Field of Veiw)
        arguments << cg.split(" ");  // Camera Globe, Target and Additional Parameters when specified
    }

    arguments << f;  // -CommandLinesList | -SaveSnapshotsList | -SaveSnapShots | -SaveSnapShot
    arguments << ss; // stud style
    arguments << ae; // automate edge color
    arguments << ac; // part edge contrast
    arguments << ai; // part color value light/dark index (saturation)
    arguments << hs; // stud cylinder color
    arguments << hp; // part edge color
    arguments << pb; // black edge color
    arguments << hd; // dark edge color
    arguments << w;  // -SaveWidth
    arguments << h;  // -SaveHeight
    arguments << l;  // -LDrawDir
    arguments << o;  // -HaveStdOut
    arguments << v;  // -vv (Verbose)

    QString ini;
    if(!Preferences::ldviewIni.isEmpty()) {
        ini = QString("-IniFile=%1") .arg(Preferences::ldviewIni);
        addArgument(arguments, ini, "-IniFile", 0/*POVGen*/, ldviewParmsArgs.size());
    }

    QString altldc;
    if (!Preferences::altLDConfigPath.isEmpty()) {
        altldc = QString("-LDConfig=%1").arg(Preferences::altLDConfigPath);
        addArgument(arguments, altldc, "-LDConfig", 0/*POVGen*/, ldviewParmsArgs.size());
    }

    if (haveLdrNames) {
        if (useLDViewSCall()) {
            //-SaveSnapShots=1
            if ((!useLDViewSList() && !usingListCmdArg) ||
                (useLDViewSList() && snapshotLdrs.size() < SNAPSHOTS_LIST_THRESHOLD))
                arguments = arguments + snapshotLdrs;  // 13. LDR input file(s)
        } else {
            // SaveSnapShot=1
            arguments << QDir::toNativeSeparators(ldrNames.first());

        }

        removeEmptyStrings(arguments);

        emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("Executing LDView %1 CSI render - please wait...")
                                                          .arg(pp ? "Perspective" : "Orthographic"));

        // execute LDView process
        if (executeLDViewProcess(arguments, Options::CSI) != 0) // ldrName entries that ARE NOT IM exist - e.g. first step
            return -1;
    }

    // Build IM arguments and process IM [Not implemented - not updated with perspective 'pp' routines]
    QStringList im_arguments;
    if (enableIM && haveLdrNamesIM) {
        QString a  = QString("-AutoCrop=0");
        im_arguments << CA;                         // 00. Camera FOV in degrees
        im_arguments << cg.split(" ");              // 01. Camera globe
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
        if (!ini.isEmpty())
            im_arguments << ini;                    // 11. LDView.ini

        if (!altldc.isEmpty())
            im_arguments << altldc;                 // 12.Alternate LDConfig

        removeEmptyStrings(arguments);

        if (useLDViewSCall()){

            if (enableIM) {
                if (haveLdrNamesIM) {
                    // IM each ldrNameIM file
                    emit gui->messageSig(LOG_STATUS, "Executing LDView render Image Matte CSI - please wait...");

                    Q_FOREACH (QString ldrNameIM, ldrNamesIM){
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
        Q_FOREACH (QString ldrName, ldrNames){
            QString pngFileTmpPath = ldrName.replace(".ldr",".png");
            QString pngFilePath = QString("%1/%2").arg(assemPath).arg(QFileInfo(pngFileTmpPath).fileName());
            QFile destinationFile(pngFilePath);
            QFile sourceFile(pngFileTmpPath);
            if (! destinationFile.exists() || destinationFile.remove()) {
                if (! sourceFile.rename(destinationFile.fileName()))
                    emit gui->messageSig(LOG_ERROR,QObject::tr("LDView CSI image move failed for %1").arg(pngFilePath));
            } else {
                emit gui->messageSig(LOG_ERROR,QObject::tr("LDView could not remove old CSI image file %1").arg(pngFilePath));
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

  // test ldrNames
  QString cleanLdrName = ldrNames.first();
  QStringList cleanLdrNames = ldrNames;

  if (keySub || pliType == BOM)
    cleanLdrName = QString(cleanLdrNames.first()).replace(";", "_");

  QFileInfo fileInfo(cleanLdrName);
  if ( ! fileInfo.exists()) {
      emit gui->messageSig(LOG_ERROR,QObject::tr("PLI render input file was not found at the specified path [%1]")
                                                 .arg(cleanLdrName));
    return -1;
  }

  // paths
  QString tempPath  = QDir::toNativeSeparators(QDir::currentPath() + "/" + Paths::tmpDir);
  QString partsDir  = pliType == BOM ? Paths::bomDir : Paths::partsDir;
  QString partsPath = QDir::toNativeSeparators(QDir::currentPath() + "/" + (pliType == SUBMODEL ? Paths::submodelDir : partsDir));

  // Populate render attributes
  QStringList ldviewParmslist = splitParms(metaType.ldviewParms.value());
  QString transform  = metaType.rotStep.value().type.toUpper();
  bool customViewpoint = metaType.cameraAngles.customViewpoint();
  bool noCA          = !customViewpoint && pliType == SUBMODEL ? Preferences::applyCALocally || transform == QLatin1String("ABS") : transform == QLatin1String("ABS");
  bool pp            = Preferences::perspectiveProjection;
  float modelScale   = metaType.modelScale.value();
  float cameraFoV    = metaType.cameraFoV.value();
  float cameraAngleX = noCA ? 0.0f : metaType.cameraAngles.value(XX);
  float cameraAngleY = noCA ? 0.0f : metaType.cameraAngles.value(YY);
  Vector3 target     = Vector3(metaType.target.x(),metaType.target.y(),metaType.target.z());
  StudStyleMeta* ssm = meta.LPub.studStyle.value() ? &meta.LPub.studStyle : &metaType.studStyle;
  AutoEdgeColorMeta* aecm = meta.LPub.autoEdgeColor.enable.value() ? &meta.LPub.autoEdgeColor : &metaType.autoEdgeColor;
  HighContrastColorMeta* hccm = meta.LPub.studStyle.value() ? &meta.LPub.highContrast : &metaType.highContrast;

  // Assemble compareKey if Single Call
  QString compareKey;
  if (useLDViewSCall()){
      compareKey = QString("%1_%2_%3_%4")
                           .arg(double(modelScale))             // 1
                           .arg(double(cameraFoV))              // 2
                           .arg(double(cameraAngleX))           // 3
                           .arg(double(cameraAngleY));          // 4
      // append target vector if specified
      if (metaType.target.isPopulated())
          compareKey.append(QString("_%1_%2_%3")
                            .arg(double(metaType.target.x()))   // 5
                            .arg(double(metaType.target.y()))   // 6
                            .arg(double(metaType.target.z()))); // 7
      // append rotate type if specified
      if (keySub > PliBeginSub5Rc || metaType.rotStep.isPopulated())
          compareKey.append(QString("_%1")                      // 8
                            .arg(transform.isEmpty() ? "REL" : transform));
  }

  /* determine camera distance */
  int cd = int(metaType.cameraDistance.value());
  if (!cd)
      cd = int(cameraDistance(meta,modelScale)*1700/1000);

  //qDebug() << qPrintable(QString("LDView (Default) Camera Distance: %1").arg(cd));

  // set page size
  bool useImageSize = metaType.imageSize.value(XX) > 0;
  int width  = useImageSize ? int(metaType.imageSize.value(XX)) : LPub::pageSize(meta.LPub.page, XX);
  int height = useImageSize ? int(metaType.imageSize.value(YY)) : LPub::pageSize(meta.LPub.page, YY);

  // arguments settings
  bool usingListCmdArg   = false;
  bool usingSnapshotArgs = false;
  QStringList attributes, snapshotLdrs;

  // projection settings
  qreal cdf = LP3D_CDF;
  bool pd = false, pl = false, pf = false, pz = false;
  QString dz, dl, df = QString("-FOV=%1").arg(double(cameraFoV));
  QString CA, cg;

  // additional LDView parameters;
  QStringList ldviewParmsArgs;

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
  if (pp && pl && !pz)
      dz = QString("-DefaultZoom=%1").arg(double(modelScale));

  auto getRendererSettings = [
          &pp,
          &dl,
          &df,
          &dz,
          &pl,
          &cd,
          &cdf,
          &target,
          &modelScale,
          &cameraAngleX,
          &cameraAngleY,
          &useImageSize] (
          QString &CA,
          QString &cg)
  {
      CA = pp ? df : QString("-ca%1") .arg(LP3D_CA);              // replace CA with FOV
      // Set alternate target position or use specified image size
      QString _mc;
      if (target.isPopulated())
          _mc = QString("-ModelCenter=%1,%2,%3").arg(double(target.x)).arg(double(target.y)).arg(double(target.z));
      if ((!_mc.isEmpty() && !pl) || (useImageSize && _mc.isEmpty())){
          // Set model center
          QString _dl = QString("-DefaultLatLong=%1,%2")
                                .arg(double(cameraAngleX))
                                .arg(double(cameraAngleY));
          QString _dz = QString("-DefaultZoom=%1").arg(double(modelScale));
          // Set zoom to fit when use image size specified
          QString _sz;
          if (useImageSize && _mc.isEmpty())
              _sz = QString(" -SaveZoomToFit=1");
          cg = QString("%1 %2 %3%4").arg(_mc).arg(_dl).arg(_dz).arg(_sz.isEmpty() ? "" : _sz);
      } else {
          cg = pp ? pl ? QString("-DefaultLatLong=%1 %2")
                         .arg(dl)
                         .arg(dz)                             // replace Camera Globe with DefaultLatLon and add DefaultZoom
                       : QString("-cg%1,%2,%3")
                         .arg(double(cameraAngleX))
                         .arg(double(cameraAngleY))
                         .arg(QString::number(cd * cdf,'f',0) )
                       : QString("-cg%1,%2,%3")
                         .arg(double(cameraAngleX))
                         .arg(double(cameraAngleY))
                         .arg(cd);
      }
  };

  auto processAttributes = [
          this,
          &meta,
          &keySub,
          &usingSnapshotArgs,
          &getRendererSettings] (
      QStringList &attributes,
      Vector3     &target,
      bool        &noCA,
      int         &cd,
      QString     &CA,
      QString     &cg,
      float       &modelScale,
      float       &cameraFoV,
      float       &cameraAngleX,
      float       &cameraAngleY)
  {
      if (keySub > PliBeginSub2Rc || usingSnapshotArgs) {
          // set scale FOV and camera angles
          bool hr;
          if ((hr = attributes.size() == nHasRotstep) || attributes.size() == nHasTargetAndRotstep)
              noCA = attributes.at(hr ? nRotTrans : nRot_Trans).toUpper() == QLatin1String("ABS");
          // set target attribute
          if (attributes.size() >= nHasTarget)
              target = Vector3(attributes.at(nTargetX).toFloat(),attributes.at(nTargetY).toFloat(),attributes.at(nTargetZ).toFloat());
          // set scale FOV and camera angles

          modelScale   = attributes.at(nModelScale).toFloat();
          cameraFoV    = attributes.at(nCameraFoV).toFloat();
          cameraAngleX = noCA ? 0.0f : attributes.at(nCameraAngleXX).toFloat();
          cameraAngleY = noCA ? 0.0f : attributes.at(nCameraAngleYY).toFloat();
          cd = int(cameraDistance(meta,modelScale)*1700/1000);
          getRendererSettings(CA,cg);
      }
  };

  /* Create the PLI DAT file(s) */

  QString f, cleanPngName;
  bool hasSubstitutePart   = false;
  bool usingDefaultArgs    = true;
  bool usingSingleSetArgs  = false;
  bool snapshotArgsChanged = false;

  if (useLDViewSCall() && pliType != SUBMODEL) {  // Use LDView SingleCall

      // process part attributes
      QString snapshotsCmdLineArgs,snapshotArgs;
      QStringList snapShotsListArgs, subSnapShotsListArgs;
      cleanLdrNames.clear();

      for (QString const &ldrName : ldrNames) {

          cleanLdrName = ldrName;

          if (keySub || pliType == BOM)
              cleanLdrName = QString(ldrName).replace(";", "_");
          cleanLdrNames << cleanLdrName;

          if (!QFileInfo(cleanLdrName).exists()) {
              emit gui->messageSig(LOG_ERROR, QObject::tr("LDR file %1 not found.").arg(cleanLdrName));
              continue;
          }

          // get attribues from ldrName key
          attributes = getImageAttributes(ldrName);

          // determine if is substitute part
          hasSubstitutePart = keySub && attributes.endsWith("SUB");

          // attributes are different from default
          usingSnapshotArgs = compareImageAttributes(attributes, compareKey, usingDefaultArgs);
          if (usingSnapshotArgs){
              processAttributes(attributes, target, noCA, cd, CA, cg, modelScale, cameraFoV, cameraAngleX, cameraAngleY);
              snapshotArgsChanged = !usingDefaultArgs;
              usingDefaultArgs    = usingDefaultArgs ? false : usingDefaultArgs;
              usingSnapshotArgs   = false;                // reset
              compareKey          = attributes.join("_");

              if (!snapshotArgsChanged)
                  snapshotArgs = QString("%1 %2").arg(CA).arg(cg);

          } else {
              getRendererSettings(CA,cg);
          }

          // if substitute, trigger command list
          if (hasSubstitutePart) {

             usingDefaultArgs = false;
             cleanPngName = cleanLdrName.replace("_SUB.ldr",".png");

             // use command list as pngName and ldrName must be specified
             subSnapShotsListArgs.append(QString("%1 %2 -SaveSnapShot=%3 %4").arg(CA).arg(cg).arg(cleanPngName).arg(cleanLdrName));

          } else {

             // if using different snapshot args, trigger command list
             if (!usingDefaultArgs) {

                 const QString saveArgs = QString("-SaveSnapShots=1 %1").arg(cleanLdrName);
                 snapShotsListArgs.append(QString("%1 %2 %3").arg(CA).arg(cg).arg(saveArgs));
             }
          }

          snapshotLdrs.append(cleanLdrName);
      }

      // using same snapshot args for all parts
      usingSnapshotArgs = !usingDefaultArgs && !snapshotArgsChanged && !hasSubstitutePart;

      if (snapshotLdrs.size()) {
          // using default args or same snapshot args for all parts
          if ((usingSingleSetArgs = usingDefaultArgs || usingSnapshotArgs)) {

              // using same snapshot args for all parts
              if (usingSnapshotArgs) {
                  QStringList keys = snapshotArgs.split(" ");
                  CA = keys.at(0);
                  cg = keys.mid(1).join(" ");
              }

              // use single line snapshots command
              if (snapshotLdrs.size() < SNAPSHOTS_LIST_THRESHOLD || !useLDViewSList()) {

                  snapshotsCmdLineArgs = QString("-SaveSnapShots=1");

              }
              // create snapshot list
              else {

                  usingListCmdArg = true;
                  QString SnapshotsList = tempPath + QDir::separator() + "pliSnapshotsList.lst";
                  if (!createSnapshotsList(snapshotLdrs,SnapshotsList))
                      return -1;
                  snapshotsCmdLineArgs = QString("-SaveSnapshotsList=%1").arg(SnapshotsList);
              }

              f  = snapshotsCmdLineArgs;
          }
          // create a command lines list - we have subSnapShotsListArgs or not usingDefaultArgs
          else
          {
              usingListCmdArg = true;
              QString CommandLinesList = tempPath + QDir::separator() + "pliCommandLinesList.lst";
              QFile CommandLinesListFile(CommandLinesList);
              if ( ! CommandLinesListFile.open(QFile::WriteOnly | QFile::Text)) {
                  emit gui->messageSig(LOG_ERROR,QObject::tr("Failed to create LDView (Single Call) PLI CommandLines list file!"));
                  return -1;
              }

              QTextStream out(&CommandLinesListFile);
              if (snapshotLdrs.size()) {
                  // add normal snapshot lines
                  Q_FOREACH (QString argsLine,snapShotsListArgs) {
                      out << argsLine << lpub_endl;
                      if (Preferences::debugLogging)
                          emit gui->messageSig(LOG_DEBUG, QObject::tr("Wrote %1 to PLI Command line list").arg(argsLine));
                  }
                  // add substitute snapshot lines
                  Q_FOREACH (QString argsLine,subSnapShotsListArgs) {
                      out << argsLine << lpub_endl;
                      if (Preferences::debugLogging)
                          emit gui->messageSig(LOG_DEBUG, QObject::tr("Wrote %1 to PLI Substitute Command line list").arg(argsLine));
                  }
              }
              CommandLinesListFile.close();

              f  = QString("-CommandLinesList=%1").arg(CommandLinesList);    // run in renderCsi
          }
      }

  } else { // End Use SingleCall

      usingSingleSetArgs = usingDefaultArgs;
      cleanPngName = pngName;
      if (keySub) {
          // process substitute attributes
          attributes = getImageAttributes(pngName);
          hasSubstitutePart = attributes.endsWith("SUB");
          processAttributes(attributes, target, noCA, cd, CA, cg, modelScale, cameraFoV, cameraAngleX, cameraAngleY);
      } else {
          getRendererSettings(CA,cg);
      }

      if (keySub || pliType == BOM)
          cleanPngName.replace(";", "_");

      f  = QString("-SaveSnapShot=%1") .arg(cleanPngName);
  }

  QString ss,ae,ac,ai,hs,hp,pb,hd;
  getStudStyleAndAutoEdgeSettings(ssm, hccm, aecm, ss, ae, ac, ai, hs, hp, pb, hd);
  QString w  = QString("-SaveWidth=%1")  .arg(width);
  QString h  = QString("-SaveHeight=%1") .arg(height);
  QString l  = QString("-LDrawDir=%1")   .arg(Preferences::ldrawLibPath);
  QString o  = QString("-HaveStdOut=1");
  QString v  = QString("-vv");

  QStringList arguments;
  if (usingSingleSetArgs){
      arguments << CA;             // Camera Angle (i.e. Field of Veiw)
      arguments << cg.split(" ");  // Camera Globe, Target and Additional Parameters when specified
  }

  // append additional LDView parameters
  if (ldviewParmsArgs.size()) {
    for (int i = 0; i < ldviewParmsArgs.size(); i++) {
      addArgument(arguments, ldviewParmsArgs[i]);
    }
    emit gui->messageSig(LOG_INFO,QObject::tr("LDView additional PLI renderer parameters: %1")
                         .arg(ldviewParmsArgs.join(" ")));
  }

  arguments << f;  // -CommandLinesList | -SaveSnapshotsList | -SaveSnapShots | -SaveSnapShot
  arguments << ss; // stud style
  arguments << ae; // automate edge color
  arguments << ac; // part edge contrast
  arguments << ai; // part color value light/dark index (saturation)
  arguments << hs; // stud cylinder color
  arguments << hp; // part edge color
  arguments << pb; // black edge color
  arguments << hd; // dark edge color
  arguments << w;  // -SaveWidth
  arguments << h;  // -SaveHeight
  arguments << l;  // -LDrawDir
  arguments << o;  // -HaveStdOut
  arguments << v;  // -vv (Verbose)

  QString newArg;
  if(!Preferences::ldviewIni.isEmpty()){
      newArg = QString("-IniFile=%1") .arg(Preferences::ldviewIni);
      addArgument(arguments, newArg, "-IniFile", 0/*POVGen*/, ldviewParmsArgs.size());
  }

  if (!Preferences::altLDConfigPath.isEmpty()) {
      newArg = QString("-LDConfig=%1").arg(Preferences::altLDConfigPath);
      addArgument(arguments, newArg, "-LDConfig", 0/*POVGen*/, ldviewParmsArgs.size());
  }

  if (useLDViewSCall() && pliType != SUBMODEL) {
      //-SaveSnapShots=1
      if (!hasSubstitutePart &&
           ((!useLDViewSList() && !usingListCmdArg) ||
            (useLDViewSList() && snapshotLdrs.size() < SNAPSHOTS_LIST_THRESHOLD)))
          arguments = arguments + snapshotLdrs;  // 13. LDR input file(s)
  } else {
      //-SaveSnapShot=%1
      arguments << QDir::toNativeSeparators(cleanLdrNames.first());
  }

  removeEmptyStrings(arguments);

  emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("Executing LDView %1 PLI render - please wait...")
                                                    .arg(pp ? "Perspective" : "Orthographic"));

  // execute LDView process
  if (executeLDViewProcess(arguments, Options::PLI) != 0)
      return -1;

  // move generated PLI images to parts subfolder
  if (useLDViewSCall() && pliType != SUBMODEL) {
      for (QString cleanLdrName : cleanLdrNames) {
          QString pngFileTmpPath = cleanLdrName.endsWith("_SUB.ldr") ?
                                   cleanLdrName.replace("_SUB.ldr",".png") :
                                   cleanLdrName.replace(".ldr",".png");
          QString pngFilePath = partsPath + QDir::separator() + QFileInfo(pngFileTmpPath).fileName();
          QFile destinationFile(pngFilePath);
          QFile sourceFile(pngFileTmpPath);
          if (! destinationFile.exists() || destinationFile.remove()) {
              if (! sourceFile.rename(destinationFile.fileName()))
                  emit gui->messageSig(LOG_ERROR,QObject::tr("LDView PLI image move failed for %1").arg(pngFilePath));
          } else {
              emit gui->messageSig(LOG_ERROR,QObject::tr("LDView could not remove old PLI image file %1").arg(pngFilePath));
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

  // process native settings

  float camDistance    = meta.LPub.assem.cameraDistance.value();
  float cameraAngleX   = meta.LPub.assem.cameraAngles.value(XX);
  float cameraAngleY   = meta.LPub.assem.cameraAngles.value(YY);
  float modelScale     = meta.LPub.assem.modelScale.value();
  float cameraFoV      = meta.LPub.assem.cameraFoV.value();
  float cameraZNear    = meta.LPub.assem.cameraZNear.value();
  float cameraZFar     = meta.LPub.assem.cameraZFar.value();
  bool  isOrtho        = meta.LPub.assem.isOrtho.value();
  bool  customViewpoint= meta.LPub.assem.cameraAngles.customViewpoint();
  bool  fadeParts      = meta.LPub.assem.fadeSteps.enable.value();
  bool  highlightParts = meta.LPub.assem.highlightStep.enable.value();
  bool  lpubFade       = meta.LPub.assem.fadeSteps.lpubFade.value();
  bool  lpubHighlight  = meta.LPub.assem.highlightStep.lpubHighlight.value();
  QString cameraName   = meta.LPub.assem.cameraName.value();
  Vector3 position     = Vector3(meta.LPub.assem.position.x(),meta.LPub.assem.position.y(),meta.LPub.assem.position.z());
  Vector3 target       = Vector3(meta.LPub.assem.target.x(),meta.LPub.assem.target.y(),meta.LPub.assem.target.z());
  Vector3 upvector     = Vector3(meta.LPub.assem.upvector.x(),meta.LPub.assem.upvector.y(),meta.LPub.assem.upvector.z());
  StudStyleMeta* ssm   = meta.LPub.studStyle.value() ? &meta.LPub.studStyle : &meta.LPub.assem.studStyle;
  AutoEdgeColorMeta* aecm = meta.LPub.autoEdgeColor.enable.value() ? &meta.LPub.autoEdgeColor : &meta.LPub.assem.autoEdgeColor;
  HighContrastColorMeta* hccm = meta.LPub.studStyle.value() ? &meta.LPub.highContrast : &meta.LPub.assem.highContrast;
  CameraViews::CameraView cameraView = meta.LPub.assem.cameraAngles.cameraView();
  if (nType == NTypeCalledOut) {
    camDistance        = meta.LPub.callout.csi.cameraDistance.value();
    cameraAngleX       = meta.LPub.callout.csi.cameraAngles.value(XX);
    cameraAngleY       = meta.LPub.callout.csi.cameraAngles.value(YY);
    cameraView         = meta.LPub.callout.csi.cameraAngles.cameraView();
    customViewpoint    = meta.LPub.callout.csi.cameraAngles.customViewpoint();
    modelScale         = meta.LPub.callout.csi.modelScale.value();
    cameraFoV          = meta.LPub.callout.csi.cameraFoV.value();
    cameraZNear        = meta.LPub.callout.csi.cameraZNear.value();
    cameraZFar         = meta.LPub.callout.csi.cameraZFar.value();
    isOrtho            = meta.LPub.callout.csi.isOrtho.value();
    fadeParts          = meta.LPub.callout.csi.fadeSteps.enable.value();
    highlightParts     = meta.LPub.callout.csi.highlightStep.enable.value();
    lpubFade           = meta.LPub.callout.csi.fadeSteps.lpubFade.value();
    lpubHighlight      = meta.LPub.callout.csi.highlightStep.lpubHighlight.value();
    cameraName         = meta.LPub.callout.csi.cameraName.value();
    position           = Vector3(meta.LPub.callout.csi.position.x(),meta.LPub.callout.csi.position.y(),meta.LPub.callout.csi.position.z());
    target             = Vector3(meta.LPub.callout.csi.target.x(),meta.LPub.callout.csi.target.y(),meta.LPub.callout.csi.target.z());
    upvector           = Vector3(meta.LPub.callout.csi.upvector.x(),meta.LPub.callout.csi.upvector.y(),meta.LPub.callout.csi.upvector.z());
    ssm                = meta.LPub.studStyle.value() ? &meta.LPub.studStyle : &meta.LPub.callout.csi.studStyle;
    aecm               = meta.LPub.autoEdgeColor.enable.value() ? &meta.LPub.autoEdgeColor : &meta.LPub.callout.csi.autoEdgeColor;
    hccm               = meta.LPub.studStyle.value() ? &meta.LPub.highContrast : &meta.LPub.callout.csi.highContrast;
  } else if (nType == NTypeMultiStep) {
    camDistance        = meta.LPub.multiStep.csi.cameraDistance.value();
    cameraAngleX       = meta.LPub.multiStep.csi.cameraAngles.value(XX);
    cameraAngleY       = meta.LPub.multiStep.csi.cameraAngles.value(YY);
    cameraView         = meta.LPub.multiStep.csi.cameraAngles.cameraView();
    customViewpoint    = meta.LPub.multiStep.csi.cameraAngles.customViewpoint();
    modelScale         = meta.LPub.multiStep.csi.modelScale.value();
    cameraFoV          = meta.LPub.multiStep.csi.cameraFoV.value();
    cameraZNear        = meta.LPub.multiStep.csi.cameraZNear.value();
    cameraZFar         = meta.LPub.multiStep.csi.cameraZFar.value();
    isOrtho            = meta.LPub.multiStep.csi.isOrtho.value();
    fadeParts          = meta.LPub.multiStep.csi.fadeSteps.enable.value();
    highlightParts     = meta.LPub.multiStep.csi.highlightStep.enable.value();
    lpubFade           = meta.LPub.multiStep.csi.fadeSteps.lpubFade.value();
    lpubHighlight      = meta.LPub.multiStep.csi.highlightStep.lpubHighlight.value();
    cameraName         = meta.LPub.multiStep.csi.cameraName.value();
    position           = Vector3(meta.LPub.multiStep.csi.position.x(),meta.LPub.multiStep.csi.position.y(),meta.LPub.multiStep.csi.position.z());
    target             = Vector3(meta.LPub.multiStep.csi.target.x(),meta.LPub.multiStep.csi.target.y(),meta.LPub.multiStep.csi.target.z());
    upvector           = Vector3(meta.LPub.multiStep.csi.upvector.x(),meta.LPub.multiStep.csi.upvector.y(),meta.LPub.multiStep.csi.upvector.z());
    ssm                = meta.LPub.studStyle.value() ? &meta.LPub.studStyle : &meta.LPub.multiStep.csi.studStyle;
    aecm               = meta.LPub.autoEdgeColor.enable.value() ? &meta.LPub.autoEdgeColor : &meta.LPub.multiStep.csi.autoEdgeColor;
    hccm               = meta.LPub.studStyle.value() ? &meta.LPub.highContrast : &meta.LPub.multiStep.csi.highContrast;
  }

  // Camera Angles always passed to Native renderer except if ABS rotstep
  bool noCA            = !customViewpoint && meta.rotStep.value().type.toUpper() == QLatin1String("ABS");
  bool pp              = Preferences::perspectiveProjection;
  bool useImageSize    = meta.LPub.assem.imageSize.value(XX) > 0;

  // Renderer options
  NativeOptions *Options = lpub->currentStep->viewerOptions;
  const QString viewerStepKey = lpub->currentStep->viewerStepKey;
  if (Options) {
    Options->ViewerStepKey     = viewerStepKey;
    Options->CameraDistance    = camDistance > 0 ? camDistance : cameraDistance(meta,modelScale);
    Options->CameraName        = cameraName;
    Options->FoV               = cameraFoV;
    Options->HighlightNewParts = false; // gui->suppressColourMeta();
    Options->ImageHeight       = useImageSize ? int(meta.LPub.assem.imageSize.value(YY)) : LPub::pageSize(meta.LPub.page, YY);
    Options->ImageType         = Options::CSI;
    Options->ImageWidth        = useImageSize ? int(meta.LPub.assem.imageSize.value(XX)) : LPub::pageSize(meta.LPub.page, XX);
    Options->InputFileName     = ldrName;
    Options->IsOrtho           = isOrtho;
    Options->Viewpoint         = static_cast<int>(cameraView);
    Options->CustomViewpoint   = customViewpoint;
    Options->Latitude          = noCA ? 0.0f : cameraAngleX;
    Options->LineWidth         = lcGetPreferences().mLineWidth;
    Options->Longitude         = noCA ? 0.0f : cameraAngleY;
    Options->ModelScale        = modelScale;
    Options->OutputFileName    = pngName;
    Options->PageHeight        = LPub::pageSize(meta.LPub.page, YY);
    Options->PageWidth         = LPub::pageSize(meta.LPub.page, XX);
    Options->Position          = position;
    Options->Resolution        = resolution();
    Options->Target            = target;
    Options->UpVector          = upvector;
    Options->ZFar              = cameraZFar;
    Options->ZNear             = cameraZNear;
    Options->ZoomExtents       = false;
    Options->FadeParts         = fadeParts || Preferences::enableFadeSteps;
    Options->HighlightParts    = highlightParts || Preferences::enableHighlightStep;
    Options->LPubFadeHighlight = lpubFade || lpubHighlight;
    Options->AutoEdgeColor     = aecm->enable.value();
    Options->EdgeContrast      = aecm->contrast.value();
    Options->EdgeSaturation    = aecm->saturation.value();
    Options->StudStyle         = ssm->value();
    Options->LightDarkIndex    = hccm->lightDarkIndex.value();
    Options->StudCylinderColor = hccm->studCylinderColor.value();
    Options->PartEdgeColor     = hccm->partEdgeColor.value();
    Options->BlackEdgeColor    = hccm->blackEdgeColor.value();
    Options->DarkEdgeColor     = hccm->darkEdgeColor.value();
    Options->DDF               = meta.LPub.cameraDDF.value();

#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_DEBUG,QObject::tr("Render CSI using viewer step key '%1' for image '%2'.")
                                               .arg(viewerStepKey)
                                               .arg(QFileInfo(pngName).fileName()));
#endif
  } else {
    emit gui->messageSig(LOG_ERROR,QObject::tr("Failed to retrieve CSI render options %1 for image '%2'.")
                                               .arg(viewerStepKey.isEmpty() ? "and viewer step key" : QString("using viewer step key '%1'").arg(viewerStepKey))
                                               .arg(QFileInfo(pngName).fileName()));
    return -1;
  }

  // Update render image
  emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("Executing Native %1 CSI image render - please wait...")
                                                    .arg(pp ? "Perspective" : "Orthographic"));

  if (Gui::exportingObjects()) {
      if (csiKeys.size()) {
          emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("Rendering CSI Objects..."));
          QString baseName = csiKeys.first();
          QString outPath  = Gui::m_saveDirectoryName;
          int ldvExport    = DT_LDV_FUNCTION;

          switch (Options->ExportMode) {
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
              ldvExport = DT_DEFAULT;
              break;
          case EXPORT_WAVEFRONT:
              Options->ExportMode = int(EXPORT_WAVEFRONT);
              Options->ExportFileName = QDir::toNativeSeparators(outPath+"/"+baseName+".obj");
              ldvExport = DT_DEFAULT;
              break;
          default:
              emit gui->messageSig(LOG_ERROR,QObject::tr("Invalid CSI Object export option."));
              return -1;
          }
          // These exports are performed by the Native LDV module (LDView).
          if (ldvExport == DT_LDV_FUNCTION) {
              if (Gui::m_exportMode == EXPORT_POVRAY     ||
                  Gui::m_exportMode == EXPORT_STL        ||
                  Gui::m_exportMode == EXPORT_HTML_PARTS ||
                  Gui::m_exportMode == EXPORT_3DS_MAX) {
                  Options->IniFlag = Gui::m_exportMode == EXPORT_POVRAY ? NativePOVIni :
                                     Gui::m_exportMode == EXPORT_STL ? NativeSTLIni : Native3DSIni;
                  /*  Options->IniFlag = Gui::m_exportMode == EXPORT_POVRAY ? NativePOVIni :
                                         Gui::m_exportMode == EXPORT_STL ? NativeSTLIni : EXPORT_HTML_PARTS; */
              }

              ldrName = QDir::currentPath() + "/" + Paths::tmpDir + "/exportcsi.ldr";

              FloatPairMeta cameraAngles;
              cameraAngles.setValues(meta.LPub.assem.cameraAngles.value(0),
                                     meta.LPub.assem.cameraAngles.value(1));

              // RotateParts #2 - 8 parms, rotate parts for ldvExport - apply camera angles
              int rc;
              if ((rc = rotateParts(addLine, meta.rotStep, csiParts, ldrName, QString(),cameraAngles,ldvExport,Options::CSI)) < 0) {
                  return rc;
              }

              /* determine camera distance */
              int cd = int(meta.LPub.assem.cameraDistance.value());
              if (!cd)
                  cd = int(cameraDistance(meta,modelScale)*1700/1000);

              /* apply camera angles */
              noCA  = Preferences::applyCALocally || noCA;

              QString df = QString("-FOV=%1").arg(double(cameraFoV));
              QString CA = pp ? df : QString("-ca%1") .arg(LP3D_CA);              // replace CA with FOV

              qreal cdf = LP3D_CDF;
              QString cg = QString("-cg%1,%2,%3")
                                   .arg(noCA ? double(0.0f) : double(cameraAngleX))
                                   .arg(noCA ? double(0.0f) : double(cameraAngleY))
                                   .arg(pp ? QString::number(cd * cdf,'f',0) : QString::number(cd) );

              QString ss,ae,ac,ai,hs,hp,pb,hd;
              getStudStyleAndAutoEdgeSettings(ssm, hccm, aecm, ss, ae, ac, ai, hs, hp, pb, hd);
              QString w  = QString("-SaveWidth=%1") .arg(double(Options->ImageWidth));
              QString h  = QString("-SaveHeight=%1") .arg(double(Options->ImageHeight));
              QString o  = QString("-HaveStdOut=1");
              QString v  = QString("-vv");

              QStringList arguments;
              arguments << CA;
              arguments << cg;
              arguments << ss; // stud style
              arguments << ae; // automate edge color
              arguments << ac; // part edge contrast
              arguments << ai; // part color value light/dark index (saturation)
              arguments << hs; // stud cylinder color
              arguments << hp; // part edge color
              arguments << pb; // black edge color
              arguments << hd; // dark edge color
              arguments << w;
              arguments << h;

              if (!Preferences::altLDConfigPath.isEmpty())
                  arguments << QString("-LDConfig=\"%1\"").arg(QDir::toNativeSeparators(Preferences::altLDConfigPath));
              arguments << QString("-ExportFile=\"%1\"") .arg(QDir::toNativeSeparators(Options->ExportFileName));
              arguments << QString("-LDrawDir\"%1\"") .arg(QDir::toNativeSeparators(Preferences::ldrawLibPath));

              arguments << QString("\"%1\"").arg(QDir::toNativeSeparators(ldrName));

              arguments << o;
              arguments << v;

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
  float camDistance    = metaType.cameraDistance.value();
  float modelScale     = metaType.modelScale.value();
  float cameraAngleX   = metaType.cameraAngles.value(XX);
  float cameraAngleY   = metaType.cameraAngles.value(YY);
  float cameraFoV      = metaType.cameraFoV.value();
  float cameraZNear    = metaType.cameraZNear.value();
  float cameraZFar     = metaType.cameraZFar.value();
  bool  isOrtho        = metaType.isOrtho.value();
  bool  customViewpoint= metaType.cameraAngles.customViewpoint();
  QString cameraName   = metaType.cameraName.value();
  Vector3 position     = Vector3(metaType.position.x(),metaType.position.y(),metaType.position.z());
  Vector3 target       = Vector3(metaType.target.x(),metaType.target.y(),metaType.target.z());
  Vector3 upvector     = Vector3(metaType.upvector.x(),metaType.upvector.y(),metaType.upvector.z());
  StudStyleMeta* ssm = meta.LPub.studStyle.value() ? &meta.LPub.studStyle : &metaType.studStyle;
  AutoEdgeColorMeta* aecm = meta.LPub.autoEdgeColor.enable.value() ? &meta.LPub.autoEdgeColor : &metaType.autoEdgeColor;
  HighContrastColorMeta* hccm = meta.LPub.studStyle.value() ? &meta.LPub.highContrast : &metaType.highContrast;
  CameraViews::CameraView cameraView = metaType.cameraAngles.cameraView();
  QString nameKey;

  bool useImageSize    = metaType.imageSize.value(XX) > 0;

  // Camera Angles always passed to Native renderer except if ABS rotstep
  bool noCA            = !customViewpoint && metaType.rotStep.value().type.toUpper() == QLatin1String("ABS");
  bool pp              = Preferences::perspectiveProjection;
  QString cleanPngName = pngName;
  QStringList attributes;

  if (keySub || pliType == BOM) {
      cleanPngName.replace(";", "_");
      attributes = getImageAttributes(pngName);
      if (attributes.size() >= nTypeNameKey)
          nameKey = QString("%1_%2").arg(attributes.at(nType)).arg(attributes.at(nColorCode));
  }

  // Process substitute part attributes
  if (keySub) {
    bool hr;
    if ((hr = attributes.size() == nHasRotstep) || attributes.size() == nHasTargetAndRotstep)
      noCA = attributes.at(hr ? nRotTrans : nRot_Trans).toUpper() == QLatin1String("ABS");
    if (attributes.size() >= nHasTarget)
      target = Vector3(attributes.at(nTargetX).toFloat(),attributes.at(nTargetY).toFloat(),attributes.at(nTargetZ).toFloat());
    if (keySub > PliBeginSub2Rc) {
      modelScale   = attributes.at(nModelScale).toFloat();
      cameraFoV    = attributes.at(nCameraFoV).toFloat();
      cameraAngleX = attributes.at(nCameraAngleXX).toFloat();
      cameraAngleY = attributes.at(nCameraAngleYY).toFloat();
    }
  }

  // Update renderer options
  NativeOptions *Options = nullptr;
  QString viewerStepKey;
  switch(pliType) {
  case SUBMODEL:
      Options  = lpub->currentStep->subModel.viewerOptions;
      viewerStepKey = lpub->currentStep->subModel.viewerSubmodelKey;
      break;
  case PART:
      Options  = lpub->currentStep->pli.viewerOptions;
      viewerStepKey = lpub->currentStep->pli.viewerPliPartKey;
      break;
  case BOM:
      if (!nameKey.isEmpty()) {
        Options = lpub->page.pli.viewerOptsList[nameKey]; 
        viewerStepKey = QString("%1;%2;0").arg(attributes.at(nType)).arg(attributes.at(nColorCode));
      }
      break;
  }

  if (Options) {
    Options->ViewerStepKey  = viewerStepKey;
    Options->CameraDistance = camDistance > 0 ? camDistance : cameraDistance(meta,modelScale);
    Options->CameraName     = cameraName;
    Options->FoV            = cameraFoV;
    Options->ImageHeight    = useImageSize ? int(metaType.imageSize.value(YY)) : LPub::pageSize(meta.LPub.page, YY);
    Options->ImageType      = pliType == SUBMODEL ? Options::SMI : Options::PLI;
    Options->ImageWidth     = useImageSize ? int(metaType.imageSize.value(XX)) : LPub::pageSize(meta.LPub.page, XX);
    Options->InputFileName  = ldrNames.first();
    Options->IsOrtho        = isOrtho;
    Options->Viewpoint      = static_cast<int>(cameraView);
    Options->CustomViewpoint= customViewpoint;
    Options->Latitude       = noCA ? 0.0f : cameraAngleX;
    Options->Longitude      = noCA ? 0.0f : cameraAngleY;
    Options->LineWidth      = lcGetPreferences().mLineWidth;
    Options->ModelScale     = modelScale;
    Options->OutputFileName = cleanPngName;
    Options->PageHeight     = LPub::pageSize(meta.LPub.page, YY);
    Options->PageWidth      = LPub::pageSize(meta.LPub.page, XX);
    Options->Position       = position;
    Options->Resolution     = resolution();
    Options->Target         = target;
    Options->UpVector       = upvector;
    Options->ZFar           = cameraZFar;
    Options->ZNear          = cameraZNear;
    Options->ZoomExtents    = false;
    Options->AutoEdgeColor  = aecm->enable.value();
    Options->EdgeContrast   = aecm->contrast.value();
    Options->EdgeSaturation = aecm->saturation.value();
    Options->StudStyle      = ssm->value();
    Options->LightDarkIndex = hccm->lightDarkIndex.value();
    Options->StudCylinderColor = hccm->studCylinderColor.value();
    Options->PartEdgeColor  = hccm->partEdgeColor.value();
    Options->BlackEdgeColor = hccm->blackEdgeColor.value();
    Options->DarkEdgeColor  = hccm->darkEdgeColor.value();
    Options->DDF            = meta.LPub.cameraDDF.value();

#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_DEBUG,QObject::tr("Render %1 using viewer step key '%2' for image '%3'.")
                                               .arg(pliType == SUBMODEL ? "SMI" : pliType == BOM ? "BOM" : "PLI")
                                               .arg(viewerStepKey)
                                               .arg(QFileInfo(cleanPngName).fileName()));
#endif
  } else {
    emit gui->messageSig(LOG_ERROR,QObject::tr("Failed to retrieve %1 render options %2 for image '%3'.")
                                               .arg(pliType == SUBMODEL ? "SMI" : pliType == BOM ? "BOM" : "PLI")
                                               .arg(viewerStepKey.isEmpty() ? "and viewer step key" : QString("using viewer step key '%1'").arg(viewerStepKey))
                                               .arg(QFileInfo(cleanPngName).fileName()));
    return -1;
  }

  // Render image
  emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("Executing Native %1 %2 image render - please wait...")
                                                    .arg(pp ? "Perspective" : "Orthographic")
                                                    .arg(pliType == SUBMODEL ? "SMI" : pliType == BOM ? "BOM" : "PLI"));

  if (!RenderNativeImage(Options)) {
      return -1;
  }

  return 0;
}

float Render::ViewerCameraDistance(
  Meta &meta,
  float scale)
{
    float distance = stdCameraDistance(meta,scale);
    if (Preferences::preferredRenderer == RENDERER_POVRAY)      // 1
        distance = (distance*0.455f)*1700.0f/1000.0f;
    else if (Preferences::preferredRenderer == RENDERER_LDVIEW) // 2
        distance = (distance*0.775f)*1700.0f/1000.0f;
    return distance;
}

bool Render::RenderNativeView(const NativeOptions *O, bool RenderImage/*false*/)
{
    lcModel* ActiveModel = lcGetActiveProject()->GetMainModel();

    if (!ActiveModel)
    {
        emit gui->messageSig(LOG_ERROR,QObject::tr("Failed to retrieve active model."));
        return false;
    }

    lcView* ActiveView = nullptr;

    if (!RenderImage)
    {
        ActiveView = gui->GetActiveView();
        if (!ActiveView)
        {
            emit gui->messageSig(LOG_ERROR,QObject::tr("Failed to set active view."));
            return false;
        }
        gui->enableApplyLightAction();
        gui->GetPartSelectionWidget()->SetDefaultPart();
    }

    lcGetActiveProject()->SetRenderAttributes(
                getDistanceRendererIndex(),
                O->ImageType,
                O->ImageWidth,
                O->ImageHeight,
                O->PageWidth,
                O->PageHeight,
                O->ImageFileName,
                O->Resolution);

    lcPreferences& Preferences = lcGetPreferences();

    lcCamera* Camera      = nullptr;

    lcStep Step           = 1;
    lcViewpoint Viewpoint = lcViewpoint::Count;
    int ViewPointCompare  = static_cast<int>(Viewpoint);
    bool HasCameraView    = O->Viewpoint < ViewPointCompare;

    bool ZoomExtents      = O->ZoomExtents;
    bool DefaultCamera    = O->CameraName.isEmpty();
    bool IsOrtho          = DefaultCamera ? Preferences.mNativeProjection : O->IsOrtho;
    bool UsingViewpoint   = Preferences.mNativeViewpoint < ViewPointCompare || HasCameraView;
    bool UsingTransform   = O->Position.isPopulated() && O->UpVector.isPopulated();
    bool UsingTarget      = O->Target.isPopulated();
    bool IsNativeRenderer = Preferences::preferredRenderer == RENDERER_NATIVE;
    const float DefaultFoV = Preferences::preferredRenderer == RENDERER_LDVIEW &&
                             Preferences::perspectiveProjection ?
                             CAMERA_FOV_LDVIEW_P_DEFAULT :
                             CAMERA_FOV_DEFAULT;
    float SavedDDF        = 0;
    float CameraFoV       = 0, CameraZFar = 0, CameraZNear = 0;

    if (IsNativeRenderer && notEqual(lcGetProfileFloat(LC_PROFILE_DEFAULT_DISTANCE_FACTOR),O->DDF,1))
    {
        SavedDDF = Preferences.mDDF;
        gApplication->mPreferences.mDDF = O->DDF;
    }

    if (DefaultCamera && ActiveView/*!RenderImage*/)
     {
         Camera = ActiveView->GetCamera();

         if (notEqual(Preferences.mCNear,O->ZNear,1))
             Camera->m_zNear = O->ZNear;

         if (notEqual(Preferences.mCFar,O->ZFar,1))
             Camera->m_zNear = O->ZFar;
     }
     else // Custom Camera or RenderImage
     {
         Camera = new lcCamera(false);

         if (notEqual(Camera->m_zNear, O->ZNear,1))
             Camera->m_zNear = O->ZNear;

         if (notEqual(Camera->m_zFar, O->ZFar,1))
             Camera->m_zFar = O->ZFar;
     }

    if (!UsingViewpoint || notEqual(Preferences.mCFoV,O->FoV))
    {
        if (IsNativeRenderer)
            Camera->m_fovy = O->FoV;
        else
            Camera->m_fovy = O->FoV + Camera->m_fovy - DefaultFoV;
    }

    if (UsingViewpoint)
    { // Using ViewPoints (Front, Back, Top, Bottom, Left, Right, Home, LatLon)

        if (HasCameraView)
            Viewpoint = static_cast<lcViewpoint>(O->Viewpoint);
        else
            Viewpoint = static_cast<lcViewpoint>(Preferences.mNativeViewpoint);

        bool ViewpointLatLon = Viewpoint == lcViewpoint::LatLon;

        ViewpointLatLon |= Viewpoint == lcViewpoint::Home && O->CustomViewpoint;

        if (!RenderImage)
        {
            if (ViewpointLatLon)
                ActiveView->SetViewpointLatLon(O->Latitude, O->Longitude, O->CameraDistance);
            else
                ActiveView->SetViewpoint(Viewpoint);
        }
        else
        {
            if (ViewpointLatLon)
                Camera->SetAngles(O->Latitude, O->Longitude, O->CameraDistance);
            else
                Camera->SetViewpoint(Viewpoint);
        }
    } // Using ViewPoints
    else
    { // Using Projection (Angles + Distance + Perspective|Orthographic)

        // Not sure why/if we need this adjustment?
        if (RenderImage)
            Camera->m_fovy += Preferences::nativeImageCameraFoVAdjust; // 15

        // Camera Target, Position and UpVector: Switch Y and Z axis with -Y(LC -Z) in the up direction (Reset)
        if (UsingTarget)
            Camera->SetTargetPosition(lcVector3LDrawToLeoCAD(lcVector3(O->Target.x, O->Target.y, O->Target.z)),Step,false);

        if (UsingTransform) {
            Camera->SetPosition(lcVector3LDrawToLeoCAD(lcVector3(O->Position.x, O->Position.y, O->Position.z)),Step,false);
            Camera->SetUpVector(lcVector3LDrawToLeoCAD(lcVector3(O->UpVector.x, O->UpVector.y, O->UpVector.z)),Step,false);

            Camera->UpdatePosition(Step);
        }
        else
        {
            Camera->SetAngles(O->Latitude, O->Longitude, O->CameraDistance);
        }

        if (!RenderImage)
        {
            if (!ZoomExtents)
            {
                int PreferedZoomExtents = Preferences.mZoomExtents;
                if (PreferedZoomExtents)
                    ZoomExtents = (PreferedZoomExtents == 1/*On Ortho*/ && IsOrtho) ||
                                   PreferedZoomExtents == 2/*Always*/;
            }

            if (ZoomExtents)
                ActiveView->ZoomExtents();

            ActiveView->SetProjection(IsOrtho);
        }
    } // Projection

    if (!DefaultCamera)
    {
        for (size_t CameraIndex = 0; CameraIndex < ActiveModel->GetCameras().size(); )
        {
            QString const Name = ActiveModel->GetCameras()[CameraIndex]->GetName();
            if (Name == O->CameraName)
            {
                ActiveModel->RemoveCameraIndex(CameraIndex);
                QStringList const keys = O->ViewerStepKey.split(";");
                emit gui->messageSig(LOG_NOTICE, QObject::tr("Existing camera %1%2 was removed.")
                                     .arg(O->CameraName)
                                     .arg(keys.size() > 2 ? QObject::tr(" in step %1").arg(keys.at(2)) : ""));
                break;
            }
            else
                CameraIndex++;
        }

        Camera->SetName(O->CameraName);

        CameraFoV = Camera->m_fovy;
        CameraZFar = Camera->m_zFar;
        CameraZNear = Camera->m_zNear;

        ActiveModel->AddCamera(Camera);

        Camera = nullptr;

        if (!RenderImage)
            ActiveView->SetCamera(O->CameraName);

    } // Custom Camera

    bool rc = true;

    bool UseImageSize = false;
    int  ImageWidth   = 0;
    int  ImageHeight  = 0;
    QString ImageType = O->ImageType == Options::CSI ? "CSI" :
                        O->ImageType == Options::PLI ? "PLI" :
                        O->ImageType == Options::SMI ? "SMI" : "MON";

    // Generate Image
    if (RenderImage)
    {
        ZoomExtents  = false;
        UseImageSize = O->ImageWidth != O->PageWidth || O->ImageHeight != O->PageHeight;
        ImageWidth   = int(O->PageWidth);
        ImageHeight  = int(UseImageSize ? O->PageHeight / 2 : O->PageHeight);

        const lcStep CurrentStep = ActiveModel->GetCurrentStep();
        const lcStep ImageStep   = ActiveModel->GetLastStep();

        std::unique_ptr<lcView> ImageView = std::unique_ptr<lcView>(new lcView(lcViewType::View, ActiveModel));

        if (DefaultCamera)
            ImageView->SetCamera(Camera, true);
        else
            ImageView->SetCamera(O->CameraName);
        ImageView->SetProjection(IsOrtho);
        ImageView->SetOffscreenContext();
        ImageView->MakeCurrent();

        if ((rc = ImageView->BeginRenderToImage(ImageWidth, ImageHeight)))
        {
            struct NativeImage
            {
                QImage RenderedImage;
                QRect Bounds;
            };
            NativeImage Image;

            ActiveModel->SetTemporaryStep(ImageStep);

            ImageView->OnDraw();

            Image.RenderedImage = ImageView->GetRenderImage();

            ImageView->EndRenderToImage();

            ActiveModel->SetTemporaryStep(CurrentStep);

            if (!ActiveModel->IsActive())
                ActiveModel->CalculateStep(LC_STEP_MAX);

            auto CalculateImageBounds = [&O, &UseImageSize](NativeImage& Image)
            {
                QImage& RenderedImage = Image.RenderedImage;
                int Width  = RenderedImage.width();
                int Height = RenderedImage.height();

                if (UseImageSize)
                {
                    int AdjX = (Width - O->ImageWidth) / 2;
                    int AdjY = (Height - O->ImageHeight) / 2;
                    Image.Bounds = QRect(QPoint(AdjX, AdjY), QPoint(QPoint(Width, Height) - QPoint(AdjX, AdjY)));
                }
                else
                {
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
                }
            };

            CalculateImageBounds(Image);

            QImageWriter Writer(O->OutputFileName);

            if (Writer.format().isEmpty())
                Writer.setFormat("PNG");

            if (!Writer.write(QImage(Image.RenderedImage.copy(Image.Bounds))))
            {
                QString const message = QObject::tr("Could not write to Native %1 %2 file:<br>[%3].<br>Reason: %4.")
                                            .arg(ImageType)
                                            .arg((O->ExportMode == EXPORT_PDF ||
                                                  O->ExportMode == EXPORT_PNG ||
                                                  O->ExportMode == EXPORT_JPG ||
                                                  O->ExportMode == EXPORT_BMP)
                                                     ? QObject::tr("image")
                                                     : QObject::tr("%1 object")
                                                         .arg(nativeExportNames[O->ExportMode]))
                                            .arg(O->OutputFileName)
                                            .arg(Writer.errorString());
                emit gui->messageSig(LOG_ERROR,message);
                rc = false;
            }
            else
            {
                emit gui->messageSig(LOG_INFO,QObject::tr("Native %1 image file rendered '%2'")
                                     .arg(ImageType).arg(O->OutputFileName));
            }

            lcGetActiveProject()->SetImageSize(Image.Bounds.width(), Image.Bounds.height());

        }
        else
        {
            emit gui->messageSig(LOG_ERROR,QObject::tr("Begin Native %1 image render returned code %2 - "
                                                       "Unable to bind render framebuffer.").arg(ImageType).arg(rc));
        }

        bool DoNativeExport = O->ExportMode != EXPORT_NONE &&
                              O->ExportMode != EXPORT_PDF  &&
                              O->ExportMode != EXPORT_PNG  &&
                              O->ExportMode != EXPORT_JPG  &&
                              O->ExportMode != EXPORT_BMP;
        if (DoNativeExport)
        {
            if (!NativeExport(O))
            {
                emit gui->messageSig(LOG_ERROR,QObject::tr("%1 Objects render failed.").arg(ImageType));
                rc = false;
            }
        }
    }

    if (Preferences::debugLogging)
    {
        const QString studStyleNames[] =
        {
            QLatin1String("0 Plain"),
            QLatin1String("1 Thin Line Logo"),
            QLatin1String("2 Outline Logo"),
            QLatin1String("3 Sharp Top Logo"),
            QLatin1String("4 Rounded Top Logo"),
            QLatin1String("5 Flattened Logo"),
            QLatin1String("6 High Contrast"),
            QLatin1String("7 High Contrast with Logo")
        };
        lcPreferences& Preferences = lcGetPreferences();
        QStringList arguments;
        if (RenderImage)
        {
            arguments << (O->InputFileName.isEmpty()   ? QString() : QString("InputFileName: %1,").arg(O->InputFileName));
            arguments << (O->OutputFileName.isEmpty()  ? QString() : QString("OutputFileName: %1,").arg(O->OutputFileName));
            arguments << (O->ExportFileName.isEmpty()  ? QString() : QString("ExportFileName: %1,").arg(O->ExportFileName));
            arguments << (O->ImageFileName.isEmpty()   ? QString() : QString("ImageFileName: %1,").arg(O->ImageFileName));
            arguments << (O->IniFlag == -1             ? QString() : QString("IniFlag: %1,").arg(iniFlagNames[O->IniFlag]));
            arguments << (O->ExportMode == EXPORT_NONE ? QString() : QString("ExportMode: %1,").arg(nativeExportNames[O->ExportMode]));
            arguments << (O->ExportArgs.size() == 0    ? QString() : QString("ExportArgs: %1,").arg(O->ExportArgs.join(" ")));
            arguments << QString("TransBackground: %1,").arg(O->TransBackground ? "True" : "False");
            arguments << QString("HighlightNewParts: %1,").arg(O->HighlightNewParts ? "True" : "False");
            arguments << QString("UseImageSize: %1,").arg(UseImageSize ? "True" : "False");
        }
        else
        {
            arguments << QString("ViewerStepKey: %1,").arg(O->ViewerStepKey);
            if (O->RotStep.isPopulated())
                arguments << QString("RotStep: X(%1) Y(%2) Z(%3) %4,").arg(double(O->RotStep.x)).arg(double(O->RotStep.y)).arg(double(O->RotStep.z)).arg(O->RotStepType);
        }
        arguments << QString("LineWidth: %1,").arg(double(O->LineWidth));
        arguments << QString("Resolution: %1,").arg(double(O->Resolution));
        arguments << QString("ImageWidth: %1,").arg(RenderImage ? ImageWidth : O->ImageWidth);
        arguments << QString("ImageHeight: %1,").arg(RenderImage ? ImageHeight : O->ImageHeight);
        arguments << QString("PageWidth: %1,").arg(O->PageWidth);
        arguments << QString("PageHeight: %1,").arg(O->PageHeight);
        if (RenderImage)
        {
            arguments << QString("PreferencesFoV: %1,").arg(double(Preferences.mCFoV));
            arguments << QString("OptionsFoV: %1,").arg(double(O->FoV));
        }
        arguments << QString("CameraFoVy: %1,").arg(double(DefaultCamera ? Camera->m_fovy : CameraFoV));
        arguments << QString("CameraZNear: %1,").arg(double(DefaultCamera ? Camera->m_zNear : CameraZNear));
        arguments << QString("CameraZFar: %1,").arg(double(DefaultCamera ? Camera->m_zFar : CameraZFar));
        arguments << QString("CameraDDF: %1,").arg(Preferences.mDDF);
        arguments << QString("CameraDistance (Scale: %1): %2,").arg(double(O->ModelScale)).arg(double(O->CameraDistance),0,'f',0);
        arguments << QString("CameraName: %1,").arg(DefaultCamera ? "Default" : O->CameraName);
        arguments << QString("CameraProjection: %1,").arg(IsOrtho ? "Orthographic" : "Perspective");
        arguments << QString("UsingViewpoint: %1,").arg(UsingViewpoint ? "True" : "False");
        if (UsingViewpoint)
        {
            arguments << QString("CameraViewPoint: %1,").arg(CameraViews::cameraViewNames[O->Viewpoint]);
            arguments << QString("CustomViewpoint: %1,").arg(O->CustomViewpoint ? "True" : "False");
        }
        arguments << QString("CameraLatitude: %1,").arg(double(O->Latitude));
        arguments << QString("CameraLongitude: %1,").arg(double(O->Longitude));
        arguments << QString("ZoomExtents: %1,").arg(ZoomExtents ? "True" : "False");
        arguments << QString("CameraTarget: X(%1) Y(%2) Z(%3),").arg(double(O->Target.x)).arg(double(O->Target.y)).arg(double(O->Target.z));
        if (O->Position.isPopulated())
            arguments << QString("CameraPosition: X(%1) Y(%2) Z(%3),").arg(double(O->Position.x)).arg(double(O->Position.y)).arg(double(O->Position.z));
        if (O->UpVector.isPopulated())
            arguments << QString("CameraUpVector: X(%1) Y(%2) Z(%3),").arg(double(O->UpVector.x)).arg(double(O->UpVector.y)).arg(double(O->UpVector.z));
        arguments << QString("LPubFadeHighlight: %1,").arg(O->LPubFadeHighlight ? "True" : "False");
        arguments << QString("FadeParts: %1,").arg(O->FadeParts ? "True" : "False");
        arguments << QString("HighlightParts: %1,").arg(O->HighlightParts ? "True" : "False");
        if (O->AutoEdgeColor || O->AutoEdgeColor != lpub->GetAutomateEdgeColor())
        {
            arguments << QString("AutomateEdgeColor: True,");
            if (O->EdgeContrast != Preferences.mPartEdgeContrast)
                arguments << QString("EdgeContrast: %1,").arg(O->EdgeContrast);
            if (O->EdgeSaturation != Preferences.mPartColorValueLDIndex)
                arguments << QString("Saturation: %1,").arg(O->EdgeSaturation);
        }
        bool IsHighContrastStudStyle = O->StudStyle >= static_cast<int>(lcStudStyle::HighContrast);
        bool StudStyleChanged = (O->StudStyle != lpub->GetStudStyle() ||
                                (IsHighContrastStudStyle && O->StudCylinderColorEnabled != lpub->GetStudCylinderColorEnabled()));
        if (O->StudStyle || StudStyleChanged)
            arguments << QString("StudStyle: %1,").arg(studStyleNames[O->StudStyle]);
        if (IsHighContrastStudStyle)
        {
            if (O->LightDarkIndex != Preferences.mPartColorValueLDIndex)
                arguments << QString("LightDarkIndex: %1,").arg(O->LightDarkIndex);
            if (!O->StudCylinderColorEnabled)
                arguments << QString("StudCylinderColorEnabled: False,");
            if (O->StudCylinderColor != Preferences.mStudCylinderColor)
                arguments << QString("StudCylinderColor: %1,%2,%3,%4,").arg(LC_RGBA_RED(O->StudCylinderColor)).arg(LC_RGBA_GREEN(O->StudCylinderColor)).arg(LC_RGBA_BLUE(O->StudCylinderColor)).arg(LC_RGBA_ALPHA(O->StudCylinderColor));
            if (!O->PartEdgeColorEnabled)
                arguments << QString("PartEdgeColorEnabled: False,");
            if (O->PartEdgeColor != Preferences.mPartEdgeColor)
                arguments << QString("PartEdgeColor: %1,%2,%3,%4,").arg(LC_RGBA_RED(O->PartEdgeColor)).arg(LC_RGBA_GREEN(O->PartEdgeColor)).arg(LC_RGBA_BLUE(O->PartEdgeColor)).arg(LC_RGBA_ALPHA(O->PartEdgeColor));
            if (!O->BlackEdgeColorEnabled)
                arguments << QString("BlackEdgeColorEnabled: False,");
            if (O->BlackEdgeColor != Preferences.mBlackEdgeColor)
                arguments << QString("BlackEdgeColor: %1,%2,%3,%4,").arg(LC_RGBA_RED(O->BlackEdgeColor)).arg(LC_RGBA_GREEN(O->BlackEdgeColor)).arg(LC_RGBA_BLUE(O->BlackEdgeColor)).arg(LC_RGBA_ALPHA(O->BlackEdgeColor));
            if (!O->DarkEdgeColorEnabled)
                arguments << QString("DarkEdgeColorEnabled: False,");
            if (O->DarkEdgeColor != Preferences.mDarkEdgeColor)
                arguments << QString("DarkEdgeColor: %1,%2,%3,%4,").arg(LC_RGBA_RED(O->DarkEdgeColor)).arg(LC_RGBA_GREEN(O->DarkEdgeColor)).arg(LC_RGBA_BLUE(O->DarkEdgeColor)).arg(LC_RGBA_ALPHA(O->DarkEdgeColor));
        }
        removeEmptyStrings(arguments);

        if (arguments.last().endsWith(","))
            arguments.last().chop(1);

        QString message = QObject::tr("%1 %2 Arguments: %3%4")
                                      .arg(RenderImage ? QObject::tr("Native Renderer") : QObject::tr("Visual Editor"))
                                      .arg(ImageType)
                                      .arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
      qDebug() << qUtf8Printable(message.arg(QLatin1String(".\r\n")));
#else
      emit gui->messageSig(LOG_INFO, message.arg(QLatin1String("\r\n")));
#endif
    }

    if (SavedDDF > 0)
        Preferences.mDDF = SavedDDF;

    if (!DefaultCamera)
        delete Camera;

    return rc;
}

bool Render::RenderNativeImage(const NativeOptions *Options)
{

    bool IsHighContrastStudStyle = Options->StudStyle >= static_cast<int>(lcStudStyle::HighContrast);
    bool StudStyleChanged = (Options->StudStyle != lpub->GetStudStyle() ||
                            (IsHighContrastStudStyle && Options->StudCylinderColorEnabled != lpub->GetStudCylinderColorEnabled()));
    if(StudStyleChanged)
        lpub->SetStudStyle(Options, true/*reload*/);

    if(Options->AutoEdgeColor != lpub->GetAutomateEdgeColor()) {
        if (Options->AutoEdgeColor && IsHighContrastStudStyle)
            emit gui->messageSig(LOG_NOTICE,QObject::tr("High contrast stud and edge color settings are ignored when automate edge color is enabled."));
        lpub->SetAutomateEdgeColor(Options);
    }

    bool Loaded = false;

    Loaded = lpub->OpenProject(Options, NATIVE_IMAGE, true/*UseFile*/);
    if (!Loaded) {
        emit gui->messageSig(LOG_ERROR, QObject::tr("Could not open Loader for ViewerStepKey: '%1', FileName: '%2', [Use File]")
                                                    .arg(Options->ViewerStepKey)
                                                    .arg(QFileInfo(Options->InputFileName).fileName()));
    }

    return Loaded;
}

bool Render::LoadViewer(const NativeOptions *Options) {

    if (!lpub->currentStep)
        return false;

    if (!Preferences::modeGUI)
        return true;

    bool Loaded = false;

    bool keyMatch = Options->ViewerStepKey == lpub->viewerStepKey;

    if (!keyMatch) {
        QString const currentStepKey = lpub->currentStep->viewerStepKey;
        emit gui->messageSig(LOG_ERROR, QObject::tr("The specified step key: '%1' does not match the current step key: '%2'")
                                                    .arg(Options->ViewerStepKey).arg(currentStepKey));
        if (Gui::abortProcess())
            return Loaded;
    }

    gui->clearBuildModRange();

    gui->clearVisualEditUndoRedoText();

    bool IsHighContrastStudStyle = Options->StudStyle >= static_cast<int>(lcStudStyle::HighContrast);
    bool StudStyleChanged = (Options->StudStyle != lpub->GetStudStyle() ||
                            (IsHighContrastStudStyle && Options->StudCylinderColorEnabled != lpub->GetStudCylinderColorEnabled()));
    if(StudStyleChanged)
        lpub->SetStudStyle(Options, true/*reload*/);

    if(Options->AutoEdgeColor != lpub->GetAutomateEdgeColor()) {
        if (Options->AutoEdgeColor && IsHighContrastStudStyle) {
            QString message = QObject::tr("High contrast stud and edge color settings are ignored when automate edge color is enabled.");
            if (Preferences::getShowMessagePreference(Preferences::ParseErrors)) {
                Where file(QFileInfo(lpub->ldrawFile.getViewerStepFilePath(Options->ViewerStepKey)).fileName());
                QString parseMessage = QString("%1<br>(file: %2)").arg(message).arg(file.modelName);
                Preferences::MsgID msgID(Preferences::AnnotationErrors,file.nameToString());
                Preferences::showMessage(msgID, parseMessage, "Model File", QObject::tr("parse model file warning"));
            } else {
                emit gui->messageSig(LOG_NOTICE,message);
            }
        }

        lpub->SetAutomateEdgeColor(Options);
    }
    Loaded = lpub->OpenProject(Options);
    if (!Loaded) {
        emit gui->messageSig(LOG_ERROR, QObject::tr("Could not open Loader for ViewerStepKey: '%1', FileName: '%2', [Use Key]")
                                                    .arg(Options->ViewerStepKey)
                                                    .arg(QFileInfo(Options->InputFileName).fileName()));
    }

    return Loaded;
}

bool Render::NativeExport(const NativeOptions *Options) {

    bool Exported = false;

    const QString mode = nativeExportNames[Options->ExportMode];

    lcHTMLExportOptions HTMLOptions;

    if (Options->ExportMode == EXPORT_HTML_STEPS ||
        Options->ExportMode == EXPORT_WAVEFRONT  ||
        Options->ExportMode == EXPORT_COLLADA    ||
        Options->ExportMode == EXPORT_CSV        ||
        Options->ExportMode == EXPORT_BRICKLINK/*||
        Options->ExportMode == EXPORT_3DS_MAX*/) {

        emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("Native %1 Export...").arg(mode));

        if (Options->ExportMode == EXPORT_HTML_STEPS) {

            HTMLOptions = lcHTMLExportOptions(lcGetActiveProject());

            HTMLOptions.PathName = Options->ExportFileName;

            if (Preferences::modeGUI) {
                lcQHTMLDialog Dialog(gui, &HTMLOptions);

                if (Dialog.exec() == QDialog::Accepted)
                    emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("%1 export configuration completed.").arg(mode));
                else {
                    emit gui->messageSig(LOG_NOTICE, QObject::tr("%1 export cancelled!").arg(mode));
                    return !Exported;
                }
            }
        }

        bool IsHighContrastStudStyle = Options->StudStyle >= static_cast<int>(lcStudStyle::HighContrast);
        bool StudStyleChanged = (Options->StudStyle != lpub->GetStudStyle() ||
                                (IsHighContrastStudStyle && Options->StudCylinderColorEnabled != lpub->GetStudCylinderColorEnabled()));
        if(StudStyleChanged)
            lpub->SetStudStyle(Options, true/*reload*/);

        if(Options->AutoEdgeColor != lpub->GetAutomateEdgeColor()) {
            if (Options->AutoEdgeColor && IsHighContrastStudStyle)
                emit gui->messageSig(LOG_NOTICE,QObject::tr("High contrast stud and edge color settings are ignored when automate edge color is enabled."));
            lpub->SetAutomateEdgeColor(Options);
        }

        Exported = lpub->OpenProject(Options, NATIVE_EXPORT, true/*UseFile*/);

        if (!Exported) {
            emit gui->messageSig(LOG_ERROR, QObject::tr("Could not open Loader for ViewerStepKey: '%1', Export: %2, FileName: '%3', [Use File]")
                                                        .arg(Options->ViewerStepKey)
                                                        .arg(mode)
                                                        .arg(QFileInfo(Options->InputFileName).fileName()));
            return Exported;
        }
    }
    else
    {
        Exported = doLDVCommand(Options->ExportArgs, Options->ExportMode);
        return Exported;
    }

    if (Options->ExportMode == EXPORT_CSV)
    {
        Exported = lcGetActiveProject()->ExportCSV(Options->ExportFileName);
        if (Exported) {
            emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("%1 export completed.").arg(mode).arg(mode));
        } else {
            emit gui->messageSig(LOG_ERROR, QObject::tr("%1 export failed!").arg(mode).arg(mode));
        }
    }
    else
    if (Options->ExportMode == EXPORT_BRICKLINK)
    {
        Exported = lcGetActiveProject()->ExportBrickLink();
        if (Exported) {
            emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("%1 export completed.").arg(mode));
        } else {
            emit gui->messageSig(LOG_ERROR, QObject::tr("%1 export failed!").arg(mode));
        }
    }
    else
    if (Options->ExportMode == EXPORT_WAVEFRONT)
    {
        Exported = lcGetActiveProject()->ExportWavefront(Options->ExportFileName);
        if (Exported) {
            emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("%1 export completed. File: '%2'")
                                                   .arg(mode)
                                                   .arg(Options->ExportFileName));
        } else {
            emit gui->messageSig(LOG_ERROR, QObject::tr("%1 export failed!"));
        }
    }
    else
    if (Options->ExportMode == EXPORT_COLLADA)
    {
        Exported = lcGetActiveProject()->ExportCOLLADA(Options->ExportFileName);
        if (Exported) {
           emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("%1 export completed. File: '%2'")
                                                  .arg(mode)
                                                  .arg(Options->ExportFileName));
        } else {
            emit gui->messageSig(LOG_ERROR, QObject::tr("%1 export failed!").arg(mode));
        }
    }
    else
    if (Options->ExportMode == EXPORT_HTML_STEPS)
    {
        HTMLOptions.SaveDefaults();

        Exported = lcGetActiveProject()->ExportHTML(HTMLOptions);

        if (Exported) {
            QString htmlIndex = QDir::fromNativeSeparators(
                        HTMLOptions.PathName + "/" +
                        QFileInfo(Options->InputFileName).baseName() +
                        "-index.html");
            if (Preferences::modeGUI) {
                gui->setExportedFile(htmlIndex);

                gui->showExportedFile();
            } else {
                emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("%1 export completed. Index: '%2'")
                                                                  .arg(mode)
                                                                  .arg(htmlIndex));
            }
        } else {
            emit gui->messageSig(LOG_ERROR, QObject::tr("%1 export failed!").arg(mode));
        }
    }
/*
    // These are executed through the LDV Native renderer
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

    return Exported;
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
    QStringList arguments = args;

    if (exportMode == EXPORT_NONE && iniFlag == NumIniFiles) {
        emit gui->messageSig(LOG_ERROR, QObject::tr("Invalid export mode and ini flag codes specified."));
        return false;
    }

    switch (exportMode){
    case EXPORT_HTML_PARTS:
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

    bool rc = true;
    bool exportHTML = exportMode == EXPORT_HTML_PARTS;
    const QString workingDirectory = QDir::currentPath();
    const QString mode = nativeExportNames[exportMode];
    emit gui->messageSig(LOG_TRACE, QObject::tr("Native %1 Export for command: %2")
                                                .arg(mode)
                                                .arg(arguments.join(" ")));

    ldvWidget = new LDVWidget(nullptr,IniFlag(iniFlag),true);

    if (exportHTML)
        gui->connect(ldvWidget, SIGNAL(loadBLCodesSig()), gui, SLOT(loadBLCodes()));

    if (! ldvWidget->doCommand(arguments))  {
        emit gui->messageSig(LOG_ERROR, QObject::tr("Failed to generate %1 Export for command: %2")
                                                    .arg(mode)
                                                    .arg(arguments.join(" ")));
        rc = false;
    }

    if (! QDir::setCurrent(workingDirectory)) {
        emit gui->messageSig(LOG_ERROR, QObject::tr("Failed to restore %1 export working directory: %2")
                                                    .arg(mode)
                                                    .arg(workingDirectory));
        rc = false;
    }

    if (exportHTML)
        gui->disconnect(ldvWidget, SIGNAL(loadBLCodesSig()), gui, SLOT(loadBLCodes()));

    return rc;
}

const QString Render::getPovrayRenderQuality(int quality)
{
    // POV-Ray 3.2 Command-Line and INI-File Options
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

const QString Render::getRenderImageFile(int renderType)
{
    QDir renderDir(QString("%1/%2").arg(QDir::currentPath())
                   .arg(renderType == POVRAY_RENDER ? Paths::povrayRenderDir : Paths::blenderRenderDir));
    if (!renderDir.exists()) {
        if (renderType == POVRAY_RENDER)
            Paths::mkPovrayDir();
        else
            Paths::mkBlenderDir();
    }

    QString fileName = gui->getViewerConfigKey(gui->getViewerStepKey()).replace(";","_");

    if (fileName.isEmpty()){
        fileName = renderType == POVRAY_RENDER ? QLatin1String("povray_image_render") : QLatin1String("blender_image_render");
        emit gui->messageSig(LOG_NOTICE, QObject::tr("Failed to receive model file name - using [%1]").arg(fileName));
    }

    QString imageFile = QDir::toNativeSeparators(QString("%1/%2.png")
                       .arg(renderDir.absolutePath())
                       .arg(fileName));

    return imageFile;

}

const QString Render::getRenderModelFile(int renderType, bool saveCurrentModel) {

    QString modelFile;
    QString filePath = QDir::currentPath() + QDir::separator() + Paths::tmpDir + QDir::separator();

    if (renderType == POVRAY_RENDER) {

        modelFile = QDir::toNativeSeparators(filePath + "csi_povray.ldr");

    } else if (renderType == BLENDER_RENDER) {

        modelFile = QDir::toNativeSeparators(filePath + "csi_blender.ldr");

        if (saveCurrentModel)
            gui->saveCurrent3DViewerModel(modelFile);
    }
    return modelFile;
}

bool Render::isSingleSubfile(const QStringList &partLines)
{
    int lineIndex = 0;
    bool singleSubfile = false;
    if (partLines.isEmpty())
        return false;
    else if (partLines.count() == 1) {
        singleSubfile = lpub->ldrawFile.isSingleSubfileLine(partLines.first());
    } else {
        int partCount = 0;
        for (int i = 0; i < partLines.count(); i++) {
            const QString &partLine = partLines[i];
            if (partCount > 1)
                return false;
            if (!partLine.isEmpty() && !partLine.startsWith("0 "))
                partCount++;
            if (partCount == 1 && partLine.startsWith("1 ")) {
                singleSubfile = lpub->ldrawFile.isSingleSubfileLine(partLine);
                lineIndex = i;
            }
        }
    }
    if (singleSubfile) {
        QStringList tokens;
        split(partLines[lineIndex], tokens);
        const QString type = tokens[tokens.size()-1].toLower();
        if (lpub->ldrawFile.topLevelFile() == type)
            if (lpub->ldrawFile.isMpd())
                return false;
        const QStringList &contents = lpub->ldrawFile.contents(type);
        for (const QString &line : contents) {
            if (line.startsWith("0 "))
                if (!isGhost(line) && !isSubstitute(line))
                    continue;
            if (lpub->ldrawFile.isSingleSubfileLine(line))
                return false;
        }
    }
    return singleSubfile;
}

bool Render::pruneNativeParts(QStringList &rotatedParts)
{
    //rotatedParts.takeFirst();
    bool singleSubfile = false;
    int mpdFileIndx = -1, mpdNoFileIndx = -1;
    for (int i = 0; i < rotatedParts.count(); i++) {
        if (!singleSubfile) {
            singleSubfile = rotatedParts[i].startsWith("0 FILE");
            mpdFileIndx = singleSubfile ? i : -1;
        }
        if (singleSubfile && rotatedParts[i].startsWith("0 FILE")) {
            singleSubfile = false;
            break;
        }
        if (singleSubfile) {
            if (rotatedParts[i].startsWith("0 NOFILE")) {
                mpdNoFileIndx = i;
                break;
            }
        }
    }
    if (singleSubfile) {
        rotatedParts.removeAt(mpdFileIndx);
        if (mpdNoFileIndx >= 0)
            rotatedParts.removeAt(mpdNoFileIndx);
    }
    return singleSubfile;
}

// create Native version of the CSI/PLI file - consolidate subfiles and parts into single file
int Render::createNativeModelFile(
    QStringList &rotatedParts,
    bool         doFadeStep,
    bool         doHighlightStep,
    int          type,
    bool         singleSubfile)
{
  Options::Mt imageType = static_cast<Options::Mt>(type);
  const QLatin1String mpdModelMeta("0 FILE ");
  bool mpdModel = rotatedParts.at(0).startsWith(mpdModelMeta);
  QStringList argv, nativeParts, nativeSubfiles, nativeSubfileParts;
  if (mpdModel || !singleSubfile)
      nativeParts = rotatedParts;

  int         rc;

  if (rotatedParts.size()) {
      /* Parse the rotated parts looking for subModels,
       * renaming fade and highlight step parts - so we can test - and
       * merging and formatting submodels by calling mergeNativeSubfiles and
       * returning all parts by reference
      */
      for (int index = 0; index < rotatedParts.size(); index++) {

          QString nativeLine = rotatedParts[index];
          split(nativeLine, argv);
          if (argv.size() == 15 && argv[0] == "1") {

              /* process subfiles in nativeRotatedParts */
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
                      isCustomSubModel = lpub->ldrawFile.isSubmodel(customType);
                      isCustomPart = lpub->ldrawFile.isUnofficialPart(customType);
                    }
                }

              if (doHighlightStep) {
                  QString highlightSfx = QString("%1.").arg(HIGHLIGHT_SFX);
                  bool isHighlightItem = type.contains(highlightSfx);
                  // Highlight file
                  if (isHighlightItem) {
                      customType = type;
                      customType = customType.replace(highlightSfx,".");
                      isCustomSubModel = lpub->ldrawFile.isSubmodel(customType);
                      isCustomPart = lpub->ldrawFile.isUnofficialPart(customType);
                    }
                }

              if (imageType == Options::MON) {
                  if (type.startsWith("mono_"))
                      isCustomSubModel = true;
              }

              if (lpub->ldrawFile.isSubmodel(type) || lpub->ldrawFile.isUnofficialPart(type) || isCustomSubModel || isCustomPart) {
                  /* capture subfiles (full string) to be processed when finished */
                  if (!nativeSubfiles.contains(type.toLower()))
                       nativeSubfiles << type.toLower();
                }
            }
        } //end for

      /* process extracted submodels and unofficial files */
      if (nativeSubfiles.size()) {
          nativeSubfiles.removeDuplicates();
          if ((rc = mergeNativeSubfiles(nativeSubfiles, nativeSubfileParts, doFadeStep, doHighlightStep,imageType)) != 0){
              emit gui->messageSig(LOG_ERROR,QObject::tr("Failed to process viewer submodels"));
              return rc;
            }
        }

      /* add sub model content to nativeRotatedParts file */
      if (! nativeSubfileParts.empty()) {
          bool _singleSubfile = false;
          QString rotStepLine;
          // check if main model and submodel shares the same name
          if (!singleSubfile)
              mpdModel = nativeParts.at(0).startsWith(mpdModelMeta);
          if (mpdModel && nativeParts.at(0) == nativeSubfileParts.at(0)) {
              int typeCount = 0;
              bool mpdModelNotTransformed = false;
              for (const QString &line : nativeParts) {
                  if (line[0] != '0') {
                      typeCount++;
                      if (typeCount == 1)
                          mpdModelNotTransformed = line.contains(QLatin1String(" 0 0 0 1 0 0 0 1 0 0 0 1 "));
                  }
                  else if (line.startsWith("0 // ROTSTEP "))
                      rotStepLine = line;
              }
              _singleSubfile = mpdModelNotTransformed && nativeSubfiles.size() == 1;
              if (_singleSubfile) {
                  nativeParts.clear();
                  nativeSubfiles.clear();
              }
          }

          for (int i = 0; i < nativeSubfileParts.size(); i++) {
              QString smLine = nativeSubfileParts[i];
              nativeParts << smLine;
          }

          if (singleSubfile && !rotStepLine.isEmpty())
              nativeParts.insert(3,rotStepLine);
      }

      /* remove scenario where main model and submodel share the same name*/
      auto tc = [] (const QString &s)
      {
          return QString(s).replace(s.indexOf(s.at(0)),1,s.at(0).toUpper());
      };
      if (mpdModel) {
          QFileInfo fi(nativeParts[0].split(mpdModelMeta).last());
          if (nativeSubfiles.contains(fi.fileName())) {
              QString baseName  = fi.completeBaseName();
              QString modelName = QString(fi.fileName()).replace(baseName, QString("%1-main").arg(baseName));
              nativeParts[0]    = QString("0 FILE %1").arg(modelName);
              nativeParts[1]    = QString("0 %1").arg(tc(baseName));
              nativeParts[2]    = QString("0 Name: %1").arg(modelName);
          }
      }

      /* prune FILE and NOFILE from single subfile  */
      if (singleSubfile)
          pruneNativeParts(nativeParts);

      /* return rotated parts by reference */
      rotatedParts = nativeParts;
    }

  return 0;
}

int Render::mergeNativeSubfiles(QStringList &subFiles,
                                QStringList &subFileParts,
                                bool doFadeStep,
                                bool doHighlightStep,
                                int imageType)
{
  QStringList nativeSubfiles     = subFiles;
  QStringList nativeSubfileParts = subFileParts;
  QStringList newSubfiles;
  QStringList argv;

  if (nativeSubfiles.size()) {

      /* read in all detected sub model file content */
      for (int index = 0; index < nativeSubfiles.size(); index++) {

          QString ldrName(QDir::currentPath() + "/" +
                          Paths::tmpDir + "/" +
                          nativeSubfiles[index]);

          /* initialize the working submodel file - define header. */
          QString modelName = QFileInfo(nativeSubfiles[index]).completeBaseName()/*.toLower()*/;
          modelName = modelName.replace(
                      modelName.indexOf(modelName.at(0)),1,modelName.at(0).toUpper());

          nativeSubfileParts << QString("0 FILE %1").arg(nativeSubfiles[index]);
          if (imageType != Options::MON) {
              nativeSubfileParts << QString("0 %1").arg(modelName);
              nativeSubfileParts << QString("0 Name: %1").arg(nativeSubfiles[index]);
          }

          /* read the actual submodel file */
          QFile ldrfile(ldrName);
          if ( ! ldrfile.open(QFile::ReadOnly | QFile::Text)) {
              emit gui->messageSig(LOG_ERROR,QString("Could not read submodel file %1: %2")
                                   .arg(ldrName)
                                   .arg(ldrfile.errorString()));
              return -1;
            }
          /* populate file contents into working submodel native parts */
          QTextStream in(&ldrfile);
          while ( ! in.atEnd()) {
              QString nativeLine = in.readLine(0);
              split(nativeLine, argv);

              if (argv.size() == 15 && argv[0] == "1") {
                  /* check and process any subfiles in nativeRotatedParts */
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
                          isCustomSubModel = lpub->ldrawFile.isSubmodel(customType);
                          isCustomPart = lpub->ldrawFile.isUnofficialPart(customType);
                        }
                    }

                  if (doHighlightStep) {
                      QString highlightSfx = QString("%1.").arg(HIGHLIGHT_SFX);
                      bool isHighlightItem = type.contains(highlightSfx);
                      // Highlight file
                      if (isHighlightItem) {
                          customType = type;
                          customType = customType.replace(highlightSfx,".");
                          isCustomSubModel = lpub->ldrawFile.isSubmodel(customType);
                          isCustomPart = lpub->ldrawFile.isUnofficialPart(customType);
                        }
                    }

                  if (imageType == Options::MON) {
                      if (type.startsWith("mono_"))
                          isCustomSubModel = true;
                  }

                  if (lpub->ldrawFile.isSubmodel(type) || lpub->ldrawFile.isUnofficialPart(type) || isCustomSubModel || isCustomPart) {
                      /* capture all subfiles (full string) to be processed when finished */
                      if (!newSubfiles.contains(type.toLower()))
                              newSubfiles << type.toLower();
                    }
                }
              if (isGhost(nativeLine))
                  argv.prepend(GHOST_META);
              nativeLine = argv.join(" ");
              nativeSubfileParts << nativeLine;
            }
          nativeSubfileParts << QLatin1String("0 NOFILE");
        }

      /* recurse and process any identified submodel files */
      if (newSubfiles.size() > 0){
          newSubfiles.removeDuplicates();
          int rc;
          if ((rc = mergeNativeSubfiles(newSubfiles, nativeSubfileParts, doFadeStep, doHighlightStep,imageType)) != 0){
              emit gui->messageSig(LOG_ERROR,QObject::tr("Failed to recurse viewer submodels"));
              return rc;
            }
        }
      subFileParts = nativeSubfileParts;
    }
  return 0;
}

int Render::mergeSubmodelContent(QStringList &submodelParts)
{
    auto writeContent = [] (const QStringList &content, QStringList &submodels) {
        if (content.size()) {
            for (int i = 0; i < content.size(); i++) {
                QString line = content[i];
                QStringList tokens;
                split(line,tokens);
                if (tokens.size() == 15) {
                    if (lpub->ldrawFile.isSubmodel(tokens[14])) {
                        submodels << tokens[14];
                    }
                }
            }
            return 0;
        }
        return 1;
    };

    QStringList submodels, parsedModels;
    if (writeContent(submodelParts, submodels) != 0)
        return 1;

    while (!submodels.isEmpty()) {
        const QString submodel = submodels.takeFirst();
        if (! parsedModels.contains(submodel)) {
            const QStringList &content = lpub->ldrawFile.smiContents(submodel);
            if (content.size()) {
                QString modelName = QFileInfo(submodel).completeBaseName();
                modelName = modelName.replace(
                            modelName.indexOf(modelName.at(0)),1,modelName.at(0).toUpper());
                submodelParts << QString("0 FILE %1").arg(submodel);
                submodelParts << QString("0 %1").arg(modelName);
                submodelParts << QString("0 Name: %1").arg(submodel);
                submodelParts << content;
                submodelParts << QLatin1String("0 NOFILE");
                if (writeContent(content, submodels) != 0)
                    return 1;
            }
        }
        parsedModels << submodel;
    }
    return 0;
}
