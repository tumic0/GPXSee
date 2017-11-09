#include <QDir>
#include <QtAlgorithms>
#include <QPainter>
#include "rectc.h"
#include "tar.h"
#include "atlas.h"


#define ZOOM_THRESHOLD 0.9

#define TL(m) ((m)->xy2pp((m)->bounds().topLeft()))
#define BR(m) ((m)->xy2pp((m)->bounds().bottomRight()))

static bool resCmp(const OfflineMap *m1, const OfflineMap *m2)
{
	qreal r1, r2;

	r1 = m1->resolution(m1->bounds().center());
	r2 = m2->resolution(m2->bounds().center());

	return r1 > r2;
}

static bool xCmp(const OfflineMap *m1, const OfflineMap *m2)
{
	return TL(m1).x() < TL(m2).x();
}

static bool yCmp(const OfflineMap *m1, const OfflineMap *m2)
{
	return TL(m1).y() > TL(m2).y();
}

bool Atlas::isAtlas(Tar &tar, const QString &path)
{
	QFileInfo fi(path);
	QByteArray ba;
	QString suffix = fi.suffix().toLower();

	if (suffix == "tar") {
		if (!tar.load(path)) {
			_errorString = "Error reading tar file";
			return false;
		}
		QString tbaFileName = fi.completeBaseName() + ".tba";
		ba = tar.file(tbaFileName);
	} else if (suffix == "tba") {
		QFile tbaFile(path);
		if (!tbaFile.open(QIODevice::ReadOnly)) {
			_errorString = QString("Error opening tba file: %1")
			  .arg(tbaFile.errorString());
			return false;
		}
		ba = tbaFile.readAll();
	}

	if (ba.startsWith("Atlas 1.0"))
		return true;
	else {
		_errorString = "Missing or invalid tba file";
		return false;
	}
}

void Atlas::computeZooms()
{
	qSort(_maps.begin(), _maps.end(), resCmp);

	_zooms.append(QPair<int, int>(0, _maps.count() - 1));
	for (int i = 1; i < _maps.count(); i++) {
		qreal last = _maps.at(i-1)->resolution(_maps.at(i)->bounds().center());
		qreal cur = _maps.at(i)->resolution(_maps.at(i)->bounds().center());
		if (cur < last * ZOOM_THRESHOLD) {
			_zooms.last().second = i-1;
			_zooms.append(QPair<int, int>(i, _maps.count() - 1));
		}
	}
}

void Atlas::computeBounds()
{
	QList<QPointF> offsets;

	for (int i = 0; i < _maps.count(); i++)
		offsets.append(QPointF());

	for (int z = 0; z < _zooms.count(); z++) {
		QList<OfflineMap*> m;
		for (int i = _zooms.at(z).first; i <= _zooms.at(z).second; i++)
			m.append(_maps.at(i));

		qSort(m.begin(), m.end(), xCmp);
		offsets[_maps.indexOf(m.first())].setX(0);
		for (int i = 1; i < m.size(); i++) {
			qreal w = round(m.first()->pp2xy(TL(m.at(i))).x());
			offsets[_maps.indexOf(m.at(i))].setX(w);
		}

		qSort(m.begin(), m.end(), yCmp);
		offsets[_maps.indexOf(m.first())].setY(0);
		for (int i = 1; i < m.size(); i++) {
			qreal h = round(m.first()->pp2xy(TL(m.at(i))).y());
			offsets[_maps.indexOf(m.at(i))].setY(h);
		}
	}

	for (int i = 0; i < _maps.count(); i++)
		_bounds.append(QPair<QRectF, QRectF>(QRectF(TL(_maps.at(i)),
		  BR(_maps.at(i))), QRectF(offsets.at(i), _maps.at(i)->bounds().size())));
}

Atlas::Atlas(const QString &fileName, QObject *parent) : Map(parent)
{
	Tar tar;
	QFileInfo fi(fileName);

	_valid = false;
	_zoom = 0;
	_name = fi.dir().dirName();
	_ci = -1; _cz = -1;

	if (!isAtlas(tar, fileName))
		return;

	QDir dir(fi.absolutePath());
	QFileInfoList layers = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (int n = 0; n < layers.count(); n++) {
		QDir zdir(layers.at(n).absoluteFilePath());
		QFileInfoList maps = zdir.entryInfoList(QDir::Dirs
		  | QDir::NoDotAndDotDot);
		for (int i = 0; i < maps.count(); i++) {
			QString mapFile = maps.at(i).absoluteFilePath() + "/"
			  + maps.at(i).fileName() + ".map";

			OfflineMap *map;
			if (tar.isOpen())
				map = new OfflineMap(mapFile, tar, this);
			else
				map = new OfflineMap(mapFile, this);

			if (map->isValid())
				_maps.append(map);
			else {
				_errorString = QString("Error loading map: %1: %2")
				  .arg(mapFile, map->errorString());
				return;
			}
		}
	}
	if (_maps.isEmpty()) {
		_errorString = "No maps found in atlas";
		return;
	}

	computeZooms();
	computeBounds();

	_valid = true;
}

Atlas::~Atlas()
{
	for (int i = 0; i < _maps.size(); i++)
		delete _maps.at(i);
}

QRectF Atlas::bounds() const
{
	QSizeF s(0, 0);

	for (int i = _zooms.at(_zoom).first; i <= _zooms.at(_zoom).second; i++) {
		if (_bounds.at(i).second.right() > s.width())
			s.setWidth(_bounds.at(i).second.right());
		if (_bounds.at(i).second.bottom() > s.height())
			s.setHeight(_bounds.at(i).second.bottom());
	}

	return QRectF(QPointF(0, 0), s);
}

qreal Atlas::resolution(const QPointF &p) const
{
	int idx = _zooms.at(_zoom).first;

	for (int i = _zooms.at(_zoom).first; i <= _zooms.at(_zoom).second; i++) {
		if (_bounds.at(i).second.contains(_maps.at(i)->xy2pp(p))) {
			idx = i;
			break;
		}
	}

	return _maps.at(idx)->resolution(p);
}

qreal Atlas::zoom() const
{
	return _zoom;
}

qreal Atlas::zoomFit(const QSize &size, const RectC &br)
{
	_zoom = 0;

	if (!br.isValid()) {
		_zoom = _zooms.size() - 1;
		return _zoom;
	}

	for (int z = 0; z < _zooms.count(); z++) {
		for (int i = _zooms.at(z).first; i <= _zooms.at(z).second; i++) {
			if (!_bounds.at(i).first.contains(_maps.at(i)->ll2pp(br.center())))
				continue;

			QRect sbr = QRectF(_maps.at(i)->ll2xy(br.topLeft()),
			  _maps.at(i)->ll2xy(br.bottomRight())).toRect().normalized();

			if (sbr.size().width() > size.width()
			  || sbr.size().height() > size.height())
				return _zoom;

			_zoom = z;
			break;
		}
	}

	return _zoom;
}

qreal Atlas::zoomFit(qreal resolution, const Coordinates &c)
{
	_zoom = 0;

	for (int z = 0; z < _zooms.count(); z++) {
		for (int i = _zooms.at(z).first; i <= _zooms.at(z).second; i++) {
			if (!_bounds.at(i).first.contains(_maps.at(i)->ll2pp(c)))
				continue;

			if (_maps.at(i)->resolution(_maps.at(i)->ll2xy(c)) < resolution)
				return _zoom;

			_zoom = z;
			break;
		}
	}

	return _zoom;
}

qreal Atlas::zoomIn()
{
	_zoom = qMin(_zoom + 1, _zooms.size() - 1);
	return _zoom;
}

qreal Atlas::zoomOut()
{
	_zoom = qMax(_zoom - 1, 0);
	return _zoom;
}

QPointF Atlas::ll2xy(const Coordinates &c)
{
	QPointF pp;

	if (_cz != _zoom) {
		_ci = -1;
		_cz = _zoom;
	}

	if (_ci >= 0)
		pp = _maps.at(_ci)->ll2pp(c);
	if (_ci < 0 || !_bounds.at(_ci).first.contains(pp)) {
		_ci = _zooms.at(_zoom).first;
		for (int i = _zooms.at(_zoom).first; i <= _zooms.at(_zoom).second; i++) {
			pp = _maps.at(i)->ll2pp(c);
			if (_bounds.at(i).first.contains(pp)) {
				_ci = i;
				break;
			}
		}
	}

	QPointF p = _maps.at(_ci)->pp2xy(pp);
	return p + _bounds.at(_ci).second.topLeft();
}

Coordinates Atlas::xy2ll(const QPointF &p)
{
	int idx = _zooms.at(_zoom).first;

	for (int i = _zooms.at(_zoom).first; i <= _zooms.at(_zoom).second; i++) {
		if (_bounds.at(i).second.contains(_maps.at(i)->xy2pp(p))) {
			idx = i;
			break;
		}
	}

	QPointF p2 = p - _bounds.at(idx).second.topLeft();
	return _maps.at(idx)->xy2ll(p2);
}

void Atlas::draw(QPainter *painter, const QRectF &rect)
{
	// All in one map
	for (int i = _zooms.at(_zoom).first; i <= _zooms.at(_zoom).second; i++) {
		if (_bounds.at(i).second.contains(rect)) {
			draw(painter, rect, i);
			return;
		}
	}

	// Multiple maps
	painter->fillRect(rect, _backgroundColor);
	for (int i = _zooms.at(_zoom).first; i <= _zooms.at(_zoom).second; i++) {
		QRectF ir = rect.intersected(_bounds.at(i).second);
		if (!ir.isNull())
			draw(painter, ir, i);
	}
}

void Atlas::draw(QPainter *painter, const QRectF &rect, int mapIndex)
{
	OfflineMap *map = _maps.at(mapIndex);
	const QPointF offset = _bounds.at(mapIndex).second.topLeft();
	QRectF pr = QRectF(rect.topLeft() - offset, rect.size());

	map->load();

	painter->translate(offset);
	map->draw(painter, pr);
	painter->translate(-offset);
}

void Atlas::unload()
{
	for (int i = 0; i < _maps.count(); i++)
		_maps.at(i)->unload();
}
