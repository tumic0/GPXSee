#ifndef IMGMAP_H
#define IMGMAP_H

#include <QFont>
#include "map.h"
#include "projection.h"
#include "transform.h"
#include "common/range.h"
#include "IMG/img.h"


class IMGMap : public Map
{
	Q_OBJECT

public:
	IMGMap(const QString &fileName, QObject *parent = 0);

	QString name() const {return _img.name();}

	QRectF bounds();

	virtual int zoom() const {return _zoom;}
	virtual void setZoom(int zoom);
	virtual int zoomFit(const QSize &, const RectC &);
	virtual int zoomIn();
	virtual int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void setProjection(const Projection &projection);

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private:
	friend class RasterTile;

	Transform transform(int zoom) const;
	void updateTransform();
	void drawPolygons(QPainter *painter, QList<IMG::Poly> &polygons);
	void drawLines(QPainter *painter, QList<IMG::Poly> &lines,
	  const QPoint &tile);
	void drawPoints(QPainter *painter, QList<IMG::Point> &points);

	QString _fileName;
	IMG _img;
	int _zoom;
	Range _zooms;
	Projection _projection;
	Transform _transform;

	QFont _largeFont, _normalFont, _smallFont, _poiFont;

	bool _valid;
	QString _errorString;
};

#endif // IMGMAP_H
