#ifndef TIFFFILE_H
#define TIFFFILE_H

#include <QFile>
#include <QtEndian>

class TIFFFile : public QFile
{
public:
	TIFFFile() : _be(false) {}
	TIFFFile(const QString &path) : QFile(path), _be(false) {}

	bool readHeader(quint32 &ifd);
	template<class T> bool readValue(T &val)
	{
		T data;

		if (QFile::read((char*)&data, sizeof(T)) < (qint64)sizeof(T))
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
	bool _be;
};

#endif // TIFFFILE_H
