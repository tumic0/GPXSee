#include <QFile>
#include <QDataStream>
#include "common/util.h"
#include "pmtiles.h"

template<typename T>
static bool varint(char **bp, const char *be, T &val)
{
	unsigned int shift = 0;
	val = 0;

	while ((*bp < be) && (shift < sizeof(T) * 8)) {
		val |= static_cast<T>((quint8)**bp & 0x7F) << shift;
		shift += 7;
		if (!((quint8)*(*bp)++ & 0x80))
			return true;
	}

	return false;
}

bool PMTiles::readHeader(QFile &file, Header &hdr, QString &err)
{
	QDataStream stream(&file);
	stream.setByteOrder(QDataStream::LittleEndian);
	stream >> hdr.magic >> hdr.rootOffset >> hdr.rootLength
	  >> hdr.metadataOffset >> hdr.metadataLength >> hdr.leafOffset
	  >> hdr.leafLength >> hdr.tileOffset >> hdr.tileLength
	  >> hdr.addressedTiles >> hdr.tileEntries >> hdr.tileContents
	  >> hdr.c >> hdr.ic >> hdr.tc >> hdr.tt >> hdr.minZ >> hdr.maxZ
	  >> hdr.minLon >> hdr.minLat >> hdr.maxLon >> hdr.maxLat;

	if (stream.status() || (hdr.magic & 0xFFFFFFFFFFFFFF) != 0x73656c69544d50) {
		err = "Not a PMTiles file";
		return false;
	}
	if ((hdr.magic >> 56) != 3) {
		err = QString("%1: unsupported PMTiles version").arg(hdr.magic >> 56);
		return false;
	}
	if ((hdr.ic < 1) || (hdr.ic > 2)) {
		err = QString("%1: unsupported internal compression").arg(hdr.ic);
		return false;
	}
	if ((hdr.tc < 1) || (hdr.tc > 2)) {
		err = QString("%1: unsupported tile compression").arg(hdr.tc);
		return false;
	}

	return true;
}

const PMTiles::Directory *PMTiles::findDir(const QVector<Directory> &list,
  quint64 tileId)
{
	qint64 m = 0;
	qint64 n = list.size() - 1;

	while (m <= n) {
		quint64 k = (n + m) >> 1;
		qint64 cmp = (qint64)tileId - (qint64)list.at(k).tileId;
		if (cmp > 0)
			m = k + 1;
		else if (cmp < 0)
			n = k - 1;
		else
			return &list.at(k);
	}

	if (n >= 0) {
		if (!list.at(n).runLength)
			return &list.at(n);
		if (tileId - list.at(n).tileId < (quint64)list.at(n).runLength)
			return &list.at(n);
	}

	return 0;
}

QByteArray PMTiles::readData(QFile &file, quint64 offset, quint64 size,
  quint8 compression)
{
	QByteArray ba;

	if (!file.seek(offset))
		return QByteArray();

	ba.resize(size);
	if (file.read(ba.data(), ba.size()) != ba.size())
		return QByteArray();

	return (compression == 2) ? Util::gunzip(ba) : ba;
}

QVector<PMTiles::Directory> PMTiles::readDir(QFile &file, quint64 offset,
  quint64 size, quint8 compression)
{
	QByteArray uba(readData(file, offset, size, compression));
	if (uba.isNull())
		return QVector<Directory>();

	char *bp = uba.data();
	const char *be = uba.constData() + uba.size();
	quint64 n;
	if (!varint(&bp, be, n))
		return QVector<Directory>();

	QVector<Directory> dirs;
	dirs.resize(n);
	for (int i = 0; i < dirs.size(); i++) {
		quint64 tileId;
		if (!varint(&bp, be, tileId))
			return QVector<Directory>();
		dirs[i].tileId = i ? tileId + dirs[i-1].tileId : tileId;
	}
	for (int i = 0; i < dirs.size(); i++)
		if (!varint(&bp, be, dirs[i].runLength))
			return QVector<Directory>();
	for (int i = 0; i < dirs.size(); i++)
		if (!varint(&bp, be, dirs[i].length))
			return QVector<Directory>();
	for (int i = 0; i < dirs.size(); i++) {
		quint64 offset;
		if (!varint(&bp, be, offset))
			return QVector<Directory>();
		dirs[i].offset = offset
		  ? offset - 1 : dirs[i-1].offset + dirs[i-1].length;
	}

	return dirs;
}

quint64 PMTiles::id(unsigned zoom, const QPoint &tile)
{
	unsigned x = tile.x();
	unsigned y = tile.y();
	quint64 acc = ((1 << (zoom * 2)) - 1) / 3;

	for (int a = zoom - 1; a >= 0; a--) {
		quint64 s = 1 << a;
		quint64 rx = s & x;
		quint64 ry = s & y;

		acc += ((3 * rx) ^ ry) << a;
		if (ry == 0) {
			if (rx != 0) {
				x = s - 1 - x;
				y = s - 1 - y;
			}
			std::swap(x, y);
		}
	}

	return acc;
}
