#include "deltastream.h"


using namespace IMG;

static int bitSize(quint8 baseSize, bool variableSign, bool extraBit)
{
	int bits = 2;
	if (baseSize <= 9)
		bits += baseSize;
	else
		bits += 2 * baseSize - 9;

	if (variableSign)
		bits++;
	if (extraBit)
		bits++;

	return bits;
}

DeltaStream::DeltaStream(const SubFile &file, SubFile::Handle &hdl,
  quint32 length, quint8 info, bool extraBit, bool extended)
  : BitStream1(file, hdl, length), _readBits(0xFFFFFFFF)
{
	_extraBit = extraBit ? 1 : 0;
	if (!(sign(_lonSign) && sign(_latSign)))
		return;
	if (extended) {
		quint32 b;
		if (!read(1, b))
			return;
	}
	_lonBits = bitSize(info & 0x0F, !_lonSign, extraBit);
	_latBits = bitSize(info >> 4, !_latSign, false);
	_readBits = _lonBits + _latBits;
}

bool DeltaStream::readDelta(int bits, int sign, int extraBit,
  qint32 &delta)
{
	quint32 value;

	if (!read(bits, value))
		return false;

	value >>= extraBit;

	if (!sign) {
		qint32 signMask = 1 << (bits - extraBit - 1);
		if (value & signMask) {
			qint32 comp = value ^ signMask;
			if (comp)
				delta = comp - signMask;
			else {
				qint32 other;
				if (!readDelta(bits - extraBit, sign, false, other))
					return false;
				if (other < 0)
					delta = 1 - signMask + other;
				else
					delta = signMask - 1 + other;
			}
		} else {
			delta = value;
		}
	} else {
		delta = value * sign;
	}

	return true;
}

bool DeltaStream::sign(int &val)
{
	quint32 bit;
	val = 0;

	if (!read(1, bit))
		return false;
	if (bit) {
		if (!read(1, bit))
			return false;
		val = bit ? -1 : 1;
	}

	return true;
}
