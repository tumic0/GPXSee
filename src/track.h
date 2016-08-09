#ifndef TRACK_H
#define TRACK_H

#include <QVector>
#include <QDateTime>
#include "trackpoint.h"

class Track
{
public:
	Track(const QVector<Trackpoint> &data);

	QVector<QPointF> track() const;
	QVector<QPointF> elevation() const;
	QVector<QPointF> speed() const;
	QVector<QPointF> heartRate() const;
	QVector<QPointF> temperature() const;

	qreal distance() const;
	qreal time() const;
	QDateTime date() const;

	bool isNull() const {return _dd.isEmpty();}

private:
	const QVector<Trackpoint> &_data;
	QVector<qreal> _dd;
};

#endif // TRACK_H
