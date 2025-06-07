#ifndef MAPITEM_H
#define MAPITEM_H

#include "planeitem.h"

class MapAction;
class Projection;

class MapItem : public PlaneItem
{
	Q_OBJECT

public:
	MapItem(MapAction *action, Map *map, GraphicsItem *parent = 0);

	QPainterPath shape() const {return _painterPath;}
	QRectF boundingRect() const {return _painterPath.boundingRect();}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	RectC bounds() const {return _bounds;}
	void setMap(Map *map);

	void setColor(const QColor &color);
	void setOpacity(qreal opacity);
	void setWidth(qreal width);
	void setPenStyle(Qt::PenStyle style);
	void setDigitalZoom(int zoom);

	ToolTip info(bool extended) const;

	const QColor color() const {return _pen.color();}
	const QString &name() const {return _name;}

public slots:
	void hover(bool hvr);

signals:
	void triggered();

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

private:
	void updatePainterPath(Map *map);

	RectC _bounds;
	QString _name;
	QString _fileName;

	int _digitalZoom;

	qreal _width;
	QPen _pen;
	QBrush _brush;
	qreal _opacity;

	QPainterPath _painterPath;
};

#endif // MAPITEM_H
