#ifndef __TCOBJECTARRAY_H__
#define __TCOBJECTARRAY_H__

#include <TCFoundation/TCArray.h>

class TCExport TCObjectArray : public TCArray<>
{
	public:
		explicit TCObjectArray(unsigned int = 0);
		TCObjectArray (const TCObjectArray &other);

		void addObject(TCObject*);
		void insertObject(TCObject*, unsigned int = 0);
		int replaceObject(TCObject*, unsigned int);
		int indexOfObject(TCObject*) const;
		int indexOfObjectIdenticalTo(TCObject*) const;
		int removeObject(TCObject*);
		int removeObjectIdenticalTo(TCObject*);
		virtual int removeObjectAtIndex(int);
		virtual void removeAll(void);
		/*virtual*/ TCObject* objectAtIndex(unsigned int);
		/*virtual*/ const TCObject* objectAtIndex(unsigned int) const;
		/*virtual*/ TCObject* operator[](unsigned int);
		/*virtual*/ const TCObject* operator[](unsigned int) const;
		virtual TCObject *copy(void) const;
		virtual void sort(void);
	protected:
		virtual ~TCObjectArray(void);
		virtual void dealloc(void);
		//virtual void addItem(void*);
		//virtual void insertItem(void*, unsigned int = 0);
		//virtual int replaceItem(void*, unsigned int);
		//virtual int indexOfItem(void*) const;
		//virtual int removeItem(void*);
		//virtual int removeItem(int);
		//virtual void* itemAtIndex(unsigned int);
		//virtual const void* itemAtIndex(unsigned int) const;
		static int sortFunction(const void *left, const void *right);
};


#endif // __TCOBJECTARRAY_H__
