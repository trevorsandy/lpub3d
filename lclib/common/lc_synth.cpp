#include "lc_global.h"
#include "lc_synth.h"
#include "lc_library.h"
#include "lc_application.h"
#include "lc_file.h"
#include "lc_meshloader.h"
#include "pieceinf.h"
#include <locale.h>

class lcSynthInfoCurved : public lcSynthInfo
{
public:
	lcSynthInfoCurved(float Length, float DefaultScale, int NumSections, bool RigidEdges);

	void GetDefaultControlPoints(std::vector<lcPieceControlPoint>& ControlPoints) const override;
	void VerifyControlPoints(std::vector<lcPieceControlPoint>& ControlPoints) const override;

protected:
	float GetSectionTwist(const lcMatrix44& StartTransform, const lcMatrix44& EndTransform) const;
	void CalculateSections(const std::vector<lcPieceControlPoint>& ControlPoints, std::vector<lcMatrix44>& Sections, SectionCallbackFunc SectionCallback) const override;
	static void AddTubeParts(lcLibraryMeshData& MeshData, const std::vector<lcMatrix44>& Sections, float Radius, bool IsInner);

	struct lcSynthComponent
	{
		lcMatrix44 Transform;
		float Length;
	};

	lcSynthComponent mStart;
	lcSynthComponent mMiddle;
	lcSynthComponent mEnd;
	float mCenterLength = 0.0f;
	size_t mSectionCount;
	float mDefaultScale;
	bool mRigidEdges;
};

class lcSynthInfoFlexibleHose : public lcSynthInfoCurved
{
public:
	lcSynthInfoFlexibleHose(float Length, int NumSections, const char* EdgePart2);

protected:
	void AddParts(lcMemFile& File, lcLibraryMeshData& MeshData, const std::vector<lcMatrix44>& Sections) const override;

	const char* mEdgePart2;
};

class lcSynthInfoFlexSystemHose : public lcSynthInfoCurved
{
public:
	lcSynthInfoFlexSystemHose(float Length, int NumSections);

	void GetDefaultControlPoints(std::vector<lcPieceControlPoint>& ControlPoints) const override;

protected:
	void AddParts(lcMemFile& File, lcLibraryMeshData& MeshData, const std::vector<lcMatrix44>& Sections) const override;
};

class lcSynthInfoPneumaticTube : public lcSynthInfoCurved
{
public:
	lcSynthInfoPneumaticTube(float Length, int NumSections, const char* EndPart);

protected:
	void AddParts(lcMemFile& File, lcLibraryMeshData& MeshData, const std::vector<lcMatrix44>& Sections) const override;

	const char* mEndPart;
};

class lcSynthInfoRibbedHose : public lcSynthInfoCurved
{
public:
	lcSynthInfoRibbedHose(float Length, int NumSections);

protected:
	void AddParts(lcMemFile& File, lcLibraryMeshData& MeshData, const std::vector<lcMatrix44>& Sections) const override;
};

class lcSynthInfoFlexibleAxle : public lcSynthInfoCurved
{
public:
	lcSynthInfoFlexibleAxle(float Length, int NumSections);

protected:
	void AddParts(lcMemFile& File, lcLibraryMeshData& MeshData, const std::vector<lcMatrix44>& Sections) const override;
};

class lcSynthInfoBraidedString : public lcSynthInfoCurved
{
public:
	lcSynthInfoBraidedString(float Length, int NumSections);

protected:
	void CalculateSections(const std::vector<lcPieceControlPoint>& ControlPoints, std::vector<lcMatrix44>& Sections, SectionCallbackFunc SectionCallback) const override;
	void AddParts(lcMemFile& File, lcLibraryMeshData& MeshData, const std::vector<lcMatrix44>& Sections) const override;
};

class lcSynthInfoStraight : public lcSynthInfo
{
public:
	explicit lcSynthInfoStraight(float Length);

	void VerifyControlPoints(std::vector<lcPieceControlPoint>& ControlPoints) const override;

protected:
	void CalculateSections(const std::vector<lcPieceControlPoint>& ControlPoints, std::vector<lcMatrix44>& Sections, SectionCallbackFunc SectionCallback) const override;
};

class lcSynthInfoShockAbsorber : public lcSynthInfoStraight
{
public:
	explicit lcSynthInfoShockAbsorber(const char* SpringPart);

	void GetDefaultControlPoints(std::vector<lcPieceControlPoint>& ControlPoints) const override;

protected:
	void AddParts(lcMemFile& File, lcLibraryMeshData& MeshData, const std::vector<lcMatrix44>& Sections) const override;

	const char* mSpringPart;
};

class lcSynthInfoActuator : public lcSynthInfoStraight
{
public:
	explicit lcSynthInfoActuator(const char* BodyPart, const char* PistonPart, const char* AxlePart, float Length, float AxleOffset);

	void GetDefaultControlPoints(std::vector<lcPieceControlPoint>& ControlPoints) const override;

protected:
	void AddParts(lcMemFile& File, lcLibraryMeshData& MeshData, const std::vector<lcMatrix44>& Sections) const override;

	const char* mBodyPart;
	const char* mPistonPart;
	const char* mAxlePart;
	float mAxleOffset;
};

class lcSynthInfoUniversalJoint : public lcSynthInfo
{
public:
	lcSynthInfoUniversalJoint(float Length, float EndOffset, const char* EndPart, const char* CenterPart);

	void GetDefaultControlPoints(std::vector<lcPieceControlPoint>& ControlPoints) const override;
	void VerifyControlPoints(std::vector<lcPieceControlPoint>& ControlPoints) const override;

protected:
	void CalculateSections(const std::vector<lcPieceControlPoint>& ControlPoints, std::vector<lcMatrix44>& Sections, SectionCallbackFunc SectionCallback) const override;
	void AddParts(lcMemFile& File, lcLibraryMeshData& MeshData, const std::vector<lcMatrix44>& Sections) const override;

	float mEndOffset;
	const char* mEndPart;
	const char* mCenterPart;
};

void lcSynthInit()
{
	lcPiecesLibrary* Library = lcGetPiecesLibrary();

	static const struct
	{
		char PartID[16];
		float Length;
		int NumSections;
		char EdgePart2[8];
	}
	FlexibleHoses[] =
	{
		{ "73590a.dat", 140.0f, 51, "752.dat" }, // Hose Flexible  8.5L without Tabs
		{ "73590b.dat", 140.0f, 51, "750.dat" }, // Hose Flexible  8.5L with Tabs
	};

	for (const auto& HoseInfo: FlexibleHoses)
	{
		PieceInfo* Info = Library->FindPiece(HoseInfo.PartID, nullptr, false, false);

		if (Info)
			Info->SetSynthInfo(new lcSynthInfoFlexibleHose(HoseInfo.Length, HoseInfo.NumSections, HoseInfo.EdgePart2));
	}

	static const struct
	{
		char PartID[16];
		float Length;
		int NumSections;
	}
	FlexSystemHoses[] =
	{
		{ "76263.dat",      60.0f,  29 }, // Technic Flex-System Hose  3L (60LDU)
		{ "76250.dat",      80.0f,  39 }, // Technic Flex-System Hose  4L (80LDU)
		{ "76307.dat",     100.0f,  49 }, // Technic Flex-System Hose  5L (100LDU)
		{ "76279.dat",     120.0f,  59 }, // Technic Flex-System Hose  6L (120LDU)
		{ "76289.dat",     140.0f,  69 }, // Technic Flex-System Hose  7L (140LDU)
		{ "76260.dat",     160.0f,  79 }, // Technic Flex-System Hose  8L (160LDU)
		{ "76324.dat",     180.0f,  89 }, // Technic Flex-System Hose  9L (180LDU)
		{ "76348.dat",     200.0f,  99 }, // Technic Flex-System Hose 10L (200LDU)
		{ "71505.dat",     220.0f, 109 }, // Technic Flex-System Hose 11L (220LDU)
		{ "71175.dat",     240.0f, 119 }, // Technic Flex-System Hose 12L (240LDU)
		{ "71551.dat",     260.0f, 129 }, // Technic Flex-System Hose 13L (260LDU)
		{ "71177.dat",     280.0f, 139 }, // Technic Flex-System Hose 14L (280LDU)
		{ "71194.dat",     300.0f, 149 }, // Technic Flex-System Hose 15L (300LDU)
		{ "71192.dat",     320.0f, 159 }, // Technic Flex-System Hose 16L (320LDU)
		{ "76270.dat",     340.0f, 169 }, // Technic Flex-System Hose 17L (340LDU)
		{ "71582.dat",     360.0f, 179 }, // Technic Flex-System Hose 18L (360LDU)
		{ "22463.dat",     380.0f, 189 }, // Technic Flex-System Hose 19L (380LDU)
		{ "76276.dat",     400.0f, 199 }, // Technic Flex-System Hose 20L (400LDU)
		{ "70978.dat",     420.0f, 209 }, // Technic Flex-System Hose 21L (420LDU)
		{ "76252.dat",     440.0f, 219 }, // Technic Flex-System Hose 22L (440LDU)
		{ "76254.dat",     460.0f, 229 }, // Technic Flex-System Hose 23L (460LDU)
		{ "76277.dat",     480.0f, 239 }, // Technic Flex-System Hose 24L (480LDU)
		{ "53475.dat",     520.0f, 259 }, // Technic Flex-System Hose 26L (520LDU)
		{ "76280.dat",     560.0f, 279 }, // Technic Flex-System Hose 28L (560LDU)
		{ "76389.dat",     580.0f, 289 }, // Technic Flex-System Hose 29L (580LDU)
		{ "76282.dat",     600.0f, 299 }, // Technic Flex-System Hose 30L (600LDU)
		{ "76283.dat",     620.0f, 309 }, // Technic Flex-System Hose 31L (620LDU)
		{ "57274.dat",     640.0f, 319 }, // Technic Flex-System Hose 32L (640LDU)
		{ "42688.dat",     660.0f, 329 }, // Technic Flex-System Hose 33L (660LDU)
		{ "22461.dat",     680.0f, 339 }, // Technic Flex-System Hose 34L (680LDU)
		{ "46305.dat",     800.0f, 399 }, // Technic Flex-System Hose 40L (800LDU)
		{ "76281.dat",     900.0f, 449 }, // Technic Flex-System Hose 45L (900LDU)
		{ "22296.dat",    1060.0f, 529 }, // Technic Flex-System Hose 53L (1060LDU)
	};

	for (const auto& HoseInfo: FlexSystemHoses)
	{
		PieceInfo* Info = Library->FindPiece(HoseInfo.PartID, nullptr, false, false);

		if (Info)
			Info->SetSynthInfo(new lcSynthInfoFlexSystemHose(HoseInfo.Length, HoseInfo.NumSections));
	}

	static const struct
	{
		char PartID[16];
		float Length;
		int NumSections;
	}
	PneumaticTubes[] =
	{
		{ "21761-f1.dat",   60.0f,  10 }, // Technic Pneumatic Tube  3L
		{ "26445-f1.dat",   80.0f,  20 }, // Technic Pneumatic Tube  4L
		{ "14653-f1.dat",  100.0f,  20 }, // Technic Pneumatic Tube  5L
		{ "21766-f1.dat",  120.0f,  40 }, // Technic Pneumatic Tube  6L
		{ "14657-f1.dat",  140.0f,  40 }, // Technic Pneumatic Tube  7L
		{ "21837-f1.dat",  160.0f,  40 }, // Technic Pneumatic Tube  8L
		{ "21826-f1.dat",  180.0f,  40 }, // Technic Pneumatic Tube  9L
		{ "21767-f1.dat",  200.0f,  80 }, // Technic Pneumatic Tube 10L
		{ "63539-f1.dat",  240.0f,  80 }, // Technic Pneumatic Tube 12L
		{ "37467-f1.dat",  260.0f,  80 }, // Technic Pneumatic Tube 13L
		{ "37461-f1.dat",  280.0f,  80 }, // Technic Pneumatic Tube 14L
		{ "87948-f1.dat",  320.0f,  80 }, // Technic Pneumatic Tube 16L
		{ "53168-f1.dat",  340.0f,  80 }, // Technic Pneumatic Tube 17L
		{ "21839-f1.dat",  380.0f, 160 }, // Technic Pneumatic Tube 19L
		{ "87950-f1.dat",  400.0f, 160 }, // Technic Pneumatic Tube 20L
		{ "53184-f1.dat",  420.0f, 160 }, // Technic Pneumatic Tube 21L
		{ "26436-f1.dat",  460.0f, 160 }, // Technic Pneumatic Tube 23L
		{ "21830-f1.dat",  540.0f, 160 }, // Technic Pneumatic Tube 27L
		{ "21825-f1.dat",  560.0f, 160 }, // Technic Pneumatic Tube 28L
		{ "21833-f1.dat",  600.0f, 160 }, // Technic Pneumatic Tube 30L
		{ "26440-f1.dat",  640.0f, 160 }, // Technic Pneumatic Tube 32L
		{ "96889-f1.dat",  660.0f, 160 }, // Technic Pneumatic Tube 33L
		{ "87949-f1.dat",  720.0f, 320 }, // Technic Pneumatic Tube 36L
		{ "21835-f1.dat",  740.0f, 320 }, // Technic Pneumatic Tube 37L
		{ "14661-f1.dat",  780.0f, 320 }, // Technic Pneumatic Tube 39L
		{ "26438-f1.dat",  800.0f, 320 }, // Technic Pneumatic Tube 40L
		{ "44079-f1.dat",  840.0f, 320 }, // Technic Pneumatic Tube 42L
		{ "26439-f1.dat",  960.0f, 320 }, // Technic Pneumatic Tube 48L
		{ "96890-f1.dat", 1080.0f, 320 }, // Technic Pneumatic Tube 54L
		{ "96891-f1.dat", 1600.0f, 320 }, // Technic Pneumatic Tube 80L
	};

	for (const auto& TubeInfo: PneumaticTubes)
	{
		PieceInfo* Info = Library->FindPiece(TubeInfo.PartID, nullptr, false, false);

		if (Info)
			Info->SetSynthInfo(new lcSynthInfoPneumaticTube(TubeInfo.Length, TubeInfo.NumSections, "71533k02.dat"));

		auto RegularInfo = TubeInfo;
		RegularInfo.PartID[7] = '2';
		RegularInfo.Length -= 40.0f;

		Info = Library->FindPiece(RegularInfo.PartID, nullptr, false, false);

		if (Info)
			Info->SetSynthInfo(new lcSynthInfoPneumaticTube(RegularInfo.Length, RegularInfo.NumSections, "71533k01.dat"));
	}

	static const struct
	{
		char PartID[16];
		float Length;
		int NumSections;
	}
	RibbedHoses[] =
	{
		{ "72504.dat",     31.25f,   4 }, // Technic Ribbed Hose  2L
		{ "72706.dat",     50.00f,   7 }, // Technic Ribbed Hose  3L
		{ "71952.dat",     75.00f,  11 }, // Technic Ribbed Hose  4L
		{ "72853.dat",     93.75f,  14 }, // Technic Ribbed Hose  5L
		{ "71944.dat",    112.50f,  17 }, // Technic Ribbed Hose  6L
		{ "57719.dat",    131.25f,  20 }, // Technic Ribbed Hose  7L
		{ "71951.dat",    150.00f,  23 }, // Technic Ribbed Hose  8L
		{ "71917.dat",    175.00f,  27 }, // Technic Ribbed Hose  9L
		{ "71949.dat",    193.75f,  30 }, // Technic Ribbed Hose 10L
		{ "71986.dat",    212.50f,  33 }, // Technic Ribbed Hose 11L
		{ "71819.dat",    231.25f,  36 }, // Technic Ribbed Hose 12L
		{ "71923.dat",    275.00f,  43 }, // Technic Ribbed Hose 14L
		{ "71946.dat",    293.75f,  46 }, // Technic Ribbed Hose 15L
		{ "71947.dat",    312.50f,  49 }, // Technic Ribbed Hose 16L
		{ "22900.dat",    331.25f,  52 }, // Technic Ribbed Hose 17L
		{ "72039.dat",    350.00f,  55 }, // Technic Ribbed Hose 18L
		{ "43675.dat",    375.00f,  58 }, // Technic Ribbed Hose 19L
		{ "23397.dat",    468.75f,  74 }, // Technic Ribbed Hose 24L
		{ "33763.dat",    512.50f,  81 }, // Technic Ribbed Hose 26L
	};

	for (const auto& HoseInfo: RibbedHoses)
	{
		PieceInfo* Info = Library->FindPiece(HoseInfo.PartID, nullptr, false, false);

		if (Info)
			Info->SetSynthInfo(new lcSynthInfoRibbedHose(HoseInfo.Length, HoseInfo.NumSections));
	}

	static const struct
	{
		char PartID[16];
		float Length;
		int NumSections;
	}
	FlexibleAxles[] =
	{
		{ "32580.dat",    120.00f,  15 }, // Technic Axle Flexible  7
		{ "32199.dat",    200.00f,  35 }, // Technic Axle Flexible 11
		{ "55709.dat",    200.00f,  35 }, // Technic Axle Flexible 11
		{ "32200.dat",    220.00f,  40 }, // Technic Axle Flexible 12
		{ "32201.dat",    260.00f,  50 }, // Technic Axle Flexible 14
		{ "32202.dat",    300.00f,  60 }, // Technic Axle Flexible 16
		{ "32235.dat",    360.00f,  75 }, // Technic Axle Flexible 19
	};

	for (const auto& AxleInfo: FlexibleAxles)
	{
		PieceInfo* Info = Library->FindPiece(AxleInfo.PartID, nullptr, false, false);

		if (Info)
			Info->SetSynthInfo(new lcSynthInfoFlexibleAxle(AxleInfo.Length, AxleInfo.NumSections));
	}

	static const struct
	{
		char PartID[16];
		float Length;
		int NumSections;
	}
	BraidedStrings[] =
	{
		{ "76384.dat",    200.00f,  46 }, // String Braided 11L with End Studs
		{ "75924.dat",    400.00f,  96 }, // String Braided 21L with End Studs
		{ "572C02.dat",   800.00f, 196 }, // String Braided 41L with End Studs
	};

	for (const auto& StringInfo: BraidedStrings)
	{
		PieceInfo* Info = Library->FindPiece(StringInfo.PartID, nullptr, false, false);

		if (Info)
			Info->SetSynthInfo(new lcSynthInfoBraidedString(StringInfo.Length, StringInfo.NumSections));
	}

	static const struct
	{
		char PartID[16];
		char SpringPart[16];
	}
	ShockAbsorbers[] =
	{
		{ "73129.dat", "70038.dat" }, // Technic Shock Absorber 6.5L
		{ "41838.dat", "41837.dat" }, // Technic Shock Absorber 6.5L Soft
		{ "76138.dat", "71953.dat" }, // Technic Shock Absorber 6.5L Stiff
		{ "76537.dat", "22977.dat" }, // Technic Shock Absorber 6.5L Extra Stiff
	};

	for (const auto& AbsorberInfo: ShockAbsorbers)
	{
		PieceInfo* Info = Library->FindPiece(AbsorberInfo.PartID, nullptr, false, false);

		if (Info)
			Info->SetSynthInfo(new lcSynthInfoShockAbsorber(AbsorberInfo.SpringPart));
	}

	static const struct
	{
		char PartID[16];
		char BodyPart[16];
		char PistonPart[16];
		char AxlePart[16];
		float Length;
		float AxleOffset;
	}
	Actuators[] =
	{
		{ "61927-F1.dat",    "62271c01.dat", "62274c01.dat", "47157.dat", 170.00f,  0.0f }, // Technic Linear Actuator 8 x 2 x 2 (Contracted)
		{ "61927-F2.dat",    "62271c01.dat", "62274c01.dat", "47157.dat", 270.00f,  0.0f }, // Technic Linear Actuator 8 x 2 x 2 (Extended)
		{ "61927C01.dat",    "62271c01.dat", "62274c01.dat", "47157.dat", 270.00f,  0.0f }, // Moved to 61927-f2 (was Technic Power Functions Linear Actuator (Extended))
		{ "61927.dat",       "62271c01.dat", "62274c01.dat", "47157.dat", 170.00f,  0.0f }, // Moved to 61927-f1 (was Technic Power Functions Linear Actuator (Contracted))
		{ "92693C01-F1.dat", "92693c01.dat", "92696.dat",    "92695.dat", 120.00f, 30.0f }, // Technic Linear Actuator 4 x 1 x 1 (Contracted)
		{ "92693C01-F2.dat", "92693c01.dat", "92696.dat",    "92695.dat", 180.00f, 30.0f }, // Technic Linear Actuator 4 x 1 x 1 (Extended)
		{ "92693C02.dat",    "92693c01.dat", "92696.dat",    "92695.dat", 120.00f, 30.0f }, // Moved to 92693c01-f1 (was Technic Linear Actuator Small (Contracted))
		{ "92693C03.dat",    "92693c01.dat", "92696.dat",    "92695.dat", 180.00f, 30.0f }, // Moved to 92693c01-f2 (was Technic Linear Actuator Small (Extended))
	};

	for (const auto& ActuatorInfo: Actuators)
	{
		PieceInfo* Info = Library->FindPiece(ActuatorInfo.PartID, nullptr, false, false);

		if (Info)
			Info->SetSynthInfo(new lcSynthInfoActuator(ActuatorInfo.BodyPart, ActuatorInfo.PistonPart,
					ActuatorInfo.AxlePart, ActuatorInfo.Length, ActuatorInfo.AxleOffset));
	}

	static const struct
	{
		char PartID[16];
		float Length;
		float EndOffset;
		char EndPart[16];
		char CenterPart[16];
	}
	UniversalJoints[] =
	{
		{ "61903.dat",   60.00f,  0.0f, "62520.dat", "62519.dat" }, // Technic Universal Joint 3L (Complete)
		{ "3712C01.dat", 60.00f, 30.0f, "3712.dat",  "3326.dat"  }, // Technic Universal Joint 4L with Bush Ends with Centre Type 2 (Complete)
		{ "3712C03.dat", 60.00f, 30.0f, "3712.dat",  "3326a.dat" }, // Technic Universal Joint 4L with Bush Ends with Centre Type 1 (Complete)
		{ "575C01.dat",  60.00f, 30.0f, "575.dat",   "3326a.dat" }  // Technic Universal Joint Type 1 (Complete)
	};

	for (const auto& JointInfo: UniversalJoints)
	{
		PieceInfo* Info = Library->FindPiece(JointInfo.PartID, nullptr, false, false);

		if (Info)
			Info->SetSynthInfo(new lcSynthInfoUniversalJoint(JointInfo.Length, JointInfo.EndOffset, JointInfo.EndPart, JointInfo.CenterPart));
	}

//	"758C01" // Hose Flexible  12L
}

lcSynthInfo::lcSynthInfo(float Length)
	: mLength(Length)
{
}

lcSynthInfoCurved::lcSynthInfoCurved(float Length, float DefaultScale, int NumSections, bool RigidEdges)
	: lcSynthInfo(Length), mSectionCount(NumSections), mDefaultScale(DefaultScale), mRigidEdges(RigidEdges)
{
	mCurve = true;

	mStart.Transform = lcMatrix44(lcMatrix33(lcVector3(0.0f, 0.0f, 1.0f), lcVector3(1.0f, 0.0f, 0.0f), lcVector3(0.0f, 1.0f, 0.0f)), lcVector3(0.0f, 0.0f, 0.0f));
	mEnd.Transform = lcMatrix44(lcMatrix33(lcVector3(0.0f, 0.0f, 1.0f), lcVector3(1.0f, 0.0f, 0.0f), lcVector3(0.0f, 1.0f, 0.0f)), lcVector3(0.0f, 0.0f, 0.0f));
}

lcSynthInfoFlexibleHose::lcSynthInfoFlexibleHose(float Length, int NumSections, const char* EdgePart2)
	: lcSynthInfoCurved(Length, 12.f, NumSections, true),
	mEdgePart2(EdgePart2)
{
	mStart.Length = 5.0f;
	mMiddle.Length = 2.56f;
	mEnd.Length = 5.0f;
	mCenterLength = 4.56f;
}

lcSynthInfoFlexSystemHose::lcSynthInfoFlexSystemHose(float Length, int NumSections)
	: lcSynthInfoCurved(Length, 12.f, NumSections, true)
{
	mStart.Transform = lcMatrix44Identity();
	mEnd.Transform = lcMatrix44Identity();
	mStart.Length = 1.0f;
	mMiddle.Length = 2.0f;
	mEnd.Length = 1.0f;
}

lcSynthInfoPneumaticTube::lcSynthInfoPneumaticTube(float Length, int NumSections, const char* EndPart)
	: lcSynthInfoCurved(Length, 12.f, NumSections, true),
	mEndPart(EndPart)
{
	mStart.Length = 0.0f;
	mMiddle.Length = mLength / NumSections;
	mEnd.Length = 0.0f;
}

lcSynthInfoRibbedHose::lcSynthInfoRibbedHose(float Length, int NumSections)
	: lcSynthInfoCurved(Length, 80.0f, NumSections, false)
{
	mStart.Length = 6.25f;
	mMiddle.Length = 6.25f;
	mEnd.Length = 6.25f;
}

lcSynthInfoFlexibleAxle::lcSynthInfoFlexibleAxle(float Length, int NumSections)
	: lcSynthInfoCurved(Length, 12.0f, NumSections, true)
{
	mStart.Length = 30.0f;
	mMiddle.Length = 4.0f;
	mEnd.Length = 30.0f;
}

lcSynthInfoBraidedString::lcSynthInfoBraidedString(float Length, int NumSections)
	: lcSynthInfoCurved(Length, 12.0f, NumSections, true)
{
	mStart.Transform = lcMatrix44Identity();
	mEnd.Transform = lcMatrix44Identity();
	mStart.Length = 8.0f;
	mMiddle.Length = 4.0f;
	mEnd.Length = 8.0f;
}

lcSynthInfoStraight::lcSynthInfoStraight(float Length)
	: lcSynthInfo(Length)
{
	mUnidirectional = true;
}

lcSynthInfoShockAbsorber::lcSynthInfoShockAbsorber(const char* SpringPart)
	: lcSynthInfoStraight(110.00f), mSpringPart(SpringPart)
{
}

lcSynthInfoActuator::lcSynthInfoActuator(const char* BodyPart, const char* PistonPart, const char* AxlePart, float Length, float AxleOffset)
	: lcSynthInfoStraight(Length), mBodyPart(BodyPart), mPistonPart(PistonPart), mAxlePart(AxlePart), mAxleOffset(AxleOffset)
{
}

lcSynthInfoUniversalJoint::lcSynthInfoUniversalJoint(float Length, float EndOffset, const char* EndPart, const char* CenterPart)
	: lcSynthInfo(Length), mEndOffset(EndOffset), mEndPart(EndPart), mCenterPart(CenterPart)
{
	mNondirectional = true;
}

void lcSynthInfoCurved::GetDefaultControlPoints(std::vector<lcPieceControlPoint>& ControlPoints) const
{
	ControlPoints.resize(2);

	float HalfLength = mLength / 2.0f;
	float Scale = lcMin(mDefaultScale, HalfLength);

	ControlPoints[0].Transform = lcMatrix44Translation(lcVector3(-HalfLength, 0.0f, 0.0f));
	ControlPoints[1].Transform = lcMatrix44Translation(lcVector3( HalfLength, 0.0f, 0.0f));

	ControlPoints[0].Scale = Scale;
	ControlPoints[1].Scale = Scale;
}

void lcSynthInfoCurved::VerifyControlPoints(std::vector<lcPieceControlPoint>& ControlPoints) const
{
	if (ControlPoints.size() < 2)
		GetDefaultControlPoints(ControlPoints);
}

void lcSynthInfoFlexSystemHose::GetDefaultControlPoints(std::vector<lcPieceControlPoint>& ControlPoints) const
{
	lcSynthInfoCurved::GetDefaultControlPoints(ControlPoints);

	ControlPoints[0].Transform = lcMatrix44Translation(lcVector3(0.0f, 0.0f, -mLength));
	ControlPoints[1].Transform = lcMatrix44Translation(lcVector3(0.0f, 0.0f, 0.0f));
}

void lcSynthInfoStraight::VerifyControlPoints(std::vector<lcPieceControlPoint>& ControlPoints) const
{
	if (ControlPoints.size() < 2)
		GetDefaultControlPoints(ControlPoints);
	else
		ControlPoints.resize(2);
}

void lcSynthInfoShockAbsorber::GetDefaultControlPoints(std::vector<lcPieceControlPoint>& ControlPoints) const
{
	ControlPoints.resize(2);

	ControlPoints[0].Transform = lcMatrix44Translation(lcVector3(0.0f, 0.0f, -mLength));
	ControlPoints[1].Transform = lcMatrix44Translation(lcVector3(0.0f, 0.0f, 0.0f));

	ControlPoints[0].Scale = 1.0f;
	ControlPoints[1].Scale = 1.0f;
}

void lcSynthInfoActuator::GetDefaultControlPoints(std::vector<lcPieceControlPoint>& ControlPoints) const
{
	ControlPoints.resize(2);

	ControlPoints[0].Transform = lcMatrix44(lcMatrix33(lcVector3(1.0f, 0.0f, 0.0f), lcVector3(0.0f, 0.0f, -1.0f), lcVector3(0.0f, 1.0f, 0.0f)), lcVector3(0.0f, 0.0f, 0.0f));
	ControlPoints[1].Transform = lcMatrix44(lcMatrix33(lcVector3(1.0f, 0.0f, 0.0f), lcVector3(0.0f, 0.0f, -1.0f), lcVector3(0.0f, 1.0f, 0.0f)), lcVector3(0.0f, mLength, 0.0f));

	ControlPoints[0].Scale = 1.0f;
	ControlPoints[1].Scale = 1.0f;
}

void lcSynthInfoUniversalJoint::GetDefaultControlPoints(std::vector<lcPieceControlPoint>& ControlPoints) const
{
	ControlPoints.resize(1);
	float HalfLength = mLength / 2;

	ControlPoints[0].Transform = lcMatrix44Translation(lcVector3(0.0f, HalfLength, 0.0f));
	ControlPoints[0].Scale = 1.0f;
}

void lcSynthInfoUniversalJoint::VerifyControlPoints(std::vector<lcPieceControlPoint>& ControlPoints) const
{
	if (ControlPoints.empty())
		GetDefaultControlPoints(ControlPoints);
	else
		ControlPoints.resize(1);
}

float lcSynthInfoCurved::GetSectionTwist(const lcMatrix44& StartTransform, const lcMatrix44& EndTransform) const
{
	lcVector3 StartTangent(StartTransform[1].x, StartTransform[1].y, StartTransform[1].z);
	lcVector3 EndTangent(EndTransform[1].x, EndTransform[1].y, EndTransform[1].z);
	lcVector3 StartUp(StartTransform[2].x, StartTransform[2].y, StartTransform[2].z);
	lcVector3 EndUp(EndTransform[2].x, EndTransform[2].y, EndTransform[2].z);

	float TangentDot = lcDot(StartTangent, EndTangent);
	float UpDot = lcDot(StartUp, EndUp);

	if (TangentDot > 0.99f && UpDot > 0.99f)
		return 0.0f;

	if (fabs(TangentDot) > 0.99f)
	{
		return acosf(lcClamp(lcDot(EndUp, StartUp), -1.0f, 1.0f));
	}
	else if (TangentDot > -0.99f)
	{
		lcVector3 Axis = lcCross(StartTangent, EndTangent);
		float Angle = acosf(lcClamp(TangentDot, -1.0f, 1.0f));

		lcMatrix33 Rotation = lcMatrix33FromAxisAngle(Axis, Angle);
		lcVector3 AdjustedStartUp = lcMul(StartUp, Rotation);
		return acosf(lcClamp(lcDot(EndUp, AdjustedStartUp), -1.0f, 1.0f));
	}

	lcVector3 StartSide(StartTransform[0].x, StartTransform[0].y, StartTransform[0].z);
	lcVector3 EndSide(EndTransform[0].x, EndTransform[0].y, EndTransform[0].z);

	float SideDot = lcDot(StartSide, EndSide);

	if (fabs(SideDot) < 0.99f)
	{
		lcVector3 Axis = lcCross(StartSide, EndSide);
		float Angle = acosf(SideDot);

		lcMatrix33 Rotation = lcMatrix33FromAxisAngle(Axis, Angle);
		lcVector3 AdjustedStartUp = lcMul(StartUp, Rotation);
		return acosf(lcClamp(lcDot(EndUp, AdjustedStartUp), -1.0f, 1.0f));
	}

	return 0.0f;
}

void lcSynthInfoCurved::CalculateSections(const std::vector<lcPieceControlPoint>& ControlPoints, std::vector<lcMatrix44>& Sections, SectionCallbackFunc SectionCallback) const
{
	if (ControlPoints.empty())
		return;

	float SectionLength = 0.0f;

	for (quint32 ControlPointIndex = 0; ControlPointIndex < ControlPoints.size() - 1 && Sections.size() < mSectionCount + 2; ControlPointIndex++)
	{
		lcVector3 SegmentControlPoints[4];

		lcMatrix44 StartTransform = lcMatrix44LeoCADToLDraw(ControlPoints[ControlPointIndex].Transform);
		lcMatrix44 EndTransform = lcMatrix44LeoCADToLDraw(ControlPoints[ControlPointIndex + 1].Transform);
		StartTransform = lcMatrix44(lcMul(lcMul(lcMatrix33(mStart.Transform), lcMatrix33(StartTransform)), lcMatrix33Scale(lcVector3(1.0f, -1.0f, 1.0f))), StartTransform.GetTranslation());

		if (ControlPointIndex == 0)
		{
			if (mRigidEdges)
			{
				StartTransform.SetTranslation(lcMul30(lcVector3(0.0f, mStart.Length, 0.0f), StartTransform) + StartTransform.GetTranslation());
				SectionLength = 0.0f;
			}
			else
				SectionLength = mStart.Length;

			Sections.emplace_back(StartTransform);
		}

		EndTransform = lcMatrix44(lcMul(lcMul(lcMatrix33(mEnd.Transform), lcMatrix33(EndTransform)), lcMatrix33Scale(lcVector3(1.0f, -1.0f, 1.0f))), EndTransform.GetTranslation());

		SegmentControlPoints[0] = StartTransform.GetTranslation();
		SegmentControlPoints[1] = lcMul31(lcVector3(0.0f, ControlPoints[ControlPointIndex].Scale, 0.0f), StartTransform);
		SegmentControlPoints[2] = lcMul31(lcVector3(0.0f, -ControlPoints[ControlPointIndex + 1].Scale, 0.0f), EndTransform);
		SegmentControlPoints[3] = EndTransform.GetTranslation();

		const int NumCurvePoints = 4096;
		std::vector<lcVector3> CurvePoints;
		CurvePoints.reserve(NumCurvePoints);

		for (int PointIdx = 0; PointIdx < NumCurvePoints; PointIdx++)
		{
			float t = (float)PointIdx / (float)(NumCurvePoints - 1);
			float it = 1.0f - t;

			lcVector3 Position = it * it * it * SegmentControlPoints[0] + it * it * 3.0f * t * SegmentControlPoints[1] + it * 3.0 * t * t * SegmentControlPoints[2] + t * t * t * SegmentControlPoints[3];
			CurvePoints.emplace_back(Position);
		}

		float CurrentSegmentLength = 0.0f;
		float TotalSegmentLength = 0.0f;

		for (size_t PointIdx = 0; PointIdx < CurvePoints.size() - 1; PointIdx++)
			TotalSegmentLength += lcLength(CurvePoints[PointIdx] - CurvePoints[PointIdx + 1]);

		lcVector3 StartUp = lcMul30(lcVector3(1.0f, 0.0f, 0.0f), StartTransform);
		float Twist = GetSectionTwist(StartTransform, EndTransform);
		size_t CurrentPointIndex = 0;

		while (CurrentPointIndex < CurvePoints.size() - 1)
		{
			float Length = lcLength(CurvePoints[CurrentPointIndex + 1] - CurvePoints[CurrentPointIndex]);
			CurrentSegmentLength += Length;
			SectionLength -= Length;
			CurrentPointIndex++;

			if (SectionLength > 0.0f)
				continue;

			float t = (float)CurrentPointIndex / (float)(NumCurvePoints - 1);
			float it = 1.0f - t;

			lcVector3 Tangent = lcNormalize(-3.0f * it * it * SegmentControlPoints[0] + (3.0f * it * it - 6.0f * t * it) * SegmentControlPoints[1] + (-3.0f * t * t + 6.0f * t * it) * SegmentControlPoints[2] + 3.0f * t * t * SegmentControlPoints[3]);
			lcVector3 Up;

			if (Twist)
			{
				Up = lcMul(StartUp, lcMatrix33FromAxisAngle(Tangent, Twist * (CurrentSegmentLength / TotalSegmentLength)));
				CurrentSegmentLength = 0.0f;
			}
			else
				Up = StartUp;

			lcVector3 Side = lcNormalize(lcCross(Tangent, Up));
			Up = lcNormalize(lcCross(Side, Tangent));
			StartUp = Up;

			Sections.emplace_back(lcMatrix44(lcMatrix33(Up, Tangent, Side), CurvePoints[CurrentPointIndex]));

			if (SectionCallback)
				SectionCallback(CurvePoints[CurrentPointIndex], ControlPointIndex, t);

			if (Sections.size() == mSectionCount + 2)
				break;

			if (mCenterLength != 0.0f && (Sections.size() == mSectionCount / 2 + 1))
				SectionLength += mCenterLength;
			else
				SectionLength += mMiddle.Length;

			if (Sections.size() == mSectionCount + 1 && !mRigidEdges)
				SectionLength += mEnd.Length;
		}
	}

	while (Sections.size() < mSectionCount + 2)
	{
		lcMatrix44 EndTransform = lcMatrix44LeoCADToLDraw(ControlPoints.back().Transform);
		EndTransform = lcMatrix44(lcMul(lcMul(lcMatrix33(mEnd.Transform), lcMatrix33(EndTransform)), lcMatrix33Scale(lcVector3(1.0f, -1.0f, 1.0f))), EndTransform.GetTranslation());
		lcVector3 Position = lcMul31(lcVector3(0.0f, SectionLength, 0.0f), EndTransform);
		EndTransform.SetTranslation(Position);
		Sections.emplace_back(EndTransform);

		if (SectionCallback)
			SectionCallback(Position, static_cast<quint32>(ControlPoints.size()) - 1, 1.0f);

		if (mCenterLength != 0.0f && (Sections.size() == mSectionCount / 2 + 1))
			SectionLength += mCenterLength;
		else
			SectionLength += mMiddle.Length;

		if (Sections.size() == mSectionCount + 1 && !mRigidEdges)
			SectionLength += mEnd.Length;
	}
}

void lcSynthInfoBraidedString::CalculateSections(const std::vector<lcPieceControlPoint>& ControlPoints, std::vector<lcMatrix44>& Sections, SectionCallbackFunc SectionCallback) const
{
	if (ControlPoints.empty())
		return;

	float SectionLength = 0.0f;

	for (quint32 ControlPointIndex = 0; ControlPointIndex < ControlPoints.size() - 1 && Sections.size() < mSectionCount + 2; ControlPointIndex++)
	{
		lcVector3 SegmentControlPoints[4];

		lcMatrix44 StartTransform = lcMatrix44LeoCADToLDraw(ControlPoints[ControlPointIndex].Transform);
		lcMatrix44 EndTransform = lcMatrix44LeoCADToLDraw(ControlPoints[ControlPointIndex + 1].Transform);
		StartTransform = lcMatrix44(lcMul(lcMul(lcMatrix33(mStart.Transform), lcMatrix33(StartTransform)), lcMatrix33Scale(lcVector3(1.0f, -1.0f, 1.0f))), StartTransform.GetTranslation());

		if (ControlPointIndex == 0)
		{
			if (mRigidEdges)
			{
				StartTransform.SetTranslation(lcMul30(lcVector3(mStart.Length, 0.0f, 0.0f), StartTransform) + StartTransform.GetTranslation());
				SectionLength = 0.0f;
			}
			else
				SectionLength = mStart.Length;

			Sections.emplace_back(StartTransform);
		}

		EndTransform = lcMatrix44(lcMul(lcMul(lcMatrix33(mEnd.Transform), lcMatrix33(EndTransform)), lcMatrix33Scale(lcVector3(1.0f, -1.0f, 1.0f))), EndTransform.GetTranslation());

		SegmentControlPoints[0] = StartTransform.GetTranslation();
		SegmentControlPoints[1] = lcMul31(lcVector3(ControlPoints[ControlPointIndex].Scale, 0.0f, 0.0f), StartTransform);
		SegmentControlPoints[2] = lcMul31(lcVector3(-ControlPoints[ControlPointIndex + 1].Scale, 0.0f, 0.0f), EndTransform);
		SegmentControlPoints[3] = EndTransform.GetTranslation();

		const int NumCurvePoints = 4096;
		std::vector<lcVector3> CurvePoints;
		CurvePoints.reserve(NumCurvePoints);

		for (int PointIdx = 0; PointIdx < NumCurvePoints; PointIdx++)
		{
			float t = (float)PointIdx / (float)(NumCurvePoints - 1);
			float it = 1.0f - t;

			lcVector3 Position = it * it * it * SegmentControlPoints[0] + it * it * 3.0f * t * SegmentControlPoints[1] + it * 3.0 * t * t * SegmentControlPoints[2] + t * t * t * SegmentControlPoints[3];
			CurvePoints.emplace_back(Position);
		}

		float CurrentSegmentLength = 0.0f;
		float TotalSegmentLength = 0.0f;

		for (size_t PointIdx = 0; PointIdx < CurvePoints.size() - 1; PointIdx++)
			TotalSegmentLength += lcLength(CurvePoints[PointIdx] - CurvePoints[PointIdx + 1]);

		lcVector3 StartUp = lcMul30(lcVector3(0.0f, 1.0f, 0.0f), StartTransform);
		float Twist = GetSectionTwist(StartTransform, EndTransform);
		size_t CurrentPointIndex = 0;

		while (CurrentPointIndex < CurvePoints.size() - 1)
		{
			float Length = lcLength(CurvePoints[CurrentPointIndex + 1] - CurvePoints[CurrentPointIndex]);
			CurrentSegmentLength += Length;
			SectionLength -= Length;
			CurrentPointIndex++;

			if (SectionLength > 0.0f)
				continue;

			float t = (float)CurrentPointIndex / (float)(NumCurvePoints - 1);
			float it = 1.0f - t;

			lcVector3 Tangent = lcNormalize(-3.0f * it * it * SegmentControlPoints[0] + (3.0f * it * it - 6.0f * t * it) * SegmentControlPoints[1] + (-3.0f * t * t + 6.0f * t * it) * SegmentControlPoints[2] + 3.0f * t * t * SegmentControlPoints[3]);
			lcVector3 Up;

			if (Twist)
			{
				Up = lcMul(StartUp, lcMatrix33FromAxisAngle(Tangent, Twist * (CurrentSegmentLength / TotalSegmentLength)));
				CurrentSegmentLength = 0.0f;
			}
			else
				Up = StartUp;

			lcVector3 Side = lcNormalize(lcCross(Tangent, Up));
			Up = lcNormalize(lcCross(Side, Tangent));
			StartUp = Up;

			Sections.emplace_back(lcMatrix44(lcMatrix33(Tangent, Up, -Side), CurvePoints[CurrentPointIndex]));

			if (SectionCallback)
				SectionCallback(CurvePoints[CurrentPointIndex], ControlPointIndex, t);

			if (Sections.size() == mSectionCount + 2)
				break;

			if (mCenterLength != 0.0f && (Sections.size() == mSectionCount / 2 + 1))
				SectionLength += mCenterLength;
			else
				SectionLength += mMiddle.Length;

			if (Sections.size() == mSectionCount + 1 && !mRigidEdges)
				SectionLength += mEnd.Length;
		}
	}

	while (Sections.size() < mSectionCount + 2)
	{
		lcMatrix44 EndTransform = lcMatrix44LeoCADToLDraw(ControlPoints.back().Transform);
		EndTransform = lcMatrix44(lcMul(lcMul(lcMatrix33(mEnd.Transform), lcMatrix33(EndTransform)), lcMatrix33Scale(lcVector3(1.0f, -1.0f, 1.0f))), EndTransform.GetTranslation());
		lcVector3 Position = lcMul31(lcVector3(SectionLength, 0.0f, 0.0f), EndTransform);
		EndTransform.SetTranslation(Position);
		Sections.emplace_back(EndTransform);

		if (SectionCallback)
			SectionCallback(Position, static_cast<quint32>(ControlPoints.size()) - 1, 1.0f);

		if (mCenterLength != 0.0f && (Sections.size() == mSectionCount / 2 + 1))
			SectionLength += mCenterLength;
		else
			SectionLength += mMiddle.Length;

		if (Sections.size() == mSectionCount + 1 && !mRigidEdges)
			SectionLength += mEnd.Length;
	}
}

void lcSynthInfoStraight::CalculateSections(const std::vector<lcPieceControlPoint>& ControlPoints, std::vector<lcMatrix44>& Sections, SectionCallbackFunc SectionCallback) const
{
	for (quint32 ControlPointIndex = 0; ControlPointIndex < ControlPoints.size(); ControlPointIndex++)
	{
		lcMatrix44 Transform = lcMatrix44LeoCADToLDraw(ControlPoints[ControlPointIndex].Transform);
		Sections.emplace_back(Transform);

		if (SectionCallback)
			SectionCallback(Transform.GetTranslation(), ControlPointIndex, 1.0f);
	}
}

void lcSynthInfoUniversalJoint::CalculateSections(const std::vector<lcPieceControlPoint>& ControlPoints, std::vector<lcMatrix44>& Sections, SectionCallbackFunc SectionCallback) const
{
	for (quint32 ControlPointIndex = 0; ControlPointIndex < ControlPoints.size(); ControlPointIndex++)
	{
		lcMatrix44 Transform = lcMatrix44LeoCADToLDraw(ControlPoints[ControlPointIndex].Transform);
		Sections.emplace_back(Transform);

		if (SectionCallback)
			SectionCallback(Transform.GetTranslation(), ControlPointIndex, 1.0f);
	}
}

void lcSynthInfoFlexibleHose::AddParts(lcMemFile& File, lcLibraryMeshData&, const std::vector<lcMatrix44>& Sections) const
{
	if (Sections.empty())
		return;

	char Line[256];
	const int NumEdgeParts = 2;

	const lcMatrix33 EdgeTransforms[2] =
	{
		lcMatrix33(lcVector3(0.0f, 0.0f, -1.0f), lcVector3(0.0f, -1.0f, 0.0f), lcVector3(1.0f, 0.0f, 0.0f)),
		lcMatrix33(lcVector3(0.0f, 0.0f, -1.0f), lcVector3(0.0f,  1.0f, 0.0f), lcVector3(1.0f, 0.0f, 0.0f))
	};

	const char* EdgeParts[2] =
	{
		"755.dat", mEdgePart2
	};

	for (int PartIdx = 0; PartIdx < NumEdgeParts; PartIdx++)
	{
		const int SectionIndex = 0;
		lcMatrix33 Transform(lcMul(lcMul(EdgeTransforms[PartIdx], lcMatrix33Scale(lcVector3(1.0f, -1.0f, 1.0f))), lcMatrix33(Sections[SectionIndex])));
		lcVector3 Offset = lcMul31(lcVector3(0.0f, -5.0f, 0.0f), Sections[SectionIndex]);

		sprintf(Line, "1 16 %f %f %f %f %f %f %f %f %f %f %f %f %s\n", Offset[0], Offset[1], Offset[2], Transform[0][0], Transform[1][0], Transform[2][0],
				Transform[0][1], Transform[1][1], Transform[2][1], Transform[0][2], Transform[1][2], Transform[2][2], EdgeParts[PartIdx]);

		File.WriteBuffer(Line, strlen(Line));
	}

	for (size_t SectionIndex = 1; SectionIndex < Sections.size() - 1; SectionIndex++)
	{
		lcMatrix33 Transform = lcMatrix33(Sections[SectionIndex]);
		lcVector3 Offset = Sections[SectionIndex].GetTranslation();

		if (SectionIndex < Sections.size() / 2)
			Transform = lcMul(lcMatrix33(lcVector3(0.0f, 0.0f, -1.0f), lcVector3(0.0f, -1.0f, 0.0f), lcVector3(-1.0f, 0.0f, 0.0f)), Transform);
		else if (SectionIndex != Sections.size() / 2)
			Transform = lcMul(lcMatrix33(lcVector3(0.0f, 0.0f, -1.0f), lcVector3(0.0f,  1.0f, 0.0f), lcVector3( 1.0f, 0.0f, 0.0f)), Transform);
		else
			Transform = lcMul(lcMatrix33(lcVector3(0.0f, 0.0f, -1.0f), lcVector3(0.0f,  1.0f, 0.0f), lcVector3( 1.0f, 0.0f, 0.0f)), Transform);

		const char* Part = SectionIndex != Sections.size() / 2 ? "754.dat" : "756.dat";

		sprintf(Line, "1 16 %f %f %f %f %f %f %f %f %f %f %f %f %s\n", Offset[0], Offset[1], Offset[2], Transform[0][0], Transform[1][0], Transform[2][0],
				Transform[0][1], Transform[1][1], Transform[2][1], Transform[0][2], Transform[1][2], Transform[2][2], Part);

		File.WriteBuffer(Line, strlen(Line));
	}

	for (int PartIdx = 0; PartIdx < NumEdgeParts; PartIdx++)
	{
		const size_t SectionIndex = Sections.size() - 1;
		lcMatrix33 Transform(lcMul(EdgeTransforms[PartIdx], lcMatrix33(Sections[SectionIndex])));
		lcVector3 Offset = lcMul31(lcVector3(0.0f, 5.0f - 2.56f, 0.0f), Sections[SectionIndex]);

		sprintf(Line, "1 16 %f %f %f %f %f %f %f %f %f %f %f %f %s\n", Offset[0], Offset[1], Offset[2], Transform[0][0], Transform[1][0], Transform[2][0],
				Transform[0][1], Transform[1][1], Transform[2][1], Transform[0][2], Transform[1][2], Transform[2][2], EdgeParts[PartIdx]);

		File.WriteBuffer(Line, strlen(Line));
	}
}

void lcSynthInfoCurved::AddTubeParts(lcLibraryMeshData& MeshData, const std::vector<lcMatrix44>& Sections, float Radius, bool IsInner)
{
	// a unit circle
	static const lcVector3 Vertices[16] =
	{
		{  1.000000f, 0.0f,  0.000000f },
		{  0.923880f, 0.0f,  0.382683f },
		{  0.707107f, 0.0f,  0.707107f },
		{  0.382683f, 0.0f,  0.923880f },
		{  0.000000f, 0.0f,  1.000000f },
		{ -0.382683f, 0.0f,  0.923880f },
		{ -0.707107f, 0.0f,  0.707107f },
		{ -0.923880f, 0.0f,  0.382683f },
		{ -1.000000f, 0.0f,  0.000000f },
		{ -0.923879f, 0.0f, -0.382684f },
		{ -0.707107f, 0.0f, -0.707107f },
		{ -0.382683f, 0.0f, -0.923880f },
		{  0.000000f, 0.0f, -1.000000f },
		{  0.382684f, 0.0f, -0.923879f },
		{  0.707107f, 0.0f, -0.707107f },
		{  0.923880f, 0.0f, -0.382683f },
	};
	const int NumSectionVertices = LC_ARRAY_COUNT(Vertices);

	int BaseVertex;
	lcMeshLoaderVertex* VertexBuffer;
	quint32* IndexBuffer;
	MeshData.AddVertices(LC_MESHDATA_SHARED, NumSectionVertices * (Sections.size() - 1), &BaseVertex, &VertexBuffer);

	float NormalDirection = IsInner ? -1.0f : 1.0f;

	for (size_t SectionIndex = 1; SectionIndex < Sections.size(); SectionIndex++)
	{
		for (int VertexIdx = 0; VertexIdx < NumSectionVertices; VertexIdx++)
		{
			VertexBuffer->Position = lcMul31(Radius * Vertices[VertexIdx], Sections[SectionIndex]);
			VertexBuffer->Normal = lcMul30(NormalDirection * Vertices[VertexIdx], Sections[SectionIndex]);
			VertexBuffer->NormalWeight = 4.0f;
			VertexBuffer++;
		}
	}

	MeshData.AddIndices(LC_MESHDATA_SHARED, LC_MESH_TRIANGLES, 16, 6 * NumSectionVertices * (Sections.size() - 2), &IndexBuffer);

	int Offset1, Offset2;
	if (IsInner)
	{
		Offset1 = NumSectionVertices - 1;	// -1 mod NumVertices
		Offset2 = 0;
	}
	else
	{
		Offset1 = 0;
		Offset2 = 1;
	}

	for (size_t SectionIndex = 1; SectionIndex < Sections.size() - 1; SectionIndex++)
	{
		for (int VertexIdx = 0; VertexIdx < NumSectionVertices; VertexIdx++)
		{
			int Vertex1 = BaseVertex + (VertexIdx + Offset1) % NumSectionVertices;
			int Vertex2 = BaseVertex + (VertexIdx + Offset2) % NumSectionVertices;

			*IndexBuffer++ = Vertex1;
			*IndexBuffer++ = Vertex2;
			*IndexBuffer++ = Vertex1 + NumSectionVertices;
			*IndexBuffer++ = Vertex2;
			*IndexBuffer++ = Vertex2 + NumSectionVertices;
			*IndexBuffer++ = Vertex1 + NumSectionVertices;
		}
		BaseVertex += NumSectionVertices;
	}
}

void lcSynthInfoFlexSystemHose::AddParts(lcMemFile& File, lcLibraryMeshData& MeshData, const std::vector<lcMatrix44>& Sections) const
{
	if (Sections.empty())
		return;

	char Line[256];

	{
		const size_t SectionIndex = 0;
		lcMatrix33 Transform(lcMul(lcMatrix33Scale(lcVector3(-1.0f, 1.0f, 1.0f)), lcMatrix33(Sections[SectionIndex])));
		lcVector3 Offset = lcMul31(lcVector3(0.0f, -1.0f, 0.0f), Sections[SectionIndex]);

		sprintf(Line, "1 16 %f %f %f %f %f %f %f %f %f %f %f %f u9053.dat\n", Offset[0], Offset[1], Offset[2], Transform[0][0], Transform[1][0], Transform[2][0],
				Transform[0][1], Transform[1][1], Transform[2][1], Transform[0][2], Transform[1][2], Transform[2][2]);

		File.WriteBuffer(Line, strlen(Line));
	}

	{
		const size_t SectionIndex = Sections.size() - 1;
		lcMatrix33 Transform(lcMul(lcMatrix33Scale(lcVector3(1.0f, -1.0f, 1.0f)), lcMatrix33(Sections[SectionIndex])));
		lcVector3 Offset = lcMul31(lcVector3(0.0f, 1.0f, 0.0f), Sections[SectionIndex]);

		sprintf(Line, "1 16 %f %f %f %f %f %f %f %f %f %f %f %f u9053.dat\n", Offset[0], Offset[1], Offset[2], Transform[0][0], Transform[1][0], Transform[2][0],
				Transform[0][1], Transform[1][1], Transform[2][1], Transform[0][2], Transform[1][2], Transform[2][2]);

		File.WriteBuffer(Line, strlen(Line));
	}

	AddTubeParts(MeshData, Sections, 4.0f, false);
	AddTubeParts(MeshData, Sections, 2.0f, true);
}

void lcSynthInfoPneumaticTube::AddParts(lcMemFile& File, lcLibraryMeshData& MeshData, const std::vector<lcMatrix44>& Sections) const
{
	if (Sections.empty())
		return;

	char Line[256];

	{
		const size_t SectionIndex = 0;
		lcMatrix33 EdgeTransform(lcVector3(0.0f, 0.0f, -1.0f), lcVector3(0.0f, -1.0f, 0.0f), lcVector3(1.0f, 0.0f, 0.0f));
		lcMatrix33 Transform(lcMul(lcMul(EdgeTransform, lcMatrix33Scale(lcVector3(1.0f, 1.0f, 1.0f))), lcMatrix33(Sections[SectionIndex])));
		lcVector3 Offset = lcMul31(lcVector3(0.0f, 0.0f, 0.0f), Sections[SectionIndex]);

		sprintf(Line, "1 16 %f %f %f %f %f %f %f %f %f %f %f %f %s\n", Offset[0], Offset[1], Offset[2], Transform[0][0], Transform[1][0], Transform[2][0],
				Transform[0][1], Transform[1][1], Transform[2][1], Transform[0][2], Transform[1][2], Transform[2][2],
				mEndPart);

		File.WriteBuffer(Line, strlen(Line));
	}

	{
		const size_t SectionIndex = Sections.size() - 1;
		lcMatrix33 EdgeTransform(lcVector3(0.0f, 0.0f, -1.0f), lcVector3(0.0f, -1.0f, 0.0f), lcVector3(1.0f, 0.0f, 0.0f));
		lcMatrix33 Transform(lcMul(lcMul(EdgeTransform, lcMatrix33Scale(lcVector3(1.0f, -1.0f, 1.0f))), lcMatrix33(Sections[SectionIndex])));
		lcVector3 Offset = lcMul31(lcVector3(0.0f, 0.0f, 0.0f), Sections[SectionIndex]);

		sprintf(Line, "1 16 %f %f %f %f %f %f %f %f %f %f %f %f %s\n", Offset[0], Offset[1], Offset[2], Transform[0][0], Transform[1][0], Transform[2][0],
				Transform[0][1], Transform[1][1], Transform[2][1], Transform[0][2], Transform[1][2], Transform[2][2],
				mEndPart);

		File.WriteBuffer(Line, strlen(Line));
	}

	AddTubeParts(MeshData, Sections, 5.0f, false);
	AddTubeParts(MeshData, Sections, 3.0f, true);
}

void lcSynthInfoRibbedHose::AddParts(lcMemFile& File, lcLibraryMeshData&, const std::vector<lcMatrix44>& Sections) const
{
	if (Sections.empty())
		return;

	char Line[256];

	{
		const int SectionIndex = 0;
		lcMatrix33 Transform(lcMul(lcMatrix33Scale(lcVector3(1.0f, -1.0f, 1.0f)), lcMatrix33(Sections[SectionIndex])));
		lcVector3 Offset = Sections[SectionIndex].GetTranslation();

		sprintf(Line, "1 16 %f %f %f %f %f %f %f %f %f %f %f %f 79.dat\n", Offset[0], Offset[1], Offset[2], Transform[0][0], Transform[1][0], Transform[2][0],
				Transform[0][1], Transform[1][1], Transform[2][1], Transform[0][2], Transform[1][2], Transform[2][2]);

		File.WriteBuffer(Line, strlen(Line));
	}

	for (size_t SectionIndex = 1; SectionIndex < Sections.size() - 1; SectionIndex++)
	{
		const lcMatrix44& Transform = Sections[SectionIndex];

		sprintf(Line, "1 16 %f %f %f %f %f %f %f %f %f %f %f %f 80.dat\n", Transform[3][0], Transform[3][1], Transform[3][2], Transform[0][0], Transform[1][0], Transform[2][0],
				Transform[0][1], Transform[1][1], Transform[2][1], Transform[0][2], Transform[1][2], Transform[2][2]);

		File.WriteBuffer(Line, strlen(Line));
	}

	{
		const size_t SectionIndex = Sections.size() - 1;
		lcMatrix33 Transform(Sections[SectionIndex]);
		lcVector3 Offset = lcMul31(lcVector3(0.0f, -6.25f, 0.0f), Sections[SectionIndex]);

		sprintf(Line, "1 16 %f %f %f %f %f %f %f %f %f %f %f %f 79.dat\n", Offset[0], Offset[1], Offset[2], Transform[0][0], Transform[1][0], Transform[2][0],
				Transform[0][1], Transform[1][1], Transform[2][1], Transform[0][2], Transform[1][2], Transform[2][2]);

		File.WriteBuffer(Line, strlen(Line));
	}
}

void lcSynthInfoFlexibleAxle::AddParts(lcMemFile& File, lcLibraryMeshData& MeshData, const std::vector<lcMatrix44>& Sections) const
{
	if (Sections.empty())
		return;

	char Line[256];
	const int NumEdgeParts = 6;

	lcMatrix33 EdgeTransforms[6] = 
	{
		lcMatrix33(lcVector3(-1.0f, 0.0f, 0.0f), lcVector3(0.0f, -5.0f, 0.0f), lcVector3(0.0f, 0.0f,  1.0f)),
		lcMatrix33(lcVector3( 0.0f, 1.0f, 0.0f), lcVector3(1.0f,  0.0f, 0.0f), lcVector3(0.0f, 0.0f, -1.0f)),
		lcMatrix33(lcVector3( 0.0f, 1.0f, 0.0f), lcVector3(1.0f,  0.0f, 0.0f), lcVector3(0.0f, 0.0f, -1.0f)),
		lcMatrix33(lcVector3( 0.0f, 1.0f, 0.0f), lcVector3(1.0f,  0.0f, 0.0f), lcVector3(0.0f, 0.0f, -1.0f)),
		lcMatrix33(lcVector3( 0.0f, 1.0f, 0.0f), lcVector3(1.0f,  0.0f, 0.0f), lcVector3(0.0f, 0.0f, -1.0f)),
		lcMatrix33(lcVector3( 0.0f, 1.0f, 0.0f), lcVector3(1.0f,  0.0f, 0.0f), lcVector3(0.0f, 0.0f, -1.0f)),
	};

	const char* EdgeParts[6] =
	{
		"stud3a.dat", "s/faxle1.dat", "s/faxle2.dat", "s/faxle3.dat", "s/faxle4.dat", "s/faxle5.dat"
	};

	for (int PartIdx = 0; PartIdx < NumEdgeParts; PartIdx++)
	{
		const int SectionIndex = 0;
		lcMatrix33 Transform(lcMul(lcMul(EdgeTransforms[PartIdx], lcMatrix33Scale(lcVector3(1.0f, -1.0f, 1.0f))), lcMatrix33(Sections[SectionIndex])));
		lcVector3 Offset = lcMul31(lcVector3(0.0f, -4.0f * (5 - PartIdx), 0.0f), Sections[SectionIndex]);

		sprintf(Line, "1 16 %f %f %f %f %f %f %f %f %f %f %f %f %s\n", Offset[0], Offset[1], Offset[2], Transform[0][0], Transform[1][0], Transform[2][0],
				Transform[0][1], Transform[1][1], Transform[2][1], Transform[0][2], Transform[1][2], Transform[2][2], EdgeParts[PartIdx]);

		File.WriteBuffer(Line, strlen(Line));
	}

	for (int PartIdx = 0; PartIdx < NumEdgeParts; PartIdx++)
	{
		const size_t SectionIndex = Sections.size() - 1;
		lcMatrix33 Transform(lcMul(EdgeTransforms[PartIdx], lcMatrix33(Sections[SectionIndex])));
		lcVector3 Offset = lcMul31(lcVector3(0.0f, 4.0f * (5 - PartIdx), 0.0f), Sections[SectionIndex]);

		sprintf(Line, "1 16 %f %f %f %f %f %f %f %f %f %f %f %f %s\n", Offset[0], Offset[1], Offset[2], Transform[0][0], Transform[1][0], Transform[2][0],
				Transform[0][1], Transform[1][1], Transform[2][1], Transform[0][2], Transform[1][2], Transform[2][2], EdgeParts[PartIdx]);

		File.WriteBuffer(Line, strlen(Line));
	}

	const lcMeshLoaderVertex SectionVertices[28] =
	{
		{ lcVector3(-6.000f, 0.0f,  0.000f), lcVector3(-1.000f, 0.0f,  0.000f), 2.0f },
		{ lcVector3(-5.602f, 0.0f,  2.000f), lcVector3(-0.942f, 0.0f,  0.336f), 4.0f },
		{ lcVector3(-5.602f, 0.0f,  2.000f), lcVector3( 0.000f, 0.0f,  1.000f), 4.0f },
		{ lcVector3(-2.000f, 0.0f,  2.000f), lcVector3( 0.000f, 0.0f,  1.000f), 4.0f },
		{ lcVector3(-2.000f, 0.0f,  2.000f), lcVector3(-1.000f, 0.0f,  0.000f), 4.0f },
		{ lcVector3(-2.000f, 0.0f,  5.602f), lcVector3(-1.000f, 0.0f,  0.000f), 4.0f },
		{ lcVector3(-2.000f, 0.0f,  5.602f), lcVector3(-0.336f, 0.0f,  0.942f), 4.0f },
		{ lcVector3( 0.000f, 0.0f,  6.000f), lcVector3( 0.000f, 0.0f,  1.000f), 2.0f },
		{ lcVector3( 2.000f, 0.0f,  5.602f), lcVector3( 0.336f, 0.0f,  0.942f), 4.0f },
		{ lcVector3( 2.000f, 0.0f,  5.602f), lcVector3( 1.000f, 0.0f,  0.000f), 4.0f },
		{ lcVector3( 2.000f, 0.0f,  2.000f), lcVector3( 1.000f, 0.0f,  0.000f), 4.0f },
		{ lcVector3( 2.000f, 0.0f,  2.000f), lcVector3( 0.000f, 0.0f,  1.000f), 4.0f },
		{ lcVector3( 5.602f, 0.0f,  2.000f), lcVector3( 0.000f, 0.0f,  1.000f), 4.0f },
		{ lcVector3( 5.602f, 0.0f,  2.000f), lcVector3( 0.942f, 0.0f,  0.336f), 4.0f },
		{ lcVector3( 6.000f, 0.0f,  0.000f), lcVector3( 1.000f, 0.0f,  0.000f), 2.0f },
		{ lcVector3( 5.602f, 0.0f, -2.000f), lcVector3( 0.942f, 0.0f, -0.336f), 4.0f },
		{ lcVector3( 5.602f, 0.0f, -2.000f), lcVector3( 0.000f, 0.0f, -1.000f), 4.0f },
		{ lcVector3( 2.000f, 0.0f, -2.000f), lcVector3( 0.000f, 0.0f, -1.000f), 4.0f },
		{ lcVector3( 2.000f, 0.0f, -2.000f), lcVector3( 1.000f, 0.0f,  0.000f), 4.0f },
		{ lcVector3( 2.000f, 0.0f, -5.602f), lcVector3( 1.000f, 0.0f,  0.000f), 4.0f },
		{ lcVector3( 2.000f, 0.0f, -5.602f), lcVector3( 0.336f, 0.0f, -0.942f), 4.0f },
		{ lcVector3( 0.000f, 0.0f, -6.000f), lcVector3( 0.000f, 0.0f, -1.000f), 2.0f },
		{ lcVector3(-2.000f, 0.0f, -5.602f), lcVector3(-0.336f, 0.0f, -0.942f), 4.0f },
		{ lcVector3(-2.000f, 0.0f, -5.602f), lcVector3(-1.000f, 0.0f,  0.000f), 4.0f },
		{ lcVector3(-2.000f, 0.0f, -2.000f), lcVector3(-1.000f, 0.0f,  0.000f), 4.0f },
		{ lcVector3(-2.000f, 0.0f, -2.000f), lcVector3( 0.000f, 0.0f, -1.000f), 4.0f },
		{ lcVector3(-5.602f, 0.0f, -2.000f), lcVector3( 0.000f, 0.0f, -1.000f), 4.0f },
		{ lcVector3(-5.602f, 0.0f, -2.000f), lcVector3(-0.942f, 0.0f, -0.336f), 4.0f }
	};

	const int NumSectionVertices = LC_ARRAY_COUNT(SectionVertices);

	int BaseVertex;
	lcMeshLoaderVertex* VertexBuffer;
	quint32* IndexBuffer;
	MeshData.AddVertices(LC_MESHDATA_SHARED, NumSectionVertices * (Sections.size() - 1), &BaseVertex, &VertexBuffer);
	MeshData.AddIndices(LC_MESHDATA_SHARED, LC_MESH_LINES, 24, 2 * 12 * (Sections.size() - 2), &IndexBuffer);

	for (size_t SectionIndex = 1; SectionIndex < Sections.size(); SectionIndex++)
	{
		for (int VertexIdx = 0; VertexIdx < NumSectionVertices; VertexIdx++)
		{
			VertexBuffer->Position = lcMul31(SectionVertices[VertexIdx].Position, Sections[SectionIndex]);
			VertexBuffer->Normal = lcMul30(SectionVertices[VertexIdx].Normal, Sections[SectionIndex]);
			VertexBuffer->NormalWeight = SectionVertices[VertexIdx].NormalWeight;
			VertexBuffer++;
		}
	}

	int BaseLinesVertex = BaseVertex;

	for (size_t SectionIndex = 1; SectionIndex < Sections.size() - 1; SectionIndex++)
	{
		const int Indices[] = { 1, 3, 5, 8, 10, 12, 15, 17, 19, 22, 24, 26 };
	
		for (int VertexIdx = 0; VertexIdx < 12; VertexIdx++)
		{
			*IndexBuffer++ = BaseLinesVertex + Indices[VertexIdx];
			*IndexBuffer++ = BaseLinesVertex + Indices[VertexIdx] + NumSectionVertices;
		}

		BaseLinesVertex += NumSectionVertices;
	}

	MeshData.AddIndices(LC_MESHDATA_SHARED, LC_MESH_TRIANGLES, 16, 6 * NumSectionVertices * (Sections.size() - 2), &IndexBuffer);

	for (size_t SectionIndex = 1; SectionIndex < Sections.size() - 1; SectionIndex++)
	{
		for (int VertexIdx = 0; VertexIdx < NumSectionVertices; VertexIdx++)
		{
			int Vertex1 = BaseVertex + VertexIdx;
			int Vertex2 = BaseVertex + (VertexIdx + 1) % NumSectionVertices;

			*IndexBuffer++ = Vertex1;
			*IndexBuffer++ = Vertex2;
			*IndexBuffer++ = Vertex1 + NumSectionVertices;
			*IndexBuffer++ = Vertex2;
			*IndexBuffer++ = Vertex2 + NumSectionVertices;
			*IndexBuffer++ = Vertex1 + NumSectionVertices;
		}
		BaseVertex += NumSectionVertices;
	}
}

void lcSynthInfoBraidedString::AddParts(lcMemFile& File, lcLibraryMeshData& MeshData, const std::vector<lcMatrix44>& SectionsIn) const
{
	std::vector<lcMatrix44> Sections;
	Sections.resize(SectionsIn.size());

	for (size_t SectionIndex = 0; SectionIndex < Sections.size(); SectionIndex++)
	{
		lcMatrix33 Transform(lcMul(lcMatrix33Scale(lcVector3(1.0f, -1.0f, 1.0f)), lcMatrix33(SectionsIn[SectionIndex])));
		lcVector3 Offset = SectionsIn[SectionIndex].GetTranslation();
		Sections[SectionIndex] = lcMatrix44(Transform, Offset);
	}

	char Line[256];

	{
		const int SectionIndex = 0;
		lcMatrix33 Transform(Sections[SectionIndex]);
		lcVector3 Offset = lcMul31(lcVector3(-8.0f, 0.0f, 0.0f), Sections[SectionIndex]);

		sprintf(Line, "1 16 %f %f %f %f %f %f %f %f %f %f %f %f 572A.dat\n", Offset[0], Offset[1], Offset[2], Transform[0][0], Transform[1][0], Transform[2][0],
				Transform[0][1], Transform[1][1], Transform[2][1], Transform[0][2], Transform[1][2], Transform[2][2]);

		File.WriteBuffer(Line, strlen(Line));
	}

	const size_t NumSegments = 16;
	const int NumBraids = 4;
	const float PositionTable[16] =
	{
		-1.5f, -1.386f, -1.061f, -0.574f, 0.0f, 0.574f, 1.061f, 1.386f, 1.5f, 1.386f, 1.061f, 0.574f, 0.0f, -0.574f, -1.061f, -1.386f
	};

	int BaseVertex;
	lcMeshLoaderVertex* VertexBuffer;
	quint32* IndexBuffer;
	MeshData.AddVertices(LC_MESHDATA_SHARED, NumBraids * ((Sections.size() - 2) * NumSegments + 1), &BaseVertex, &VertexBuffer);
	MeshData.AddIndices(LC_MESHDATA_SHARED, LC_MESH_LINES, 24, NumBraids * (Sections.size() - 2) * NumSegments * 2, &IndexBuffer);

	for (int BraidIdx = 0; BraidIdx < NumBraids; BraidIdx++)
	{
		int BaseX = (BraidIdx == 0 || BraidIdx == 2) ? 0 : 8;
		int BaseY = (BraidIdx == 0 || BraidIdx == 3) ? 12 : 4;

		for (size_t SectionIndex = 1; SectionIndex < Sections.size() - 1; SectionIndex++)
		{
			lcMatrix33 Transform1 = lcMatrix33(Sections[SectionIndex]);
			lcMatrix33 Transform2 = lcMatrix33(Sections[SectionIndex + 1]);
			lcVector3 Offset1 = Sections[SectionIndex].GetTranslation();
			lcVector3 Offset2 = Sections[SectionIndex + 1].GetTranslation();

			for (size_t SegmentIdx = 0; SegmentIdx < ((SectionIndex < Sections.size() - 2) ? NumSegments : NumSegments + 1); SegmentIdx++)
			{
				float t = (float)SegmentIdx / (float)NumSegments;

				lcVector3 Vertex1 = lcVector3(t * 4.0f, PositionTable[(BaseX + SegmentIdx) % NumSegments], PositionTable[(BaseY + SegmentIdx) % NumSegments]) + lcVector3(0.0f, 1.5f, 0.0f);
				lcVector3 Vertex2 = lcVector3((1.0f - t) * -4.0f, PositionTable[(BaseX + SegmentIdx) % NumSegments], PositionTable[(BaseY + SegmentIdx) % NumSegments]) + lcVector3(0.0f, 1.5f, 0.0f);

				lcVector3 Vertex = (lcMul(Vertex1, Transform1) + Offset1) * (1.0f - t) + (lcMul(Vertex2, Transform2) + Offset2) * t;

				VertexBuffer->Position = Vertex;
				VertexBuffer->Normal = lcVector3(0.0f, 0.0f, 0.0f);
				VertexBuffer->NormalWeight = 0.0f;
				VertexBuffer++;

				if (SegmentIdx != NumSegments)
				{
					*IndexBuffer++ = BaseVertex;
					*IndexBuffer++ = BaseVertex + 1;
					BaseVertex++;
				}
			}
		}

		BaseVertex++;
	}

	int NumSlices = 16;
	MeshData.AddVertices(LC_MESHDATA_SHARED, NumSlices * ((Sections.size() - 2) * NumSegments + 1), &BaseVertex, &VertexBuffer);
	MeshData.AddIndices(LC_MESHDATA_SHARED, LC_MESH_TRIANGLES, 16, NumSlices * (Sections.size() - 2) * NumSegments * 6, &IndexBuffer);

	for (size_t SectionIndex = 1; SectionIndex < Sections.size() - 1; SectionIndex++)
	{
		lcMatrix33 Transform1 = lcMatrix33(Sections[SectionIndex]);
		lcMatrix33 Transform2 = lcMatrix33(Sections[SectionIndex + 1]);
		lcVector3 Offset1 = Sections[SectionIndex].GetTranslation();
		lcVector3 Offset2 = Sections[SectionIndex + 1].GetTranslation();

		for (size_t SegmentIdx = 0; SegmentIdx < ((SectionIndex < Sections.size() - 2) ? NumSegments : NumSegments + 1); SegmentIdx++)
		{
			float t1 = (float)SegmentIdx / (float)NumSegments;
			int BaseX = 8;
			int BaseY = 4;

			for (int SliceIdx = 0; SliceIdx < NumSlices; SliceIdx++)
			{
				lcVector3 Vertex11 = lcVector3(t1 * 4.0f, PositionTable[(BaseX + SliceIdx) % NumSlices], PositionTable[(BaseY + SliceIdx) % NumSlices]) + lcVector3(0.0f, 1.5f, 0.0f);
				lcVector3 Vertex12 = lcVector3((1.0f - t1) * -4.0f, PositionTable[(BaseX + SliceIdx) % NumSlices], PositionTable[(BaseY + SliceIdx) % NumSlices]) + lcVector3(0.0f, 1.5f, 0.0f);
				VertexBuffer->Position = (lcMul(Vertex11, Transform1) + Offset1) * (1.0f - t1) + (lcMul(Vertex12, Transform2) + Offset2) * t1;

				lcVector3 Normal11 = lcVector3(0.0f, PositionTable[(BaseX + SliceIdx) % NumSlices] / 1.5f, PositionTable[(BaseY + SliceIdx) % NumSlices] / 1.5f);
				lcVector3 Normal12 = lcVector3(0.0f, PositionTable[(BaseX + SliceIdx) % NumSlices] / 1.5f, PositionTable[(BaseY + SliceIdx) % NumSlices] / 1.5f);
				VertexBuffer->Normal = lcMul(Normal11, Transform1) * (1.0f - t1) + lcMul(Normal12, Transform2) * t1;
				VertexBuffer->NormalWeight = 1.0f;
				VertexBuffer++;

				if (SegmentIdx != NumSegments)
				{
					*IndexBuffer++ = BaseVertex + SliceIdx;
					*IndexBuffer++ = BaseVertex + (SliceIdx + 1) % NumSlices;
					*IndexBuffer++ = BaseVertex + (SliceIdx + 1) % NumSlices + NumSlices;

					*IndexBuffer++ = BaseVertex + SliceIdx + NumSlices;
					*IndexBuffer++ = BaseVertex + SliceIdx;
					*IndexBuffer++ = BaseVertex + (SliceIdx + 1) % NumSlices + NumSlices;
				}
			}

			BaseVertex += NumSlices;
		}
	}

	{
		const size_t SectionIndex = Sections.size() - 1;
		lcMatrix33 Transform(Sections[SectionIndex]);
		lcVector3 Offset = lcMul31(lcVector3(8.0f, 0.0f, 0.0f), Sections[SectionIndex]);

		sprintf(Line, "1 16 %f %f %f %f %f %f %f %f %f %f %f %f 572A.dat\n", Offset[0], Offset[1], Offset[2], Transform[0][0], Transform[1][0], Transform[2][0],
				Transform[0][1], Transform[1][1], Transform[2][1], Transform[0][2], Transform[1][2], Transform[2][2]);

		File.WriteBuffer(Line, strlen(Line));
	}
}

void lcSynthInfoShockAbsorber::AddParts(lcMemFile& File, lcLibraryMeshData&, const std::vector<lcMatrix44>& Sections) const
{
	char Line[256];
	lcVector3 Offset;

	Offset = Sections[0].GetTranslation();
	sprintf(Line, "1 0 %f %f %f 1 0 0 0 1 0 0 0 1 4254.dat\n", Offset[0], Offset[1], Offset[2]);
	File.WriteBuffer(Line, strlen(Line));

	Offset = Sections[1].GetTranslation();
	sprintf(Line, "1 16 %f %f %f 1 0 0 0 1 0 0 0 1 4255.dat\n", Offset[0], Offset[1], Offset[2]);
	File.WriteBuffer(Line, strlen(Line));

	float Distance = Sections[0].GetTranslation().y - Sections[1].GetTranslation().y;
	float Scale = (Distance - 66.0f) / 44.0f;

	Offset = Sections[0].GetTranslation();
	sprintf(Line, "1 494 %f %f %f 1 0 0 0 %f 0 0 0 1 %s\n", Offset[0], Offset[1] - 10 - 44.0f * Scale, Offset[2], Scale, mSpringPart);
	File.WriteBuffer(Line, strlen(Line));
}

void lcSynthInfoActuator::AddParts(lcMemFile& File, lcLibraryMeshData&, const std::vector<lcMatrix44>& Sections) const
{
	char Line[256];
	lcVector3 Offset;

	Offset = Sections[0].GetTranslation();
	sprintf(Line, "1 16 %f %f %f 1 0 0 0 1 0 0 0 1 %s\n", Offset[0], Offset[1], Offset[2], mBodyPart);
	File.WriteBuffer(Line, strlen(Line));

	Offset = lcMul(Sections[0], lcMatrix44Translation(lcVector3(0.0f, 0.0f, mAxleOffset))).GetTranslation();
	sprintf(Line, "1 25 %f %f %f 0 1 0 -1 0 0 0 0 1 %s\n", Offset[0], Offset[1], Offset[2], mAxlePart);
	File.WriteBuffer(Line, strlen(Line));

	Offset = Sections[1].GetTranslation();
	sprintf(Line, "1 72 %f %f %f 1 0 0 0 1 0 0 0 1 %s\n", Offset[0], Offset[1], Offset[2], mPistonPart);
	File.WriteBuffer(Line, strlen(Line));
}

void lcSynthInfoUniversalJoint::AddParts(lcMemFile& File, lcLibraryMeshData&, const std::vector<lcMatrix44>& Sections) const
{
	char Line[256];
	lcVector3 Offset = Sections[0].GetTranslation();

	float Angle = atan2f(Offset.x, Offset.z);
	lcMatrix44 Rotation = lcMatrix44RotationZ(Angle);
	lcMatrix44 Transform = lcMatrix44LeoCADToLDraw(Rotation);

	sprintf(Line, "1 16 0 0 0 %f %f %f %f %f %f %f %f %f %s\n", Transform[0][0], Transform[1][0], Transform[2][0],
			Transform[0][1], Transform[1][1], Transform[2][1], Transform[0][2], Transform[1][2], Transform[2][2], mCenterPart);
	File.WriteBuffer(Line, strlen(Line));

	Angle = atan2f(Offset.y, hypotf(Offset.x, Offset.z));
	Rotation = lcMul(Rotation, lcMatrix44RotationX(Angle));
	Transform = lcMul(
			lcMatrix44Translation(lcVector3(0.0f, 0.0f, mEndOffset)),
			lcMatrix44LeoCADToLDraw(Rotation)
			);

	sprintf(Line, "1 16 %f %f %f %f %f %f %f %f %f %f %f %f %s\n", Transform[3][0], Transform[3][1], Transform[3][2],
			Transform[0][0], Transform[1][0], Transform[2][0],
			Transform[0][1], Transform[1][1], Transform[2][1], Transform[0][2], Transform[1][2], Transform[2][2], mEndPart);
	File.WriteBuffer(Line, strlen(Line));

	Rotation = lcMatrix44FromEulerAngles(lcVector3(0.0f, LC_PI/2, LC_PI));
	Transform = lcMatrix44LeoCADToLDraw(lcMul(lcMatrix44Translation(lcVector3(0.0f, mEndOffset, 0.0f)), Rotation));
	sprintf(Line, "1 16 %f %f %f %f %f %f %f %f %f %f %f %f %s\n", Transform[3][0], Transform[3][1], Transform[3][2],
			Transform[0][0], Transform[1][0], Transform[2][0],
			Transform[0][1], Transform[1][1], -Transform[2][1], Transform[0][2], Transform[1][2], Transform[2][2], mEndPart);
	File.WriteBuffer(Line, strlen(Line));
}

lcMesh* lcSynthInfo::CreateMesh(const std::vector<lcPieceControlPoint>& ControlPoints) const
{
	std::vector<lcMatrix44> Sections;

	CalculateSections(ControlPoints, Sections, nullptr);

	lcLibraryMeshData MeshData;
	lcMemFile File; // todo: rewrite this to pass the parts directly

	AddParts(File, MeshData, Sections);

	File.WriteU8(0);
	File.Seek(0, SEEK_SET);

	lcMeshLoader MeshLoader(MeshData, false, nullptr, false);
	if (MeshLoader.LoadMesh(File, LC_MESHDATA_SHARED))
		return MeshData.CreateMesh();

	return nullptr;
}

int lcSynthInfo::InsertControlPoint(std::vector<lcPieceControlPoint>& ControlPoints, const lcVector3& Start, const lcVector3& End) const
{
	std::vector<lcMatrix44> Sections;

	quint32 BestSegment = UINT32_MAX;
	float BestTime;
	float BestDistance = FLT_MAX;
	lcVector3 BestPosition;

	CalculateSections(ControlPoints, Sections,
		[&](const lcVector3& CurvePoint, int SegmentIndex, float t)
		{
			float Distance = lcRayPointDistance(lcVector3LDrawToLeoCAD(CurvePoint), Start, End);
			if (Distance < BestDistance)
			{
				BestSegment = SegmentIndex;
				BestTime = t;
				BestDistance = Distance;
				BestPosition = lcVector3LDrawToLeoCAD(CurvePoint);
			}
		}
	);

	if (BestSegment != UINT32_MAX)
	{
		lcPieceControlPoint ControlPoint = ControlPoints[BestSegment];
		ControlPoint.Transform.SetTranslation(BestPosition);

		if (BestSegment != ControlPoints.size() - 1)
		{
			lcPieceControlPoint NextControlPoint = ControlPoints[BestSegment + 1];
			ControlPoint.Scale = ControlPoint.Scale * (1.0f - BestTime) + NextControlPoint.Scale * BestTime;
		}

		ControlPoints.insert(ControlPoints.begin() + BestSegment + 1, ControlPoint);
	}

	return BestSegment + 1;
}
