#pragma once

#include "lc_view.h"
/*** LPub3D Mod - preview widget for LPub3D ***/
#include "lc_application.h"
/*** LPub3D Mod end ***/

class lcPreview;

class lcPreviewDockWidget : public QMainWindow
{
	Q_OBJECT

public:
	explicit lcPreviewDockWidget(QMainWindow* Parent = nullptr);

	bool SetCurrentPiece(const QString& PartType, int ColorCode);
	void ClearPreview();
	void UpdatePreview();

protected slots:
	void SetPreviewLock();

protected:
	QAction* mLockAction;
	QToolBar* mToolBar;
	QLabel* mLabel;
	lcPreview* mPreview;
	lcViewWidget* mViewWidget;
};

class lcPreview : public lcView
{
public:
/*** LPub3D Mod - preview widget for LPub3D ***/
	lcPreview(bool SubstituteView = false);
/*** LPub3D Mod end ***/

	QString GetDescription() const
	{
		return mDescription;
	}

	bool IsModel() const
	{
		return mIsModel;
	}

	void ClearPreview();
	void UpdatePreview();
	bool SetCurrentPiece(const QString& PartType, int ColorCode);

protected:
	std::unique_ptr<Project> mLoader;

	QString mDescription;
	bool mIsModel = false;
};
