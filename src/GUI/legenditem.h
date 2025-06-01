#ifndef LEGENDITEM_H
#define LEGENDITEM_H

#include <QGraphicsItem>

class LegendEntryItem;
class PathItem;
class PlaneItem;

class LegendItem : public QGraphicsItem
{
public:
	LegendItem(QGraphicsItem *parent = 0);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void addItem(PathItem *item);
	void addItem(PlaneItem *item);
	void setDigitalZoom(qreal zoom);
	void setColor(const QColor &color);
	void setBackgroundColor(const QColor &color);
	void drawBackground(bool draw);
	void clear();

private:
	QList<LegendEntryItem*> _items;
	QRectF _boundingRect;
	QColor _bgColor;
	bool _drawBackground;
};

#endif // LEGENDITEM_H
