
/****************************************************************************
**
** Copyright (C) 2018 - 2020 Trevor SANDY. All rights reserved.
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

#include <QDir>
#include <QImage>
#include <QPainter>
#include <QFileInfo>
#include <QImageWriter>
#include "LDVImageMatte.h"

#include <TCFoundation/mystring.h>
#include <TRE/TREGLExtensions.h>
#include <WPngImage.hh>

#include "QsLog.h"
#include "lpubalert.h"
#include "render.h"
#include "name.h"
#include "paths.h"

QHash<QString, QString> LDVImageMatte::csiKey2csiFile;  // csiKey, csiFile
QHash<QString, QString> LDVImageMatte::csiFile2csiKey;  // csiFileName, csiKey

/*
 * This constructor
 */
LDVImageMatte::LDVImageMatte() {}

/*
 * This function inserts an image file entry clipped to the csiKey
 * and inserts a csi key entry clipped to the imageFile.
 * Required attributes are a csiKey (csiName() + stepNumber)
 * and the csiFile defined with absolute path
 */
void LDVImageMatte::insertMatteCSIImage(QString key, QString value)
{
  bool ok;
  int validInt = QString(key).right(1).toInt(&ok);
  if (ok && validInt >= 0) {
      csiKey2csiFile.insert(key,value);
      csiFile2csiKey.insert(value,key);
    }
  else
    if (QFileInfo(key).completeSuffix().toLower() == QString("png").toLower()) {
        csiFile2csiKey.insert(key,value);
        csiKey2csiFile.insert(value,value);
      }
//  if (!key.isEmpty()) {
//      logDebug() << qPrintable(QString("[DEBUG] Insert CSI Key:  %1 for File: %2").arg(key).arg(csiKey2csiFile[key]));
//      logDebug() << qPrintable(QString("[DEBUG] Insert CSI File: %1 for Key : %2").arg(value).arg(csiFile2csiKey[value]));
//    }
}

/*
 * This function deletes an image file entry clipped to the csiKey
 * or a csi key entry clipped to the imageFile.
 * Required attribute is a csiKey (csiName() + stepNumber)
 * or the csiFile defined with absolute path
 */
void LDVImageMatte::removeMatteCSIImage(QString item) {

  bool ok;
  int removed = 0;
  QString csiKey, csiFile;
  int validInt = QString(item).right(1).toInt(&ok);
  if (ok && validInt >= 0) {
      csiKey = item;
      if (csiKey2csiFile.contains(csiKey)) {
          csiFile =  csiKey2csiFile[csiKey];
          removed = csiKey2csiFile.remove(csiKey);
        if (removed != 0)
            removed = csiFile2csiKey.remove(csiFile);
        }
    }
  else
    if (QFileInfo(item).completeSuffix().toLower() == QString("png").toLower()) {
        csiFile = item;
        if (csiFile2csiKey.contains(csiFile))
          csiKey =  csiFile2csiKey[csiFile];
        removed = csiFile2csiKey.remove(csiFile);
        if (removed != 0)
          removed = csiKey2csiFile.remove(csiKey);
      }
  emit lpubAlert->messageSig(LOG_INFO,QString("Removed %1 item, CSI Key: %2 for File: %3").arg(removed).arg(csiKey).arg(csiFile));
}

/*
 * This function provides the translate from csiKey to csiFile or
 * csiFile to csiKey and returns either the csiKey or csiFile
 * absolute path for the current step. If there is no translation,
 * an empty string is returned.
 */
QString LDVImageMatte::getMatteCSIImage(QString key)
{
  bool ok;
  int validInt = QString(key).right(1).toInt(&ok);
  if (ok && validInt >= 0) {
      if (csiKey2csiFile.contains(key))
        return csiKey2csiFile[key];
    }
  else
    if (QFileInfo(key).completeSuffix().toLower() == QString("png").toLower()){
        if (csiFile2csiKey.contains(key))
          return csiFile2csiKey[key];
      }
  return QString();
}

/*
 * This function returns true or false if there is a valid
 * image file or csiKey entry.
 * Required attributes are a csiKey (csiName() + stepNumber)
 * or the csiFile includeing absolute path
 */
bool LDVImageMatte::validMatteCSIImage(QString key)
{
  bool ok;
  int validInt = QString(key).right(1).toInt(&ok);
  if (ok && validInt >= 0) {
      if (csiKey2csiFile.contains(key)) {
          return true;
        }
    }
  else
    if (QFileInfo(key).completeSuffix().toLower() == QString("png")){
        if (csiFile2csiKey.contains(key)) {
            return true;
          }
      }
  return false;
}

/*
 * This function clears the csiKey2csiFile QHash
 */
void LDVImageMatte::clearMatteCSIImages(){
  csiKey2csiFile.clear();
  csiFile2csiKey.clear();
}

// Generate PNG IM images...
bool LDVImageMatte::matteCSIImage(QStringList &arguments, QString &csiKey) {

  if (!validMatteCSIImage(csiKey)){
      emit lpubAlert->messageSig(LOG_ERROR,QString("csiKey %1 does not exist.")
                                 .arg(csiKey));
      return false;
    }

  QString ext;
  QFileInfo csiIMFileInfo;
  QString tempPath = QDir::currentPath() + "/" + Paths::tmpDir;
  if (Render::useLDViewSCall()){
      ext = ".png";
      csiIMFileInfo.setFile(QString("%1/%2").arg(tempPath).arg(QFileInfo(getMatteCSIImage(csiKey)).fileName()));
    } else {
      ext = ".ldr";
      csiIMFileInfo.setFile(QString("%1/csi%2").arg(tempPath).arg(ext));
    }

  // Ldr IM file extensions
  QString base_ldr_ext = QString(".%1").arg(LPUB3D_IM_BASE_LDR_EXT);
  QString overlay_ldr_ext = QString(".%1").arg(LPUB3D_IM_OVERLAY_LDR_EXT);

  // Check previous ldr file
  QString baseLdrFile = QString("%1/%2").arg(tempPath)
                                .arg(QString(csiIMFileInfo.fileName()).replace(ext,base_ldr_ext));
  if (QFileInfo(baseLdrFile).exists()) {
      arguments.append(baseLdrFile);                 // ldrName
    } else {
      emit lpubAlert->messageSig(LOG_ERROR,QString("Could not find baseLdrFile %1")
                                 .arg(baseLdrFile));
      return false;
    }

  // Check current ldr file
  QString overlayLdrFile = QString("%1/%2").arg(tempPath)
                                .arg(QString(csiIMFileInfo.fileName()).replace(ext,overlay_ldr_ext));
  if (QFileInfo(overlayLdrFile).exists()) {
      arguments.append(overlayLdrFile);                 // ldrName
    } else {
      emit lpubAlert->messageSig(LOG_ERROR,QString("Could not find overlayLdrFile %1")
                                 .arg(overlayLdrFile));
      return false;
    }

  // Generate IM png file pair
  if (Render::executeLDViewProcess(arguments, Options::CSI) != 0) {
      emit lpubAlert->messageSig(LOG_ERROR,QString("LDView CSI IM render failed for arguments %1")
                                 .arg(arguments.join(" ")));
      return false;
    }

  // Png IM file extensions
  QString base_png_ext = QString(".%1").arg(LPUB3D_IM_BASE_PNG_EXT);
  QString overlay_png_ext = QString(".%1").arg(LPUB3D_IM_OVERLAY_PNG_EXT);

  // Check previous png file
  QString basePngFile = QString(csiIMFileInfo.absoluteFilePath()).replace(ext,base_png_ext);
  if (!QFileInfo(basePngFile).exists()) {
      emit lpubAlert->messageSig(LOG_ERROR,QString("Could not find basePngFile %1")
                                 .arg(basePngFile));
      return false;
    }

  // Check current png file
  QString overlayPngFile = QString(csiIMFileInfo.absoluteFilePath()).replace(ext,overlay_png_ext);
  if (!QFileInfo(overlayPngFile).exists()) {
      emit lpubAlert->messageSig(LOG_ERROR,QString("Could not find overlayPngFile %1")
                                 .arg(overlayPngFile));
      return false;
    }

  // merge images
  return matteCSIImages(csiKey, basePngFile, overlayPngFile);
}

bool LDVImageMatte::matteCSIImages(QString csiKey, QString &baseImagePath, QString &overlayImagePath)
{

  QFileInfo overlayImageInfo(overlayImagePath);
  if (!overlayImageInfo.exists()) {
      emit lpubAlert->messageSig(LOG_ERROR,QString("Base Image File Not Found %1.").arg(overlayImageInfo.absoluteFilePath()));
      return false;
    }

  WPngImage overlayImage;
  const auto overlayImageStatus = overlayImage.loadImage(overlayImageInfo.absoluteFilePath().toLatin1().constData(),WPngImage::kPixelFormat_RGBA16);
  if (overlayImageStatus.printErrorMsg()) return false;

  QFileInfo baseImageInfo(baseImagePath);
  if (!baseImageInfo.exists()) {
      emit lpubAlert->messageSig(LOG_ERROR,QString("Overlay Image File Not Found %1.").arg(baseImageInfo.absoluteFilePath()));
      return false;
    }

  WPngImage baseImage;
  const auto prevStatus = baseImage.loadImage(baseImageInfo.absoluteFilePath().toLatin1().constData(),WPngImage::kPixelFormat_RGBA16);
  if (prevStatus.printErrorMsg())
    return false;

  // Compare base and overlay image sizes
  LogType logType;
  QString imageWidthMsg (QString("Matte Image -  Base Width: %1,  Overlay Width: %2")
                        .arg(overlayImage.width())
                        .arg(baseImage.width()));
  logType = overlayImage.width() != baseImage.width() ? LOG_INFO : LOG_STATUS;
  emit lpubAlert->messageSig(logType,imageWidthMsg);

  QString imageHeightMsg(QString("Matte Image - Base Height: %1, Overlay Height: %2")
                         .arg(overlayImage.height())
                         .arg(baseImage.height()));
  logType = overlayImage.height() != baseImage.height() ? LOG_INFO : LOG_STATUS;
  emit lpubAlert->messageSig(logType,imageHeightMsg);

  // draw the overlay image on top of the base image
//  baseImage.drawImage(0,0, overlayImage);

  // draw the overlay image pixel on top of the base image pixel
  for(int y = 0; y < baseImage.height(); ++y)
    {
      for(int x = 0; x < baseImage.width(); ++x)
        {
          baseImage.drawPixel(x, y, overlayImage.get16(x, y));
        }
    }

  struct ClippedImage
  {
    WPngImage Image;
    QString Path;
    QRect Bounds;
    QString BoundMsg;
  };

  ClippedImage clippedImage;
  auto CalculateImageBounds = [](ClippedImage& _Image)
  {
    WPngImage& Image = _Image.Image;

    int MinX = Image.width();
    int MinY = Image.height();
    int MaxX = 0;
    int MaxY = 0;

    for(int y = 0; y < Image.height(); ++y)
      {
        for(int x = 0; x < Image.width(); ++x)
          {
            //qDebug() << QString("Image.get16(x[%1], y[%2]).a[%3]").arg(x).arg(y).arg(Image.get16(x, y).a);
            if (Image.get16(x, y).a)  // .a = 0
              {
                MinX = qMin(x, MinX);
                MinY = qMin(y, MinY);
                MaxX = qMax(x, MaxX);
                MaxY = qMax(y, MaxY);
              }
          }
      }

    _Image.Bounds = QRect(QPoint(MinX, MinY), QPoint(MaxX, MaxY));
    _Image.Image.resizeCanvas(_Image.Bounds.x(),_Image.Bounds.y(),_Image.Bounds.width(), _Image.Bounds.height());
    _Image.BoundMsg = QString("Matte Image %1 clipped to Width %2 x Height %3")
                              .arg(QFileInfo(_Image.Path).fileName())
                              .arg(_Image.Image.width())
                              .arg(_Image.Image.height());
  };

  clippedImage.Image    = baseImage;
  clippedImage.Path     = getMatteCSIImage(csiKey);
  CalculateImageBounds(clippedImage);

  const auto clippedImageStatus = clippedImage.Image.saveImage(clippedImage.Path.toLatin1().constData());
  if (clippedImageStatus.printErrorMsg()) {
      return false;
    } else {
      emit lpubAlert->messageSig(LOG_INFO, clippedImage.BoundMsg);
    }

  return true;
}
