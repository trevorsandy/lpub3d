#include "LDPovExporter.h"
#include <TCFoundation/mystring.h>
#include <TCFoundation/TCLocalStrings.h>
#include <TCFoundation/TCMacros.h>
#include <TCFoundation/TCUserDefaults.h>
#include <LDLoader/LDLMainModel.h>
#include <LDLoader/LDLModelLine.h>
#include <LDLoader/LDLTriangleLine.h>
#include <LDLoader/LDLQuadLine.h>
#include <LDLoader/LDLLineLine.h>
#include <LDLoader/LDLCommentLine.h>
#include <LDLoader/LDLPalette.h>
#include <time.h>
#include <sys/stat.h>
#include <tinyxml.h>
#include <TRE/TREGL.h>
#include <TRE/TREShapeGroup.h>
#include <assert.h>
#include <stdarg.h>

#include <lpub_preferences.h>
#include <QDebug>

#define SMOOTH_THRESHOLD 0.906307787f
#define SMOOTH_EPSILON 0.001f

#if defined WIN32 && defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif

CharStringMap LDPovExporter::sm_replacementChars;

LDPovExporter::Shape::Shape(
	const TCVector *pts,
	size_t count,
	const TCFloat *matrix)
{
	points.reserve(count);
	if (matrix == NULL)
	{
		for (size_t i = 0; i < count; i++)
		{
			points.push_back(pts[i]);
		}
	}
	else
	{
		for (size_t i = 0; i < count; i++)
		{
			points.push_back(pts[i].transformPoint(matrix));
		}
	}
	for (size_t i = 0; i < points.size(); i++)
	{
		for (size_t j = i + 1; j < points.size(); j++)
		{
			if ((points[i] - points[j]).length() < SMOOTH_EPSILON)
			{
				throw "Shape points too close together.";
			}
		}
	}
}

LDPovExporter::Shape::Shape(const TCVector &p1, const TCVector &p2)
{
	points.reserve(2);
	points.push_back(p1);
	points.push_back(p2);
}

LDPovExporter::Shape::Shape(
	const TCVector &p1,
	const TCVector &p2,
	const TCVector &p3)
{
	points.reserve(3);
	points.push_back(p1);
	points.push_back(p2);
	points.push_back(p3);
}

LDPovExporter::Shape::Shape(
	const TCVector &p1,
	const TCVector &p2,
	const TCVector &p3,
	const TCVector &p4)
{
	points.reserve(4);
	points.push_back(p1);
	points.push_back(p2);
	points.push_back(p3);
	points.push_back(p4);
}

LDPovExporter::LineKey::LineKey(void)
{
}

//The parametric equations for a line passing through (x1 y1 z1), (x2 y2
//z2) are
//        x = x1 + (x2 - x1)*t
//        y = y1 + (y2 - y1)*t
//        z = z1 + (z2 - z1)*t
LDPovExporter::LineKey::LineKey(const TCVector &point1, const TCVector &point2)
{
	if (point1 < point2)
	{
		direction = point2 - point1;
	}
	else
	{
		direction = point1 - point2;
	}
	direction.normalize();
	if (fabs(direction[2]) > TCVector::getEpsilon())
	{
		// 0 = z1 + (z2 - z1) * t;
		// -z1 = (z2 - z1) * t;
		// -z1 / (z2 - z1) = t;
		TCFloat t = -point1[2] / (point2[2] - point1[2]);
		intercept[0] = point1[0] + (point2[0] - point1[0]) * t;
		intercept[1] = point1[1] + (point2[1] - point1[1]) * t;
	}
	else if (fabs(direction[1]) > TCVector::getEpsilon())
	{
		TCFloat t = -point1[1] / (point2[1] - point1[1]);
		intercept[0] = point1[0] + (point2[0] - point1[0]) * t;
		intercept[2] = point1[2] + (point2[2] - point1[2]) * t;
	}
	else if (fabs(direction[0]) > TCVector::getEpsilon())
	{
		TCFloat t = -point1[0] / (point2[0] - point1[0]);
		intercept[1] = point1[1] + (point2[1] - point1[1]) * t;
		intercept[2] = point1[2] + (point2[2] - point1[2]) * t;
	}
	else
	{
		debugPrintf("point1: %s\npoint2: %s\n",
			     point1.string(12).c_str(),
			     point2.string(12).c_str());

		throw "Line points cannot be equal.";
	}
}

LDPovExporter::LineKey::LineKey(const LineKey &other)
{
	*this = other;
}

LDPovExporter::LineKey &LDPovExporter::LineKey::operator=(const LineKey &other)
{
	direction = other.direction;
	intercept = other.intercept;
	return *this;
}

bool LDPovExporter::LineKey::operator==(const LineKey &other) const
{
	return direction == other.direction && intercept == other.intercept;
}

bool LDPovExporter::LineKey::operator<(const LineKey &other) const
{
	if (direction < other.direction)
	{
		return true;
	}
	else if (other.direction < direction)
	{
		return false;
	}
	else if (intercept < other.intercept)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void LDPovExporter::SmoothTriangle::setNormal(
	const TCVector &point,
	const TCVector &normal)
{
	TCVector &oldNormal = normals[point];
	if (LDPovExporter::shouldFlipNormal(normal, oldNormal))
	{
		oldNormal = -normal;
	}
	else
	{
		oldNormal = normal;
	}
}

bool LDPovExporter::SmoothTriangle::initLineKeys(
	const SizeTVectorMap &indexToVert)
{

	for (size_t i = 0; i < 3; i++)
	{

		size_t next = (i + 1) % 3;

		lineKeys[i] = LineKey(indexToVert.find(vertexIndices[i])->second,
			indexToVert.find(vertexIndices[next])->second);
		try
		{
			lineKeys[i] = LineKey(indexToVert.find(vertexIndices[i])->second,
				indexToVert.find(vertexIndices[next])->second);
		}
		catch (...)
		{
			debugPrintf("Invalid triangle.\n");
			return false;
		}
	}
	return true;
}

LDPovExporter::LDPovExporter(void):
LDExporter("PovExporter/"),
m_mesh2(true),
m_primSubCheck(false)
{
	char *ldrawDir = copyString(LDLModel::lDrawDir());

	if (ldrawDir)
	{
		StringList subDirectories;

		subDirectories.push_back("/pov/p/");
		subDirectories.push_back("/pov/parts/");
		replaceStringCharacter(ldrawDir, '\\', '/');
		stripTrailingPathSeparators(ldrawDir);
		m_ldrawDir = ldrawDir;
		delete[] ldrawDir;
		for (StringList::const_iterator it = subDirectories.begin();
			it != subDirectories.end(); ++it)
		{
			m_searchPath.push_back(m_ldrawDir + *it);
		}
	}
	loadSettings();
	if (sm_replacementChars.size() == 0)
	{
		sm_replacementChars['.'] = "_dot_";
		sm_replacementChars['-'] = "_dash_";
		sm_replacementChars['/'] = "_slash_";
		sm_replacementChars['\\'] = "_slash_";
		sm_replacementChars['#'] = "_hash_";
		sm_replacementChars[':'] = "_colon_";
		sm_replacementChars['!'] = "_bang_";
		sm_replacementChars['('] = "_openparen_";
		sm_replacementChars[')'] = "_closeparen_";
		sm_replacementChars['['] = "_openbracket_";
		sm_replacementChars[']'] = "_closebracket_";
		sm_replacementChars['{'] = "_openbrace_";
		sm_replacementChars['}'] = "_closebrace_";
		sm_replacementChars[' '] = "_space_";
		sm_replacementChars['\t'] = "_tab_";
		sm_replacementChars['&'] = "_ampersand_";
		sm_replacementChars['~'] = "_tilde_";
		sm_replacementChars['`'] = "_backtick_";
		sm_replacementChars['@'] = "_at_";
		sm_replacementChars['$'] = "_dollar_";
		sm_replacementChars['%'] = "_percent_";
		sm_replacementChars['^'] = "_caret_";
		sm_replacementChars['*'] = "_star_";
		sm_replacementChars['+'] = "_plus_";
		sm_replacementChars['='] = "_equals_";
		sm_replacementChars['\''] = "_tick_";
		sm_replacementChars['\"'] = "_quote_";
		sm_replacementChars[';'] = "_semicolon_";
		sm_replacementChars['|'] = "_pipe_";
		sm_replacementChars['?'] = "_question_";
		sm_replacementChars['<'] = "_lessthan_";
		sm_replacementChars['>'] = "_greaterthan_";
		sm_replacementChars[','] = "_comma_";
	}
}

LDPovExporter::~LDPovExporter(void)
{
}

ucstring LDPovExporter::getTypeDescription(void) const
{
	return ls(_UC("PovTypeDescription"));
}

void LDPovExporter::loadSettings(void)
{
	char *temp;

	LDExporter::loadSettings();
	m_quality = longForKey("Quality", 2);
	m_refls = boolForKey("Reflections", true);
	m_shads = boolForKey("Shadows", true);
	m_findReplacements = boolForKey("FindReplacements", false);
	m_xmlMap = boolForKey("XmlMap", true);
	m_xmlMapPath = pathForKey("XmlMapPath");
	m_inlinePov = boolForKey("InlinePov", true);
	m_smoothCurves = boolForKey("SmoothCurves", true);
	m_hideStuds = boolForKey("HideStuds", false);
	m_unmirrorStuds = boolForKey("UnmirrorStuds", true);
	m_floor = boolForKey("Floor", true);
	m_floorAxis = longForKey("FloorAxis", 1);
	m_selectedAspectRatio = longForKey("SelectedAspectRatio", -1);
	m_customAspectRatio = floatForKey("CustomAspectRatio", 1.5f);
	m_edgeRadius = floatForKey("EdgeRadius", 0.15f);
	m_ambient = floatForKey("Ambient", 0.4f);
	m_diffuse = floatForKey("Diffuse", 0.4f);
	m_refl = floatForKey("Refl", 0.08f);
	m_phong = floatForKey("Phong", 0.5f);
	m_phongSize = floatForKey("PhongSize", 40.0f);
	m_transRefl = floatForKey("TransRefl", 0.2f);
	m_transFilter = floatForKey("TransFilter", 0.85f);
	m_transIoR = floatForKey("TransIoR", 1.25f);
	m_rubberRefl = floatForKey("RubberRefl", 0.0f);
	m_rubberPhong = floatForKey("RubberPhong", 0.1f);
	m_rubberPhongSize = floatForKey("RubberPhongSize", 10.0f);
	m_chromeRefl = floatForKey("ChromeRefl", 0.85f);
	m_chromeBril = floatForKey("ChromeBril", 5.0f);
	m_chromeSpec = floatForKey("ChromeSpecular", 0.8f);
	m_chromeRough = floatForKey("ChromeRoughness", 0.01f);
	m_fileVersion = floatForKey("FileVersion", 3.6f);
	temp = stringForKey("TopInclude");
	if (temp != NULL)
	{
		m_topInclude = temp;
		delete[] temp;
	}
	else
	{
		m_topInclude = "";
	}
	temp = stringForKey("BottomInclude");
	if (temp != NULL)
	{
		m_bottomInclude = temp;
		delete[] temp;
	}
	else
	{
		m_bottomInclude = "";
	}
	m_lights = Preferences::ldvLights;
}

LDExporterSetting *LDPovExporter::addEdgesSettings(
	LDExporterSetting *pGroup) const
{
	LDExporterSetting *pEdgesGroup = LDExporter::addEdgesSettings(pGroup);
	if (pEdgesGroup == NULL)
	{
		return NULL;
	}
	addSetting(pEdgesGroup, ls(_UC("PovEdgeRadius")), m_edgeRadius,
		udKey("EdgeRadius").c_str(), 0.001f, 1000.0f);
	return pEdgesGroup;
}

LDExporterSetting *LDPovExporter::addGeometrySettings(void) const
{
	LDExporterSetting *pGroup = LDExporter::addGeometrySettings();
	if (pGroup == NULL)
	{
		return NULL;
	}
	addSetting(pGroup, LDExporterSetting(ls(_UC("PovSmoothCurves")),
		m_smoothCurves, udKey("SmoothCurves").c_str()));
	addSetting(pGroup, LDExporterSetting(ls(_UC("PovHideStuds")), m_hideStuds,
		udKey("HideStuds").c_str()));
	return pGroup;
}

void LDPovExporter::initSettings(void) const
{
	// Uncomment the following lines to test a top-level boolean group setting.
	//addSetting(LDExporterSetting(_UC("Top Level Test Group"), m_shads,
	//	udKey("Shadows").c_str()));
	//m_settings.back().setGroupSize(2);
	//addSetting(LDExporterSetting(_UC("Top Level Test Group Item 1"), m_shads,
	//	udKey("Shadows").c_str()));
	//addSetting(LDExporterSetting(_UC("Top Level Test Group Item 2"), m_shads,
	//	udKey("Shadows").c_str()));
	// End of top-level boolean group test.
	LDExporterSetting *pGroup= addSettingGroup(ls(_UC("PovGeneral")));
	if (pGroup == NULL)
	{
		return;
	}
	addSetting(pGroup, LDExporterSetting(ls(_UC("PovFileVersion")),
		m_fileVersion, udKey("FileVersion").c_str()));
	if (addSetting(pGroup, LDExporterSetting(ls(_UC("PovQuality")),
		udKey("Quality").c_str())))
	{
		LDExporterSetting &setting = m_settings.back();

		setting.addOption(0, ls(_UC("Pov0Quality")));
		setting.addOption(1, ls(_UC("Pov1Quality")));
		setting.addOption(2, ls(_UC("Pov2Quality")));
		setting.addOption(3, ls(_UC("Pov3Quality")));
		try
		{
			setting.selectOption(m_quality);
		}
		catch (...)
		{
			setting.selectOption(2);
		}
	}
	if (addSetting(pGroup, LDExporterSetting(ls(_UC("PovAspectRatio")),
		udKey("SelectedAspectRatio").c_str())))
	{
		LDExporterSetting &setting = m_settings.back();

		setting.addOption(-1, _UC("Automatic"));
		setting.addOption(0, _UC("5:4"));
		setting.addOption(1, _UC("4:3"));
		setting.addOption(2, _UC("3:2"));
		setting.addOption(3, _UC("5:3"));
		setting.addOption(4, _UC("16:9"));
		setting.addOption(5, _UC("2.35:1"));
		setting.addOption(6, ls(_UC("PovCurAspectRatio")));
		setting.addOption(7, ls(_UC("PovCustom")));
		try
		{
			setting.selectOption(m_selectedAspectRatio + 1);
		}
		catch (...)
		{
			setting.selectOption(2);
		}
	}
	addSetting(pGroup, LDExporterSetting(ls(_UC("PovCustomAspectRatio")),
		m_customAspectRatio, udKey("CustomAspectRatio").c_str()));
	m_settings.back().setTooltip("PovCustomAspectRatioTT");
	addSetting(pGroup, LDExporterSetting(ls(_UC("PovFloor")), m_floor,
		udKey("Floor").c_str()));
	m_settings.back().setGroupSize(1);
	if (addSetting(LDExporterSetting(ls(_UC("PovFloorAxis")),
		udKey("FloorAxis").c_str())))
	{
		LDExporterSetting &setting = m_settings.back();

		setting.addOption(0, ls(_UC("PovFloorAxisX")));
		setting.addOption(1, ls(_UC("PovFloorAxisY")));
		setting.addOption(2, ls(_UC("PovFloorAxisZ")));
		try
		{
			setting.selectOption(m_floorAxis);
		}
		catch (...)
		{
			setting.selectOption(1);
		}
		setting.setTooltip("PovFloorAxisTT");
	}
	addSetting(pGroup, LDExporterSetting(ls(_UC("PovReflections")), m_refls,
		udKey("Reflections").c_str()));
	addSetting(pGroup, LDExporterSetting(ls(_UC("PovShadows")), m_shads,
		udKey("Shadows").c_str()));
	addSetting(pGroup, LDExporterSetting(ls(_UC("PovUnmirrorStuds")),
		m_unmirrorStuds, udKey("UnmirrorStuds").c_str()));
	m_settings.back().setTooltip("PovUnmirrorStudsTT");
	addSetting(pGroup, LDExporterSetting(ls(_UC("PovTopInclude")),
		m_topInclude.c_str(), udKey("TopInclude").c_str()));
	m_settings.back().setTooltip("PovTopIncludeTT");
	addSetting(pGroup, LDExporterSetting(ls(_UC("PovBottomInclude")),
		m_bottomInclude.c_str(), udKey("BottomInclude").c_str()));
	m_settings.back().setTooltip("PovBottomIncludeTT");
	LDExporter::initSettings();
	// Uncomment the below to test top-level number settings to verify
	// alignment.
	//addSetting(ls(_UC("PovQuality")), m_quality, udKey("Quality").c_str(), 0,
	//	3);
	// Uncomment the below to test a long setting.
	//addSetting(_UC("Long Setting"), 42l, udKey("TestLongSetting").c_str(), -10l,
	//	100l);
	pGroup = addSettingGroup(ls(_UC("PovNativeGeometry")));
	if (pGroup == NULL)
	{
		return;
	}
	addSetting(pGroup, LDExporterSetting(ls(_UC("PovInlinePov")), m_inlinePov,
		udKey("InlinePov").c_str()));
	m_settings.back().setTooltip("PovInlinePovTT");
	addSetting(pGroup, LDExporterSetting(ls(_UC("PovFindReplacements")),
		m_findReplacements, udKey("FindReplacements").c_str()));
	m_settings.back().setTooltip("PovFindReplacementsTT");
	if (addSetting(pGroup, LDExporterSetting(ls(_UC("PovXmlMap")), m_xmlMap,
		udKey("XmlMap").c_str())))
	{
		m_settings.back().setGroupSize(1);
		m_settings.back().setTooltip("PovXmlMapTT");
		if (addSetting(LDExporterSetting(ls(_UC("PovXmlMapPath")),
			m_xmlMapPath.c_str(), udKey("XmlMapPath").c_str())))
		{
			m_settings.back().setIsPath(true);
			m_settings.back().setTooltip("PovXmlMapPathTT");
		}
	}
	pGroup = addSettingGroup(ls(_UC("PovLighting")));
	if (pGroup == NULL)
	{
		return;
	}
	addSetting(pGroup, ls(_UC("PovAmbient")), m_ambient,
		udKey("Ambient").c_str(), 0.0f, 1.0f);
	addSetting(pGroup, ls(_UC("PovDiffuse")), m_diffuse,
		udKey("Diffuse").c_str(), 0.0f, 1.0f);
	pGroup = addSettingGroup(ls(_UC("PovMaterialProps")));
	if (pGroup == NULL)
	{
		return;
	}
	addSetting(pGroup, ls(_UC("PovRefl")), m_refl, udKey("Refl").c_str(), 0.0f,
		1.0f);
	addSetting(pGroup, ls(_UC("PovPhong")), m_phong, udKey("Phong").c_str(),
		0.0f, 10.0f);
	addSetting(pGroup, ls(_UC("PovPhongSize")), m_phongSize,
		udKey("PhongSize").c_str(), 0.0f, 10000.0f);
	pGroup = addSettingGroup(ls(_UC("PovTransMaterialProps")));
	if (pGroup == NULL)
	{
		return;
	}
	addSetting(pGroup, ls(_UC("PovRefl")), m_transRefl,
		udKey("TransRefl").c_str(), 0.0f, 1.0f);
	addSetting(pGroup, ls(_UC("PovFilter")), m_transFilter,
		udKey("TransFilter").c_str(), 0.0f, 1.0f);
	addSetting(pGroup, ls(_UC("PovIoR")), m_transIoR, udKey("TransIoR").c_str(),
		1.0f, 1000.0f);
	pGroup = addSettingGroup(ls(_UC("PovRubberMaterialProps")));
	if (pGroup == NULL)
	{
		return;
	}
	addSetting(pGroup, ls(_UC("PovRefl")), m_rubberRefl,
		udKey("RubberRefl").c_str(), 0.0f, 1.0f);
	addSetting(pGroup, ls(_UC("PovPhong")), m_rubberPhong,
		udKey("RubberPhong").c_str(), 0.0f, 10.0f);
	addSetting(pGroup, ls(_UC("PovPhongSize")), m_rubberPhongSize,
		udKey("RubberPhongSize").c_str(), 0.0f, 10000.0f);
	pGroup = addSettingGroup(ls(_UC("PovChromeMaterialProps")));
	if (pGroup == NULL)
	{
		return;
	}
	addSetting(pGroup, ls(_UC("PovRefl")), m_chromeRefl,
		udKey("ChromeRefl").c_str(), 0.0f, 1.0f);
	addSetting(pGroup, ls(_UC("PovBril")), m_chromeBril,
		udKey("ChromeBril").c_str(), 0.0f, 10000.0f);
	addSetting(pGroup, ls(_UC("PovSpec")), m_chromeSpec,
		udKey("ChromeSpecular").c_str(), 0.0f, 1.0f);
	addSetting(pGroup, ls(_UC("PovRough")), m_chromeRough,
		udKey("ChromeRoughness").c_str(), 0.0f, 1.0f);
}

void LDPovExporter::dealloc(void)
{
	LDExporter::dealloc();
}

void LDPovExporter::loadXmlMatrices(TiXmlElement *matrices)
{
	TiXmlElement *element;

	for (element = matrices->FirstChildElement(); element != NULL;
		element = element->NextSiblingElement())
	{
		m_xmlMatrices[element->Value()] = element->GetText();
	}
}

void LDPovExporter::loadPovCodes(
	TiXmlElement *element,
	PovMapping &mapping)
{
	TiXmlElement *child;

	for (child = element->FirstChildElement("POVCode"); child != NULL;
		child = child->NextSiblingElement("POVCode"))
	{
		std::string code = child->GetText();

		mapping.povCodes.push_back(code);
	}
}

void LDPovExporter::loadPovFilenames(
	TiXmlElement *element,
	PovMapping &mapping,
	const std::string &povVersion /*= std::string()*/)
{
	TiXmlElement *child;

	for (child = element->FirstChildElement("POVFilename"); child != NULL;
		child = child->NextSiblingElement("POVFilename"))
	{
		std::string filename = child->GetText();

		mapping.povFilenames.push_back(filename);
		if (povVersion.size() > 0)
		{
			m_includeVersions[filename] = povVersion;
		}
	}
}

void LDPovExporter::loadPovDependency(
	TiXmlElement *element,
	PovMapping &mapping)
{
	std::string name = element->GetText();

	if (name.size() > 0)
	{
		TiXmlElement *dependencyElement =
			m_dependenciesElement->FirstChildElement(name);

		if (dependencyElement != NULL)
		{
			std::string povVersion;

			loadPovDependencies(dependencyElement, mapping);
			TiXmlElement *child = element->FirstChildElement("POVVersion");
			if (child != NULL)
			{
				povVersion = child->GetText();
			}
			loadPovFilenames(dependencyElement, mapping, povVersion);
			loadPovCodes(dependencyElement, mapping);
		}
	}
}

void LDPovExporter::loadPovDependencies(
	TiXmlElement *element,
	PovMapping &mapping)
{
	if (m_dependenciesElement != NULL)
	{
		TiXmlElement *child;

		for (child = element->FirstChildElement("Dependency"); child != NULL;
			child = child->NextSiblingElement("Dependency"))
		{
			loadPovDependency(child, mapping);
		}
	}
}

std::string LDPovExporter::loadPovMapping(
	TiXmlElement *element,
	const char *ldrawElementName,
	PovMapping &mapping)
{
	TiXmlElement *child = element->FirstChildElement("POVName");
	std::string ldrawValue;

	if (child == NULL)
	{
		return "";
	}
	for (; child != NULL; child = child->NextSiblingElement("POVName"))
	{
		PovName name;
		TiXmlAttribute *attr;

		name.name = child->GetText();
		for (attr = child->FirstAttribute(); attr != NULL; attr = attr->Next())
		{
			name.attributes[attr->Name()] = attr->Value();
		}
		mapping.names.push_back(name);
	}
	child = element->FirstChildElement("IoR");
	if (child != NULL)
	{
		mapping.ior = child->GetText();
	}
	child = element->FirstChildElement(ldrawElementName);
	if (child == NULL)
	{
		return "";
	}
	ldrawValue = child->GetText();
	loadPovDependencies(element, mapping);
	loadPovFilenames(element, mapping);
	return ldrawValue;
}

void LDPovExporter::loadXmlColors(TiXmlElement *colors)
{
	TiXmlElement *element;

	for (element = colors->FirstChildElement(); element != NULL;
		element = element->NextSiblingElement())
	{
		PovMapping colorMapping;
		std::string ldrawValue = loadPovMapping(element, "LDrawNumber",
			colorMapping);

		if (ldrawValue.size() > 0)
		{
			m_xmlColors[(TCULong)atoi(ldrawValue.c_str())] = colorMapping;
		}
	}
}

void LDPovExporter::loadXmlElements(TiXmlElement *elements)
{
	TiXmlElement *element;

	for (element = elements->FirstChildElement(); element != NULL;
		element = element->NextSiblingElement())
	{
		PovElement povElement;
		std::string ldrawFilename = loadPovMapping(element, "LDrawFilename",
			povElement);

		if (ldrawFilename.size() > 0)
		{
			TiXmlElement *child = element->FirstChildElement("MatrixRef");
			std::string matrixString;
			TCFloat *m;

			if (child)
			{
				matrixString = m_xmlMatrices[child->GetText()];
			}
			if (matrixString.size() == 0)
			{
				child = element->FirstChildElement("Matrix");

				if (child)
				{
					matrixString = child->GetText();
				}
			}
			m = povElement.matrix;
			if (sscanf(matrixString.c_str(),
				"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,", &m[0],
				&m[1], &m[2], &m[3], &m[4], &m[5], &m[6], &m[7], &m[8], &m[9],
				&m[10], &m[11], &m[12], &m[13], &m[14], &m[15]) != 16)
			{
				TCVector::initIdentityMatrix(m);
			}
			m_xmlElements[ldrawFilename] = povElement;
		}

	}
}

void LDPovExporter::loadLDrawPovXml(void)
{
	std::string filename;

	if (m_xmlMapPath.size() > 0)
	{
		filename = m_xmlMapPath;
	}
	else
	{
		filename = TCUserDefaults::getAppPath();
#ifdef COCOA
		filename += "../Resources/";
#endif // COCOA
		filename += "LGEO.xml";
	}
	TiXmlDocument doc(filename);

	if (doc.LoadFile())
	{
		TiXmlHandle hDoc(&doc);
		TiXmlElement *root =
			hDoc.FirstChildElement("LDrawPOV").Element();
		TiXmlElement *element;

		if (root == NULL)
		{
			return;
		}
		m_dependenciesElement = root->FirstChildElement("Dependencies");
		element = root->FirstChildElement("Colors");
		if (element != NULL)
		{
			loadXmlColors(element);
		}
		element = root->FirstChildElement("Matrices");
		if (element != NULL)
		{
			loadXmlMatrices(element);
		}
		element = root->FirstChildElement("Elements");
		if (element != NULL)
		{
			loadXmlElements(element);
		}
	}
}

int LDPovExporter::doExport(LDLModel *pTopModel)
{
	std::string filename = m_filename;

	loadSettings();
	m_pTopModel = pTopModel;
	if (filename.size() == 0)
	{
		filename = pTopModel->getFilename();
		char *dotSpot = strrchr(&filename[0], '.');

		if (dotSpot)
		{
			filename.resize(dotSpot - &filename[0]);
		}
		filename += ".pov";
	}
	if ((m_pPovFile = ucfopen(filename.c_str(), "w")) != NULL)
	{
		if (m_xmlMap)
		{
			loadLDrawPovXml();
		}
		if (!writeHeader())
		{
			return 1;
		}
		if (m_topInclude.size() > 0)
		{
			fprintf(m_pPovFile, "#include \"%s\"\n\n", m_topInclude.c_str());
		}
		if (!writeCamera())
		{
			return 1;
		}
		if (!writeLights())
		{
			return 1;
		}
		writeSeamMacro();
		fprintf(m_pPovFile, "\nbackground { color rgb <LDXBgR,LDXBgG,LDXBgB> }\n\n");
		if (m_edges)
		{
			TCFloat matrix[16];

			TCVector::initIdentityMatrix(matrix);
			m_pTopModel->scanPoints(this,
				(LDLScanPointCallback)&LDPovExporter::scanEdgePoint, matrix);
			writeEdgeLineMacro();
			writeEdgeColor();
		}
		m_colorsUsed[7] = true;
		if (!scanModelColors(m_pTopModel, false))
		{
			return 1;
		}
		if (!writeModelColors())
		{
			return 1;
		}
		if (!writeEdges())
		{
			return 1;
		}
		if (!writeModel(m_pTopModel, TCVector::getIdentityMatrix(), false))
		{
			return 1;
		}
		writeMainModel();
		writeFloor();
		if (m_bottomInclude.size() > 0)
		{
			fprintf(m_pPovFile, "#include \"%s\"\n\n", m_bottomInclude.c_str());
		}
		fclose(m_pPovFile);
	}
	else
	{
		consolePrintf(_UC("%s"), (const char *)ls(_UC("PovErrorCreatingPov")));
	}
	return 0;
}

void LDPovExporter::writeMainModel(void)
{
	fprintf(m_pPovFile, "// ");
	if (m_pTopModel->getName())
	{
		fprintf(m_pPovFile, "%s\n", m_pTopModel->getName());
	}
	else
	{
		char *name = filenameFromPath(m_pTopModel->getFilename());

		fprintf(m_pPovFile, "%s\n", name);
		delete[] name;
	}
	fprintf(m_pPovFile, "object {\n\t%s\n",
		getDeclareName(m_pTopModel, false).c_str());
	writeColor(7);
	fprintf(m_pPovFile, "\n}\n\n");
}

void LDPovExporter::writeFloor(void)
{
	fprintf(m_pPovFile, "// Floor\n");
	fprintf(m_pPovFile, "#if (LDXFloor != 0)\n");
	fprintf(m_pPovFile, "object {\n");
	fprintf(m_pPovFile, "\tplane { LDXFloorAxis, LDXFloorLoc hollow }\n");
	fprintf(m_pPovFile, "\ttexture {\n");
	fprintf(m_pPovFile,
		"\t\tpigment { color rgb <LDXFloorR,LDXFloorG,LDXFloorB> }\n");
	fprintf(m_pPovFile,
		"\t\tfinish { ambient LDXFloorAmb diffuse LDXFloorDif }\n");
	fprintf(m_pPovFile, "\t}\n");
	fprintf(m_pPovFile, "}\n");
	fprintf(m_pPovFile, "#end\n\n");
}

std::string LDPovExporter::getAspectRatio(void)
{
	switch (m_selectedAspectRatio)
	{
	case -1:
		return "image_width/image_height";
	case 0:
		m_width = 5;
		m_height = 4;
		return "5/4";
	case 2:
		m_width = 3;
		m_height = 2;
		return "3/2";
	case 3:
		m_width = 5;
		m_height = 3;
		return "5/3";
	case 4:
		m_width = 16;
		m_height = 9;
		return "16/9";
	case 5:
		m_width = 235;
		m_height = 100;
		return "2.35";
	case 6:
		{
			std::string aspect = ftostr(m_width);

			aspect += "/";
			aspect += ftostr(m_height);
			return aspect;
		}
	case 7:
		m_width = m_customAspectRatio;
		m_height = 1.0f;
		return ftostr(m_width / m_height);
	default:
		m_width = 4;
		m_height = 3;
		return "4/3";
	}
}

void LDPovExporter::writeDeclare(
	const char *name,
	const std::string &value,
	const char *commentName /*= NULL*/)
{
	if (commentName != NULL)
	{
		fprintf(m_pPovFile, "#declare %s = %s;\t// %s\n", name, value.c_str(),
			(const char *)ls(commentName));
	}
	else
	{
		fprintf(m_pPovFile, "#declare %s = %s;\n",  name, value.c_str());
	}
}

void LDPovExporter::writeDeclare(
	const char *name,
	const char *value,
	const char *commentName /*= NULL*/)
{
	writeDeclare(name, std::string(value), commentName);
}

void LDPovExporter::writeDeclare(
	const char *name,
	double value,
	const char *commentName /*= NULL*/)
{
	writeDeclare(name, ftostr(value), commentName);
}

void LDPovExporter::writeDeclare(
	const char *name,
	float value,
	const char *commentName /*= NULL*/)
{
	writeDeclare(name, ftostr(value), commentName);
}

void LDPovExporter::writeDeclare(
	const char *name,
	long value,
	const char *commentName /*= NULL*/)
{
	writeDeclare(name, ltostr(value), commentName);
}

void LDPovExporter::writeDeclare(
	const char *name,
	bool value,
	const char *commentName /*= NULL*/)
{
	writeDeclare(name, value ? 1l : 0l, commentName);
}

bool LDPovExporter::writeHeader(void)
{
	time_t genTime = time(NULL);
	const char *author = m_pTopModel->getAuthor();
	char *filename = filenameFromPath(m_pTopModel->getFilename());
	std::string floorAxis;
	std::string floorLoc;
	std::string cameraLocString;
	std::string cameraLookAtString;
	std::string cameraSkyString;

	fprintf(m_pPovFile, "// %s %s%s%s %s\n", (const char *)ls("PovGeneratedBy"),
		m_appName.c_str(), m_appVersion.size() > 0 ? " " : "",
		m_appVersion.c_str(), m_appCopyright.c_str());
	fprintf(m_pPovFile, "// %s %s\n", (const char *)ls("PovSee"),
		m_appUrl.c_str());
	fprintf(m_pPovFile, "// %s %s", (const char *)ls("PovDate"),
		ctime(&genTime));
	if (filename != NULL)
	{
		fprintf(m_pPovFile, "// %s %s\n", (const char *)ls("PovLDrawFile"),
			filename);
		delete[] filename;
	}
	if (author != NULL)
	{
		fprintf(m_pPovFile, "// %s %s\n", (const char *)ls("PovLDrawAuthor"),
			author);
	}
	fprintf(m_pPovFile, ls("PovNote"), m_appName.c_str());
	fprintf(m_pPovFile, "#version %g;\n\n", m_fileVersion);
	writeDeclare("LDXQual", m_quality, "PovQualDesc");
	writeDeclare("LDXSW", m_seamWidth, "PovSeamWidthDesc");
	writeDeclare("LDXStuds", !m_hideStuds, "PovStudsDesc");
	writeDeclare("LDXRefls", m_refls, "PovReflsDesc");
	writeDeclare("LDXShads", m_shads, "PovShadsDesc");
	writeDeclare("LDXFloor", m_floor, "PovFloorDesc");
	if (m_edges)
	{
		writeDeclare("LDXSkipEdges", false, "PovSkipEdgesDesc");
	}
	fprintf(m_pPovFile, "\n");

	fprintf(m_pPovFile, "%s\n", (const char *)ls("PovBoundsSection"));
	writeDeclare("LDXMinX", m_boundingMin[0]);
	writeDeclare("LDXMinY", m_boundingMin[1]);
	writeDeclare("LDXMinZ", m_boundingMin[2]);
	writeDeclare("LDXMaxX", m_boundingMax[0]);
	writeDeclare("LDXMaxY", m_boundingMax[1]);
	writeDeclare("LDXMaxZ", m_boundingMax[2]);
	writeDeclare("LDXCenterX", m_center[0]);
	writeDeclare("LDXCenterY", m_center[1]);
	writeDeclare("LDXCenterZ", m_center[2]);
	writeDeclare("LDXCenter", "<LDXCenterX,LDXCenterY,LDXCenterZ>");
	writeDeclare("LDXRadius", m_radius);
	fprintf(m_pPovFile, "\n");

	fprintf(m_pPovFile, "%s\n", (const char *)ls("PovCameraSection"));
	getCameraStrings(cameraLocString, cameraLookAtString, cameraSkyString);
	writeDeclare("LDXCameraLoc", cameraLocString, "PovCameraLocDesc");
	writeDeclare("LDXCameraLookAt", cameraLookAtString, "PovCameraLookAtDesc");
	writeDeclare("LDXCameraSky", cameraSkyString, "PovCameraSkyDesc");
	fprintf(m_pPovFile, "\n");

	switch (m_floorAxis)
	{
	case 0:
		floorAxis = "x";
		floorLoc = "LDXMinX";
		break;
	case 2:
		floorAxis = "z";
		floorLoc = "LDXMaxZ";
		break;
	default:
		floorAxis = "y";
		floorLoc = "LDXMaxY";
		break;
	}
	writeDeclare("LDXFloorLoc", floorLoc, "PovFloorLocDesc");
	writeDeclare("LDXFloorAxis", floorAxis, "PovFloorAxisDesc");
	writeDeclare("LDXFloorR", 0.8, "PovFloorRDesc");
	writeDeclare("LDXFloorG", 0.8, "PovFloorGDesc");
	writeDeclare("LDXFloorB", 0.8, "PovFloorBDesc");
	writeDeclare("LDXFloorAmb", 0.4, "PovFloorAmbDesc");
	writeDeclare("LDXFloorDif", 0.4, "PovFloorDifDesc");
	writeDeclare("LDXAmb", m_ambient);
	writeDeclare("LDXDif", m_diffuse);
	writeDeclare("LDXRefl", m_refl);
	writeDeclare("LDXPhong", m_phong);
	writeDeclare("LDXPhongS", m_phongSize);
	writeDeclare("LDXTRefl", m_transRefl);
	writeDeclare("LDXTFilt", m_transFilter);
	writeDeclare("LDXIoR", m_transIoR);
	writeDeclare("LDXRubberRefl", m_rubberRefl);
	writeDeclare("LDXRubberPhong", m_rubberPhong);
	writeDeclare("LDXRubberPhongS", m_rubberPhongSize);
	writeDeclare("LDXChromeRefl", m_chromeRefl);
	writeDeclare("LDXChromeBril", m_chromeBril);
	writeDeclare("LDXChromeSpec", m_chromeSpec);
	writeDeclare("LDXChromeRough", m_chromeRough);
	writeDeclare("LDXIPov", m_inlinePov, "PovInlinePovDesc");
	if (m_edges)
	{
		writeDeclare("LDXEdgeRad", m_edgeRadius);
		writeDeclare("LDXEdgeR", "0.0");
		writeDeclare("LDXEdgeG", "0.0");
		writeDeclare("LDXEdgeB", "0.0");
	}
	writeDeclare("LDXBgR", m_backgroundR, "PovBgRDesc");
	writeDeclare("LDXBgG", m_backgroundG, "PovBgGDesc");
	writeDeclare("LDXBgB", m_backgroundB, "PovBgBDesc");
	if (m_xmlMap)
	{
		writeDeclare("LDXOrigVer", "version", "OrigVerDesc");
	}
	fprintf(m_pPovFile, "\n");
	return true;
}

std::string LDPovExporter::getModelFilename(const LDLModel *pModel)
{
	std::string buf;
	const char *modelFilename = pModel->getName();

	if (!modelFilename && pModel == m_pTopModel)
	{
		char *temp = filenameFromPath(m_pTopModel->getFilename());
		buf = temp;
		delete[] temp;
	}
	else
	{
		buf = modelFilename;
	}
	return buf;
}

std::string LDPovExporter::getDeclareName(
	LDLModel *pModel,
	bool mirrored,
	bool inPart /*= false*/)
{
	return getDeclareName(getModelFilename(pModel), mirrored,
		inPart, pModel->isPart());
}

std::string LDPovExporter::replaceSpecialChacters(const char *string)
{
	size_t newLen = 0;
	size_t i;
	std::string retVal;

	for (i = 0; string[i]; i++)
	{
		CharStringMap::const_iterator it = sm_replacementChars.find(string[i]);

		if (it != sm_replacementChars.end())
		{
			newLen += it->second.size();
		}
		else
		{
			newLen++;
		}
	}
	retVal.reserve(newLen);
	for (i = 0; string[i]; i++)
	{
		CharStringMap::const_iterator it = sm_replacementChars.find(string[i]);

		if (it != sm_replacementChars.end())
		{
			retVal += it->second;
		}
		else
		{
			retVal += string[i];
		}
	}
	return retVal;
}

std::string LDPovExporter::getDeclareName(
	const std::string &modelFilename,
	bool mirrored,
	bool inPart /*= false*/,
	bool isPart)
{
	StringStringMap::const_iterator it;
	std::string key;
	bool isXml = m_xmlElements.find(lowerCaseString(modelFilename)) !=
		m_xmlElements.end();

	if (mirrored && !isXml)
	{
		key = modelFilename + ":mirror";
	}
	else
	{
		key = modelFilename;
	}
	if (m_smoothCurves && !isXml)
	{
		if (inPart)
		{
			if (isPart)
			{
				key += ":sub_part";
			}
			else
			{
				key += ":in_part";
			}
		}
	}
	it = m_declareNames.find(lowerCaseString(key));
	if (it != m_declareNames.end())
	{
		return it->second;
	}
	std::string replaced = std::string("LDX_") +
		replaceSpecialChacters(modelFilename.c_str());
	//if (replaced.size() == modelFilename.size())
	//{
	//	// No replacements, so it could be a POV reserved word.  Append _ldx to
	//	// the end to guarantee that isn't the case.
	//	replaced += "_ldx";
	//}
	std::string retValue;

	//convertStringToLower(&replaced[0]);
	if (isdigit(replaced[0]))
	{
		retValue = "_";
		retValue += replaced;
	}
	else
	{
		retValue = replaced;
	}
	if (mirrored && !isXml)
	{
		retValue += "_mirror";
	}
	if (m_smoothCurves && !isXml)
	{
		if (inPart)
		{
			if (isPart)
			{
				retValue += "_sub_part";
			}
			else
			{
				retValue += "_in_part";
			}
		}
	}
	m_declareNames[lowerCaseString(key)] = retValue;
	return retValue;
}

bool LDPovExporter::scanModelColors(LDLModel *pModel, bool inPart)
{
	LDLFileLineArray *fileLines = pModel->getFileLines();
	std::string declareName = getDeclareName(pModel, false, inPart);

	if (m_processedModels[declareName])
	{
		return true;
	}
	m_processedModels[declareName] = true;
	if (fileLines)
	{
		int i;
		int count = pModel->getActiveLineCount();

		for (i = 0; i < count; i++)
		{
			LDLFileLine *pFileLine = (*fileLines)[i];

			if (!pFileLine->isValid())
			{
				continue;
			}
			if (pFileLine->getLineType() == LDLLineTypeModel)
			{
				LDLModelLine *pModelLine = (LDLModelLine *)pFileLine;
				LDLModel *pSubModel = pModelLine->getModel(true);

				if (pSubModel)
				{
					m_colorsUsed[pModelLine->getColorNumber()] = true;
					scanModelColors(pSubModel, pSubModel->isPart() || inPart);
				}
			}
			else if (pFileLine->isShapeLine())
			{
				LDLShapeLine *pShapeLine = (LDLShapeLine *)pFileLine;

				if (pShapeLine->getLineType() != LDLLineTypeLine &&
					pShapeLine->getLineType() != LDLLineTypeConditionalLine)
				{
					m_colorsUsed[pShapeLine->getColorNumber()] = true;
				}
			}
		}
	}
	return true;
}

bool LDPovExporter::writeModelColors(void)
{
	for (IntBoolMap::const_iterator it = m_colorsUsed.begin();
		it != m_colorsUsed.end(); ++it)
	{
		writeColorDeclaration(it->first);
	}
	m_processedModels.clear();
	return 1;
}

bool LDPovExporter::shouldDrawConditional(
	const TCVector &p1,
	const TCVector &p2,
	const TCVector &p3,
	const TCVector &p4,
	const TCFloat *matrix)
{
	// Use matrix--which contains a combination of the projection and the
	// model-view matrix--to calculate coords in the plane of the screen, so
	// we can test optional lines.
	TCFloat s1x, s1y;
	TCFloat s2x, s2y;
	TCFloat s3x, s3y;
	TCFloat s4x, s4y;

	// Only draw optional line p1-p2 if p3 and p4 are on the same side of p1-p2.
	// Note that we don't actually adjust for the window size, because it
	// doesn't effect the calculation.  Also, we don't care what the z value is,
	// so we don't bother to compute it.
	TREShapeGroup::transformPoint(p1, matrix, &s1x, &s1y);
	TREShapeGroup::transformPoint(p2, matrix, &s2x, &s2y);
	TREShapeGroup::transformPoint(p3, matrix, &s3x, &s3y);
	TREShapeGroup::transformPoint(p4, matrix, &s4x, &s4y);

	// If we do not turn the same direction \_/ for both test points
	// then they're on opposite sides of segment p1-p2 and we should
	// skip drawing this conditional line.
	if (TREShapeGroup::turnVector(s2x-s1x, s2y-s1y, s3x-s2x, s3y-s2y) ==
		TREShapeGroup::turnVector(s2x-s1x, s2y-s1y, s4x-s2x, s4y-s2y))
	{
		return true;	// Draw it
	}
	else
	{
		return false;	// Skip it.
	}
}

bool LDPovExporter::writeEdges(void)
{
	if (m_edgePoints.size() > 0)
	{
		fprintf(m_pPovFile,
			"#declare LDXEdges = union\n"
			"{\n");
		for (VectorList::const_iterator it = m_edgePoints.begin();
			it != m_edgePoints.end(); ++it)
		{
			const TCVector &point1 = *it;

			it++;
			if (it != m_edgePoints.end())
			{
				const TCVector &point2 = *it;

				fprintf(m_pPovFile, "	EdgeLine(");
				writePoint(point1);
				fprintf(m_pPovFile, ",");
				writePoint(point2);
				fprintf(m_pPovFile, ",EdgeColor)\n");
			}
		}
		fprintf(m_pPovFile, "}\n\n");
	}
	if (m_condEdgePoints.size() > 0)
	{
		TCFloat projectionMatrix[16];
		TCFloat modelViewMatrix[16];
		TCFloat matrix[16];

		treGlGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);
		treGlGetFloatv(GL_MODELVIEW_MATRIX, modelViewMatrix);
		TCVector::multMatrix(projectionMatrix, modelViewMatrix, matrix);
		fprintf(m_pPovFile,
			"#declare LDXConditionalEdges = union\n"
			"{\n");
		for (VectorList::const_iterator it = m_condEdgePoints.begin();
			it != m_condEdgePoints.end(); ++it)
		{
			const TCVector &point1 = *it;

			it++;
			if (it != m_condEdgePoints.end())
			{
				const TCVector &point2 = *it;

				it++;
				if (it != m_condEdgePoints.end())
				{
					const TCVector &controlPoint1 = *it;

					it++;
					if (it != m_condEdgePoints.end())
					{
						const TCVector &controlPoint2 = *it;

						if (shouldDrawConditional(point1, point2, controlPoint1,
							controlPoint2, matrix))
						{
							fprintf(m_pPovFile, "	EdgeLine(");
							writePoint(point1);
							fprintf(m_pPovFile, ",");
							writePoint(point2);
							fprintf(m_pPovFile, ",EdgeColor)\n");
						}
					}
				}
			}
		}
		fprintf(m_pPovFile, "}\n\n");
	}
	return true;
}

bool LDPovExporter::writeModel(
	LDLModel *pModel,
	const TCFloat *matrix,
	bool inPart)
{
	LDLFileLineArray *fileLines = pModel->getFileLines();
	bool mirrored = m_unmirrorStuds &&
		TCVector::determinant(matrix) < 0.0f && pModel->hasStuds();
	std::string declareName = getDeclareName(pModel, mirrored, inPart);

	if (m_processedModels[declareName] || m_emptyModels[declareName])
	{
		return true;
	}
	m_processedModels[declareName] = true;
	if (!findModelInclude(pModel))
	{
		if (fileLines)
		{
			int count = pModel->getActiveLineCount();

			for (int i = 0; i < count; i++)
			{
				LDLFileLine *pFileLine = (*fileLines)[i];

				if (!pFileLine->isValid())
				{
					continue;
				}
				if (pFileLine->getLineType() == LDLLineTypeModel)
				{
					LDLModelLine *pModelLine = (LDLModelLine *)pFileLine;
					LDLModel *pOtherModel = pModelLine->getModel(true);

					if (pOtherModel != NULL)
					{
						TCFloat newMatrix[16];

						TCVector::multMatrix(matrix, pModelLine->getMatrix(),
							newMatrix);
						writeModel(pOtherModel, newMatrix,
							inPart || pModel->isPart());
					}
				}
			}
			return writeModelObject(pModel, mirrored, matrix, inPart);
		}
	}
	return true;
}

// NOTE: static function
void LDPovExporter::cleanupFloats(TCFloat *array, int count /*= 16*/)
{
	int i;

	for (i = 0; i < count; i++)
	{
		if (fabs(array[i]) < 1e-6)
		{
			array[i] = 0.0f;
		}
	}
}

// NOTE: static function
void LDPovExporter::cleanupDoubles(double *array, int count /*= 16*/)
{
	int i;

	for (i = 0; i < count; i++)
	{
		if (fabs(array[i]) < 1e-9)
		{
			array[i] = 0.0f;
		}
	}
}

void LDPovExporter::getCameraStrings(
	std::string &locationString,
	std::string &lookAtString,
	std::string &skyString)
{
	TCFloat tmpMatrix[16];
	TCFloat matrix[16];
	TCFloat centerMatrix[16];
	TCFloat positionMatrix[16];
	TCFloat cameraMatrix[16];
	TCFloat otherMatrix[16] = {1,0,0,0,0,-1,0,0,0,0,-1,0,0,0,0,1};
	char tmpString[1024];
	TCVector directionVector = TCVector(0.0f, 0.0f, 1.0f);
	TCVector locationVector;
	TCVector lookAtVector;
	TCVector upVector = TCVector(0.0f, -1.0f, 0.0f);
	double direction[3];
	double up[3];
	double location[3];
	LDLFacing facing;
	double lookAt[3];
	double tempV[3];
	std::string aspectRatio = getAspectRatio();

	TCVector cameraPosition = m_camera.getPosition();

	TCVector::initIdentityMatrix(positionMatrix);
	positionMatrix[12] = cameraPosition[0] - m_xPan;
	positionMatrix[13] = -cameraPosition[1] + m_yPan;
	positionMatrix[14] = -cameraPosition[2];
	TCVector::initIdentityMatrix(centerMatrix);
	centerMatrix[12] = m_center[0];
	centerMatrix[13] = m_center[1];
	centerMatrix[14] = m_center[2];
	TCVector::multMatrix(otherMatrix, m_rotationMatrix, tmpMatrix);
	TCVector::invertMatrix(tmpMatrix, cameraMatrix);
	TCVector::multMatrix(centerMatrix, cameraMatrix, tmpMatrix);
	TCVector::multMatrix(tmpMatrix, positionMatrix, matrix);

	facing = m_camera.getFacing();
	facing[0] = -facing[0];
	facing.getInverseMatrix(cameraMatrix);
	TCVector::multMatrix(matrix, cameraMatrix, tmpMatrix);
	memcpy(matrix, tmpMatrix, sizeof(matrix));
	cleanupFloats(matrix);
	locationVector = TCVector(matrix[12], matrix[13], matrix[14]);
	location[0] = (double)matrix[12];
	location[1] = (double)matrix[13];
	location[2] = (double)matrix[14];
	cleanupFloats(matrix);
	// Note that the location accuracy isn't nearly as important as the
	// directional accuracy, so we don't have to re-do this string prior
	// to putting it on the clipboard in the POV code copy.
	sprintf(tmpString, "< %s,%s,%s >", ftostr(location[0]).c_str(),
		ftostr(location[1]).c_str(), ftostr(location[2]).c_str());
	locationString = tmpString;

	matrix[12] = matrix[13] = matrix[14] = 0.0f;
	directionVector = directionVector.transformPoint(matrix);
	upVector = upVector.transformPoint(matrix);
	// Grab the values prior to normalization.  That will make the
	// normalization more accurate in double precision.
	directionVector.upConvert(direction);
	lookAtVector = locationVector + directionVector *
		locationVector.length();
	upVector.upConvert(up);
	directionVector = directionVector.normalize();
	upVector = upVector.normalize();
	cleanupFloats(directionVector, 3);
	cleanupFloats(upVector, 3);
	TCVector::doubleNormalize(up);
	TCVector::doubleNormalize(direction);
	TCVector::doubleMultiply(direction, tempV,
		TCVector::doubleLength(location));
	TCVector::doubleAdd(location, tempV, lookAt);
	// Re-do the strings with higher accuracy, so they'll be
	// accepted by POV-Ray.
	sprintf(tmpString, "< %s,%s,%s >", ftostr(up[0], 20).c_str(),
		ftostr(up[1], 20).c_str(), ftostr(up[2], 20).c_str());
	skyString = tmpString;
	sprintf(tmpString, "< %s,%s,%s >", ftostr(lookAt[0], 20).c_str(),
		ftostr(lookAt[1], 20).c_str(), ftostr(lookAt[2], 20).c_str());
	lookAtString = tmpString;
}

void LDPovExporter::writeLight(TCFloat lat, TCFloat lon, int num)
{
	TCVector lightVector(0.0f, 0.0f, 2.0f);
	TCVector lightLoc;
	TCFloat latMatrix[16];
	TCFloat lonMatrix[16];
	TCFloat lightMatrix[16];
	TCFloat tempMatrix[16];
	TCFloat flipMatrix[16];
	TCFloat latRad = (TCFloat)deg2rad(lat);
	TCFloat lonRad = (TCFloat)deg2rad(-lon);

	TCVector::initIdentityMatrix(latMatrix);
	TCVector::initIdentityMatrix(lonMatrix);
	TCVector::initIdentityMatrix(flipMatrix);
	latMatrix[5] = (TCFloat)cos(latRad);
	latMatrix[6] = (TCFloat)-sin(latRad);
	latMatrix[9] = (TCFloat)sin(latRad);
	latMatrix[10] = (TCFloat)cos(latRad);
	lonMatrix[0] = (TCFloat)cos(lonRad);
	lonMatrix[2] = (TCFloat)sin(lonRad);
	lonMatrix[8] = (TCFloat)-sin(lonRad);
	lonMatrix[10] = (TCFloat)cos(lonRad);
	flipMatrix[5] = -1.0f;
	flipMatrix[10] = -1.0f;
	TCVector::multMatrix(lonMatrix, latMatrix, tempMatrix);
	TCVector::multMatrix(flipMatrix, tempMatrix, lightMatrix);
	lightVector.transformPoint(lightMatrix, lightLoc);
	fprintf(m_pPovFile,
		"#ifndef (LDXSkipLight%d)\n"
		"light_source {\t// %s: %s,%s,LDXRadius*2\n"
		"	<%s*LDXRadius,%s*LDXRadius,%s*LDXRadius> + LDXCenter\n"
		"	color rgb <1,1,1>\n"
		"}\n"
		"#end\n",
		num,
		(const char *)ls("PovLatLon"),
		ftostr(lat).c_str(),
		ftostr(lon).c_str(),
		ftostr(lightLoc[0]).c_str(),
		ftostr(lightLoc[1]).c_str(),
		ftostr(lightLoc[2]).c_str());
}

bool LDPovExporter::writeLights(void)
{
	fprintf(m_pPovFile, "// Lights\n");
	QStringList lights = m_lights.split(",", QString::SkipEmptyParts);
	bool fieldOk[3];
	for (QString light: lights){
	       QStringList lightEntryFields = light.split(" ",QString::SkipEmptyParts);
	       float latitude = QString("%1").arg(lightEntryFields.at(2)).toFloat(&fieldOk[0]);
	       float longitude = QString("%1").arg(lightEntryFields.at(4)).toFloat(&fieldOk[1]);
	       int lightNumber = QString("%1").arg(lightEntryFields.at(0)).replace(".","").toInt(&fieldOk[2]);

               if (fieldOk[0] && fieldOk[1] && fieldOk[2])
               {
                      writeLight(latitude,longitude,lightNumber);
               }
               else
               {
                      fprintf(stdout, "Could not create light '%s'.\n",light.toLatin1().constData());
                      return false;
               }
        }
//	writeLight(45.0, 0.0, 1);
//	writeLight(30.0, 120.0, 2);
//	writeLight(60.0, -120.0, 3);
	//writeLight(45.0, 0.0, m_radius * 2.0f);
	//writeLight(30.0, 120.0, m_radius * 2.0f);
	//writeLight(60.0, -120.0, m_radius * 2.0f);
	return true;
}

bool LDPovExporter::writeCamera(void)
{

	fprintf(m_pPovFile, "// Camera\n");
	fprintf(m_pPovFile,
		"#ifndef (LDXSkipCamera)\n"
		"camera {\n"
		"\t#declare LDXCamAspect = %s;\n"
		"\tlocation LDXCameraLoc\n"
		"\tsky LDXCameraSky\n"
		"\tright LDXCamAspect * < -1,0,0 >\n"
		"\tlook_at LDXCameraLookAt\n"
		"\tangle %s\n"
		"}\n"
		"#end\n\n",
		getAspectRatio().c_str(), ftostr(getHFov()).c_str());
	return true;
}

std::string LDPovExporter::findInclude(const std::string &filename)
{
	for (StringList::const_iterator it = m_searchPath.begin();
		it != m_searchPath.end(); ++it)
	{
		std::string path = *it + filename;
		struct stat statData;

		if (stat(path.c_str(), &statData) == 0)
		{
			if ((statData.st_mode & S_IFDIR) == 0)
			{
				return path;
			}
		}
	}
	return "";
}

bool LDPovExporter::writeCode(const std::string &code, bool lineFeed /*= true*/)
{
	if (m_codes.find(code) == m_codes.end())
	{
		fprintf(m_pPovFile, "%s\n", code.c_str());
		if (lineFeed)
		{
			fprintf(m_pPovFile, "\n");
		}
		m_codes.insert(code);
		return true;
	}
	return false;
}

bool LDPovExporter::writeInclude(
	const std::string &filename,
	bool lineFeed /*= true*/,
	const LDLModel *pModel /*= NULL*/)
{
	if (m_includes.find(filename) == m_includes.end())
	{
		StringStringMap::iterator it = m_includeVersions.find(filename);
		std::string version;

		if (it != m_includeVersions.end())
		{
			version = it->second;
		}
		if (version.size() > 0)
		{
			fprintf(m_pPovFile, "#if (version > %s) #version %s; #end\n",
				version.c_str(), version.c_str());
		}
		fprintf(m_pPovFile, "#include \"%s\"", filename.c_str());
		if (pModel)
		{
			writeDescriptionComment(pModel);
		}
		else
		{
			fprintf(m_pPovFile, "\n");
		}
		if (version.size() > 0)
		{
			fprintf(m_pPovFile, "#if (version < LDXOrigVer) #version LDXOrigVer; #end");
		}
		if (lineFeed)
		{
			fprintf(m_pPovFile, "\n");
		}
		m_includes.insert(filename);
		return true;
	}
	return false;
}

std::string LDPovExporter::findMainPovName(const PovMapping &mapping)
{
	for (PovNameList::const_iterator it = mapping.names.begin();
		it != mapping.names.end(); ++it)
	{
		const PovName &name = *it;

		if (name.attributes.size() == 0)
		{
			return name.name;
		}
	}
	return "";
}

const PovName *LDPovExporter::findPovName(
	const PovMapping &mapping,
	const char *attrName,
	const char *attrValue)
{
	for (PovNameList::const_iterator it = mapping.names.begin();
		it != mapping.names.end(); ++it)
	{
		const PovName &name = *it;
		StringStringMap::const_iterator itss = name.attributes.find(attrName);

		if (itss != name.attributes.end() && itss->second == attrValue)
		{
			return &name;
		}
	}
	return NULL;
}

bool LDPovExporter::findXmlModelInclude(const LDLModel *pModel)
{
	const std::string modelFilename = getModelFilename(pModel);
	std::string key = lowerCaseString(modelFilename);

	PovElementMap::const_iterator it = m_xmlElements.find(key);
	if (it != m_xmlElements.end())
	{
		const PovElement &element = it->second;
		StringList::const_iterator itFilename;
		StringList::const_iterator itCode;
		//bool wrote = false;
		size_t i = 0;
		std::string declareFilename;

		for (itCode = element.povCodes.begin();
			itCode != element.povCodes.end(); ++itCode)
		{
			writeCode(*itCode);
		}
		for (itFilename = element.povFilenames.begin();
			itFilename != element.povFilenames.end(); ++itFilename)
		{
			const LDLModel *pDescModel = NULL;

			if (i == element.povFilenames.size() - 1)
			{
				pDescModel = pModel;
			}
			writeInclude(*itFilename, true, pDescModel);
			//if (writeInclude(*itFilename, true, pDescModel))
			//{
			//	wrote = true;
			//}
			i++;
		}
		declareFilename = modelFilename;
		convertStringToLower(&declareFilename[0]);
		m_declareNames[declareFilename] = findMainPovName(element);
		m_declareNames[declareFilename + ":mirror"] =
			m_declareNames[declareFilename];
		m_matrices[key] = element.matrix;
		return true;
	}
	return false;
}

bool LDPovExporter::findModelInclude(const LDLModel *pModel)
{
	const std::string &modelFilename = getModelFilename(pModel);
	if (m_xmlMap && findXmlModelInclude(pModel))
	{
		return true;
	}
	if (!m_findReplacements)
	{
		return false;
	}
	std::string incFilename = modelFilename;
	size_t dotSpot = incFilename.find('.');

	if (dotSpot < incFilename.size())
	{
		incFilename.resize(dotSpot);
	}
	incFilename += ".inc";
	for (StringList::const_iterator it = m_searchPath.begin();
		it != m_searchPath.end(); ++it)
	{
		std::string filename = *it + incFilename;
		FILE *pIncFile = ucfopen(filename.c_str(), "r");

		if (pIncFile)
		{
			char buf[1024];
			bool found = false;
			std::string declareName;

			for ( ; ; )
			{
				char *spot;

				if (fgets(buf, sizeof(buf), pIncFile) == NULL)
				{
					break;
				}
				buf[sizeof(buf) - 1] = 0;
				stripLeadingWhitespace(buf);
				if (stringHasCaseInsensitivePrefix(buf, "#declare"))
				{
					size_t declareLen = strlen("#declare");
					memmove(buf, &buf[declareLen],
						strlen(buf) + 1 - declareLen);
					replaceStringCharacter(buf, '\t', ' ');
					// Replace all multi-spaces with single spaces.
					spot = buf;
					while ((spot = strstr(spot, "  ")) != NULL)
					{
						memmove(spot, spot + 1, strlen(spot));
					}
					if (buf[0] == ' ')
					{
						char declareBuf[1024];

						if (sscanf(buf, " %s", declareBuf) == 1)
						{
							if (declareBuf[0])
							{
								declareName = declareBuf;
								found = true;
							}
						}
					}
				}
			}
			fclose(pIncFile);
			if (found)
			{
				m_declareNames[lowerCaseString(modelFilename)] = declareName;
				writeInclude(incFilename, true, pModel);
				return true;
			}
		}
	}
	return false;
}

void LDPovExporter::writeDescriptionComment(const LDLModel *pModel)
{
		if (pModel->getDescription() != NULL)
		{
			fprintf(m_pPovFile," // %s\n", pModel->getDescription());
		}
		else
		{
			fprintf(m_pPovFile,"\n");
		}
}

bool LDPovExporter::findModelGeometry(
	LDLModel *pModel,
	IntShapeListMap &colorGeometryMap,
	bool mirrored,
	const TCFloat *matrix,
	bool inPart,
	int colorNumber /*= 16*/)
{
	LDLFileLineArray *fileLines = pModel->getFileLines();
	int count = pModel->getActiveLineCount();
	bool retValue = false;
	bool skipping = false;
	const char *modelName = pModel->getName();

	if (modelName != NULL && m_xmlElements.find(lowerCaseString(modelName)) !=
		m_xmlElements.end())
	{
		return false;
	}
	if (m_smoothCurves)
	{
		if (inPart && matrix == NULL)
		{
			skipping = true;
		}
		m_primSubCheck = true;
	}
	for (int i = 0; i < count; i++)
	{
		LDLFileLine *pFileLine = (*fileLines)[i];

		if (!pFileLine->isValid())
		{
			continue;
		}
		if (pFileLine->getLineType() == LDLLineTypeModel)
		{
			LDLModelLine *pModelLine = (LDLModelLine *)pFileLine;
			LDLModel *pOtherModel = pModelLine->getModel(true);

			if (pOtherModel != NULL)
			{
				if (!skipping && m_smoothCurves &&
					(pModel->isPart() || matrix != NULL) &&
					(!m_primSub ||
					!performPrimitiveSubstitution(pOtherModel, false)))
				{
					TCFloat newMatrix[16];
					int newColorNumber = pModelLine->getColorNumber();

					if (matrix)
					{
						TCVector::multMatrix(matrix, pModelLine->getMatrix(),
							newMatrix);
					}
					else
					{
						memcpy(newMatrix, pModelLine->getMatrix(),
							sizeof(newMatrix));
					}
					if (newColorNumber == 16)
					{
						newColorNumber = colorNumber;
					}
					if (findModelGeometry(pOtherModel, colorGeometryMap,
						mirrored, newMatrix, false, newColorNumber))
					{
						retValue = true;
					}
				}
				if (!m_emptyModels[getDeclareName(pOtherModel, mirrored,
					inPart)])
				{
					retValue = true;
				}
			}
		}
		else if (pFileLine->getLineType() == LDLLineTypeTriangle ||
			pFileLine->getLineType() == LDLLineTypeQuad ||
			(m_smoothCurves && pFileLine->getLineType() == LDLLineTypeLine))
		{
			if (!skipping)
			{
				LDLShapeLine *pShapeLine = (LDLShapeLine *)pFileLine;
				int newColorNumber = pShapeLine->getColorNumber();

				if (newColorNumber == 16)
				{
					newColorNumber = colorNumber;
				}
				try
				{
					colorGeometryMap[newColorNumber].push_back(
						Shape(pShapeLine->getPoints(), pShapeLine->getNumPoints(),
						matrix));
					retValue = true;
				}
				catch (...)
				{
					// Don't do anything.
				}
			}
		}
	}
	m_primSubCheck = false;
	if (pModel == m_pTopModel)
	{
		// Even if there isn't any output, you'll get a parse error if we
		// don't spit out the object for the main model, so set haveOutput
		// to true.
		// Note that we still have to run the rest of this function to fill in
		// the various data structures.
		return true;
	}
	return retValue;
}

bool LDPovExporter::writeModelObject(
	LDLModel *pModel,
	bool mirrored,
	const TCFloat *matrix,
	bool inPart)
{
	if (!m_primSub || !performPrimitiveSubstitution(pModel, false))
	{
		std::string modelFilename = getModelFilename(pModel);
		LDLFileLineArray *fileLines = pModel->getFileLines();
		int i;
		int count = pModel->getActiveLineCount();
		std::string declareName = getDeclareName(pModel, mirrored, inPart);
		IntShapeListMap colorGeometryMap;

		if (!m_writtenModels.insert(declareName).second)
		{
			return true;
		}
		if (findModelGeometry(pModel, colorGeometryMap, mirrored,
			NULL, inPart))
		{
			bool ifStarted = false;
			bool elseStarted = false;
			bool povMode = false;
			bool studsStarted = false;

			if (pModel->isPart() && pModel != m_pTopModel)
			{
				TCVector min, max;

				pModel->getBoundingBox(min, max);
				fprintf(m_pPovFile, "#declare %s =\n", declareName.c_str());
				fprintf(m_pPovFile,
					"#if (LDXQual = 0)\n"
					"box {\n\t");
				writePoint(min);
				fprintf(m_pPovFile, ",");
				writePoint(max);
				fprintf(m_pPovFile, "\n"
					"}\n"
					"#else\n"
					"union {\n");
			}
			else
			{
				fprintf(m_pPovFile, "#declare %s = union {\n", declareName.c_str());
			}
			for (i = 0; i < count; i++)
			{
				LDLFileLine *pFileLine = (*fileLines)[i];

				if (!pFileLine->isValid())
				{
					continue;
				}
				if (pFileLine->getLineType() == LDLLineTypeModel)
				{
					if (((LDLModelLine *)pFileLine)->getModel(true) != NULL)
					{
						writeModelLine((LDLModelLine *)pFileLine, studsStarted,
							mirrored, matrix, inPart || pModel->isPart());
					}
				}
				else if (pFileLine->getLineType() == LDLLineTypeComment)
				{
					endStuds(studsStarted);
					writeCommentLine((LDLCommentLine *)pFileLine, ifStarted,
						elseStarted, povMode);
				}
			}
			endStuds(studsStarted);
			if (ifStarted)
			{
				consolePrintf("0 L3P ENDPOV missing from file.\n");
			}
			writeGeometry(colorGeometryMap);
			if (pModel == m_pTopModel)
			{
				if (m_edges)
				{
					fprintf(m_pPovFile,
						"#if (LDXSkipEdges = 0)\n"
						"	object { LDXEdges }\n"
						"#end\n");
					if (m_conditionalEdges)
					{
						fprintf(m_pPovFile, "	object { LDXConditionalEdges }\n");
					}
				}
				fprintf(m_pPovFile,
					"#if (LDXRefls = 0)\n"
					"	no_reflection\n"
					"#end\n");
				fprintf(m_pPovFile,
					"#if (LDXShads = 0)\n"
					"	no_shadow\n"
					"#end\n");
			}
			if (pModel->isPart() && pModel != m_pTopModel)
			{
				fprintf(m_pPovFile, "}\n#end\n\n");
			}
			else
			{
				fprintf(m_pPovFile, "}\n\n");
			}
		}
		else
		{
			m_emptyModels[declareName] = true;
		}
	}
	return true;
}

void LDPovExporter::writeMesh(int colorNumber, const ShapeList &list)
{
	startMesh();
	for (ShapeList::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		const TCVectorVector &points = it->points;

		if (points.size() == 3)
		{
			writeTriangle(&points[0]);
		}
		else if (points.size() == 4)
		{
			writeTriangle(&points[0]);
			writeTriangle(&points[0], 4, 2);
		}
	}
	if (colorNumber != 16)
	{
		fprintf(m_pPovFile, "\t");
		writeColor(colorNumber);
		fprintf(m_pPovFile, "\n");
	}
	endMesh();
}

void LDPovExporter::writeMesh2(
	int colorNumber,
	const VectorSizeTMap &vertices,
	const VectorSizeTMap &normals,
	const SmoothTriangleVector &triangles)
{
	int total = 0;
	VectorSizeTMap::const_iterator it;
	size_t i;

	startMesh2();
	startMesh2Section("vertex_vectors");
	fprintf(m_pPovFile, "%d,\n\t\t\t", (int)vertices.size());
	for (it = vertices.begin(); it != vertices.end(); ++it)
	{
		writeMesh2Vertices(&it->first, 1, total);
	}
	endMesh2Section();
	startMesh2Section("normal_vectors");
	fprintf(m_pPovFile, "%d,\n\t\t\t", (int)normals.size());
	total = 0;
	for (it = normals.begin(); it != normals.end(); ++it)
	{
		writeMesh2Vertices(&it->first, 1, total);
	}
	endMesh2Section();
	startMesh2Section("face_indices");
	fprintf(m_pPovFile, "%d,\n\t\t\t", (int)triangles.size());
	total = 0;
	for (i = 0; i < triangles.size(); i++)
	{
		const SmoothTriangle &triangle = triangles[i];

		writeMesh2Indices(triangle.vertexIndices[0], triangle.vertexIndices[1],
			triangle.vertexIndices[2], total);
	}
	endMesh2Section();
	startMesh2Section("normal_indices");
	fprintf(m_pPovFile, "%d,\n\t\t\t", (int)triangles.size());
	total = 0;
	for (i = 0; i < triangles.size(); i++)
	{
		const SmoothTriangle &triangle = triangles[i];

		writeMesh2Indices(triangle.normalIndices[0], triangle.normalIndices[1],
			triangle.normalIndices[2], total);
	}
	endMesh2Section();
	if (colorNumber != 16)
	{
		fprintf(m_pPovFile, "\t");
		writeColor(colorNumber);
		fprintf(m_pPovFile, "\n");
	}
	endMesh();
}

void LDPovExporter::writeMesh2(int colorNumber, const ShapeList &list)
{
	int total = 0;
	int current = 0;
	int vertexCount = 0;
	int faceCount = 0;
	ShapeList::const_iterator it;

	for (it = list.begin(); it != list.end(); ++it)
	{
		const TCVectorVector &points = it->points;

		if (points.size() == 3)
		{
			faceCount += 1;
			vertexCount += 3;
		}
		else if (points.size() == 4)
		{
			faceCount += 2;
			vertexCount += 4;
		}
	}
	if (faceCount == 0 || vertexCount == 0)
	{
		return;
	}
	startMesh2();
	startMesh2Section("vertex_vectors");
	fprintf(m_pPovFile, "%d,\n\t\t\t", vertexCount);
	for (it = list.begin(); it != list.end(); ++it)
	{
		const TCVectorVector &points = it->points;

		if (points.size() == 3 || points.size() == 4)
		{
			writeMesh2Vertices(&points[0], points.size(), total);
		}
	}
	endMesh2Section();
	total = 0;
	startMesh2Section("face_indices");
	fprintf(m_pPovFile, "%d,\n\t\t\t", faceCount);
	for (it = list.begin(); it != list.end(); ++it)
	{
		const TCVectorVector &points = it->points;

		if (points.size() == 3)
		{
			writeMesh2Indices(current, current + 1, current + 2, total);
			current += 3;
		}
		else if (points.size() == 4)
		{
			writeMesh2Indices(current, current + 1, current + 2, total);
			writeMesh2Indices(current, current + 2, current + 3, total);
			current += 4;
		}
	}
	endMesh2Section();
	if (colorNumber != 16)
	{
		fprintf(m_pPovFile, "\t");
		writeColor(colorNumber);
		fprintf(m_pPovFile, "\n");
	}
	endMesh();
}

//The parametric equations for a line passing through (x1 y1 z1), (x2 y2 z2) are
//        x = x1 + (x2 - x1)*t
//        y = y1 + (y2 - y1)*t
//        z = z1 + (z2 - z1)*t
void LDPovExporter::smoothGeometry(
	int colorNumber,
	const ShapeList &list,
	const ShapeList &edges,
	VectorSizeTMap &vertices,
	VectorSizeTMap &normals,
	SmoothTriangleVector &triangles)
{
	int triangleCount = 0;
	int current = 0;
	size_t index;
	VectorSizeTMap::iterator itmvs;
	size_t i;

	ShapeList::const_iterator it;
	// One entry per infinite line.  Each entry is a list of all the triangles
	// that share that infinite line.
	TriangleEdgesMap triangleEdges;

	// One entry per triangle vertex.  Each entry is a list of all the triangles
	// that share that vertex.
	TrianglePPointsMap trianglePoints;

	// Map from vertex index to vertex.
	SizeTVectorMap indexToVert;
	// One entry per infinite line.  The entry contains a map of all the edge
	// end points along that infinite line.
	EdgeMap edgesMap;

	for (it = edges.begin(); it != edges.end(); ++it)
	{
		const TCVectorVector &points = it->points;

		if (points.size() == 2)
		{
		      if (points[0] < points[1])
		      {
			edgesMap[LineKey(points[0], points[1])].
				push_back(LinePair(points[0], points[1]));
		      }
		      else if (points[0] > points[1])
		      {
			edgesMap[LineKey(points[0], points[1])].
				push_back(LinePair(points[1], points[0]));
		      }
		}
	}

	for (it = list.begin(); it != list.end(); ++it)
	{
		const TCVectorVector &points = it->points;
		size_t count = points.size();

		if (count != 2)
		{
			for (i = 0; i < count; i++)
			{
				// Make sure points[i] is in the map
				vertices[points[i]];
			}
		}
	}
	index = 0;
	for (itmvs = vertices.begin(); itmvs != vertices.end(); ++itmvs)
	{
		itmvs->second = index++;
		indexToVert[itmvs->second] = itmvs->first;
	}
	for (it = list.begin(); it != list.end(); ++it)
	{
		const TCVectorVector &points = it->points;

		if (points.size() == 3)
		{
			triangleCount += 1;
		}
		else if (points.size() == 4)
		{
			triangleCount += 2;
		}
	}
	triangles.resize(triangleCount);
	for (it = list.begin(); it != list.end(); ++it)
	{
		const TCVectorVector &points = it->points;

		if (points.size() != 2)
		{
			SmoothTriangle &triangle = triangles[current];

			triangle.colorNumber = colorNumber;
			if (!initSmoothTriangle(triangle, vertices, trianglePoints,
				indexToVert, points[0], points[1], points[2]))
			{
				triangles.erase(triangles.begin() + current);
				current++;
				continue;
			}
			if (points.size() == 4)
			{
				SmoothTriangle &triangle2 = triangles[++current];

				triangle2.colorNumber = triangle.colorNumber;
				if (!initSmoothTriangle(triangle2, vertices, trianglePoints,
					indexToVert, points[0], points[2], points[3]))
				{
					triangles.erase(triangles.begin() + current);
					current++;
					continue;
				}
			}
			current++;
		}
	}
	for (i = 0; i < triangles.size(); i++)
	{
		SmoothTriangle &triangle = triangles[i];

		for (size_t j = 0; j < 3; j++)
		{
			triangleEdges[triangle.lineKeys[j]].insert(&triangle);
		}
	}
	for (TriangleEdgesMap::iterator itmte = triangleEdges.begin();
		itmte != triangleEdges.end(); ++itmte)
	{
		const LineKey &lineKey = itmte->first;
		SmoothTrianglePSet &otherTriangles = itmte->second;

		if (otherTriangles.size() > 1)
		{
			EdgeMap::const_iterator itme = edgesMap.find(lineKey);

			if (itme != edgesMap.end())
			{
				for (SmoothTrianglePSet::iterator itsst = otherTriangles.begin();
					itsst != otherTriangles.end(); ++itsst)
				{
					SmoothTriangle &triangle = **itsst;
					int index2 = findEdge(triangle, lineKey);

					if (index2 >= 0)
					{
						triangle.hardEdges[index2] = true;
					}
				}
			}
		}
	}
	for (TrianglePPointsMap::iterator itmtp = trianglePoints.begin();
		itmtp != trianglePoints.end(); ++itmtp)
	{
		SmoothTrianglePList &triangleList = itmtp->second;
		const TCVector &point = itmtp->first;

		if (triangleList.size() > 1)
		{
#if defined(_MSC_VER) && _MSC_VER <= 1200
			SmoothTrianglePVector otherTriangles;

			otherTriangles.reserve(triangleList.size());
			for (SmoothTrianglePList::const_iterator it = triangleList.begin();
				it != triangleList.end(); ++it)
			{
				otherTriangles.push_back(*it);
			}
#else
			SmoothTrianglePVector otherTriangles(triangleList.begin(),
				triangleList.end());
#endif
			size_t processed = 0;
			size_t j, k;
			TCVectorVector otherNormals;

			otherNormals.resize(otherTriangles.size());
			for (i = 0; i < otherTriangles.size() && processed < otherTriangles.size();
				i++)
			{
				SmoothTriangle &passTriangle = *otherTriangles[i];

				if (passTriangle.smoothPass == 0)
				{
					LineKeySet hardEdges;
					LineKeySet softEdges;
					int index1 = findPoint(passTriangle, point, indexToVert);
					int index2 = (index1 + 2) % 3;
					TCVector normal = passTriangle.normals[point];
					TCVector lastNormal(normal);
					TCVector firstNormal(normal);

					if (passTriangle.hardEdges[index1])
					{
						hardEdges.insert(passTriangle.lineKeys[index1]);
					}
					else
					{
						softEdges.insert(passTriangle.lineKeys[index1]);
					}
					if (passTriangle.hardEdges[index2])
					{
						hardEdges.insert(passTriangle.lineKeys[index2]);
					}
					else
					{
						softEdges.insert(passTriangle.lineKeys[index2]);
					}
					passTriangle.smoothPass = i + 1;
					processed++;
					for (j = i + 1; j < otherTriangles.size() &&
						processed < otherTriangles.size(); j++)
					{
						// Unfortunately, we have to process all the rest n^2
						// times, because we don't have any control over what
						// order they'll be in.  That means this is a O(n^3)
						// operation, which is horrible.  Fortunately, n is
						// almost always less than 10, since it's the number
						// of triangles that have this point in common.
						for (k = i + 1; k < otherTriangles.size() &&
							processed < otherTriangles.size(); k++)
						{
							SmoothTriangle &triangle = *otherTriangles[k];

							if (triangle.smoothPass == 0)
							{
								int index3 = findPoint(triangle, point, indexToVert);
								int index4 = (index3 + 2) % 3;

								if (softEdges.find(triangle.lineKeys[index3]) !=
									softEdges.end() ||
									softEdges.find(triangle.lineKeys[index4]) !=
									softEdges.end())
								{
									TCVector triNormal = triangle.normals[point];
									bool smoothed = false;

									if (!triangle.hardEdges[index3])
									{
										softEdges.insert(triangle.lineKeys[index3]);
									}
									if (!triangle.hardEdges[index4])
									{
										softEdges.insert(triangle.lineKeys[index4]);
									}
									if (triNormal == firstNormal ||
										triNormal == lastNormal)
									{
										smoothed = true;
									}
									else if (shouldSmooth(triNormal, lastNormal))
									{
										if (shouldFlipNormal(triNormal, normal))
										{
											normal -= triNormal;
										}
										else
										{
											normal += triNormal;
										}
										smoothed = true;
									}
									if (smoothed)
									{
										triangle.smoothPass = i + 1;
										processed++;
										lastNormal = triNormal;
									}
								}
							}
						}
					}
					otherNormals[i] = normal.normalize();
				}
			}
			for (i = 0; i < otherTriangles.size(); i++)
			{
				SmoothTriangle &triangle = *otherTriangles[i];

				if (triangle.smoothPass > 0)
				{
					triangle.setNormal(point, otherNormals[triangle.smoothPass - 1]);
				}
				triangle.smoothPass = 0;
			}
		}
	}
	for (i = 0; i < triangles.size(); i++)
	{
		SmoothTriangle &triangle = triangles[i];

		for (VectorVectorMap::const_iterator itmvv = triangle.normals.begin();
			itmvv != triangle.normals.end(); ++itmvv)
		{
			normals[itmvv->second];
		}
	}
	index = 0;
	for (itmvs = normals.begin(); itmvs != normals.end(); ++itmvs)
	{
		itmvs->second = index++;
	}
	for (i = 0; i < triangles.size(); i++)
	{
		SmoothTriangle &triangle = triangles[i];

		for (int j = 0; j < 3; j++)
		{
			const TCVector &point = indexToVert[triangle.vertexIndices[j]];
			const TCVector &normal = triangle.normals[point];

#ifdef _DEBUG
			if (normals.find(normal) == normals.end())
			{
				assert(false);
				debugPrintf("Normal mapping error.\n");
			}
#endif // _DEBUG
			triangle.normalIndices[j] = (int)normals[normal];
		}
	}
}

int LDPovExporter::findEdge(
	const SmoothTriangle &triangle,
	const LineKey &lineKey)
{
	for (int i = 0; i < 3; i++)
	{
		if (triangle.lineKeys[i] == lineKey)
		{
			return i;
		}
	}
	return -1;
}

int LDPovExporter::findPoint(
	const SmoothTriangle &triangle,
	const TCVector &point,
	const SizeTVectorMap &points)
{
	for (int i = 0; i < 3; i++)
	{
		if (points.find(triangle.vertexIndices[i])->second == point)
		{
			return i;
		}
	}
	return -1;
}

bool LDPovExporter::edgesOverlap(const LinePair &edge1, const LinePair &edge2)
{
	if ((edge1.first <= edge2.first && edge1.second >= edge2.first) ||
		(edge1.first <= edge2.second && edge1.second >= edge2.second))
	{
		return true;
	}
	return false;
}

bool LDPovExporter::onEdge(
	const LinePair &edge,
	const LineList &edges)
{
	for (LineList::const_iterator it = edges.begin(); it != edges.end(); ++it)
	{
		if (edgesOverlap(edge, *it))
		{
			return true;
		}
	}
	return false;
}

// Note: static method
bool LDPovExporter::shouldFlipNormal(
	const TCVector &normal1,
	const TCVector &normal2)
{
	if (normal1[0] == 0.0f && normal1[1] == 0.0f &&
		normal1[2] == 0.0f)
	{
		return false;
	}
	else
	{
		TCFloat dotProduct = normal1.dot(normal2);

		if (!fEq(dotProduct, 0.0f) && dotProduct < 0.0f)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

bool LDPovExporter::normalsCheck(const TCVector &normal1, TCVector normal2)
{
	if (shouldFlipNormal(normal1, normal2))
	{
		normal2 *= -1.0;
	}
	if (normal1.dot(normal2) > SMOOTH_THRESHOLD)
	{
		return true;
	}
	return false;
}

bool LDPovExporter::shouldSmooth(
	const TCVector &normal1,
	const TCVector &normal2)
{
	TCFloat dotProduct;
	TCVector normal2Norm(normal2);

	normal2Norm.normalize();
	if (shouldFlipNormal(normal1, normal2Norm))
	{
		dotProduct = normal2Norm.dot(-normal1);
	}
	else
	{
		dotProduct = normal2Norm.dot(normal1);
	}
	// The following number is the cos of 25 degrees.  I don't want to
	// calculate it on the fly.  We only want to apply this normal if the
	// difference between it an the original normal is less than 25 degrees.
	// If the normal only applies to two faces, then the faces have to be
	// more than 50 degrees apart for this to happen.  Note that low-res
	// studs have 45-degree angles between the faces, so 50 gives a little
	// leeway.
	return dotProduct > SMOOTH_THRESHOLD;
}

bool LDPovExporter::trySmooth(const TCVector &normal1, TCVector &normal2)
{
	TCFloat dotProduct;
	bool flip;
	TCVector normal2Norm(normal2);

	normal2Norm.normalize();
	if (shouldFlipNormal(normal1, normal2Norm))
	{
		dotProduct = normal2Norm.dot(-normal1);
		flip = true;
	}
	else
	{
		dotProduct = normal2Norm.dot(normal1);
		flip = false;
	}
	// The following number is the cos of 25 degrees.  I don't want to
	// calculate it on the fly.  We only want to apply this normal if the
	// difference between it an the original normal is less than 25 degrees.
	// If the normal only applies to two faces, then the faces have to be
	// more than 50 degrees apart for this to happen.  Note that low-res
	// studs have 45-degree angles between the faces, so 50 gives a little
	// leeway.
	if (dotProduct > SMOOTH_THRESHOLD)
	{
		if (flip)
		{
			normal2 -= normal1;
		}
		else
		{
			normal2 += normal1;
		}
		return true;
	}
	return false;
}

bool LDPovExporter::initSmoothTriangle(
	SmoothTriangle &triangle,
	VectorSizeTMap &vertices,
	TrianglePPointsMap &trianglePoints,
	SizeTVectorMap &indexToVert,
	const TCVector &point1,
	const TCVector &point2,
	const TCVector &point3)
{
	memset(triangle.normalIndices, 0, sizeof(triangle.normalIndices));
	triangle.smoothPass = 0;
	triangle.vertexIndices[0] = (int)vertices[point1];
	triangle.vertexIndices[1] = (int)vertices[point2];
	triangle.vertexIndices[2] = (int)vertices[point3];
	if (!triangle.initLineKeys(indexToVert))
	{
		return false;
	}
	TCVector normal = ((point3 - point1) * (point2 - point1)).normalize();
	triangle.normals.insert(VectorVectorMap::value_type(point1, normal));
	triangle.normals.insert(VectorVectorMap::value_type(point2, normal));
	triangle.normals.insert(VectorVectorMap::value_type(point3, normal));
	triangle.edgeNormals[0] = normal;
	triangle.edgeNormals[1] = normal;
	triangle.edgeNormals[2] = normal;
	triangle.hardEdges[0] = false;
	triangle.hardEdges[1] = false;
	triangle.hardEdges[2] = false;
	trianglePoints[point1].push_back(&triangle);
	trianglePoints[point2].push_back(&triangle);
	trianglePoints[point3].push_back(&triangle);
	return true;
}

void LDPovExporter::writeGeometry(IntShapeListMap &colorGeometryMap)
{
	ShapeList &edges = colorGeometryMap[24];

	for (IntShapeListMap::const_iterator it = colorGeometryMap.begin();
		it != colorGeometryMap.end(); ++it)
	{
		if (m_smoothCurves)
		{
			VectorSizeTMap vertices;
			VectorSizeTMap normals;
			SmoothTriangleVector triangles;
			TCFloat origEpsilon = TCVector::getEpsilon();

			TCVector::setEpsilon(SMOOTH_EPSILON);
			smoothGeometry(it->first, it->second, edges, vertices, normals,
				triangles);
			if (vertices.size() > 0 && normals.size() > 0)
			{
				writeMesh2(it->first, vertices, normals, triangles);
			}
			else
			{
				assert(triangles.size() == 0);
			}
			TCVector::setEpsilon(origEpsilon);
		}
		else if (m_mesh2)
		{
			writeMesh2(it->first, it->second);
		}
		else
		{
			writeMesh(it->first, it->second);
		}
	}
}

void LDPovExporter::writeSeamMacro(void)
{
	fprintf(m_pPovFile,
		"\n#macro LDXSeamMatrix(Width, Height, Depth, CenterX, CenterY, CenterZ)\n"
		"#local aw = 0;\n"
		"#local ah = 0;\n"
		"#local ad = 0;\n"
		"#local ax = 0;\n"
		"#local ay = 0;\n"
		"#local az = 0;\n"
		"#if (Width != 0)\n"
		"#local aw = 1-LDXSW/Width;\n"
		"#end\n"
		"#if (Height != 0)\n"
		"#local ah = 1-LDXSW/Height;\n"
		"#end\n"
		"#if (Depth != 0)\n"
		"#local ad = 1-LDXSW/Depth;\n"
		"#end\n"
		"#if (Width != 0 & CenterX != 0)\n"
		"#local ax = LDXSW/(Width / CenterX);\n"
		"#end\n"
		"#if (Height != 0 & CenterY != 0)\n"
		"#local ay = LDXSW/(Height / CenterY);\n"
		"#end\n"
		"#if (Depth != 0 & CenterZ != 0)\n"
		"#local az = LDXSW/(Depth / CenterZ);\n"
		"#end\n"
		"#if (aw <= 0)\n"
		"#local aw = 1;\n"
		"#local ax = 0;\n"
		"#end\n"
		"#if (ah <= 0)\n"
		"#local ah = 1;\n"
		"#local ay = 0;\n"
		"#end\n"
		"#if (ad <= 0)\n"
		"#local ad = 1;\n"
		"#local az = 0;\n"
		"#end\n"
		"matrix <aw,0,0,0,ah,0,0,0,ad,ax,ay,az>\n"
		"#end\n\n");
}

void LDPovExporter::writeSeamMatrix(LDLModelLine *pModelLine)
{
	LDLModel *pModel = pModelLine->getModel(true);

	if (pModel && pModel->isPart() && pModel != m_pTopModel &&
		!pModel->getNoShrinkFlag())
	{
		TCVector min, max, size, center;

		pModel->getBoundingBox(min, max);
		size = max - min;
		center = (min + max) / 2.0f;
		fprintf(m_pPovFile, "LDXSeamMatrix(%s, %s, %s, %s, %s, %s)\n\t\t",
			ftostr(size[0]).c_str(), ftostr(size[1]).c_str(), ftostr(size[2]).c_str(),
			ftostr(center[0]).c_str(), ftostr(center[1]).c_str(),
			ftostr(center[2]).c_str());
		//fprintf(m_pPovFile, "matrix <%s,0,0,0,%s,0,0,0,%s,%s,%s,%s>\n\t\t",
		//	getSizeSeamString(size[0]).c_str(),
		//	getSizeSeamString(size[1]).c_str(),
		//	getSizeSeamString(size[2]).c_str(),
		//	getOfsSeamString(center[0], size[0]).c_str(),
		//	getOfsSeamString(center[1], size[1]).c_str(),
		//	getOfsSeamString(center[2], size[2]).c_str());
	}
}

bool LDPovExporter::writeXmlMatrix(const char *filename)
{
	if (filename != NULL)
	{
		std::string key = lowerCaseString(filename);
		MatrixMap::const_iterator it = m_matrices.find(key);

		if (it != m_matrices.end())
		{
			const TCFloat *matrix = it->second;

			writeMatrix(matrix);
			return true;
		}
	}
	return false;
}

void LDPovExporter::writeMatrix(const TCFloat *matrix)
{
	bool allZero = true;
	int col;

	fprintf(m_pPovFile, "matrix <");
	for (col = 0; col < 4 && allZero; col++)
	{
		for (int row = 0; row < 3 && allZero; row++)
		{
			float value = matrix[col * 4 + row];

			if (value != 0.0f)
			{
				allZero = false;
			}
		}
	}
	if (allZero)
	{
		debugPrintf("zero matrix.\n");
	}
	for (col = 0; col < 4; col++)
	{
		for (int row = 0; row < 3; row++)
		{
			float value = matrix[col * 4 + row];

			if (allZero && row == col)
			{
				value = 1.0f;
			}
			if (row == 0 && col == 0)
			{
				fprintf(m_pPovFile, "%s", ftostr(value).c_str());
			}
			else
			{
				fprintf(m_pPovFile, ",%s", ftostr(value).c_str());
			}
		}
	}
	fprintf(m_pPovFile, ">");
}

bool LDPovExporter::writeColor(int colorNumber, bool slope)
{
	if (colorNumber != 16)
	{
		if (slope &&
			m_xmlColors.find((TCULong)colorNumber) == m_xmlColors.end())
		{
			// Slope versions of colors only get created for colors specifically
			// in the XML map.  For custom colors, no _slope version of the
			// color definition gets created, so don't include that here.
			slope = false;
		}
		fprintf(m_pPovFile,
			"\t#if (version >= 3.1) material #else texture #end { LDXColor%d%s }",
			colorNumber, slope ? "_slope" : "");
		return true;
	}
	return false;
}

LDPovExporter::ColorType LDPovExporter::getColorType(int colorNumber)
{
	int r, g, b, a;
	LDLPalette *pPalette = m_pTopModel->getMainModel()->getPalette();
	LDLColorInfo colorInfo;

	pPalette->getRGBA(colorNumber, r, g, b, a);
	colorInfo = pPalette->getAnyColorInfo(colorNumber);
	if (a == 255)
	{
		if (colorInfo.rubber)
		{
			return CTRubber;
		}
		else if (colorInfo.chrome)
		{
			return CTChrome;
		}
		else
		{
			return CTOpaque;
		}
	}
	else
	{
		return CTTransparent;
	}
}

void LDPovExporter::writeInnerColorDeclaration(
	int colorNumber,
	bool slope)
{
	if (colorNumber != 16)
	{
		int r, g, b, a;
		PovColorMap::const_iterator it = m_xmlColors.end();
		LDLPalette *pPalette = m_pTopModel->getMainModel()->getPalette();
		LDLColorInfo colorInfo;

		pPalette->getRGBA(colorNumber, r, g, b, a);
		colorInfo = pPalette->getAnyColorInfo(colorNumber);
		if (m_xmlMap)
		{
			it = m_xmlColors.find(colorNumber);
		}
		if (!slope)
		{
			fprintf(m_pPovFile, "\n");
		}
		if (it == m_xmlColors.end())
		{
			const char *macroName = NULL;
			double dr = r / 255.0;
			double dg = g / 255.0;
			double db = b / 255.0;

			switch (getColorType(colorNumber))
			{
			case CTOpaque:
				macroName = "LDXOpaqueColor";
				break;
			case CTTransparent:
				macroName = "LDXTransColor";
				dr = alphaMod(r);
				dg = alphaMod(g);
				db = alphaMod(b);
				break;
			case CTChrome:
				macroName = "LDXChromeColor";
				break;
			case CTRubber:
				macroName = "LDXRubberColor";
				break;
			}
			if (macroName != NULL)
			{
				fprintf(m_pPovFile, "#declare LDXColor%d = %s(%s,%s,%s)\n",
					colorNumber, macroName, ftostr(dr).c_str(),
					ftostr(dg).c_str(), ftostr(db).c_str());
				return;
			}
		}
		fprintf(m_pPovFile,
			"#declare LDXColor%d%s = #if (version >= 3.1) material { #end\n\ttexture {\n",
			colorNumber, slope ? "_slope" : "");
		if (it != m_xmlColors.end())
		{
			fprintf(m_pPovFile, "\t\t%s\n",
				it->second.names.front().name.c_str());
			if (slope)
			{
				fprintf(m_pPovFile, "\t\t#if (LDXQual > 1) normal { bumps 0.3 scale 25*0.02 } #end\n");
			}
			fprintf(m_pPovFile, "\t}\n");
			if (it->second.ior.size() > 0)
			{
				fprintf(m_pPovFile, "\t#if (LDXQual > 1) interior { %s } #end\n", it->second.ior.c_str());
			}
		}
		else
		{
			fprintf(m_pPovFile, "\t\tpigment { ");
			if (a != 255)
			{
				fprintf(m_pPovFile, "#if (LDXQual > 1) ");
			}
			writeRGBA(r, g, b, a);
			if (a != 255)
			{
				fprintf(m_pPovFile, " #else ");
				writeRGBA(r, g, b, 255);
				fprintf(m_pPovFile, " #end");
			}
			fprintf(m_pPovFile, " }\n");
			fprintf(m_pPovFile, "#if (LDXQual > 1)\n");
			fprintf(m_pPovFile, "\t\tfinish { ambient LDXAmb diffuse LDXDif }\n");
			if (a == 255)
			{
				if (colorInfo.rubber)
				{
					fprintf(m_pPovFile, "\t\tfinish { phong LDXRubberPhong phong_size LDXRubberPhongS reflection LDXRubberRefl ");
				}
				else
				{
					fprintf(m_pPovFile, "\t\tfinish { phong LDXPhong phong_size LDXPhongS reflection ");
					if (colorInfo.chrome)
					{
						fprintf(m_pPovFile, "LDXChromeRefl brilliance LDXChromeBril metallic specular LDXChromeSpec roughness LDXChromeRough");
					}
					else
					{
						fprintf(m_pPovFile, "LDXRefl ");
					}
				}
				fprintf(m_pPovFile, "}\n");
			}
			else
			{
				fprintf(m_pPovFile, "\t\tfinish { phong LDXPhong phong_size LDXPhongS reflection LDXTRefl }\n");
			}
			if (a != 255)
			{
				fprintf(m_pPovFile, "\t\t#if (version >= 3.1) #else finish { refraction 1 ior LDXIoR } #end\n");
			}
			fprintf(m_pPovFile, "#end\n");
			fprintf(m_pPovFile, "\t}\n");
			if (a != 255)
			{
				fprintf(m_pPovFile, "#if (version >= 3.1) #if (LDXQual > 1)\n");
				fprintf(m_pPovFile, "\tinterior { ior LDXIoR }\n");
				fprintf(m_pPovFile, "#end #end\n");
			}
		}
		fprintf(m_pPovFile, "#if (version >= 3.1) } #end\n");
	}
}

void LDPovExporter::writeLDXOpaqueColor(void)
{
	if (m_macros.find("LDXOpaqueColor") == m_macros.end())
	{
		fprintf(m_pPovFile, "#ifndef (LDXSkipOpaqueColorMacro)\n");
		fprintf(m_pPovFile, "#macro LDXOpaqueColor(r, g, b)\n");
		fprintf(m_pPovFile, "#if (version >= 3.1) material { #end\n");
		fprintf(m_pPovFile, "	texture {\n");
		fprintf(m_pPovFile, "		pigment { rgbf <r,g,b,0> }\n");
		fprintf(m_pPovFile, "#if (LDXQual > 1)\n");
		fprintf(m_pPovFile, "		finish { ambient LDXAmb diffuse LDXDif }\n");
		fprintf(m_pPovFile, "		finish { phong LDXPhong phong_size LDXPhongS "
			"reflection LDXRefl }\n");
		fprintf(m_pPovFile, "#end\n");
		fprintf(m_pPovFile, "	}\n");
		fprintf(m_pPovFile, "#if (version >= 3.1) } #end\n");
		fprintf(m_pPovFile, "#end\n");
		fprintf(m_pPovFile, "#end\n\n");
		m_macros.insert("LDXOpaqueColor");
	}
}

void LDPovExporter::writeLDXTransColor(void)
{
	if (m_macros.find("LDXTransColor") == m_macros.end())
	{
		fprintf(m_pPovFile, "#ifndef (LDXSkipTransColorMacro)\n");
		fprintf(m_pPovFile, "#macro LDXTransColor(r, g, b)\n");
		fprintf(m_pPovFile, "#if (version >= 3.1) material { #end\n");
		fprintf(m_pPovFile, "	texture {\n");
		fprintf(m_pPovFile, "		pigment { #if (LDXQual > 1) rgbf <r,g,b,LDXTFilt>"
			" #else rgbf <0.6,0.6,0.6,0> #end }\n");
		fprintf(m_pPovFile, "#if (LDXQual > 1)\n");
		fprintf(m_pPovFile, "		finish { ambient LDXAmb diffuse LDXDif }\n");
		fprintf(m_pPovFile, "		finish { phong LDXPhong phong_size LDXPhongS "
			"reflection LDXTRefl }\n");
		fprintf(m_pPovFile, "		#if (version >= 3.1) #else finish { "
			"refraction 1 ior LDXIoR } #end\n");
		fprintf(m_pPovFile, "#end\n");
		fprintf(m_pPovFile, "	}\n");
		fprintf(m_pPovFile, "#if (version >= 3.1) #if (LDXQual > 1)\n");
		fprintf(m_pPovFile, "	interior { ior LDXIoR }\n");
		fprintf(m_pPovFile, "#end #end\n");
		fprintf(m_pPovFile, "#if (version >= 3.1) } #end\n");
		fprintf(m_pPovFile, "#end\n");
		fprintf(m_pPovFile, "#end\n\n");
		m_macros.insert("LDXTransColor");
	}
}

void LDPovExporter::writeLDXChromeColor(void)
{
	if (m_macros.find("LDXChromeColor") == m_macros.end())
	{
		fprintf(m_pPovFile, "#ifndef (LDXSkipChromeColorMacro)\n");
		fprintf(m_pPovFile, "#macro LDXChromeColor(r, g, b)\n");
		fprintf(m_pPovFile, "#if (version >= 3.1) material { #end\n");
		fprintf(m_pPovFile, "	texture {\n");
		fprintf(m_pPovFile, "		pigment { rgbf <r,g,b,0> }\n");
		fprintf(m_pPovFile, "#if (LDXQual > 1)\n");
		fprintf(m_pPovFile, "		finish { ambient LDXAmb diffuse LDXDif }\n");
		fprintf(m_pPovFile, "		finish { phong LDXPhong phong_size LDXPhongS "
			"reflection LDXChromeRefl brilliance LDXChromeBril metallic specular "
			"LDXChromeSpec roughness LDXChromeRough}\n");
		fprintf(m_pPovFile, "#end\n");
		fprintf(m_pPovFile, "	}\n");
		fprintf(m_pPovFile, "#if (version >= 3.1) } #end\n");
		fprintf(m_pPovFile, "#end\n");
		fprintf(m_pPovFile, "#end\n\n");
		m_macros.insert("LDXChromeColor");
	}
}

void LDPovExporter::writeLDXRubberColor(void)
{
	if (m_macros.find("LDXRubberColor") == m_macros.end())
	{
		fprintf(m_pPovFile, "#ifndef (LDXSkipRubberColorMacro)\n");
		fprintf(m_pPovFile, "#macro LDXRubberColor(r, g, b)\n");
		fprintf(m_pPovFile, "#if (version >= 3.1) material { #end\n");
		fprintf(m_pPovFile, "	texture {\n");
		fprintf(m_pPovFile, "		pigment { rgbf <r,g,b,0> }\n");
		fprintf(m_pPovFile, "#if (LDXQual > 1)\n");
		fprintf(m_pPovFile, "		finish { ambient LDXAmb diffuse LDXDif }\n");
		fprintf(m_pPovFile, "		finish { phong LDXRubberPhong phong_size "
			"LDXRubberPhongS reflection LDXRubberRefl }\n");
		fprintf(m_pPovFile, "#end\n");
		fprintf(m_pPovFile, "	}\n");
		fprintf(m_pPovFile, "#if (version >= 3.1) } #end\n");
		fprintf(m_pPovFile, "#end\n");
		fprintf(m_pPovFile, "#end\n\n");
		m_macros.insert("LDXRubberColor");
	}
}

void LDPovExporter::writeColorDeclaration(int colorNumber)
{
	if (colorNumber != 16)
	{
		int r, g, b, a;
		LDLPalette *pPalette = m_pTopModel->getMainModel()->getPalette();
		LDLColorInfo colorInfo;
		PovColorMap::const_iterator it = m_xmlColors.end();
		bool wroteXml = false;

		pPalette->getRGBA(colorNumber, r, g, b, a);
		if (m_xmlMap)
		{
			it = m_xmlColors.find(colorNumber);
			if (it != m_xmlColors.end())
			{
				StringList::const_iterator itls;

				PovMapping color = it->second;
				for (itls = color.povCodes.begin();
					itls != color.povCodes.end(); ++itls)
				{
					writeCode(*itls);
					wroteXml = true;
				}
				for (itls = color.povFilenames.begin();
					itls != color.povFilenames.end(); ++itls)
				{
					writeInclude(*itls);
					wroteXml = true;
				}
			}
		}
		if (!wroteXml)
		{
			switch (getColorType(colorNumber))
			{
			case CTOpaque:
				writeLDXOpaqueColor();
				break;
			case CTTransparent:
				writeLDXTransColor();
				break;
			case CTChrome:
				writeLDXChromeColor();
				break;
			case CTRubber:
				writeLDXRubberColor();
				break;
			}
		}
		fprintf(m_pPovFile, "#ifndef (LDXColor%d)", colorNumber);
		colorInfo = pPalette->getAnyColorInfo(colorNumber);
		if (colorInfo.name[0])
		{
			fprintf(m_pPovFile, " // %s", colorInfo.name);
		}
		writeInnerColorDeclaration(colorNumber, false);
		if (it != m_xmlColors.end())
		{
			writeInnerColorDeclaration(colorNumber, true);
		}
		fprintf(m_pPovFile, "#end\n\n");
	}
}

// Note: static function
double LDPovExporter::alphaMod(int color)
{
	if (color > 127)
	{
		return (255.0 - ((255.0 - color) * .1)) / 255.0;
	}
	else
	{
		return color * 1.5 / 255.0;
		//return color / 255.0;
	}
}

void LDPovExporter::writeRGBA(int r, int g, int b, int a)
{
	const char *filter = "0";
	double dr, dg, db;

	if (a != 255)
	{
		filter = "LDXTFilt";
		dr = alphaMod(r);
		dg = alphaMod(g);
		db = alphaMod(b);
	}
	else
	{
		dr = r / 255.0;
		dg = g / 255.0;
		db = b / 255.0;
	}
	fprintf(m_pPovFile, "rgbf <%s,%s,%s,%s>", ftostr(dr).c_str(),
		ftostr(dg).c_str(), ftostr(db).c_str(), filter);
}

void LDPovExporter::writeCommentLine(
	LDLCommentLine *pCommentLine,
	bool &ifStarted,
	bool &elseStarted,
	bool &povMode)
{
	const char *comment = pCommentLine->getProcessedLine();

	if (stringHasCaseInsensitivePrefix(comment, "0 L3P IFPOV"))
	{
		if (ifStarted)
		{
			consolePrintf("Found 0 L3P IFPOV inside a 0 L3P IF%sPOV section; "
				"ignoring.\n", povMode ? "" : "NOT");
		}
		else
		{
			ifStarted = true;
			elseStarted = false;
			povMode = true;
			fprintf(m_pPovFile, "#if (LDXIPov)\n");
		}
	}
	else if (stringHasCaseInsensitivePrefix(comment, "0 L3P IFNOTPOV"))
	{
		if (ifStarted)
		{
			consolePrintf("Found 0 L3P IFNOTPOV inside a 0 L3P IF%sPOV section;"
				" ignoring.\n", povMode ? "" : "NOT");
		}
		else
		{
			ifStarted = true;
			elseStarted = false;
			povMode = false;
			fprintf(m_pPovFile, "#if (!LDXIPov)\n");
		}
	}
	else if (stringHasCaseInsensitivePrefix(comment, "0 L3P ELSEPOV"))
	{
		if (elseStarted)
		{
			consolePrintf("Found 0 L3P ELSEPOV inside a 0 L3P ELSEPOV section; "
				"ignoring.\n");
		}
		else if (ifStarted)
		{
			povMode = !povMode;
			elseStarted = true;
			fprintf(m_pPovFile, "#else\n");
		}
		else
		{
			consolePrintf("Found 0 L3P ELSEPOV without being in a 0 L3P IFPOV "
				"or 0 L3P IFNOTPOV section;\nignoring.\n");
		}
	}
	else if (stringHasCaseInsensitivePrefix(comment, "0 L3P ENDPOV"))
	{
		if (ifStarted)
		{
			ifStarted = false;
			fprintf(m_pPovFile, "#end\n");
		}
		else
		{
			consolePrintf("Found 0 L3P ENDPOV without being in a 0 L3P IFPOV or"
				" 0 L3P IFNOTPOV section;\nignoring.\n");
		}
	}
	else if (ifStarted)
	{
		if (povMode)
		{
			char *povLine = copyString(
				strchr(pCommentLine->getLine(), '0') + 1);

			if (povLine[0] == ' ')
			{
				memmove(povLine, &povLine[1], strlen(&povLine[1]) + 1);
			}
			stripTrailingWhitespace(povLine);
			fprintf(m_pPovFile, "%s\n", povLine);
			delete[] povLine;
		}
	}
	else if (strlen(comment) > 1)
	{
		// Note the processed line will always have a length of 1 on empty
		// comments.  No point outputting them.
		char *line = copyString(strchr(pCommentLine->getLine(), '0') + 1);

		stripLeadingWhitespace(line);
		stripTrailingWhitespace(line);
		fprintf(m_pPovFile, "// %s\n", line);
		delete[] line;
	}
}

bool LDPovExporter::isStud(LDLModel *pModel)
{
	if (pModel->isPrimitive())
	{
		const char *modelName = pModel->getName();

		if (modelName == NULL)
		{
			modelName = "";
		}
		if (stringHasCaseInsensitivePrefix(modelName, "stu"))
		{
			return true;
		}
	}
	return false;
}

bool LDPovExporter::writeModelLine(
	LDLModelLine *pModelLine,
	bool &studsStarted,
	bool mirrored,
	const TCFloat *matrix,
	bool inPart)
{
	LDLModel *pModel = pModelLine->getModel(true);
	TCFloat newModelMatrix[16] = { 0.0f };

	if (m_unmirrorStuds && pModelLine->getModel(true)->hasStuds())
	{
		TCVector::multMatrix(matrix, pModelLine->getMatrix(), newModelMatrix);
		if (TCVector::determinant(newModelMatrix) < 0.0f)
		{
			mirrored = true;
		}
		else
		{
			mirrored = false;
		}
	}
	else
	{
		mirrored = false;
	}
	if (pModel)
	{
		std::string declareName = getDeclareName(pModel, mirrored, inPart);
		PovElementMap::const_iterator it = m_xmlElements.end();

		if (m_emptyModels[declareName])
		{
			return false;
		}
		if (isStud(pModel))
		{
			startStuds(studsStarted);
		}
		else
		{
			endStuds(studsStarted);
		}
		if (m_xmlMap)
		{
			const std::string modelFilename = getModelFilename(pModel);
			std::string key = modelFilename;

			it = m_xmlElements.find(lowerCaseString(key));
			if (it != m_xmlElements.end() &&
				pModel->colorNumberIsTransparent(pModelLine->getColorNumber()))
			{
				const PovName *name = findPovName(it->second, "Alternate",
					"Clear");

				if (name != NULL)
				{
					declareName = name->name;
				}
			}
		}
		writeInnerModelLine(declareName, pModelLine, mirrored, false,
			studsStarted, inPart);
		if (it != m_xmlElements.end())
		{
			const PovName *name = findPovName(it->second, "Texture", "Slope");

			if (name != NULL)
			{
				writeInnerModelLine(name->name, pModelLine, mirrored, true,
					studsStarted, inPart);
			}
		}
	}
	return true;
}

void LDPovExporter::indentStud(bool studsStarted)
{
	if (studsStarted)
	{
		fprintf(m_pPovFile, "\t");
	}
}

void LDPovExporter::writeInnerModelLine(
	const std::string &declareName,
	LDLModelLine *pModelLine,
	bool mirrored,
	bool slope,
	bool studsStarted,
	bool inPart)
{
	LDLModel *pModel = pModelLine->getModel(true);
	bool origMirrored = mirrored;

	indentStud(studsStarted);
	fprintf(m_pPovFile, "\tobject {\n");
	indentStud(studsStarted);
	fprintf(m_pPovFile, "\t\t%s\n", declareName.c_str());
	indentStud(studsStarted);
	fprintf(m_pPovFile, "\t\t");
	if (writeXmlMatrix(getModelFilename(pModel).c_str()))
	{
		fprintf(m_pPovFile, "\n\t\t");
	}
	if (!inPart)
	{
		writeSeamMatrix(pModelLine);
	}
	if (origMirrored &&
		stringHasCaseInsensitiveSuffix(pModel->getFilename(), "stud.dat"))
	{
		float mirrorMatrix[] = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, -1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		};
		TCFloat newStudMatrix[16];

		TCVector::multMatrix(pModelLine->getMatrix(), mirrorMatrix,
			newStudMatrix);
		writeMatrix(newStudMatrix);
	}
	else
	{
		writeMatrix(pModelLine->getMatrix());
	}
	fprintf(m_pPovFile, "\n\t");
	indentStud(studsStarted);
	if (writeColor(pModelLine->getColorNumber(), slope))
	{
		fprintf(m_pPovFile, "\n");
		indentStud(studsStarted);
		fprintf(m_pPovFile, "\t");
	}
	fprintf(m_pPovFile, "}\n");
}

void LDPovExporter::endMesh()
{
	fprintf(m_pPovFile, "\t}\n");
}

void LDPovExporter::startStuds(bool &started)
{
	if (!started)
	{
		fprintf(m_pPovFile, "\t#if (LDXStuds)\n");
		started = true;
	}
}

void LDPovExporter::endStuds(bool &started)
{
	if (started)
	{
		fprintf(m_pPovFile, "\t#end // LDXStuds\n");
		started = false;
	}
}

void LDPovExporter::startMesh(void)
{
	fprintf(m_pPovFile, "\tmesh {\n");
}

void LDPovExporter::startMesh2(void)
{
	fprintf(m_pPovFile, "\tmesh2 {\n");
}

void LDPovExporter::startMesh2Section(const char *sectionName)
{
	fprintf(m_pPovFile, "\t\t%s {\n\t\t\t", sectionName);
}

void LDPovExporter::endMesh2Section()
{
	fprintf(m_pPovFile, "\n\t\t}\n");
}

void LDPovExporter::writeTriangleLine(LDLTriangleLine *pTriangleLine)
{
	writeTriangle(pTriangleLine->getPoints());
}

void LDPovExporter::writeQuadLine(LDLQuadLine *pQuadLine)
{
	writeTriangle(pQuadLine->getPoints());
	writeTriangle(pQuadLine->getPoints(), 4, 2);
}

void LDPovExporter::writeTriangleLineVertices(
	LDLTriangleLine *pTriangleLine,
	int &total)
{
	writeMesh2Vertices(pTriangleLine->getPoints(), 3, total);
}

void LDPovExporter::writeQuadLineIndices(
	LDLQuadLine * /*pQuadLine*/,
	int &current,
	int &total)
{
	writeMesh2Indices(current, current + 1, current + 2, total);
	writeMesh2Indices(current, current + 2, current + 3, total);
	current += 4;
}

void LDPovExporter::writeTriangleLineIndices(
	LDLTriangleLine * /*pTriangleLine*/,
	int &current,
	int &total)
{
	writeMesh2Indices(current, current + 1, current + 2, total);
	current += 3;
}

void LDPovExporter::writeQuadLineVertices(LDLQuadLine *pQuadLine, int &total)
{
	writeMesh2Vertices(pQuadLine->getPoints(), 4, total);
}

void LDPovExporter::writeEdgeColor(void)
{
	fprintf(m_pPovFile,
		"#ifndef (EdgeColor)\n"
		"#declare EdgeColor = material {\n"
		"	texture {\n"
		"		pigment { rgbf <LDXEdgeR,LDXEdgeG,LDXEdgeB,0> }\n"
		"		finish { ambient 1 diffuse 0 }\n"
		"	}\n"
		"}\n"
		"#end\n\n");
}

void LDPovExporter::writeEdgeLineMacro(void)
{
	fprintf(m_pPovFile,
		"#macro EdgeLine(Point1, Point2, Color)\n"
		"object {\n"
		"	#if (Point1.x != Point2.x | Point1.y != Point2.y | Point1.z != Point2.z)\n"
		"		merge {\n"
		"			cylinder {\n"
		"				Point1,Point2,LDXEdgeRad\n"
		"			}\n"
		"			sphere {\n"
		"				Point1,LDXEdgeRad\n"
		"			}\n"
		"			sphere {\n"
		"				Point2,LDXEdgeRad\n"
		"		}\n"
		"	#else\n"
		"		sphere {\n"
		"			Point1,LDXEdgeRad\n"
		"		}\n"
		"	#end\n"
		"	}\n"
		"	material { Color }\n"
		"	no_shadow\n"
		"}\n"
		"#end\n\n");
}

void LDPovExporter::writeTriangle(
	const TCVector *points,
	int size /*= -1*/,
	int start /*= 0*/)
{
	fprintf(m_pPovFile, "\t\ttriangle { ");
	writePoints(points, 3, size, start);
	fprintf(m_pPovFile, " }\n");
}

void LDPovExporter::writeMesh2Indices(int i0, int i1, int i2, int &total)
{
	if (total > 0)
	{
		fprintf(m_pPovFile, ",");
		if (total % 4 == 0)
		{
			fprintf(m_pPovFile, "\n\t\t\t");
		}
		else
		{
			fprintf(m_pPovFile, " ");
		}
	}
	total++;
	fprintf(m_pPovFile, "<%d, %d, %d>", i0, i1, i2);
}

void LDPovExporter::writeMesh2Vertices(
	const TCVector *pVertices,
	size_t count,
	int &total)
{
	for (size_t i = 0; i < count; i++)
	{
		if (total > 0)
		{
			fprintf(m_pPovFile, ",");
			if (total % 4 == 0)
			{
				fprintf(m_pPovFile, "\n\t\t\t");
			}
			else
			{
				fprintf(m_pPovFile, " ");
			}
		}
		total++;
		writePoint(pVertices[i]);
	}
}

void LDPovExporter::writePoints(
	const TCVector *points,
	int count,
	int size /*= -1*/,
	int start /*= 0*/)
{
	if (size == -1)
	{
		size = count;
	}
	for (int i = 0; i < count; i++)
	{
		if (i > 0)
		{
			fprintf(m_pPovFile, " ");
		}
		writePoint(points[(i + start) % size]);
	}
}

void LDPovExporter::writePoint(const TCVector &point)
{
	fprintf(m_pPovFile, "<%s,%s,%s>", ftostr(point[0]).c_str(),
		ftostr(point[1]).c_str(), ftostr(point[2]).c_str());
}

void LDPovExporter::scanEdgePoint(
	const TCVector &point,
	const LDLFileLine *pFileLine)
{
	if (pFileLine->getLineType() == LDLLineTypeLine)
	{
		m_edgePoints.push_back(point);
	}
	else if (pFileLine->getLineType() == LDLLineTypeConditionalLine)
	{
		if (m_conditionalEdges)
		{
			m_condEdgePoints.push_back(point);
		}
	}
}

// NOTE: static function
const char *LDPovExporter::get48Prefix(bool is48)
{
	if (is48)
	{
		return "48/";
	}
	else
	{
		return "";
	}
}

bool LDPovExporter::writeRoundClipRegion(TCFloat fraction, bool closeOff)
{
	// If fraction is 1.0, this function just returns true.  If fraction is
	// greater than 1.0, it just returns false.  Otherwise, it outputs a
	// clipped_by statement that includes 1 or two planes, either in union
	// (explicitly) or in intersection (implicit to the clipped_by statement).
	// Note that planes are specified with a surface normal, and a distance
	// from the origin along that surface normal.  The surface normal indicates
	// the direction of the "top" side of the plane, and the plane isn't really
	// a plane in the since of a 2D slice.  It's a solid object, and everything
	// "under" it is considered part of it.
	if (fraction > 1.0f)
	{
		return false;
	}
	else if (fEq(fraction, 0.5f))
	{
		// We want half, so spit out a plane with a surface normal pointing
		// along the negative Z axis, such that the plane's Z coordinates are
		// all 0.
		fprintf(m_pPovFile,
			"	clipped_by\n"
			"	{\n"
			"		plane\n"
			"		{\n"
			"			<0,0,-1>,0\n"
			"		}\n");
		if (closeOff)
		{
			fprintf(m_pPovFile, "	}\n");
		}
	}
	else if (fEq(fraction, 1.0f))
	{
		if (!closeOff)
		{
			fprintf(m_pPovFile,
				"	clipped_by\n"
				"	{\n");
		}
	}
	else
	{
		double angle = 2.0 * M_PI * fraction + M_PI / 2.0;
		double x = cos(angle);
		double z = sin(angle);

		cleanupDoubles(&x, 1);
		cleanupDoubles(&z, 1);
		if (fraction < 0.5f)
		{
			// Spit out two planes.  The clipped_by statement automatically
			// takes the CSG intersection of the two.  The first plane restricts
			// us to Z >= 0 (just like the fraction=0.5 case above).  The second
			// plane uses fraction to determine the angle needed for the proper
			// pie slice.
			fprintf(m_pPovFile,
				"	clipped_by\n"
				"	{\n"
				"		plane\n"
				"		{\n"
				"			<0,0,-1>,0\n"
				"		}\n"
				"		plane\n"
				"		{\n"
				"			<%s,0,%s>,0\n"
				"		}\n", ftostr(x, 20).c_str(), ftostr(z, 20).c_str());
		}
		else
		{
			// Spit out a union of two planes.  The first plane makes it so that
			// everything with Z >= 0 is kept, so the first half of the circular
			// item is all present.  The second plane restricts the remaining
			// portion using fraction to determine the proper angle.
			fprintf(m_pPovFile,
				"	clipped_by\n"
				"	{\n"
				"		union\n"
				"		{\n"
				"			plane\n"
				"			{\n"
				"				<0,0,-1>,0\n"
				"			}\n"
				"			plane\n"
				"			{\n"
				"				<%s,0,%s>,0\n"
				"			}\n"
				"		}\n", ftostr(x, 20).c_str(), ftostr(z, 20).c_str());
		}
		if (closeOff)
		{
			fprintf(m_pPovFile, "	}\n");
		}
	}
	return true;
}

std::string LDPovExporter::getPrimName(
	const std::string &base,
	bool is48,
	bool inPart,
	int num /*= -1*/,
	int den /*= -1*/)
{
	const char *prefix48 = get48Prefix(is48);
	char buf[1024];

	if (num > 0 && den > 0)
	{
		sprintf(buf, "%s%d-%d%s.dat", prefix48, num, den, base.c_str());
	}
	else if (num > 0)
	{
		sprintf(buf, "%s%s%d.dat", prefix48, base.c_str(), num);
	}
	else
	{
		sprintf(buf, "%s%s.dat", prefix48, base.c_str());
	}
	return getDeclareName(buf, false, inPart);
}

bool LDPovExporter::writePrimitive(const char *format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	const char *primName = va_arg(argPtr, const char *);
	if (!m_writtenModels.insert(primName).second)
	{
		va_end(argPtr);
		return false;
	}
	va_end(argPtr);
	va_start(argPtr, format);
	vfprintf(m_pPovFile, format, argPtr);
	va_end(argPtr);
	return true;
}

bool LDPovExporter::substituteEighthSphere(
	bool /*bfc*/,
	bool is48 /*= false*/)
{
	const char *format =
		"#declare %s = sphere // Sphere .125\n"
		"{\n"
		"	<0,0,0>,1\n"
		"	clipped_by\n"
		"	{\n"
		"		box\n"
		"		{\n"
		"			<0,0,0>,<2,2,2>\n"
		"		}\n"
		"	}\n"
		"}\n\n";
	if (m_primSubCheck)
	{
		return true;
	}
	writePrimitive(format, getPrimName("1-8sphe", is48, true).c_str());
	writePrimitive(format, getPrimName("1-8sphe", is48, false).c_str());
	return true;
}

bool LDPovExporter::substituteEighthSphereCorner(
	bool /*bfc*/,
	bool is48 /*= false*/)
{
	const char *format =
		"#declare %s = sphere // Sphere Corner .125\n"
		"{\n"
		"	<0,0,0>,sqrt(2)\n"
		"	clipped_by\n"
		"	{\n"
		"		box\n"
		"		{\n"
		"			<0,0,0>,<1,1,1>\n"
		"		}\n"
		"	}\n"
		"}\n\n";
	if (m_primSubCheck)
	{
		return true;
	}
	writePrimitive(format, getPrimName("1-8sphc", is48, true).c_str());
	writePrimitive(format, getPrimName("1-8sphc", is48, false).c_str());
	return true;
}

bool LDPovExporter::substituteCylinder(
	TCFloat fraction,
	bool /*bfc*/,
	bool inPart,
	bool is48)
{
	if (m_primSubCheck)
	{
		return true;
	}
	if (writePrimitive(
		"#declare %s = cylinder // Cylinder %s\n"
		"{\n"
		"	<0,0,0>,<0,1,0>,1 open\n",
		getPrimName("cyli", is48, inPart, m_filenameNumerator,
		m_filenameDenom).c_str(), ftostr(fraction).c_str()))
	{
		writeRoundClipRegion(fraction);
		fprintf(m_pPovFile, "}\n\n");
	}
	return true;
}

bool LDPovExporter::substituteCylinder(
	TCFloat fraction,
	bool bfc,
	bool is48 /*= false*/)
{
	substituteCylinder(fraction, bfc, true, is48);
	return substituteCylinder(fraction, bfc, false, is48);
}

bool LDPovExporter::substituteSlopedCylinder(
	TCFloat fraction,
	bool bfc,
	bool is48 /*= false*/)
{
	substituteSlopedCylinder(fraction, bfc, true, is48);
	return substituteSlopedCylinder(fraction, bfc, false, is48);
}

bool LDPovExporter::substituteSlopedCylinder(
	TCFloat fraction,
	bool /*bfc*/,
	bool inPart,
	bool is48)
{
	if (m_primSubCheck)
	{
		return true;
	}
	if (writePrimitive(
		"#declare %s = cylinder // Sloped Cylinder %s\n"
		"{\n"
		"	<0,0,0>,<0,2,0>,1 open\n",
		getPrimName("cyls", is48, inPart, m_filenameNumerator,
		m_filenameDenom).c_str(), ftostr(fraction).c_str()))
	{
		writeRoundClipRegion(fraction, false);
		fprintf(m_pPovFile,
			"		plane\n"
			"		{\n"
			"			<1,1,0>,%s\n"
			"		}\n"
			"	}\n", ftostr(sqrt(2.0) / 2.0, 20).c_str());
		fprintf(m_pPovFile, "}\n\n");
	}
	return true;
}

bool LDPovExporter::substituteSlopedCylinder2(
	TCFloat fraction,
	bool bfc,
	bool is48 /*= false*/)
{
	substituteSlopedCylinder2(fraction, bfc, true, is48);
	return substituteSlopedCylinder2(fraction, bfc, false, is48);
}

bool LDPovExporter::substituteSlopedCylinder2(
	TCFloat fraction,
	bool /*bfc*/,
	bool inPart,
	bool is48)
{
	double angle = 2.0 * M_PI * fraction + M_PI;
	double x = cos(angle);
	double z = sin(angle);

	cleanupDoubles(&x, 1);
	cleanupDoubles(&z, 1);
	if (!fEq(fraction, 0.25) && fraction > 0.25)
	{
		return false;
	}
	if (m_primSubCheck)
	{
		return true;
	}
	if (writePrimitive(
		"#declare %s = cylinder // Sloped Cylinder2 %s\n"
		"{\n"
		"	<0,0,0>,<0,1,0>,1 open\n",
		getPrimName("cyls2", is48, inPart, m_filenameNumerator,
		m_filenameDenom).c_str(), ftostr(fraction).c_str()))
	{
		fprintf(m_pPovFile,
			"	clipped_by\n"
			"	{\n"
			"		plane\n"
			"		{\n"
			"			<1,0,0>,0\n"
			"		}\n"
			"		plane\n"
			"		{\n"
			"			<%s,0,%s>,0\n"
			"		}\n"
			"		plane\n"
			"		{\n"
			"			<1,1,0>,0\n"
			"		}\n"
			"	}\n", ftostr(x, 20).c_str(), ftostr(z, 20).c_str());
		fprintf(m_pPovFile, "}\n\n");
	}
	return true;
}

bool LDPovExporter::substituteDisc(
	TCFloat fraction,
	bool bfc,
	bool is48 /*= false*/)
{
	substituteDisc(fraction, bfc, true, is48);
	return substituteDisc(fraction, bfc, false, is48);
}

bool LDPovExporter::substituteDisc(
	TCFloat fraction,
	bool /*bfc*/,
	bool inPart,
	bool is48)
{
	if (m_primSubCheck)
	{
		return true;
	}
	if (writePrimitive(
		"#declare %s = disc // Disc %s\n"
		"{\n"
		"	<0,0,0>,<0,1,0>,1\n",
		getPrimName("disc", is48, inPart, m_filenameNumerator,
		m_filenameDenom).c_str(), ftostr(fraction).c_str()))
	{
		writeRoundClipRegion(fraction);
		fprintf(m_pPovFile, "}\n\n");
	}
	return true;
}

bool LDPovExporter::substituteChrd(
	TCFloat fraction,
	bool bfc,
	bool is48 /*= false*/)
{
	substituteChrd(fraction, bfc, true, is48);
	return substituteChrd(fraction, bfc, false, is48);
}

bool LDPovExporter::substituteChrd(
	TCFloat fraction,
	bool /*bfc*/,
	bool inPart,
	bool is48)
{
	if (m_primSubCheck)
	{
		return true;
	}
	double angle = 2.0 * M_PI * fraction;
	double cosAngle = cos(angle);
	double sinAngle = sin(angle);
	TCVector p0;
	TCVector p1(1.0f, 0.0f, 0.0f);
	TCVector p2((TCFloat)cosAngle, 0.0f, (TCFloat)sinAngle);
	double x = -sinAngle;
	double z = cosAngle - 1.0;
	double ofs = -p0.distToLine(p1, p2);

	writePrimitive(
		"#declare %s = disc // Disc %s\n"
		"{\n"
		"	<0,0,0>,<0,1,0>,1\n"
		"	clipped_by\n"
		"	{\n"
		"		plane\n"
		"		{\n"
		"			<%s,0,%s>,%s\n"
		"		}\n"
		"	}\n"
		"}\n\n",
		getPrimName("chrd", is48, inPart, m_filenameNumerator,
		m_filenameDenom).c_str(), ftostr(fraction).c_str(),
		ftostr(x, 20).c_str(), ftostr(z, 20).c_str(), ftostr(ofs, 20).c_str());
	return true;
}

bool LDPovExporter::substituteNotDisc(
	TCFloat fraction,
	bool bfc,
	bool is48 /*= false*/)
{
	substituteNotDisc(fraction, bfc, true, is48);
	return substituteNotDisc(fraction, bfc, false, is48);
}

bool LDPovExporter::substituteNotDisc(
	TCFloat fraction,
	bool /*bfc*/,
	bool inPart,
	bool is48)
{
	if (m_primSubCheck)
	{
		return true;
	}
	if (writePrimitive(
		"#declare %s = disc // Not-Disc %s\n"
		"{\n"
		"	<0,0,0>,<0,1,0>,2,1\n",
		getPrimName("ndis", is48, inPart, m_filenameNumerator,
		m_filenameDenom).c_str(), ftostr(fraction).c_str()))
	{
		writeRoundClipRegion(fraction, false);
		fprintf(m_pPovFile,
			"		box\n"
			"		{\n"
			"			<-1,-1,-1>,<1,1,1>\n"
			"		}\n"
			"	}\n");
		fprintf(m_pPovFile, "}\n\n");
	}
	return true;
}

bool LDPovExporter::substituteTangent(
	TCFloat fraction,
	bool bfc,
	bool is48 /*= false*/)
{
	substituteTangent(fraction, bfc, true, is48);
	return substituteTangent(fraction, bfc, false, is48);
}

bool LDPovExporter::substituteTangent(
	TCFloat fraction,
	bool /*bfc*/,
	bool inPart,
	bool is48)
{
	if (m_primSubCheck)
	{
		return true;
	}
	if (writePrimitive(
		"#declare %s = disc // Tangent %s\n"
		"{\n"
		"	<0,0,0>,<0,1,0>,2,1\n",
		getPrimName("tang", is48, inPart, m_filenameNumerator,
		m_filenameDenom).c_str(), ftostr(fraction).c_str()))
	{
		writeRoundClipRegion(fraction, false);
		fprintf(m_pPovFile,
			"		prism\n"
			"		{\n"
			"			linear_spline\n"
			"			1,-1,16,\n"
			"			<1,0.1989>,<0.8478,0.5665>,<0.5665,0.8478>,<0.1989,1>,\n"
			"			<-0.1989,1>,<-0.5665,0.8478>,<-0.8478,0.5665>,<-1,0.1989>,\n"
			"			<-1,-0.1989>,<-0.8478,-0.5665>,<-0.5665,-0.8478>,<-0.1989,-1>,\n"
			"			<0.1989,-1>,<0.5665,-0.8478>,<0.8478,-0.5665>,<1,-0.1989>\n"
			"		}\n"
			"	}\n");
		fprintf(m_pPovFile, "}\n\n");
	}
	return true;
}

bool LDPovExporter::substituteCone(
	TCFloat fraction,
	int size,
	bool bfc,
	bool is48 /*= false*/)
{
	substituteCone(fraction, size, bfc, true, is48);
	return substituteCone(fraction, size, bfc, false, is48);
}

bool LDPovExporter::substituteCone(
	TCFloat fraction,
	int size,
	bool /*bfc*/,
	bool inPart,
	bool is48)
{
	if (m_primSubCheck)
	{
		return true;
	}
	std::string base = "con";

	base += ltostr(size);
	if (writePrimitive(
		"#declare %s = cone // Cone %s\n"
		"{\n"
		"	<0,0,0>,%d,<0,1,0>,%d open\n",
		getPrimName(base, is48, inPart, m_filenameNumerator,
		m_filenameDenom).c_str(), ftostr(fraction).c_str(), size + 1, size))
	{
		writeRoundClipRegion(fraction);
		fprintf(m_pPovFile, "}\n\n");
	}
	return true;
}

bool LDPovExporter::substituteRing(
	TCFloat fraction,
	int size,
	bool bfc,
	bool is48 /*= false*/,
	bool isOld /*= false*/)
{
	substituteRing(fraction, size, bfc, true, is48, isOld);
	return substituteRing(fraction, size, bfc, false, is48, isOld);
}

bool LDPovExporter::substituteRing(
	TCFloat fraction,
	int size,
	bool /*bfc*/,
	bool inPart,
	bool is48,
	bool isOld)
{
	if (m_primSubCheck)
	{
		return true;
	}
	bool bWrote = false;
	if (isOld)
	{
		bWrote = writePrimitive(
			"#declare %s = disc // Ring %s\n",
			getPrimName("ring", is48, inPart, size).c_str(),
			ftostr(fraction).c_str());
	}
	else
	{
		std::string base = "ri";

		if (size < 10 || m_filenameDenom < 10)
		{
			base += "n";
			if (size < 10 && m_filenameDenom < 10)
			{
				base += "g";
			}
		}
		base += ltostr(size);
		bWrote = writePrimitive(
			"#declare %s = disc // Ring %s\n",
			getPrimName(base, is48, inPart, m_filenameNumerator,
			m_filenameDenom).c_str(), ftostr(fraction).c_str());
	}
	if (bWrote)
	{
		fprintf(m_pPovFile,
			"{\n"
			"	<0,0,0>,<0,1,0>,%d,%d\n", size + 1, size);
		writeRoundClipRegion(fraction);
		fprintf(m_pPovFile, "}\n\n");
	}
	return true;
}

bool LDPovExporter::substituteTorusIO(
	bool inner,
	TCFloat fraction,
	int size,
	bool bfc,
	bool isMixed,
	bool is48 /*= false*/)
{
	substituteTorusIO(inner, fraction, size, bfc, true, isMixed, is48);
	return substituteTorusIO(inner, fraction, size, bfc, false, isMixed, is48);
}

bool LDPovExporter::substituteTorusIO(
	bool inner,
	TCFloat fraction,
	int size,
	bool /*bfc*/,
	bool inPart,
	bool /*isMixed*/,
	bool /*is48*/)
{
	if (m_primSubCheck)
	{
		return true;
	}

	fprintf(m_pPovFile,
		"#declare %s = torus // Torus %s\n"
		"{\n"
		"	1,%s\n",
		getDeclareName(m_modelName, false, inPart).c_str(),
		ftostr(fraction).c_str(), ftostr(getTorusFraction(size), 20).c_str());
	writeRoundClipRegion(fraction, false);
	if (inner)
	{
		fprintf(m_pPovFile,
			"		cylinder\n"
			"		{\n"
			"			<0,0,0>,<0,1,0>,1"
			"		}\n");
	}
	else
	{
		fprintf(m_pPovFile,
			"		difference\n"
			"		{\n"
			"			cylinder\n"
			"			{\n"
			"				<0,0,0>,<0,1,0>,2"
			"			}\n"
			"			cylinder\n"
			"			{\n"
			"				<0,0,0>,<0,1,0>,1"
			"			}\n"
			"		}\n");
	}
	fprintf(m_pPovFile,
		"	}\n"
		"}\n\n");
	return true;
}

bool LDPovExporter::substituteTorusQ(
	TCFloat fraction,
	int size,
	bool bfc,
	bool isMixed,
	bool is48 /*= false*/)
{
	substituteTorusQ(fraction, size, bfc, true, isMixed, is48);
	return substituteTorusQ(fraction, size, bfc, false, isMixed, is48);
}

bool LDPovExporter::substituteTorusQ(
	TCFloat fraction,
	int size,
	bool /*bfc*/,
	bool inPart,
	bool /*isMixed*/,
	bool /*is48*/)
{
	if (m_primSubCheck)
	{
		return true;
	}

	fprintf(m_pPovFile,
		"#declare %s = torus // Torus %s\n"
		"{\n"
		"	1,%s\n",
		getDeclareName(m_modelName, false, inPart).c_str(),
		ftostr(fraction).c_str(), ftostr(getTorusFraction(size), 20).c_str());
	writeRoundClipRegion(fraction);
	fprintf(m_pPovFile, "}\n\n");
	return true;
}

void LDPovExporter::writeLogo(void)
{
	writePrimitive(
		"#declare LDXLegoSpace = 49.5;\n"
		"#declare %s =\n"
		"union {\n"
		"	merge {\n"
		"		// L\n"
		"		sphere {<-88,0,0>,4}\n"
		"		cylinder {<-88,0,0>,<0,0,0>,4 open}\n"
		"		sphere {<0,0,0>,4}\n"
		"		cylinder {<0,0,0>,<0,0,25>,4 open}\n"
		"		sphere {<0,0,25>,4}\n"
		"		clipped_by {plane{y,0}}\n"
		"	}\n"
		"\n"
		"	merge {\n"
		"		// E\n"
		"		sphere {<-88,0,25>,4}\n"
		"		cylinder {<-88,0,25>,<-88,0,0>,4 open}\n"
		"		sphere {<-88,0,0>,4}\n"
		"		cylinder {<-88,0,0>,<0,0,0>,4 open}\n"
		"		sphere {<0,0,0>,4}\n"
		"		cylinder {<0,0,0>,<0,0,25>,4 open}\n"
		"		sphere {<0,0,25>,4}\n"
		"		sphere {<-44,0,17>,4}\n"
		"		cylinder {<-44,0,0>,<-44,0,17>,4 open}\n"
		"		clipped_by {plane{y,0}}\n"
		"		translate <0,0,LDXLegoSpace>\n"
		"	}\n"
		"\n"
		"	merge {\n"
		"		// G\n"
		"		sphere {<-74,0,25>,4}\n"
		"		torus {12.5,4 clipped_by{plane{x,0}} translate<-74,0,12.5>}\n"
		"		cylinder {<-74,0,0>,<-14,0,0>,4 open}\n"
		"		torus {12.5,4 clipped_by{plane{-x,0}} translate<-14,0,12.5>}\n"
		"		cylinder {<-14,0,25>,<-44,0,25>,4 open}\n"
		"		sphere {<-44,0,25>,4}\n"
		"		cylinder {<-44,0,25>,<-44,0,17>,4 open}\n"
		"		sphere {<-44,0,17>,4}\n"
		"		clipped_by {plane{y,0}}\n"
		"		translate <0,0,LDXLegoSpace*2>\n"
		"	}\n"
		"\n"
		"	merge {\n"
		"		// O\n"
		"		torus {12.5,4 clipped_by{plane{x,0}} translate<-74,0,12.5>}\n"
		"		cylinder {<-74,0,0>,<-14,0,0>,4 open}\n"
		"		torus {12.5,4 clipped_by{plane{-x,0}} translate<-14,0,12.5>}\n"
		"		cylinder {<-14,0,25>,<-74,0,25>,4 open}\n"
		"		clipped_by {plane{y,0}}\n"
		"		translate <0,0,LDXLegoSpace*3>\n"
		"	}\n"
		"\n"
		"	matrix <1,0,-0.22,0,1,0,0,0,1,0,0,0>\n"
		"	scale .045\n"
		"	translate <1.85,-4,-4.45>\n"
		"}\n\n", "LDXStudLogo");
}

bool LDPovExporter::substituteStud(void)
{
	if (m_xmlElements.find("stud.dat") == m_xmlElements.end())
	{
		substituteStudInPart(true);
		return substituteStudInPart(false);
	}
	else
	{
		return false;
	}
}

bool LDPovExporter::substituteStudInPart(bool inPart)
{
	if (m_primSubCheck)
	{
		return true;
	}
	writeLogo();
	writePrimitive(
			"#declare %s =\n"
			"#if (LDXQual <= 2)\n"
			"cylinder { <0,0,0>, <0,-4,0>, 6 }\n"
			"#else\n"
			"union {\n"
			"	cylinder { <0,0,0>, <0,-4,0>, 6 }\n"
			"	object { LDXStudLogo }\n"
			"}\n"
			"#end\n"
			"\n", getPrimName("stud", false, inPart).c_str());
	return true;
}
