#ifndef HASH_H
#define HASH_H

#include <QtGlobal>
#include <QPoint>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define HASH_T uint
#else // QT6
#define HASH_T size_t
#endif // QT6

inline HASH_T qHash(const QPoint &p)
{
	return ::qHash(p.x()) ^ ::qHash(p.y());
}

#endif // HASH_H
