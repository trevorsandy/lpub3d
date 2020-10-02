/****************************************************************************
**
** Copyright (C) 2018 - 2020 Trevor SANDY. All rights reserved.
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

#ifndef MESSAGEBOXRESIZABLE_H
#define MESSAGEBOXRESIZABLE_H

// Resizeable QMessageBox Hack
#include <QWidget>
#include <QMessageBox>
#include <QTextBrowser>
#include <QPushButton>
#include <QAbstractButton>

enum DetailButtonLabel { ShowLabel = 0, HideLabel = 1 };

class QMessageBoxDetailsText : public QWidget
{
    Q_OBJECT
public:
    class TextBrowser : public QTextBrowser
    {
    public:
        TextBrowser(QWidget *parent=0) : QTextBrowser(parent) { }
        void contextMenuEvent(QContextMenuEvent * e) override;
    };
    QMessageBoxDetailsText(QWidget *parent=0);
    void setHtml(const QString &text) { textBrowser->setHtml(text); }
    void setText(const QString &text) { textBrowser->setPlainText(text); }
    QString text() const { return textBrowser->toPlainText(); }
    bool copy()
    {
#ifdef QT_NO_CLIPBOARD
        return false;
#else
        if (!copyAvailable)
            return false;
        textBrowser->copy();
        return true;
#endif // QT_NO_CLIPBOARD
    }
    void selectAll()
    {
        textBrowser->selectAll();
    }
private slots:
    void textCopyAvailable(bool available)
    {
        copyAvailable = available;
    }
private:
    bool copyAvailable;
    TextBrowser *textBrowser;
};

class DetailPushButton : public QPushButton
{
public:
    DetailPushButton(QWidget *parent) : QPushButton(label(ShowLabel), parent)
    {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    QString label(DetailButtonLabel label) const
    {
        return label == ShowLabel ? QMessageBox::tr("Show Details...") : QMessageBox::tr("Hide Details...");
    }

    void setLabel(DetailButtonLabel lbl)
    {
        setText(label(lbl));
    }

    QSize sizeHint() const override;
};

class QMessageBoxResizable: public QMessageBox
{
    Q_OBJECT

public:
    QMessageBoxResizable(QWidget *parent=nullptr);
    ~QMessageBoxResizable(){}
    void setDetailedText(const QString &text, bool html = true);
private slots:
    void showDetails(bool clicked);
private:
    virtual bool event(QEvent *e);
    QMessageBoxDetailsText *detailsTextBrowser;
    DetailPushButton       *detailsPushButton;
    bool                    autoAddOkButton;
};

#endif // MESSAGEBOXRESIZABLE_H
