/*
	WARNING: This code uses internal Qt API - the QZipReader class for reading
	ZIP files - and things may break if Qt changes the API. For Qt5 this is not
	a problem as we can "see the future" now and there are no changes in all
	the supported Qt5 versions up to the last one (5.15). In Qt6 the class
	might change or even disappear in the future, but this is very unlikely
	as there were no changes for several years and The Qt Company's policy
	is: "do not invest any resources into any desktop related stuff unless
	absolutely necessary". There is an issue (QTBUG-3897) since the year 2009 to
	include the ZIP reader into the public API, which aptly illustrates the
	effort The Qt Company is willing to make about anything desktop related...
*/

#include <QtEndian>
#include <QtMath>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QLocale>
#include <private/qzipreader_p.h>
#include "common/rectc.h"
#include "dem.h"

#define SRTM3_SAMPLES  1201
#define SRTM1_SAMPLES  3601
#define SRTM05_SAMPLES 7201

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
	else if (data->size() == SRTM_SIZE(SRTM05_SAMPLES))
		samples = SRTM05_SAMPLES;
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

QString DEM::Tile::latStr() const
{
	const char ns = (_lat >= 0) ? 'N' : 'S';
	return QString("%1%2").arg(ns).arg(qAbs(_lat), 2, 10, QChar('0'));
}

QString DEM::Tile::lonStr() const
{
	const char ew = (_lon >= 0) ? 'E' : 'W';
	return QString("%1%2").arg(ew).arg(qAbs(_lon), 3, 10, QChar('0'));
}

QString DEM::Tile::baseName() const
{
	return QString("%1%2.hgt").arg(latStr(), lonStr());
}

QString DEM::_dir;
DEM::TileCache DEM::_data;

QString DEM::fileName(const QString &baseName)
{
	return QDir(_dir).absoluteFilePath(baseName);
}

void DEM::setCacheSize(int size)
{
	_data.setMaxCost(size * 1024);
}

void DEM::setDir(const QString &path)
{
	_dir = path;
}

void DEM::clearCache()
{
	_data.clear();
}

qreal DEM::elevation(const Coordinates &c)
{
	if (_dir.isEmpty())
		return NAN;

	Tile tile(qFloor(c.lon()), qFloor(c.lat()));

	QByteArray *ba = _data[tile];
	if (!ba) {
		QString bn(tile.baseName());
		QString fn(fileName(bn));
		QString zn(fn + ".zip");

		if (QFileInfo::exists(zn)) {
			QZipReader zip(zn, QIODevice::ReadOnly);
			ba = new QByteArray(zip.fileData(bn));
			qreal ele = height(c, ba);
			_data.insert(tile, ba, ba->size());
			return ele;
		} else {
			QFile file(fn);
			if (!file.open(QIODevice::ReadOnly)) {
				qWarning("%s: %s", qPrintable(file.fileName()),
				  qPrintable(file.errorString()));
				_data.insert(tile, new QByteArray());
				return NAN;
			} else {
				ba = new QByteArray(file.readAll());
				qreal ele = height(c, ba);
				_data.insert(tile, ba, ba->size());
				return ele;
			}
		}
	} else
		return height(c, ba);
}

QList<Area> DEM::tiles()
{
	QDir dir(_dir);
	QFileInfoList files(dir.entryInfoList(QDir::Files | QDir::Readable));
	QRegularExpression re("([NS])([0-9]{2})([EW])([0-9]{3})");
	QLocale l(QLocale::system());
	QList<Area> list;

	for (int i = 0; i < files.size(); i++) {
		QString basename(files.at(i).baseName());
		QRegularExpressionMatch match(re.match(basename));
		if (!match.hasMatch())
			continue;

		int lat = match.captured(2).toInt();
		int lon = match.captured(4).toInt();
		if (match.captured(1) == "S")
			lat = -lat;
		if (match.captured(3) == "W")
			lon = -lon;

		Area area(RectC(Coordinates(lon, lat + 1), Coordinates(lon + 1, lat)));
		area.setName(basename);
		area.setDescription(files.at(i).suffix().toUpper() + ", "
		  + l.formattedDataSize(files.at(i).size()));

		list.append(area);
	}

	return list;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const DEM::Tile &tile)
{
	dbg.nospace() << "Tile(" << tile.baseName() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
