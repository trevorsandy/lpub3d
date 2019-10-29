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
#include <QTextCodec>
#include <QDebug>

#include "texteditdialog.h"
#include "ui_texteditdialog.h"
#include "name.h"

TextEditDialog::TextEditDialog(QString &goods, QString &editFont, QString &editFontColor, QString windowTitle, bool _richText, bool fontActions, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextEditDialog)
{
    ui->setupUi(this);

    setWindowTitle(windowTitle);

    rendererArgs = windowTitle.contains("Renderer Arguments");
    if (rendererArgs)
        ui->statusBar->showMessage("Enter space delimited renderer arguments.");

    text      = goods;
    richText  = _richText;

    QFont textFont("Arial");
    textFont.setStyleHint(QFont::SansSerif);
    if (editFont.isEmpty())
        font = textFont;
    else
        font.fromString(editFont);
    fontColor.setNamedColor(editFontColor.isEmpty() ? "Black" : editFontColor);

    // Suppress font actions - used for adding renderer arguments
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
    // Include rich text actions
    {
        ui->textEdit->setFont(font);
        ui->actionRichText->setChecked(richText);

        connect(ui->textEdit, &QTextEdit::currentCharFormatChanged,
                this, &TextEditDialog::currentCharFormatChanged);

        QByteArray data = text.toUtf8();
        QTextCodec *codec = Qt::codecForHtml(data);
        QString content = codec->toUnicode(data);
        if (richText && Qt::mightBeRichText(content)) {
            ui->textEdit->setHtml(content);
        } else {
            if (fontColor.isValid())
                ui->textEdit->setTextColor(fontColor);
            content = QString::fromLocal8Bit(data);
            ui->textEdit->setPlainText(content);
        }

        fontChanged(ui->textEdit->font());
    }

    setModal(true);
    setMinimumSize(40,20);
}

TextEditDialog::~TextEditDialog()
{
    delete ui;
}

void TextEditDialog::on_actionRichText_triggered()
{
    richText = ui->actionRichText->isChecked();
    setWindowTitle(QString("Edit %1 Text").arg(richText ? "Rich" : "Plain"));
    fontChanged(ui->textEdit->font());
}

void TextEditDialog::currentCharFormatChanged(const QTextCharFormat &format)
{
    if (rendererArgs)
        return;
    fontChanged(format.font());
}

void TextEditDialog::fontChanged(const QFont &f)
{
    ui->actionBold->setChecked(f.bold());
    if (richText){
        ui->actionItalic->setChecked(f.italic());
        ui->actionUnderline->setChecked(f.underline());
    } else {
        ui->actionItalic->setVisible(false);
        ui->actionUnderline->setVisible(false);
    }

    QString fontProperties = QString("Font: %1, Size: %2")
            .arg(QString("%1%2").arg(QFontInfo(f).family())
                 .arg(f.bold() ? " Bold" : f.italic() ? " Italic" : f.underline() ? " Underline" : ""))
            .arg(QString::number(f.pointSize()));
    ui->statusBar->showMessage(fontProperties);
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

void TextEditDialog::on_actionFont_triggered()
{
    bool fontSelected;
    font = QFontDialog::getFont(&fontSelected, font);
    if (fontSelected)
        ui->textEdit->setFont(font);
}

void TextEditDialog::on_actionFontColor_triggered()
{
    fontColor = ui->textEdit->textColor();
    fontColor = QColorDialog::getColor(fontColor,nullptr);
    if (fontColor.isValid()) {
        ui->textEdit->setTextColor(fontColor);
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

// get text - simple
bool TextEditDialog::getText(
      QString &goods,
      QString  windowTitle)
{
    QString empty = QString();
    TextEditDialog *dialog = new TextEditDialog(goods,empty,empty,windowTitle);

    bool ok = dialog->exec() == QDialog::Accepted;
    if (ok) {
        goods = dialog->text;
    }
    return ok;
}

// get text - rich text
bool TextEditDialog::getText(
      QString &goods,
      QString &editFont,
      QString &editFontColor,
      bool    &richText,
      QString  windowTitle,
      bool     fontActions)
{
    QString unformattedGoods = goods;
    if (!goods.isEmpty()) {
        QStringList list;
        foreach (QString string, QStringList(goods.split("\\n"))){
            string = string.trimmed();
            QRegExp rx2("\"");
            int pos = 0;
            QChar esc('\\');
            while ((pos = rx2.indexIn(string, pos)) != -1) {
                if (pos < string.size()) {
                    QChar ch = string.at(pos-1);
                    if (ch == esc) {
                        string.remove(pos-1,1);
                        pos += rx2.matchedLength() - 1;
                    }
                }
            }
            // if last character is \, append space ' ' so not to escape closing string double quote
            if (string.at(string.size()-1) == QChar('\\'))
                string.append(QChar(' '));
            list << string;
        }
        unformattedGoods = list.join(" ");
    }

    TextEditDialog *dialog = new TextEditDialog(unformattedGoods,editFont,editFontColor,windowTitle,richText,fontActions);

    bool ok = dialog->exec() == QDialog::Accepted;
    if (ok) {
        goods    = dialog->text;
        richText = dialog->richText;
    }
    if (!richText){
        editFont      = dialog->font.toString();
        editFontColor = dialog->fontColor.name();
    }
    return ok;
}


