#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <QGraphicsView>
#include <QVector>
#include <QHash>
#include <QList>
#include <QPrinter>
#include "units.h"
#include "colorshop.h"
#include "waypoint.h"

class GPX;
class POI;
class Map;
class POIItem;
class MarkerItem;
class ScaleItem;

class TrackView : public QGraphicsView
{
	Q_OBJECT

public:
	TrackView(QWidget *parent = 0);
	~TrackView();

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
	QHash<WayPoint, POIItem*> _pois;
	Map *_map;
	ScaleItem *_mapScale;

	ColorShop _colorShop;
	qreal _maxLen;

	qreal _scale;
	int _zoom;
};

#endif // TRACKVIEW_H
