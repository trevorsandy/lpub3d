#include "TCProgressAlert.h"
#include "TCStringArray.h"
#include "TCAlertManager.h"
#include "mystring.h"

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

TCProgressAlert::TCProgressAlert(const char *source, const char *message,
								 float progress, TCStringArray *extraInfo)
	:TCAlert(TCProgressAlert::alertClass(), message, extraInfo),
	m_source(copyString(source)),
	m_progress(progress),
	m_aborted(false)
{
}

TCProgressAlert::TCProgressAlert(const char *source, const wchar_t *message,
								 float progress,
								 const ucstringVector &extraInfo)
	:TCAlert(TCProgressAlert::alertClass(), message, extraInfo),
	m_source(copyString(source)),
	m_progress(progress),
	m_aborted(false)
{
}

TCProgressAlert::~TCProgressAlert(void)
{
}

void TCProgressAlert::dealloc(void)
{
	delete[] m_source;
	TCAlert::dealloc();
}

void TCProgressAlert::send(
	const char *source,
	const char *message,
	float progress,
	TCStringArray *extraInfo,
	TCAlertSender *sender)
{
	send(source, message, progress, NULL, extraInfo, sender);
}

void TCProgressAlert::send(
	const char *source,
	const wchar_t *message,
	float progress,
	const ucstringVector &extraInfo,
	TCAlertSender *sender)
{
	send(source, message, progress, NULL, extraInfo, sender);
}

void TCProgressAlert::send(
	const char *source,
	const char *message,
	float progress,
	bool *aborted,
	TCStringArray *extraInfo,
	TCAlertSender *sender)
{
	TCProgressAlert *alert = new TCProgressAlert(source, message, progress,
		extraInfo);

	TCAlertManager::sendAlert(alert, sender);
	if (aborted)
	{
		*aborted = alert->getAborted();
	}
	alert->release();
}

void TCProgressAlert::send(
	const char *source,
	const char *message,
	float progress,
	bool *aborted,
	TCAlertSender *sender)
{
	TCProgressAlert *alert = new TCProgressAlert(source, message, progress);

	TCAlertManager::sendAlert(alert, sender);
	if (aborted)
	{
		*aborted = alert->getAborted();
	}
	alert->release();
}

void TCProgressAlert::send(
	const char *source,
	const char *message,
	float progress,
	TCAlertSender *sender)
{
	TCProgressAlert *alert = new TCProgressAlert(source, message, progress);

	TCAlertManager::sendAlert(alert, sender);
	alert->release();
}

void TCProgressAlert::send(
	const char *source,
	const wchar_t *message,
	float progress,
	bool *aborted,
	TCAlertSender *sender)
{
	send(source, message, progress, aborted, ucstringVector(), sender);
}

void TCProgressAlert::send(
	const char *source,
	const wchar_t *message,
	float progress,
	TCAlertSender *sender)
{
	send(source, message, progress, NULL, ucstringVector(), sender);
}

void TCProgressAlert::send(
	const char *source,
	const wchar_t *message,
	float progress,
	bool *aborted,
	const ucstringVector &extraInfo,
	TCAlertSender *sender)
{
	TCProgressAlert *alert = new TCProgressAlert(source, message, progress,
		extraInfo);

	TCAlertManager::sendAlert(alert, sender);
	if (aborted)
	{
		*aborted = alert->getAborted();
	}
	alert->release();
}
