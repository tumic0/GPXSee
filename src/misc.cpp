#include <cmath>
#include <QObject>
#include "misc.h"

double niceNum(double x, int round)
{
	int expv;
	double f;
	double nf;

	expv = (int)floor(log10(x));
	f = x / pow(10.0, expv);

	if (round) {
		if (f < 1.5)
			nf = 1.0;
		else if (f < 3.0)
			nf = 2.0;
		else if (f < 7.0)
			nf = 5.0;
		else
			nf = 10.0;
	} else {
		if (f <= 1.0)
			nf = 1.0;
		else if (f <= 2.0)
			nf = 2.0;
		else if (f <= 5.0)
			nf = 5.0;
		else
			nf = 10.0;
	}

	return nf * pow(10.0, expv);
}

QString timeSpan(qreal time)
{
	unsigned h, m, s;

	h = time / 3600;
	m = (time - (h * 3600)) / 60;
	s = time - (h * 3600) - (m * 60);

	return QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0'))
	  .arg(s, 2, 10, QChar('0'));
}

QString distance(qreal value, Units units)
{
	if (units == Imperial) {
		if (value < MIINM)
			return QString::number(value * M2FT, 'f', 0)
			  + UNIT_SPACE + QObject::tr("ft");
		else
			return QString::number(value * M2MI, 'f', 1)
			  + UNIT_SPACE + QObject::tr("mi");
	} else {
		if (value < KMINM)
			return QString::number(value, 'f', 0) + UNIT_SPACE
			  + QObject::tr("m");
		else
			return QString::number(value * M2KM, 'f', 1)
			  + UNIT_SPACE + QObject::tr("km");
	}
}
