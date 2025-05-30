#ifndef LEGENDITEM_H
#define LEGENDITEM_H

#include <QGraphicsItem>

class LegendItem  : public QGraphicsItem
{
public:
	LegendItem(QGraphicsItem *parent = 0);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setDigitalZoom(qreal zoom);
	void addItem(const QColor &color, const QString &text);
	void setColor(const QColor &color);
	void setBackgroundColor(const QColor &color);
	void drawBackground(bool draw);
	void clear();

private:
	struct Item
	{
		Item(const QColor &color, const QString &text)
		  : color(color), text(text) {}

		QColor color;
		QString text;
	};

	QList<Item> _items;
	QRectF _boundingRect;
	QColor _color, _bgColor;
	QFont _font;
	bool _drawBackground;
};

#endif // LEGENDITEM_H
