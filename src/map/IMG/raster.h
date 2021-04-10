#ifndef IMG_RASTER_H
#define IMG_RASTER_H

#include <QRect>
#include <QDebug>
#include "common/rectc.h"
#include "common/garmin.h"

namespace IMG {

class LBLFile;

class Raster {
public:
	Raster() : _lbl(0) {}
	Raster(const LBLFile *lbl, quint32 id, const QRect &rect)
	  : _lbl(lbl), _id(id), _rect(rect) {}

	const LBLFile *lbl() const {return _lbl;}
	quint32 id() const {return _id;}
	const RectC rect() const
	{
		return RectC(Coordinates(toWGS32(_rect.left()), toWGS32(_rect.top())),
		  Coordinates(toWGS32(_rect.right()), toWGS32(_rect.bottom())));
	}
	bool isValid() const {return (_lbl != 0);}

private:
	const LBLFile *_lbl;
	quint32 _id;
	QRect _rect;
};

}

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const IMG::Raster &raster)
{
	dbg.nospace() << "Raster(" << raster.rect() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // IMG_RASTER_H
