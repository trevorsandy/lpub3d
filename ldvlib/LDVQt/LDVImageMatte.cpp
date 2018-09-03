
/****************************************************************************
**
** Copyright (C) 2018 Trevor SANDY. All rights reserved.
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
 * This function inserts an image file entry mapped to the csiKey
 * and inserts a csi key entry mapped to the imageFile.
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
 * This function deletes an image file entry mapped to the csiKey
 * or a csi key entry mapped to the imageFile.
 * Required attribute is a csiKey (csiName() + stepNumber)
 * or the csiFile defined with absolute path
 */
void LDVImageMatte::removeMatteCSIImage(QString key) {

  bool ok;
  int removed = 0;
  QString item;
  int validInt = QString(key).right(1).toInt(&ok);
  if (ok && validInt >= 0) {
      if (csiKey2csiFile.contains(key))
        item =  csiKey2csiFile[key];
      removed = csiKey2csiFile.remove(key);
    }
  else
    if (QFileInfo(key).completeSuffix().toLower() == QString("png").toLower()) {
        if (csiFile2csiKey.contains(key))
          item =  csiFile2csiKey[key];
        removed = csiFile2csiKey.remove(key);
      }
  emit lpubAlert->messageSig(LOG_INFO,QString("Removed %1 item, CSI Key: %2 for Item : %3").arg(removed).arg(key).arg(item));
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

bool LDVImageMatte::matteCSIImage(QStringList &arguments, QString &csiKey) {

  bool renderFailed = false;
  QString ext;
  QFileInfo csiFileInfo;
  QString tempPath = QDir::currentPath() + "/" +  Paths::tmpDir;
  if (Render::useLDViewSCall()){
      ext = ".png";
      csiFileInfo.setFile(getMatteCSIImage(csiKey));
    } else {
      ext = ".ldr";
      csiFileInfo.setFile(QString("%1/csi%2").arg(tempPath).arg(ext));
    }

  // ldr and png IM file extensions
  QString im_prev_png_ext = QString(".%1").arg(LPUB3D_IM_PREV_PNG_EXT);
  QString im_prev_ldr_ext = QString(".%1").arg(LPUB3D_IM_PREV_LDR_EXT);
  QString im_curr_png_ext = QString(".%1").arg(LPUB3D_IM_CURR_PNG_EXT);
  QString im_curr_ldr_ext = QString(".%1").arg(LPUB3D_IM_CURR_LDR_EXT);

  // Generate previous png and ZMap files
  QString prevPngFile = QString(csiFileInfo.absoluteFilePath()).replace(ext,im_prev_png_ext);
  QString prevLdrFile = QString("%1/%2").arg(tempPath)
                                .arg(QString(csiFileInfo.fileName()).replace(ext,im_prev_ldr_ext));
  if (QFileInfo(prevLdrFile).exists()) {

      arguments.insert(5, QString("-SaveSnapShot=%1").arg(prevPngFile));
      arguments.append(prevLdrFile);                 // ldrName

      if (Render::executeLDViewProcess(arguments, Render::CSI) == -1)
        renderFailed = true;
    } else {
      emit lpubAlert->messageSig(LOG_ERROR,QString("Could not find prevLdrFile %1")
                                 .arg(prevLdrFile));
      return false;
    }

  // Generate current png and ZMap files
  QString currPngFile = QString(csiFileInfo.absoluteFilePath()).replace(ext,im_curr_png_ext);
  QString currLdrFile = QString("%1/%2").arg(tempPath)
                                .arg(QString(csiFileInfo.fileName()).replace(ext,im_curr_ldr_ext));
  if (QFileInfo(currLdrFile).exists()) {

      arguments.removeOne(QString("-SaveSnapShot=%1").arg(prevPngFile));
      arguments.removeOne(prevLdrFile);

      arguments.insert(5, QString("-SaveSnapShot=%1").arg(currPngFile));
      arguments.append(currLdrFile);                   // ldrName

      if (Render::executeLDViewProcess(arguments, Render::CSI) == -1)
        renderFailed = true;
    } else {
      emit lpubAlert->messageSig(LOG_ERROR,QString("Could not find currLdrFile %1")
                                 .arg(currLdrFile));
      return false;
    }

  if (renderFailed) {
      emit lpubAlert->messageSig(LOG_ERROR,QString("LDView CSI render failed for arguments %1")
                                 .arg(arguments.join(" ")));
      return false;
    }

  // merge images
  return matteCSIImages(csiKey, prevPngFile, currPngFile);
}

bool LDVImageMatte::matteCSIImages(QString csiKey, QString &prevImagePath, QString &currImagePath)
{
  // Process current ZMap file
  QFileInfo currImageInfo(currImagePath);
  if (!currImageInfo.exists())
    {
      emit lpubAlert->messageSig(LOG_ERROR,QString("Current Step's Image File Not Found.").arg(prevImagePath));
      return false;
    }

  QString currZMapPath = QString(currImageInfo.absoluteFilePath()).replace(".png",".ldvz");

  FILE *currZMap = fopen(currZMapPath.toLatin1().constData(), "rb");

  bool currZMapOk = zMapFileIsValid(currZMap, false);

  int32_t currZDataSize[2];
  size_t currZDataCount;
  std::vector<GLfloat> currZData;

  if (currZMapOk)
    {
      if (fread(&currZDataSize, sizeof(currZDataSize[0]), 2, currZMap) != 2)
        {
          emit lpubAlert->messageSig(LOG_ERROR,QString("Error Reading Current Step's Z Map Dimensions."));
          return false;
        }
      currZDataCount = currZDataSize[0] * currZDataSize[1];
      if (currZDataCount >= (2 << 28))
        {
          emit lpubAlert->messageSig(LOG_ERROR,QString("Current Step's  Z Map probably too big."));
          return false;
        }

      currZData.resize(currZDataCount);
      if (fread(&currZData[0], sizeof(GLfloat), currZData.size(), currZMap) != currZData.size())
        {
          emit lpubAlert->messageSig(LOG_ERROR,QString("Error Reading Current Step's Z Map Data."));
          return false;
        }
    }

  // Process previous ZMap file
  QFileInfo prevImageInfo(prevImagePath);
  if (!prevImageInfo.exists())
    {
      emit lpubAlert->messageSig(LOG_ERROR,QString("Previous Step's Image File Not Found.").arg(prevImagePath));
      return false;
    }

  QString prevZMapPath = QString(prevImageInfo.absoluteFilePath()).replace(".png",".ldvz");

  FILE *prevZMap = fopen(prevZMapPath.toLatin1().constData(), "rb");

  bool prevZMapOk = zMapFileIsValid(prevZMap, true);

  int32_t prevZDataSize[2];
  size_t prevZDataCount;
  std::vector<GLfloat> prevZData;

  if (prevZMapOk)
    {
      if (fread(&prevZDataSize, sizeof(prevZDataSize[0]), 2, prevZMap) != 2)
        {
          emit lpubAlert->messageSig(LOG_ERROR,QString("Error Reading Previous Step's Z Map Dimensions."));
          return false;
        }
      prevZDataCount = prevZDataSize[0] * prevZDataSize[1];
      if (prevZDataCount >= (2 << 28))
        {
          emit lpubAlert->messageSig(LOG_ERROR,QString("Previous Step's  Z Map probably too big."));
          return false;
        }
      prevZData.resize(prevZDataCount);
      if (fread(&prevZData[0], sizeof(GLfloat), prevZData.size(), prevZMap) != prevZData.size())
        {
          emit lpubAlert->messageSig(LOG_ERROR,QString("Error Reading Previous Step's Z Map Data."));
          return false;
        }
    }

  QImage currImage(currImageInfo.absoluteFilePath());
  currImage = currImage.convertToFormat(QImage::Format_ARGB32);
  if (currImage.isNull() )
    {
      emit lpubAlert->messageSig(LOG_ERROR,QString("Current Step's Image is Null."));
      return false;
    }

  QImage prevImage(prevImageInfo.absoluteFilePath());
  prevImage = prevImage.convertToFormat(QImage::Format_ARGB32);
  if (prevImage.isNull() )
    {
      emit lpubAlert->messageSig(LOG_ERROR,QString("Previous Step's Image is Null."));
      return false;
    }

  LogType logType;
  QString imageWidthMsg (QString("Matte Image -  Current Width: %1,  Previous Width: %2")
                        .arg(currImage.width())
                        .arg(prevImage.width()));
  logType = currImage.width() != prevImage.width() ? LOG_INFO : LOG_STATUS;
  emit lpubAlert->messageSig(logType,imageWidthMsg);

  QString imageHeightMsg(QString("Matte Image - Current Height: %1, Previous Height: %2")
                         .arg(currImage.height())
                         .arg(prevImage.height()));
  logType = currImage.height() != prevImage.height() ? LOG_INFO : LOG_STATUS;
  emit lpubAlert->messageSig(logType,imageHeightMsg);

  QImage newImage(currZDataSize[0], currZDataSize[1], QImage::Format_ARGB32_Premultiplied);
  newImage.fill(qRgba(255,255,255,0));

  for (int32_t colY = 0; colY < currZDataSize[1]; ++colY)
    {
      GLfloat *currZSpot = &currZData[colY * currZDataSize[0]];
      GLfloat *prevZSpot = &prevZData[colY * prevZDataSize[0]];

      for (int32_t rowX = 0; rowX < currZDataSize[0]; ++rowX)
        {
          QRgb pixelSpot;
          if (currZSpot < prevZSpot) {   // this seems to always be true
            pixelSpot = currImage.pixel( rowX, colY );
          } else {
            pixelSpot = blendPixel(currImage.pixel( rowX, colY ),
                                   prevImage.pixel( rowX, colY ));
          }
          newImage.setPixel( rowX, colY, pixelSpot);
        }
    }

  QPainter painter(&newImage);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.drawImage(0,0,newImage);
  painter.end();

  struct MappedImage
  {
    QImage Image;
    QString Path;
    QRect Bounds;
    QString BoundMsg;
  };

  MappedImage mappedImage;
  auto CalculateImageBounds = [](MappedImage& _Image)
  {
    QImage& Image = _Image.Image;
    int Width  = Image.width();
    int Height = Image.height();

    int MinX = Width;
    int MinY = Height;
    int MaxX = 0;
    int MaxY = 0;

    for (int x = 0; x < Width; x++)
      {
        for (int y = 0; y < Height; y++)
          {
            if (qAlpha(Image.pixel(x, y)))
              {
                MinX = qMin(x, MinX);
                MinY = qMin(y, MinY);
                MaxX = qMax(x, MaxX);
                MaxY = qMax(y, MaxY);
              }
          }
      }

    _Image.Bounds = QRect(QPoint(MinX, MinY), QPoint(MaxX, MaxY));
    _Image.BoundMsg = QString("%1 (w:%2 x h:%3)")
                              .arg(_Image.Path)
                              .arg(_Image.Bounds.width())
                              .arg(_Image.Bounds.height());
  };

  QString newImagePath = getMatteCSIImage(csiKey);
  mappedImage.Image    = newImage;
  mappedImage.Path     = newImagePath;
  CalculateImageBounds(mappedImage);

  QImageWriter Writer(mappedImage.Path);
  if (Writer.format().isEmpty())
    Writer.setFormat("PNG");

  if (Writer.write(QImage(mappedImage.Image.copy(mappedImage.Bounds)))) {
      emit lpubAlert->messageSig(LOG_INFO, mappedImage.BoundMsg);
    } else {
      emit lpubAlert->messageSig(LOG_ERROR, QString("Failed to write clipped image '%1': %2.")
                                 .arg(mappedImage.Path, Writer.errorString()));
      return false;
    }
  return true;
}

// UTILITY FUNCTIONS

QRgb LDVImageMatte::blendPixel(const QRgb &_currPxl, const QRgb &_prevPxl)
{
  QColor currPxl(_currPxl);
  QColor prevPxl(_prevPxl);

  int rOut = (currPxl.red() * currPxl.alpha() / 255) + (prevPxl.red() * prevPxl.alpha() * (255 - currPxl.alpha()) / (255*255));
  int gOut = (prevPxl.green() * currPxl.alpha() / 255) + (prevPxl.green() * prevPxl.alpha() * (255 - currPxl.alpha()) / (255*255));
  int bOut = (prevPxl.blue() * currPxl.alpha() / 255) + (prevPxl.blue() * prevPxl.alpha() * (255 - currPxl.alpha()) / (255*255));
  int aOut =  currPxl.alpha() + (prevPxl.alpha() * (255 - currPxl.alpha()) / 255);

  return qRgba(rOut,gOut,bOut,aOut);
}

bool LDVImageMatte::zMapFileIsValid(FILE* zMapFile, bool current)
{
  QString step = current ? "Current Step's " : "Previous Step's ";

  char magic[5] = { 0 };
  char endian[5] = { 0 };
  if (fread(magic, 4, 1, zMapFile) != 1)
    {
      emit lpubAlert->messageSig(LOG_ERROR, QString("Error Reading %1 Z Map Magic Number.").arg(step));
      return false;
    }
  if (strcmp(magic, "ldvz") != 0)
    {
      emit lpubAlert->messageSig(LOG_ERROR,QString("Invalid %1 Z Map Magic Number.").arg(step));
      return false;
    }
  if (fread(endian, 4, 1, zMapFile) != 1)
    {
      emit lpubAlert->messageSig(LOG_ERROR,QString("Error Reading %1 Z Map Endian.").arg(step));
      return false;
    }
  const char *expectedEndian = "BIGE";
  if (isLittleEndian())
    {
      expectedEndian = "LITE";
    }
  if (strcmp(endian, expectedEndian) != 0)
    {
      emit lpubAlert->messageSig(LOG_ERROR,QString("Invalid Z Map Endian."));
      return false;
    }
  return true;
}
