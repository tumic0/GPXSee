#include "subfile.h"
#include "huffmanstream.h"
#include "huffmantext.h"

using namespace IMG;

bool HuffmanText::load(const RGNFile *rgn, SubFile::Handle &rgnHdl)
{
	if (!_table.load(rgn, rgnHdl))
		return false;

	Q_ASSERT(!(_table.symbolBits() & 7));
	return !(_table.symbolBits() & 7);
}

bool HuffmanText::decode(const SubFile *file, SubFile::Handle &hdl,
  quint32 size, QVector<quint8> &str) const
{
	BitStream4F bs(*file, hdl, size);
	HuffmanStream<BitStream4F> hs(bs, _table);
	quint32 sym;

	while (hs.readSymbol(sym)) {
		for (int i = 0; i < (_table.symbolBits() >> 3); i++) {
			str.append((quint8)sym);
			if (((quint8)sym == '\0'))
				return true;
			sym = sym >> 8;
		}
	}

	return false;
}
