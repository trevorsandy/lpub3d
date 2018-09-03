/* WPngImage
   ---------
   This source code is released under the MIT license.
   For full documentation, consult the WPngImage.html file.
*/

#ifndef WPNGIMAGE_RESTRICT_TO_CPP98
#define WPNGIMAGE_RESTRICT_TO_CPP98 0
#endif

#ifndef WPNGIMAGE_INCLUDE_GUARD
#define WPNGIMAGE_INCLUDE_GUARD

//============================================================================
// Includes and macro definitions
//============================================================================
#include <string>
#include <vector>
#include <cstddef>
#include <iostream>
#if !WPNGIMAGE_RESTRICT_TO_CPP98
#include <cstdint>
#include <functional>
#define WPNGIMAGE_CONSTEXPR constexpr
#else
#include <climits>
#if CHAR_BIT != 8 || UCHAR_MAX != 255
#error "WPngImage requires 8-bit bytes"
#endif
#define WPNGIMAGE_CONSTEXPR
#endif

#define WPNGIMAGE_VERSION 0x010400
#define WPNGIMAGE_VERSION_STRING "1.4.0"
#define WPNGIMAGE_COPYRIGHT_STRING "WPngImage v" WPNGIMAGE_VERSION_STRING " (C)2018 Juha Nieminen"


//============================================================================
// WPngImage class
//============================================================================
class WPngImage
{
 public:
    //------------------------------------------------------------------------
    // Types
    //------------------------------------------------------------------------
#if !WPNGIMAGE_RESTRICT_TO_CPP98
    using Byte = std::uint8_t;
    using UInt16 = std::uint16_t;
    using Float = float;
    using Int32 = std::int_fast32_t;
#else
    typedef unsigned char Byte;
    typedef float Float;
#if UINT_MAX == 65535
    typedef unsigned int UInt16;
#elif USHRT_MAX == 65535
    typedef unsigned short UInt16;
#else
#error "WPngImage requires an unsigned 16-bit integer type"
#endif
#if INT_MAX >= 2147483647
    typedef int Int32;
#else
    typedef long Int32;
#endif
#endif

    struct Pixel8;
    struct Pixel16;
    struct PixelF;

    struct HSV { Float h, s, v, a; };
    struct HSL { Float h, s, l, a; };
    struct XYZ { Float x, y, z, a; };
    struct YXY { Float Y, x, y, a; };
    struct CMY { Float c, m, y, a; };
    struct CMYK { Float c, m, y, k, a; };


    //------------------------------------------------------------------------
    // Enumerations
    //------------------------------------------------------------------------
    enum PngFileFormat
    {
        kPngFileFormat_none,
        kPngFileFormat_GA8,
        kPngFileFormat_GA16,
        kPngFileFormat_RGBA8,
        kPngFileFormat_RGBA16
    };

    enum PixelFormat
    {
        kPixelFormat_GA8,
        kPixelFormat_GA16,
        kPixelFormat_GAF,
        kPixelFormat_RGBA8,
        kPixelFormat_RGBA16,
        kPixelFormat_RGBAF
    };

    enum PngReadConvert
    {
        kPngReadConvert_closestMatch,
        kPngReadConvert_8bit,
        kPngReadConvert_16bit,
        kPngReadConvert_Float,
        kPngReadConvert_Grayscale,
        kPngReadConvert_RGBA
    };

    enum PngWriteConvert
    {
        kPngWriteConvert_original,
        kPngWriteConvert_closestMatch
    };


    //------------------------------------------------------------------------
    // Constructors, assignment, destructor
    //------------------------------------------------------------------------
    WPngImage();
    WPngImage(int width, int height, PixelFormat = kPixelFormat_RGBA8);
    WPngImage(int width, int height, Pixel8, PixelFormat = kPixelFormat_RGBA8);
    WPngImage(int width, int height, Pixel16, PixelFormat = kPixelFormat_RGBA16);
    WPngImage(int width, int height, PixelF, PixelFormat = kPixelFormat_RGBAF);

    WPngImage(const WPngImage&);
    WPngImage& operator=(const WPngImage&);
    ~WPngImage();

#if !WPNGIMAGE_RESTRICT_TO_CPP98
    WPngImage(WPngImage&&) noexcept;
    WPngImage& operator=(WPngImage&&) noexcept;
#endif

    void swap(WPngImage&);
    void move(WPngImage&);

    static const bool isUsingLibpng;


    //------------------------------------------------------------------------
    // Image loading and creation
    //------------------------------------------------------------------------
    enum IOStatusValue
    {
        kIOStatus_Ok,
        kIOStatus_Error_CantOpenFile,
        kIOStatus_Error_NotPNG,
        kIOStatus_Error_PNGLibraryError
    };

    struct IOStatus
    {
        IOStatusValue value;
        std::string fileName, pngLibErrorMsg;
        int errnoValue;

        IOStatus(IOStatusValue v): value(v), errnoValue(0) {}
        IOStatus(IOStatusValue v, int ev): value(v), errnoValue(ev) {}
        IOStatus(IOStatusValue v, const std::string& m):
            value(v), pngLibErrorMsg(m), errnoValue(0) {}

        operator IOStatusValue() const { return value; }

        bool printErrorMsg(std::ostream& = std::cerr) const;
    };

    IOStatus loadImage(const char* fileName, PngReadConvert = kPngReadConvert_closestMatch);
    IOStatus loadImage(const char* fileName, PixelFormat);

    IOStatus loadImage(const std::string& fileName,
                       PngReadConvert = kPngReadConvert_closestMatch);
    IOStatus loadImage(const std::string& fileName, PixelFormat);

    IOStatus loadImageFromRAM(const void* pngData, std::size_t pngDataSize,
                              PngReadConvert = kPngReadConvert_closestMatch);
    IOStatus loadImageFromRAM(const void* pngData, std::size_t pngDataSize, PixelFormat);

    void newImage(int width, int height, PixelFormat = kPixelFormat_RGBA8);
    void newImage(int width, int height, Pixel8, PixelFormat = kPixelFormat_RGBA8);
    void newImage(int width, int height, Pixel16, PixelFormat = kPixelFormat_RGBA16);
    void newImage(int width, int height, PixelF, PixelFormat = kPixelFormat_RGBAF);


    //------------------------------------------------------------------------
    // Image writing
    //------------------------------------------------------------------------
    IOStatus saveImage(const char* fileName,
                       PngWriteConvert = kPngWriteConvert_closestMatch) const;
    IOStatus saveImage(const char* fileName, PngFileFormat) const;

    IOStatus saveImage(const std::string& fileName,
                       PngWriteConvert = kPngWriteConvert_closestMatch) const;
    IOStatus saveImage(const std::string& fileName, PngFileFormat) const;

    IOStatus saveImageToRAM(std::vector<unsigned char>& dest,
                            PngWriteConvert = kPngWriteConvert_closestMatch) const;
    IOStatus saveImageToRAM(std::vector<unsigned char>& dest, PngFileFormat) const;

#if WPNGIMAGE_RESTRICT_TO_CPP98
    typedef void(*ByteStreamOutputFunc)(const unsigned char*, std::size_t);
#else
    using ByteStreamOutputFunc = std::function<void(const unsigned char*, std::size_t)>;
#endif
    IOStatus saveImageToRAM(ByteStreamOutputFunc,
                            PngWriteConvert = kPngWriteConvert_closestMatch) const;
    IOStatus saveImageToRAM(ByteStreamOutputFunc, PngFileFormat) const;


    //------------------------------------------------------------------------
    // Image information
    //------------------------------------------------------------------------
    int width() const { return mWidth; }
    int height() const { return mHeight; }

    PngFileFormat originalFileFormat() const;
    void setFileFormat(PngFileFormat);
    void setFileFormat(PngWriteConvert);

    PixelFormat currentPixelFormat() const;
    bool isGrayscalePixelFormat() const;
    bool isRGBAPixelFormat() const;
    bool is8BPCPixelFormat() const;
    bool is16BPCPixelFormat() const;
    bool isFloatPixelFormat() const;

    bool allPixelsHaveFullAlpha() const;
    void convertToPixelFormat(PixelFormat);


    //------------------------------------------------------------------------
    // Pixel handling
    //------------------------------------------------------------------------
    Pixel8 get8(int x, int y) const;
    Pixel16 get16(int x, int y) const;
    PixelF getF(int x, int y) const;

    void set(int x, int y, Pixel8);
    void set(int x, int y, Pixel16);
    void set(int x, int y, PixelF);

    void fill(Pixel8);
    void fill(Pixel16);
    void fill(PixelF);

#if WPNGIMAGE_RESTRICT_TO_CPP98
    typedef Pixel8(*TransformFunc8)(Pixel8);
    typedef Pixel16(*TransformFunc16)(Pixel16);
    typedef PixelF(*TransformFuncF)(PixelF);
#else
    using TransformFunc8 = std::function<Pixel8(Pixel8)>;
    using TransformFunc16 = std::function<Pixel16(Pixel16)>;
    using TransformFuncF = std::function<PixelF(PixelF)>;
#endif
    void transform(TransformFunc8);
    void transform(TransformFunc16);
    void transform(TransformFuncF);
    void transform(TransformFunc8, WPngImage& dest) const;
    void transform(TransformFunc16, WPngImage& dest) const;
    void transform(TransformFuncF, WPngImage& dest) const;

    void transform8(TransformFunc8 f) { transform(f); }
    void transform16(TransformFunc16 f) { transform(f); }
    void transformF(TransformFuncF f) { transform(f); }
    void transform8(TransformFunc8 f, WPngImage& dest) const { transform(f, dest); }
    void transform16(TransformFunc16 f, WPngImage& dest) const { transform(f, dest); }
    void transformF(TransformFuncF f, WPngImage& dest) const { transform(f, dest); }


    //------------------------------------------------------------------------
    // Image transformations
    //------------------------------------------------------------------------
    void flipHorizontally();
    void flipVertically();
    void rotate180();
    void rotate90cw();
    void rotate90ccw();


    //------------------------------------------------------------------------
    // Drawing
    //------------------------------------------------------------------------
    void drawPixel(int x, int y, Pixel8);
    void drawPixel(int x, int y, Pixel16);
    void drawPixel(int x, int y, PixelF);

    void putImage(int destX, int destY, const WPngImage&);
    void putImage(int destX, int destY, const WPngImage&,
                  int srcX, int srcY, int srcWidth, int srcHeight);

    void drawImage(int destX, int destY, const WPngImage&);
    void drawImage(int destX, int destY, const WPngImage&,
                   int srcX, int srcY, int srcWidth, int srcHeight);

    void putHorLine(int x, int y, int length, Pixel8);
    void putHorLine(int x, int y, int length, Pixel16);
    void putHorLine(int x, int y, int length, PixelF);
    void putVertLine(int x, int y, int length, Pixel8);
    void putVertLine(int x, int y, int length, Pixel16);
    void putVertLine(int x, int y, int length, PixelF);
    void drawHorLine(int x, int y, int length, Pixel8);
    void drawHorLine(int x, int y, int length, Pixel16);
    void drawHorLine(int x, int y, int length, PixelF);
    void drawVertLine(int x, int y, int length, Pixel8);
    void drawVertLine(int x, int y, int length, Pixel16);
    void drawVertLine(int x, int y, int length, PixelF);

    void putRect(int x, int y, int width, int height, Pixel8, bool filled);
    void putRect(int x, int y, int width, int height, Pixel16, bool filled);
    void putRect(int x, int y, int width, int height, PixelF, bool filled);
    void drawRect(int x, int y, int width, int height, Pixel8, bool filled);
    void drawRect(int x, int y, int width, int height, Pixel16, bool filled);
    void drawRect(int x, int y, int width, int height, PixelF, bool filled);

    void resizeCanvas(int newOriginX, int newOriginY, int newWidth, int newHeight);
    void resizeCanvas(int newOriginX, int newOriginY, int newWidth, int newHeight, Pixel8);
    void resizeCanvas(int newOriginX, int newOriginY, int newWidth, int newHeight, Pixel16);
    void resizeCanvas(int newOriginX, int newOriginY, int newWidth, int newHeight, PixelF);

    void premultiplyAlpha();


    //------------------------------------------------------------------------
    // Raw data
    //------------------------------------------------------------------------
    const Pixel8* getRawPixelData8() const;
    Pixel8* getRawPixelData8();
    const Pixel16* getRawPixelData16() const;
    Pixel16* getRawPixelData16();
    const PixelF* getRawPixelDataF() const;
    PixelF* getRawPixelDataF();



//----------------------------------------------------------------------------
 private:
//----------------------------------------------------------------------------
    template<typename, typename, typename> struct Pixel;
    template<typename, typename> struct IPixel;
    struct PngDataBase;
    template<typename> struct PngData;

    PngDataBase* mData;
    int mWidth, mHeight;

    template<typename Pixel_t>
    void newImageWithPixelValue(int, int, Pixel_t, PixelFormat);

    void putImage(int, int, const WPngImage&, int, int, int, int, bool);
    void manageCanvasResize(WPngImage&, int, int);
    template<typename Pixel_t> void addHorLine(int, int, int, const Pixel_t&, bool);
    template<typename Pixel_t> void addVertLine(int, int, int, const Pixel_t&, bool);
    template<typename Pixel_t> void addRect(int, int, int, int, const Pixel_t&, bool, bool);

    static PixelFormat getPixelFormat(PngReadConvert, PngFileFormat);
    static PngFileFormat getClosestMatchFileFormat(PixelFormat);
    static PngFileFormat getFileFormat(PngWriteConvert, PngFileFormat, PixelFormat);
    void setPixel(std::size_t, const Pixel8&);
    void setPixel(std::size_t, const Pixel16&);
    void setPixelRow(PngFileFormat, int, Byte*, int) const;
    void setPixelRow(PngFileFormat, int, UInt16*, int) const;

    struct PngStructs;
    IOStatus readPngData(PngStructs&, bool, PngReadConvert, PixelFormat);
    IOStatus performLoadImage(const char*, bool, PngReadConvert, PixelFormat);
    IOStatus performLoadImageFromRAM(const void*, std::size_t, bool, PngReadConvert, PixelFormat);
    IOStatus writePngData(PngStructs&, PngFileFormat) const;
    void performWritePngData(PngStructs&, PngFileFormat, int, int, int) const;
    IOStatus performSaveImage(const char*, PngFileFormat) const;
    IOStatus performSaveImageToRAM
    (std::vector<unsigned char>*, ByteStreamOutputFunc, PngFileFormat) const;
};


//============================================================================
// Pixel base class
//============================================================================
template<typename Pixel_t, typename CT, typename OperatorParam_t>
struct WPngImage::Pixel
{
    typedef CT Component_t;

    CT r, g, b, a;

    WPNGIMAGE_CONSTEXPR Pixel(CT red, CT green, CT blue, CT alpha):
        r(red), g(green), b(blue), a(alpha) {}

    WPNGIMAGE_CONSTEXPR bool operator==(const Pixel_t&) const;
    WPNGIMAGE_CONSTEXPR bool operator!=(const Pixel_t&) const;

    Pixel_t& operator+=(OperatorParam_t);
    Pixel_t operator+(OperatorParam_t) const;
    Pixel_t& operator-=(OperatorParam_t);
    Pixel_t operator-(OperatorParam_t) const;
    Pixel_t& operator*=(OperatorParam_t);
    Pixel_t operator*(OperatorParam_t) const;
    Pixel_t& operator/=(OperatorParam_t);
    Pixel_t operator/(OperatorParam_t) const;

    Pixel_t& operator+=(const Pixel_t&);
    Pixel_t operator+(const Pixel_t&) const;
    Pixel_t& operator-=(const Pixel_t&);
    Pixel_t operator-(const Pixel_t&) const;
    Pixel_t& operator*=(const Pixel_t&);
    Pixel_t operator*(const Pixel_t&) const;
    Pixel_t& operator/=(const Pixel_t&);
    Pixel_t operator/(const Pixel_t&) const;

    void set(CT v) { r=v; g=v; b=v; }
    void set(CT v, CT iA) { r=v; g=v; b=v; a=iA; }
    void set(CT iR, CT iG, CT iB) { r=iR; g=iG; b=iB; }
    void set(CT iR, CT iG, CT iB, CT iA) { r=iR; g=iG; b=iB; a=iA; }

    HSV toHSV() const;
    void set(const HSV&);
    void setFromHSV(Float hValue, Float sValue, Float vValue);
    void setFromHSV(Float hValue, Float sValue, Float vValue, Float aValue);
    HSL toHSL() const;
    void set(const HSL&);
    void setFromHSL(Float hValue, Float sValue, Float lValue);
    void setFromHSL(Float hValue, Float sValue, Float lValue, Float aValue);
    XYZ toXYZ() const;
    void set(const XYZ&);
    void setFromXYZ(Float xValue, Float yValue, Float zValue);
    void setFromXYZ(Float xValue, Float yValue, Float zValue, Float aValue);
    YXY toYXY() const;
    void set(const YXY&);
    void setFromYXY(Float YValue, Float xValue, Float yValue);
    void setFromYXY(Float YValue, Float xValue, Float yValue, Float aValue);
    CMY toCMY() const;
    void set(const CMY&);
    void setFromCMY(Float cValue, Float mValue, Float yValue);
    void setFromCMY(Float cValue, Float mValue, Float yValue, Float aValue);
    CMYK toCMYK() const;
    void set(const CMYK&);
    void setFromCMYK(Float cValue, Float mValue, Float yValue, Float kValue);
    void setFromCMYK(Float cValue, Float mValue, Float yValue, Float kValue, Float aValue);

    void blendWith(const Pixel_t& src);
    Pixel_t blendedPixel(const Pixel_t& src) const;

    void averageWith(const Pixel_t&);
    Pixel_t averagedPixel(const Pixel_t&) const;
    void averageWith(const Pixel_t*, std::size_t amount);
    Pixel_t averagedPixel(const Pixel_t*, std::size_t amount) const;

#if !WPNGIMAGE_RESTRICT_TO_CPP98
    template<typename... Rest>
    void averageWith(const Pixel_t&, Rest&&...);

    template<typename... Rest>
    Pixel_t averagedPixel(const Pixel_t&, Rest&&...) const;
#endif

    CT toGrayCIE() const;
    Pixel_t grayCIEPixel() const;
};


// ---------------------------------------------------------------------------
// Pixel base class specialization for integer component types
// ---------------------------------------------------------------------------
template<typename Pixel_t, typename CT>
struct WPngImage::IPixel: public WPngImage::Pixel<Pixel_t, CT, Int32>
{
    WPNGIMAGE_CONSTEXPR IPixel(CT red, CT green, CT blue, CT alpha):
        Pixel<Pixel_t, CT, Int32>::Pixel(red, green, blue, alpha)
    {}

    CT toGray(Int32 rWeight = 299, Int32 gWeight = 587, Int32 bWeight = 114) const;
    Pixel_t grayPixel(Int32 rWeight = 299, Int32 gWeight = 587, Int32 bWeight = 114) const;
};


//============================================================================
// WPngImage::Pixel8 definition
//============================================================================
struct WPngImage::Pixel8: public WPngImage::IPixel<Pixel8, Byte>
{
    static const Byte kComponentMaxValue = ~Byte(0);

    WPNGIMAGE_CONSTEXPR Pixel8():
        IPixel(0, 0, 0, kComponentMaxValue) {}

    explicit WPNGIMAGE_CONSTEXPR Pixel8(Byte gray):
        IPixel(gray, gray, gray, kComponentMaxValue) {}

    WPNGIMAGE_CONSTEXPR Pixel8(Byte gray, Byte alpha):
        IPixel(gray, gray, gray, alpha) {}

    WPNGIMAGE_CONSTEXPR Pixel8(Byte red, Byte green, Byte blue):
        IPixel(red, green, blue, kComponentMaxValue) {}

    WPNGIMAGE_CONSTEXPR Pixel8(Byte red, Byte green, Byte blue, Byte alpha):
        IPixel(red, green, blue, alpha) {}

    explicit Pixel8(Pixel16);
    explicit Pixel8(PixelF);
    explicit Pixel8(const HSV& hsv): IPixel(0, 0, 0, 0) { set(hsv); }
    explicit Pixel8(const HSL& hsl): IPixel(0, 0, 0, 0) { set(hsl); }
    explicit Pixel8(const XYZ& xyz): IPixel(0, 0, 0, 0) { set(xyz); }
    explicit Pixel8(const YXY& yxy): IPixel(0, 0, 0, 0) { set(yxy); }
    explicit Pixel8(const CMY& cmy): IPixel(0, 0, 0, 0) { set(cmy); }
    explicit Pixel8(const CMYK& cmyk): IPixel(0, 0, 0, 0) { set(cmyk); }

    void interpolate(const Pixel8&, Byte factor);
    Pixel8 interpolatedPixel(const Pixel8&, Byte factor) const;
    void rawInterpolate(const Pixel8&, Byte factor);
    Pixel8 rawInterpolatedPixel(const Pixel8&, Byte factor) const;

    void premultiplyAlpha();
    Pixel8 premultipliedAlphaPixel() const;
};

inline WPngImage::Pixel8 operator+(WPngImage::Int32 value, const WPngImage::Pixel8& p)
{ return p + value; }

WPngImage::Pixel8 operator-(WPngImage::Int32, const WPngImage::Pixel8&);

inline WPngImage::Pixel8 operator*(WPngImage::Int32 value, const WPngImage::Pixel8& p)
{ return p * value; }

WPngImage::Pixel8 operator/(WPngImage::Int32, const WPngImage::Pixel8&);


//============================================================================
// WPngImage::Pixel16 definition
//============================================================================
struct WPngImage::Pixel16: public WPngImage::IPixel<Pixel16, UInt16>
{
    static const UInt16 kComponentMaxValue = ~UInt16(0);

    WPNGIMAGE_CONSTEXPR Pixel16():
        IPixel(0, 0, 0, kComponentMaxValue) {}

    explicit WPNGIMAGE_CONSTEXPR Pixel16(UInt16 gray):
        IPixel(gray, gray, gray, kComponentMaxValue) {}

    WPNGIMAGE_CONSTEXPR Pixel16(UInt16 gray, UInt16 alpha):
        IPixel(gray, gray, gray, alpha) {}

    WPNGIMAGE_CONSTEXPR Pixel16(UInt16 red, UInt16 green, UInt16 blue):
        IPixel(red, green, blue, kComponentMaxValue) {}

    WPNGIMAGE_CONSTEXPR Pixel16(UInt16 red, UInt16 green, UInt16 blue, UInt16 alpha):
        IPixel(red, green, blue, alpha) {}

    explicit Pixel16(Pixel8);
    explicit Pixel16(PixelF);
    explicit Pixel16(const HSV& hsv): IPixel(0, 0, 0, 0) { set(hsv); }
    explicit Pixel16(const HSL& hsl): IPixel(0, 0, 0, 0) { set(hsl); }
    explicit Pixel16(const XYZ& xyz): IPixel(0, 0, 0, 0) { set(xyz); }
    explicit Pixel16(const YXY& yxy): IPixel(0, 0, 0, 0) { set(yxy); }
    explicit Pixel16(const CMY& cmy): IPixel(0, 0, 0, 0) { set(cmy); }
    explicit Pixel16(const CMYK& cmyk): IPixel(0, 0, 0, 0) { set(cmyk); }

    void interpolate(const Pixel16&, UInt16 factor);
    Pixel16 interpolatedPixel(const Pixel16&, UInt16 factor) const;
    void rawInterpolate(const Pixel16&, UInt16 factor);
    Pixel16 rawInterpolatedPixel(const Pixel16&, UInt16 factor) const;

    void premultiplyAlpha();
    Pixel16 premultipliedAlphaPixel() const;
};

inline WPngImage::Pixel16 operator+(WPngImage::Int32 value, const WPngImage::Pixel16& p)
{ return p + value; }

WPngImage::Pixel16 operator-(WPngImage::Int32, const WPngImage::Pixel16&);

inline WPngImage::Pixel16 operator*(WPngImage::Int32 value, const WPngImage::Pixel16& p)
{ return p * value; }

WPngImage::Pixel16 operator/(WPngImage::Int32, const WPngImage::Pixel16&);


//============================================================================
// WPngImage::PixelF definition
//============================================================================
struct WPngImage::PixelF: public WPngImage::Pixel<PixelF, Float, Float>
{
    WPNGIMAGE_CONSTEXPR PixelF():
        Pixel(0, 0, 0, 1.0f) {}

    explicit WPNGIMAGE_CONSTEXPR PixelF(Float gray):
        Pixel(gray, gray, gray, 1.0f) {}

    WPNGIMAGE_CONSTEXPR PixelF(Float gray, Float alpha):
        Pixel(gray, gray, gray, alpha) {}

    WPNGIMAGE_CONSTEXPR PixelF(Float red, Float green, Float blue):
        Pixel(red, green, blue, 1.0f) {}

    WPNGIMAGE_CONSTEXPR PixelF(Float red, Float green, Float blue, Float alpha):
        Pixel(red, green, blue, alpha) {}

    explicit PixelF(Pixel8);
    explicit PixelF(Pixel16);
    explicit PixelF(const HSV& hsv): Pixel(0, 0, 0, 0) { set(hsv); }
    explicit PixelF(const HSL& hsl): Pixel(0, 0, 0, 0) { set(hsl); }
    explicit PixelF(const XYZ& xyz): Pixel(0, 0, 0, 0) { set(xyz); }
    explicit PixelF(const YXY& yxy): Pixel(0, 0, 0, 0) { set(yxy); }
    explicit PixelF(const CMY& cmy): Pixel(0, 0, 0, 0) { set(cmy); }
    explicit PixelF(const CMYK& cmyk): Pixel(0, 0, 0, 0) { set(cmyk); }

    Float toGray(Float rWeight = 0.299f, Float gWeight = 0.587f, Float bWeight = 0.114f) const;
    PixelF grayPixel(Float rWeight = 0.299f, Float gWeight = 0.587f, Float bWeight = 0.114f) const;
    void interpolate(const PixelF&, Float factor);
    PixelF interpolatedPixel(const PixelF&, Float factor) const;
    void rawInterpolate(const PixelF&, Float factor);
    PixelF rawInterpolatedPixel(const PixelF&, Float factor) const;

    void premultiplyAlpha();
    PixelF premultipliedAlphaPixel() const;

    void clamp();
    PixelF clampedPixel() const;
};

inline WPngImage::PixelF operator+(WPngImage::Float value, const WPngImage::PixelF& p)
{ return p + value; }

WPngImage::PixelF operator-(WPngImage::Float, const WPngImage::PixelF&);

inline WPngImage::PixelF operator*(WPngImage::Float value, const WPngImage::PixelF& p)
{ return p * value; }

WPngImage::PixelF operator/(WPngImage::Float, const WPngImage::PixelF&);


//============================================================================
// Pixel base class inline function implementations
//============================================================================
template<typename Pixel_t, typename CT, typename OperatorParam_t>
WPNGIMAGE_CONSTEXPR
bool WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator==(const Pixel_t& rhs) const
{
    return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
WPNGIMAGE_CONSTEXPR
bool WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator!=(const Pixel_t& rhs) const
{
    return r != rhs.r || g != rhs.g || b != rhs.b || a != rhs.a;
}

#if !WPNGIMAGE_RESTRICT_TO_CPP98
template<typename Pixel_t, typename CT, typename OperatorParam_t>
template<typename... Rest>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::averageWith
(const Pixel_t& first, Rest&&... rest)
{
    const Pixel_t pixels[] = { first, rest... };
    averageWith(pixels, 1 + sizeof...(Rest));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
template<typename... Rest>
Pixel_t WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::averagedPixel
(const Pixel_t& first, Rest&&... rest) const
{
    const Pixel_t pixels[] = { first, rest... };
    return averagedPixel(pixels, 1 + sizeof...(Rest));
}
#endif

#if !WPNGIMAGE_RESTRICT_TO_CPP98
#undef WPNGIMAGE_CONSTEXPR
#endif
#endif
