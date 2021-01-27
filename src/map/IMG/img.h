#ifndef IMG_H
#define IMG_H

#include "mapdata.h"

class QFile;

class IMG : public MapData
{
public:
	IMG(const QString &fileName);

	const QString &fileName() const {return _fileName;}

private:
	friend class SubFile;

	unsigned blockBits() const {return _blockBits;}
	bool readBlock(QFile &file, int blockNum, char *data);
	qint64 read(QFile &file, char *data, qint64 maxSize);
	template<class T> bool readValue(QFile &file, T &val);

	QString _fileName;
	quint8 _key;
	unsigned _blockBits;
};

#endif // IMG_H
