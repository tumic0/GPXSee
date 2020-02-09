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


bool BitStream4::read(int bits, quint32 &val)
{
	if (bits <= 32 - (int)(_used + _unused)) {
		val = bits ? (_data << _used) >> (32 - bits) : 0;
		_used += bits;
		return true;
	}

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

bool BitStream4::flush()
{
	if (_length && !_file.seek(_hdl, _hdl.pos() + _length))
		return false;

	_length = 0;
	_used = 32;
	_unused = 0;

	return true;
}
