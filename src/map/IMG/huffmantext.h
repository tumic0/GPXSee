#ifndef IMG_HUFFMANTEXT_H
#define IMG_HUFFMANTEXT_H

#include "huffmantable.h"

namespace IMG {

class HuffmanText
{
public:
	HuffmanText() : _table(0) {}

	bool load(const RGNFile *rgn, SubFile::Handle &rgnHdl)
	  {return _table.load(rgn, rgnHdl);}
	bool decode(const SubFile *file, SubFile::Handle &hdl,
	  QVector<quint8> &str) const;

private:
	bool fetch(const SubFile *file, SubFile::Handle &hdl, quint32 &data,
	  quint32 &bits, quint32 &usedBits, quint32 &usedData) const;

	HuffmanTable _table;
};

}

#endif // IMG_HUFFMANTEXT_H
