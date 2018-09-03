#ifndef __TCSTLINCLUDES_H__
#define __TCSTLINCLUDES_H__

#if defined(WIN32) && !defined(_QT)

// In Windows, we have to disable a number of warnings in order to use any STL
// classes without getting tons of warnings.  The following warning is shut off
// completely; it's just the warning that identifiers longer than 255 characters
// will be truncated in the debug info.  I really don't care about this.  Note
// that the other warnings are only disabled during the #include of the STL
// headers due to the warning(push) and warning(pop).
#pragma warning(push, 1)	// Minimum warnings during STL includes
// Disable more warnings.  Note that they're reenable at the pop below.
// 4702: Unreachable code.
// 4786: Identifier truncated to 255 characters in debug info.
#pragma warning(disable: 4702 4786)	// Unreachable code warnings pop up also.
#ifndef _DEBUG
#pragma warning(disable: 4710) // "Not inlined" warnings in release mode.
#endif // _DEBUG
#endif // WIN32 && NOT _QT
#include <string>
#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include <stack>
#include <set>
#include <string.h>

#ifdef WIN32

#if defined(_MSC_VER)
#if _MSC_VER < 1400	// VC < VC 2005
#define strcasecmp stricmp
#define strncasecmp strnicmp
#define wcscasecmp wcsicmp
#define wcsncasecmp wcsnicmp
#else //  VC < VC 2005
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define wcscasecmp _wcsicmp
#define wcsncasecmp _wcsnicmp
#endif // VC < VC 2005
#endif

#ifndef USE_CPP11

#if defined(_MSC_VER) && _MSC_VER <= 1200

#ifdef _NO_BOOST

// MS VC++ 6.0 doesn't have the min/max STL functions.  The following from
// Boost gets them defined.
#define TC_PREVENT_MACRO_SUBSTITUTION

namespace std {
  template <class _Typ>
  inline const _Typ& min TC_PREVENT_MACRO_SUBSTITUTION (const _Typ& __a, const _Typ& __b) {
    return __b < __a ? __b : __a;
  }
  template <class _Typ>
  inline const _Typ& max TC_PREVENT_MACRO_SUBSTITUTION (const _Typ& __a, const _Typ& __b) {
    return  __a < __b ? __b : __a;
  }
}

#else // _NO_BOOST

// MS VC++ 6.0 doesn't have the min/max STL functions.  The following include from
// Boost gets them defined.
#include <boost/config.hpp>

#endif // !_NO_BOOST

#endif

#pragma warning(pop)
#endif // WIN32

#endif

#ifdef NO_WSTRING
// NOTE: on system without wstring, the std namespace isn't used.
typedef basic_string<wchar_t> wstring;
#endif

struct less_no_case
{
	bool operator()(const std::string& _Left, const std::string& _Right) const
	{
		return strcasecmp(_Left.c_str(), _Right.c_str()) < 0;
	}
};

#endif //__TCSTLINCLUDES_H__
