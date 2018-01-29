#ifndef MAPSOURCE_H
#define MAPSOURCE_H

#include <QList>
#include "common/range.h"
#include "common/rectc.h"

class Map;
class QXmlStreamReader;

class MapSource
{
public:
	MapSource() : _map(0) {}
	~MapSource();

	bool loadFile(const QString &path);
	const QString &errorString() const {return _errorString;}

	Map* map() const {return _map;}

private:
	RectC bounds(QXmlStreamReader &reader);
	Range zooms(QXmlStreamReader &reader);
	void map(QXmlStreamReader &reader);

	QString _errorString;
	Map *_map;
};

#endif // MAPSOURCE_H
