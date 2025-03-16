#ifndef SPEEDGRAPH_H
#define SPEEDGRAPH_H

#include <QList>
#include "graphtab.h"

class SpeedGraphItem;
class Track;

class SpeedGraph : public GraphTab
{
	Q_OBJECT

public:
	SpeedGraph(QWidget *parent = 0);
	~SpeedGraph();

	QString label() const {return tr("Speed");}
	QList<GraphItem*> loadData(const Data &data, Map *map);
	void clear();
	void setUnits(Units units);
	void setTimeType(TimeType type);
	void showTracks(bool show);

private:
	GraphItem *loadGraph(const Graph &graph, const Track &track,
	  const QColor &color, bool primary);
	qreal avg() const;
	qreal max() const;
	void setYUnits();
	void setInfo();

	QVector<QPointF> _avg;
	QVector<QPointF> _mavg;
	QVector<QPointF> _max;

	Units _units;
	TimeType _timeType;

	bool _showTracks;
	QList<SpeedGraphItem *> _tracks;
};

#endif // SPEEDGRAPH_H
