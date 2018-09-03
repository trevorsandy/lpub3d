#ifndef __TCALERTMANAGER_H__
#define __TCALERTMANAGER_H__

#include <TCFoundation/TCObject.h>
#include <TCFoundation/TCTypedObjectArray.h>
#include <TCFoundation/TCTypedPointerArray.h>
#include <TCFoundation/TCStringArray.h>
#include <string.h>

class TCAlert;
class TCAlertSender;

typedef void (TCObject::*TCAlertCallback)(TCAlert* alert);

typedef TCTypedPointerArray<TCObject *> TCObjectPointerArray;
typedef TCTypedObjectArray<TCObjectPointerArray> TCObjectPointerArrayArray;
typedef TCTypedPointerArray<TCAlertCallback *> TCAlertCallbackArray;
typedef TCTypedObjectArray<TCAlertCallbackArray> TCAlertCallbackArrayArray;

class TCAlertManager : public TCObject
{
public:
	static void sendAlert(TCAlert *alert, TCAlertSender *sender = NULL);
	static void sendAlert(const char *alertClass, TCAlertSender *sender = NULL,
		CUCSTR message = _UC(""));
	static void registerHandler(const char *alertClass, TCObject *handler,
		TCAlertCallback alertCallback);
	static void unregisterHandler(const char *alertClass, TCObject *handler);
	static void unregisterHandler(TCObject *handler);
protected:
	TCAlertManager(void);
	virtual ~TCAlertManager(void);
	virtual void dealloc(void);
	static TCAlertManager *defaultAlertManager(void);
	int alertClassIndex(const char *alertClass);
	void defSendAlert(TCAlert *alert);
	void defRegisterHandler(const char *alertClass, TCObject *handler,
		TCAlertCallback callback);
	void defUnregisterHandler(const char *alertClass, TCObject *handler);
	void defUnregisterHandler(TCObject *handler);

	TCStringArray *m_alertClasses;
	TCObjectPointerArrayArray *m_handlers;
	TCAlertCallbackArrayArray *m_callbacks;

	static TCAlertManager *sm_defaultAlertManager;
	static class TCAlertManagerCleanup
	{
	public:
		~TCAlertManagerCleanup(void);
	} sm_alertManagerCleanup;
	friend class TCAlertManagerCleanup;
};

#endif // __TCALERTMANAGER_H__
