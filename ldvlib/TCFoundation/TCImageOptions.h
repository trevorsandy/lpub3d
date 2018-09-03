#ifndef __TCIMAGEOPTIONS_H__
#define __TCIMAGEOPTIONS_H__

#include <TCFoundation/TCObject.h>
#include <TCFoundation/TCStlIncludes.h>

class TCExport TCImageOptions : public TCObject
{
public:
	TCImageOptions(bool compress = true);
	const char *getFormatName(void) const { return m_formatName.c_str(); }
	virtual void save(void) {}

protected:
	virtual ~TCImageOptions(void);
	virtual void dealloc(void);

	std::string m_formatName;
	bool m_compress;
};

#endif // __TCIMAGEOPTIONS_H__
