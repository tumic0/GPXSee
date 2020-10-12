#ifndef TIFFFILE_H
#define TIFFFILE_H

#include <QIODevice>
#include <QtEndian>

#define TIFF_BYTE      1
#define TIFF_ASCII     2
#define TIFF_SHORT     3
#define TIFF_LONG      4
#define TIFF_RATIONAL  5
#define TIFF_SRATIONAL 10
#define TIFF_DOUBLE    12

class TIFFFile
{
public:
	TIFFFile(QIODevice *device);

	bool isValid() const {return _ifd != 0;}
	bool isBE() const {return _be;}
	quint32 ifd() const {return _ifd;}

	bool seek(qint64 pos) {return _device->seek(_offset + pos);}
	qint64 pos() const {return _offset + _device->pos();}
	QByteArray read(qint64 maxSize) {return _device->read(maxSize);}

	template<class T> bool readValue(T &val)
	{
		T data;

		if (_device->read((char*)&data, sizeof(T)) < (qint64)sizeof(T))
			return false;

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
		if (_be)
			val = data;
		else {
			for (size_t i = 0; i < sizeof(T); i++)
				*((char *)&val + i) = *((char*)&data + sizeof(T) - 1 - i);
		}
#else
		if (_be) {
			for (size_t i = 0; i < sizeof(T); i++)
				*((char *)&val + i) = *((char*)&data + sizeof(T) - 1 - i);
		} else
			val = data;
#endif

		return true;
	}

private:
	QIODevice *_device;
	bool _be;
	quint32 _ifd;
	qint64 _offset;
};

#endif // TIFFFILE_H
