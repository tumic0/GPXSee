#ifndef IMGMAP_H
#define IMGMAP_H

#include "map.h"
#include "projection.h"
#include "transform.h"
#include "IMG/mapdata.h"

class TextItem;

class IMGMap : public Map
{
	Q_OBJECT

public:
	IMGMap(const QString &fileName, QObject *parent = 0);
	~IMGMap() {delete _data;}

	QString name() const {return _data->name();}

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

	void load();
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private:
	friend class RasterTile;

	Transform transform(int zoom) const;
	void updateTransform();
	void drawPolygons(QPainter *painter, const QList<MapData::Poly> &polygons);
	void drawLines(QPainter *painter, const QList<MapData::Poly> &lines);
	void drawTextItems(QPainter *painter, const QList<TextItem*> &textItems);

	void processPolygons(QList<MapData::Poly> &polygons,
	  QList<TextItem *> &textItems);
	void processLines(QList<MapData::Poly> &lines, const QRect &tileRect,
	  QList<TextItem*> &textItems);
	void processPoints(QList<MapData::Point> &points, QList<TextItem*> &textItems);
	void processShields(QList<MapData::Poly> &lines, const QRect &tileRect,
	  QList<TextItem*> &textItems);
	void processStreetNames(QList<MapData::Poly> &lines, const QRect &tileRect,
	  QList<TextItem*> &textItems);

	MapData *_data;
	int _zoom;
	Projection _projection;
	Transform _transform;

	bool _valid;
	QString _errorString;
};

#endif // IMGMAP_H
