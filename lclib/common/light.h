#pragma once

#include "object.h"
#include "lc_math.h"

#define LC_LIGHT_HIDDEN            0x0001
#define LC_LIGHT_DISABLED          0x0002
#define LC_LIGHT_SPOT              0x0004
#define LC_LIGHT_DIRECTIONAL       0x0008
#define LC_LIGHT_POSITION_SELECTED 0x0010
#define LC_LIGHT_POSITION_FOCUSED  0x0020
#define LC_LIGHT_TARGET_SELECTED   0x0040
#define LC_LIGHT_TARGET_FOCUSED    0x0080

#define LC_LIGHT_SELECTION_MASK    (LC_LIGHT_POSITION_SELECTED | LC_LIGHT_TARGET_SELECTED)
#define LC_LIGHT_FOCUS_MASK        (LC_LIGHT_POSITION_FOCUSED | LC_LIGHT_TARGET_FOCUSED)

enum lcLightSection
{
	LC_LIGHT_SECTION_POSITION,
	LC_LIGHT_SECTION_TARGET
};

/*** LPub3D Mod - enable lights ***/
enum lcLightType
{
	LC_UNDEFINED_LIGHT,
	LC_POINTLIGHT,
	LC_SUNLIGHT,
	LC_SPOTLIGHT,
	LC_AREALIGHT,
	LC_NUM_LIGHT_TYPES
};

enum lcLightProperty
{
	LC_UNDEFINED_SHAPE = -1,
	LC_LIGHT_NONE,
	LC_LIGHT_SHAPE_SQUARE = LC_LIGHT_NONE,
	LC_LIGHT_SHAPE,
	LC_LIGHT_SHAPE_DISK = LC_LIGHT_SHAPE,
	LC_LIGHT_COLOR,
	LC_LIGHT_SHAPE_RECTANGLE = LC_LIGHT_COLOR,
	LC_LIGHT_TYPE,
	LC_LIGHT_SHAPE_ELLIPSE = LC_LIGHT_TYPE,
	LC_LIGHT_FACTOR,
	LC_LIGHT_SPECULAR,
	LC_LIGHT_EXPONENT,
	LC_LIGHT_SPOT_SIZE,
	LC_LIGHT_CUTOFF,
	LC_LIGHT_USE_CUTOFF
};

class lcLightProps
{
public:
	lcVector3 mLightColor;
	lcVector2 mLightFactor;
	float     mLightSpecular;
	float     mSpotExponent;
	float     mSpotCutoff;
	float     mSpotSize;
	bool      mEnableCutoff;
	int       mLightShape;
};
/*** LPub3D Mod end ***/

class lcLight : public lcObject
{
public:
	lcLight(float px, float py, float pz);
/*** LPub3D Mod - enable lights ***/
	lcLight(float px, float py, float pz, float tx, float ty, float tz, int LightType);
/*** LPub3D Mod end ***/
	virtual ~lcLight();

	bool IsPointLight() const
	{
		return (mState & (LC_LIGHT_SPOT | LC_LIGHT_DIRECTIONAL)) == 0;
	}

	bool IsSpotLight() const
	{
		return (mState & LC_LIGHT_SPOT) != 0;
	}

	bool IsDirectionalLight() const
	{
		return (mState & LC_LIGHT_DIRECTIONAL) != 0;
	}

	virtual bool IsSelected() const override
	{
		return (mState & LC_LIGHT_SELECTION_MASK) != 0;
	}

	virtual bool IsSelected(quint32 Section) const override
	{
		switch (Section)
		{
		case LC_LIGHT_SECTION_POSITION:
			return (mState & LC_LIGHT_POSITION_SELECTED) != 0;
			break;

		case LC_LIGHT_SECTION_TARGET:
			return (mState & LC_LIGHT_TARGET_SELECTED) != 0;
			break;
		}
		return false;
	}

	virtual void SetSelected(bool Selected) override
	{
		if (Selected)
		{
			if (IsPointLight())
				mState |= LC_LIGHT_POSITION_SELECTED;
			else
				mState |= LC_LIGHT_SELECTION_MASK;
		}
		else
			mState &= ~(LC_LIGHT_SELECTION_MASK | LC_LIGHT_FOCUS_MASK);
	}

	virtual void SetSelected(quint32 Section, bool Selected) override
	{
		switch (Section)
		{
		case LC_LIGHT_SECTION_POSITION:
			if (Selected)
				mState |= LC_LIGHT_POSITION_SELECTED;
			else
				mState &= ~(LC_LIGHT_POSITION_SELECTED | LC_LIGHT_POSITION_FOCUSED);
			break;

		case LC_LIGHT_SECTION_TARGET:
			if (Selected)
			{
				if (!IsPointLight())
					mState |= LC_LIGHT_TARGET_SELECTED;
			}
			else
				mState &= ~(LC_LIGHT_TARGET_SELECTED | LC_LIGHT_TARGET_FOCUSED);
			break;
		}
	}

	virtual bool IsFocused() const override
	{
		return (mState & LC_LIGHT_FOCUS_MASK) != 0;
	}

	virtual bool IsFocused(quint32 Section) const override
	{
		switch (Section)
		{
		case LC_LIGHT_SECTION_POSITION:
			return (mState & LC_LIGHT_POSITION_FOCUSED) != 0;
			break;

		case LC_LIGHT_SECTION_TARGET:
			return (mState & LC_LIGHT_TARGET_FOCUSED) != 0;
			break;
		}
		return false;
	}

	virtual void SetFocused(quint32 Section, bool Focused) override
	{
		switch (Section)
		{
		case LC_LIGHT_SECTION_POSITION:
			if (Focused)
				mState |= LC_LIGHT_POSITION_SELECTED | LC_LIGHT_POSITION_FOCUSED;
			else
				mState &= ~(LC_LIGHT_POSITION_SELECTED | LC_LIGHT_POSITION_FOCUSED);
			break;

		case LC_LIGHT_SECTION_TARGET:
			if (Focused)
			{
				if (!IsPointLight())
					mState |= LC_LIGHT_TARGET_SELECTED | LC_LIGHT_TARGET_FOCUSED;
			}
			else
				mState &= ~(LC_LIGHT_TARGET_SELECTED | LC_LIGHT_TARGET_FOCUSED);
			break;
		}
	}

	virtual quint32 GetFocusSection() const override
	{
		if (mState & LC_LIGHT_POSITION_FOCUSED)
			return LC_LIGHT_SECTION_POSITION;

		if (!IsPointLight() && (mState & LC_LIGHT_TARGET_FOCUSED))
			return LC_LIGHT_SECTION_TARGET;

		return ~0U;
	}

	virtual quint32 GetAllowedTransforms() const override
	{
		return LC_OBJECT_TRANSFORM_MOVE_X | LC_OBJECT_TRANSFORM_MOVE_Y | LC_OBJECT_TRANSFORM_MOVE_Z;
	}

	virtual lcVector3 GetSectionPosition(quint32 Section) const override
	{
		switch (Section)
		{
		case LC_LIGHT_SECTION_POSITION:
			return mPosition;

		case LC_LIGHT_SECTION_TARGET:
			return mTargetPosition;
		}

		return lcVector3(0.0f, 0.0f, 0.0f);
	}

	void SaveLDraw(QTextStream& Stream) const;

public:
	virtual void RayTest(lcObjectRayTest& ObjectRayTest) const override;
	virtual void BoxTest(lcObjectBoxTest& ObjectBoxTest) const override;
	virtual void DrawInterface(lcContext* Context, const lcScene& Scene) const override;
	virtual void RemoveKeyFrames() override;

	void InsertTime(lcStep Start, lcStep Time);
	void RemoveTime(lcStep Start, lcStep Time);

	bool IsVisible() const
	{ return (mState & LC_LIGHT_HIDDEN) == 0; }

	const char* GetName() const override
	{ return m_strName; }

	void CompareBoundingBox(lcVector3& Min, lcVector3& Max);
	void UpdatePosition(lcStep Step);
	void MoveSelected(lcStep Step, bool AddKey, const lcVector3& Distance);
	bool Setup(int LightIndex);
	void CreateName(const lcArray<lcLight*>& Lights);
/*** LPub3D Mod - enable lights ***/
	void UpdateLight(lcStep Step, lcLightProps Props, int Property);
	lcLightProps GetLightProps() const
	{
		lcLightProps props;
		props.mLightColor    = mLightColor;
		props.mLightFactor   = mLightFactor;
		props.mLightSpecular = mLightSpecular;
		props.mSpotExponent  = mSpotExponent;
		props.mSpotCutoff    = mSpotCutoff;
		props.mSpotSize      = mSpotSize;
		props.mEnableCutoff  = mEnableCutoff;
		props.mLightShape    = mLightShape;
		return props;
	}
/*** LPub3D Mod end ***/

	// Temporary parameters
	lcMatrix44 mWorldLight;
	lcVector3 mPosition;
	lcVector3 mTargetPosition;
	lcVector4 mAmbientColor;
	lcVector4 mDiffuseColor;
	lcVector4 mSpecularColor;
	lcVector3 mAttenuation;
/*** LPub3D Mod - enable lights ***/
	lcVector3 mLightColor;
	lcVector2 mLightFactor;
	bool  mEnableCutoff;
	int   mLightType;
	int   mLightShape;
	float mLightSpecular;
	float mSpotSize;
/*** LPub3D Mod end ***/
	float mSpotCutoff;
	float mSpotExponent;
/*** LPub3D Mod - enable lights ***/
	char m_strName[81];
/*** LPub3D Mod end ***/

protected:
	lcArray<lcObjectKey<lcVector3>> mPositionKeys;
	lcArray<lcObjectKey<lcVector3>> mTargetPositionKeys;
	lcArray<lcObjectKey<lcVector4>> mAmbientColorKeys;
	lcArray<lcObjectKey<lcVector4>> mDiffuseColorKeys;
	lcArray<lcObjectKey<lcVector4>> mSpecularColorKeys;
	lcArray<lcObjectKey<lcVector3>> mAttenuationKeys;
/*** LPub3D Mod - enable lights ***/
	lcArray<lcObjectKey<lcVector3>> mLightColorKeys;
	lcArray<lcObjectKey<lcVector2>> mLightFactorKeys;
	lcArray<lcObjectKey<int>> mLightTypeKeys;
	lcArray<lcObjectKey<int>> mLightShapeKeys;
	lcArray<lcObjectKey<float>> mLightSpecularKeys;
	lcArray<lcObjectKey<float>> mLightSpotSizeKeys;
/*** LPub3D Mod end ***/
	lcArray<lcObjectKey<float>> mSpotCutoffKeys;
	lcArray<lcObjectKey<float>> mSpotExponentKeys;

/*** LPub3D Mod - enable lights ***/
	void Initialize(const lcVector3& Position, const lcVector3& TargetPosition, int LightType);
	void DrawDirectionalLight(lcContext* Context) const;
/*** LPub3D Mod end ***/
	void DrawPointLight(lcContext* Context) const;
	void DrawSpotLight(lcContext* Context) const;

	quint32 mState;
/*** LPub3D Mod - enable point lignt, moved m_strName to public ***/
};

