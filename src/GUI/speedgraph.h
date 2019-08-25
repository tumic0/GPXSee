#ifndef SPEEDGRAPH_H
#define SPEEDGRAPH_H

#include <QList>
#include "graphtab.h"

class SpeedGraphItem;

class SpeedGraph : public GraphTab
{
	Q_OBJECT

public:
	SpeedGraph(QWidget *parent = 0);
	~SpeedGraph();

	QString label() const {return tr("Speed");}
	QList<GraphItem*> loadData(const Data &data);
	void clear();
	void setUnits(Units units);
	void setTimeType(TimeType type);
	void showTracks(bool show);

private:
	qreal avg() const;
	qreal max() const {return bounds().bottom();}
	void setYUnits();
	void setInfo();

	QVector<QPointF> _avg;
	QVector<QPointF> _mavg;

	Units _units;
	TimeType _timeType;

	bool _showTracks;
	QList<SpeedGraphItem *> _tracks;
};

#endif // SPEEDGRAPH_H
