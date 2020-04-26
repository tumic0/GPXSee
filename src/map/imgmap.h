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

	virtual int zoom() const {return _zoom;}
	virtual void setZoom(int zoom);
	virtual int zoomFit(const QSize &, const RectC &);
	virtual int zoomIn();
	virtual int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void setProjection(const Projection &projection);

	void load();
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private:
	void ll2xy(QList<MapData::Poly> &polys);
	void ll2xy(QList<MapData::Point> &points);
	Transform transform(int zoom) const;
	void updateTransform();

	QList<MapData *> _data;
	int _zoom;
	Projection _projection;
	Transform _transform;
	QRectF _bounds;
	RectC _dataBounds;

	bool _valid;
	QString _errorString;
};

#endif // IMGMAP_H
