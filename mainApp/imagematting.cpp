
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

#include "imagematting.h"
#include "name.h"
#include "QsLog.h"

QHash<QString, int> ImageMatting::csikey2sequence;     // csiKey, sequence
QHash<int, QString> ImageMatting::csisequence2csifile; // sequence, csiFile

ImageMatting::ImageMatting()
{
//  csikey2sequence.clear();
//  step2csiimage.clear();
}

void ImageMatting::insertStepCSIImage(QString csiKey, QString csiFile)
{
    int sequence = csikey2sequence.size() + 1;
    csikey2sequence.insert(csiKey, sequence);
    //logDebug() << qPrintable(QString("Inserting csiKey %1 at sequence %2").arg(csiKey).arg(sequence));
    csisequence2csifile.insert(sequence,csiFile);
    //logNotice() << qPrintable(QString("Setting sequence %1 for csiFile %2").arg(csiKey).arg(csiFile));

}

QString ImageMatting::previousStepCSIImage(QString csiKey)
{
    int sequence = -1;

    if (csikey2sequence.contains(csiKey))
      sequence = csikey2sequence[csiKey] - 1;
    else
      return QString();

    if (sequence > 1) {
      //logNotice() << qPrintable(QString("Request: sequence %2, cisKey %1, return sequence %3").arg(csiKey).arg(csikey2sequence[csiKey]).arg(sequence));
      //logNotice() << qPrintable(QString("Return : sequence %2, csiFile %1").arg(csisequence2csifile[sequence]).arg(sequence));
      return csisequence2csifile[sequence];
    }

    return QString();
}

bool ImageMatting::stepCSIImageExist(QString csiKey)
{
  if (csikey2sequence.contains(csiKey))
      return true;
  return false;
}
