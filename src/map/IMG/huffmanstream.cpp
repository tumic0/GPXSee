#include "huffmanstream.h"

using namespace IMG;

bool HuffmanDeltaStreamF::init(bool line)
{
	if (line) {
		if (!(sign(_lonSign) && sign(_latSign)))
			return false;
	} else {
		_lonSign = 0;
		_latSign = 0;
	}

	quint32 eb;
	if (!read(1, eb))
		return false;

	Q_ASSERT(!eb);
	if (eb)
		return false;

	return true;
}

bool HuffmanDeltaStreamR::init()
{
	if (!(sign(_lonSign) && sign(_latSign)))
		return false;

	return true;
}

bool HuffmanDeltaStreamR::init(quint32 data, quint32 dataSize)
{
	_symbolData = data;
	_symbolDataSize = dataSize;

	if (!(sign(_lonSign) && sign(_latSign)))
		return false;

	return true;
}
