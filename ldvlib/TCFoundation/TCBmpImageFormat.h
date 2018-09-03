#ifndef __TCBMPIMAGEFORMAT_H__
#define __TCBMPIMAGEFORMAT_H__

#include <TCFoundation/TCImageFormat.h>

#define BMP_FILE_HEADER_SIZE 14
#define BMP_INFO_HEADER_SIZE 40
#define BMP_HEADER_SIZE (BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE)

class TCExport TCBmpImageFormat : public TCImageFormat
{
	public:
		TCBmpImageFormat(void);

		virtual bool checkSignature(const TCByte *data, long length);
		virtual bool checkSignature(FILE *file);
		virtual bool loadData(TCImage *image, TCByte *data, long length);
		virtual bool loadFile(TCImage *image, FILE *file);
		virtual bool saveFile(TCImage *image, FILE *file);
		static int writeHeader(int width, int height, TCByte *buf);
	protected:
		virtual ~TCBmpImageFormat(void);
		virtual void dealloc(void);
		static bool readValue(FILE *file, unsigned short &value);
		static bool readValue(FILE *file, unsigned long &value);
		static bool readValue(FILE *file, long &value);
		static bool readFileHeader(TCImage *image, FILE *file);
		static bool readInfoHeader(TCImage *image, FILE *file);
		virtual bool readImageData(TCImage *image, FILE *file);
		static int writeValue(TCByte *buf, unsigned short value, int offset);
		static int writeValue(TCByte *buf, unsigned long value, int offset);
		static int writeValue(TCByte *buf, long value, int offset);
		//static bool writeValue(FILE *file, unsigned short value);
		//static bool writeValue(FILE *file, unsigned long value);
		//static bool writeValue(FILE *file, long value);
		static int writeFileHeader(int width, int height, TCByte *buf);
		static int writeInfoHeader(int width, int height, TCByte *buf, int offset);
		//static bool writeFileHeader(TCImage *image, FILE *file);
		//static bool writeInfoHeader(TCImage *image, FILE *file);
		virtual bool writeImageData(TCImage *image, FILE *file);
};

#endif // __TCBMPIMAGEFORMAT_H__
