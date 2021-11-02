#ifndef IMG_HUFFMANTEXT_H
#define IMG_HUFFMANTEXT_H

#include "huffmantable.h"

namespace IMG {

class HuffmanText
{
public:
	HuffmanText() : _table(0) {}

	bool load(const RGNFile *rgn, SubFile::Handle &rgnHdl);
	bool decode(const SubFile *file, SubFile::Handle &hdl, quint32 size,
	  QVector<quint8> &str) const;

private:
	HuffmanTable _table;
};

}

#endif // IMG_HUFFMANTEXT_H
