#ifndef TRACKITEM_H
#define TRACKITEM_H

#include <QDateTime>
#include <QPen>
#include "pathitem.h"
#include "units.h"
#include "track.h"
#include "markeritem.h"


class TrackItem : public PathItem
{
	Q_OBJECT

public:
	TrackItem(const Track &track, QGraphicsItem *parent = 0);

	QPainterPath shape() const {return _shape;}
	QRectF boundingRect() const {return _shape.boundingRect();}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	const QPainterPath &path() const {return _path;}

	void setScale(qreal scale);
	void setUnits(enum Units units);
	void setColor(const QColor &color);

	void showMarker(bool show) {_marker->setVisible(show);}
	void moveMarker(qreal distance);

private:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

	void updateShape();
	QString toolTip();

	QPainterPath _path;
	QPainterPath _shape;
	QPen _pen;

	MarkerItem *_marker;

	Units _units;
	QDateTime _date;
	qreal _time;
	qreal _distance;
};

#endif // TRACKITEM_H
