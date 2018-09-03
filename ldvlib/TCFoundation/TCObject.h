#ifndef __TCOBJECT_H__
#define __TCOBJECT_H__

#include <TCFoundation/TCDefines.h>

#ifdef _QT
#include <stdlib.h>
#endif // _QT

class TCAutoreleasePool;

class TCExport TCObject
{
public:
	TCObject(void);
	virtual TCObject* retain(void);
	virtual void release(void);
	virtual TCObject* autorelease(void);
	virtual int isEqual(TCObject& other);
	virtual TCObject *copy(void) const;
	int getRetainCount(void) { return retainCount; }
	virtual int compare(const TCObject *other) const;

	template <typename _Ty> static _Ty *retain(_Ty *object)
	{
		if (object != NULL)
		{
			return (_Ty *)object->retain();
		}
		else
		{
			return NULL;
		}
	}
	static void release(TCObject *object);
	template <typename _Ty> static _Ty *autorelease(_Ty *object)
	{
		if (object != NULL)
		{
			return (_Ty *)object->autorelease();
		}
		else
		{
			return NULL;
		}
	}
	template <typename _Ty> static _Ty *copy(const _Ty *object)
	{
		if (object != NULL)
		{
			return (_Ty *)object->copy();
		}
		else
		{
			return NULL;
		}
	}


	// Local Strings
//#if !defined(WIN32) && !defined(COCOA) && !defined(_OSMESA)
//	static const QString &ls(const char *key);
//#else
	static const char *ls(const char *key);
//#endif
	static const wchar_t *ls(const wchar_t *key);
	static const char *lsUtf8(const char *key);

protected:
	virtual ~TCObject(void);
	virtual void dealloc(void);

#ifdef _LEAK_DEBUG
	char className[32];
#endif
	int retainCount;
	friend class TCAutoreleasePool;
};

#endif
