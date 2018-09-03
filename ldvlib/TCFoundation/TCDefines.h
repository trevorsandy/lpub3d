#ifndef __TCDEFINES_H__
#define __TCDEFINES_H__

#include <string>

#define PNGDATA_1X 41
#define PNGDATA_2X 42

// MSVC SUCKS! We should not have to check the fricken MS compiler version
// before defining nullptr when __cplusplus <= 199711, but they don't give
// a sane __cplusplus, so we have to just ignore that and go by MSVC
// compiler version. Once again, MSVC SUCKS!
#if defined(_MSC_VER)
#  if _MSC_VER < 1800 // 1800 is Visual Studio 2013.
#    define nullptr NULL
#  endif
#elif __cplusplus <= 199711L
#  define nullptr NULL
#endif

#ifdef COCOA
// The following is necessary to get rid of some truly screwed up warnings that
// show up when compiling on the Mac.
#pragma GCC visibility push(default)
#endif // COCOA

#ifdef WIN32

#define RT_PNGDATA_1X MAKEINTRESOURCE(PNGDATA_1X)
#define RT_PNGDATA_2X MAKEINTRESOURCE(PNGDATA_2X)

// The following shouldn't be necessary here, but due to bugs in Microsoft's
// precompiled headers, it is.  The warning being disabled below is the one
// that warns about identifiers longer than 255 characters being truncated to
// 255 characters in the debug info.  (Also, "not inlined", and one other.)
#pragma warning(disable : 4786 4702 4710)

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#define _CRT_NONSTDC_NO_DEPRECATE

#ifdef _WIN32_WINDOWS
#undef _WIN32_WINDOWS
#endif

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#define _WIN32_WINDOWS 0x0600
#define _WIN32_WINNT 0x0600

#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
   #define DEBUG_CLIENTBLOCK   new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif // _DEBUG

#include <winsock2.h>
#include <crtdbg.h>

#undef min
#undef max

#pragma warning( disable : 4514 4127 )

/*** LPub3D Mod -
  This is automatically defined when building a static library. When
  including library in project, _TC_STATIC should must be
  explicitly defined to avoid troubles with unnecessary importing/exporting.
/*** LPub3D Mod end ***/

#ifdef _BUILDING_TCFOUNDATION
#define TCExport __declspec(dllexport)
#elif defined _BUILDING_TCFOUNDATION_LIB || defined _TC_STATIC
#define TCExport
#else
#define TCExport __declspec(dllimport)
#endif

#else // WIN32

#define TCExport

#endif // WIN32

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#ifdef __APPLE__
#endif // __APPLE__

#define START_IGNORE_DEPRECATION \
_Pragma("clang diagnostic push") \
_Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")

#define END_IGNORE_DEPRECATION _Pragma("clang diagnostic pop")

#if defined (__APPLE__) || defined (_OSMESA)
#include <wchar.h>
#endif // __APPLE__ || _OSMESA

// NOTE: the following should be 1, 2, and 4 bytes each.  So on a 64-bit system,
// the following defines aren't appropriate, and something else needs to be
// substituted.

typedef unsigned char TCByte;
typedef unsigned char TCUChar;
typedef unsigned short TCUShort;
typedef unsigned int TCUInt;
#ifdef __U32_TYPE
typedef __U32_TYPE TCULong;
#else // __U32_TYPE
// I know the following seems wrong, but int should always be 32-bit.
typedef unsigned int TCULong;
#endif // __U32_TYPE
typedef char TCChar;
typedef short TCShort;
typedef int TCInt;
// I know the following seems wrong, but int should always be 32-bit.
typedef int TCLong;

// Define LDVIEW_DOUBLES to have LDView use doubles instead of floats.  Comment
// out the definition for floats.
//#define LDVIEW_DOUBLES

// I'm not sure if floats are 64 bits on a 64-bit system or not.  I know that
// TCFloat has to be 32 bits when LDVIEW_DOUBLES isn't defined in order for it
// to work.
#ifdef LDVIEW_DOUBLES
typedef double TCFloat;
#else // LDVIEW_DOUBLES
typedef float TCFloat;
#endif // LDVIEW_DOUBLES

// The following must always be defined to 32 bits.
typedef float TCFloat32;

#ifndef __THROW
#define __THROW
#endif //__THROW

//#define TC_NO_UNICODE

#ifdef TC_NO_UNICODE
typedef char UCCHAR;
typedef char * UCSTR;
typedef const char * CUCSTR;
typedef std::string UCSTRING;
#define _UC(x) x
#else // TC_NO_UNICODE
typedef wchar_t UCCHAR;
typedef wchar_t * UCSTR;
typedef const wchar_t * CUCSTR;
typedef std::wstring UCSTRING;
#define _UC(x) L ## x
#endif // TC_NO_UNICODE

#endif // __TCDEFINES_H__
