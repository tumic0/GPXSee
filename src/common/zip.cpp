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
#define VERSION 20
#define ENCRYPTED 0x01
#define COMPRESSION_NONE     0
#define CCOMPRESSION_DEFLATE 8

#define UINT16(data) qFromLittleEndian<quint16>(data)
#define UINT32(data) qFromLittleEndian<quint32>(data)

Zip::Zip(const QString &path) : _deleteDevice(true), _valid(false)
{
	_device = new QFile(path);
	if (_device && _device->open(QIODeviceBase::ReadOnly))
		_valid = readHeaders();
}

Zip::Zip(QIODevice *device)
  : _device(device), _deleteDevice(false), _valid(false)
{
	if (device)
		_valid = readHeaders();
}

Zip::~Zip()
{
	if (_deleteDevice)
		delete _device;
}

bool Zip::readHeaders()
{
	if (!(_device->isOpen() && (_device->openMode() & QIODevice::ReadOnly)))
		return false;

	quint32 magic;
	if (!(_device->read((char*)&magic, sizeof(MAGIC)) == sizeof(magic)
	  && qFromLittleEndian(magic) == MAGIC))
		return false;

	EndOfDirectory eod;
	for (int i = 0; ; i++) {
		qint64 pos = _device->size() - int(sizeof(EndOfDirectory)) - i;
		if ((pos < 0) || (i > 65535))
			return false;

		if (!(_device->seek(pos) && _device->read((char *)&eod,
		  sizeof(EndOfDirectory)) == sizeof(EndOfDirectory)))
			return false;
		if (UINT32(eod.signature) == 0x06054b50)
			break;
	}

	quint32 offset = UINT32(eod.dir_start_offset);
	quint16 numEntries = UINT16(eod.num_dir_entries);

	if (!_device->seek(offset))
		return false;
	for (int i = 0; i < numEntries; i++) {
		CentralFileHeader h;
		int read = _device->read((char *)&h, sizeof(CentralFileHeader));
		if (read < (int)sizeof(CentralFileHeader))
			return false;
		if (UINT32(h.signature) != 0x02014b50)
			return false;

		quint16 l = UINT16(h.file_name_length);
		QByteArray fileName(_device->read(l));
		if (fileName.size() != l)
			return false;

		if (!_device->seek(_device->pos() + UINT16(h.extra_field_length)
		  + UINT16(h.file_comment_length)))
			return false;

		_fileHeaders.insert(fileName, h);
	}

	return true;
}

QByteArray Zip::file(const QString &fileName) const
{
	if (!_valid)
		return QByteArray();

	QHash<QString, CentralFileHeader>::const_iterator it
	  = _fileHeaders.find(fileName);
	if (it == _fileHeaders.constEnd())
		return QByteArray();

	const CentralFileHeader &h = it.value();
	quint16 versionNeeded = UINT16(h.version_needed);
	if (versionNeeded > VERSION)
		return QByteArray();

	quint16 generalPurposeBits = UINT16(h.general_purpose_bits);
	quint32 compressedSize = UINT32(h.compressed_size);
	quint32 uncompressedSize = UINT32(h.uncompressed_size);
	quint32 start = UINT32(h.offset_local_header);

	LocalFileHeader lh;
	if (!(_device->seek(start) && _device->read((char*)&lh,
	  sizeof(LocalFileHeader)) == sizeof(LocalFileHeader)))
		return QByteArray();
	quint16 skip = UINT16(lh.file_name_length) + UINT16(lh.extra_field_length);
	if (!_device->seek(_device->pos() + skip))
		return QByteArray();

	if ((generalPurposeBits & ENCRYPTED))
		return QByteArray();

	quint16 compressionMethod = UINT16(lh.compression_method);
	if (compressionMethod == COMPRESSION_NONE)
		return _device->read(compressedSize);
	else if (compressionMethod == CCOMPRESSION_DEFLATE) {
		QByteArray ba(_device->read(compressedSize));
		QByteArray uba(uncompressedSize, Qt::Initialization::Uninitialized);

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
