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

	void elevationGraph(QVector<QPointF> &graph) const;
	void speedGraph(QVector<QPointF> &graph) const;
	void track(QVector<QPointF> &track) const;
	qreal distance();
	qreal time();

private:
	Parser _parser;
	QVector<TrackPoint> _data;
	QString _error;
};

#endif // GPX_H
