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
	Map *loadFile(const QString &path);
	const QString &errorString() const {return _errorString;}

private:
	RectC bounds(QXmlStreamReader &reader);
	Range zooms(QXmlStreamReader &reader);
	Map *map(QXmlStreamReader &reader);

	QString _errorString;
};

#endif // MAPSOURCE_H
