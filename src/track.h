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

	bool isNull() const {return (_dd.count() < 2);}

private:
	const QVector<Trackpoint> &_data;
	QVector<qreal> _dd;
	QVector<qreal> _td;
};

#endif // TRACK_H
