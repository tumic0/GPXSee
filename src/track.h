#ifndef TRACK_H
#define TRACK_H

#include <QVector>
#include <QDateTime>
#include "trackpoint.h"
#include "graph.h"

class Track
{
public:
	Track(const QVector<Trackpoint> &data);

	const QVector<Trackpoint> &track() const {return _data;}
	Graph elevation() const;
	Graph speed() const;
	Graph heartRate() const;
	Graph temperature() const;

	qreal distance() const;
	qreal time() const;
	QDateTime date() const;

	bool isNull() const {return (_data.size() < 2);}

private:
	const QVector<Trackpoint> &_data;
	QVector<qreal> _distance;
	QVector<qreal> _time;
};

#endif // TRACK_H
