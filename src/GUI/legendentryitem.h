#ifndef LEGENDENTRYITEM_H
#define LEGENDENTRYITEM_H

#include <QObject>
#include <QGraphicsItem>
#include <QFont>

class LegendEntryItem : public QObject, public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)

public:
	LegendEntryItem(const QColor &color, const QString &text,
	  QGraphicsItem *parent = 0);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setTextColor(const QColor &color);

signals:
	void selected(bool);

private:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

	QRectF _boundingRect;
	QColor _color, _textColor;
	QString _text;
	QFont _font;
};

#endif // LEGENDENTRYITEM_H
