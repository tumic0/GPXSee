#include <QApplication>
#include "coordinates.h"
#include "format.h"

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
		return QString::number(value, 'f', 0) + UNIT_SPACE
		  + qApp->translate("Format", "m");
	else
		return QString::number(value * M2FT, 'f', 0) + UNIT_SPACE
		  + qApp->translate("Format", "ft");
}

QString Format::coordinates(const Coordinates &value)
{
	QChar yH = (value.lat() < 0) ? 'S' : 'N';
	QChar xH = (value.lon() < 0) ? 'W' : 'E';

	return QString::number(qAbs(value.lat()), 'f', 5) + yH + "," + QChar(0x00A0)
	  + QString::number(qAbs(value.lon()), 'f', 5) + xH;
}
