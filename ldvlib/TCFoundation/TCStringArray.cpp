#include "TCStringArray.h"
#include "mystring.h"

#include <string.h>
#include <stdio.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

TCStringArray::TCStringArray(unsigned int allocated, int caseSensitive)
			  :TCArray<>(allocated),
			   caseSensitive(caseSensitive)
{
#ifdef _LEAK_DEBUG
	strcpy(className, "TCStringArray");
#endif
}

TCStringArray::TCStringArray(const TCCharStarStar items, int numItems,
							 int caseSensitive)
			  :TCArray<>(numItems),
			   caseSensitive(caseSensitive)
{
#ifdef _LEAK_DEBUG
	strcpy(className, "TCStringArray");
#endif
	int i;

	for (i = 0; i < numItems; i++)
	{
		addString(items[i]);
	}
}

TCStringArray::~TCStringArray(void)
{
}

void TCStringArray::dealloc(void)
{
	for (unsigned int i = 0; i < count; i++)
	{
		delete[] (char*)(items[i]);
	}
	TCArray<>::dealloc();
}

int TCStringArray::addString(const char* newString)
{
	addItem(copyString(newString));
	return count - 1;
}

void TCStringArray::insertString(const char* newString, unsigned int index)
{
	insertItem(copyString(newString), index);
}

int TCStringArray::replaceString(const char* newString, unsigned int index)
{
	if (index < count)
	{
		delete[] (char*)(items[index]);
		items[index] = (void*)copyString(newString);
		return 1;
	}
	else
	{
		return 0;
	}
}

int TCStringArray::appendString(const char* newSuffix, unsigned int index)
{
	if (index < count)
	{
		char *currentString = stringAtIndex(index);
		char *newString = new char[strlen(currentString) + strlen(newSuffix) +
			1];

		strcpy(newString, currentString);
		strcat(newString, newSuffix);
		delete[] currentString;
		items[index] = newString;
		return 1;
	}
	else
	{
		return 0;
	}
}

int TCStringArray::indexOfString(const char* string)
{
	for (unsigned int i = 0; i < count; i++)
	{
		if (string && items[i])
		{
			if ((caseSensitive && strcmp((char*)items[i], string) == 0) ||
				(!caseSensitive && strcasecmp((char*)items[i], string) == 0))
			{
				return i;
			}
		}
		else if (!string && !items[i])
		{
			return i;
		}
	}
	return -1;
}

int TCStringArray::removeString(const char* string)
{
	return removeStringAtIndex(indexOfString(string));
}

int TCStringArray::removeStringAtIndex(int index)
{
	if (index >= 0 && (unsigned)index < count)
	{
		delete[] stringAtIndex(index);
		return removeItemAtIndex(index);
	}
	else
	{
		return 0;
	}
}

void TCStringArray::removeAll(void)
{
	while (count)
	{
		removeStringAtIndex(count - 1);
	}
}

char* TCStringArray::operator[](unsigned int index)
{
	return stringAtIndex(index);
}

const char* TCStringArray::operator[](unsigned int index) const
{
	return stringAtIndex(index);
}

const char* TCStringArray::stringAtIndex(unsigned int index) const
{
	return (const char*)((TCStringArray *)this)->itemAtIndex(index);
}

char* TCStringArray::stringAtIndex(unsigned int index)
{
	return (char*)itemAtIndex(index);
}

int TCStringArray::readFile(const char* filename)
{
	FILE* file = ucfopen(filename, "r");

	if (file)
	{
		char buf[1024];

		while (1)
		{
			int length;

			if (!fgets(buf, 1024, file))
			{
				fclose(file);
				return 1;
			}
			length = (int)strlen(buf);
			if (buf[length - 1] != '\n')
			{
				fprintf(stderr, "Line too long.\n");
				return 0;
			}
			buf[--length] = 0;
			if (length && buf[length - 1] == '\r')
			{
				buf[--length] = 0;
			}
			addString(buf);
		}
	}
	else
	{
		return 0;
	}
}

void TCStringArray::copyContents(TCStringArray *otherStringArray) const
{
	unsigned int i;

	for (i = 0; i < count; i++)
	{
		otherStringArray->items[i] = copyString((char*)items[i]);
		otherStringArray->count = count;
	}
}

TCObject *TCStringArray::copy(void) const
{
	TCStringArray *newStringArray = new TCStringArray(count, caseSensitive);

	copyContents(newStringArray);
	return newStringArray;
}
