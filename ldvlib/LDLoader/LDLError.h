#ifndef __LDLERROR_H__
#define __LDLERROR_H__

#include <TCFoundation/TCAlert.h>
#include <stdlib.h>

class TCStringArray;

typedef enum
{
	LDLEGeneral,
	LDLEFirstError = LDLEGeneral,
	LDLEParse,
	LDLEFileNotFound,
	LDLEMatrix,
	LDLEPartDeterminant,
	LDLENonFlatQuad,
	LDLEConcaveQuad,
	LDLEVertexOrder,
	LDLEMatchingPoints,
//	LDLEOpenGL,
	LDLEColinear,
	LDLEBFCWarning,
	LDLEBFCError,
	LDLEMPDError,
	LDLEWhitespace,
	LDLEMovedTo,
	LDLEUnofficialPart,
	LDLEModelLoop,
	LDLEMetaCommand,
	LDLELastError = LDLEMetaCommand
} LDLErrorType;

typedef std::map<LDLErrorType, std::string> LDLErrorTypeMap;

typedef enum
{
	LDLACriticalError,	// This means the model couldn't be loaded at all.
	LDLAError,
	LDLAWarning
} LDLAlertLevel;

class LDLError: public TCAlert
{
public:
	LDLError(LDLErrorType type, const wchar_t *message, const char *filename,
		const char *fileLine, const char *formattedFileLine, int lineNumber,
		const ucstringVector &extraInfo = ucstringVector());
	LDLError(LDLErrorType type, const char *message, const char *filename,
		const char *fileLine, const char *formattedFileLine, int lineNumber,
		TCStringArray *extraInfo = NULL);
	LDLErrorType getType(void) { return m_type; }
	const char *getFilename(void) const { return m_filename; }
	const char *getFileLine(void) const { return m_fileLine; }
	const char *getFormattedFileLine(void) const { return m_formattedFileLine; }
	int getLineNumber(void) { return m_lineNumber; }
	static const char *alertClass(void) { return "LDLError"; }
	virtual const char *getTypeName(void);
	virtual const wchar_t *getTypeNameW(void);
	static CUCSTR getTypeNameUC(LDLErrorType type)
#ifdef TC_NO_UNICODE
	{
		return getTypeName(type);
	}
#else // TC_NO_UNICODE
	{
		return getTypeNameW(type);
	}
#endif // !TC_NO_UNICODE
	static const char *getTypeName(LDLErrorType type);
	static const wchar_t *getTypeNameW(LDLErrorType type);
	void setLevel(LDLAlertLevel value) { m_level = value; }
	LDLAlertLevel getLevel(void) { return m_level; }
	void cancelLoad(void) { m_loadCanceled = true; }
	bool getLoadCanceled(void) { return m_loadCanceled; }
protected:
	virtual ~LDLError(void);
	virtual void dealloc(void);
	static void initTypeMap(void);

	LDLErrorType m_type;
	char *m_filename;
	char *m_fileLine;
	char *m_formattedFileLine;
	int m_lineNumber;
	LDLAlertLevel m_level;
	bool m_loadCanceled;
	static LDLErrorTypeMap sm_typeMap;
};

#endif // __LDLERROR_H__
