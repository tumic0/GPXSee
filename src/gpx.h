#ifndef GPX_H
#define GPX_H

#include <QVector>
#include <QPointF>
#include <QString>
#include "parser.h"

class GPX
{
public:
	bool loadFile(const QString &fileName);
	const QString &errorString() const {return _error;}
	QVector<QPointF> elevationGraph() const;
	QVector<QPointF> speedGraph() const;
	QVector<QPointF> track() const;

private:
	Parser _parser;
	QVector<TrackPoint> _data;
	QString _error;
};

#endif // GPX_H
