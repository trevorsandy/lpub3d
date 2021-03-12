#pragma once

#include "lc_math.h"
#include "object.h"
#include "lc_commands.h"

#define LC_SEL_NO_PIECES                0x0001 // No pieces in model
#define LC_SEL_PIECE                    0x0002 // At last 1 piece selected
#define LC_SEL_SELECTED                 0x0004 // At last 1 object selected
#define LC_SEL_UNSELECTED               0x0008 // At least 1 piece unselected
#define LC_SEL_HIDDEN                   0x0010 // At least one piece hidden
#define LC_SEL_HIDDEN_SELECTED          0x0020 // At least one piece selected is hidden
#define LC_SEL_VISIBLE_SELECTED         0x0040 // At least one piece selected is not hidden
#define LC_SEL_GROUPED                  0x0080 // At least one piece selected is grouped
#define LC_SEL_FOCUS_GROUPED            0x0100 // Focused piece is grouped
#define LC_SEL_CAN_GROUP                0x0200 // Can make a new group
#define LC_SEL_MODEL_SELECTED           0x0400 // At least one model reference is selected
#define LC_SEL_CAN_ADD_CONTROL_POINT    0x0800 // Can add control points to focused piece
#define LC_SEL_CAN_REMOVE_CONTROL_POINT 0x1000 // Can remove control points from focused piece

class lcGLWidget;

/*** LPub3D Mod - native renderer options ***/
class NativeOptions;
/*** LPub3D Mod end ***/
/*** LPub3D Mod - enable lights ***/
class lcLightProps;
/*** LPub3D Mod end ***/

struct lcInstructionsPageLayout
{
	lcModel* Model;
	lcStep Step;
};

enum class lcSelectionMode
{
	Single,
	Piece,
	Color,
	PieceColor
};

enum class lcTransformType
{
	First,
	AbsoluteTranslation = First,
	RelativeTranslation,
	AbsoluteRotation,
	RelativeRotation,
	Count
};

class lcModelProperties
{
public:
	void LoadDefaults();
	void SaveDefaults();

	bool operator==(const lcModelProperties& Properties) const
	{
		if (mFileName != Properties.mFileName || mModelName != Properties.mModelName || mAuthor != Properties.mAuthor ||
			mDescription != Properties.mDescription || mComments != Properties.mComments)
			return false;

		if (mAmbientColor != Properties.mAmbientColor)
			return false;

/*** LPub3D Mod - preview widget for LPub3D ***/
		if (mUnoffPartColorCode != Properties.mUnoffPartColorCode)
			return false;
/*** LPub3D Mod end ***/

		return true;
	}

	void SaveLDraw(QTextStream& Stream) const;
	bool ParseLDrawHeader(QString Line, bool FirstLine);
	void ParseLDrawLine(QTextStream& Stream);

	QString mFileName;
	QString mDescription;
	QString mModelName;
	QString mAuthor;
	QString mComments;

	lcVector3 mAmbientColor;

/*** LPub3D Mod - preview widget for LPub3D ***/
	int mUnoffPartColorCode;
/*** LPub3D Mod end ***/
};

struct lcModelHistoryEntry
{
	QByteArray File;
	QString Description;
};

class lcModel
{
public:
	lcModel(const QString& FileName, Project* Project, bool Preview);
	~lcModel();

	lcModel(const lcModel&) = delete;
	lcModel& operator=(const lcModel&) = delete;

	Project* GetProject() const
	{
		return mProject;
	}

	bool IsModified() const
	{
		return mSavedHistory != mUndoHistory[0];
	}

	bool IsPreview()
	{
		return mIsPreview;
	}

	bool GetPieceWorldMatrix(lcPiece* Piece, lcMatrix44& ParentWorldMatrix) const;
	bool IncludesModel(const lcModel* Model) const;
	void CreatePieceInfo(Project* Project);
	void UpdatePieceInfo(std::vector<lcModel*>& UpdatedModels);
	void UpdateMesh();
	void UpdateAllViews() const;

	PieceInfo* GetPieceInfo() const
	{
		return mPieceInfo;
	}

	const lcArray<lcPiece*>& GetPieces() const
	{
		return mPieces;
	}

	const lcArray<lcCamera*>& GetCameras() const
	{
		return mCameras;
	}

	const lcArray<lcLight*>& GetLights() const
	{
		return mLights;
	}

	const lcArray<lcGroup*>& GetGroups() const
	{
		return mGroups;
	}

	const lcModelProperties& GetProperties() const
	{
		return mProperties;
	}

/*** LPub3D Mod - preview widget ***/
	void SetUnoffPartColorCode(int ColorCode)
	{
		mProperties.mUnoffPartColorCode = ColorCode;
	}
/*** LPub3D Mod end ***/

	void SetFileName(const QString& FileName)
	{
		if (mProperties.mModelName == mProperties.mFileName)
			mProperties.mModelName = FileName;
		mProperties.mFileName = FileName;
	}

	const QString& GetFileName() const
	{
		return mProperties.mFileName;
	}

/*** LPub3D Mod - Selected Parts ***/
	const QString& GetModelName() const
	{
		return mProperties.mModelName;
	}
/*** LPub3D Mod end ***/

	void SetDescription(const QString& Description)
	{
		mProperties.mDescription = Description;
	}

	const QStringList& GetFileLines() const
	{
		return mFileLines;
	}

	lcStep GetLastStep() const;

	lcStep GetCurrentStep() const
	{
		return mCurrentStep;
	}

	void SetActive(bool Active);
	void CalculateStep(lcStep Step);
	void SetCurrentStep(lcStep Step);
	void SetTemporaryStep(lcStep Step)
	{
		mCurrentStep = Step;
		CalculateStep(Step);
	}

	void ShowFirstStep();
	void ShowLastStep();
	void ShowPreviousStep();
	void ShowNextStep();
	void InsertStep(lcStep Step);
	void RemoveStep(lcStep Step);

	void AddPiece();
	void DeleteAllCameras();
	void DeleteSelectedObjects();
	void ResetSelectedPiecesPivotPoint();
	void RemoveSelectedPiecesKeyFrames();
	void InsertControlPoint();
	void RemoveFocusedControlPoint();
	void ShowSelectedPiecesEarlier();
	void ShowSelectedPiecesLater();
	void SetPieceSteps(const QList<QPair<lcPiece*, lcStep>>& PieceSteps);
	void RenamePiece(PieceInfo* Info);
/*** LPub3D Mod - Apply Viewpoint zoom extent ***/
	bool ApplyViewpointZoomExtent();
/*** LPub3D Mod end ***/

	void MoveSelectionToModel(lcModel* Model);
	void InlineSelectedModels();

	lcGroup* AddGroup(const QString& Prefix, lcGroup* Parent);
	lcGroup* GetGroup(const QString& Name, bool CreateIfMissing);
	void RemoveGroup(lcGroup* Group);
	void GroupSelection();
	void UngroupSelection();
	void AddSelectedPiecesToGroup();
	void RemoveFocusPieceFromGroup();
	void ShowEditGroupsDialog();

	void SaveLDraw(QTextStream& Stream, bool SelectedOnly) const;
	void LoadLDraw(QIODevice& Device, Project* Project);
	bool LoadBinary(lcFile* File);
	bool LoadLDD(const QString& FileData);
	bool LoadInventory(const QByteArray& Inventory);
	int SplitMPD(QIODevice& Device);
	void Merge(lcModel* Other);

	void SetSaved()
	{
		if (mUndoHistory.empty())
			SaveCheckpoint(QString());

		if (!mIsPreview)
			mSavedHistory = mUndoHistory[0];
	}

	void SetMinifig(const lcMinifig& Minifig);
	void SetPreviewPiece(lcPiece* Piece)
	{
		AddPiece(Piece);
	}

	void Cut();
	void Copy();
	void Paste();
	void DuplicateSelectedPieces();

	void GetScene(lcScene* Scene, lcCamera* ViewCamera, bool AllowHighlight, bool AllowFade) const;
/*** LPub3D Mod - true fade ***/	
	void AddSubModelRenderMeshes(lcScene* Scene, const lcMatrix44& WorldMatrix, int DefaultColorIndex, lcRenderMeshState RenderMeshState, bool ParentActive, bool LPubFade) const;
/*** LPub3D Mod end ***/
	QImage GetStepImage(bool Zoom, int Width, int Height, lcStep Step);
	QImage GetPartsListImage(int MaxWidth, lcStep Step) const;
	void SaveStepImages(const QString& BaseName, bool AddStepSuffix, bool Zoom, int Width, int Height, lcStep Start, lcStep End);
	std::vector<lcInstructionsPageLayout> GetPageLayouts(std::vector<const lcModel*>& AddedModels);

	void RayTest(lcObjectRayTest& ObjectRayTest) const;
	void BoxTest(lcObjectBoxTest& ObjectBoxTest) const;
	bool SubModelMinIntersectDist(const lcVector3& WorldStart, const lcVector3& WorldEnd, float& MinDistance) const;
	bool SubModelBoxTest(const lcVector4 Planes[6]) const;
	void SubModelCompareBoundingBox(const lcMatrix44& WorldMatrix, lcVector3& Min, lcVector3& Max) const;

	bool HasPieces() const
	{
		return !mPieces.IsEmpty();
	}

	bool AnyPiecesSelected() const;
	bool AnyObjectsSelected() const;
	lcModel* GetFirstSelectedSubmodel() const;
	void GetSubModels(lcArray<lcModel*>& SubModels) const;
	bool GetMoveRotateTransform(lcVector3& Center, lcMatrix33& RelativeRotation) const;
	bool GetPieceFocusOrSelectionCenter(lcVector3& Center) const;
	lcVector3 GetSelectionOrModelCenter() const;
	bool GetFocusPosition(lcVector3& Position) const;
	lcObject* GetFocusObject() const;
	bool GetSelectionCenter(lcVector3& Center) const;
	bool GetPiecesBoundingBox(lcVector3& Min, lcVector3& Max) const;
	void GetPartsList(int DefaultColorIndex, bool ScanSubModels, bool AddSubModels, lcPartsList& PartsList) const;
	void GetPartsListForStep(lcStep Step, int DefaultColorIndex, lcPartsList& PartsList) const;
	void GetModelParts(const lcMatrix44& WorldMatrix, int DefaultColorIndex, std::vector<lcModelPartsEntry>& ModelParts) const;
	void GetSelectionInformation(int* Flags, lcArray<lcObject*>& Selection, lcObject** Focus) const;
	lcArray<lcObject*> GetSelectionModePieces(lcPiece* SelectedPiece) const;

	void FocusOrDeselectObject(const lcObjectSection& ObjectSection);
	void ClearSelection(bool UpdateInterface);
	void ClearSelectionAndSetFocus(lcObject* Object, quint32 Section, bool EnableSelectionMode);
	void ClearSelectionAndSetFocus(const lcObjectSection& ObjectSection, bool EnableSelectionMode);
	void SetSelectionAndFocus(const lcArray<lcObject*>& Selection, lcObject* Focus, quint32 Section, bool EnableSelectionMode);
	void AddToSelection(const lcArray<lcObject*>& Objects, bool EnableSelectionMode, bool UpdateInterface);
	void RemoveFromSelection(const lcArray<lcObject*>& Objects);
	void RemoveFromSelection(const lcObjectSection& ObjectSection);
	void SelectAllPieces();
	void InvertSelection();

	void HideSelectedPieces();
	void HideUnselectedPieces();
	void UnhideSelectedPieces();
	void UnhideAllPieces();

	void FindPiece(bool FindFirst, bool SearchForward);

	void UndoAction();
	void RedoAction();

	lcVector3 SnapPosition(const lcVector3& Delta) const;
	lcVector3 SnapRotation(const lcVector3& Delta) const;
	lcMatrix33 GetRelativeRotation() const;

	const lcVector3& GetMouseToolDistance() const
	{
		return mMouseToolDistance;
	}

/*** LPub3D Mod - viewer interface ***/
	bool IsActive()
	{
		return mActive;
	}

	void AddCamera(lcCamera* camera)
	{
		mCameras.Add(camera);
	}

	void RemoveCameraIndex(int CameraIdx)
	{
		mCameras.RemoveIndex(CameraIdx);
	}

	void AddPiece(lcPiece* Piece); /*** LPub3D Mod - viewer interface (moved from protected) ***/
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Build Modification ***/
	bool GetModAction()
	{
		return mModAction;
	}
	void ResetModAction()
	{
		mModAction = false;
	}
/*** LPub3D Mod end ***/

	void BeginMouseTool();
	void EndMouseTool(lcTool Tool, bool Accept);
	void InsertPieceToolClicked(const lcMatrix44& WorldMatrix);
	void PointLightToolClicked(const lcVector3& Position);
/*** LPub3D Mod - enable lights ***/
	void BeginDirectionalLightTool(const lcVector3& Position, const lcVector3& Target, int LightType);
	void UpdateDirectionalLightTool(const lcVector3& Position);
/*** LPub3D Mod end ***/
	void BeginCameraTool(const lcVector3& Position, const lcVector3& Target);
	void UpdateCameraTool(const lcVector3& Position);
	void UpdateMoveTool(const lcVector3& Distance, bool AlternateButtonDrag);
	void UpdateRotateTool(const lcVector3& Angles, bool AlternateButtonDrag);
	void UpdateScaleTool(const float Scale);
	void EraserToolClicked(lcObject* Object);
	void PaintToolClicked(lcObject* Object);
	void ColorPickerToolClicked(lcObject* Object);
	void UpdateZoomTool(lcCamera* Camera, float Mouse);
	void UpdatePanTool(lcCamera* Camera, const lcVector3& Distance);
	void UpdateOrbitTool(lcCamera* Camera, float MouseX, float MouseY);
	void UpdateRollTool(lcCamera* Camera, float Mouse);
	void ZoomRegionToolClicked(lcCamera* Camera, float AspectRatio, const lcVector3& Position, const lcVector3& TargetPosition, const lcVector3* Corners);
	void LookAt(lcCamera* Camera);
	void MoveCamera(lcCamera* Camera, const lcVector3& Direction);
	void ZoomExtents(lcCamera* Camera, float Aspect);
	void Zoom(lcCamera* Camera, float Amount);

/*** LPub3D Mod - Camera Globe ***/
	void MoveDefaultCamera(lcCamera *Camera, const lcVector3& ObjectDistance);
	void SetCameraGlobe(lcCamera* Camera, float Latitude, float Longitude, float Distance);
	void UpdateDefaultCameraProperties(lcCamera* DefaultCamera);
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Selected Parts ***/
	void SetSelectedPieces(QVector<int> &LineTypeIndexes);
/*** LPub3D Mod end ***/
	void MoveSelectedObjects(const lcVector3& Distance, bool Relative, bool AlternateButtonDrag, bool Update, bool Checkpoint)
	{
		MoveSelectedObjects(Distance, Distance, Relative, AlternateButtonDrag, Update, Checkpoint);
	}

	void MoveSelectedObjects(const lcVector3& PieceDistance, const lcVector3& ObjectDistance, bool Relative, bool AlternateButtonDrag, bool Update, bool Checkpoint);
	void RotateSelectedPieces(const lcVector3& Angles, bool Relative, bool RotatePivotPoint, bool Update, bool Checkpoint);
	void ScaleSelectedPieces(const float Scale, bool Update, bool Checkpoint);
	void TransformSelectedObjects(lcTransformType TransformType, const lcVector3& Transform);
	void SetSelectedPiecesColorIndex(int ColorIndex);
	void SetSelectedPiecesPieceInfo(PieceInfo* Info);
	void SetSelectedPiecesStepShow(lcStep Step);
	void SetSelectedPiecesStepHide(lcStep Step);

	void SetCameraOrthographic(lcCamera* Camera, bool Ortho);
	void SetCameraFOV(lcCamera* Camera, float FOV);
	void SetCameraZNear(lcCamera* Camera, float ZNear);
	void SetCameraZFar(lcCamera* Camera, float ZFar);
	void SetCameraName(lcCamera* Camera, const QString& Name);
/*** LPub3D Mod - enable lights ***/
	void SetLightName(lcLight* Light, const QString& Name);
	void UpdateLight(lcLight* Light, const lcLightProps Props, int Property);
/*** LPub3D Mod end ***/

	void ShowPropertiesDialog();
	void ShowSelectByNameDialog();
	void ShowSelectByColorDialog();
	void ShowArrayDialog();
	void ShowMinifigDialog();
	void UpdateInterface();

protected:
	void DeleteModel();
	void DeleteHistory();
	void SaveCheckpoint(const QString& Description);
	void LoadCheckPoint(lcModelHistoryEntry* CheckPoint);

	QString GetGroupName(const QString& Prefix);
	void RemoveEmptyGroups();
/*** LPub3D Mod - Build Modification ***/
	quint32 RemoveSelectedObjects();
/*** LPub3D Mod end ***/
/*** LPub3D Mod - true fade ***/
	bool mLPubFade;
/*** LPub3D Mod end ***/

	void SelectGroup(lcGroup* TopGroup, bool Select);

//	void AddPiece(lcPiece* Piece); /*** LPub3D Mod - viewer interface (moved to public) ***/
	void InsertPiece(lcPiece* Piece, int Index);

	lcModelProperties mProperties;
	Project* const mProject;
	PieceInfo* mPieceInfo;

	bool mIsPreview;
/*** LPub3D Mod - Selected Parts ***/
	bool mModAction;
/*** LPub3D Mod end ***/
	bool mActive;
	lcStep mCurrentStep;
	lcVector3 mMouseToolDistance;

	lcArray<lcPiece*> mPieces;
	lcArray<lcCamera*> mCameras;
	lcArray<lcLight*> mLights;
	lcArray<lcGroup*> mGroups;
	QStringList mFileLines;

	lcModelHistoryEntry* mSavedHistory;
	std::vector<lcModelHistoryEntry*> mUndoHistory;
	std::vector<lcModelHistoryEntry*> mRedoHistory;

	Q_DECLARE_TR_FUNCTIONS(lcModel);
};
