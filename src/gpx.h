#ifndef GPX_H
#define GPX_H

#include <QVector>
#include <QList>
#include <QPointF>
#include <QString>
#include "parser.h"

class GPX
{
public:
	bool loadFile(const QString &fileName);
	const QString &errorString() const {return _error;}
	int errorLine() const {return _errorLine;}

	int count() const {return _data.count();}
	void elevationGraph(int i, QVector<QPointF> &graph) const;
	void speedGraph(int i, QVector<QPointF> &graph) const;
	void track(int i, QVector<QPointF> &track) const;
	qreal distance(int i) const;
	qreal time(int i) const;
	QDateTime date(int i) const;

private:
	Parser _parser;
	QList<QVector<TrackPoint> > _data;
	QString _error;
	int _errorLine;
};

#endif // GPX_H
