#ifndef __LDPARTCOUNT_H__
#define __LDPARTCOUNT_H__

#include <TCFoundation/TCDefines.h>
#include <TCFoundation/TCStlIncludes.h>

class LDLModel;

typedef std::map<int, int> IntIntMap;
typedef std::vector<int> IntVector;

class LDPartCount
{
public:
	LDPartCount(void);
	LDPartCount(const LDPartCount &other);
	virtual ~LDPartCount(void);
	void setModel(const char *filename, LDLModel *model);
	const LDLModel *getModel(void) const { return m_model; };
	void addPart(int color, int defaultColor);
	size_t getTotalCount(void) const { return m_totalCount; };
	size_t getNumColors(void) const { return m_colorCounts.size(); }
	const IntVector &getColors(void) const;
	int getColorCount(int color) const;
	const char *getFilename(void) const { return m_filename.c_str(); }
protected:
	std::string m_filename;
	LDLModel *m_model;
	size_t m_totalCount;
	IntIntMap m_colorCounts;
	mutable IntVector m_colors;
	mutable bool m_colorsCalculated;
};

#endif // __LDPARTCOUNT_H__
