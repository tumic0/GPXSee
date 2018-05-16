#ifndef CADENCEGRAPH_H
#define CADENCEGRAPH_H

#include "graphtab.h"

class CadenceGraph : public GraphTab
{
	Q_OBJECT

public:
	CadenceGraph(QWidget *parent = 0);

	QString label() const {return tr("Cadence");}
	QList<GraphItem*> loadData(const Data &data);
	void clear();
	void showTracks(bool show);
	void showRoutes(bool show) {Q_UNUSED(show);}

private:
	qreal avg() const;
	qreal max() const {return bounds().bottom();}
	void setInfo();

	QList<QPointF> _avg;

	bool _showTracks;
};

#endif // CADENCEGRAPH_H
