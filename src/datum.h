#ifndef DATUM_H
#define DATUM_H

#include <QMap>
#include "ellipsoid.h"
#include "coordinates.h"

class Datum
{
public:
	Datum() : _ellipsoid(Ellipsoid()), _dx(0.0), _dy(0.0), _dz(0.0) {}
	Datum(const Ellipsoid &ellipsoid, double dx, double dy, double dz)
	  : _ellipsoid(ellipsoid), _dx(dx), _dy(dy), _dz(dz) {}

	const Ellipsoid &ellipsoid() const {return _ellipsoid;}
	double dx() const {return _dx;}
	double dy() const {return _dy;}
	double dz() const {return _dz;}

	bool isNull() const
	  {return (_ellipsoid.isNull() && _dx == 0.0 && _dy == 0.0 && _dz == 0.0);}

	Coordinates toWGS84(const Coordinates &c) const;

	static bool loadList(const QString &path);
	static const QString &errorString() {return _errorString;}
	static int errorLine() {return _errorLine;}

	static Datum datum(const QString &name);

private:
	Ellipsoid _ellipsoid;
	double _dx, _dy, _dz;

	static QMap<QString, Datum> _datums;
	static QString _errorString;
	static int _errorLine;
};

#endif // DATUM_H
