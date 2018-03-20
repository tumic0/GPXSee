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
	if (units == Imperial) {
		if (value < MIINM)
			return QString::number(value * M2FT, 'f', 0) + UNIT_SPACE
			  + qApp->translate("Format", "ft");
		else
			return QString::number(value * M2MI, 'f', 1) + UNIT_SPACE
			  + qApp->translate("Format", "mi");
	} else if (units == Nautical) {
		if (value < NMIINM)
			return QString::number(value * M2FT, 'f', 0) + UNIT_SPACE
			  + qApp->translate("Format", "ft");
		else
			return QString::number(value * M2NMI, 'f', 1) + UNIT_SPACE
			  + qApp->translate("Format", "nmi");
	} else {
		if (value < KMINM)
			return QString::number(value, 'f', 0) + UNIT_SPACE
			  + qApp->translate("Format", "m");
		else
			return QString::number(value * M2KM, 'f', 1) + UNIT_SPACE
			  + qApp->translate("Format", "km");
	}
}

QString Format::elevation(qreal value, Units units)
{
	if (units == Metric)
		return QString::number(qRound(value)) + UNIT_SPACE
		  + qApp->translate("Format", "m");
	else
		return QString::number(qRound(value * M2FT)) + UNIT_SPACE
		  + qApp->translate("Format", "ft");
}

QString Format::coordinates(const Coordinates &value, CoordinatesFormat type)
{
	QChar yH = (value.lat() < 0) ? 'S' : 'N';
	QChar xH = (value.lon() < 0) ? 'W' : 'E';

	switch (type) {
		case DegreesMinutes:
			return deg2DMM(qAbs(value.lat())) + yH + "," + QChar(0x00A0)
			  + deg2DMM(qAbs(value.lon())) + xH;
			break;
		case DMS:
			return deg2DMS(qAbs(value.lat())) + yH + "," + QChar(0x00A0)
			  + deg2DMS(qAbs(value.lon())) + xH;
			break;
		default:
			return QString::number(qAbs(value.lat()), 'f', 5) + yH + ","
			  + QChar(0x00A0) + QString::number(qAbs(value.lon()), 'f', 5) + xH;
	}
}
