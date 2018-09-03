#ifndef __TCTYPEDPOINTERARRAY_H__
#define __TCTYPEDPOINTERARRAY_H__

#include <TCFoundation/TCArray.h>

#if defined(WIN32) && !defined(_QT)
#pragma warning ( disable: 4710 )
#endif // WIN32 && NOT _QT

template <class Type> class TCTypedPointerArray : public TCArray<>
{
	public:
		explicit TCTypedPointerArray(unsigned int count = 0)
			:TCArray<>(count) {}
		TCTypedPointerArray(const TCTypedPointerArray<Type> &other)
			:TCArray<>(other) {}

		virtual TCObject *copy(void) const
		{
			return new TCTypedPointerArray<Type>(*this);
		}
		void addPointer(Type pointer)
			{ TCArray<>::addItem(pointer); }
		void insertPointer(Type pointer, unsigned int index = 0)
			{ TCArray<>::insertItem(pointer, index); }
		int replacePointer(Type pointer, unsigned int index)
			{ return TCArray<>::replaceItem(pointer, index); }
		int indexOfPointer(Type pointer)
			{ return TCArray<>::indexOfItem(pointer); }
		int removePointer(Type pointer)
			{ return TCArray<>::removeItem(pointer); }
		int removePointerAtIndex(int index)
			{ return TCArray<>::removeItemAtIndex(index); }
		Type pointerAtIndex(unsigned int index)
			{ return (Type)TCArray<>::itemAtIndex(index); }
		Type operator[](unsigned int index)
			{ return (Type)TCArray<>::itemAtIndex(index); }
	protected:
};

#endif // __TCTYPEDPOINTERARRAY_H__
