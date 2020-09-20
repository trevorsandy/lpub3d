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

#include <QProcess>

namespace Ui {
class RenderDialog;
}

class QProcess;
class RenderProcess : public QProcess
{
    Q_OBJECT

public:
    explicit RenderProcess(
            QObject *parent = nullptr) :
        QProcess(parent){}
    ~RenderProcess();
};

class RenderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenderDialog(
            QWidget* Parent = nullptr,
            int renderType = 0,
            int importOnly = 0);
    ~RenderDialog();

private slots:
    void reject();
    void on_RenderSettingsButton_clicked();
    void on_RenderButton_clicked();
    void on_OutputBrowseButton_clicked();
    void on_StandardOutButton_clicked();
    void resetOutputEdit(bool);
    void Update();

protected slots:
    QString ReadStdErr(bool &hasError) const;
    void ReadStdOut();
    void WriteStdOut();
    void UpdateElapsedTime();

protected:
    void resizeEvent(QResizeEvent* event);
    QString GetOutputFileName() const;
    QString GetPOVFileName() const;
    QString GetLogFileName(bool = true) const;
    void CloseProcess();
    bool PromptCancel();
    void ShowResult();
#ifdef Q_OS_WIN
    int TerminateChildProcess(const qint64 pid, const qint64 ppid);
#endif

#ifndef QT_NO_PROCESS
//    QProcess* mProcess;
    RenderProcess* mProcess;
#endif
    void* mOutputBuffer;
    QTimer mUpdateTimer;
    QTime mRenderTime;
    QFile mOutputFile;
    QImage mImage;
    QString mViewerStepKey;
    QString mModelFile;

    QStringList mCsiKeyList;
    QStringList mStdOutList;

    bool mTransBackground;
    int mWidth;
    int mHeight;
    int mResolution;
    int mQuality;
    double mScale;

    int mRenderType;
    int mImportOnly;
    int mPreviewWidth;
    int mPreviewHeight;
    int mBlendProgValue;
    int mBlendProgMax;

    Ui::RenderDialog* ui;
};

#endif // RENDERDIALOG_H_
