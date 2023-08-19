#ifndef MARKERINFOITEM_H
#define MARKERINFOITEM_H

#include <QGraphicsItem>
#include <QFont>
#include "format.h"

class Coordinates;

class MarkerInfoItem : public QGraphicsItem
{
public:
	enum Type {
		None,
		Date,
		Position
	};

	MarkerInfoItem(QGraphicsItem *parent = 0);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setDate(const QDateTime &date);
	void setCoordinates(const Coordinates &c);

	void setColor(const QColor &color);
	void setBackgroundColor(const QColor &color);
	void drawBackground(bool draw);

	static void setCoordinatesFormat(const CoordinatesFormat &format)
	  {_format = format;}

private:
	void updateBoundingRect();

	QString _s1, _s2;
	QRectF _boundingRect;
	QColor _color, _bgColor;
	QFont _font;
	bool _drawBackground;

	static CoordinatesFormat _format;
};

#endif // MARKERINFOITEM_H
