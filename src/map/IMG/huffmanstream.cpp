#include "huffmanstream.h"


HuffmanStream::HuffmanStream(const SubFile &file, SubFile::Handle &hdl,
  quint32 length, const HuffmanTable &table, bool line)
  : BitStream4(file, hdl, length), _table(table), _symbolDataSize(0),
  _symbolData(0)
{
	if (line) {
		if (!(sign(_lonSign) && sign(_latSign)))
			return;
	} else {
		_lonSign = 0;
		_latSign = 0;
	}

	quint32 eb;
	if (!read(1, eb))
		return;
	if (eb) {
		qWarning("Extended polygon/lines not supported");
		flush();
	}
}

bool HuffmanStream::readNext(qint32 &lonDelta, qint32 &latDelta)
{
	if (!readDelta(_lonSign, lonDelta))
		return false;
	if (!readDelta(_latSign, latDelta))
		return false;

	if (!(lonDelta|latDelta))
		return false;

	return true;
}

bool HuffmanStream::readOffset(qint32 &lonDelta, qint32 &latDelta)
{
	if (!readDelta(1, lonDelta))
		return false;
	if (!readDelta(1, latDelta))
		return false;

	return true;
}

bool HuffmanStream::sign(int &val)
{
	quint32 bit;
	val = 0;

	if (!read(1, bit))
		return false;
	if (bit) {
		if (!read(1, bit))
			return false;
		val = bit ? -1 : 1;
	}

	return true;
}

bool HuffmanStream::readDelta(int sign, qint32 &symbol)
{
	uchar size;

	if (_symbolDataSize < 32) {
		quint32 next;
		quint8 nextSize = qMin((quint32)(32 - _symbolDataSize),
		  bitsAvailable());

		if (!read(nextSize, next))
			return false;

		_symbolData = (_symbolData << nextSize) | next;
		_symbolDataSize += nextSize;
	}

	symbol = _table.symbol(_symbolData << (32U - _symbolDataSize), size);

	if (size <= _symbolDataSize)
		_symbolDataSize -= size;
	else
		return false;

	if (symbol && !sign) {
		if (!_symbolDataSize)
			return false;
		else {
			sign = ((1U << (_symbolDataSize - 1)) & _symbolData) ? -1 : 1;
			_symbolDataSize--;
		}
	}
	symbol = sign * symbol;

	return true;
}


