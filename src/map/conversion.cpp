#include <QFile>
#include "conversion.h"

static bool parameter(int key, double val, int units, Projection::Setup &setup)
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

static int projectionSetup(const QList<QByteArray> &list,
  Projection::Setup &setup)
{
	bool r1, r2, r3;

	for (int i = 5; i < 26; i += 3) {
		QString ks = list[i].trimmed();
		if (ks.isEmpty())
			break;

		int key = ks.toInt(&r1);
		double val = list[i+1].trimmed().toDouble(&r2);
		int un = list[i+2].trimmed().toInt(&r3);
		if (!r1 || !r2 || !r3)
			return (i - 5)/3 + 1;

		if (!parameter(key, val, un, setup))
			return (i - 5)/3 + 1;
	}

	return 0;
}

QMap<int, Conversion::Entry> Conversion::_conversions = defaults();

QMap<int, Conversion::Entry> Conversion::defaults()
{
	QMap<int, Conversion::Entry> map;
	map.insert(3856, Entry("Popular Visualisation Pseudo-Mercator", 1024,
	  Projection::Setup(), 9001, 4400));
	return map;
}

Conversion Conversion::conversion(int id)
{
	QMap<int, Entry>::const_iterator it = _conversions.find(id);

	if (it == _conversions.constEnd())
		return Conversion();
	else {
		const Entry &e = it.value();
		return Conversion(e.method(), e.setup(), e.units(), e.cs());
	}
}

void Conversion::loadList(const QString &path)
{
	QFile file(path);
	bool res;
	int ln = 0, pn;


	if (!file.open(QFile::ReadOnly)) {
		qWarning("Error opening projections file: %s: %s", qPrintable(path),
		  qPrintable(file.errorString()));
		return;
	}

	while (!file.atEnd()) {
		ln++;

		QByteArray line = file.readLine(4096);
		QList<QByteArray> list = line.split(',');
		if (list.size() != 26) {
			qWarning("%s:%d: Format error", qPrintable(path), ln);
			continue;
		}

		QString name(list.at(0).trimmed());
		int proj = list.at(1).trimmed().toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid projection code", qPrintable(path), ln);
			continue;
		}
		int units = list.at(2).trimmed().toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid linear units code", qPrintable(path), ln);
			continue;
		}
		int transform = list.at(3).trimmed().toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid coordinate transformation code",
			  qPrintable(path), ln);
			continue;
		}
		int cs = list.at(4).trimmed().toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid coordinate system code",
			  qPrintable(path), ln);
			continue;
		}

		if (!LinearUnits(units).isValid()) {
			qWarning("%s:%d: Unknown linear units code", qPrintable(path), ln);
			continue;
		}
		if (!Projection::Method(transform).isValid()) {
			qWarning("%s:%d: Unknown coordinate transformation code",
			  qPrintable(path), ln);
			continue;
		}
		if (!CoordinateSystem(cs).isValid()) {
			qWarning("%s:%d: Unknown coordinate system code", qPrintable(path),
			  ln);
			continue;
		}

		Projection::Setup setup;
		if ((pn = projectionSetup(list, setup))) {
			qWarning("%s: %d: Invalid projection parameter #%d",
			  qPrintable(path), ln, pn);
			continue;
		}

		_conversions.insert(proj, Entry(name, transform, setup, units, cs));
	}
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
QDebug operator<<(QDebug dbg, const Conversion &conversion)
{
	dbg.nospace() << "Conversion(" << conversion.method() << ", "
	  << conversion.units() << ", " << conversion.setup() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
