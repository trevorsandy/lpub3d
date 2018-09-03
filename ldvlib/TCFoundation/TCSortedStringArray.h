#ifndef __TCSORTEDSTRINGARRAY_H__
#define __TCSORTEDSTRINGARRAY_H__

#include <TCFoundation/TCStringArray.h>

class TCExport TCSortedStringArray : public TCStringArray
{
	public:
		explicit TCSortedStringArray(unsigned int = 0, int = 1);

		virtual int addString(const char*);
		virtual int indexOfString(const char*);
		virtual TCObject *copy(void) const;
	protected:
		virtual ~TCSortedStringArray(void);
		virtual void insertString(const char*, unsigned int = 0);
		virtual int replaceString(const char*, unsigned int);
};

#endif // __TCSORTEDSTRINGARRAY_H__
