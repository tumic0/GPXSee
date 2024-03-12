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
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QLocale>
#include <private/qzipreader_p.h>
#include "rectc.h"
#include "dem.h"


static unsigned int isqrt(unsigned int x)
{
	unsigned int r = 0;

	while ((r + 1) * (r + 1) <= x)
		r++;

	return r;
}

static double interpolate(double dx, double dy, double p0, double p1, double p2,
  double p3)
{
	return p0 * (1.0 - dx) * (1.0 - dy) + p1 * dx * (1.0 - dy)
	  + p2 * dy * (1.0 - dx) + p3 * dx * dy;
}

static double value(int col, int row, int samples, const QByteArray &data)
{
	int pos = ((samples - 1 - row) * samples + col) * 2;
	qint16 val = qFromBigEndian(*((const qint16*)(data.constData() + pos)));

	return (val == -32768) ? NAN : val;
}

QMutex DEM::_lock;

DEM::Entry::Entry(const QByteArray &data) : _data(data)
{
	_samples = isqrt(_data.size() / 2);
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

double DEM::height(const Coordinates &c, const Entry *e)
{
	if (!e->samples())
		return NAN;

	double lat = (c.lat() - floor(c.lat())) * (e->samples() - 1);
	double lon = (c.lon() - floor(c.lon())) * (e->samples() - 1);
	int row = (int)lat;
	int col = (int)lon;

	double p0 = value(col, row, e->samples(), e->data());
	double p1 = value(col + 1, row, e->samples(), e->data());
	double p2 = value(col, row + 1, e->samples(), e->data());
	double p3 = value(col + 1, row + 1, e->samples(), e->data());

	return interpolate(lon - col, lat - row, p0, p1, p2, p3);
}

DEM::Entry *DEM::loadTile(const Tile &tile)
{
	QString bn(tile.baseName());
	QString fn(QDir(_dir).absoluteFilePath(bn));
	QString zn(fn + ".zip");

	if (QFileInfo::exists(zn)) {
		QZipReader zip(zn, QIODevice::ReadOnly);
		return new Entry(zip.fileData(bn));
	} else {
		QFile file(fn);
		if (!file.open(QIODevice::ReadOnly)) {
			qWarning("%s: %s", qPrintable(file.fileName()),
			  qPrintable(file.errorString()));
			return new Entry();
		} else
			return new Entry(file.readAll());
	}
}

double DEM::elevation(const Coordinates &c)
{
	if (_dir.isEmpty())
		return NAN;

	Tile tile(floor(c.lon()), floor(c.lat()));
	Entry *e = _data.object(tile);
	double ele;

	if (!e) {
		e = loadTile(tile);
		ele = height(c, e);
		_data.insert(tile, e, e->data().size());
	} else
		ele = height(c, e);

	return ele;
}

QList<Area> DEM::tiles()
{
	static const QRegularExpression re("([NS])([0-9]{2})([EW])([0-9]{3})");
	QDir dir(_dir);
	QFileInfoList files(dir.entryInfoList(QDir::Files | QDir::Readable));
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
		area.setStyle(PolygonStyle(QColor(0xFF, 0, 0, 0x40),
		  QColor(0xFF, 0, 0, 0x80), 2));

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
