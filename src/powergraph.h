#ifndef POWERGRAPH_H
#define POWERGRAPH_H

#include "graphtab.h"

class PowerGraph : public GraphTab
{
	Q_OBJECT

public:
	PowerGraph(QWidget *parent = 0);

	QString label() const {return tr("Power");}
	void loadData(const Data &data, const QList<PathItem *> &paths);
	void clear();
	void setUnits(enum Units) {}
	void showTracks(bool show);
	void showRoutes(bool show) {Q_UNUSED(show);}

private:
	qreal avg() const;
	qreal max() const {return bounds().bottom();}
	void setInfo();

	QList<QPointF> _avg;

	enum Units _units;
	bool _showTracks;
};

#endif // POWERGRAPH_H
