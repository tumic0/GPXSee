#ifndef PATHITEM_H
#define PATHITEM_H

#include <QPen>
#include <QTimeZone>
#include "data/path.h"
#include "graphicsscene.h"
#include "markerinfoitem.h"
#include "format.h"
#include "units.h"

class Map;
class PathTickItem;
class GraphItem;
class MarkerItem;

class PathItem : public QObject, public GraphicsItem
{
	Q_OBJECT

public:
	PathItem(const Path &path, Map *map, QGraphicsItem *parent = 0);
	virtual ~PathItem() {}

	QPainterPath shape() const {return _shape;}
	QRectF boundingRect() const {return _shape.boundingRect();}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	virtual QDateTime date() const = 0;

	const Path &path() const {return _path;}

	void addGraph(GraphItem *graph);

	void setMap(Map *map);
	void setGraph(int index);

	void setColor(const QColor &color);
	void setWidth(qreal width);
	void setPenStyle(Qt::PenStyle style);
	void setDigitalZoom(int zoom);
	void setMarkerColor(const QColor &color);
	void showMarker(bool show);
	void showMarkerInfo(MarkerInfoItem::Type type);
	void showTicks(bool show);

	void setMarkerPosition(qreal pos);

	void updateTicks();
	void updateMarkerInfo();
	void updateStyle();

	static void setUnits(Units units) {_units = units;}
	static void setTimeZone(const QTimeZone &zone) {_timeZone = zone;}

public slots:
	void hover(bool hover);

signals:
	void selected(bool);

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);

	static Units _units;
	static QTimeZone _timeZone;

private:
	const PathSegment *segment(qreal x) const;
	QPointF position(qreal distance) const;
	void updatePainterPath();
	void updateShape();
	void addSegment(const Coordinates &c1, const Coordinates &c2);
	void setMarkerInfo(qreal pos);
	void updateColor();
	void updateWidth();
	void updatePenStyle();
	qreal width() const;
	const QColor &color() const;
	Qt::PenStyle penStyle() const;

	qreal xInM() const;
	unsigned tickSize() const;

	Path _path;

	Map *_map;
	QList<GraphItem *> _graphs;
	GraphItem *_graph;
	MarkerItem *_marker;
	MarkerInfoItem *_markerInfo;
	QVector<PathTickItem*> _ticks;

	QPen _pen;
	QPainterPath _shape;
	QPainterPath _painterPath;

	qreal _width;
	QColor _color;
	Qt::PenStyle _penStyle;
	bool _showMarker;
	bool _showTicks;
	MarkerInfoItem::Type _markerInfoType;
	qreal _markerDistance;
	int _digitalZoom;
};

#endif // PATHITEM_H
