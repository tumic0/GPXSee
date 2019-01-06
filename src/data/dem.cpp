#include <QtEndian>
#include "common/coordinates.h"
#include "dem.h"

#define SRTM3_SAMPLES 1201
#define SRTM1_SAMPLES 3601

#define SRTM_SIZE(samples) \
	((samples) * (samples) * 2)


static qreal height(const Coordinates &c, const QByteArray data)
{
	int samples;

	if (data.size() == SRTM_SIZE(SRTM3_SAMPLES))
		samples = SRTM3_SAMPLES;
	else if (data.size() == SRTM_SIZE(SRTM1_SAMPLES))
		samples = SRTM1_SAMPLES;
	else
		return NAN;

	int lat = int(qRound((c.lat() - int(c.lat())) * (samples - 1)));
	int lon = int(qRound((c.lon() - int(c.lon())) * (samples - 1)));

	int pos = ((samples - 1 - lat) * samples + lon) * 2;
	qint16 val = qFromBigEndian(*((const qint16*)(data.constData() + pos)));

	return (val == -32768) ? NAN : val;
}

QString DEM::fileName(const Key &key) const
{
	const char ns = (key.lat >= 0) ? 'N' : 'S';
	const char ew = (key.lon >= 0) ? 'E' : 'W';

	QString basename = QString("%1%2%3%4.hgt").arg(ns)
	  .arg(qAbs(key.lat), 2, 10, QChar('0')).arg(ew)
	  .arg(qAbs(key.lon), 3, 10, QChar('0'));
	return _dir.absoluteFilePath(basename);
}

qreal DEM::elevation(const Coordinates &c)
{
	Key k((int)c.lon(), (int)c.lat());

	QMap<Key, QByteArray>::const_iterator it(_data.find(k));
	if (it == _data.constEnd()) {
		QFile file(fileName(k));
		if (!file.open(QIODevice::ReadOnly))
			return NAN;
		else {
			it = _data.insert(k, file.readAll());
			return height(c, *it);
		}
	} else
		return height(c, *it);
}
