#ifndef EMPTYMAP_H
#define EMPTYMAP_H

#include "common/coordinates.h"
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
	qreal zoomFit(const QSize &size, const RectC &br);
	qreal zoomFit(qreal resolution, const Coordinates &c);
	qreal zoomIn();
	qreal zoomOut();

	QPointF ll2xy(const Coordinates &c)
		{return static_cast<const EmptyMap &>(*this).ll2xy(c);}
	Coordinates xy2ll(const QPointF &p)
		{return static_cast<const EmptyMap &>(*this).xy2ll(p);}

	void draw(QPainter *painter, const QRectF &rect);

private:
	QPointF ll2xy(const Coordinates &c) const;
	Coordinates xy2ll(const QPointF &p) const;

	QString _name;
	qreal _scale;
};

#endif // EMPTYMAP_H
