#ifndef PATHTICKITEM_H
#define PATHTICKITEM_H

#include <QFont>
#include <QGraphicsItem>

class PathTickItem : public QGraphicsItem
{
public:
	PathTickItem(const QRectF &tickRect, int value, QGraphicsItem *parent = 0);

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setPos(const QPointF &pos);
	void setColor(const QColor &color) {_brush = QBrush(color);}

	static QRect tickRect(int value);

private:
	QRectF _tickRect;
	QString _text;
	QBrush _brush;

	static QFont _font;
};

#endif // PATHTICKITEM_H
