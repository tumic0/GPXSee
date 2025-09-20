#ifndef IMG_RASTER_H
#define IMG_RASTER_H

#include <QRect>
#include <QByteArray>
#include <QDebug>
#include "common/rectc.h"
#include "common/garmin.h"

namespace IMG {

class LBLFile;

class Raster {
public:
	Raster() {}
	Raster(const QByteArray &img, const QRect &rect)
	  : _img(img), _rect(rect) {}

	const QByteArray &img() const {return _img;}
	const RectC rect() const
	{
		return RectC(Coordinates(Garmin::toWGS32(_rect.left()),
		  Garmin::toWGS32(_rect.top())), Coordinates(
		  Garmin::toWGS32(_rect.right()), Garmin::toWGS32(_rect.bottom())));
	}
	bool isValid() const {return !_img.isNull();}

private:
	QByteArray _img;
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
