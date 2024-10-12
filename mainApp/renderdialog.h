/****************************************************************************
**
** Copyright (C) 2019 - 2024 Trevor SANDY. All rights reserved.
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
#include <QElapsedTimer>

#include <QProcess>

namespace Ui {
class RenderDialog;
}

class QProcess;
class RenderProcess : public QProcess
{
    Q_OBJECT

public:
    explicit RenderProcess(QObject *parent = nullptr)
    : QProcess(parent)
    {
    }
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
    void on_InputBrowseButton_clicked();
    void on_OutputBrowseButton_clicked();
    void on_RenderOutputButton_clicked();
    void on_InputGenerateCheck_toggled();
    void resetEdit();
    void enableReset(QString const &);
    void validateInput();
    void Update();

protected slots:
    void ReadStdOut();
    void WriteStdOut();
    void UpdateElapsedTime();

protected:
    void resizeEvent(QResizeEvent* event);
    QString GetOutputFileName() const;
    QString GetPOVFileName() const;
    QString GetLogFileName(bool = true) const;
    QString ReadStdErr(bool &hasError) const;
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
    QAction   *resetInputAct;
    QAction   *resetOutputAct;

    void* mOutputBuffer;
    QTimer mUpdateTimer;
    QElapsedTimer mRenderTime;
    QFile mOutputFile;
    QImage mImage;
    QString mViewerStepKey;
    QString mModelFile;
    QString mImportModule;
    QString mMn;

    QStringList mCsiKeyList;
    QStringList mCsiKeyListData;
    QStringList mStdOutList;

    bool mHaveKeys;
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
