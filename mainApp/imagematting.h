
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

#ifndef IMAGEMATTING_H
#define IMAGEMATTING_H

#include <QHash>
#include <QString>

/*
 * This class encapsulates image matting functions
 *
 */

class ImageMatting {
public:
  /*
   * This constructor clears the hash maps
   */
  ImageMatting();
  /*
   * This function provides the translate from csiKey to csiFile
   * and returns the absolute path csiFile value if it exist.
   * If there is no translation, an empty string is returned.
   */
  static QString previousStepCSIImage(QString csiKey);
  /*
   * This function inserts an image file entry
   * Required attributes are a csiKey (csiName + stepNumber)
   * and the csiFile includeing absolute path
   */
  void insertStepCSIImage(QString csiKey, QString csiFile);
  /*
   * This function performs a lookup of the provided csiKey
   * and returns true if found or false if not found
   */
  bool stepCSIImageExist(QString csiKey);
private:
  static QHash<QString, int> csikey2sequence;       // csiKey, sequence
  static QHash<int, QString> csisequence2csifile; // sequence, csiFile

};

#endif // IMAGEMATTING_H


