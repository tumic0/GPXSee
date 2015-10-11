#ifndef INFOITEM_H
#define INFOITEM_H

#include <QGraphicsItem>
#include <QMap>

class InfoItem : public QGraphicsItem
{
public:
	InfoItem();

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void insert(const QString &key, const QString &value);

private:
	QMap<QString, QString> _map;
};

#endif // INFOITEM_H
