#ifndef LPUB_QTCOMPAT_H
#define LPUB_QTCOMPAT_H

// These overrides are taken from lpub_qt_compat.h
// Credit: Sergey A. Tachenov, https://github.com/stachenov/quazip

#include <QtCore/Qt>
#include <QtCore/QtGlobal>

// Legacy encodings are still everywhere, but the Qt team decided we
// don't need them anymore and moved them out of Core in Qt 6.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#  include <QtCore5Compat/QTextCodec>
#else
#  include <QtCore/QTextCodec>
#endif

// this is yet another stupid move and deprecation
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
using Qt::SkipEmptyParts;
#else
#include <QtCore/QString>
const auto SkipEmptyParts = QString::SplitBehavior::SkipEmptyParts;
#endif

// and yet another... (why didn't they just make qSort delegate to std::sort?)
#include <QtCore/QList>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
#include <algorithm>
template<typename T>
inline void lpub_sort(T begin, T end) {
    std::sort(begin, end);
}
template<typename T, typename C>
inline void lpub_sort(T begin, T end, C comparator) {
    std::sort(begin, end, comparator);
}
#else
#include <QtCore/QtAlgorithms>
template<typename T>
inline void lpub_sort(T begin, T end) {
    qsort(begin, end);
}
template<typename T, typename C>
inline void lpub_sort(T begin, T end, C comparator) {
    qSort(begin, end, comparator);
}
#endif

// I'm not even sure what this one is, but nevertheless
#include <QtCore/QFileInfo>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
inline QString lpub_symlink_target(const QFileInfo &fi) {
    return fi.symLinkTarget();
}
#else
inline QString lpub_symlink_target(const QFileInfo &fi) {
    return fi.readLink(); // What's the difference? I've no idea.
}
#endif

#include <QtCore/QTextStream>
// and another stupid move
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
const auto lpub_endl = Qt::endl;
#else
const auto lpub_endl = endl;
#endif

#endif // LPUB_QTCOMPAT_H
