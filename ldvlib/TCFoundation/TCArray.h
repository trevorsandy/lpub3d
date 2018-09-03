#ifndef __TCARRAY_H__
#define __TCARRAY_H__

#include <TCFoundation/TCObject.h>
#include <TCFoundation/mystring.h>
#include <cstdlib>

typedef int (*TCArraySortFunction)(const void*, const void*);

template <class Type = void*>
class TCExport TCArray : public TCObject
{
public:
	explicit TCArray(unsigned int allocated = 0):
		items(NULL),
		count(0),
		allocated(allocated)
	{
		if (allocated)
		{
			items = new Type[allocated];
		}
#ifdef _LEAK_DEBUG
		strcpy(className, "TCArray");
#endif
	}

	TCArray (const TCArray &other):
		items(NULL),
		count(other.count),
		allocated(other.count)
	{
		if (allocated)
		{
			items = new Type[count];
		}
		memcpy(items, other.items, count * sizeof(Type));
	}

	virtual void addItem(Type newItem)
	{
		insertItem(newItem, count);
	}

	virtual void insertItem(Type newItem, unsigned int index = 0)
	{
		int expanded = 0;

		if (index > count)
		{
			return;
		}
		if (count + 1 > allocated)
		{
			Type* newItems;

			expanded = 1;
			if (allocated)
			{
				allocated *= 2;
			}
			else
			{
				allocated = 1;
			}
			newItems = new Type[allocated];
			if (index)
			{
				memcpy(newItems, items, index * sizeof (Type));
			}
			if (index < count)
			{
				memcpy(newItems + index + 1, items + index,
				 (count - index) * sizeof (Type));
			}
			delete[] items;
			items = newItems;
		}
		if (!expanded && index < count)
		{
			memmove(items + index + 1, items + index,
			 (count - index) * sizeof (Type));
		}
		items[index] = newItem;
		count++;
	}

	virtual int replaceItem(Type newItem, unsigned int index)
	{
		if (index < count)
		{
			items[index] = newItem;
			return 1;
		}
		else
		{
			return 0;
		}
	}

	virtual int indexOfItem(Type item) const
	{
		for (unsigned int i = 0; i < count; i++)
		{
			if (items[i] == item)
			{
				return i;
			}
		}
		return -1;
	}

	virtual int removeItem(Type item)
	{
		return removeItemAtIndex(indexOfItem(item));
	}

	virtual int removeItemAtIndex(int index)
	{
		if (index >= 0 && (unsigned)index < count)
		{
			count--;
			if ((unsigned)index < count)
			{
				memmove(items + index, items + index + 1, (count - index) *
					sizeof(Type));
			}
			return 1;
		}
		else
		{
			return 0;
		}
	}

	virtual int removeItems(int index, int numItems)
	{
		int i;

		for (i = index + numItems - 1; i >= index; i--)
		{
			if (!removeItemAtIndex(i))
			{
				return 0;
			}
		}
		return 1;
	}

	virtual void removeAll(void)
	{
		while (count)
		{
			removeItemAtIndex(count - 1);
		}
	}

	virtual Type itemAtIndex(unsigned int index)
	{
		if (index < count)
		{
			return items[index];
		}
		else
		{
			Type retValue;

			memset(&retValue, 0, sizeof(retValue));
			return retValue;
		}
	}

	virtual const Type itemAtIndex(unsigned int index) const
	{
		if (index < count)
		{
			return items[index];
		}
		else
		{
			Type retValue;

			memset(&retValue, 0, sizeof(retValue));
			return retValue;
		}
	}

	int getCount(void) const
	{
		return count;
	}

	virtual void shrinkToFit(void)
	{
		setCapacity(count);
	}

	virtual int setCapacity(unsigned newCapacity, bool updateCount = false, bool clear = false)
	{
		if (newCapacity >= count)
		{
			Type* newItems;

			allocated = newCapacity;
			newItems = new Type[allocated];
			if (count)
			{
				memcpy(newItems, items, count * sizeof (Type));
			}
			delete[] items;
			items = newItems;
			if (updateCount)
			{
				if (clear)
				{
					memset(&newItems[count], 0, (newCapacity - count) * sizeof (Type));
				}
				count = newCapacity;
			}
			return 1;
		}
		else
		{
			return 0;
		}
	}

	void sortUsingFunction(TCArraySortFunction function)
	{
		qsort(items, count, sizeof (Type), function);
	}

	Type *getItems(void)
	{
		return items;
	}

	const Type *getItems(void) const
	{
		return items;
	}

	virtual TCObject *copy(void) const
	{
		return new TCArray(*this);
	}

protected:
	virtual ~TCArray(void)
	{
	}

	virtual void dealloc(void)
	{
		delete[] items;
		TCObject::dealloc();
	}

	Type* items;
	unsigned int count;
	unsigned int allocated;
};

#endif // __TCARRAY_H__
