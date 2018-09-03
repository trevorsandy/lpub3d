#ifndef NO_PNG_IMAGE_FORMAT
#include "TCPngImageFormat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mystring.h"

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

TCPngImageFormat::TCPngImageFormat(void)
	:commentData(NULL),
	commentDataCount(0)
{
	name = "PNG";
#ifdef _LEAK_DEBUG
	strcpy(className, "TCPngImageFormat");
#endif
}

TCPngImageFormat::~TCPngImageFormat(void)
{
}

void TCPngImageFormat::dealloc(void)
{
	if (commentDataCount)
	{
		deleteStringArray(commentData, commentDataCount);
	}
	TCImageFormat::dealloc();
}

bool TCPngImageFormat::checkSignature(const TCByte *data, long length)
{
	if (length >= 8)
	{
		return png_sig_cmp((TCByte *)data, 0, 8) ? false : true;
	}
	else
	{
		return false;
	}
}

bool TCPngImageFormat::checkSignature(FILE *file)
{
	bool retValue = false;
	TCByte header[8];
	long filePos = ftell(file);

	if (fread(header, 1, 8, file) == 8)
	{
		retValue = png_sig_cmp(header, 0, 8) ? false : true;
	}
	fseek(file, filePos, SEEK_SET);
	return retValue;
}

bool TCPngImageFormat::setupProgressive(void)
{
	if (!setup())
	{
		return false;
	}
#if defined(WIN32) && !defined(_QT)
#pragma warning( push )
#pragma warning( disable : 4611 )
#endif // WIN32 && NOT _QT
	if (setjmp(jumpBuf))
#if defined(WIN32) && !defined(_QT)
#pragma warning( pop )
#endif // WIN32 && NOT _QT
	{
		png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
		return false;
	}
	png_set_progressive_read_fn(pngPtr, this, staticInfoCallback,
		staticRowCallback, NULL);
	canceled = false;
	return true;
}

void TCPngImageFormat::infoCallback(void)
{
	int bitDepth;
	int colorType;
	bool haveAlpha = false;

	png_get_IHDR(pngPtr, infoPtr, &imageWidth, &imageHeight, &bitDepth,
		&colorType, NULL, NULL, NULL);
    if (colorType & PNG_COLOR_TYPE_PALETTE)
	{
		// If the image has a palette, we want to expand it to RGB(A)
        png_set_expand(pngPtr);
	}
    if (colorType == PNG_COLOR_TYPE_GRAY)
	{
		// !UNTESTED!
		// We don't support grayscale, so have PNG library convert to RGB.
		png_set_gray_to_rgb(pngPtr);
		if (bitDepth < 8)
		{
			// If it is grayscale, and less than 8 bits, expand to 8 bits.
			png_set_expand(pngPtr);
		}
	}
    if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
	{
		// If it is a paletted image with a tRNS mask, expand that to alpha.
        png_set_expand(pngPtr);
		haveAlpha = true;
	}
	if (haveAlpha || (colorType & PNG_COLOR_MASK_ALPHA))
	{
		image->setDataFormat(TCRgba8);
	}
	else
	{
		image->setDataFormat(TCRgb8);
	}
	image->setSize(imageWidth, imageHeight);
	image->allocateImageData();
	numPasses = png_set_interlace_handling(pngPtr);
	png_read_update_info(pngPtr, infoPtr);
}

float TCPngImageFormat::passToFraction(int pass)
{
	if (numPasses == 1)
	{
		return 1.0f;
	}
	else
	{
		return (float)(1 << pass) / 64.0f;
	}
}

void TCPngImageFormat::rowCallback(
	png_bytep rowData,
	png_uint_32 rowNum,
	int pass)
{
	TCByte *imageData;
	TCByte *rowSpot;
	int imageRowSize;

	if (!rowData)
	{
		return;
	}
	imageData = image->getImageData();
	imageRowSize = image->getRowSize();
	if (image->getFlipped())
	{
		rowSpot = imageData + (imageHeight - rowNum - 1) * imageRowSize;
	}
	else
	{
		rowSpot = imageData + rowNum * imageRowSize;
	}
	png_progressive_combine_row(pngPtr, rowSpot, rowData);
	if (!callProgressCallback(_UC("LoadingPNGRow"),
		((float)rowNum + 1.0f) / imageHeight * passToFraction(pass)))
	{
		canceled = true;
		longjmp(jumpBuf, 1);
	}
}

void TCPngImageFormat::errorCallback(png_const_charp /*msg*/)
{
	longjmp(jumpBuf, 1);
}

void TCPngImageFormat::staticErrorCallback(
	png_structp pngPtr, 
	png_const_charp msg)
{
	TCPngImageFormat *format = (TCPngImageFormat *)png_get_error_ptr(pngPtr);

	if (format)
	{
		format->errorCallback(msg);
	}
}
	
void TCPngImageFormat::staticInfoCallback(png_structp pngPtr,
										  png_infop /*infoPtr*/)
{
	TCPngImageFormat *format =
		(TCPngImageFormat *)png_get_progressive_ptr(pngPtr);

	if (format)
	{
		format->infoCallback();
	}
}

void TCPngImageFormat::staticRowCallback(png_structp pngPtr, png_bytep newRow,
										 png_uint_32 rowNum, int pass)
{
	TCPngImageFormat *format =
		(TCPngImageFormat *)png_get_progressive_ptr(pngPtr);

	if (format)
	{
		format->rowCallback(newRow, rowNum, pass);
	}
}

bool TCPngImageFormat::loadData(TCImage *limage, TCByte *data, long length)
{
	if (setupProgressive())
	{
		image = limage;
#if defined(WIN32) && !defined(_QT)
#pragma warning( push )
#pragma warning( disable : 4611 )
#endif // WIN32 && NOT _QT
		if (setjmp(jumpBuf))
#if defined(WIN32) && !defined(_QT)
#pragma warning( pop )
#endif // WIN32 && NOT _QT
		{
			png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
			return false;
		}
		callProgressCallback(_UC("LoadingPNG"), 0.0f);
		png_process_data(pngPtr, infoPtr, data, length);
		png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
		callProgressCallback(NULL, 2.0f);
		return true;
	}
	return false;
}

bool TCPngImageFormat::setup(void)
{
	pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, this,
		staticErrorCallback, NULL);
	if (!pngPtr)
	{
		return false;
	}
	infoPtr = png_create_info_struct(pngPtr);
	if (!infoPtr)
	{
		png_destroy_read_struct(&pngPtr, NULL, NULL);
		return false;
	}
	return true;
}

bool TCPngImageFormat::loadFile(TCImage *limage, FILE *file)
{
	if (setupProgressive())
	{
		TCByte buf[1024];

		image = limage;
#if defined(WIN32) && !defined(_QT)
#pragma warning( push )
#pragma warning( disable : 4611 )
#endif // WIN32 && NOT _QT
		if (setjmp(jumpBuf))
#if defined(WIN32) && !defined(_QT)
#pragma warning( pop )
#endif // WIN32 && NOT _QT
		{
			png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
			return false;
		}
		callProgressCallback(_UC("LoadingPNG"), 0.0f);
		while (true)
		{
			size_t bytesRead = fread(buf, 1, sizeof(buf), file);

			if (bytesRead == 0)
			{
				break;
			}
			png_process_data(pngPtr, infoPtr, buf, bytesRead);
		}
		png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
		callProgressCallback(NULL, 2.0f);
		return true;
	}
	return false;
}

bool TCPngImageFormat::saveFile(TCImage *limage, FILE *file)
{
	debugPrintf(2, "TCPngImageFormat::saveFile() 1\n");
	bool retValue = false;
	png_structp lpngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		this, staticErrorCallback, NULL);

	canceled = false;
	if (lpngPtr)
	{
		png_infop linfoPtr = png_create_info_struct(lpngPtr);

		if (linfoPtr)
		{
#if defined(WIN32) && !defined(_QT)
#pragma warning( push )
#pragma warning( disable : 4611 )
#endif // WIN32 && NOT _QT
			if (!setjmp(jumpBuf))
#if defined(WIN32) && !defined(_QT)
#pragma warning( pop )
#endif // WIN32 && NOT _QT
			{
				int i;
				int rowSize = limage->getRowSize();
				int width = limage->getWidth();
				int height = limage->getHeight();
				int pngColorType = 0;
				int bitDepth;
				TCByte *imageData = limage->getImageData();
				bool failed = false;

				if (limage->getComment() && strlen(limage->getComment()) > 0)
				{
					png_textp textPtr;
					int count;

					if (commentDataCount)
					{
						deleteStringArray(commentData, commentDataCount);
					}
					commentData =
						componentsSeparatedByString(limage->getComment(),
						":!:!:", commentDataCount);
					if (commentDataCount % 2)
					{
						deleteStringArray(commentData, commentDataCount);
						commentData = new char*[2];
						commentDataCount = 2;
						commentData[0] = copyString("Comment");
						commentData[1] = copyString(limage->getComment());
					}
					count = commentDataCount / 2;
					textPtr = new png_text[count];
					for (i = 0; i < count; i++)
					{
						memset(&textPtr[i], 0, sizeof(png_text));
						textPtr[i].compression = PNG_TEXT_COMPRESSION_NONE;
						textPtr[i].key = commentData[i * 2];
						textPtr[i].text = commentData[i * 2 + 1];
					}
					png_set_text(lpngPtr, linfoPtr, textPtr, count);
					delete[] textPtr;
				}
				png_uint_32 ppm = (png_uint_32)(limage->getDpi() / 0.0254);
				png_set_pHYs(lpngPtr, linfoPtr, ppm, ppm, PNG_RESOLUTION_METER);
				switch (limage->getDataFormat())
				{
				case TCRgb8:
					pngColorType = PNG_COLOR_TYPE_RGB;
					bitDepth = 8;
					break;
				case TCRgba8:
					pngColorType = PNG_COLOR_TYPE_RGB_ALPHA;
					bitDepth = 8;
					break;
				case TCRgb16:
					pngColorType = PNG_COLOR_TYPE_RGB;
					bitDepth = 16;
					break;
				case TCRgba16:
					pngColorType = PNG_COLOR_TYPE_RGB_ALPHA;
					bitDepth = 16;
					break;
				default:
					failed = true;
					break;
				}
				if (!failed)
				{
					png_init_io(lpngPtr, file);
					png_set_sRGB(lpngPtr, linfoPtr, PNG_sRGB_INTENT_PERCEPTUAL);
					png_set_IHDR(lpngPtr, linfoPtr, width, height, bitDepth,
						pngColorType, PNG_INTERLACE_NONE,
						PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
					png_write_info(lpngPtr, linfoPtr);
					callProgressCallback(_UC("SavingPNG"), 0.0f);
					for (i = 0; i < height; i++)
					{
						if (!callProgressCallback(NULL,
							(float)(i) / (float)height))
						{
							canceled = true;
							break;
						}
						if (limage->getFlipped())
						{
							png_write_row(lpngPtr,
								imageData + (height - i - 1) * rowSize);
						}
						else
						{
							png_write_row(lpngPtr, imageData + i * rowSize);
						}
					}
					png_write_end(lpngPtr, linfoPtr);
					callProgressCallback(NULL, 1.0f);
					retValue = true;
				}
			}
			png_destroy_write_struct(&lpngPtr, &linfoPtr);
		}
		else
		{
			png_destroy_write_struct(&lpngPtr, (png_infopp)NULL);
		}
	}
	callProgressCallback(NULL, 2.0f);
	debugPrintf(2, "TCPngImageFormat::saveFile() 2\n");
	return retValue && !canceled;
}
#endif // NO_PNG_IMAGE_FORMAT
