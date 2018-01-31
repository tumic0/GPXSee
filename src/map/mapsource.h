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
	bool loadFile(const QString &path, Map **map);
	const QString &errorString() const {return _errorString;}

private:
	RectC bounds(QXmlStreamReader &reader);
	Range zooms(QXmlStreamReader &reader);
	void map(QXmlStreamReader &reader, Map **map);

	QString _errorString;
};

#endif // MAPSOURCE_H
