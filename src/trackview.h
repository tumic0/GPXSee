#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <QGraphicsView>
#include <QVector>
#include <QHash>
#include <QList>
#include <QPrinter>
#include "units.h"
#include "palette.h"
#include "waypoint.h"

class GPX;
class POI;
class Map;
class WaypointItem;
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

	int trackCount() const {return _paths.count();}
	int waypointCount() const {return _locations.count();}

public slots:
	void movePositionMarker(qreal val);

private slots:
	void redraw();

private:
	void addTrack(const QVector<QPointF> &track);
	void addWaypoints(const QList<Waypoint> &waypoints);
	void addPOI(const QVector<Waypoint> &waypoints);

	QRectF trackBoundingRect() const;
	QRectF waypointBoundingRect() const;
	qreal trackScale() const;
	qreal waypointScale() const;
	qreal mapScale(int zoom) const;
	void rescale(qreal scale);

	void showMarkers(bool show);
	void setTrackLineWidth(qreal width);

	void wheelEvent(QWheelEvent *event);
	void drawBackground(QPainter *painter, const QRectF &rect);
	void resizeEvent(QResizeEvent *e);
	void paintEvent(QPaintEvent *e);

	QGraphicsScene *_scene;
	QList<QGraphicsPathItem*> _paths;
	QList<MarkerItem*> _markers;
	QList<WaypointItem*> _locations;
	QHash<Waypoint, WaypointItem*> _pois;
	QList<QVector<QPointF> > _tracks;
	QVector<QPointF> _waypoints;

	Map *_map;
	ScaleItem *_mapScale;

	Palette _palette;
	qreal _maxPath, _maxDistance;

	qreal _scale;
	int _zoom;
};

#endif // TRACKVIEW_H
