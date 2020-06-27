#include <QMap>
#include <QtEndian>
#include "vectortile.h"
#include "img.h"


typedef QMap<QByteArray, VectorTile*> TileMap;

static SubFile::Type tileType(const char str[3])
{
	if (!memcmp(str, "TRE", 3))
		return SubFile::TRE;
	else if (!memcmp(str, "RGN", 3))
		return SubFile::RGN;
	else if (!memcmp(str, "LBL", 3))
		return SubFile::LBL;
	else if (!memcmp(str, "TYP", 3))
		return SubFile::TYP;
	else if (!memcmp(str, "GMP", 3))
		return SubFile::GMP;
	else if (!memcmp(str, "NET", 3))
		return SubFile::NET;
	else
		return SubFile::Unknown;
}

IMG::IMG(const QString &fileName) : _file(fileName)
{
#define CHECK(condition) \
	if (!(condition)) { \
		_errorString = "Unsupported or invalid IMG file"; \
		qDeleteAll(tileMap); \
		return; \
	}

	TileMap tileMap;
	QByteArray typFile;

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
	_name = QString::fromLatin1(nba.constData(), nba.size()-1).trimmed();
	_blockBits = e1 + e2;

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

	// Read FAT blocks describing the IMG sub-files
	for (int i = 0; i < cnt; i++) {
		quint16 block;
		CHECK(_file.seek(offset) && readValue(flag) && read(name, sizeof(name))
		  && read(type, sizeof(type)) && readValue(size) && readValue(part));
		SubFile::Type tt = tileType(type);

		QByteArray fn(name, sizeof(name));
		if (VectorTile::isTileFile(tt)) {
			VectorTile *tile;
			TileMap::const_iterator it = tileMap.find(fn);
			if (it == tileMap.constEnd()) {
				tile = new VectorTile();
				tileMap.insert(fn, tile);
			} else
				tile = *it;

			SubFile *file = part ? tile->file(tt)
			  : tile->addFile(this, tt);
			CHECK(file);

			_file.seek(offset + 0x20);
			for (int i = 0; i < 240; i++) {
				CHECK(readValue(block));
				if (block == 0xFFFF)
					break;
				file->addBlock(block);
			}
		} else if (tt == SubFile::TYP) {
			SubFile *typ = 0;
			if (typFile.isNull()) {
				_typ = new SubFile(this);
				typ = _typ;
				typFile = fn;
			} else if (fn == typFile)
				typ = _typ;

			if (typ) {
				_file.seek(offset + 0x20);
				for (int i = 0; i < 240; i++) {
					CHECK(readValue(block));
					if (block == 0xFFFF)
						break;
					typ->addBlock(block);
				}
			}
		}

		offset += 512;
	}

	// Create tile tree
	int minMapZoom = 24;
	for (TileMap::const_iterator it = tileMap.constBegin();
	  it != tileMap.constEnd(); ++it) {
		VectorTile *tile = it.value();

		if (!tile->init()) {
			qWarning("%s: %s: Invalid map tile", qPrintable(_file.fileName()),
			  qPrintable(it.key()));
			delete tile;
			continue;
		}

		double min[2], max[2];
		min[0] = tile->bounds().left();
		min[1] = tile->bounds().bottom();
		max[0] = tile->bounds().right();
		max[1] = tile->bounds().top();
		_tileTree.Insert(min, max, tile);

		_bounds |= tile->bounds();
		if (tile->zooms().min() < _zooms.min())
			_zooms.setMin(tile->zooms().min());
		if (tile->zooms().min() < minMapZoom)
			minMapZoom = tile->zooms().min();
	}

	// Detect and mark basemap
	TileTree::Iterator it;
	for (_tileTree.GetFirst(it); !_tileTree.IsNull(it); _tileTree.GetNext(it)) {
		VectorTile *tile = _tileTree.GetAt(it);
		if (tile->zooms().min() > minMapZoom)
			_baseMap = true;
		if (tile->zooms().min() == minMapZoom)
			tile->markAsBasemap();
	}
	// Allow some extra zoom out on maps without basemaps, but not too much as
	// this would kill the rendering performance
	if (!_baseMap)
		_zooms.setMin(_zooms.min() - 2);

	if (!_tileTree.Count())
		_errorString = "No usable map tile found";
	else
		_valid = true;
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

	val = qFromLittleEndian(data);

	return true;
}

bool IMG::readBlock(int blockNum, char *data)
{
	if (!_file.seek((quint64)blockNum << _blockBits))
		return false;
	if (read(data, 1U<<_blockBits) < 1U<<_blockBits)
		return false;

	return true;
}
