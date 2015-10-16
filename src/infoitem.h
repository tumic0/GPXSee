#ifndef INFOITEM_H
#define INFOITEM_H

#include <QGraphicsItem>
#include <QList>

class InfoItem : public QGraphicsItem
{
public:
	InfoItem(QGraphicsItem *parent = 0);

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void insert(const QString &key, const QString &value);
	void clear() {_list.clear();}

private:
	class KV {
	public:
		QString key;
		QString value;

		KV(const QString &k, const QString &v)
		  {key = k; value = v;}
		bool operator==(const KV &other) const
		  {return this->key == other.key;}
	};

	QList<KV> _list;
};

#endif // INFOITEM_H
