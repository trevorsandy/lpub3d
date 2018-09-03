
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

#include <QImage>
#include <QPainter>
#include <QImageWriter>

#include "imagematt.h"
#include "name.h"
#include "lpub.h"

#ifdef __APPLE__
#  ifdef _OSMESA
#    include <GL/gl.h>
#  else // _OSMESA
#    include <OpenGL/gl.h>
#  endif // _OSMESA
#else	// __APPLE__
#  include <GL/gl.h>
#endif	// __APPLE__

QHash<QString, int> ImageMatt::csikey2sequence;     // csiKey, sequence
QHash<int, QString> ImageMatt::csisequence2csifile; // sequence, csiFile

ImageMatt::ImageMatt()
{
//  csikey2sequence.clear();
//  step2csiimage.clear();
}

void ImageMatt::insertStepCSIImage(QString csiKey, QString csiFile)
{
    int sequence = csikey2sequence.size() + 1;
    csikey2sequence.insert(csiKey, sequence);
    //logDebug() << qPrintable(QString("Inserting csiKey %1 at sequence %2").arg(csiKey).arg(sequence));
    csisequence2csifile.insert(sequence,csiFile);
    //logNotice() << qPrintable(QString("Setting sequence %1 for csiFile %2").arg(csiKey).arg(csiFile));
}

QString ImageMatt::previousStepCSIImage(QString csiKey)
{
    int sequence = -1;

    if (csikey2sequence.contains(csiKey))
      sequence = csikey2sequence[csiKey];
    else
      return QString();

      logNotice() << qPrintable(QString("[DEBUG] Request: cisKey %1, sequence %2, return sequence %3").arg(csiKey).arg(csikey2sequence[csiKey]).arg(sequence));
    if (sequence > 1) {
      logNotice() << qPrintable(QString("[DEBUG] Return : sequence %2, csiFile %1").arg(csisequence2csifile[sequence - 1]).arg(sequence -1));
      return csisequence2csifile[sequence - 1];
    }

    return QString();
}

/*
 * This function provides the translate from csiKey to csiFile
 * and returns the absolute path csiFile for the current step.
 * If there is no translation, an empty string is returned.
 */
QString ImageMatt::currentStepCSIImage(QString csiKey)
{
    int sequence = -1;

    if (csikey2sequence.contains(csiKey))
      sequence = csikey2sequence[csiKey];
    else
      return QString();

      logNotice() << qPrintable(QString("[DEBUG] Request: cisKey %1, sequence %2, return sequence %3").arg(csiKey).arg(csikey2sequence[csiKey]).arg(sequence));
    if (sequence > 0) {
      logNotice() << qPrintable(QString("[DEBUG] Return : sequence %2, csiFile %1").arg(csisequence2csifile[sequence]).arg(sequence));
      return csisequence2csifile[sequence];
    }

    return QString();
}

bool ImageMatt::stepCSIImageExist(QString csiKey)
{
  if (csikey2sequence.contains(csiKey))
      return true;
  return false;
}

bool ImageMatt::processZMap(QString &prevImagePath, QString &currImagePath)
{
        QFileInfo currImageInfo(currImagePath);
        if (!currImageInfo.exists())
        {
                emit gui->messageSig(LOG_ERROR,QString("Current Step's Image File Not Found.").arg(prevImagePath));
                return false;
        }
        QFileInfo prevImageInfo(prevImagePath);
        if (!prevImageInfo.exists())
        {
                emit gui->messageSig(LOG_ERROR,QString("Previous Step's Image File Not Found.").arg(prevImagePath));
                return false;
        }

        // Get zMap Files
        QString currZMapPath = QString("%1/%2.ldvz").arg(currImageInfo.absolutePath()).arg(currImageInfo.baseName());
        QString prevZMapPath = QString("%1/%2.ldvz").arg(prevImageInfo.absolutePath()).arg(prevImageInfo.baseName());

        FILE *currZMap = fopen(currZMapPath.toLatin1().constData(), "rb");
        FILE *prevZMap = fopen(prevZMapPath.toLatin1().constData(), "rb");

        // Validate zMap files
        bool currZMapOk = zMapFileIsValid(currZMap, false);
        bool prevZMapOk = zMapFileIsValid(prevZMap, true);

        int32_t currZDataSize[2];
        size_t currZDataCount;
        std::vector<GLfloat> currZData;

        int32_t prevZDataSize[2];
        size_t prevZDataCount;
        std::vector<GLfloat> prevZData;

        if (currZMapOk)
        {
                 if (fread(&currZDataSize, sizeof(currZDataSize[0]), 2, currZMap) != 2)
                 {
                         emit gui->messageSig(LOG_ERROR,QString("Error Reading Current Step's Z Map Dimensions."));
                         return false;
                 }
                 currZDataCount = currZDataSize[0] * currZDataSize[1];
                 if (currZDataCount >= (2 << 28))
                 {
                         emit gui->messageSig(LOG_ERROR,QString("Current Step's  Z Map probably too big."));
                         return false;
                 }

                 currZData.resize(currZDataCount);
                 if (fread(&currZData[0], sizeof(GLfloat), currZData.size(), currZMap) != currZData.size())
                 {
                         emit gui->messageSig(LOG_ERROR,QString("Error Reading Current Step's Z Map Data."));
                         return false;
                 }
        }
        if (prevZMapOk)
        {
                 if (fread(&prevZDataSize, sizeof(prevZDataSize[0]), 2, prevZMap) != 2)
                 {
                         emit gui->messageSig(LOG_ERROR,QString("Error Reading Previous Step's Z Map Dimensions."));
                         return false;
                 }
                 prevZDataCount = prevZDataSize[0] * prevZDataSize[1];
                 if (prevZDataCount >= (2 << 28))
                 {
                         emit gui->messageSig(LOG_ERROR,QString("Previous Step's  Z Map probably too big."));
                         return false;
                 }
                 prevZData.resize(prevZDataCount);
                 if (fread(&prevZData[0], sizeof(GLfloat), prevZData.size(), prevZMap) != prevZData.size())
                 {
                         emit gui->messageSig(LOG_ERROR,QString("Error Reading Previous Step's Z Map Data."));
                         return false;
                 }
        }

        QImage currImage(currImageInfo.absoluteFilePath());
        currImage = currImage.convertToFormat(QImage::Format_ARGB32);
        if (currImage.isNull() )
        {
                 emit gui->messageSig(LOG_ERROR,QString("Current Step's Image is Null."));
                 return false;
        }
        QImage prevImage(prevImageInfo.absoluteFilePath());
        prevImage = prevImage.convertToFormat(QImage::Format_ARGB32);
        if (prevImage.isNull() )
        {
                 emit gui->messageSig(LOG_ERROR,QString("Previous Step's Image is Null."));
                 return false;
        }

        // Note - debug only code to adjust one-pixel difference in current and previous image height.
//        if (prevImage.size() != currImage.size())
//            prevImage = QImage(prevImage.copy(QRect(0,0,currImage.width(),currImage.height())));

        struct MappedImage
        {
                 QImage Image;
                 QRect Bounds;
        };

        MappedImage NewImage;

        auto CalculateImageBounds = [](MappedImage& _Image)
        {
                QImage& Image = _Image.Image;
                int Width = Image.width();
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
        };

        QImage TempImage(currZDataSize[0], currZDataSize[1], QImage::Format_ARGB32_Premultiplied);
        TempImage.fill(qRgba(255,255,255,0));

        QPainter painter(&TempImage);
        painter.setRenderHint(QPainter::Antialiasing);

        QRgb* pixels = new QRgb[currZDataSize[0] * currZDataSize[1]];

        for (int32_t colY = 0; colY < currZDataSize[1]; ++colY)
        {
                GLfloat *currZSpot = &currZData[colY * currZDataSize[0]];
                GLfloat *prevZSpot = &prevZData[colY * prevZDataSize[0]];

                for (int32_t rowX = 0; rowX < currZDataSize[0]; ++rowX)
                {
                      if (currZSpot < prevZSpot)
                        pixels[rowX + colY * currZDataSize[0]] = currImage.pixel( rowX, colY );
                      else
                        pixels[rowX + colY * currZDataSize[0]] = blendPixel(currImage.pixel( rowX, colY ),
                                                                            prevImage.pixel( rowX, colY ));
                }
        }
        painter.drawImage(0,0,TempImage);

        painter.end();

        NewImage.Image = TempImage;

        CalculateImageBounds(NewImage);

        QImageWriter Writer(currImagePath);

        if (Writer.format().isEmpty())
                Writer.setFormat("PNG");

        //if (!Writer.write(QImage(NewImage.Image.copy(NewImage.Bounds))))
       if (!Writer.write(NewImage.Image))
        {
                emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not write to Mapped CSI image file '%1': %2.")
                                     .arg(currImagePath, Writer.errorString()));
                return false;
        }

	return true;
}

// UTILITY FUNCTIONS

bool ImageMatt::zMapFileIsValid(FILE* zMapFile, bool current)
{
        QString step = current ? "Current Step's " : "Previous Step's ";

        char magic[5] = { 0 };
        char endian[5] = { 0 };
        if (fread(magic, 4, 1, zMapFile) != 1)
        {
                emit gui->messageSig(LOG_ERROR, QString("Error Reading %1 Z Map Magic Number.").arg(step));
                return false;
        }
        if (strcmp(magic, "ldvz") != 0)
        {
                emit gui->messageSig(LOG_ERROR,QString("Invalid %1 Z Map Magic Number.").arg(step));
                return false;
        }
        if (fread(endian, 4, 1, zMapFile) != 1)
        {
                emit gui->messageSig(LOG_ERROR,QString("Error Reading %1 Z Map Endian.").arg(step));
                return false;
        }
        const char *expectedEndian = "BIGE";
        if (isLittleEndian())
        {
                expectedEndian = "LITE";
        }
        if (strcmp(endian, expectedEndian) != 0)
        {
                emit gui->messageSig(LOG_ERROR,QString("Invalid Z Map Endian."));
                return false;
        }
        return true;
}

bool ImageMatt::isLittleEndian()
{
	short int number = 0x1;
	char *numPtr = (char*)&number;
	return (numPtr[0] == 1);
}

int ImageMatt::roundUp(int value, int nearest)
{
	return (value + nearest - 1) / nearest * nearest;
}

QRgb ImageMatt::blendPixel(const QRgb &_currPxl, const QRgb &_prevPxl)
{
	QColor currPxl(_currPxl);
	QColor prevPxl(_prevPxl);

	int rOut = (currPxl.red() * currPxl.alpha() / 255) + (prevPxl.red() * prevPxl.alpha() * (255 - currPxl.alpha()) / (255*255));
	int gOut = (prevPxl.green() * currPxl.alpha() / 255) + (prevPxl.green() * prevPxl.alpha() * (255 - currPxl.alpha()) / (255*255));
	int bOut = (prevPxl.blue() * currPxl.alpha() / 255) + (prevPxl.blue() * prevPxl.alpha() * (255 - currPxl.alpha()) / (255*255));
	int aOut =  currPxl.alpha() + (prevPxl.alpha() * (255 - currPxl.alpha()) / 255);

	return qRgba(rOut,gOut,bOut,aOut);
}
