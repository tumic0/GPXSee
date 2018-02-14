#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#include <cmath>
#include <QString>
#include <QMap>
#include <QDebug>

class Ellipsoid
{
public:
	Ellipsoid() : _radius(NAN), _flattening(NAN) {}
	Ellipsoid(double radius, double flattening)
	  : _radius(radius), _flattening(flattening) {}

	double radius() const {return _radius;}
	double flattening() const {return _flattening;}

	bool isNull() const
	  {return (std::isnan(_radius) && std::isnan(_flattening));}
	bool isValid() const
	  {return !(std::isnan(_radius) || std::isnan(_flattening));}

	static const Ellipsoid *ellipsoid(int id);
	static void loadList(const QString &path);

private:
	double _radius;
	double _flattening;

	static QMap<int, Ellipsoid> WGS84();
	static QMap<int, Ellipsoid> _ellipsoids;
};

inline bool operator==(const Ellipsoid &e1, const Ellipsoid &e2)
  {return (e1.radius() == e2.radius() && e1.flattening() == e2.flattening());}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Ellipsoid &ellipsoid);
#endif // QT_NO_DEBUG

#endif // ELLIPSOID_H
