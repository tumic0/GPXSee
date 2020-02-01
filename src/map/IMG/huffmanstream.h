#ifndef HUFFMANSTREAM_H
#define HUFFMANSTREAM_H

#include "bitstream.h"
#include "huffmantable.h"

class HuffmanStream : public BitStream4 {
public:
	HuffmanStream(const SubFile &file, SubFile::Handle &hdl, quint32 length,
	  const HuffmanTable &table, bool line);

	bool readNext(qint32 &lonDelta, qint32 &latDelta)
	{
		if (!(readDelta(_lonSign, lonDelta) && readDelta(_latSign, latDelta)))
			return false;

		return (lonDelta || latDelta);
	}

	bool readOffset(qint32 &lonDelta, qint32 &latDelta)
	  {return (readDelta(1, lonDelta) && readDelta(1, latDelta));}
	bool atEnd() const
	  {return _symbolDataSize + bitsAvailable() < _table.maxSymbolSize();}

private:
	bool sign(int &val);
	bool readDelta(int sign, qint32 &delta);

	const HuffmanTable &_table;
	quint32 _symbolDataSize;
	quint32 _symbolData;
	int _lonSign, _latSign;
};

#endif // HUFFMANSTREAM_H
