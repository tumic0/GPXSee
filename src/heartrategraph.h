#ifndef HEARTRATEGRAPH_H
#define HEARTRATEGRAPH_H

#include "graphtab.h"

class HeartRateGraph : public GraphTab
{
	Q_OBJECT

public:
	HeartRateGraph(QWidget *parent = 0);

	QString label() const {return tr("Heart rate");}
	void loadData(const Data &data, const QList<PathItem *> &paths);
	void clear();
	void showTracks(bool show);

private:
	qreal avg() const;
	qreal max() const {return bounds().bottom();}
	void setInfo();

	QList<QPointF> _avg;

	bool _showTracks;
};

#endif // HEARTRATEGRAPH_H
