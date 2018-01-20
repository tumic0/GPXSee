#ifndef PCS_H
#define PCS_H

#include <QDebug>
#include <QList>
#include "gcs.h"
#include "projection.h"

class PCS
{
public:
	PCS() : _gcs(0) {}
	PCS(const GCS *gcs, const Projection::Method &m, const Projection::Setup &s)
	  : _gcs(gcs), _method(m), _setup(s) {}
	PCS(const GCS *gcs, int proj);

	const GCS *gcs() const {return _gcs;}
	const Projection::Method &method() const {return _method;}
	const Projection::Setup &setup() const {return _setup;}

	bool isNull() const
	  {return (_gcs->isNull() && _method.isNull() && _setup.isNull());}

	static void loadList(const QString &path);
	static const PCS *pcs(int id);
	static const PCS *pcs(const GCS *gcs, int proj);

private:
	class Entry;

	const GCS *_gcs;
	Projection::Method _method;
	Projection::Setup _setup;

	static QList<Entry> _pcss;
	static GCS _nullGCS;
};

QDebug operator<<(QDebug dbg, const PCS &pcs);

#endif // PCS_H
