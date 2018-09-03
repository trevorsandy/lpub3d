#ifndef __TCDICTIONARY_H__
#define __TCDICTIONARY_H__

#include <TCFoundation/TCObject.h>

class TCObjectArray;
class TCSortedStringArray;

class TCExport TCDictionary: public TCObject
{
	public:
		explicit TCDictionary(int = 1);
		virtual void setObjectForKey(TCObject*, const char*);
		/*virtual*/ TCObject* objectForKey(const char*);
		TCObjectArray *allObjects(void) { return objects; }
		TCSortedStringArray *allKeys(void) { return keys; }
		virtual int removeObjectForKey(const char*);
		virtual void removeAll(void);
		virtual int isCaseSensitve(void);
		virtual TCObject *copy(void) const;
	protected:
		virtual ~TCDictionary(void);
		virtual void dealloc(void);
		virtual int indexOfKey(const char*);

		TCObjectArray* objects;
		TCSortedStringArray* keys;
	private:
		TCDictionary(bool) {}
};

#endif // __TCDICTIONARY_H__
