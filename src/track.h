#ifndef TRACK_H
#define TRACK_H

#include <QVector>
#include <QDateTime>
#include "trackdata.h"
#include "graph.h"


class Track
{
public:
	Track(const TrackData &data);

	const TrackData &track() const {return _data;}
	Graph elevation() const;
	Graph speed() const;
	Graph heartRate() const;
	Graph temperature() const;

	qreal distance() const;
	qreal time() const;
	QDateTime date() const;

	bool isNull() const {return (_data.size() < 2);}

private:
	const TrackData &_data;
	QVector<qreal> _distance;
	QVector<qreal> _time;
};

#endif // TRACK_H
