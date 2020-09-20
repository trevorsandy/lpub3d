
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

#ifndef LDVIMAGEMATTE_H
#define LDVIMAGEMATTE_H

#include <QHash>
#include <QString>
#include <QRgb>

/*
 * This class encapsulates image matting functions
 *
 */
class LDVImageMatte
{
public:
  /*
   * This constructor
   */
  LDVImageMatte();

  /*
   * This function inserts an image file entry mapped to the csiKey
   * and a csi key entry mapped to the imageFile.
   * Required attributes are a csiKey (csiName() + stepNumber)
   * and the csiFile defined with absolute path
   */
  static void insertMatteCSIImage(QString key, QString value);

  /*
   * This function deletes an image file entry mapped to the csiKey
   * or a csi key entry mapped to the imageFile.
   * Required attribute is a csiKey (csiName() + stepNumber)
   * or the csiFile defined with absolute path
   */
  static void removeMatteCSIImage(QString key);

  /*
   * This function provides the translate from csiKey to csiFile or
   * csiFile to csiKey and returns either the csiKey or csiFile
   * absolute path for the current step. If there is no translation,
   * an empty string is returned.
   */
  static QString getMatteCSIImage(QString key);

  /*
   * This function returns true or false if there is a valid
   * image file or csiKey entry.
   * Required attributes are a csiKey (csiName() + stepNumber)
   * or the csiFile includeing absolute path
   */
  static bool validMatteCSIImage(QString key);

  /*
   * This function clears the csiKey2csiFile
   * and csiFile2csiKey QHashes
   */
  static void clearMatteCSIImages();

  /*
   * This function splits the input CSI ldr step file
   * into previous steps and current step ldr files.
   * Each file is subsequently rendered along with its
   * respective ZMap file.
   */
 static bool matteCSIImage(QStringList &arguments, QString &csiKey);

  /*
   * This function performs a comparison of the ZMap files
   * for two images (previous steps and current step) to
   * combine the images using pixel blending.
   */
  static bool matteCSIImages(QString csiKey, QString &baseImagePath, QString & overlayImagePath);

private:
  static QHash<QString, QString> csiKey2csiFile;    // csiKey, csiFileName
  static QHash<QString, QString> csiFile2csiKey;    // csiFileName, csiKey

};

#endif // LDVIMAGEMATTE_H
