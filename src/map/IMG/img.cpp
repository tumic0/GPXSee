#include <QSet>
#include <QtEndian>
#include "common/programpaths.h"
#include "vectortile.h"
#include "img.h"


#define CHECK(condition) \
	if (!(condition)) { \
		_errorString = "Invalid/corrupted IMG file"; \
		return; \
	}

struct CTX
{
	CTX(const RectC &rect, int bits, QList<IMG::Poly> *polygons,
	  QList<IMG::Poly> *lines, QList<IMG::Point> *points)
	  : rect(rect), bits(bits), polygons(polygons), lines(lines),
	  points(points) {}

	const RectC &rect;
	int bits;
	QList<IMG::Poly> *polygons;
	QList<IMG::Poly> *lines;
	QList<IMG::Point> *points;
};

IMG::IMG(const QString &fileName) : _file(fileName), _valid(false)
{
	if (!_file.open(QFile::ReadOnly)) {
		_errorString = _file.errorString();
		return;
	}

	// Read IMG header
	char signature[7], identifier[7];
	_file.read((char*)&_key, 1) && _file.seek(0x10)
	  && read(signature, sizeof(signature)) && _file.seek(0x41)
	  && read(identifier, sizeof(identifier));
	if (memcmp(signature, "DSKIMG", sizeof(signature))
	  || memcmp(identifier, "GARMIN", sizeof(identifier))) {
		_errorString = "Not a Garmin IMG file";
		return;
	}
	char d1[20], d2[31];
	quint8 e1, e2;
	CHECK(_file.seek(0x49) && read(d1, sizeof(d1)) && _file.seek(0x61)
	  && readValue(e1) && readValue(e2) && _file.seek(0x65)
	  && read(d2, sizeof(d2)));
	QByteArray nba(QByteArray(d1, sizeof(d1)) + QByteArray(d2, sizeof(d2)));
	_name = QString(nba).trimmed();
	_blockSize = 1 << (e1 + e2);

	// Read the FAT table
	quint8 flag;
	quint64 offset = 0x200;
	// Skip unused FAT blocks if any
	while (true) {
		CHECK(_file.seek(offset) && readValue(flag));
		if (flag)
			break;
		offset += 512;
	}

	// Read first FAT block with FAT table size
	char name[8], type[3];
	quint32 size;
	quint16 part;
	CHECK(_file.seek(offset + 12) && readValue(size));
	offset += 512;
	int cnt = (size - offset) / 512;


	QMap<QString, VectorTile*> tileMap;
	QMap<QString, SubFile*> TYPMap;

	// Read FAT blocks describing the IMG sub-files
	for (int i = 0; i < cnt; i++) {
		quint16 block;
		CHECK(_file.seek(offset) && readValue(flag) && read(name, sizeof(name))
		  && read(type, sizeof(type)) && readValue(size) && readValue(part));
		SubFile::Type tt = SubFile::type(type);

		if (tt == SubFile::GMP) {
			_errorString = "NT maps not supported";
			return;
		}

		QString fn(QByteArray(name, sizeof(name)));
		if (VectorTile::isTileFile(tt)) {
			VectorTile *tile;
			QMap<QString, VectorTile*>::iterator it = tileMap.find(fn);
			if (it == tileMap.end()) {
				tile = new VectorTile();
				tileMap.insert(fn, tile);
			} else
				tile = *it;

			SubFile *file = part ? tile->file(tt)
			  : tile->addFile(this, tt, size);
			CHECK(file);

			_file.seek(offset + 0x20);
			for (int i = 0; i < 240; i++) {
				CHECK(readValue(block));
				if (block == 0xFFFF)
					break;
				file->addBlock(block);
			}
		} else if (tt == SubFile::TYP) {
			SubFile *typ;
			QMap<QString, SubFile*>::iterator it = TYPMap.find(fn);
			if (it == TYPMap.end()) {
				typ = new SubFile(this, size);
				TYPMap.insert(fn, typ);
			} else
				typ = *it;

			_file.seek(offset + 0x20);
			for (int i = 0; i < 240; i++) {
				CHECK(readValue(block));
				if (block == 0xFFFF)
					break;
				typ->addBlock(block);
			}
		}

		offset += 512;
	}

	// Create tile tree
	for (QMap<QString, VectorTile*>::iterator it = tileMap.begin();
	  it != tileMap.end(); ++it) {
		CHECK((*it)->init());

		double min[2], max[2];
		min[0] = (*it)->bounds().left();
		min[1] = (*it)->bounds().bottom();
		max[0] = (*it)->bounds().right();
		max[1] = (*it)->bounds().top();
		_tileTree.Insert(min, max, *it);

		_bounds |= (*it)->bounds();
	}

	// Read TYP file if any
	if (!TYPMap.isEmpty()) {
		if (TYPMap.size() > 1)
			qWarning("%s: Multiple TYP files, using %s",
			  qPrintable(_file.fileName()), qPrintable(TYPMap.keys().first()));
		SubFile *typ = TYPMap.values().first();
		_style = Style(typ);
		qDeleteAll(TYPMap);
	} else {
		QFile typFile(ProgramPaths::typFile());
		if (typFile.exists()) {
			SubFile typ(&typFile);
			_style = Style(&typ);
		}
	}

	_valid = true;
}

IMG::~IMG()
{
	TileTree::Iterator it;
	for (_tileTree.GetFirst(it); !_tileTree.IsNull(it); _tileTree.GetNext(it))
		delete _tileTree.GetAt(it);
}

static bool cb(VectorTile *tile, void *context)
{
	CTX *ctx = (CTX*)context;
	tile->objects(ctx->rect, ctx->bits, ctx->polygons, ctx->lines, ctx->points);
	return true;
}

void IMG::objects(const RectC &rect, int bits, QList<Poly> *polygons,
  QList<Poly> *lines, QList<Point> *points)
{
	CTX ctx(rect, bits, polygons, lines, points);
	double min[2], max[2];

	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();

	_tileTree.Search(min, max, cb, &ctx);
}

qint64 IMG::read(char *data, qint64 maxSize)
{
	qint64 ret = _file.read(data, maxSize);
	if (_key)
		for (int i = 0; i < ret; i++)
			data[i] ^= _key;
	return ret;
}

template<class T> bool IMG::readValue(T &val)
{
	T data;

	if (read((char*)&data, sizeof(T)) < (qint64)sizeof(T))
		return false;

	if (sizeof(T) > 1)
		val = qFromLittleEndian(data);
	else
		val = data;

	return true;
}

bool IMG::readBlock(int blockNum, QByteArray &data)
{
	QByteArray *block = _blockCache[blockNum];
	if (!block) {
		if (!_file.seek((qint64)blockNum * (qint64)_blockSize))
			return false;
		data.resize(_blockSize);
		if (read(data.data(), _blockSize) < _blockSize)
			return false;
		_blockCache.insert(blockNum, new QByteArray(data));
	} else
		data = *block;

	return true;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const IMG::Point &point)
{
	dbg.nospace() << "Point(" << hex << point.type << ", " << point.label
	  << ", " << point.poi << ")";
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const IMG::Poly &poly)
{
	dbg.nospace() << "Poly(" << hex << poly.type << ", " << poly.label << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
