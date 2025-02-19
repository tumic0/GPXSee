#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsView>
#include <QVector>
#include <QHash>
#include <QList>
#include <QFlags>
#include "common/rectc.h"
#include "data/waypoint.h"
#include "map/projection.h"
#include "searchpointer.h"
#include "units.h"
#include "format.h"
#include "markerinfoitem.h"
#include "palette.h"
#include "graphicsscene.h"


class QGeoPositionInfoSource;
class QGeoPositionInfo;
class QGestureEvent;
class QPinchGesture;
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
class PlaneItem;
class MapItem;
class Area;
class QTimeZone;
class MapAction;
class CrosshairItem;
class MotionInfoItem;

class MapView : public QGraphicsView
{
	Q_OBJECT

public:
	enum Flag {
		NoFlags = 0,
		HiRes = 1,
		Expand = 2
	};
	Q_DECLARE_FLAGS(Flags, Flag)

	enum Layer {
		NoLayers = 0,
		Raster = 1,
		Vector = 2
	};
	Q_DECLARE_FLAGS(Layers, Layer)

	MapView(Map *map, POI *poi, QWidget *parent = 0);

	QList<PathItem *> loadData(const Data &data);
	void loadMaps(const QList<MapAction*> &maps);
	void loadDEMs(const QList<Area> &dems);

	void setPalette(const Palette &palette);
	void setPOI(POI *poi);
	void setMap(Map *map);
	void setPositionSource(QGeoPositionInfoSource *source);
	void setGraph(int index);
	void showExtendedInfo(bool show) {_scene->showExtendedInfo(show);}

	void plot(QPainter *painter, const QRectF &target, qreal scale, Flags flags);

	void clear();

	void setUnits(Units units);
	void setMarkerColor(const QColor &color);
	void setCrosshairColor(const QColor &color);
	void setInfoColor(const QColor &color);
	void drawInfoBackground(bool draw);
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
	void setCoordinatesFormat(CoordinatesFormat format);
	void setTimeZone(const QTimeZone &zone);
	void setMapConfig(const Projection &in, const Projection &out, bool hidpi);
	void setDevicePixelRatio(qreal ratio);
	void clearMapCache();
	void fitContentToSize();

	RectC boundingRect() const;
	const Projection &inputProjection() const {return _inputProjection;}

#ifdef Q_OS_ANDROID
signals:
	void clicked(const QPoint &pos);
#endif // Q_OS_ANDROID

public slots:
	void showMap(bool show);
	void showPOI(bool show);
	void showPosition(bool show);
	void showPOILabels(bool show);
	void showPOIIcons(bool show);
	void showCursorCoordinates(bool show);
	void showPositionCoordinates(bool show);
	void showTicks(bool show);
	void showMarkers(bool show);
	void showMarkerInfo(MarkerInfoItem::Type type);
	void showOverlappedPOIs(bool show);
	void showWaypointLabels(bool show);
	void showWaypointIcons(bool show);
	void showTracks(bool show);
	void showRoutes(bool show);
	void showAreas(bool show);
	void showWaypoints(bool show);
	void showRouteWaypoints(bool show);
	void setMarkerPosition(qreal pos);
	void followPosition(bool follow);
	void showMotionInfo(bool show);
	void useStyles(bool use);
	void drawHillShading(bool draw);
	void selectLayers(MapView::Layers layers);

private slots:
	void updatePOI();
	void reloadMap();
	void updatePosition(const QGeoPositionInfo &pos);

private:
	typedef QHash<SearchPointer<Waypoint>, WaypointItem*> POIHash;

	PathItem *addTrack(const Track &track);
	PathItem *addRoute(const Route &route);
	MapItem *addMap(MapAction *map);
	void addArea(const Area &area);
	void addWaypoints(const QVector<Waypoint> &waypoints);
	void addPOI(const QList<Waypoint> &waypoints);
	void loadPOI();
	void clearPOI();

	int fitMapZoom() const;
	QPointF contentCenter() const;
	void rescale();
	void centerOn(const QPointF &pos);
	void zoom(int zoom, const QPoint &pos, bool shift);
	void digitalZoom(int zoom);
	void updatePOIVisibility();
	bool gestureEvent(QGestureEvent *event);
	void pinchGesture(QPinchGesture *gesture);
	void skipColor() {_palette.nextColor();}
	void setHidpi(bool hidpi);

	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	void drawBackground(QPainter *painter, const QRectF &rect);
	void paintEvent(QPaintEvent *event);
	void leaveEvent(QEvent *event);

	bool event(QEvent *event);
	void scrollContentsBy(int dx, int dy);

	GraphicsScene *_scene;
	ScaleItem *_mapScale;
	CoordinatesItem *_cursorCoordinates, *_positionCoordinates;
	CrosshairItem *_crosshair;
	MotionInfoItem *_motionInfo;
	QList<TrackItem*> _tracks;
	QList<RouteItem*> _routes;
	QList<WaypointItem*> _waypoints;
	QList<PlaneItem*> _areas;
	POIHash _pois;

	RectC _tr, _rr, _wr, _ar;
	qreal _res;

	Map *_map;
	POI *_poi;
	QGeoPositionInfoSource *_positionSource;

	Palette _palette;
	qreal _mapOpacity;
	Projection _outputProjection, _inputProjection;

	bool _showMap, _showTracks, _showRoutes, _showAreas, _showWaypoints,
	  _showWaypointLabels, _showPOI, _showPOILabels, _showRouteWaypoints,
	  _showMarkers, _showPathTicks, _showPOIIcons, _showWaypointIcons,
	  _showPosition, _showPositionCoordinates, _showMotionInfo;
	MarkerInfoItem::Type _markerInfoType;
	bool _overlapPOIs, _followPosition;
	int _trackWidth, _routeWidth, _areaWidth;
	Qt::PenStyle _trackStyle, _routeStyle, _areaStyle;
	int _waypointSize, _poiSize;
	QColor _backgroundColor, _waypointColor, _poiColor, _markerColor;
	qreal _areaOpacity;
	bool _infoBackground;

	bool _hillShading;
	Layers _layers;

	int _digitalZoom;
	bool _plot;
	QCursor _cursor;

	qreal _deviceRatio;
	bool _hidpi;
	bool _opengl;

	int _pinchZoom;
	int _wheelDelta;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MapView::Layers)

#endif // MAPVIEW_H
