#include "GUI/units.h"
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

static QMap<uint,uint> orderMapInit()
{
	QMap<uint,uint> map;

	map.insert(TYPE(LIGHTS), 0);
	map.insert(TYPE(FOGSIG), 0);

	map.insert(TYPE(CGUSTA), 1);
	map.insert(TYPE(RSCSTA), 1);
	map.insert(SUBTYPE(BUAARE, 1), 2);
	map.insert(SUBTYPE(BUAARE, 5), 3);
	map.insert(SUBTYPE(BUAARE, 4), 4);
	map.insert(SUBTYPE(BUAARE, 3), 5);
	map.insert(SUBTYPE(BUAARE, 2), 6);
	map.insert(SUBTYPE(BUAARE, 6), 7);
	map.insert(SUBTYPE(BUAARE, 0), 8);
	map.insert(TYPE(RDOSTA), 9);
	map.insert(TYPE(RADSTA), 10);
	map.insert(TYPE(RTPBCN), 11);
	map.insert(TYPE(BCNISD), 12);
	map.insert(TYPE(BCNLAT), 13);
	map.insert(TYPE(I_BCNLAT), 13);
	map.insert(TYPE(BCNSAW), 14);
	map.insert(TYPE(BCNSPP), 15);
	map.insert(TYPE(BOYCAR), 16);
	map.insert(TYPE(BOYINB), 17);
	map.insert(TYPE(BOYISD), 18);
	map.insert(TYPE(BOYLAT), 19);
	map.insert(TYPE(I_BOYLAT), 19);
	map.insert(TYPE(BOYSAW), 20);
	map.insert(TYPE(BOYSPP), 21);
	map.insert(TYPE(MORFAC), 22);
	map.insert(TYPE(OFSPLF), 23);
	map.insert(TYPE(OBSTRN), 24);
	map.insert(TYPE(WRECKS), 25);
	map.insert(TYPE(UWTROC), 26);
	map.insert(TYPE(WATTUR), 27);
	map.insert(TYPE(CURENT), 28);
	map.insert(TYPE(PILBOP), 29);
	map.insert(TYPE(SISTAT), 30);
	map.insert(TYPE(I_SISTAT), 30);
	map.insert(TYPE(RDOCAL), 31);
	map.insert(TYPE(I_RDOCAL), 31);
	map.insert(TYPE(I_TRNBSN), 32);
	map.insert(TYPE(HRBFAC), 33);
	map.insert(TYPE(I_HRBFAC), 33);
	map.insert(TYPE(PILPNT), 34);
	map.insert(TYPE(ACHBRT), 35);
	map.insert(TYPE(I_ACHBRT), 35);
	map.insert(TYPE(RADRFL), 36);
	map.insert(TYPE(CRANES), 37);
	map.insert(TYPE(I_CRANES), 37);
	map.insert(TYPE(I_WTWGAG), 38);
	map.insert(TYPE(PYLONS), 39);
	map.insert(TYPE(SLCONS), 40);
	map.insert(TYPE(LNDMRK), 41);
	map.insert(TYPE(SILTNK), 42);
	map.insert(TYPE(LNDELV), 43);
	map.insert(TYPE(SMCFAC), 44);
	map.insert(TYPE(BUISGL), 45);

	map.insert(TYPE(I_DISMAR), 0xFFFFFFFE);
	map.insert(TYPE(SOUNDG), 0xFFFFFFFF);

	return map;
}

static QMap<uint,uint> orderMap = orderMapInit();

static uint order(uint type)
{
	uint st = ((type>>16) == BUAARE) ? type : (type & 0xFFFF0000);
	QMap<uint, uint>::const_iterator it(orderMap.find(st));
	return (it == orderMap.constEnd()) ? (type>>16) + 512 : it.value();
}

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

static void pointBounds(const Coordinates &c, double min[2], double max[2])
{
	min[0] = c.lon();
	min[1] = c.lat();
	max[0] = c.lon();
	max[1] = c.lat();
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

	if ((f = ISO8211::field(r, "SG2D")))
		return f;
	else if ((f = ISO8211::field(r, "SG3D")))
		return f;
	else
		return 0;
}

static bool pointCb(const MapData::Point *point, void *context)
{
	QList<MapData::Point> *points = (QList<MapData::Point>*)context;
	points->append(*point);
	return true;
}

static bool lineCb(const MapData::Line *line, void *context)
{
	QList<MapData::Line> *lines = (QList<MapData::Line>*)context;
	lines->append(*line);
	return true;
}

static bool polygonCb(const MapData::Poly *polygon, void *context)
{
	QList<MapData::Poly> *polygons = (QList<MapData::Poly>*)context;
	polygons->append(*polygon);
	return true;
}

static bool polygonPointCb(const MapData::Poly *polygon, void *context)
{
	QList<MapData::Point> *points = (QList<MapData::Point>*)context;
	uint type = polygon->type();
	uint baseType = type>>16;

	if (baseType == TSSLPT || baseType == RCTLPT || baseType == I_TRNBSN
	  || baseType == BRIDGE || baseType == I_BRIDGE || baseType == BUAARE
	  || baseType == RESARE || baseType == I_RESARE || baseType == LNDARE
	  || baseType == LNDRGN
	  || type == SUBTYPE(ACHARE, 2) || type == SUBTYPE(ACHARE, 3)
	  || type == SUBTYPE(ACHARE, 9) || type == SUBTYPE(I_ACHARE, 2)
	  || type == SUBTYPE(I_ACHARE, 3) || type == SUBTYPE(I_ACHARE, 9)
	  || type == SUBTYPE(I_BERTHS, 6))
		points->append(MapData::Point(baseType, polygon->bounds().center(),
		  polygon->attributes(), polygon->HUNI(), true));

	return true;
}

static bool linePointCb(const MapData::Line *line, void *context)
{
	QList<MapData::Point> *points = (QList<MapData::Point>*)context;
	uint baseType = line->type()>>16;

	if (baseType == RDOCAL || baseType == I_RDOCAL)
		points->append(MapData::Point(baseType, line->bounds().center(),
		  line->attributes(), 1));

	return true;
}

static Coordinates coordinates(int x, int y, uint COMF)
{
	return Coordinates(x / (double)COMF, y / (double)COMF);
}

static Coordinates point(const ISO8211::Record &r, uint COMF)
{
	const ISO8211::Field *f = SGXD(r);
	if (!f)
		return Coordinates();

	int y = f->data().at(0).at(0).toInt();
	int x = f->data().at(0).at(1).toInt();

	return coordinates(x, y, COMF);
}

static uint depthLevel(double minDepth)
{
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

static QString hUnits(uint type)
{
	switch (type) {
		case 1:
			return "m";
		case 2:
			return "ft";
		case 3:
			return "km";
		case 4:
			return "hm";
		case 5:
			return "mi";
		case 6:
			return "nm";
		default:
			return QString();
	}
}

static QString sistat(uint type)
{
	switch (type) {
		case 1:
			return "SS (Port Control)";
		case 3:
			return "SS (INT)";
		case 6:
			return "SS (Lock)";
		case 8:
			return "SS (Bridge)";
		default:
			return "SS";
	}
}

static QString weed(uint type)
{
	switch (type) {
		case 2:
			return "Wd";
		case 3:
			return "Sg";
		default:
			return QString();
	}
}

static uint restrictionCategory(uint type, const MapData::Attributes &attr)
{
	uint catrea = attr.value(CATREA).toUInt();

	if (!catrea) {
		uint restrn = attr.value(
		  (type == RESARE) ? RESTRN : I_RESTRN).toUInt();

		if (restrn == 1)
			return 2;
		else if (restrn == 7)
			return 17;
		else
			return 0;
	} else
		return catrea;
}

MapData::Point::Point(uint type, const Coordinates &c, const QString &label)
  : _type(SUBTYPE(type, 0)), _pos(c), _label(label), _polygon(false)
{
	_id = ((quint64)order(_type))<<32 | (uint)qHash(c);
}

MapData::Point::Point(uint type, const Coordinates &c, const Attributes &attr,
  uint HUNI, bool polygon) : _pos(c), _attr(attr), _polygon(polygon)
{
	uint subtype = 0;

	if (type == HRBFAC)
		subtype = CATHAF;
	else if (type == I_HRBFAC)
		subtype = I_CATHAF;
	else if (type == LNDMRK)
		subtype = CATLMK;
	else if (type == WRECKS)
		subtype = CATWRK;
	else if (type == MORFAC)
		subtype = CATMOR;
	else if (type == UWTROC)
		subtype = WATLEV;
	else if (type == BUAARE)
		subtype = CATBUA;
	else if (type == SMCFAC)
		subtype = CATSCF;
	else if (type == BUISGL)
		subtype = FUNCTN;
	else if (type == WATTUR)
		subtype = CATWAT;
	else if (type == RDOCAL)
		subtype = TRAFIC;
	else if (type == I_RDOCAL)
		subtype = TRAFIC;
	else if (type == SILTNK)
		subtype = CATSIL;
	else if (type == WEDKLP)
		subtype = CATWED;
	else if (type == LIGHTS)
		subtype = CATLIT;
	else if (type == I_DISMAR)
		subtype = CATDIS;
	else if (type == I_BERTHS)
		subtype = I_CATBRT;
	else if (type == ACHARE)
		subtype = CATACH;
	else if (type == I_ACHARE)
		subtype = I_CATACH;

	QList<QByteArray> list(_attr.value(subtype).split(','));
	std::sort(list.begin(), list.end());
	_type = (type == RESARE || type == I_RESARE)
	  ? SUBTYPE(type, restrictionCategory(type, _attr))
	  : SUBTYPE(type, list.first().toUInt());
	_id = ((quint64)order(_type))<<32 | (uint)qHash(c);
	_label = QString::fromLatin1(_attr.value(OBJNAM));

	if (type == I_DISMAR) {
		if (_attr.contains(I_WTWDIS) && _attr.contains(I_HUNITS))
			_label = hUnits(_attr.value(I_HUNITS).toUInt()) + " "
			  + QString::fromLatin1(_attr.value(I_WTWDIS));
	} else if (type == I_RDOCAL || type == RDOCAL) {
		QByteArray cc(_attr.value(COMCHA));
		if (!cc.isEmpty())
			_label = QString("VHF ") + QString::fromLatin1(cc);
	} else if (type == CURENT) {
		QByteArray cv(_attr.value(CURVEL));
		if (!cv.isEmpty())
			_label = QString::fromLatin1(cv) + QString::fromUtf8("\xE2\x80\x89kt");
	} else if (type == SISTAT) {
		if (_label.isEmpty() && _attr.contains(CATSIT))
			_label = sistat(_attr.value(CATSIT).toUInt());
	} else if (type == I_SISTAT) {
		if (_label.isEmpty() && _attr.contains(I_CATSIT))
			_label = sistat(_attr.value(I_CATSIT).toUInt());
	} else if (type == WEDKLP) {
		if (_label.isEmpty())
			_label = weed(_type & 0xFF);
	} else if (type == LNDELV) {
		if (_label.isEmpty())
			_label = QString::fromLatin1(_attr.value(ELEVAT))
			  + QString::fromUtf8("\xE2\x80\x89m");
		else
			_label += "\n(" + QString::fromLatin1(_attr.value(ELEVAT))
			  + "\xE2\x80\x89m)";
	} else if (type == BRIDGE || type == I_BRIDGE) {
		double clr = _attr.value(VERCLR).toDouble();
		if (clr > 0) {
			_label = QString::fromUtf8("\xE2\x86\x95") + UNIT_SPACE
			  + QString::number(clr) + UNIT_SPACE + hUnits(HUNI);
		}
	}
}

MapData::Poly::Poly(uint type, const Polygon &path, const Attributes &attr,
  uint HUNI) : _path(path), _attr(attr), _HUNI(HUNI)
{
	uint subtype = 0;

	if (type == ACHARE)
		subtype = CATACH;
	else if (type == I_ACHARE)
		subtype = I_CATACH;
	else if (type == HRBFAC)
		subtype = CATHAF;
	else if (type == MARKUL)
		subtype = CATMFA;
	else if (type == I_BERTHS)
		subtype = I_CATBRT;

	switch (type) {
		case DEPARE:
			_type = SUBTYPE(type, depthLevel(_attr.value(DRVAL1).toDouble()));
			break;
		case RESARE:
		case I_RESARE:
			_type = SUBTYPE(type, restrictionCategory(type, attr));
			break;
		default:
			_type = SUBTYPE(type, _attr.value(subtype).toUInt());
	}

}

MapData::Line::Line(uint type, const QVector<Coordinates> &path,
  const Attributes &attr) : _path(path), _attr(attr)
{
	uint subtype = 0;

	if (type == RECTRC)
		subtype = CATTRK;
	else if (type == RCRTCL)
		subtype = CATTRK;
	else if (type == RDOCAL)
		subtype = TRAFIC;
	else if (type == I_RDOCAL)
		subtype = TRAFIC;

	_type = SUBTYPE(type, _attr.value(subtype).toUInt());

	if (type == DEPCNT)
		_label = QString::fromLatin1(_attr.value(VALDCO));
	else if (type == LNDELV)
		_label = QString::fromLatin1(_attr.value(ELEVAT));
	else
		_label = QString::fromLatin1(_attr.value(OBJNAM));
}

RectC MapData::Line::bounds() const
{
	RectC b;

	for (int i = 0; i < _path.size(); i++)
		b = b.united(_path.at(i));

	return b;
}

QVector<MapData::Sounding> MapData::soundings(const ISO8211::Record &r,
  uint COMF, uint SOMF)
{
	QVector<Sounding> s;
	const ISO8211::Field *f = ISO8211::field(r, "SG3D");
	if (!f)
		return QVector<Sounding>();

	s.reserve(f->data().size());
	for (int i = 0; i < f->data().size(); i++) {
		int y = f->data().at(i).at(0).toInt();
		int x = f->data().at(i).at(1).toInt();
		int z = f->data().at(i).at(2).toInt();
		s.append(Sounding(coordinates(x, y, COMF), z / (double)SOMF));
	}

	return s;
}

QVector<MapData::Sounding> MapData::soundingGeometry(const ISO8211::Record &r,
  const RecordMap &vi, const RecordMap &vc, uint COMF, uint SOMF)
{
	quint8 type;
	quint32 id;
	RecordMapIterator it;

	const ISO8211::Field *FSPT = ISO8211::field(r, "FSPT");
	if (!FSPT || FSPT->data().at(0).size() != 4)
		return QVector<Sounding>();

	if (!parseNAME(FSPT, &type, &id))
		return QVector<Sounding>();

	if (type == RCNM_VI) {
		it = vi.find(id);
		if (it == vi.constEnd())
			return QVector<Sounding>();
	} else if (type == RCNM_VC) {
		it = vc.find(id);
		if (it == vc.constEnd())
			return QVector<Sounding>();
	} else
		return QVector<Sounding>();

	return soundings(it.value(), COMF, SOMF);
}

Coordinates MapData::pointGeometry(const ISO8211::Record &r,
  const RecordMap &vi, const RecordMap &vc, uint COMF)
{
	quint8 type;
	quint32 id;
	RecordMapIterator it;

	const ISO8211::Field *FSPT = ISO8211::field(r, "FSPT");
	if (!FSPT || FSPT->data().at(0).size() != 4)
		return Coordinates();

	if (!parseNAME(FSPT, &type, &id))
		return Coordinates();

	if (type == RCNM_VI) {
		it = vi.find(id);
		if (it == vi.constEnd())
			return Coordinates();
	} else if (type == RCNM_VC) {
		it = vc.find(id);
		if (it == vc.constEnd())
			return Coordinates();
	} else
		return Coordinates();

	return point(it.value(), COMF);
}

QVector<Coordinates> MapData::lineGeometry(const ISO8211::Record &r,
  const RecordMap &vc, const RecordMap &ve, uint COMF)
{
	QVector<Coordinates> path;
	Coordinates c[2];
	uint ORNT;
	quint8 type;
	quint32 id;

	const ISO8211::Field *FSPT = ISO8211::field(r, "FSPT");
	if (!FSPT || FSPT->data().at(0).size() != 4)
		return QVector<Coordinates>();

	for (int i = 0; i < FSPT->data().size(); i++) {
		if (!parseNAME(FSPT, &type, &id, i) || type != RCNM_VE)
			return QVector<Coordinates>();
		ORNT = FSPT->data().at(i).at(1).toUInt();

		RecordMapIterator it = ve.find(id);
		if (it == ve.constEnd())
			return QVector<Coordinates>();
		const ISO8211::Record &FRID = it.value();
		const ISO8211::Field *VRPT = ISO8211::field(FRID, "VRPT");
		if (!VRPT || VRPT->data().size() != 2)
			return QVector<Coordinates>();

		for (int j = 0; j < 2; j++) {
			if (!parseNAME(VRPT, &type, &id, j) || type != RCNM_VC)
				return QVector<Coordinates>();

			RecordMapIterator jt = vc.find(id);
			if (jt == vc.constEnd())
				return QVector<Coordinates>();
			c[j] = point(jt.value(), COMF);
			if (c[j].isNull())
				return QVector<Coordinates>();
		}

		const ISO8211::Field *vertexes = SGXD(FRID);
		if (ORNT == 2) {
			path.append(c[1]);
			if (vertexes) {
				for (int j = vertexes->data().size() - 1; j >= 0; j--) {
					const QVector<QVariant> &cv = vertexes->data().at(j);
					path.append(coordinates(cv.at(1).toInt(), cv.at(0).toInt(),
					  COMF));
				}
			}
			path.append(c[0]);
		} else {
			path.append(c[0]);
			if (vertexes) {
				for (int j = 0; j < vertexes->data().size(); j++) {
					const QVector<QVariant> &cv = vertexes->data().at(j);
					path.append(coordinates(cv.at(1).toInt(), cv.at(0).toInt(),
					  COMF));
				}
			}
			path.append(c[1]);
		}
	}

	return path;
}

Polygon MapData::polyGeometry(const ISO8211::Record &r, const RecordMap &vc,
  const RecordMap &ve, uint COMF)
{
	Polygon path;
	QVector<Coordinates> v;
	Coordinates c[2];
	uint ORNT, USAG;
	quint8 type;
	quint32 id;

	const ISO8211::Field *FSPT = ISO8211::field(r, "FSPT");
	if (!FSPT || FSPT->data().at(0).size() != 4)
		return Polygon();

	for (int i = 0; i < FSPT->data().size(); i++) {
		if (!parseNAME(FSPT, &type, &id, i) || type != RCNM_VE)
			return Polygon();
		ORNT = FSPT->data().at(i).at(1).toUInt();
		USAG = FSPT->data().at(i).at(2).toUInt();

		if (USAG == 2 && path.isEmpty()) {
			path.append(v);
			v.clear();
		}

		RecordMapIterator it = ve.find(id);
		if (it == ve.constEnd())
			return Polygon();
		const ISO8211::Record &FRID = it.value();
		const ISO8211::Field *VRPT = ISO8211::field(FRID, "VRPT");
		if (!VRPT || VRPT->data().size() != 2)
			return Polygon();

		for (int j = 0; j < 2; j++) {
			if (!parseNAME(VRPT, &type, &id, j) || type != RCNM_VC)
				return Polygon();

			RecordMapIterator jt = vc.find(id);
			if (jt == vc.constEnd())
				return Polygon();
			c[j] = point(jt.value(), COMF);
			if (c[j].isNull())
				return Polygon();
		}

		const ISO8211::Field *vertexes = SGXD(FRID);
		if (ORNT == 2) {
			v.append(c[1]);
			if (USAG == 3)
				v.append(Coordinates());
			if (vertexes) {
				for (int j = vertexes->data().size() - 1; j >= 0; j--) {
					const QVector<QVariant> &cv = vertexes->data().at(j);
					v.append(coordinates(cv.at(1).toInt(), cv.at(0).toInt(),
					  COMF));
				}
			}
			if (USAG == 3)
				v.append(Coordinates());
			v.append(c[0]);
		} else {
			v.append(c[0]);
			if (USAG == 3)
				v.append(Coordinates());
			if (vertexes) {
				for (int j = 0; j < vertexes->data().size(); j++) {
					const QVector<QVariant> &cv = vertexes->data().at(j);
					v.append(coordinates(cv.at(1).toInt(), cv.at(0).toInt(),
					  COMF));
				}
			}
			if (USAG == 3)
				v.append(Coordinates());
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

MapData::Attributes MapData::attributes(const ISO8211::Record &r)
{
	Attributes attr;

	const ISO8211::Field *ATTF = ISO8211::field(r, "ATTF");
	if (!(ATTF && ATTF->data().at(0).size() == 2))
		return attr;

	for (int i = 0; i < ATTF->data().size(); i++) {
		const QVector<QVariant> &av = ATTF->data().at(i);
		attr.insert(av.at(0).toUInt(), av.at(1).toByteArray());
	}

	return attr;
}

MapData::Point *MapData::pointObject(const Sounding &s)
{
	return new Point(SOUNDG, s.c, QString::number(s.depth));
}

MapData::Point *MapData::pointObject(const ISO8211::Record &r,
  const RecordMap &vi, const RecordMap &vc, uint COMF, uint OBJL, uint HUNI)
{
	Coordinates c(pointGeometry(r, vi, vc, COMF));
	return (c.isNull() ? 0 : new Point(OBJL, c, attributes(r), HUNI));
}

MapData::Line *MapData::lineObject(const ISO8211::Record &r,
  const RecordMap &vc, const RecordMap &ve, uint COMF, uint OBJL)
{
	QVector<Coordinates> path(lineGeometry(r, vc, ve, COMF));
	return (path.isEmpty() ? 0 : new Line(OBJL, path, attributes(r)));
}

MapData::Poly *MapData::polyObject(const ISO8211::Record &r,
  const RecordMap &vc, const RecordMap &ve, uint COMF, uint OBJL, uint HUNI)
{
	Polygon path(polyGeometry(r, vc, ve, COMF));
	return (path.isEmpty() ? 0 : new Poly(OBJL, path, attributes(r), HUNI));
}

bool MapData::processRecord(const ISO8211::Record &record,
  QVector<ISO8211::Record> &fe, RecordMap &vi, RecordMap &vc, RecordMap &ve,
  RecordMap &vf, uint &COMF, uint &SOMF, uint &HUNI)
{
	if (record.size() < 2)
		return false;

	const ISO8211::Field &f = record.at(1);
	const QByteArray &ba = f.tag();

	if (ba == "VRID") {
		if (f.data().at(0).size() < 2)
			return false;
		int RCNM = f.data().at(0).at(0).toInt();
		uint RCID = f.data().at(0).at(1).toUInt();

		switch (RCNM) {
			case RCNM_VI:
				vi.insert(RCID, record);
				break;
			case RCNM_VC:
				vc.insert(RCID, record);
				break;
			case RCNM_VE:
				ve.insert(RCID, record);
				break;
			case RCNM_VF:
				vf.insert(RCID, record);
				break;
			default:
				return false;
		}
	} else if (ba == "FRID") {
		fe.append(record);
	} else if (ba == "DSPM") {
		if (!(f.subfield("COMF", &COMF) && f.subfield("SOMF", &SOMF)))
			return false;
		if (!f.subfield("HUNI", &HUNI))
			return false;
	}

	return true;
}

MapData::MapData(const QString &path)
{
	RecordMap vi, vc, ve, vf;
	QVector<ISO8211::Record> fe;
	ISO8211 ddf(path);
	ISO8211::Record record;
	uint PRIM, OBJL, COMF = 1, SOMF = 1, HUNI = 1;
	Poly *poly;
	Line *line;
	Point *point;
	double min[2], max[2];


	if (!ddf.readDDR())
		return;
	while (ddf.readRecord(record))
		if (!processRecord(record, fe, vi, vc, ve, vf, COMF, SOMF, HUNI))
			qWarning("Invalid S-57 record");

	for (int i = 0; i < fe.size(); i++) {
		const ISO8211::Record &r = fe.at(i);
		const ISO8211::Field &f = r.at(1);

		if (f.data().at(0).size() < 5)
			continue;
		PRIM = f.data().at(0).at(2).toUInt();
		OBJL = f.data().at(0).at(4).toUInt();

		switch (PRIM) {
			case PRIM_P:
				if (OBJL == SOUNDG) {
					QVector<Sounding> s(soundingGeometry(r, vi, vc, COMF, SOMF));
					for (int i = 0; i < s.size(); i++) {
						point = pointObject(s.at(i));
						pointBounds(point->pos(), min, max);
						_points.Insert(min, max, point);
					}
				} else {
					if ((point = pointObject(r, vi, vc, COMF, OBJL, HUNI))) {
						pointBounds(point->pos(), min, max);
						_points.Insert(min, max, point);
					} else
						warning(f, PRIM);
				}
				break;
			case PRIM_L:
				if ((line = lineObject(r, vc, ve, COMF, OBJL))) {
					rectcBounds(line->bounds(), min, max);
					_lines.Insert(min, max, line);
				} else
					warning(f, PRIM);
				break;
			case PRIM_A:
				if ((poly = polyObject(r, vc, ve, COMF, OBJL, HUNI))) {
					rectcBounds(poly->bounds(), min, max);
					_areas.Insert(min, max, poly);
				} else
					warning(f, PRIM);
				break;
		}
	}
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

void MapData::points(const RectC &rect, QList<Point> *points) const
{
	double min[2], max[2];

	rectcBounds(rect, min, max);
	_points.Search(min, max, pointCb, points);
	_areas.Search(min, max, polygonPointCb, points);
	_lines.Search(min, max, linePointCb, points);
}

void MapData::lines(const RectC &rect, QList<Line> *lines) const
{
	double min[2], max[2];

	rectcBounds(rect, min, max);
	_lines.Search(min, max, lineCb, lines);
}

void MapData::polygons(const RectC &rect, QList<Poly> *polygons) const
{
	double min[2], max[2];

	rectcBounds(rect, min, max);
	_areas.Search(min, max, polygonCb, polygons);
}
