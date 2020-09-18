#include "bitstream.h"


bool BitStream1::flush()
{
	if (_length && !_file.seek(_hdl, _file.pos(_hdl) + _length))
		return false;

	_length = 0;
	_remaining = 0;

	return true;
}

bool BitStream4F::flush()
{
	if (_length && !_file.seek(_hdl, _file.pos(_hdl) + _length))
		return false;

	_length = 0;
	_used = 32;
	_unused = 0;

	return true;
}

bool BitStream4F::read(int bits, quint32 &val)
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

	if (!_file.readVUInt32SW(_hdl, bytes, _data))
		return false;

	_used -= 32 - bits;
	_length -= bytes;
	_unused = (4 - bytes) * 8;
	_data <<= _unused;

	val = _data >> (32 - _used) | old;

	return true;
}

BitStream4R::BitStream4R(const SubFile &file, SubFile::Handle &hdl,
  quint32 length) : BitStream4(file, hdl, length)
{
	_file.seek(_hdl, _file.pos(_hdl) - 4);
}

bool BitStream4R::readBytes(int bytes, quint32 &val)
{
	quint32 bits = _used % 8;
	quint32 b;

	if (bits) {
		if (!read(8 - bits, b))
			return false;
		Q_ASSERT(!b);
	}

	val = 0;
	for (int i = 0; i < bytes; i++) {
		if (!read(8, b))
			return false;
		val |= (b << (i * 8));
	}

	return true;
}

bool BitStream4R::readVUInt32(quint32 &val)
{
	quint32 b;
	quint8 bytes, shift;

	if (!readBytes(1, b))
		return false;

	if ((b & 1) == 0) {
		if ((b & 2) == 0) {
			bytes = ((b >> 2) & 1) ^ 3;
			shift = 5;
		} else {
			shift = 6;
			bytes = 1;
		}
	} else {
		shift = 7;
		bytes = 0;
	}

	val = b >> (8 - shift);

	if (bytes) {
		if (!readBytes(bytes, b))
			return false;
		val = val | (b << shift);
	}

	return true;
}

bool BitStream4R::readVuint32SM(quint32 &val1, quint32 &val2, quint32 &val2Bits)
{
	quint32 b, eb;

	if (!readBytes(1, b))
		return false;

	if (!(b & 1)) {
		val1 = b >> 3;
		val2 = b >> 1 & 3;
		val2Bits = 2;
	} else {
		eb = b & 2;
		val2 = b >> 2 & 0x3f;
		val2Bits = eb * 2 + 6;
		if (!readBytes((eb >> 1 | 2) - 1, b))
			return false;
		if (eb) {
			val2 = val2 | (b & 0xf) << 6;
			b = b >> 4 & 0xfff;
		}
		val1 = b;
	}

	return true;
}

bool BitStream4R::skip(quint32 bytes)
{
	if (bytes * 8 > bitsAvailable())
		return false;

	quint32 ab = (32 - (_used + _unused))/8;
	if (bytes <= ab)
		_used += bytes * 8;
	else {
		quint32 seek = ((bytes - ab)/4)*4;
		quint32 read = (bytes - ab)%4;
		if (seek && !_file.seek(_hdl, _file.pos(_hdl) - seek))
			return false;
		_length -= seek;
		if (read) {
			quint32 rb = qMin(_length, 4U);
			if (!_file.readUInt32(_hdl, _data))
				return false;
			if (!_file.seek(_hdl, _file.pos(_hdl) - 8))
				return false;
			_length -= rb;
			_unused = (4 - rb) * 8;
			_used = read * 8;
		} else
			_used = 32;
	}

	return true;
}

void BitStream4R::resize(quint32 bytes)
{
	quint32 ab = (32 - (_used + _unused) + 7)/8;

	if (ab <= bytes)
		_length = bytes - ab;
	else {
		_length = 0;
		_unused += (ab - bytes) * 8;
	}
}

void BitStream4R::save(State &state)
{
	state.pos = _file.pos(_hdl);
	state.length = _length;
	state.used = _used;
	state.unused = _unused;
	state.data = _data;
}

bool BitStream4R::restore(const State &state)
{
	_length = state.length;
	_used = state.used;
	_unused = state.unused;
	_data = state.data;

	return _file.seek(_hdl, state.pos);
}
