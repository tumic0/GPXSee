#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsView>
#include <QVector>
#include <QHash>
#include <QList>
#include "common/rectc.h"
#include "data/waypoint.h"
#include "searchpointer.h"
#include "units.h"
#include "format.h"
#include "palette.h"

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

class MapView : public QGraphicsView
{
	Q_OBJECT

public:
	MapView(Map *map, POI *poi, QWidget *parent = 0);

	QList<PathItem*> loadData(const Data &data);

	void setPalette(const Palette &palette);
	void setPOI(POI *poi);
	void setMap(Map *map);

	void plot(QPainter *painter, const QRectF &target, qreal scale, bool hires);

	void clear();

	void setUnits(Units units);
	void setMarkerColor(const QColor &color);
	void setTrackWidth(int width);
	void setRouteWidth(int width);
	void setTrackStyle(Qt::PenStyle style);
	void setRouteStyle(Qt::PenStyle style);
	void setWaypointSize(int size);
	void setWaypointColor(const QColor &color);
	void setPOISize(int size);
	void setPOIColor(const QColor &color);
	void setMapOpacity(int opacity);
	void setBackgroundColor(const QColor &color);
	void useOpenGL(bool use);
	void useAntiAliasing(bool use);

public slots:
	void showMap(bool show);
	void showPOI(bool show);
	void setPOIOverlap(bool overlap);
	void showWaypointLabels(bool show);
	void showPOILabels(bool show);
	void showTracks(bool show);
	void showRoutes(bool show);
	void showWaypoints(bool show);
	void showRouteWaypoints(bool show);
	void clearMapCache();
	void setCoordinatesFormat(CoordinatesFormat format);

private slots:
	void updatePOI();
	void reloadMap();

private:
	PathItem *addTrack(const Track &track);
	PathItem *addRoute(const Route &route);
	void addWaypoints(const QList<Waypoint> &waypoints);
	void addPOI(const QList<Waypoint> &waypoints);
	void loadPOI();
	void clearPOI();

	int mapZoom() const;
	QPointF contentCenter() const;
	void rescale();
	void centerOn(const QPointF &pos);
	void zoom(int zoom, const QPoint &pos, const Coordinates &c);
	void digitalZoom(int zoom);
	void updatePOIVisibility();

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
	QHash<SearchPointer<Waypoint>, WaypointItem*> _pois;

	RectC _tr, _rr, _wr;
	qreal _res;

	Map *_map;
	POI *_poi;
	Palette _palette;
	Units _units;
	CoordinatesFormat _coordinatesFormat;

	qreal _opacity;
	QColor _backgroundColor;
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
	int _waypointSize;
	int _poiSize;
	QColor _waypointColor;
	QColor _poiColor;
	QColor _markerColor;

	int _digitalZoom;
	bool _plot;
};

#endif // MAPVIEW_H
