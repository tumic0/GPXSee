#ifndef ENCMAP_H
#define ENCMAP_H

#include "map.h"
#include "projection.h"
#include "transform.h"
#include "ENC/mapdata.h"

class QFile;

class ENCMap : public Map
{
	Q_OBJECT

public:
	ENCMap(const QString &fileName, QObject *parent = 0);

	QString name() const {return _data.name();}

	QRectF bounds() {return _bounds;}
	RectC llBounds() {return _llBounds;}

	int zoom() const {return _zoom;}
	void setZoom(int zoom);
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	void load();
	void unload();
	void setOutputProjection(const Projection &projection);
	void setDevicePixelRatio(qreal deviceRatio, qreal mapRatio);

	QPointF ll2xy(const Coordinates &c)
	  {return _transform.proj2img(_projection.ll2xy(c));}
	Coordinates xy2ll(const QPointF &p)
	  {return _projection.xy2ll(_transform.img2proj(p));}

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	bool isValid() const {return _data.isValid();}
	QString errorString() const {return _data.errorString();}

	static Map *create(const QString &path, const Projection &, bool *isMap);

private:
	Transform transform(int zoom) const;
	void updateTransform();
	QString key(int zoom, const QPoint &xy) const;

	ENC::MapData _data;
	Projection _projection;
	Transform _transform;
	qreal _tileRatio;
	RectC _llBounds;
	QRectF _bounds;
	int _zoom;

	bool _valid;
	QString _errorString;
};

#endif // ENCMAP_H
