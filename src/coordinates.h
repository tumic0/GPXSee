#ifndef COORDINATES_H
#define COORDINATES_H

#include <cmath>
#include <QPointF>
#include <QDebug>

class Coordinates
{
public:
	Coordinates() {_lon = NAN; _lat = NAN;}
	Coordinates(const Coordinates &c) {_lon = c._lon; _lat = c._lat;}
	Coordinates(qreal lon, qreal lat) {_lon = lon; _lat = lat;}

	qreal &rlon() {return _lon;}
	qreal &rlat() {return _lat;}
	void setLon(qreal lon) {_lon = lon;}
	void setLat(qreal lat) {_lat = lat;}
	qreal lon() const {return _lon;}
	qreal lat() const {return _lat;}

	bool isNull() const
		{return (std::isnan(_lon) || std::isnan(_lat)) ? true : false;}
	bool isValid() const
		{return (_lon >= -180.0 && _lon <= 180.0 && _lat >= -90.0
		&& _lat <= 90.0) ? true : false;}

	qreal distanceTo(const Coordinates &c) const;

	QPointF toMercator() const;
	static Coordinates fromMercator(const QPointF &m);

private:
	qreal _lat, _lon;
};

bool operator==(const Coordinates &c1, const Coordinates &c2);
QDebug operator<<(QDebug dbg, const Coordinates &trackpoint);

#endif // COORDINATES_H
