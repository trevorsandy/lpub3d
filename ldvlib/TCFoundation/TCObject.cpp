#include "TCObject.h"
#include "TCAutoreleasePool.h"
#include "TCLocalStrings.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif
#endif // WIN32

TCObject::TCObject(void)
		 :retainCount(1)
{
#ifdef _LEAK_DEBUG
	strcpy(className, "TCObject");
#endif
}

TCObject::~TCObject(void)
{
}

TCObject* TCObject::retain(void)
{
	retainCount++;
	return this;
}

void TCObject::release(void)
{
//	assert(retainCount > 0);
	retainCount--;
	if (!retainCount)
	{
		dealloc();
	}
}

void TCObject::dealloc(void)
{
#if defined WIN32 && defined _DEBUG && defined __THIS_IS_NOT_DEFINED
	TCAutoreleasePool::registerDelete(this);
#else
	delete this;
#endif
//	printf("TCObject::dealloc()\n");
}

TCObject* TCObject::autorelease(void)
{
	TCAutoreleasePool::registerRelease(this);
	return this;
}

int TCObject::isEqual(TCObject& other)
{
	return this == &other;
}

TCObject *TCObject::copy(void) const
{
	return NULL;
}

void TCObject::release(TCObject *object)
{
	// NOTE: object cannot be a TCObject *&, because inheritance won't allow
	// a sub-class to match a reference.  So we cann't have this function also
	// set the original pointer to NULL.
	if (object != NULL)
	{
		object->release();
	}
}

int TCObject::compare(const TCObject *other) const
{
	if (this < other)
	{
		return -1;
	}
	else if (this > other)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

//#if !defined(WIN32) && !defined(COCOA) && !defined(_OSMESA)
//const QString &TCObject::ls(const char *key)
//#else
const char *TCObject::ls(const char *key)
//#endif
{
	return TCLocalStrings::get(key);
}

const wchar_t *TCObject::ls(const wchar_t *key)
{
	return TCLocalStrings::get(key);
}

const char *TCObject::lsUtf8(const char *key)
{
	return TCLocalStrings::getUtf8(key);
}
