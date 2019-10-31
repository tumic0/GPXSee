#include <QtEndian>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtCore/qmath.h>
#else // QT5
#include <QtMath>
#endif // QT5
#include <QDir>
#include <QFile>
#include "common/coordinates.h"
#include "dem.h"


#define SRTM3_SAMPLES 1201
#define SRTM1_SAMPLES 3601

#define SRTM_SIZE(samples) \
	((samples) * (samples) * 2)

static qreal interpolate(qreal dx, qreal dy, qreal p0, qreal p1, qreal p2,
  qreal p3)
{
	return p0 * (1 - dx) * (1 - dy) + p1 * dx * (1 - dy) + p2 * dy * (1 - dx)
	  + p3 * dx * dy;
}

static qreal value(int col, int row, int samples, const QByteArray *data)
{
	int pos = ((samples - 1 - row) * samples + col) * 2;
	qint16 val = qFromBigEndian(*((const qint16*)(data->constData() + pos)));

	return (val == -32768) ? NAN : val;
}

static qreal height(const Coordinates &c, const QByteArray *data)
{
	int samples;

	if (data->size() == SRTM_SIZE(SRTM3_SAMPLES))
		samples = SRTM3_SAMPLES;
	else if (data->size() == SRTM_SIZE(SRTM1_SAMPLES))
		samples = SRTM1_SAMPLES;
	else
		return NAN;

	int row = (int)((c.lat() - qFloor(c.lat())) * (samples - 1));
	int col = (int)((c.lon() - qFloor(c.lon())) * (samples - 1));
	qreal dx = ((c.lon() - qFloor(c.lon())) * (samples - 1)) - col;
	qreal dy = ((c.lat() - qFloor(c.lat())) * (samples - 1)) - row;

	qreal p0 = value(col, row, samples, data);
	qreal p1 = value(col + 1, row, samples, data);
	qreal p2 = value(col, row + 1, samples, data);
	qreal p3 = value(col + 1, row + 1, samples, data);

	return interpolate(dx, dy, p0, p1, p2, p3);
}


QString DEM::_dir;
QCache<DEM::Key, QByteArray> DEM::_data;

QString DEM::fileName(const Key &key)
{
	const char ns = (key.lat() >= 0) ? 'N' : 'S';
	const char ew = (key.lon() >= 0) ? 'E' : 'W';

	QString basename = QString("%1%2%3%4.hgt").arg(ns)
	  .arg(qAbs(key.lat()), 2, 10, QChar('0')).arg(ew)
	  .arg(qAbs(key.lon()), 3, 10, QChar('0'));
	return QDir(_dir).absoluteFilePath(basename);
}

void DEM::setDir(const QString &path)
{
	_dir = path;
}

qreal DEM::elevation(const Coordinates &c)
{
	if (_dir.isEmpty())
		return NAN;

	Key k(qFloor(c.lon()), qFloor(c.lat()));

	QByteArray *ba = _data[k];
	if (!ba) {
		QFile file(fileName(k));
		if (!file.open(QIODevice::ReadOnly)) {
			qWarning("%s: %s", qPrintable(file.fileName()),
			  qPrintable(file.errorString()));
			_data.insert(k, new QByteArray());
			return NAN;
		} else {
			ba = new QByteArray(file.readAll());
			qreal ele = height(c, ba);
			_data.insert(k, ba);
			return ele;
		}
	} else
		return height(c, ba);
}
