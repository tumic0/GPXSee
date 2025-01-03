#ifndef IMG_SUBFILE_H
#define IMG_SUBFILE_H

#include <QVector>
#include <QFile>
#include "imgdata.h"


#define BLOCK_BITS 12 /* 4096 bytes */

namespace IMG {

class SubFile
{
public:
	enum Type {Unknown, TRE, RGN, LBL, NET, NOD, DEM, TYP, GMP};

	class Handle
	{
	public:
		Handle(const SubFile *subFile, QFile *file = 0)
		  : _file(file), _blockNum(-1), _blockPos(-1), _pos(-1), _delete(false)
		{
			if (!subFile)
				return;

			if (!_file) {
				_file = new QFile(subFile->fileName());
				if (!_file->open(QIODevice::ReadOnly | QIODevice::Unbuffered))
					qWarning("%s: %s", qPrintable(_file->fileName()),
					  qPrintable(_file->errorString()));
				_delete = true;
			}
			_data.resize(subFile->blockSize());
		}
		~Handle()
		{
			if (_delete)
				delete _file;
		}

	private:
		friend class SubFile;

		QFile *_file;
		QByteArray _data;
		int _blockNum;
		int _blockPos;
		int _pos;
		bool _delete;
	};

	SubFile(const IMGData *img)
	  : _gmpOffset(0), _img(img), _blocks(new QVector<quint16>()), _path(0) {}
	SubFile(const SubFile *gmp, quint32 offset) : _gmpOffset(offset),
	  _img(gmp->_img), _blocks(gmp->_blocks), _path(gmp->_path)
	{
		Q_ASSERT(offset);
	}
	SubFile(const QString &path)
	  : _gmpOffset(0), _img(0), _blocks(0), _path(new QString(path)) {}
	~SubFile()
	{
		if (!_gmpOffset) {
			delete _blocks;
			delete _path;
		}
	}

	void addBlock(quint16 block) {_blocks->append(block);}

	bool seek(Handle &handle, quint32 pos) const;
	quint32 pos(Handle &handle) const {return handle._pos;}

	bool read(Handle &handle, char *buff, quint32 size) const;

	bool readByte(Handle &handle, quint8 *val) const
	{
		*val = handle._data.at(handle._blockPos++);
		handle._pos++;
		return (handle._blockPos >= handle._data.size())
		  ? seek(handle, handle._pos) : true;
	}

	template<typename T>
	bool readUInt8(Handle &handle, T &val) const
	{
		quint8 b;
		if (!readByte(handle, &b))
			return false;
		val = b;
		return true;
	}

	template<typename T>
	bool readInt8(Handle &handle, T &val) const
	{
		quint8 b;
		if (!readByte(handle, &b))
			return false;
		val = (qint8)b;
		return true;
	}

	template<typename T>
	bool readUInt16(Handle &handle, T &val) const
	{
		quint8 b0, b1;
		if (!(readByte(handle, &b0) && readByte(handle, &b1)))
			return false;
		val = b0 | ((quint16)b1) << 8;
		return true;
	}

	bool readInt16(Handle &handle, qint16 &val) const
	{
		if (!readUInt16(handle, (quint16&)val))
			return false;
		if ((quint16)val > 0x7FFF)
			val = (val & 0x7FFF) - 0x8000;
		return true;
	}

	bool readUInt24(Handle &handle, quint32 &val) const
	{
		quint8 b0, b1, b2;
		if (!(readByte(handle, &b0) && readByte(handle, &b1)
		  && readByte(handle, &b2)))
			return false;
		val = b0 | ((quint32)b1) << 8 | ((quint32)b2) << 16;
		return true;
	}

	bool readInt24(Handle &handle, qint32 &val) const
	{
		if (!readUInt24(handle, (quint32&)val))
			return false;
		if (val > 0x7FFFFF)
			val = (val & 0x7FFFFF) - 0x800000;
		return true;
	}

	bool readUInt32(Handle &handle, quint32 &val) const
	{
		quint8 b0, b1, b2, b3;
		if (!(readByte(handle, &b0) && readByte(handle, &b1)
		  && readByte(handle, &b2) && readByte(handle, &b3)))
			return false;
		val = b0 | ((quint32)b1) << 8 | ((quint32)b2) << 16
		  | ((quint32)b3) << 24;
		return true;
	}

	bool readInt32(Handle &handle, qint32 &val) const
	{
		return readUInt32(handle, (quint32&)val);
	}

	bool readVUInt32SW(Handle &hdl, quint32 bytes, quint32 &val) const
	{
		quint8 b;

		val = 0;
		for (quint32 i = bytes; i; i--) {
			if (!readByte(hdl, &b))
				return false;
			val |= ((quint32)b) << ((i-1) * 8);
		}

		return true;
	}

	bool readVUInt32(Handle &hdl, quint32 &val) const;
	bool readVUInt32(Handle &hdl, quint32 bytes, quint32 &val) const;
	bool readVBitfield32(Handle &hdl, quint32 &bitfield) const;

	const QString &fileName() const {return _path ? *_path : _img->fileName();}
	int blockSize() const
	{
		return _path ? 1U<<BLOCK_BITS : 1U<<_img->blockBits();
	}

protected:
	quint32 _gmpOffset;

private:
	const IMGData *_img;
	QVector<quint16> *_blocks;
	const QString *_path;
};

}

#endif // IMG_SUBFILE_H
