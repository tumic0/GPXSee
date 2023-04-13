#ifndef PCS_H
#define PCS_H

#include <QDebug>
#include <QMap>
#include "common/kv.h"
#include "gcs.h"
#include "linearunits.h"
#include "coordinatesystem.h"
#include "conversion.h"

class PCS
{
public:
	PCS() {}
	PCS(const GCS &gcs, const Conversion &conversion)
	  : _gcs(gcs), _conversion(conversion) {}

	const GCS &gcs() const {return _gcs;}
	const Conversion &conversion() const {return _conversion;}

	bool isNull() const {return (_gcs.isNull() && _conversion.isNull());}
	bool isValid() const {return (_gcs.isValid() && _conversion.isValid());}

	static void loadList(const QString &path);
	static PCS pcs(int id);
	static QList<KV<int, QString> > list();

private:
	class Entry {
	public:
		Entry(const QString &name, int gcs, int proj)
			: _name(name), _gcs(gcs), _proj(proj) {}

		const QString &name() const {return _name;}
		int proj() const {return _proj;}
		int gcs() const {return _gcs;}

	private:
		QString _name;
		int _gcs, _proj;
	};

	static QMap<int, Entry> defaults();

	GCS _gcs;
	Conversion _conversion;

	static QMap<int, Entry> _pcss;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const PCS &pcs);
#endif // QT_NO_DEBUG

#endif // PCS_H
