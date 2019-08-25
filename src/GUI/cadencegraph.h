#ifndef CADENCEGRAPH_H
#define CADENCEGRAPH_H

#include "graphtab.h"

class CadenceGraphItem;

class CadenceGraph : public GraphTab
{
	Q_OBJECT

public:
	CadenceGraph(QWidget *parent = 0);
	~CadenceGraph();

	QString label() const {return tr("Cadence");}
	QList<GraphItem*> loadData(const Data &data);
	void clear();
	void showTracks(bool show);

private:
	qreal avg() const;
	qreal max() const {return bounds().bottom();}
	void setInfo();

	QVector<QPointF> _avg;

	bool _showTracks;
	QList<CadenceGraphItem *> _tracks;
};

#endif // CADENCEGRAPH_H
