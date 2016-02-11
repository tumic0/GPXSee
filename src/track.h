#ifndef TRACK_H
#define TRACK_H

#include <QVector>
#include <QDateTime>
#include "trackpoint.h"

class Track
{
public:
	Track(const QVector<TrackPoint> &data) : _data(data) {}

	void elevationGraph(QVector<QPointF> &graph) const;
	void speedGraph(QVector<QPointF> &graph) const;
	void track(QVector<QPointF> &track) const;
	qreal distance() const;
	qreal time() const;
	QDateTime date() const;

private:
	const QVector<TrackPoint> &_data;
};

#endif // TRACK_H
