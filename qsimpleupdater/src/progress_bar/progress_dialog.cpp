/*
 * (C) Copyright 2014 Alex Spataru
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 */

#include <progress_dialog.h>
#include <ui_progress_dialog.h>
#include "declarations.h"

ProgressDialog::ProgressDialog (QWidget *parent) : QDialog (parent), ui (new Ui::ProgressDialog)
{
    // Create and configure UI
    ui->setupUi (this);

    // Make the window look like a dialog
    QIcon _blank;
    setWindowIcon (_blank);
    setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    autoHide = true;
    pageDirection = DIRECTION_NOT_SET;

    // Close dialog when cancel button is clicked
    connect (ui->progressDlgCancelBtn, SIGNAL (clicked()), this, SLOT (cancel()));
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::setBtnToCancel()
{
    ui->progressDlgCancelBtn->setText(QApplication::translate("ProgressDialog", "Cancel", nullptr));
    autoHide = false;
}

void ProgressDialog::setBtnToClose()
{
    ui->progressDlgCancelBtn->setText(QApplication::translate("ProgressDialog", "Close", nullptr));
    autoHide = true;
}

void ProgressDialog::setPageDirection(int d)
{
    pageDirection = d;
}

void ProgressDialog::setAutoHide(bool b)
{
    autoHide = b;
}

void ProgressDialog::setLabelText(QString text, bool alert) {
    ui->progressDlgMessageLbl->setStyleSheet(alert ? QLatin1String("QLabel { color : rgb(255, 51, 0); }") : styleSheet());
    ui->progressDlgMessageLbl->setText(text);
    repaint();
}

void ProgressDialog::setRange(int min, int max)
{
    ui->progressDlgProgressBar->setMinimum(min);
    ui->progressDlgProgressBar->setMaximum(max);
    QApplication::processEvents();
}

void ProgressDialog::setValue(int value)
{
    ui->progressDlgProgressBar->setValue(value);
    QApplication::processEvents(/*QEventLoop::ExcludeUserInputEvents*/);
}

void ProgressDialog::cancel (void)
{
    if (autoHide) {
        hide();
        ui->progressDlgProgressBar->reset();
    }

    if (pageDirection == PAGE_NEXT) {
        emit cancelNextPageContinuous();
    } else if (pageDirection == PAGE_PREVIOUS) {
        emit cancelPreviousPageContinuous();
    }

    emit cancelClicked();
}

void ProgressDialog::setDownloadInfo () {
    setWindowTitle(tr("Library Update"));
    setLabelText(tr("Retrieving download..."));
}

void ProgressDialog::setWindowFlags(Qt::WindowFlags type) {
     QWidget::setWindowFlags(type);
}
