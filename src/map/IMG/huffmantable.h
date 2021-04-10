#ifndef IMG_HUFFMANTABLE_H
#define IMG_HUFFMANTABLE_H

#include "huffmanbuffer.h"

namespace IMG {

class RGNFile;

class HuffmanTable {
public:
	HuffmanTable(quint8 id) : _buffer(id) {}

	bool load(const RGNFile *rgn, SubFile::Handle &rgnHdl);
	quint8 maxSymbolSize() const {return _s2;}
	quint32 symbol(quint32 data, quint8 &size) const;

	quint8 id() const {return _buffer.id();}

private:
	HuffmanBuffer _buffer;
	quint8 _s0, _s1, _s2, _s3;
	quint8 *_s10, *_s14, *_s18;
	quint8 _s1c, _s1d, _s1e, _s1f, _s20;
	quint16 _s22;
};

}

#endif // IMG_HUFFMANTABLE_H
