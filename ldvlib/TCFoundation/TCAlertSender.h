#ifndef __TCALERTSENDER_H__
#define __TCALERTSENDER_H__

#include <TCFoundation/TCObject.h>

class TCAlertSender: public TCObject
{
public:
	virtual TCObject *getAlertSender(void) { return this; }
};

#endif // __TCALERTSENDER_H__
