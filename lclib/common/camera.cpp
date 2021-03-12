#include "lc_global.h"
#include "lc_math.h"
#include "lc_colors.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "lc_file.h"
#include "camera.h"
#include "lc_application.h"
#include "lc_context.h"
/*** LPub3D Mod - Camera Globe ***/
#include "project.h"
/*** LPub3D Mod end ***/

#define LC_CAMERA_POSITION_EDGE 7.5f
#define LC_CAMERA_TARGET_EDGE 7.5f

#define LC_CAMERA_SAVE_VERSION 7 // LeoCAD 0.80

/*** LPub3D Mod - LPUB meta command ***/
lcCamera::lcCamera(bool Simple, bool LPubMeta)
	: lcObject(lcObjectType::Camera), mLPubMeta(LPubMeta)
{
/*** LPub3D Mod end ***/
	Initialize();

	if (Simple)
		mState |= LC_CAMERA_SIMPLE;
	else
	{
/*** LPub3D Mod - Update Default Camera ***/
		float PP = GetCDP() / GetDDF();
		mPosition = lcVector3(-PP, -PP, 75.0f);
/*** LPub3D Mod end ***/
		mTargetPosition = lcVector3(0.0f, 0.0f, 0.0f);
		mUpVector = lcVector3(-0.2357f, -0.2357f, 0.94281f);

		mPositionKeys.ChangeKey(mPosition, 1, true);
		mTargetPositionKeys.ChangeKey(mTargetPosition, 1, true);
		mUpVectorKeys.ChangeKey(mUpVector, 1, true);

		UpdatePosition(1);
	}
}

lcCamera::lcCamera(float ex, float ey, float ez, float tx, float ty, float tz)
/*** LPub3D Mod - LPUB meta command ***/
	: lcObject(lcObjectType::Camera), mLPubMeta(true)
/*** LPub3D Mod end ***/
{
	// Fix the up vector
	lcVector3 UpVector(0, 0, 1), FrontVector(ex - tx, ey - ty, ez - tz), SideVector;
	FrontVector.Normalize();
	if (FrontVector == UpVector)
		SideVector = lcVector3(1, 0, 0);
	else
		SideVector = lcCross(FrontVector, UpVector);
	UpVector = lcCross(SideVector, FrontVector);
	UpVector.Normalize();

	Initialize();

	mPositionKeys.ChangeKey(lcVector3(ex, ey, ez), 1, true);
	mTargetPositionKeys.ChangeKey(lcVector3(tx, ty, tz), 1, true);
	mUpVectorKeys.ChangeKey(UpVector, 1, true);

	UpdatePosition(1);
}

lcCamera::~lcCamera()
{
}

lcViewpoint lcCamera::GetViewpoint(const QString& ViewpointName)
{
	const QLatin1String ViewpointNames[] =
	{
		QLatin1String("front"),
		QLatin1String("back"),
		QLatin1String("top"),
		QLatin1String("bottom"),
		QLatin1String("left"),
		QLatin1String("right"),
		QLatin1String("home")
	};

	LC_ARRAY_SIZE_CHECK(ViewpointNames, lcViewpoint::Count);

	for (int ViewpointIndex = 0; ViewpointIndex < static_cast<int>(lcViewpoint::Count); ViewpointIndex++)
		if (ViewpointNames[ViewpointIndex] == ViewpointName)
			return static_cast<lcViewpoint>(ViewpointIndex);

	return lcViewpoint::Count;
}

void lcCamera::Initialize()
{
/*** LPub3D Mod - LPUB meta command ***/
	m_fovy = gApplication->mPreferences.mCFoV;
	m_zNear = gApplication->mPreferences.mCNear;
	m_zFar = gApplication->mPreferences.mCFar;
/*** LPub3D Mod end ***/
	mState = 0;
}

void lcCamera::SetName(const QString& Name)
{
	mName = Name;
}

void lcCamera::CreateName(const lcArray<lcCamera*>& Cameras)
{
	if (!mName.isEmpty())
	{
		bool Found = false;

		for (const lcCamera* Camera : Cameras)
		{
			if (Camera->GetName() == mName)
			{
				Found = true;
				break;
			}
		}

		if (!Found)
			return;
	}

	int MaxCameraNumber = 0;
	const QLatin1String Prefix("Camera ");

	for (const lcCamera* Camera : Cameras)
	{
		QString CameraName = Camera->GetName();

		if (CameraName.startsWith(Prefix))
		{
			bool Ok = false;
			int CameraNumber = CameraName.midRef(Prefix.size()).toInt(&Ok);

			if (Ok && CameraNumber > MaxCameraNumber)
				MaxCameraNumber = CameraNumber;
		}
	}

	mName = Prefix + QString::number(MaxCameraNumber + 1);
}

void lcCamera::SaveLDraw(QTextStream& Stream) const
{
	QLatin1String LineEnding("\r\n");
/*** LPub3D Mod - LPUB meta command ***/
	lcVector3 Vector;
	QString Meta(mLPubMeta ? "!LPUB" : "!LEOCAD");
	Stream << QLatin1String(QString("0 %1 CAMERA FOV ").arg(Meta).toLatin1()) << m_fovy << QLatin1String(" ZNEAR ") << m_zNear << QLatin1String(" ZFAR ") << m_zFar << LineEnding;
/*** LPub3D Mod end ***/

	if (mPositionKeys.GetSize() > 1)
		mPositionKeys.SaveKeysLDraw(Stream, "CAMERA POSITION_KEY ");
	else
/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
	{
		Vector = mLPubMeta ? lcVector3LeoCADToLDraw(mPosition) : mPosition;
		Stream << QLatin1String(QString("0 %1 CAMERA POSITION ").arg(Meta).toLatin1()) << Vector[0] << ' ' << Vector[1] << ' ' << Vector[2] << LineEnding;
	}
/*** LPub3D Mod end ***/

	if (mTargetPositionKeys.GetSize() > 1)
		mTargetPositionKeys.SaveKeysLDraw(Stream, "CAMERA TARGET_POSITION_KEY ");
	else
/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
	{
		Vector = mLPubMeta ? lcVector3LeoCADToLDraw(mTargetPosition) : mTargetPosition;
		Stream << QLatin1String(QString("0 %1 CAMERA TARGET_POSITION ").arg(Meta).toLatin1()) << Vector[0] << ' ' << Vector[1] << ' ' << Vector[2] << LineEnding;
	}
/*** LPub3D Mod end ***/

	if (mUpVectorKeys.GetSize() > 1)
		mUpVectorKeys.SaveKeysLDraw(Stream, "CAMERA UP_VECTOR_KEY ");
	else
/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
	{
		Vector = mLPubMeta ? lcVector3LeoCADToLDraw(mUpVector) : mUpVector;
		Stream << QLatin1String(QString("0 %1 CAMERA UP_VECTOR ").arg(Meta).toLatin1()) << Vector[0] << ' ' << Vector[1] << ' ' << Vector[2] << LineEnding;
	}
/*** LPub3D Mod end ***/

/*** LPub3D Mod - LPUB meta command ***/
	Stream << QLatin1String(QString("0 %1 CAMERA ").arg(mLPubMeta ? "!LPUB" : "!LEOCAD").toLatin1());
/*** LPub3D Mod end ***/

	if (IsHidden())
		Stream << QLatin1String("HIDDEN");

	if (IsOrtho())
		Stream << QLatin1String("ORTHOGRAPHIC ");

	Stream << QLatin1String("NAME ") << mName << LineEnding;
}

bool lcCamera::ParseLDrawLine(QTextStream& Stream)
{
/*** LPub3D Mod - Camera Globe ***/
	bool latOk = false, lonOk = false, tarOk = false;
	float Lat = 23.0f, Lon = 45.0f, Dist = 1.0f;
/*** LPub3D Mod end ***/

	while (!Stream.atEnd())
	{
		QString Token;
		Stream >> Token;

		if (Token == QLatin1String("HIDDEN"))
			SetHidden(true);
		else if (Token == QLatin1String("ORTHOGRAPHIC"))
			SetOrtho(true);
/*** LPub3D Mod - Camera Globe ***/
		else if ((latOk = Token == QLatin1String("LATITUDE")))
			Stream >> Lat;
		else if ((lonOk = Token == QLatin1String("LONGITUDE")))
			Stream >> Lon;
		else if (Token == QLatin1String("DISTANCE"))
			Stream >> Dist;
/*** LPub3D Mod end ***/
		else if (Token == QLatin1String("FOV"))
			Stream >> m_fovy;
		else if (Token == QLatin1String("ZNEAR"))
			Stream >> m_zNear;
		else if (Token == QLatin1String("ZFAR"))
			Stream >> m_zFar;
		else if (Token == QLatin1String("POSITION"))
		{
/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Z(LD -Y) in the up direction ***/
			Stream >> mPosition[0] >> mPosition[1] >> mPosition[2];
			if (mLPubMeta)
				mPosition = lcVector3LDrawToLeoCAD(mPosition);
/*** LPub3D Mod end ***/
			mPositionKeys.ChangeKey(mPosition, 1, true);
		}
/*** LPub3D Mod - Camera Globe ***/
		else if ((tarOk = Token == QLatin1String("TARGET_POSITION")))
/*** LPub3D Mod end ***/
		{
/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Z(LD -Y) in the up direction ***/
			Stream >> mTargetPosition[0] >> mTargetPosition[1] >> mTargetPosition[2];
			if (mLPubMeta)
				mTargetPosition = lcVector3LDrawToLeoCAD(mTargetPosition);
/*** LPub3D Mod end ***/
			mTargetPositionKeys.ChangeKey(mTargetPosition, 1, true);
		}
		else if (Token == QLatin1String("UP_VECTOR"))
		{
/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Z(LD -Y) in the up direction ***/
			Stream >> mUpVector[0] >> mUpVector[1] >> mUpVector[2];
			if (mLPubMeta)
				mUpVector = lcVector3LDrawToLeoCAD(mUpVector);
/*** LPub3D Mod end ***/
			mUpVectorKeys.ChangeKey(mUpVector, 1, true);
		}
		else if (Token == QLatin1String("POSITION_KEY"))
			mPositionKeys.LoadKeysLDraw(Stream);
		else if (Token == QLatin1String("TARGET_POSITION_KEY"))
			mTargetPositionKeys.LoadKeysLDraw(Stream);
		else if (Token == QLatin1String("UP_VECTOR_KEY"))
			mUpVectorKeys.LoadKeysLDraw(Stream);
		else if (Token == QLatin1String("NAME"))
		{
			mName = Stream.readAll().trimmed();
			return true;
		}
	}

/*** LPub3D Mod - Camera Globe ***/
	if (latOk && lonOk && tarOk)
		SetAngles(Lat, Lon, Dist, mTargetPosition);
	else if (latOk && lonOk)
		SetAngles(Lat, Lon, Dist);
/*** LPub3D Mod end ***/

	return false;
}

/////////////////////////////////////////////////////////////////////////////
// Camera save/load

bool lcCamera::FileLoad(lcFile& file)
{
	quint8 version, ch;

	version = file.ReadU8();

	if (version > LC_CAMERA_SAVE_VERSION)
		return false;

	if (version > 5)
	{
		if (file.ReadU8() != 1)
			return false;

		quint16 time;
		float param[4];
		quint8 type;
		quint32 n;

		file.ReadU32(&n, 1);
		while (n--)
		{
			file.ReadU16(&time, 1);
			file.ReadFloats(param, 4);
			file.ReadU8(&type, 1);
		}

		file.ReadU32(&n, 1);
		while (n--)
		{
			file.ReadU16(&time, 1);
			file.ReadFloats(param, 4);
			file.ReadU8(&type, 1);
		}
	}

	if (version == 4)
	{
		char Name[81];
		file.ReadBuffer(Name, 80);
	}
	else
	{
		ch = file.ReadU8();
		if (ch == 0xFF)
			return false;
		char Name[81];
		file.ReadBuffer(Name, ch);
	}

	if (version < 3)
	{
		double d[3];

		file.ReadDoubles(d, 3);
		file.ReadDoubles(d, 3);
		file.ReadDoubles(d, 3);
	}

	if (version == 3)
	{
		ch = file.ReadU8();

		while (ch--)
		{
			quint8 step;
			double eye[3], target[3], up[3];

			file.ReadDoubles(eye, 3);
			file.ReadDoubles(target, 3);
			file.ReadDoubles(up, 3);
			file.ReadU8(&step, 1);

			file.ReadS32(); // snapshot
			file.ReadS32(); // cam
		}
	}

	if (version < 4)
	{
		file.ReadDouble(); // m_fovy
		file.ReadDouble(); // m_zFar
		file.ReadDouble(); // m_zNear
	}
	else
	{
		qint32 n;

		if (version < 6)
		{
			quint16 time;
			float param[4];
			quint8 type;

			n = file.ReadS32();
			while (n--)
			{
				file.ReadU16(&time, 1);
				file.ReadFloats(param, 3);
				file.ReadU8(&type, 1);
			}

			n = file.ReadS32();
			while (n--)
			{
				file.ReadU16(&time, 1);
				file.ReadFloats(param, 3);
				file.ReadU8(&type, 1);
			}
		}

		float f;
		file.ReadFloats(&f, 1); // m_fovy
		file.ReadFloats(&f, 1); // m_zFar
		file.ReadFloats(&f, 1); // m_zNear

		if (version < 5)
		{
			n = file.ReadS32();
		}
		else
		{
			ch = file.ReadU8();
			file.ReadU8();
		}
	}

	if ((version > 1) && (version < 4))
	{
		quint32 show;
		qint32 user;

		file.ReadU32(&show, 1);
//		if (version > 2)
		file.ReadS32(&user, 1);
	}

	return true;
}

void lcCamera::CompareBoundingBox(lcVector3& Min, lcVector3& Max)
{
	const lcVector3 Points[2] =
	{
		mPosition, mTargetPosition
	};

	for (int i = 0; i < 2; i++)
	{
		const lcVector3& Point = Points[i];

		// TODO: this should check the entire mesh

		Min = lcMin(Point, Min);
		Max = lcMax(Point, Max);
	}
}

void lcCamera::MoveSelected(lcStep Step, bool AddKey, const lcVector3& Distance)
{
	if (IsSimple())
		AddKey = false;

	if (IsSelected(LC_CAMERA_SECTION_POSITION))
	{
		mPosition += Distance;
		mPositionKeys.ChangeKey(mPosition, Step, AddKey);
	}

	if (IsSelected(LC_CAMERA_SECTION_TARGET))
	{
		mTargetPosition += Distance;
		mTargetPositionKeys.ChangeKey(mTargetPosition, Step, AddKey);
	}
	else if (IsSelected(LC_CAMERA_SECTION_UPVECTOR))
	{
		mUpVector += Distance;
		mUpVector.Normalize();
		mUpVectorKeys.ChangeKey(mUpVector, Step, AddKey);
	}

	lcVector3 FrontVector(mTargetPosition - mPosition);
	lcVector3 SideVector = lcCross(FrontVector, mUpVector);

	if (fabsf(lcDot(mUpVector, SideVector)) > 0.99f)
		SideVector = lcVector3(1, 0, 0);

	mUpVector = lcCross(SideVector, FrontVector);
	mUpVector.Normalize();
}

void lcCamera::MoveRelative(const lcVector3& Distance, lcStep Step, bool AddKey)
{
	if (IsSimple())
		AddKey = false;

	lcVector3 Relative = lcMul30(Distance, lcMatrix44Transpose(mWorldView)) * 5.0f;

	mPosition += Relative;
	mPositionKeys.ChangeKey(mPosition, Step, AddKey);

	mTargetPosition += Relative;
	mTargetPositionKeys.ChangeKey(mTargetPosition, Step, AddKey);

	UpdatePosition(Step);
}

void lcCamera::UpdatePosition(lcStep Step)
{
	if (!IsSimple())
	{
		mPosition = mPositionKeys.CalculateKey(Step);
		mTargetPosition = mTargetPositionKeys.CalculateKey(Step);
		mUpVector = mUpVectorKeys.CalculateKey(Step);
	}

	lcVector3 FrontVector(mPosition - mTargetPosition);
	lcVector3 SideVector = lcCross(FrontVector, mUpVector);
	mUpVector = lcNormalize(lcCross(SideVector, FrontVector));

	mWorldView = lcMatrix44LookAt(mPosition, mTargetPosition, mUpVector);
}

void lcCamera::CopyPosition(const lcCamera* Camera)
{
	m_fovy = Camera->m_fovy;
	m_zNear = Camera->m_zNear;
	m_zFar = Camera->m_zFar;

	mWorldView = Camera->mWorldView;
	mPosition = Camera->mPosition;
	mTargetPosition = Camera->mTargetPosition;
	mUpVector = Camera->mUpVector;
	mState |= (Camera->mState & LC_CAMERA_ORTHO);
}

void lcCamera::CopySettings(const lcCamera* camera)
{
	m_fovy = camera->m_fovy;
	m_zNear = camera->m_zNear;
	m_zFar = camera->m_zFar;

	mState |= (camera->mState & LC_CAMERA_ORTHO);
}

void lcCamera::DrawInterface(lcContext* Context, const lcScene& Scene) const
{
	Q_UNUSED(Scene);
	Context->SetMaterial(lcMaterialType::UnlitColor);

	lcMatrix44 ViewWorldMatrix = lcMatrix44AffineInverse(mWorldView);
	ViewWorldMatrix.SetTranslation(lcVector3(0, 0, 0));

	lcMatrix44 CameraViewMatrix = lcMul(ViewWorldMatrix, lcMatrix44Translation(mPosition));
	Context->SetWorldMatrix(CameraViewMatrix);

	float Verts[(12 + 8 + 8 + 3 + 4) * 3];
	float* CurVert = Verts;

	float Length = lcLength(mPosition - mTargetPosition);

	*CurVert++ =  LC_CAMERA_POSITION_EDGE; *CurVert++ =  LC_CAMERA_POSITION_EDGE; *CurVert++ =  LC_CAMERA_POSITION_EDGE;
	*CurVert++ = -LC_CAMERA_POSITION_EDGE; *CurVert++ =  LC_CAMERA_POSITION_EDGE; *CurVert++ =  LC_CAMERA_POSITION_EDGE;
	*CurVert++ = -LC_CAMERA_POSITION_EDGE; *CurVert++ = -LC_CAMERA_POSITION_EDGE; *CurVert++ =  LC_CAMERA_POSITION_EDGE;
	*CurVert++ =  LC_CAMERA_POSITION_EDGE; *CurVert++ = -LC_CAMERA_POSITION_EDGE; *CurVert++ =  LC_CAMERA_POSITION_EDGE;
	*CurVert++ =  LC_CAMERA_POSITION_EDGE; *CurVert++ =  LC_CAMERA_POSITION_EDGE; *CurVert++ = -LC_CAMERA_POSITION_EDGE;
	*CurVert++ = -LC_CAMERA_POSITION_EDGE; *CurVert++ =  LC_CAMERA_POSITION_EDGE; *CurVert++ = -LC_CAMERA_POSITION_EDGE;
	*CurVert++ = -LC_CAMERA_POSITION_EDGE; *CurVert++ = -LC_CAMERA_POSITION_EDGE; *CurVert++ = -LC_CAMERA_POSITION_EDGE;
	*CurVert++ =  LC_CAMERA_POSITION_EDGE; *CurVert++ = -LC_CAMERA_POSITION_EDGE; *CurVert++ = -LC_CAMERA_POSITION_EDGE;
	*CurVert++ =  LC_CAMERA_POSITION_EDGE; *CurVert++ =  LC_CAMERA_POSITION_EDGE; *CurVert++ = -LC_CAMERA_POSITION_EDGE * 2;
	*CurVert++ = -LC_CAMERA_POSITION_EDGE; *CurVert++ =  LC_CAMERA_POSITION_EDGE; *CurVert++ = -LC_CAMERA_POSITION_EDGE * 2;
	*CurVert++ = -LC_CAMERA_POSITION_EDGE; *CurVert++ = -LC_CAMERA_POSITION_EDGE; *CurVert++ = -LC_CAMERA_POSITION_EDGE * 2;
	*CurVert++ =  LC_CAMERA_POSITION_EDGE; *CurVert++ = -LC_CAMERA_POSITION_EDGE; *CurVert++ = -LC_CAMERA_POSITION_EDGE * 2;

	*CurVert++ =  LC_CAMERA_TARGET_EDGE; *CurVert++ =  LC_CAMERA_TARGET_EDGE; *CurVert++ =  LC_CAMERA_TARGET_EDGE - Length;
	*CurVert++ = -LC_CAMERA_TARGET_EDGE; *CurVert++ =  LC_CAMERA_TARGET_EDGE; *CurVert++ =  LC_CAMERA_TARGET_EDGE - Length;
	*CurVert++ = -LC_CAMERA_TARGET_EDGE; *CurVert++ = -LC_CAMERA_TARGET_EDGE; *CurVert++ =  LC_CAMERA_TARGET_EDGE - Length;
	*CurVert++ =  LC_CAMERA_TARGET_EDGE; *CurVert++ = -LC_CAMERA_TARGET_EDGE; *CurVert++ =  LC_CAMERA_TARGET_EDGE - Length;
	*CurVert++ =  LC_CAMERA_TARGET_EDGE; *CurVert++ =  LC_CAMERA_TARGET_EDGE; *CurVert++ = -LC_CAMERA_TARGET_EDGE - Length;
	*CurVert++ = -LC_CAMERA_TARGET_EDGE; *CurVert++ =  LC_CAMERA_TARGET_EDGE; *CurVert++ = -LC_CAMERA_TARGET_EDGE - Length;
	*CurVert++ = -LC_CAMERA_TARGET_EDGE; *CurVert++ = -LC_CAMERA_TARGET_EDGE; *CurVert++ = -LC_CAMERA_TARGET_EDGE - Length;
	*CurVert++ =  LC_CAMERA_TARGET_EDGE; *CurVert++ = -LC_CAMERA_TARGET_EDGE; *CurVert++ = -LC_CAMERA_TARGET_EDGE - Length;

	*CurVert++ =  LC_CAMERA_TARGET_EDGE; *CurVert++ =  LC_CAMERA_TARGET_EDGE + 25.0f; *CurVert++ =  LC_CAMERA_TARGET_EDGE;
	*CurVert++ = -LC_CAMERA_TARGET_EDGE; *CurVert++ =  LC_CAMERA_TARGET_EDGE + 25.0f; *CurVert++ =  LC_CAMERA_TARGET_EDGE;
	*CurVert++ = -LC_CAMERA_TARGET_EDGE; *CurVert++ = -LC_CAMERA_TARGET_EDGE + 25.0f; *CurVert++ =  LC_CAMERA_TARGET_EDGE;
	*CurVert++ =  LC_CAMERA_TARGET_EDGE; *CurVert++ = -LC_CAMERA_TARGET_EDGE + 25.0f; *CurVert++ =  LC_CAMERA_TARGET_EDGE;
	*CurVert++ =  LC_CAMERA_TARGET_EDGE; *CurVert++ =  LC_CAMERA_TARGET_EDGE + 25.0f; *CurVert++ = -LC_CAMERA_TARGET_EDGE;
	*CurVert++ = -LC_CAMERA_TARGET_EDGE; *CurVert++ =  LC_CAMERA_TARGET_EDGE + 25.0f; *CurVert++ = -LC_CAMERA_TARGET_EDGE;
	*CurVert++ = -LC_CAMERA_TARGET_EDGE; *CurVert++ = -LC_CAMERA_TARGET_EDGE + 25.0f; *CurVert++ = -LC_CAMERA_TARGET_EDGE;
	*CurVert++ =  LC_CAMERA_TARGET_EDGE; *CurVert++ = -LC_CAMERA_TARGET_EDGE + 25.0f; *CurVert++ = -LC_CAMERA_TARGET_EDGE;

	*CurVert++ = 0.0f; *CurVert++ = 0.0f; *CurVert++ = 0.0f;
	*CurVert++ = 0.0f; *CurVert++ = 0.0f; *CurVert++ = -Length;
	*CurVert++ = 0.0f; *CurVert++ = 25.0f; *CurVert++ = 0.0f;

	const GLushort Indices[40 + 24 + 24 + 4 + 16] =
	{
		0, 1, 1, 2, 2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 4, 1, 5, 2, 6, 3, 7,
		8, 9, 9, 10, 10, 11, 11, 8,
		8, 28, 9, 28, 10, 28, 11, 28,
		12, 13, 13, 14, 14, 15, 15, 12,
		16, 17, 17, 18, 18, 19, 19, 16,
		12, 16, 13, 17, 14, 18, 15, 19,
		20, 21, 21, 22, 22, 23, 23, 20,
		24, 25, 25, 26, 26, 27, 27, 24,
		20, 24, 21, 25, 22, 26, 23, 27,
		28, 29, 28, 30,
		31, 32, 32, 33, 33, 34, 34, 31,
		28, 31, 28, 32, 28, 33, 28, 34
	};

	Context->SetVertexBufferPointer(Verts);
	Context->SetVertexFormatPosition(3);
	Context->SetIndexBufferPointer(Indices);

	float LineWidth = lcGetPreferences().mLineWidth;

	if (!IsSelected())
	{
		Context->SetLineWidth(LineWidth);
		Context->SetInterfaceColor(LC_COLOR_CAMERA);

		Context->DrawIndexedPrimitives(GL_LINES, 40 + 24 + 24 + 4, GL_UNSIGNED_SHORT, 0);
	}
	else
	{
		if (IsSelected(LC_CAMERA_SECTION_POSITION))
		{
			Context->SetLineWidth(2.0f * LineWidth);
			if (IsFocused(LC_CAMERA_SECTION_POSITION))
				Context->SetInterfaceColor(LC_COLOR_FOCUSED);
			else
				Context->SetInterfaceColor(LC_COLOR_SELECTED);
		}
		else
		{
			Context->SetLineWidth(LineWidth);
			Context->SetInterfaceColor(LC_COLOR_CAMERA);
		}

		Context->DrawIndexedPrimitives(GL_LINES, 40, GL_UNSIGNED_SHORT, 0);

		if (IsSelected(LC_CAMERA_SECTION_TARGET))
		{
			Context->SetLineWidth(2.0f * LineWidth);
			if (IsFocused(LC_CAMERA_SECTION_TARGET))
				Context->SetInterfaceColor(LC_COLOR_FOCUSED);
			else
				Context->SetInterfaceColor(LC_COLOR_SELECTED);
		}
		else
		{
			Context->SetLineWidth(LineWidth);
			Context->SetInterfaceColor(LC_COLOR_CAMERA);
		}

		Context->DrawIndexedPrimitives(GL_LINES, 24, GL_UNSIGNED_SHORT, 40 * 2);

		if (IsSelected(LC_CAMERA_SECTION_UPVECTOR))
		{
			Context->SetLineWidth(2.0f * LineWidth);
			if (IsFocused(LC_CAMERA_SECTION_UPVECTOR))
				Context->SetInterfaceColor(LC_COLOR_FOCUSED);
			else
				Context->SetInterfaceColor(LC_COLOR_SELECTED);
		}
		else
		{
			Context->SetLineWidth(LineWidth);
			Context->SetInterfaceColor(LC_COLOR_CAMERA);
		}

		Context->DrawIndexedPrimitives(GL_LINES, 24, GL_UNSIGNED_SHORT, (40 + 24) * 2);

		Context->SetInterfaceColor(LC_COLOR_CAMERA);
		Context->SetLineWidth(LineWidth);

		float SizeY = tanf(LC_DTOR * m_fovy / 2) * Length;
		float SizeX = SizeY * 1.333f;

		*CurVert++ =  SizeX; *CurVert++ =  SizeY; *CurVert++ = -Length;
		*CurVert++ = -SizeX; *CurVert++ =  SizeY; *CurVert++ = -Length;
		*CurVert++ = -SizeX; *CurVert++ = -SizeY; *CurVert++ = -Length;
		*CurVert++ =  SizeX; *CurVert++ = -SizeY; *CurVert++ = -Length;

		Context->DrawIndexedPrimitives(GL_LINES, 4 + 16, GL_UNSIGNED_SHORT, (40 + 24 + 24) * 2);
	}
}

void lcCamera::RemoveKeyFrames()
{
	mPositionKeys.RemoveAll();
	mPositionKeys.ChangeKey(mPosition, 1, true);

	mTargetPositionKeys.RemoveAll();
	mTargetPositionKeys.ChangeKey(mTargetPosition, 1, true);

	mUpVectorKeys.RemoveAll();
	mUpVectorKeys.ChangeKey(mUpVector, 1, true);
}

void lcCamera::RayTest(lcObjectRayTest& ObjectRayTest) const
{
	lcVector3 Min = lcVector3(-LC_CAMERA_POSITION_EDGE, -LC_CAMERA_POSITION_EDGE, -LC_CAMERA_POSITION_EDGE);
	lcVector3 Max = lcVector3(LC_CAMERA_POSITION_EDGE, LC_CAMERA_POSITION_EDGE, LC_CAMERA_POSITION_EDGE);

	lcVector3 Start = lcMul31(ObjectRayTest.Start, mWorldView);
	lcVector3 End = lcMul31(ObjectRayTest.End, mWorldView);

	float Distance;
	if (lcBoundingBoxRayIntersectDistance(Min, Max, Start, End, &Distance, nullptr) && (Distance < ObjectRayTest.Distance))
	{
		ObjectRayTest.ObjectSection.Object = const_cast<lcCamera*>(this);
		ObjectRayTest.ObjectSection.Section = LC_CAMERA_SECTION_POSITION;
		ObjectRayTest.Distance = Distance;
	}

	Min = lcVector3(-LC_CAMERA_TARGET_EDGE, -LC_CAMERA_TARGET_EDGE, -LC_CAMERA_TARGET_EDGE);
	Max = lcVector3(LC_CAMERA_TARGET_EDGE, LC_CAMERA_TARGET_EDGE, LC_CAMERA_TARGET_EDGE);

	lcMatrix44 WorldView = mWorldView;
	WorldView.SetTranslation(lcMul30(-mTargetPosition, WorldView));

	Start = lcMul31(ObjectRayTest.Start, WorldView);
	End = lcMul31(ObjectRayTest.End, WorldView);

	if (lcBoundingBoxRayIntersectDistance(Min, Max, Start, End, &Distance, nullptr) && (Distance < ObjectRayTest.Distance))
	{
		ObjectRayTest.ObjectSection.Object = const_cast<lcCamera*>(this);
		ObjectRayTest.ObjectSection.Section = LC_CAMERA_SECTION_TARGET;
		ObjectRayTest.Distance = Distance;
	}

	lcMatrix44 ViewWorld = lcMatrix44AffineInverse(mWorldView);
	lcVector3 UpVectorPosition = lcMul31(lcVector3(0, 25, 0), ViewWorld);

	WorldView = mWorldView;
	WorldView.SetTranslation(lcMul30(-UpVectorPosition, WorldView));

	Start = lcMul31(ObjectRayTest.Start, WorldView);
	End = lcMul31(ObjectRayTest.End, WorldView);

	if (lcBoundingBoxRayIntersectDistance(Min, Max, Start, End, &Distance, nullptr) && (Distance < ObjectRayTest.Distance))
	{
		ObjectRayTest.ObjectSection.Object = const_cast<lcCamera*>(this);
		ObjectRayTest.ObjectSection.Section = LC_CAMERA_SECTION_UPVECTOR;
		ObjectRayTest.Distance = Distance;
	}
}

void lcCamera::BoxTest(lcObjectBoxTest& ObjectBoxTest) const
{
	lcVector3 Min(-LC_CAMERA_POSITION_EDGE, -LC_CAMERA_POSITION_EDGE, -LC_CAMERA_POSITION_EDGE);
	lcVector3 Max(LC_CAMERA_POSITION_EDGE, LC_CAMERA_POSITION_EDGE, LC_CAMERA_POSITION_EDGE);

	lcVector4 LocalPlanes[6];

	for (int PlaneIdx = 0; PlaneIdx < 6; PlaneIdx++)
	{
		lcVector3 Normal = lcMul30(ObjectBoxTest.Planes[PlaneIdx], mWorldView);
		LocalPlanes[PlaneIdx] = lcVector4(Normal, ObjectBoxTest.Planes[PlaneIdx][3] - lcDot3(mWorldView[3], Normal));
	}

	if (lcBoundingBoxIntersectsVolume(Min, Max, LocalPlanes))
	{
		ObjectBoxTest.Objects.Add(const_cast<lcCamera*>(this));
		return;
	}

	Min = lcVector3(-LC_CAMERA_TARGET_EDGE, -LC_CAMERA_TARGET_EDGE, -LC_CAMERA_TARGET_EDGE);
	Max = lcVector3(LC_CAMERA_TARGET_EDGE, LC_CAMERA_TARGET_EDGE, LC_CAMERA_TARGET_EDGE);

	lcMatrix44 WorldView = mWorldView;
	WorldView.SetTranslation(lcMul30(-mTargetPosition, WorldView));

	for (int PlaneIdx = 0; PlaneIdx < 6; PlaneIdx++)
	{
		lcVector3 Normal = lcMul30(ObjectBoxTest.Planes[PlaneIdx], WorldView);
		LocalPlanes[PlaneIdx] = lcVector4(Normal, ObjectBoxTest.Planes[PlaneIdx][3] - lcDot3(WorldView[3], Normal));
	}

	if (lcBoundingBoxIntersectsVolume(Min, Max, LocalPlanes))
	{
		ObjectBoxTest.Objects.Add(const_cast<lcCamera*>(this));
		return;
	}

	lcMatrix44 ViewWorld = lcMatrix44AffineInverse(mWorldView);
	lcVector3 UpVectorPosition = lcMul31(lcVector3(0, 25, 0), ViewWorld);

	WorldView = mWorldView;
	WorldView.SetTranslation(lcMul30(-UpVectorPosition, WorldView));

	for (int PlaneIdx = 0; PlaneIdx < 6; PlaneIdx++)
	{
		lcVector3 Normal = lcMul30(ObjectBoxTest.Planes[PlaneIdx], WorldView);
		LocalPlanes[PlaneIdx] = lcVector4(Normal, ObjectBoxTest.Planes[PlaneIdx][3] - lcDot3(WorldView[3], Normal));
	}

	if (lcBoundingBoxIntersectsVolume(Min, Max, LocalPlanes))
	{
		ObjectBoxTest.Objects.Add(const_cast<lcCamera*>(this));
		return;
	}
}

void lcCamera::InsertTime(lcStep Start, lcStep Time)
{
	mPositionKeys.InsertTime(Start, Time);
	mTargetPositionKeys.InsertTime(Start, Time);
	mUpVectorKeys.InsertTime(Start, Time);
}

void lcCamera::RemoveTime(lcStep Start, lcStep Time)
{
	mPositionKeys.RemoveTime(Start, Time);
	mTargetPositionKeys.RemoveTime(Start, Time);
	mUpVectorKeys.RemoveTime(Start, Time);
}

void lcCamera::ZoomExtents(float AspectRatio, const lcVector3& Center, const std::vector<lcVector3>& Points, lcStep Step, bool AddKey)
{
	if (IsOrtho())
	{
		float MinX = FLT_MAX, MaxX = -FLT_MAX, MinY = FLT_MAX, MaxY = -FLT_MAX;

		for (lcVector3 Point : Points)
		{
			Point = lcMul30(Point, mWorldView);

			MinX = lcMin(MinX, Point.x);
			MinY = lcMin(MinY, Point.y);
			MaxX = lcMax(MaxX, Point.x);
			MaxY = lcMax(MaxY, Point.y);
		}

		lcVector3 ViewCenter = lcMul30(Center, mWorldView);
		float Width = qMax(fabsf(MaxX - ViewCenter.x), fabsf(ViewCenter.x - MinX)) * 2;
		float Height = qMax(fabsf(MaxY - ViewCenter.y), fabsf(ViewCenter.y - MinY)) * 2;

		if (Width > Height * AspectRatio)
			Height = Width / AspectRatio;

		float f = Height / (m_fovy * (LC_PI / 180.0f));

		lcVector3 FrontVector(mTargetPosition - mPosition);
		mPosition = Center - lcNormalize(FrontVector) * f;
		mTargetPosition = Center;
	}
	else
	{
		lcVector3 Position(mPosition + Center - mTargetPosition);
		lcMatrix44 ProjectionMatrix = lcMatrix44Perspective(m_fovy, AspectRatio, m_zNear, m_zFar);

		std::tie(mPosition, std::ignore) = lcZoomExtents(Position, mWorldView, ProjectionMatrix, Points.data(), Points.size());
		mTargetPosition = Center;
	}

	if (IsSimple())
		AddKey = false;

	mPositionKeys.ChangeKey(mPosition, Step, AddKey);
	mTargetPositionKeys.ChangeKey(mTargetPosition, Step, AddKey);

	UpdatePosition(Step);
}

void lcCamera::ZoomRegion(float AspectRatio, const lcVector3& Position, const lcVector3& TargetPosition, const lcVector3* Corners, lcStep Step, bool AddKey)
{
	if (IsOrtho())
	{
		float MinX = FLT_MAX, MaxX = -FLT_MAX, MinY = FLT_MAX, MaxY = -FLT_MAX;

		for (int PointIdx = 0; PointIdx < 2; PointIdx++)
		{
			lcVector3 Point = lcMul30(Corners[PointIdx], mWorldView);

			MinX = lcMin(MinX, Point.x);
			MinY = lcMin(MinY, Point.y);
			MaxX = lcMax(MaxX, Point.x);
			MaxY = lcMax(MaxY, Point.y);
		}

		float Width = MaxX - MinX;
		float Height = MaxY - MinY;

		if (Width > Height * AspectRatio)
			Height = Width / AspectRatio;

		float f = Height / (m_fovy * (LC_PI / 180.0f));

		lcVector3 FrontVector(mTargetPosition - mPosition);
		mPosition = TargetPosition - lcNormalize(FrontVector) * f;
		mTargetPosition = TargetPosition;
	}
	else
	{
		lcMatrix44 WorldView = lcMatrix44LookAt(Position, TargetPosition, mUpVector);
		lcMatrix44 ProjectionMatrix = lcMatrix44Perspective(m_fovy, AspectRatio, m_zNear, m_zFar);

		std::tie(mPosition, std::ignore) = lcZoomExtents(Position, WorldView, ProjectionMatrix, Corners, 2);
		mTargetPosition = TargetPosition;
	}

	if (IsSimple())
		AddKey = false;

	mPositionKeys.ChangeKey(mPosition, Step, AddKey);
	mTargetPositionKeys.ChangeKey(mTargetPosition, Step, AddKey);

	UpdatePosition(Step);
}

void lcCamera::Zoom(float Distance, lcStep Step, bool AddKey)
{
	lcVector3 FrontVector(mPosition - mTargetPosition);
	FrontVector.Normalize();
/*** LPub3D Mod - Update Default Camera ***/
	FrontVector *= GetDDF() * Distance;
/*** LPub3D Mod end ***/

	// Don't zoom ortho in if it would cross the ortho focal plane.
	if (IsOrtho())
	{
		if ((Distance > 0) && (lcDot(mPosition + FrontVector - mTargetPosition, mPosition - mTargetPosition) <= 0))
			return;

		mPosition += FrontVector;
	}
	else
	{
		mPosition += FrontVector;
		mTargetPosition += FrontVector;
	}

	if (IsSimple())
		AddKey = false;

	mPositionKeys.ChangeKey(mPosition, Step, AddKey);
	mTargetPositionKeys.ChangeKey(mTargetPosition, Step, AddKey);

	UpdatePosition(Step);
}

void lcCamera::Pan(const lcVector3& Distance, lcStep Step, bool AddKey)
{
	mPosition += Distance;
	mTargetPosition += Distance;

	if (IsSimple())
		AddKey = false;

	mPositionKeys.ChangeKey(mPosition, Step, AddKey);
	mTargetPositionKeys.ChangeKey(mTargetPosition, Step, AddKey);

	UpdatePosition(Step);
}

void lcCamera::Orbit(float DistanceX, float DistanceY, const lcVector3& CenterPosition, lcStep Step, bool AddKey)
{
	lcVector3 FrontVector(mPosition - mTargetPosition);

	lcVector3 Z(lcNormalize(lcVector3(FrontVector[0], FrontVector[1], 0)));
	if (qIsNaN(Z[0]) || qIsNaN(Z[1]))
		Z = lcNormalize(lcVector3(mUpVector[0], mUpVector[1], 0));

	if (mUpVector[2] < 0)
	{
		Z[0] = -Z[0];
		Z[1] = -Z[1];
	}

	lcMatrix44 YRot(lcVector4(Z[0], Z[1], 0.0f, 0.0f), lcVector4(-Z[1], Z[0], 0.0f, 0.0f), lcVector4(0.0f, 0.0f, 1.0f, 0.0f), lcVector4(0.0f, 0.0f, 0.0f, 1.0f));
	lcMatrix44 transform = lcMul(lcMul(lcMul(lcMatrix44AffineInverse(YRot), lcMatrix44RotationY(DistanceY)), YRot), lcMatrix44RotationZ(-DistanceX));

	mPosition = lcMul31(mPosition - CenterPosition, transform) + CenterPosition;
	mTargetPosition = lcMul31(mTargetPosition - CenterPosition, transform) + CenterPosition;

	mUpVector = lcMul31(mUpVector, transform);

	if (IsSimple())
		AddKey = false;

	mPositionKeys.ChangeKey(mPosition, Step, AddKey);
	mTargetPositionKeys.ChangeKey(mTargetPosition, Step, AddKey);
	mUpVectorKeys.ChangeKey(mUpVector, Step, AddKey);

	UpdatePosition(Step);
}

void lcCamera::Roll(float Distance, lcStep Step, bool AddKey)
{
	lcVector3 FrontVector(mPosition - mTargetPosition);
	lcMatrix44 Rotation = lcMatrix44FromAxisAngle(FrontVector, Distance);

	mUpVector = lcMul30(mUpVector, Rotation);

	if (IsSimple())
		AddKey = false;

	mUpVectorKeys.ChangeKey(mUpVector, Step, AddKey);

	UpdatePosition(Step);
}

void lcCamera::Center(const lcVector3& NewCenter, lcStep Step, bool AddKey)
{
	const lcMatrix44 Inverse = lcMatrix44AffineInverse(mWorldView);
	const lcVector3 Direction = -lcVector3(Inverse[2]);

//	float Yaw, Pitch, Roll;
	float Roll;

	if (fabsf(Direction.z) < 0.9999f)
	{
//		Yaw = atan2f(Direction.y, Direction.x);
//		Pitch = asinf(Direction.z);
		Roll = atan2f(Inverse[0][2], Inverse[1][2]);
	}
	else
	{
//		Yaw = 0.0f;
//		Pitch = asinf(Direction.z);
		Roll = atan2f(Inverse[0][1], Inverse[1][1]);
	}

	mTargetPosition = NewCenter;

	lcVector3 FrontVector(mPosition - mTargetPosition);
	lcMatrix44 Rotation = lcMatrix44FromAxisAngle(FrontVector, Roll);

	lcVector3 UpVector(0, 0, 1), SideVector;
	FrontVector.Normalize();
	if (fabsf(lcDot(UpVector, FrontVector)) > 0.99f)
		SideVector = lcVector3(-1, 0, 0);
	else
		SideVector = lcCross(FrontVector, UpVector);
	UpVector = lcCross(SideVector, FrontVector);
	UpVector.Normalize();
	mUpVector = lcMul30(UpVector, Rotation);

	if (IsSimple())
		AddKey = false;

	mTargetPositionKeys.ChangeKey(mTargetPosition, Step, AddKey);
	mUpVectorKeys.ChangeKey(mUpVector, Step, AddKey);

	UpdatePosition(Step);
}

void lcCamera::SetViewpoint(lcViewpoint Viewpoint)
{
	lcVector3 Positions[] =
	{
		lcVector3(    0.0f, -GetCDP(),     0.0f), // lcViewpoint::Front     /*** LPub3D Mod - Update Default Camera ***/
		lcVector3(    0.0f,  GetCDP(),     0.0f), // lcViewpoint::Back      /*** LPub3D Mod - Update Default Camera ***/
		lcVector3(    0.0f,     0.0f,  GetCDP()), // lcViewpoint::Top       /*** LPub3D Mod - Update Default Camera ***/
		lcVector3(    0.0f,     0.0f, -GetCDP()), // lcViewpoint::Bottom    /*** LPub3D Mod - Update Default Camera ***/
		lcVector3( GetCDP(),     0.0f,     0.0f), // lcViewpoint::Left      /*** LPub3D Mod - Update Default Camera ***/
		lcVector3(-GetCDP(),     0.0f,     0.0f), // lcViewpoint::Right     /*** LPub3D Mod - Update Default Camera ***/
		lcVector3(  375.0f,   -375.0f,   187.5f)  // lcViewpoint::Home
	};

	lcVector3 Ups[] =
	{
		lcVector3(0.0f, 0.0f, 1.0f),
		lcVector3(0.0f, 0.0f, 1.0f),
		lcVector3(0.0f, 1.0f, 0.0f),
		lcVector3(0.0f,-1.0f, 0.0f),
		lcVector3(0.0f, 0.0f, 1.0f),
		lcVector3(0.0f, 0.0f, 1.0f),
		lcVector3(0.2357f, -0.2357f, 0.94281f)
	};

	mPosition = Positions[static_cast<int>(Viewpoint)];
	mTargetPosition = lcVector3(0, 0, 0);
	mUpVector = Ups[static_cast<int>(Viewpoint)];

	mPositionKeys.ChangeKey(mPosition, 1, false);
	mTargetPositionKeys.ChangeKey(mTargetPosition, 1, false);
	mUpVectorKeys.ChangeKey(mUpVector, 1, false);

	UpdatePosition(1);
}

void lcCamera::SetViewpoint(const lcVector3& Position)
{
	mPosition = Position;
	mTargetPosition = lcVector3(0, 0, 0);

	lcVector3 UpVector(0, 0, 1), FrontVector(Position), SideVector;
	FrontVector.Normalize();
	if (fabsf(lcDot(UpVector, FrontVector)) > 0.99f)
		SideVector = lcVector3(-1, 0, 0);
	else
		SideVector = lcCross(FrontVector, UpVector);
	UpVector = lcCross(SideVector, FrontVector);
	UpVector.Normalize();
	mUpVector = UpVector;

	mPositionKeys.ChangeKey(mPosition, 1, false);
	mTargetPositionKeys.ChangeKey(mTargetPosition, 1, false);
	mUpVectorKeys.ChangeKey(mUpVector, 1, false);

	UpdatePosition(1);
}

void lcCamera::SetViewpoint(const lcVector3& Position, const lcVector3& Target, const lcVector3& Up)
{
	mPosition = Position;
	mTargetPosition = Target;

	lcVector3 Direction = Target - Position;
	lcVector3 UpVector, SideVector;
	SideVector = lcCross(Direction, Up);
	UpVector = lcCross(SideVector, Direction);
	UpVector.Normalize();
	mUpVector = UpVector;

	mPositionKeys.ChangeKey(mPosition, 1, false);
	mTargetPositionKeys.ChangeKey(mTargetPosition, 1, false);
	mUpVectorKeys.ChangeKey(mUpVector, 1, false);

	UpdatePosition(1);
}

/*** LPub3D Mod - Camera Globe ***/
void lcCamera::SetAngles(const float &Latitude, const float &Longitude, const float &Distance)
{
	SetAngles(Latitude, Longitude, Distance, mTargetPosition, 1, false);
}

void lcCamera::SetAngles(const float &Latitude, const float &Longitude, const float &Distance, const lcVector3 &Target)
{
	SetAngles(Latitude, Longitude, Distance, Target, 1, false);
}

void lcCamera::SetAngles(const float &Latitude, const float &Longitude, const float &Distance, const lcVector3 &Target, lcStep Step, bool AddKey)
{
	mPosition = lcVector3(0, -1, 0);
	mTargetPosition = lcVector3(0, 0, 0); //Target;
/*** LPub3D Mod end ***/
	mUpVector = lcVector3(0, 0, 1);

	lcMatrix33 LongitudeMatrix = lcMatrix33RotationZ(LC_DTOR * Longitude);
	mPosition = lcMul(mPosition, LongitudeMatrix);

	lcVector3 SideVector = lcMul(lcVector3(-1, 0, 0), LongitudeMatrix);
	lcMatrix33 LatitudeMatrix = lcMatrix33FromAxisAngle(SideVector, LC_DTOR * Latitude);

/*** LPub3D Mod - Camera Globe ***/
	// Convert distance to LeoCAD format from Lego Draw Unit (LDU) format - e.g. 3031329
	int   Width      = lcGetActiveProject()->GetModelWidth();
	int   Renderer   = lcGetActiveProject()->GetRenderer();
	float Resolution = lcGetActiveProject()->GetResolution();
	float CameraDistance = NativeCameraDistance(Distance, GetCDF(), Width, Resolution, Renderer);

	mPosition = lcMul(mPosition, LatitudeMatrix) * CameraDistance;
	mUpVector = lcMul(mUpVector, LatitudeMatrix);

	// Set LookAt Viewpoint
	if (Target != mTargetPosition) {
		mTargetPosition = Target;
		lcVector3 Direction = mTargetPosition - mPosition;
		SideVector = lcCross(Direction, mUpVector);
		lcVector3 UpVector = lcCross(SideVector, Direction);
		UpVector.Normalize();
		mUpVector = UpVector;
	}

	mPositionKeys.ChangeKey(mPosition, 1, AddKey);
	mTargetPositionKeys.ChangeKey(mTargetPosition, 1, AddKey);
	mUpVectorKeys.ChangeKey(mUpVector, 1, AddKey);

	UpdatePosition(Step);
/*** LPub3D Mod end ***/
}

void lcCamera::GetAngles(float& Latitude, float& Longitude, float& Distance) const
{
	lcVector3 FrontVector(mPosition - mTargetPosition);
	lcVector3 X(1, 0, 0);
	lcVector3 Y(0, 1, 0);
	lcVector3 Z(0, 0, 1);

	FrontVector.Normalize();
	Latitude = acos(lcDot(-FrontVector, Z)) * LC_RTOD - 90.0f;

	lcVector3 CameraXY = -lcNormalize(lcVector3(FrontVector.x, FrontVector.y, 0.0f));
	Longitude = acos(lcDot(CameraXY, Y)) * LC_RTOD;

	if (lcDot(CameraXY, X) > 0)
		Longitude = -Longitude;

/*** LPub3D Mod - Camera Globe ***/
	// Convert distance to Lego Draw Unit (LDU) format from LeoCAD format - e.g. 1250
	int   Width      = lcGetActiveProject()->GetModelWidth();
	int   Renderer   = lcGetActiveProject()->GetRenderer();
	float Resolution = lcGetActiveProject()->GetResolution();
	Distance = StandardCameraDistance(lcLength(mPosition), GetCDF(), Width, Resolution, Renderer);
/*** LPub3D Mod end ***/
}

/*** LPub3D Mod - Camera Globe ***/
float lcCamera::GetScale()
{
	return 1 / (lcLength(mPosition) / GetCDF()) ;
}
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Update Default Camera ***/
float lcCamera::GetCDP() const
{
	// Camera Default Position
	return gApplication->mPreferences.mCDP;
}

float lcCamera::GetDDF() const
{
	// Default Distance Factor
	return -gApplication->mPreferences.mDDF;
}

float lcCamera::GetCDF() const
{
	// Camera Distance Factor = Camera Default Position / Default Distance Factor
	return (gApplication->mPreferences.mCDP / -gApplication->mPreferences.mDDF) * -5.0f/*Default Distance Factor*/;
}
/*** LPub3D Mod end ***/
