/****************************************************************************
**
** Copyright (C) 2019 Trevor SANDY. All rights reserved.
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
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QFontDialog>
#include <QColorDialog>

#include "texteditdialog.h"
#include "ui_texteditdialog.h"

TextEditDialog::TextEditDialog(QString &goods, QString windowTitle, bool richText, bool fontActions, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextEditDialog)
{
    ui->setupUi(this);

    setWindowTitle(windowTitle);

    text = goods;

    if (windowTitle.contains("Renderer Arguments"))
        ui->statusBar->showMessage("Enter space delimited renderer arguments.");

// Suppress font actions
    if (!fontActions && !richText)
    {
        ui->actionRichText->setVisible(false);
        ui->actionFontColor->setVisible(false);
        ui->actionFont->setVisible(false);
        ui->actionBold->setVisible(false);
        ui->actionItalic->setVisible(false);
        ui->actionUnderline->setVisible(false);
        ui->textEdit->setPlainText(text);
    }
    else
    {
        QString fontString = "Arial,24,-1,255,75,0,0,0,0,0";
        QFont font;
        font.fromString(fontString);
        ui->textEdit->setFont(font);
        if (!text.isEmpty())
            ui->actionRichText->setChecked(richText);
        ui->textEdit->setHtml(text);

        ui->actionFontColor->setEnabled(richText);
        ui->actionBold->setEnabled(richText);
        ui->actionItalic->setEnabled(richText);
        ui->actionUnderline->setEnabled(richText);
    }

    setModal(true);
    setMinimumSize(40,20);
}

TextEditDialog::~TextEditDialog()
{
    delete ui;
}

bool TextEditDialog::getText(
      QString &goods,
      QString  windowTitle)
{
  TextEditDialog *dialog = new TextEditDialog(goods,windowTitle);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    goods = dialog->text;
  }
  return ok;
}

bool TextEditDialog::getText(
      QString &goods,
      bool    &richText,
      QString  windowTitle,
      bool     fontActions)
{
  TextEditDialog *dialog = new TextEditDialog(goods,windowTitle,richText,fontActions);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    goods = dialog->text;
    richText = dialog->richText;
  }
  return ok;
}

void TextEditDialog::on_actionAccept_triggered()
{
    if (ui->actionRichText->isChecked())
        text = ui->textEdit->toHtml();
    else
        text = ui->textEdit->toPlainText();
    QDialog::accept();
}

void TextEditDialog::on_actionCancel_triggered()
{
    QDialog::reject();
}

void TextEditDialog::on_actionNew_triggered()
{
    ui->textEdit->setText(QString());
}

void TextEditDialog::on_actionCopy_triggered()
{
    ui->textEdit->copy();
}

void TextEditDialog::on_actionCut_triggered()
{
    ui->textEdit->cut();
}

void TextEditDialog::on_actionPaste_triggered()
{
    ui->textEdit->paste();
}

void TextEditDialog::on_actionUndo_triggered()
{
     ui->textEdit->undo();
}

void TextEditDialog::on_actionRedo_triggered()
{
    ui->textEdit->redo();
}

void TextEditDialog::on_actionRichText_triggered()
{
    if (richText){
        ui->textEdit->setText(QString());
        QString fontString = "Arial,24,-1,255,75,0,0,0,0,0";
        QFont font;
        font.fromString(fontString);
        ui->textEdit->setFont(font);
    }
    ui->actionFontColor->setEnabled(richText);
    ui->actionBold->setEnabled(richText);
    ui->actionItalic->setEnabled(richText);
    ui->actionUnderline->setEnabled(richText);
}

void TextEditDialog::on_actionFont_triggered()
{
    bool fontSelected;
    QFont font = QFontDialog::getFont(&fontSelected, this);
    if (fontSelected)
        ui->textEdit->setFont(font);
}

void TextEditDialog::on_actionFontColor_triggered()
{
    QColor _color = ui->textEdit->textColor();
    _color = QColorDialog::getColor(_color,nullptr);
    if (_color.isValid()) {
        ui->textEdit->setTextColor(_color);
    }
}

void TextEditDialog::on_actionUnderline_triggered()
{
    ui->textEdit->setFontUnderline(ui->actionUnderline->isChecked());
}

void TextEditDialog::on_actionItalic_triggered()
{
    ui->textEdit->setFontItalic(ui->actionItalic->isChecked());
}

void TextEditDialog::on_actionBold_triggered()
{
    ui->actionBold->isChecked() ? ui->textEdit->setFontWeight(QFont::Bold) :
                                  ui->textEdit->setFontWeight(QFont::Normal);
}



