#pragma once

#include "lc_glwidget.h"
#include "lc_scene.h"
#include "lc_viewsphere.h"
#include "lc_commands.h"
/*** LPub3D Mod - preview widget for LPub3D ***/
#include "lc_application.h"
/*** LPub3D Mod end ***/

class lcQGLWidget;
class lcPreviewWidget;

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
	lcPreviewWidget* mPreview;
	lcQGLWidget* mViewWidget;
};

class lcPreviewWidget : public lcGLWidget
{
public:
/*** LPub3D Mod - preview widget for LPub3D ***/
	lcPreviewWidget(bool Substitute = false);
/*** LPub3D Mod end ***/
	~lcPreviewWidget();

	QString GetDescription() const
	{
		return mDescription;
	}

	void ClearPreview();
	void UpdatePreview();
	bool SetCurrentPiece(const QString& PartType, int ColorCode);
	void SetCamera(lcCamera* Camera);
	void SetDefaultCamera();
	void ZoomExtents();

	// exclusively called from viewSphere
	void SetViewpoint(const lcVector3& Position);
	void StartOrbitTracking();

	bool IsModel() const
	{
		return mIsModel;
	}
	
	void OnDraw() override;
	void OnLeftButtonDown() override;
	void OnLeftButtonUp() override;
	void OnLeftButtonDoubleClick() override;
	void OnMiddleButtonDown() override;
	void OnMiddleButtonUp() override;
	void OnRightButtonDown() override;
	void OnRightButtonUp() override;
	void OnMouseMove() override;
	void OnMouseWheel(float Direction) override;

protected:
	void DrawViewport();

	void StopTracking(bool Accept);
	void OnButtonDown(lcTrackButton TrackButton);

	Project* mLoader;
	lcViewSphere mViewSphere;

	QString mDescription;
/*** LPub3D Mod - preview widget for LPub3D ***/
	bool mIsSubstitute;
/*** LPub3D Mod end ***/
	bool mIsModel;
};
