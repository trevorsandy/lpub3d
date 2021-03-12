#pragma once

#include "object.h"
#include "lc_array.h"
#include "lc_application.h"

#define LC_HTML_SINGLEPAGE    0x01
#define LC_HTML_INDEX         0x02
#define LC_HTML_LISTEND       0x08
#define LC_HTML_LISTSTEP      0x10
#define LC_HTML_SUBMODELS     0x40
#define LC_HTML_CURRENT_ONLY  0x80

class lcHTMLExportOptions
{
public:
	lcHTMLExportOptions(const Project* Project);
	void SaveDefaults();

	QString PathName;
	bool TransparentImages;
	bool SubModels;
	bool CurrentOnly;
	bool SinglePage;
	bool IndexPage;
	int StepImagesWidth;
	int StepImagesHeight;
	bool PartsListStep;
	bool PartsListEnd;
/*** LPub3D Mod - Highlight New Parts ***/
	bool HighlightNewParts;
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Fade Previous Steps ***/
	bool FadeSteps;
/*** LPub3D Mod end ***/
};

struct lcInstructionsPageLayout;

class Project
{
public:
	Project(bool IsPreview = false);
	~Project();

	Project(const Project&) = delete;
	Project(Project&&) = delete;
	Project& operator=(const Project&) = delete;
	Project& operator=(Project&&) = delete;

	const lcArray<lcModel*>& GetModels() const
	{
		return mModels;
	}

	lcModel* GetModel(const QString& FileName) const;

	lcModel* GetActiveModel() const
	{
		return mActiveModel;
	}

	int GetActiveModelIndex() const
	{
		return mModels.FindIndex(mActiveModel);
	}

	lcModel* GetMainModel() const
	{
		return !mModels.IsEmpty() ? mModels[0] : nullptr;
	}

	bool IsModified() const;
	void MarkAsModified();
	QString GetTitle() const;

	QString GetFileName() const
	{
		return mFileName;
	}
/*** LPub3D Mod - set Timeline top item ***/
	QString &GetTimelineTopItem()
	{
		return mTimelineTopItem;
	}
	void SetTimelineTopItem(
		const QString& TimelineTitle)
	{
		mTimelineTopItem  = TimelineTitle;
	}
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Camera Globe and Image Export ***/
	void SetRenderAttributes(
		const int Type,
		const int ImageWidth,
		const int ImageHeight,
		const int PageWidth,
		const int PageHeight,
		const int Renderer,
		const QString FileName,
		const float Resolution);
	void SetImageSize(
		const int ImageWidth,
		const int ImageHeight);
	int GetImageWidth() const;
	int GetImageHeight() const;
	int GetModelWidth() const;
	QString GetImageName() const
	{
		return mImageFileName;
	}
	int GetImageType() const
	{
		return mImageType;
	}
	int GetPageWidth() const
	{
		return mPageWidth;
	}
	int GetPageHeight() const
	{
		return mPageHeight;
	}
	float GetResolution() const
	{
		return mResolution;
	}
	bool GetViewerLoaded() const
	{
		return mViewerLoaded;
	}
	int GetRenderer()
	{
		return mRenderer;
	}
/*** LPub3D Mod end ***/
/*** LPub3D Mod - viewer interface ***/
	void AddModel(lcModel *Model)
	{
		mModels.Add(Model);
	}
	void DeleteAllModels()
	{
		mModels.DeleteAll();
	}
	void SetModified(bool value)
	{
		mModified = value;
	}
	void SetFileName(const QString& FileName); // moved from protected
/*** LPub3D Mod end ***/

	QString GetImageFileName(bool AllowCurrentFolder) const;

	std::vector<lcInstructionsPageLayout> GetPageLayouts() const;

	void SetActiveModel(int ModelIndex);
	void SetActiveModel(const QString& FileName);

	lcModel* CreateNewModel(bool ShowModel);
	QString GetNewModelName(QWidget* ParentWidget, const QString& DialogTitle, const QString& CurrentName, const QStringList& ExistingModels) const;
	void ShowModelListDialog();

/*** LPub3D Mod - preview widget ***/
	bool Load(const QString& LoadFileName, const QString& StepKey, int Type);
/*** LPub3D Mod end ***/
	bool Load(const QString& FileName);
	bool Save(const QString& FileName);
	bool Save(QTextStream& Stream);
	void Merge(Project* Other);
	bool ImportLDD(const QString& FileName);
	bool ImportInventory(const QByteArray& Inventory, const QString& Name, const QString& Description);

	void SaveImage();
	bool ExportModel(const QString& FileName, lcModel* Model) const;
	void Export3DStudio(const QString& FileName);
	void ExportBrickLink();
	void ExportCOLLADA(const QString& FileName);
	void ExportCSV();
	void ExportHTML(const lcHTMLExportOptions& Options);
	bool ExportPOVRay(const QString& FileName);
	void ExportWavefront(const QString& FileName);

	void UpdatePieceInfo(PieceInfo* Info) const;

protected:
	QString GetExportFileName(const QString& FileName, const QString& DefaultExtension, const QString& DialogTitle, const QString& DialogFilter) const;
	std::vector<lcModelPartsEntry> GetModelParts();
//	void SetFileName(const QString& FileName);   /*** LPub3D Mod - viewer interface (moved to public) ***/

	bool mIsPreview;
	bool mModified;
	QString mFileName;
	QFileSystemWatcher mFileWatcher;

	lcArray<lcModel*> mModels;
	lcModel* mActiveModel;

	Q_DECLARE_TR_FUNCTIONS(Project);
/*** LPub3D Mod - Camera Globe and Image Export ***/
	QString mImageFileName;
	int mPageWidth;
	int mPageHeight;
	int mImageType;
	int mRenderer;
	float mResolution;
	bool mViewerLoaded;
/*** LPub3D Mod end ***/
/*** LPub3D Mod - set Timeline top item ***/
	QString mTimelineTopItem;
/*** LPub3D Mod end ***/
};

inline lcModel* lcGetActiveModel()
{
	const Project* const Project = lcGetActiveProject();
	return Project ? Project->GetActiveModel() : nullptr;
}
