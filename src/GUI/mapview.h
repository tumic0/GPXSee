#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsView>
#include <QVector>
#include <QHash>
#include <QList>
#include "common/rectc.h"
#include "common/config.h"
#include "data/waypoint.h"
#include "data/polygon.h"
#include "map/projection.h"
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
class CoordinatesItem;
class PathItem;
class GraphItem;
class AreaItem;
class Area;
class GraphicsScene;

class MapView : public QGraphicsView
{
	Q_OBJECT

public:
	MapView(Map *map, POI *poi, QWidget *parent = 0);

	QList<PathItem *> loadData(const Data &data);

	void setPalette(const Palette &palette);
	void setPOI(POI *poi);
	void setMap(Map *map);

	void plot(QPainter *painter, const QRectF &target, qreal scale, bool hires);

	void clear();

	void setUnits(Units units);
	void setMarkerColor(const QColor &color);
	void setTrackWidth(int width);
	void setRouteWidth(int width);
	void setAreaWidth(int width);
	void setTrackStyle(Qt::PenStyle style);
	void setRouteStyle(Qt::PenStyle style);
	void setAreaStyle(Qt::PenStyle style);
	void setAreaOpacity(int opacity);
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
	void showAreas(bool show);
	void showWaypoints(bool show);
	void showRouteWaypoints(bool show);
	void showMarkers(bool show);
	void showCoordinates(bool show);
	void showTicks(bool show);
	void clearMapCache();
	void setCoordinatesFormat(CoordinatesFormat format);
	void setDevicePixelRatio(qreal deviceRatio, qreal mapRatio);
	void setProjection(int id);

	void fitContentToSize();

private slots:
	void updatePOI();
	void reloadMap();

private:
	typedef QHash<SearchPointer<Waypoint>, WaypointItem*> POIHash;

	PathItem *addTrack(const Track &track);
	PathItem *addRoute(const Route &route);
	void addArea(const Area &area);
	void addWaypoints(const QVector<Waypoint> &waypoints);
	void addPOI(const QList<Waypoint> &waypoints);
	void loadPOI();
	void clearPOI();

	int fitMapZoom() const;
	QPointF contentCenter() const;
	void rescale();
	void centerOn(const QPointF &pos);
	void zoom(int zoom, const QPoint &pos);
	void digitalZoom(int zoom);
	void updatePOIVisibility();
	void skipColor() {_palette.nextColor();}

	void mouseDoubleClickEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void drawBackground(QPainter *painter, const QRectF &rect);
	void paintEvent(QPaintEvent *event);
	void scrollContentsBy(int dx, int dy);
	void mouseMoveEvent(QMouseEvent *event);
	void leaveEvent(QEvent *event);

	GraphicsScene *_scene;
	ScaleItem *_mapScale;
	CoordinatesItem *_coordinates;
	QList<TrackItem*> _tracks;
	QList<RouteItem*> _routes;
	QList<WaypointItem*> _waypoints;
	QList<AreaItem*> _areas;
	POIHash _pois;

	RectC _tr, _rr, _wr, _ar;
	qreal _res;

	Map *_map;
	POI *_poi;

	Palette _palette;
	Units _units;
	CoordinatesFormat _coordinatesFormat;
	qreal _mapOpacity;
	Projection _projection;

	bool _showMap, _showTracks, _showRoutes, _showAreas, _showWaypoints,
	  _showWaypointLabels, _showPOI, _showPOILabels, _showRouteWaypoints,
	  _showMarkers, _showPathTicks;
	bool _overlapPOIs;
	int _trackWidth, _routeWidth, _areaWidth;
	Qt::PenStyle _trackStyle, _routeStyle, _areaStyle;
	int _waypointSize, _poiSize;
	QColor _backgroundColor, _waypointColor, _poiColor, _markerColor;
	qreal _areaOpacity;

	int _digitalZoom;
	bool _plot;

#ifdef ENABLE_HIDPI
	qreal _deviceRatio;
	qreal _mapRatio;
#endif // ENABLE_HIDPI
	bool _opengl;
};

#endif // MAPVIEW_H
