#ifndef IMG_HUFFMANTEXT_H
#define IMG_HUFFMANTEXT_H

#include "huffmanbuffer.h"

namespace IMG {

class HuffmanText
{
public:
	HuffmanText() : _buffer(0) {}

	bool load(const RGNFile *rgn, SubFile::Handle &rgnHdl);
	bool decode(const SubFile *file, SubFile::Handle &hdl,
	  QVector<quint8> &str) const;

private:
	bool fetch(const SubFile *file, SubFile::Handle &hdl, quint32 &data,
	  quint32 &bits, quint32 &usedBits, quint32 &usedData) const;

	HuffmanBuffer _buffer;

	quint32 _b0;
	quint32 _b1;
	quint32 _b2;
	quint32 _b3;
	quint32 _vs;
	quint32 _bs3;
	quint32 _bs1;
	quint32 _mul;
	quint8 *_bp1;
	quint8 *_bp2;
	quint8 *_bp3;
	quint8 *_bp4;
};

}

#endif // IMG_HUFFMANTEXT_H
