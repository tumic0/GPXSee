#ifndef IMG_HUFFMANTABLE_H
#define IMG_HUFFMANTABLE_H

#include "huffmanbuffer.h"

namespace IMG {

class RGNFile;

class HuffmanTable {
public:
	HuffmanTable(quint8 id) : _buffer(id) {}

	bool load(const RGNFile *rgn, SubFile::Handle &rgnHdl);
	quint8 maxDataBits() const {return _symBits;}
	quint32 symbol(quint32 data, quint8 &size) const;

	quint8 id() const {return _buffer.id();}

private:
	HuffmanBuffer _buffer;
	const quint8 *_aclTable, *_bsrchTable, *_huffmanTable;
	quint8 _aclBits, _aclEntryBytes, _symBits, _symBytes, _indexBytes,
	  _bsrchEntryBytes, _bsrchEntries, _symbolBits, _symbolBytes;
	bool _huffman;
};

}

#endif // IMG_HUFFMANTABLE_H
