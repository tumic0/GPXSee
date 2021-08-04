#ifndef PATHTICKITEM_H
#define PATHTICKITEM_H

#include <QFont>
#include <QGraphicsItem>
#include "graphicsscene.h"

class PathTickItem : public GraphicsItem
{
public:
	PathTickItem(const QRectF &tickRect, int value, QGraphicsItem *parent = 0);

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setPos(const QPointF &pos);
	void setColor(const QColor &color) {_brush = QBrush(color);}
	void setDigitalZoom(int zoom) {setScale(pow(2, -zoom));}

	int type() const {return parentItem()->type();}
	ToolTip info() const
	{
		return static_cast<GraphicsItem*>(parentItem())->info();
	}

	static QRect tickRect(int value);

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
	QRectF _tickRect;
	QString _text;
	QBrush _brush;

	static QFont _font;
};

#endif // PATHTICKITEM_H
