#ifndef IMGMAP_H
#define IMGMAP_H

#include "map.h"
#include "projection.h"
#include "transform.h"
#include "IMG/mapdata.h"


class IMGMap : public Map
{
	Q_OBJECT

public:
	IMGMap(const QString &fileName, QObject *parent = 0);
	~IMGMap() {qDeleteAll(_data);}

	QString name() const {return _data.first()->name();}

	QRectF bounds() {return _bounds;}
	RectC llBounds() {return _dataBounds;}

	int zoom() const {return _zoom;}
	void setZoom(int zoom);
	int zoomFit(const QSize &, const RectC &);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c)
	  {return _transform.proj2img(_projection.ll2xy(c));}
	Coordinates xy2ll(const QPointF &p)
	  {return _projection.xy2ll(_transform.img2proj(p));}

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void setOutputProjection(const Projection &projection);

	void load();
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private:
	Transform transform(int zoom) const;
	void updateTransform();

	QList<IMG::MapData *> _data;
	int _zoom;
	Projection _projection;
	Transform _transform;
	QRectF _bounds;
	RectC _dataBounds;

	bool _valid;
	QString _errorString;
};

#endif // IMGMAP_H
