#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <QGraphicsView>
#include <QVector>
#include <QHash>
#include <QList>
#include "units.h"
#include "palette.h"
#include "waypoint.h"
#include "rectc.h"
#include "searchpointer.h"
#include "geoitems/geoitems.h"

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
	PathView(GeoItems &geoItems, Map *map, POI *poi, QWidget *parent = 0);

	void setPOI(POI *poi);
	void setMap(Map *map);

	void plot(QPainter *painter, const QRectF &target, qreal scale, bool hires);

	int waypointCount() const {return _waypoints.count();}

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
signals:
	void digitalZoomChanged(int zoom);

public slots:
	void showMap(bool show);
	void showPOI(bool show);
	void setPOIOverlap(bool overlap);
	void showPOILabels(bool show);
	void clearMapCache();

private slots:
	void updatePOI();
	void reloadMap();

	void addTrackItem(const Track &t, TrackItem *track);
	void addRouteItem(const Route &r, RouteItem *route);
	void addWaypointItem(const Waypoint &w, WaypointItem *waypoints);
	void clear();
	void setUnits(enum Units units);

private:
	void addPOI(const QVector<Waypoint> &waypoints);
	void loadPOI();
	void clearPOI();

	qreal mapZoom() const;
	QPointF contentCenter() const;
	void rescale();
	void centerOn(const QPointF &pos);
	void zoom(int zoom, const QPoint &pos, const Coordinates &c);
	void digitalZoom(int zoom);
	void resetDigitalZoom();
	void updatePOIVisibility();
	void updateWaypointsBoundingRect(const Coordinates &wp);

	void mouseDoubleClickEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void drawBackground(QPainter *painter, const QRectF &rect);
	void resizeEvent(QResizeEvent *event);
	void paintEvent(QPaintEvent *event);
	void scrollContentsBy(int dx, int dy);

	QGraphicsScene *_scene;
	ScaleItem *_mapScale;
	QList<WaypointItem*> _waypoints;
	QHash<SearchPointer<Waypoint>, WaypointItem*> _pois;

	GeoItems &_geoItems;

	RectC _tr, _rr, _wr;
	qreal _res;

	Map *_map;
	POI *_poi;

	qreal _opacity;
	QColor _backgroundColor;
	bool _showMap;
	bool _showPOI;
	bool _showPOILabels;
	bool _overlapPOIs;
	bool _showRouteWaypoints;
	int _poiSize;
	QColor _poiColor;

	int _digitalZoom;
	bool _plot;
};

#endif // TRACKVIEW_H
