#ifndef SUBFILE_H
#define SUBFILE_H

#include <QVector>
#include <QDebug>

class IMG;
class QFile;

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

	SubFile(IMG *img, quint32 size) : _img(img), _file(0), _size(size) {}
	SubFile(QFile *file);

	void addBlock(quint16 block) {_blocks.append(block);}
	bool isValid() const;

	quint32 size() const;
	bool seek(Handle &handle, quint32 pos) const;
	bool readByte(Handle &handle, quint8 &val) const;

	bool readUInt16(Handle &handle, quint16 &val) const
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

	quint16 offset() const {return _blocks.first();}
	QString fileName() const;

	static Type type(const char str[3]);

	friend QDebug operator<<(QDebug dbg, const SubFile &file);

private:
	IMG *_img;
	QFile *_file;
	quint32 _size;
	QVector<quint16> _blocks;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const SubFile &file);
#endif // QT_NO_DEBUG

#endif // SUBFILE_H
