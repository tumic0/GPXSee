#ifndef IMG_ZOOM_H
#define IMG_ZOOM_H

#include <QDebug>
#include "common/hash.h"

namespace IMG {

class Zoom
{
public:
	Zoom() : _level(0), _bits(0) {}
	Zoom(quint8 level, quint8 bits) : _level(level), _bits(bits) {}

	quint8 level() const {return _level;}
	quint8 bits() const {return _bits;}

	bool operator<(const Zoom &other) const
	{
		return (_bits == other.bits())
		  ? _level < other._level
		  : _bits < other._bits;
	}
	bool operator==(const Zoom &other) const
	{
		return _level == other._level && _bits == other._bits;
	}

private:
	quint8 _level;
	quint8 _bits;
};

inline HASH_T qHash(const Zoom &zoom)
{
	return ::qHash(zoom.level()) ^ ::qHash(zoom.bits());
}

}

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const IMG::Zoom &zoom)
{
	dbg.nospace() << "Zoom(" << zoom.bits() << ", " << zoom.level() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // IMG_ZOOM_H
