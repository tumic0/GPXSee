#include <QtEndian>
#include <QFile>
#ifdef _MSC_VER
#include <QtZlib/zlib.h>
#else // _MSC_VER
#include <zlib.h>
#endif // _MSC_VER
#include "zip.h"

#define MAGIC 0x04034b50
#define ENCRYPTED 0x01
#define COMPRESSION_NONE    0
#define COMPRESSION_DEFLATE 8

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

static bool findEOD(QIODevice *device, EndOfDirectory *eod)
{
	qint64 ds = device->size() - (qint64)sizeof(EndOfDirectory);

	for (qint64 pos = ds; pos >= qMax(ds - 65535, 0ll); pos--) {
		if (!(device->seek(pos) && device->read((char*)eod,
		  sizeof(EndOfDirectory)) == sizeof(EndOfDirectory)))
			break;
		if (UINT32(eod->signature) == 0x06054b50)
			return true;
	}

	return false;
}

bool Zip::readHeaders()
{
	quint32 magic;
	if (!(_device->read((char*)&magic, sizeof(magic)) == sizeof(magic)
	  && qFromLittleEndian(magic) == MAGIC)) {
		_errorString = "Not a ZIP file";
		return false;
	}

	EndOfDirectory eod;
	if (!findEOD(_device, &eod)) {
		_errorString = "EOCD record not found";
		return false;
	}

	quint32 offset = UINT32(eod.dir_start_offset);
	quint16 numEntries = UINT16(eod.num_dir_entries);

	if (!_device->seek(offset)) {
		_errorString = "Invalid central directory offset";
		return false;
	}
	for (int i = 0; i < numEntries; i++) {
		CentralFileHeader h;
		if (_device->read((char*)&h, sizeof(CentralFileHeader))
		  != sizeof(CentralFileHeader)) {
			_errorString = "Error reading central directory entry";
			return false;
		}
		if (UINT32(h.signature) != 0x02014b50) {
			_errorString = "Invalid central directory entry";
			return false;
		}
		if (UINT16(h.version_needed) > 20
		  || UINT16(h.general_purpose_bits) & ENCRYPTED) {
			_errorString = "Unsupported ZIP version/feature required";
			return false;
		}

		quint16 l = UINT16(h.file_name_length);
		QByteArray fileName(_device->read(l));
		if (!((fileName.size() == l) && _device->seek(_device->pos()
		  + UINT16(h.extra_field_length) + UINT16(h.file_comment_length)))) {
			_errorString = "Error reading central directory entry data";
			return false;
		}

		_files.insert(fileName, UINT32(h.offset_local_header));
	}

	return true;
}

Zip::Zip(const QString &path) : _deleteDevice(true), _valid(false)
{
	_device = new QFile(path);
	if (_device && _device->open(QIODevice::ReadOnly))
		_valid = readHeaders();
	else
		_errorString = _device ? _device->errorString() : "Internal error";
}

Zip::Zip(QIODevice *device)
  : _device(device), _deleteDevice(false), _valid(false)
{
	if (_device && _device->isReadable())
		_valid = readHeaders();
	else
		_errorString = _device ? "File not readable" : "Internal error";
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

	QMap<QString, quint32>::const_iterator it(_files.find(fileName));
	if (it == _files.constEnd())
		return QByteArray();

	LocalFileHeader lh;
	if (!(_device->seek(it.value()) && _device->read((char*)&lh,
	  sizeof(LocalFileHeader)) == sizeof(LocalFileHeader)))
		return QByteArray();
	quint32 skip = UINT16(lh.file_name_length) + UINT16(lh.extra_field_length);
	if (!_device->seek(_device->pos() + skip))
		return QByteArray();

	quint16 compressionMethod = UINT16(lh.compression_method);
	if (compressionMethod == COMPRESSION_NONE)
		return _device->read(UINT32(lh.compressed_size));
	else if (compressionMethod == COMPRESSION_DEFLATE) {
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

	return (device->peek((char*)&magic, sizeof(magic)) == sizeof(magic)
	  && qFromLittleEndian(magic) == MAGIC);
}
