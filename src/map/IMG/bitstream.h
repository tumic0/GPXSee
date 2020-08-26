#ifndef BITSTREAM_H
#define BITSTREAM_H

#include "subfile.h"

class BitStream1 {
public:
	BitStream1(const SubFile &file, SubFile::Handle &hdl, quint32 length)
	  : _file(file), _hdl(hdl), _length(length), _remaining(0) {}

	bool read(int bits, quint32 &val);
	bool flush();
	quint64 bitsAvailable() const {return (quint64)_length * 8 + _remaining;}

private:
	const SubFile &_file;
	SubFile::Handle &_hdl;
	quint32 _length, _remaining;
	quint8 _data;
};

class BitStream4 {
public:
	BitStream4(const SubFile &file, SubFile::Handle &hdl, quint32 length)
	  : _file(file), _hdl(hdl), _length(length), _used(32), _unused(0),
	  _data(0) {}

	bool flush();
	quint64 bitsAvailable() const
	  {return (quint64)_length * 8 + (32 - _used) - _unused;}

protected:
	const SubFile &_file;
	SubFile::Handle &_hdl;
	quint32 _length, _used, _unused;
	quint32 _data;
};

class BitStream4F : public BitStream4 {
public:
	BitStream4F(const SubFile &file, SubFile::Handle &hdl, quint32 length)
	  : BitStream4(file, hdl, length) {}

	bool read(int bits, quint32 &val);
};

class BitStream4R : public BitStream4 {
public:
	BitStream4R(const SubFile &file, SubFile::Handle &hdl, quint32 length);

	template<typename T> bool read(int bits, T &val);
	bool readBytes(int bytes, quint32 &val);
	bool readVUInt32(quint32 &val);
	bool readVuint32SM(quint32 &val1, quint32 &val2, quint32 &val2Bits);

	bool skip(quint32 bytes);
	void resize(quint32 length);
};

template<typename T>
bool BitStream4R::read(int bits, T &val)
{
	if (bits <= 32 - (int)(_used + _unused)) {
		val = bits ? (_data << _used) >> (32 - bits) : 0;
		_used += bits;
		return true;
	}

	if (_unused)
		return false;
	quint32 old = (_used < 32) ? (_data << _used) >> (32 - bits) : 0;
	quint32 bytes = qMin(_length, 4U);

	if (!_file.readUInt32(_hdl, _data))
		return false;
	if (!_file.seek(_hdl, _file.pos(_hdl) - 8))
		return false;

	_length -= bytes;
	_used -= 32 - bits;
	_unused = (4 - bytes) * 8;

	val = _data >> (32 - _used) | old;

	return true;
}

#endif // BITSTREAM_H
