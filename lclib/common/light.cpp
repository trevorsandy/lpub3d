#include "lc_global.h"
#include "lc_math.h"
#include "lc_colors.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "light.h"
#include "lc_application.h"
#include "lc_context.h"

#define LC_LIGHT_POSITION_EDGE 7.5f
#define LC_LIGHT_TARGET_EDGE 5.0f
#define LC_LIGHT_SPHERE_RADIUS 5.0f
/*** LPub3D Mod - enable lights ***/
#define LC_LIGHT_BASEFACE_EDGE 12.5f
/*** LPub3D Mod end ***/

// New omni light.
lcLight::lcLight(float px, float py, float pz)
	: lcObject(LC_OBJECT_LIGHT)
{
	Initialize(lcVector3(px, py, pz), lcVector3(0.0f, 0.0f, 0.0f), LC_POINTLIGHT);
	UpdatePosition(1);
}

// New directional or spot light.
/*** LPub3D Mod - enable lights ***/
lcLight::lcLight(float px, float py, float pz, float tx, float ty, float tz, int LightType)
	: lcObject(LC_OBJECT_LIGHT)
{
	Initialize(lcVector3(px, py, pz), lcVector3(tx, ty, tz), LightType);
	if (LightType == LC_SPOTLIGHT)
		mState |= LC_LIGHT_SPOT;
	else
		mState |= LC_LIGHT_DIRECTIONAL;
/*** LPub3D Mod end ***/
	UpdatePosition(1);
}

/*** LPub3D Mod - enable lights ***/
void lcLight::Initialize(const lcVector3& Position, const lcVector3& TargetPosition, int LightType)
{
	mState = 0;
	memset(m_strName, 0, sizeof(m_strName));
	mEnableCutoff   = false;
	mPosition       = Position;
	mTargetPosition = TargetPosition;
	mAmbientColor   = lcVector4(0.0f, 0.0f, 0.0f, 1.0f);
	mDiffuseColor   = lcVector4(0.8f, 0.8f, 0.8f, 1.0f);
	mSpecularColor  = lcVector4(1.0f, 1.0f, 1.0f, 1.0f);
	mAttenuation    = lcVector3(1.0f, 0.0f, 0.0f);
	mLightColor     = lcVector3(1.0f, 1.0f, 1.0f); //RGB - White
	mLightType      = LightType ? LightType : int(LC_POINTLIGHT);
	mLightFactor[0] = LightType ? LightType == LC_SUNLIGHT ? 11.4f : 0.25f : 0.0f;
	mLightFactor[1] = LightType == LC_AREALIGHT ? 0.25f : LightType == LC_SPOTLIGHT ? 0.150f : 0.0f;
	mLightSpecular  = 1.0f;
	mSpotSize       = 75.0f;
	mLightShape     = 0 /*Square*/;
	mSpotCutoff     = LightType ? LightType != LC_SUNLIGHT ? 40.0f : 0.0f : 30.0f;
	mSpotExponent   = 10.0f; /*Energy/Power*/

	ChangeKey(mPositionKeys, mPosition, 1, true);
	ChangeKey(mTargetPositionKeys, mTargetPosition, 1, true);
	ChangeKey(mAmbientColorKeys, mAmbientColor, 1, true);
	ChangeKey(mDiffuseColorKeys, mDiffuseColor, 1, true);
	ChangeKey(mSpecularColorKeys, mSpecularColor, 1, true);
	ChangeKey(mAttenuationKeys, mAttenuation, 1, true);
	ChangeKey(mLightShapeKeys, mLightShape, 1, true);
	ChangeKey(mLightColorKeys, mLightColor, 1, true);
	ChangeKey(mLightTypeKeys, mLightType, 1, true);
	ChangeKey(mLightFactorKeys, mLightFactor, 1, true);
	ChangeKey(mLightSpecularKeys, mLightSpecular, 1, true);
	ChangeKey(mSpotCutoffKeys, mSpotCutoff, 1, true);
	ChangeKey(mSpotExponentKeys, mSpotExponent, 1, true);
	ChangeKey(mLightSpotSizeKeys, mSpotSize, 1, true);

/*** LPub3D Mod end ***/
}

lcLight::~lcLight()
{
}

void lcLight::SaveLDraw(QTextStream& Stream) const
{
/*** LPub3D Mod - enable lights ***/
	QLatin1String LineEnding("\r\n");

	if (mPositionKeys.GetSize() > 1)
		SaveKeysLDraw(Stream, mPositionKeys, "LIGHT POSITION_KEY ");
	else
		Stream << QLatin1String("0 !LEOCAD LIGHT POSITION ") << mPosition[0] << ' ' << mPosition[1] << ' ' << mPosition[2] << LineEnding;

	if (mTargetPositionKeys.GetSize() > 1)
		SaveKeysLDraw(Stream, mTargetPositionKeys, "LIGHT TARGET_POSITION_KEY ");
	else
		Stream << QLatin1String("0 !LEOCAD LIGHT TARGET_POSITION ") << mTargetPosition[0] << ' ' << mTargetPosition[1] << ' ' << mTargetPosition[2] << LineEnding;

	if (mLightColorKeys.GetSize() > 1)
		SaveKeysLDraw(Stream, mLightColorKeys, "LIGHT COLOR_RGB_KEY ");
	else
		Stream << QLatin1String("0 !LEOCAD LIGHT COLOR_RGB ") << mLightColor[0] << ' ' << mLightColor[1] << ' ' << mLightColor[2] << LineEnding;

	if (mLightType == LC_SUNLIGHT)
	{
		if (mSpotExponentKeys.GetSize() > 1)
			SaveKeysLDraw(Stream, mSpotExponentKeys, "LIGHT STRENGTH_KEY ");
		else
			Stream << QLatin1String("0 !LEOCAD LIGHT STRENGTH ") << mSpotExponent << LineEnding;

		if (mLightFactorKeys.GetSize() > 1)
			SaveKeysLDraw(Stream, mLightFactorKeys, "LIGHT ANGLE_KEY ");
		else
			Stream << QLatin1String("0 !LEOCAD LIGHT ANGLE ") << mLightFactor[0] << LineEnding;
	}
	else
	{
		if (mSpotExponentKeys.GetSize() > 1)
			SaveKeysLDraw(Stream, mSpotExponentKeys, "LIGHT POWER_KEY ");
		else
			Stream << QLatin1String("0 !LEOCAD LIGHT POWER ") << mSpotExponent << LineEnding;

		switch (mLightType)
		{
		case LC_POINTLIGHT:
			if (mLightFactorKeys.GetSize() > 1)
				SaveKeysLDraw(Stream, mLightFactorKeys, "LIGHT RADIUS_KEY ");
			else
				Stream << QLatin1String("0 !LEOCAD LIGHT RADIUS ") << mLightFactor[0] << LineEnding;
			break;
		case LC_SPOTLIGHT:
			if (mLightFactorKeys.GetSize() > 1) {
				SaveKeysLDraw(Stream, mLightFactorKeys, "LIGHT RADIUS_AND_SPOT_BLEND_KEY ");
			} else {
				Stream << QLatin1String("0 !LEOCAD LIGHT RADIUS ") << mLightFactor[0] << LineEnding;
				Stream << QLatin1String("0 !LEOCAD SPOT_BLEND ") << mLightFactor[1] << LineEnding;
			}
			if (mLightSpotSizeKeys.GetSize() > 1)
				SaveKeysLDraw(Stream, mLightSpotSizeKeys, "LIGHT SPOT_SIZE_KEY ");
			else
				Stream << QLatin1String("0 !LEOCAD LIGHT SPOT_SIZE ") << mSpotSize << LineEnding;
			break;
		case LC_AREALIGHT:
			if (mLightFactorKeys.GetSize() > 1) {
				SaveKeysLDraw(Stream, mLightFactorKeys, "LIGHT SIZE_KEY ");
			} else {
				if (mLightShape == LC_LIGHT_SHAPE_RECTANGLE || mLightShape == LC_LIGHT_SHAPE_ELLIPSE)
					Stream << QLatin1String("0 !LEOCAD LIGHT WIDTH ") << mLightFactor[0] << QLatin1String(" HEIGHT ") << mLightFactor[1] << LineEnding;
				else
					Stream << QLatin1String("0 !LEOCAD LIGHT SIZE ") << mLightFactor[0] << LineEnding;
			}
			if (mLightShapeKeys.GetSize() > 1) {
				SaveKeysLDraw(Stream, mSpotExponentKeys, "LIGHT SHAPE_KEY ");
			} else {
				Stream << QLatin1String("0 !LEOCAD LIGHT SHAPE ");

				QString Shape = "Undefined ";
				switch(mLightShape){
				case LC_LIGHT_SHAPE_SQUARE:
					Shape = "Square ";
					break;
				case LC_LIGHT_SHAPE_DISK:
					Shape = "Disk ";
					break;
				case LC_LIGHT_SHAPE_RECTANGLE:
					Shape = "Rectangle ";
					break;
				case LC_LIGHT_SHAPE_ELLIPSE:
					Shape = "Ellipse ";
					break;
				}
				Stream << QLatin1String(Shape.toLatin1()) << LineEnding;
			}

			break;
		}
	}

	if (mLightSpecularKeys.GetSize() > 1)
		SaveKeysLDraw(Stream, mLightSpecularKeys, "LIGHT SPECULAR_KEY ");
	else
		Stream << QLatin1String("0 !LEOCAD LIGHT SPECULAR ") << mLightSpecular << LineEnding;

	if (mLightType != LC_SUNLIGHT && mEnableCutoff)
	{
		if (mSpotCutoffKeys.GetSize() > 1)
			SaveKeysLDraw(Stream, mSpotCutoffKeys, "LIGHT CUTOFF_DISTANCE_KEY ");
		else
			Stream << QLatin1String("0 !LEOCAD LIGHT CUTOFF_DISTANCE ") << mSpotCutoff << LineEnding;
	}

	if (mLightTypeKeys.GetSize() > 1)
	{
		SaveKeysLDraw(Stream, mLightTypeKeys, "LIGHT TYPE_KEY ");
	}
	else
	{
		Stream << QLatin1String("0 !LEOCAD LIGHT TYPE ");

		QString Type = "Undefined ";
		switch(mLightType){
		case LC_POINTLIGHT:
			Type = "Point ";
			break;
		case LC_SUNLIGHT:
			Type = "Sun ";
			break;
		case LC_SPOTLIGHT:
			Type = "Spot ";
			break;
		case LC_AREALIGHT:
			Type = "Area ";
			break;
		}
		Stream << QLatin1String(Type.toLatin1()) << QLatin1String("NAME ") << m_strName << LineEnding;
	}
/*** LPub3D Mod end ***/
}

void lcLight::CreateName(const lcArray<lcLight*>& Lights)
{
	if (m_strName[0])
	{
		bool Found = false;

		for (lcLight* Light : Lights)
		{
			if (!strcmp(Light->m_strName, m_strName))
			{
				Found = true;
				break;
			}
		}

		if (!Found)
			return;
	}

	int i, max = 0;
/*** LPub3D Mod - enable lights ***/
	const char* Prefix = (mLightType == LC_POINTLIGHT ? "Pointlight "
						: mLightType == LC_SUNLIGHT   ? "Sunlight "
						: mLightType == LC_SPOTLIGHT  ? "Spotlight "
						: "Arealight ");

	for (int LightIdx = 0; LightIdx < Lights.GetSize(); LightIdx++)
		if (strncmp(Lights[LightIdx]->m_strName, Prefix, strlen(Prefix)) == 0)
			if (sscanf(Lights[LightIdx]->m_strName + strlen(Prefix), " %d", &i) == 1)
				if (i > max)
					max = i;

	sprintf(m_strName, "%s %d", Prefix, max+1);
/*** LPub3D Mod end ***/
}

void lcLight::CompareBoundingBox(lcVector3& Min, lcVector3& Max)
{
	const lcVector3 Points[2] =
	{
		mPosition, mTargetPosition
	};

	for (int i = 0; i < (IsPointLight() ? 1 : 2); i++)
	{
		const lcVector3& Point = Points[i];

		// TODO: this should check the entire mesh

		Min = lcMin(Point, Min);
		Max = lcMax(Point, Max);
	}
}

/*** LPub3D Mod - enable lights ***/
void lcLight::UpdateLight(lcStep Step, lcLightProps Props, int Property)
{
	switch(Property){
	case LC_LIGHT_SHAPE:
		mLightShape = Props.mLightShape;
		ChangeKey(mLightShapeKeys, mLightShape, Step, false);
		break;
	case LC_LIGHT_COLOR:
		mLightColor = Props.mLightColor;
		ChangeKey(mLightColorKeys, mLightColor, Step, false);
		break;
	case LC_LIGHT_FACTOR:
		mLightFactor = Props.mLightFactor;
		ChangeKey(mLightFactorKeys, mLightFactor, Step, false);
		break;
	case LC_LIGHT_SPECULAR:
		mLightSpecular = Props.mLightSpecular;
		ChangeKey(mLightSpecularKeys, mLightSpecular, Step, false);
		break;
	case LC_LIGHT_EXPONENT:
		mSpotExponent = Props.mSpotExponent;
		ChangeKey(mSpotExponentKeys, mSpotExponent, Step, false);
		break;
	case LC_LIGHT_SPOT_SIZE:
		mSpotSize = Props.mSpotSize;
		ChangeKey(mLightSpotSizeKeys, mSpotSize, Step, false);
		break;
	case LC_LIGHT_CUTOFF:
		mSpotCutoff = Props.mSpotCutoff;
		ChangeKey(mSpotCutoffKeys, mSpotCutoff, Step, false);
		break;
	case LC_LIGHT_USE_CUTOFF:
		mEnableCutoff = Props.mEnableCutoff;
		break;
	}
	UpdatePosition(Step);
}
/*** LPub3D Mod end ***/

void lcLight::RayTest(lcObjectRayTest& ObjectRayTest) const
{
	if (IsPointLight())
	{
		float Distance;

		if (lcSphereRayMinIntersectDistance(mPosition, LC_LIGHT_SPHERE_RADIUS, ObjectRayTest.Start, ObjectRayTest.End, &Distance))
		{
			ObjectRayTest.ObjectSection.Object = const_cast<lcLight*>(this);
			ObjectRayTest.ObjectSection.Section = LC_LIGHT_SECTION_POSITION;
			ObjectRayTest.Distance = Distance;
		}

		return;
	}

	lcVector3 Min = lcVector3(-LC_LIGHT_POSITION_EDGE, -LC_LIGHT_POSITION_EDGE, -LC_LIGHT_POSITION_EDGE);
	lcVector3 Max = lcVector3(LC_LIGHT_POSITION_EDGE, LC_LIGHT_POSITION_EDGE, LC_LIGHT_POSITION_EDGE);

	lcVector3 Start = lcMul31(ObjectRayTest.Start, mWorldLight);
	lcVector3 End = lcMul31(ObjectRayTest.End, mWorldLight);

	float Distance;
	if (lcBoundingBoxRayIntersectDistance(Min, Max, Start, End, &Distance, nullptr) && (Distance < ObjectRayTest.Distance))
	{
		ObjectRayTest.ObjectSection.Object = const_cast<lcLight*>(this);
		ObjectRayTest.ObjectSection.Section = LC_LIGHT_SECTION_POSITION;
		ObjectRayTest.Distance = Distance;
	}

	Min = lcVector3(-LC_LIGHT_TARGET_EDGE, -LC_LIGHT_TARGET_EDGE, -LC_LIGHT_TARGET_EDGE);
	Max = lcVector3(LC_LIGHT_TARGET_EDGE, LC_LIGHT_TARGET_EDGE, LC_LIGHT_TARGET_EDGE);

	lcMatrix44 WorldTarget = mWorldLight;
	WorldTarget.SetTranslation(lcMul30(-mTargetPosition, WorldTarget));

	Start = lcMul31(ObjectRayTest.Start, WorldTarget);
	End = lcMul31(ObjectRayTest.End, WorldTarget);

	if (lcBoundingBoxRayIntersectDistance(Min, Max, Start, End, &Distance, nullptr) && (Distance < ObjectRayTest.Distance))
	{
		ObjectRayTest.ObjectSection.Object = const_cast<lcLight*>(this);
		ObjectRayTest.ObjectSection.Section = LC_LIGHT_SECTION_TARGET;
		ObjectRayTest.Distance = Distance;
	}
}

void lcLight::BoxTest(lcObjectBoxTest& ObjectBoxTest) const
{
	if (IsPointLight())
	{
		for (int PlaneIdx = 0; PlaneIdx < 6; PlaneIdx++)
			if (lcDot3(mPosition, ObjectBoxTest.Planes[PlaneIdx]) + ObjectBoxTest.Planes[PlaneIdx][3] > LC_LIGHT_SPHERE_RADIUS)
				return;

		ObjectBoxTest.Objects.Add(const_cast<lcLight*>(this));
		return;
	}

	lcVector3 Min(-LC_LIGHT_POSITION_EDGE, -LC_LIGHT_POSITION_EDGE, -LC_LIGHT_POSITION_EDGE);
	lcVector3 Max(LC_LIGHT_POSITION_EDGE, LC_LIGHT_POSITION_EDGE, LC_LIGHT_POSITION_EDGE);

	lcVector4 LocalPlanes[6];

	for (int PlaneIdx = 0; PlaneIdx < 6; PlaneIdx++)
	{
		lcVector3 Normal = lcMul30(ObjectBoxTest.Planes[PlaneIdx], mWorldLight);
		LocalPlanes[PlaneIdx] = lcVector4(Normal, ObjectBoxTest.Planes[PlaneIdx][3] - lcDot3(mWorldLight[3], Normal));
	}

	if (lcBoundingBoxIntersectsVolume(Min, Max, LocalPlanes))
	{
		ObjectBoxTest.Objects.Add(const_cast<lcLight*>(this));
		return;
	}

	Min = lcVector3(-LC_LIGHT_TARGET_EDGE, -LC_LIGHT_TARGET_EDGE, -LC_LIGHT_TARGET_EDGE);
	Max = lcVector3(LC_LIGHT_TARGET_EDGE, LC_LIGHT_TARGET_EDGE, LC_LIGHT_TARGET_EDGE);

	lcMatrix44 WorldTarget = mWorldLight;
	WorldTarget.SetTranslation(lcMul30(-mTargetPosition, WorldTarget));

	for (int PlaneIdx = 0; PlaneIdx < 6; PlaneIdx++)
	{
		lcVector3 Normal = lcMul30(ObjectBoxTest.Planes[PlaneIdx], WorldTarget);
		LocalPlanes[PlaneIdx] = lcVector4(Normal, ObjectBoxTest.Planes[PlaneIdx][3] - lcDot3(WorldTarget[3], Normal));
	}

	if (lcBoundingBoxIntersectsVolume(Min, Max, LocalPlanes))
	{
		ObjectBoxTest.Objects.Add(const_cast<lcLight*>(this));
		return;
	}
}

void lcLight::MoveSelected(lcStep Step, bool AddKey, const lcVector3& Distance)
{
	if (IsSelected(LC_LIGHT_SECTION_POSITION))
	{
		mPosition += Distance;
		ChangeKey(mPositionKeys, mPosition, Step, AddKey);
	}

	if (IsSelected(LC_LIGHT_SECTION_TARGET))
	{
		mTargetPosition += Distance;
		ChangeKey(mTargetPositionKeys, mTargetPosition, Step, AddKey);
	}
}

void lcLight::InsertTime(lcStep Start, lcStep Time)
{
	lcObject::InsertTime(mPositionKeys, Start, Time);
	lcObject::InsertTime(mTargetPositionKeys, Start, Time);
	lcObject::InsertTime(mAmbientColorKeys, Start, Time);
	lcObject::InsertTime(mDiffuseColorKeys, Start, Time);
	lcObject::InsertTime(mSpecularColorKeys, Start, Time);
	lcObject::InsertTime(mAttenuationKeys, Start, Time);
/*** LPub3D Mod - enable lights ***/
	lcObject::InsertTime(mLightShapeKeys, Start, Time);
	lcObject::InsertTime(mLightColorKeys, Start, Time);
	lcObject::InsertTime(mLightTypeKeys, Start, Time);
	lcObject::InsertTime(mLightFactorKeys, Start, Time);
	lcObject::InsertTime(mLightSpecularKeys, Start, Time);
	lcObject::InsertTime(mLightSpotSizeKeys, Start, Time);
/*** LPub3D Mod end ***/
	lcObject::InsertTime(mSpotCutoffKeys, Start, Time);
	lcObject::InsertTime(mSpotExponentKeys, Start, Time);
}

void lcLight::RemoveTime(lcStep Start, lcStep Time)
{
	lcObject::RemoveTime(mPositionKeys, Start, Time);
	lcObject::RemoveTime(mTargetPositionKeys, Start, Time);
	lcObject::RemoveTime(mAmbientColorKeys, Start, Time);
	lcObject::RemoveTime(mDiffuseColorKeys, Start, Time);
	lcObject::RemoveTime(mSpecularColorKeys, Start, Time);
	lcObject::RemoveTime(mAttenuationKeys, Start, Time);
/*** LPub3D Mod - enable lights ***/
	lcObject::RemoveTime(mLightShapeKeys, Start, Time);
	lcObject::RemoveTime(mLightColorKeys, Start, Time);
	lcObject::RemoveTime(mLightTypeKeys, Start, Time);
	lcObject::RemoveTime(mLightFactorKeys, Start, Time);
	lcObject::RemoveTime(mLightSpecularKeys, Start, Time);
	lcObject::RemoveTime(mLightSpotSizeKeys, Start, Time);
/*** LPub3D Mod end ***/
	lcObject::RemoveTime(mSpotCutoffKeys, Start, Time);
	lcObject::RemoveTime(mSpotExponentKeys, Start, Time);
}

void lcLight::UpdatePosition(lcStep Step)
{
	mPosition = CalculateKey(mPositionKeys, Step);
	mTargetPosition = CalculateKey(mTargetPositionKeys, Step);
	mAmbientColor = CalculateKey(mAmbientColorKeys, Step);
	mDiffuseColor = CalculateKey(mDiffuseColorKeys, Step);
	mSpecularColor = CalculateKey(mSpecularColorKeys, Step);
	mAttenuation = CalculateKey(mAttenuationKeys, Step);
/*** LPub3D Mod - enable lights ***/
	mLightShape  = CalculateKey(mLightShapeKeys, Step);
	mLightColor = CalculateKey(mLightColorKeys, Step);
	mLightType = CalculateKey(mLightTypeKeys, Step);
	mLightFactor = CalculateKey(mLightFactorKeys, Step);
	mLightSpecular = CalculateKey(mLightSpecularKeys, Step);
	mSpotSize = CalculateKey(mLightSpotSizeKeys, Step);
/*** LPub3D Mod end ***/
	mSpotCutoff = CalculateKey(mSpotCutoffKeys, Step);
	mSpotExponent = CalculateKey(mSpotExponentKeys, Step);

	if (IsPointLight())
	{
		mWorldLight = lcMatrix44Identity();
		mWorldLight.SetTranslation(-mPosition);
	}
	else
	{
		lcVector3 FrontVector(mTargetPosition - mPosition);
		lcVector3 UpVector(1, 1, 1);

		if (fabs(FrontVector[0]) < fabs(FrontVector[1]))
		{
			if (fabs(FrontVector[0]) < fabs(FrontVector[2]))
				UpVector[0] = -(UpVector[1] * FrontVector[1] + UpVector[2] * FrontVector[2]);
			else
				UpVector[2] = -(UpVector[0] * FrontVector[0] + UpVector[1] * FrontVector[1]);
		}
		else
		{
			if (fabs(FrontVector[1]) < fabs(FrontVector[2]))
				UpVector[1] = -(UpVector[0] * FrontVector[0] + UpVector[2] * FrontVector[2]);
			else
				UpVector[2] = -(UpVector[0] * FrontVector[0] + UpVector[1] * FrontVector[1]);
		}

		mWorldLight = lcMatrix44LookAt(mPosition, mTargetPosition, UpVector);
	}
}

void lcLight::DrawInterface(lcContext* Context, const lcScene& Scene) const
{
	Q_UNUSED(Scene);
	Context->SetMaterial(LC_MATERIAL_UNLIT_COLOR);

	if (IsPointLight())
		DrawPointLight(Context);
/*** LPub3D Mod - enable lights ***/
	else if (IsDirectionalLight())
		DrawDirectionalLight(Context);
/*** LPub3D Mod end ***/
	else
		DrawSpotLight(Context);
}

/*** LPub3D Mod - enable lights ***/
void lcLight::DrawDirectionalLight(lcContext* Context) const
{
	lcVector3 FrontVector(mTargetPosition - mPosition);
	lcVector3 UpVector(1, 1, 1);

	if (fabs(FrontVector[0]) < fabs(FrontVector[1]))
	{
		if (fabs(FrontVector[0]) < fabs(FrontVector[2]))
			UpVector[0] = -(UpVector[1] * FrontVector[1] + UpVector[2] * FrontVector[2]);
		else
			UpVector[2] = -(UpVector[0] * FrontVector[0] + UpVector[1] * FrontVector[1]);
	}
	else
	{
		if (fabs(FrontVector[1]) < fabs(FrontVector[2]))
			UpVector[1] = -(UpVector[0] * FrontVector[0] + UpVector[2] * FrontVector[2]);
		else
			UpVector[2] = -(UpVector[0] * FrontVector[0] + UpVector[1] * FrontVector[1]);
	}

	lcMatrix44 LightMatrix = lcMatrix44LookAt(mPosition, mTargetPosition, UpVector);
	LightMatrix = lcMatrix44AffineInverse(LightMatrix);
	LightMatrix.SetTranslation(lcVector3(0, 0, 0));

	lcMatrix44 LightViewMatrix = lcMul(LightMatrix, lcMatrix44Translation(mPosition));
	Context->SetWorldMatrix(LightViewMatrix);

	float Verts[(20 + 8 + 2 + 16) * 3];
	float* CurVert = Verts;


		for (int EdgeIdx = 0; EdgeIdx < 8; EdgeIdx++)
		{
			float c = cosf(float(EdgeIdx) / 4 * LC_PI) * LC_LIGHT_POSITION_EDGE;
			float s = sinf(float(EdgeIdx) / 4 * LC_PI) * LC_LIGHT_POSITION_EDGE;

			*CurVert++ = c;
			*CurVert++ = s;
			*CurVert++ = LC_LIGHT_POSITION_EDGE;
			*CurVert++ = c;
			*CurVert++ = s;
			*CurVert++ = -LC_LIGHT_POSITION_EDGE;
		}

	if (mLightType == LC_SUNLIGHT) {

		// set base face to same size (LC_LIGHT_TARGET_EDGE) as body - was 12.5f
		*CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
		*CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
		*CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
		*CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;

	} else {

		*CurVert++ = -LC_LIGHT_BASEFACE_EDGE; *CurVert++ = -LC_LIGHT_BASEFACE_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
		*CurVert++ =  LC_LIGHT_BASEFACE_EDGE; *CurVert++ = -LC_LIGHT_BASEFACE_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
		*CurVert++ =  LC_LIGHT_BASEFACE_EDGE; *CurVert++ =  LC_LIGHT_BASEFACE_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
		*CurVert++ = -LC_LIGHT_BASEFACE_EDGE; *CurVert++ =  LC_LIGHT_BASEFACE_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
	}


	float Length = FrontVector.Length();

	*CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE - Length;
	*CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE - Length;
	*CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE - Length;
	*CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE - Length;
	*CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE - Length;
	*CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE - Length;
	*CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE - Length;
	*CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE - Length;

	*CurVert++ = 0.0f; *CurVert++ = 0.0f; *CurVert++ = 0.0f;
	*CurVert++ = 0.0f; *CurVert++ = 0.0f; *CurVert++ = -Length;

	const GLushort Indices[56 + 24 + 2 + 40] =
	{
		// base body
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
		0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14, 0,
		1, 3, 3, 5, 5, 7, 7, 9, 9, 11, 11, 13, 13, 15, 15, 1,
		// base face
		16, 17, 17, 18, 18, 19, 19, 16,
		// targe box
		20, 21, 21, 22, 22, 23, 23, 20,
		24, 25, 25, 26, 26, 27, 27, 24,
		20, 24, 21, 25, 22, 26, 23, 27,
		// target line - from base to target
		28, 29,
	};

	Context->SetVertexBufferPointer(Verts);
	Context->SetVertexFormatPosition(3);
	Context->SetIndexBufferPointer(Indices);

	float LineWidth = lcGetPreferences().mLineWidth;

	if (!IsSelected())
	{
		Context->SetLineWidth(LineWidth);
		Context->SetInterfaceColor(LC_COLOR_LIGHT);

		Context->DrawIndexedPrimitives(GL_LINES, 56 + 24 + 2, GL_UNSIGNED_SHORT, 0);
	}
	else
	{
		if (IsSelected(LC_LIGHT_SECTION_POSITION))
		{
			Context->SetLineWidth(2.0f * LineWidth);
			if (IsFocused(LC_LIGHT_SECTION_POSITION))
				Context->SetInterfaceColor(LC_COLOR_FOCUSED);
			else
				Context->SetInterfaceColor(LC_COLOR_SELECTED);
		}
		else
		{
			Context->SetLineWidth(LineWidth);
			Context->SetInterfaceColor(LC_COLOR_LIGHT);
		}

		Context->DrawIndexedPrimitives(GL_LINES, 56, GL_UNSIGNED_SHORT, 0);

		if (IsSelected(LC_LIGHT_SECTION_TARGET))
		{
			Context->SetLineWidth(2.0f * LineWidth);
			if (IsFocused(LC_LIGHT_SECTION_TARGET))
				Context->SetInterfaceColor(LC_COLOR_FOCUSED);
			else
				Context->SetInterfaceColor(LC_COLOR_SELECTED);
		}
		else
		{
			Context->SetLineWidth(LineWidth);
			Context->SetInterfaceColor(LC_COLOR_LIGHT);
		}

		Context->DrawIndexedPrimitives(GL_LINES, 24, GL_UNSIGNED_SHORT, 56 * 2);

		Context->SetLineWidth(LineWidth);
		Context->SetInterfaceColor(LC_COLOR_LIGHT);

		Context->DrawIndexedPrimitives(GL_LINES, 2 + 40, GL_UNSIGNED_SHORT, (56 + 24) * 2);
	}
}
/*** LPub3D Mod end ***/

void lcLight::DrawSpotLight(lcContext* Context) const
{
	lcVector3 FrontVector(mTargetPosition - mPosition);
	lcVector3 UpVector(1, 1, 1);

	if (fabs(FrontVector[0]) < fabs(FrontVector[1]))
	{
		if (fabs(FrontVector[0]) < fabs(FrontVector[2]))
			UpVector[0] = -(UpVector[1] * FrontVector[1] + UpVector[2] * FrontVector[2]);
		else
			UpVector[2] = -(UpVector[0] * FrontVector[0] + UpVector[1] * FrontVector[1]);
	}
	else
	{
		if (fabs(FrontVector[1]) < fabs(FrontVector[2]))
			UpVector[1] = -(UpVector[0] * FrontVector[0] + UpVector[2] * FrontVector[2]);
		else
			UpVector[2] = -(UpVector[0] * FrontVector[0] + UpVector[1] * FrontVector[1]);
	}

	lcMatrix44 LightMatrix = lcMatrix44LookAt(mPosition, mTargetPosition, UpVector);
	LightMatrix = lcMatrix44AffineInverse(LightMatrix);
	LightMatrix.SetTranslation(lcVector3(0, 0, 0));

	lcMatrix44 LightViewMatrix = lcMul(LightMatrix, lcMatrix44Translation(mPosition));
	Context->SetWorldMatrix(LightViewMatrix);

	float Verts[(20 + 8 + 2 + 16) * 3];
	float* CurVert = Verts;

	for (int EdgeIdx = 0; EdgeIdx < 8; EdgeIdx++)
	{
		float c = cosf((float)EdgeIdx / 4 * LC_PI) * LC_LIGHT_POSITION_EDGE;
		float s = sinf((float)EdgeIdx / 4 * LC_PI) * LC_LIGHT_POSITION_EDGE;

		*CurVert++ = c;
		*CurVert++ = s;
		*CurVert++ = LC_LIGHT_POSITION_EDGE;
		*CurVert++ = c;
		*CurVert++ = s;
		*CurVert++ = -LC_LIGHT_POSITION_EDGE;
	}

	*CurVert++ = -12.5f; *CurVert++ = -12.5f; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
	*CurVert++ =  12.5f; *CurVert++ = -12.5f; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
	*CurVert++ =  12.5f; *CurVert++ =  12.5f; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
	*CurVert++ = -12.5f; *CurVert++ =  12.5f; *CurVert++ = -LC_LIGHT_POSITION_EDGE;

	float Length = FrontVector.Length();

	*CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE - Length;
	*CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE - Length;
	*CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE - Length;
	*CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE - Length;
	*CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE - Length;
	*CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE - Length;
	*CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE - Length;
	*CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE - Length;

	*CurVert++ = 0.0f; *CurVert++ = 0.0f; *CurVert++ = 0.0f;
	*CurVert++ = 0.0f; *CurVert++ = 0.0f; *CurVert++ = -Length;

	const GLushort Indices[56 + 24 + 2 + 40] =
	{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
		0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14, 0,
		1, 3, 3, 5, 5, 7, 7, 9, 9, 11, 11, 13, 13, 15, 15, 1,
		16, 17, 17, 18, 18, 19, 19, 16,
		20, 21, 21, 22, 22, 23, 23, 20,
		24, 25, 25, 26, 26, 27, 27, 24,
		20, 24, 21, 25, 22, 26, 23, 27,
		28, 29,
		30, 31, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38,
		38, 39, 39, 40, 40, 41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 30,
		28, 30, 28, 34, 28, 38, 28, 42
	};

	Context->SetVertexBufferPointer(Verts);
	Context->SetVertexFormatPosition(3);
	Context->SetIndexBufferPointer(Indices);

	float LineWidth = lcGetPreferences().mLineWidth;

	if (!IsSelected())
	{
		Context->SetLineWidth(LineWidth);
		Context->SetInterfaceColor(LC_COLOR_LIGHT);

		Context->DrawIndexedPrimitives(GL_LINES, 56 + 24 + 2, GL_UNSIGNED_SHORT, 0);
	}
	else
	{
		if (IsSelected(LC_LIGHT_SECTION_POSITION))
		{
			Context->SetLineWidth(2.0f * LineWidth);
			if (IsFocused(LC_LIGHT_SECTION_POSITION))
				Context->SetInterfaceColor(LC_COLOR_FOCUSED);
			else
				Context->SetInterfaceColor(LC_COLOR_SELECTED);
		}
		else
		{
			Context->SetLineWidth(LineWidth);
			Context->SetInterfaceColor(LC_COLOR_LIGHT);
		}

		Context->DrawIndexedPrimitives(GL_LINES, 56, GL_UNSIGNED_SHORT, 0);

		if (IsSelected(LC_LIGHT_SECTION_TARGET))
		{
			Context->SetLineWidth(2.0f * LineWidth);
			if (IsFocused(LC_LIGHT_SECTION_TARGET))
				Context->SetInterfaceColor(LC_COLOR_FOCUSED);
			else
				Context->SetInterfaceColor(LC_COLOR_SELECTED);
		}
		else
		{
			Context->SetLineWidth(LineWidth);
			Context->SetInterfaceColor(LC_COLOR_LIGHT);
		}

		Context->DrawIndexedPrimitives(GL_LINES, 24, GL_UNSIGNED_SHORT, 56 * 2);

		Context->SetLineWidth(LineWidth);
		Context->SetInterfaceColor(LC_COLOR_LIGHT);

		float Radius = tanf(LC_DTOR * mSpotCutoff) * Length;

		for (int EdgeIdx = 0; EdgeIdx < 16; EdgeIdx++)
		{
			*CurVert++ = cosf((float)EdgeIdx / 16 * LC_2PI) * Radius;
			*CurVert++ = sinf((float)EdgeIdx / 16 * LC_2PI) * Radius;
			*CurVert++ = -Length;
		}

		Context->DrawIndexedPrimitives(GL_LINES, 2 + 40, GL_UNSIGNED_SHORT, (56 + 24) * 2);
	}
}

void lcLight::DrawPointLight(lcContext* Context) const
{
	const int Slices = 6;
	const int NumIndices = 3 * Slices + 6 * Slices * (Slices - 2) + 3 * Slices;
	const int NumVertices = (Slices - 1) * Slices + 2;
	const float Radius = LC_LIGHT_SPHERE_RADIUS;
	lcVector3 Vertices[NumVertices];
	quint16 Indices[NumIndices];

	lcVector3* Vertex = Vertices;
	quint16* Index = Indices;

	*Vertex++ = lcVector3(0, 0, Radius);

	for (int i = 1; i < Slices; i++ )
	{
		float r0 = Radius * sinf(i * (LC_PI / Slices));
		float z0 = Radius * cosf(i * (LC_PI / Slices));

		for (int j = 0; j < Slices; j++)
		{
			float x0 = r0 * sinf(j * (LC_2PI / Slices));
			float y0 = r0 * cosf(j * (LC_2PI / Slices));

			*Vertex++ = lcVector3(x0, y0, z0);
		}
	}

	*Vertex++ = lcVector3(0, 0, -Radius);

	for (int i = 0; i < Slices - 1; i++ )
	{
		*Index++ = 0;
		*Index++ = 1 + i;
		*Index++ = 1 + i + 1;
	}

	*Index++ = 0;
	*Index++ = 1;
	*Index++ = 1 + Slices - 1;

	for (int i = 0; i < Slices - 2; i++ )
	{
		int Row1 = 1 + i * Slices;
		int Row2 = 1 + (i + 1) * Slices;

		for (int j = 0; j < Slices - 1; j++ )
		{
			*Index++ = Row1 + j;
			*Index++ = Row2 + j + 1;
			*Index++ = Row2 + j;

			*Index++ = Row1 + j;
			*Index++ = Row1 + j + 1;
			*Index++ = Row2 + j + 1;
		}

		*Index++ = Row1 + Slices - 1;
		*Index++ = Row2 + 0;
		*Index++ = Row2 + Slices - 1;

		*Index++ = Row1 + Slices - 1;
		*Index++ = Row2 + 0;
		*Index++ = Row1 + 0;
	}

	for (int i = 0; i < Slices - 1; i++ )
	{
		*Index++ = (Slices - 1) * Slices + 1;
		*Index++ = (Slices - 1) * (Slices - 1) + i;
		*Index++ = (Slices - 1) * (Slices - 1) + i + 1;
	}

	*Index++ = (Slices - 1) * Slices + 1;
	*Index++ = (Slices - 1) * (Slices - 1) + (Slices - 2) + 1;
	*Index++ = (Slices - 1) * (Slices - 1);

	Context->SetWorldMatrix(lcMatrix44Translation(mPosition));

	if (IsFocused(LC_LIGHT_SECTION_POSITION))
		Context->SetInterfaceColor(LC_COLOR_FOCUSED);
	else if (IsSelected(LC_LIGHT_SECTION_POSITION))
		Context->SetInterfaceColor(LC_COLOR_SELECTED);
	else
		Context->SetInterfaceColor(LC_COLOR_LIGHT);

	Context->SetVertexBufferPointer(Vertices);
	Context->SetVertexFormatPosition(3);
	Context->SetIndexBufferPointer(Indices);
	Context->DrawIndexedPrimitives(GL_TRIANGLES, NumIndices, GL_UNSIGNED_SHORT, 0);
}

void lcLight::RemoveKeyFrames()
{
	mPositionKeys.RemoveAll();
	ChangeKey(mPositionKeys, mPosition, 1, true);

	mTargetPositionKeys.RemoveAll();
	ChangeKey(mTargetPositionKeys, mTargetPosition, 1, true);

	mAmbientColorKeys.RemoveAll();
	ChangeKey(mAmbientColorKeys, mAmbientColor, 1, true);

	mDiffuseColorKeys.RemoveAll();
	ChangeKey(mDiffuseColorKeys, mDiffuseColor, 1, true);

	mSpecularColorKeys.RemoveAll();
	ChangeKey(mSpecularColorKeys, mSpecularColor, 1, true);
/*** LPub3D Mod - enable lights ***/
	mLightShapeKeys.RemoveAll();
	ChangeKey(mLightShapeKeys, mLightShape, 1, false);

	mLightColorKeys.RemoveAll();
	ChangeKey(mLightColorKeys, mLightColor, 1, true);

	mLightFactorKeys.RemoveAll();
	ChangeKey(mLightFactorKeys, mLightFactor, 1, true);

	mLightTypeKeys.RemoveAll();
	ChangeKey(mLightTypeKeys, mLightType, 1, true);

	mLightSpecularKeys.RemoveAll();
	ChangeKey(mLightSpecularKeys, mLightSpecular, 1, true);

	mLightSpotSizeKeys.RemoveAll();
	ChangeKey(mLightSpotSizeKeys, mSpotSize, 1, false);
/*** LPub3D Mod end ***/
	mAttenuationKeys.RemoveAll();
	ChangeKey(mAttenuationKeys, mAttenuation, 1, true);

	mSpotCutoffKeys.RemoveAll();
	ChangeKey(mSpotCutoffKeys, mSpotCutoff, 1, true);

	mSpotExponentKeys.RemoveAll();
	ChangeKey(mSpotExponentKeys, mSpotExponent, 1, true);
}

bool lcLight::Setup(int LightIndex)
{
	Q_UNUSED(LightIndex);

	return true;
}
