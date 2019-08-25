#ifndef HEARTRATEGRAPH_H
#define HEARTRATEGRAPH_H

#include "graphtab.h"

class HeartRateGraphItem;

class HeartRateGraph : public GraphTab
{
	Q_OBJECT

public:
	HeartRateGraph(QWidget *parent = 0);
	~HeartRateGraph();

	QString label() const {return tr("Heart rate");}
	QList<GraphItem*> loadData(const Data &data);
	void clear();
	void showTracks(bool show);

private:
	qreal avg() const;
	qreal max() const {return bounds().bottom();}
	void setInfo();

	QVector<QPointF> _avg;

	bool _showTracks;
	QList<HeartRateGraphItem*> _tracks;
};

#endif // HEARTRATEGRAPH_H
