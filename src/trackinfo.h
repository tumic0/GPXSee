#ifndef TRACKINFO_H
#define TRACKINFO_H

#include <QGraphicsScene>

class InfoItem;

class TrackInfo : public QGraphicsScene
{
	Q_OBJECT

public:
	TrackInfo(QObject *parent = 0);

	void insert(const QString &key, const QString &value);
	void plot(QPainter *painter, const QRectF &target, qreal scale);
	bool isEmpty() const;
	QSizeF contentSize() const;

private:
	InfoItem *_info;
};

#endif // TRACKINFO_H
