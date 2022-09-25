#ifndef WAYPOINTITEM_H
#define WAYPOINTITEM_H

#include <cmath>
#include <QFont>
#include <QTimeZone>
#include "data/waypoint.h"
#include "map/map.h"
#include "format.h"
#include "graphicsscene.h"


class WaypointItem : public GraphicsItem
{
public:
	WaypointItem(const Waypoint &waypoint, Map *map, QGraphicsItem *parent = 0);

	const Waypoint &waypoint() const {return _waypoint;}

	void setMap(Map *map) {setPos(map->ll2xy(_waypoint.coordinates()));}
	void setSize(int size);
	void setColor(const QColor &color);
	void showLabel(bool show);
	void showIcon(bool show);
	void setDigitalZoom(int zoom) {setScale(pow(2, -zoom));}
	void updateStyle();

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
	void updateColor();
	void updateSize();
	int size() const;
	const QColor &color() const;

	Waypoint _waypoint;

	QColor _color;
	int _size;
	bool _showLabel;
	bool _showIcon;

	QFont _font;
	QRect _labelBB;
	const QPixmap *_icon;
	QPainterPath _shape;

	static Units _units;
	static CoordinatesFormat _format;
	static QTimeZone _timeZone;
};

#endif // WAYPOINTITEM_H
