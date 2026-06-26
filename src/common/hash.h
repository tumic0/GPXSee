#ifndef HASH_H
#define HASH_H

#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QPoint>
#include <qhashfunctions.h>

#define HASH_T uint

template <typename T1, typename T2>
uint qHashMulti(uint seed, const T1 &key1, const T2 &key2)
{
	QtPrivate::QHashCombine hash;
	seed = hash(seed, key1);
	seed = hash(seed, key2);
	return seed;
}

template <typename T1, typename T2, typename T3>
uint qHashMulti(uint seed, const T1 &key1, const T2 &key2, const T3 &key3)
{
	QtPrivate::QHashCombine hash;
	seed = hash(seed, key1);
	seed = hash(seed, key2);
	seed = hash(seed, key3);
	return seed;
}

inline uint qHash(const QPoint &p, uint seed = 0)
{
	return qHashMulti(seed, p.x(), p.y());
}
#else // QT6
#define HASH_T size_t
#endif // QT6

#endif // HASH_H
