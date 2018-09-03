#ifndef __LDCONSOLEALERTHANDLER_H__
#define __LDCONSOLEALERTHANDLER_H__

#include <TCFoundation/TCObject.h>
#include <TCFoundation/TCStlIncludes.h>

class LDLError;

class LDConsoleAlertHandler: public TCObject
{
public:
	LDConsoleAlertHandler(int verbosity = 0);
protected:
	virtual ~LDConsoleAlertHandler(void) {}
	virtual void dealloc(void);
	void ldlErrorCallback(LDLError *error);
	bool shouldShowError(LDLError *error);
	
	int m_verbosity;
};

#endif // __LDCONSOLEALERTHANDLER_H__
