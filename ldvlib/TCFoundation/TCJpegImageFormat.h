#ifndef __TCJPEGIMAGEFORMAT_H__
#define __TCJPEGIMAGEFORMAT_H__
#ifndef NO_JPG_IMAGE_FORMAT

#include <TCFoundation/TCImageFormat.h>
#include <setjmp.h>

struct jpeg_decompress_struct;
struct jpeg_compress_struct;
struct jpeg_error_mgr;
struct jpeg_common_struct;

class TCExport TCJpegImageFormat : public TCImageFormat
{
public:
	TCJpegImageFormat(void);

	virtual bool checkSignature(const TCByte *data, long length);
	virtual bool checkSignature(FILE *file);
	virtual bool loadData(TCImage *image, TCByte *data, long length);
	virtual bool loadFile(TCImage *image, FILE *file);
	virtual bool saveFile(TCImage *image, FILE *file);
	virtual TCImageOptions *newCompressionOptions(void);

protected:
	virtual ~TCJpegImageFormat(void);
	virtual void dealloc(void);
	bool setup(jpeg_decompress_struct &cinfo, jpeg_error_mgr &jerr);
	bool setup(jpeg_compress_struct &cinfo, jpeg_error_mgr &jerr);
	void errorExit(jpeg_common_struct &cinfo);
	virtual bool load(TCImage *image, FILE *file, TCByte *data, long length);

	static void staticErrorExit(jpeg_common_struct *cinfo);

	TCImage *image;
	unsigned long imageWidth;
	unsigned long imageHeight;
	jmp_buf jumpBuf;
};

#endif // NO_JPG_IMAGE_FORMAT
#endif // __TCJPEGIMAGEFORMAT_H__
