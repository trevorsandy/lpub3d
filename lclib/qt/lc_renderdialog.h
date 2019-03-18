#pragma once

#include <QDialog>

namespace Ui {
class lcRenderDialog;
}

class lcRenderDialog : public QDialog
{
	Q_OBJECT

public:
	explicit lcRenderDialog(QWidget* Parent);
	~lcRenderDialog();

public slots:
	void reject();
	void on_RenderButton_clicked();
	void on_OutputBrowseButton_clicked();
/*** LPub3D Mod - LDV Settings ***/
    void on_LdvLDrawPreferencesButton_clicked();
    void on_LdvExportSettingsButton_clicked();
/*** LPub3D Mod end ***/
	void Update();

protected slots:
	void ReadStdErr();

protected:
	QString GetOutputFileName() const;
	QString GetPOVFileName() const;
	void CloseProcess();
	bool PromptCancel();
	void ShowResult();

#ifndef QT_NO_PROCESS
	QProcess* mProcess;
#endif
	QTimer mUpdateTimer;
	QFile mOutputFile;
	void* mOutputBuffer;
	QImage mImage;
	QStringList mStdErrList;
/*** LPub3D Mod - Vars ***/
    QStringList mCsiKeyList;
    QString mViewerCsiKey;
    QString mModelFile;
    QTime mRenderTime;
/*** LPub3D Mod end ***/

	Ui::lcRenderDialog* ui;
};
