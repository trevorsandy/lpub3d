#include "TCImage.h"
#ifndef NO_PNG_IMAGE_FORMAT
#include "TCPngImageFormat.h"
#endif // NO_PNG_IMAGE_FORMAT
#ifndef NO_BMP_IMAGE_FORMAT
#include "TCBmpImageFormat.h"
#endif // NO_BMP_IMAGE_FORMAT
#ifndef NO_JPG_IMAGE_FORMAT
#include "TCJpegImageFormat.h"
#endif // NO_JPG_IMAGE_FORMAT
#include "TCImageOptions.h"
#include "mystring.h"
#include "stb_image_resize.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif

TCImageFormatArray *TCImage::imageFormats = NULL;

TCImage::TCImageCleanup TCImage::imageCleanup;

TCImage::TCImageCleanup::~TCImageCleanup(void)
{
	if (TCImage::imageFormats)
	{
		TCImage::imageFormats->release();
		TCImage::imageFormats = NULL;
	}
}

TCImage::TCImage(void)
		:imageData(NULL),
		 dataFormat(TCRgb8),
		 bytesPerPixel(3),
		 width(0),
		 height(0),
		 croppedX(0),
		 croppedY(0),
		 dpi(72),
		 lineAlignment(1),
		 flipped(false),
		 formatName(NULL),
		 userImageData(false),
		 comment(NULL),
		 compressionOptions(NULL)
{
	initStandardFormats();
#ifdef _LEAK_DEBUG
	strcpy(className, "TCImage");
#endif
}

TCImage::~TCImage(void)
{
}

void TCImage::dealloc(void)
{
	if (!userImageData)
	{
		delete[] imageData;
	}
	delete[] formatName;
	delete[] comment;
	TCObject::release(compressionOptions);
	TCObject::dealloc();
}

void TCImage::setDataFormat(TCImageDataFormat format)
{
	dataFormat = format;
	switch (dataFormat)
	{
	case TCRgb8:
		bytesPerPixel = 3;
		break;
	case TCRgba8:
		bytesPerPixel = 4;
		break;
	case TCRgb16:
		bytesPerPixel = 6;
		break;
	case TCRgba16:
		bytesPerPixel = 8;
		break;
	}
	syncImageData();
}

void TCImage::setSize(int xSize, int ySize)
{
	width = xSize;
	height = ySize;
	syncImageData();
}

void TCImage::getSize(int &xSize, int &ySize)
{
	xSize = width;
	ySize = height;
}

void TCImage::setLineAlignment(int value)
{
	lineAlignment = value;
	syncImageData();
}

void TCImage::setFlipped(bool value)
{
	if (value != flipped)
	{
		flipped = value;
	}
}

void TCImage::setImageData(TCByte *value)
{
	if (!userImageData)
	{
		delete[] imageData;
	}
	imageData = value;
	if (imageData)
	{
		userImageData = true;
	}
	else
	{
		userImageData = false;
	}
}

void TCImage::allocateImageData(void)
{
	if (!userImageData)
	{
		delete[] imageData;
	}
	if (width > 0 && height > 0)
	{
		imageData = new TCByte[roundUp(width * bytesPerPixel, lineAlignment) *
			height];
	}
	else
	{
		imageData = NULL;
	}
}

int TCImage::getRowSize(void)
{
	return roundUp(width * bytesPerPixel, lineAlignment);
}

int TCImage::roundUp(int value, int nearest)
{
	return (value + nearest - 1) / nearest * nearest;
}

void TCImage::syncImageData(void)
{
	if (imageData)
	{
		allocateImageData();
	}
}

void TCImage::addImageFormat(TCImageFormat *imageFormat, bool release)
{
	imageFormats->addObject(imageFormat);
	if (release)
	{
		imageFormat->release();
	}
}

void TCImage::initStandardFormats(void)
{
	if (!imageFormats)
	{
		// Note: Due to what may or may not be a bug in the Microsoft linker,
		// image formats CANNOT self-register.  In order for a class to be
		// included from a library into an executable, it has to be referenced
		// from other code that is used from the library directly.  Declaring a
		// global variable that does this doesn't work.
		imageFormats = new TCImageFormatArray;
#ifndef NO_PNG_IMAGE_FORMAT
		addImageFormat(new TCPngImageFormat, true);
#endif // NO_PNG_IMAGE_FORMAT
#ifndef NO_BMP_IMAGE_FORMAT
		addImageFormat(new TCBmpImageFormat, true);
#endif // NO_BMP_IMAGE_FORMAT
#ifndef NO_JPG_IMAGE_FORMAT
		addImageFormat(new TCJpegImageFormat, true);
#endif // NO_JPG_IMAGE_FORMAT
	}
}

void TCImage::setFormatName(const char *value)
{
	if (value != formatName && (value == NULL || formatName == NULL ||
		strcmp(value, formatName) != 0))
	{
		delete[] formatName;
		formatName = copyString(value);
		TCObject::release(compressionOptions);
		compressionOptions = NULL;
	}
}

bool TCImage::loadData(
	TCByte *data,
	long length,
	TCImageProgressCallback progressCallback /*= NULL*/,
	void *progressUserData /*= NULL*/)
{
	TCImageFormat *imageFormat = formatForData(data, length);

	if (imageFormat)
	{
		imageFormat->setProgressCallback(progressCallback, progressUserData);
		if (imageFormat->loadData(this, data, length))
		{
			setFormatName(imageFormat->getName());
			return true;
		}
	}
	return false;
}

bool TCImage::loadFile(
	FILE *file,
	TCImageProgressCallback progressCallback /*= NULL*/,
	void *progressUserData /*= NULL*/)
{
	TCImageFormat *imageFormat = formatForFile(file);

	if (imageFormat)
	{
		imageFormat->setProgressCallback(progressCallback, progressUserData);
		if (imageFormat->loadFile(this, file))
		{
			setFormatName(imageFormat->getName());
			return true;
		}
	}
	return false;
}

bool TCImage::loadFile(
	const char *filename,
	TCImageProgressCallback progressCallback /*= NULL*/,
	void *progressUserData /*= NULL*/)
{
	TCImageFormat *imageFormat = formatForFile(filename);

	if (imageFormat)
	{
		imageFormat->setProgressCallback(progressCallback, progressUserData);
		if (imageFormat->loadFile(this, filename))
		{
			setFormatName(imageFormat->getName());
			return true;
		}
	}
	return false;
}

bool TCImage::saveFile(
	const char *filename,
	TCImageProgressCallback progressCallback /*= NULL*/,
	void *progressUserData /*= NULL*/)
{
	TCImageFormat *imageFormat = formatWithName(formatName);

	if (imageFormat)
	{
		imageFormat->setProgressCallback(progressCallback, progressUserData);
		if (imageFormat->saveFile(this, filename))
		{
			getCompressionOptions()->save();
			return true;
		}
	}
	return false;
}

TCImageFormat *TCImage::formatForFile(const char *filename)
{
	TCImageFormat *retValue = NULL;
	FILE *file = ucfopen(filename, "rb");

	if (file)
	{
		retValue = formatForFile(file);
		fclose(file);
	}
	return retValue;
}

TCImageFormat *TCImage::formatForFile(FILE *file)
{
	int i;
	int count = imageFormats->getCount();

	for (i = 0; i < count; i++)
	{
		TCImageFormat *imageFormat = imageFormats->objectAtIndex(i);
		if (imageFormat->checkSignature(file))
		{
			return imageFormat;
		}
	}
	return NULL;
}

TCImageFormat *TCImage::formatForData(const TCByte *data, long length)
{
	int i;
	int count = imageFormats->getCount();

	for (i = 0; i < count; i++)
	{
		TCImageFormat *imageFormat = imageFormats->objectAtIndex(i);
		if (imageFormat->checkSignature(data, length))
		{
			return imageFormat;
		}
	}
	return NULL;
}

TCImageFormat *TCImage::formatWithName(char *name)
{
	if (name)
	{
		int i;
		int count = imageFormats->getCount();

		for (i = 0; i < count; i++)
		{
			TCImageFormat *imageFormat = imageFormats->objectAtIndex(i);
			if (imageFormat->getName() && strcmp(name, imageFormat->getName())
				== 0)
			{
				return imageFormat;
			}
		}
	}
	return NULL;
}

TCImage *TCImage::createSubImage(int x, int y, int cx, int cy)
{
	TCImage *newImage;
	int i;
	int rowSize;
	int newRowSize;

	if (imageData == NULL || x + cx > width || y + cy > height || cx <= 0 || cy <= 0)
	{
		return NULL;
	}
	newImage = new TCImage;
	newImage->dataFormat = dataFormat;
	newImage->bytesPerPixel = bytesPerPixel;
	newImage->width = cx;
	newImage->height = cy;
	newImage->lineAlignment = lineAlignment;
	newImage->flipped = flipped;
	newImage->setFormatName(formatName);
	newImage->allocateImageData();
	rowSize = getRowSize();
	newRowSize = newImage->getRowSize();
	for (i = 0; i < cy; i++)
	{
		if (flipped)
		{
			memcpy(newImage->imageData + (cy - i - 1) * newRowSize,
				imageData + (height - y - i - 1) * rowSize + x * bytesPerPixel,
				newRowSize);
		}
		else
		{
			memcpy(newImage->imageData + i * newRowSize,
				imageData + (y + i) * rowSize + x * bytesPerPixel, newRowSize);
		}
	}
	return newImage;
}

void TCImage::setComment(const char *value)
{
	if (value != comment)
	{
		delete[] comment;
		comment = copyString(value);
	}
}

void TCImage::autoCrop(void)
{
	autoCrop(imageData[0], imageData[1], imageData[2]);
}

void TCImage::autoCrop(TCUShort r, TCUShort g, TCUShort b)
{
	int minx = 0;
	int maxx = width - 1;
	int miny = 0;
	int maxy = height - 1;
	int bytesPerLine;
	bool found = false;
	int x;
	int y;

	bytesPerLine = roundUp(width * bytesPerPixel, 4);
	for (x = 0; x < width && !found; x++)
	{
		int xOffset = x * bytesPerPixel;

		for (y = 0; y < height && !found; y++)
		{
			int offset = xOffset + y * bytesPerLine;

			if (imageData[offset] != r || imageData[offset + 1] != g ||
				imageData[offset + 2] != b)
			{
				found = true;
				minx = x;
			}
		}
	}
	if (found)
	{
		found = false;
		for (y = 0; y < height && !found; y++)
		{
			int yOffset = y * bytesPerLine;

			for (x = 0; x < width && !found; x++)
			{
				int offset = yOffset + x * bytesPerPixel;

				if (imageData[offset] != r || imageData[offset + 1] != g ||
					imageData[offset + 2] != b)
				{
					found = true;
					miny = y;
				}
			}
		}
		found = false;
		for (x = width - 1; x >= 0 && !found; x--)
		{
			int xOffset = x * bytesPerPixel;

			for (y = 0; y < height && !found; y++)
			{
				int offset = xOffset + y * bytesPerLine;

				if (imageData[offset] != r || imageData[offset + 1] != g ||
					imageData[offset + 2] != b)
				{
					found = true;
					maxx = x;
				}
			}
		}
		found = false;
		for (y = height - 1; y >= 0 && !found; y--)
		{
			int yOffset = y * bytesPerLine;

			for (x = 0; x < width && !found; x++)
			{
				int offset = yOffset + x * bytesPerPixel;

				if (imageData[offset] != r || imageData[offset + 1] != g ||
					imageData[offset + 2] != b)
				{
					found = true;
					maxy = y;
				}
			}
		}
	}
	else
	{
		maxx = 0;
		maxy = 0;
	}
	int newWidth;
	int newHeight;
	int newBytesPerLine;
	TCByte *newImageData;

	croppedX = minx;
	croppedY = miny;
	newWidth = maxx - minx + 1;
	newHeight = maxy - miny + 1;
	newBytesPerLine = roundUp(newWidth * bytesPerPixel, 4);
	newImageData = new TCByte[newHeight * newBytesPerLine];
	for (y = 0; y < newHeight; y++)
	{
		memcpy(&newImageData[y * newBytesPerLine],
			&imageData[(y + miny) * bytesPerLine + minx * bytesPerPixel],
			newBytesPerLine);
	}
	if (!userImageData)
	{
		delete[] imageData;
	}
	imageData = newImageData;
	userImageData = false;
	width = newWidth;
	height = newHeight;
}

TCImageOptions *TCImage::getCompressionOptions(void)
{
	if (!compressionOptions)
	{
		TCImageFormat *imageFormat = formatWithName(formatName);

		if (imageFormat)
		{
			compressionOptions = imageFormat->newCompressionOptions();
		}
	}
	return compressionOptions;
}

TCImage *TCImage::getScaledImage(
	double scaleFactor,
	bool premultipliedAlpha /*= false*/)
{
	return getScaledImage((int)(width * scaleFactor),
		(int)(height * scaleFactor), premultipliedAlpha);
}

TCImage *TCImage::getScaledImage(
	int scaledWidth,
	int scaledHeight,
	bool premultipliedAlpha /*= false*/)
{
	TCImage* other = new TCImage;
	other->setDataFormat(dataFormat);
	other->setSize(scaledWidth, scaledHeight);
	other->setDpi(dpi);
	other->setLineAlignment(lineAlignment);
	other->setFlipped(flipped);
	other->allocateImageData();
	int alphaChannel = STBIR_ALPHA_CHANNEL_NONE;
	if (bytesPerPixel == 4)
	{
		alphaChannel = 3;
	}
	int flags = premultipliedAlpha ? STBIR_FLAG_ALPHA_PREMULTIPLIED : 0;
	if (stbir_resize_uint8_generic(imageData, width, height, getRowSize(),
		other->imageData, other->width, other->height, other->getRowSize(),
		bytesPerPixel, alphaChannel, flags, STBIR_EDGE_CLAMP,
		STBIR_FILTER_DEFAULT, STBIR_COLORSPACE_SRGB, NULL) == 0)
	{
		other->release();
		return NULL;
	}
	return other;
}

#ifdef WIN32

// Note: static method
TCImage *TCImage::createFromResource(
	HMODULE hModule,
	int resourceId,
	int lineAlignment /*= 1*/,
	bool flipped /*= false*/,
	double scaleFactor /*= 1.0*/)
{
	TCImage* image = NULL;
	double imageScale = 1.0;
	if (scaleFactor > 1.0)
	{
		image = createFromResource(hModule, resourceId, lineAlignment, flipped,
			RT_PNGDATA_2X);
		imageScale = 2.0;
	}
	if (image == NULL)
	{
		image = createFromResource(hModule, resourceId, lineAlignment, flipped,
			RT_PNGDATA_1X);
		imageScale = 1.0;
	}
	if (image != NULL)
	{
		if (scaleFactor == imageScale)
		{
			return image;
		}
		// Windows 32-bit BMPs are expected to have pre-multiplied alpha.
		// It took me MANY hours to figure this out.
		TCImage* scaledImage = image->getScaledImage(scaleFactor / imageScale,
			true);
		image->release();
		return scaledImage;
	}
	return NULL;
}

TCImage *TCImage::createFromResource(
	HMODULE hModule,
	int resourceId,
	int lineAlignment,
	bool flipped,
	LPCTSTR resourceType)
{
	HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(resourceId),
		resourceType);
	TCImage *retVal = NULL;

	if (hResource != NULL)
	{
		HGLOBAL hGlobal = LoadResource(hModule, hResource);

		if (hGlobal != NULL)
		{
			// Note: data below does not need to be unlocked/freed/etc.
			TCByte *data = (TCByte *)LockResource(hGlobal);

			if (data)
			{
				DWORD length = SizeofResource(hModule, hResource);

				if (length)
				{
					retVal = new TCImage;

					retVal->setFlipped(flipped);
					retVal->setLineAlignment(lineAlignment);
					if (!retVal->loadData(data, length))
					{
						retVal->release();
						retVal = NULL;
					}
				}
			}
		}
	}
	return retVal;
}

// Note: static method.
HBITMAP TCImage::createDIBSection(
	HDC hBitmapDC,
	int bitmapWidth,
	int bitmapHeight,
	BYTE **bmBuffer,
	bool force32 /*= false*/)
{
	BITMAPINFO bmi;

	memset(&bmi, 0, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = bitmapWidth;
	bmi.bmiHeader.biHeight = bitmapHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = force32 ? 32 : 24;
	bmi.bmiHeader.biCompression = BI_RGB;
	// NOTE: CreateDIBSection ignores the DPI settings, so don't even try to set
	// them.
	return CreateDIBSection(hBitmapDC, &bmi, DIB_RGB_COLORS, (void**)bmBuffer,
		NULL, 0);
}

HBITMAP TCImage::createMask(
	bool updateSource /*= false*/,
	TCByte threshold /*= 128*/)
{
	TCByte *dstData;
	int dstBytesPerLine;
	int srcBytesPerLine = getRowSize();
	HBITMAP hNewBitmap = NULL;
	int maskSize;

	dstBytesPerLine = roundUp((width + 7) / 8, 2);
	maskSize = dstBytesPerLine * height;
	dstData = new TCByte[maskSize];
	memset(dstData, 0, maskSize);
	for (int y = 0; y < height; y++)
	{
		int srcYOffset = flipped ? srcBytesPerLine * (height - y - 1) :
			srcBytesPerLine * y;
		int dstYOffset = dstBytesPerLine * y;

		for (int x = 0; x < width; x++)
		{
			TCByte alpha = imageData[srcYOffset + x * 4 + 3];

			if (alpha < threshold)
			{
				int byteOffset = dstYOffset + x / 8;
				int bitOffset = 7 - (x % 8);

				dstData[byteOffset] |= (1 << bitOffset);
				if (updateSource)
				{
					imageData[srcYOffset + x * 4 + 0] = 0;
					imageData[srcYOffset + x * 4 + 1] = 0;
					imageData[srcYOffset + x * 4 + 2] = 0;
				}
			}
		}
	}
	hNewBitmap = CreateBitmap(width, height, 1, 1, dstData);
	delete[] dstData;
	return hNewBitmap;
}

HBITMAP TCImage::createBmp(bool force32 /*= false*/, int rightPad /*= 0*/)
{
	HDC hdc = CreateCompatibleDC(NULL);
	BYTE *bmBuffer = NULL;
	int srcBytesPerLine = getRowSize();
	int dstBytesPerPixel = force32 ? 4 : 3;
	int dstBytesPerLine = roundUp((width + rightPad) * dstBytesPerPixel, 4);
	// Windows 32-bit BMPs are expected to have pre-multiplied alpha.
	// It took me MANY hours to figure this out. If force32 is set, alphaNum
	// will be changed to the alpha value for each pixel, and then used
	// as the numberator in an alphaNum / 255 expression.
	TCByte alphaNum = 255;

	HBITMAP hBitmap = createDIBSection(hdc, width + rightPad, height, &bmBuffer,
		force32);
	if (rightPad != 0)
	{
		// I THINK it's already all zeroes when it comes out of
		// CreateDIBSection, but I can't find official documentation that states
		// that, so do this to be absolutely sure. Note that if rightPad is 0,
		// every single byte in bmBuffer will be written to in the for loop
		// below, so there is no need to clear that memory.
		memset(bmBuffer, 0, dstBytesPerLine * height);
	}
	for (int y = 0; y < height; y++)
	{
		int srcYOffset = srcBytesPerLine * y;
		int dstYOffset = dstBytesPerLine * y;

		for (int x = 0; x < width; x++)
		{
			alphaNum = imageData[srcYOffset + x * 4 + 3];
			if (force32)
			{
				bmBuffer[dstYOffset + x * dstBytesPerPixel + 3] =
					alphaNum;
			}
			bmBuffer[dstYOffset + x * dstBytesPerPixel + 0] =
				imageData[srcYOffset + x * 4 + 2] * alphaNum / 255;
			bmBuffer[dstYOffset + x * dstBytesPerPixel + 1] =
				imageData[srcYOffset + x * 4 + 1] * alphaNum / 255;
			bmBuffer[dstYOffset + x * dstBytesPerPixel + 2] =
				imageData[srcYOffset + x * 4 + 0] * alphaNum / 255;
		}
	}
	DeleteDC(hdc);
	return hBitmap;
}

void TCImage::getBmpAndMask(
	HBITMAP &hBitmap,
	HBITMAP &hMask,
	bool updateSource /*= false*/,
	bool force32 /*= false*/)
{
	TCByte threshold = force32 ? 1 : 128;
	hMask = createMask(updateSource, threshold);
	hBitmap = createBmp(force32);
}

HBITMAP TCImage::loadBmpFromPngResource(
	HMODULE hModule,
	int resourceId,
	double scaleFactor /*= 1.0*/,
	bool force32 /*= false*/,
	int rightPad /*= 0*/)
{
	TCImage *image = TCImage::createFromResource(hModule, resourceId, 4, true,
		scaleFactor);
	if (image == NULL)
	{
		return NULL;
	}
	HBITMAP hBitmap = image->createBmp(force32, rightPad);
	image->release();
	return hBitmap;
}

HICON TCImage::loadIconFromPngResource(
	HMODULE hModule,
	int resourceId,
	double scaleFactor /*= 1.0*/,
	bool force32 /*= false*/)
{
	TCImage *image = TCImage::createFromResource(hModule, resourceId, 4, true,
		scaleFactor);

	if (image == NULL)
	{
		return NULL;
	}
	HBITMAP hBitmap;
	HBITMAP hMask;
	ICONINFO ii;
	HICON hIcon;

	image->getBmpAndMask(hBitmap, hMask, true, force32);
	image->release();
	memset(&ii, 0, sizeof(ii));
	ii.fIcon = FALSE;
	ii.hbmMask = hMask;
	ii.hbmColor = hBitmap;
	hIcon = CreateIconIndirect(&ii);
	if (hBitmap)
	{
		DeleteObject(hBitmap);
	}
	if (hMask)
	{
		DeleteObject(hMask);
	}
	return hIcon;
}

#endif // WIN32
