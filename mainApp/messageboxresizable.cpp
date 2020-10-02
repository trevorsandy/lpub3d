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

#include <messageboxresizable.h>

#include <name.h>

#include <QEvent>
#include <QVBoxLayout>
#include <QStyleOptionButton>
#include <QApplication>
#include <QLabel>
#include <QMenu>
#include <QContextMenuEvent>

QMessageBoxResizable::QMessageBoxResizable(QWidget *parent)
    : QMessageBox(parent),
      detailsTextBrowser(nullptr),
      detailsPushButton(nullptr),
      autoAddOkButton(true)
{
    setMouseTracking(true);
    setSizeGripEnabled(true);
}

void QMessageBoxResizable::showDetails(bool clicked){
    Q_UNUSED(clicked);
    if (detailsPushButton && detailsTextBrowser) {
        detailsPushButton->setLabel(detailsTextBrowser->isHidden() ? HideLabel : ShowLabel);
        detailsTextBrowser->setHidden(!detailsTextBrowser->isHidden());
        this->adjustSize();
    }
}

void QMessageBoxResizable::setDetailedText(const QString &text, bool html){

    if (text.isEmpty()) {
        if (detailsTextBrowser) {
            detailsTextBrowser->hide();
            detailsTextBrowser->deleteLater();
        }
        detailsTextBrowser = 0;
        removeButton(detailsPushButton);
        if (detailsPushButton) {
            detailsPushButton->hide();
            detailsPushButton->deleteLater();
        }
        detailsPushButton = 0;
    } else {
        if (!detailsTextBrowser) {
            detailsTextBrowser = new QMessageBoxDetailsText(this);
            detailsTextBrowser->hide();
        }
        if (!detailsPushButton) {
            const bool _autoAddOkButton = autoAddOkButton; // QTBUG-39334, addButton() clears the flag.
            detailsPushButton = new DetailPushButton(this);
            addButton(detailsPushButton, QMessageBox::ActionRole);
            autoAddOkButton = _autoAddOkButton;

            detailsPushButton->disconnect(); // disconnect all actions to disable automatic close, re-add clicked action
            connect(detailsPushButton, &DetailPushButton::clicked, this, [=]()
            {
                detailsPushButton->setLabel(detailsTextBrowser->isHidden() ? ShowLabel : HideLabel);
            });
            connect(detailsPushButton,SIGNAL(clicked(bool)),
                    this,SLOT(showDetails(bool)));
        }
        if (html)
            detailsTextBrowser->setHtml(text);
        else
            detailsTextBrowser->setText(text);
    }

    QGridLayout *grid = qobject_cast<QGridLayout *>( this->layout() );
    if (grid) {
        if (detailsTextBrowser)
            grid->addWidget(detailsTextBrowser, grid->rowCount(), 0, 1, grid->columnCount());
        grid->setSizeConstraint(QLayout::SetNoConstraint);
        this->setLayout(grid);
    }

    this->adjustSize();
}

bool QMessageBoxResizable::event(QEvent *e){
    bool res = QMessageBox::event(e);
    switch (e->type()) {
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        if (QWidget *textBrowser = findChild<QTextBrowser *>()) {
            textBrowser->setMaximumHeight(QWIDGETSIZE_MAX);
        }
    default:
        break;
    }
    return res;
}

void QMessageBoxDetailsText::TextBrowser::contextMenuEvent(QContextMenuEvent * e)
{
    QMenu *menu = createStandardContextMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->popup(e->globalPos());
}

QMessageBoxDetailsText::QMessageBoxDetailsText(QWidget *parent)
    : QWidget(parent)
    , copyAvailable(false)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);
    textBrowser = new TextBrowser();
    textBrowser->setFixedHeight(100);
    textBrowser->setFocusPolicy(Qt::NoFocus);
    textBrowser->setReadOnly(true);

    textBrowser->setWordWrapMode(QTextOption::WordWrap);
    textBrowser->setLineWrapMode(QTextEdit::FixedColumnWidth);
    textBrowser->setLineWrapColumnOrWidth(LINE_WRAP_WIDTH);
    textBrowser->setOpenExternalLinks(true);

    layout->addWidget(textBrowser);
    setLayout(layout);
    connect(textBrowser, SIGNAL(copyAvailable(bool)),
            this, SLOT(textCopyAvailable(bool)));
}

QSize DetailPushButton::sizeHint() const {
    ensurePolished();
    QStyleOptionButton opt;
    initStyleOption(&opt);
    const QFontMetrics fm = fontMetrics();
    opt.text = label(ShowLabel);
    QSize sz = fm.size(Qt::TextShowMnemonic, opt.text);
    QSize ret = style()->sizeFromContents(QStyle::CT_PushButton, &opt, sz, this).
                  expandedTo(QApplication::globalStrut());
    opt.text = label(HideLabel);
    sz = fm.size(Qt::TextShowMnemonic, opt.text);
    ret = ret.expandedTo(style()->sizeFromContents(QStyle::CT_PushButton, &opt, sz, this).
                  expandedTo(QApplication::globalStrut()));
    return ret;
}
