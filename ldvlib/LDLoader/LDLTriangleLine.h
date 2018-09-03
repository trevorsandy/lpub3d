#ifndef __LDLTRIANGLELINE_H__
#define __LDLTRIANGLELINE_H__

#include <LDLoader/LDLShapeLine.h>

class LDLQuadLine;
class LDLLineLine;

class LDLTriangleLine : public LDLShapeLine
{
public:
	virtual TCObject *copy(void) const;
	virtual bool parse(void);
	virtual int getNumPoints(void) const { return 3; }
	virtual LDLLineType getLineType(void) const { return LDLLineTypeTriangle; }
	virtual LDLFileLineArray *getReplacementLines(void);
protected:
	LDLTriangleLine(LDLModel *parentModel, const char *line, int lineNumber,
		const char *originalLine = NULL);
	LDLTriangleLine(const LDLTriangleLine &other);
	void checkForColinearPoints(void);
	int getColinearIndex(void);
	LDLFileLineArray *removePoint(int index);
	LDLFileLineArray *removeMatchingPoint(void);
	LDLFileLineArray *removeColinearPoint(void);
	LDLLineLine *newLineLine(int p1, int p2);

	int m_colinearIndex;
	int m_matchingIndex;

	friend class LDLFileLine; // Needed because constructors are protected.
	friend class LDLQuadLine; // Needed because constructors are protected, and bad
						// quads can be converted to one or two triangles.
};

#endif // __LDLTRIANGLELINE_H__
