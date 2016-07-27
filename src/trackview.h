#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <QGraphicsView>
#include <QVector>
#include <QHash>
#include <QList>
#include "units.h"
#include "palette.h"
#include "waypoint.h"

class GPX;
class POI;
class Map;
class Track;
class TrackItem;
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

	int trackCount() const {return _paths.count();}
	int waypointCount() const {return _waypoints.count();}

public slots:
	void movePositionMarker(qreal val);
	void redraw();

private:
	void addTrack(const Track &track);
	void addWaypoints(const QList<Waypoint> &waypoints);
	void addPOI(const QVector<Waypoint> &waypoints);

	QRectF trackBoundingRect() const;
	QRectF waypointBoundingRect() const;
	qreal trackScale() const;
	qreal waypointScale() const;
	qreal mapScale(int zoom) const;
	void rescale(qreal scale);
	void rescale();
	void zoom(int z, const QPointF &pos);

	void wheelEvent(QWheelEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void drawBackground(QPainter *painter, const QRectF &rect);
	void resizeEvent(QResizeEvent *e);
	void paintEvent(QPaintEvent *e);

	QGraphicsScene *_scene;
	QList<TrackItem*> _paths;
	QList<MarkerItem*> _markers;
	QList<WaypointItem*> _waypoints;
	QHash<Waypoint, WaypointItem*> _pois;

	Map *_map;
	ScaleItem *_mapScale;

	Palette _palette;
	qreal _maxPath, _maxDistance;

	qreal _scale;
	int _zoom;

	Units _units;
	bool _plot;
};

#endif // TRACKVIEW_H
