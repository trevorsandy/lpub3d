/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
 *
 * This class implements a simple dialog that lets the user specify annotation
 * options for the parts list images.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef PLIANNOTATIONDIALOG_H
#define PLIANNOTATIONDIALOG_H

#include <QDialog>
#include "meta.h"

class PliAnnotationGui;
class PliAnnotationDialog : public QDialog
{
  Q_OBJECT
public:

  PliAnnotationDialog(
    PliAnnotationMeta &goods,
    QString            name,
    QWidget           *parent);

  virtual ~PliAnnotationDialog();

  static bool getPliAnnotationOption(
    PliAnnotationMeta &goods,
    QString            name,
    QWidget           *parent);

    PliAnnotationMeta    meta;

public slots:
  void accept();
  void cancel();

private:
  PliAnnotationGui *annotation;
};

#endif // PLIANNOTATIONDIALOG_H
