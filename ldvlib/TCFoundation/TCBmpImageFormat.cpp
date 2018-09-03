#include "TCBmpImageFormat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

typedef unsigned short WORD;
typedef unsigned long DWORD;

TCBmpImageFormat::TCBmpImageFormat(void)
{
	name = "BMP";
#ifdef _LEAK_DEBUG
	strcpy(className, "TCBmpImageFormat");
#endif
}

TCBmpImageFormat::~TCBmpImageFormat(void)
{
}

void TCBmpImageFormat::dealloc(void)
{
	TCImageFormat::dealloc();
}

bool TCBmpImageFormat::checkSignature(const TCByte *data, long length)
{
	if (length >= 2)
	{
		return data[0] == 'B' && data[1] == 'M';
	}
	else
	{
		return false;
	}
}

bool TCBmpImageFormat::checkSignature(FILE *file)
{
	bool retValue = false;
	TCByte header[2];
	long filePos = ftell(file);

	if (fread(header, 1, 2, file) == 2)
	{
		retValue = header[0] == 'B' && header[1] == 'M';
	}
	fseek(file, filePos, SEEK_SET);
	return retValue;
}

bool TCBmpImageFormat::loadFile(TCImage *image, FILE *file)
{
	if (!readFileHeader(image, file))
	{
		return false;
	}
	if (!readInfoHeader(image, file))
	{
		return false;
	}
	return readImageData(image, file);
}

bool TCBmpImageFormat::loadData(TCImage * /*image*/, TCByte * /*data*/,
								long /*length*/)
{
	return false;
}

bool TCBmpImageFormat::readValue(FILE *file, unsigned short &value)
{
	TCByte buf[2];

	// Read the value in little endian format
	if (fread(buf, 1, 2, file) == 2)
	{
		value = (unsigned short)(buf[0] | ((short)buf[1] << 8));
		return true;
	}
	else
	{
		return false;
	}
}

bool TCBmpImageFormat::readValue(FILE *file, unsigned long &value)
{
	TCByte buf[4];

	// Read the value in little endian format
	if (fread(buf, 1, 4, file) == 4)
	{
		value = buf[0] | ((short)buf[1] << 8) | ((short)buf[2] << 16) | ((short)buf[3] << 24);
		return true;
	}
	else
	{
		return false;
	}
}

bool TCBmpImageFormat::readValue(FILE *file, long &value)
{
	unsigned long temp;

	if (readValue(file, temp))
	{
		value = (long)temp;
		return true;
	}
	else
	{
		return false;
	}
}

bool TCBmpImageFormat::readFileHeader(TCImage * /*image*/, FILE *file)
{
	WORD wTemp;
	DWORD dwTemp;

	if (!readValue(file, wTemp)) // 'BM'
	{
		return false;
	}
	if (!readValue(file, dwTemp))
	{
		return false;
	}
	if (!readValue(file, dwTemp)) // Reserved
	{
		return false;
	}
	if (!readValue(file, dwTemp))
	{
		return false;
	}
	return true;
}

bool TCBmpImageFormat::readInfoHeader(TCImage *image, FILE *file)
{
	int rowSize = image->roundUp(image->getWidth() * 3, 4);
	DWORD imageSize = rowSize * image->getHeight();
	DWORD dwTemp;
	long lTemp;
	WORD wTemp;
	long width, height;

	if (!readValue(file, dwTemp))
	{
		return false;
	}
	if (!readValue(file, width))
	{
		return false;
	}
	if (!readValue(file, height))
	{
		return false;
	}
	image->setSize((int)width, (int)height);
	if (!readValue(file, wTemp)) // # of planes
	{
		return false;
	}
	if (wTemp != 1)
	{
		return false;
	}
	if (!readValue(file, wTemp)) // BPP
	{
		return false;
	}
	if (wTemp != 24)
	{
		return false;
	}
	if (!readValue(file, dwTemp)) // Compression
	{
		return false;
	}
	if (dwTemp != 0)
	{
		return false;
	}
	if (!readValue(file, imageSize))
	{
		return false;
	}
	if (!readValue(file, lTemp)) // X Pixels per meter: 72 DPI
	{
		return false;
	}
	if (!readValue(file, lTemp)) // Y Pixels per meter: 72 DPI
	{
		return false;
	}
	if (!readValue(file, dwTemp)) // # of colors used
	{
		return false;
	}
	if (dwTemp != 0)
	{
		return false;
	}
	if (!readValue(file, dwTemp)) // # of important colors: 0 == all
	{
		return false;
	}
	if (dwTemp != 0)
	{
		return false;
	}
	return true;
}

bool TCBmpImageFormat::readImageData(TCImage *image, FILE *file)
{
	int rowSize = image->roundUp(image->getWidth() * 3, 4);
	bool failed = false;
	bool canceled = false;
	int i, j;
	bool rgba = image->getDataFormat() == TCRgba8;
	int imageRowSize = image->getRowSize();
	TCByte *rowData = new TCByte[rowSize];

	callProgressCallback(_UC("LoadingBMP"), 0.0f);
	memset(rowData, 0, rowSize);
	image->allocateImageData();
	for (i = 0; i < image->getHeight() && !failed && !canceled; i++)
	{
		if (fread(rowData, 1, rowSize, file) != (unsigned)rowSize)
		{
			failed = true;
		}
		else
		{
			if (rgba)
			{
				int lineOffset;

				if (image->getFlipped())
				{
					lineOffset = i * imageRowSize;
				}
				else
				{
					lineOffset = (image->getHeight() - i - 1) * imageRowSize;
				}
				for (j = 0; j < image->getWidth(); j++)
				{
					image->getImageData()[lineOffset + j * 4 + 0] =
						rowData[j * 3 + 2];
					image->getImageData()[lineOffset + j * 4 + 1] =
						rowData[j * 3 + 1];
					image->getImageData()[lineOffset + j * 4 + 2] =
						rowData[j * 3 + 0];
				}
			}
			else
			{
				int lineOffset;

				if (image->getFlipped())
				{
					lineOffset = i * imageRowSize;
				}
				else
				{
					lineOffset = (image->getHeight() - i - 1) * imageRowSize;
				}
				for (j = 0; j < image->getWidth(); j++)
				{
					image->getImageData()[lineOffset + j * 3 + 0] =
						rowData[j * 3 + 2];
					image->getImageData()[lineOffset + j * 3 + 1] =
						rowData[j * 3 + 1];
					image->getImageData()[lineOffset + j * 3 + 2] =
						rowData[j * 3 + 0];
				}
			}
		}
		if (!callProgressCallback(NULL,
			(float)(i + 1) / (float)image->getHeight()))
		{
			canceled = true;
		}
	}
	delete[] rowData;
	callProgressCallback(NULL, 2.0f);
	return !failed && !canceled;
}

int TCBmpImageFormat::writeValue(TCByte *buf, unsigned short value, int offset)
{
	// Write the value in little endian format
	buf[offset] = (TCByte)value;
	buf[offset + 1] = (TCByte)(value >> 8);
	return offset + 2;
}

int TCBmpImageFormat::writeValue(TCByte *buf, unsigned long value, int offset)
{
	// Write the value in little endian format
	buf[offset] = (TCByte)value;
	buf[offset + 1] = (TCByte)(value >> 8);
	buf[offset + 2] = (TCByte)(value >> 16);
	buf[offset + 3] = (TCByte)(value >> 24);
	return offset + 4;
}

int TCBmpImageFormat::writeValue(TCByte *buf, long value, int offset)
{
	return writeValue(buf, (unsigned long)value, offset);
}

int TCBmpImageFormat::writeHeader(int width, int height, TCByte *buf)
{
	int offset = writeFileHeader(width, height, buf);

	return writeInfoHeader(width, height, buf, offset);
}

int TCBmpImageFormat::writeFileHeader(int width, int height, TCByte *buf)
{
	int rowSize = TCImage::roundUp(width * 3, 4);
	DWORD imageSize = rowSize * height;
	int offset = 0;

	offset = writeValue(buf, (WORD)0x4D42, offset); // 'BM'
	offset = writeValue(buf, (DWORD)(BMP_HEADER_SIZE + imageSize), offset);
	offset = writeValue(buf, (DWORD)0, offset); // Reserved
	return writeValue(buf, (DWORD)BMP_HEADER_SIZE, offset);
}

int TCBmpImageFormat::writeInfoHeader(
	int width,
	int height,
	TCByte *buf,
	int offset)
{
	int rowSize = TCImage::roundUp(width * 3, 4);
	DWORD imageSize = rowSize * height;

	offset = writeValue(buf, (DWORD)BMP_INFO_HEADER_SIZE, offset);
	offset = writeValue(buf, (long)width, offset);
	offset = writeValue(buf, (long)height, offset);
	offset = writeValue(buf, (WORD)1, offset); // # of planes
	offset = writeValue(buf, (WORD)24, offset); // BPP
	offset = writeValue(buf, (DWORD)0, offset); // Compression
	offset = writeValue(buf, (DWORD)imageSize, offset);
	offset = writeValue(buf, (long)2835, offset); // X Pixels per meter: 72 DPI
	offset = writeValue(buf, (long)2835, offset); // Y Pixels per meter: 72 DPI
	offset = writeValue(buf, (DWORD)0, offset); // # of colors used
	return writeValue(buf, (DWORD)0, offset); // # of important colors: 0 == all
}

bool TCBmpImageFormat::writeImageData(TCImage *image, FILE *file)
{
	int rowSize = image->roundUp(image->getWidth() * 3, 4);
	bool failed = false;
	bool canceled = false;
	int i, j;
	bool rgba = image->getDataFormat() == TCRgba8;
	int imageRowSize = image->getRowSize();
	TCByte *rowData = new TCByte[rowSize];

	callProgressCallback(_UC("SavingBMP"), 0.0f);
	memset(rowData, 0, rowSize);
	for (i = 0; i < image->getHeight() && !failed && !canceled; i++)
	{
		if (rgba)
		{
			int lineOffset;

			if (image->getFlipped())
			{
				lineOffset = i * imageRowSize;
			}
			else
			{
				lineOffset = (image->getHeight() - i - 1) * imageRowSize;
			}
			for (j = 0; j < image->getWidth(); j++)
			{
				rowData[j * 3 + 2] = image->getImageData()[lineOffset +
					j * 4 + 0];
				rowData[j * 3 + 1] = image->getImageData()[lineOffset +
					j * 4 + 1];
				rowData[j * 3 + 0] = image->getImageData()[lineOffset +
					j * 4 + 2];
			}
		}
		else
		{
			int lineOffset;

			if (image->getFlipped())
			{
				lineOffset = i * imageRowSize;
			}
			else
			{
				lineOffset = (image->getHeight() - i - 1) * imageRowSize;
			}
			for (j = 0; j < image->getWidth(); j++)
			{
				rowData[j * 3 + 2] = image->getImageData()[lineOffset +
					j * 3 + 0];
				rowData[j * 3 + 1] = image->getImageData()[lineOffset +
					j * 3 + 1];
				rowData[j * 3 + 0] = image->getImageData()[lineOffset +
					j * 3 + 2];
			}
		}
		if (fwrite(rowData, 1, rowSize, file) != (unsigned)rowSize)
		{
			failed = true;
		}
		if (!callProgressCallback(NULL,
			(float)(i + 1) / (float)image->getHeight()))
		{
			canceled = true;
		}
	}
	delete[] rowData;
	callProgressCallback(NULL, 2.0f);
	return !failed && !canceled;
}

bool TCBmpImageFormat::saveFile(TCImage *image, FILE *file)
{
	TCByte header[BMP_HEADER_SIZE];
	int size = writeHeader(image->getWidth(), image->getHeight(), header);

	if (fwrite(header, 1, size, file) != (size_t)size)
	{
		return false;
	}
	return writeImageData(image, file);
}
