#include "netfile.h"

bool NETFile::init()
{
	Handle hdl;

	if (!(seek(hdl, 0x15) && readUInt32(hdl, _offset)
	  && readUInt32(hdl, _size) && readByte(hdl, _multiplier)))
		return false;

	_multiplier = 1<<_multiplier;

	return true;
}

bool NETFile::lblOffset(Handle &hdl, quint32 netOffset, quint32 &lblOffset)
{
	if (!_init) {
		if (!(_init = init()))
			return false;
	}

	if (!(seek(hdl, _offset + netOffset * _multiplier)
	  && readUInt24(hdl, lblOffset)))
		return false;

	lblOffset &= 0x3FFFFF;

	return true;
}
