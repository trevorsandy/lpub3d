/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LDRAWINI_GLOBAL_H
#define LDRAWINI_GLOBAL_H

/**
This is automatically defined when building a static library, but when
including ldrawini sources directly into a project, LDRAWINI_STATIC should
be defined explicitly to avoid possible troubles with unnecessary
importing/exporting.
*/
#ifdef LDRAWINI_STATIC
#define LDINIEXPORT
#else
/**
* When building a DLL with MSVC, LDRAWINI_BUILD must be defined.
* qglobal.h takes care of defining Q_DECL_* correctly for msvc/gcc.
*/
#if defined(LDRAWINI_BUILD)
#define LDINIEXPORT __declspec(dllexport)
#else
#define LDINIEXPORT __declspec(dllimport)
#endif
#endif // LDRAWINI_STATIC

/* Hide curly brackets {} in defines to avoid Beautifier indenting whole file */
#ifdef __cplusplus
#define LDRAWINI_BEGIN_STDC extern "C" { // only need to export C interface if
										 // used by C++ source code
#define LDRAWINI_END_STDC   }
typedef bool LDrawIniBoolT;
#else
#define LDRAWINI_BEGIN_STDC
#define LDRAWINI_END_STDC
typedef char LDrawIniBoolT;
#ifndef false
#define false 0
#endif
#ifndef true
#define true (!false)
#endif
#endif

#ifdef __GNUC__
#define UNUSED __attribute__((__unused__))
#else
#define UNUSED
#endif

#define LDRAWINI_EXTRA_NTFS_MAGIC 0x000Au
#define LDRAWINI_EXTRA_NTFS_TIME_MAGIC 0x0001u

#endif // LDRAWINI_GLOBAL_H