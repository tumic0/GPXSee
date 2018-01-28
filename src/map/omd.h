#ifndef OMD_H
#define OMD_H

#include <QList>
#include "common/range.h"
#include "common/rectc.h"

class Map;
class QXmlStreamReader;

class OMD
{
public:
	~OMD();

	bool loadFile(const QString &path);
	const QString &errorString() const {return _errorString;}

	const QList<Map*> &maps() const {return _maps;}

private:
	RectC bounds(QXmlStreamReader &reader);
	Range zooms(QXmlStreamReader &reader);
	void map(QXmlStreamReader &reader);
	void omd(QXmlStreamReader &reader);

	QString _errorString;
	QList<Map*> _maps;
};

#endif // OMD_H
