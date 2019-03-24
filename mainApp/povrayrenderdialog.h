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

#ifndef POVRAYRENDERDIALOG_H
#define POVRAYRENDERDIALOG_H

#include <QDialog>
#include <QFile>
#include <QTimer>
#include <QTime>

namespace Ui {
class PovrayRenderDialog;
}

class QProcess;
class PovrayRenderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PovrayRenderDialog(QWidget* Parent);
    ~PovrayRenderDialog();

public slots:
    void reject();
    void on_RenderButton_clicked();
    void on_OutputBrowseButton_clicked();
    void on_LdvLDrawPreferencesButton_clicked();
    void on_LdvExportSettingsButton_clicked();
    void Update();

protected slots:
    QString ReadStdErrLog(bool &hasError) const;
    void UpdateElapsedTime();

protected:
    QString GetOutputFileName() const;
    QString GetPOVFileName() const;
    QString GetLogFileName() const;
    void CloseProcess();
    bool PromptCancel();
    void ShowResult();

#ifndef QT_NO_PROCESS
    QProcess* mProcess;
#endif
    void* mOutputBuffer;
    QTimer mUpdateTimer;
    QTime mRenderTime;
    QFile mOutputFile;
    QImage mImage;
    QStringList mCsiKeyList;
    QString mViewerCsiKey;
    QString mModelFile;
    
    Ui::PovrayRenderDialog* ui;
};

#endif // POVRAYRENDERDIALOG_H_
