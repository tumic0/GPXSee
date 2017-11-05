#ifndef PATHITEM_H
#define PATHITEM_H

#include <QGraphicsObject>
#include <QPen>
#include "markeritem.h"
#include "path.h"
#include "units.h"


class Map;

class PathItem : public QGraphicsObject
{
	Q_OBJECT

public:
	PathItem(const Path &path, Map *map, QGraphicsItem *parent = 0);

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

public slots:
	void moveMarker(qreal distance);
	void hover(bool hover);

signals:
	void selected(bool);

protected:
	Path _path;
	MarkerItem *_marker;

	QString _name;
	QString _desc;

private:
	virtual QString toolTip(Units units) const = 0;

	QPointF position(qreal distance) const;
	void updatePainterPath(Map *map);
	void updateShape();

	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

	Map *_map;
	qreal _markerDistance;
	int _digitalZoom;

	qreal _width;
	QPen _pen;
	QPainterPath _shape;
	QPainterPath _painterPath;
};

#endif // PATHITEM_H
