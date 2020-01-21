#ifndef SUBFILE_H
#define SUBFILE_H

#include <QVector>
#include <QDebug>

class QFile;
class IMG;

class SubFile
{
public:
	enum Type {Unknown, TRE, RGN, LBL, NET, TYP, GMP};

	struct Handle
	{
		Handle() : blockNum(-1), blockPos(-1), pos(-1) {}

		QByteArray data;
		int blockNum;
		int blockPos;
		int pos;
	};

	SubFile(IMG *img)
	  : _gmpOffset(0), _img(img), _blocks(new QVector<quint16>()), _file(0) {}
	SubFile(SubFile *gmp, quint32 offset) : _gmpOffset(offset), _img(gmp->_img),
	  _blocks(gmp->_blocks), _file(gmp->_file) {}
	SubFile(QFile *file)
	  : _gmpOffset(0), _img(0), _blocks(0), _file(file) {}
	~SubFile()
	{
		if (!_gmpOffset)
			delete _blocks;
	}

	void addBlock(quint16 block) {_blocks->append(block);}

	bool seek(Handle &handle, quint32 pos) const;

	template<typename T>
	bool readUInt8(Handle &handle, T &val) const
	{
		quint8 b;
		if (!readByte(handle, b))
			return false;
		val = b;
		return true;
	}

	template<typename T>
	bool readUInt16(Handle &handle, T &val) const
	{
		quint8 b0, b1;
		if (!(readByte(handle, b0) && readByte(handle, b1)))
			return false;
		val = b0 | ((quint16)b1) << 8;
		return true;
	}

	bool readInt16(Handle &handle, qint16 &val) const
	{
		if (!readUInt16(handle, (quint16&)val))
			return false;
		if((quint16)val > 0x7FFF)
			val = (val & 0x7FFF) - 0x8000;
		return true;
	}

	bool readUInt24(Handle &handle, quint32 &val) const
	{
		quint8 b0, b1, b2;
		if (!(readByte(handle, b0) && readByte(handle, b1)
		  && readByte(handle, b2)))
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
		if (!(readByte(handle, b0) && readByte(handle, b1)
		  && readByte(handle, b2) && readByte(handle, b3)))
			return false;
		val = b0 | ((quint32)b1) << 8 | ((quint32)b2) << 16
		  | ((quint32)b3) << 24;
		return true;
	}

	bool readVUInt32(Handle &hdl, quint32 &val) const;
	bool readVUInt32SW(Handle &hdl, quint32 bytes, quint32 &val) const;
	bool readVBitfield32(Handle &hdl, quint32 &bitfield) const;

	quint16 offset() const {return _blocks->first();}
	QString fileName() const;

protected:
	quint32 _gmpOffset;

private:
	bool readByte(Handle &handle, quint8 &val) const;

	IMG *_img;
	QVector<quint16> *_blocks;
	QFile *_file;
};

#endif // SUBFILE_H
