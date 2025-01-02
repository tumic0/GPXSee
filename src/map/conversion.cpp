#include <QFile>
#include "common/csv.h"
#include "angularunits.h"
#include "conversion.h"

static bool parameter(int key, double val, int units, Conversion::Setup &setup)
{
	switch (key) {
		case 8801:
		case 8811:
		case 8821:
		case 8832:
			{AngularUnits au(units);
			if (au.isNull())
				return false;
			setup.setLatitudeOrigin(au.toDegrees(val));}
			return true;
		case 8802:
		case 8812:
		case 8822:
		case 8833:
			{AngularUnits au(units);
			if (au.isNull())
				return false;
			setup.setLongitudeOrigin(au.toDegrees(val));}
			return true;
		case 8805:
		case 8815:
		case 8819:
			setup.setScale(val);
			return true;
		case 8806:
		case 8816:
		case 8826:
			{LinearUnits lu(units);
			if (lu.isNull())
				return false;
			setup.setFalseEasting(lu.toMeters(val));}
			return true;
		case 8807:
		case 8817:
		case 8827:
			{LinearUnits lu(units);
			if (lu.isNull())
				return false;
			setup.setFalseNorthing(lu.toMeters(val));}
			return true;
		case 8813:
		case 8818:
		case 8823:
			{AngularUnits au(units);
			if (au.isNull())
				return false;
			setup.setStandardParallel1(au.toDegrees(val));}
			return true;
		case 1036:
		case 8814:
		case 8824:
			{AngularUnits au(units);
			if (au.isNull())
				return false;
			setup.setStandardParallel2(au.toDegrees(val));}
			return true;
		default:
			return false;
	}
}

static int projectionSetup(const QByteArrayList &list, Conversion::Setup &setup)
{
	bool r1, r2, r3;

	for (int i = 5; i < 26; i += 3) {
		const QByteArray &ks = list.at(i);
		if (ks.isEmpty())
			break;

		int key = ks.toInt(&r1);
		double val = list.at(i+1).toDouble(&r2);
		int un = list.at(i+2).toInt(&r3);
		if (!r1 || !r2 || !r3)
			return (i - 5)/3 + 1;

		if (!parameter(key, val, un, setup))
			return (i - 5)/3 + 1;
	}

	return 0;
}

Conversion::Method::Method(int id)
{
	switch (id) {
		case 1024:
		case 1041:
		case 9801:
		case 9802:
		case 9804:
		case 9807:
		case 9809:
		case 9815:
		case 9818:
		case 9819:
		case 9820:
		case 9822:
		case 9829:
			_id = id;
			break;
		default:
			_id = 0;
	}
}

QMap<int, Conversion::Entry> Conversion::_conversions = defaults();

QMap<int, Conversion::Entry> Conversion::defaults()
{
	QMap<int, Conversion::Entry> map;
	map.insert(3856, Entry("Popular Visualisation Pseudo-Mercator", 1024,
	  Setup(), 9001, 4400));
	return map;
}

Conversion Conversion::conversion(int id)
{
	QMap<int, Entry>::const_iterator it(_conversions.find(id));

	if (it == _conversions.constEnd())
		return Conversion();
	else {
		const Entry &e = it.value();
		return Conversion(e.method(), e.setup(), e.units(), e.cs());
	}
}

bool Conversion::loadList(const QString &path)
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
		if (entry.size() < 26) {
			qWarning("%s:%d: Invalid column count", qPrintable(path),
			  csv.line() - 1);
			return false;
		}

		QString name(entry.at(0));
		int proj = entry.at(1).toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid projection code", qPrintable(path),
			  csv.line() - 1);
			continue;
		}
		int units = entry.at(2).toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid linear units code", qPrintable(path),
			  csv.line() - 1);
			continue;
		}
		int transform = entry.at(3).toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid coordinate transformation code",
			  qPrintable(path), csv.line() - 1);
			continue;
		}
		int cs = entry.at(4).toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid coordinate system code",
			  qPrintable(path), csv.line() - 1);
			continue;
		}

		if (!LinearUnits(units).isValid()) {
			qWarning("%s:%d: Unknown linear units code", qPrintable(path),
			  csv.line() - 1);
			continue;
		}
		if (!Method(transform).isValid()) {
			qWarning("%s:%d: Unknown coordinate transformation code",
			  qPrintable(path), csv.line() - 1);
			continue;
		}
		if (!CoordinateSystem(cs).isValid()) {
			qWarning("%s:%d: Unknown coordinate system code", qPrintable(path),
			  csv.line() - 1);
			continue;
		}

		Setup setup;
		int pn = projectionSetup(entry, setup);
		if (pn) {
			qWarning("%s: %d: Invalid projection parameter #%d",
			  qPrintable(path), csv.line(), pn);
			continue;
		}

		_conversions.insert(proj, Entry(name, transform, setup, units, cs));
	}

	return true;
}

QList<KV<int, QString> > Conversion::list()
{
	QList<KV<int, QString> > list;

	for (QMap<int, Entry>::const_iterator it = _conversions.constBegin();
	  it != _conversions.constEnd(); ++it)
		list.append(KV<int, QString>(it.key(), it.value().name()));

	return list;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Conversion::Setup &setup)
{
	dbg.nospace() << "Setup(" << setup.latitudeOrigin() << ", "
	  << setup.longitudeOrigin() << ", " << setup.scale() << ", "
	  << setup.falseEasting() << ", " << setup.falseNorthing() << ", "
	  << setup.standardParallel1() << ", " << setup.standardParallel2() << ")";
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const Conversion::Method &method)
{
	dbg.nospace() << "Method(" << method.id() << ")";
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const Conversion &conversion)
{
	dbg.nospace() << "Conversion(" << conversion.method() << ", "
	  << conversion.units() << ", " << conversion.setup() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
