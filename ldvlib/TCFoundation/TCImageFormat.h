#ifndef __TCIMAGEFORMAT_H__
#define __TCIMAGEFORMAT_H__

#if defined(WIN32) && defined(_M_X64)
// In 64-bit Win builds, the following warning shows up everywhere the jmp_buf
// variable is used in a class.  That's used by the PNG and JPEG libraries for
// error handling.
#pragma warning (disable: 4324)
#endif // WIN32

#include <TCFoundation/TCAlertSender.h>
#include <TCFoundation/TCImage.h>

class TCExport TCImageFormat : public TCAlertSender
{
	public:
		TCImageFormat(void);

		virtual bool checkSignature(const TCByte *data, long length) = 0;
		virtual bool checkSignature(const char *filename);
		virtual bool checkSignature(FILE *file) = 0;
		virtual bool loadData(TCImage *image, TCByte *data, long length) = 0;
		virtual bool loadFile(TCImage *image, const char *filename);
		virtual bool loadFile(TCImage *image, FILE *file) = 0;
		virtual bool saveFile(TCImage *image, const char *filename);
		virtual bool saveFile(TCImage *image, FILE *file) = 0;
		const char *getName(void) const { return name; }
		virtual void setProgressCallback(TCImageProgressCallback value,
			void *userData)
		{
			progressCallback = value;
			progressUserData = userData;
		}
	virtual TCImageOptions *newCompressionOptions(void);
	protected:
		virtual ~TCImageFormat(void);
		virtual void dealloc(void);
		virtual bool callProgressCallback(CUCSTR message, float progress);

		const char *name;
		TCImageProgressCallback progressCallback;
		void *progressUserData;
};

#endif // __TCIMAGEFORMAT_H__
