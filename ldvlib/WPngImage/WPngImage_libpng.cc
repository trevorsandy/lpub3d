/* WPngImage
   ---------
   This source code is released under the MIT license.
   For full documentation, consult the WPngImage.html file.
*/

#include "WPngImage.hh"
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cassert>
#include <png.h>

#if !WPNGIMAGE_RESTRICT_TO_CPP98
#define WPNGIMAGE_DELETED = delete
#else
#define WPNGIMAGE_DELETED
#endif

const bool WPngImage::isUsingLibpng = true;

//============================================================================
// Auxiliary functions for reading PNG data
//============================================================================
static WPngImage::PngFileFormat getFileFormat(unsigned bitDepth, unsigned colorType)
{
    if(bitDepth == 16)
        return (colorType == PNG_COLOR_TYPE_GRAY ||
                colorType == PNG_COLOR_TYPE_GRAY_ALPHA ?
                WPngImage::kPngFileFormat_GA16 :
                WPngImage::kPngFileFormat_RGBA16);
    return (colorType == PNG_COLOR_TYPE_GRAY ||
            colorType == PNG_COLOR_TYPE_GRAY_ALPHA ?
            WPngImage::kPngFileFormat_GA8 :
            WPngImage::kPngFileFormat_RGBA8);
}


//============================================================================
// Auxiliary struct for reading/writing PNG data
//============================================================================
struct WPngImage::PngStructs
{
    png_structp mPngStructPtr;
    png_infop mPngInfoPtr;
    bool mReading;
    std::string mPngLibErrorMsg;

    PngStructs(bool);
    ~PngStructs();
    PngStructs(const PngStructs&) WPNGIMAGE_DELETED;
    PngStructs& operator=(const PngStructs&) WPNGIMAGE_DELETED;

    static void handlePngError(png_structp, png_const_charp);
    static void handlePngWarning(png_structp, png_const_charp);
};

WPngImage::PngStructs::PngStructs(bool forReading):
    mPngStructPtr(0), mPngInfoPtr(0), mReading(forReading), mPngLibErrorMsg()
{
    if(forReading)
        mPngStructPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    else
        mPngStructPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

    if(mPngStructPtr) mPngInfoPtr = png_create_info_struct(mPngStructPtr);

    png_set_error_fn(mPngStructPtr, this, &handlePngError, &handlePngWarning);
}

WPngImage::PngStructs::~PngStructs()
{
    if(mReading)
    {
        if(mPngStructPtr && mPngInfoPtr) png_destroy_read_struct(&mPngStructPtr, &mPngInfoPtr, 0);
        else if(mPngStructPtr) png_destroy_read_struct(&mPngStructPtr, 0, 0);
    }
    else
    {
        if(mPngStructPtr && mPngInfoPtr) png_destroy_write_struct(&mPngStructPtr, &mPngInfoPtr);
        else if(mPngStructPtr) png_destroy_write_struct(&mPngStructPtr, 0);
    }
}

void WPngImage::PngStructs::handlePngError(png_structp png_ptr, png_const_charp errorMsg)
{
    WPngImage::PngStructs* obj =
        reinterpret_cast<WPngImage::PngStructs*>(png_get_error_ptr(png_ptr));
    obj->mPngLibErrorMsg = errorMsg;
    longjmp(png_jmpbuf(png_ptr), 1);
}

void WPngImage::PngStructs::handlePngWarning(png_structp, png_const_charp)
{}

namespace
{
    struct FilePtr
    {
        std::FILE* fp;

        FilePtr(): fp(0) {}
        ~FilePtr() { if(fp) std::fclose(fp); }

     private:
        FilePtr(const FilePtr&);
        FilePtr& operator=(const FilePtr&);
    };

    inline WPngImage::UInt16 getPNGComponent16
    (const std::vector<unsigned char>& rawImageData, std::size_t index)
    {
        return ((WPngImage::UInt16(rawImageData[index]) << 8) |
                WPngImage::UInt16(rawImageData[index + 1]));
    }

    inline void setPNGComponent16
    (std::vector<unsigned char>& rawImageData, std::size_t index, WPngImage::UInt16 value)
    {
        rawImageData[index] = (unsigned char)(value >> 8);
        rawImageData[index + 1] = (unsigned char)value;
    }
}


//============================================================================
// Read PNG data
//============================================================================
WPngImage::IOStatus WPngImage::readPngData
(PngStructs& structs, bool useConversion, PngReadConvert conversion, PixelFormat pixelFormat)
{
    png_read_info(structs.mPngStructPtr, structs.mPngInfoPtr);

    const int imageWidth = png_get_image_width(structs.mPngStructPtr, structs.mPngInfoPtr);
    const int imageHeight = png_get_image_height(structs.mPngStructPtr, structs.mPngInfoPtr);
    const unsigned bitDepth = png_get_bit_depth(structs.mPngStructPtr, structs.mPngInfoPtr);
    const unsigned colorType = png_get_color_type(structs.mPngStructPtr, structs.mPngInfoPtr);
    const PngFileFormat fileFormat = ::getFileFormat(bitDepth, colorType);

    png_set_add_alpha(structs.mPngStructPtr, 0xffff, PNG_FILLER_AFTER);
    png_set_palette_to_rgb(structs.mPngStructPtr);
    png_set_gray_to_rgb(structs.mPngStructPtr);

    png_read_update_info(structs.mPngStructPtr, structs.mPngInfoPtr);
    const unsigned rowBytes = png_get_rowbytes(structs.mPngStructPtr, structs.mPngInfoPtr);

    newImage(imageWidth, imageHeight,
             useConversion ? getPixelFormat(conversion, fileFormat) : pixelFormat);
    if(!mData) return kIOStatus_Ok;

    setFileFormat(fileFormat);

    if(bitDepth == 16)
    {
        assert(rowBytes <= 4*2*unsigned(imageWidth));
        std::vector<unsigned char> dataRow(4*2*imageWidth);

        for(int y = 0, destIndex = 0; y < imageHeight; ++y)
        {
            png_read_row(structs.mPngStructPtr, (png_bytep) &dataRow[0], 0);
            for(int x = 0; x < imageWidth*8; x += 8, ++destIndex)
                setPixel(std::size_t(destIndex),
                         Pixel16(getPNGComponent16(dataRow, x),
                                 getPNGComponent16(dataRow, x + 2),
                                 getPNGComponent16(dataRow, x + 4),
                                 getPNGComponent16(dataRow, x + 6)));
        }
    }
    else
    {
        assert(rowBytes <= 4*unsigned(imageWidth));
        std::vector<Byte> dataRow(imageWidth * 4);

        for(int y = 0, destIndex = 0; y < imageHeight; ++y)
        {
            png_read_row(structs.mPngStructPtr, (png_bytep) &dataRow[0], 0);
            for(int x = 0; x < imageWidth*4; x += 4, ++destIndex)
                setPixel(std::size_t(destIndex),
                         Pixel8(dataRow[x], dataRow[x+1], dataRow[x+2], dataRow[x+3]));
        }
    }

    png_read_end(structs.mPngStructPtr, structs.mPngInfoPtr);
    return kIOStatus_Ok;
}


//----------------------------------------------------------------------------
// Load PNG image from file
//----------------------------------------------------------------------------
WPngImage::IOStatus WPngImage::performLoadImage
(const char* fileName, bool useConversion, PngReadConvert conversion, PixelFormat pixelFormat)
{
    FilePtr iFile;
    iFile.fp = std::fopen(fileName, "rb");
    if(!iFile.fp) return IOStatus(kIOStatus_Error_CantOpenFile, errno);

    png_byte header[8] = {};
    std::fread(header, 1, 8, iFile.fp);
    if(png_sig_cmp(header, 0, 8)) return kIOStatus_Error_NotPNG;
    std::fseek(iFile.fp, 0, SEEK_SET);

    PngStructs structs(true);
    if(!structs.mPngInfoPtr) return kIOStatus_Error_PNGLibraryError;

    if(setjmp(png_jmpbuf(structs.mPngStructPtr)))
        return IOStatus(kIOStatus_Error_PNGLibraryError, structs.mPngLibErrorMsg);

    png_init_io(structs.mPngStructPtr, iFile.fp);
    return readPngData(structs, useConversion, conversion, pixelFormat);
}


//----------------------------------------------------------------------------
// Read PNG data from RAM
//----------------------------------------------------------------------------
namespace
{
    struct RAMPngData
    {
        png_const_charp mData;
        std::size_t mDataSize, mCurrentDataIndex;
    };
}

static void pngDataReader(png_structp png_ptr, png_bytep data, png_size_t length)
{
    RAMPngData* obj = reinterpret_cast<RAMPngData*>(png_get_io_ptr(png_ptr));

    if(obj->mCurrentDataIndex + length > obj->mDataSize)
        png_error(png_ptr, "Read error");
    else
    {
        std::memcpy(data, obj->mData + obj->mCurrentDataIndex, length);
        obj->mCurrentDataIndex += length;
    }
}

WPngImage::IOStatus WPngImage::performLoadImageFromRAM
(const void* pngData, std::size_t pngDataSize,
 bool useConversion, PngReadConvert conversion, PixelFormat pixelFormat)
{
    RAMPngData ramPngData;
    ramPngData.mData = (png_const_charp)pngData;
    ramPngData.mDataSize = pngDataSize;
    ramPngData.mCurrentDataIndex = 0;

    if(png_sig_cmp((png_bytep)ramPngData.mData, 0, 8)) return kIOStatus_Error_NotPNG;

    PngStructs structs(true);
    if(!structs.mPngInfoPtr) return kIOStatus_Error_PNGLibraryError;

    if(setjmp(png_jmpbuf(structs.mPngStructPtr)))
        return IOStatus(kIOStatus_Error_PNGLibraryError, structs.mPngLibErrorMsg);

    png_set_read_fn(structs.mPngStructPtr, &ramPngData, &pngDataReader);
    return readPngData(structs, useConversion, conversion, pixelFormat);
}


//============================================================================
// Write PNG data
//============================================================================
void WPngImage::performWritePngData
(PngStructs& structs, PngFileFormat fileFormat,
 int bitDepth, int colorType, int colorComponents) const
{
    const int imageWidth = width(), imageHeight = height();

    png_set_IHDR(structs.mPngStructPtr, structs.mPngInfoPtr, imageWidth, imageHeight,
                 bitDepth, colorType, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(structs.mPngStructPtr, structs.mPngInfoPtr);

    if(bitDepth == 16)
    {
        std::vector<UInt16> rowData(imageWidth * colorComponents);
        std::vector<unsigned char> rowDataBytes(rowData.size() * 2);
        for(int y = 0; y < imageHeight; ++y)
        {
            setPixelRow(fileFormat, y, &rowData[0], colorComponents);
            for(std::size_t i = 0; i < rowData.size(); ++i)
                setPNGComponent16(rowDataBytes, i * 2, rowData[i]);
            png_write_row(structs.mPngStructPtr, (png_bytep)(&rowDataBytes[0]));
        }
    }
    else
    {
        std::vector<Byte> rowData(imageWidth * colorComponents);
        for(int y = 0; y < imageHeight; ++y)
        {
            setPixelRow(fileFormat, y, &rowData[0], colorComponents);
            png_write_row(structs.mPngStructPtr, (png_bytep)(&rowData[0]));
        }
    }

    png_write_end(structs.mPngStructPtr, structs.mPngInfoPtr);
}

WPngImage::IOStatus WPngImage::writePngData(PngStructs& structs, PngFileFormat fileFormat) const
{
    const bool writeAlphas = !allPixelsHaveFullAlpha();

    switch(fileFormat)
    {
      case kPngFileFormat_GA8:
          performWritePngData
              (structs, fileFormat, 8,
               writeAlphas ? PNG_COLOR_TYPE_GRAY_ALPHA : PNG_COLOR_TYPE_GRAY,
               writeAlphas ? 2 : 1);
          break;

      case kPngFileFormat_GA16:
          performWritePngData
              (structs, fileFormat, 16,
               writeAlphas ? PNG_COLOR_TYPE_GRAY_ALPHA : PNG_COLOR_TYPE_GRAY,
               writeAlphas ? 2 : 1);
          break;

      case kPngFileFormat_none:
      case kPngFileFormat_RGBA8:
          performWritePngData
              (structs, fileFormat, 8,
               writeAlphas ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
               writeAlphas ? 4 : 3);
          break;

      case kPngFileFormat_RGBA16:
          performWritePngData
              (structs, fileFormat, 16,
               writeAlphas ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
               writeAlphas ? 4 : 3);
          break;
    }

    return kIOStatus_Ok;
}


//----------------------------------------------------------------------------
// Save PNG image to file
//----------------------------------------------------------------------------
WPngImage::IOStatus
WPngImage::performSaveImage(const char* fileName, PngFileFormat fileFormat) const
{
    if(!mData) return kIOStatus_Ok;

    FilePtr oFile;
    oFile.fp = std::fopen(fileName, "wb");
    if(!oFile.fp) return IOStatus(kIOStatus_Error_CantOpenFile, errno);

    PngStructs structs(false);
    if(!structs.mPngInfoPtr) return kIOStatus_Error_PNGLibraryError;

    if(setjmp(png_jmpbuf(structs.mPngStructPtr)))
        return IOStatus(kIOStatus_Error_PNGLibraryError, structs.mPngLibErrorMsg);

    png_init_io(structs.mPngStructPtr, oFile.fp);

    return writePngData(structs, fileFormat == kPngFileFormat_none ?
                        getClosestMatchFileFormat(currentPixelFormat()) : fileFormat);
}


//----------------------------------------------------------------------------
// Write PNG data to RAM
//----------------------------------------------------------------------------
namespace
{
    struct PngDestData
    {
        std::vector<unsigned char>* destVector;
        WPngImage::ByteStreamOutputFunc destFunc;
        PngDestData(): destVector(0), destFunc(0) {}
        PngDestData(const PngDestData&) WPNGIMAGE_DELETED;
        PngDestData& operator=(const PngDestData&) WPNGIMAGE_DELETED;
    };
}

static void pngDataWriter(png_structp png_ptr, png_bytep data, png_size_t length)
{
    PngDestData* obj = reinterpret_cast<PngDestData*>(png_get_io_ptr(png_ptr));
    if(obj->destVector) obj->destVector->insert(obj->destVector->end(), data, data + length);
    if(obj->destFunc) obj->destFunc((const unsigned char*)data, length);
}

static void pngDataFlush(png_structp)
{}

WPngImage::IOStatus WPngImage::performSaveImageToRAM
(std::vector<unsigned char>* destVector, ByteStreamOutputFunc destFunc,
 PngFileFormat fileFormat) const
{
    if(!mData) return kIOStatus_Ok;

    PngStructs structs(false);
    if(!structs.mPngInfoPtr) return kIOStatus_Error_PNGLibraryError;

    if(setjmp(png_jmpbuf(structs.mPngStructPtr)))
        return IOStatus(kIOStatus_Error_PNGLibraryError, structs.mPngLibErrorMsg);

    PngDestData destData;
    destData.destVector = destVector;
    destData.destFunc = destFunc;

    png_set_write_fn(structs.mPngStructPtr, &destData, &pngDataWriter, &pngDataFlush);

    return writePngData(structs, fileFormat == kPngFileFormat_none ?
                        getClosestMatchFileFormat(currentPixelFormat()) : fileFormat);
}
