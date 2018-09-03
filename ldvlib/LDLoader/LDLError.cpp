#include "LDLError.h"
#include <TCFoundation/mystring.h>
#include <TCFoundation/TCStringArray.h>
#include <TCFoundation/TCLocalStrings.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

LDLErrorTypeMap LDLError::sm_typeMap;

LDLError::LDLError(LDLErrorType type, const wchar_t *message,
				   const char *filename, const char *fileLine,
				   const char *formattedFileLine, int lineNumber,
				   const ucstringVector &extraInfo /*= ucstringVector()*/)
	:TCAlert(LDLError::alertClass(), message, extraInfo),
	m_type(type),
	m_filename(copyString(filename)),
	m_fileLine(copyString(fileLine)),
	m_formattedFileLine(copyString(formattedFileLine)),
	m_lineNumber(lineNumber),
	m_level(LDLAError),
	m_loadCanceled(false)
{
#ifdef _LEAK_DEBUG
	strcpy(className, "LDLError");
#endif
}

LDLError::LDLError(LDLErrorType type, const char *message, const char *filename,
				   const char *fileLine, const char *formattedFileLine, int lineNumber,
				   TCStringArray *extraInfo /*= NULL*/)
	:TCAlert(LDLError::alertClass(), message, extraInfo),
	m_type(type),
	m_filename(copyString(filename)),
	m_fileLine(copyString(fileLine)),
	m_formattedFileLine(copyString(formattedFileLine)),
	m_lineNumber(lineNumber),
	m_level(LDLAError),
	m_loadCanceled(false)
{
#ifdef _LEAK_DEBUG
	strcpy(className, "LDLError");
#endif
}

LDLError::~LDLError(void)
{
}

void LDLError::dealloc(void)
{
	delete[] m_filename;
	delete[] m_fileLine;
	delete[] m_formattedFileLine;
	TCAlert::dealloc();
}

const char *LDLError::getTypeName(void)
{
	return getTypeName(m_type);
}

const wchar_t *LDLError::getTypeNameW(void)
{
	return getTypeNameW(m_type);
}

void LDLError::initTypeMap(void)
{
	if (sm_typeMap.size() == 0)
	{
		sm_typeMap[LDLEGeneral] = "LDLEGeneral";
		sm_typeMap[LDLEParse] = "LDLEParse";
		sm_typeMap[LDLEFileNotFound] = "LDLEFileNotFound";
		sm_typeMap[LDLEMatrix] = "LDLEMatrix";
		sm_typeMap[LDLEPartDeterminant] = "LDLEPartDeterminant";
		sm_typeMap[LDLENonFlatQuad] = "LDLENonFlatQuad";
		sm_typeMap[LDLEConcaveQuad] = "LDLEConcaveQuad";
		sm_typeMap[LDLEMatchingPoints] = "LDLEMatchingPoints";
		sm_typeMap[LDLEColinear] = "LDLEColinear";
		sm_typeMap[LDLEBFCWarning] = "LDLEBFCWarning";
		sm_typeMap[LDLEBFCError] = "LDLEBFCError";
		sm_typeMap[LDLEMPDError] = "LDLEMPDError";
		sm_typeMap[LDLEVertexOrder] = "LDLEVertexOrder";
		sm_typeMap[LDLEWhitespace] = "LDLEWhitespace";
		sm_typeMap[LDLEMovedTo] = "LDLEMovedTo";
		sm_typeMap[LDLEUnofficialPart] = "LDLEUnofficialPart";
		sm_typeMap[LDLEModelLoop] = "LDLEModelLoop";
		sm_typeMap[LDLEMetaCommand] = "LDLEMetaCommand";
	}
}

const char *LDLError::getTypeName(LDLErrorType type)
{
	initTypeMap();
	LDLErrorTypeMap::const_iterator it = sm_typeMap.find(type);

	if (it != sm_typeMap.end())
	{
		return TCLocalStrings::get(it->second.c_str());
	}
	else
	{
		return TCLocalStrings::get("LDLEUnknown");
	}
}

const wchar_t *LDLError::getTypeNameW(LDLErrorType type)
{
	initTypeMap();
	LDLErrorTypeMap::const_iterator it = sm_typeMap.find(type);

	if (it != sm_typeMap.end())
	{
		std::wstring wkey;

		stringtowstring(wkey, it->second);
#ifdef NO_WSTRING
		return L"";
#else // NO_WSTRING
		return TCLocalStrings::get(wkey.c_str());
#endif // NO_WSTRING
	}
	else
	{
		return TCLocalStrings::get(L"LDLEUnknown");
	}
}
