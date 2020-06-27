#include "bitstream.h"


bool BitStream1::read(int bits, quint32 &val)
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

bool BitStream1::flush()
{
	if (_length && !_file.seek(_hdl, _hdl.pos() + _length))
		return false;

	_length = 0;
	_remaining = 0;

	return true;
}

bool BitStream4::flush()
{
	if (_length && !_file.seek(_hdl, _hdl.pos() + _length))
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
	_file.seek(_hdl, _hdl.pos() - 4);
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

	return read(bytes * 8, val);
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
		val1 = b >> 3 & 0x1f;
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
		if (seek && !_file.seek(_hdl, _hdl.pos() - seek))
			return false;
		_length -= seek;
		if (read) {
			quint32 rb = qMin(_length, 4U);
			if (!_file.readUInt32(_hdl, _data))
				return false;
			if (!_file.seek(_hdl, _hdl.pos() - 8))
				return false;
			_length -= rb;
			_unused = (4 - rb) * 8;
			_used = read * 8;
		} else
			_used = 32;
	}

	return true;
}

void BitStream4R::resize(quint32 length)
{
	quint32 ab = (32 - _used)/8;

	if (ab < length)
		_length = length - ab;
	else {
		_length = 0;
		_used += length * 8;
	}
}
