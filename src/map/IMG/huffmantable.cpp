#include "common/garmin.h"
#include "huffmantable.h"

using namespace Garmin;
using namespace IMG;

static inline quint32 readVUint32(const quint8 *buffer, quint32 bytes)
{
	quint32 val = 0;

	for (quint32 i = 0; i < bytes; i++)
		val |= ((quint32)*(buffer + i)) << (i * 8);

	return val;
}

bool HuffmanTable::load(const RGNFile *rgn, SubFile::Handle &rgnHdl)
{
	if (!_buffer.load(rgn, rgnHdl))
		return false;

	_aclBits = (quint8)_buffer.at(0) & 0x0F;
	_huffman = (quint8)_buffer.at(0) & 0x10;
	_symBits = (quint8)_buffer.at(1);
	_symBytes = bs(_symBits);
	_bsrchEntries = (quint8)_buffer.at(2);
	_symbolBits = (quint8)_buffer.at(3);
	_symbolBytes = bs(_symbolBits);
	_aclEntryBytes = _symbolBytes + 1;
	_indexBytes = vs(_buffer.at(4));
	_bsrchEntryBytes = _symBytes + _indexBytes + 1;
	_bsrchTable = (const quint8*)(_buffer.constData()) + 4 + _indexBytes;
	_aclTable = _bsrchTable + _bsrchEntryBytes * _bsrchEntries;
	_huffmanTable = _aclTable + (_aclEntryBytes << _aclBits);

	return (_symBits <= 32 && _symbolBits <= 32 && _symbolBits >= 8);
}

quint32 HuffmanTable::symbol(quint32 data, quint8 &size) const
{
	quint32 lo, hi;
	const quint8 *tp;


	if (!_aclBits) {
		hi = _bsrchEntries - 1;
		lo = 0;
	} else {
		quint32 offset = _aclEntryBytes * (data >> (32 - _aclBits));
		tp = _aclTable + offset;

		if (*tp & 1) {
			size = *tp >> 1;
			return readVUint32(tp + 1, _symbolBytes);;
		}

		lo = *tp >> 1;
		hi = *(tp + 1);
	}

	tp = _bsrchTable + (lo * _bsrchEntryBytes);
	data >>= 32 - _symBits;

	while (lo < hi) {
		const quint8 *prev = tp;
		quint32 m = (lo + 1 + hi) >> 1;
		tp = _bsrchTable + (m * _bsrchEntryBytes);
		quint32 nd = readVUint32(tp, _symBytes);

		if (data <= nd) {
			if (data == nd)
				lo = m;
			else
				tp = prev;

			hi = m - (data < nd);
			m = lo;
		}
		lo = m;
	}

	quint32 i = readVUint32(tp, _symBytes);
	tp = tp + _symBytes;
	size = *tp;

	if (!_huffman)
		return readVUint32(tp + 1, _symbolBytes);
	else {
		quint32 bi = readVUint32(tp + 1, _indexBytes);
		quint32 ci = (data - i) >> (_symBits - size);
		quint32 si = (ci + bi) * _symbolBits;
		quint32 sbi = si & 7;
		quint32 shift = 8 - sbi;
		tp = _huffmanTable + (si >> 3);
		quint32 val = readVUint32(tp + 1, bs(_symbolBits - shift));

		return (val << shift) | (*tp >> sbi);
	}
}
