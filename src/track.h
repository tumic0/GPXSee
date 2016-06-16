#ifndef TRACK_H
#define TRACK_H

#include <QVector>
#include <QDateTime>
#include "trackpoint.h"

class Track
{
public:
	Track(const QVector<Trackpoint> &data);

	void elevationGraph(QVector<QPointF> &graph) const;
	void speedGraph(QVector<QPointF> &graph) const;
	void heartRateGraph(QVector<QPointF> &graph) const;
	void temperatureGraph(QVector<QPointF> &graph) const;
	void track(QVector<QPointF> &track) const;
	qreal distance() const {return _distance;}
	qreal time() const;
	QDateTime date() const;

private:
	const QVector<Trackpoint> &_data;
	QVector<qreal> _dd;
	qreal _distance;
};

#endif // TRACK_H
