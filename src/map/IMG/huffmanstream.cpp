#include "huffmanstream.h"

bool HuffmanStreamF::init(bool line)
{
	if (line) {
		if (!(sign(_lonSign) && sign(_latSign)))
			return false;
	} else {
		_lonSign = 0;
		_latSign = 0;
	}

	quint32 eb;
	if (!_bs.read(1, eb))
		return false;
	if (eb) {
		qWarning() << "Extended lines/polygons not supported";
		return false;
	}

	return true;
}

bool HuffmanStreamR::init()
{
	if (!(sign(_lonSign) && sign(_latSign)))
		return false;

	return true;
}

bool HuffmanStreamR::init(int lonSign, int latSign, quint32 data,
  quint32 dataSize)
{
	_lonSign = lonSign;
	_latSign = latSign;
	_symbolData = data;
	_symbolDataSize = dataSize;

	return true;
}
