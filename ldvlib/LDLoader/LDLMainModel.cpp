#include "LDLMainModel.h"
#include "LDLPalette.h"
#include <TCFoundation/TCDictionary.h>
#include <TCFoundation/TCStringArray.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <TCFoundation/TCLocalStrings.h>
#include <TCFoundation/TCUserDefaults.h>
#include "LDrawIni.h"
#include <clocale>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

LDLMainModel::LDLMainModel(void)
	:m_alertSender(NULL),
	m_loadedModels(NULL),
	m_mainPalette(new LDLPalette),
	m_extraSearchDirs(NULL),
	m_seamWidth(0.0f),
	m_highlightColorNumber(0x2FFFFFF)
{
	// We need to use '.' as our decimal separator, no matter what the system
	// default decimal separator is.
	std::setlocale(LC_NUMERIC, "C");
	// Initialize all flags to false.
	memset(&m_mainFlags, 0, sizeof(m_mainFlags));
	m_mainFlags.processLDConfig = true;
	m_mainFlags.checkPartTracker = true;
	m_mainFlags.scanConditionalControlPoints = true;
}

TCObject *LDLMainModel::copy(void) const
{
	return NULL;
}

TCDictionary *LDLMainModel::getLoadedModels(void)
{
	if (!m_loadedModels)
	{
		m_loadedModels = new TCDictionary(0);
	}
	return m_loadedModels;
}

void LDLMainModel::dealloc(void)
{
	TCObject::release(m_loadedModels);
	TCObject::release(m_mainPalette);
	TCObject::release(m_extraSearchDirs);
	LDLModel::dealloc();
}

void LDLMainModel::ldrawDirNotFound(void)
{
	LDLError *error = newError(LDLEGeneral,
		TCLocalStrings::get(_UC("LDLMainModelNoLDrawDir")));
	error->setLevel(LDLACriticalError);
	sendAlert(error);
	error->release();
}

bool LDLMainModel::load(const char *filename)
{
	std::ifstream stream;
	LDLError *error;

	setFilename(filename);
	if (TCUserDefaults::boolForKey("VerifyLDrawDir", true, false))
	{
		if (!verifyLDrawDir(lDrawDir()))
		{
			ldrawDirNotFound();
			return false;
		}
	}
	else
	{
		lDrawDir(); // Initializes sm_lDrawIni
	}
	if (sm_lDrawIni)
	{
		LDrawIniComputeRealDirs(sm_lDrawIni, 1, 0, filename);
	}
	if (!strlen(lDrawDir()))
	{
		ldrawDirNotFound();
		return false;
	}
	m_mainModel = this;
	if (openStream(filename, stream))
	{
		bool retValue;

		if (m_mainFlags.processLDConfig)
		{
			processLDConfig();
		}
		retValue = LDLModel::load(stream);
		if (sm_lDrawIni)
		{
			// If bool isn't 1 byte, then the filename case callback won't
			// work, so doesn't get used.  Check to see if this will be a
			// problem.
			if (sizeof(bool) != sizeof(char) && fileCaseCallback)
			{
				char *tmpStr;
				size_t len = strlen(lDrawDir());
				bool failed = false;
				struct stat statData;

				tmpStr = new char[len + 10];
				sprintf(tmpStr, "%s/P", lDrawDir());
				if (stat(tmpStr, &statData) != 0)
				{
					// Check to see if we can access the P directory inside the
					// LDraw directory.  If not, then we have a problem that
					// needs to be reported to the user.
					failed = true;
				}
				if (!failed)
				{
					sprintf(tmpStr, "%s/PARTS", lDrawDir());
					if (stat(tmpStr, &statData) != 0)
					{
						// Check to see if we can access the PARTS directory
						// inside the LDraw directory.  If not, then we have a
						// problem that needs to be reported to the user.
						failed = true;
					}
				}
				delete[] tmpStr;
				if (failed)
				{
					// Either P or PARTS was inaccessible, so let the user
					// know that they need to rename the directories to be in
					// upper case.
					reportError(LDLEGeneral,
						TCLocalStrings::get(_UC("LDLMainModelFileCase")));
				}
			}
		}
		// The ancestor map has done its job; may as well free up the memory it
		// was using.
		m_ancestorMap.clear();
		if (getHaveMpdTexmaps())
		{
			TCDictionary* subModelDict = getLoadedModels();
			if (subModelDict != NULL)
			{
				TCObjectArray *subModels = subModelDict->allObjects();
				int subModelCount = subModels->getCount();
				for (int i = 0; i < subModelCount; ++i)
				{
					LDLModel *subModel = (LDLModel *)(*subModels)[i];
					subModel->loadMpdTexmaps();
				}
			}
		}
		return retValue;
	}
	else
	{
		error = newError(LDLEFileNotFound,
			TCLocalStrings::get(_UC("LDLMainModelNoMainModel")));
		error->setLevel(LDLACriticalError);
		sendAlert(error);
		error->release();
		return false;
	}
}

void LDLMainModel::processLDConfig(void)
{
	std::ifstream configStream;
	std::string filename;
	if (!m_ldConfig.empty())
	{
		// First, check the standard model path
		if (!openSubModelNamed(m_ldConfig.c_str(), filename, configStream,
			false))
		{
			// Next, check the root LDraw dir
			combinePathParts(filename, lDrawDir(), "/", m_ldConfig);
			openFile(filename.c_str(), configStream);
		}
	}
	if (!configStream.is_open())
	{
		combinePathParts(filename, lDrawDir(), "/ldconfig.ldr");
		openFile(filename.c_str(), configStream);
	}
	if (configStream.is_open())
	{
		configStream.close();
		subModelNamed(filename.c_str());
	}
}

void LDLMainModel::print(void)
{
	printf("Model Count: %d\n", sm_modelCount);
	LDLModel::print(0);
}

bool LDLMainModel::transparencyIsDisabled(void) const
{
	return m_mainFlags.redBackFaces || m_mainFlags.greenFrontFaces || m_mainFlags.blueNeutralFaces;
}

bool LDLMainModel::colorNumberIsTransparent(int colorNumber)
{
	if (transparencyIsDisabled())
	{
		return false;
	}
	if (m_mainPalette != NULL)
	{
		int r, g, b, a;

		m_mainPalette->getRGBA(colorNumber, r, g, b, a);
		return a < 255;
	}
	else
	{
		return (colorNumber >= 32 && colorNumber < 48) ||
			(colorNumber >= 0x3000000 && colorNumber < 0x4000000) ||
			(colorNumber >= 0x5000000 && colorNumber < 0x7000000) ||
			(colorNumber >= 0x7000000 && colorNumber < 0x8000000) ||
			colorNumber == 57;
	}
}

void LDLMainModel::getRGBA(int colorNumber, int& r, int& g, int& b, int& a)
{
	m_mainPalette->getRGBA(colorNumber, r, g, b, a);
	if (transparencyIsDisabled())
	{
		a = 255;
	}
}

bool LDLMainModel::hasSpecular(int colorNumber)
{
	return m_mainPalette->hasSpecular(colorNumber);
}

bool LDLMainModel::hasShininess(int colorNumber)
{
	return m_mainPalette->hasShininess(colorNumber);
}

void LDLMainModel::getSpecular(int colorNumber, float *specular)
{
	memcpy(specular, m_mainPalette->getAnyColorInfo(colorNumber).specular,
		4 * sizeof(float));
}

void LDLMainModel::getShininess(int colorNumber, float &shininess)
{
	shininess = m_mainPalette->getAnyColorInfo(colorNumber).shininess;
}

int LDLMainModel::getEdgeColorNumber(int colorNumber)
{
	if (getBlackEdgeLines())
	{
		int r, g, b, a;
		m_mainPalette->getRGBA(colorNumber, r, g, b, a);
		int edgeColorNumber;

		if (30 * r + 59 * g + 11 * b <= 3600)
		{
			edgeColorNumber =
				m_mainPalette->getColorNumberForName("Dark Gray Edge");
			if (edgeColorNumber < 0)
			{
				edgeColorNumber = 0x2555555;
				//edgeColorNumber = 8;
			}
		}
		else
		{
			edgeColorNumber =
				m_mainPalette->getColorNumberForName("Black Edge");
			if (edgeColorNumber < 0)
			{
				edgeColorNumber = 0x2000000;
			}
		}
		return edgeColorNumber;
	}
	return m_mainPalette->getEdgeColorNumber(colorNumber);
}

void LDLMainModel::setExtraSearchDirs(TCStringArray *value)
{
	if (m_extraSearchDirs != value)
	{
		TCObject::release(m_extraSearchDirs);
		m_extraSearchDirs = value;
		TCObject::retain(m_extraSearchDirs);
	}
}

void LDLMainModel::setSeamWidth(float value)
{
	if (value != m_seamWidth)
	{
		m_seamWidth = value;
		m_flags.haveBoundingBox = false;
	}
}

void LDLMainModel::release()
{
	LDLModel::release();
}
