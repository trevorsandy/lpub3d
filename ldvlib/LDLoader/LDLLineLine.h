#ifndef __LDLLINELINE_H__
#define __LDLLINELINE_H__

#include <LDLoader/LDLShapeLine.h>

class LDLTriangleLine;

class LDLLineLine : public LDLShapeLine
{
public:
	LDLLineLine(LDLModel *parentModel, const char *line, int lineNumber,
		const char *originalLine = NULL);
	virtual TCObject *copy(void) const;
	virtual bool parse(void);
	virtual int getNumPoints(void) const { return 2; }
	virtual LDLLineType getLineType(void) const { return LDLLineTypeLine; }
protected:
	LDLLineLine(const LDLLineLine &other);

	friend class LDLFileLine;		// Needed because constructors are protected.
	friend class LDLTriangleLine;	// Needed because constructors are protected, and
							// some bad triangles can be converted to a line.
};

#endif // __LDLLINELINE_H__
