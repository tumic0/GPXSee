#include <QFile>
#include "common/wgs84.h"
#include "gcs.h"


class GCS::Entry {
public:
	Entry(int id, int gd, const QString &name, const GCS &gcs)
	  : _id(id), _gd(gd), _name(name), _gcs(gcs) {}

	int id() const {return _id;}
	int gd() const {return _gd;}
	const QString &name() const {return _name;}
	const GCS &gcs() const {return _gcs;}

private:
	int _id, _gd;
	QString _name;
	GCS _gcs;
};

static int parameter(const QString &str, bool *res)
{
	QString field = str.trimmed();
	if (field.isEmpty()) {
		*res = true;
		return 0;
	}

	return field.toInt(res);
}

static double parameterd(const QString &str, bool *res)
{
	QString field = str.trimmed();
	if (field.isEmpty()) {
		*res = true;
		return NAN;
	}

	return field.toDouble(res);
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
	list.append(GCS::Entry(4326, 6326, "WGS 84", WGS84()));
	return list;
}

const GCS *GCS::gcs(int id)
{
	for (int i = 0; i < _gcss.size(); i++)
		if (_gcss.at(i).id() == id)
			return &(_gcss.at(i).gcs());

	return 0;
}

const GCS *GCS::gcs(int geodeticDatum, int primeMeridian, int angularUnits)
{
	for (int i = 0; i < _gcss.size(); i++) {
		const Entry &e = _gcss.at(i);
		if (e.gd() == geodeticDatum && e.gcs().primeMeridian() == primeMeridian
		  && e.gcs().angularUnits() == angularUnits)
			return &(e.gcs());
	}

	return 0;
}

const GCS *GCS::gcs(const QString &name)
{
	for (int i = 0; i < _gcss.size(); i++)
		if (_gcss.at(i).name() == name)
			return &(_gcss.at(i).gcs());

	return 0;
}

void GCS::loadList(const QString &path)
{
	QFile file(path);
	bool res;
	int ln = 0;
	const Ellipsoid *e;


	if (!file.open(QFile::ReadOnly)) {
		qWarning("Error opening PCS file: %s: %s", qPrintable(path),
		  qPrintable(file.errorString()));
		return;
	}

	while (!file.atEnd()) {
		ln++;

		QByteArray line = file.readLine();
		QList<QByteArray> list = line.split(',');
		if (list.size() != 14) {
			qWarning("%s:%d: Format error", qPrintable(path), ln);
			continue;
		}

		int id = parameter(list[1], &res);
		if (!res) {
			qWarning("%s:%d: Invalid GCS code", qPrintable(path), ln);
			continue;
		}
		int gd = parameter(list[2], &res);
		if (!res) {
			qWarning("%s:%d: Invalid geodetic datum code", qPrintable(path),
			  ln);
			continue;
		}
		int au = list[3].trimmed().toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid angular units code", qPrintable(path),
			  ln);
			continue;
		}
		int el = list[4].trimmed().toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid ellipsoid code", qPrintable(path), ln);
			continue;
		}
		int pm = list[5].trimmed().toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid prime meridian code", qPrintable(path),
			  ln);
			continue;
		}
		int ct = list[6].trimmed().toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid coordinates transformation code",
			  qPrintable(path), ln);
			continue;
		}
		double dx = list[7].trimmed().toDouble(&res);
		if (!res) {
			qWarning("%s:%d: Invalid dx", qPrintable(path), ln);
			continue;
		}
		double dy = list[8].trimmed().toDouble(&res);
		if (!res) {
			qWarning("%s:%d: Invalid dy", qPrintable(path), ln);
			continue;
		}
		double dz = list[9].trimmed().toDouble(&res);
		if (!res) {
			qWarning("%s:%d: Invalid dz", qPrintable(path), ln);
			continue;
		}
		double rx = parameterd(list[10], &res);
		if (!res) {
			qWarning("%s:%d: Invalid rx", qPrintable(path), ln);
			continue;
		}
		double ry = parameterd(list[11], &res);
		if (!res) {
			qWarning("%s:%d: Invalid ry", qPrintable(path), ln);
			continue;
		}
		double rz = parameterd(list[12], &res);
		if (!res) {
			qWarning("%s:%d: Invalid rz", qPrintable(path), ln);
			continue;
		}
		double ds = parameterd(list[13], &res);
		if (!res) {
			qWarning("%s:%d: Invalid ds", qPrintable(path), ln);
			continue;
		}

		if (!(e = Ellipsoid::ellipsoid(el))) {
			qWarning("%s:%d: Unknown ellipsoid code", qPrintable(path), ln);
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
				  qPrintable(path), ln);
				continue;
		}
		if (!datum.isValid()) {
			qWarning("%s:%d: Invalid coordinates transformation parameters",
			  qPrintable(path), ln);
			continue;
		}

		GCS gcs(datum, pm, au);
		if (gcs.isValid())
			_gcss.append(Entry(id, gd, list[0].trimmed(), gcs));
		else
			qWarning("%s:%d: Unknown prime meridian/angular units code",
			  qPrintable(path), ln);
	}
}

Coordinates GCS::toWGS84(const Coordinates &c) const
{
	return datum().toWGS84(Coordinates(_primeMeridian.toGreenwich(c.lon()),
	  c.lat()));
}

Coordinates GCS::fromWGS84(const Coordinates &c) const
{
	Coordinates ds(datum().fromWGS84(c));
	return Coordinates(_primeMeridian.fromGreenwich(ds.lon()), ds.lat());
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const GCS &gcs)
{
	dbg.nospace() << "GCS(" << gcs.datum() << ", " << gcs.primeMeridian()
	  << ", " << gcs.angularUnits() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
