#ifndef __LDLUNKNOWNLINE_H__
#define __LDLUNKNOWNLINE_H__

#include <LDLoader/LDLFileLine.h>

class LDLUnknownLine : public LDLFileLine
{
public:
	virtual TCObject *copy(void) const;
	virtual bool parse(void);
	virtual LDLLineType getLineType(void) const { return LDLLineTypeUnknown; }
protected:
	LDLUnknownLine(LDLModel *parentModel, const char *line, int lineNumber,
		const char *originalLine = NULL);
	LDLUnknownLine(const LDLUnknownLine &other);
	virtual ~LDLUnknownLine(void);

	friend class LDLFileLine; // Needed because constructors are protected.
};

#endif // __LDLUNKNOWNLINE_H__
