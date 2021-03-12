#pragma once

/*** LPub3D Mod - libLC conversion ***/
#include <QMainWindow>
/*** LPub3D Mod end ***/
#include "lc_array.h"
/*** LPub3D Mod - Options moved to application.h ***/
//#include "lc_math.h"
#include "application.h"
/*** LPub3D Mod end ***/

class Project;
class lcPiecesLibrary;
enum class lcViewSphereLocation;

enum class lcShadingMode
{
	Wireframe,
	Flat,
	DefaultLights,
	Full
};

/*** LPub3D Mod - preview widget for LPub3D ***/
enum class lcPreviewLocation
{
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight
};

enum class lcPreviewPosition
{
	Dockable,
	Floating,
	Viewport     // not implemented
};
/*** LPub3D Mod end ***/

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
	bool mBackgroundGradient;
	quint32 mBackgroundSolidColor;
	quint32 mBackgroundGradientColorTop;
	quint32 mBackgroundGradientColorBottom;
	bool mDrawAxes;
	quint32 mAxesColor;
	quint32 mTextColor;
	quint32 mMarqueeBorderColor;
	quint32 mMarqueeFillColor;
	quint32 mOverlayColor;
	quint32 mActiveViewColor;
	quint32 mInactiveViewColor;
	bool mDrawEdgeLines;
	float mLineWidth;
	bool mAllowLOD;
	float mMeshLODDistance;
	bool mFadeSteps;
	quint32 mFadeStepsColor;
	bool mHighlightNewParts;
	quint32 mHighlightNewPartsColor;
	bool mGridEnabled = true;
	bool mDrawGridStuds;
	quint32 mGridStudColor;
	bool mDrawGridLines;
	int mGridLineSpacing;
	quint32 mGridLineColor;
	bool mDrawGridOrigin;
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

	int mPreviewViewSphereEnabled;
	int mPreviewViewSphereSize;
	lcViewSphereLocation mPreviewViewSphereLocation;
	int mDrawPreviewAxis;

	quint32 mStudCylinderColor;
	quint32 mPartEdgeColor;
	quint32 mBlackEdgeColor;
	quint32 mDarkEdgeColor;
	float mPartEdgeContrast;
	float mPartColorValueLDIndex;
	bool  mAutomateEdgeColor;

/*** LPub3D Mod - preview widget for LPub3D ***/
	lcPreviewLocation mPreviewLocation;
	lcPreviewPosition mPreviewPosition;
	int mPreviewSize;
	int mPreviewEnabled;
	QString mPreviewLoadPath;
/*** LPub3D Mod end ***/

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
/*** LPub3D Mod - Selected Parts ***/
	bool mBuildMofificationEnabled;
/*** LPub3D Mod end ***/
};

/*** LPub3D Mod - Options moved to application.h ***/
/***
struct lcCommandLineOptions
{
	bool ParseOK;
	bool Exit;
	bool SaveImage;
	bool SaveWavefront;
	bool Save3DS;
	bool SaveCOLLADA;
	bool SaveHTML;
	bool SetCameraAngles;
	bool SetCameraPosition;
	bool Orthographic;
	bool SetFoV;
	bool SetZPlanes;
	bool SetFadeStepsColor;
	bool SetHighlightColor;
	bool FadeSteps;
	bool ImageHighlight;
	bool AutomateEdgeColor;
	int ImageWidth;
	int ImageHeight;
	int AASamples;
	lcStudStyle StudStyle;
	lcStep ImageStart;
	lcStep ImageEnd;
	lcVector3 CameraPosition[3];
	lcVector2 CameraLatLon;
	float FoV;
	float PartEdgeContrast;
	float PartColorValueLDIndex;
	lcVector2 ZPlanes;
	lcViewpoint Viewpoint;
	quint32 StudCylinderColor;
	quint32 PartEdgeColor;
	quint32 BlackEdgeColor;
	quint32 DarkEdgeColor;
	quint32 FadeStepsColor;
	quint32	HighlightColor;
	QString ImageName;
	QString ModelName;
	QString CameraName;
	QString ProjectName;
	QString SaveWavefrontName;
	QString Save3DSName;
	QString SaveCOLLADAName;
	QString SaveHTMLName;
	QList<QPair<QString, bool>> LibraryPaths;
	QString StdOut;
	QString StdErr;
};
***/
/*** LPub3D Mod end ***/

enum class lcStartupMode
{
	ShowWindow,
	Success,
	Error
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
	lcCommandLineOptions ParseCommandLineOptions();
/*** LPub3D Mod - add parent, remove ShowWindow ***/
	lcStartupMode Initialize(const QList<QPair<QString, bool>>& LibraryPaths, QMainWindow *parent = 0);
/*** LPub3D Mod end ***/
/*** LPub3D Mod - process command line ***/
	int Process3DViewerCommandLine();
/*** LPub3D Mod end ***/
	void Shutdown();
	void ShowPreferencesDialog();
	void SaveTabLayout() const;

	bool LoadPartsLibrary(const QList<QPair<QString, bool>>& LibraryPaths, bool OnlyUsePaths);

	void SetClipboard(const QByteArray& Clipboard);
	void ExportClipboard(const QByteArray& Clipboard);

/*** LPub3D Mod - true fade ***/
	bool LPubFadeSteps();
	bool UseLPubFadeColour();
	QString LPubFadeColour();
/*** LPub3D Mod end ***/

	Project* mProject = nullptr;
	lcPiecesLibrary* mLibrary = nullptr;
	lcPreferences mPreferences;
	QByteArray mClipboard;

protected:
	bool InitializeRenderer();
	void ShutdownRenderer();
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
