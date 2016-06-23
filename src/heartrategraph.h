#ifndef HEARTRATEGRAPH_H
#define HEARTRATEGRAPH_H

#include "graphtab.h"

class GPX;

class HeartRateGraph : public GraphTab
{
	Q_OBJECT

public:
	HeartRateGraph(QWidget *parent = 0);

	QString label() const {return tr("Heart rate");}
	void loadGPX(const GPX &gpx);
	void clear();
	void setUnits(enum Units units);

private:
	qreal avg() const;
	qreal max() const {return bounds().bottom();}
	void setXUnits();
	void addInfo();

	QList<QPointF> _avg;
	enum Units _units;
};

#endif // HEARTRATEGRAPH_H
