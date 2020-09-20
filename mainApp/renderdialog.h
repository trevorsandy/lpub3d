/****************************************************************************
**
** Copyright (C) 2019 - 2020 Trevor SANDY. All rights reserved.
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

#ifndef RENDERDIALOG_H
#define RENDERDIALOG_H

#include <QDialog>
#include <QFile>
#include <QTimer>
#include <QTime>

namespace Ui {
class RenderDialog;
}

class QProcess;
class RenderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenderDialog(QWidget* Parent = nullptr, int type = 0);
    ~RenderDialog();

public slots:
    void reject();
    void on_RenderSettingsButton_clicked();
    void on_RenderButton_clicked();
    void on_OutputBrowseButton_clicked();
    void on_OutputResetButton_clicked();
    void Update();

protected slots:
    QString ReadStdErrLog(bool &hasError) const;
    void UpdateElapsedTime();

protected:
    void resizeEvent(QResizeEvent* event);
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
    QString mViewerStepKey;
    QString mModelFile;

    QStringList mCsiKeyList;

    bool mTransBackground;
    int mWidth;
    int mHeight;
    int mResolution;
    int mQuality;
    double mScale;

    int mRenderType;
    int mPreviewWidth;
    int mPreviewHeight;

    Ui::RenderDialog* ui;
};

#endif // RENDERDIALOG_H_
