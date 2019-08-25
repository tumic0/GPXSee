#ifndef POWERGRAPH_H
#define POWERGRAPH_H

#include "graphtab.h"

class PowerGraphItem;

class PowerGraph : public GraphTab
{
	Q_OBJECT

public:
	PowerGraph(QWidget *parent = 0);
	~PowerGraph();

	QString label() const {return tr("Power");}
	QList<GraphItem*> loadData(const Data &data);
	void clear();
	void showTracks(bool show);

private:
	qreal avg() const;
	qreal max() const {return bounds().bottom();}
	void setInfo();

	QVector<QPointF> _avg;

	bool _showTracks;
	QList<PowerGraphItem*> _tracks;
};

#endif // POWERGRAPH_H
