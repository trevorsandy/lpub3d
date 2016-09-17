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

#ifndef DIALOGEXPORTPAGES_H
#define DIALOGEXPORTPAGES_H

#include <QDialog>
#include "lpub.h"

namespace Ui {
  class DialogExportPages;
}

class DialogExportPages : public QDialog
{
  Q_OBJECT

public:
  explicit DialogExportPages(QWidget *parent = 0);
  ~DialogExportPages();

  bool allPages();
  bool currentPage();
  bool pageRange();
  bool resetCache();
  bool mixedPageSize();
  QString const pageRangeText();

private:
  Ui::DialogExportPages *ui;
};

#endif // DIALOGEXPORTPAGES_H
