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
#include "common/rectc.h"
#include "dem.h"


static unsigned int isqrt(size_t x)
{
	size_t l = 0;
	size_t m;
	size_t r = x + 1;

	while (l != r - 1) {
		m = (l + r) / 2;

		if (m * m <= x)
			l = m;
		else
			r = m;
	}

	return (unsigned int)l;
}

static double interpolate(double dx, double dy, double p0, double p1, double p2,
  double p3)
{
	return p0 * (1.0 - dx) * (1.0 - dy) + p1 * dx * (1.0 - dy)
	  + p2 * dy * (1.0 - dx) + p3 * dx * dy;
}

static double value(int col, int row, int samples, const QByteArray &data)
{
	int pos = ((samples - 1 - row) * samples + col) * sizeof(qint16);
	qint16 val = qFromBigEndian(*((const qint16*)(data.constData() + pos)));

	return (val == -32768) ? NAN : val;
}


DEM::Entry::Entry(const QByteArray &data) : _data(data)
{
	_samples = isqrt(_data.size() / sizeof(qint16));
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

QString DEM::Tile::fileName() const
{
	return QString("%1%2.hgt").arg(latStr(), lonStr());
}


QMutex DEM::_lock;
QString DEM::_dir;
DEM::TileCache DEM::_data;

void DEM::setCacheSize(int size)
{
	_lock.lock();
	_data.setMaxCost(size);
	_lock.unlock();
}

void DEM::setDir(const QString &path)
{
	_dir = path;
}

void DEM::clearCache()
{
	_lock.lock();
	_data.clear();
	_lock.unlock();
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
	QString fileName(tile.fileName());
	QString path(QDir(_dir).absoluteFilePath(fileName));
	QString zipPath(path + ".zip");

	if (QFileInfo::exists(zipPath)) {
		QZipReader zip(zipPath, QIODevice::ReadOnly);
		return new Entry(zip.fileData(fileName));
	} else {
		QFile file(path);
		if (!file.open(QIODevice::ReadOnly)) {
			qWarning("%s: %s", qUtf8Printable(file.fileName()),
			  qUtf8Printable(file.errorString()));
			return new Entry();
		} else
			return new Entry(file.readAll());
	}
}

double DEM::elevationLockFree(const Coordinates &c)
{
	Tile tile(floor(c.lon()), floor(c.lat()));
	Entry *e = _data.object(tile);
	double ele;

	if (!e) {
		e = loadTile(tile);
		ele = height(c, e);
		_data.insert(tile, e, e->data().size() / 1024);
	} else
		ele = height(c, e);

	return ele;
}

double DEM::elevation(const Coordinates &c)
{
	if (_dir.isEmpty())
		return NAN;

	_lock.lock();
	double ele = elevationLockFree(c);
	_lock.unlock();

	return ele;
}

MatrixD DEM::elevation(const MatrixC &m)
{
	if (_dir.isEmpty())
		return MatrixD(m.h(), m.w(), NAN);

	MatrixD ret(m.h(), m.w());

	_lock.lock();
	for (int i = 0; i < m.size(); i++)
		ret.at(i) = elevationLockFree(m.at(i));
	_lock.unlock();

	return ret;
}

bool DEM::elevation(const RectC &rect)
{
	if (_dir.isEmpty())
		return false;

	QDir dir(_dir);
	int left = floor(rect.left());
	int top = floor(rect.top());
	int right = floor(rect.right());
	int bottom = floor(rect.bottom());

	for (int i = bottom; i <= top; i++) {
		for (int j = left; j <= right; j++) {
			QString path(dir.absoluteFilePath(Tile(j, i).fileName()));
			if (QFileInfo::exists(path) || QFileInfo::exists(path + ".zip"))
				return true;
		}
	}

	return false;
}

QList<Area> DEM::tiles()
{
	static const QRegularExpression re(
	  "^([NS])([0-9]{2})([EW])([0-9]{3})(\\.hgt|\\.hgt\\.zip)$");
	QDir dir(_dir);
	QFileInfoList files(dir.entryInfoList(QDir::Files | QDir::Readable));
	QLocale l(QLocale::system());
	QList<Area> list;

	for (int i = 0; i < files.size(); i++) {
		QRegularExpressionMatch match(re.match(files.at(i).fileName()));
		if (!match.hasMatch())
			continue;

		int lat = match.captured(2).toInt();
		int lon = match.captured(4).toInt();
		if (match.captured(1) == "S")
			lat = -lat;
		if (match.captured(3) == "W")
			lon = -lon;

		Area area(RectC(Coordinates(lon, lat + 1), Coordinates(lon + 1, lat)));
		area.setName(files.at(i).baseName());
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
	dbg.nospace() << "Tile(" << tile.fileName() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
