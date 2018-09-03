#ifndef NO_JPG_IMAGE_FORMAT
#include "TCJpegImageFormat.h"
#include "TCJpegOptions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mystring.h"

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif

extern "C"
{
// jpeglib.h doesn't work if XMD_H isn't defined.
#define XMD_H
#include <jpeglib.h>
#undef XMD_H
}

#define SIG_LENGTH 10

TCJpegImageFormat::TCJpegImageFormat(void):
image(NULL),
imageWidth(0),
imageHeight(0)
{
	name = "JPG";
#ifdef _LEAK_DEBUG
	strcpy(className, "TCJpegImageFormat");
#endif
}

TCJpegImageFormat::~TCJpegImageFormat(void)
{
}

void TCJpegImageFormat::dealloc(void)
{
	TCImageFormat::dealloc();
}

bool TCJpegImageFormat::checkSignature(const TCByte *data, long length)
{
	if (length >= SIG_LENGTH)
	{
		TCByte jfifSig0[] = { 0xFF, 0xD8, 0xFF, 0xE0 };
		TCByte jfifSig1[] = { 'J', 'F', 'I', 'F' };
		TCByte exifSig0[] = { 0xFF, 0xD8, 0xFF, 0xE1 };
		TCByte exifSig1[] = { 'E', 'x', 'i', 'f' };

		if ((memcmp(data, jfifSig0, sizeof(jfifSig0)) == 0 &&
			memcmp(&data[6], jfifSig1, sizeof(jfifSig1)) == 0) ||
			(memcmp(data, exifSig0, sizeof(exifSig0)) == 0 &&
			memcmp(&data[6], exifSig1, sizeof(exifSig1)) == 0))
		{
			return true;
		}
	}
	return false;
}

bool TCJpegImageFormat::checkSignature(FILE *file)
{
	bool retValue = false;
	TCByte header[SIG_LENGTH];
	long filePos = ftell(file);

	if (fread(header, 1, SIG_LENGTH, file) == SIG_LENGTH)
	{
		retValue = checkSignature(header, SIG_LENGTH);
	}
	fseek(file, filePos, SEEK_SET);
	return retValue;
}

bool TCJpegImageFormat::loadData(TCImage *limage, TCByte *data, long length)
{
	return load(limage, NULL, data, length);
}

extern "C"
{

void TCJpegImageFormat::staticErrorExit(jpeg_common_struct *cinfo)
{
	((TCJpegImageFormat *)cinfo->client_data)->errorExit(*cinfo);
}

}

void TCJpegImageFormat::errorExit(jpeg_common_struct & /*cinfo*/)
{
	longjmp(jumpBuf, 1);
}

bool TCJpegImageFormat::setup(jpeg_decompress_struct &cinfo, jpeg_error_mgr &jerr)
{
	cinfo.client_data = this;
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = staticErrorExit;
	jpeg_create_decompress(&cinfo);
	return true;
}

bool TCJpegImageFormat::setup(jpeg_compress_struct &cinfo, jpeg_error_mgr &jerr)
{
	cinfo.client_data = this;
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = staticErrorExit;
	jpeg_create_compress(&cinfo);
	return true;
}

bool TCJpegImageFormat::loadFile(TCImage *limage, FILE *file)
{
	return load(limage, file, NULL, 0);
}

bool TCJpegImageFormat::load(TCImage *limage, FILE *file, TCByte *data, long length)
{
	bool retValue = false;
	bool canceled = false;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	if (setup(cinfo, jerr))
	{
		std::vector<TCByte> grayRow;

		image = limage;
		if (file != NULL)
		{
			jpeg_stdio_src(&cinfo, file);
		}
		else if (data != NULL && length > 0)
		{
#if JPEG_LIB_VERSION >= 80 || defined(MEM_SRCDST_SUPPORTED)
			jpeg_mem_src(&cinfo, data, length);
#else
			return false;
#endif
		}
		else
		{
			return false;
		}
		// WARNING: Do NOT put any C++ objects that need destructors inside the
		// following if statement.  Doing so will result in a memory leak if any
		// error occurs.
#if defined(WIN32) && !defined(_QT)
#pragma warning( push )
#pragma warning( disable : 4611 )
#endif // WIN32 && NOT _QT
		if (setjmp(jumpBuf))
#if defined(WIN32) && !defined(_QT)
#pragma warning( pop )
#endif // WIN32 && NOT _QT
		{
			// If we get here, there was an error below.
			// Note: jpeg_destroy_decompress is called automatically by the JPEG
			// library error handling code.  We really are good to just exit.
			return false;
		}
		callProgressCallback(_UC("LoadingJPG"), 0.0f);
		if (jpeg_read_header(&cinfo, TRUE) == JPEG_HEADER_OK)
		{
			cinfo.output_components = 3;
			cinfo.out_color_space = JCS_RGB;
			if (jpeg_start_decompress(&cinfo))
			{
				if ((cinfo.out_color_space == JCS_RGB ||
					cinfo.out_color_space == JCS_GRAYSCALE) &&
					(cinfo.output_components == 3 ||
					cinfo.output_components == 1))
				{
					TCByte *imageData;
					int rowSize;
					int offset = 0;
					int dir = 1;

					imageWidth = cinfo.output_width;
					imageHeight = cinfo.output_height;
					if (cinfo.output_components != 3)
					{
						grayRow.resize(imageWidth * cinfo.output_components);
					}
					image->setDataFormat(TCRgb8);
					image->setSize((int)imageWidth, (int)imageHeight);
					image->allocateImageData();
					rowSize = image->getRowSize();
					imageData = image->getImageData();
					if (image->getFlipped())
					{
						offset = rowSize * ((int)imageHeight - 1);
						dir = -1;
					}
					while (cinfo.output_scanline < cinfo.output_height &&
						!canceled)
					{
						unsigned int result;

						if (cinfo.output_components == 1)
						{
							unsigned char *grayBytes = &grayRow[0];
							// grayscale
							result = jpeg_read_scanlines(&cinfo, &grayBytes, 1);
							for (unsigned int i = 0; i < imageWidth; i++)
							{
								TCByte value = grayRow[i];

								imageData[offset + i * 3] = value;
								imageData[offset + i * 3 + 1] = value;
								imageData[offset + i * 3 + 2] = value;
							}
						}
						else
						{
							unsigned char *rowBytes = &imageData[offset];
							result = jpeg_read_scanlines(&cinfo, &rowBytes,
								1);
						}
						offset += result * rowSize * dir;
						if (!callProgressCallback(NULL, (float)cinfo.output_scanline /
							(float)cinfo.output_height))
						{
							canceled = true;
						}
					}
					retValue = true;
				}
			}
		}
		if (retValue && !canceled)
		{
			if (!jpeg_finish_decompress(&cinfo))
			{
				retValue = false;
			}
		}
		else
		{
			jpeg_abort_decompress(&cinfo);
		}
		jpeg_destroy_decompress(&cinfo);
	}
	callProgressCallback(NULL, 2.0f);
	return retValue && !canceled;
}

bool TCJpegImageFormat::saveFile(TCImage *limage, FILE *file)
{
	bool retValue = false;
	bool canceled = false;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	if (limage->getDataFormat() != TCRgb8 && limage->getDataFormat() != TCRgba8)
	{
		return false;
	}
	if (setup(cinfo, jerr))
	{
		std::vector<TCByte> row;
		TCByte *imageData;
		int rowSize;
		int offset = 0;
		int dir = 1;
		TCByte *rowBytes = NULL;
		image = limage;
		TCJpegOptions *options =
			(TCJpegOptions *)image->getCompressionOptions();

		imageWidth = image->getWidth();
		imageHeight = image->getHeight();
		// WARNING: Do NOT put any C++ objects that need destructors inside the
		// following if statement.  Doing so will result in a memory leak if any
		// error occurs.
#if defined(WIN32) && !defined(_QT)
#pragma warning( push )
#pragma warning( disable : 4611 )
#endif // WIN32 && NOT _QT
		if (setjmp(jumpBuf))
#if defined(WIN32) && !defined(_QT)
#pragma warning( pop )
#endif // WIN32 && NOT _QT
		{
			// If we get here, there was an error below.
			// Note: jpeg_destroy_compress is called automatically by the JPEG
			// library error handling code.  We really are good to just exit.
			return false;
		}
		jpeg_stdio_dest(&cinfo, file);
		callProgressCallback(_UC("SavingJPG"), 0.0f);
		cinfo.in_color_space = JCS_RGB;
		cinfo.image_width = (JDIMENSION)imageWidth;
		cinfo.image_height = (JDIMENSION)image->getHeight();
		cinfo.input_components = 3;
		imageData = image->getImageData();
		rowSize = image->getRowSize();
		if (image->getFlipped())
		{
			offset = rowSize * ((int)imageHeight - 1);
			dir = -1;
		}
		if (image->getDataFormat() == TCRgba8)
		{
			row.resize(image->getWidth() * 3);
			rowBytes = &row[0];
		}
		jpeg_set_defaults(&cinfo);
		jpeg_default_colorspace(&cinfo);
		jpeg_set_quality(&cinfo, options->getQuality(), FALSE);
		switch (options->getSubSampling())
		{
		case TCJpegOptions::SS420:
			// 4:2:0
			cinfo.comp_info[0].h_samp_factor = 2;
			cinfo.comp_info[0].v_samp_factor = 2;
			cinfo.comp_info[1].h_samp_factor = 1;
			cinfo.comp_info[1].v_samp_factor = 1;
			cinfo.comp_info[2].h_samp_factor = 1;
			cinfo.comp_info[2].v_samp_factor = 1;
			break;
		case TCJpegOptions::SS422:
			// 4:2:2
			cinfo.comp_info[0].h_samp_factor = 2;
			cinfo.comp_info[0].v_samp_factor = 1;
			cinfo.comp_info[1].h_samp_factor = 1;
			cinfo.comp_info[1].v_samp_factor = 1;
			cinfo.comp_info[2].h_samp_factor = 1;
			cinfo.comp_info[2].v_samp_factor = 1;
			break;
		case TCJpegOptions::SS444:
			// 4:4:4
			cinfo.comp_info[0].h_samp_factor = 1;
			cinfo.comp_info[0].v_samp_factor = 1;
			cinfo.comp_info[1].h_samp_factor = 1;
			cinfo.comp_info[1].v_samp_factor = 1;
			cinfo.comp_info[2].h_samp_factor = 1;
			cinfo.comp_info[2].v_samp_factor = 1;
			break;
		}
		if (options->getProgressive())
		{
			jpeg_simple_progression(&cinfo);
		}
		jpeg_start_compress(&cinfo, TRUE);
		while (cinfo.next_scanline < cinfo.image_height && !canceled)
		{
			unsigned int result;

			if (image->getDataFormat() == TCRgba8)
			{
				int dstIndex = 0;
				int srcIndex = offset;

				// Convert RGBA to RGB.
				for (unsigned int x = 0; x < imageWidth; x++)
				{
					rowBytes[dstIndex++] = imageData[srcIndex++];
					rowBytes[dstIndex++] = imageData[srcIndex++];
					rowBytes[dstIndex++] = imageData[srcIndex++];
					srcIndex++;
				}
			}
			else
			{
				rowBytes = &imageData[offset];
			}
			result = jpeg_write_scanlines(&cinfo, &rowBytes, 1);
			offset += result * rowSize * dir;
			if (!callProgressCallback(NULL, (float)cinfo.next_scanline /
				(float)cinfo.image_height))
			{
				canceled = true;
			}
		}
		if (canceled)
		{
			jpeg_abort_compress(&cinfo);
		}
		else
		{
			jpeg_finish_compress(&cinfo);
			retValue = true;
		}
		jpeg_destroy_compress(&cinfo);
	}
	callProgressCallback(NULL, 2.0f);
	return retValue && !canceled;
}

TCImageOptions *TCJpegImageFormat::newCompressionOptions(void)
{
	return new TCJpegOptions;
}
#endif // NO_JPG_IMAGE_FORMAT
