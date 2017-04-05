#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <QGraphicsView>
#include <QVector>
#include <QHash>
#include <QList>
#include "units.h"
#include "palette.h"
#include "waypoint.h"

class Data;
class POI;
class Map;
class Track;
class Route;
class TrackItem;
class RouteItem;
class WaypointItem;
class ScaleItem;
class PathItem;

class PathView : public QGraphicsView
{
	Q_OBJECT

public:
	PathView(Map *map, POI *poi, QWidget *parent = 0);
	~PathView();

	QList<PathItem*> loadData(const Data &data);

	void setPalette(const Palette &palette);
	void setPOI(POI *poi);
	void setMap(Map *map);
	void setUnits(enum Units units);

	void plot(QPainter *painter, const QRectF &target);

	int trackCount() const {return _tracks.count();}
	int routeCount() const {return _routes.count();}
	int waypointCount() const {return _waypoints.count();}

	void clear();

	void useOpenGL(bool use);

public slots:
	void redraw();

	void showMap(bool show);
	void showPOI(bool show);
	void setPOIOverlap(bool overlap);
	void showWaypointLabels(bool show);
	void showPOILabels(bool show);
	void showTracks(bool show);
	void showRoutes(bool show);
	void showWaypoints(bool show);
	void showRouteWaypoints(bool show);
	void setTrackWidth(int width);
	void setRouteWidth(int width);
	void setTrackStyle(Qt::PenStyle style);
	void setRouteStyle(Qt::PenStyle style);

private slots:
	void updatePOI();

private:
	PathItem *addTrack(const Track &track);
	PathItem *addRoute(const Route &route);
	void addWaypoints(const QList<Waypoint> &waypoints);
	void addPOI(const QVector<Waypoint> &waypoints);
	void loadPOI();
	void clearPOI();

	qreal mapScale() const;
	QPointF contentCenter() const;
	void rescale();
	void zoom(int zoom, const QPoint &pos, const Coordinates &c);
	void digitalZoom(int zoom);
	void resetDigitalZoom();
	void updatePOIVisibility();
	void updateWaypointsBoundingRect(const QPointF &wp);

	void mouseDoubleClickEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void drawBackground(QPainter *painter, const QRectF &rect);
	void resizeEvent(QResizeEvent *event);
	void paintEvent(QPaintEvent *event);
	void scrollContentsBy(int dx, int dy);

	QGraphicsScene *_scene;
	ScaleItem *_mapScale;
	QList<TrackItem*> _tracks;
	QList<RouteItem*> _routes;
	QList<WaypointItem*> _waypoints;
	QHash<Waypoint, WaypointItem*> _pois;

	QRectF _tr, _rr, _wr;
	QPointF _wp;
	qreal _res;

	Map *_map;
	POI *_poi;
	Palette _palette;
	Units _units;

	bool _showMap;
	bool _showTracks;
	bool _showRoutes;
	bool _showWaypoints;
	bool _showWaypointLabels;
	bool _showPOI;
	bool _showPOILabels;
	bool _overlapPOIs;
	bool _showRouteWaypoints;
	int _trackWidth;
	int _routeWidth;
	Qt::PenStyle _trackStyle;
	Qt::PenStyle _routeStyle;

	int _digitalZoom;
	bool _plot;
};

#endif // TRACKVIEW_H
