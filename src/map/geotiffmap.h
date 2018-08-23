#ifndef GEOTIFFMAP_H
#define GEOTIFFMAP_H

#include "transform.h"
#include "projection.h"
#include "map.h"

class GeoTIFFMap : public Map
{
	Q_OBJECT

public:
	GeoTIFFMap(const QString &fileName, QObject *parent = 0);
	~GeoTIFFMap();

	QString name() const;

	QRectF bounds();

	int zoom() const {return 0;}
	void setZoom(int) {}
	int zoomFit(const QSize &, const RectC &) {return 0;}
	int zoomIn() {return 0;}
	int zoomOut() {return 0;}

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, bool block);

	void setDevicePixelRatio(qreal ratio) {_ratio = ratio;}
	void setOpenGLEnabled(bool enabled) {_opengl = enabled;}
	void load();
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private:
	QString _path;
	Projection _projection;
	Transform _transform;
	QImage *_img;
	QSize _size;
	qreal _ratio;
	bool _opengl;

	bool _valid;
	QString _errorString;
};

#endif // GEOTIFFMAP_H
