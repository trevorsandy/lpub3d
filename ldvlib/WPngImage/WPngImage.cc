/* WPngImage
   ---------
   This source code is released under the MIT license.
   For full documentation, consult the WPngImage.html file.
*/

#include "WPngImage.hh"
#include <vector>
#include <cmath>
#include <algorithm>
#include <utility>
#include <limits>
#include <cstring>

typedef WPngImage::Byte Byte;
typedef WPngImage::UInt16 UInt16;
typedef WPngImage::Float Float;
typedef WPngImage::Int32 Int32;
typedef std::size_t StdSize_t;

#if !WPNGIMAGE_RESTRICT_TO_CPP98
typedef std::uint_fast32_t UInt32;
#else
#if UINT_MAX >= 4294967295
typedef unsigned UInt32;
#else
typedef unsigned long UInt32;
#endif
#endif


//============================================================================
// Operations for different component types
//============================================================================
namespace
{
    //------------------------------------------------------------------------
    // Conversions between component types
    //------------------------------------------------------------------------
    inline Byte UInt16ToByte(UInt16 value)
    {
        return (Byte)(value >> 8);
    }

    inline UInt16 ByteToUInt16(Byte value)
    {
        return (UInt16)value | ((UInt16)value << 8);
    }

    inline Float ByteToFloat(Byte value)
    {
        return Float(value) * (1.0f / 255.0f);
    }

    inline Float UInt16ToFloat(UInt16 value)
    {
        return Float(value) * (1.0f / 65535.0f);
    }

    template<typename CT> CT floatToCT(Float);

    template<>
    inline Byte floatToCT<Byte>(Float value)
    {
        return value <= 0.0f ? 0 : value >= 1.0f ? 255U : (Byte)(value * 255.0f + 0.5f);
    }

    template<>
    inline UInt16 floatToCT<UInt16>(Float value)
    {
        return value <= 0.0f ? 0 : value >= 1.0f ? 65535U : (UInt16)(value * 65535.0f + 0.5f);
    }

    template<>
    inline Float floatToCT<Float>(Float value) { return value; }

    template<typename DestType, typename SrcType>
    DestType convertType(SrcType);

    template<> Byte convertType<Byte, Byte>(Byte c) { return c; }
    template<> Byte convertType<Byte, UInt16>(UInt16 c) { return UInt16ToByte(c); }
    template<> Byte convertType<Byte, Float>(Float c) { return floatToCT<Byte>(c); }
    template<> UInt16 convertType<UInt16, Byte>(Byte c) { return ByteToUInt16(c); }
    template<> UInt16 convertType<UInt16, UInt16>(UInt16 c) { return c; }
    template<> UInt16 convertType<UInt16, Float>(Float c) { return floatToCT<UInt16>(c); }
    template<> Float convertType<Float, Byte>(Byte c) { return ByteToFloat(c); }
    template<> Float convertType<Float, UInt16>(UInt16 c) { return UInt16ToFloat(c); }
    template<> Float convertType<Float, Float>(Float c) { return c; }


    //------------------------------------------------------------------------
    // Arithmetic operations between color components
    //------------------------------------------------------------------------
    inline Byte addValues(Byte v1, Int32 v2)
    {
        const Int32 res = Int32(v1) + v2;
        return res < 0 ? 0 : res > 255 ? 255 : res;
    }

    inline UInt16 addValues(UInt16 v1, Int32 v2)
    {
        const Int32 res = Int32(v1) + v2;
        return res < 0 ? 0 : res > 65535 ? 65535 : res;
    }

    inline Float addValues(Float v1, Float v2) { return v1 + v2; }

    inline Byte subValues(Int32 v1, Byte v2)
    {
        const Int32 res = v1 - Int32(v2);
        return res < 0 ? 0 : res > 255 ? 255 : res;
    }

    inline UInt16 subValues(Int32 v1, UInt16 v2)
    {
        const Int32 res = v1 - Int32(v2);
        return res < 0 ? 0 : res > 65535 ? 65535 : res;
    }

    inline Byte mulValues(Byte v1, Int32 v2)
    {
        const Int32 res = Int32(v1) * v2;
        return res > 255 ? 255 : res;
    }

    inline UInt16 mulValues(UInt16 v1, Int32 v2)
    {
        const Int32 res = Int32(v1) * v2;
        return res > 65535 ? 65535 : res;
    }

    inline Float mulValues(Float v1, Float v2) { return v1 * v2; }

    inline Byte divValues(Byte v1, Int32 v2)
    {
        return v2 == 0 ? 255 : v2 < 0 ? 0 : (Int32(v1) + v2/2) / v2;
    }

    inline Byte divValues(Int32 v1, Byte v2)
    {
        return v2 == 0 ? 255 : (v1 + Int32(v2)/2) / Int32(v2);
    }

    inline UInt16 divValues(UInt16 v1, Int32 v2)
    {
        return v2 == 0 ? 65535 : v2 < 0 ? 0 : (Int32(v1) + v2/2) / v2;
    }

    inline UInt16 divValues(Int32 v1, UInt16 v2)
    {
        return v2 == 0 ? 65535 : (v1 + Int32(v2)/2) / Int32(v2);
    }

    inline Float divValues(Float v1, Float v2) { return v1 / v2; }


    inline Byte addComponents(Byte v1, Byte v2) { return addValues(v1, Int32(v2)); }
    inline UInt16 addComponents(UInt16 v1, UInt16 v2) { return addValues(v1, Int32(v2)); }
    inline Float addComponents(Float v1, Float v2) { return v1 + v2; }
    inline Byte subComponents(Byte v1, Byte v2) { return addValues(v1, -Int32(v2)); }
    inline UInt16 subComponents(UInt16 v1, UInt16 v2) { return addValues(v1, -Int32(v2)); }
    inline Float subComponents(Float v1, Float v2) { return v1 - v2; }
    inline Byte mulComponents(Byte v1, Byte v2) { return mulValues(v1, Int32(v2)); }
    inline UInt16 mulComponents(UInt16 v1, UInt16 v2) { return mulValues(v1, Int32(v2)); }
    inline Float mulComponents(Float v1, Float v2) { return v1 * v2; }
    inline Byte divComponents(Byte v1, Byte v2) { return divValues(v1, Int32(v2)); }
    inline UInt16 divComponents(UInt16 v1, UInt16 v2) { return divValues(v1, Int32(v2)); }

    inline Float divComponents(Float v1, Float v2)
    {
        return v2 == 0.0f ? std::numeric_limits<Float>::max() : v1 / v2;
    }

    inline Byte componentMultipliedByAlpha(Byte c, Byte a)
    {
        return Byte((UInt32(c) * UInt32(a) + 127) / UInt32(255));
    }

    inline UInt16 componentMultipliedByAlpha(UInt16 c, UInt16 a)
    {
        return UInt16((UInt32(c) * UInt32(a) + 32767) / UInt32(65535));
    }

    inline Float componentMultipliedByAlpha(Float c, Float a)
    {
        return c * a;
    }


    //------------------------------------------------------------------------
    // Averaging and blending between color components
    //------------------------------------------------------------------------
    inline Byte average(Byte v1, Byte v2) { return Byte((Int32(v1) + Int32(v2)) / 2); }
    inline UInt16 average(UInt16 v1, UInt16 v2) { return UInt16((Int32(v1) + Int32(v2)) / 2); }
    inline Float average(Float v1, Float v2) { return (v1 + v2) * 0.5f; }

    inline Byte blendAlphas(Byte destA, Byte srcA)
    {
        return Byte(UInt32(srcA) + (UInt32(destA) * (255U - UInt32(srcA)) + 127U) / 255U);
    }

    inline UInt16 blendAlphas(UInt16 destA, UInt16 srcA)
    {
        return UInt16(UInt32(srcA) +
                      (UInt32(destA) * (65535U - UInt32(srcA)) + 32767U) / 65535U);
    }

    inline Float blendAlphas(Float destA, Float srcA)
    {
        return srcA + destA * (1.0f - srcA);
    }

    Float blendComponents(Float destC, Float destA, Float srcC, Float srcA, Float blendedA)
    {
        return blendedA == 0.0f ? 0.0f :
            (srcC * srcA + destC * destA * (1.0f - srcA)) / blendedA;
    }

    template<typename CT, typename UInt_t, UInt_t maxVal>
    inline CT blendIntComponents(CT destC, CT destA, CT srcC, CT srcA, CT blendedA)
    {
        return blendedA == 0 ? 0 :
            CT((UInt_t(srcC) * UInt_t(srcA) * maxVal +
                UInt_t(destC) * UInt_t(destA) * (maxVal - UInt_t(srcA)))
               / UInt_t(blendedA) / maxVal);
    }

    Byte blendComponents(Byte destC, Byte destA, Byte srcC, Byte srcA, Byte blendedA)
    {
        return blendIntComponents<Byte, UInt32, 255U>(destC, destA, srcC, srcA, blendedA);
    }

    inline UInt16 blendComponents(UInt16 destC, UInt16 destA, UInt16 srcC, UInt16 srcA,
                                  UInt16 blendedA)
    {
#if WPNGIMAGE_RESTRICT_TO_CPP98
        if(sizeof(StdSize_t) >= 8)
            return blendIntComponents<UInt16, StdSize_t, 65535U>
                (destC, destA, srcC, srcA, blendedA);
        else
            return UInt16(blendComponents(Float(destC) / 65535.0f, Float(destA) / 65535.0f,
                                          Float(srcC) / 65535.0f, Float(srcA) / 65535.0f,
                                          Float(blendedA) / 65535.0f) * 65535.0f);
#else
        return blendIntComponents<UInt16, std::uint_fast64_t, 65535U>
            (destC, destA, srcC, srcA, blendedA);
#endif
    }


    //------------------------------------------------------------------------
    // Averaging pixels
    //------------------------------------------------------------------------
    template<typename CT, typename Calc_t> struct ComponentsOp;

    template<typename Calc_t>
    struct ComponentsOp<Byte, Calc_t>
    {
        static Byte divide(Calc_t v1, Calc_t v2) { return Byte((v1 + v2/2) / v2); }
    };

    template<typename Calc_t>
    struct ComponentsOp<UInt16, Calc_t>
    {
        static UInt16 divide(Calc_t v1, Calc_t v2) { return UInt16((v1 + v2/2) / v2); }
    };

    template<>
    struct ComponentsOp<Float, Float>
    {
        static Float divide(Float v1, Float v2) { return v1/v2; }
    };

    template<typename Pixel_t, typename Calc_t>
    inline Pixel_t calculateAverage(const Pixel_t& first, const Pixel_t* rest, std::size_t amount)
    {
        const Calc_t firstA = static_cast<Calc_t>(first.a);
        Calc_t sums[4] = { static_cast<Calc_t>(first.r) * firstA,
                           static_cast<Calc_t>(first.g) * firstA,
                           static_cast<Calc_t>(first.b) * firstA, firstA };

        for(std::size_t i = 0; i < amount; ++i)
        {
            const Calc_t a = static_cast<Calc_t>(rest[i].a);
            sums[0] += static_cast<Calc_t>(rest[i].r) * a;
            sums[1] += static_cast<Calc_t>(rest[i].g) * a;
            sums[2] += static_cast<Calc_t>(rest[i].b) * a;
            sums[3] += a;
        }

        if(sums[3] == 0) return Pixel_t(0, 0, 0, 0);

        typedef typename Pixel_t::Component_t CT;
        return Pixel_t(ComponentsOp<CT, Calc_t>::divide(sums[0], sums[3]),
                       ComponentsOp<CT, Calc_t>::divide(sums[1], sums[3]),
                       ComponentsOp<CT, Calc_t>::divide(sums[2], sums[3]),
                       ComponentsOp<CT, Calc_t>::divide(sums[3], Calc_t(amount + 1)));
    }

    WPngImage::Pixel8 averagePixels
    (const WPngImage::Pixel8& first, const WPngImage::Pixel8* rest, std::size_t amount)
    {
        return calculateAverage<WPngImage::Pixel8, UInt32>(first, rest, amount);
    }

    WPngImage::Pixel16 averagePixels
    (const WPngImage::Pixel16& first, const WPngImage::Pixel16* rest, std::size_t amount)
    {
#if WPNGIMAGE_RESTRICT_TO_CPP98
        if(sizeof(StdSize_t) >= 8)
            return calculateAverage<WPngImage::Pixel16, StdSize_t>(first, rest, amount);
        else
            return calculateAverage<WPngImage::Pixel16, Float>(first, rest, amount);
#else
        return calculateAverage<WPngImage::Pixel16, std::uint_fast64_t>(first, rest, amount);
#endif
    }

    WPngImage::PixelF averagePixels
    (const WPngImage::PixelF& first, const WPngImage::PixelF* rest, std::size_t amount)
    {
        return calculateAverage<WPngImage::PixelF, Float>(first, rest, amount);
    }
}


//============================================================================
// WPngImage::Pixel operator#(int) implementations
//============================================================================
template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t& WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator+=(OperatorParam_t value)
{
    r = addValues(r, value);
    g = addValues(g, value);
    b = addValues(b, value);
    return static_cast<Pixel_t&>(*this);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator+(OperatorParam_t value) const
{
    return Pixel_t(addValues(r, value), addValues(g, value), addValues(b, value), a);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t& WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator-=(OperatorParam_t value)
{
    return operator+=(-value);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator-(OperatorParam_t value) const
{
    return operator+(-value);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t& WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator*=(OperatorParam_t value)
{
    r = mulValues(r, value);
    g = mulValues(g, value);
    b = mulValues(b, value);
    return static_cast<Pixel_t&>(*this);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator*(OperatorParam_t value) const
{
    return Pixel_t(mulValues(r, value), mulValues(g, value), mulValues(b, value), a);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t& WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator/=(OperatorParam_t value)
{
    r = divValues(r, value);
    g = divValues(g, value);
    b = divValues(b, value);
    return static_cast<Pixel_t&>(*this);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator/(OperatorParam_t value) const
{
    return Pixel_t(divValues(r, value), divValues(g, value), divValues(b, value), a);
}


//============================================================================
// WPngImage::Pixel operator#(Pixel_t) implementations
//============================================================================
template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t& WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator+=(const Pixel_t& rhs)
{
    r = addComponents(r, rhs.r);
    g = addComponents(g, rhs.g);
    b = addComponents(b, rhs.b);
    a = average(a, rhs.a);
    return static_cast<Pixel_t&>(*this);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator+(const Pixel_t& rhs) const
{
    return Pixel_t(addComponents(r, rhs.r), addComponents(g, rhs.g), addComponents(b, rhs.b),
                   average(a, rhs.a));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t& WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator-=(const Pixel_t& rhs)
{
    r = subComponents(r, rhs.r);
    g = subComponents(g, rhs.g);
    b = subComponents(b, rhs.b);
    a = average(a, rhs.a);
    return static_cast<Pixel_t&>(*this);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator-(const Pixel_t& rhs) const
{
    return Pixel_t(subComponents(r, rhs.r), subComponents(g, rhs.g), subComponents(b, rhs.b),
                   average(a, rhs.a));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t& WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator*=(const Pixel_t& rhs)
{
    r = mulComponents(r, rhs.r);
    g = mulComponents(g, rhs.g);
    b = mulComponents(b, rhs.b);
    a = average(a, rhs.a);
    return static_cast<Pixel_t&>(*this);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator*(const Pixel_t& rhs) const
{
    return Pixel_t(mulComponents(r, rhs.r), mulComponents(g, rhs.g), mulComponents(b, rhs.b),
                   average(a, rhs.a));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t& WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator/=(const Pixel_t& rhs)
{
    r = divComponents(r, rhs.r);
    g = divComponents(g, rhs.g);
    b = divComponents(b, rhs.b);
    a = average(a, rhs.a);
    return static_cast<Pixel_t&>(*this);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::operator/(const Pixel_t& rhs) const
{
    return Pixel_t(divComponents(r, rhs.r), divComponents(g, rhs.g), divComponents(b, rhs.b),
                   average(a, rhs.a));
}


//============================================================================
// Other WPngImage::Pixel method implementations
//============================================================================
template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::blendWith(const Pixel_t& src)
{
    const CT blendedA = blendAlphas(a, src.a);
    r = blendComponents(r, a, src.r, src.a, blendedA);
    g = blendComponents(g, a, src.g, src.a, blendedA);
    b = blendComponents(b, a, src.b, src.a, blendedA);
    a = blendedA;
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::blendedPixel(const Pixel_t& src) const
{
    const CT blendedA = blendAlphas(a, src.a);
    return Pixel_t(blendComponents(r, a, src.r, src.a, blendedA),
                   blendComponents(g, a, src.g, src.a, blendedA),
                   blendComponents(b, a, src.b, src.a, blendedA), blendedA);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::averageWith(const Pixel_t& p)
{
    *this = averagedPixel(&p, 1);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::averagedPixel(const Pixel_t& p) const
{
    return averagedPixel(&p, 1);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::averageWith
(const Pixel_t* pixels, std::size_t amount)
{
    *this = averagedPixel(pixels, amount);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::averagedPixel
(const Pixel_t* pixels, std::size_t amount) const
{
    return averagePixels(static_cast<const Pixel_t&>(*this), pixels, amount);
}


//============================================================================
// Color conversion functions
//============================================================================
namespace
{
    // http://www.easyrgb.com/index.php?X=MATH
    template<bool calculateL>
    WPngImage::HSV convertToHSV(WPngImage::PixelF rgba)
    {
        WPngImage::HSV hsv;
        const Float varMin = std::min(std::min(rgba.r, rgba.g), rgba.b);
        const Float varMax = std::max(std::max(rgba.r, rgba.g), rgba.b);
        const Float dMax = varMax - varMin;

        hsv.v = calculateL ? (varMax + varMin) * 0.5f : varMax;

        if(dMax == 0)
        {
            hsv.h = 0;
            hsv.s = 0;
        }
        else
        {
            hsv.s = (calculateL ?
                     (dMax / (hsv.v < 0.5f ? varMax + varMin : 2.0f - varMax - varMin)) :
                     dMax / varMax);
            const Float dR = (((varMax - rgba.r) / 6.0f) + (dMax * 0.5f)) / dMax;
            const Float dG = (((varMax - rgba.g) / 6.0f) + (dMax * 0.5f)) / dMax;
            const Float dB = (((varMax - rgba.b) / 6.0f) + (dMax * 0.5f)) / dMax;
            if(rgba.r == varMax) hsv.h = dB - dG;
            else if(rgba.g == varMax) hsv.h = (1.0f / 3.0f) + dR - dB;
            else if(rgba.b == varMax) hsv.h = (2.0f / 3.0f) + dG - dR;
            hsv.h = std::fmod(hsv.h, 1.0f);
            if(hsv.h < 0.0f) hsv.h += 1.0f;
        }

        hsv.a = rgba.a;
        return hsv;
    }

    WPngImage::PixelF convertFromHSV(Float h, Float s, Float v, Float a)
    {
        if(s == 0)
            return WPngImage::PixelF(v, v, v, a);

        h = std::fmod(h, 1.0f);
        if(h < 0.0f) h += 1.0f;

        Float varH = h * 6.0f;
        if(varH == 6.0f) varH = 0.0f;
        const int varI = int(varH);
        const Float var1 = v * (1.0f - s);
        const Float var2 = v * (1.0f - s * (varH - Float(varI)));
        const Float var3 = v * (1.0f - s * (1.0f - (varH - Float(varI))));

        WPngImage::PixelF rgba;
        switch(varI)
        {
          case 0: rgba.r = v; rgba.g = var3; rgba.b = var1; break;
          case 1: rgba.r = var2; rgba.g = v; rgba.b = var1; break;
          case 2: rgba.r = var1; rgba.g = v; rgba.b = var3; break;
          case 3: rgba.r = var1; rgba.g = var2; rgba.b = v; break;
          case 4: rgba.r = var3; rgba.g = var1; rgba.b = v; break;
          default: rgba.r = v; rgba.g = var1; rgba.b = var2; break;
        }

        rgba.a = a;
        return rgba;
    }

    Float hueToRGB(Float v1, Float v2, float vH)
    {
        if(vH < 0.0f) vH += 1.0f;
        else if(vH > 1.0f) vH -= 1.0f;
        if(6.0f * vH < 1.0f) return v1 + (v2 - v1) * 6.0f * vH;
        if(2.0f * vH < 1.0f) return v2;
        if(3.0f * vH < 2.0f) return v1 + (v2 - v1) * ((2.0f / 3.0f) - vH) * 6.0f;
        return v1;
    }

    WPngImage::PixelF convertFromHSL(Float h, Float s, Float l, Float a)
    {
        if(s == 0)
            return WPngImage::PixelF(l, l, l, a);

        h = std::fmod(h, 1.0f);
        if(h < 0.0f) h += 1.0f;

        const Float var2 = (l < 0.5f ?
                            l * (1.0f + s) :
                            l + s - s * l);
        const Float var1 = 2.0f * l - var2;

        return WPngImage::PixelF(hueToRGB(var1, var2, h + (1.0f / 3.0f)),
                                 hueToRGB(var1, var2, h),
                                 hueToRGB(var1, var2, h - (1.0f / 3.0f)), a);
    }

    inline Float rgbaToXyzComponent(Float component)
    {
        return (component > 0.04045f ?
                std::pow((component + 0.055f) / 1.055f, 2.4f) :
                component / 12.92f);
    }

    WPngImage::XYZ convertToXYZ(WPngImage::PixelF rgba)
    {
        const Float r = rgbaToXyzComponent(rgba.r) * 100.0f;
        const Float g = rgbaToXyzComponent(rgba.g) * 100.0f;
        const Float b = rgbaToXyzComponent(rgba.b) * 100.0f;
        const WPngImage::XYZ xyz =
        {
            r * 0.4124f + g * 0.3576f + b * 0.1805f,
            r * 0.2126f + g * 0.7152f + b * 0.0722f,
            r * 0.0193f + g * 0.1192f + b * 0.9505f,
            rgba.a
        };
        return xyz;
    }

    inline Float xyzToRgbaComponent(Float component)
    {
        return (component > 0.0031308f ?
                1.055f * std::pow(component, 1.0f / 2.4f) - 0.055f :
                12.92f * component);
    }

    WPngImage::PixelF convertFromXYZ(Float x, Float y, Float z, Float a)
    {
        x /= 100.0f;
        y /= 100.0f;
        z /= 100.0f;
        return WPngImage::PixelF
            (xyzToRgbaComponent(x *  3.2406f + y * -1.5372f + z * -0.4986f),
             xyzToRgbaComponent(x * -0.9689f + y *  1.8758f + z *  0.0415f),
             xyzToRgbaComponent(x *  0.0557f + y * -0.2040f + z *  1.0570f), a);
    }

    WPngImage::YXY convertToYXY(WPngImage::PixelF rgba)
    {
        const WPngImage::XYZ xyz = convertToXYZ(rgba);
        WPngImage::YXY yxy;
        const Float xyzSum = xyz.x + xyz.y + xyz.z;
        if(xyzSum == 0.0)
            yxy.Y = yxy.x = yxy.y = 0;
        else
        {
            yxy.Y = xyz.y;
            yxy.x = xyz.x / xyzSum;
            yxy.y = xyz.y / xyzSum;
        };
        yxy.a = rgba.a;
        return yxy;
    }

    WPngImage::PixelF convertFromYXY(Float Y, Float x, Float y, Float a)
    {
        if(y == 0) return WPngImage::PixelF(0, 0, 0, a);
        const Float factor = Y / y;
        return convertFromXYZ(x * factor, Y, (1.0 - x - y) * factor, a);
    }

    WPngImage::CMY convertToCMY(WPngImage::PixelF rgba)
    {
        const WPngImage::CMY cmy = { 1.0f - rgba.r, 1.0f - rgba.g, 1.0f - rgba.b, rgba.a };
        return cmy;
    }

    WPngImage::PixelF convertFromCMY(Float c, Float m, Float y, Float a)
    {
        return WPngImage::PixelF(1.0f - c, 1.0f - m, 1.0f - y, a);
    }

    WPngImage::CMYK convertToCMYK(WPngImage::PixelF rgba)
    {
        WPngImage::CMYK cmyk = { 1.0f - rgba.r, 1.0f - rgba.g, 1.0f - rgba.b, 1.0f, rgba.a };

        if(cmyk.c < cmyk.k) cmyk.k = cmyk.c;
        if(cmyk.m < cmyk.k) cmyk.k = cmyk.m;
        if(cmyk.y < cmyk.k) cmyk.k = cmyk.y;
        if(cmyk.k == 1.0f)
        {
            cmyk.c = cmyk.m = cmyk.y = 0.0f;
        }
        else
        {
            const Float invK = 1.0f - cmyk.k;
            cmyk.c = (cmyk.c - cmyk.k) / invK;
            cmyk.m = (cmyk.m - cmyk.k) / invK;
            cmyk.y = (cmyk.y - cmyk.k) / invK;
        }

        return cmyk;
    }

    WPngImage::PixelF convertFromCMYK(Float c, Float m, Float y, Float k, Float a)
    {
        const Float invK = 1.0f - k;
        return WPngImage::PixelF(1.0f - (c * invK + k),
                                 1.0f - (m * invK + k),
                                 1.0f - (y * invK + k), a);
    }
}


//============================================================================
// WPngImage::Pixel color conversion function implementations
//============================================================================
template<typename Pixel_t, typename CT, typename OperatorParam_t>
WPngImage::HSV WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::toHSV() const
{
    return convertToHSV<false>(PixelF(static_cast<const Pixel_t&>(*this)));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::set(const HSV& hsv)
{
    *this = Pixel_t(convertFromHSV(hsv.h, hsv.s, hsv.v, hsv.a));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::setFromHSV
(Float hValue, Float sValue, Float vValue, Float aValue)
{
    *this = Pixel_t(convertFromHSV(hValue, sValue, vValue, aValue));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::setFromHSV
(Float hValue, Float sValue, Float vValue)
{
    setFromHSV(hValue, sValue, vValue, a);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
WPngImage::HSL WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::toHSL() const
{
    const WPngImage::HSV hsv =
        convertToHSV<true>(PixelF(static_cast<const Pixel_t&>(*this)));
    const WPngImage::HSL hsl = { hsv.h, hsv.s, hsv.v, hsv.a };
    return hsl;
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::set(const HSL& hsl)
{
    *this = Pixel_t(convertFromHSL(hsl.h, hsl.s, hsl.l, hsl.a));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::setFromHSL
(Float hValue, Float sValue, Float lValue, Float aValue)
{
    *this = Pixel_t(convertFromHSL(hValue, sValue, lValue, aValue));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::setFromHSL
(Float hValue, Float sValue, Float lValue)
{
    setFromHSL(hValue, sValue, lValue, a);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
WPngImage::XYZ WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::toXYZ() const
{
    return convertToXYZ(PixelF(static_cast<const Pixel_t&>(*this)));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::set(const XYZ& xyz)
{
    *this = Pixel_t(convertFromXYZ(xyz.x, xyz.y, xyz.z, xyz.a));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::setFromXYZ
(Float xValue, Float yValue, Float zValue, Float aValue)
{
    *this = Pixel_t(convertFromXYZ(xValue, yValue, zValue, aValue));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::setFromXYZ
(Float xValue, Float yValue, Float zValue)
{
    setFromXYZ(xValue, yValue, zValue, a);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
WPngImage::YXY WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::toYXY() const
{
    return convertToYXY(PixelF(static_cast<const Pixel_t&>(*this)));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::set(const YXY& yxy)
{
    *this = Pixel_t(convertFromYXY(yxy.Y, yxy.x, yxy.y, yxy.a));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::setFromYXY
(Float YValue, Float xValue, Float yValue, Float aValue)
{
    *this = Pixel_t(convertFromYXY(YValue, xValue, yValue, aValue));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::setFromYXY
(Float YValue, Float xValue, Float yValue)
{
    setFromYXY(YValue, xValue, yValue, a);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
WPngImage::CMY WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::toCMY() const
{
    return convertToCMY(PixelF(static_cast<const Pixel_t&>(*this)));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::set(const CMY& cmy)
{
    *this = Pixel_t(convertFromCMY(cmy.c, cmy.m, cmy.y, cmy.a));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::setFromCMY
(Float cValue, Float mValue, Float yValue, Float aValue)
{
    *this = Pixel_t(convertFromCMY(cValue, mValue, yValue, aValue));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::setFromCMY
(Float cValue, Float mValue, Float yValue)
{
    setFromCMY(cValue, mValue, yValue, a);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
WPngImage::CMYK WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::toCMYK() const
{
    return convertToCMYK(PixelF(static_cast<const Pixel_t&>(*this)));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::set(const CMYK& cmyk)
{
    *this = Pixel_t(convertFromCMYK(cmyk.c, cmyk.m, cmyk.y, cmyk.k, cmyk.a));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::setFromCMYK
(Float cValue, Float mValue, Float yValue, Float kValue, Float aValue)
{
    *this = Pixel_t(convertFromCMYK(cValue, mValue, yValue, kValue, aValue));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
void WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::setFromCMYK
(Float cValue, Float mValue, Float yValue, Float kValue)
{
    setFromCMYK(cValue, mValue, yValue, kValue, a);
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
CT WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::toGrayCIE() const
{
    const PixelF p(static_cast<const Pixel_t&>(*this));
    const Float rLinear = rgbaToXyzComponent(p.r);
    const Float gLinear = rgbaToXyzComponent(p.g);
    const Float bLinear = rgbaToXyzComponent(p.b);
    const Float yLinear = rLinear * 0.2126f + gLinear * 0.7152f + bLinear * 0.0722f;
    return floatToCT<CT>(xyzToRgbaComponent(yLinear));
}

template<typename Pixel_t, typename CT, typename OperatorParam_t>
Pixel_t WPngImage::Pixel<Pixel_t, CT, OperatorParam_t>::grayCIEPixel() const
{
    const CT gray = toGrayCIE();
    return Pixel_t(gray, gray, gray, a);
}


//============================================================================
// WPngImage::IPixel implementations
//============================================================================
template<typename Pixel_t, typename CT>
CT WPngImage::IPixel<Pixel_t, CT>::toGray(Int32 rWeight, Int32 gWeight, Int32 bWeight) const
{
    const Int32 sum = rWeight + gWeight + bWeight;
    return sum == 0 ? 0 :
        CT((Int32(Pixel<Pixel_t, CT, Int32>::r) * rWeight +
            Int32(Pixel<Pixel_t, CT, Int32>::g) * gWeight +
            Int32(Pixel<Pixel_t, CT, Int32>::b) * bWeight + sum/2) / sum);
}

template<typename Pixel_t, typename CT>
Pixel_t WPngImage::IPixel<Pixel_t, CT>::grayPixel(Int32 rWeight, Int32 gWeight, Int32 bWeight) const
{
    const CT gray = toGray(rWeight, gWeight, bWeight);
    return Pixel_t(gray, gray, gray, Pixel<Pixel_t, CT, Int32>::a);
}


//============================================================================
// WPngImage::Pixel8 implementations
//============================================================================
WPngImage::Pixel8::Pixel8(Pixel16 p16):
    IPixel(UInt16ToByte(p16.r), UInt16ToByte(p16.g), UInt16ToByte(p16.b), UInt16ToByte(p16.a))
{}

WPngImage::Pixel8::Pixel8(PixelF pF):
    IPixel(floatToCT<Byte>(pF.r), floatToCT<Byte>(pF.g), floatToCT<Byte>(pF.b),
           floatToCT<Byte>(pF.a))
{}

WPngImage::Pixel8 WPngImage::Pixel8::interpolatedPixel(const Pixel8& p2, Byte factor) const
{
    const UInt32 invFactor = 255 - factor;

    if(a == p2.a)
        return Pixel8((r * invFactor + p2.r * factor) / 255,
                      (g * invFactor + p2.g * factor) / 255,
                      (b * invFactor + p2.b * factor) / 255, a);

    const UInt32 factor1 = a * invFactor, factor2 = p2.a * factor;
    const UInt32 sumOfFactors = factor1 + factor2;
    if(sumOfFactors == 0) return Pixel8(0, 0, 0, 0);
    return Pixel8((r * factor1 + p2.r * factor2) / sumOfFactors,
                  (g * factor1 + p2.g * factor2) / sumOfFactors,
                  (b * factor1 + p2.b * factor2) / sumOfFactors,
                  sumOfFactors / 255);
}

void WPngImage::Pixel8::interpolate(const Pixel8& p2, Byte factor)
{
    *this = interpolatedPixel(p2, factor);
}

WPngImage::Pixel8 WPngImage::Pixel8::rawInterpolatedPixel(const Pixel8& p2, Byte factor) const
{
    const UInt32 invFactor = 255 - factor;
    return Pixel8((r * invFactor + p2.r * factor) / 255,
                  (g * invFactor + p2.g * factor) / 255,
                  (b * invFactor + p2.b * factor) / 255,
                  (a * invFactor + p2.a * factor) / 255);
}

void WPngImage::Pixel8::rawInterpolate(const Pixel8& p2, Byte factor)
{
    *this = rawInterpolatedPixel(p2, factor);
}

void WPngImage::Pixel8::premultiplyAlpha()
{
    r = componentMultipliedByAlpha(r, a);
    g = componentMultipliedByAlpha(g, a);
    b = componentMultipliedByAlpha(b, a);
}

WPngImage::Pixel8 WPngImage::Pixel8::premultipliedAlphaPixel() const
{
    return WPngImage::Pixel8
        (componentMultipliedByAlpha(r, a),
         componentMultipliedByAlpha(g, a),
         componentMultipliedByAlpha(b, a), a);
}

WPngImage::Pixel8 operator-(WPngImage::Int32 value, const WPngImage::Pixel8& p)
{
    return WPngImage::Pixel8
        (subValues(value, p.r), subValues(value, p.g), subValues(value, p.b), p.a);
}

WPngImage::Pixel8 operator/(WPngImage::Int32 value, const WPngImage::Pixel8& p)
{
    return WPngImage::Pixel8
        (divValues(value, p.r), divValues(value, p.g), divValues(value, p.b), p.a);
}


//============================================================================
// WPngImage::Pixel16 implementations
//============================================================================
WPngImage::Pixel16::Pixel16(Pixel8 p8):
    IPixel(ByteToUInt16(p8.r), ByteToUInt16(p8.g), ByteToUInt16(p8.b), ByteToUInt16(p8.a))
{}

WPngImage::Pixel16::Pixel16(PixelF pF):
    IPixel(floatToCT<UInt16>(pF.r), floatToCT<UInt16>(pF.g), floatToCT<UInt16>(pF.b),
           floatToCT<UInt16>(pF.a))
{}

WPngImage::Pixel16 WPngImage::Pixel16::interpolatedPixel(const Pixel16& p2, UInt16 factor) const
{
#if WPNGIMAGE_RESTRICT_TO_CPP98
    if(sizeof(StdSize_t) < 8)
        return Pixel16(PixelF(*this).interpolatedPixel(PixelF(p2), Float(factor / 65535.0f)));

    typedef StdSize_t UInt64;
#else
    using UInt64 = std::uint_fast64_t;
#endif

    const UInt64 invFactor = 65535 - factor;

    if(a == p2.a)
        return Pixel16((r * invFactor + p2.r * factor) / 65535,
                       (g * invFactor + p2.g * factor) / 65535,
                       (b * invFactor + p2.b * factor) / 65535, a);

    const UInt64 factor1 = a * invFactor, factor2 = p2.a * factor;
    const UInt64 sumOfFactors = factor1 + factor2;
    if(sumOfFactors == 0) return Pixel16(0, 0, 0, 0);
    return Pixel16((r * factor1 + p2.r * factor2) / sumOfFactors,
                   (g * factor1 + p2.g * factor2) / sumOfFactors,
                   (b * factor1 + p2.b * factor2) / sumOfFactors,
                   sumOfFactors / 65535);
}

void WPngImage::Pixel16::interpolate(const Pixel16& p2, UInt16 factor)
{
    *this = interpolatedPixel(p2, factor);
}

WPngImage::Pixel16 WPngImage::Pixel16::rawInterpolatedPixel(const Pixel16& p2, UInt16 factor) const
{
#if WPNGIMAGE_RESTRICT_TO_CPP98
    if(sizeof(StdSize_t) < 8)
        return Pixel16(PixelF(*this).rawInterpolatedPixel(PixelF(p2), Float(factor / 65535.0f)));

    typedef StdSize_t UInt64;
#else
    using UInt64 = std::uint_fast64_t;
#endif

    const UInt64 invFactor = 65535 - factor;
    return Pixel16((r * invFactor + p2.r * factor) / 65535,
                   (g * invFactor + p2.g * factor) / 65535,
                   (b * invFactor + p2.b * factor) / 65535,
                   (a * invFactor + p2.a * factor) / 65535);
}

void WPngImage::Pixel16::rawInterpolate(const Pixel16& p2, UInt16 factor)
{
    *this = rawInterpolatedPixel(p2, factor);
}

void WPngImage::Pixel16::premultiplyAlpha()
{
    r = componentMultipliedByAlpha(r, a);
    g = componentMultipliedByAlpha(g, a);
    b = componentMultipliedByAlpha(b, a);
}

WPngImage::Pixel16 WPngImage::Pixel16::premultipliedAlphaPixel() const
{
    return WPngImage::Pixel16
        (componentMultipliedByAlpha(r, a),
         componentMultipliedByAlpha(g, a),
         componentMultipliedByAlpha(b, a), a);
}

WPngImage::Pixel16 operator-(WPngImage::Int32 value, const WPngImage::Pixel16& p)
{
    return WPngImage::Pixel16
        (subValues(value, p.r), subValues(value, p.g), subValues(value, p.b), p.a);
}

WPngImage::Pixel16 operator/(WPngImage::Int32 value, const WPngImage::Pixel16& p)
{
    return WPngImage::Pixel16
        (divValues(value, p.r), divValues(value, p.g), divValues(value, p.b), p.a);
}


//============================================================================
// WPngImage::PixelF implementations
//============================================================================
WPngImage::PixelF::PixelF(Pixel8 p8):
    Pixel(ByteToFloat(p8.r), ByteToFloat(p8.g), ByteToFloat(p8.b), ByteToFloat(p8.a))
{}

WPngImage::PixelF::PixelF(Pixel16 p16):
    Pixel(UInt16ToFloat(p16.r), UInt16ToFloat(p16.g), UInt16ToFloat(p16.b), UInt16ToFloat(p16.a))
{}

Float WPngImage::PixelF::toGray(Float rWeight, Float gWeight, Float bWeight) const
{
    const Float sum = rWeight + gWeight + bWeight;
    return sum == 0 ? 0.0f : (r * rWeight + g * gWeight + b * bWeight) / sum;
}

WPngImage::PixelF WPngImage::PixelF::grayPixel(Float rWeight, Float gWeight, Float bWeight) const
{
    const Float gray = toGray(rWeight, gWeight, bWeight);
    return PixelF(gray, gray, gray, a);
}

WPngImage::PixelF WPngImage::PixelF::interpolatedPixel(const PixelF& p2, Float factor) const
{
    const Float invFactor = 1.0f - factor;

    if(a == p2.a)
        return PixelF(r * invFactor + p2.r * factor,
                      g * invFactor + p2.g * factor,
                      b * invFactor + p2.b * factor, a);

    const Float factor1 = a * invFactor, factor2 = p2.a * factor;
    const Float sumOfFactors = factor1 + factor2;
    if(sumOfFactors == 0.0f) return PixelF(0, 0, 0, 0);
    const Float invSumOfFactors = 1.0f / sumOfFactors;
    return PixelF((r * factor1 + p2.r * factor2) * invSumOfFactors,
                  (g * factor1 + p2.g * factor2) * invSumOfFactors,
                  (b * factor1 + p2.b * factor2) * invSumOfFactors,
                  sumOfFactors);
}

void WPngImage::PixelF::interpolate(const PixelF& p, Float factor)
{
    *this = interpolatedPixel(p, factor);
}

WPngImage::PixelF WPngImage::PixelF::rawInterpolatedPixel(const PixelF& p2, Float factor) const
{
    const Float invFactor = 1.0f - factor;
    return PixelF(r * invFactor + p2.r * factor,
                  g * invFactor + p2.g * factor,
                  b * invFactor + p2.b * factor,
                  a * invFactor + p2.a * factor);
}

void WPngImage::PixelF::rawInterpolate(const PixelF& p, Float factor)
{
    *this = rawInterpolatedPixel(p, factor);
}

void WPngImage::PixelF::premultiplyAlpha()
{
    r *= a;
    g *= a;
    b *= a;
}

WPngImage::PixelF WPngImage::PixelF::premultipliedAlphaPixel() const
{
    return WPngImage::PixelF(r * a, g * a, b * a, a);
}

void WPngImage::PixelF::clamp()
{
    if(r > 1.0f) r = 1.0f;
    else if(r < 0.0f) r = 0.0f;
    if(g > 1.0f) g = 1.0f;
    else if(g < 0.0f) g = 0.0f;
    if(b > 1.0f) b = 1.0f;
    else if(b < 0.0f) b = 0.0f;
    if(a > 1.0f) a = 1.0f;
    else if(a < 0.0f) a = 0.0f;
}

WPngImage::PixelF WPngImage::PixelF::clampedPixel() const
{
    WPngImage::PixelF pixel = *this;
    pixel.clamp();
    return pixel;
}

WPngImage::PixelF operator-(WPngImage::Float value, const WPngImage::PixelF& p)
{
    return WPngImage::PixelF(value - p.r, value - p.g, value - p.b, p.a);
}

WPngImage::PixelF operator/(WPngImage::Float value, const WPngImage::PixelF& p)
{
    return WPngImage::PixelF(value / p.r, value / p.g, value / p.b, p.a);
}


//============================================================================
// WPngImage::Pixel explicit instantiations
//============================================================================
template struct WPngImage::Pixel<WPngImage::Pixel8, Byte, Int32>;
template struct WPngImage::Pixel<WPngImage::Pixel16, UInt16, Int32>;
template struct WPngImage::Pixel<WPngImage::PixelF, Float, WPngImage::Float>;
template struct WPngImage::IPixel<WPngImage::Pixel8, Byte>;
template struct WPngImage::IPixel<WPngImage::Pixel16, UInt16>;



//============================================================================
// Structs for handling grayscale pixels
//============================================================================
namespace
{
    template<typename CT>
    struct PixelG
    {
        typedef CT Component_t;
        CT g, a;

        template<typename OtherCT>
        PixelG(const PixelG<OtherCT>& other):
            g(convertType<CT>(other.g)),
            a(convertType<CT>(other.a)) {}

        template<typename Pixel_t>
        PixelG(const Pixel_t& pixel):
            g(convertType<CT>(WPngImage::PixelF(pixel).toGrayCIE())),
            a(convertType<CT>(pixel.a)) {}

        PixelG(const WPngImage::PixelF& pixel):
            g(convertType<CT>(pixel.toGrayCIE())),
            a(convertType<CT>(pixel.a)) {}

        template<typename Pixel_t>
        Pixel_t toPixel() const
        {
            typedef typename Pixel_t::Component_t ToCT;
            const ToCT c = convertType<ToCT>(g);
            return Pixel_t(c, c, c, convertType<ToCT>(a));
        }

        void blendWith(const PixelG& src)
        {
            const CT blendedA = blendAlphas(a, src.a);
            g = blendComponents(g, a, src.g, src.a, blendedA);
            a = blendedA;
        }

        void premultiplyAlpha()
        {
            g = componentMultipliedByAlpha(g, a);
        }

        PixelG premultipliedAlphaPixel() const
        {
            return PixelG(componentMultipliedByAlpha(g, a), a);
        }
    };

    typedef PixelG<Byte> PixelG8;
    typedef PixelG<UInt16> PixelG16;
    typedef PixelG<Float> PixelGF;
}


//============================================================================
// Generic function for converting between pixel formats
//============================================================================
namespace
{
    template<typename DestPixel_t, typename CT>
    DestPixel_t convertToPixel(const PixelG<CT>& src)
    {
        return src.template toPixel<DestPixel_t>();
    }

    template<typename DestPixel_t, typename SrcPixel_t>
    DestPixel_t convertToPixel(const SrcPixel_t& src)
    {
        return DestPixel_t(src);
    }

    template<> WPngImage::Pixel8 convertToPixel<WPngImage::Pixel8, WPngImage::Pixel8>
    (const WPngImage::Pixel8& src) { return src; }

    template<> WPngImage::Pixel16 convertToPixel<WPngImage::Pixel16, WPngImage::Pixel16>
    (const WPngImage::Pixel16& src) { return src; }

    template<> WPngImage::PixelF convertToPixel<WPngImage::PixelF, WPngImage::PixelF>
    (const WPngImage::PixelF& src) { return src; }

    template<typename DestCT, typename SrcPixel_t>
    PixelG<DestCT> convertToPixelG(const SrcPixel_t& src)
    { return PixelG<DestCT>(src); }

    template<> PixelG8 convertToPixelG<Byte, PixelG8>(const PixelG8& src) { return src; }
    template<> PixelG16 convertToPixelG<UInt16, PixelG16>(const PixelG16& src) { return src; }

    template<typename DestPixel_t, typename SrcPixel_t>
    void assignPixel(DestPixel_t& dest, const SrcPixel_t& src)
    {
        dest = DestPixel_t(src);
    }

    template<typename DestPixel_t, typename CT>
    void assignPixel(DestPixel_t& dest, const PixelG<CT>& src)
    {
        dest = src.template toPixel<DestPixel_t>();
    }

    template<typename DestCT, typename SrcCT>
    void assignPixel(PixelG<DestCT>& dest, const PixelG<SrcCT>& src)
    {
        dest = PixelG<DestCT>(src);
    }

    template<> void assignPixel(WPngImage::Pixel8& dest, const WPngImage::Pixel8& src)
    { dest = src; }
    template<> void assignPixel(WPngImage::Pixel16& dest, const WPngImage::Pixel16& src)
    { dest = src; }
    template<> void assignPixel(WPngImage::PixelF& dest, const WPngImage::PixelF& src)
    { dest = src; }
    template<> void assignPixel(PixelG8& dest, const PixelG8& src) { dest = src; }
    template<> void assignPixel(PixelG16& dest, const PixelG16& src) { dest = src; }
    template<> void assignPixel(PixelGF& dest, const PixelGF& src) { dest = src; }
}


//============================================================================
// WPngImage::PngData
//============================================================================
struct WPngImage::PngDataBase
{
    PixelFormat mPixelFormat;
    PngFileFormat mPngFileFormat;

    PngDataBase(PixelFormat);
    virtual ~PngDataBase() {}

    virtual bool assignAllDataFrom(const PngDataBase*) = 0;
    virtual PngDataBase* createCopy() const = 0;

    virtual Pixel8 getPixel8(std::size_t) const = 0;
    virtual Pixel16 getPixel16(std::size_t) const = 0;
    virtual PixelF getPixelF(std::size_t) const = 0;
    virtual PixelG8 getPixelG8(std::size_t) const = 0;
    virtual PixelG16 getPixelG16(std::size_t) const = 0;
    virtual bool allPixelsHaveFullAlpha() const = 0;
    virtual void setPixel(std::size_t, const Pixel8&) = 0;
    virtual void setPixel(std::size_t, const Pixel16&) = 0;
    virtual void setPixel(std::size_t, const PixelF&) = 0;
    virtual void setPixel(std::size_t, const PixelG8&) = 0;
    virtual void setPixel(std::size_t, const PixelG16&) = 0;
    virtual void setPixel(std::size_t, const PixelGF&) = 0;
    virtual void drawPixel(std::size_t, const Pixel8&) = 0;
    virtual void drawPixel(std::size_t, const Pixel16&) = 0;
    virtual void drawPixel(std::size_t, const PixelF&) = 0;
    virtual void fill(const Pixel8&) = 0;
    virtual void fill(const Pixel16&) = 0;
    virtual void fill(const PixelF&) = 0;
    virtual void transform(TransformFunc8) = 0;
    virtual void transform(TransformFunc16) = 0;
    virtual void transform(TransformFuncF) = 0;
    virtual void transform(TransformFunc8, WPngImage& dest) const = 0;
    virtual void transform(TransformFunc16, WPngImage& dest) const = 0;
    virtual void transform(TransformFuncF, WPngImage& dest) const = 0;
    virtual void copyPixelTo(std::size_t, PngDataBase*, std::size_t) const = 0;
    virtual void copyAllPixelsTo(PngDataBase*) const = 0;
    virtual void copyPixelLineTo
    (std::size_t, std::size_t, PngDataBase*, std::size_t, bool) const = 0;
    virtual void addLine(std::size_t, std::size_t, std::size_t, const Pixel8&, bool) = 0;
    virtual void addLine(std::size_t, std::size_t, std::size_t, const Pixel16&, bool) = 0;
    virtual void addLine(std::size_t, std::size_t, std::size_t, const PixelF&, bool) = 0;
    virtual void premultiplyAlpha() = 0;
    virtual void flipHorizontally(int, int) = 0;
    virtual void flipVertically(int, int) = 0;
    virtual void rotate180(int, int) = 0;
    virtual void rotate90cwSquare(int) = 0;
    virtual void rotate90cwNonsquare(int, int) = 0;
    virtual void rotate90ccwSquare(int) = 0;
    virtual void rotate90ccwNonsquare(int, int) = 0;
};

WPngImage::PngDataBase::PngDataBase(PixelFormat pixelFormat):
    mPixelFormat(pixelFormat),
    mPngFileFormat(kPngFileFormat_none)
{}

template<typename PixelData_t>
struct WPngImage::PngData: public PngDataBase
{
    std::vector<PixelData_t> mPixelData;

    template<typename Pixel_t>
    PngData(int, int, Pixel_t, PixelFormat);

    virtual bool assignAllDataFrom(const PngDataBase*);
    virtual PngDataBase* createCopy() const;

    virtual Pixel8 getPixel8(std::size_t) const;
    virtual Pixel16 getPixel16(std::size_t) const;
    virtual PixelF getPixelF(std::size_t) const;
    virtual PixelG8 getPixelG8(std::size_t) const;
    virtual PixelG16 getPixelG16(std::size_t) const;
    virtual bool allPixelsHaveFullAlpha() const;
    virtual void setPixel(std::size_t, const Pixel8&);
    virtual void setPixel(std::size_t, const Pixel16&);
    virtual void setPixel(std::size_t, const PixelF&);
    virtual void setPixel(std::size_t, const PixelG8&);
    virtual void setPixel(std::size_t, const PixelG16&);
    virtual void setPixel(std::size_t, const PixelGF&);
    virtual void drawPixel(std::size_t, const Pixel8&);
    virtual void drawPixel(std::size_t, const Pixel16&);
    virtual void drawPixel(std::size_t, const PixelF&);
    virtual void fill(const Pixel8&);
    virtual void fill(const Pixel16&);
    virtual void fill(const PixelF&);
    virtual void transform(TransformFunc8);
    virtual void transform(TransformFunc16);
    virtual void transform(TransformFuncF);
    virtual void transform(TransformFunc8, WPngImage& dest) const;
    virtual void transform(TransformFunc16, WPngImage& dest) const;
    virtual void transform(TransformFuncF, WPngImage& dest) const;
    virtual void copyPixelTo(std::size_t, PngDataBase*, std::size_t) const;
    virtual void copyAllPixelsTo(PngDataBase*) const;
    virtual void copyPixelLineTo
    (std::size_t, std::size_t, PngDataBase*, std::size_t, bool) const;
    virtual void addLine(std::size_t, std::size_t, std::size_t, const Pixel8&, bool);
    virtual void addLine(std::size_t, std::size_t, std::size_t, const Pixel16&, bool);
    virtual void addLine(std::size_t, std::size_t, std::size_t, const PixelF&, bool);
    void blendLine(std::size_t, std::size_t, std::size_t, const PixelData_t&);
    void assignLine(std::size_t, std::size_t, std::size_t, const PixelData_t&);
    virtual void premultiplyAlpha();
    virtual void flipHorizontally(int, int);
    virtual void flipVertically(int, int);
    virtual void rotate180(int, int);
    virtual void rotate90cwSquare(int);
    virtual void rotate90cwNonsquare(int, int);
    virtual void rotate90ccwSquare(int);
    virtual void rotate90ccwNonsquare(int, int);
};


//============================================================================
// WPngImage::PngData implementations
//============================================================================
// Constructor
//----------------------------------------------------------------------------
template<typename PixelData_t>
template<typename Pixel_t>
WPngImage::PngData<PixelData_t>::PngData
(int width, int height, Pixel_t pixel, PixelFormat pixelFormat):
    PngDataBase(pixelFormat),
    mPixelData(width * height, PixelData_t(pixel))
{}

template<typename PixelData_t>
bool WPngImage::PngData<PixelData_t>::assignAllDataFrom(const PngDataBase* src)
{
    const PngData<PixelData_t>* srcPngData = dynamic_cast<const PngData<PixelData_t>*>(src);
    if(!srcPngData) return false;
    mPixelFormat = srcPngData->mPixelFormat;
    mPngFileFormat = srcPngData->mPngFileFormat;
    mPixelData = srcPngData->mPixelData;
    return true;
}

template<typename PixelData_t>
WPngImage::PngDataBase* WPngImage::PngData<PixelData_t>::createCopy() const
{
    return new PngData<PixelData_t>(*this);
}

//----------------------------------------------------------------------------
// Get pixel
//----------------------------------------------------------------------------
template<typename PixelData_t>
WPngImage::Pixel8 WPngImage::PngData<PixelData_t>::getPixel8(std::size_t index) const
{
    return convertToPixel<Pixel8>(mPixelData[index]);
}

template<typename PixelData_t>
WPngImage::Pixel16 WPngImage::PngData<PixelData_t>::getPixel16(std::size_t index) const
{
    return convertToPixel<Pixel16>(mPixelData[index]);
}

template<typename PixelData_t>
PixelG8 WPngImage::PngData<PixelData_t>::getPixelG8(std::size_t index) const
{
    return convertToPixelG<Byte>(mPixelData[index]);
}

template<typename PixelData_t>
PixelG16 WPngImage::PngData<PixelData_t>::getPixelG16(std::size_t index) const
{
    return convertToPixelG<UInt16>(mPixelData[index]);
}

template<typename PixelData_t>
WPngImage::PixelF WPngImage::PngData<PixelData_t>::getPixelF(std::size_t index) const
{
    return convertToPixel<PixelF>(mPixelData[index]);
}

namespace
{
    bool pixelHasFullAlpha(const PixelG8& pixel) { return pixel.a == 255; }
    bool pixelHasFullAlpha(const PixelG16& pixel) { return pixel.a == 65535; }
    bool pixelHasFullAlpha(const PixelGF& pixel) { return pixel.a >= 1.0f; }
    bool pixelHasFullAlpha(const WPngImage::Pixel8& pixel) { return pixel.a == 255; }
    bool pixelHasFullAlpha(const WPngImage::Pixel16& pixel) { return pixel.a == 65535; }
    bool pixelHasFullAlpha(const WPngImage::PixelF& pixel) { return pixel.a >= 1.0f; }
}

template<typename PixelData_t>
bool WPngImage::PngData<PixelData_t>::allPixelsHaveFullAlpha() const
{
    for(std::size_t i = 0; i < mPixelData.size(); ++i)
        if(!pixelHasFullAlpha(mPixelData[i]))
            return false;
    return true;
}

//----------------------------------------------------------------------------
// Set pixel
//----------------------------------------------------------------------------
template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::setPixel(std::size_t index, const Pixel8& pixel)
{
    assignPixel(mPixelData[index], pixel);
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::setPixel(std::size_t index, const Pixel16& pixel)
{
    assignPixel(mPixelData[index], pixel);
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::setPixel(std::size_t index, const PixelF& pixel)
{
    assignPixel(mPixelData[index], pixel);
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::setPixel(std::size_t index, const PixelG8& pixel)
{
    assignPixel(mPixelData[index], pixel);
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::setPixel(std::size_t index, const PixelG16& pixel)
{
    assignPixel(mPixelData[index], pixel);
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::setPixel(std::size_t index, const PixelGF& pixel)
{
    assignPixel(mPixelData[index], pixel);
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::drawPixel(std::size_t index, const Pixel8& pixel)
{
    mPixelData[index].blendWith(PixelData_t(pixel));
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::drawPixel(std::size_t index, const Pixel16& pixel)
{
    mPixelData[index].blendWith(PixelData_t(pixel));
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::drawPixel(std::size_t index, const PixelF& pixel)
{
    mPixelData[index].blendWith(PixelData_t(pixel));
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::fill(const Pixel8& pixel)
{
    for(std::size_t i = 0; i < mPixelData.size(); ++i)
        assignPixel(mPixelData[i], pixel);
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::fill(const Pixel16& pixel)
{
    for(std::size_t i = 0; i < mPixelData.size(); ++i)
        assignPixel(mPixelData[i], pixel);
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::fill(const PixelF& pixel)
{
    for(std::size_t i = 0; i < mPixelData.size(); ++i)
        assignPixel(mPixelData[i], pixel);
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::transform(TransformFunc8 func)
{
    for(std::size_t i = 0; i < mPixelData.size(); ++i)
        assignPixel(mPixelData[i], func(convertToPixel<Pixel8>(mPixelData[i])));
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::transform(TransformFunc16 func)
{
    for(std::size_t i = 0; i < mPixelData.size(); ++i)
        assignPixel(mPixelData[i], func(convertToPixel<Pixel16>(mPixelData[i])));
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::transform(TransformFuncF func)
{
    for(std::size_t i = 0; i < mPixelData.size(); ++i)
        assignPixel(mPixelData[i], func(convertToPixel<PixelF>(mPixelData[i])));
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::transform(TransformFunc8 func, WPngImage& dest) const
{
    for(std::size_t i = 0; i < mPixelData.size(); ++i)
        dest.mData->setPixel(i, func(convertToPixel<Pixel8>(mPixelData[i])));
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::transform(TransformFunc16 func, WPngImage& dest) const
{
    for(std::size_t i = 0; i < mPixelData.size(); ++i)
        dest.mData->setPixel(i, func(convertToPixel<Pixel16>(mPixelData[i])));
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::transform(TransformFuncF func, WPngImage& dest) const
{
    for(std::size_t i = 0; i < mPixelData.size(); ++i)
        dest.mData->setPixel(i, func(convertToPixel<PixelF>(mPixelData[i])));
}


//----------------------------------------------------------------------------
// Copying
//----------------------------------------------------------------------------
template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::copyPixelTo(std::size_t srcIndex,
                                                  PngDataBase* dest, std::size_t destIndex) const
{
    dest->setPixel(destIndex, mPixelData[srcIndex]);
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::copyAllPixelsTo(PngDataBase* dest) const
{
    for(std::size_t i = 0; i < mPixelData.size(); ++i)
        dest->setPixel(i, mPixelData[i]);
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::copyPixelLineTo
(std::size_t srcStartIndex, std::size_t amount,
 PngDataBase* dest, std::size_t destStartIndex, bool useBlending) const
{
    if(useBlending)
    {
        switch(dest->mPixelFormat)
        {
          case kPixelFormat_GA8:
          case kPixelFormat_RGBA8:
              for(std::size_t i = 0; i < amount; ++i)
                  dest->setPixel(destStartIndex + i, dest->getPixel8(destStartIndex + i)
                                 .blendedPixel(getPixel8(srcStartIndex + i)));
              break;

          case kPixelFormat_GA16:
          case kPixelFormat_RGBA16:
              for(std::size_t i = 0; i < amount; ++i)
                  dest->setPixel(destStartIndex + i, dest->getPixel16(destStartIndex + i)
                                 .blendedPixel(getPixel16(srcStartIndex + i)));
              break;

          case kPixelFormat_GAF:
          case kPixelFormat_RGBAF:
              for(std::size_t i = 0; i < amount; ++i)
                  dest->setPixel(destStartIndex + i, dest->getPixelF(destStartIndex + i)
                                 .blendedPixel(getPixelF(srcStartIndex + i)));
              break;
        }
    }
    else
    {
        for(std::size_t i = 0; i < amount; ++i)
            dest->setPixel(destStartIndex + i, mPixelData[srcStartIndex + i]);
    }
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::blendLine
(std::size_t startIndex, std::size_t length, std::size_t step, const PixelData_t& pixel)
{
    for(std::size_t i = 0; i < length; ++i, startIndex += step)
        mPixelData[startIndex].blendWith(pixel);
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::assignLine
(std::size_t startIndex, std::size_t length, std::size_t step, const PixelData_t& pixel)
{
    for(std::size_t i = 0; i < length; ++i, startIndex += step)
        mPixelData[startIndex] = pixel;
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::addLine
(std::size_t startIndex, std::size_t length, std::size_t step, const Pixel8& pixel,
 bool useBlending)
{
    if(useBlending)
        blendLine(startIndex, length, step, PixelData_t(pixel));
    else
        assignLine(startIndex, length, step, PixelData_t(pixel));
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::addLine
(std::size_t startIndex, std::size_t length, std::size_t step, const Pixel16& pixel,
 bool useBlending)
{
    if(useBlending)
        blendLine(startIndex, length, step, PixelData_t(pixel));
    else
        assignLine(startIndex, length, step, PixelData_t(pixel));
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::addLine
(std::size_t startIndex, std::size_t length, std::size_t step, const PixelF& pixel,
 bool useBlending)
{
    if(useBlending)
        blendLine(startIndex, length, step, PixelData_t(pixel));
    else
        assignLine(startIndex, length, step, PixelData_t(pixel));
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::premultiplyAlpha()
{
    for(typename std::vector<PixelData_t>::iterator iter = mPixelData.begin();
        iter != mPixelData.end(); ++iter)
    {
        iter->premultiplyAlpha();
    }
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::flipHorizontally(int width, int height)
{
    const int maxX = width / 2;
    PixelData_t *data = &mPixelData[0];
    for(int y = 0; y < height; ++y, data += width)
        for(int x1 = 0, x2 = width - 1; x1 < maxX; ++x1, --x2)
            std::swap(data[x1], data[x2]);
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::flipVertically(int width, int height)
{
    const int maxY = height / 2;
    PixelData_t *data1 = &mPixelData[0], *data2 = &mPixelData[width * (height - 1)];
    for(int y = 0; y < maxY; ++y, data1 += width, data2 -= width)
        for(int x = 0; x < width; ++x)
            std::swap(data1[x], data2[x]);
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::rotate180(int width, int height)
{
    const int maxY = height / 2;
    PixelData_t *data1 = &mPixelData[0], *data2 = &mPixelData[width * (height - 1)];
    for(int y = 0; y < maxY; ++y, data1 += width, data2 -= width)
        for(int x1 = 0, x2 = width - 1; x1 < width; ++x1, --x2)
            std::swap(data1[x1], data2[x2]);

    if(height % 2 == 1)
    {
        const int maxX = width / 2;
        for(int x1 = 0, x2 = width - 1; x1 < maxX; ++x1, --x2)
            std::swap(data1[x1], data1[x2]);
    }
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::rotate90cwSquare(int width)
{
    PixelData_t *startPtr1 = &mPixelData[0];
    PixelData_t *startPtr2 = &mPixelData[width - 1];
    PixelData_t *startPtr3 = &mPixelData[width*width - 1];
    PixelData_t *startPtr4 = &mPixelData[width*(width - 1)];

    for(int length = width - 1; length > 0; length -= 2)
    {
        PixelData_t *ptr1 = startPtr1, *ptr2 = startPtr2, *ptr3 = startPtr3, *ptr4 = startPtr4;
        for(int x = 0; x < length; ++x)
        {
            const PixelData_t pixel = *ptr4;
            *ptr4 = *ptr3;
            *ptr3 = *ptr2;
            *ptr2 = *ptr1;
            *ptr1 = pixel;
            ++ptr1;
            ptr2 += width;
            --ptr3;
            ptr4 -= width;
        }

        startPtr1 += width + 1;
        startPtr2 += width - 1;
        startPtr3 -= width + 1;
        startPtr4 -= width - 1;
    }
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::rotate90cwNonsquare(int width, int height)
{
    std::vector<PixelData_t> newPixelData;
    newPixelData.reserve(mPixelData.size());

    PixelData_t *data = &mPixelData[width * (height - 1)];
    const int ptrOffset = width * height + 1;

    for(int x = 0; x < width; ++x, data += ptrOffset)
        for(int y = 0; y < height; ++y, data -= width)
            newPixelData.push_back(*data);

    mPixelData.swap(newPixelData);
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::rotate90ccwSquare(int width)
{
    PixelData_t *startPtr1 = &mPixelData[0];
    PixelData_t *startPtr2 = &mPixelData[width - 1];
    PixelData_t *startPtr3 = &mPixelData[width*width - 1];
    PixelData_t *startPtr4 = &mPixelData[width*(width - 1)];

    for(int length = width - 1; length > 0; length -= 2)
    {
        PixelData_t *ptr1 = startPtr1, *ptr2 = startPtr2, *ptr3 = startPtr3, *ptr4 = startPtr4;
        for(int x = 0; x < length; ++x)
        {
            const PixelData_t pixel = *ptr1;
            *ptr1 = *ptr2;
            *ptr2 = *ptr3;
            *ptr3 = *ptr4;
            *ptr4 = pixel;
            ++ptr1;
            ptr2 += width;
            --ptr3;
            ptr4 -= width;
        }

        startPtr1 += width + 1;
        startPtr2 += width - 1;
        startPtr3 -= width + 1;
        startPtr4 -= width - 1;
    }
}

template<typename PixelData_t>
void WPngImage::PngData<PixelData_t>::rotate90ccwNonsquare(int width, int height)
{
    std::vector<PixelData_t> newPixelData;
    newPixelData.reserve(mPixelData.size());

    PixelData_t *data = &mPixelData[width - 1];
    const int ptrOffset = width * height + 1;

    for(int x = 0; x < width; ++x, data -= ptrOffset)
        for(int y = 0; y < height; ++y, data += width)
            newPixelData.push_back(*data);

    mPixelData.swap(newPixelData);
}


//============================================================================
// WPngImage constructors, assignment, destructor
//============================================================================
WPngImage::WPngImage(): mData(0), mWidth(0), mHeight(0)
{}

WPngImage::WPngImage(int width, int height, PixelFormat pixelFormat):
    mData(0), mWidth(0), mHeight(0)
{
    newImage(width, height, pixelFormat);
}

WPngImage::WPngImage(int width, int height, Pixel8 pixel, PixelFormat pixelFormat):
    mData(0), mWidth(0), mHeight(0)
{
    newImage(width, height, pixel, pixelFormat);
}

WPngImage::WPngImage(int width, int height, Pixel16 pixel, PixelFormat pixelFormat):
    mData(0), mWidth(0), mHeight(0)
{
    newImage(width, height, pixel, pixelFormat);
}

WPngImage::WPngImage(int width, int height, PixelF pixel, PixelFormat pixelFormat):
    mData(0), mWidth(0), mHeight(0)
{
    newImage(width, height, pixel, pixelFormat);
}

WPngImage::WPngImage(const WPngImage& rhs):
    mData(rhs.mData ? rhs.mData->createCopy() : 0),
    mWidth(rhs.mWidth), mHeight(rhs.mHeight)
{}

WPngImage::~WPngImage()
{
    delete mData;
}

WPngImage& WPngImage::operator=(const WPngImage& rhs)
{
    if(this != &rhs)
    {
        mWidth = rhs.mWidth;
        mHeight = rhs.mHeight;

        if(mData && rhs.mData && mData->assignAllDataFrom(rhs.mData))
            return *this;

        delete mData;
        mData = rhs.mData ? rhs.mData->createCopy() : 0;
    }
    return *this;
}

#if !WPNGIMAGE_RESTRICT_TO_CPP98
WPngImage::WPngImage(WPngImage&& rhs) noexcept:
    mData(rhs.mData), mWidth(rhs.mWidth), mHeight(rhs.mHeight)
{
    rhs.mData = nullptr;
    rhs.mWidth = rhs.mHeight = 0;
}

WPngImage& WPngImage::operator=(WPngImage&& rhs) noexcept
{
    if(this != &rhs)
    {
        mData = rhs.mData;
        mWidth = rhs.mWidth;
        mHeight = rhs.mHeight;
        rhs.mData = nullptr;
        rhs.mWidth = rhs.mHeight = 0;
    }
    return *this;
}
#endif

void WPngImage::swap(WPngImage& other)
{
    std::swap(mData, other.mData);
    std::swap(mWidth, other.mWidth);
    std::swap(mHeight, other.mHeight);
}

void WPngImage::move(WPngImage& rhs)
{
    if(this != &rhs)
    {
        mData = rhs.mData;
        mWidth = rhs.mWidth;
        mHeight = rhs.mHeight;
        rhs.mData = 0;
        rhs.mWidth = rhs.mHeight = 0;
    }
}


//============================================================================
// Create a new image
//============================================================================
template<typename Pixel_t>
void WPngImage::newImageWithPixelValue(int width, int height, Pixel_t pixel,
                                       PixelFormat pixelFormat)
{
    delete mData;
    mData = 0;
    mWidth = mHeight = 0;

    if(width <= 0 || height <= 0)
        return;

    switch(pixelFormat)
    {
      case kPixelFormat_GA8:
          mData = new PngData<PixelG8>(width, height, pixel, pixelFormat);
          break;

      case kPixelFormat_GA16:
          mData = new PngData<PixelG16>(width, height, pixel, pixelFormat);
          break;

      case kPixelFormat_GAF:
          mData = new PngData<PixelGF>(width, height, pixel, pixelFormat);
          break;

      case kPixelFormat_RGBA8:
          mData = new PngData<Pixel8>(width, height, pixel, pixelFormat);
          break;

      case kPixelFormat_RGBA16:
          mData = new PngData<Pixel16>(width, height, pixel, pixelFormat);
          break;

      case kPixelFormat_RGBAF:
          mData = new PngData<PixelF>(width, height, pixel, pixelFormat);
          break;
    }

    if(mData)
    {
        mWidth = width;
        mHeight = height;
    }
}

void WPngImage::newImage(int width, int height, PixelFormat pixelFormat)
{
    newImageWithPixelValue(width, height, Pixel8(), pixelFormat);
}

void WPngImage::newImage(int width, int height, Pixel8 pixel, PixelFormat pixelFormat)
{
    newImageWithPixelValue(width, height, pixel, pixelFormat);
}

void WPngImage::newImage(int width, int height, Pixel16 pixel, PixelFormat pixelFormat)
{
    newImageWithPixelValue(width, height, pixel, pixelFormat);
}

void WPngImage::newImage(int width, int height, PixelF pixel, PixelFormat pixelFormat)
{
    newImageWithPixelValue(width, height, pixel, pixelFormat);
}


//============================================================================
// Image information getters
//============================================================================
WPngImage::PixelFormat WPngImage::currentPixelFormat() const
{
    return mData ? mData->mPixelFormat : kPixelFormat_RGBA8;
}

WPngImage::PngFileFormat WPngImage::originalFileFormat() const
{
    return mData ? mData->mPngFileFormat : kPngFileFormat_none;
}

void WPngImage::setFileFormat(PngFileFormat newFormat)
{
    if(mData) mData->mPngFileFormat = newFormat;
}

void WPngImage::setFileFormat(PngWriteConvert conversion)
{
    if(mData) mData->mPngFileFormat =
                  getFileFormat(conversion, mData->mPngFileFormat, currentPixelFormat());
}

bool WPngImage::isGrayscalePixelFormat() const
{
    const PixelFormat pixelFormat = currentPixelFormat();
    return (pixelFormat == kPixelFormat_GA8 ||
            pixelFormat == kPixelFormat_GA16 ||
            pixelFormat == kPixelFormat_GAF);
}

bool WPngImage::isRGBAPixelFormat() const
{
    const PixelFormat pixelFormat = currentPixelFormat();
    return (pixelFormat == kPixelFormat_RGBA8 ||
            pixelFormat == kPixelFormat_RGBA16 ||
            pixelFormat == kPixelFormat_RGBAF);
}

bool WPngImage::is8BPCPixelFormat() const
{
    const PixelFormat pixelFormat = currentPixelFormat();
    return (pixelFormat == kPixelFormat_RGBA8 ||
            pixelFormat == kPixelFormat_GA8);
}

bool WPngImage::is16BPCPixelFormat() const
{
    const PixelFormat pixelFormat = currentPixelFormat();
    return (pixelFormat == kPixelFormat_RGBA16 ||
            pixelFormat == kPixelFormat_GA16);
}

bool WPngImage::isFloatPixelFormat() const
{
    const PixelFormat pixelFormat = currentPixelFormat();
    return (pixelFormat == kPixelFormat_RGBAF ||
            pixelFormat == kPixelFormat_GAF);
}

void WPngImage::convertToPixelFormat(PixelFormat newPixelFormat)
{
    if(mData && newPixelFormat != mData->mPixelFormat)
    {
        WPngImage newImage(width(), height(), Pixel8(), newPixelFormat);
        mData->copyAllPixelsTo(newImage.mData);
        newImage.mData->mPngFileFormat = mData->mPngFileFormat;
        this->swap(newImage);
    }
}


//============================================================================
// Get and set pixels
//============================================================================
WPngImage::Pixel8 WPngImage::get8(int x, int y) const
{
    if(x < 0 || x >= mWidth || y < 0 || y >= mHeight || !mData)
        return Pixel8(0, 0, 0, 0);
    return mData->getPixel8(std::size_t(y * mWidth + x));
}

WPngImage::Pixel16 WPngImage::get16(int x, int y) const
{
    if(x < 0 || x >= mWidth || y < 0 || y >= mHeight || !mData)
        return Pixel16(0, 0, 0, 0);
    return mData->getPixel16(std::size_t(y * mWidth + x));
}

WPngImage::PixelF WPngImage::getF(int x, int y) const
{
    if(x < 0 || x >= mWidth || y < 0 || y >= mHeight || !mData)
        return PixelF(0, 0, 0, 0);
    return mData->getPixelF(std::size_t(y * mWidth + x));
}

void WPngImage::set(int x, int y, Pixel8 pixel)
{
    if(x >= 0 && x < mWidth && y >= 0 && y < mHeight && mData)
        mData->setPixel(std::size_t(y * mWidth + x), pixel);
}

void WPngImage::set(int x, int y, Pixel16 pixel)
{
    if(x >= 0 && x < mWidth && y >= 0 && y < mHeight && mData)
        mData->setPixel(std::size_t(y * mWidth + x), pixel);
}

void WPngImage::set(int x, int y, PixelF pixel)
{
    if(x >= 0 && x < mWidth && y >= 0 && y < mHeight && mData)
        mData->setPixel(std::size_t(y * mWidth + x), pixel);
}

void WPngImage::fill(Pixel8 pixel)
{
    if(mData) mData->fill(pixel);
}

void WPngImage::fill(Pixel16 pixel)
{
    if(mData) mData->fill(pixel);
}

void WPngImage::fill(PixelF pixel)
{
    if(mData) mData->fill(pixel);
}

void WPngImage::transform(TransformFunc8 func)
{
    if(mData) mData->transform(func);
}

void WPngImage::transform(TransformFunc16 func)
{
    if(mData) mData->transform(func);
}

void WPngImage::transform(TransformFuncF func)
{
    if(mData) mData->transform(func);
}

void WPngImage::transform(TransformFunc8 func, WPngImage& dest) const
{
    if(mData)
    {
        if(dest.width() != width() || dest.height() != height())
            dest.newImage(width(), height(),
                          dest.mData ? dest.currentPixelFormat() : currentPixelFormat());
        mData->transform(func, dest);
    }
}

void WPngImage::transform(TransformFunc16 func, WPngImage& dest) const
{
    if(mData)
    {
        if(dest.width() != width() || dest.height() != height())
            dest.newImage(width(), height(),
                          dest.mData ? dest.currentPixelFormat() : currentPixelFormat());
        mData->transform(func, dest);
    }
}

void WPngImage::transform(TransformFuncF func, WPngImage& dest) const
{
    if(mData)
    {
        if(dest.width() != width() || dest.height() != height())
            dest.newImage(width(), height(),
                          dest.mData ? dest.currentPixelFormat() : currentPixelFormat());
        mData->transform(func, dest);
    }
}

// These are called by the loading functions. They assume the index is valid.
void WPngImage::setPixel(std::size_t index, const Pixel8& p)
{
    mData->setPixel(index, p);
}

void WPngImage::setPixel(std::size_t index, const Pixel16& p)
{
    mData->setPixel(index, p);
}


//============================================================================
// Image transformations
//============================================================================
void WPngImage::flipHorizontally()
{
    if(mData) mData->flipHorizontally(mWidth, mHeight);
}

void WPngImage::flipVertically()
{
    if(mData) mData->flipVertically(mWidth, mHeight);
}

void WPngImage::rotate180()
{
    if(mData) mData->rotate180(mWidth, mHeight);
}

void WPngImage::rotate90cw()
{
    if(mData)
    {
        if(mWidth == mHeight)
        {
            mData->rotate90cwSquare(mWidth);
        }
        else
        {
            mData->rotate90cwNonsquare(mWidth, mHeight);
            std::swap(mWidth, mHeight);
        }
    }
}

void WPngImage::rotate90ccw()
{
    if(mData)
    {
        if(mWidth == mHeight)
        {
            mData->rotate90ccwSquare(mWidth);
        }
        else
        {
            mData->rotate90ccwNonsquare(mWidth, mHeight);
            std::swap(mWidth, mHeight);
        }
    }
}


//============================================================================
// Drawing functions
//============================================================================
void WPngImage::drawPixel(int x, int y, Pixel8 pixel)
{
    if(x >= 0 && x < mWidth && y >= 0 && y < mHeight && mData)
        mData->drawPixel(std::size_t(y*width() + x), pixel);
}

void WPngImage::drawPixel(int x, int y, Pixel16 pixel)
{
    if(x >= 0 && x < mWidth && y >= 0 && y < mHeight && mData)
        mData->drawPixel(std::size_t(y*width() + x), pixel);
}

void WPngImage::drawPixel(int x, int y, PixelF pixel)
{
    if(x >= 0 && x < mWidth && y >= 0 && y < mHeight && mData)
        mData->drawPixel(std::size_t(y*width() + x), pixel);
}

void WPngImage::putImage(int destX, int destY, const WPngImage& src,
                         int srcX, int srcY, int srcWidth, int srcHeight, bool useBlending)
{
    if(!mData || !src.mData || srcWidth <= 0 || srcHeight <= 0 ||
       width() <= 0 || height() <= 0 || src.width() <= 0 || src.height() <= 0 ||
       srcX >= src.width() || srcY >= src.height() ||
       srcX + srcWidth <= 0 || srcY + srcHeight <= 0)
        return;

    if(srcX < 0) { destX -= srcX; srcWidth += srcX; srcX = 0; }
    if(srcY < 0) { destY -= srcY; srcHeight += srcY; srcY = 0; }
    if(srcX + srcWidth > src.width()) srcWidth = src.width() - srcX;
    if(srcY + srcHeight > src.height()) srcHeight = src.height() - srcY;

    if(destX >= width() || destY >= height() ||
       destX + srcWidth <= 0 || destY + srcHeight <= 0)
        return;

    if(destX < 0) { srcX -= destX; srcWidth += destX; destX = 0; }
    if(destY < 0) { srcY -= destY; srcHeight += destY; destY = 0; }
    if(destX + srcWidth > width()) srcWidth = width() - destX;
    if(destY + srcHeight > height()) srcHeight = height() - destY;

    for(int lineInd = 0; lineInd < srcHeight; ++lineInd)
    {
        const int srcStartInd = (srcY + lineInd) * src.width() + srcX;
        const int destStartInd = (destY + lineInd) * width() + destX;
        src.mData->copyPixelLineTo(std::size_t(srcStartInd), srcWidth,
                                   mData, std::size_t(destStartInd), useBlending);
    }
}

void WPngImage::putImage(int destX, int destY, const WPngImage& src)
{
    putImage(destX, destY, src, 0, 0, src.width(), src.height(), false);
}

void WPngImage::putImage(int destX, int destY, const WPngImage& src,
                         int srcX, int srcY, int srcWidth, int srcHeight)
{
    putImage(destX, destY, src, srcX, srcY, srcWidth, srcHeight, false);
}

void WPngImage::drawImage(int destX, int destY, const WPngImage& src)
{
    putImage(destX, destY, src, 0, 0, src.width(), src.height(), true);
}

void WPngImage::drawImage(int destX, int destY, const WPngImage& src,
                          int srcX, int srcY, int srcWidth, int srcHeight)
{
    putImage(destX, destY, src, srcX, srcY, srcWidth, srcHeight, true);
}

template<typename Pixel_t>
void WPngImage::addHorLine(int x, int y, int length, const Pixel_t& pixel, bool useBlending)
{
    if(!mData || length == 0 || y < 0 || y >= height()) return;
    if(length < 0) { length = -length; x = x - length + 1; }
    if(x >= width()) return;
    if(x < 0) { length += x; x = 0; }
    if(length <= 0) return;
    if(x + length > width()) { length = width() - x; }
    mData->addLine(std::size_t(y*width() + x), std::size_t(length), 1, pixel, useBlending);
}

template<typename Pixel_t>
void WPngImage::addVertLine(int x, int y, int length, const Pixel_t& pixel, bool useBlending)
{
    if(!mData || length == 0 || x < 0 || x >= width()) return;
    if(length < 0) { length = -length; y = y - length + 1; }
    if(y >= height()) return;
    if(y < 0) { length += y; y = 0; }
    if(length <= 0) return;
    if(y + length > height()) { length = height() - y; }
    mData->addLine(std::size_t(y*width() + x), std::size_t(length), std::size_t(width()), pixel,
                   useBlending);
}

void WPngImage::putHorLine(int x, int y, int length, Pixel8 pixel)
{
    addHorLine(x, y, length, pixel, false);
}

void WPngImage::putHorLine(int x, int y, int length, Pixel16 pixel)
{
    addHorLine(x, y, length, pixel, false);
}

void WPngImage::putHorLine(int x, int y, int length, PixelF pixel)
{
    addHorLine(x, y, length, pixel, false);
}

void WPngImage::putVertLine(int x, int y, int length, Pixel8 pixel)
{
    addVertLine(x, y, length, pixel, false);
}

void WPngImage::putVertLine(int x, int y, int length, Pixel16 pixel)
{
    addVertLine(x, y, length, pixel, false);
}

void WPngImage::putVertLine(int x, int y, int length, PixelF pixel)
{
    addVertLine(x, y, length, pixel, false);
}

void WPngImage::drawHorLine(int x, int y, int length, Pixel8 pixel)
{
    addHorLine(x, y, length, pixel, true);
}

void WPngImage::drawHorLine(int x, int y, int length, Pixel16 pixel)
{
    addHorLine(x, y, length, pixel, true);
}

void WPngImage::drawHorLine(int x, int y, int length, PixelF pixel)
{
    addHorLine(x, y, length, pixel, true);
}

void WPngImage::drawVertLine(int x, int y, int length, Pixel8 pixel)
{
    addVertLine(x, y, length, pixel, true);
}

void WPngImage::drawVertLine(int x, int y, int length, Pixel16 pixel)
{
    addVertLine(x, y, length, pixel, true);
}

void WPngImage::drawVertLine(int x, int y, int length, PixelF pixel)
{
    addVertLine(x, y, length, pixel, true);
}

template<typename Pixel_t>
void WPngImage::addRect(int x, int y, int rectWidth, int rectHeight, const Pixel_t& pixel,
                        bool filled, bool useBlending)
{
    if(!mData || rectWidth == 0 || rectHeight == 0) return;
    if(rectWidth < 0) { rectWidth = -rectWidth; x = x - rectWidth + 1; }
    if(rectHeight < 0) { rectHeight = -rectHeight; y = y - rectHeight + 1; }
    if(rectWidth == 1) return addVertLine(x, y, rectHeight, pixel, useBlending);
    if(rectHeight == 1) return addHorLine(x, y, rectWidth, pixel, useBlending);

    if(!filled)
    {
        addHorLine(x, y, rectWidth, pixel, useBlending);
        addHorLine(x, y + rectHeight - 1, rectWidth, pixel, useBlending);
        addVertLine(x, y + 1, rectHeight - 2, pixel, useBlending);
        addVertLine(x + rectWidth - 1, y + 1, rectHeight - 2, pixel, useBlending);
    }
    else
    {
        if(x >= width() || y >= height()) return;
        if(x < 0) { rectWidth += x; x = 0; }
        if(y < 0) { rectHeight += y; y = 0; }
        if(rectWidth <= 0 || rectHeight <= 0) return;
        if(x + rectWidth > width()) { rectWidth = width() - x; }
        if(y + rectHeight > height()) { rectHeight = height() - y; }

        const std::size_t indexBegin = std::size_t(y * width() + x);
        const std::size_t step = std::size_t(width());

        if(rectWidth == 1)
        {
            mData->addLine(indexBegin, std::size_t(rectHeight), step, pixel, useBlending);
        }
        else
        {
            const std::size_t indexEnd = indexBegin + step * std::size_t(rectHeight);
            const std::size_t length = std::size_t(rectWidth);

            for(std::size_t index = indexBegin; index < indexEnd; index += step)
                mData->addLine(index, length, 1, pixel, useBlending);
        }
    }
}

void WPngImage::putRect(int x, int y, int width, int height, Pixel8 pixel, bool filled)
{
    addRect(x, y, width, height, pixel, filled, false);
}

void WPngImage::putRect(int x, int y, int width, int height, Pixel16 pixel, bool filled)
{
    addRect(x, y, width, height, pixel, filled, false);
}

void WPngImage::putRect(int x, int y, int width, int height, PixelF pixel, bool filled)
{
    addRect(x, y, width, height, pixel, filled, false);
}

void WPngImage::drawRect(int x, int y, int width, int height, Pixel8 pixel, bool filled)
{
    addRect(x, y, width, height, pixel, filled, true);
}

void WPngImage::drawRect(int x, int y, int width, int height, Pixel16 pixel, bool filled)
{
    addRect(x, y, width, height, pixel, filled, true);
}

void WPngImage::drawRect(int x, int y, int width, int height, PixelF pixel, bool filled)
{
    addRect(x, y, width, height, pixel, filled, true);
}

void WPngImage::resizeCanvas(int newOriginX, int newOriginY, int newWidth, int newHeight)
{
    resizeCanvas(newOriginX, newOriginY, newWidth, newHeight, Pixel8(0, 0, 0, 0));
}

void WPngImage::manageCanvasResize(WPngImage& newImage, int newOriginX, int newOriginY)
{
    newImage.putImage(-newOriginX, -newOriginY, *this);
    newImage.mData->mPngFileFormat = mData->mPngFileFormat;
    this->swap(newImage);
}

void WPngImage::resizeCanvas(int newOriginX, int newOriginY, int newWidth, int newHeight,
                             Pixel8 pixel)
{
    if(mData && (newOriginX != 0 || newOriginY != 0 ||
                 newWidth != width() || newHeight != height()))
    {
        WPngImage newImage(newWidth, newHeight, pixel, currentPixelFormat());
        manageCanvasResize(newImage, newOriginX, newOriginY);
    }
}

void WPngImage::resizeCanvas(int newOriginX, int newOriginY, int newWidth, int newHeight,
                             Pixel16 pixel)
{
    if(mData && (newOriginX != 0 || newOriginY != 0 ||
                 newWidth != width() || newHeight != height()))
    {
        WPngImage newImage(newWidth, newHeight, pixel, currentPixelFormat());
        manageCanvasResize(newImage, newOriginX, newOriginY);
    }
}

void WPngImage::resizeCanvas(int newOriginX, int newOriginY, int newWidth, int newHeight,
                             PixelF pixel)
{
    if(mData && (newOriginX != 0 || newOriginY != 0 ||
                 newWidth != width() || newHeight != height()))
    {
        WPngImage newImage(newWidth, newHeight, pixel, currentPixelFormat());
        manageCanvasResize(newImage, newOriginX, newOriginY);
    }
}

void WPngImage::premultiplyAlpha()
{
    if(mData) mData->premultiplyAlpha();
}

const WPngImage::Pixel8* WPngImage::getRawPixelData8() const
{
    return mData && mData->mPixelFormat == kPixelFormat_RGBA8 ?
        &(static_cast<const PngData<Pixel8>*>(mData)->mPixelData[0]) : 0;
}

WPngImage::Pixel8* WPngImage::getRawPixelData8()
{
    return mData && mData->mPixelFormat == kPixelFormat_RGBA8 ?
        &(static_cast<PngData<Pixel8>*>(mData)->mPixelData[0]) : 0;
}

const WPngImage::Pixel16* WPngImage::getRawPixelData16() const
{
    return mData && mData->mPixelFormat == kPixelFormat_RGBA16 ?
        &(static_cast<const PngData<Pixel16>*>(mData)->mPixelData[0]) : 0;
}

WPngImage::Pixel16* WPngImage::getRawPixelData16()
{
    return mData && mData->mPixelFormat == kPixelFormat_RGBA16 ?
        &(static_cast<PngData<Pixel16>*>(mData)->mPixelData[0]) : 0;
}

const WPngImage::PixelF* WPngImage::getRawPixelDataF() const
{
    return mData && mData->mPixelFormat == kPixelFormat_RGBAF ?
        &(static_cast<const PngData<PixelF>*>(mData)->mPixelData[0]) : 0;
}

WPngImage::PixelF* WPngImage::getRawPixelDataF()
{
    return mData && mData->mPixelFormat == kPixelFormat_RGBAF ?
        &(static_cast<PngData<PixelF>*>(mData)->mPixelData[0]) : 0;
}


//============================================================================
// Auxiliary functions for reading PNG data
//============================================================================
WPngImage::PixelFormat WPngImage::getPixelFormat(WPngImage::PngReadConvert conversion,
                                                 WPngImage::PngFileFormat fileFormat)
{
    switch(conversion)
    {
      case WPngImage::kPngReadConvert_closestMatch:
          switch(fileFormat)
          {
            case WPngImage::kPngFileFormat_none: return WPngImage::kPixelFormat_RGBA8;
            case WPngImage::kPngFileFormat_GA8: return WPngImage::kPixelFormat_GA8;
            case WPngImage::kPngFileFormat_GA16: return WPngImage::kPixelFormat_GA16;
            case WPngImage::kPngFileFormat_RGBA8: return WPngImage::kPixelFormat_RGBA8;
            case WPngImage::kPngFileFormat_RGBA16: return WPngImage::kPixelFormat_RGBA16;
          }
          break;

      case WPngImage::kPngReadConvert_8bit:
          switch(fileFormat)
          {
            case WPngImage::kPngFileFormat_none: return WPngImage::kPixelFormat_RGBA8;
            case WPngImage::kPngFileFormat_GA8: return WPngImage::kPixelFormat_GA8;
            case WPngImage::kPngFileFormat_GA16: return WPngImage::kPixelFormat_GA8;
            case WPngImage::kPngFileFormat_RGBA8: return WPngImage::kPixelFormat_RGBA8;
            case WPngImage::kPngFileFormat_RGBA16: return WPngImage::kPixelFormat_RGBA8;
          }
          break;

      case WPngImage::kPngReadConvert_16bit:
          switch(fileFormat)
          {
            case WPngImage::kPngFileFormat_none: return WPngImage::kPixelFormat_RGBA16;
            case WPngImage::kPngFileFormat_GA8: return WPngImage::kPixelFormat_GA16;
            case WPngImage::kPngFileFormat_GA16: return WPngImage::kPixelFormat_GA16;
            case WPngImage::kPngFileFormat_RGBA8: return WPngImage::kPixelFormat_RGBA16;
            case WPngImage::kPngFileFormat_RGBA16: return WPngImage::kPixelFormat_RGBA16;
          }
          break;

      case WPngImage::kPngReadConvert_Float:
          switch(fileFormat)
          {
            case WPngImage::kPngFileFormat_none: return WPngImage::kPixelFormat_RGBAF;
            case WPngImage::kPngFileFormat_GA8: return WPngImage::kPixelFormat_GAF;
            case WPngImage::kPngFileFormat_GA16: return WPngImage::kPixelFormat_GAF;
            case WPngImage::kPngFileFormat_RGBA8: return WPngImage::kPixelFormat_RGBAF;
            case WPngImage::kPngFileFormat_RGBA16: return WPngImage::kPixelFormat_RGBAF;
          }
          break;

      case WPngImage::kPngReadConvert_Grayscale:
          switch(fileFormat)
          {
            case WPngImage::kPngFileFormat_none: return WPngImage::kPixelFormat_GA8;
            case WPngImage::kPngFileFormat_GA8: return WPngImage::kPixelFormat_GA8;
            case WPngImage::kPngFileFormat_GA16: return WPngImage::kPixelFormat_GA16;
            case WPngImage::kPngFileFormat_RGBA8: return WPngImage::kPixelFormat_GA8;
            case WPngImage::kPngFileFormat_RGBA16: return WPngImage::kPixelFormat_GA16;
          }
          break;

      case WPngImage::kPngReadConvert_RGBA:
          switch(fileFormat)
          {
            case WPngImage::kPngFileFormat_none: return WPngImage::kPixelFormat_RGBA8;
            case WPngImage::kPngFileFormat_GA8: return WPngImage::kPixelFormat_RGBA8;
            case WPngImage::kPngFileFormat_GA16: return WPngImage::kPixelFormat_RGBA16;
            case WPngImage::kPngFileFormat_RGBA8: return WPngImage::kPixelFormat_RGBA8;
            case WPngImage::kPngFileFormat_RGBA16: return WPngImage::kPixelFormat_RGBA16;
          }
          break;
    }
    return WPngImage::kPixelFormat_RGBA8;
}


//----------------------------------------------------------------------------
// Load PNG image from file
//----------------------------------------------------------------------------
WPngImage::IOStatus WPngImage::loadImage(const char* fileName, PngReadConvert conversion)
{
    IOStatus status = performLoadImage(fileName, true, conversion, kPixelFormat_RGBA8);
    if(status != kIOStatus_Ok) status.fileName = fileName;
    return status;
}

WPngImage::IOStatus WPngImage::loadImage(const char* fileName, PixelFormat pixelFormat)
{
    IOStatus status = performLoadImage(fileName, false, kPngReadConvert_closestMatch, pixelFormat);
    if(status != kIOStatus_Ok) status.fileName = fileName;
    return status;
}

WPngImage::IOStatus WPngImage::loadImage(const std::string& fileName, PngReadConvert conversion)
{
    return loadImage(fileName.c_str(), conversion);
}

WPngImage::IOStatus WPngImage::loadImage(const std::string& fileName, PixelFormat pixelFormat)
{
    return loadImage(fileName.c_str(), pixelFormat);
}


//----------------------------------------------------------------------------
// Read PNG data from RAM
//----------------------------------------------------------------------------
WPngImage::IOStatus WPngImage::loadImageFromRAM(const void* pngData, std::size_t pngDataSize,
                                                PngReadConvert conversion)
{
    return performLoadImageFromRAM
        (pngData, pngDataSize, true, conversion, kPixelFormat_RGBA8);
}

WPngImage::IOStatus WPngImage::loadImageFromRAM(const void* pngData, std::size_t pngDataSize,
                                                PixelFormat pixelFormat)
{
    return performLoadImageFromRAM
        (pngData, pngDataSize, false, kPngReadConvert_closestMatch, pixelFormat);
}


//============================================================================
// Write PNG data
//============================================================================
bool WPngImage::allPixelsHaveFullAlpha() const
{
    return mData->allPixelsHaveFullAlpha();
}

template<typename CT>
static void setColorComponentsG(CT* dest, int colorComponents, const PixelG<CT>& pixel)
{
    dest[0] = pixel.g;
    if(colorComponents > 1) dest[1] = pixel.a;
}

template<typename CT, typename Pixel_t>
static void setColorComponents(CT* dest, int colorComponents, const Pixel_t& pixel)
{
    dest[0] = pixel.r;
    dest[1] = pixel.g;
    dest[2] = pixel.b;
    if(colorComponents > 3) dest[3] = pixel.a;
}

void WPngImage::setPixelRow
(PngFileFormat fileFormat, int y, Byte* dest, int colorComponents) const
{
    const int imageWidth = width();
    const std::size_t indexStart = y * imageWidth;

    switch(fileFormat)
    {
      case kPngFileFormat_GA8:
          for(int x = 0; x < imageWidth; ++x)
              setColorComponentsG(dest + (x * colorComponents), colorComponents,
                                  mData->getPixelG8(indexStart + x));
          break;

      case kPngFileFormat_RGBA8:
          for(int x = 0; x < imageWidth; ++x)
              setColorComponents(dest + (x * colorComponents), colorComponents,
                                 mData->getPixel8(indexStart + x));
          break;

      default: break;
    }
}

void WPngImage::setPixelRow
(PngFileFormat fileFormat, int y, UInt16* dest, int colorComponents) const
{
    const int imageWidth = width();
    const std::size_t indexStart = y * imageWidth;

    switch(fileFormat)
    {
      case kPngFileFormat_GA16:
          for(int x = 0; x < imageWidth; ++x)
              setColorComponentsG(dest + (x * colorComponents), colorComponents,
                                  mData->getPixelG16(indexStart + x));
          break;

      case kPngFileFormat_RGBA16:
          for(int x = 0; x < imageWidth; ++x)
              setColorComponents(dest + (x * colorComponents), colorComponents,
                                 mData->getPixel16(indexStart + x));
          break;

      default: break;
    }
}

//----------------------------------------------------------------------------
// Save PNG image to file
//----------------------------------------------------------------------------
WPngImage::PngFileFormat WPngImage::getClosestMatchFileFormat(PixelFormat pixelFormat)
{
    switch(pixelFormat)
    {
      case WPngImage::kPixelFormat_GA8: return WPngImage::kPngFileFormat_GA8;
      case WPngImage::kPixelFormat_GA16:
      case WPngImage::kPixelFormat_GAF: return WPngImage::kPngFileFormat_GA16;
      case WPngImage::kPixelFormat_RGBA8: return WPngImage::kPngFileFormat_RGBA8;
      case WPngImage::kPixelFormat_RGBA16:
      case WPngImage::kPixelFormat_RGBAF: return WPngImage::kPngFileFormat_RGBA16;
    }
    return WPngImage::kPngFileFormat_RGBA8;
}

WPngImage::PngFileFormat WPngImage::getFileFormat
(PngWriteConvert conversion, PngFileFormat originalFileFormat, PixelFormat currentPixelFormat)
{
    if(conversion == WPngImage::kPngWriteConvert_closestMatch)
        return getClosestMatchFileFormat(currentPixelFormat);
    return originalFileFormat;
}

WPngImage::IOStatus WPngImage::saveImage(const char* fileName, PngFileFormat fileFormat) const
{
    IOStatus status = performSaveImage(fileName, fileFormat);
    if(status != kIOStatus_Ok) status.fileName = fileName;
    return status;
}

WPngImage::IOStatus WPngImage::saveImage
(const char* fileName, PngWriteConvert conversion) const
{
    IOStatus status = performSaveImage
        (fileName, getFileFormat(conversion, originalFileFormat(), currentPixelFormat()));
    if(status != kIOStatus_Ok) status.fileName = fileName;
    return status;
}

WPngImage::IOStatus WPngImage::saveImage
(const std::string& fileName, PngWriteConvert conversion) const
{
    return saveImage(fileName.c_str(), conversion);
}

WPngImage::IOStatus WPngImage::saveImage
(const std::string& fileName, PngFileFormat fileFormat) const
{
    return saveImage(fileName.c_str(), fileFormat);
}


//----------------------------------------------------------------------------
// Write PNG data to RAM
//----------------------------------------------------------------------------
WPngImage::IOStatus WPngImage::saveImageToRAM(std::vector<unsigned char>& dest,
                                              PngFileFormat fileFormat) const
{
    return performSaveImageToRAM(&dest, 0, fileFormat);
}

WPngImage::IOStatus WPngImage::saveImageToRAM(std::vector<unsigned char>& dest,
                                              PngWriteConvert conversion) const
{
    return saveImageToRAM
        (dest, getFileFormat(conversion, originalFileFormat(), currentPixelFormat()));
}

WPngImage::IOStatus WPngImage::saveImageToRAM(ByteStreamOutputFunc destFunc,
                                              PngFileFormat fileFormat) const
{
    return performSaveImageToRAM(0, destFunc, fileFormat);
}

WPngImage::IOStatus WPngImage::saveImageToRAM(ByteStreamOutputFunc destFunc,
                                              PngWriteConvert conversion) const
{
    return saveImageToRAM
        (destFunc, getFileFormat(conversion, originalFileFormat(), currentPixelFormat()));
}


//============================================================================
// WPngImage::IOStatus implementations
//============================================================================
bool WPngImage::IOStatus::printErrorMsg(std::ostream& os) const
{
    switch(value)
    {
      case WPngImage::kIOStatus_Ok:
          return false;

      case WPngImage::kIOStatus_Error_CantOpenFile:
          os << fileName << ": " << std::strerror(errnoValue) << "\n";
          return true;

      case WPngImage::kIOStatus_Error_NotPNG:
          if(fileName.empty()) os << "Input data is not a PNG image.\n";
          else os << fileName << " is not a PNG file.\n";
          return true;

      case WPngImage::kIOStatus_Error_PNGLibraryError:
          os << "Error reading ";
          if(fileName.empty()) os << "input PNG data";
          else os << fileName;
          if(!pngLibErrorMsg.empty())
              os << ": " << pngLibErrorMsg;
          os << "\n";
          return true;
    }
    return false;
}
