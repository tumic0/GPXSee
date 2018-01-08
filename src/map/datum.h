#ifndef DATUM_H
#define DATUM_H

#include <cmath>
#include <QList>
#include <QDebug>
#include "ellipsoid.h"
#include "common/coordinates.h"

class Datum
{
public:
	Datum() : _ellipsoid(Ellipsoid()), _dx(NAN), _dy(NAN), _dz(NAN),
	  _WGS84(false) {}
	Datum(const Ellipsoid &ellipsoid, double dx, double dy, double dz);
	Datum(const QString &name);
	Datum(int id);

	const Ellipsoid &ellipsoid() const {return _ellipsoid;}
	double dx() const {return _dx;}
	double dy() const {return _dy;}
	double dz() const {return _dz;}

	bool isNull() const
	  {return (_ellipsoid.isNull() && std::isnan(_dx) && std::isnan(_dy)
	    && std::isnan(_dz));}

	Coordinates toWGS84(const Coordinates &c) const;
	Coordinates fromWGS84(const Coordinates &c) const;

	static bool loadList(const QString &path);
	static const QString &errorString() {return _errorString;}
	static int errorLine() {return _errorLine;}

private:
	class Entry;

	static QList<Entry> WGS84();

	Ellipsoid _ellipsoid;
	double _dx, _dy, _dz;
	bool _WGS84;

	static QList<Entry> _datums;
	static QString _errorString;
	static int _errorLine;
};

QDebug operator<<(QDebug dbg, const Datum &datum);

#endif // DATUM_H
