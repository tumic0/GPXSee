#include <QApplication>
#include "common/coordinates.h"
#include "format.h"


static QString deg2DMS(double val)
{
	int deg = val;
	double r1 = val - deg;

	int min = r1 * 60.0;
	double r2 = r1 - (min / 60.0);

	double sec = r2 * 3600.0;

	return QString("%1°%2'%3\"").arg(deg).arg(min, 2, 10, QChar('0'))
	  .arg(sec, 4, 'f', 1, QChar('0'));
}

static QString deg2DMM(double val)
{
	int deg = val;
	double r1 = val - deg;

	double min = r1 * 60.0;

	return QString("%1°%2'").arg(deg).arg(min, 6, 'f', 3, QChar('0'));
}


QString Format::timeSpan(qreal time, bool full)
{
	unsigned h, m, s;

	h = time / 3600;
	m = (time - (h * 3600)) / 60;
	s = time - (h * 3600) - (m * 60);

	if (full || h)
		return QString("%1:%2:%3").arg(h, 2, 10, QChar('0'))
		  .arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
	else
		return QString("%1:%2").arg(m, 2, 10, QChar('0'))
		  .arg(s, 2, 10, QChar('0'));
}

QString Format::distance(qreal value, Units units)
{
	QLocale l(QLocale::system());

	if (units == Imperial) {
		if (value < MIINM)
			return l.toString(value * M2FT, 'f', 0) + UNIT_SPACE
			  + qApp->translate("Format", "ft");
		else if (value < 10 * MIINM)
			return l.toString(value * M2MI, 'f', 2) + UNIT_SPACE
			  + qApp->translate("Format", "mi");
		else
			return l.toString(value * M2MI, 'f', 1) + UNIT_SPACE
			  + qApp->translate("Format", "mi");
	} else if (units == Nautical) {
		if (value < NMIINM)
			return l.toString(value * M2FT, 'f', 0) + UNIT_SPACE
			  + qApp->translate("Format", "ft");
		else if (value < 10 * NMIINM)
			return l.toString(value * M2NMI, 'f', 2) + UNIT_SPACE
			  + qApp->translate("Format", "nmi");
		else
			return l.toString(value * M2NMI, 'f', 1) + UNIT_SPACE
			  + qApp->translate("Format", "nmi");
	} else {
		if (value < KMINM)
			return l.toString(value, 'f', 0) + UNIT_SPACE
			  + qApp->translate("Format", "m");
		else if (value < 10 * KMINM)
			return l.toString(value * M2KM, 'f', 2) + UNIT_SPACE
			  + qApp->translate("Format", "km");
		else
			return l.toString(value * M2KM, 'f', 1) + UNIT_SPACE
			  + qApp->translate("Format", "km");
	}
}

QString Format::elevation(qreal value, Units units)
{
	QLocale l(QLocale::system());

	if (units == Metric)
		return l.toString(qRound(value)) + UNIT_SPACE
		  + qApp->translate("Format", "m");
	else
		return l.toString(qRound(value * M2FT)) + UNIT_SPACE
		  + qApp->translate("Format", "ft");
}


QString Format::lon(const Coordinates &c, CoordinatesFormat type)
{
	QChar xH = (c.lon() < 0) ? 'W' : 'E';

	switch (type) {
		case DegreesMinutes:
			return deg2DMM(qAbs(c.lon())) + xH;
		case DMS:
			return deg2DMS(qAbs(c.lon())) + xH;
		default:
			QLocale l(QLocale::system());
			return l.toString(qAbs(c.lon()), 'f', 5) + xH;
	}
}

QString Format::lat(const Coordinates &c, CoordinatesFormat type)
{
	QChar yH = (c.lat() < 0) ? 'S' : 'N';

	switch (type) {
		case DegreesMinutes:
			return deg2DMM(qAbs(c.lat())) + yH;
		case DMS:
			return deg2DMS(qAbs(c.lat())) + yH;
		default:
			QLocale l(QLocale::system());
			return l.toString(qAbs(c.lat()), 'f', 5) + yH;
	}
}

QString Format::coordinates(const Coordinates &c, CoordinatesFormat type)
{
	return lat(c, type) + "," + QChar(0x00A0) + lon(c, type);
}
