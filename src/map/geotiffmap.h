#ifndef GEOTIFFMAP_H
#define GEOTIFFMAP_H

#include "transform.h"
#include "projection.h"
#include "map.h"

class Image;

class GeoTIFFMap : public Map
{
	Q_OBJECT

public:
	GeoTIFFMap(const QString &fileName, QObject *parent = 0);
	~GeoTIFFMap();

	QString name() const;

	QRectF bounds();
	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void load();
	void unload();
	void setDevicePixelRatio(qreal ratio);

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private:
	QString _fileName;
	Projection _projection;
	Transform _transform;
	Image *_img;
	QSize _size;
	qreal _ratio;

	bool _valid;
	QString _errorString;
};

#endif // GEOTIFFMAP_H
