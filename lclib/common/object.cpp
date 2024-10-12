#include "lc_global.h"
#include "object.h"

lcObject::lcObject(lcObjectType ObjectType)
	: mObjectType(ObjectType)
{
}

lcObject::~lcObject()
{
}

QString lcObject::GetCheckpointString(lcObjectPropertyId PropertyId)
{
	switch (PropertyId)
	{
	case lcObjectPropertyId::PieceId:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Piece Id");

	case lcObjectPropertyId::PieceColor:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Piece Color");

/*** LPub3D Mod - LPUB meta properties ***/
	case lcObjectPropertyId::PieceType:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Piece Type");

	case lcObjectPropertyId::PieceFileID:
	case lcObjectPropertyId::PieceModel:
	case lcObjectPropertyId::PieceIsSubmodel:
/*** LPub3D Mod end ***/
	case lcObjectPropertyId::PieceStepShow:
	case lcObjectPropertyId::PieceStepHide:
		break;

	case lcObjectPropertyId::CameraName:
		return QT_TRANSLATE_NOOP("Checkpoint", "Renaming Camera");

	case lcObjectPropertyId::CameraType:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Camera Type");

	case lcObjectPropertyId::CameraFOV:
	case lcObjectPropertyId::CameraNear:
	case lcObjectPropertyId::CameraFar:
/*** LPub3D Mod - LPUB meta properties ***/
	case lcObjectPropertyId::CameraLatitude:
	case lcObjectPropertyId::CameraLongitude:
	case lcObjectPropertyId::CameraDistance:
/*** LPub3D Mod end ***/	
	case lcObjectPropertyId::CameraPositionX:
	case lcObjectPropertyId::CameraPositionY:
	case lcObjectPropertyId::CameraPositionZ:
	case lcObjectPropertyId::CameraTargetX:
	case lcObjectPropertyId::CameraTargetY:
	case lcObjectPropertyId::CameraTargetZ:
	case lcObjectPropertyId::CameraUpX:
	case lcObjectPropertyId::CameraUpY:
	case lcObjectPropertyId::CameraUpZ:
/*** LPub3D Mod - LPUB meta properties ***/
	case lcObjectPropertyId::CameraImageScale:
	case lcObjectPropertyId::CameraImageResolution:
	case lcObjectPropertyId::CameraImageWidth:
	case lcObjectPropertyId::CameraImageHeight:
	case lcObjectPropertyId::CameraImagePageWidth:
	case lcObjectPropertyId::CameraImagePageHeight:
/*** LPub3D Mod end ***/	
		break;

	case lcObjectPropertyId::LightName:
		return QT_TRANSLATE_NOOP("Checkpoint", "Renaming Light");

	case lcObjectPropertyId::LightType:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Light Type");

/*** LPub3D Mod - LPUB meta properties ***/
	case lcObjectPropertyId::LightFormat:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Light Format");
/*** LPub3D Mod end ***/

	case lcObjectPropertyId::LightColor:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Light Color");

	case lcObjectPropertyId::LightBlenderPower:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Light Blender Power");

/*** LPub3D Mod - LPUB meta properties ***/
	case lcObjectPropertyId::LightBlenderCutoffDistance:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Light Blender Cutoff Distance");

	case lcObjectPropertyId::LightBlenderDiffuse:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Light Blender Diffuse Factor");
		
	case lcObjectPropertyId::LightBlenderSpecular:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Light Blender Specular Factor");
/*** LPub3D Mod end ***/
	
	case lcObjectPropertyId::LightPOVRayPower:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Light POV-Ray Power");

	case lcObjectPropertyId::LightCastShadow:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Light Shadow");

	case lcObjectPropertyId::LightPOVRayFadeDistance:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Light POV-Ray Fade Distance");

	case lcObjectPropertyId::LightPOVRayFadePower:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Light POV-Ray Fade Power");

	case lcObjectPropertyId::LightPointBlenderRadius:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Point Light Blender Radius");

	case lcObjectPropertyId::LightSpotBlenderRadius:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Light Spot Light Blender Radius");

	case lcObjectPropertyId::LightDirectionalBlenderAngle:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Directional Light Blender Angle");

	case lcObjectPropertyId::LightAreaSizeX:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Light Area X Size");

	case lcObjectPropertyId::LightAreaSizeY:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Light Area Y Size");

	case lcObjectPropertyId::LightSpotConeAngle:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Spot Light Cone Angle");

	case lcObjectPropertyId::LightSpotPenumbraAngle:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Spot Light Penumbra Angle");

	case lcObjectPropertyId::LightSpotPOVRayTightness:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Spot Light POV-Ray Tightness");

	case lcObjectPropertyId::LightAreaShape:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Area Light Shape");

	case lcObjectPropertyId::LightAreaPOVRayGridX:
	case lcObjectPropertyId::LightAreaPOVRayGridY:
		return QT_TRANSLATE_NOOP("Checkpoint", "Changing Area Light Grid");

	case lcObjectPropertyId::ObjectPositionX:
	case lcObjectPropertyId::ObjectPositionY:
	case lcObjectPropertyId::ObjectPositionZ:
	case lcObjectPropertyId::ObjectRotationX:
	case lcObjectPropertyId::ObjectRotationY:
	case lcObjectPropertyId::ObjectRotationZ:
	case lcObjectPropertyId::Count:
		break;
	}

	return QString();
}
