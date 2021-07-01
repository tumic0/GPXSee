#ifndef PCS_H
#define PCS_H

#include <QDebug>
#include <QList>
#include "common/kv.h"
#include "gcs.h"
#include "linearunits.h"
#include "coordinatesystem.h"
#include "projection.h"

class PCS
{
public:
	PCS() {}
	PCS(const GCS &gcs, const Projection::Method &method,
	  const Projection::Setup &setup, const LinearUnits &units,
	  const CoordinateSystem &cs = CoordinateSystem())
	  : _gcs(gcs), _method(method), _setup(setup), _units(units), _cs(cs) {}
	PCS(const GCS &gcs, int proj);

	const GCS &gcs() const {return _gcs;}
	const Projection::Method &method() const {return _method;}
	const Projection::Setup &setup() const {return _setup;}
	const LinearUnits &units() const {return _units;}
	const CoordinateSystem &coordinateSystem() const {return _cs;}

	bool isNull() const {
		return (_gcs.isNull() && _units.isNull() && _method.isNull()
		  && _setup.isNull() && _cs.isNull());
	}
	bool isValid() const {
		// We do not check the CoordinateSystem here as it is not always defined
		// and except of WMTS/WMS it is not needed.
		return (_gcs.isValid() && _units.isValid() && _method.isValid());
	}

	static void loadList(const QString &path);
	static PCS pcs(int id);
	static PCS pcs(const GCS &gcs, int proj);
	static QList<KV<int, QString> > list();

private:
	class Entry;

	static QList<Entry> defaults();

	GCS _gcs;
	Projection::Method _method;
	Projection::Setup _setup;
	LinearUnits _units;
	CoordinateSystem _cs;

	static QList<Entry> _pcss;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const PCS &pcs);
#endif // QT_NO_DEBUG

#endif // PCS_H
