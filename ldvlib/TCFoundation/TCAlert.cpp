#include "TCAlert.h"
#include "TCStringArray.h"
#include "mystring.h"

#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG

TCAlert::TCAlert(
		const char *alertClass,
		const char *message,
		TCStringArray *extraInfo):
	m_alertClass(copyString(alertClass)),
	m_message(copyString(message)),
	m_extraInfo(TCObject::retain(extraInfo)),
	m_sender(NULL)
{
}

TCAlert::TCAlert(
		const char *alertClass,
		const wchar_t *message,
		const ucstringVector &extraInfo /*= WStringList()*/):
	m_alertClass(copyString(alertClass)),
	m_message(NULL),
	m_wMessage(message),
	m_extraInfo(NULL),
	m_ucExtraInfo(extraInfo),
	m_sender(NULL)
{
}

TCAlert::~TCAlert(void)
{
}

void TCAlert::dealloc(void)
{
	delete[] m_alertClass;
	delete[] m_message;
	TCObject::release(m_extraInfo);
	TCObject::dealloc();
}

const char *TCAlert::getMessage(void)
{
	if (!m_message)
	{
		std::string temp;

		wstringtostring(temp, m_wMessage);
		m_message = copyString(temp.c_str());
	}
	return m_message;
}

const wchar_t *TCAlert::getWMessage(void)
{
	if (!m_wMessage.size() && m_message && m_message[0])
	{
		mbstowstring(m_wMessage, m_message);
	}
#ifdef NO_WSTRING
        return NULL;
#else // NO_WSTRING
	return m_wMessage.c_str();
#endif // NO_WSTRING
}

CUCSTR TCAlert::getMessageUC(void)
{
#ifdef TC_NO_UNICODE
	return getMessage();
#else // TC_NO_UNICODE
	return getWMessage();
#endif // TC_NO_UNICODE
}

TCStringArray *TCAlert::getExtraInfo(void)
{
	if (!m_extraInfo && m_ucExtraInfo.size() > 0)
	{
		m_extraInfo = new TCStringArray;
		std::string temp;

		for (ucstringVector::iterator it = m_ucExtraInfo.begin();
			it != m_ucExtraInfo.end(); ++it)
		{
#ifdef TC_NO_UNICODE
			temp = *it;
#else // TC_NO_UNICODE
			wstringtostring(temp, *it);
#endif // TC_NO_UNICODE
			m_extraInfo->addString(temp.c_str());
		}
	}
	return m_extraInfo;
}

const ucstringVector &TCAlert::getUCExtraInfo(void)
{
	if (!m_ucExtraInfo.size() && m_extraInfo && m_extraInfo->getCount() > 0)
	{
		int i;
		int count = m_extraInfo->getCount();

		m_ucExtraInfo.resize(count);
		for (i = 0; i < count; i++)
		{
			ucstring &temp = m_ucExtraInfo[i];
#ifdef TC_NO_UNICODE
			temp = m_extraInfo->stringAtIndex(i);
#else // TC_NO_UNICODE
			mbstowstring(temp, m_extraInfo->stringAtIndex(i));
#endif // TC_NO_UNICODE
		}
	}
	return m_ucExtraInfo;
}
