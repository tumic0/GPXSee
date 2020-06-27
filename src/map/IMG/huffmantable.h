#ifndef HUFFMANTABLE_H
#define HUFFMANTABLE_H

#include "subfile.h"

class HuffmanTable {
public:
	HuffmanTable() : _s2(0) {}

	bool load(const SubFile &file, SubFile::Handle &hdl, quint32 offset,
	  quint32 size, quint32 id);
	bool isNull() const {return _s2 == 0;}
	quint8 maxSymbolSize() const {return _s2;}
	quint32 symbol(quint32 data, quint8 &size) const;

	quint8 id() const {return _id;}

private:
	bool getBuffer(const SubFile &file, SubFile::Handle &hdl, quint32 offset,
	  quint32 size, quint8 id);

	QByteArray _buffer;
	quint8 _s0, _s1, _s2, _s3;
	quint8 *_s10, *_s14, *_s18;
	quint8 _s1c, _s1d, _s1e, _s1f, _s20;
	quint16 _s22;

	quint8 _id;
};

#endif // HUFFMANTABLE_H
