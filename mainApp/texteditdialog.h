/****************************************************************************
**
** Copyright (C) 2019 - 2025 Trevor SANDY. All rights reserved.
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
 * This class implements a window that lets the user edit plain text.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#ifndef TEXTEDITDIALOG_H
#define TEXTEDITDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class TextEditDialog;
}
class QTextCharFormat;
QT_END_NAMESPACE

class TextEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TextEditDialog(QWidget  *parent = nullptr);
    ~TextEditDialog();

     void initialize(      QString  &goods,
                     const QString  &editFont,
                     const QString  &editFontColor,
                     const QString  &windowTitle,
                           bool      richText = false,
                           bool      fontActions = false);

    static bool getText(
      QString  &goods,
      QString   windowTitle);

    static bool getText(
      QString  &goods,
      QString  &editFont,
      QString  &editFontColor,
      bool     &richText,
      QString   windowTitle,
      bool      fontActions = false);

    static bool richText;
    static bool rendererArgs;
    static QString text;
    static QFont   font;
    static QColor  fontColor;

private slots:
    void currentCharFormatChanged(const QTextCharFormat &format);

    void on_actionNew_triggered();

    void on_actionAccept_triggered();

    void on_actionCancel_triggered();

    void on_actionCopy_triggered();

    void on_actionCut_triggered();

    void on_actionPaste_triggered();

    void on_actionUndo_triggered();

    void on_actionRedo_triggered();

    void on_actionRichText_triggered();

    void on_actionFont_triggered();

    void on_actionFontColor_triggered();

    void on_actionBold_triggered();

    void on_actionUnderline_triggered();

    void on_actionItalic_triggered();

    void closeEvent(QCloseEvent *event) override;

    bool maybeSave();

private:
    Ui::TextEditDialog *ui;
    void fontChanged(const QFont &f);
};

extern class TextEditDialog *tedit;

#endif // TEXTEDITDIALOG_H
