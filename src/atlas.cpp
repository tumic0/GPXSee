#include <QDir>
#include <QtAlgorithms>
#include <QPainter>
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

bool Atlas::isAtlas(const QFileInfoList &files)
{
	for (int i = 0; i < files.count(); i++) {
		const QString &fileName = files.at(i).fileName();
		if (fileName.endsWith(".tar")) {
			if (!_tar.load(files.at(i).absoluteFilePath())) {
				qWarning("%s: %s: error loading tar file", qPrintable(_name),
				  qPrintable(fileName));
				return false;
			}
			QStringList tarFiles = _tar.files();
			for (int j = 0; j < tarFiles.size(); j++)
				if (tarFiles.at(j).endsWith(".tba"))
					return true;
		} else if (fileName.endsWith(".tba"))
			return true;
	}

	return false;
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
		qreal w = 0, h = 0;

		QList<OfflineMap*> m;
		for (int i = _zooms.at(z).first; i <= _zooms.at(z).second; i++)
			m.append(_maps.at(i));

		qSort(m.begin(), m.end(), xCmp);
		offsets[_maps.indexOf(m.first())].setX(w);
		for (int i = 1; i < m.size(); i++) {
			w += round(m.at(i-1)->pp2xy(TL(m.at(i))).x());
			offsets[_maps.indexOf(m.at(i))].setX(w);
		}

		qSort(m.begin(), m.end(), yCmp);
		offsets[_maps.indexOf(m.first())].setY(h);
		for (int i = 1; i < m.size(); i++) {
			h += round(m.at(i-1)->pp2xy(TL(m.at(i))).y());
			offsets[_maps.indexOf(m.at(i))].setY(h);
		}
	}

	for (int i = 0; i < _maps.count(); i++)
		_bounds.append(QPair<QRectF, QRectF>(QRectF(TL(_maps.at(i)),
		  BR(_maps.at(i))), QRectF(offsets.at(i), _maps.at(i)->bounds().size())));
}

Atlas::Atlas(const QString &path, QObject *parent) : Map(parent)
{
	_valid = false;
	_zoom = 0;

	QFileInfo fi(path);
	_name = fi.fileName();

	QDir dir(path);
	QFileInfoList files = dir.entryInfoList(QDir::Files);
	if (!isAtlas(files))
		return;

	QFileInfoList layers = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (int n = 0; n < layers.count(); n++) {
		QDir zdir(layers.at(n).absoluteFilePath());
		QFileInfoList maps = zdir.entryInfoList(QDir::Dirs
		  | QDir::NoDotAndDotDot);
		for (int i = 0; i < maps.count(); i++) {
			OfflineMap *map;
			if (_tar.isOpen())
				map = new OfflineMap(_tar, maps.at(i).absoluteFilePath(), this);
			else
				map = new OfflineMap(maps.at(i).absoluteFilePath(), this);
			if (map->isValid())
				_maps.append(map);
		}
	}
	if (_maps.isEmpty()) {
		qWarning("%s: No usable maps available", qPrintable(_name));
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

qreal Atlas::zoomFit(const QSize &size, const QRectF &br)
{
	_zoom = 0;

	for (int z = 0; z < _zooms.count(); z++) {
		for (int i = _zooms.at(z).first; i <= _zooms.at(z).second; i++) {
			if (_bounds.at(i).first.contains(_maps.at(i)->ll2pp(br.center())))
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

QPointF Atlas::ll2xy(const Coordinates &c) const
{
	int idx = _zooms.at(_zoom).first;

	for (int i = _zooms.at(_zoom).first; i <= _zooms.at(_zoom).second; i++) {
		if (_bounds.at(i).first.contains(_maps.at(i)->ll2pp(c))) {
			idx = i;
			break;
		}
	}

	QPointF p = _maps.at(idx)->ll2xy(c);
	return p + _bounds.at(idx).second.topLeft();
}

Coordinates Atlas::xy2ll(const QPointF &p) const
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
		QRectF ir = rect.intersected(_bounds.at(i).second);
		if (ir == rect) {
			draw(painter, rect, i);
			return;
		}
	}

	// Multiple maps
	painter->fillRect(rect, Qt::white);
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
