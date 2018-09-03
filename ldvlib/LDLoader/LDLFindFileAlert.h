#ifndef __LDLFINDFILEALERT_H__
#define __LDLFINDFILEALERT_H__

#include <TCFoundation/TCAlert.h>
#include <stdlib.h>

class LDLFindFileAlert: public TCAlert
{
public:
	LDLFindFileAlert(const char *filename);
	void setFilename(const char *value);
	const char *getFilename(void) { return m_filename; }
	void setFileFound(bool value) { m_fileFound = value; }
	bool getFileFound(void) { return m_fileFound; }
	void setPartFlag(bool value) { m_part = value; }
	bool getPartFlag(void) { return m_part; }

	static const char *alertClass(void) { return "LDLFindFileAlert"; }
protected:
	virtual ~LDLFindFileAlert(void);
	virtual void dealloc(void);

	char *m_filename;
	bool m_fileFound;
	bool m_part;
};

#endif // __LDLFINDFILEALERT_H__
