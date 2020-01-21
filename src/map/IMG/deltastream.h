#ifndef DELTASTREAM_H
#define DELTASTREAM_H

#include "bitstream.h"

class DeltaStream : public BitStream1 {
public:
	DeltaStream(const SubFile &file, SubFile::Handle &hdl, quint32 length,
	  quint8 info, bool extraBit, bool extended);

	bool readNext(qint32 &lonDelta, qint32 &latDelta)
	{
		return hasNext()
			? (readDelta(_lonBits, _lonSign, _extraBit, lonDelta)
			  && readDelta(_latBits, _latSign, false, latDelta)) : false;
	}
	bool atEnd() const {return (_readBits != 0xFFFFFFFF && !hasNext());}

private:
	bool hasNext() const {return bitsAvailable() >= _readBits;}
	bool sign(int &val);
	bool readDelta(int bits, int sign, int extraBit, qint32 &delta);

	int _lonSign, _latSign, _extraBit;
	quint32 _lonBits, _latBits, _readBits;
};

#endif // DELTASTREAM_H
