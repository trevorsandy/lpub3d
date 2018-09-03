#ifndef __TCALERT_H__
#define __TCALERT_H__

#include <TCFoundation/TCObject.h>
#include <TCFoundation/TCStlIncludes.h>
#include <TCFoundation/mystring.h>

#if defined(_QT) || defined(__APPLE__)
#include <stdlib.h>
#endif // _QT || __APPLE__

#define USER_ALERTS 0x1000

class TCStringArray;
class TCAlertSender;

class TCAlert : public TCObject
{
public:
	TCAlert(const char *alertClass, const char *message,
		TCStringArray *extraInfo = NULL);
	TCAlert(const char *alertClass, const wchar_t *message,
		const ucstringVector &extraInfo = ucstringVector());
	void setSender(TCAlertSender *value) { m_sender = value; }
	TCAlertSender *getSender(void) { return m_sender; }
	//TCULong getAlertClass(void) { return m_alertClass; }
	const char *getAlertClass(void) const { return m_alertClass; }
	const char *getMessage(void);
	const wchar_t *getWMessage(void);
	CUCSTR getMessageUC(void);
	TCStringArray *getExtraInfo(void);
	const ucstringVector &getUCExtraInfo(void);
	static TCULong alertClass(void) { return 0; }
protected:
	virtual ~TCAlert(void);
	virtual void dealloc(void);

	//TCULong m_alertClass;
	char *m_alertClass;
	char *m_message;
	std::wstring m_wMessage;
	TCStringArray *m_extraInfo;
	ucstringVector m_ucExtraInfo;
	TCAlertSender *m_sender;
};

#endif // __TCALERT_H__
