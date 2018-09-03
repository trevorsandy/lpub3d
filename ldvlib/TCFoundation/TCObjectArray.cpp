#include "TCObjectArray.h"
#include "mystring.h"

#include <string.h>
#include <stdio.h>

#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG

TCObjectArray::TCObjectArray(unsigned int allocated)
				  :TCArray<>(allocated)
{
	if (allocated)
	{
		memset(items, 0, allocated * sizeof(void *));
	}
#ifdef _LEAK_DEBUG
	strcpy(className, "TCObjectArray");
#endif
}

TCObjectArray::TCObjectArray(const TCObjectArray &other)
	:TCArray<>(other.count)
{
	unsigned int i;

	count = allocated;
	for (i = 0; i < count; i++)
	{
		TCObject *object = (TCObject*)other.items[i];

		if (object)
		{
			TCObject *newObject = object->copy();
			items[i] = newObject;
		}
		else
		{
			items[i] = NULL;
		}
	}
}

TCObjectArray::~TCObjectArray(void)
{
}

void TCObjectArray::dealloc(void)
{
	for (unsigned int i = 0; i < count; i++)
	{
		TCObject::release(objectAtIndex(i));
	}
	TCArray<>::dealloc();
}

void TCObjectArray::addObject(TCObject* newObject)
{
	insertObject(newObject, count);
}

int TCObjectArray::replaceObject(TCObject* newObject, unsigned int index)
{
	if (index < count)
	{
		TCObject* oldObject = objectAtIndex(index);

		if (oldObject != newObject)
		{
			TCObject::release(oldObject);
			items[index] = newObject;
			TCObject::retain(newObject);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

void TCObjectArray::insertObject(TCObject* newObject, unsigned int index)
{
	TCObject::retain(newObject);
	insertItem(newObject, index);
}

int TCObjectArray::indexOfObjectIdenticalTo(TCObject* object) const
{
	return indexOfItem(object);
}

int TCObjectArray::indexOfObject(TCObject* object) const
{
	for (unsigned int i = 0; i < count; i++)
	{
		if (object && items[i])
		{
			if (object->isEqual(*((TCObject*)(items[i]))))
			{
				return i;
			}
		}
		else if (!object && !items[i])
		{
			return i;
		}
	}
	return -1;
}

int TCObjectArray::removeObject(TCObject* object)
{
	return removeObjectAtIndex(indexOfObject(object));
}

int TCObjectArray::removeObjectIdenticalTo(TCObject* object)
{
	return removeObjectAtIndex(indexOfObjectIdenticalTo(object));
}

int TCObjectArray::removeObjectAtIndex(int index)
{
	if (index >= 0 && (unsigned)index < count)
	{
		TCObject::release(objectAtIndex(index));
		return removeItemAtIndex(index);
	}
	else
	{
		return 0;
	}
}

void TCObjectArray::removeAll(void)
{
	while (count)
	{
		removeObjectAtIndex(count - 1);
	}
}

TCObject* TCObjectArray::objectAtIndex(unsigned int index)
{
	return (TCObject*)itemAtIndex(index);
}

const TCObject* TCObjectArray::objectAtIndex(unsigned int index) const
{
	return (TCObject*)itemAtIndex(index);
}

TCObject* TCObjectArray::operator[](unsigned int index)
{
	return objectAtIndex(index);
}

const TCObject* TCObjectArray::operator[](unsigned int index) const
{
	return objectAtIndex(index);
}

TCObject *TCObjectArray::copy(void) const
{
	return new TCObjectArray(*this);
}

//void TCObjectArray::addItem(void* newItem)
//{
//	TCArray::addItem(newItem);
//}
//
//void TCObjectArray::insertItem(void* newItem, unsigned int index)
//{
//	TCArray::insertItem(newItem, index);
//}
//
//int TCObjectArray::replaceItem(void* newItem, unsigned int index)
//{
//	return TCArray::replaceItem(newItem, index);
//}
//
//int TCObjectArray::indexOfItem(void* item) const
//{
//	return TCArray::indexOfItem(item);
//}
//
//int TCObjectArray::removeItem(void* item)
//{
//	return TCArray::removeItem(item);
//}
//
//int TCObjectArray::removeItem(int index)
//{
//	return TCArray::removeItem(index);
//}
//
//void* TCObjectArray::itemAtIndex(unsigned int index)
//{
//	return TCArray::itemAtIndex(index);
//}
//
//const void* TCObjectArray::itemAtIndex(unsigned int index) const
//{
//	return TCArray::itemAtIndex(index);
//}

void TCObjectArray::sort(void)
{
	sortUsingFunction(sortFunction);
}

int TCObjectArray::sortFunction(const void *left, const void *right)
{
	return (*(const TCObject **)left)->compare(*(const TCObject **)right);
}
