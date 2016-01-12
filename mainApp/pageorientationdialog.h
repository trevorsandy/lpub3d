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
 * This file implements a dialog that allows the user to change the page
 * pageOrientation.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef PAGEORIENTATIONDIALOG_H
#define PAGEORIENTATIONDIALOG_H


#include <QDialog>
#include "meta.h"

class PageOrientationGui;
class PageOrientationDialog : public QDialog
{
  Q_OBJECT
public:

  PageOrientationDialog(
    OrientationEnc  &goods,
    QString          name,
    QWidget         *parent);

  virtual ~PageOrientationDialog();

  static bool getPageOrientation(
    OrientationEnc &goods,
    QString        name,
    QWidget       *parent = 0);

    PageOrientationMeta meta;

public slots:
  void accept();
  void cancel();

private:
  PageOrientationGui *pageOrientation;
};

#endif // PAGEORIENTATIONDIALOG_H
