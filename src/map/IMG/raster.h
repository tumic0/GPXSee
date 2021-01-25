#ifndef RASTER_H
#define RASTER_H

#include <QRect>
#include <QByteArray>
#include <QDebug>
#include "common/rectc.h"
#include "common/garmin.h"

class Raster {
public:
	Raster() {}
	Raster(const QByteArray &img, const QRect &rect) : _img(img), _rect(rect) {}

	const QByteArray &img() const {return _img;}
	const RectC rect() const
	{
		return RectC(Coordinates(toWGS32(_rect.left()), toWGS32(_rect.top())),
		  Coordinates(toWGS32(_rect.right()), toWGS32(_rect.bottom())));
	}
	bool isValid() const {return !_img.isNull();}

private:
	QByteArray _img;
	QRect _rect;
};

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const Raster &raster)
{
	dbg.nospace() << "Raster(" << raster.img().size() << ", " << raster.rect()
	  << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // RASTER_H
