#ifndef __TCPNGIMAGEFORMAT_H__
#define __TCPNGIMAGEFORMAT_H__
#ifndef NO_PNG_IMAGE_FORMAT
#ifdef _AIX
#include <stdlib.h>
#endif

#include <TCFoundation/TCImageFormat.h>

#include <png.h>

class TCExport TCPngImageFormat : public TCImageFormat
{
public:
	TCPngImageFormat(void);

	virtual bool checkSignature(const TCByte *data, long length);
	virtual bool checkSignature(FILE *file);
	virtual bool loadData(TCImage *image, TCByte *data, long length);
	virtual bool loadFile(TCImage *image, FILE *file);
	virtual bool saveFile(TCImage *image, FILE *file);
protected:
	virtual ~TCPngImageFormat(void);
	virtual void dealloc(void);
	bool setup(void);
	bool setupProgressive(void);
	void infoCallback(void);
	void rowCallback(png_bytep rowData, png_uint_32 rowNum, int pass);
	void errorCallback(png_const_charp msg);
	float passToFraction(int pass);

	static void staticErrorCallback(png_structp pngPtr, png_const_charp msg);
	static void staticInfoCallback(png_structp pngPtr, png_infop infoPtr);
	static void staticRowCallback(png_structp pngPtr, png_bytep newRow,
		png_uint_32 rowNum, int pass);

	png_structp pngPtr;
	png_infop infoPtr;
	png_uint_32 imageWidth;
	png_uint_32 imageHeight;
	TCImage *image;
	char **commentData;
	int commentDataCount;
	jmp_buf jumpBuf;
	bool canceled;
	int numPasses;
};

#endif // NO_PNG_IMAGE_FORMAT
#endif // __TCPNGIMAGEFORMAT_H__
