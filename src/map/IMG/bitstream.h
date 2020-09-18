#ifndef BITSTREAM_H
#define BITSTREAM_H

#include "subfile.h"

class BitStream1 {
public:
	BitStream1(const SubFile &file, SubFile::Handle &hdl, quint32 length)
	  : _file(file), _hdl(hdl), _length(length), _remaining(0) {}

	template<typename T> bool read(int bits, T &val);
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
	bool flush();
};

class BitStream4R : public BitStream4 {
public:
	struct State {
		quint32 pos;
		quint32 length;
		quint32 used;
		quint32 unused;
		quint32 data;
	};

	BitStream4R(const SubFile &file, SubFile::Handle &hdl, quint32 length);

	template<typename T> bool read(int bits, T &val);
	bool readBytes(int bytes, quint32 &val);
	bool readVUInt32(quint32 &val);
	bool readVuint32SM(quint32 &val1, quint32 &val2, quint32 &val2Bits);

	bool skip(quint32 bytes);
	void resize(quint32 bytes);
	void save(State &state);
	bool restore(const State &state);
};


template<typename T>
bool BitStream1::read(int bits, T &val)
{
	val = 0;

	for (int pos = 0; pos < bits; ) {
		if (!_remaining) {
			if (!_length || !_file.readUInt8(_hdl, _data))
				return false;
			_remaining = 8;
			_length--;
		}

		quint32 get = bits - pos;
		if (get >= _remaining) {
			val |= _data << pos;
			pos += _remaining;
			_remaining = 0;
		} else {
			quint32 mask = (1<<get) - 1;
			val |= (_data & mask)<<pos;
			_data >>= get;
			_remaining -= get;
			break;
		}
	}

	return true;
}

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
