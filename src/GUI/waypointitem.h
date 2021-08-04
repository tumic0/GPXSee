#ifndef WAYPOINTITEM_H
#define WAYPOINTITEM_H

#include <cmath>
#include <QGraphicsItem>
#include <QFont>
#include <QTimeZone>
#include "data/waypoint.h"
#include "map/map.h"
#include "units.h"
#include "graphicsscene.h"
#include "format.h"


class WaypointItem : public GraphicsItem
{
public:
	WaypointItem(const Waypoint &waypoint, Map *map, QGraphicsItem *parent = 0);

	const Waypoint &waypoint() const {return _waypoint;}

	void setMap(Map *map) {setPos(map->ll2xy(_waypoint.coordinates()));}
	void setSize(int size);
	void setColor(const QColor &color);
	void showLabel(bool show);
	void setDigitalZoom(int zoom) {setScale(pow(2, -zoom));}

	QPainterPath shape() const {return _shape;}
	QRectF boundingRect() const {return _shape.boundingRect();}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	ToolTip info() const;

	static void setUnits(Units units) {_units = units;}
	static void setCoordinatesFormat(CoordinatesFormat format)
	  {_format = format;}
	static void setTimeZone(const QTimeZone &zone) {_timeZone = zone;}

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
	void updateCache();

	Waypoint _waypoint;
	QPainterPath _shape;
	QColor _color;
	int _size;
	bool _showLabel;
	QFont _font;
	QRect _labelBB;

	static Units _units;
	static CoordinatesFormat _format;
	static QTimeZone _timeZone;
};

#endif // WAYPOINTITEM_H
