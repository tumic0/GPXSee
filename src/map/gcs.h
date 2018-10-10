#ifndef GCS_H
#define GCS_H

#include "datum.h"
#include "angularunits.h"
#include "primemeridian.h"

class GCS
{
public:
	GCS() {}
	GCS(const Datum &datum, const PrimeMeridian &primeMeridian,
	  const AngularUnits &angularUnits) : _datum(datum),
	  _primeMeridian(primeMeridian), _angularUnits(angularUnits) {}

	const PrimeMeridian &primeMeridian() const {return _primeMeridian;}
	const AngularUnits &angularUnits() const {return _angularUnits;}
	const Datum &datum() const {return _datum;}

	bool isNull() const {return _datum.isNull() && _angularUnits.isNull()
	  && _primeMeridian.isNull();}
	bool isValid() const {return _datum.isValid() && _angularUnits.isValid()
	  && _primeMeridian.isValid();}

	Coordinates toWGS84(const Coordinates &c) const;
	Coordinates fromWGS84(const Coordinates &c) const;

	static const GCS *gcs(int id);
	static const GCS *gcs(int geodeticDatum, int primeMeridian,
	  int angularUnits);
	static const GCS *gcs(const QString &name);
	static const GCS &WGS84();

	static void loadList(const QString &path);

private:
	class Entry;

	static QList<Entry> defaults();

	Datum _datum;
	PrimeMeridian _primeMeridian;
	AngularUnits _angularUnits;

	static QList<Entry> _gcss;
};

inline bool operator==(const GCS &gcs1, const GCS &gcs2)
  {return (gcs1.datum() == gcs2.datum()
	&& gcs1.primeMeridian() == gcs2.primeMeridian()
	&& gcs1.angularUnits() == gcs2.angularUnits());}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const GCS &gcs);
#endif // QT_NO_DEBUG

#endif // GCS_H
