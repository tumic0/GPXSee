#ifndef CONVERSION_H
#define CONVERSION_H

#include <cmath>
#include <QDebug>
#include <common/kv.h>
#include "coordinatesystem.h"
#include "linearunits.h"

class Conversion {
public:
	class Setup {
	public:
		Setup() : _latitudeOrigin(NAN), _longitudeOrigin(NAN), _scale(NAN),
		  _falseEasting(NAN), _falseNorthing(NAN), _standardParallel1(NAN),
		  _standardParallel2(NAN) {}
		Setup(double latitudeOrigin, double longitudeOrigin, double scale,
		  double falseEasting, double falseNorthing, double standardParallel1,
		  double standardParallel2) : _latitudeOrigin(latitudeOrigin),
		  _longitudeOrigin(longitudeOrigin), _scale(scale),
		  _falseEasting(falseEasting), _falseNorthing(falseNorthing),
		  _standardParallel1(standardParallel1),
		  _standardParallel2(standardParallel2) {}

		double latitudeOrigin() const {return _latitudeOrigin;}
		double longitudeOrigin() const {return _longitudeOrigin;}
		double scale() const {return _scale;}
		double falseEasting() const {return _falseEasting;}
		double falseNorthing() const {return _falseNorthing;}
		double standardParallel1() const {return _standardParallel1;}
		double standardParallel2() const {return _standardParallel2;}

		void setLatitudeOrigin(double val) {_latitudeOrigin = val;}
		void setLongitudeOrigin(double val) {_longitudeOrigin = val;}
		void setScale(double val) {_scale = val;}
		void setFalseEasting(double val) {_falseEasting = val;}
		void setFalseNorthing(double val) {_falseNorthing = val;}
		void setStandardParallel1(double val) {_standardParallel1 = val;}
		void setStandardParallel2(double val) {_standardParallel2 = val;}

		bool isNull() const
		{
			return std::isnan(_latitudeOrigin)
			  && std::isnan(_longitudeOrigin) && std::isnan(_scale)
			  && std::isnan(_falseEasting) && std::isnan(_falseNorthing)
			  && std::isnan(_standardParallel1)
			  && std::isnan(_standardParallel2);
		}

	private:
		double _latitudeOrigin;
		double _longitudeOrigin;
		double _scale;
		double _falseEasting;
		double _falseNorthing;
		double _standardParallel1;
		double _standardParallel2;
	};

	class Method {
	public:
		Method() : _id(0) {}
		Method(int id);

		int id() const {return _id;}
		bool isNull() const {return (_id == 0);}
		bool isValid() const {return !isNull();}
	private:
		int _id;
	};

	Conversion() {}
	Conversion(const Method &method, const Setup &setup,
	  const LinearUnits &units, const CoordinateSystem &cs = CoordinateSystem())
	  : _method(method), _setup(setup), _units(units), _cs(cs) {}

	const Method &method() const {return _method;}
	const Setup &setup() const {return _setup;}
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
		Entry(const QString &name, const Method &method, const Setup &setup,
		  const LinearUnits &units, const CoordinateSystem &cs = CoordinateSystem())
		  : _name(name), _method(method), _setup(setup), _units(units), _cs(cs) {}

		const QString &name() const {return _name;}
		const Method &method() const {return _method;}
		const Setup &setup() const {return _setup;}
		const LinearUnits &units() const {return _units;}
		const CoordinateSystem &cs() const {return _cs;}

	private:
		QString _name;
		Method _method;
		Setup _setup;
		LinearUnits _units;
		CoordinateSystem _cs;
	};

	static QMap<int, Entry> defaults();

	Method _method;
	Setup _setup;
	LinearUnits _units;
	CoordinateSystem _cs;

	static QMap<int, Entry> _conversions;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Conversion::Setup &setup);
QDebug operator<<(QDebug dbg, const Conversion::Method &method);
QDebug operator<<(QDebug dbg, const Conversion &conversion);
#endif // QT_NO_DEBUG

#endif // CONVERSION_H
