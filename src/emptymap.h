#ifndef EMPTYMAP_H
#define EMPTYMAP_H

#include "map.h"

class EmptyMap : public Map
{
	Q_OBJECT

public:
	EmptyMap(QObject *parent = 0);

	const QString &name() const {return _name;}

	QRectF bounds() const;
	qreal resolution(const QPointF &p) const;

	qreal zoom() const {return _scale;}
	qreal zoomFit(const QSize &size, const QRectF &br);
	qreal zoomIn();
	qreal zoomOut();

	QPointF ll2xy(const Coordinates &c) const;
	Coordinates xy2ll(const QPointF &p) const;

	void draw(QPainter *painter, const QRectF &rect);

private:
	QString _name;
	qreal _scale;
};

#endif // EMPTYMAP_H
