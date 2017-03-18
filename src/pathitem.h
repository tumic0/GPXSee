#ifndef PATHITEM_H
#define PATHITEM_H

#include <QGraphicsObject>
#include <QPen>
#include "markeritem.h"
#include "path.h"


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

	void setColor(const QColor &color);
	void setWidth(int width);
	void setStyle(Qt::PenStyle style);

	void showMarker(bool show) {_marker->setVisible(show);}

public slots:
	void moveMarker(qreal distance);

signals:
	void selected(bool);

protected:
	Path _path;
	MarkerItem *_marker;

private:
	QPointF position(qreal distance) const;
	void updatePainterPath(Map *map);
	void updateShape();

	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

	Map *_map;
	qreal _md;

	int _width;
	QPen _pen;
	QPainterPath _shape;
	QPainterPath _painterPath;
};

#endif // PATHITEM_H
