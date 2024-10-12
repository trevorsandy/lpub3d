#pragma once

enum class lcObjectPropertyId
{
	PieceId,
	PieceColor,
/*** LPub3D Mod - LPUB meta properties ***/
	PieceType,
	PieceFileID,
	PieceModel,
	PieceIsSubmodel,
/*** LPub3D Mod end ***/
	PieceStepShow,
	PieceStepHide,
	CameraName,
	CameraType,
	CameraFOV,
	CameraNear,
	CameraFar,
/*** LPub3D Mod - LPUB meta properties ***/
	CameraLatitude,
	CameraLongitude,
	CameraDistance,
/*** LPub3D Mod end ***/
	CameraPositionX,
	CameraPositionY,
	CameraPositionZ,
	CameraTargetX,
	CameraTargetY,
	CameraTargetZ,
	CameraUpX,
	CameraUpY,
	CameraUpZ,
/*** LPub3D Mod - LPUB meta properties ***/
	CameraImageScale,
	CameraImageResolution,
	CameraImageWidth,
	CameraImageHeight,
	CameraImagePageWidth,
	CameraImagePageHeight,
/*** LPub3D Mod end ***/
	LightName,
	LightType,
/*** LPub3D Mod - LPUB meta properties ***/
	LightFormat,
/*** LPub3D Mod end ***/
	LightColor,
	LightBlenderPower,
/*** LPub3D Mod - LPUB meta properties ***/
	LightBlenderCutoffDistance,
	LightBlenderDiffuse,
	LightBlenderSpecular,
/*** LPub3D Mod end ***/
	LightPOVRayPower,
	LightCastShadow,
	LightPOVRayFadeDistance,
	LightPOVRayFadePower,
	LightPointBlenderRadius,
	LightSpotBlenderRadius,
	LightDirectionalBlenderAngle,
	LightAreaSizeX,
	LightAreaSizeY,
	LightSpotConeAngle,
	LightSpotPenumbraAngle,
	LightSpotPOVRayTightness,
	LightAreaShape,
	LightAreaPOVRayGridX,
	LightAreaPOVRayGridY,
	ObjectPositionX,
	ObjectPositionY,
	ObjectPositionZ,
	ObjectRotationX,
	ObjectRotationY,
	ObjectRotationZ,
	Count
};

template<typename T>
struct lcObjectPropertyKey
{
	lcStep Step;
	T Value;
};

template<typename T>
class lcObjectProperty
{
public:
	explicit lcObjectProperty(const T& DefaultValue)
		: mValue(DefaultValue)
	{
	}

	operator const T& () const
	{
		return mValue;
	}

	void SetValue(const T& Value)
	{
		mValue = Value;
	}

	void RemoveAllKeys()
	{
		mKeys.clear();
	}

	void Update(lcStep Step);
	bool ChangeKey(const T& Value, lcStep Step, bool AddKey);
	void InsertTime(lcStep Start, lcStep Time);
	void RemoveTime(lcStep Start, lcStep Time);
	bool HasKeyFrame(lcStep Time) const;
	bool SetKeyFrame(lcStep Time, bool KeyFrame);

/*** LPub3D Mod - LPUB meta command ***/
	void Save(QTextStream& Stream, const char* ObjectName, const char* VariableName, bool SaveEmpty, bool LPubMeta = true) const;
	bool Load(QTextStream& Stream, const QString& Token, const char* VariableName, bool LPubMeta = true);
/*** LPub3D Mod end ***/

protected:
	T mValue;
	std::vector<lcObjectPropertyKey<T>> mKeys;
};
