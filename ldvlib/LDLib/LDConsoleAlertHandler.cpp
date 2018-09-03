#include "LDConsoleAlertHandler.h"
#include <TCFoundation/TCAlertManager.h>
#include <LDLoader/LDLError.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

LDConsoleAlertHandler::LDConsoleAlertHandler(int verbosity):
	m_verbosity(verbosity)
{
	TCAlertManager::registerHandler(LDLError::alertClass(), this, (TCAlertCallback)&LDConsoleAlertHandler::ldlErrorCallback);
}

void LDConsoleAlertHandler::dealloc(void)
{
	TCAlertManager::unregisterHandler(this);
	TCObject::dealloc();
}

void LDConsoleAlertHandler::ldlErrorCallback(LDLError *error)
{
	if (!shouldShowError(error))
	{
		return;
	}
	const char *level = NULL;
	switch (error->getLevel())
	{
	case LDLACriticalError:
		level = ls("CriticalError");
		break;
	case LDLAError:
		level = ls("Error");
		break;
	default:
	case LDLAWarning:
		level = ls("Warning");
		break;
	}
	consolePrintf("%s: %s\n", level, error->getMessage());
	if (error->getFilename() != NULL)
	{
		consolePrintf("%s%s\n", ls("ErrorTreeFilePrefix"),
			error->getFilename());
	}
	else
	{
		consolePrintf("%s\n", ls("ErrorTreeUnknownFile"));
	}
	if (error->getFileLine() != NULL)
	{
		if (error->getLineNumber() >= 0)
		{
			consolePrintf(ls("ErrorTreeLine#"), error->getLineNumber());
			consolePrintf(": ");
		}
		else
		{
			consolePrintf("%s\n", ls("ErrorTreeUnknownLine#"));
		}
		consolePrintf("%s\n", error->getFileLine());
	}
	else
	{
		consolePrintf("%s\n", ls("ErrorTreeUnknownLine"));
	}
	TCStringArray *extraInfo = error->getExtraInfo();
	if (extraInfo)
	{
		for (int i = 0; i < extraInfo->getCount(); ++i)
		{
			consolePrintf("    %s\n", extraInfo->stringAtIndex(i));
		}
	}
}

bool LDConsoleAlertHandler::shouldShowError(LDLError *error)
{
	switch (error->getLevel())
	{
	case LDLACriticalError:
		return m_verbosity >= 0;
	case LDLAError:
		return m_verbosity >= 1;
	default:
	case LDLAWarning:
		return m_verbosity >= 2;
	}
}
