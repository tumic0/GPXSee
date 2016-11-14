#ifndef PATHITEM_H
#define PATHITEM_H

#include <QGraphicsObject>
#include <QPen>
#include "markeritem.h"
#include "units.h"


class PathItem : public QGraphicsObject
{
	Q_OBJECT

public:
	PathItem(QGraphicsItem *parent = 0);

	QPainterPath shape() const {return _shape;}
	QRectF boundingRect() const {return _shape.boundingRect();}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	const QPainterPath &path() const {return _path;}
	void showMarker(bool show) {_marker->setVisible(show);}
	void setScale(qreal scale);
	void setUnits(enum Units units);
	void setColor(const QColor &color);

public slots:
	void moveMarker(qreal distance);

signals:
	void selected(bool);

protected:
	void updateShape();

	QVector<qreal> _distance;
	QPainterPath _path;
	QPainterPath _shape;
	QPen _pen;

	MarkerItem *_marker;

	Units _units;

private:
	QPointF position(qreal distance) const;

	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
};

#endif // PATHITEM_H
