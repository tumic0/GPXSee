#include "common/garmin.h"
#include "subfile.h"
#include "huffmantext.h"

using namespace Garmin;
using namespace IMG;

bool HuffmanText::fetch(const SubFile *file, SubFile::Handle &hdl,
  quint32 &data, quint32 &bits, quint32 &usedBits, quint32 &usedData) const
{
	quint32 rs, ls, old;

	bits = _table.symBits() - bits;

	if (usedBits < bits) {
		old = usedBits ? usedData >> (32 - usedBits) : 0;
		if (!file->readVUInt32SW(hdl, 4, usedData))
			return false;
		ls = bits - usedBits;
		rs = 32 - (bits - usedBits);
		old = usedData >> rs | old << ls;
	} else {
		ls = bits;
		rs = usedBits - bits;
		old = usedData >> (32 - bits);
	}

	usedData = usedData << ls;
	data = data | old << (32 - _table.symBits());
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

	while (true) {
		if (!fetch(file, hdl, data, bits, usedBits, usedData))
			return false;

		quint8 size;
		quint32 sym = _table.symbol(data, size);

		if (_table.symBits() < size)
			return false;
		data = data << size;
		bits = _table.symBits() - size;

		if (!(_table.symbolBits() & 7)) {
			for (quint32 i = 0; i < (_table.symbolBits() >> 3); i++) {
				str.append((quint8)sym);
				if (((quint8)sym == '\0'))
					return true;
				sym = sym >> 8;
			}
		} else {
			Q_ASSERT(false);
			return false;
		}
	}
}
