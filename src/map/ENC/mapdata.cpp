#include <QFile>
#include "common/util.h"
#include "objects.h"
#include "attributes.h"
#include "mapdata.h"

using namespace ENC;

#define RCNM_VI 110
#define RCNM_VC 120
#define RCNM_VE 130
#define RCNM_VF 140

#define PRIM_P 1
#define PRIM_L 2
#define PRIM_A 3

static void warning(const ISO8211::Field &FRID, uint PRIM)
{
	uint RCID = 0xFFFFFFFF;
	FRID.subfield("RCID", &RCID);

	switch (PRIM) {
		case PRIM_P:
			qWarning("%u: invalid point feature", RCID);
			break;
		case PRIM_L:
			qWarning("%u: invalid line feature", RCID);
			break;
		case PRIM_A:
			qWarning("%u: invalid area feature", RCID);
			break;
	}
}

static void rectcBounds(const RectC &rect, double min[2], double max[2])
{
	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();
}

static bool parseNAME(const ISO8211::Field *f, quint8 *type, quint32 *id,
  int idx = 0)
{
	QByteArray ba(f->data().at(idx).at(0).toByteArray());
	if (ba.size() != 5)
		return false;

	*type = (quint8)(*ba.constData());
	*id = UINT32(ba.constData() + 1);

	return true;
}

static const ISO8211::Field *SGXD(const ISO8211::Record &r)
{
	const ISO8211::Field *f;

	if ((f = r.field("SG2D")))
		return f;
	else if ((f = r.field("SG3D")))
		return f;
	else
		return 0;
}

static bool pointCb(MapData::Point *point, void *context)
{
	QList<MapData::Point*> *points = (QList<MapData::Point*>*)context;
	points->append(point);
	return true;
}

static bool lineCb(MapData::Line *line, void *context)
{
	QList<MapData::Line*> *lines = (QList<MapData::Line*>*)context;
	lines->append(line);
	return true;
}

static bool polygonCb(MapData::Poly *polygon, void *context)
{
	QList<MapData::Poly*> *polygons = (QList<MapData::Poly*>*)context;
	polygons->append(polygon);
	return true;
}

static uint depthLevel(const QString &str)
{
	double minDepth = str.isEmpty() ? -1 : str.toDouble();

	if (minDepth < 0)
		return 0;
	else if (minDepth < 2)
		return 1;
	else if (minDepth < 5)
		return 2;
	else if (minDepth < 10)
		return 3;
	else if (minDepth < 20)
		return 4;
	else if (minDepth < 50)
		return 5;
	else
		return 6;
}

RectC MapData::Line::bounds() const
{
	RectC b;

	for (int i = 0; i < _path.size(); i++)
		b = b.united(_path.at(i));

	return b;
}

Coordinates MapData::coordinates(int x, int y) const
{
	return Coordinates(x / (double)_COMF, y / (double)_COMF);
}

Coordinates MapData::point(const ISO8211::Record &r)
{
	const ISO8211::Field *f = SGXD(r);
	if (!f)
		return Coordinates();

	int y = f->data().at(0).at(0).toInt();
	int x = f->data().at(0).at(1).toInt();

	return coordinates(x, y);
}

QVector<MapData::Sounding> MapData::soundings(const ISO8211::Record &r)
{
	QVector<Sounding> s;
	const ISO8211::Field *f = r.field("SG3D");
	if (!f)
		return QVector<Sounding>();

	s.reserve(f->data().size());
	for (int i = 0; i < f->data().size(); i++) {
		int y = f->data().at(i).at(0).toInt();
		int x = f->data().at(i).at(1).toInt();
		int z = f->data().at(i).at(2).toInt();
		s.append(Sounding(coordinates(x, y), z / (double)_SOMF));
	}

	return s;
}

QVector<MapData::Sounding> MapData::soundingGeometry(const ISO8211::Record &r)
{
	quint8 type;
	quint32 id;
	RecordMapIterator it;

	const ISO8211::Field *FSPT = r.field("FSPT");
	if (!FSPT || FSPT->data().at(0).size() != 4)
		return QVector<Sounding>();

	if (!parseNAME(FSPT, &type, &id))
		return QVector<Sounding>();

	if (type == RCNM_VI)
		it = _vi.find(id);
	else if (type == RCNM_VC)
		it = _vc.find(id);
	else
		return QVector<Sounding>();
	if (it == _ve.constEnd())
		return QVector<Sounding>();

	const ISO8211::Record &FRID = it.value();

	return soundings(FRID);
}

Coordinates MapData::pointGeometry(const ISO8211::Record &r)
{
	quint8 type;
	quint32 id;
	RecordMapIterator it;

	const ISO8211::Field *FSPT = r.field("FSPT");
	if (!FSPT || FSPT->data().at(0).size() != 4)
		return Coordinates();

	if (!parseNAME(FSPT, &type, &id))
		return Coordinates();

	if (type == RCNM_VI)
		it = _vi.find(id);
	else if (type == RCNM_VC)
		it = _vc.find(id);
	else
		return Coordinates();
	if (it == _ve.constEnd())
		return Coordinates();

	const ISO8211::Record &FRID = it.value();

	return point(FRID);
}

QVector<Coordinates> MapData::lineGeometry(const ISO8211::Record &r)
{
	QVector<Coordinates> path;
	Coordinates c[2];
	uint ORNT, MASK;
	quint8 type;
	quint32 id;

	const ISO8211::Field *FSPT = r.field("FSPT");
	if (!FSPT || FSPT->data().at(0).size() != 4)
		return QVector<Coordinates>();

	for (int i = 0; i < FSPT->data().size(); i++) {
		if (!parseNAME(FSPT, &type, &id, i) || type != RCNM_VE)
			return QVector<Coordinates>();
		ORNT = FSPT->data().at(i).at(1).toUInt();
		MASK = FSPT->data().at(i).at(3).toUInt();
		Q_ASSERT(MASK != 1);

		RecordMapIterator it = _ve.find(id);
		if (it == _ve.constEnd())
			return QVector<Coordinates>();
		const ISO8211::Record &FRID = it.value();
		const ISO8211::Field *VRPT = FRID.field("VRPT");
		if (!VRPT || VRPT->data().size() != 2)
			return QVector<Coordinates>();

		for (int j = 0; j < 2; j++) {
			if (!parseNAME(VRPT, &type, &id, j) || type != RCNM_VC)
				return QVector<Coordinates>();

			RecordMapIterator jt = _vc.find(id);
			if (jt == _vc.constEnd())
				return QVector<Coordinates>();
			c[j] = point(jt.value());
			if (c[j].isNull())
				return QVector<Coordinates>();
		}

		const ISO8211::Field *vertexes = SGXD(FRID);
		if (ORNT == 2) {
			path.append(c[1]);
			if (vertexes) {
				for (int j = vertexes->data().size() - 1; j >= 0; j--) {
					const QVector<QVariant> &cv = vertexes->data().at(j);
					path.append(coordinates(cv.at(1).toInt(), cv.at(0).toInt()));
				}
			}
			path.append(c[0]);
		} else {
			path.append(c[0]);
			if (vertexes) {
				for (int j = 0; j < vertexes->data().size(); j++) {
					const QVector<QVariant> &cv = vertexes->data().at(j);
					path.append(coordinates(cv.at(1).toInt(), cv.at(0).toInt()));
				}
			}
			path.append(c[1]);
		}
	}

	return path;
}

Polygon MapData::polyGeometry(const ISO8211::Record &r)
{
	Polygon path;
	QVector<Coordinates> v;
	Coordinates c[2];
	uint ORNT, USAG, MASK;
	quint8 type;
	quint32 id;

	const ISO8211::Field *FSPT = r.field("FSPT");
	if (!FSPT || FSPT->data().at(0).size() != 4)
		return Polygon();

	for (int i = 0; i < FSPT->data().size(); i++) {
		if (!parseNAME(FSPT, &type, &id, i) || type != RCNM_VE)
			return Polygon();
		ORNT = FSPT->data().at(i).at(1).toUInt();
		USAG = FSPT->data().at(i).at(2).toUInt();
		MASK = FSPT->data().at(i).at(3).toUInt();
		Q_ASSERT(MASK != 1);

		if (USAG == 2 && path.isEmpty()) {
			path.append(v);
			v.clear();
		}

		RecordMapIterator it = _ve.find(id);
		if (it == _ve.constEnd())
			return Polygon();
		const ISO8211::Record &FRID = it.value();
		const ISO8211::Field *VRPT = FRID.field("VRPT");
		if (!VRPT || VRPT->data().size() != 2)
			return Polygon();

		for (int j = 0; j < 2; j++) {
			if (!parseNAME(VRPT, &type, &id, j) || type != RCNM_VC)
				return Polygon();

			RecordMapIterator jt = _vc.find(id);
			if (jt == _vc.constEnd())
				return Polygon();
			c[j] = point(jt.value());
			if (c[j].isNull())
				return Polygon();
		}

		const ISO8211::Field *vertexes = SGXD(FRID);
		if (ORNT == 2) {
			v.append(c[1]);
			if (vertexes) {
				for (int j = vertexes->data().size() - 1; j >= 0; j--) {
					const QVector<QVariant> &cv = vertexes->data().at(j);
					v.append(coordinates(cv.at(1).toInt(), cv.at(0).toInt()));
				}
			}
			v.append(c[0]);
		} else {
			v.append(c[0]);
			if (vertexes) {
				for (int j = 0; j < vertexes->data().size(); j++) {
					const QVector<QVariant> &cv = vertexes->data().at(j);
					v.append(coordinates(cv.at(1).toInt(), cv.at(0).toInt()));
				}
			}
			v.append(c[1]);
		}

		if (USAG == 2 && v.first() == v.last()) {
			path.append(v);
			v.clear();
		}
	}

	if (!v.isEmpty())
		path.append(v);

	return path;
}

MapData::Attr MapData::pointAttr(const ISO8211::Record &r, uint OBJL)
{
	QString label;
	uint subtype = 0;

	const ISO8211::Field *ATTF = r.field("ATTF");
	if (!(ATTF && ATTF->data().at(0).size() == 2))
		return Attr();

	for (int i = 0; i < ATTF->data().size(); i++) {
		const QVector<QVariant> &av = ATTF->data().at(i);
		uint key = av.at(0).toUInt();

		if (key == OBJNAM)
			label = av.at(1).toString();
		if ((OBJL == HRBFAC && key == CATHAF)
		  || (OBJL == LNDMRK && key == CATLMK)
		  || (OBJL == WRECKS && key == CATWRK))
			subtype = av.at(1).toString().toUInt();
	}

	return Attr(subtype, label);
}

MapData::Attr MapData::lineAttr(const ISO8211::Record &r, uint OBJL)
{
	QString label;
	uint subtype = 0;

	const ISO8211::Field *ATTF = r.field("ATTF");
	if (!(ATTF && ATTF->data().at(0).size() == 2))
		return Attr();

	for (int i = 0; i < ATTF->data().size(); i++) {
		const QVector<QVariant> &av = ATTF->data().at(i);
		uint key = av.at(0).toUInt();

		if (key == OBJNAM)
			label = av.at(1).toString();
		if ((OBJL == DEPCNT && key == VALDCO))
			label = av.at(1).toString();
	}

	return Attr(subtype, label);
}

MapData::Attr MapData::polyAttr(const ISO8211::Record &r, uint OBJL)
{
	QString label;
	uint subtype = 0;

	const ISO8211::Field *ATTF = r.field("ATTF");
	if (!(ATTF && ATTF->data().at(0).size() == 2))
		return Attr();

	for (int i = 0; i < ATTF->data().size(); i++) {
		const QVector<QVariant> &av = ATTF->data().at(i);
		uint key = av.at(0).toUInt();

		if (OBJL == DEPARE && key == DRVAL1)
			subtype = depthLevel(av.at(1).toString());
		else if (OBJL == RESARE && key == CATREA)
			subtype = av.at(1).toString().toUInt();
	}

	return Attr(subtype, label);
}

MapData::Point *MapData::pointObject(const Sounding &s)
{
	return new Point(SOUNDG<<16, s.c, QString::number(s.depth));
}

MapData::Point *MapData::pointObject(const ISO8211::Record &r, uint OBJL)
{
	Coordinates c(pointGeometry(r));
	Attr attr(pointAttr(r, OBJL));

	return (c.isNull() ? 0 : new Point(OBJL<<16|attr.subtype(), c,
	  attr.label()));
}

MapData::Line *MapData::lineObject(const ISO8211::Record &r, uint OBJL)
{
	QVector<Coordinates> path(lineGeometry(r));
	Attr attr(lineAttr(r, OBJL));

	return (path.isEmpty() ? 0 : new Line(OBJL<<16|attr.subtype(), path,
	  attr.label()));
}

MapData::Poly *MapData::polyObject(const ISO8211::Record &r, uint OBJL)
{
	Polygon path(polyGeometry(r));
	Attr attr(polyAttr(r, OBJL));

	return (path.isEmpty() ? 0 : new Poly(OBJL<<16|attr.subtype(), path));
}

bool MapData::processRecord(const ISO8211::Record &record)
{
	const ISO8211::Field &f = record.at(1);
	const QByteArray &ba = f.tag();

	if (ba == "VRID") {
		if (f.data().at(0).size() < 2)
			return false;
		int RCNM = f.data().at(0).at(0).toInt();
		uint RCID = f.data().at(0).at(1).toUInt();

		switch (RCNM) {
			case RCNM_VI:
				_vi.insert(RCID, record);
				break;
			case RCNM_VC:
				_vc.insert(RCID, record);
				break;
			case RCNM_VE:
				_ve.insert(RCID, record);
				break;
			case RCNM_VF:
				_vf.insert(RCID, record);
				break;
			default:
				return false;
		}
	} else if (ba == "FRID") {
		_fe.append(record);
	} else if (ba == "DSID") {
		QByteArray DSNM;

		if (!f.subfield("DSNM", &DSNM))
			return false;

		_name = DSNM;
	} else if (ba == "DSPM") {
		if (!(f.subfield("COMF", &_COMF) && f.subfield("SOMF", &_SOMF)))
			return false;
	}

	return true;
}

bool MapData::bounds(const ISO8211::Record &record, Rect &rect)
{
	bool xok, yok;
	const ISO8211::Field *f = SGXD(record);
	if (!f)
		return false;

	for (int i = 0; i < f->data().size(); i++) {
		const QVector<QVariant> &c = f->data().at(i);
		rect.unite(c.at(1).toInt(&xok), c.at(0).toInt(&yok));
		if (!(xok && yok))
			return false;
	}

	return true;
}

MapData::Rect MapData::bounds(const RecordMap &map)
{
	Rect b, r;

	for (RecordMapIterator it = map.constBegin(); it != map.constEnd(); ++it)
		if (bounds(it.value(), r))
			b |= r;

	return b;
}

RectC MapData::bounds() const
{
	const RecordMap *maps[] = {&_vi, &_vc, &_ve, &_vf};
	Rect b;

	for (size_t i = 0; i < ARRAY_SIZE(maps); i++)
		b |= bounds(*maps[i]);

	return RectC(
	  Coordinates(b.minX() / (double)_COMF, b.maxY() / (double)_COMF),
	  Coordinates(b.maxX() / (double)_COMF, b.minY() / (double)_COMF));
}

MapData::MapData(const QString &path): _valid(false)
{
	QFile file(path);

	if (!file.open(QIODevice::ReadOnly)) {
		_errorString = file.errorString();
		return;
	}

	if (!_ddf.readDDR(file)) {
		_errorString = _ddf.errorString();
		return;
	}
	while (!file.atEnd()) {
		ISO8211::Record record;
		if (!_ddf.readRecord(file, record)) {
			_errorString = _ddf.errorString();
			return;
		}
		if (!processRecord(record))
			return;
	}

	_valid = true;
}

MapData::~MapData()
{
	LineTree::Iterator lit;
	for (_lines.GetFirst(lit); !_lines.IsNull(lit); _lines.GetNext(lit))
		delete _lines.GetAt(lit);

	PolygonTree::Iterator ait;
	for (_areas.GetFirst(ait); !_areas.IsNull(ait); _areas.GetNext(ait))
		delete _areas.GetAt(ait);

	PointTree::Iterator pit;
	for (_points.GetFirst(pit); !_points.IsNull(pit); _points.GetNext(pit))
		delete _points.GetAt(pit);
}

void MapData::load()
{
	uint PRIM, OBJL;
	Poly *poly;
	Line *line;
	Point *point;
	double min[2], max[2];

	for (int i = 0; i < _fe.size(); i++) {
		const ISO8211::Record &r = _fe.at(i);
		const ISO8211::Field &f = r.at(1);

		if (f.data().at(0).size() < 5)
			continue;
		PRIM = f.data().at(0).at(2).toUInt();
		OBJL = f.data().at(0).at(4).toUInt();

		switch (PRIM) {
			case PRIM_P:
				if (OBJL == SOUNDG) {
					QVector<Sounding> s(soundingGeometry(r));
					for (int i = 0; i < s.size(); i++) {
						point = pointObject(s.at(i));
						min[0] = point->pos().lon();
						min[1] = point->pos().lat();
						max[0] = point->pos().lon();
						max[1] = point->pos().lat();
						_points.Insert(min, max, point);
					}
				} else {
					if ((point = pointObject(r, OBJL))) {
						min[0] = point->pos().lon();
						min[1] = point->pos().lat();
						max[0] = point->pos().lon();
						max[1] = point->pos().lat();
						_points.Insert(min, max, point);
					} else
						warning(f, PRIM);
				}
				break;
			case PRIM_L:
				if ((line = lineObject(r, OBJL))) {
					rectcBounds(line->bounds(), min, max);
					_lines.Insert(min, max, line);
				} else
					warning(f, PRIM);
				break;
			case PRIM_A:
				if ((poly = polyObject(r, OBJL))) {
					rectcBounds(poly->bounds(), min, max);
					_areas.Insert(min, max, poly);
				} else
					warning(f, PRIM);
				break;
		}
	}
}

void MapData::clear()
{
	LineTree::Iterator lit;
	for (_lines.GetFirst(lit); !_lines.IsNull(lit); _lines.GetNext(lit))
		delete _lines.GetAt(lit);
	_lines.RemoveAll();

	PolygonTree::Iterator ait;
	for (_areas.GetFirst(ait); !_areas.IsNull(ait); _areas.GetNext(ait))
		delete _areas.GetAt(ait);
	_areas.RemoveAll();

	PointTree::Iterator pit;
	for (_points.GetFirst(pit); !_points.IsNull(pit); _points.GetNext(pit))
		delete _points.GetAt(pit);
	_points.RemoveAll();
}

void MapData::points(const RectC &rect, QList<Point*> *points)
{
	double min[2], max[2];

	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();

	_points.Search(min, max, pointCb, points);
}

void MapData::lines(const RectC &rect, QList<Line*> *lines)
{
	double min[2], max[2];

	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();

	_lines.Search(min, max, lineCb, lines);
}

void MapData::polygons(const RectC &rect, QList<Poly*> *polygons)
{
	double min[2], max[2];

	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();

	_areas.Search(min, max, polygonCb, polygons);
}
