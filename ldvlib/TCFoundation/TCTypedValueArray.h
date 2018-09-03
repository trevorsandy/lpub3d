#ifndef __TCTYPEDVALUEARRAY_H__
#define __TCTYPEDVALUEARRAY_H__

#include <TCFoundation/TCArray.h>

#if defined(WIN32) && !defined(_QT)
#pragma warning ( disable: 4710 )
#endif // WIN32 && NOT _QT

template <class Type> class TCTypedValueArray : public TCArray<Type>
{
	public:
		explicit TCTypedValueArray(unsigned int count = 0)
			:TCArray<Type>(count) {}
		TCTypedValueArray(const TCTypedValueArray<Type> &other)
			:TCArray<Type>(other) {}

		virtual TCObject *copy(void) const
		{
			return new TCTypedValueArray<Type>(*this);
		}
		void addValue(Type value)
			{ TCArray<Type>::addItem(value); }
		void insertValue(Type value, unsigned int index = 0)
			{ TCArray<Type>::insertItem(value, index); }
		int replaceValue(Type value, unsigned int index)
			{ return TCArray<Type>::replaceItem(value, index); }
		int indexOfValue(Type value) const
			{ return TCArray<Type>::indexOfItem(value); }
		int removeValueAtIndex(int index)
			{ return TCArray<Type>::removeItemAtIndex(index); }
		int removeValues(int index, int numValues)
			{ return TCArray<Type>::removeItems(index, numValues); }
		Type valueAtIndex(unsigned int index)
			{ return (Type)TCArray<Type>::itemAtIndex(index); }
		const Type valueAtIndex(unsigned int index) const
			{ return (const Type)TCArray<Type>::itemAtIndex(index); }
		Type operator[](unsigned int index)
			{ return TCArray<Type>::itemAtIndex(index); }
		const Type operator[](unsigned int index) const
			{ return (const Type)TCArray<Type>::itemAtIndex(index); }
		Type *getValues(void)
			{ return (Type *)TCArray<Type>::getItems(); }
		const Type *getValues(void) const
			{ return (const Type *)TCArray<Type>::getItems(); }
		void sort(void)
            { TCArray<Type>::sortUsingFunction((TCArraySortFunction)valueSortFunction); }
	protected:
		~TCTypedValueArray(void) {}
		static int valueSortFunction(const void *left, const void *right)
		{
			Type leftVal = *(Type*)left;
			Type rightVal = *(Type*)right;

			if (leftVal < rightVal)
			{
				return -1;
			}
			else if (leftVal > rightVal)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
};

typedef TCTypedValueArray<int> TCIntArray;
typedef TCTypedValueArray<TCULong> TCULongArray;

#endif // __TCTYPEDVALUEARRAY_H__
