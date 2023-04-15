#ifndef CONVERSION_H
#define CONVERSION_H

#include "projection.h"

class Conversion {
public:
	Conversion() {}
	Conversion(const Projection::Method &method,
	  const Projection::Setup &setup, const LinearUnits &units,
	  const CoordinateSystem &cs = CoordinateSystem()) : _method(method),
	  _setup(setup), _units(units), _cs(cs) {}

	const Projection::Method &method() const {return _method;}
	const Projection::Setup &setup() const {return _setup;}
	const LinearUnits &units() const {return _units;}
	const CoordinateSystem &cs() const {return _cs;}

	bool isNull() const {
		return (_units.isNull() && _method.isNull() && _setup.isNull()
		  && _cs.isNull());
	}
	bool isValid() const {
		/* We do not check the CoordinateSystem here as it is not always defined
		   and except of WMTS/WMS it is not needed. The projection setup is
		   always valid as we do not have any checks for it. */
		return (_units.isValid() && _method.isValid());
	}

	static bool loadList(const QString &path);
	static Conversion conversion(int id);
	static QList<KV<int, QString> > list();

private:
	class Entry {
	public:
		Entry(const QString &name, const Projection::Method &method,
			  const Projection::Setup &setup, const LinearUnits &units,
			  const CoordinateSystem &cs = CoordinateSystem())
			: _name(name), _method(method), _setup(setup), _units(units),
			_cs(cs) {}

		const QString &name() const {return _name;}
		const Projection::Method &method() const {return _method;}
		const Projection::Setup &setup() const {return _setup;}
		const LinearUnits &units() const {return _units;}
		const CoordinateSystem &cs() const {return _cs;}

	private:
		QString _name;
		Projection::Method _method;
		Projection::Setup _setup;
		LinearUnits _units;
		CoordinateSystem _cs;
	};

	static QMap<int, Entry> defaults();

	Projection::Method _method;
	Projection::Setup _setup;
	LinearUnits _units;
	CoordinateSystem _cs;

	static QMap<int, Entry> _conversions;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Conversion &conversion);
#endif // QT_NO_DEBUG

#endif // CONVERSION_H
