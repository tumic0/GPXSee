#ifndef MAPSFORGE_SUBFILE_H
#define MAPSFORGE_SUBFILE_H

#include <QFile>

namespace Mapsforge {

class SubFile
{
public:
	SubFile(QFile &file, quint64 offset, quint64 size)
	  : _file(file), _offset(offset), _end(offset + size) {}

	quint64 pos() const {return _file.pos() - _offset;}
	bool seek(quint64 pos)
	{
		if (pos >= (quint64)_end)
			return false;
		return _file.seek(pos + _offset);
	}

	bool readByte(quint8 &val)
	{
		if (_file.pos() >= _end)
			return false;
		return (_file.read((char*)&val, 1) == 1);
	}

	template<typename T>
	bool readUInt16(T &val)
	{
		quint8 b0, b1;
		if (!(readByte(b0) && readByte(b1)))
			return false;
		val = b1 | ((quint16)b0) << 8;
		return true;
	}

	bool readUInt32(quint32 &val)
	{
		quint8 b0, b1, b2, b3;
		if (!(readByte(b0) && readByte(b1) && readByte(b2) && readByte(b3)))
			return false;
		val = b3 | ((quint32)b2) << 8 | ((quint32)b1) << 16 | ((quint32)b0) << 24;
		return true;
	}

	bool readUInt64(quint64 &val)
	{
		quint8 b0, b1, b2, b3, b4, b5, b6, b7;
		if (!(readByte(b0) && readByte(b1) && readByte(b2) && readByte(b3)
		 && readByte(b4) && readByte(b5) && readByte(b6) && readByte(b7)))
			return false;
		val = b7 | ((quint64)b6) << 8 | ((quint64)b5) << 16
		  | ((quint64)b4) << 24 | ((quint64)b3) << 32 | ((quint64)b2) << 40
		  | ((quint64)b1) << 48 | ((quint64)b0) << 56;
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
		if (_file.pos() + len >= _end)
			return false;

		str.resize(len);
		if (_file.read(str.data(), len) != len)
			return false;

		return true;
	}

private:
	QFile &_file;
	qint64 _offset;
	qint64 _end;
};

}

#endif // MAPSFORGE_SUBFILE_H
