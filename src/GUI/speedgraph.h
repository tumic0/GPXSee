#ifndef SPEEDGRAPH_H
#define SPEEDGRAPH_H

#include <QList>
#include "graphtab.h"

class SpeedGraph : public GraphTab
{
	Q_OBJECT

public:
	SpeedGraph(QWidget *parent = 0);

	QString label() const {return tr("Speed");}
	void loadData(const Data &data, const QList<PathItem *> &paths);
	void clear();
	void setUnits(Units units);
	void setTimeType(TimeType type);
	void showTracks(bool show);

private:
	qreal avg() const;
	qreal max() const {return bounds().bottom();}
	void setYUnits();
	void setInfo();

	QList<QPointF> _avg;
	QList<QPointF> _mavg;

	Units _units;
	TimeType _timeType;
	bool _showTracks;
};

#endif // SPEEDGRAPH_H
