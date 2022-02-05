#include "bitstream.h"

using namespace Garmin;
using namespace IMG;

bool BitStream1::flush()
{
	if (_length && !_file.seek(_hdl, _file.pos(_hdl) + _length))
		return false;

	_length = 0;
	_remaining = 0;

	return true;
}

bool BitStream1::readUInt24(quint32 &val)
{
	quint8 b;

	val = 0;

	for (int i = 0; i < 3; i++) {
		if (!read(8, b))
			return false;
		val |= (b << (i << 3));
	}

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

bool BitStream4F::read(quint32 bits, quint32 &val)
{
	if (bits <= available()) {
		val = bits ? (_data << _used) >> (32U - bits) : 0;
		_used += bits;
		return true;
	}

	Q_ASSERT(_length && !_unused);
	quint32 old = (_used < 32U) ? (_data << _used) >> (32U - bits) : 0;
	quint32 bytes = qMin(_length, 4U);

	if (!_file.readVUInt32SW(_hdl, bytes, _data))
		return false;

	_used -= 32U - bits;
	_length -= bytes;
	_unused = (4U - bytes) << 3;
	_data <<= _unused;

	val = _data >> (32 - _used) | old;

	return true;
}

BitStream4R::BitStream4R(const SubFile &file, SubFile::Handle &hdl,
  quint32 length) : BitStream4(file, hdl, length)
{
	_file.seek(_hdl, _file.pos(_hdl) - 4);
}

bool BitStream4R::readBytes(quint32 bytes, quint32 &val)
{
	quint32 b;

	val = 0;
	for (quint32 i = 0; i < bytes; i++) {
		if (!read(8, b))
			return false;
		val |= (b << (i << 3));
	}

	return true;
}

bool BitStream4R::readBytesAligned(quint32 bytes, quint32 &val)
{
	quint32 bits = _used & 7U;
	quint32 b;

	if (bits && !read(8U - bits, b))
		return false;

	return readBytes(bytes, val);
}

bool BitStream4R::readVUInt32(quint32 &val)
{
	quint32 b;
	quint8 bytes, shift;

	if (!readBytesAligned(1, b))
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

bool BitStream4R::readVUint32SM(quint32 &val1, quint32 &val2, quint32 &val2Bits)
{
	quint32 b, eb;

	if (!readBytesAligned(1, b))
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
	quint32 ab = available() >> 3;

	if (bytes <= ab)
		_used += bytes << 3;
	else {
		if (bytes > ab + _length)
			return false;

		quint32 seek = (bytes - ab) & 0xFFFFFFFC;
		quint32 read = (bytes - ab) & 3U;
		if (seek && !_file.seek(_hdl, _file.pos(_hdl) - seek))
			return false;
		_length -= seek;
		if (read) {
			quint32 rb = qMin(_length, 4U);
			if (!_file.readUInt32(_hdl, _data))
				return false;
			if (!_file.seek(_hdl, _file.pos(_hdl) - 8U))
				return false;
			_length -= rb;
			_unused = (4U - rb) << 3;
			_used = read << 3;
		} else
			_used = 32U;
	}

	return true;
}

void BitStream4R::resize(quint32 bytes)
{
	quint32 ab = bs(available());

	if (ab <= bytes)
		_length = bytes - ab;
	else {
		_length = 0;
		_unused += (ab - bytes) << 3;
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
