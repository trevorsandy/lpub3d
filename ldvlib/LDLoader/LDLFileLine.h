#ifndef __LDLFILELINE_H__
#define __LDLFILELINE_H__

#include <TCFoundation/TCObject.h>
#include <LDLoader/LDLError.h>
#include <TCFoundation/TCTypedObjectArray.h>
#include <TCFoundation/TCStlIncludes.h>
#include <TCFoundation/TCVector.h>
#include <stdarg.h>

// The following is needed in order to declare the array below (which is used
// in the definition of LDLFileLine itself).
class LDLFileLine;
class LDLCommentLine;
class LDLModel;
class TCImage;

typedef TCTypedObjectArray<LDLFileLine> LDLFileLineArray;
typedef TCTypedObjectArray<LDLCommentLine> LDLCommentLineArray;
typedef TCTypedObjectArray<LDLModel> LDLModelArray;
typedef TCTypedObjectArray<TCImage> TCImageArray;

typedef enum
{
	LDLLineTypeComment,
	LDLLineTypeModel,
	LDLLineTypeLine,
	LDLLineTypeTriangle,
	LDLLineTypeQuad,
	LDLLineTypeConditionalLine,
	LDLLineTypeEmpty,
	LDLLineTypeUnknown
} LDLLineType;

class LDLModel;
class LDLMainModel;

class LDLFileLine : public TCObject
{
public:
	enum TexmapType
	{
		TTPlanar,
		TTCylindrical,
		TTSpherical,
	};
	operator const char *(void) const;
	const char *getLine(void) const { return m_line; }
	virtual const char *getFormattedLine(void) const;
	const char *getOriginalLine(void) const { return m_originalLine; }
	int getLineNumber(void) const { return m_lineNumber; }
	virtual bool parse(void) = 0;
	virtual LDLError *getError(void) { return m_error; }
	virtual void print(int indent) const;
	virtual LDLLineType getLineType(void) const = 0;
	virtual bool isActionLine(void) const { return false; }
	virtual bool isShapeLine(void) const { return false; }
	virtual LDLModel *getParentModel(void) { return m_parentModel; }
	virtual const LDLModel *getParentModel(void) const { return m_parentModel; }
	virtual LDLMainModel *getMainModel(void);
	virtual const LDLMainModel *getMainModel(void) const;
	virtual bool isValid(void) const { return m_valid; }
	virtual bool isReplaced(void) const { return m_replaced; }
	virtual void setReplaced(bool value) { m_replaced = value; }
	virtual void forgetLine(void) { delete[] m_line; m_line = NULL; }
	virtual LDLFileLineArray *getReplacementLines(void);
	virtual bool isXZPlanar(void) const;
	virtual bool isXZPlanar(const TCFloat *matrix) const;
	virtual void setStepIndex(int value) { m_stepIndex = value; }
	virtual int getStepIndex(void) const { return m_stepIndex; }
	virtual TCObject *getAlertSender(void);
	virtual void setTexmapSettings(TexmapType type,
		const std::string &filename, TCImage *image, const TCVector *points,
		const TCFloat *extra);
	const std::string getTexmapFilename(void) const { return m_texmapFilename; }
	TexmapType getTexmapType(void) const { return m_texmapType; }
	TCImage *getTexmapImage(void) { return m_texmapImage; }
	const TCVector *getTexmapPoints(void) const { return m_texmapPoints; }
	const TCFloat *getTexmapExtra(void) const { return m_texmapExtra; }

	void setLineNumber(int value) { m_lineNumber = value; }
	void setParentModel(LDLModel *value);

	static LDLFileLine *initFileLine(LDLModel *parentModel, const char *line,
		int lineNumber, const char *originalLine = NULL);
protected:
	LDLFileLine(LDLModel *parentModel, const char *line, int lineNumber,
		const char *originalLine = NULL);
	LDLFileLine(const LDLFileLine &other);
	virtual ~LDLFileLine(void);
	virtual void dealloc(void);
	virtual void setErrorV(LDLErrorType type, CUCSTR format, va_list argPtr);
	virtual void setError(LDLErrorType type, CUCSTR format, ...);
	virtual void setWarning(LDLErrorType type, CUCSTR format, ...);
	virtual const char *findWord(int index) const;
	std::string getTypeAndColorPrefix(void) const;

	static bool lineIsEmpty(const char *line);
	static int scanLineType(const char *line);

	LDLModel *m_parentModel;
	char *m_line;
	char *m_originalLine;
	char *m_formattedLine;
	int m_lineNumber;
	LDLError *m_error;
	bool m_valid;
	bool m_replaced;
	int m_stepIndex;
	std::string m_texmapFilename;
	TCImage *m_texmapImage;
	TexmapType m_texmapType;
	TCVector m_texmapPoints[3];
	TCFloat m_texmapExtra[2];
};

#endif // __LDLFILELINE_H__
