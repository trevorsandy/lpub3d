#pragma once

/*** LPub3D Mod - libLC conversion ***/
#include <QMainWindow>
/*** LPub3D Mod end ***/
#include "lc_array.h"

class Project;
class lcPiecesLibrary;

enum lcShadingMode
{
	LC_SHADING_WIREFRAME,
	LC_SHADING_FLAT,
	LC_SHADING_DEFAULT_LIGHTS,
	LC_SHADING_FULL,
	LC_NUM_SHADING_MODES
};

enum class lcViewCubeLocation
{
	DISABLED,
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT
};

class lcPreferences
{
public:
	void LoadDefaults();
	void SaveDefaults();

	int mMouseSensitivity;
	lcShadingMode mShadingMode;
	bool mDrawAxes;
	bool mDrawEdgeLines;
	float mLineWidth;
	bool mDrawGridStuds;
	quint32 mGridStudColor;
	bool mDrawGridLines;
	int mGridLineSpacing;
	quint32 mGridLineColor;
	bool mFixedAxes;
	lcViewCubeLocation mViewCubeLocation;
	int mViewCubeSize;

/*** LPub3D Mod - Native Renderer settings ***/
    int mNativeViewpoint;
    int mNativeOrthographic;
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

	Project* mProject;
	lcPiecesLibrary* mLibrary;
	lcPreferences mPreferences;
	QByteArray mClipboard;
/*** LPub3D Mod - process fade parts ***/
	bool mFadeParts;
/*** LPub3D Mod end ***/

protected:
	QString GetTabLayoutKey() const;
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

