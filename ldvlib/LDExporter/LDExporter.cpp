#include "LDExporter.h"
#include "LDPovExporter.h"
#include <TCFoundation/TCUserDefaults.h>
#include <TCFoundation/mystring.h>
#include <TCFoundation/TCLocalStrings.h>
#include <TCFoundation/TCStringArray.h>
#include <TCFoundation/TCMacros.h>
#include <LDLoader/LDLMainModel.h>
#include <LDLoader/LDLAutoCamera.h>

LDExporter::LDExporter(const char *udPrefix /*= ""*/):
m_width(4.0f),
m_height(3.0f),
m_fov(25.0f),
m_xPan(0.0f),
m_yPan(0.0f),
m_radius(0.0f),
m_backgroundR(0.0f),
m_backgroundG(0.0f),
m_backgroundB(0.0f),
m_backgroundA(1.0f),
m_udPrefix(udPrefix),
m_appName("LDExporter"),
m_settingsInitialized(false)
{
	TCFloat flipMatrix[16];
	TCFloat tempMatrix[16];
	char buildDate[5] =
	{
		__DATE__[7],
		__DATE__[8],
		__DATE__[9],
		__DATE__[10],
		0
	};

	m_appCopyright = "Copyright (C) ";
	m_appCopyright += buildDate;
	m_appCopyright += " Travis Cobbs, Peter Bartfai, Trevor Sandy";
	TCVector::initIdentityMatrix(flipMatrix);
	// The following flips around to LDraw coordinate system
	flipMatrix[5] = -1.0f;
	flipMatrix[10] = -1.0f;
	// The following is for 2/3 view (30 degrees latitude, 45 degrees longitude)
	tempMatrix[0] = (TCFloat)(sqrt(2.0) / 2.0);
	tempMatrix[1] = (TCFloat)(sqrt(2.0) / 4.0);
	tempMatrix[2] = (TCFloat)(-sqrt(1.5) / 2.0);
	tempMatrix[3] = 0.0f;
	tempMatrix[4] = 0.0f;
	tempMatrix[5] = (TCFloat)(sin(M_PI / 3.0));
	tempMatrix[6] = 0.5f;
	tempMatrix[7] = 0.0f;
	tempMatrix[8] = (TCFloat)(sqrt(2.0) / 2.0);
	tempMatrix[9] = (TCFloat)(-sqrt(2.0) / 4.0);
	tempMatrix[10] = (TCFloat)(sqrt(1.5) / 2.0);
	tempMatrix[11] = 0.0f;
	tempMatrix[12] = 0.0f;
	tempMatrix[13] = 0.0f;
	tempMatrix[14] = 0.0f;
	tempMatrix[15] = 1.0f;
	TCVector::multMatrix(flipMatrix, tempMatrix, m_rotationMatrix);
	loadSettings();
}

LDExporter::~LDExporter(void)
{
}

void LDExporter::setBackgroundColor(
	TCFloat r,
	TCFloat g,
	TCFloat b,
	TCFloat a /*= 1.0f*/)
{
	m_backgroundR = r;
	m_backgroundG = g;
	m_backgroundB = b;
	m_backgroundA = a;
}

void LDExporter::loadSettings(void)
{
	m_primSub = boolForKey("PrimitiveSubstitution", true);
	m_seamWidth = floatForKey("SeamWidth", 0.5);
	m_edges = boolForKey("Edges", false);
	m_conditionalEdges = boolForKey("ConditionalEdges", false);
}

std::string LDExporter::getExtension(void) const
{
	consolePrintf("Programmer error: LDExporter::getExtension called.\n");
	return "";
}

ucstring LDExporter::getTypeDescription(void) const
{
	consolePrintf("Programmer error: LDExporter::getTypeDescription called.\n");
	return _UC("");
}

const LDExporterSettingList &LDExporter::getSettings(void) const
{
	return const_cast<LDExporter *>(this)->getSettings();
}

LDExporterSettingList &LDExporter::getSettings(void)
{
	if (!m_settingsInitialized)
	{
		initSettings();
		m_settingsInitialized = true;
	}
	return m_settings;
}

void LDExporter::addPrimSubSetting(LDExporterSetting *pGroup) const
{
	addSetting(pGroup, LDExporterSetting(ls(_UC("LDXPrimSub")), m_primSub,
		udKey("PrimitiveSubstitution").c_str()));
}

void LDExporter::addSeamWidthSetting(LDExporterSetting *pGroup) const
{
	addSetting(pGroup, ls(_UC("LDXSeamWidth")), m_seamWidth,
		udKey("SeamWidth").c_str(), 0.0f, 5.0f);
}

LDExporterSetting *LDExporter::addEdgesSettings(LDExporterSetting *pGroup) const
{
	LDExporterSetting *pEdgesGroup = addEdgesSetting(pGroup);
	addConditionalEdgesSetting(pEdgesGroup);
	return pEdgesGroup;
}

LDExporterSetting *LDExporter::addEdgesSetting(LDExporterSetting *pGroup) const
{
	if (addSetting(pGroup, LDExporterSetting(ls(_UC("LDXEdges")), m_edges,
		udKey("Edges").c_str())))
	{
		return &m_settings.back();
	}
	else
	{
		return NULL;
	}
}

void LDExporter::addConditionalEdgesSetting(LDExporterSetting *pGroup) const
{
	addSetting(pGroup, LDExporterSetting(ls(_UC("LDXConditionalEdges")),
		m_conditionalEdges, udKey("ConditionalEdges").c_str()));
}

LDExporterSetting *LDExporter::addGeometrySettings(void) const
{
	LDExporterSetting *pGroup = addSettingGroup(ls(_UC("LDXGeometry")));
	if (pGroup == NULL)
	{
		return NULL;
	}
	addPrimSubSetting(pGroup);
	addSeamWidthSetting(pGroup);
	addEdgesSettings(pGroup);
	return pGroup;
}

void LDExporter::initSettings(void) const
{
	addGeometrySettings();
}

LDExporterSetting *LDExporter::addSettingGroup(CUCSTR name) const
{
	if (addSetting(LDExporterSetting(name, 0)))
	{
		return &m_settings.back();
	}
	else
	{
		return NULL;
	}
}

bool LDExporter::addSetting(
	LDExporterSetting *pGroup,
	CUCSTR name,
	TCFloat value,
	const char *key,
	TCFloat min,
	TCFloat max) const
{
	if (addSetting(name, value, key, min, max))
	{
		pGroup->setGroupSize(pGroup->getGroupSize() + 1);
		return true;
	}
	else
	{
		return false;
	}
}

bool LDExporter::addSetting(
	LDExporterSetting *pGroup,
	CUCSTR name,
	long value,
	const char *key,
	long min,
	long max) const
{
	if (addSetting(name, value, key, min, max))
	{
		pGroup->setGroupSize(pGroup->getGroupSize() + 1);
		return true;
	}
	else
	{
		return false;
	}
}

bool LDExporter::addSetting(
	LDExporterSetting *pGroup,
	const LDExporterSetting &setting) const
{
	if (addSetting(setting))
	{
		pGroup->setGroupSize(pGroup->getGroupSize() + 1);
		return true;
	}
	else
	{
		return false;
	}
}

bool LDExporter::addSetting(
	CUCSTR name,
	TCFloat value,
	const char *key,
	TCFloat min,
	TCFloat max) const
{
	if (addSetting(LDExporterSetting(name, value, key)))
	{
		LDExporterSetting &setting = m_settings.back();

		setting.setMinValue(min);
		setting.setMaxValue(max);
		return true;
	}
	return false;
}

bool LDExporter::addSetting(
	CUCSTR name,
	long value,
	const char *key,
	long min,
	long max) const
{
	if (addSetting(LDExporterSetting(name, value, key)))
	{
		LDExporterSetting &setting = m_settings.back();

		setting.setMinValue(min);
		setting.setMaxValue(max);
		return true;
	}
	return false;
}

bool LDExporter::addSetting(const LDExporterSetting &setting) const
{
	m_settings.push_back(setting);
	return true;
}

void LDExporter::dealloc(void)
{
	LDLPrimitiveCheck::dealloc();
}

// Note: static method.
int LDExporter::run(void)
{
	LDExporter exporter;
	return exporter.runInternal();
}

int LDExporter::doExport(LDLModel * /*pTopModel*/)
{
	consolePrintf("Programmer error: LDExporter::doExport called.\n");
	return 1;
}

int LDExporter::doExport(TREModel * /*pTopModel*/)
{
	consolePrintf("Programmer error: LDExporter::doExport called.\n");
	return 1;
}

int LDExporter::doExport(
	LDLModel * /*pTopLdlModel*/,
	TREModel * /*pTopTreModel*/)
{
	consolePrintf("Programmer error: LDExporter::doExport called.\n");
	return 1;
}

TCFloat LDExporter::getHFov(void)
{
	if (m_width > m_height)
	{
		return (TCFloat)(2.0 * rad2deg(atan(tan(deg2rad(m_fov / 2.0)) *
			(double)m_width / (double)m_height)));
	}
	else
	{
		return m_fov;
	}
}

int LDExporter::runInternal(void)
{
	char *outputType = stringForKey("OutputType", NULL, false);
	int retValue;
	LDExporter *pExporter = NULL;

	if (outputType == NULL)
	{
		outputType = stringForKey("OT", "POV", false);
	}
	if (strcasecmp(outputType, "POV") == 0)
	{
		pExporter = new LDPovExporter;
	}
	retValue = runInternal(pExporter);
	delete[] outputType;
	TCObject::release(pExporter);
	return retValue;
}

int LDExporter::runInternal(LDExporter *pExporter)
{
	int retValue;

	if (pExporter != NULL)
	{
		std::string filename = getFilename();

		if (filename.size() > 0)
		{
			LDLMainModel *pMainModel = new LDLMainModel;

			if (pMainModel->load(filename.c_str()))
			{
				char *cameraGlobe = stringForKey("CameraGlobe", NULL, false);
				LDLCamera camera;
				LDLAutoCamera *pAutoCamera = new LDLAutoCamera;

				pMainModel->getBoundingBox(pExporter->m_boundingMin,
					pExporter->m_boundingMax);
				pExporter->m_center = (pExporter->m_boundingMin +
					pExporter->m_boundingMax) / 2.0f;
				pExporter->m_radius =
					pMainModel->getMaxRadius(pExporter->m_center, true);
				pAutoCamera->setModel(pMainModel);
				pAutoCamera->setModelCenter(pExporter->m_center);
				pAutoCamera->setRotationMatrix(m_rotationMatrix);
				pAutoCamera->setCamera(camera);
				pAutoCamera->setCameraGlobe(cameraGlobe);
				//pAutoCamera->setDistanceMultiplier(distanceMultiplier);
				// Width and height are only needed for aspect ratio, not
				// absolute size.
				pAutoCamera->setWidth(m_width);
				pAutoCamera->setHeight(m_height);
				pAutoCamera->setFov(m_fov);
				pAutoCamera->setScanConditionalControlPoints(false);

				pAutoCamera->zoomToFit();
				pExporter->m_camera = pAutoCamera->getCamera();
				pAutoCamera->release();
				retValue = pExporter->doExport(pMainModel);
			}
			else
			{
#ifdef TC_NO_UNICODE
				consolePrintf(ls(_UC("ErrorLoadingModel")), filename.c_str());
#else // TC_NO_UNICODE
				std::wstring wfilename;
				stringtowstring(wfilename, filename);
				consolePrintf(ls(_UC("LDXErrorLoadingModel")), wfilename.c_str());
#endif // TC_NO_UNICODE
				retValue = 1;
			}
			TCObject::release(pMainModel);
		}
		else
		{
			consolePrintf(ls(_UC("LDXNoFilename")));
			retValue = 1;
		}
	}
	else
	{
		consolePrintf(ls(_UC("LDXUnknownOutputType")));
		retValue = 1;
	}
	return retValue;
}

std::string LDExporter::getFilename(void)
{
	std::string retValue;
	const TCStringArray *commandLine = TCUserDefaults::getProcessedCommandLine();

	if (commandLine)
	{
		int i;
		int count = commandLine->getCount();

		for (i = 0; i < count && !retValue.size(); i++)
		{
			const char *arg = commandLine->stringAtIndex(i);

			if (arg[0] != '-')
			{
				retValue = arg;
			}
		}
	}
	return retValue;
}

std::string LDExporter::udKey(const char *key) const
{
	return m_udPrefix + key;
}

void LDExporter::setStringForKey(
	const char* value,
	const char* key,
	bool sessionSpecific /*= true*/)
{
	TCUserDefaults::setStringForKey(value, udKey(key).c_str(), sessionSpecific);
}

char* LDExporter::stringForKey(
	const char* key,
	const char* defaultValue /*= NULL*/,
	bool sessionSpecific /*= true*/)
{
	return TCUserDefaults::stringForKey(udKey(key).c_str(), defaultValue,
		sessionSpecific);
}

void LDExporter::setLongForKey(
	long value,
	const char* key,
	bool sessionSpecific /*= true*/)
{
	TCUserDefaults::setLongForKey(value, udKey(key).c_str(), sessionSpecific);
}

long LDExporter::longForKey(
	const char* key,
	long defaultValue /*= 0*/,
	bool sessionSpecific /*= true*/)
{
	return TCUserDefaults::longForKey(udKey(key).c_str(), defaultValue, 
		sessionSpecific);
}

void LDExporter::setBoolForKey(
	bool value,
	const char *key,
	bool sessionSpecific /*= true*/)
{
	TCUserDefaults::setBoolForKey(value, udKey(key).c_str(), sessionSpecific);
}

bool LDExporter::boolForKey(
	const char *key,
	bool defaultValue /*= false*/,
	bool sessionSpecific /*= true*/)
{
	return TCUserDefaults::boolForKey(udKey(key).c_str(), defaultValue,
		sessionSpecific);
}

std::string LDExporter::pathForKey(
	const char *key,
	const char *defaultValue /*= NULL*/,
	bool sessionSpecific /*= true*/)
{
	char *value = TCUserDefaults::pathForKey(udKey(key).c_str(), defaultValue,
		sessionSpecific);
	std::string retValue;

	if (value != NULL)
	{
		retValue = value;
		delete[] value;
	}
	return retValue;
}

void LDExporter::setFloatForKey(
	float value,
	const char* key,
	bool sessionSpecific /*= true*/)
{
	TCUserDefaults::setFloatForKey(value, udKey(key).c_str(), sessionSpecific);
}

float LDExporter::floatForKey(
	const char* key,
	float defaultValue /*= 0.0f*/,
	bool sessionSpecific /*= true*/)
{
	return TCUserDefaults::floatForKey(udKey(key).c_str(), defaultValue,
		sessionSpecific);
}

