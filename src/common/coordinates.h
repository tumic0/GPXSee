#ifndef COORDINATES_H
#define COORDINATES_H

#include <cmath>
#include <QDebug>

#define deg2rad(d) (((d)*M_PI)/180.0)
#define rad2deg(d) (((d)*180.0)/M_PI)

class Coordinates
{
public:
	Coordinates() {_lon = NAN; _lat = NAN;}
	Coordinates(double lon, double lat) {_lon = lon; _lat = lat;}

	double &rlon() {return _lon;}
	double &rlat() {return _lat;}
	void setLon(double lon) {_lon = lon;}
	void setLat(double lat) {_lat = lat;}
	double lon() const {return _lon;}
	double lat() const {return _lat;}

	bool isNull() const
	  {return std::isnan(_lon) && std::isnan(_lat);}
	bool isValid() const
	  {return (_lon >= -180.0 && _lon <= 180.0
	    && _lat >= -90.0 && _lat <= 90.0);}

	double distanceTo(const Coordinates &c) const;

private:
	double _lat, _lon;
};

inline bool operator==(const Coordinates &c1, const Coordinates &c2)
  {return (c1.lat() == c2.lat() && c1.lon() == c2.lon());}
inline bool operator!=(const Coordinates &c1, const Coordinates &c2)
  {return !(c1 == c2);}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Coordinates &c);
#endif // QT_NO_DEBUG

#endif // COORDINATES_H
