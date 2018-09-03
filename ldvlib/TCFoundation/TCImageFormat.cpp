#include "TCImageFormat.h"
#include "TCProgressAlert.h"
#include "TCImageOptions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#endif // WIN32

#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif

TCImageFormat::TCImageFormat(void):
name(NULL),
progressCallback(NULL),
progressUserData(NULL)
{
#ifdef _LEAK_DEBUG
	strcpy(className, "TCImageFormat");
#endif
}

TCImageFormat::~TCImageFormat(void)
{
}

void TCImageFormat::dealloc(void)
{
	TCObject::dealloc();
}

bool TCImageFormat::checkSignature(const char *filename)
{
	FILE *file = ucfopen(filename, "rb");
	bool retValue = false;

	if (file)
	{
		retValue = checkSignature(file);
		fclose(file);
	}
	return retValue;
}

bool TCImageFormat::loadFile(TCImage *image, const char *filename)
{
	FILE *file = ucfopen(filename, "rb");
	bool retValue = false;

	if (file)
	{
		retValue = loadFile(image, file);
		fclose(file);
	}
	return retValue;
}

bool TCImageFormat::saveFile(TCImage *image, const char *filename)
{
	FILE *file = ucfopen(filename, "wb");
	bool retValue = false;

	if (file)
	{
		retValue = saveFile(image, file);
		fclose(file);
		if (!retValue)
		{
			unlink(filename);
		}
	}
	return retValue;
}

bool TCImageFormat::callProgressCallback(CUCSTR message, float progress)
{
	if (progressCallback)
	{
		return progressCallback(message, progress, progressUserData);
	}
	else
	{
		bool aborted = false;

		if (message == NULL)
		{
			message = _UC("");
		}
		TCProgressAlert::send("TCImageFormat", message, progress, &aborted,
			this);
		return !aborted;
	}
}

TCImageOptions *TCImageFormat::newCompressionOptions(void)
{
	return new TCImageOptions;
}
