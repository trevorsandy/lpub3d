#ifndef __TCSTRINGARRAY_H__
#define __TCSTRINGARRAY_H__

#include <TCFoundation/TCArray.h>

#if defined(__APPLE__)
#	define stricmp strcasecmp
#endif

typedef char ** TCCharStarStar;

class TCExport TCStringArray : public TCArray<>
{
	public:
		explicit TCStringArray(unsigned int = 0, int = 1);
		TCStringArray(const TCCharStarStar items, int numItems,
			int caseSensitive = 1);

		virtual int addString(const char*);
		virtual void insertString(const char*, unsigned int = 0);
		virtual int replaceString(const char*, unsigned int);
		virtual int appendString(const char*, unsigned int);
		virtual int indexOfString(const char*);
		virtual int removeString(const char*);
		virtual int removeStringAtIndex(int);
		virtual void removeAll(void);
		virtual const char* stringAtIndex(unsigned int) const;
		virtual char* stringAtIndex(unsigned int);
		virtual char* operator[](unsigned int);
		virtual const char* operator[](unsigned int) const;
		virtual int readFile(const char*);
		int isCaseSensitive(void) { return caseSensitive; }
		virtual TCObject *copy(void) const;
	protected:
		virtual ~TCStringArray(void);
		virtual void dealloc(void);
		virtual void copyContents(TCStringArray *otherStringArray) const;

		int caseSensitive;
};

#endif // __TCSTRINGARRAY_H__
