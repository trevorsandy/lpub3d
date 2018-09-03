#include "TCImageOptions.h"

#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif

TCImageOptions::TCImageOptions(bool compress /*= true*/):
m_compress(compress)
{
#ifdef _LEAK_DEBUG
	strcpy(className, "TCImageOptions");
#endif
}

TCImageOptions::~TCImageOptions(void)
{
}

void TCImageOptions::dealloc(void)
{
	TCObject::dealloc();
}

