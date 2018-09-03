#include "TCSortedStringArray.h"
#include "mystring.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

TCSortedStringArray::TCSortedStringArray(unsigned int allocated,
										 int caseSensitive)
				  :TCStringArray(allocated, caseSensitive)
{
#ifdef _LEAK_DEBUG
	strcpy(className, "TCSortedStringArray");
#endif
}

TCSortedStringArray::~TCSortedStringArray(void)
{
}

int TCSortedStringArray::addString(const char* newString)
{
	int i;
	int step = (count + 1) / 2;
	int (*compare)(const char*, const char*);
	int comparison;

	if (caseSensitive)
	{
		compare = strcmp;
	}
	else
	{
		compare = strcasecmp;
	}
	if (count)
	{
		i = step - 1;
		while (step > 1 && i >= 0 && i < (int)count)
		{
			char *item = (char*)items[i];

			comparison = compare(item, newString);
			step = (step + 1) / 2;
			if (comparison < 0)
			{
				i += step;
				if (i >= (int)count)
				{
					i = count - 1;
				}
			}
			else if (comparison > 0)
			{
				i -= step;
				if (i < 0)
				{
					i = 0;
				}
			}
			else
			{
				return -1;
			}
		}
		if (i < 0)
		{
			i = 0;
		}
		else if (i >= (int)count)
		{
			i = count;
		}
		else
		{
			do
			{
				char *item = (char*)items[i];

				comparison = compare(item, newString);
				if (comparison < 0)
				{
					i++;
				}
				else if (comparison == 0)
				{
					return -1;
				}
/*
				else if (i > 0 && compare((char*)items[i - 1], newString) > 0)
				{
					i--;
					comparison = -1;
				}
*/
			}
			while (comparison < 0 && i < (int)count);
		}
	}
	else
	{
		i = 0;
	}
	insertString(newString, i);
/*
	if (count == 10)
	{
		for (int j = 0; j < (int)count; j++)
		{
			printf("%s\n", items[j]);
		}
	}
*/
	return i;
/*
	int i;
	int (*compare)(const char*, const char*);

	if (caseSensitive)
	{
		compare = strcmp;
	}
	else
	{
		compare = strcasecmp;
	}
	for (i = 0; i < (int)count && compare((char*)items[i], newString) < 0; i++)
		;
	insertString(newString, i);
	return i;
*/
}

void TCSortedStringArray::insertString(const char* newString, unsigned int index)
{
	TCStringArray::insertString(newString, index);
}

int TCSortedStringArray::replaceString(const char* newString, unsigned int index)
{
	return TCStringArray::replaceString(newString, index);
}

int caseSensitiveCompare(const void* left, const void* right)
{
	char** string1 = (char**)left;
	char** string2 = (char**)right;

	return strcmp(*string1, *string2);
}

int caseInsensitiveCompare(const void* left, const void* right)
{
	char** string1 = (char**)left;
	char** string2 = (char**)right;

	return strcasecmp(*string1, *string2);
}

int TCSortedStringArray::indexOfString(const char* string)
{
	int i;
	int step = (count + 1) / 2;
	int (*compare)(const char*, const char*);
	int comparison;

	if (caseSensitive)
	{
		compare = strcmp;
	}
	else
	{
		compare = strcasecmp;
	}
	if (count)
	{
		i = step - 1;
		while (step > 1 && i >= 0 && i < (int)count)
		{
			char *item = (char*)items[i];

			comparison = compare(item, string);
			step = (step + 1) / 2;
			if (comparison < 0)
			{
				i += step;
				if (i >= (int)count)
				{
					i = count - 1;
				}
			}
			else if (comparison > 0)
			{
				i -= step;
				if (i < 0)
				{
					i = 0;
				}
			}
			else
			{
				return i;
			}
		}
		if (i >= 0 && i < (int)count)
		{
			do
			{
				comparison = compare((char*)items[i], string);
				if (comparison < 0)
				{
					i++;
				}
				else if (comparison == 0)
				{
					return i;
				}
/*
				else if (i > 0 && compare((char*)items[i - 1], string) > 0)
				{
					i--;
					comparison = -1;
				}
*/
			}
			while (comparison < 0 && i < (int)count);
		}
	}
	return -1;
/*
	int (*compare)(const void*, const void*);

	if (!items)
	{
		return -1;
	}
	if (caseSensitive)
	{
		compare = caseSensitiveCompare;
	}
	else
	{
		compare = caseInsensitiveCompare;
	}
	void* match = bsearch(&string, items, count, sizeof(char*),
		(int (*)(const void*, const void*))compare);

	if (match)
	{
		return (void**)match - items;
	}
	else
	{
		return -1;
	}
*/
}

TCObject *TCSortedStringArray::copy(void) const
{
	TCStringArray *newSortedStringArray =
		new TCSortedStringArray(count, caseSensitive);

	copyContents(newSortedStringArray);
	return newSortedStringArray;
}
