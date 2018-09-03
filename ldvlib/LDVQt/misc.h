#ifndef __MISC_H__
#define __MISC_H__

#include <string.h>
#ifdef _AIX
#include <strings.h>
#endif

#include <QtCore/qstring.h>
#include <TCFoundation/mystring.h>

void wcstoqstring(QString &dst, const wchar_t *src, int length = -1);
QString wcstoqstring(const wchar_t *src, int length = -1);
void wstringtoqstring(QString &dst, const std::wstring &src);
void ucstringtoqstring(QString &dst, const ucstring &src);
void qstringtoucstring(ucstring &dst, const QString &src);

#endif
