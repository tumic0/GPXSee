#ifndef BITSTREAM_H
#define BITSTREAM_H

#include "subfile.h"

class BitStream1 {
public:
	BitStream1(const SubFile &file, SubFile::Handle &hdl, quint32 length)
	  : _file(file), _hdl(hdl), _length(length), _remaining(0) {}

	bool read(int bits, quint32 &val);
	bool flush();
	quint32 bitsAvailable() const {return _length * 8 + _remaining;}

private:
	const SubFile &_file;
	SubFile::Handle &_hdl;
	quint32 _length, _remaining;
	quint8 _data;
};

class BitStream4 {
public:
	BitStream4(const SubFile &file, SubFile::Handle &hdl, quint32 length)
	  : _file(file), _hdl(hdl), _length(length), _used(32), _unused(0),
	  _data(0) {}

	bool read(int bits, quint32 &val);
	bool flush();
	quint32 bitsAvailable() const {return _length * 8 + (32 - _used) - _unused;}

private:
	const SubFile &_file;
	SubFile::Handle &_hdl;
	quint32 _length, _used, _unused;
	quint32 _data;
};

#endif // BITSTREAM_H
