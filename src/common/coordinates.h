#ifndef COORDINATES_H
#define COORDINATES_H

#include <cmath>
#include <QPair>
#include <QDebug>

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif // M_PI
#define deg2rad(d) (((d)*M_PI)/180.0)
#define rad2deg(d) (((d)*180.0)/M_PI)

class Coordinates
{
public:
	Coordinates() {_lon = NAN; _lat = NAN;}
	Coordinates(qreal lon, qreal lat) {_lon = lon; _lat = lat;}

	qreal &rlon() {return _lon;}
	qreal &rlat() {return _lat;}
	void setLon(qreal lon) {_lon = lon;}
	void setLat(qreal lat) {_lat = lat;}
	qreal lon() const {return _lon;}
	qreal lat() const {return _lat;}

	bool isNull() const
	  {return std::isnan(_lon) && std::isnan(_lat);}
	bool isValid() const
	  {return (_lon >= -180.0 && _lon <= 180.0
	    && _lat >= -90.0 && _lat <= 90.0);}

	qreal distanceTo(const Coordinates &c) const;
	QPair<Coordinates, Coordinates> boundingRect(qreal distance) const;

private:
	qreal _lat, _lon;
};

inline bool operator==(const Coordinates &c1, const Coordinates &c2)
  {return (c1.lat() == c2.lat() && c1.lon() == c2.lon());}
inline bool operator!=(const Coordinates &c1, const Coordinates &c2)
  {return !(c1 == c2);}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Coordinates &c);
#endif // QT_NO_DEBUG

#endif // COORDINATES_H
