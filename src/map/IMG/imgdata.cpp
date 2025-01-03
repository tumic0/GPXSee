#include <QMap>
#include <QtEndian>
#include <QFile>
#include "vectortile.h"
#include "imgdata.h"

using namespace IMG;

static SubFile::Type tileType(const char str[3])
{
	/* Note: we do not load NOD files from non-NT maps as we have no usage
	   for them */

	if (!memcmp(str, "TRE", 3))
		return SubFile::TRE;
	else if (!memcmp(str, "RGN", 3))
		return SubFile::RGN;
	else if (!memcmp(str, "LBL", 3))
		return SubFile::LBL;
	else if (!memcmp(str, "TYP", 3))
		return SubFile::TYP;
	else if (!memcmp(str, "NET", 3))
		return SubFile::NET;
	else if (!memcmp(str, "DEM", 3))
		return SubFile::DEM;
	else if (!memcmp(str, "GMP", 3))
		return SubFile::GMP;
	else
		return SubFile::Unknown;
}

bool IMGData::readSubFileBlocks(QFile *file, quint64 offset, SubFile *subFile)
{
	quint16 block;

	if (!file->seek(offset + 0x20))
		return false;
	for (int i = 0; i < 240; i++) {
		if (!readValue(file, block))
			return false;
		if (block == 0xFFFF)
			break;
		subFile->addBlock(block);
	}

	return true;
}

bool IMGData::readIMGHeader(QFile *file)
{
	char signature[7], identifier[7];
	if (!(file->read((char*)&_key, 1) && file->seek(0x10)
	  && read(file, signature, sizeof(signature)) && file->seek(0x41)
	  && read(file, identifier, sizeof(identifier)))
	  || memcmp(signature, "DSKIMG", sizeof(signature))
	  || memcmp(identifier, "GARMIN", sizeof(identifier))) {
		_errorString = "Not a Garmin IMG file";
		return false;
	}

	char d1[20], d2[31];
	quint8 e1, e2;
	if (!(file->seek(0x49) && read(file, d1, sizeof(d1)) && file->seek(0x61)
	  && readValue(file, e1) && readValue(file, e2) && file->seek(0x65)
	  && read(file, d2, sizeof(d2)))) {
		_errorString = "Error reading IMG header";
		return false;
	}

	QByteArray nba(QByteArray(d1, sizeof(d1)) + QByteArray(d2, sizeof(d2)));
	_name = QString::fromLatin1(nba.constData(), nba.size()-1).trimmed();
	_blockBits = e1 + e2;

	return true;
}

bool IMGData::readFAT(QFile *file, TileMap &tileMap)
{
	QByteArray typFile;
	quint8 flag;
	quint64 offset = 0x200;

	// Skip unused FAT blocks if any
	while (true) {
		if (!(file->seek(offset) && readValue(file, flag)))
			return false;
		if (flag)
			break;
		offset += 512;
	}

	// Read first FAT block with FAT table size
	char name[8], type[3];
	quint32 size;
	quint16 part;
	if (!(file->seek(offset + 12) && readValue(file, size)))
		return false;
	offset += 512;
	int cnt = (size - offset) / 512;

	// Read FAT blocks describing the IMG sub-files
	for (int i = 0; i < cnt; i++) {
		if (!(file->seek(offset) && readValue(file, flag)
		  && read(file, name, sizeof(name))
		  && read(file, type, sizeof(type)) && readValue(file, size)
		  && readValue(file, part)))
			return false;
		SubFile::Type tt = tileType(type);

		QByteArray fn(name, sizeof(name));
		if (VectorTile::isTileFile(tt)) {
			VectorTile *tile;
			TileMap::const_iterator it(tileMap.find(fn));
			if (it == tileMap.constEnd()) {
				tile = new VectorTile();
				tileMap.insert(fn, tile);
			} else
				tile = *it;

			SubFile *subFile = part ? tile->file(tt) : tile->addFile(this, tt);
			if (!(subFile && readSubFileBlocks(file, offset, subFile)))
				return false;
		} else if (tt == SubFile::TYP) {
			SubFile *typ = 0;
			if (typFile.isNull()) {
				_typ = new SubFile(this);
				typ = _typ;
				typFile = fn;
			} else if (fn == typFile)
				typ = _typ;

			if (typ && !readSubFileBlocks(file, offset, typ))
				return false;
		}

		offset += 512;
	}

	return true;
}

bool IMGData::createTileTree(QFile *file, const TileMap &tileMap)
{
	for (TileMap::const_iterator it = tileMap.constBegin();
	  it != tileMap.constEnd(); ++it) {
		VectorTile *tile = it.value();

		if (!tile->init(file)) {
			qWarning("%s: %s: Invalid map tile", qPrintable(_fileName),
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
		_hasDEM |= tile->hasDem();
	}

	return (_tileTree.Count() > 0);
}

IMGData::IMGData(const QString &fileName) : MapData(fileName)
{
	QFile file(fileName);
	TileMap tileMap;

	if (!file.open(QFile::ReadOnly)) {
		_errorString = file.errorString();
		return;
	}

	if (!readIMGHeader(&file))
		return;
	if (!readFAT(&file, tileMap)) {
		_errorString = "Error reading FAT data";
		qDeleteAll(tileMap);
		return;
	}
	if (!createTileTree(&file, tileMap)) {
		_errorString = "No usable map tile found";
		return;
	}

	computeZooms();

	_valid = true;
}

qint64 IMGData::read(QFile *file, char *data, qint64 maxSize) const
{
	qint64 ret = file->read(data, maxSize);
	if (_key)
		for (int i = 0; i < ret; i++)
			data[i] ^= _key;
	return ret;
}

template<class T> bool IMGData::readValue(QFile *file, T &val) const
{
	T data;

	if (read(file, (char*)&data, sizeof(T)) < (qint64)sizeof(T))
		return false;

	val = qFromLittleEndian(data);

	return true;
}

bool IMGData::readBlock(QFile *file, int blockNum, char *data) const
{
	if (!file->seek((quint64)blockNum << _blockBits))
		return false;
	if (read(file, data, 1ULL<<_blockBits) < (qint64)(1ULL<<_blockBits))
		return false;

	return true;
}
