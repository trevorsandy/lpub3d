#include "TCArray.h"
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//
//#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
//#define new DEBUG_CLIENTBLOCK
//#endif // _DEBUG
//
//TCArray::TCArray(unsigned int allocated)
//		  :items(NULL),
//			count(0),
//			allocated(allocated)
//{
//	if (allocated)
//	{
//		items = new void*[allocated];
//	}
//#ifdef _LEAK_DEBUG
//	strcpy(className, "TCArray");
//#endif
//}
//
//TCArray::TCArray(const TCArray &other)
//	:items(NULL),
//	count(other.count),
//	allocated(other.count)
//{
//	if (allocated)
//	{
//		items = new void*[count];
//	}
//	memcpy(items, other.items, count * sizeof(void*));
//}
//
//TCArray::~TCArray(void)
//{
//}
//
//void TCArray::dealloc(void)
//{
//	delete items;
//	TCObject::dealloc();
//}
//
//void TCArray::addItem(void* newItem)
//{
//	insertItem(newItem, count);
//}
//
//int TCArray::setCapacity(unsigned newCapacity)
//{
//	if (newCapacity >= count)
//	{
//		void** newItems;
//
//		allocated = newCapacity;
//		newItems = new void*[allocated];
//		if (count)
//		{
//			memcpy(newItems, items, count * sizeof (void*));
//		}
//		delete items;
//		items = newItems;
//		return 1;
//	}
//	else
//	{
//		return 0;
//	}
//}
//
//void TCArray::insertItem(void* newItem, unsigned int index)
//{
//	int expanded = 0;
//
//	if (index > count)
//	{
//		return;
//	}
//	if (count + 1 > allocated)
//	{
//		void** newItems;
//
//		expanded = 1;
//		if (allocated)
//		{
//			allocated *= 2;
//		}
//		else
//		{
//			allocated = 1;
//		}
//		newItems = new void*[allocated];
//		if (index)
//		{
//			memcpy(newItems, items, index * sizeof (void*));
//		}
//		if (index < count)
//		{
//			memcpy(newItems + index + 1, items + index,
//			 (count - index) * sizeof (void*));
//		}
//		delete items;
//		items = newItems;
//	}
//	if (!expanded && index < count)
//	{
//		memmove(items + index + 1, items + index,
//		 (count - index) * sizeof (void*));
//	}
//	items[index] = newItem;
//	count++;
//}
//
//int TCArray::replaceItem(void* newItem, unsigned int index)
//{
//	if (index < count)
//	{
//		items[index] = newItem;
//		return 1;
//	}
//	else
//	{
//		return 0;
//	}
//}
//
//int TCArray::indexOfItem(void* item) const
//{
//	for (unsigned int i = 0; i < count; i++)
//	{
//		if (items[i] == item)
//		{
//			return i;
//		}
//	}
//	return -1;
//}
//
//int TCArray::removeItem(void* item)
//{
//	return removeItem(indexOfItem(item));
//}
//
//void TCArray::removeAll(void)
//{
//	while (count)
//	{
//		removeItem(count - 1);
//	}
//}
//
//int TCArray::removeItem(int index)
//{
//	if (index >= 0 && (unsigned)index < count)
//	{
//		count--;
//		if ((unsigned)index < count)
//		{
//			memmove(items + index, items + index + 1, (count - index) *
//			 sizeof(void*));
//		}
//		return 1;
//	}
//	else
//	{
//		return 0;
//	}
//}
//
//int TCArray::removeItems(int index, int numItems)
//{
//	int i;
//
//	for (i = index + numItems - 1; i >= index; i--)
//	{
//		if (!removeItem(i))
//		{
//			return 0;
//		}
//	}
//	return 1;
//}
//
//void* TCArray::itemAtIndex(unsigned int index)
//{
//	if (index < count)
//	{
//		return items[index];
//	}
//	else
//	{
//		return NULL;
//	}
//}
//
//const void* TCArray::itemAtIndex(unsigned int index) const
//{
//	if (index < count)
//	{
//		return items[index];
//	}
//	else
//	{
//		return NULL;
//	}
//}
//
//void TCArray::sortUsingFunction(TCArraySortFunction function)
//{
//	qsort(items, count, sizeof (void*), function);
//}
//
///*
//void* TCArray::operator[](unsigned int index)
//{
//	if (index < count)
//	{
//		return items[index];
//	}
//	else
//	{
//		return NULL;
//	}
//}
//*/
//
//TCObject *TCArray::copy(void) const
//{
//	return new TCArray(*this);
//}
