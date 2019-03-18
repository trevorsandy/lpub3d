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

ProgressDialog::ProgressDialog (QWidget *parent) : QDialog (parent), ui (new Ui::ProgressDialog)
{
    // Create and configure UI
    ui->setupUi (this);

    // Make the window look like a dialog
    QIcon _blank;
    setWindowIcon (_blank);
    setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    autoHide = true;

    // Close dialog when cancel button is clicked
    connect (ui->progressDlgCancelBtn, SIGNAL (clicked()), this, SLOT (cancel()));
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::setBtnToClose()
{
    ui->progressDlgCancelBtn->setText(QApplication::translate("ProgressDialog", "Close", nullptr));
    autoHide = true;
}

void ProgressDialog::setAutoHide(bool b)
{
    autoHide = b;
}

void ProgressDialog::progressBarSetLabelText(QString text) {
    ui->progressDlgMessageLbl->setText(text);
}

void ProgressDialog::progressBarSetRange(int min, int max)
{
    ui->progressDlgProgressBar->setMinimum(min);
    ui->progressDlgProgressBar->setMaximum(max);
    QApplication::processEvents();
}

void ProgressDialog::progressBarSetValue(int value)
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
    emit cancelClicked();
}

void ProgressDialog::setDownloadInfo () {
    setWindowTitle(tr("Library Update"));
    ui->progressDlgMessageLbl->setText(tr("Retrieving download..."));
}

void ProgressDialog::setWindowFlags(Qt::WindowFlags type) {
     QWidget::setWindowFlags(type);
}
