#ifndef IMG_H
#define IMG_H

#include <QFile>
#include "mapdata.h"

class IMG : public MapData
{
public:
	IMG(const QString &fileName);

	QString fileName() const {return _file.fileName();}

private:
	friend class SubFile;

	unsigned blockBits() const {return _blockBits;}
	bool readBlock(int blockNum, char *data);
	qint64 read(char *data, qint64 maxSize);
	template<class T> bool readValue(T &val);

	QFile _file;
	quint8 _key;
	unsigned _blockBits;
};

#endif // IMG_H
