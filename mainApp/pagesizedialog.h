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
 * size and/or orientation.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef PAGESIZEDIALOG_H
#define PAGESIZEDIALOG_H

#include <QDialog>
#include "meta.h"

class PageSizeGui;
class PageSizeDialog : public QDialog
{
  Q_OBJECT
public:

  PageSizeDialog(
    float           goods[],
    QString          name,
    QWidget         *parent);

  virtual ~PageSizeDialog();

  static bool getPageSize(
    float           goods[],
    QString        name,
    QWidget       *parent = 0);

    UnitsMeta       meta;

public slots:
  void accept();
  void cancel();

private:
  PageSizeGui *pageSize;
};

#endif // PAGESIZEDIALOG_H
