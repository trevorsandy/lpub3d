#ifndef __LDPARTSLIST_H__
#define __LDPARTSLIST_H__

#include <TCFoundation/TCObject.h>
#include <TCFoundation/TCStlIncludes.h>
#include <string.h>
#include <LDLib/LDPartCount.h>

typedef std::map<std::string, LDPartCount> LDPartCountMap;
typedef std::vector<LDPartCount> LDPartCountVector;

class LDLModel;

class LDPartsList : public TCObject
{
public:
	LDPartsList(void);
	virtual void scanModel(LDLModel *model, int defaultColor);
	virtual const LDPartCountVector &getPartCounts(void)
	{
		return m_partCounts;
	}
	int getTotalParts(void) { return m_totalParts; }
protected:
	virtual ~LDPartsList(void);
	virtual void dealloc(void);
	virtual void scanSubModel(LDLModel *model, int defaultColor);

	LDPartCountMap m_partCountMap;
	LDPartCountVector m_partCounts;
	int m_totalParts;
};

#endif // __LDPARTSLIST_H__
