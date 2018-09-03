#ifndef __TCAUTORELEASEPOOL_H__
#define __TCAUTORELEASEPOOL_H__

#include <TCFoundation/TCObject.h>

template <class Type> class TCArray;

class TCAutoreleasePool
{
	public:
		TCExport TCAutoreleasePool(void);
		TCExport static void registerRelease(TCObject*);
		TCExport static void processReleases(void);
		TCExport ~TCAutoreleasePool(void);
#if defined WIN32 && defined _DEBUG && defined __THIS_IS_NOT_DEFINED
		TCExport static void registerDelete(TCObject*);
#endif // WIN32 && _DEBUG
	protected:
		typedef struct PoolEntry
		{
			TCObject* value;
			int count;
			PoolEntry* next;
		} PoolEntry;

		TCExport void poolRegisterRelease(TCObject*);
		TCExport void poolProcessReleases(void);

		PoolEntry pool[256];
		bool haveReleases;
#if defined WIN32 && defined _DEBUG && defined __THIS_IS_NOT_DEFINED
		TCExport void poolRegisterDelete(TCObject*);
		TCArray* deletions;
#endif // WIN32 && _DEBUG

		static TCAutoreleasePool* currentPool;

		static class TCAutoreleasePoolCleanup
		{
		public:
			~TCAutoreleasePoolCleanup(void);
		} poolCleanup;
		friend class TCAutoreleasePoolCleanup;
};

#endif
