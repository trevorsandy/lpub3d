#ifndef __TCTYPEDOBJECTARRAY_H__
#define __TCTYPEDOBJECTARRAY_H__

#include <TCFoundation/TCObjectArray.h>

#if defined(WIN32) && !defined(_QT)
#pragma warning ( disable: 4710 )
#endif // WIN32 && NOT _QT

template <class Type> class TCTypedObjectArray : public TCObjectArray
{
	public:
		explicit TCTypedObjectArray(unsigned int count = 0)
			:TCObjectArray(count) {}
		TCTypedObjectArray(const TCTypedObjectArray<Type> &other)
			:TCObjectArray(other) {}
		virtual TCObject *copy(void) const
		{
			return new TCTypedObjectArray<Type>(*this);
		}

		void addObject(Type* object)
			{ TCObjectArray::addObject(object); }
		void insertObject(Type* object, unsigned int index = 0)
			{ TCObjectArray::insertObject(object, index); }
		int replaceObject(Type* object, unsigned int index)
			{ return TCObjectArray::replaceObject(object, index); }
		int indexOfObject(Type* object) const
			{ return TCObjectArray::indexOfObject(object); }
		int indexOfObjectIdenticalTo(Type* object) const
			{ return TCObjectArray::indexOfObjectIdenticalTo(object); }
		int removeObject(Type* object)
			{ return TCObjectArray::removeObject(object); }
		int removeObjectIdenticalTo(Type* object)
			{ return TCObjectArray::removeObjectIdenticalTo(object); }
		int removeObjectAtIndex(int index)
			{ return TCObjectArray::removeObjectAtIndex(index); }
		Type* objectAtIndex(unsigned int index)
			{ return (Type*)TCObjectArray::objectAtIndex(index); }
		const Type* objectAtIndex(unsigned int index) const
			{ return (Type*)TCObjectArray::objectAtIndex(index); }
		Type* operator[](unsigned int index)
			{ return (Type*)TCObjectArray::objectAtIndex(index); }
		const Type* operator[](unsigned int index) const
			{ return (Type*)TCObjectArray::objectAtIndex(index); }
	protected:
};

#endif // __TCTYPEDOBJECTARRAY_H__
