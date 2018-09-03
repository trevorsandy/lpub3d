#ifndef __LDOBIINFO_H__
#define __LDOBIINFO_H__

#include <TCFoundation/TCStlIncludes.h>
#include <TCFoundation/TCDefines.h>

typedef std::set<std::string> StringSet;
typedef std::stack<TCULong> ULongStack;
typedef std::stack<bool> BoolStack;

class LDLCommentLine;

class LDObiInfo
{
public:
	LDObiInfo(void) {}
	bool isActive(void) { return m_colors.size() > 0; }
	TCULong getColor(int defaultColor = 0) const;
	TCULong getEdgeColor(int defaultColor = 0) const;
	void start(LDLCommentLine *commentLine, const StringSet &tokens);
	void start(TCULong color, TCULong edgeColor, bool sticky);
	void end(void);
	void actionHappened(void);
private:
	// Make sure there's no accidental copying.
	LDObiInfo(const LDObiInfo &) {}
	LDObiInfo &operator=(const LDObiInfo &) { return *this; }
	bool checkConditional(LDLCommentLine *commentLine, const StringSet &tokens);

	ULongStack m_colors;
	ULongStack m_edgeColors;
	BoolStack m_stickies;
};

#endif // __LDOBIINFO_H__
