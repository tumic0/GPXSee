#include "common/garmin.h"
#include "subfile.h"
#include "huffmantext.h"

using namespace IMG;

static inline quint32 readVUint32(const quint8 *buffer, quint32 bytes)
{
	quint32 val = 0;

	for (quint32 i = 0; i < bytes; i++)
		val = val | (quint32)*(buffer - i) << ((bytes - i - 1) << 3);

	return val;
}

bool HuffmanText::load(const RGNFile *rgn, SubFile::Handle &rgnHdl)
{
	if (!_buffer.load(rgn, rgnHdl))
		return false;

	quint8 *buffer = (quint8 *)_buffer.constData();
	_b0 = buffer[0];
	_b1 = buffer[1];
	_b2 = buffer[2];
	_b3 = buffer[3];
	_vs = vs(buffer[4]);
	_bs3 = bs(_b3);
	_bs1 = bs(_b1);
	_mul = _bs1 + 1 + _vs;
	_bp1 = buffer + _vs + 4;
	_bp2 = _bp1 + _mul * _b2;
	_bp3 = _bp2 + ((_bs3 + 1) << (_b0 & 0xf));
	_bp4 = _bp3 - 1;

	return true;
}

bool HuffmanText::fetch(const SubFile *file, SubFile::Handle &hdl,
  quint32 &data, quint32 &bits, quint32 &usedBits, quint32 &usedData) const
{
	quint32 rs, ls, old;

	bits = _b1 - bits;

	if (usedBits < bits) {
		old = usedBits ? usedData >> (0x20 - usedBits) : 0;
		if (!file->readVUInt32SW(hdl, 4, usedData))
			return false;
		ls = bits - usedBits;
		rs = 0x20 - (bits - usedBits);
		old = usedData >> rs | old << ls;
	} else {
		ls = bits;
		rs = usedBits - bits;
		old = usedData >> (0x20 - bits);
	}

	usedData = usedData << ls;
	data = data | old << (0x20 - _b1);
	usedBits = rs;

	return true;
}

bool HuffmanText::decode(const SubFile *file, SubFile::Handle &hdl,
  QVector<quint8> &str) const
{
	quint32 bits = 0;
	quint32 data = 0;
	quint32 usedBits = 0;
	quint32 usedData = 0;
	quint32 ls = 8;
	quint32 lo = _vs * 8 - 8;


	while (true) {
		if (!fetch(file, hdl, data, bits, usedBits, usedData))
			return false;

		quint32 off = (data >> (0x20 - (_b0 & 0xf))) * (_bs3 + 1);
		quint32 sb = _bp2[off];
		quint32 ss = 0;
		quint32 sym = _b2 - 1;
		quint32 size;

		if ((_b0 & 0xf) == 0 || (sb & 1) == 0) {
			if ((_b0 & 0xf) != 0) {
				ss = sb >> 1;
				sym = _bp2[off + 1];
			}

			quint8 *tp = _bp1 + ss * _mul;
			quint32 sd = data >> (0x20 - _b1);
			while (ss < sym) {
				quint32 cnt = (sym + 1 + ss) >> 1;
				quint8 *prev = _bp1 + cnt * _mul;
				quint32 nd = readVUint32(prev + _bs1 - 1, _bs1);

				if (sd <= nd) {
					sym = cnt - (sd < nd);
					if (sd < nd) {
						prev = tp;
						cnt = ss;
					}
				}
				tp = prev;
				ss = cnt;
			}

			quint32 o1 = readVUint32(tp + _bs1 - 1, _bs1);
			tp = tp + _bs1;
			quint32 o2 = readVUint32(tp + _vs, _vs);
			size = tp[0];
			quint32 os = (sd - o1) >> (_b1 - size);

			if ((_b0 & 0x10) == 0) {
				sym = readVUint32(_bp4 + (o2 + 1 + os) * _bs3, _bs3);
			} else {
				quint32 v = (os + o2) * _b3;
				quint32 idx = v >> 3;
				quint32 r = v & 7;
				quint32 shift = 8 - r;
				sym = _bp3[idx] >> r;
				if (shift < _b3) {
					quint32 sz = bs(_b3 - shift);
					quint32 val = readVUint32(_bp3 + idx + sz, sz);
					sym = sym | val << shift;
				}
			}
		} else {
			sym = readVUint32(_bp2 + off + _bs3, _bs3);
			size = (sb >> 1);
		}

		if (_b1 < size)
			return false;
		data = data << size;
		bits = _b1 - size;

		if ((_b3 & 7) == 0) {
			for (quint32 i = 0; i < (_b3 >> 3); i++) {
				str.append((quint8)sym);
				if (((quint8)sym == '\0'))
					return true;
				sym = sym >> 8;
			}
		} else {
			quint32 cnt = _b3;

			if (ls <= _b3) {
				do {
					quint32 shift = ls;
					lo = sym << (8 - shift) | (quint32)((quint8)lo >> shift);
					sym = sym >> shift;
					str.append((uchar)lo);
					if (((uchar)lo == '\0'))
						return true;
					cnt = cnt - ls;
					ls = 8;
				} while (7 < cnt);
				ls = 8;
			}
			if (cnt != 0) {
				lo = sym << (8 - cnt) | (quint32)((quint8)lo >> cnt);
				ls = ls - cnt;
			}
		}
	}
}
