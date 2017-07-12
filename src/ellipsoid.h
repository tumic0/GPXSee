#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#include <QString>
#include <QMap>

class Ellipsoid
{
public:
	Ellipsoid() : _radius(0.0), _flattening(0.0) {}
	Ellipsoid(double radius, double flattening)
	  : _radius(radius), _flattening(flattening) {}

	double radius() const {return _radius;}
	double flattening() const {return _flattening;}

	bool isNull() const {return (_radius == 0.0 && _flattening == 0.0);}

	static bool loadList(const QString &path);
	static const QString &errorString() {return _errorString;}
	static int errorLine() {return _errorLine;}

	static Ellipsoid ellipsoid(int id);

private:
	double _radius;
	double _flattening;

	static QMap<int, Ellipsoid> _ellipsoids;
	static QString _errorString;
	static int _errorLine;
};

#endif // ELLIPSOID_H
