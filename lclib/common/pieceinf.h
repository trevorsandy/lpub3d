#pragma once

#include <stdio.h>
#include "lc_math.h"
#include "lc_array.h"

enum class lcPieceInfoType
{
	Part,
	Placeholder,
	Model,
	Project
};

#define LC_PIECE_NAME_LEN 256

enum class lcPieceInfoState
{
	Unloaded,
	Loading,
	Loaded
};

struct lcModelPartsEntry
{
	lcMatrix44 WorldMatrix;
	const PieceInfo* Info;
	lcMesh* Mesh;
	int ColorIndex;
};

class lcSynthInfo;
enum class lcZipFileType;

class PieceInfo
{
public:
	PieceInfo();
	~PieceInfo();

	PieceInfo(const PieceInfo&) = delete;
	PieceInfo(PieceInfo&&) = delete;
	PieceInfo& operator=(const PieceInfo&) = delete;
	PieceInfo& operator=(PieceInfo&&) = delete;

	const lcBoundingBox& GetBoundingBox() const
	{
		return mBoundingBox;
	}

	void SetBoundingBox(const lcVector3& Min, const lcVector3& Max)
	{
		mBoundingBox.Min = Min;
		mBoundingBox.Max = Max;
	}

	lcSynthInfo* GetSynthInfo() const
	{
		return mSynthInfo;
	}

	void SetSynthInfo(lcSynthInfo* SynthInfo)
	{
		mSynthInfo = SynthInfo;
	}

	lcMesh* GetMesh() const
	{
		return mMesh;
	}

	lcModel* GetModel() const
	{
		return mModel;
	}

	Project* GetProject() const
	{
		return mProject;
	}

	void SetMesh(lcMesh* Mesh);

	int AddRef()
	{
		mRefCount++;
		return mRefCount;
	}

	int Release()
	{
		mRefCount--;
		return mRefCount;
	}

	int GetRefCount() const
	{
		return mRefCount;
	}

	bool IsPlaceholder() const
	{
		return mType == lcPieceInfoType::Placeholder;
	}

	bool IsModel() const
	{
		return mType == lcPieceInfoType::Model;
	}

	bool IsProject() const
	{
		return mType == lcPieceInfoType::Project;
	}

	bool IsTemporary() const
	{
		return mType != lcPieceInfoType::Part;
	}

	void SetZipFile(lcZipFileType ZipFileType, int ZipFileIndex)
	{
		mZipFileType = ZipFileType;
		mZipFileIndex = ZipFileIndex;
	}

	bool IsPatterned() const
	{
		if (mType != lcPieceInfoType::Part)
			return false;

		const char* Name = mFileName;

		// Heuristic: Names matching '^[Uu]?[0-9]*[A-Za-z][^.][^.]' are patterned.

		if (*Name == 'U' || *Name == 'u')
			Name++;

		while (*Name)
		{
			if (*Name < '0' || *Name > '9')
				break;

			Name++;
		}

		if (!*Name || !((*Name >= 'A' && *Name <= 'Z') || (*Name >= 'a' && *Name <= 'z')))
			return false;

		if (Name[1] && Name[1] != '.' && Name[2] && Name[2] != '.')
			return true;

		return false;
	}

	bool IsSubPiece() const
	{
		return (m_strDescription[0] == '~');
	}

/*** LPub3D Mod - part type check ***/
	bool IsPartType () const
	{
		return (m_iPartType != 0);
	}
/*** LPub3D Mod end ***/

/*** LPub3D Mod - project piece ***/
	bool IsProjectPiece() const
	{
		return mProjectPiece;
	}
	bool DisplayProjectAsPiece() const;
/*** LPub3D Mod end ***/
	void ZoomExtents(float FoV, float AspectRatio, lcMatrix44& ProjectionMatrix, lcMatrix44& ViewMatrix) const;
	void AddRenderMesh(lcScene& Scene);
/*** LPub3D Mod - true fade ***/	
	void AddRenderMeshes(lcScene* Scene, const lcMatrix44& WorldMatrix, int ColorIndex, lcRenderMeshState RenderMeshState, bool ParentActive, bool LPubFade = false) const;
/*** LPub3D Mod end ***/

	void CreatePlaceholder(const char* Name);

	void SetPlaceholder();
	void SetModel(lcModel* Model, bool UpdateMesh, Project* CurrentProject, bool SearchProjectFolder);
	void CreateProject(Project* Project, const char* PieceName);
	bool GetPieceWorldMatrix(lcPiece* Piece, lcMatrix44& WorldMatrix) const;
	bool IncludesModel(const lcModel* Model) const;
	bool MinIntersectDist(const lcVector3& Start, const lcVector3& End, float& MinDistance, lcPieceInfoRayTest& PieceInfoRayTest) const;
	bool BoxTest(const lcMatrix44& WorldMatrix, const lcVector4 Planes[6]) const;
	void GetPartsList(int DefaultColorIndex, bool ScanSubModels, bool AddSubModels, lcPartsList& PartsList) const;
	void GetModelParts(const lcMatrix44& WorldMatrix, int DefaultColorIndex, std::vector<lcModelPartsEntry>& ModelParts) const;
	void CompareBoundingBox(const lcMatrix44& WorldMatrix, lcVector3& Min, lcVector3& Max) const;
	void AddSubModelBoundingBoxPoints(const lcMatrix44& WorldMatrix, std::vector<lcVector3>& Points) const;
	void UpdateBoundingBox(std::vector<lcModel*>& UpdatedModels);

	void Load();
	void Unload();

public:
	char mFileName[LC_PIECE_NAME_LEN];
	char m_strDescription[128];
/*** LPub3D Mod 166 - part type check ***/
	int m_iPartType;
/*** LPub3D Mod end ***/
	lcZipFileType mZipFileType;
	int mZipFileIndex;
	lcPieceInfoState mState;
	int mFolderType;
	int mFolderIndex;

protected:
	void ReleaseMesh();

	int mRefCount;
/*** LPub3D Mod - project piece ***/
	bool mProjectPiece;
/*** LPub3D Mod end ***/
	lcPieceInfoType mType;
	lcModel* mModel;
	Project* mProject;
	lcMesh* mMesh;
	lcBoundingBox mBoundingBox;
	lcSynthInfo* mSynthInfo;
};
