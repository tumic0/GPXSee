#include <QFile>
#include "common/csv.h"
#include "gcs.h"


class GCS::Entry {
public:
	Entry(int id, int gd, const QString &name, const GCS &gcs)
	  : _id(id), _gd(gd), _name(name), _gcs(gcs) {}
	Entry(int id) : _id(id) {}

	int id() const {return _id;}
	int gd() const {return _gd;}
	const QString &name() const {return _name;}
	const GCS &gcs() const {return _gcs;}

	bool operator<(const Entry &other) const
	  {return _id < other._id;}

private:
	int _id, _gd;
	QString _name;
	GCS _gcs;
};

static int parameter(const QByteArray &str, bool *res)
{
	if (str.isEmpty()) {
		*res = true;
		return 0;
	}

	return str.toInt(res);
}

static double parameterd(const QByteArray &str, bool *res)
{
	if (str.isEmpty()) {
		*res = true;
		return NAN;
	}

	return str.toDouble(res);
}


QList<GCS::Entry> GCS::_gcss = defaults();

const GCS &GCS::WGS84()
{
	static GCS g(Datum::WGS84(), 8901, 9122);
	return g;
}

QList<GCS::Entry> GCS::defaults()
{
	QList<GCS::Entry> list;
	list.append(GCS::Entry(4326, 6326, "WGS 1984", WGS84()));
	list.append(GCS::Entry(4326, 6326, "WGS 84", WGS84()));
	list.append(GCS::Entry(4326, 6326, "WGS84", WGS84()));
	return list;
}

GCS GCS::gcs(int id)
{
	// There are GCSs without EPSG code (id = 0) in the list!
	if (!id)
		return GCS();

	QList<GCS::Entry>::iterator it = std::lower_bound(
	  _gcss.begin(), _gcss.end(), id);

	return (it == _gcss.end() || id != it->id()) ? GCS() : it->gcs();
}

GCS GCS::gcs(int geodeticDatum, int primeMeridian, int angularUnits)
{
	for (int i = 0; i < _gcss.size(); i++) {
		const Entry &e = _gcss.at(i);
		if (e.gd() == geodeticDatum && e.gcs().primeMeridian() == primeMeridian
		  && e.gcs().angularUnits() == angularUnits)
			return e.gcs();
	}

	return GCS();
}

GCS GCS::gcs(const QString &name)
{
	for (int i = 0; i < _gcss.size(); i++)
		if (_gcss.at(i).name() == name)
			return _gcss.at(i).gcs();

	return GCS();
}

bool GCS::loadList(const QString &path)
{
	QFile file(path);
	CSV csv(&file);
	QByteArrayList entry;
	bool res;

	if (!file.open(QFile::ReadOnly)) {
		qWarning("%s: %s", qPrintable(path), qPrintable(file.errorString()));
		return false;
	}

	while (!csv.atEnd()) {
		if (!csv.readEntry(entry)) {
			qWarning("%s:%d: Parse error", qPrintable(path), csv.line());
			return false;
		}
		if (entry.size() < 14) {
			qWarning("%s:%d: Invalid column count", qPrintable(path),
			  csv.line() - 1);
			return false;
		}

		int id = parameter(entry.at(1), &res);
		if (!res) {
			qWarning("%s:%d: Invalid GCS code", qPrintable(path),
			  csv.line() - 1);
			continue;
		}
		int gd = parameter(entry.at(2), &res);
		if (!res) {
			qWarning("%s:%d: Invalid geodetic datum code", qPrintable(path),
			  csv.line() - 1);
			continue;
		}
		int au = entry.at(3).toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid angular units code", qPrintable(path),
			  csv.line() - 1);
			continue;
		}
		int el = entry.at(4).toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid ellipsoid code", qPrintable(path),
			  csv.line() - 1);
			continue;
		}
		int pm = entry.at(5).toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid prime meridian code", qPrintable(path),
			  csv.line() - 1);
			continue;
		}
		int ct = entry.at(6).toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid coordinates transformation code",
			  qPrintable(path), csv.line() - 1);
			continue;
		}
		double dx = entry.at(7).toDouble(&res);
		if (!res) {
			qWarning("%s:%d: Invalid dx", qPrintable(path), csv.line() - 1);
			continue;
		}
		double dy = entry.at(8).toDouble(&res);
		if (!res) {
			qWarning("%s:%d: Invalid dy", qPrintable(path), csv.line() - 1);
			continue;
		}
		double dz = entry.at(9).toDouble(&res);
		if (!res) {
			qWarning("%s:%d: Invalid dz", qPrintable(path), csv.line() - 1);
			continue;
		}
		double rx = parameterd(entry.at(10), &res);
		if (!res) {
			qWarning("%s:%d: Invalid rx", qPrintable(path), csv.line() - 1);
			continue;
		}
		double ry = parameterd(entry.at(11), &res);
		if (!res) {
			qWarning("%s:%d: Invalid ry", qPrintable(path), csv.line() - 1);
			continue;
		}
		double rz = parameterd(entry.at(12), &res);
		if (!res) {
			qWarning("%s:%d: Invalid rz", qPrintable(path), csv.line() - 1);
			continue;
		}
		double ds = parameterd(entry.at(13), &res);
		if (!res) {
			qWarning("%s:%d: Invalid ds", qPrintable(path), csv.line() - 1);
			continue;
		}

		const Ellipsoid &e = Ellipsoid::ellipsoid(el);
		if (e.isNull()) {
			qWarning("%s:%d: Unknown ellipsoid code", qPrintable(path),
			  csv.line() - 1);
			continue;
		}

		Datum datum;
		switch (ct) {
			case 9603:
				datum = Datum(e, dx, dy, dz);
				break;
			case 9606:
				datum = Datum(e, dx, dy, dz, -rx, -ry, -rz, ds);
				break;
			case 9607:
				datum = Datum(e, dx, dy, dz, rx, ry, rz, ds);
				break;
			default:
				qWarning("%s:%d: Unknown coordinates transformation method",
				  qPrintable(path), csv.line() - 1);
				continue;
		}
		if (!datum.isValid()) {
			qWarning("%s:%d: Invalid coordinates transformation parameters",
			  qPrintable(path), csv.line() - 1);
			continue;
		}

		GCS gcs(datum, pm, au);
		if (gcs.isValid())
			_gcss.append(Entry(id, gd, entry.at(0), gcs));
		else
			qWarning("%s:%d: Unknown prime meridian/angular units code",
			  qPrintable(path), csv.line() - 1);
	}

	std::sort(_gcss.begin(), _gcss.end());

	return true;
}

QList<KV<int, QString> > GCS::list()
{
	QList<KV<int, QString> > list;

	for (int i = 0; i < _gcss.size(); i++) {
		const Entry &e = _gcss.at(i);
		if (!e.id() || (!list.isEmpty() && e.id() == list.last().key()))
			continue;

		list.append(KV<int, QString>(e.id(), e.name() + " / Geographic 2D"));
	}

	return list;
}

QList<KV<int, QString> > GCS::WGS84List()
{
	QList<KV<int, QString> > list;
	list.append(KV<int, QString>(4326, "Geographic 2D"));
	return list;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const GCS &gcs)
{
	dbg.nospace() << "GCS(" << gcs.datum() << ", " << gcs.primeMeridian()
	  << ", " << gcs.angularUnits() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
