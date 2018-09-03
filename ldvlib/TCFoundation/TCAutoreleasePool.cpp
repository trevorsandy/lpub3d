#include "TCAutoreleasePool.h"
#include "TCObject.h"
#include "TCArray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

TCAutoreleasePool* TCAutoreleasePool::currentPool = NULL;

TCAutoreleasePool::TCAutoreleasePoolCleanup TCAutoreleasePool::poolCleanup;

TCAutoreleasePool::TCAutoreleasePoolCleanup::~TCAutoreleasePoolCleanup(void)
{
	delete TCAutoreleasePool::currentPool;
	TCAutoreleasePool::currentPool = NULL;
}

TCAutoreleasePool::TCAutoreleasePool(void)
				  :haveReleases(false)
#if defined WIN32 && defined _DEBUG && defined __THIS_IS_NOT_DEFINED
				  ,deletions(new TCArray)
#endif // WIN32 && _DEBUG
{
	assert(currentPool == NULL);
	currentPool = this;
	memset(pool, 0, sizeof(pool));
}

TCAutoreleasePool::~TCAutoreleasePool(void)
{
	poolProcessReleases();
	currentPool = NULL;
}

void TCAutoreleasePool::poolRegisterRelease(TCObject* theObject)
{
//	int bucket = ((unsigned)theObject & 0xFF00) >> 8;
	int bucket = 0;

	haveReleases = true;
	if (pool[bucket].value == NULL)
	{
		pool[bucket].value = theObject;
		pool[bucket].count = 1;
	}
	else
	{
		PoolEntry* entry = pool + bucket;

		while (entry->value != theObject && entry->next)
		{
			entry = entry->next;
		}
		if (entry->value == theObject)
		{
			entry->count++;
		}
		else
		{
			entry->next = new PoolEntry;
			entry = entry->next;
			entry->value = theObject;
			entry->count = 1;
			entry->next = NULL;
		}
	}
}

#if defined WIN32 && defined _DEBUG && defined __THIS_IS_NOT_DEFINED

void TCAutoreleasePool::poolRegisterDelete(TCObject* theObject)
{
	deletions->addItem(theObject);
}

void TCAutoreleasePool::registerDelete(TCObject* theObject)
{
	if (!currentPool)
	{
		currentPool = new TCAutoreleasePool;
	}
	currentPool->poolRegisterDelete(theObject);
}

#endif // WIN32 && _DEBUG

void TCAutoreleasePool::registerRelease(TCObject* theObject)
{
	if (!currentPool)
	{
		currentPool = new TCAutoreleasePool;
	}
	currentPool->poolRegisterRelease(theObject);
}

void TCAutoreleasePool::processReleases(void)
{
	if (!currentPool)
	{
		currentPool = new TCAutoreleasePool;
	}
	currentPool->poolProcessReleases();
}

void TCAutoreleasePool::poolProcessReleases(void)
{
	if (haveReleases)
	{
		PoolEntry* page;

		for (page = pool; page < pool + 256; page++)
		{
			if (page->value)
			{
				PoolEntry* entry = page;

				while (entry)
				{
					PoolEntry* temp;

					entry->value->retainCount -= entry->count;
					assert(entry->value->retainCount >= 0);
					if (entry->value->retainCount == 0)
					{
						entry->value->dealloc();
					}
					temp = entry;
					entry = entry->next;
					if (temp == page)
					{
						temp->value = 0;
						temp->next = NULL;
					}
					else
					{
						delete temp;
					}
				}
			}
		}
		haveReleases = false;
	}
#if defined WIN32 && defined _DEBUG && defined __THIS_IS_NOT_DEFINED
	if (deletions->getCount())
	{
		int i;

		for (i = deletions->getCount() - 1; i >= 0; i--)
		{
			TCObject* theObject = (TCObject*)deletions->itemAtIndex(i);

			delete theObject;
			deletions->removeItemAtIndex(i);
		}
	}
#endif // WIN32 && _DEBUG
}
