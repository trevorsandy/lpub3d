/****************************************************************************
**
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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
#include <QEvent>

namespace Ui {
  class DialogExportPages;
}

class DialogExportPages : public QDialog
{
  Q_OBJECT

public:
  explicit DialogExportPages(QWidget *parent = nullptr);
  ~DialogExportPages();

  bool allPages();
  bool currentPage();
  bool pageRange();
  bool resetCache();
  bool ignoreMixedPageSizesMsg();
  bool doNotShowPageProcessDlg();
  bool pdfPageImage();
  int pageDisplayPause();
  void groupBoxPixelRatio(bool);
  qreal exportPixelRatio();
  QString const pageRangeText();
  void getPixelRatioMsg(double value);

private slots:
  void on_lineEditPageRange_textChanged(const QString &arg1);
  void on_pushButtonExportSettings_clicked();
  void on_pushButtonExportPreferences_clicked();
  void on_spinPixelRatio_valueChanged(double arg1);
  void lineEditPageRangeFocusChanged(bool);

private:
  Ui::DialogExportPages *ui;
  QString linePageRange;
  int flag;
};

class FocusWatcher : public QObject
{
   Q_OBJECT
public:
   explicit FocusWatcher(QObject* parent = nullptr) : QObject(parent)
   {
      if (parent)
         parent->installEventFilter(this);
   }
   virtual bool eventFilter(QObject *obj, QEvent *event) override
   {
      Q_UNUSED(obj)
      if (event->type() == QEvent::FocusIn)
         emit focusChanged(true);
      else if (event->type() == QEvent::FocusOut)
         emit focusChanged(false);

      return false;
   }

Q_SIGNALS:
   void focusChanged(bool in);
};

#endif // DIALOGEXPORTPAGES_H
