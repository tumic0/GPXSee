#ifndef PATHITEM_H
#define PATHITEM_H

#include <QGraphicsObject>
#include <QPen>
#include "data/path.h"
#include "markeritem.h"
#include "units.h"
#include "graphicsscene.h"

class Map;
class PathTickItem;

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

	const Path &path() const {return _path;}

	void setMap(Map *map);

	void setUnits(Units units);
	void setColor(const QColor &color);
	void setWidth(qreal width);
	void setStyle(Qt::PenStyle style);
	void setDigitalZoom(int zoom);
	void setMarkerColor(const QColor &color);
	void showMarker(bool show);
	void showTicks(bool show);

public slots:
	void moveMarker(qreal distance);
	void hover(bool hover);

signals:
	void selected(bool);

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);

	Units _units;

private:
	const PathSegment *segment(qreal x) const;
	QPointF position(qreal distance) const;
	void updatePainterPath();
	void updateShape();
	void addSegment(const Coordinates &c1, const Coordinates &c2);

	qreal xInM() const;
	unsigned tickSize() const;
	void updateTicks();

	Path _path;
	Map *_map;
	qreal _markerDistance;
	int _digitalZoom;

	qreal _width;
	QPen _pen;
	QPainterPath _shape;
	QPainterPath _painterPath;
	bool _showMarker;
	bool _showTicks;

	MarkerItem *_marker;
	QVector<PathTickItem*> _ticks;
};

#endif // PATHITEM_H
