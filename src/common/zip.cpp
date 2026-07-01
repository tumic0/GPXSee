#include <QtEndian>
#include <QFile>
#include <QDebug>
#ifdef _MSC_VER
#include <QtZlib/zlib.h>
#else // _MSC_VER
#include <zlib.h>
#endif // _MSC_VER
#include "zip.h"

#define MAGIC 0x04034b50
#define ENCRYPTED 0x01
#define COMPRESSION_NONE     0
#define CCOMPRESSION_DEFLATE 8

#define UINT16(data) qFromLittleEndian<quint16>(data)
#define UINT32(data) qFromLittleEndian<quint32>(data)

struct EndOfDirectory
{
	uchar signature[4];
	uchar this_disk[2];
	uchar start_of_directory_disk[2];
	uchar num_dir_entries_this_disk[2];
	uchar num_dir_entries[2];
	uchar directory_size[4];
	uchar dir_start_offset[4];
	uchar comment_length[2];
};

struct CentralFileHeader
{
	uchar signature[4];
	uchar version_made[2];
	uchar version_needed[2];
	uchar general_purpose_bits[2];
	uchar compression_method[2];
	uchar last_mod_file[4];
	uchar crc_32[4];
	uchar compressed_size[4];
	uchar uncompressed_size[4];
	uchar file_name_length[2];
	uchar extra_field_length[2];
	uchar file_comment_length[2];
	uchar disk_start[2];
	uchar internal_file_attributes[2];
	uchar external_file_attributes[4];
	uchar offset_local_header[4];
};

struct LocalFileHeader
{
	uchar signature[4];
	uchar version_needed[2];
	uchar general_purpose_bits[2];
	uchar compression_method[2];
	uchar last_mod_file[4];
	uchar crc_32[4];
	uchar compressed_size[4];
	uchar uncompressed_size[4];
	uchar file_name_length[2];
	uchar extra_field_length[2];
};

static bool readHeaders(QIODevice *device, QHash<QString, quint32> &files)
{
	if (!(device->isOpen() && device->isReadable()))
		return false;

	quint32 magic;
	if (!(device->read((char*)&magic, sizeof(MAGIC)) == sizeof(magic)
	  && qFromLittleEndian(magic) == MAGIC))
		return false;

	EndOfDirectory eod;
	for (int i = 0; ; i++) {
		qint64 pos = device->size() - int(sizeof(EndOfDirectory)) - i;
		if ((pos < 0) || (i > 65535))
			return false;

		if (!(device->seek(pos) && device->read((char *)&eod,
		  sizeof(EndOfDirectory)) == sizeof(EndOfDirectory)))
			return false;
		if (UINT32(eod.signature) == 0x06054b50)
			break;
	}

	quint32 offset = UINT32(eod.dir_start_offset);
	quint16 numEntries = UINT16(eod.num_dir_entries);

	if (!device->seek(offset))
		return false;
	for (int i = 0; i < numEntries; i++) {
		CentralFileHeader h;
		int read = device->read((char *)&h, sizeof(CentralFileHeader));
		if (read < (int)sizeof(CentralFileHeader))
			return false;
		if (UINT32(h.signature) != 0x02014b50)
			return false;
		if (UINT16(h.version_needed) > 20)
			return false;
		if (UINT16(h.general_purpose_bits) & ENCRYPTED)
			return false;

		quint16 l = UINT16(h.file_name_length);
		QByteArray fileName(device->read(l));
		if (fileName.size() != l)
			return false;

		if (!device->seek(device->pos() + UINT16(h.extra_field_length)
		  + UINT16(h.file_comment_length)))
			return false;

		files.insert(fileName, UINT32(h.offset_local_header));
	}

	return true;
}

Zip::Zip(const QString &path) : _deleteDevice(true), _valid(false)
{
	_device = new QFile(path);
	if (_device && _device->open(QIODevice::ReadOnly))
		_valid = readHeaders(_device, _files);
}

Zip::Zip(QIODevice *device)
  : _device(device), _deleteDevice(false), _valid(false)
{
	if (device)
		_valid = readHeaders(_device, _files);
}

Zip::~Zip()
{
	if (_deleteDevice)
		delete _device;
}

QByteArray Zip::file(const QString &fileName) const
{
	if (!_valid)
		return QByteArray();

	QHash<QString, quint32>::const_iterator it(_files.find(fileName));
	if (it == _files.constEnd())
		return QByteArray();

	LocalFileHeader lh;
	if (!(_device->seek(it.value()) && _device->read((char*)&lh,
	  sizeof(LocalFileHeader)) == sizeof(LocalFileHeader)))
		return QByteArray();
	quint16 skip = UINT16(lh.file_name_length) + UINT16(lh.extra_field_length);
	if (!_device->seek(_device->pos() + skip))
		return QByteArray();

	quint16 compressionMethod = UINT16(lh.compression_method);
	if (compressionMethod == COMPRESSION_NONE)
		return _device->read(UINT32(lh.compressed_size));
	else if (compressionMethod == CCOMPRESSION_DEFLATE) {
		QByteArray ba(_device->read(UINT32(lh.compressed_size)));
		QByteArray uba(UINT32(lh.uncompressed_size),
		  Qt::Initialization::Uninitialized);

		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		strm.avail_in = ba.size();
		strm.next_in = (Bytef*)ba.constData();
		strm.avail_out = uba.size();
		strm.next_out = (Bytef*)uba.data();

		if (inflateInit2(&strm, -MAX_WBITS) != Z_OK)
			return QByteArray();
		int ret = inflate(&strm, Z_NO_FLUSH);
		(void)inflateEnd(&strm);

		return (ret == Z_STREAM_END) ? uba : QByteArray();
	} else
		return QByteArray();
}

bool Zip::isZIP(QIODevice *device)
{
	quint32 magic;

	return (device->peek((char *)&magic, sizeof(magic)) == (qint64)sizeof(magic)
	  && qFromLittleEndian(magic) == MAGIC);
}
