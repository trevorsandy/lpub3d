#pragma once

/*** LPub3D Mod - libLC conversion ***/
#include <QMainWindow>
/*** LPub3D Mod end ***/
#include "lc_array.h"

class Project;
class lcPiecesLibrary;

enum class lcShadingMode
{
	Wireframe,
	Flat,
	DefaultLights,
	Full
};

enum class lcViewSphereLocation
{
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight
};

enum class lcColorTheme
{
	Dark,
	System
};

class lcPreferences
{
public:
	void LoadDefaults();
	void SaveDefaults();
	void SetInterfaceColors(lcColorTheme ColorTheme);

	int mMouseSensitivity;
	lcShadingMode mShadingMode;
	bool mDrawAxes;
	quint32 mAxesColor;
	quint32 mOverlayColor;
	quint32 mActiveViewColor;
	bool mDrawEdgeLines;
	float mLineWidth;
	bool mAllowLOD;
	float mMeshLODDistance;
	bool mFadeSteps;
	quint32 mFadeStepsColor;
	bool mHighlightNewParts;
	quint32 mHighlightNewPartsColor;
	bool mDrawGridStuds;
	quint32 mGridStudColor;
	bool mDrawGridLines;
	int mGridLineSpacing;
	quint32 mGridLineColor;
	bool mFixedAxes;
	bool mViewSphereEnabled;
	lcViewSphereLocation mViewSphereLocation;
	int mViewSphereSize;
	quint32 mViewSphereColor;
	quint32 mViewSphereTextColor;
	quint32 mViewSphereHighlightColor;
	bool mAutoLoadMostRecent;
	bool mRestoreTabLayout;
	lcColorTheme mColorTheme;
/*** LPub3D Mod - Update Default Camera ***/
	bool mDefaultCameraProperties;
	float mDDF;
	float mCDP;
	float mCFoV;
	float mCNear;
	float mCFar;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Native Renderer settings ***/
	int mNativeViewpoint;
	int mNativeProjection;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Timeline part icons ***/
	bool mViewPieceIcons;
/*** LPub3D Mod end ***/
/*** LPub3D Mod - true fade ***/
	bool mLPubTrueFade;
	bool mConditionalLines;
/*** LPub3D Mod end ***/
/*** LPub3D Mod - add background colour to preferences ***/
	quint32 mBackgroundSolidColor;
/*** LPub3D Mod end ***/
/*** LPub3D Mod - preview widget ***/
	quint32 mActivePreviewColor;
	int mViewSpherePreviewSize;
	lcViewSphereLocation mViewSpherePreviewLocation;
/*** LPub3D Mod end ***/
};

/*** LPub3D Mod - change QApplication to QObject ***/
class lcApplication : public QObject
/*** LPub3D Mod end ***/
{
	Q_OBJECT

public:
/*** LPub3D Mod - initialization - move arguments to LP3D Application ***/
	lcApplication();
/*** LPub3D Mod end ***/
	~lcApplication();

	void SetProject(Project* Project);
/*** LPub3D Mod - add parent, remove ShowWindow ***/
	bool Initialize(QList<QPair<QString, bool>>& LibraryPaths, QMainWindow *parent = 0);
/*** LPub3D Mod end ***/
/*** LPub3D Mod - process command line ***/
	int Process3DViewerCommandLine();
/*** LPub3D Mod end ***/
	void Shutdown();
	void ShowPreferencesDialog();
	void SaveTabLayout() const;

	bool LoadPartsLibrary(const QList<QPair<QString, bool>>& LibraryPaths, bool OnlyUsePaths, bool ShowProgress);

	void SetClipboard(const QByteArray& Clipboard);
	void ExportClipboard(const QByteArray& Clipboard);

/*** LPub3D Mod - true fade ***/
	bool LPubFadeSteps();
	bool UseLPubFadeColour();
	QString LPubFadeColour();
/*** LPub3D Mod end ***/

	Project* mProject;
	lcPiecesLibrary* mLibrary;
	lcPreferences mPreferences;
	QByteArray mClipboard;

protected:
	void UpdateStyle();
	QString GetTabLayoutKey() const;

	QString mDefaultStyle;
};

extern lcApplication* gApplication;

inline lcPiecesLibrary* lcGetPiecesLibrary()
{
	return gApplication->mLibrary;
}

inline Project* lcGetActiveProject()
{
	return gApplication->mProject;
}

inline lcPreferences& lcGetPreferences()
{
	return gApplication->mPreferences;
}

