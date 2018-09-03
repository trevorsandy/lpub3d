#ifndef __LDLEMPTYLINE_H__
#define __LDLEMPTYLINE_H__

#include <LDLoader/LDLFileLine.h>

class LDLEmptyLine : public LDLFileLine
{
public:
	virtual TCObject *copy(void) const;
	virtual bool parse(void);
	virtual LDLLineType getLineType(void) const { return LDLLineTypeEmpty; }
protected:
	LDLEmptyLine(LDLModel *parentModel, const char *line, int lineNumber,
		const char *originalLine = NULL);
	LDLEmptyLine(const LDLEmptyLine &other);
	virtual ~LDLEmptyLine(void);

	friend class LDLFileLine; // Needed because constructors are protected.
};

#endif // __LDLEMPTYLINE_H__
