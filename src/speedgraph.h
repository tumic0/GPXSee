#ifndef SPEEDGRAPH_H
#define SPEEDGRAPH_H

#include <QList>
#include "graphtab.h"

class GPX;

class SpeedGraph : public GraphTab
{
	Q_OBJECT

public:
	SpeedGraph(QWidget *parent = 0);

	QString label() const {return tr("Speed");}
	void loadGPX(const GPX &gpx);
	void clear();
	void setUnits(enum Units units);
	void showTracks(bool show);
	void showRoutes(bool show) {Q_UNUSED(show);}

private:
	qreal avg() const;
	qreal max() const {return bounds().bottom();}
	void setXUnits();
	void setYUnits();
	void setInfo();

	QList<QPointF> _avg;
	enum Units _units;
};

#endif // SPEEDGRAPH_H
