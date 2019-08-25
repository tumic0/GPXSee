#ifndef GEARRATIOGRAPH_H
#define GEARRATIOGRAPH_H

#include <QMap>
#include "graphtab.h"

class GearRatioGraphItem;

class GearRatioGraph : public GraphTab
{
	Q_OBJECT

public:
	GearRatioGraph(QWidget *parent = 0);
	~GearRatioGraph();

	QString label() const {return tr("Gear ratio");}
	QList<GraphItem*> loadData(const Data &data);
	void clear();
	void showTracks(bool show);

private:
	qreal top() const;
	qreal min() const {return bounds().top();}
	qreal max() const {return bounds().bottom();}
	void setInfo();

	QMap<qreal, qreal> _map;

	bool _showTracks;
	QList<GearRatioGraphItem*> _tracks;
};

#endif // GEARRATIOGRAPH_H
