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
	Ellipsoid(int id);

	double radius() const {return _radius;}
	double flattening() const {return _flattening;}

	bool isNull() const
	  {return (std::isnan(_radius) && std::isnan(_flattening));}

	static bool loadList(const QString &path);
	static const QString &errorString() {return _errorString;}
	static int errorLine() {return _errorLine;}

private:
	static void error(const QString &str);

	double _radius;
	double _flattening;

	static QMap<int, Ellipsoid> _ellipsoids;
	static QString _errorString;
	static int _errorLine;
};

QDebug operator<<(QDebug dbg, const Ellipsoid &ellipsoid);

#endif // ELLIPSOID_H
