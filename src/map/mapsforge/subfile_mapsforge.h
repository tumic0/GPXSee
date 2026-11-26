#ifndef MAPSFORGE_SUBFILE_H
#define MAPSFORGE_SUBFILE_H

#include <QFile>

namespace Mapsforge {

class SubFile
{
public:
	SubFile(QFile &file, quint64 offset, quint64 size) : _file(file)
	{
		_data = file.map(offset, size);
		_end = _data ? _data + size : 0;
		_dp = _data;
	}
	~SubFile()
	{
		_file.unmap(_data);
	}

	quint64 pos() const {return _dp - _data;}
	bool seek(quint64 pos)
	{
		if (_data + pos >= _end)
			return false;

		_dp = _data + pos;

		return true;
	}

	bool readByte(quint8 &val)
	{
		if (_dp + 1 > _end)
			return false;

		val = *_dp++;

		return true;
	}

	template<typename T>
	bool readUInt16(T &val)
	{
		if (_dp + 2 > _end)
			return false;

		val = *(_dp + 1) | ((quint16)*_dp) << 8;
		_dp += 2;

		return true;
	}

	bool readUInt32(quint32 &val)
	{
		if (_dp + 4 > _end)
			return false;

		val = *(_dp + 3) | ((quint32)*(_dp + 2)) << 8
		  | ((quint32)*(_dp + 1)) << 16 | ((quint32)*_dp) << 24;
		_dp += 4;

		return true;
	}

	bool readUInt64(quint64 &val)
	{
		if (_dp + 8 > _end)
			return false;
		val = *(_dp + 7) | ((quint64)*(_dp + 6)) << 8
		  | ((quint64)*(_dp + 5)) << 16 | ((quint64)*(_dp + 4)) << 24
		  | ((quint64)*(_dp + 3)) << 32 | ((quint64)*(_dp + 2)) << 40
		  | ((quint64)*(_dp + 1)) << 48 | ((quint64)*_dp) << 56;
		_dp += 8;

		return true;
	}

	bool readInt32(qint32 &val)
	{
		return readUInt32(reinterpret_cast<quint32&>(val));
	}

	bool readVUInt32(quint32 &val)
	{
		int shift = 0;
		quint8 b;

		val = 0;
		do {
			if (!readByte(b))
				return false;
			val |= (quint32)(b & 0x7F) << shift;
			shift += 7;
		} while (b & 0x80);

		return true;
	}

	bool readVInt32(qint32 &val)
	{
		int shift = 0;
		quint8 b;

		val = 0;
		while (true) {
			if (!readByte(b))
				return false;
			if (b & 0x80) {
				val |= (qint32)(b & 0x7F) << shift;
				shift += 7;
			} else {
				val |= (qint32)(b & 0x3F) << shift;
				if (b & 0x40)
					val = -val;
				break;
			}
		}

		return true;
	}

	bool readString(QByteArray &str)
	{
		quint32 len;

		if (!readVUInt32(len))
			return false;
		if (_dp + len > _end)
			return false;

		str.resize(len);
		memcpy(str.data(), _dp, len);
		_dp += len;

		return true;
	}

private:
	QFile &_file;
	uchar *_data;
	uchar *_end;
	uchar *_dp;
};

}

#endif // MAPSFORGE_SUBFILE_H
