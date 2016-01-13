#ifndef TRACK_H
#define TRACK_H

#include <QGraphicsView>
#include <QVector>
#include <QHash>
#include <QList>
#include <QPrinter>
#include "poi.h"
#include "gpx.h"
#include "map.h"
#include "units.h"
#include "colorshop.h"


class POIItem;
class MarkerItem;
class ScaleItem;

class Track : public QGraphicsView
{
	Q_OBJECT

public:
	Track(QWidget *parent = 0);
	~Track();

	void loadGPX(const GPX &gpx);

	void loadPOI(const POI &poi);
	void clearPOI();
	void clear();

	void setMap(Map *map);
	void setUnits(enum Units units);

	void plot(QPainter *painter, const QRectF &target);
	enum QPrinter::Orientation orientation() const;

public slots:
	void movePositionMarker(qreal val);

private slots:
	void redraw();

private:
	QRectF trackBoundingRect() const;
	qreal trackScale() const;
	qreal mapScale() const;
	void rescale(qreal scale);

	void showMarkers(bool show);
	void setTrackLineWidth(qreal width);

	void wheelEvent(QWheelEvent *event);
	void drawBackground(QPainter *painter, const QRectF &rect);
	void resizeEvent(QResizeEvent *e);
	void paintEvent(QPaintEvent *e);

	QGraphicsScene *_scene;
	QList<QVector<QPointF> > _tracks;
	QList<QGraphicsPathItem*> _trackPaths;
	QList<MarkerItem*> _markers;
	QHash<Entry, POIItem*> _pois;
	Map *_map;
	ScaleItem *_mapScale;

	ColorShop _colorShop;
	qreal _maxLen;

	qreal _scale;
	int _zoom;
};

#endif // TRACK_H
